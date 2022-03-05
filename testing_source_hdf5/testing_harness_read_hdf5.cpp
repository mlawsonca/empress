#include <stdio.h> //needed for printf
#include <stdlib.h> //needed for atoi/atol/malloc/free/rand
// #include <assert.h>
#include <string.h> //needed for strcmp

// #include <ctime>
#include <stdint.h> //needed for uint
// #include <ratio>
#include <chrono>
#include <math.h> //needed for pow()
#include <sys/time.h>
// #include <sys/stat.h>
// #include <fstream>
#include <vector>

#include <hdf5.h>
#include <numeric>      // std::accumulate

// #include <sstream> //fix - do I need this?
// #include <boost/archive/text_iarchive.hpp>
// #include <boost/archive/text_oarchive.hpp>

#include <testing_harness_helper_functions_hdf5.hh>

#include <my_metadata_client_hdf5.hh>
#include <client_timing_constants_read_hdf5.hh>
#include <3d_read_for_testing_hdf5.hh>

// #include <testing_harness_hdf5.hh>



using namespace std;

bool extreme_debug_logging = false;
bool debug_logging = false;
bool error_logging  = true;
bool zero_rank_logging = false;
bool testing_logging = true;

// static debugLog error_log = debugLog(error_logging, zero_rank_logging);
// static debugLog debug_log = debugLog(debug_logging, zero_rank_logging);
// static debugLog extreme_debug_log = debugLog(extreme_debug_logging, zero_rank_logging);
// static debugLog testing_log = debugLog(testing_logging, zero_rank_logging);

debugLog error_log = debugLog(error_logging, zero_rank_logging);
debugLog debug_log = debugLog(debug_logging, zero_rank_logging);
debugLog extreme_debug_log = debugLog(extreme_debug_logging, false);
debugLog testing_log = debugLog(testing_logging, zero_rank_logging);

static bool output_timing = true;
bool read_data = true;

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


