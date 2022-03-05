#include <vector>

#include <my_metadata_args.h>
#include <client_timing_constants_write.hh>
#include <testing_harness_debug_helper_functions.hh>

#include <hdf5_hl.h>
#include <hdf5.h>

using namespace std;

extern void add_timing_point(int catg);

// static bool extreme_debug_logging = true;
// static bool debug_logging = false;
// static bool error_logging = true;
// static bool testing_logging = false;

// debugLog error_log = debugLog(error_logging);
// debugLog debug_log = debugLog(debug_logging);
// static debugLog extreme_debug_log = debugLog(extreme_debug_logging);
// debugLog testing_log = debugLog(testing_logging);


extern debugLog extreme_debug_log;

	// open_var(timestep_entry.file_id, var_names[var_indx], var_version, var_id);


static void convert_dim_bounds_to_hyperslab_and_vol(const vector<md_dim_bounds> &dims, hsize_t *count, hsize_t *offset, 
	hsize_t *stride, hsize_t *block, uint64_t &chunk_vol);
void convert_dim_bounds_to_counts(const vector<md_dim_bounds> &dims, hsize_t *counts);
static void hdf5_create_and_close_chunked_var(uint32_t num_dims, hsize_t *var_counts,
					hsize_t *chunk_counts, hid_t file_id, const string  var_name
					);
void hdf5_open_var(hid_t timestep_file_id, string var_name, uint32_t var_version, hid_t &var_id);



void hdf5_open_var(hid_t timestep_file_id, string var_name, uint32_t var_version, hid_t &var_id)
{
	string VARNAME = var_name + to_string(var_version);

	extreme_debug_log << "about to open var: " << VARNAME << endl;

  	var_id = H5Dopen(timestep_file_id, VARNAME.c_str(),H5P_DEFAULT); assert(var_id >= 0);

	extreme_debug_log << "finished opening var. " << endl;
}

void hdf5_write_chunk_data(hid_t timestep_file_id, const md_catalog_var_entry &var, const vector<md_dim_bounds> &chunk_dims,
						int rank, const std::vector<double> &data_vct
						)
{
    add_timing_point(WRITE_CHUNK_DATA_START);

	hid_t var_id;
	hsize_t	chunk_count[chunk_dims.size()];
	hsize_t chunk_offset[chunk_dims.size()];
	hsize_t chunk_stride[chunk_dims.size()];
	hsize_t chunk_block [chunk_dims.size()]; 
	uint64_t chunk_vol;

	extreme_debug_log << "starting hdf5_write_chunk_data" << endl;
	// hsize_t *var_counts;

	convert_dim_bounds_to_hyperslab_and_vol (chunk_dims, chunk_count, chunk_offset, chunk_stride, chunk_block, chunk_vol);
	// data_vct.resize(chunk_vol);

	// convert_dim_bounds_to_counts(var.dims, var_counts);

	extreme_debug_log << "got hyperslab. about to open var" << endl;


	hdf5_open_var(timestep_file_id, var.name, var.version, var_id);


  	hid_t var_data_space = H5Dget_space( var_id ); assert(var_data_space >= 0);


    hid_t chunk_data_space  = H5Screate_simple(chunk_dims.size(), chunk_count, NULL); assert(chunk_data_space >= 0);

    /*
     * Select hyperslab in the file.
     */
    herr_t status = H5Sselect_hyperslab(var_data_space, H5S_SELECT_SET, chunk_offset, chunk_stride, chunk_count, chunk_block); assert(status >= 0);

 //    /*
 //     * Initialize data buffer 
 //     */
 //    add_timing_point(CREATE_DATA_START);

    //note - have decided to just generate the data once per run
	// generate_data_for_proc(var, rank, chunk_dims, data_vct, chunk_count[0], chunk_count[1],
	// 					chunk_count[2], chunk_vol);


 //    add_timing_point(CREATE_DATA_DONE);


    // extreme_debug_log << "data_vct.size(): " << data_vct.size() << endl;
    // if(data_vct.size() > 0) {
   	//     extreme_debug_log << "data_vct[0]: " << data_vct[0] << endl; 	
    // }

    /*
     * Create property list for collective dataset write.
     */
    //create a property list for dataset creation
    hid_t property_list_id = H5Pcreate(H5P_DATASET_XFER); assert(property_list_id >= 0);
    status = H5Pset_dxpl_mpio(property_list_id, H5FD_MPIO_COLLECTIVE); assert(status >= 0);
    
   	/*
     * Write a subset of data to the dataset
    */
    status = H5Dwrite(var_id, H5T_NATIVE_DOUBLE, chunk_data_space, var_data_space,
		      property_list_id, &data_vct[0]); assert(status >= 0);

    extreme_debug_log << "H5Dwrite status: " << status << endl;
   	status = H5Pclose(property_list_id); assert(status >= 0);
    status = H5Sclose(chunk_data_space); assert(status >= 0);
    status = H5Sclose(var_data_space); assert(status >= 0);
	status = H5Dclose(var_id); assert(status >= 0);

    add_timing_point(WRITE_CHUNK_DATA_DONE);
}

 

