/* 
 * Copyright 2018 National Technology & Engineering Solutions of
 * Sandia, LLC (NTESS). Under the terms of Contract DE-NA0003525 with NTESS,
 * the U.S. Government retains certain rights in this software.
 *
 * The MIT License (MIT)
 * 
 * Copyright (c) 2018 Sandia Corporation
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */


#include <stdlib.h> //needed for atoi/atol/malloc/free/rand
#include <stdint.h> //needed for uint
#include <vector>
#include <stdio.h> //needed for printf
#include <assert.h> //needed for assert
#include <string.h> //needed for strcmp
// #include <chrono> //needed for high_resolution_clock
#include <math.h> //needed for pow()
#include <sys/time.h> //needed for timeval
#include <float.h> //needed for DBL_MAX
#include <numeric> //needed for accumulate


#include <mpi.h>
#include <hdf5_hl.h>
#include <hdf5.h>

// #include <testing_harness_debug_helper_functions.hh> 
#include <my_metadata_client_hdf5.hh>
#include <client_timing_constants_write_hdf5.hh>
#include <testing_harness_helper_functions_hdf5.hh>
// #include <testing_harness_hdf5.hh>


using namespace std;

bool extreme_debug_logging = false;
bool debug_logging = false;
bool error_logging = true;
bool testing_logging = false;
bool zero_rank_logging = false;

// static debugLog error_log = debugLog(error_logging, zero_rank_logging);
// static debugLog debug_log = debugLog(debug_logging, zero_rank_logging);
// static debugLog extreme_debug_log = debugLog(extreme_debug_logging, false);
// static debugLog testing_log = debugLog(testing_logging, zero_rank_logging);

debugLog error_log = debugLog(error_logging, zero_rank_logging);
debugLog debug_log = debugLog(debug_logging, zero_rank_logging);
debugLog extreme_debug_log = debugLog(extreme_debug_logging, false);
debugLog testing_log = debugLog(testing_logging, zero_rank_logging);

static bool output_timing = true;
bool do_debug_testing = false;

static bool write_data = true;
bool read_data = false; //ignore, just since using one helper function file for both reading and writing


std::vector<long double> time_pts;
std::vector<int> catg_of_time_pts;


int zero_time_sec;

void add_timing_point(int catg) {
    if (output_timing) {
        struct timeval now;
        gettimeofday(&now, NULL);
        // cout << "time_pts.push_back: " << ((now.tv_sec - zero_time_sec) + now.tv_usec / 1000000.0) << endl;
        time_pts.push_back( (now.tv_sec - zero_time_sec) + now.tv_usec / 1000000.0);
        catg_of_time_pts.push_back(catg);
    }
}


// static void create_file(const string &run_name, string job_id, uint64_t timestep_id, hid_t &file_id);
// static void open_file_indv(const string &run_name, string job_id, uint64_t timestep_id, hid_t &file_id);


static void write_chunk_data(hid_t var_id, uint32_t num_dims, hsize_t *offset, 
						hsize_t *chunk_dims, uint32_t total_y_length, uint32_t total_z_length, 
						int rank, vector<double> &data_vct
						);

void create_var_attrs(int rank, md_dim_bounds proc_dims, int timestep_num, int var_indx, string VARNAME, uint32_t num_types, 
		bool write_data, vector<var_attribute_str> &attrs,
		double &timestep_temp_max, double &timestep_temp_min);

void create_timestep_attrs(hid_t timestep_file_id, int rank, int timestep_num, uint32_t num_client_procs, 
	double timestep_temp_max, double timestep_temp_min, char *max_type_name, char *min_type_name,
	double *all_timestep_temp_maxes_for_all_procs, double *all_timestep_temp_mins_for_all_procs);

void create_run_attrs(string run_name, string job_id, uint32_t num_timesteps, double *all_timestep_temp_maxes_for_all_procs, 
	double *all_timestep_temp_mins_for_all_procs, char *max_type_name, char *min_type_name);

void do_cleanup(int rank, uint32_t num_client_procs);

// static void create_attr_text(int rank, hid_t var_id, const string &attr_data); 

