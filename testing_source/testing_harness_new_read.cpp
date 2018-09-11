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

#include <Globals.hh>
#include <opbox/services/dirman/DirectoryManager.hh>
#include <gutties/Gutties.hh>

#include <sstream>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

#include <3d_read_for_testing.hh>
#include <my_metadata_client.h>
// #include <client_timing_constants.hh>
#include <client_timing_constants_read.hh>
#include <my_metadata_client_lua_functs.h>


  //dw settings
std::string default_config_string = R"EOF(
# Tell the nnti transport to use infiniband

nnti.logger.severity       error
nnti.transport.name        ibverbs
webhook.interfaces         ib0,lo
 
#config.additional_files.env_name.if_defined   CONFIG
lunasa.lazy_memory_manager   malloc
lunasa.eager_memory_manager  tcmalloc

# Select the type of dirman to use. Currently we only have centralized, which
# just sticks all the directory info on one node (called root). 
dirman.type           centralized

# Turn these on if you want to see more debug messages
#bootstrap.debug           true
#webhook.debug             true
#opbox.debug               true
#dirman.debug              true
#dirman.cache.others.debug true
#dirman.cache.mine.debug   true

)EOF";

using namespace std;

static bool extreme_debug_logging = false;
static bool debug_logging = false;
static bool error_logging  = true;
static bool zero_rank_logging = false;
static bool testing_logging = false;

debugLog error_log = debugLog(error_logging, zero_rank_logging);
debugLog debug_log = debugLog(debug_logging, zero_rank_logging);
debugLog extreme_debug_log = debugLog(extreme_debug_logging, false);
debugLog testing_log = debugLog(testing_logging, zero_rank_logging);

//link to testing_debug.cpp in cmake to do debug tests
// bool do_debug_testing = false;

static bool output_timing = true;

bool hdf5_read_data = true;

bool output_objector_params = false;
bool output_obj_names = false;

std::vector<int> catg_of_time_pts;
std::vector<long double> time_pts;

std::vector<int> all_num_params_to_fetch; //keeps track of how many calls to the objector should be made for a given query (how many attrs)
vector<objector_params> all_objector_params; //keeps track of the parameters that would have been given to the objector

vector<vector<string>> all_object_names; //keeps track of all of the obj names returned by an objector call
vector<vector<uint64_t>> all_offsets_and_counts; //keeps track of all of the offsets and counts returned by an objector call
vector<int> all_read_types; //keeps track, when the objector outputs a list of names, what the md query was
vector<int> all_num_objects_to_fetch; //keeps track, of the number of obj names produced for each objector call

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


//used wherever the objector should have been called
void add_objector_point(int catg, int num_params_to_fetch) {
    if (output_timing) {
        add_timing_point(catg);
    }
    all_num_params_to_fetch.push_back(num_params_to_fetch);
}


//used wherever the objector was called
void add_objector_output_point(int catg, int num_params_to_fetch, uint16_t read_type) {
    if (output_timing) {
        add_timing_point(catg);
    }
    all_num_objects_to_fetch.push_back(num_params_to_fetch);
    all_read_types.push_back(read_type);
}



static int setup_dirman(const string &dirman_file_path, const string &dir_path, md_server &dirman, vector<gutties::name_and_node_t> &server_nodes, int rank, uint32_t num_servers, uint32_t num_clients);
// static void setup_servers(std::vector<md_server> &servers, uint32_t num_servers, const vector<gutties::name_and_node_t> &server_nodes);
static void setup_server(int rank, uint32_t num_servers, const vector<gutties::name_and_node_t> &server_nodes, md_server &server);
int extra_testing_collective(const md_server &server, int rank, uint32_t num_servers, uint32_t num_client_procs, const md_catalog_run_entry &run, 
                  const vector<md_dim_bounds> &chunk_dims);

void gather_and_print_output_params(const vector<objector_params> &params, int rank, uint32_t num_client_procs);
void gather_and_print_object_names(const vector<vector<string>> &all_object_names, const vector<vector<uint64_t>> &all_offsets_and_counts,
                            const vector<int> &all_read_types,
                            int rank, uint32_t num_client_procs, 
                            uint32_t num_timesteps, uint32_t num_vars);