void hdf5_create_timestep_and_vars(string run_name, uint64_t job_id, uint64_t timestep_id, 
					uint32_t num_vars, hsize_t nx, hsize_t ny, hsize_t nz,
					const vector<md_dim_bounds> &chunk_dims)
{
    add_timing_point(HDF5_CREATE_TIMESTEP_START);

	hsize_t var_counts[] = {nx, ny, nz};
	hsize_t chunk_counts[chunk_dims.size()];

    char var_names[10][11] = {"temperat", "temperat", "pressure", "pressure", "velocity_x", "velocity_y", "velocity_z", "magn_field", "energy_var", "density_vr"};  

    uint32_t version1 = 1;
    uint32_t version2 = 2;


	convert_dim_bounds_to_counts(chunk_dims, chunk_counts);

	string FILENAME = run_name + "/" + to_string(job_id) + "/" + to_string(timestep_id);
	extreme_debug_log << "just created FILENAME: " << FILENAME << endl;

    hid_t timestep_file_id = H5Fcreate(FILENAME.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT); assert(timestep_file_id >= 0);

    add_timing_point(HDF5_CREATE_TIMESTEP_DONE);

	for (int var_indx = 0; var_indx < num_vars; var_indx++) {
		uint32_t var_version;
    	if(var_indx == 1 || var_indx == 3) { //these have been designated ver2
    		var_version = version2;
    	}
    	else {
    		var_version = version1;
    	}

		string VARNAME = var_names[var_indx] + to_string(var_version);
		extreme_debug_log << "VARNAME: " << VARNAME << endl;

		hdf5_create_and_close_chunked_var(chunk_dims.size(), var_counts, chunk_counts,
			timestep_file_id, VARNAME);
	}

    herr_t status = H5Fclose(timestep_file_id); assert(status >= 0);
    
    add_timing_point(HDF5_CREATE_VARS_DONE);

}



void hdf5_open_timestep_file_collectively_for_write(const string &run_name, uint64_t job_id, uint64_t timestep_id, hid_t &file_id)
{
    add_timing_point(HDF5_OPEN_TIMESTEP_COLLECTIVELY_START);

	string FILENAME = run_name + "/" + to_string(job_id) + "/" + to_string(timestep_id);
	extreme_debug_log << "FILENAME: " << FILENAME << endl;

	hid_t property_list_id = H5Pcreate(H5P_FILE_ACCESS); assert(property_list_id >= 0);

    herr_t status = H5Pset_fapl_mpio(property_list_id, MPI_COMM_WORLD, MPI_INFO_NULL); assert(status >= 0);

    file_id = H5Fopen(FILENAME.c_str(), H5F_ACC_RDWR, property_list_id); assert(file_id >= 0);

    status = H5Pclose(property_list_id); assert(status >= 0);

    add_timing_point(HDF5_OPEN_TIMESTEP_COLLECTIVELY_DONE);
}