int extra_testing_collective(string run_name, string job_id, int rank, uint32_t num_client_procs, 
	md_dim_bounds chunk_dims, vector<string> var_names, vector<string> type_names);

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
    // char name[100];
    // gethostname(name, sizeof(name));
    // extreme_debug_log << name << endl;

    if (argc != 10) { 
        error_log << "Error. Program should be given 9 arguments. npx, npy, npz, nx, ny, nz, number of datasets, estm num time_pts, job_id" << endl;
        cout << ERR_ARGC << " " << 0 <<  endl;
        return RC_ERR;
    }

    uint32_t num_x_procs = stoul(argv[1],nullptr,0);
    uint32_t num_y_procs = stoul(argv[2],nullptr,0);
    uint32_t num_z_procs = stoul(argv[3],nullptr,0);
    uint64_t total_x_length = stoull(argv[4],nullptr,0);
    uint64_t total_y_length = stoull(argv[5],nullptr,0);
    uint64_t total_z_length = stoull(argv[6],nullptr,0);
    uint32_t num_timesteps = stoul(argv[7],nullptr,0);
    // uint32_t num_types = stoul(argv[8],nullptr,0);
    uint32_t estm_num_time_pts = stoul(argv[8],nullptr,0); 
    string job_id = argv[9]; 

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

    // vector<md_dim_bounds> dims(3);

    //to fix - should just get this from the catalog?


    vector<string> var_names = {"temperat", "temperat", "pressure", "pressure", "velocity_x", "velocity_y", "velocity_z", "magn_field", "energy_var", "density_vr"};  
    
    //note - since I decided not to implement a type catalog, need to supply this info for the extra testing section
    vector<string> type_names = {"blob_freq0", "blob_ifreq0", "blob_rare0", "max_val_type0", "min_val_type0", 
    "note_freq0", "note_ifreq0", "note_rare0", "ranges_type10", "ranges_type20", "max_temp0", "min_temp0"}; 


    uint32_t num_vars = 10;

    uint32_t version1 = 1;
    uint32_t version2 = 2;

    srand(rank);

    //just for testing-------------------------------------------
    long double *all_time_pts_buf;
    int *all_catg_time_pts_buf;

    //--------------------------------------------------------------

    std::string serial_str;

    int x_length_per_proc = total_x_length / num_x_procs; //fix - deal with edge case?
    int y_length_per_proc = total_y_length / num_y_procs;
    int z_length_per_proc = total_z_length / num_z_procs;

    int x_pos = rank % num_x_procs;
    int y_pos = (rank / num_x_procs) % num_y_procs;
    int z_pos = ((rank / (num_x_procs * num_y_procs)) % num_z_procs);
    int x_offset = x_pos * x_length_per_proc;
    int y_offset = y_pos * y_length_per_proc;
    int z_offset = z_pos * z_length_per_proc;
    
    extreme_debug_log << "x_pos: " << x_pos << endl;
    extreme_debug_log << "y_pos: " << y_pos << endl;
    extreme_debug_log << "z_pos: " << z_pos << endl;
    extreme_debug_log << "x_offset : " << x_offset  << endl;
    extreme_debug_log << "y_offset : " << y_offset  << endl;
    extreme_debug_log << "z_offset : " << z_offset  << endl;
    extreme_debug_log << "x_length_per_proc : " << x_length_per_proc  << endl;
    extreme_debug_log << "y_length_per_proc : " << y_length_per_proc  << endl;
    extreme_debug_log << "z_length_per_proc : " << z_length_per_proc  << endl;
  
    uint32_t num_dims = 3;

    md_dim_bounds proc_dims;
    proc_dims.num_dims = num_dims;
    proc_dims.d0_min = x_offset;
    proc_dims.d0_max = x_offset + x_length_per_proc - 1;
    proc_dims.d1_min = y_offset;
    proc_dims.d1_max = y_offset + y_length_per_proc - 1;    
    proc_dims.d2_min = z_offset;
    proc_dims.d2_max = z_offset + z_length_per_proc - 1;

    /*
     * HDF5 APIs definitions
     */ 	

    hid_t       file_id, var_id;         /* file and dataset identifiers */
    hid_t       var_data_space, chunk_data_space;      /* file and memory chunk_data_space identifiers */
    // int         *data;                    /* pointer to data buffer to write */
    hid_t	property_list_id;                 /* property list identifier */
    herr_t	status;


    hsize_t     var_dims[num_dims] = {total_x_length, total_y_length, total_z_length}; /* dataset dimensions */
    hsize_t     chunk_dims[num_dims] = {x_length_per_proc, y_length_per_proc, z_length_per_proc};           

    /* chunk selection parameters */
    hsize_t	offset[num_dims] = {x_offset, y_offset, z_offset};
    hsize_t	*count = chunk_dims;         

    hsize_t	stride[num_dims] = {1, 1, 1};
    hsize_t	block[num_dims] = {1, 1, 1};


    string run_name = "XGC";

    hsize_t chunk_vol = chunk_dims[0] * chunk_dims[1] * chunk_dims[2];
	// std::vector<double> data_vct (chunk_vol);


    const int num_timesteps_to_fetch = 6;

   	vector<uint64_t> timestep_ids_to_fetch = {(num_timesteps-1) / 6, (num_timesteps-1) / 5, (num_timesteps-1) / 4,
                        (num_timesteps-1) / 3, (num_timesteps-1) / 2, num_timesteps-1};    
    char *serialized_c_str;
    int num_bytes_and_entries_and_sqrt[3];
    // vector<vector<md_catalog_var_entry>> all_var_entries;
    // all_var_entries.reserve(num_timesteps_to_fetch);

    vector<string> file_names_to_fetch;
    vector<string> var_names_to_fetch;
    vector<string> all_var_names;

    vector<uint64_t> var_ids_to_fetch = { 0, num_vars/6, num_vars/2, num_vars/4, num_vars/3, num_vars - 2, num_vars - 1};
    vector<string> type_names_to_fetch =  { type_names[0], type_names[1], type_names[2]}; 

    add_timing_point(INIT_VARS_DONE);


    MPI_Barrier(MPI_COMM_WORLD);
    add_timing_point(READING_START);


    for (int i = 0; i < timestep_ids_to_fetch.size(); i++) {
    	uint64_t timestep_id = timestep_ids_to_fetch.at(i);
    	string FILENAME = run_name + "/" + job_id + "/" + to_string(timestep_id);
    	file_names_to_fetch.push_back(FILENAME);
    }

    for (int var_indx = 0; var_indx < num_vars; var_indx++) {
    	uint32_t var_version;
    	if (var_indx == 1 || var_indx == 3) {
    		var_version = version2;
    	}
    	else {
    		var_version = version1;
    	}
	   	string VARNAME = var_names.at(var_indx) + to_string(var_version);
	   	all_var_names.push_back(VARNAME);
    }

    for (int i = 0; i < var_ids_to_fetch.size(); i++) {
    	uint64_t var_indx = var_ids_to_fetch.at(i);
	   	string VARNAME = all_var_names.at(var_indx);
	   	var_names_to_fetch.push_back(VARNAME);
    }

    int plane_x_procs;
    int plane_y_procs;

    add_timing_point(FIND_VARS_DONE);


    // uint32_t num_runs;
	//fix - if we add more than 1 run this will need to be changed
    // metadata_catalog_run(server, txn_id, num_runs, runs);

    // run = runs.at(0); 

    //pattern testing for vars

    if(read_data) {
        add_timing_point(READING_PATTERNS_START);

	    MPI_Barrier(MPI_COMM_WORLD);
	    read_pattern_1 ( offset, stride, count, block, file_names_to_fetch.at(0), all_var_names,
	    					chunk_vol, num_dims);
	    extreme_debug_log << "finished pattern 1" << endl;

	    MPI_Barrier(MPI_COMM_WORLD);
	  	read_pattern_2 ( offset, stride, count, block, file_names_to_fetch.at(1), var_names_to_fetch.at(0),
	    					chunk_vol, num_dims);
	    extreme_debug_log << "finished pattern 2" << endl;

	    vector<string> var_names_pattern_3 = { var_names_to_fetch.at(1), var_names_to_fetch.at(2), var_names_to_fetch.at(3) };

	    MPI_Barrier(MPI_COMM_WORLD);
	  	read_pattern_3 ( offset, stride, count, block, file_names_to_fetch.at(2), var_names_pattern_3,
	    					chunk_vol, num_dims);
	    extreme_debug_log << "finished pattern 3" << endl;

	    int my_sqrt = floor(sqrt(num_client_procs));
	    while(num_client_procs % my_sqrt != 0) {
	        my_sqrt -= 1;
	    }

	    //want to use all read procs in pattern 4 and 6 as "x and y" procs
	    extreme_debug_log << "num_client_procs: " << num_client_procs << endl;
	    plane_x_procs = my_sqrt;
	    plane_y_procs = num_client_procs / plane_x_procs;
	    extreme_debug_log << "num x procs: " << num_x_procs << " num y procs: "; 
	    extreme_debug_log << num_y_procs << " num z procs: " << num_z_procs << " num x procs plane: " << 	
	    					plane_x_procs << " num y procs plane: " << plane_y_procs << endl;


	    MPI_Barrier(MPI_COMM_WORLD);
		read_pattern_4 ( rank, plane_x_procs, plane_y_procs, stride, block, file_names_to_fetch.at(3), 
						var_names_to_fetch.at(4), num_dims);

	    extreme_debug_log << "finished pattern 4" << endl;

	    MPI_Barrier(MPI_COMM_WORLD);
		read_pattern_5 ( rank, num_x_procs, num_y_procs, num_z_procs, stride, block, file_names_to_fetch.at(4),
					var_names_to_fetch.at(5), num_dims );
	    extreme_debug_log << "finished pattern 5" << endl;

	    MPI_Barrier(MPI_COMM_WORLD);
	    read_pattern_6 ( rank, plane_x_procs, plane_y_procs, stride, block, 
						file_names_to_fetch.at(5), var_names_to_fetch.at(6), num_dims );
							
	    extreme_debug_log << "finished pattern 6" << endl;
	

    	add_timing_point(READING_PATTERNS_DONE);
    	testing_log << "Starting read patterns for types \n";

	    // MPI_Barrier(MPI_COMM_WORLD);
	    // read_pattern_1 ( offset, stride, count, block, file_names_to_fetch.at(0), all_var_names,
	    // 					chunk_vol, num_dims);
	    // extreme_debug_log << "finished pattern 1 type" << endl;

	    MPI_Barrier(MPI_COMM_WORLD);
		read_pattern_2_type (rank, file_names_to_fetch.at(1), type_names_to_fetch, var_names_to_fetch.at(0), 
					proc_dims, num_client_procs );
	    extreme_debug_log << "finished pattern 2 type" << endl;

	    MPI_Barrier(MPI_COMM_WORLD);
		read_pattern_3_type (rank, file_names_to_fetch.at(2), type_names_to_fetch, var_names_pattern_3, 
				proc_dims, num_client_procs );
	    extreme_debug_log << "finished pattern 3 type" << endl;

	 //    MPI_Barrier(MPI_COMM_WORLD);
		// read_pattern_4 ( rank, plane_x_procs, plane_y_procs, stride, block, file_names_to_fetch.at(3), 
		// 				var_names_to_fetch.at(4), num_dims);

	 //    extreme_debug_log << "finished pattern 4 type" << endl;

	 //    MPI_Barrier(MPI_COMM_WORLD);
		// read_pattern_5 ( rank, num_x_procs, num_y_procs, num_z_procs, stride, block, file_names_to_fetch.at(4),
		// 			var_names_to_fetch.at(5), num_dims );
	 //    extreme_debug_log << "finished pattern 5 type" << endl;

	 //    MPI_Barrier(MPI_COMM_WORLD);
	 //    read_pattern_6 ( rank, plane_x_procs, plane_y_procs, stride, block, 
		// 				file_names_to_fetch.at(5), var_names_to_fetch.at(6), num_dims );
							
	 //    extreme_debug_log << "finished pattern 6 type" << endl;

    	add_timing_point(READING_TYPE_PATTERNS_DONE);
	}




    MPI_Barrier(MPI_COMM_WORLD);
    add_timing_point(EXTRA_TESTING_START);

    //reminder - have to pass var names else will be in alphabetic rather than insertion order
	extra_testing_collective(run_name, job_id, rank, num_client_procs, proc_dims, all_var_names, type_names) ;

    add_timing_point(EXTRA_TESTING_DONE);

    add_timing_point(READING_DONE); 


//-------------------------------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------------------------------        
cleanup:
    MPI_Barrier (MPI_COMM_WORLD);
    // add_timing_point(WRITING_DONE_FOR_ALL_PROCS_START_CLEANUP);
        
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
    // cout << "finished \n";
}