void gatherv_int(vector<int> values, uint32_t num_client_procs, int rank, int *each_proc_num_values,
            int *displacement_for_each_proc, int **all_values);
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
int main(int argc, char **argv) {
    int rc;
    // char name[100];
    // gethostname(name, sizeof(name));
    // extreme_debug_log << name << endl;

    if (argc != 8) { 
        error_log << "Error. Program should be given 7 arguments. Dirman filepath, npx, npy, npz, number of timesteps, estm num time_pts, num_servers" << endl;
        cout << (int)ERR_ARGC << " " << 0 <<  endl;
        return RC_ERR;
    }

    string dirman_file_path = argv[1];
    uint32_t num_x_procs = stoul(argv[2],nullptr,0);
    uint32_t num_y_procs = stoul(argv[3],nullptr,0);
    uint32_t num_z_procs = stoul(argv[4],nullptr,0);
    //todo - these might not always be constant across all vars. should calculate proc dims per var
    // uint64_t total_x_length = stoull(argv[5],nullptr,0);
    // uint64_t total_y_length = stoull(argv[6],nullptr,0);
    // uint64_t total_z_length = stoull(argv[7],nullptr,0);
    uint32_t num_timesteps = stoul(argv[5],nullptr,0); //todo - just get this from catalogging timestep?
    // uint32_t num_types = stoul(argv[9],nullptr,0);
    uint32_t estm_num_time_pts = stoul(argv[6],nullptr,0); 
    uint32_t num_servers = stoul(argv[7],nullptr,0);


    catg_of_time_pts.reserve(estm_num_time_pts);
    time_pts.reserve(estm_num_time_pts);
    struct timeval now;
    gettimeofday(&now, NULL);
    zero_time_sec = 86400 * (now.tv_sec / 86400);
    
    // struct timeval start, mpi_init_done, register_ops_done, dirman_init_done, server_setup_done, pattern_read_end, read_end;
    // int num_wall_time_pts = 7;
    add_timing_point(PROGRAM_START);

    MPI_Init(&argc, &argv);

    int rank;
    int num_client_procs;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size( MPI_COMM_WORLD, &num_client_procs);

    add_timing_point(MPI_INIT_DONE);

    // gettimeofday(&mpi_init_done, NULL);
    // clock_times.push_back( (mpi_init_done.tv_sec - zero_time_sec) + mpi_init_done.tv_usec / 1000000.0);

    extreme_debug_log.set_rank(rank);
    debug_log.set_rank(rank);
    error_log.set_rank(rank);
    testing_log.set_rank(rank);

    rc = metadata_init_read_and_delete ();
    

    add_timing_point(MD_CLIENT_INIT_DONE);
    // gettimeofday(&register_ops_done, NULL);
    // clock_times.push_back( (register_ops_done.tv_sec - zero_time_sec) + register_ops_done.tv_usec / 1000000.0);

    if(rank == 0) {
        extreme_debug_log << "starting" << endl;
    }
    string dir_path="/metadata/testing";
    md_server dirman;

    // vector<md_dim_bounds> dims(3);

    uint64_t run_id = 1;
    uint64_t txn_id = -1;

    vector<gutties::name_and_node_t> server_nodes;
    server_nodes.resize(num_servers);

    srand(rank);

    //just for testing-------------------------------------------
    long double *all_time_pts_buf;
    int *all_catg_time_pts_buf;
    // long double *all_clock_times;

    // double *total_times; 
    //--------------------------------------------------------------


    std::string serial_str;
    vector<md_server> servers;
    md_server server;

    uint32_t total_x_length, total_y_length, total_z_length;
    uint32_t x_length_per_proc, y_length_per_proc, z_length_per_proc;
    uint32_t x_pos, y_pos, z_pos;
    uint32_t x_offset, y_offset, z_offset;
    uint32_t num_dims = 3;
    vector<md_dim_bounds> proc_dims(num_dims);

    const int num_timesteps_to_fetch = 6;
    // uint64_t timestep_ids[num_timesteps_to_fetch] = {(num_timesteps-1) / 6, (num_timesteps-1) / 5, (num_timesteps-1) / 4,
    //                     (num_timesteps-1) / 3, (num_timesteps-1) / 2, num_timesteps-1};
    uint64_t timestep_ids[num_timesteps_to_fetch] = {(num_timesteps-1) / 6, (num_timesteps-1) / 5, (num_timesteps-1) / 4,
                        (num_timesteps-1) / 3, (num_timesteps-1) / 2, num_timesteps-1};    
    char *serialized_c_str;
    int num_bytes_and_entries_and_sqrt[3];
    vector<vector<md_catalog_var_entry>> all_var_entries;
    all_var_entries.reserve(num_timesteps_to_fetch);
   

    std::vector<md_catalog_var_entry> vars_to_fetch_pattern2;
    std::vector<md_catalog_var_entry> vars_to_fetch_pattern3;
    std::vector<md_catalog_var_entry> vars_to_fetch_pattern4;
    std::vector<md_catalog_var_entry> vars_to_fetch_pattern5;
    std::vector<md_catalog_var_entry> vars_to_fetch_pattern6;

    // std::vector<string> var_names_to_fetch(num_vars_to_fetch);
    // std::vector<uint32_t> var_vers_to_fetch(num_vars_to_fetch);
    int plane_x_procs;
    int plane_y_procs;


    uint32_t num_runs;
    vector<md_catalog_run_entry> runs;
    md_catalog_run_entry run;

    bool var_found;
    uint64_t var_ids[3];

    vector<uint64_t> type_ids_to_read = {1, 2, 3};

    all_objector_params.reserve(num_client_procs * 20);
    all_object_names.reserve(100 * num_client_procs * 20);
    all_num_objects_to_fetch.reserve(num_client_procs * 20);
    all_offsets_and_counts.reserve(6 * 100 * num_client_procs * 20);

    // uint32_t num_essential_params;

    add_timing_point(INIT_VARS_DONE);

    rc = setup_dirman(dirman_file_path, dir_path, dirman, server_nodes, rank, num_servers, num_client_procs);
    if (rc != RC_OK) {
        add_timing_point(ERR_DIRMAN);
        error_log << "Error. Was unable to setup the dirman. Exiting" << endl;
        goto cleanup;
    }

    add_timing_point(DIRMAN_SETUP_DONE);
    // gettimeofday(&dirman_init_done, NULL);
    // clock_times.push_back( (dirman_init_done.tv_sec - zero_time_sec) + dirman_init_done.tv_usec / 1000000.0);


    setup_server(rank, num_servers, server_nodes, server);

    // setup_servers(servers, num_servers, server_nodes);
    add_timing_point(SERVER_SETUP_DONE_INIT_DONE);

    // gettimeofday(&server_setup_done, NULL);
    // clock_times.push_back( (server_setup_done.tv_sec - zero_time_sec) + server_setup_done.tv_usec / 1000000.0);

    // server = servers.at(rank % num_servers); 

    MPI_Barrier(MPI_COMM_WORLD);
    add_timing_point(READING_START);

    // add_timing_point(CATALOG_RUN_START);
    rc = metadata_catalog_run(server, txn_id, num_runs, runs);
    if (rc != RC_OK) {
        add_timing_point(ERR_CATALOG_RUN); 
        goto cleanup;                 
        error_log << "Error cataloging the run entries. Proceeding \n";
    }
    add_timing_point(CATALOG_RUN_DONE);

    // MPI_Barrier(MPI_COMM_WORLD);
    // add_timing_point(BCAST_CATALOGS_START);

    run = runs.at(0); //todo - if we add more than 1 run this will need to be changed

    //CATALOG ENTRIES AND BCAST
   if (rank == 0) {
        std::vector<md_catalog_var_entry> entries;
        stringstream ss;
        boost::archive::text_oarchive oa(ss);
        uint32_t num_vars;
        serial_str;
        for(int i=0; i<num_timesteps_to_fetch; i++) {
            extreme_debug_log << "about to catalog var for run_id: " << run_id << " timestep_id: " << timestep_ids[i] << endl;
            //returns information about the variables associated with the given txn_id 
            rc = metadata_catalog_var(server, run_id, timestep_ids[i], txn_id, num_vars, entries);
            if (rc != RC_OK) {
                add_timing_point(ERR_CATALOG_VAR); 
                goto cleanup;                 
                error_log << "Error cataloging the var entries. Proceeding \n";
            }
            extreme_debug_log << "entries.size(): " << entries.size() << endl;
            all_var_entries.push_back(entries);
        }

        oa << all_var_entries;
        extreme_debug_log << "total num read procs: " << to_string(num_client_procs) << endl;
        serial_str = ss.str();
        serialized_c_str = (char *) serial_str.c_str();
        int my_sqrt = floor(sqrt(num_client_procs));
        while(num_client_procs % my_sqrt != 0) {
            my_sqrt -= 1;
        }

        num_bytes_and_entries_and_sqrt[0] = serial_str.size() + 1;
        num_bytes_and_entries_and_sqrt[1] = num_vars;
        num_bytes_and_entries_and_sqrt[2] = my_sqrt;
        extreme_debug_log <<  "my_sqrt: " << to_string(my_sqrt) << endl;
    }
    MPI_Bcast(num_bytes_and_entries_and_sqrt, 3, MPI_INT, 0, MPI_COMM_WORLD);

    if(rank != 0) {
        extreme_debug_log << "num_bytes is "<< to_string(num_bytes_and_entries_and_sqrt[0]) + " and rank: " << to_string(rank) << endl;
        serialized_c_str = (char *) malloc(num_bytes_and_entries_and_sqrt[0]);  
        extreme_debug_log << "num of vars is " << to_string(num_bytes_and_entries_and_sqrt[1]) << endl;
    }

    MPI_Bcast(serialized_c_str, num_bytes_and_entries_and_sqrt[0], MPI_CHAR, 0, MPI_COMM_WORLD);

    if (rank != 0) {
        // extreme_debug_log << "serialized_c_str: " << (string)serialized_c_str << endl;
        if(num_bytes_and_entries_and_sqrt[0] > 1) {
            stringstream ss1;
            ss1 << serialized_c_str; 
            boost::archive::text_iarchive ia(ss1);
            ia >> all_var_entries;  
        }
        free(serialized_c_str);
    }

    add_timing_point(BCAST_CATALOGS_DONE);

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
    
    extreme_debug_log << "x_pos: " << x_pos << endl;
    extreme_debug_log << "y_pos: " << y_pos << endl;
    extreme_debug_log << "z_pos: " << z_pos << endl;
    extreme_debug_log << "x_offset : " << x_offset  << endl;
    extreme_debug_log << "y_offset : " << y_offset  << endl;
    extreme_debug_log << "z_offset : " << z_offset  << endl;
    extreme_debug_log << "x_length_per_proc : " << x_length_per_proc  << endl;
    extreme_debug_log << "y_length_per_proc : " << y_length_per_proc  << endl;
    extreme_debug_log << "z_length_per_proc : " << z_length_per_proc  << endl;
  
    proc_dims [0].min = x_offset;
    proc_dims [0].max = x_offset + x_length_per_proc - 1;
    proc_dims [1].min = y_offset;
    proc_dims [1].max = y_offset + y_length_per_proc - 1;
    proc_dims [2].min = z_offset;
    proc_dims [2].max = z_offset + z_length_per_proc - 1;

    for (int j=0; j<3; j++) {
        debug_log <<  "proc_dims [" << to_string(j) << "] min is: " << to_string(proc_dims [j].min) << endl;
        debug_log <<  "proc_dims [" << to_string(j) << "] max is: " << to_string(proc_dims [j].max) << endl;
    }        

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
        error_log << "error. var_id " << var_ids[0] << " not found for pattern 2. returning \n";
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
        error_log << "error. var_ids " << var_ids[0] << ", " << var_ids[1] << ", " << var_ids[2] << " not found for pattern 3. returning \n";
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
        error_log << "error. var_id " << var_ids[0] << " not found for pattern 4. returning \n";
        return -1;
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
        error_log << "error. var_id " << var_ids[0] << " not found for pattern 5. returning \n";
        return -1;
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
        error_log << "error. var_id " << var_ids[0] << " not found for pattern 6. returning \n";
        return -1;
    }

    add_timing_point(FIND_VARS_DONE);

    MPI_Barrier(MPI_COMM_WORLD);
    add_timing_point(READ_PATTERN_1_START);
    //pattern testing for vars
    rc = read_pattern_1 ( proc_dims, run, all_var_entries.at(0), 
            num_bytes_and_entries_and_sqrt[1], txn_id);
    if(rc != RC_OK) {
        add_timing_point(ERR_PATTERN_1);
    }
    debug_log << "finished pattern 1" << endl;
    add_timing_point(READ_PATTERN_1_DONE);

    MPI_Barrier(MPI_COMM_WORLD);
    if(vars_to_fetch_pattern2.size() != 1) {
        error_log << "Error. For pattern 2, expecting vars_to_fetch.size() to equal 1 but instead it equals " << vars_to_fetch_pattern2.size() << endl;
    }
    else {
        debug_log << "txn_id: " << txn_id << " varid to fetch: " << vars_to_fetch_pattern2.at(0).var_id << endl;

        add_timing_point(READ_PATTERN_2_START);
        rc = read_pattern_2 (rank, proc_dims, num_servers, num_client_procs, server, false,
                 txn_id, run, vars_to_fetch_pattern2.at(0), type_ids_to_read); 
        debug_log << "finished pattern 2" << endl;
        if(rc != RC_OK) {
            add_timing_point(ERR_PATTERN_2);
        }
        add_timing_point(READ_PATTERN_2_DONE);
    }

    MPI_Barrier(MPI_COMM_WORLD);
    if(vars_to_fetch_pattern3.size() != 3) {
        error_log << "Error. For pattern 3, expecting vars_to_fetch.size() to equal 3 but instead it equals " << vars_to_fetch_pattern3.size() << endl;
    }
    else {
        add_timing_point(READ_PATTERN_3_START);
        rc = read_pattern_3 (rank, proc_dims, num_servers, num_client_procs, server, false,
                txn_id, run, vars_to_fetch_pattern3, type_ids_to_read);
        debug_log << "finished pattern 3" << endl;
        if(rc != RC_OK) {
            add_timing_point(ERR_PATTERN_3); 
        }
        add_timing_point(READ_PATTERN_3_DONE);   
    }

    //want to use all read procs in pattern 4 and 6 as "x and y" procs
    extreme_debug_log << "num_client_procs: " << num_client_procs << endl;
    plane_x_procs = num_bytes_and_entries_and_sqrt[2];
    plane_y_procs = num_client_procs / plane_x_procs;
    extreme_debug_log << "num x procs: " << num_x_procs << " num y procs: "; 
    extreme_debug_log << num_y_procs << " num z procs: " << num_z_procs << " num x procs plane: ";
    extreme_debug_log << plane_x_procs << " num y procs plane: " << plane_y_procs << endl;

    MPI_Barrier(MPI_COMM_WORLD);
    if(vars_to_fetch_pattern4.size() != 1) {
        error_log << "Error. For pattern 4, expecting vars_to_fetch.size() to equal 1 but instead it equals " << vars_to_fetch_pattern4.size() << endl;
    }
    else {
        add_timing_point(READ_PATTERN_4_START);
        rc = read_pattern_4 (rank, plane_x_procs, plane_y_procs,
                    total_x_length, total_y_length, total_z_length, txn_id, run, vars_to_fetch_pattern4.at(0));

        debug_log << "finished pattern 4" << endl;
        if(rc != RC_OK) {
            add_timing_point(ERR_PATTERN_4);
        }
        add_timing_point(READ_PATTERN_4_DONE);
    }

    MPI_Barrier(MPI_COMM_WORLD);
    if(vars_to_fetch_pattern5.size() != 1) {
        error_log << "Error. For pattern 5, expecting vars_to_fetch.size() to equal 1 but instead it equals " << vars_to_fetch_pattern5.size() << endl;
    }
    else {
        add_timing_point(READ_PATTERN_5_START);
        rc = read_pattern_5( rank, num_x_procs, num_y_procs, num_z_procs,
                        total_x_length, total_y_length, total_z_length, txn_id, run, vars_to_fetch_pattern5.at(0));
        debug_log << "finished pattern 5" << endl;
        if(rc != RC_OK) {
            add_timing_point(ERR_PATTERN_5);  
        }
        add_timing_point(READ_PATTERN_5_DONE);
    }

    MPI_Barrier(MPI_COMM_WORLD);
    if(vars_to_fetch_pattern6.size() != 1) {
        error_log << "Error. For pattern 6, expecting vars_to_fetch.size() to equal 1 but instead it equals " << vars_to_fetch_pattern6.size() << endl;
    }
    else {
        add_timing_point(READ_PATTERN_6_START);
        rc = read_pattern_6 (rank, plane_x_procs, plane_y_procs,
                        total_x_length, total_y_length, total_z_length, txn_id, run, vars_to_fetch_pattern6.at(0));
        debug_log << "finished pattern 6" << endl;
        if(rc != RC_OK) {
            add_timing_point(ERR_PATTERN_6);
        }
        add_timing_point(READ_PATTERN_6_DONE);
    }
    add_timing_point(READING_PATTERNS_DONE);

    testing_log << "Starting read patterns for types \n";

    //pattern testing for types
    //pattern testing for vars
    // rc = read_pattern_1 (rank, num_servers, servers, true,
    //                 num_x_procs, num_y_procs, num_z_procs,
    //                 all_var_entries.at(0), num_bytes_and_entries_and_sqrt[1],
    //                 total_x_length, total_y_length, total_z_length, txn_id);
    // if(rc != RC_OK) {
    //     add_timing_point(ERR_PATTERN_1);
    // }
    // debug_log << "finished pattern 1t" << endl;

    MPI_Barrier(MPI_COMM_WORLD);
    if(vars_to_fetch_pattern2.size() != 1) {
        error_log << "Error. For pattern 2, expecting vars_to_fetch.size() to equal 1 but instead it equals " << vars_to_fetch_pattern2.size() << endl;
    }
    else {
        add_timing_point(READ_PATTERN_2_TYPES_START);
        debug_log << "txn_id: " << txn_id << " varid to fetch: " << vars_to_fetch_pattern2.at(0).var_id << endl;
        rc = read_pattern_2 (rank, proc_dims, num_servers, num_client_procs, server, true,
                 txn_id, run, vars_to_fetch_pattern2.at(0), type_ids_to_read); 
        debug_log << "finished pattern 2" << endl;
        if(rc != RC_OK) {
            add_timing_point(ERR_PATTERN_2);
        }
        add_timing_point(READ_PATTERN_2_TYPES_DONE);        
    }

    MPI_Barrier(MPI_COMM_WORLD);
    if(vars_to_fetch_pattern3.size() != 3) {
        error_log << "Error. For pattern 3, expecting vars_to_fetch.size() to equal 3 but instead it equals " << vars_to_fetch_pattern3.size() << endl;
    }
    else {
        add_timing_point(READ_PATTERN_3_TYPES_START);
        rc = read_pattern_3 (rank, proc_dims, num_servers, num_client_procs, server, true,
                txn_id, run, vars_to_fetch_pattern3, type_ids_to_read);
        debug_log << "finished pattern 3t" << endl;
        if(rc != RC_OK) {
            add_timing_point(ERR_PATTERN_3); 
        }   
        add_timing_point(READ_PATTERN_3_TYPES_DONE);
    }

    // if(vars_to_fetch_pattern4.size() != 1) {
    //     error_log << "Error. For pattern 4, expecting vars_to_fetch.size() to equal 1 but instead it equals " << vars_to_fetch_pattern4.size() << endl;
    // }
    // else {
    //     rc = read_pattern_4 (rank, num_servers, servers, true, plane_x_procs, plane_y_procs,
    //                 total_x_length, total_y_length, total_z_length, txn_id, vars_to_fetch_pattern4.at(0));
    //     debug_log << "finished pattern 4t" << endl;
    //     if(rc != RC_OK) {
    //         add_timing_point(ERR_PATTERN_4);
    //     }
    // }

    // if(vars_to_fetch_pattern5.size() != 1) {
    //     error_log << "Error. For pattern 5, expecting vars_to_fetch.size() to equal 1 but instead it equals " << vars_to_fetch_pattern5.size() << endl;
    // }
    // else {
    //     rc = read_pattern_5( rank, num_servers, servers, true, num_x_procs, num_y_procs, num_z_procs,
    //                     total_x_length, total_y_length, total_z_length, txn_id, vars_to_fetch_pattern5.at(0));
    //     debug_log << "finished pattern 5t" << endl;
    //     if(rc != RC_OK) {
    //         add_timing_point(ERR_PATTERN_5);  
    //     }
    // }

    // if(vars_to_fetch_pattern6.size() != 1) {
    //     error_log << "Error. For pattern 6, expecting vars_to_fetch.size() to equal 1 but instead it equals " << vars_to_fetch_pattern6.size() << endl;
    // }
    // else {
    //     rc = read_pattern_6 (rank, num_servers, servers, true, plane_x_procs, plane_y_procs,
    //                     total_x_length, total_y_length, total_z_length, txn_id, vars_to_fetch_pattern6.at(0));
    //     debug_log << "finished pattern 6t" << endl;
    //     if(rc != RC_OK) {
    //         add_timing_point(ERR_PATTERN_6);
    //     }
    // }
    // gettimeofday(&pattern_read_end, NULL);
    // clock_times.push_back( (pattern_read_end.tv_sec - zero_time_sec) + pattern_read_end.tv_usec / 1000000.0 );
    add_timing_point(READING_TYPE_PATTERNS_DONE);

    // num_essential_params = all_objector_params.size();

    MPI_Barrier(MPI_COMM_WORLD);
    add_timing_point(EXTRA_TESTING_START);

    rc = extra_testing_collective( server,rank, num_servers, num_client_procs, run, proc_dims); 
    if (rc != RC_OK) {
        error_log << "Error with extra_testing \n";
    }

    add_timing_point(EXTRA_TESTING_DONE);
    add_timing_point(READING_DONE); 


    extreme_debug_log << "starting output \n";