// static herr_t attr_info(hid_t var_id, const char *name, const H5A_info_t *ainfo, void *opdata);

/*
argv[1] = dirman hexid
argv[2] = number of subdivision of the x dimension (number of processes)
argv[3] = number of subdivisions of the y dimension (number of processes)
argv[4] = number of subdivisions of the z dimension (number of processes)
argv[5] = total length in the x dimension 
argv[6] = total length in the y dimension 
argv[7] = total length in the z dimension 
argv[8] = number of datasets to be stored in the database
argv[9] = number of types stored in each dataset
argv[10] = estimate of the number of testing time points we will have 
*/
int main(int argc, char **argv) {
    int rc;
    // char name[100];
    // gethostname(name, sizeof(name));
    // extreme_debug_log << name << endl;

    if (argc != 10) { 
        error_log << "Error. Program should be given 9 arguments. npx, npy, npz, nx, ny, nz, number of timesteps, estm num time_pts, job_id" << endl;
        cout << (int)ERR_ARGC << " " << 0 <<  endl;
        return RC_ERR;
    }

    debug_log << "starting" << endl;

    uint32_t num_x_procs = stoul(argv[1],nullptr,0);
    uint32_t num_y_procs = stoul(argv[2],nullptr,0);
    uint32_t num_z_procs = stoul(argv[3],nullptr,0);
    uint64_t total_x_length = stoull(argv[4],nullptr,0);
    uint64_t total_y_length = stoull(argv[5],nullptr,0);
    uint64_t total_z_length = stoull(argv[6],nullptr,0);
    uint32_t num_timesteps = stoul(argv[7],nullptr,0);
    uint32_t estm_num_time_pts = stoul(argv[8],nullptr,0); 
    string job_id = argv[9];
    // uint64_t job_id = stoull(argv[11],nullptr,0);


    //all 10 char plus null character
    uint32_t num_vars = 10;
    uint32_t num_types = 10;
    // char var_names[10][11] = {"temperat_1", "temperat_2", "pressure_1", "pressure_2", "velocity_1", "location_1", "magn_field", "energy_var", "density_vr", "conductivi"};  
    // char var_names[10][11] = {"temperat", "temperat", "pressure", "pressure", "velocity_x", "velocity_y", "velocity_z", "magn_field", "energy_var", "density_vr"};  

    vector<string> var_names = {"temperat", "temperat", "pressure", "pressure", "velocity_x", "velocity_y", "velocity_z", "magn_field", "energy_var", "density_vr"};  

    catg_of_time_pts.reserve(estm_num_time_pts);
    time_pts.reserve(estm_num_time_pts);

    struct timeval now;
    gettimeofday(&now, NULL);
    zero_time_sec = 86400 * (now.tv_sec / 86400);

    add_timing_point(PROGRAM_START);

    MPI_Init(&argc, &argv);

    int rank;
    int num_client_procs;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size( MPI_COMM_WORLD, &num_client_procs);

    add_timing_point(MPI_INIT_DONE);

    extreme_debug_log.set_rank(rank);
    debug_log.set_rank(rank);
    error_log.set_rank(rank);
    testing_log.set_rank(rank);


    if(rank == 0) {
        extreme_debug_log << "starting" << endl;
    }

    uint32_t version1 = 1;
    uint32_t version2 = 2;

  	//add the '0' for the type version 
    char run_timestep_type_names[2][10] = {"max_temp0", "min_temp0"}; 
    int num_run_timestep_types = 2;

    uint32_t x_length_per_proc = total_x_length / num_x_procs; //todo - deal with edge case?
    uint32_t y_length_per_proc = total_y_length / num_y_procs;
    uint32_t z_length_per_proc = total_z_length / num_z_procs;

    uint64_t my_var_id;
    uint32_t num_dims;
    uint32_t var_num;
    uint32_t var_version;

    uint32_t x_pos;
    uint32_t y_pos;
    uint32_t z_pos;
    uint32_t x_offset;
    uint32_t y_offset;
    uint32_t z_offset;

    double all_timestep_temp_maxes_for_all_procs[num_timesteps];
    double all_timestep_temp_mins_for_all_procs[num_timesteps];

    //reminder - if you don't do this, rank 0 and 1 have the same outputs
    srand(rank+1);
    extreme_debug_log << "rank: " << rank << " first rgn: " << rand() << endl;

    //--------------------------------------------------------------



    x_pos = rank % num_x_procs;
    y_pos = (rank / num_x_procs) % num_y_procs; 
    z_pos = ((rank / (num_x_procs * num_y_procs)) % num_z_procs);

    x_offset = x_pos * x_length_per_proc;
    y_offset = y_pos * y_length_per_proc;
    z_offset = z_pos * z_length_per_proc;

    extreme_debug_log << "zoffset: " << to_string(z_offset) << endl;
    extreme_debug_log << "x pos is " << to_string(x_pos) << " and x_offset is " << to_string(x_offset) << endl;
    extreme_debug_log << "y pos is " << to_string(y_pos) << " and y_offset is " << to_string(y_offset) << endl;
    extreme_debug_log << "z pos is " << to_string(z_pos) << " and z_offset is " << to_string(z_offset) << endl;
    extreme_debug_log << "num x procs " << to_string(num_x_procs) << " num y procs " << to_string(num_y_procs) << " num z procs " << to_string(num_z_procs) << endl;


    num_dims = 3;

    /*
     * HDF5 APIs definitions
     */ 	

    hid_t run_attr_table_id;
    hid_t       run_file_id, var_id;         
    hid_t       var_data_space, chunk_data_space;     
    herr_t	status;

    // hsize_t     var_dims[num_dims]; /* dataset dimensions */
    hsize_t     chunk_dims[num_dims] = {x_length_per_proc, y_length_per_proc, z_length_per_proc};           

    /* chunk selection parameters */
    hsize_t	offset[num_dims] = {x_offset, y_offset, z_offset};

    md_dim_bounds proc_dims;
    proc_dims.num_dims = num_dims;
    proc_dims.d0_min = x_offset;
    proc_dims.d0_max = x_offset + x_length_per_proc - 1;
    proc_dims.d1_min = y_offset;
    proc_dims.d1_max = y_offset + y_length_per_proc - 1;    
    proc_dims.d2_min = z_offset;
    proc_dims.d2_max = z_offset + z_length_per_proc - 1;


    // add_timing_point(DIMS_INIT_DONE);
    add_timing_point(INIT_VARS_DONE);

    MPI_Barrier(MPI_COMM_WORLD);

    add_timing_point(WRITING_START);



    string run_name = "XGC";

    if (rank == 0) {

        add_timing_point(CREATE_NEW_RUN_START);

    	metadata_create_run (run_name, job_id, run_file_id, run_attr_table_id);
    	//can't close the file since each new timestep needs to link to the run attr table
    	// //close so as to prevent memory leaks since we wont be adding run attributes until later
    	status = H5Fclose(run_file_id); assert(status >= 0);
    	status = H5PTclose(run_attr_table_id); assert(status >= 0);

        add_timing_point(CREATE_NEW_RUN_DONE);
    }



    add_timing_point(CREATE_TIMESTEPS_START);

    for(int timestep_id = 0; timestep_id < num_timesteps; timestep_id++) {
    	MPI_Barrier(MPI_COMM_WORLD); //want to measure last-first for writing each timestep_id
        add_timing_point(CREATE_NEW_TIMESTEP_START);

        double timestep_temp_max;
        double timestep_temp_min;

        md_timestep_entry timestep_entry;

        vector<var_attribute_str> attrs;

        //create all of the md/data layout for the file before opening to avoid the collectives
        if (rank == 0) {
        	//create the file and associated metadata tables individually
            add_timing_point(CREATE_TIMESTEP_MD_TABLES_START);

            open_run_for_write (run_name, job_id, run_file_id); 

       		metadata_create_timestep(run_name, job_id, timestep_id, run_file_id, 
				timestep_entry);

            H5Fclose (run_file_id); 


	    	for (int var_indx = 0; var_indx < num_vars; var_indx++) {
	    		uint32_t var_version;
		    	if(var_indx == 1 || var_indx == 3) { //these have been designated ver2
		    		var_version = version2;
		    	}
		    	else {
		    		var_version = version1;
		    	}

	    		string VARNAME = var_names.at(var_indx) + to_string(var_version);
	    		extreme_debug_log << "VARNAME: " << VARNAME << endl;

	    		hsize_t var_dims[num_dims] = {total_x_length, total_y_length, total_z_length};


				metadata_create_and_close_chunked_var(num_dims, var_dims, chunk_dims,
					timestep_entry.file_id, VARNAME);
       		}

       		close_timestep(timestep_entry);
            add_timing_point(CREATE_TIMESTEP_MD_TABLES_DONE);
       	}

       	debug_log << "rank: " << rank << " about to open file " << endl;
        add_timing_point(OPEN_TIMESTEP_START);

       	open_timestep_file_collectively_for_write(run_name, job_id, timestep_id, timestep_entry.file_id);

        // add_timing_point(OPEN_TIMESTEP_DONE);

    	
        add_timing_point(CREATE_VARS_AND_ATTRS_FOR_NEW_TIMESTEP_START); 

	    for (int var_indx = 0; var_indx < num_vars; var_indx++) {
	    	add_timing_point(CREATE_VAR_AND_ATTRS_FOR_NEW_VAR_START);

    		uint32_t var_version;
	    	if(var_indx == 1 || var_indx == 3) { //these have been designated ver2
	    		var_version = version2;
	    	}
	    	else {
	    		var_version = version1;
	    	}
	    	string VARNAME = var_names[var_indx] + to_string(var_version);


  			if (write_data) { 
  				hid_t var_id;
  				vector<double> data_vctr;
                add_timing_point(CREATE_AND_WRITE_CHUNK_DATA_START);

				open_var(timestep_entry.file_id, var_names[var_indx], var_version, var_id);

				write_chunk_data(var_id, num_dims, offset, chunk_dims, 
	    			total_y_length, total_z_length, rank, data_vctr);

	            add_timing_point(CREATE_AND_WRITE_CHUNK_DATA_DONE); 

		   		status = H5Dclose(var_id); assert(status >= 0);

	            // add_timing_point(VAR_CLOSE_DONE);

                if(var_indx == 0 ) {
                    add_timing_point(CHUNK_MAX_MIN_FIND_START);

                    timestep_temp_max = -1 *RAND_MAX;
                    timestep_temp_min = RAND_MAX;

                    for (double val : data_vctr) {
                        if (val < timestep_temp_min) {
                            timestep_temp_min = val;
                        }
                        if (val > timestep_temp_max) {
                            timestep_temp_max = val;
                        }

                    }
                    timestep_temp_min -= 100*timestep_id;
                    timestep_temp_max += 100*timestep_id;
                    add_timing_point(CHUNK_MAX_MIN_FIND_DONE);

                } //end if(var_indx == 0)
            } //end if write data 
	    	
	    	create_var_attrs(rank, proc_dims, timestep_id, var_indx, VARNAME, num_types,
	    			write_data, attrs, timestep_temp_max, timestep_temp_min);
  			
        } //var loop done 
        // add_timing_point(CLOSE_TIMESTEP_DONE); 
        //close since the attr writing can't be done with a collectively opened file
      	H5Fclose(timestep_entry.file_id);

        add_timing_point(CREATE_VARS_AND_ATTRS_FOR_NEW_TIMESTEP_DONE); 

       vector<var_attribute_str> all_attrs;

       add_timing_point(GATHER_ATTRS_START); 

       ser_gatherv_and_deser(attrs, num_client_procs, rank, all_attrs);

       add_timing_point(GATHER_ATTRS_DONE); 

       if(rank == 0) {
       	//first need to open the table
       		add_timing_point(INSERT_VAR_ATTRS_START); 

			open_timestep_file_and_var_attr_table_for_write(run_name, job_id, timestep_id, 
				timestep_entry.file_id, timestep_entry.var_attr_table_id );

       		metadata_insert_var_attribute_batch(timestep_entry.var_attr_table_id, all_attrs );

		   	status = H5PTclose(timestep_entry.var_attr_table_id); assert(status >= 0);
		    add_timing_point(INSERT_VAR_ATTRS_DONE); 

		}

		create_timestep_attrs(timestep_entry.file_id, rank, timestep_id, num_client_procs, 
			timestep_temp_max, timestep_temp_min, run_timestep_type_names[0], run_timestep_type_names[1], 
			all_timestep_temp_maxes_for_all_procs, all_timestep_temp_mins_for_all_procs);	    
		    
		if(rank == 0) {
			H5Fclose(timestep_entry.file_id);
		}
		add_timing_point(CREATE_TIMESTEP_DONE); 

	}
    add_timing_point(CREATE_ALL_TIMESTEPS_DONE); 

    

    if (rank == 0) {

		create_run_attrs(run_name, job_id, num_timesteps, all_timestep_temp_maxes_for_all_procs, 
			all_timestep_temp_mins_for_all_procs, run_timestep_type_names[0], run_timestep_type_names[1]);
	}


//-------------------------------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------------------------------    

    if(do_debug_testing && rank == 0) {
        // rc = debug_testing(server);
	    hsize_t var_dims[num_dims] = {total_x_length, total_y_length, total_z_length};

	    testing_log << "about to start debug testing " << endl;
        rc = debug_testing(rank, chunk_dims, num_client_procs, run_name, num_timesteps, num_vars, 
        		num_dims, var_dims, job_id, var_names, version1, version2, write_data);

        if (rc != RC_OK) {
            error_log << "Error with debug testing \n";
        }             
    }


//-------------------------------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------------------------------        
cleanup:
	do_cleanup(rank, num_client_procs);

    return rc;

}

