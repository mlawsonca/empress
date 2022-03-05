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

#include <3d_read_for_testing_class_proj.hh>
#include <my_metadata_client.h>
// #include <md_client_timing_constants.hh>
#include <client_timing_constants_read_class_proj.hh>
// #include <my_metadata_client_lua_functs.h>


using namespace std;

// static bool extreme_debug_logging = false;
// static bool debug_logging = false;
// static bool zero_rank_logging = false;
// static bool testing_logging = false;
static bool error_logging  = true;

static debugLog error_log = debugLog(error_logging, false);
// static debugLog debug_log = debugLog(debug_logging, zero_rank_logging);
// static debugLog extreme_debug_log = debugLog(extreme_debug_logging, false);
// static debugLog testing_log = debugLog(testing_logging, zero_rank_logging);

//link to testing_debug.cpp in cmake to do debug tests
// bool do_debug_testing = false;
extern void add_timing_point(int catg);

int extra_testing (const md_server &server, int rank, uint32_t num_servers, uint32_t num_client_procs, uint32_t total_num_var_types,
                  MPI_Comm read_comm);

int read_init(md_server server, int rank, uint32_t num_servers, uint32_t num_client_procs,
		uint32_t num_x_procs, uint32_t num_y_procs, uint32_t num_z_procs,
		uint32_t num_timesteps_to_fetch, uint64_t txn_id, uint64_t *timestep_ids,
		MPI_Comm read_comm, MPI_Comm shared_server_comm, md_catalog_run_entry &run, 
		std::vector<md_dim_bounds> &proc_dims, int &plane_x_procs, int &plane_y_procs,
    	vector<vector<md_catalog_var_entry>> &all_var_entries,
		std::vector<md_catalog_var_entry> &vars_to_fetch_pattern2,
		std::vector<md_catalog_var_entry> &vars_to_fetch_pattern3, 
		std::vector<md_catalog_var_entry> &vars_to_fetch_pattern4,
		std::vector<md_catalog_var_entry> &vars_to_fetch_pattern5,
		std::vector<md_catalog_var_entry> &vars_to_fetch_pattern6
		);

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
int do_reads(md_server server, int rank, uint32_t num_x_procs, uint32_t num_y_procs, uint32_t num_z_procs,
		uint32_t num_timesteps, uint32_t num_servers, uint32_t num_client_procs, uint64_t txn_id,
		MPI_Comm read_comm, MPI_Comm shared_server_comm,
		const md_catalog_run_entry &run, const vector<md_dim_bounds> &proc_dims,
		uint32_t plane_x_procs, uint32_t plane_y_procs, const vector<vector<md_catalog_var_entry>> &all_var_entries,
		const vector<md_catalog_var_entry> &vars_to_fetch_pattern2, 
		const vector<md_catalog_var_entry> &vars_to_fetch_pattern3,
		const vector<md_catalog_var_entry> &vars_to_fetch_pattern4, 
		const vector<md_catalog_var_entry> &vars_to_fetch_pattern5,
		const vector<md_catalog_var_entry> &vars_to_fetch_pattern6
	) 
{
	int rc = RC_OK;

    vector<uint64_t> type_ids_to_read = {1, 2, 3};
    uint32_t total_num_var_types = 10;

   
    // MPI_Comm shared_server_comm;
   	// if(num_servers > 1) {
    // 	MPI_Comm_split(read_comm, rank%num_servers, rank, &shared_server_comm);
    // }
    // else {
    // 	shared_server_comm = read_comm;
    // }


    //extreme_debug_log.set_rank(rank);
    //debug_log.set_rank(rank);
    error_log.set_rank(rank);
    //testing_log.set_rank(rank);


    //debug_log << "rank: " << rank << " about to barrier the read comm" << endl;

    MPI_Barrier(read_comm);
    //debug_log << "rank: " << rank << " just finished first barrier for read comm" << endl;

    add_timing_point(READING_START);

    rc = read_pattern_1 (rank, proc_dims, num_servers, num_client_procs, server,
                 txn_id, run, all_var_entries.at(0), type_ids_to_read, read_comm); 
    if(rc != RC_OK) {
        add_timing_point(ERR_PATTERN_1);
    }
    //debug_log << "finished pattern 1t" << endl;


    MPI_Barrier(read_comm);
    if(vars_to_fetch_pattern2.size() != 1) {
        error_log << "Error. For pattern 2, expecting vars_to_fetch.size() to equal 1 but instead it equals " << vars_to_fetch_pattern2.size() << endl;
    }
    else {
        //debug_log << "txn_id: " << txn_id << " varid to fetch: " << vars_to_fetch_pattern2.at(0).var_id << endl;
        rc = read_pattern_2 (rank, proc_dims, num_servers, num_client_procs, server,
                 txn_id, run, vars_to_fetch_pattern2.at(0), type_ids_to_read, read_comm); 
        //debug_log << "finished pattern 2" << endl;
        if(rc != RC_OK) {
            add_timing_point(ERR_PATTERN_2);
        }
    }

    MPI_Barrier(read_comm);
    if(vars_to_fetch_pattern3.size() != 3) {
        error_log << "Error. For pattern 3, expecting vars_to_fetch.size() to equal 3 but instead it equals " << vars_to_fetch_pattern3.size() << endl;
    }
    else {
        rc = read_pattern_3 (rank, proc_dims, num_servers, num_client_procs, server,
                txn_id, run, vars_to_fetch_pattern3, type_ids_to_read, read_comm);
        //debug_log << "finished pattern 3t" << endl;
        if(rc != RC_OK) {
            add_timing_point(ERR_PATTERN_3); 
        }   
    }

    MPI_Barrier(read_comm);
    if(vars_to_fetch_pattern4.size() != 1) {
        error_log << "Error. For pattern 4, expecting vars_to_fetch.size() to equal 1 but instead it equals " << vars_to_fetch_pattern4.size() << endl;
    }
    else {
        rc = read_pattern_4 (rank, plane_x_procs, plane_y_procs, num_servers, num_client_procs,
        		server, txn_id, run, vars_to_fetch_pattern4.at(0), type_ids_to_read, read_comm); 
        //debug_log << "finished pattern 4t" << endl;
        if(rc != RC_OK) {
            add_timing_point(ERR_PATTERN_4);
        }
    }

    MPI_Barrier(read_comm);
    if(vars_to_fetch_pattern5.size() != 1) {
        error_log << "Error. For pattern 5, expecting vars_to_fetch.size() to equal 1 but instead it equals " << vars_to_fetch_pattern5.size() << endl;
    }
    else {
        rc = read_pattern_5 (rank, num_x_procs, num_y_procs, num_z_procs, 
        		num_servers, num_client_procs,
        		server, txn_id, run, vars_to_fetch_pattern5.at(0), type_ids_to_read, read_comm); 
        //debug_log << "finished pattern 5t" << endl;
        if(rc != RC_OK) {
            add_timing_point(ERR_PATTERN_5);  
        }
    }

    MPI_Barrier(read_comm);
    if(vars_to_fetch_pattern6.size() != 1) {
        error_log << "Error. For pattern 6, expecting vars_to_fetch.size() to equal 1 but instead it equals " << vars_to_fetch_pattern6.size() << endl;
    }
    else {

        rc = read_pattern_6 (rank, plane_x_procs, plane_y_procs, num_servers, num_client_procs,
        		server, txn_id, run, vars_to_fetch_pattern6.at(0), type_ids_to_read, read_comm); 
        //debug_log << "finished pattern 6t" << endl;
        if(rc != RC_OK) {
            add_timing_point(ERR_PATTERN_6);
        }
    }    

    add_timing_point(READING_PATTERNS_DONE);

    MPI_Barrier(read_comm);
    add_timing_point(EXTRA_TESTING_START);

    //debug_log << "rank: " << rank << " about to do extra testing" << endl;

    rc = extra_testing ( server,rank, num_servers, num_client_procs, total_num_var_types, read_comm); 
    if (rc != RC_OK) {
        error_log << "Error with extra_testing" << endl;
    }
    //debug_log << "rank: " << rank << " done with extra testing" << endl;

    add_timing_point(EXTRA_TESTING_DONE);
    add_timing_point(READING_DONE); 


    //extreme_debug_log << "starting output" << endl;
        
    return rc;

}