//-------------------------------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------------------------------        
cleanup:
    MPI_Barrier (MPI_COMM_WORLD);
    add_timing_point(READING_DONE_FOR_ALL_PROCS_START_CLEANUP);

    metadata_finalize_client();

    if( (time_pts.size() == 0 || time_pts.at(time_pts.size() - 1) != ERR_DIRMAN) && rank < num_servers) {
        metadata_finalize_server(server);
        add_timing_point(SERVER_SHUTDOWN_DONE);
        debug_log << "just finished finalizing" << endl;
        if(rank == 0) {
            metadata_finalize_server(dirman);
            add_timing_point(DIRMAN_SHUTDOWN_DONE);
        }
    }
    extreme_debug_log << "got here 1 \n";


    if (output_objector_params) {
        gather_and_print_output_params(all_objector_params, rank, num_client_procs);
    }
    else if (output_obj_names) {
        gather_and_print_object_names(all_object_names, all_offsets_and_counts, all_read_types, rank, num_client_procs, num_timesteps, 10);
    }
    extreme_debug_log << "got here 2 \n";

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    if(output_timing) {

        double accum = std::accumulate(all_num_params_to_fetch.begin(), all_num_params_to_fetch.end(), 0.0);
        if( all_objector_params.size() != accum ) {
            error_log << "error. all_objector_params.size(): " << all_objector_params.size() << " sum(all_num_params_to_fetch): " <<
                accum << endl;
        }
        else {
            debug_log << "all_objector_params.size() == accum == " << accum << endl;
        }

        // double accum2 = std::accumulate(all_num_objects_to_fetch.begin(), all_num_objects_to_fetch.end(), 0.0);
        // if( all_object_names.size() != accum2 ) {
        //     error_log << "error. all_num_objects_to_fetch.size(): " << all_object_names.size() << " sum(all_object_names): " <<
        //         accum2 << endl;
        // }
        // else {
        //     debug_log << "all_num_objects_to_fetch.size() == accum2 == " << accum2 << endl;
        // }

        int each_proc_num_params[num_client_procs];
        int param_displacement_for_each_proc[num_client_procs];
        int *num_objector_params_for_all_procs;

        gatherv_int(all_num_params_to_fetch, num_client_procs, rank, each_proc_num_params,
            param_displacement_for_each_proc, &num_objector_params_for_all_procs);

        int each_proc_num_objector_outputs[num_client_procs];
        int object_name_displacement_for_each_proc[num_client_procs];
        int *num_objects_to_fetch_for_all_procs;

        gatherv_int(all_num_objects_to_fetch, num_client_procs, rank, each_proc_num_objector_outputs,
            object_name_displacement_for_each_proc, &num_objects_to_fetch_for_all_procs);

        int each_proc_num_time_pts[num_client_procs];
        int displacement_for_each_proc[num_client_procs];
        int *all_catg_time_pts_buf;

        gatherv_int(catg_of_time_pts, num_client_procs, rank, each_proc_num_time_pts,
            displacement_for_each_proc, &all_catg_time_pts_buf);

        int sum = 0;
        if(rank == 0) {
            // for(int i = 0; i < num_client_procs; i++) {
            //     sum += each_proc_num_time_pts[i];
            // }
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

            // for(int i = 0; i<(num_wall_time_pts * num_client_procs); i++) {
            //     if(i%num_wall_time_pts == 0) {
            //         std::cout << (int)CLOCK_TIMES_NEW_PROC << " ";
            //     }
            //     if(all_clock_times[i] != 0) {
            //         printf("%.6Lf ", all_clock_times[i]);
            //     }
            //     if (i%20 == 0 && i!=0) {
            //         std::cout << std::endl;
            //     } 
            // }
            // std::cout << (int)CLOCK_TIMES_DONE << " ";
            int obj_count = 0;
            int objector_output_count = 0;
            for(int i=0; i<sum; i++) {
                if (output_obj_names  && all_catg_time_pts_buf[i] == BOUNDING_BOX_TO_OBJ_NAMES) {
                    printf("%d %d ", all_catg_time_pts_buf[i], num_objects_to_fetch_for_all_procs[objector_output_count]);
                    objector_output_count += 1;
                }
                else if (output_objector_params && all_catg_time_pts_buf[i] == BOUNDING_BOX_TO_OBJ_NAMES) {
                    printf("%d %d ", all_catg_time_pts_buf[i], num_objector_params_for_all_procs[obj_count]);
                    obj_count += 1;
                }
                else {
                    // cout << "all_time_pts_buf[i]: " << all_time_pts_buf[i] << endl;
                    printf("%d %Lf ", all_catg_time_pts_buf[i], all_time_pts_buf[i]);
                }
                // std::cout << all_catg_time_pts_buf[i] << " " << all_time_pts_buf[i] << " ";
                if (i%20 == 0 && i!=0) {
                    std::cout << std::endl;
                }
            }
            std::cout << std::endl;

            // // free(all_clock_times);
            free(all_time_pts_buf);
            free(all_catg_time_pts_buf);

            free(num_objector_params_for_all_procs);
            free(num_objects_to_fetch_for_all_procs);

        }
    }

    
    // free(total_times)
    MPI_Barrier (MPI_COMM_WORLD);
    MPI_Finalize();
    gutties::bootstrap::Finish();
    debug_log << "got to cleanup7" << endl;

    return rc;

}