// static void create_file(const string &run_name, string job_id, uint64_t timestep_id, hid_t &file_id)
// {
// 	string FILENAME = run_name + "/" + job_id + "/" + to_string(timestep_id);
// 	extreme_debug_log << "FILENAME: " << FILENAME << endl;

//     /*
//     * Set up file access property list with parallel I/O access
//     */
//     //create a property list for file access
// 	hid_t property_list_id = H5Pcreate(H5P_FILE_ACCESS); assert(property_list_id >= 0);

//     herr_t status = H5Pset_fapl_mpio(property_list_id, MPI_COMM_WORLD, MPI_INFO_NULL); assert(status >= 0);

//     // add_timing_point(PARALLEL_FILE_ACCESS_INIT_DONE); 

//     /*
//      * Create a new file collectively and release property list identifier.
//      */
//     //note: New files are always created in read-write mode,
//     file_id = H5Fcreate(FILENAME.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, property_list_id); assert(file_id >= 0);

//     status = H5Pclose(property_list_id); assert(status >= 0);
// }


// static void open_file_indv(const string &run_name, string job_id, uint64_t timestep_id, hid_t &file_id)
// {
// 	string FILENAME = run_name + "/" + job_id + "/" + to_string(timestep_id);
// 	extreme_debug_log << "FILENAME: " << FILENAME << endl;