static void hdf5_create_and_close_chunked_var(uint32_t num_dims, hsize_t *var_counts,
					hsize_t *chunk_counts, hid_t file_id, const string  var_name
					) 
{
	add_timing_point(HDF5_CREATE_CHUNKED_VAR_START);

	extreme_debug_log << "starting to create chunked var" << endl;


	// hsize_t var_counts[num_dims], chunk_counts[num_dims];

	// convert_dim_bounds_to_counts(var_dims, var_counts);
	// convert_dim_bounds_to_counts(chunk_dims, chunk_counts);


	extreme_debug_log << "var_counts: " << var_counts[0] << ", " << var_counts[1] << ", " << var_counts[2] << endl;
	extreme_debug_log << "chunk_counts: " << chunk_counts[0] << ", " << chunk_counts[1] << ", " << chunk_counts[2] << endl;

	/*
     * Create the var and chunk data space 
     */
    hid_t var_data_space = H5Screate_simple(num_dims, var_counts, NULL); assert(var_data_space >= 0);

    /*
     * Create chunked dataset.
     */
    //create a property list for dataset creation
    hid_t property_list_id = H5Pcreate(H5P_DATASET_CREATE); assert(property_list_id >= 0);

    //add to the property list the size of a chunk
    herr_t status = H5Pset_chunk(property_list_id, num_dims, chunk_counts); assert(status >= 0);

	extreme_debug_log << "about to create dataset" << endl;

    hid_t var_id = H5Dcreate(file_id, var_name.c_str(), H5T_NATIVE_DOUBLE, var_data_space,
			H5P_DEFAULT, property_list_id, H5P_DEFAULT);  assert(var_id >= 0);

	extreme_debug_log << "done creating dataset" << endl;

    status = H5Dclose(var_id); assert(status >= 0);
    status = H5Pclose(property_list_id); assert(status >= 0);
    status = H5Sclose(var_data_space); assert(status >= 0);

	add_timing_point(HDF5_CREATE_CHUNKED_VAR_DONE);
}

void hdf5_close_timestep_file(hid_t timestep_file_id)
{
    herr_t status = H5Fclose(timestep_file_id); assert(status >= 0);
}


static void convert_dim_bounds_to_hyperslab_and_vol(const vector<md_dim_bounds> &dims, 
	hsize_t *count, hsize_t *offset, hsize_t *stride, hsize_t *block, uint64_t &chunk_vol)
{	
	uint32_t num_dims = dims.size();

	if(num_dims >= 1) {
		stride[0] = 1;
		block[0] = 1;
		offset[0] = dims[0].min;
		count[0] = dims[0].max - dims[0].min + 1;
		chunk_vol = count[0];
		if(num_dims >= 2) {
			stride[1] = 1;
			block[1] = 1;
			offset[1] = dims[1].min;
			count[1] = dims[1].max - dims[1].min + 1;
			chunk_vol *= count[1];
			if(num_dims >= 2) {
				stride[2] = 1;
				block[2] = 1;
				offset[2] = dims[2].min;
				count[2] = dims[2].max - dims[2].min + 1;	
				chunk_vol *= count[2];
			}	
		}
	}
}


void convert_dim_bounds_to_counts(const vector<md_dim_bounds> &dims, hsize_t *counts)
{	
	uint32_t num_dims = dims.size();

	if(num_dims >= 1) {
		counts[0] = dims[0].max - dims[0].min + 1;
		if(num_dims >= 2) {
			counts[1] = dims[1].max - dims[1].min + 1;
			if(num_dims >= 2) {
				counts[2] = dims[2].max - dims[2].min + 1;	
			}	
		} 
	}
}