static int setup_dirman(const string &dirman_file_path, const string &dir_path, md_server &dirman, 
                        vector<gutties::name_and_node_t> &server_procs, int rank, uint32_t num_servers, uint32_t num_clients) {

    bool ok;
    int rc;
    char *serialized_c_str;
    int length_ser_c_str;
    // gutties::name_and_node_t *server_ary;

    if(rank == 0) { 
        struct stat buffer;
        bool dirman_initted = false;
        std::ifstream file;
        string dirman_hexid;
        DirectoryInfo dir;

        while (!dirman_initted) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            dirman_initted = (stat (dirman_file_path.c_str(), &buffer) == 0);
            debug_log << "dirman not initted yet \n";
        }
        file.open(dirman_file_path);
        if(!file) {
            return RC_ERR;
        }
        while( file.peek() == std::ifstream::traits_type::eof() ) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            debug_log << "dirman file empty \n";
        }
        file >> dirman_hexid;
        debug_log << "just got the hexid: " << dirman_hexid << endl;


        gutties::Configuration config(default_config_string);
        config.Append("dirman.root_node", dirman_hexid);
        config.Append("webhook.port", to_string(4000+num_servers+rank)); //note if you up number of servers you'll want to up this
        config.AppendFromReferences();
        debug_log << "just configged" << endl; 
        gutties::bootstrap::Start(config, opbox::bootstrap);

        //Add the directory manager's URL to the config string so the clients know
        //Where to access the directory 
        //-------------------------------------------
        //TODO: This connect is temporarily necessary
        gutties::nodeid_t dirman_nodeid(dirman_hexid);
        extreme_debug_log << "Dirman node ID " << dirman_nodeid.GetHex() << " " << dirman_nodeid.GetIP() << " port " << dirman_nodeid.GetPort() << endl;
        dirman.name_and_node.node = dirman_nodeid;
        extreme_debug_log << "about to connect peer to node" << endl;
        rc = net::Connect(&dirman.peer_ptr, dirman.name_and_node.node);
        assert(rc==RC_OK && "could not connect");
        extreme_debug_log << "just connected" << endl;
        //-------------------------------------------
        extreme_debug_log << "app name is " << dir_path << endl;
        ok = dirman::GetRemoteDirectoryInfo(gutties::ResourceURL(dir_path), &dir);
        assert(ok && "Could not get info about the directory?");
        extreme_debug_log << "just got directory info" << endl;

        while( dir.children.size() < num_servers) {
            debug_log << "dir.children.size() < num_servers. dir.children.size(): "  << dir.children.size() << " num_servers: " << num_servers << endl;

            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            ok = dirman::GetRemoteDirectoryInfo(gutties::ResourceURL(dir_path), &dir);
            extreme_debug_log << "and now dir.children.size() < num_servers. dir.children.size(): "  << dir.children.size() << " num_servers: " << num_servers << endl;

            assert(ok && "Could not get info about the directory?");
            extreme_debug_log << "just got directory info" << endl;
        }
        debug_log << "dir init is done" << endl;
        server_procs = dir.children;
        extreme_debug_log << "about to serialize server_procs \n";
        stringstream ss;
        boost::archive::text_oarchive oa(ss);
        if(server_procs.size() > 0) { 
        //leaving off here. looks kike sometimes the string isn't getting copied properly? (could have embedded nulls)
            oa << server_procs;
            string serialized_str = ss.str();
            length_ser_c_str = serialized_str.size() + 1;
            serialized_c_str = (char *) malloc(length_ser_c_str);
            serialized_str.copy(serialized_c_str, serialized_str.size());
        }
        else {
            error_log << "error. thinks the correct number of servers is 0 \n";
            return RC_ERR;
        }
    }
    MPI_Bcast(&length_ser_c_str, 1, MPI_INT, 0, MPI_COMM_WORLD);

    if(rank != 0) {
        debug_log << "num_bytes is "<< to_string(length_ser_c_str) + " and rank: " << to_string(rank) << endl;
        serialized_c_str = (char *) malloc(length_ser_c_str);  
    }

    MPI_Bcast(serialized_c_str, length_ser_c_str, MPI_CHAR, 0, MPI_COMM_WORLD);

    if (rank != 0) {
        // extreme_debug_log << "serialized_c_str: " << (string)serialized_c_str << endl;
        stringstream ss1;
        ss1.write(serialized_c_str, length_ser_c_str);
        boost::archive::text_iarchive ia(ss1);
        ia >> server_procs;

        //todo - should decide if we want to have a dedicated dirman, if 
        //not, just need to adjust so "dirman" is server rank 0 
        gutties::Configuration config(default_config_string);
        config.Append("dirman.root_node", server_procs[0].node.GetHex());
        config.Append("webhook.port", to_string(4000+num_servers+rank)); //note if you up number of servers you'll want to up this
        config.AppendFromReferences();
        debug_log << "just configged" << endl; 
        gutties::bootstrap::Start(config, opbox::bootstrap);

    } 
    if (length_ser_c_str > 0) {
        free(serialized_c_str);
    }


    for (int i=0; i<num_servers; i++) {
        extreme_debug_log << "server " << i << " in ary has hexid " << server_procs[i].node.GetHex() << endl;
    }



    // server_procs = temp(server_ary, server_ary +(int)num_servers);
    extreme_debug_log << "done initing dirman" << endl;
    return RC_OK;
 } 