//     /*
//      * open the file for writing and reading
//      */
//     //note: New files are always created in read-write mode,
//     file_id = H5Fopen(FILENAME.c_str(), H5F_ACC_RDWR, H5P_DEFAULT); assert(file_id >= 0);

// }


static void write_chunk_data(hid_t var_id, uint32_t num_dims, hsize_t	*offset, 
						hsize_t *chunk_dims, uint32_t total_y_length, uint32_t total_z_length, 
						int rank, std::vector<double> &data_vct
						)
{

	hsize_t	*count = chunk_dims;
    hsize_t	stride[num_dims] = {1, 1, 1}; //todo - what if 2D?
    hsize_t	block[num_dims] = {1, 1, 1}; //todo - what if 2D?

    hsize_t chunk_vol = chunk_dims[0] * chunk_dims[1] * chunk_dims[2]; //todo - what if 2D?
	data_vct.reserve(chunk_vol);


  	hid_t var_data_space = H5Dget_space( var_id ); assert(var_data_space >= 0);


    hid_t chunk_data_space  = H5Screate_simple(num_dims, chunk_dims, NULL); assert(chunk_data_space >= 0);

    /*
     * Select hyperslab in the file.
     */
    // var_data_space = H5Dget_space(var_id);  assert(var_data_space >= 0);
    herr_t status = H5Sselect_hyperslab(var_data_space, H5S_SELECT_SET, offset, stride, count, block); assert(status >= 0);

    /*
     * Initialize data buffer 
     */
    add_timing_point(CREATE_DATA_START);
	generate_data_for_proc(total_y_length, total_z_length, offset, rank,
					data_vct, chunk_dims[0], chunk_dims[1], chunk_dims[2], chunk_vol);
    add_timing_point(CREATE_DATA_DONE);

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
}