int read_init(md_server server, int rank, uint32_t num_servers, uint32_t num_client_procs,
		uint32_t num_x_procs, uint32_t num_y_procs, uint32_t num_z_procs,
		uint32_t num_timesteps_to_fetch, uint64_t txn_id, uint64_t *timestep_ids,
		MPI_Comm read_comm, MPI_Comm shared_server_comm,
		md_catalog_run_entry &run, 
		std::vector<md_dim_bounds> &proc_dims,
		int &plane_x_procs,
		int &plane_y_procs,
    	vector<vector<md_catalog_var_entry>> &all_var_entries,
		std::vector<md_catalog_var_entry> &vars_to_fetch_pattern2,
		std::vector<md_catalog_var_entry> &vars_to_fetch_pattern3, 
		std::vector<md_catalog_var_entry> &vars_to_fetch_pattern4,
		std::vector<md_catalog_var_entry> &vars_to_fetch_pattern5,
		std::vector<md_catalog_var_entry> &vars_to_fetch_pattern6
		)
   
{
    add_timing_point(READING_INIT_START);

	int rc = RC_OK;
    uint32_t total_x_length, total_y_length, total_z_length;
    uint32_t x_length_per_proc, y_length_per_proc, z_length_per_proc;
    uint32_t x_pos, y_pos, z_pos;
    uint32_t x_offset, y_offset, z_offset;

    char *serialized_c_str;
    int num_bytes_and_entries_and_sqrt[3];
    all_var_entries.reserve(num_timesteps_to_fetch);

    bool var_found;
    uint64_t var_ids[3];

    uint32_t num_runs;
    vector<md_catalog_run_entry> runs;

    int ser_run_catalog_len;
    if (rank < num_servers) {
     	std::string serial_str;

        stringstream ss;
        boost::archive::text_oarchive oa(ss);

    	rc = metadata_catalog_run(server, txn_id, num_runs, runs);
	    if (rc != RC_OK) {
	        add_timing_point(ERR_CATALOG_RUN); 
	        return RC_ERR;
	        // goto cleanup;                 
	        error_log << "Error cataloging the run entries. Proceeding" << endl;
	    }
        oa << runs;
        serial_str = ss.str();
        serialized_c_str = (char *) serial_str.c_str();
        //extreme_debug_log << "runs.size(): " << runs.size() << endl;
        //extreme_debug_log << "bcast run catalog for rank: " << rank << " serialized_c_str: " << serialized_c_str << endl;
        //extreme_debug_log << "serial_str.size(): " << serial_str.size() << endl;
        ser_run_catalog_len = serial_str.size() + 1;
    }
    MPI_Bcast(&ser_run_catalog_len, 1, MPI_INT, 0, shared_server_comm);

    if(rank >= num_servers) {
        // //extreme_debug_log << "ser_run_catalog_len: " << ser_run_catalog_len << endl;   	
        serialized_c_str = (char *) malloc(ser_run_catalog_len);  
    }

    MPI_Bcast(serialized_c_str, ser_run_catalog_len, MPI_CHAR, 0, shared_server_comm);

    if (rank >= num_servers) {
        //extreme_debug_log << "bcast run catalog serialized_c_str: " << serialized_c_str << endl;
        if(ser_run_catalog_len > 1) {
            stringstream ss1;
            //could have embedded nulls
            ss1.write(serialized_c_str, ser_run_catalog_len);
            // ss1 << serialized_c_str; 
            boost::archive::text_iarchive ia(ss1);
            ia >> runs;  
        }
        free(serialized_c_str);
    }
    add_timing_point(BCAST_RUNS_DONE);

    MPI_Barrier(read_comm);
    add_timing_point(BCAST_VAR_CATALOGS_START);

    run = runs.at(0); //todo - if we add more than 1 run this will need to be changed

    //CATALOG ENTRIES AND BCAST
   if (rank < num_servers) {
     	std::string serial_str;

        std::vector<md_catalog_var_entry> entries;
        stringstream ss;
        boost::archive::text_oarchive oa(ss);
        uint32_t num_vars;
        // serial_str;
        for(int i=0; i<num_timesteps_to_fetch; i++) {
            //extreme_debug_log << "about to catalog var for run_id: " << run.run_id << " timestep_id: " << timestep_ids[i] << endl;
            //returns information about the variables associated with the given txn_id 
            rc = metadata_catalog_var(server, run.run_id, timestep_ids[i], txn_id, num_vars, entries);
            if (rc != RC_OK) {
                add_timing_point(ERR_CATALOG_VAR);
                return RC_ERR; 
                // goto cleanup;                 
                error_log << "Error cataloging the var entries. Proceeding" << endl;
            }

            //extreme_debug_log << "entries.size(): " << entries.size() << endl;
            all_var_entries.push_back(entries);
        }

        oa << all_var_entries;
        //extreme_debug_log << "total num read procs: " << to_string(num_client_procs) << endl;
        serial_str = ss.str();
        serialized_c_str = (char *) serial_str.c_str();
        int my_sqrt = floor(sqrt(num_client_procs));
        while(num_client_procs % my_sqrt != 0) {
            my_sqrt -= 1;
        }

        num_bytes_and_entries_and_sqrt[0] = serial_str.size() + 1;
        num_bytes_and_entries_and_sqrt[1] = num_vars;
        num_bytes_and_entries_and_sqrt[2] = my_sqrt;
        //extreme_debug_log <<  "my_sqrt: " << to_string(my_sqrt) << endl;
    }
    MPI_Bcast(num_bytes_and_entries_and_sqrt, 3, MPI_INT, 0, shared_server_comm);

    if(rank >= num_servers) {
        //extreme_debug_log << "num_bytes is "<< to_string(num_bytes_and_entries_and_sqrt[0]) + " and rank: " << to_string(rank) << endl;
        serialized_c_str = (char *) malloc(num_bytes_and_entries_and_sqrt[0]);  
        //extreme_debug_log << "num of vars is " << to_string(num_bytes_and_entries_and_sqrt[1]) << endl;
    }

    MPI_Bcast(serialized_c_str, num_bytes_and_entries_and_sqrt[0], MPI_CHAR, 0, shared_server_comm);

    if (rank >= num_servers) {
        //extreme_debug_log << "bcast var catalog serialized_c_str: " << (string)serialized_c_str << endl;
        if(num_bytes_and_entries_and_sqrt[0] > 1) {
            stringstream ss1;
            ss1 << serialized_c_str; 
            boost::archive::text_iarchive ia(ss1);
            ia >> all_var_entries;  
        }
        free(serialized_c_str);
    }

    add_timing_point(BCAST_VAR_CATALOGS_DONE);

    //todo - if vars start having different dimensions, should adjust proc_dims per read/var
    total_x_length = all_var_entries.at(0).at(0).dims[0].max - all_var_entries.at(0).at(0).dims[0].min + 1; 
    total_y_length = all_var_entries.at(0).at(0).dims[1].max - all_var_entries.at(0).at(0).dims[1].min + 1;
    total_z_length = all_var_entries.at(0).at(0).dims[2].max - all_var_entries.at(0).at(0).dims[2].min + 1;

    x_length_per_proc = total_x_length / num_x_procs; //todo - deal with edge case?
    y_length_per_proc = total_y_length / num_y_procs;
    z_length_per_proc = total_z_length / num_z_procs;

    x_pos = rank % num_x_procs;
    y_pos = (rank / num_x_procs) % num_y_procs;
    z_pos = ((rank / (num_x_procs * num_y_procs)) % num_z_procs);
    x_offset = x_pos * x_length_per_proc;
    y_offset = y_pos * y_length_per_proc;
    z_offset = z_pos * z_length_per_proc;
    
    //extreme_debug_log << "x_pos: " << x_pos << endl;
    //extreme_debug_log << "y_pos: " << y_pos << endl;
    //extreme_debug_log << "z_pos: " << z_pos << endl;
    //extreme_debug_log << "x_offset : " << x_offset  << endl;
    //extreme_debug_log << "y_offset : " << y_offset  << endl;
    //extreme_debug_log << "z_offset : " << z_offset  << endl;
    //extreme_debug_log << "x_length_per_proc : " << x_length_per_proc  << endl;
    //extreme_debug_log << "y_length_per_proc : " << y_length_per_proc  << endl;
    //extreme_debug_log << "z_length_per_proc : " << z_length_per_proc  << endl;
  
    proc_dims [0].min = x_offset;
    proc_dims [0].max = x_offset + x_length_per_proc - 1;
    proc_dims [1].min = y_offset;
    proc_dims [1].max = y_offset + y_length_per_proc - 1;
    proc_dims [2].min = z_offset;
    proc_dims [2].max = z_offset + z_length_per_proc - 1;

    // for (int j=0; j<3; j++) {
        //debug_log <<  "proc_dims [" << to_string(j) << "] min is: " << to_string(proc_dims [j].min) << endl;
        //debug_log <<  "proc_dims [" << to_string(j) << "] max is: " << to_string(proc_dims [j].max) << endl;
    // }        

    var_found = false;

    for (md_catalog_var_entry var : all_var_entries.at(1)) {
        uint64_t var_id0 = 0; 
        var_ids[0] = var_id0;
        if (var.var_id == var_id0) {
            vars_to_fetch_pattern2.push_back(var);
            var_found = true;
            break;
        }
    }
    if (! var_found) {
        error_log << "error. var_id " << var_ids[0] << " not found for pattern 2. returning" << endl;
        return -1;
    }

    var_found = false;
    for (md_catalog_var_entry var : all_var_entries.at(2)) {
        uint32_t num_vars = all_var_entries.at(2).size();
        uint64_t var_id1 = num_vars/6;
        uint64_t var_id2 = num_vars/2;
        uint64_t var_id3 = num_vars/4;
        var_ids[0] = var_id1;
        var_ids[1] = var_id2;
        var_ids[2] = var_id3;

        if (var.var_id == var_id1 || var.var_id == var_id2 || var.var_id == var_id3 ) {
            vars_to_fetch_pattern3.push_back(var);
        }
        if (vars_to_fetch_pattern3.size() == 3) {
            var_found = true;
            break;
        }
    }
    if (! var_found) {
        error_log << "error. var_ids " << var_ids[0] << ", " << var_ids[1] << ", " << var_ids[2] << " not found for pattern 3. returning" << endl;
        error_log << "cataloging all_var_entries.at(2)" << endl;
        for (int i = 0; i< all_var_entries.at(2).size(); i++) {
            error_log << "all_var_entries.at(2).at(i).var_id: " << all_var_entries.at(2).at(i).var_id << endl;
        }
        return -1;
    }

    var_found = false;
    for (md_catalog_var_entry var : all_var_entries.at(3)) {
        uint64_t var_id4 = all_var_entries.at(3).size()/3;
        var_ids[0] = var_id4;

        if (var.var_id == var_id4 ) {
            vars_to_fetch_pattern4.push_back(var);
            var_found = true;
            break;
        }
    }
    if (! var_found) {
        error_log << "error. var_id " << var_ids[0] << " not found for pattern 4. returning" << endl;
        return RC_OK;
    }

    var_found = false;
    for (md_catalog_var_entry var : all_var_entries.at(4)) {
        uint64_t var_id5 = all_var_entries.at(4).size() - 2;
        var_ids[0] = var_id5;

        if (var.var_id == var_id5 ) {
            vars_to_fetch_pattern5.push_back(var);
            var_found = true;
            break;
        }
    }
    if (! var_found) {
        error_log << "error. var_id " << var_ids[0] << " not found for pattern 5. returning" << endl;
        return RC_ERR;
    }

    var_found = false;
    for (md_catalog_var_entry var : all_var_entries.at(5)) {
        uint64_t var_id6 = all_var_entries.at(5).size() - 1;
        var_ids[0] = var_id6;

        if (var.var_id == var_id6 ) {
            vars_to_fetch_pattern6.push_back(var);
            var_found = true;
            break;
        }
    }
    if (! var_found) {
        error_log << "error. var_id " << var_ids[0] << " not found for pattern 6. returning" << endl;
        return RC_ERR;
    }

    plane_x_procs = num_bytes_and_entries_and_sqrt[2];
    plane_y_procs = num_client_procs / plane_x_procs;

    //testing_log << "Starting read patterns for types" << endl;

    add_timing_point(FIND_VARS_DONE);

    return rc;
}