static void setup_server(int rank, uint32_t num_servers, 
        const vector<gutties::name_and_node_t> &server_nodes, md_server &server) {

    int server_indx = rank % num_servers; 
    server.name_and_node = server_nodes[server_indx];
    net::Connect(&server.peer_ptr, server.name_and_node.node);
    server.URL = server.name_and_node.node.GetHex();
    extreme_debug_log << "server.URL: " << server.URL << endl;
    extreme_debug_log << "server_indx: " << server_indx << endl;
}

// static void setup_servers(std::vector<md_server> &servers, uint32_t num_servers, const vector<gutties::name_and_node_t> &server_nodes) {
//     debug_log << "num_servers: " << to_string(num_servers) << endl;

//     for(int i=0; i < num_servers; i++) {
//         md_server server;
//         server.name_and_node = server_nodes[i];
//         net::Connect(&server.peer_ptr, server.name_and_node.node);
//         server.URL = server.name_and_node.node.GetHex();
//         extreme_debug_log << "server.URL: " << server.URL << endl;
//         servers.push_back(server);
//     }

//     debug_log << "servers.size: " << to_string(servers.size()) << endl;
    
// }

void gather_and_print_output_params(const vector<objector_params> &all_objector_params, int rank, uint32_t num_client_procs) {

    extreme_debug_log.set_rank(rank);


    int length_ser_c_str = 0;
    char *serialized_c_str;
    int each_proc_ser_objector_params_size[num_client_procs];
    int each_proc_num_essential_params[num_client_procs];

    int displacement_for_each_proc[num_client_procs];

    char *serialized_c_str_all_objector_params;     

    stringstream ss;
    boost::archive::text_oarchive oa(ss);
    // extreme_debug_log << "rank " << rank << " about to try serializing at the beginning" << endl;
    oa << all_objector_params;
    // extreme_debug_log << "rank " << rank << " just finished serializing at the beginning" << endl;
    string serialized_str = ss.str();
    length_ser_c_str = serialized_str.size() + 1;
    serialized_c_str = (char *) malloc(length_ser_c_str);
    serialized_str.copy(serialized_c_str, serialized_str.size());
    serialized_c_str[serialized_str.size()]='\0';
    // extreme_debug_log << "rank " << rank << " attrs ser string is of size " << length_ser_c_str << " serialized_str " << 
    //  serialized_str << " serialized_c_str: " << serialized_c_str << endl;
    // extreme_debug_log << "rank " << rank << " params ser string is of size " << length_ser_c_str << " serialized_str " << 
    //     serialized_str << endl;
    extreme_debug_log << "rank " << rank << " params ser string is of size " << length_ser_c_str << endl;
    // extreme_debug_log << "rank " << rank << " about to allgather" << endl;

    MPI_Gather(&length_ser_c_str, 1, MPI_INT, each_proc_ser_objector_params_size, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // extreme_debug_log << "rank " << rank << " about with allgather" << endl;

    int sum = 0;
    
    if (rank == 0 ) {
        for(int i=0; i<num_client_procs; i++) {
            displacement_for_each_proc[i] = sum;
            sum += each_proc_ser_objector_params_size[i];
            if(each_proc_ser_objector_params_size[i] != 0 && rank == 0) {
                extreme_debug_log << "rank " << i << " has a string of length " << each_proc_ser_objector_params_size[i] << endl;
            }
        }
        extreme_debug_log << "sum: " << sum << endl;

       serialized_c_str_all_objector_params = (char *) malloc(sum);
    }

    // extreme_debug_log << "num_bytes is "<< to_string(length_ser_c_str) + " and rank: " << to_string(rank) << endl;
    
    // extreme_debug_log << "rank " << rank << " about to allgatherv" << endl;

    MPI_Gatherv(serialized_c_str, length_ser_c_str, MPI_CHAR,
           serialized_c_str_all_objector_params, each_proc_ser_objector_params_size, displacement_for_each_proc,
           MPI_CHAR, 0, MPI_COMM_WORLD);

    // extreme_debug_log << "rank " << rank << " done with allgatherv" << endl;
    // extreme_debug_log << "entire set of attr vectors: " << serialized_c_str_all_objector_params << " and is of length: " << strlen(serialized_c_str_all_objector_params) << endl;
    if(rank == 0) {
       cout <<"rank, get_counts, job_id, sim_name, timestep, ndx, ndy, ndz, " <<
                "ceph_obj_size, var_name, var_version, data_size, x1, " <<
                "y1, z1, x2, y2, z2, " <<
                "var_x1, var_x2, var_y1, var_y2, var_z1, var_z2 \n \n";

        uint16_t num_objector_read_types = 56;
        vector<vector<objector_params>> all_rec_params(num_objector_read_types);

        for(int i = 0; i < num_client_procs; i++) {
            int offset = displacement_for_each_proc[i];
            int count = each_proc_ser_objector_params_size[i];
            if(count > 0) {
                vector<objector_params> rec_params;

                if(i != rank) {

                    extreme_debug_log << "rank " << rank << " count: " << count << " offset: " << offset << endl;
                    char serialzed_objector_params_for_one_proc[count];

                    memcpy ( serialzed_objector_params_for_one_proc, serialized_c_str_all_objector_params + offset, count);
                    objector_params rec_objector_params;

                    // extreme_debug_log << "rank " << rank << " serialized_c_str: " << (string)serialzed_objector_params_for_one_proc << endl;
                    // extreme_debug_log <<    " serialized_c_str length: " << strlen(serialzed_objector_params_for_one_proc) << 
                    //     " count: " << count << endl;

                    extreme_debug_log << "rank " << rank << " serialized_c_str length: " << strlen(serialzed_objector_params_for_one_proc) << 
                        " count: " << count << endl;
                    stringstream ss1;
                    ss1.write(serialzed_objector_params_for_one_proc, count);
                    extreme_debug_log << "about to deserialize ss: " << ss1.str() << endl;
                    boost::archive::text_iarchive ia(ss1);
                    ia >> rec_params;
                }
                else { //i == rank
                    rec_params = all_objector_params;
                }

                for (objector_params param : rec_params) {
                    param.rank = i;
                    all_rec_params.at(param.read_type).push_back(param);
                }

            }
        }
        cout << "beggining essential read types" << endl;

        int len =  to_string(max( all_rec_params.at(0).at(0).var_dims[0].max, max(all_rec_params.at(0).at(0).var_dims[1].max, 
                            all_rec_params.at(0).at(0).var_dims[2].max) )).length();
        int len2 =  to_string(num_client_procs).length();

        vector<string> read_type_names = {
            "PATTERN_1",
            "PATTERN_2",
            "PATTERN_3",
            "PATTERN_4",
            "PATTERN_5",
            "PATTERN_6",
            "PATTERN_2_ATTRS",
            "PATTERN_3_ATTRS",
            "CATALOG_ALL_VAR_ATTRIBUTES",
            "CATALOG_ALL_VAR_ATTRIBUTES_WITH_DIMS",
            "CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_BY_ID",
            "CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_BY_NAME_VER",
            "CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_DIMS_BY_ID",
            "CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_DIMS_BY_NAME_VER",
            "CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_BY_ID",
            "CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_BY_NAME_VER",
            "CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_RANGE",
            "CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_ABOVE_MAX",
            "CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_BELOW_MIN",
            "CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_DIMS_BY_ID",
            "CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_DIMS_BY_NAME_VER",
            "CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_DIMS_RANGE",
            "CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_DIMS_ABOVE_MAX",
            "CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_DIMS_BELOW_MIN",
            "CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR",
            "CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_RANGE",
            "CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_ABOVE_MAX",
            "CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_BELOW_MIN",            
            "CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_DIMS",
            "CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_DIMS_RANGE",
            "CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_DIMS_ABOVE_MAX",
            "CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_DIMS_BELOW_MIN",
            "CATALOG_ALL_VAR_ATTRIBUTES_WITH_VAR_BY_ID",
            "CATALOG_ALL_VAR_ATTRIBUTES_WITH_VAR_BY_NAME_VER",
            "CATALOG_ALL_VAR_ATTRIBUTES_WITH_VAR_DIMS_BY_ID",
            "CATALOG_ALL_VAR_ATTRIBUTES_WITH_VAR_DIMS_BY_NAME_VER",
            "CATALOG_ALL_VAR_ATTRIBUTES_WITH_VAR_SUBSTR",
            "CATALOG_ALL_VAR_ATTRIBUTES_WITH_VAR_SUBSTR_DIMS"
        };

        for ( uint16_t i = 0; i < num_objector_read_types; i++ ) {
            vector<objector_params> params = all_rec_params.at(i);
           if(i  == CATALOG_ALL_VAR_ATTRIBUTES) {
                cout << endl << "beginning non-essential params " << endl;
            }
            if(params.size() > 0) {
                cout << "read type: " << read_type_names.at(i) << ": " << endl;
            }
            for (objector_params param : params) {
                printf("%*d, true, %8llu, %s, %llu, %llu, %llu, %llu, "
                    "8000000, %10s, %lu, 8, %*llu, %*llu, %*llu, %*llu, %*llu, %*llu, " 
                    "%*llu, %*llu, %*llu, %*llu, %*llu, %*llu \n", 
                    len2, param.rank, param.job_id, param.run_name.c_str(), param.timestep_id, 
                    param.ndx, param.ndy, param.ndz, param.var_name.c_str(), param.var_version, len,
                    param.bounding_box[0].min, len, param.bounding_box[1].min, len, param.bounding_box[2].min, 
                    len, param.bounding_box[0].max, len, param.bounding_box[1].max, len, param.bounding_box[2].max, 
                    len, param.var_dims[0].min, len, param.var_dims[0].max, len, param.var_dims[1].min, 
                    len, param.var_dims[1].max, len, param.var_dims[2].min, len, param.var_dims[2].max);
            }
            if(params.size() > 0) {
                cout << endl;
            }
        }     
        free(serialized_c_str_all_objector_params);
        // cout << "begin timing output" << endl;
    }
    if(length_ser_c_str > 0) {
        free(serialized_c_str);
    }
}

objector_params get_objector_params ( const md_catalog_run_entry &run, const md_catalog_var_entry &var, 
                            const vector<md_dim_bounds> &bounding_box, OBJECTOR_PARAMS_READ_TYPES read_type,
                            const vector<md_dim_bounds> &proc_dims) {

    objector_params params;

    params.run_id = run.run_id;
    params.job_id = run.job_id;
    params.run_name = run.name;
    params.timestep_id = var.timestep_id;
    params.ndx = ( var.dims[0].max - var.dims[0].min + 1 ) / run.npx;
    params.ndy = ( var.dims[1].max - var.dims[1].min + 1) / run.npy;
    params.ndz = ( var.dims[2].max - var.dims[2].min + 1 ) / run.npz;
    params.var_id = var.var_id;
    params.var_name = var.name;
    params.var_version = var.version;      
    params.var_dims = var.dims;
    params.bounding_box = bounding_box;
    params.read_type = read_type;

    // extreme_debug_log << "initially bounding box requested was: ";
    // for(int j=0; j< bounding_box.size(); j++) {
    //     extreme_debug_log << " d" << j << "_min: " << bounding_box [j].min;
    //     extreme_debug_log << " d" << j << "_max: " << bounding_box [j].max;               
    // }
    // extreme_debug_log << endl;

    // extreme_debug_log << "proc dims: ";
    // for(int j=0; j< proc_dims.size(); j++) {
    //     extreme_debug_log << " d" << j << "_min: " << proc_dims [j].min;
    //     extreme_debug_log << " d" << j << "_max: " << proc_dims [j].max;               
    // }
    // extreme_debug_log << endl;

    for(int i = 0; i < bounding_box.size(); i++) {
        params.bounding_box[i].min = max( bounding_box[i].min, proc_dims[i].min );
        params.bounding_box[i].max = min( bounding_box[i].max, proc_dims[i].max );
    }
    // if (params.bounding_box[0].min != bounding_box[0].min || 
    //     params.bounding_box[0].max != bounding_box[0].max ||
    //     params.bounding_box[1].min != bounding_box[1].min || 
    //     params.bounding_box[1].max != bounding_box[1].max ||     
    //     params.bounding_box[2].min != bounding_box[2].min || 
    //     params.bounding_box[2].max != bounding_box[2].max ) {

    //     extreme_debug_log << "after adjustment bounding box requested was: ";
    //     for(int j=0; j< params.bounding_box.size(); j++) {
    //         extreme_debug_log << " d" << j << "_min: " << params.bounding_box [j].min;
    //         extreme_debug_log << " d" << j << "_max: " << params.bounding_box [j].max;               
    //     }
    //     extreme_debug_log << endl;
    // }
    // params.get_counts = true;
    // params.ceph_obj_size = 8000000



    return params;
}


vector<md_dim_bounds> get_overlapping_dims_for_bounding_box ( const vector<md_dim_bounds> &bounding_box, 
                            const vector<md_dim_bounds> &proc_dims
                            ) 
{
    // extreme_debug_log << "initially bounding box requested was: ";
    // for(int j=0; j< bounding_box.size(); j++) {
    //     extreme_debug_log << " d" << j << "_min: " << bounding_box [j].min;
    //     extreme_debug_log << " d" << j << "_max: " << bounding_box [j].max;               
    // }
    // extreme_debug_log << endl;

    // extreme_debug_log << "proc dims: ";
    // for(int j=0; j< proc_dims.size(); j++) {
    //     extreme_debug_log << " d" << j << "_min: " << proc_dims [j].min;
    //     extreme_debug_log << " d" << j << "_max: " << proc_dims [j].max;               
    // }
    // extreme_debug_log << endl;

    vector<md_dim_bounds> overlapping_bb(bounding_box.size());

    for(int i = 0; i < bounding_box.size(); i++) {      
        overlapping_bb[i].min = max( bounding_box[i].min, proc_dims[i].min );
        overlapping_bb[i].max = min( bounding_box[i].max, proc_dims[i].max );
        // extreme_debug_log << "bb[" << i << "].min: " << bounding_box[i].min << " proc[" << i << "].min: " << proc_dims[i].min << endl;
        // extreme_debug_log << "overlapping_bb.min: " << overlapping_bb[i].min << endl;
        // extreme_debug_log << "bb[" << i << "].max: " << bounding_box[i].max << " proc[" << i << "].max: " << proc_dims[i].max << endl;
        // extreme_debug_log << "overlapping_bb.max: " << overlapping_bb[i].max << endl;          
    }
    // if (params.bounding_box[0].min != bounding_box[0].min || 
    //     params.bounding_box[0].max != bounding_box[0].max ||
    //     params.bounding_box[1].min != bounding_box[1].min || 
    //     params.bounding_box[1].max != bounding_box[1].max ||     
    //     params.bounding_box[2].min != bounding_box[2].min || 
    //     params.bounding_box[2].max != bounding_box[2].max ) {

    //     extreme_debug_log << "after adjustment bounding box requested was: ";
    //     for(int j=0; j< params.bounding_box.size(); j++) {
    //         extreme_debug_log << " d" << j << "_min: " << params.bounding_box [j].min;
    //         extreme_debug_log << " d" << j << "_max: " << params.bounding_box [j].max;               
    //     }
    //     extreme_debug_log << endl;
    // }
    return overlapping_bb;
}

objector_params get_objector_params ( const md_catalog_run_entry &run, const md_catalog_var_entry &var, 
                            const vector<md_dim_bounds> &bounding_box, OBJECTOR_PARAMS_READ_TYPES read_type) {

    objector_params params;

    params.run_id = run.run_id;
    params.job_id = run.job_id;
    params.run_name = run.name;
    params.timestep_id = var.timestep_id;
    params.ndx = ( var.dims[0].max - var.dims[0].min + 1 ) / run.npx;
    params.ndy = ( var.dims[1].max - var.dims[1].min + 1) / run.npy;
    params.ndz = ( var.dims[2].max - var.dims[2].min + 1 ) / run.npz;
    params.var_id = var.var_id;
    params.var_name = var.name;
    params.var_version = var.version;      
    params.var_dims = var.dims;
    params.bounding_box = bounding_box;
    params.read_type = read_type;
    // params.get_counts = true;
    // params.ceph_obj_size = 8000000

    return params;
}

void gatherv_int(vector<int> values, uint32_t num_client_procs, int rank, int *each_proc_num_values,
            int *displacement_for_each_proc, int **all_values)
{   
    int num_values = values.size();
    // int *values_ary = &values[0];

    // if(rank == 0) {
    //     for (int i = 0; i < 31; i++) {
    //         extreme_debug_log << "i: " << i << " values[i]: " << values[i] << endl;
    //     }  
    // }

    MPI_Gather(&num_values, 1, MPI_INT, each_proc_num_values, 1, MPI_INT, 0, MPI_COMM_WORLD);

    int sum = 0;
    if (rank == 0 ) {
        for(int i=0; i<num_client_procs; i++) {
            displacement_for_each_proc[i] = sum;
            sum += each_proc_num_values[i];
            if(each_proc_num_values[i] != 0 && rank == 0) {
                extreme_debug_log << "rank " << i << " has a string of length " << each_proc_num_values[i] << endl;
            }
        }
        *all_values = (int *) malloc(sum * sizeof(int));

        extreme_debug_log << "sum: " << sum << endl;
    }

    // extreme_debug_log << "num_bytes is "<< to_string(length_ser_c_str) + " and rank: " << to_string(rank) << endl;
    
    // extreme_debug_log << "rank " << rank << " about to allgatherv" << endl;

    MPI_Gatherv(&values[0], num_values, MPI_INT,
           *all_values, each_proc_num_values, displacement_for_each_proc,
           MPI_INT, 0, MPI_COMM_WORLD);

    // if(rank == 0) {
    //     for (int i = 0; i < 62; i++) {
    //         extreme_debug_log << "i: " << i << " all_values[i]: " << *all_values[i] << endl;
    //     }  
    // } 

}

template <class T>
void gatherv_ser(T values, uint32_t num_client_procs, int rank, int *each_proc_ser_values_size, int *displacement_for_each_proc, 
        char **serialized_c_str_all_ser_values)
{
    int length_ser_c_str = 0;
    char *serialized_c_str;

    stringstream ss;
    boost::archive::text_oarchive oa(ss);
    oa << values;
    string serialized_str = ss.str();
    length_ser_c_str = serialized_str.size() + 1;
    serialized_c_str = (char *) malloc(length_ser_c_str);
    serialized_str.copy(serialized_c_str, serialized_str.size());
    serialized_c_str[serialized_str.size()]='\0';
    // extreme_debug_log << "rank " << rank << " object_names ser string is of size " << length_ser_c_str << " serialized_str " << 
    //     serialized_str << endl;
    extreme_debug_log << "rank " << rank << " object_names ser string is of size " << length_ser_c_str << endl;   
    // extreme_debug_log << "rank " << rank << " about to allgather" << endl;

    MPI_Gather(&length_ser_c_str, 1, MPI_INT, each_proc_ser_values_size, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // extreme_debug_log << "rank " << rank << " about with allgather" << endl;

    int sum = 0;
    
    if (rank == 0 ) {
        for(int i=0; i<num_client_procs; i++) {
            displacement_for_each_proc[i] = sum;
            sum += each_proc_ser_values_size[i];
            if(each_proc_ser_values_size[i] != 0 && rank == 0) {
                extreme_debug_log << "rank " << i << " has a string of length " << each_proc_ser_values_size[i] << endl;
            }
        }
        extreme_debug_log << "sum: " << sum << endl;

       *serialized_c_str_all_ser_values = (char *) malloc(sum);
    }

    // extreme_debug_log << "num_bytes is "<< to_string(length_ser_c_str) + " and rank: " << to_string(rank) << endl;
    
    // extreme_debug_log << "rank " << rank << " about to allgatherv" << endl;

    MPI_Gatherv(serialized_c_str, length_ser_c_str, MPI_CHAR,
           *serialized_c_str_all_ser_values, each_proc_ser_values_size, displacement_for_each_proc,
           MPI_CHAR, 0, MPI_COMM_WORLD);

    free(serialized_c_str);
}

void gather_and_print_object_names(const vector<vector<string>> &all_object_names, const vector<vector<uint64_t>> &all_offsets_and_counts,
                            const vector<int> &all_read_types,
                            int rank, uint32_t num_client_procs, 
                            uint32_t num_timesteps, uint32_t num_vars)
{

    // extreme_debug_log << "rank: " << rank << endl;

    extreme_debug_log.set_rank(rank);

    extreme_debug_log << "all_read_types.size(): " << all_read_types.size() << endl;
    extreme_debug_log << "all_object_names.size(): " << all_object_names.size() << endl;
    extreme_debug_log << "all_offsets_and_counts.size(): " << all_offsets_and_counts.size() << endl;

    vector<string> read_type_names = {
        "PATTERN_1",
        "PATTERN_2",
        "PATTERN_3",
        "PATTERN_4",
        "PATTERN_5",
        "PATTERN_6",
        "PATTERN_2_ATTRS",
        "PATTERN_3_ATTRS",
        "CATALOG_ALL_VAR_ATTRIBUTES",
        "CATALOG_ALL_VAR_ATTRIBUTES_WITH_DIMS",
        "CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_BY_ID",
        "CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_BY_NAME_VER",
        "CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_DIMS_BY_ID",
        "CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_DIMS_BY_NAME_VER",
        "CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_BY_ID",
        "CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_BY_NAME_VER",
        "CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_RANGE",
        "CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_ABOVE_MAX",
        "CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_BELOW_MIN",
        "CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_DIMS_BY_ID",
        "CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_DIMS_BY_NAME_VER",
        "CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_DIMS_RANGE",
        "CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_DIMS_ABOVE_MAX",
        "CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_DIMS_BELOW_MIN",
        "CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR",
        "CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_RANGE",
        "CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_ABOVE_MAX",
        "CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_BELOW_MIN",            
        "CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_DIMS",
        "CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_DIMS_RANGE",
        "CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_DIMS_ABOVE_MAX",
        "CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_DIMS_BELOW_MIN",
        "CATALOG_ALL_VAR_ATTRIBUTES_WITH_VAR_BY_ID",
        "CATALOG_ALL_VAR_ATTRIBUTES_WITH_VAR_BY_NAME_VER",
        "CATALOG_ALL_VAR_ATTRIBUTES_WITH_VAR_DIMS_BY_ID",
        "CATALOG_ALL_VAR_ATTRIBUTES_WITH_VAR_DIMS_BY_NAME_VER",
        "CATALOG_ALL_VAR_ATTRIBUTES_WITH_VAR_SUBSTR",
        "CATALOG_ALL_VAR_ATTRIBUTES_WITH_VAR_SUBSTR_DIMS"
    };


    int each_proc_ser_object_names_size[num_client_procs];
    int displacement_for_each_proc[num_client_procs];
    char *serialized_c_str_all_object_names;

    gatherv_ser(all_object_names, num_client_procs, rank, each_proc_ser_object_names_size, displacement_for_each_proc, 
        &serialized_c_str_all_object_names);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


    int each_proc_ser_offsets_and_counts_size[num_client_procs];
    int offset_displacement_for_each_proc[num_client_procs];
    char *serialized_c_str_all_offsets_and_counts;     

    gatherv_ser(all_offsets_and_counts, num_client_procs, rank, each_proc_ser_offsets_and_counts_size, offset_displacement_for_each_proc, 
        &serialized_c_str_all_offsets_and_counts);
    

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


    int each_proc_num_read_types[num_client_procs];
    int read_type_displacement_for_each_proc[num_client_procs];
    int *read_types_for_all_procs;

    gatherv_int(all_read_types, num_client_procs, rank, each_proc_num_read_types, read_type_displacement_for_each_proc,
        &read_types_for_all_procs);

    // if (rank == 0) {
    //     for (int i = 0; i < 328; i++) {
    //         extreme_debug_log << "i: " << i << " read_types_for_all_procs[i]: " << read_types_for_all_procs[i] << endl;
    //     }
    // }

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // extreme_debug_log << "rank " << rank << " done with allgatherv" << endl;
    // extreme_debug_log << "entire set of attr vectors: " << serialized_c_str_all_object_names << " and is of length: " << strlen(serialized_c_str_all_object_names) << endl;
    if(rank == 0) {
       uint16_t num_vars_per_run = num_vars * num_timesteps;

       vector<vector<vector<string>>> all_rec_obj_names;
       vector<vector<vector<uint64_t>>> all_rec_offsets_and_counts;
       uint16_t num_objector_read_types = 56;
        for (int read_type = 0; read_type < num_objector_read_types; read_type++) {
            all_rec_obj_names.push_back(vector<vector<string>>(num_client_procs));
            all_rec_offsets_and_counts.push_back(vector<vector<uint64_t>>(num_client_procs));
        }

        for(int proc_rank = 0; proc_rank < num_client_procs; proc_rank++) {
            int offset = displacement_for_each_proc[proc_rank];
            int count = each_proc_ser_object_names_size[proc_rank];

            int offset_2 = offset_displacement_for_each_proc[proc_rank];
            int count_2 = each_proc_ser_offsets_and_counts_size[proc_rank];

            int offset_3 = read_type_displacement_for_each_proc[proc_rank];
            int count_3 = each_proc_num_read_types[proc_rank];

            extreme_debug_log << "offset: " << offset << " count: " << count << " offset_2: " << offset_2 <<
                " count_2: " << count_2 << " offset_3: " << offset_3 << " count_3: " << count_3 << endl;

            if(count > 0) {
                vector<vector<string>> rec_obj_names;
                vector<vector<uint64_t>> rec_offsets_and_counts;
                vector<int> rec_read_types(count_3);
                // vector<int> rec_read_types2(count_3);

                if(proc_rank != rank) {

                    extreme_debug_log << "rank " << proc_rank << " count: " << count << " offset: " << offset << endl;
                    char serialzed_object_names_for_one_proc[count];
                    char serialzed_offsets_for_one_proc[count_2];
                    // int rec_read_types_ary[count_3];

                    extreme_debug_log << "about to memcopy1" << endl;
                    memcpy ( serialzed_object_names_for_one_proc, serialized_c_str_all_object_names + offset, count);
                    extreme_debug_log << "about to memcopy2" << endl;
                    memcpy ( serialzed_offsets_for_one_proc, serialized_c_str_all_offsets_and_counts + offset_2, count_2);
                    extreme_debug_log << "about to memcopy3" << endl;
                    // memcpy ( &rec_read_types[0], read_types_for_all_procs + offset_3, count_3);
                    memcpy ( &rec_read_types[0], read_types_for_all_procs + offset_3, count_3*sizeof(int));
                    // memcpy ( rec_read_types_ary, read_types_for_all_procs + offset_3*sizeof(int), count_3*sizeof(int));
                    // memcpy ( rec_read_types_ary, read_types_for_all_procs + offset_3, count_3*sizeof(int));

                    // temp_attr.dims = std::vector<md_dim_bounds>(proc_dims_vctr, proc_dims_vctr + num_dims ); 
                    // rec_read_types2 = vector<int>(rec_read_types_ary, rec_read_types_ary + count_3);
                    // extreme_debug_log << "rec_read_types2.size(): " << rec_read_types2.size() << endl;

                    // extreme_debug_log << "rank " << rank << " serialized_c_str: " << (string)serialzed_object_names_for_one_proc << endl;
                    // extreme_debug_log <<    " serialized_c_str length: " << strlen(serialzed_object_names_for_one_proc) << 
                    //     " count: " << count << endl;
                    extreme_debug_log << "rank " << rank << " serialized_c_str length: " << strlen(serialzed_object_names_for_one_proc) << 
                        " count: " << count << endl;
                    stringstream ss1;
                    ss1.write(serialzed_object_names_for_one_proc, count);
                    // extreme_debug_log << "about to deserialize ss: " << ss1.str() << endl;
                    boost::archive::text_iarchive ia(ss1);
                    ia >> rec_obj_names;

                    stringstream ss2;
                    ss2.write(serialzed_offsets_for_one_proc, count_2);
                    // extreme_debug_log << "about to deserialize ss2: " << ss2.str() << endl;
                    boost::archive::text_iarchive ia2(ss2);
                    ia2 >> rec_offsets_and_counts;

                }
                else { //i == rank
                    rec_obj_names = all_object_names;
                    rec_offsets_and_counts = all_offsets_and_counts;
                    rec_read_types = all_read_types;
                }
                if(rec_obj_names.size() != rec_offsets_and_counts.size() || rec_obj_names.size() != rec_read_types.size()) {
                    error_log << "error. for rank " << rank << " rec_obj_names.size(): " << rec_obj_names.size() << 
                        " rec_offsets_and_counts.size(): " << rec_offsets_and_counts.size() << " rec_read_types.size(): " << 
                        rec_read_types.size() << endl;
                }
                for(int bb_indx = 0; bb_indx<rec_obj_names.size(); bb_indx++) {
                    vector<string> obj_names = rec_obj_names.at(bb_indx);
                    vector<uint64_t> offsets_and_counts = rec_offsets_and_counts.at(bb_indx);
                    uint16_t read_type = rec_read_types.at(bb_indx);

                    extreme_debug_log << "in gather and print \n";
                    for(int i = 0; i<obj_names.size(); i++) {
                        extreme_debug_log << "obj name: " << obj_names.at(i) << endl;
                        extreme_debug_log << "offset x: " << offsets_and_counts.at(i*6) << " count x: " << offsets_and_counts.at(i*6+1) << endl;
                        extreme_debug_log << "offset y: " << offsets_and_counts.at(i*6+2) << " count y: " << offsets_and_counts.at(i*6+3) << endl;
                        extreme_debug_log << "offset z: " << offsets_and_counts.at(i*6+4) << " count z: " << offsets_and_counts.at(i*6+5) << endl;
                    }
                    // extreme_debug_log << "rec_read_types. at(i): " << rec_read_types.at(bb_indx) << endl;
                    // extreme_debug_log << "rec_read_types2.at(i): " << rec_read_types2.at(i) << endl;
                        // a.insert(a.end(), b.begin(), b.end());

                    // vector<string> obj_names_of_type_for_proc = ;
                    // vector<uint64_t> offests_of_type_for_proc = ;

                    all_rec_obj_names.at(read_type).at(proc_rank).insert(all_rec_obj_names.at(read_type).at(proc_rank).end(), obj_names.begin(), obj_names.end());
                    all_rec_offsets_and_counts.at(read_type).at(proc_rank).insert(all_rec_offsets_and_counts.at(read_type).at(proc_rank).end(), offsets_and_counts.begin(), offsets_and_counts.end());

                    // extreme_debug_log << "obj_names_of_type_for_proc.size() at end: "

                    // all_rec_obj_names.at(rec_read_types.at(j)).push_back(obj_names);
                    // all_rec_offsets_and_counts.at(rec_read_types.at(j)).push_back(offsets_and_counts);
                }
            }
        }

        // int len =  to_string(max( var.dims[0].max, var.dims[1].max, 
        //                     var.dims[2].max) ).length();

        int len2 =  to_string(num_client_procs - 1).length();

        cout <<"rank, object_name, x_offset, x_count, y_offset, y_count, z_offset, z_count \n";

        cout << "beggining essential read types" << endl;

        for (int i = 0; i < all_rec_obj_names.size(); i++) {
            vector<vector<string>> obj_names = all_rec_obj_names.at(i);
            vector<vector<uint64_t>> offsets_and_counts = all_rec_offsets_and_counts.at(i);
    
            if(i  == CATALOG_ALL_VAR_ATTRIBUTES) {
                cout << endl << "beginning non-essential params " << endl;
            }
            int num_matching_objs = 0;
            for(int j = 0; j < obj_names.size(); j++) {
                num_matching_objs += obj_names.at(j).size();
            }

            if(num_matching_objs > 0) {
                // cout << "i: " << i << " obj_names.size(): " << obj_names.size() << endl;
                cout << "read type: " << read_type_names.at(i) << ": " << endl;
            }
            for (int rank = 0; rank < obj_names.size(); rank++) {
                vector<uint64_t> proc_off_and_cts = offsets_and_counts.at(rank);
                for (int obj_indx = 0; obj_indx < obj_names.at(rank).size(); obj_indx++) {
                    string obj_name = obj_names.at(rank).at(obj_indx); 
                    printf("%*d, %s, %d, %d, %d, %d, %d, %d \n", len2, rank, obj_name.c_str(),
                        proc_off_and_cts.at(6*obj_indx), proc_off_and_cts.at(6*obj_indx + 1), proc_off_and_cts.at(6*obj_indx + 2),
                        proc_off_and_cts.at(6*obj_indx + 3), proc_off_and_cts.at(6*obj_indx + 4), proc_off_and_cts.at(6*obj_indx + 5) );
                }
            }
            if(num_matching_objs > 0) {
                cout << endl;
            }
        }
        free(serialized_c_str_all_object_names);
        free(serialized_c_str_all_offsets_and_counts);
        free(read_types_for_all_procs);
        if(output_timing) {
            cout << "begin timing output" << endl;
        }
    }

}

int add_object_names_offests_and_counts(const md_catalog_run_entry &run, const md_catalog_var_entry &var,
                                    const vector<md_dim_bounds> &proc_dims, uint16_t read_type
                                    )           
{         
    int rc;

    std::vector<string> obj_names;
    vector<uint64_t> offsets_and_counts;

    add_timing_point(BOUNDING_BOX_TO_OBJ_NAMES_START);

    // if (var.var_id == 0) {
    //     extreme_debug_log << "adding bounding box for: var: " << var.var_id << " timestep id: " << var.timestep_id << endl;
    //     for(int j = 0; j<3; j++) {
    //         extreme_debug_log <<  "proc_dims [" << to_string(j) << "] min is: " << to_string(proc_dims [j].min) << endl;
    //         extreme_debug_log <<  "proc_dims [" << to_string(j) << "] max is: " << to_string(proc_dims [j].max) << endl;
    //     }
    //     extreme_debug_log << endl;
    // }

    rc = boundingBoxToObjNamesAndCounts(run, var, proc_dims, obj_names, offsets_and_counts);
    if (rc != RC_OK) {
        error_log << "Error doing the bounding box to obj names and counts, returning \n";
        return RC_ERR;
    }
    add_timing_point(BOUNDING_BOX_TO_OBJ_NAMES_DONE);

    // for(int i = 0; i<obj_names.size(); i++) {
    //     extreme_debug_log << "obj name: " << obj_names.at(i) << endl;
    //     extreme_debug_log << "offset x: " << offsets_and_counts.at(i*6) << " count x: " << offsets_and_counts.at(i*6+1) << endl;
    //     extreme_debug_log << "offset y: " << offsets_and_counts.at(i*6+2) << " count y: " << offsets_and_counts.at(i*6+3) << endl;
    //     extreme_debug_log << "offset z: " << offsets_and_counts.at(i*6+4) << " count z: " << offsets_and_counts.at(i*6+5) << endl;
    // }

    extreme_debug_log << "adding " << obj_names.size() << " new objects" << endl;
    all_object_names.push_back(obj_names);
    all_offsets_and_counts.push_back(offsets_and_counts);
    add_objector_output_point(BOUNDING_BOX_TO_OBJ_NAMES, obj_names.size(), read_type);

    return rc;
}