void create_var_attrs(int rank, md_dim_bounds proc_dims, int timestep_num, int var_indx, string VARNAME, uint32_t num_types, 
		bool write_data, vector<var_attribute_str> &attrs,
		double &timestep_temp_max, double &timestep_temp_min)
{
	float type_freqs[] = {25, 5, .1, 100, 100, 20, 2.5, .5, 1, 10};
    // char type_types[10][3] = {"b", "b", "b", "d", "d", "2p","2p","2p","2d","2d"};
    char type_types[10][3] = {"b", "b", "b", "d", "d", "s","s","s","2d","2d"};
    //all 12 char plus null character
    char type_names[10][13] = {"blob_freq", "blob_ifreq", "blob_rare", "max_val_type", "min_val_type", "note_freq", "note_ifreq", "note_rare", "ranges_type1", "ranges_type2"}; 
   
	uint32_t type_version = 0;

	add_timing_point(CREATE_VAR_ATTRS_FOR_NEW_VAR_START); 

    for(uint32_t type_indx=0; type_indx<num_types; type_indx++) {
        float freq = type_freqs[type_indx];
        uint32_t odds = 100 / freq;
        uint32_t val = rand() % odds; 
        extreme_debug_log << "rank: " << to_string(rank) << " and val: " << to_string(val) << " and odds: " << to_string(odds) << endl;
        if(val == 0) { //makes sure we have the desired frequency for each type
            add_timing_point(CREATE_NEW_VAR_ATTR_START);

			string attr_data;

            if(strcmp(type_types[type_indx], "b") == 0) {
                bool flag = rank % 2;
                make_single_val_data(flag, attr_data);
            }
            else if(strcmp(type_types[type_indx], "d") == 0) {
                // int sign =  pow(-1, rand() % 2);
                int sign = pow(-1,(type_indx+1)%2); //max will be positive, min will be neg
                double val = (double)rand() / RAND_MAX;
                // val = sign * (DBL_MIN + val * (DBL_MAX - DBL_MIN) );
                //note - can use DBL MAX/MIN but the numbers end up being E305 to E308
                // val = sign * (FLT_MIN + val * (FLT_MAX - FLT_MIN) );
                //note - can use DBL MAX/MIN but the numbers end up being E35 to E38
                val = sign * val * ( pow(10,10) ) ; //produces a number in the range [0,10^10]
                make_single_val_data(val, attr_data);
                //if write data the chunk min and max for var 0 will be based on the actual data values (this is done above)
                if(!write_data && var_indx == 0 )  { 
                    if ( type_indx == 3 )  {
                        timestep_temp_max = val;
                        extreme_debug_log << "for rank: " << rank << " timestep: " << timestep_num << " just added " << val << " as the max value \n";
                    }
                    else if (type_indx == 4 ) {
                        timestep_temp_min = val;
                        extreme_debug_log << "for rank: " << rank << " timestep: " << timestep_num << " just added " << val << " as the min value \n";                                    
                    }
                }                       
            }
            else if(strcmp(type_types[type_indx], "s") == 0) { 
                attr_data = "attr data for var" + to_string(var_indx) + "timestep"+to_string(timestep_num);

            }               
            else if(strcmp(type_types[type_indx], "2d") == 0) {
                vector<int> vals(2);
                vals[0] = (int) (rand() % RAND_MAX); 
                vals[1] = vals[0] + 10000; 
                make_single_val_data(vals, attr_data);

            }
            else {
                error_log << "error. type didn't match list of possibilities" << endl;
                add_timing_point(ERR_TYPE_COMPARE);
            }
            add_timing_point(VAR_ATTR_INIT_DONE);
				
			string type_name = type_names[type_indx] + to_string(type_version);

			attrs.push_back ( var_attribute_str (type_name.c_str(), VARNAME.c_str(), proc_dims, attr_data) );

	        // if(attrs.size() < 5) {
	        //     extreme_debug_log << "for rank: " << rank << " the " << attrs.size() << "th attr has val: " << attr_data << endl;
	        // }

           add_timing_point(CREATE_NEW_VAR_ATTR_DONE);
        }//val = 0 done
    } //type loop done
    add_timing_point(CREATE_VAR_ATTRS_FOR_NEW_VAR_DONE);
}

