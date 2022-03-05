#include <vector>

#include <my_metadata_args.h>
#include <client_timing_constants_read.hh>
#include <testing_harness_debug_helper_functions.hh>

#include <hdf5_hl.h>
#include <hdf5.h>

using namespace std;

extern void add_timing_point(int catg);

extern debugLog extreme_debug_log, testing_log;
bool do_debug_testing = false;
	// open_var(timestep_entry.file_id, var_names[var_indx], var_version, var_id);


static void convert_dim_bounds_to_hyperslab_and_vol(const vector<md_dim_bounds> &dims, hsize_t *count, hsize_t *offset, 
	hsize_t *stride, hsize_t *block, uint64_t &chunk_vol);
void convert_dim_bounds_to_counts(const vector<md_dim_bounds> &dims, hsize_t *counts);

void hdf5_open_var(hid_t timestep_file_id, string var_name, uint32_t var_version, hid_t &var_id);
void find_data_range(const md_catalog_var_entry &var, hid_t var_data_space, int num_dims, const vector<double> &data_vect);

void hdf5_read_hyperslab(const md_catalog_var_entry &var,
		hid_t var_id, hid_t var_data_space, const vector<md_dim_bounds> &chunk_dims);

void print_md_dim_bounds( const vector<md_dim_bounds> &dims);
bool dims_overlap(const vector<md_dim_bounds> &attr_dims, const vector<md_dim_bounds> &proc_dims);



void hdf5_open_var(hid_t timestep_file_id, string var_name, uint32_t var_version, hid_t &var_id)
{
	string VARNAME = var_name + to_string(var_version);

	extreme_debug_log << "about to open var: " << VARNAME << endl;

  	var_id = H5Dopen(timestep_file_id, VARNAME.c_str(),H5P_DEFAULT); assert(var_id >= 0);

	extreme_debug_log << "finished opening var. " << endl;
}


void hdf5_close_timestep_file(hid_t timestep_file_id)
{
    herr_t status = H5Fclose(timestep_file_id); assert(status >= 0);
}

