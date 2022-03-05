#include <stdio.h> //needed for printf
#include <stdlib.h> //needed for atoi/atol/malloc/free/rand
#include <assert.h> //needed for assert
#include <string.h> //needed for strcmp

// #include <ctime>
#include <stdint.h> //needed for uint
// #include <ratio>
// #include <chrono> //needed for high_resolution_clock
#include <math.h> //needed for pow()
#include <sys/time.h> //needed for timeval
#include <sys/stat.h> //needed for stat
#include <fstream> //needed for ifstream
#include <vector>
#include <numeric>

#include <mpi.h>

 //needed for some opbox support
#include "dirman/DirMan.hh"
#include <sstream>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

#include <3d_read_for_testing_class_proj_hdf5.hh>
#include <my_metadata_client_hdf5.hh>
// #include <md_client_timing_constants.hh>
// #include <client_timing_constants_read_class_proj_hdf5.hh>
#include <client_timing_constants_read_class_proj.hh>
// #include <my_metadata_client_lua_functs.h>


using namespace std;

// static bool extreme_debug_logging = false;
// static bool debug_logging = false;
// static bool testing_logging = false;
// static bool zero_rank_logging = false;
static bool error_logging  = true;

static debugLog error_log = debugLog(error_logging, false);
// static debugLog debug_log = debugLog(debug_logging, zero_rank_logging);
// static debugLog extreme_debug_log = debugLog(extreme_debug_logging, false);
// static debugLog testing_log = debugLog(testing_logging, zero_rank_logging);

//link to testing_debug.cpp in cmake to do debug tests
// bool do_debug_testing = false;
extern void add_timing_point(int catg);

// void extra_testing(const string &run_name, const string &job_id, int rank, uint32_t num_client_procs,
// 					const vector<string> &all_var_names, const vector<string> &type_names, 
// 					const vector<string> &run_timestep_type_names, MPI_Comm read_comm );
void extra_testing(const string &run_name, const string &job_id, int rank, uint32_t num_client_procs,
					uint32_t num_timesteps,
					const vector<string> &var_names, const vector<string> &type_names, 
					const vector<string> &run_timestep_type_names, MPI_Comm read_comm );

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
argv[11] = the number of EMPRESS (metadata) servers 
argv[12] = the job id (from the scheduler) 

*/

//note - the rank should be the read comm rank
int do_reads(int rank, string run_name, string job_id,
		uint32_t num_client_procs,
		uint32_t num_timesteps,
		MPI_Comm read_comm,
		// const md_dim_bounds &proc_dims,
		const vector<string> &all_var_names,
		const vector<string> &type_names,
		const vector<string> &run_timestep_type_names,
		const std::vector<string> &file_names_to_fetch,
		const std::vector<string> &var_names_to_fetch,	
		hsize_t *var_dims
	) 
{
	int rc = RC_OK;

 	vector<string> type_names_to_fetch =  { type_names[0], type_names[1], type_names[2]}; 

    //extreme_debug_log.set_rank(rank);
    //debug_log.set_rank(rank);
    error_log.set_rank(rank);
    //testing_log.set_rank(rank);


    //debug_log << "about to barrier the read comm" << endl;

    MPI_Barrier(read_comm);
    add_timing_point(READING_START);

    read_pattern_1 (rank, file_names_to_fetch.at(0), all_var_names, type_names_to_fetch, num_client_procs, read_comm); 
    //debug_log << "finished pattern 1t" << endl;

    MPI_Barrier(read_comm);

	read_pattern_2 (rank, file_names_to_fetch.at(1), type_names_to_fetch, var_names_to_fetch.at(0), num_client_procs, read_comm); 
    //debug_log << "finished pattern 2" << endl;

    MPI_Barrier(read_comm);
	vector<string> var_names_pattern_3 = { var_names_to_fetch.at(1), var_names_to_fetch.at(2), var_names_to_fetch.at(3) };
	
	read_pattern_3 (rank, file_names_to_fetch.at(2), type_names_to_fetch, var_names_pattern_3, num_client_procs, read_comm);
    //debug_log << "finished pattern 3t" << endl;

    MPI_Barrier(read_comm);
	read_pattern_4 (rank, file_names_to_fetch.at(3), type_names_to_fetch, var_names_to_fetch.at(4), var_dims,
				num_client_procs, read_comm);
    //debug_log << "finished pattern 4t" << endl;

    MPI_Barrier(read_comm);
	read_pattern_5 (rank, file_names_to_fetch.at(4), type_names_to_fetch, var_names_to_fetch.at(5), var_dims,
				num_client_procs, read_comm);
    //debug_log << "finished pattern 5t" << endl;

    MPI_Barrier(read_comm);
	read_pattern_6 (rank, file_names_to_fetch.at(5), type_names_to_fetch, var_names_to_fetch.at(6),
				var_dims, num_client_procs, read_comm);
    //debug_log << "finished pattern 6t" << endl;

    add_timing_point(READING_PATTERNS_DONE);

    MPI_Barrier(read_comm);
    add_timing_point(EXTRA_TESTING_START);

    extra_testing ( run_name, job_id, rank, num_client_procs, num_timesteps, all_var_names, type_names, run_timestep_type_names, read_comm ); 

    add_timing_point(EXTRA_TESTING_DONE);
    add_timing_point(READING_DONE); 


    //extreme_debug_log << "starting output \n";
        
    return rc;

}

void read_init(
		const std::string &run_name,
		const std::string &job_id,
		const std::vector<uint64_t> &timestep_ids_to_fetch,
		const std::vector<std::string> &var_names,
		std::vector<std::string> &all_var_names,		
		std::vector<std::string> &file_names_to_fetch,
		std::vector<std::string> &var_names_to_fetch
		)
{
    add_timing_point(READING_INIT_START);

    int num_vars = 10;
    uint32_t version1 = 1;
    uint32_t version2 = 2;

    std::vector<md_var_entry> var_entries;
 	vector<uint64_t> var_ids_to_fetch = { 0, num_vars/6, num_vars/2, num_vars/4, num_vars/3, num_vars - 2, num_vars - 1};

    for (int i = 0; i < timestep_ids_to_fetch.size(); i++) {
    	uint64_t timestep_id = timestep_ids_to_fetch.at(i);
    	string FILENAME = run_name + "/" + job_id + "/" + to_string(timestep_id);
    	file_names_to_fetch.push_back(FILENAME);
    }

 //    md_timestep_entry timestep;
 //    open_file_for_read(file_names_to_fetch.at(0), timestep.file_id);
	// metadata_catalog_var (timestep.file_id, var_entries);
 //    close_timestep_file(timestep.file_id);

    for (int var_indx = 0; var_indx < num_vars; var_indx++) {
    	uint32_t var_version;
    	if (var_indx == 1 || var_indx == 3) {
    		var_version = version2;
    	}
    	else {
    		var_version = version1;
    	}
	   	all_var_names.push_back(var_names.at(var_indx) + to_string(var_version));
    }

    for (int i = 0; i < var_ids_to_fetch.size(); i++) {
    	uint64_t var_indx = var_ids_to_fetch.at(i);
	   	string VARNAME = all_var_names.at(var_indx);
	   	var_names_to_fetch.push_back(VARNAME);
    }


    //testing_log << "Starting read patterns for types \n";

    add_timing_point(FIND_VARS_DONE);
}