void create_timestep_attrs(hid_t timestep_file_id, int rank, int timestep_num, uint32_t num_client_procs, 
	double timestep_temp_max, double timestep_temp_min, char *max_type_name, char *min_type_name,
	double *all_timestep_temp_maxes_for_all_procs, double *all_timestep_temp_mins_for_all_procs)
{
	 add_timing_point(CREATE_TIMESTEP_ATTRS_START);

    if(rank == 0) {

        double all_temp_maxes[num_client_procs];
        double all_temp_mins[num_client_procs];

        MPI_Gather(&timestep_temp_max, 1, MPI_DOUBLE, all_temp_maxes, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD); 
        MPI_Gather(&timestep_temp_min, 1, MPI_DOUBLE, all_temp_mins, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);

        double timestep_var_max = -1 * RAND_MAX;
        double timestep_var_min = RAND_MAX;

        add_timing_point(GATHER_DONE_PROC_MAX_MIN_FIND_START);
        for (int proc_rank = 0; proc_rank<num_client_procs; proc_rank++) {

            if( all_temp_maxes[proc_rank] > timestep_var_max) {
                timestep_var_max = all_temp_maxes[proc_rank];
            }
            if( all_temp_mins[proc_rank] < timestep_var_min) {
                timestep_var_min = all_temp_mins[proc_rank];
            }                    
        }
        add_timing_point(PROC_MAX_MIN_FIND_DONE);

        all_timestep_temp_maxes_for_all_procs[timestep_num] = timestep_var_max;
        all_timestep_temp_mins_for_all_procs[timestep_num] = timestep_var_min;

        testing_log << "timestep " << timestep_num << " temperature max: " << timestep_var_max << endl;
        testing_log << "timestep " << timestep_num << " temperature min: " << timestep_var_min << endl;

        vector<non_var_attribute_str> attrs;

        string temp_max_data, temp_min_data;

        make_single_val_data(timestep_var_max, temp_max_data);
        make_single_val_data(timestep_var_min, temp_min_data);

        attrs.push_back ( non_var_attribute_str (max_type_name, temp_max_data) );
        attrs.push_back ( non_var_attribute_str (min_type_name, temp_min_data) );

        hid_t timestep_attr_table_id;
       	open_timestep_attr_table (timestep_file_id, timestep_attr_table_id);

    	metadata_insert_timestep_attribute_batch(timestep_attr_table_id, attrs );

    	H5PTclose(timestep_attr_table_id);

    }
    else {
        MPI_Gather(&timestep_temp_max, 1, MPI_DOUBLE, NULL, NULL, MPI_DOUBLE, 0, MPI_COMM_WORLD); 
        MPI_Gather(&timestep_temp_min, 1, MPI_DOUBLE, NULL, NULL, MPI_DOUBLE, 0, MPI_COMM_WORLD); 
    }

    add_timing_point(CREATE_TIMESTEP_ATTRS_DONE);
}