void hdf5_close_var(hid_t var_id)
{
    herr_t status = H5Dclose(var_id); assert(status >= 0);
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


static void hdf5_open_timestep_file_collectively_for_read(string FILENAME, hid_t &file_id)
{
	/*
    * Set up file access property list with parallel I/O access
    */
    //create a property list for file access
	hid_t property_list_id = H5Pcreate(H5P_FILE_ACCESS); assert(property_list_id >= 0);

    herr_t status = H5Pset_fapl_mpio(property_list_id, MPI_COMM_WORLD, MPI_INFO_NULL); assert(status >= 0);

    file_id = H5Fopen(FILENAME.c_str(), H5F_ACC_RDONLY, property_list_id); assert(file_id >= 0);

    status = H5Pclose(property_list_id); assert(status >= 0);

}

void hdf5_open_timestep_file_collectively_for_read(const string &run_name, uint64_t job_id, uint64_t timestep_id, hid_t &file_id)
{
	string FILENAME = run_name + "/" + to_string(job_id) + "/" + to_string(timestep_id);
	extreme_debug_log << "FILENAME: " << FILENAME << endl;

    hdf5_open_timestep_file_collectively_for_read(FILENAME, file_id);
}

hid_t hdf5_get_dataspace(hid_t var_id)
{
	hid_t var_data_space = H5Dget_space(var_id); assert(var_data_space >= 0);
	return var_data_space;
}

void hdf5_close_dataspace(hid_t var_data_space)
{
	herr_t status = H5Sclose(var_data_space); assert(status >= 0); 
}

void hdf5_read_hyperslab(hid_t timestep_file_id, const md_catalog_var_entry &var, const vector<md_dim_bounds> &chunk_dims )
{
	// add_timing_point(HDF5_READ_HYPERSLAB_START);

	hid_t var_data_space, var_id;
	herr_t status;


	hdf5_open_var(timestep_file_id, var.name, var.version, var_id);

    //get the storage layout of the var
    var_data_space = H5Dget_space(var_id); assert(var_data_space >= 0);

	hdf5_read_hyperslab(var, var_id, var_data_space, chunk_dims);

	// Close/release resources
    status = H5Sclose(var_data_space); assert(status >= 0); 
    status = H5Dclose(var_id); assert(status >= 0);
	// add_timing_point(HDF5_READ_HYPERSLAB_DONE);	
}

void hdf5_read_hyperslab(const md_catalog_var_entry &var,
		hid_t var_id, hid_t var_data_space, const vector<md_dim_bounds> &chunk_dims)
{

	herr_t status;
	hid_t chunk_data_space, property_list_id;
	uint32_t num_dims = chunk_dims.size();

	hsize_t	chunk_offset[chunk_dims.size()];
	hsize_t chunk_stride[chunk_dims.size()];
	hsize_t chunk_count[chunk_dims.size()];
	hsize_t chunk_block[chunk_dims.size()];
    uint64_t chunk_vol;

	convert_dim_bounds_to_hyperslab_and_vol(chunk_dims, chunk_count, chunk_offset, chunk_stride, chunk_block, chunk_vol);

	extreme_debug_log << "chunk_counts: " << chunk_count[0] << ", " << chunk_count[1] << ", " << chunk_count[2] << endl;

	vector<double> data_vct(chunk_vol);
	
	//select which hyperslab in storage to read
 	status = H5Sselect_hyperslab(var_data_space, H5S_SELECT_SET, chunk_offset, chunk_stride, chunk_count, chunk_block); assert(status >= 0);

 	//maintain the stored layout out the data reading it back in
	chunk_data_space  = H5Screate_simple(num_dims, chunk_count, NULL); assert(chunk_data_space >= 0);

    // Create property list for collective dataset read
    property_list_id = H5Pcreate(H5P_DATASET_XFER); assert(property_list_id >= 0);
    status = H5Pset_dxpl_mpio(property_list_id, H5FD_MPIO_COLLECTIVE); assert(status >= 0);

	status = H5Dread (var_id, H5T_NATIVE_DOUBLE, chunk_data_space, var_data_space,
               property_list_id, &data_vct[0]); assert(status >= 0);

    // Close/release resources
    status = H5Sclose(chunk_data_space); assert(status >= 0);
 	status = H5Pclose(property_list_id); assert(status >= 0);

 	if(do_debug_testing) {
   		find_data_range(var, var_data_space, num_dims, data_vct);
   	}
}

void find_data_range(const md_catalog_var_entry &var, hid_t var_data_space, int num_dims, const vector<double> &data_vect) 
{

	hsize_t var_dims[num_dims];
	herr_t status = H5Sget_simple_extent_dims(var_data_space, var_dims, NULL);
	uint64_t ny = var_dims[1];
	uint64_t nz = var_dims[2];

	double first_val = data_vect.at(0);
	double last_val = data_vect.at(data_vect.size()-1);
	extreme_debug_log << "first val: " << first_val << " last val: " << last_val << endl;

    int number = ny;

    int y_digits = 0;
    while (number > 0) {
        number /= 10;
        y_digits++;
    }
    y_digits++;

    int z_digits = 0;
    int number2 = nz;

    while (number2 > 0) {
        number2 /= 10;
        z_digits++;
    }
    z_digits++;


    uint64_t x1 = first_val / pow(10.0, y_digits) - 1;
    uint64_t x2 = last_val / pow(10.0, y_digits) - 1;
    uint64_t y1 = first_val - (x1+1)* pow(10.0, y_digits) -1;
    uint64_t y2 = last_val - (x2+1)* pow(10.0, y_digits) -1;

    extreme_debug_log << "ny: " << ny << " first_val-1: " << first_val-1 << " (first_val-1) mod ny: " << (int)(first_val-1) % ny << endl;

    if(num_dims == 3) {
        uint64_t z1 = round((first_val - (int)first_val)*pow(10.0, z_digits-1) -1);
        uint64_t z2 = round((last_val - (int)last_val)*pow(10.0, z_digits-1) -1);
        printf("data range for %s%d is: (%d, %d, %d),(%d, %d, %d)\n",var.name.c_str(),var.version,x1,y1,z1,x2,y2,z2);  
    }
    else if(num_dims == 2) {
        printf("data range for %s%d is: (%d, %d),(%d, %d)\n",var.name.c_str(),var.version,x1,y1,x2,y2);              
    }
}

void get_overlapping_dims_for_bb(const vector<md_dim_bounds> &proc_dims, vector<md_dim_bounds> &bounding_box)
{
    for(int i = 0; i < bounding_box.size(); i++) {
        bounding_box[i].min = max( bounding_box[i].min, proc_dims[i].min );
        bounding_box[i].max = min( bounding_box[i].max, proc_dims[i].max );
    }
}


static void read_attr(hid_t timestep_file_id, const md_catalog_var_entry &var, const md_catalog_var_attribute_entry &attr)
{
	add_timing_point(READ_DATA_FOR_ATTR_START);

	herr_t status;

	extreme_debug_log << "reading attr" << endl;

	uint64_t attr_vol = 0;
	hid_t var_data_space, var_id, attr_data_space;

	hsize_t stride[attr.num_dims];
	hsize_t block[attr.num_dims];
	hsize_t offset[attr.num_dims];
	hsize_t count[attr.num_dims];

	string VARNAME = var.name + to_string(var.version);

	extreme_debug_log << "VARNAME: " << VARNAME << endl;

	convert_dim_bounds_to_hyperslab_and_vol(attr.dims, count, offset, stride, block, attr_vol);

	extreme_debug_log << "count: " << count[0] << ", " << count[1] << ", " << count[2] << endl;
	extreme_debug_log << "offset: " << offset[0] << ", " << offset[1] << ", " << offset[2] << endl;
	extreme_debug_log << "attr_vol: " << attr_vol << endl;

	vector<double> data_vct(attr_vol);


    var_id = H5Dopen(timestep_file_id, VARNAME.c_str(), H5P_DEFAULT); assert(var_id >= 0);

    //get the storage layout of the var
    var_data_space = H5Dget_space(var_id); assert(var_data_space >= 0);

	//select which hyperslab in storage to read
 	status = H5Sselect_hyperslab(var_data_space, H5S_SELECT_SET, offset, stride, count, block); assert(status >= 0);

 	//maintain the stored layout out the data reading it back in
	attr_data_space  = H5Screate_simple(attr.num_dims, count, NULL); assert(attr_data_space >= 0);

    // // Create property list for collective dataset read
    // property_list_id = H5Pcreate(H5P_DATASET_XFER); assert(property_list_id >= 0);
    // status = H5Pset_dxpl_mpio(property_list_id, H5FD_MPIO_COLLECTIVE); assert(status >= 0);

	status = H5Dread (var_id, H5T_NATIVE_DOUBLE, attr_data_space, var_data_space,
               H5P_DEFAULT, &data_vct[0]); assert(status >= 0);

    // Close/release resources
    status = H5Sclose(attr_data_space); assert(status >= 0);
 	// status = H5Pclose(property_list_id); assert(status >= 0);

 	if(do_debug_testing) {
   		find_data_range(var, var_data_space, attr.dims.size(), data_vct);
   	}

    status = H5Dclose(var_id); assert(status >= 0);
    status = H5Sclose(var_data_space); assert(status >= 0); 	

	add_timing_point(READ_DATA_FOR_ATTR_DONE);
}

void read_data_for_attrs(const md_catalog_run_entry &run, const md_catalog_var_entry &var, 
				const vector<md_catalog_var_attribute_entry> &attrs, const vector<md_dim_bounds> &proc_dims)
{
    add_timing_point(HDF5_READ_DATA_FOR_ATTRS_START);

	string timestep_file_name = run.name + "/" + to_string(run.job_id) + "/" + to_string(var.timestep_id);

   	hid_t timestep_file_id;
   	hdf5_open_timestep_file_collectively_for_read(timestep_file_name, timestep_file_id);

    for (md_catalog_var_attribute_entry attr : attrs) {
   		if (dims_overlap ( attr.dims, proc_dims) ) {
      		get_overlapping_dims_for_bb ( proc_dims, attr.dims);
   			read_attr(timestep_file_id, var, attr);
   			if (do_debug_testing) {
   				testing_log << "attr data range: ";
				print_md_dim_bounds( attr.dims);
   				// testing_log << endl;
   			}
   		}
   	}
        // testing_log << endl;
   		// cout << "rank about to close timestep file" << endl;
   	hdf5_close_timestep_file(timestep_file_id);
   		// cout << "rank closed timestep file" << endl;
    add_timing_point(HDF5_READ_DATA_FOR_ATTRS_DONE);


}

void read_data_for_attrs(const md_catalog_run_entry &run, const vector<md_catalog_var_entry> &vars, 
				const vector<md_catalog_var_attribute_entry> &attrs, const vector<md_dim_bounds> &proc_dims)
{
    add_timing_point(HDF5_READ_DATA_FOR_ATTRS_START);

	string timestep_file_name = run.name + "/" + to_string(run.job_id) + "/" + to_string(vars.at(0).timestep_id);


   	hid_t timestep_file_id;
   	hdf5_open_timestep_file_collectively_for_read(timestep_file_name, timestep_file_id);

    for (md_catalog_var_attribute_entry attr : attrs) {
   		if (dims_overlap ( attr.dims, proc_dims) ) {
   			md_catalog_var_entry var = vars.at(attr.var_id);
      		get_overlapping_dims_for_bb ( proc_dims, attr.dims);
   			read_attr(timestep_file_id, var, attr);
   			if (do_debug_testing) {
   				testing_log << "attr data range: ";
				print_md_dim_bounds( attr.dims);
   				// testing_log << endl;
   			}
   		}
   	}
        // testing_log << endl;
   		// cout << "rank about to close timestep file" << endl;
   	hdf5_close_timestep_file(timestep_file_id);
   		// cout << "rank closed timestep file" << endl;
    add_timing_point(HDF5_READ_DATA_FOR_ATTRS_DONE);

}