void create_run_attrs(string run_name, string job_id, uint32_t num_timesteps, double *all_timestep_temp_maxes_for_all_procs, 
	double *all_timestep_temp_mins_for_all_procs, char *max_type_name, char *min_type_name
	)	
{
    add_timing_point(CREATE_RUN_ATTRS_START);

    double run_temp_max = -1 * RAND_MAX;
    double run_temp_min = RAND_MAX;

    add_timing_point(RUN_MAX_MIN_FIND_START);

    for(int timestep = 0; timestep < num_timesteps; timestep++) {
        if( all_timestep_temp_maxes_for_all_procs[timestep] > run_temp_max) {
            run_temp_max = all_timestep_temp_maxes_for_all_procs[timestep];
        }
        if( all_timestep_temp_mins_for_all_procs[timestep] < run_temp_min) {
            run_temp_min = all_timestep_temp_mins_for_all_procs[timestep];
        }    
    }
    add_timing_point(RUN_MAX_MIN_FIND_DONE);


    vector<non_var_attribute_str> attrs;

    string temp_max_data;
    string temp_min_data;

    make_single_val_data(run_temp_max, temp_max_data);
    make_single_val_data(run_temp_min, temp_min_data);

    attrs.push_back ( non_var_attribute_str (max_type_name, temp_max_data) );
    attrs.push_back ( non_var_attribute_str (min_type_name, temp_min_data) );

    hid_t run_file_id;
    hid_t run_attr_table_id;
    open_run_and_attr_table_for_write (run_name, job_id, run_file_id, run_attr_table_id);

	metadata_insert_run_attribute_batch(run_attr_table_id, attrs );

	H5PTclose(run_attr_table_id);
	H5Fclose(run_file_id);

    add_timing_point(CREATE_RUN_ATTRS_DONE);
    
}

void do_cleanup(int rank, uint32_t num_client_procs)
{
	long double *all_time_pts_buf;
    int *all_catg_time_pts_buf;

    add_timing_point(WRITING_DONE);

    MPI_Barrier (MPI_COMM_WORLD);
    testing_log << "about to start cleanup" << endl;
    add_timing_point(WRITING_DONE_FOR_ALL_PROCS_START_CLEANUP);
        
    if(output_timing) {
        int each_proc_num_time_pts[num_client_procs];
        int displacement_for_each_proc[num_client_procs];
        int *all_catg_time_pts_buf;

        gatherv_int(catg_of_time_pts, num_client_procs, rank, each_proc_num_time_pts,
            displacement_for_each_proc, &all_catg_time_pts_buf);

        int sum = 0;
        if(rank == 0) {
            sum = accumulate(each_proc_num_time_pts, each_proc_num_time_pts+num_client_procs, sum);
            extreme_debug_log << "sum: " << sum << endl;
            all_time_pts_buf = (long double *) malloc(sum * sizeof(long double));
        }
     
        int num_time_pts = time_pts.size();
        debug_log << "num_time pts: " << to_string(num_time_pts)  << "time_pts.size(): " << time_pts.size()<< endl;

        MPI_Gatherv(&time_pts[0], num_time_pts, MPI_LONG_DOUBLE, all_time_pts_buf, each_proc_num_time_pts, displacement_for_each_proc, MPI_LONG_DOUBLE, 0, MPI_COMM_WORLD); 
        
        if (rank == 0) {
            //prevent it from buffering the printf statements
            setbuf(stdout, NULL);
            std::cout << "begin timing output" << endl;

            for(int i=0; i<sum; i++) {
                    // cout << "all_time_pts_buf[i]: " << all_time_pts_buf[i] << endl;
                printf("%d %Lf ", all_catg_time_pts_buf[i], all_time_pts_buf[i]);
                // std::cout << all_catg_time_pts_buf[i] << " " << all_time_pts_buf[i] << " ";
                if (i%20 == 0 && i!=0) {
                    std::cout << std::endl;
                }
            }
            std::cout << std::endl;

            free(all_time_pts_buf);
            free(all_catg_time_pts_buf);

        }
    }

    MPI_Barrier (MPI_COMM_WORLD);
    MPI_Finalize();
    debug_log << "got to cleanup7" << endl;
    debug_log << "finished \n";
}
