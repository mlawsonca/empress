#include <stdio.h> //needed for printf
#include <stdlib.h> //needed for atoi/atol/malloc/free/rand
#include <assert.h> //needed for assert
#include <string.h> //needed for strcmp

// #include <ctime>
#include <stdint.h> //needed for uint
// #include <ratio>
#include <chrono> //needed for high_resolution_clock
#include <math.h> //needed for pow()
#include <sys/time.h> //needed for timeval
#include <sys/stat.h> //needed for stat
#include <fstream> //needed for ifstream
#include <vector>
#include <float.h> //needed for DBL_MAX
#include <numeric>
#include <thread>
#include <mpi.h>

#include <my_metadata_client.h>
// #include <my_metadata_client_lua_functs.h>

 //needed for some opbox support
#include "dirman/DirMan.hh"
#include <sstream>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

// #include <hdf5_helper_functions_write.hh>#include <testing_harness_debug_helper_functions.hh>
// #include <md_client_timing_constants.hh>
#include <client_timing_constants_write_class_proj.hh>
#include <testing_harness_class_proj.hh>


//dw settings
std::string default_config_string = R"EOF(
# Tell the nnti transport to use infiniband

nnti.logger.severity       error
nnti.transport.name        ibverbs
whookie.interfaces         ib0,lo
 
#config.additional_files.env_name.if_defined   CONFIG
lunasa.lazy_memory_manager   malloc
lunasa.eager_memory_manager  tcmalloc

# Select the type of dirman to use. Currently we only have centralized, which
# just sticks all the directory info on one node (called root). 
dirman.type           centralized

# Turn these on if you want to see more debug messages
#bootstrap.debug           true
#whookie.debug             true
#opbox.debug               true
#dirman.debug              true
#dirman.cache.others.debug true
#dirman.cache.mine.debug   true

)EOF";

using namespace std;

// static bool extreme_debug_logging = false;
// static bool debug_logging = false;
// static bool zero_rank_logging = true;
static bool error_logging = true;

debugLog error_log = debugLog(error_logging, false);
// debugLog debug_log = debugLog(debug_logging, true);
// debugLog extreme_debug_log = debugLog(extreme_debug_logging, false);

static bool output_timing = true;
static bool insert_by_batch = true;
// static bool gathered_writes = true;
// static bool gathered_writes;

std::vector<int> catg_of_time_pts;
std::vector<long double> time_pts;
// std::vector<int> db_checkpt_types;

int zero_time_sec;

void add_timing_point(int catg) {
    if (output_timing) {
        struct timeval now;
        gettimeofday(&now, NULL);
        time_pts.push_back( (now.tv_sec - zero_time_sec) + now.tv_usec / 1000000.0);
        catg_of_time_pts.push_back(catg);
    }
}

double get_time() {
    struct timeval now;
    gettimeofday(&now, NULL);
    return ( now.tv_sec - zero_time_sec + now.tv_usec / 1000000.0);
}


// void add_db_checkpt_timing_point(int catg, int checkpt_type) {
//     if (output_timing) {
//         time_pts.push_back(chrono::high_resolution_clock::now());
//         catg_of_time_pts.push_back(catg);
//         db_checkpt_types.push_back(checkpt_type);
//     }
// }

MPI_Comm shared_server_comm;

double clear_cache(uint32_t ndx, uint32_t ndy, uint32_t ndz, uint32_t num_vars);

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
    // //extreme_debug_log << name << endl;

    if (argc != 20) { 
        error_log << "Error. Program should be given 19 arguments. Dirman hexid, npx, npy, npz, nx, ny, nz,"
                  << " number of timesteps, estm num time_pts, num_servers, job_id, read_npx, read_npy, read_npz, " 
                  << "timesteps_per_checkpt, write_type, server_type, checkpt_type, do_read" << endl;
        cout << (int)ERR_ARGC << " " << 0 <<  endl;
        return RC_ERR;
    }


    string dirman_hexid = argv[1];
    uint32_t num_x_procs = stoul(argv[2],nullptr,0);
    uint32_t num_y_procs = stoul(argv[3],nullptr,0);
    uint32_t num_z_procs = stoul(argv[4],nullptr,0);
    uint64_t total_x_length = stoull(argv[5],nullptr,0);
    uint64_t total_y_length = stoull(argv[6],nullptr,0);
    uint64_t total_z_length = stoull(argv[7],nullptr,0);
    uint32_t num_timesteps = stoul(argv[8],nullptr,0);
    // uint32_t num_types = stoul(argv[9],nullptr,0);
    uint32_t estm_num_time_pts = stoul(argv[9],nullptr,0); 
    uint32_t num_servers = stoul(argv[10],nullptr,0);
    uint64_t job_id = stoull(argv[11],nullptr,0);

    uint32_t num_read_x_procs = stoul(argv[12],nullptr,0);
    uint32_t num_read_y_procs = stoul(argv[13],nullptr,0);
    uint32_t num_read_z_procs = stoul(argv[14],nullptr,0);
    uint32_t num_timesteps_per_checkpt = stoul(argv[15],nullptr,0);
    md_write_type write_type = (md_write_type)stoul(argv[16],nullptr,0);
    md_server_type server_type = (md_server_type)stoul(argv[17],nullptr,0);

    // gathered_writes = stoul(argv[16],nullptr,0);
    // int checkpt_type = stoul(argv[17],nullptr,0);

    md_db_checkpoint_type checkpt_type = (md_db_checkpoint_type)stoul(argv[18],nullptr,0);
    bool do_read = stoul(argv[19],nullptr,0);

    // //extreme_debug_log << "write_type: " << write_type << endl;

    // //extreme_debug_log << "job_id: " << job_id << endl;


    //all 10 char plus null character
    uint32_t num_vars = 10;

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

    // if(gathered_writes && num_servers > 1) {
    if(num_servers > 1) {
        //debug_log << "num_servers > 1 so am splitting the comm" << endl;
        //debug_log << "num_servers > 1 so am splitting the comm" << endl;
        MPI_Comm_split(MPI_COMM_WORLD, rank%num_servers, rank, &shared_server_comm);
    }
    else {
        shared_server_comm = MPI_COMM_WORLD;
    }

    add_timing_point(MPI_INIT_DONE);

    //extreme_debug_log.set_rank(rank);
    //debug_log.set_rank(rank);
    error_log.set_rank(rank);

    rc = metadata_init(job_id);
    if (rc != RC_OK) {
        error_log << "Error initing the md_client. Exiting" << endl;
        return RC_ERR;
    }

    add_timing_point(METADATA_CLIENT_INIT_DONE);

    // if(rank == 0) {
    //     //extreme_debug_log << "starting" << endl;
    // }
    string dir_path="/metadata/testing";
    md_server dirman;

    md_server server;
    int server_indx;
    int my_num_clients_per_server;
    int remainder;

    vector<md_dim_bounds> proc_dims_vctr(3);

    int num_run_timestep_types = 2;
    uint32_t num_types = 10;

    // //extreme_debug_log << "num_type: " << num_types << " num_run_timestep_types: " << num_run_timestep_types << endl;

    uint32_t x_length_per_proc = total_x_length / num_x_procs; //todo - deal with edge case?
    uint32_t y_length_per_proc = total_y_length / num_y_procs;
    uint32_t z_length_per_proc = total_z_length / num_z_procs;

    uint64_t chunk_vol = x_length_per_proc * y_length_per_proc * z_length_per_proc;

    uint32_t num_dims;
    uint32_t var_num;
    uint32_t var_version;

    struct md_catalog_run_entry temp_run;
    struct md_catalog_timestep_entry temp_timestep;


    uint32_t x_pos, y_pos, z_pos;
    uint32_t x_offset, y_offset, z_offset;

    double all_timestep_temp_maxes_for_all_procs[num_timesteps];
    double all_timestep_temp_mins_for_all_procs[num_timesteps];


    int db_reset = false;

    // double timestep_temp_maxes[num_timesteps][num_vars] = {0};
    // double timestep_temp_mins[num_timesteps][num_vars] = {0};

    // double timestep_temp_maxes[num_timesteps]= {0};
    // double timestep_temp_mins[num_timesteps] = {0};

    vector<faodel::NameAndNode> server_procs;
    server_procs.resize(num_servers);

    // srand(rank+1);
    //extreme_debug_log << "rank: " << rank << " first rgn: " << rand() << endl;

    //just for testing-------------------------------------------

    // long double *all_clock_times;

    // double *total_times; 
    //--------------------------------------------------------------


    x_pos = rank % num_x_procs;
    y_pos = (rank / num_x_procs) % num_y_procs; 
    z_pos = ((rank / (num_x_procs * num_y_procs)) % num_z_procs);

    x_offset = x_pos * x_length_per_proc;
    y_offset = y_pos * y_length_per_proc;
    z_offset = z_pos * z_length_per_proc;

    //extreme_debug_log << "zoffset: " << to_string(z_offset) << endl;
    //extreme_debug_log << "x pos is " << to_string(x_pos) << " and x_offset is " << to_string(x_offset) << endl;
    //extreme_debug_log << "y pos is " << to_string(y_pos) << " and y_offset is " << to_string(y_offset) << endl;
    //extreme_debug_log << "z pos is " << to_string(z_pos) << " and z_offset is " << to_string(z_offset) << endl;
    //extreme_debug_log << "num x procs " << to_string(num_x_procs) << " num y procs " << to_string(num_y_procs) << " num z procs " << to_string(num_z_procs) << endl;


    num_dims = 3;
    proc_dims_vctr[0].min = x_offset;
    proc_dims_vctr[0].max = x_offset + x_length_per_proc-1;
    proc_dims_vctr[1].min = y_offset;
    proc_dims_vctr[1].max = y_offset + y_length_per_proc -1;
    proc_dims_vctr[2].min = z_offset;
    proc_dims_vctr[2].max = z_offset + z_length_per_proc -1;
    // vector<md_dim_bounds> proc_dims_vctr = std::vector<md_dim_bounds>(proc_dims_vctr, proc_dims_vctr + num_dims );

    // length_of_chunk = (dims [0].max - dims [0].min + 1)  * (dims [1].max - dims [1].min + 1)  * (dims [2].max - dims [2].min + 1);
    
    // for (int j=0; j<num_dims; j++) {
    //     ////extreme_debug_log << "dims [" << to_string(j) << "] min is: " << to_string(proc_dims_vctr [j].min) << endl;
    //     ////extreme_debug_log << "dims [" << to_string(j) << "] max is: " << to_string(proc_dims_vctr [j].max) << endl;
    // }   


//-------------------------------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------------------------------    

    int num_read_client_procs = num_read_x_procs * num_read_y_procs * num_read_z_procs;
    int read_color, shared_read_server_color, one_per_server_color;

    MPI_Comm read_comm, shared_read_server_comm, one_per_server_comm;

    uint64_t txn_id = -1;
    // if(server_type == SERVER_DEDICATED_SQLITE_TRANSACTION_MANAGEMENT_WAL) {
    //     txn_id = job_id;
    // }

    vector<md_dim_bounds> read_proc_dims(num_dims);

    const int num_timesteps_to_fetch = 6;
    // uint64_t timestep_ids[num_timesteps_to_fetch] = {(num_timesteps-1) / 6, (num_timesteps-1) / 5, (num_timesteps-1) / 4,
    //                     (num_timesteps-1) / 3, (num_timesteps-1) / 2, num_timesteps-1};
    // uint64_t timestep_ids[num_timesteps_to_fetch] = {(num_timesteps-1) / 6, (num_timesteps-1) / 5, (num_timesteps-1) / 4,
    //                     (num_timesteps-1) / 3, (num_timesteps-1) / 2, num_timesteps-1};    

    // uint64_t timestep_ids[num_timesteps_to_fetch] = {(num_timesteps-1) / 6, (num_timesteps-1) / 5, (num_timesteps-1) / 4,
    //                     (num_timesteps-1) / 3, (num_timesteps-1) / 2, num_timesteps-1};

    //makes sure that, by the time we're ready to read md and checkpt the db, the timesteps will have been written
    uint64_t timestep_ids[num_timesteps_to_fetch] = {0, 1%num_timesteps_per_checkpt, 2%num_timesteps_per_checkpt,
                        3%num_timesteps_per_checkpt, 4%num_timesteps_per_checkpt, 5%num_timesteps_per_checkpt};

    md_catalog_run_entry run;

    vector<vector<md_catalog_var_entry>> all_var_entries;   

    std::vector<md_catalog_var_entry> vars_to_fetch_pattern2;
    std::vector<md_catalog_var_entry> vars_to_fetch_pattern3;
    std::vector<md_catalog_var_entry> vars_to_fetch_pattern4;
    std::vector<md_catalog_var_entry> vars_to_fetch_pattern5;
    std::vector<md_catalog_var_entry> vars_to_fetch_pattern6;

    // std::vector<string> var_names_to_fetch(num_vars_to_fetch);
    // std::vector<uint32_t> var_vers_to_fetch(num_vars_to_fetch);
    int plane_x_procs;
    int plane_y_procs;
    // add_timing_point(INIT_READ_VARS_DONE);

    read_color = MPI_UNDEFINED;
    shared_read_server_color = MPI_UNDEFINED;
    one_per_server_color = MPI_UNDEFINED;
    if(rank < num_read_client_procs) {
        read_color = 0;
        shared_read_server_color = rank%num_servers;
        if(rank < num_servers) {
            one_per_server_color = 0;
        }
        //debug_log << "rank: " << rank << " read_color: " << read_color << " shared_read_server_color: " << shared_read_server_color << endl;
    }
    //make a comm with just the "read" procs
    MPI_Comm_split(MPI_COMM_WORLD, read_color, rank, &read_comm);
    MPI_Comm_split(MPI_COMM_WORLD, shared_read_server_color, rank, &shared_read_server_comm);
    MPI_Comm_split(MPI_COMM_WORLD, one_per_server_color, rank, &one_per_server_comm);

 //    if(rank < num_read_client_procs) {
    //  // MPI_Comm_split(read_comm, rank%num_servers, rank, &shared_read_server_comm);

    //     //make a comm for those reading from the same server
    //     int num_shared_read_procs;
    //      MPI_Comm_size(shared_read_server_comm, &num_shared_read_procs);
    //      //debug_log << "rank: " << rank << " num_shared_read_procs: " << num_shared_read_procs << endl;
    //     // num_read_client_procs = num_read_x_procs * num_read_y_procs * num_read_z_procs;

    //     int temp;
    //     MPI_Comm_size(read_comm, &temp);
    //  //debug_log << "confirming, num read clients: " << temp << endl;
    // }



//-------------------------------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------------------------------    


    add_timing_point(INIT_VARS_DONE);


    rc = setup_dirman(argv[1], dir_path, dirman, server_procs, rank, num_servers, num_client_procs);
    if (rc != RC_OK) {
        add_timing_point(ERR_DIRMAN);
        error_log << "Error. Was unable to setup the dirman. Exiting" << endl;
        return RC_ERR;
    }

    add_timing_point(DIRMAN_SETUP_DONE);

    setup_server(rank, num_servers, server_procs, server, server_indx);

    my_num_clients_per_server = num_client_procs / num_servers; //each has this many at a min
    remainder = num_client_procs - (num_servers * my_num_clients_per_server);
    if(server_indx < remainder) {
        my_num_clients_per_server +=1;
    }

    add_timing_point(SERVER_SETUP_DONE_INIT_DONE);

    MPI_Barrier(MPI_COMM_WORLD);

    //WRITE PROCESS
    //only want one copy of each var and type in each server's db
    add_timing_point(WRITING_START);


    // add_timing_point(DIMS_INIT_DONE);

    temp_run.job_id = job_id;
    temp_run.name = "XGC";
    // if(server_type == SERVER_DEDICATED_SQLITE_TRANSACTION_MANAGEMENT_WAL) {
    //     temp_run.txn_id = txn_id;
    // }    
    // else {
        temp_run.txn_id = 0;
    // }
    temp_run.npx = num_x_procs;
    temp_run.npy = num_y_procs;
    temp_run.npz = num_z_procs;

    string rank_to_dims_funct_name = "rank_to_bounding_box";
    string rank_to_dims_funct_path = "PATH_TO_EMPRESS/lib_source/lua/lua_function.lua";
    string objector_funct_name = "boundingBoxToObjectNamesAndCounts";
    string objector_funct_path = "PATH_TO_EMPRESS/lib_source/lua/metadata_functs.lua";

    // add_timing_point(RUN_INIT_DONE);

    //reminder - needs to be set because otherwise the ones that don't create the run
    //won't have the runid
    uint64_t run_id = 1;

    if (rank < num_servers) {
        //extreme_debug_log << "rank: " << rank << " about to create_and_activate_run" << endl;

        rc = create_and_activate_run(server, temp_run, rank_to_dims_funct_name, rank_to_dims_funct_path, 
                objector_funct_name, objector_funct_path);
        if (rc != RC_OK) {
            goto cleanup;
        }
        //extreme_debug_log << "rank: " << rank << " done with create_and_activate_run" << endl;
    }

    temp_run.run_id = run_id; //todo - do we want to make them catalog to get this? or change so registering doesn't require a name

    //reminder - unless you adjust create_type to ask for a type id, if you want the types in the given order (so you know how)
    //type_id corresponds to name/version, you need to insert them in order

    if (rank < num_servers) {
        //extreme_debug_log << "rank: " << rank << " about to create_and_activate_types" << endl;
        // //debug_log << std::setprecision(16) << get_time() << " rank: " << rank << " create_and_activate_types" << endl;
        rc = create_and_activate_types(server, run_id, num_types, num_run_timestep_types, server_type, txn_id);
        if (rc != RC_OK) {
            goto cleanup;
        }
        //extreme_debug_log << "rank: " << rank << " done with create_and_activate_types" << endl;
    }

    temp_timestep.run_id = run_id; //todo - will have to change if we want more than 1 run


    // add_timing_point(CREATE_TIMESTEPS_START);
    add_timing_point(CREATE_TIMESTEPS_START);


    for(uint32_t timestep_num=0; timestep_num < num_timesteps; timestep_num++) {
        // double sum = clear_cache(x_length_per_proc, y_length_per_proc, z_length_per_proc, num_vars);
        // //debug_log << std::setprecision(16) << get_time() << " rank: " << rank << " MPI_Barrier before start of timestep " << timestep_num << endl;
 
        //debug_log << "rank: " << rank << " about to barrier before start of timestep " << timestep_num << endl;
        MPI_Barrier(MPI_COMM_WORLD); //want to measure last-first for writing each timestep
        add_timing_point(CREATE_NEW_TIMESTEP_START);
        // //debug_log <<  get_time() << " rank: " << rank << " after MPI_Barrier for timestep " << timestep_num << endl;

        double timestep_temp_max;
        double timestep_temp_min;

        //extreme_debug_log << "rank: " << rank << " timestep_num: " << timestep_num << " (rank / num_servers): " << (rank / num_servers) << endl;

        //write once per server
        // if( (timestep_num % num_client_procs) == (rank / num_servers)  ) {
        // if( (timestep_num % my_num_clients_per_server) == (rank / num_servers)  ) {
        //have the last ones insert it since they tend to be the least busy
        if(rank >= (num_client_procs - num_servers) ) {

            //extreme_debug_log << "about to create timestep: " << timestep_num << " for server: " << rank % num_servers << endl;
            //extreme_debug_log << "for rank: " << rank << " my_num_clients_per_server: " << my_num_clients_per_server << endl;

            temp_timestep.timestep_id = timestep_num;      
            temp_timestep.path = temp_run.name + "/" + to_string(temp_timestep.timestep_id); //todo - do timestep need to use sprintf for this?
            // if(server_type == SERVER_DEDICATED_SQLITE_TRANSACTION_MANAGEMENT_WAL) {
            //     temp_timestep.txn_id = txn_id;
            // }
            // else {
                temp_timestep.txn_id = timestep_num; 
            // }
            // //extreme_debug_log << " timestep: " << temp_timestep.timestep_id << 
            //     " rank: " << rank << " num servers: " << num_servers << " num_timesteps: " << num_timesteps << endl;
            // //extreme_debug_log << "server hex id: " << server.URL << endl;
            // //debug_log << std::setprecision(16) << get_time() << " rank: " << rank << " metadata_create_timestep for timestep " << timestep_num << endl;

            rc = metadata_create_timestep (server, temp_timestep.timestep_id, temp_timestep.run_id, temp_timestep.path, temp_timestep.txn_id);
            if (rc != RC_OK) {
                error_log << "Error. Was unable to create the " << temp_timestep.timestep_id << "th timestep. Exiting" << endl;
                add_timing_point(ERR_CREATE_TIMESTEP);
                goto cleanup;
            }
        }
//-------------------------------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------------------------------

        // add_timing_point(CREATE_VARS_AND_ATTRS_FOR_NEW_TIMESTEP_START); 

        //debug_log << std::setprecision(16) << get_time() << " rank: " << rank << " create_vars for timestep " << timestep_num << endl;

        rc = create_vars(server, rank, num_servers, num_client_procs, my_num_clients_per_server,
                total_x_length, total_y_length, total_z_length, num_vars, timestep_num,
                server_type, txn_id);
        if (rc != RC_OK) {
            goto cleanup;
        }

        //debug_log << std::setprecision(16) << get_time() << " rank: " << rank << " create_var attrs for timestep " << timestep_num << endl;

        srand(rank*10000+1+timestep_num);

        rc = create_var_attrs(server, rank, proc_dims_vctr, timestep_num,
                num_vars, num_types, write_type, timestep_temp_min, timestep_temp_max,
                server_type, txn_id); //if not okay, just proceed


        // add_timing_point(CREATE_VARS_AND_ATTRS_FOR_NEW_TIMESTEP_DONE); 

//-------------------------------------------------------------------------------------------------------------------------------------------    
//-------------------------------------------------------------------------------------------------------------------------------------------        
        //debug_log << std::setprecision(16) << get_time() << " rank: " << rank << " create_timestep_attrs attrs for timestep " << timestep_num << endl;

       rc = create_timestep_attrs(server, rank, timestep_temp_min, timestep_temp_max, timestep_num,
                num_types, num_run_timestep_types, num_client_procs, all_timestep_temp_mins_for_all_procs,
                all_timestep_temp_maxes_for_all_procs, server_type, txn_id); //if not okay, just proceed


        //reminder - need to barrier so that all instances have been submitted before activating

        add_timing_point(CREATE_TIMESTEP_DONE); 

        // MPI_Barrier(MPI_COMM_WORLD);
        //debug_log << "rank: " << rank << " about to barrier before activate timestep " << timestep_num << endl;
        //debug_log << std::setprecision(16) << get_time() << " rank: " << rank << " before barrier to shared_server_comm for timestep " << timestep_num << endl;

        MPI_Barrier(shared_server_comm);

        //debug_log << std::setprecision(16) << get_time() << " rank: " << rank << " after barrier to shared_server_comm for timestep " << timestep_num << endl;

        if(rank < num_servers) {
            //debug_log << std::setprecision(16) << get_time() << " rank: " << rank << " activate_timestep_components for timestep " << timestep_num << endl;

           rc = activate_timestep_components(server, timestep_num);
            if (rc != RC_OK) {
                goto cleanup;
            }          
        }
        add_timing_point(CREATE_AND_ACTIVATE_TIMESTEP_DONE); 


//-------------------------------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------------------------------    

        if(timestep_num == num_timesteps-1) {
            add_timing_point(CREATE_ALL_TIMESTEPS_DONE); 

         //    if(rank == 0) {
            //     for (int i = 0; i<num_timesteps; i++) {
            //             ////debug_log << "timestep: " << i << " max: " << all_timestep_temp_maxes_for_all_procs[i] << endl;
            //             ////debug_log << "timestep: " << i << " min: " << all_timestep_temp_mins_for_all_procs[i] << endl;
            //     }  
            // }
            
            if (rank == 0 && num_run_timestep_types > 0) {
                //debug_log << std::setprecision(16) << get_time() << " rank: " << rank << " create_run_attrs for timestep " << timestep_num << endl;

                rc = create_run_attrs(server, rank, all_timestep_temp_mins_for_all_procs, all_timestep_temp_maxes_for_all_procs,
                        run_id, num_timesteps, num_types, server_type, txn_id);

                if(rc != RC_OK) {
                    goto cleanup;
                }
            }
            add_timing_point(WRITING_DONE);
        }

        if(timestep_num%10 == 0 && rank == 0) {
            cout << "timestep: " << timestep_num << endl;
        }
//-------------------------------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------------------------------    

        if( ((timestep_num+1) % num_timesteps_per_checkpt) == 0 && rank < num_read_client_procs ) {

            //initialize the reading vars before the first checkpt
            if( timestep_num+1 == num_timesteps_per_checkpt && do_read) {
                //debug_log << std::setprecision(16) << get_time() << " rank: " << rank << " read_init for timestep " << timestep_num << endl;

                rc = read_init(server, rank, num_servers, num_read_client_procs, num_read_x_procs, 
                    num_read_y_procs, num_read_z_procs, num_timesteps_to_fetch, txn_id, timestep_ids,
                    read_comm, shared_read_server_comm, run, read_proc_dims, plane_x_procs, plane_y_procs,
                    all_var_entries, vars_to_fetch_pattern2, vars_to_fetch_pattern3, 
                    vars_to_fetch_pattern4, vars_to_fetch_pattern5, vars_to_fetch_pattern6
                );
                //debug_log << "rank: " << rank << " just finished read init " << endl;

            }
            //once the DB has exceeded RAM, the server no longer has access to the entire DB in mem so we stop doing reads
            if(do_read && !db_reset) {
                //debug_log << std::setprecision(16) << get_time() << " rank: " << rank << " do_reads for timestep " << timestep_num << endl;

                rc = do_reads(server, rank, num_read_x_procs, num_read_y_procs, num_read_z_procs,
                            num_timesteps, num_servers, num_read_client_procs, txn_id,
                            read_comm, shared_read_server_comm, run, read_proc_dims, plane_x_procs, plane_y_procs, all_var_entries,
                            vars_to_fetch_pattern2, vars_to_fetch_pattern3, vars_to_fetch_pattern4, 
                            vars_to_fetch_pattern5, vars_to_fetch_pattern6
                        ); 
                    //debug_log << "rank: " << rank << " just finished do read " << endl;
            }

            // if(rank < num_servers) {
            //  //extreme_debug_log << "checkpointing for the " << (timestep_num/num_timesteps_per_checkpt) + 1 << "th time" << endl;
            //  rc = metadata_checkpoint_database (server, job_id);
            //  if(rc != RC_OK) {
            //      error_log << "error checkpointing the db to disk" << endl;
            //  }
            // }

            if(rank < num_servers && server_type != SERVER_DEDICATED_ON_DISK && server_type != SERVER_LOCAL_ON_DISK) {
                if(rank == 0) {
                    cout << "timestep: " << timestep_num << endl;
                }
                //debug_log << std::setprecision(16) << get_time() << " rank: " << rank << " metadata_checkpoint_database for timestep " << timestep_num << endl;

                //debug_log << "about to checkpoint the database" << endl;
                //debug_log << "time: " << time_pts.back() << endl;
                rc = metadata_checkpoint_database (server, job_id, checkpt_type);
                //debug_log << "metadata_checkpoint_database: rc: " << rc << endl;
                if(rc == RC_DB_RESET) {
                    //debug_log << "db reset" << endl;
                    // if(!db_reset) {
                    //  db_reset = true;
                    //  MPI_Bcast(&db_reset, 1, MPI_INT, 0, shared_read_server_comm);
                    // }
                    db_reset = true;
                    //debug_log << "db exceeded avail RAM so db is no longer whole" << endl;
                    rc = RC_OK;
                }
                else if(rc != RC_OK) {
                    error_log << "error doing a db checkpoint of type " << (int)checkpt_type << endl;
                }
                //debug_log << "done checkpointing the database" << endl;

                //debug_log << "my_db_reset: " << db_reset << endl;
                //makes sure that if any of the servers have a db reset, all read clients know not to continue reading
                if(num_servers > 1) {
                    MPI_Allreduce(MPI_IN_PLACE, &db_reset, 1, MPI_INT, MPI_MAX, one_per_server_comm);
                }
                //debug_log << "after allreduce, db_reset: " << db_reset << endl;
            }
            // if(!db_reset) {
                MPI_Bcast(&db_reset, 1, MPI_INT, 0, shared_read_server_comm);
                //debug_log << "rank: " << rank << " db_reset: " << db_reset << endl;
            // }
        }
//-------------------------------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------------------------------    

    }

 //    if(rank == 0) {
    //     for (int i = 0; i<num_timesteps; i++) {
    //             //debug_log << "timestep: " << i << " max: " << all_timestep_temp_maxes_for_all_procs[i] << endl;
    //             //debug_log << "timestep: " << i << " min: " << all_timestep_temp_mins_for_all_procs[i] << endl;
    //     }  
    // }
    
 //    if (rank == 0 && num_run_timestep_types > 0) {
 //        rc = create_run_attrs(server, rank, all_timestep_temp_mins_for_all_procs, all_timestep_temp_maxes_for_all_procs,
 //                run_id, num_timesteps, num_types);

 //        if(rc != RC_OK) {
 //            goto cleanup;
 //        }
 //    }

 //    add_timing_point(WRITING_DONE);

//-------------------------------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------------------------------        
cleanup:
    MPI_Barrier (MPI_COMM_WORLD);
    add_timing_point(WRITING_DONE_FOR_ALL_PROCS_START_CLEANUP);
        
    metadata_finalize_client();

    //extreme_debug_log << "num_servers: " << num_servers << endl;

    if( rank < num_servers) {
        metadata_finalize_server(server);
        add_timing_point(SERVER_SHUTDOWN_DONE);
        //debug_log << "just finished finalizing" << endl;
        // //extreme_debug_log << "just finished finalizing server" << endl;
        if(rank == 0) {
            metadata_finalize_server(dirman);
            add_timing_point(DIRMAN_SHUTDOWN_DONE);
        }
    }
    //comes before output_timing_stats to make sure all ops have finished (including finalize) before outputting timing
    faodel::bootstrap::Finish();

    if(output_timing) {
        output_timing_stats(rank, num_client_procs);
    }
    
    // free(total_times)
    MPI_Barrier (MPI_COMM_WORLD);
    MPI_Finalize();
    //debug_log << "got to cleanup7" << endl;

    return rc;

}

double clear_cache(uint32_t ndx, uint32_t ndy, uint32_t ndz, uint32_t num_vars)
{
    const int num_elems = ndx*ndy*ndz*num_vars;
    double *A = (double *)malloc(num_elems*sizeof(double));

    int sign = 1;
    for(int i = 0; i < num_elems; i++) {
        A[i] = 2*i*sign;
        sign *= -1;
    }

    double sum = 0;
    for(int i = 0; i < num_elems; i++) {
        sum += A[i];
    }
    free(A);
    return sum;
}



int create_and_activate_run(md_server server, md_catalog_run_entry &run, const string &rank_to_dims_funct_name,
    const string &rank_to_dims_funct_path, const string &objector_funct_name, const string &objector_funct_path)
{
    int rc = RC_OK;

    add_timing_point(CREATE_NEW_RUN_START);

    //debug_log << "about to metadata_create_run" << endl;
    rc = metadata_create_run  ( server, run.run_id, run.job_id, run.name, run.txn_id, run.npx, run.npy, run.npz,
                        rank_to_dims_funct_name, rank_to_dims_funct_path, objector_funct_name, objector_funct_path);
    if (rc != RC_OK) {
        error_log << "Error. Was unable to create the first run. Exitting" << endl;
        add_timing_point(ERR_CREATE_RUN);
        return rc;
    }

    //debug_log << "about to metadata_activate_run" << endl;
    rc = metadata_activate_run  ( server, run.txn_id);
    if (rc != RC_OK) {
        error_log << "Error. Was unable to activate the run. Exitting" << endl;
        add_timing_point(ERR_ACTIVATE_RUN);
        return rc;
    }

    add_timing_point(CREATE_NEW_RUN_DONE);

    return rc;
}

int create_and_activate_types(md_server server, uint64_t run_id, uint32_t num_types, 
    uint32_t num_run_timestep_types, md_server_type server_type, uint64_t txn_id) 
{
    int rc = RC_OK;
    md_catalog_type_entry temp_type;

    char type_names[10][13] = {"blob_freq", "blob_ifreq", "blob_rare", "max_val_type", "min_val_type", "note_freq", "note_ifreq", "note_rare", "ranges_type1", "ranges_type2"}; 
    char run_timestep_type_names[2][9] = {"max_temp", "min_temp"}; 

    add_timing_point(CREATE_TYPES_START);

    vector<md_catalog_type_entry> all_types_to_insert;
    all_types_to_insert.reserve(num_types + num_run_timestep_types);

    for(int type_indx=0; type_indx < (num_types + num_run_timestep_types); type_indx++) {
        // add_timing_point(CREATE_NEW_TYPE_START);

        uint64_t type_indx_adj = type_indx % num_types;
        bool is_run_type = (type_indx >= num_types);

        //extreme_debug_log << "type_indx: " << type_indx << endl;

        temp_type.run_id = run_id;
        if(is_run_type) {
            temp_type.name = run_timestep_type_names[type_indx_adj];
        }
        else {
            temp_type.name = type_names[type_indx_adj];
        }
        temp_type.version = 0; 
        // if(server_type == SERVER_DEDICATED_SQLITE_TRANSACTION_MANAGEMENT_WAL) {
        //     temp_type.txn_id = txn_id;
        // } 
        // else {
            temp_type.txn_id = run_id;
        // }
        // add_timing_point(TYPE_INIT_DONE);

        uint64_t temp_type_id;

        if(insert_by_batch) {
            all_types_to_insert.push_back(temp_type);
        }
        else {
            //debug_log << "about to metadata_create_type" << endl;
            rc = metadata_create_type (server, temp_type_id, temp_type);
            if (rc != RC_OK) {
                add_timing_point(ERR_CREATE_TYPE);
                error_log << "Error. Was unable to insert the type of index " << type_indx << ". Proceeding" << endl;
                return rc;
            }
        }
    }

    if(insert_by_batch) {
        // vector<uint64_t> type_ids;
        uint64_t first_type_id;
        // rc = metadata_create_type_batch (server, type_ids, all_types_to_insert);

        //debug_log << "about to metadata_create_type_batch" << endl;
        rc = metadata_create_type_batch (server, first_type_id, all_types_to_insert);
        if (rc != RC_OK) {
            add_timing_point(ERR_CREATE_TYPE_BATCH);
            error_log << "Error. Was unable to insert the types by batch. Proceeding" << endl;
            return rc;
        }
    }

    //debug_log << "about to metadata_activate_type" << endl;
    rc = metadata_activate_type  ( server, run_id);
    if (rc != RC_OK) {
        error_log << "Error. Was unable to activate the types. Exitting" << endl;
        add_timing_point(ERR_ACTIVATE_TYPE);
        return rc;
    }

    add_timing_point(CREATE_TYPES_DONE);

    return rc;
}

int create_vars(md_server server, int rank, uint32_t num_servers, uint32_t num_client_procs, 
    uint32_t my_num_clients_per_server,
    uint64_t total_x_length, uint64_t total_y_length, uint64_t total_z_length, uint32_t num_vars,
    uint32_t timestep_num, md_server_type server_type, uint64_t txn_id
    )
{
    add_timing_point(CREATE_VARS_START);


    int rc = RC_OK;

    char var_names[10][11] = {"temperat", "temperat", "pressure", "pressure", "velocity_x", "velocity_y", "velocity_z", "magn_field", "energy_var", "density_vr"};  
 
    uint32_t version1 = 1;
    uint32_t version2 = 2;

    vector<md_dim_bounds> var_dims(3);
    vector<md_catalog_var_entry> all_vars_to_insert;
    if(insert_by_batch) {
        if(rank >= num_servers) { //if batch insert, only rank < num_servers need to insert the vars
            return RC_OK;
        }
        all_vars_to_insert.reserve(num_vars);
    }

    for(uint32_t var_indx=0; var_indx < num_vars; var_indx++) {
        // add_timing_point(CREATE_VAR_AND_ATTRS_FOR_NEW_VAR_START);

        md_catalog_var_entry temp_var;

        //extreme_debug_log << " var_indx: " << var_indx << " rank: " << rank << " num servers: " << num_servers << endl;

        // uint64_t var_id = my_num_clients_per_server * var_indx + (rank / num_servers);
        temp_var.run_id = 1;
        temp_var.timestep_id = timestep_num;
        temp_var.data_size = 8; //double - 64 bit floating point

        // if(server_type == SERVER_DEDICATED_SQLITE_TRANSACTION_MANAGEMENT_WAL) {
        //     temp_var.txn_id = txn_id;
        // }
        // else {
            temp_var.txn_id = timestep_num;         
        // }   

        temp_var.name = var_names[var_indx];
        temp_var.path = "/"+ (string) temp_var.name;
        if(var_indx == 1 || var_indx == 3) { //these have been designated ver2
            temp_var.version = version2;
        }
        else {
            temp_var.version = version1;
        }
        temp_var.var_id = var_indx;
        temp_var.num_dims = 3;
        var_dims [0].min = 0;
        var_dims [0].max = total_x_length-1;
        var_dims [1].min = 0;
        var_dims [1].max = total_y_length-1;
        var_dims [2].min = 0;
        var_dims [2].max = total_z_length-1;
        // temp_var.dims = std::vector<md_dim_bounds>(var_dims, var_dims + temp_var.num_dims );
        temp_var.dims = var_dims;

        // add_timing_point(VAR_INIT_DONE);

        uint64_t temp_var_id;

        // if ( (var_indx % my_num_clients_per_server) == (rank / num_servers) ) { 

        if(insert_by_batch && rank < num_servers) {
            //extreme_debug_log << "rank just pushed back var with id: " << temp_var.var_id << endl;
            all_vars_to_insert.push_back(temp_var);

        }
        else if (!insert_by_batch) {
            //write vars once per server
            // if ( (var_indx % num_client_procs) == (rank / num_servers) )  { 
            if ( (var_indx % my_num_clients_per_server) == (rank / num_servers) ) {
                //extreme_debug_log << "var_indx: " << var_indx << endl;
           
                rc = metadata_create_var (server, temp_var_id, temp_var);
                if (rc != RC_OK) {
                    error_log << "Error. Was unable to create the temp var. Exiting" << endl;
                    error_log << "Failed var_id " << to_string(temp_var_id) << endl;
                    add_timing_point(ERR_CREATE_VAR);
                    return rc;
                }

                //extreme_debug_log << "rank " << rank << " and server hexid: " << server.URL << endl;
                //extreme_debug_log << " Finished creating var. new var_id " << var_indx << endl;
            }
            else {
                //extreme_debug_log << "var id is greater than num vars so am skipping create" << endl;
            }
        }
    } //var loop done

    if(insert_by_batch) { //only rank < num_servers are still in the function
        // //debug_log << "rank: " << rank << " about to insert all vars of size: " << all_vars_to_insert.size() << endl;
        //debug_log << "rank: " << rank << " about to insert all vars for timestep: " << timestep_num << endl;
        rc = metadata_create_var_batch (server, all_vars_to_insert);
        if (rc != RC_OK) {
            error_log << "Error. Was unable to insert the batch vars. Proceeding" << endl;
            add_timing_point(ERR_CREATE_VAR_BATCH);
        }
    }
    add_timing_point(CREATE_VARS_DONE);

    return rc;
}

int create_var_attrs(md_server server, int rank, const vector<md_dim_bounds> &proc_dims_vctr,
    uint32_t timestep_num, uint32_t num_vars, uint32_t num_types, md_write_type write_type, 
    double &timestep_temp_min, double &timestep_temp_max, md_server_type server_type, uint64_t txn_id
    )
{
    add_timing_point(CREATE_VAR_ATTRS_START); 

    int rc = RC_OK;

    vector<md_catalog_var_attribute_entry> all_attrs_to_insert;
    if(insert_by_batch) {
        all_attrs_to_insert.reserve(3*num_vars);
    }

    //given in %
    float type_freqs[] = {25, 5, .1, 100, 100, 20, 2.5, .5, 1, 10};
    // char type_types[10][3] = {"b", "b", "b", "d", "d", "2p","2p","2p","2d","2d"};
    char type_types[10][3] = {"b", "b", "b", "d", "d", "s","s","s","2d","2d"};


    for(uint32_t var_indx=0; var_indx < num_vars; var_indx++) {

        // add_timing_point(CREATE_VAR_ATTRS_FOR_NEW_VAR_START); 

        for(uint32_t type_indx=0; type_indx<num_types; type_indx++) {
            // std::chrono::high_resolution_clock::time_point create_attr_start_time = chrono::high_resolution_clock::now();

            float freq = type_freqs[type_indx];
            uint32_t odds = 100 / freq;
            uint32_t val = rand() % odds; 
            ////extreme_debug_log << "rank: " << to_string(rank) << " and val: " << to_string(val) << " and odds: " << to_string(odds) << endl;
            if(val == 0) { //makes sure we have the desired frequency for each type
                // add_timing_point(CREATE_NEW_VAR_ATTR_START);

                uint64_t attr_id;
                md_catalog_var_attribute_entry temp_attr;
                temp_attr.timestep_id = timestep_num;
                temp_attr.type_id = type_indx+1; //note - if we change the id system this will change
                temp_attr.var_id = var_indx; //todo - are we okay with this?
                // if(server_type == SERVER_DEDICATED_SQLITE_TRANSACTION_MANAGEMENT_WAL) {
                //     temp_attr.txn_id = txn_id;
                // }
                // else {
                    temp_attr.txn_id = timestep_num;
                // }
                temp_attr.num_dims = 3;
                // temp_attr.dims = std::vector<md_dim_bounds>(proc_dims_vctr, proc_dims_vctr + num_dims ); 
                temp_attr.dims = proc_dims_vctr; //todo - you wont want this to be the entire chunk for all attrs

                if(strcmp(type_types[type_indx], "b") == 0) {
                    bool flag = rank % 2;
                    // make_single_val_data(flag, temp_attr.data);
                    temp_attr.data = to_string(flag);
                    temp_attr.data_type = ATTR_DATA_TYPE_INT;                            
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
                    // make_single_val_data(val, temp_attr.data);
                    temp_attr.data = to_string(val);
                    temp_attr.data_type = ATTR_DATA_TYPE_REAL;  
                    //if write data the chunk min and max for var 0 will be based on the actual data values (this is done above)
                    if(var_indx == 0 )  { 
                        if ( type_indx == 3 )  {
                            timestep_temp_max = val;
                            //extreme_debug_log << "for rank: " << rank << " timestep: " << timestep_num << " just added " << val << " as the max value" << endl;
                        }
                        else if (type_indx == 4 ) {
                            timestep_temp_min = val;
                            //extreme_debug_log << "for rank: " << rank << " timestep: " << timestep_num << " just added " << val << " as the min value" << endl;                                    
                        }
                    }                       
                }
                else if(strcmp(type_types[type_indx], "s") == 0) { 
                    temp_attr.data = "attr data for var" + to_string(var_indx) + "timestep"+to_string(timestep_num);
                    temp_attr.data_type = ATTR_DATA_TYPE_TEXT;

                }               
                else if(strcmp(type_types[type_indx], "2d") == 0) {
                    vector<int> vals(2);
                    vals[0] = (int) (rand() % RAND_MAX); 
                    vals[1] = vals[0] + 10000; 
                    make_single_val_data(vals, temp_attr.data);
                    temp_attr.data_type = ATTR_DATA_TYPE_BLOB;

                }
                else {
                    error_log << "error. type didn't match list of possibilities" << endl;
                    add_timing_point(ERR_TYPE_COMPARE);
                }
                // add_timing_point(VAR_ATTR_INIT_DONE);

                if(insert_by_batch) {
                    all_attrs_to_insert.push_back(temp_attr);
                    // if(all_attrs_to_insert.size() < 5) {
                    //     //extreme_debug_log << "for rank: " << rank << " the " << all_attrs_to_insert.size() << "th attr has val: " << temp_attr.data << endl;
                    // }
                }
                else {
                    // rc = metadata_insert_var_attribute_by_dims (server, attr_id, temp_attr);
                    rc = metadata_insert_var_attribute_by_dims (server, attr_id, temp_attr);
                    if (rc != RC_OK) {
                        error_log << "Error. Was unable to insert the attribute for type indx" << type_indx << "var indx " << var_indx <<". Proceeding" << endl;
                        add_timing_point(ERR_INSERT_VAR_ATTR);
                    }
                    //extreme_debug_log << "rank: " << rank << "attr.data: " << temp_attr.data << endl;
                }

               // add_timing_point(CREATE_NEW_VAR_ATTR_DONE);
            }//val = 0 done
        } //type loop done
        // add_timing_point(CREATE_VAR_ATTRS_FOR_NEW_VAR_DONE);
    }//var loop done

    if(insert_by_batch) {
        if(write_type == WRITE_GATHERED) { //have one proc gather and insert the attrs
            rc = metadata_collective_insert_var_attribute_by_dims_batch(server, all_attrs_to_insert, shared_server_comm);
            if (rc != RC_OK) {
                error_log << "Error. Was unable to collectively insert the new var attributes. Proceeding" << endl;
            }
        }
        else { //by default have each client insert independently
            //debug_log << "about to insert all attrs of size: " << all_attrs_to_insert.size() << endl;
            rc = metadata_insert_var_attribute_by_dims_batch (server, all_attrs_to_insert);
            if (rc != RC_OK) {
                error_log << "Error. Was unable to insert the batch attributes. Proceeding" << endl;
                add_timing_point(ERR_INSERT_VAR_ATTR_BATCH);
            }
        }
    }
    add_timing_point(CREATE_VAR_ATTRS_DONE);

    return rc;
}


int create_timestep_attrs(md_server server, int rank, double timestep_temp_min, double timestep_temp_max,
    uint32_t timestep_num,
    uint32_t num_types, uint32_t num_run_timestep_types, uint32_t num_client_procs,
    double *all_timestep_temp_mins_for_all_procs, double *all_timestep_temp_maxes_for_all_procs,
    md_server_type server_type, uint64_t txn_id
    )
{
    int rc = RC_OK;
    add_timing_point(CREATE_TIMESTEP_ATTRS_START);

    // if(rank == 0 && num_run_timestep_types > 0) {
    if(num_run_timestep_types > 0) {

        double all_temp_maxes[num_client_procs];
        double all_temp_mins[num_client_procs];

        MPI_Gather(&timestep_temp_max, 1, MPI_DOUBLE, all_temp_maxes, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD); 
        MPI_Gather(&timestep_temp_min, 1, MPI_DOUBLE, all_temp_mins, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);

        if(rank == 0) {
            double timestep_var_max = -1 * RAND_MAX;
            double timestep_var_min = RAND_MAX;

            // add_timing_point(GATHER_DONE_PROC_MAX_MIN_FIND_START);
            for (int proc_rank = 0; proc_rank<num_client_procs; proc_rank++) {

                if( all_temp_maxes[proc_rank] > timestep_var_max) {
                    timestep_var_max = all_temp_maxes[proc_rank];
                }
                if( all_temp_mins[proc_rank] < timestep_var_min) {
                    timestep_var_min = all_temp_mins[proc_rank];
                }                    
            }
            // add_timing_point(PROC_MAX_MIN_FIND_DONE);

            all_timestep_temp_maxes_for_all_procs[timestep_num] = timestep_var_max;
            all_timestep_temp_mins_for_all_procs[timestep_num] = timestep_var_min;

            //debug_log << "timestep " << timestep_num << " temperature max: " << timestep_var_max << endl;
            //debug_log << "timestep " << timestep_num << " temperature min: " << timestep_var_min << endl;

            uint64_t temp_max_id;
            md_catalog_timestep_attribute_entry temp_max;
            temp_max.timestep_id = timestep_num;
            temp_max.type_id = num_types+1;
            // if(server_type == SERVER_DEDICATED_SQLITE_TRANSACTION_MANAGEMENT_WAL) {
            //     temp_max.txn_id = txn_id;
            // }
            // else {
                temp_max.txn_id = timestep_num;
            // }
            temp_max.data = to_string(timestep_var_max);
            // make_single_val_data(timestep_var_max, temp_max.data);
            temp_max.data_type = ATTR_DATA_TYPE_REAL;

            uint64_t temp_min_id;
            md_catalog_timestep_attribute_entry temp_min;
            temp_min.timestep_id = timestep_num;
            temp_min.type_id = num_types+2;
            // if(server_type == SERVER_DEDICATED_SQLITE_TRANSACTION_MANAGEMENT_WAL) {
            //     temp_min.txn_id = txn_id;
            // }
            // else {
                temp_min.txn_id = timestep_num;
            // }
            temp_min.data = to_string(timestep_var_min);
            // make_single_val_data(timestep_var_min, temp_min.data);
            temp_min.data_type = ATTR_DATA_TYPE_REAL;

            if(insert_by_batch) {
                vector<md_catalog_timestep_attribute_entry> all_timestep_attrs_to_insert = {temp_max, temp_min};

                rc = metadata_insert_timestep_attribute_batch (server, all_timestep_attrs_to_insert);
                if (rc != RC_OK) {
                    error_log << "Error. Was unable to insert timestep attributes by batch. Proceeding" << endl;
                    add_timing_point(ERR_INSERT_TIMESTEP_ATTR_BATCH);
                }
                //debug_log << "Rank " << rank << " just inserted all timestep attrs for timestep " << timestep_num << endl;
            }

            else {
                rc = metadata_insert_timestep_attribute (server, temp_max_id, temp_max);
                if (rc != RC_OK) {
                    error_log << "Error. Was unable to insert the new timestep attribute. Proceeding" << endl;
                    add_timing_point(ERR_INSERT_TIMESTEP_ATTR);
                }
                //debug_log << "New timestep attribute id is " << to_string(temp_max_id) << endl;

                rc = metadata_insert_timestep_attribute (server, temp_min_id, temp_min);
                if (rc != RC_OK) {
                    error_log << "Error. Was unable to insert the new timestep attribute. Proceeding" << endl;
                    add_timing_point(ERR_INSERT_TIMESTEP_ATTR);
                }
                //debug_log << "New timestep attribute id is " << to_string(temp_min_id) << endl;
            }
        }
    }
    // else if (num_run_timestep_types > 0) {
    //     MPI_Gather(&timestep_temp_max, 1, MPI_DOUBLE, NULL, NULL, MPI_DOUBLE, 0, MPI_COMM_WORLD); 
    //     MPI_Gather(&timestep_temp_min, 1, MPI_DOUBLE, NULL, NULL, MPI_DOUBLE, 0, MPI_COMM_WORLD); 
    // }

    add_timing_point(CREATE_TIMESTEP_ATTRS_DONE);

    return rc;
}

int activate_timestep_components(md_server server, uint32_t timestep_num)
{
    int rc; 
    add_timing_point(ACTIVATE_TIMESTEP_COMPONENTS_START);

    rc = metadata_activate_timestep  ( server, timestep_num);
    if (rc != RC_OK) {
        error_log << "Error. Was unable to activate the " << timestep_num << "th timestep. Exitting" << endl;
        add_timing_point(ERR_ACTIVATE_TIMESTEP);
        return rc;
    }

    rc = metadata_activate_var ( server, timestep_num);
    if (rc != RC_OK) {
        error_log << "Error. Was unable to activate the vars for the " << timestep_num << "th timestep. Exitting" << endl;
        add_timing_point(ERR_ACTIVATE_VAR);
        return rc;
    }

    rc = metadata_activate_var_attribute (server, timestep_num);
    if (rc != RC_OK) {
        error_log << "Error. Was unable to activate the types. Exitting" << endl;
        add_timing_point(ERR_ACTIVATE_VAR_ATTR);
        return rc;
    }

    rc = metadata_activate_timestep_attribute  ( server, timestep_num);
    if (rc != RC_OK) {
        error_log << "Error. Was unable to activate the timestep attrs. Exitting" << endl;
        add_timing_point(ERR_ACTIVATE_TIMESTEP_ATTR);
        return rc;
    }

    add_timing_point(ACTIVATE_TIMESTEP_COMPONENTS_DONE);

    return rc;
}

int create_run_attrs(md_server server, int rank, double *all_timestep_temp_mins_for_all_procs, 
    double *all_timestep_temp_maxes_for_all_procs, uint64_t run_id, uint32_t num_timesteps, uint32_t num_types,
    md_server_type server_type, uint64_t txn_id
    )
{
    int rc = RC_OK;

    add_timing_point(CREATE_RUN_ATTRS_START);

    double run_temp_max = -1 * RAND_MAX;
    double run_temp_min = RAND_MAX;

    // add_timing_point(TIMESTEP_MAX_MIN_FIND_START);

    for(int timestep = 0; timestep < num_timesteps; timestep++) {
        if( all_timestep_temp_maxes_for_all_procs[timestep] > run_temp_max) {
            run_temp_max = all_timestep_temp_maxes_for_all_procs[timestep];
        }
        if( all_timestep_temp_mins_for_all_procs[timestep] < run_temp_min) {
            run_temp_min = all_timestep_temp_mins_for_all_procs[timestep];
        }    
    }
    // add_timing_point(TIMESTEP_MAX_MIN_FIND_DONE);

    uint64_t temp_max_id;
    md_catalog_run_attribute_entry temp_max;
    temp_max.run_id = run_id;
    temp_max.type_id = num_types+1;
    // if(server_type == SERVER_DEDICATED_SQLITE_TRANSACTION_MANAGEMENT_WAL) {
    //     temp_max.txn_id = txn_id;
    // }
    // else {
        temp_max.txn_id = run_id;
    // }
    temp_max.data = to_string(run_temp_max);
    // make_single_val_data(run_temp_max, temp_max.data);
    //debug_log << "run temperature max: " << run_temp_max << endl;
    temp_max.data_type = ATTR_DATA_TYPE_REAL;

    uint64_t temp_min_id;
    md_catalog_run_attribute_entry temp_min;
    temp_min.run_id = run_id;
    temp_min.type_id = num_types+2;
    // if(server_type == SERVER_DEDICATED_SQLITE_TRANSACTION_MANAGEMENT_WAL) {
    //     temp_min.txn_id = txn_id;
    // }
    // else {
        temp_min.txn_id = run_id;
    // }
    temp_min.data = to_string(run_temp_min);
    // make_single_val_data(run_temp_min, temp_min.data);
    //debug_log << "run temperature min: " << run_temp_min << endl;
    temp_min.data_type = ATTR_DATA_TYPE_REAL;

    if(insert_by_batch) {
        vector<md_catalog_run_attribute_entry> all_run_attributes_to_insert = {temp_max, temp_min};

        rc = metadata_insert_run_attribute_batch (server, all_run_attributes_to_insert);
        if (rc != RC_OK) {
            error_log << "Error. Was unable to insert the new run attributes by batch. Proceeding" << endl;
            add_timing_point(ERR_INSERT_RUN_ATTR_BATCH);
        }
    }
    else {
        rc = metadata_insert_run_attribute (server, temp_max_id, temp_max);
        if (rc != RC_OK) {
            error_log << "Error. Was unable to insert the new run attribute. Proceeding" << endl;
            add_timing_point(ERR_INSERT_RUN_ATTR);
        }
        //debug_log << "New run attribute id is " << to_string(temp_max_id) << endl;

        rc = metadata_insert_run_attribute (server, temp_min_id, temp_min);
        if (rc != RC_OK) {
            error_log << "Error. Was unable to insert the new run attribute. Proceeding" << endl;
            add_timing_point(ERR_INSERT_RUN_ATTR);
        }
        //debug_log << "New run attribute id is " << to_string(temp_min_id) << endl;
    }
    rc = metadata_activate_run_attribute  ( server, run_id);
    if (rc != RC_OK) {
        error_log << "Error. Was unable to activate the run attrs. Exitting" << endl;
        add_timing_point(ERR_ACTIVATE_RUN_ATTR);
        return rc;
    }

    add_timing_point(CREATE_RUN_ATTRS_DONE);

    return rc;
}

static int setup_dirman(const string &dirman_file_path, const string &dir_path, md_server &dirman, 
                        vector<faodel::NameAndNode> &server_procs, int rank, uint32_t num_servers, uint32_t num_clients) {

    bool ok;
    int rc;
    char *serialized_c_str;
    int length_ser_c_str;

    if(rank == 0) { 
        struct stat buffer;
        bool dirman_initted = false;
        std::ifstream file;
        string dirman_hexid;
        faodel::DirectoryInfo dir;

        while (!dirman_initted) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            dirman_initted = (stat (dirman_file_path.c_str(), &buffer) == 0);
            //debug_log << "dirman not initted yet" << endl;
        }
        file.open(dirman_file_path);
        if(!file) {
            return RC_ERR;
        }
        while( file.peek() == std::ifstream::traits_type::eof() ) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            //debug_log << "dirman file empty" << endl;
        }
        file >> dirman_hexid;
        //debug_log << "just got the hexid: " << dirman_hexid << endl;


        faodel::Configuration config(default_config_string);
        config.Append("dirman.root_node", dirman_hexid);
        config.Append("whookie.port", to_string(3000+rank)); //note if you up number of servers you'll want to up this
        config.AppendFromReferences();
        //debug_log << "just configged" << endl; 
        faodel::bootstrap::Start(config, dirman::bootstrap);

        //Add the directory manager's URL to the config string so the clients know
        //Where to access the directory 
        //-------------------------------------------
        //TODO: This connect is temporarily necessary
        faodel::nodeid_t dirman_nodeid(dirman_hexid);
        //extreme_debug_log << "Dirman node ID " << dirman_nodeid.GetHex() << " " << dirman_nodeid.GetIP() << " port " << dirman_nodeid.GetPort() << endl;
        dirman.name_and_node.node = dirman_nodeid;
        //extreme_debug_log << "about to connect peer to node" << endl;
        rc = net::Connect(&dirman.peer_ptr, dirman.name_and_node.node);
        assert(rc==RC_OK && "could not connect");
        //extreme_debug_log << "just connected" << endl;
        //-------------------------------------------
        //extreme_debug_log << "app name is " << dir_path << endl;
        ok = dirman::GetRemoteDirectoryInfo(faodel::ResourceURL(dir_path), &dir);
        assert(ok && "Could not get info about the directory?");
        //extreme_debug_log << "just got directory info" << endl;

        while( dir.members.size() < num_servers) {
            //debug_log << "dir.members.size() < num_servers. dir.members.size(): "  << dir.members.size() << " num_servers: " << num_servers << endl;

            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            ok = dirman::GetRemoteDirectoryInfo(faodel::ResourceURL(dir_path), &dir);
            //extreme_debug_log << "and now dir.members.size() < num_servers. dir.members.size(): "  << dir.members.size() << " num_servers: " << num_servers << endl;

            assert(ok && "Could not get info about the directory?");
            //extreme_debug_log << "just got directory info" << endl;
        }
        //debug_log << "dir init is done" << endl;
        server_procs = dir.members;
        //extreme_debug_log << "about to serialize server_procs" << endl;
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
            error_log << "error. thinks the correct number of servers is 0" << endl;
            return RC_ERR;
        }
    }
    MPI_Bcast(&length_ser_c_str, 1, MPI_INT, 0, MPI_COMM_WORLD);

    if(rank != 0) {
        //debug_log << "num_bytes is "<< to_string(length_ser_c_str) + " and rank: " << to_string(rank) << endl;
        serialized_c_str = (char *) malloc(length_ser_c_str);  
    }

    MPI_Bcast(serialized_c_str, length_ser_c_str, MPI_CHAR, 0, MPI_COMM_WORLD);

    if (rank != 0) {
        //extreme_debug_log << "serialized_c_str: " << (string)serialized_c_str << endl;
        stringstream ss1;
        ss1.write(serialized_c_str, length_ser_c_str);
        boost::archive::text_iarchive ia(ss1);
        ia >> server_procs;

        //todo - should decide if we want to have a dedicated dirman, if 
        //not, just need to adjust so "dirman" is server rank 0 
        faodel::Configuration config(default_config_string);
        config.Append("dirman.root_node", server_procs[0].node.GetHex());
        config.Append("whookie.port", to_string(3000+rank)); //note if you up number of servers you'll want to up this
        config.AppendFromReferences();
        //debug_log << "just configged" << endl; 
        faodel::bootstrap::Start(config, dirman::bootstrap);

    } 
    free(serialized_c_str);


    // for (int i=0; i<num_servers; i++) {
        ////extreme_debug_log << "server " << i << " in ary has hexid " << server_procs[i].node.GetHex() << endl;
    // }

    metadata_network_init();

    // server_procs = temp(server_ary, server_ary +(int)num_servers);
    //extreme_debug_log << "done initing dirman" << endl;
    return RC_OK;
 } 


//attempts to be more efficient but had problems
// static int setup_dirman(const string &dirman_hostname, const string &dir_path, md_server &dirman, 
//                         vector<faodel::NameAndNode> &server_procs, int rank, uint32_t num_servers, uint32_t num_clients, int dirman_port) {
//     bool dirman_initted = false;
//     int rc = RC_ERR;
//     bool ok;
//     char *serialized_c_str;
//     int length_ser_c_str;


        
//     if(rank == 0) {

//         faodel::DirectoryInfo dir;
//         opbox::net::peer_ptr_t peer;
//         faodel::nodeid_t dirman_nodeid(dirman_hostname, to_string(dirman_port));
//         dirman.name_and_node.node = dirman_nodeid;

//         faodel::Configuration config(default_config_string);
//         config.Append("dirman.type", "centralized");
//         config.Append("dirman.root_node", dirman_nodeid.GetHex());
//         config.Append("whookie.port", to_string(3000+rank)); //note if you up number of servers you'll want to up this
//         config.AppendFromReferences();
//         //debug_log << "just configged" << endl; 
//         faodel::bootstrap::Start(config, dirman::bootstrap);

//         do {
//             std::this_thread::sleep_for(std::chrono::milliseconds(10));
//             rc = net::Connect(&dirman.peer_ptr, dirman_nodeid); 
//             cout << "could not connect. trying again" << endl;
//         } while (rc != RC_OK);
//         // if(rc != RC_OK) return rc;
//         // assert(rc==RC_OK && "could not connect");
//         //extreme_debug_log << "just connected" << endl;

//         ok = dirman::GetDirectoryInfo(faodel::ResourceURL(dir_path), &dir); 
//         // if(!ok) return RC_ERR;
//         assert(ok && "Could not get info about the directory?");

//         do {
//             std::this_thread::sleep_for(std::chrono::milliseconds(10));
//             ok = dirman::GetDirectoryInfo(faodel::ResourceURL(dir_path), &dir); 
//             // if (!ok) return RC_ERR;
//             assert(ok && "Could not get info about the directory?");
//             cout << "dir.members.size(): " << dir.members.size() << endl;
//             cout << "num_servers: " << num_servers << endl;
//         } while (dir.members.size() < num_servers);

//         server_procs = dir.members;
//         //extreme_debug_log << "about to serialize server_procs" << endl;
//         stringstream ss;
//         boost::archive::text_oarchive oa(ss);
//         if(server_procs.size() > 0) { 
//         //leaving off here. looks kike sometimes the string isn't getting copied properly? (could have embedded nulls)
//             oa << server_procs;
//             string serialized_str = ss.str();
//             length_ser_c_str = serialized_str.size() + 1;
//             serialized_c_str = (char *) malloc(length_ser_c_str);
//             serialized_str.copy(serialized_c_str, serialized_str.size());
//         }
//         else {
//             error_log << "error. thinks the correct number of servers is 0" << endl;
//             return RC_ERR;
//         }
//     }
//     MPI_Bcast(&length_ser_c_str, 1, MPI_INT, 0, MPI_COMM_WORLD);

//     if(rank != 0) {
//         serialized_c_str = (char *) malloc(length_ser_c_str);  
//     }

//     MPI_Bcast(serialized_c_str, length_ser_c_str, MPI_CHAR, 0, MPI_COMM_WORLD);

//     if (rank != 0) {
//         //extreme_debug_log << "serialized_c_str: " << (string)serialized_c_str << endl;
//         stringstream ss1;
//         ss1.write(serialized_c_str, length_ser_c_str);
//         boost::archive::text_iarchive ia(ss1);
//         ia >> server_procs;

//         faodel::Configuration config(default_config_string);
//         config.Append("dirman.type", "none");
//         config.Append("whookie.port", to_string(3000+rank)); //note if you up number of servers you'll want to up this
//         config.AppendFromReferences();
//         //debug_log << "just configged" << endl; 
//         faodel::bootstrap::Start(config, dirman::bootstrap);
//     } 
//     free(serialized_c_str);

//     //extreme_debug_log << "done initing dirman" << endl;
//     return RC_OK;
//  } 





static void setup_server(int rank, uint32_t num_servers, 
        const vector<faodel::NameAndNode> &server_nodes, md_server &server, int &server_indx) {

    server_indx = rank % num_servers; 

    server.name_and_node = server_nodes[server_indx];
    opbox::net::Connect(&server.peer_ptr, server.name_and_node.node);
    server.URL = server.name_and_node.node.GetHex();
    //extreme_debug_log << "server.URL: " << server.URL << endl;
    //extreme_debug_log << "server_indx: " << server_indx << endl;
    //debug_log << "rank: " << rank << " server_indx: " << server_indx << " URL: " << server.URL << endl;

}


void output_timing_stats(int rank, uint32_t num_client_procs )
{
    long double *all_time_pts_buf;

    int each_proc_num_time_pts[num_client_procs];
    int displacement_for_each_proc[num_client_procs];
    int *all_catg_time_pts_buf;

    gatherv_int(catg_of_time_pts, num_client_procs, rank, each_proc_num_time_pts,
        displacement_for_each_proc, &all_catg_time_pts_buf);


    // int each_proc_num_db_checkpts[num_client_procs];
    // int displacement_for_each_proc_num_db_checkpts[num_client_procs];
    // int *all_db_checkpt_types;
    // gatherv_int(db_checkpt_types, num_client_procs, rank, each_proc_num_db_checkpts,
    //     displacement_for_each_proc_num_db_checkpts, &all_db_checkpt_types);

    int sum = 0;
    if(rank == 0) {
        // for(int i = 0; i < num_client_procs; i++) {
        //     sum += each_proc_num_time_pts[i];
        // }
        sum = accumulate(each_proc_num_time_pts, each_proc_num_time_pts+num_client_procs, sum);
        //extreme_debug_log << "sum: " << sum << endl;
        all_time_pts_buf = (long double *) malloc(sum * sizeof(long double));
    }
 
    int num_time_pts = time_pts.size();
    //debug_log << "num_time pts: " << to_string(num_time_pts)  << "time_pts.size(): " << time_pts.size()<< endl;

    MPI_Gatherv(&time_pts[0], num_time_pts, MPI_LONG_DOUBLE, all_time_pts_buf, each_proc_num_time_pts, displacement_for_each_proc, MPI_LONG_DOUBLE, 0, MPI_COMM_WORLD); 
    
    // int db_checkpt_indx = 0;
    if (rank == 0) {
        //prevent it from buffering the printf statements
        setbuf(stdout, NULL);
        std::cout << "begin timing output" << endl;

        for(int i=0; i<sum; i++) {
            // if(all_catg_time_pts_buf[i] == MD_CHECKPOINT_DATABASE_START) {
         //        printf("%d %Lf %d ", all_catg_time_pts_buf[i], all_time_pts_buf[i], all_db_checkpt_types[db_checkpt_indx]);
         //        db_checkpt_indx += 1;
            // }
            // else {
                printf("%d %Lf ", all_catg_time_pts_buf[i], all_time_pts_buf[i]);
            // }
            // std::cout << all_catg_time_pts_buf[i] << " " << all_time_pts_buf[i] << " ";
            if (i%20 == 0 && i!=0) {
                std::cout << std::endl;
            }
        }
        std::cout << std::endl;

        // // free(all_clock_times);
        free(all_time_pts_buf);
        free(all_catg_time_pts_buf);

    }
}


template <class T>
static void make_single_val_data (T val, string &serial_str) {
    //extreme_debug_log << "about to make single val data" << endl;
    stringstream ss;
    boost::archive::text_oarchive oa(ss);
    oa << val;
    serial_str = ss.str();
}

template <class T1, class T2>
static void make_double_val_data (T1 val_0, T2 val_1, string &serial_str) {
    //extreme_debug_log << "about to make double val data" << endl;
    stringstream ss;
    boost::archive::text_oarchive oa(ss);
    oa << val_0;
    oa << val_1;
    serial_str = ss.str();
}


void gatherv_int(vector<int> values, uint32_t num_client_procs, int rank, int *each_proc_num_values,
            int *displacement_for_each_proc, int **all_values)
{   
    int num_values = values.size();
    //extreme_debug_log << "rank: " << rank << " num_values: " << num_values << endl;
    //extreme_debug_log << "rank: " << rank << " catg_of_time_pts.size(): " << catg_of_time_pts.size() << " time_pts.size(): " << time_pts.size() << endl;
    // int *values_ary = &values[0];

    // if(rank == 0) {
    //     for (int i = 0; i < 31; i++) {
    //         //extreme_debug_log << "i: " << i << " values[i]: " << values[i] << endl;
    //     }  
    // }

    MPI_Gather(&num_values, 1, MPI_INT, each_proc_num_values, 1, MPI_INT, 0, MPI_COMM_WORLD);

    int sum = 0;
    if (rank == 0 ) {
        for(int i=0; i<num_client_procs; i++) {
            displacement_for_each_proc[i] = sum;
            sum += each_proc_num_values[i];
            if(each_proc_num_values[i] != 0 && rank == 0) {
                //extreme_debug_log << "rank " << i << " has a string of length " << each_proc_num_values[i] << endl;
            }
        }
        *all_values = (int *) malloc(sum * sizeof(int));

        //extreme_debug_log << "sum: " << sum << endl;
    }

    // //extreme_debug_log << "num_bytes is "<< to_string(length_ser_c_str) + " and rank: " << to_string(rank) << endl;
    
    // //extreme_debug_log << "rank " << rank << " about to allgatherv" << endl;

    MPI_Gatherv(&values[0], num_values, MPI_INT,
           *all_values, each_proc_num_values, displacement_for_each_proc,
           MPI_INT, 0, MPI_COMM_WORLD);

    // if(rank == 0) {
    //     for (int i = 0; i < 62; i++) {
    //         //extreme_debug_log << "i: " << i << " all_values[i]: " << *all_values[i] << endl;
    //     }  
    // } 

}



//asssumes the rank is 0
template <class T>
void combine_ser( int num_client_procs, int *each_proc_ser_values_size, int *displacement_for_each_proc, 
        char *serialized_c_str_all_ser_values, const vector<T> &my_vals, vector<T> &all_vals)
{

// //extreme_debug_log << "rank " << rank << " done with allgatherv" << endl;
    // //extreme_debug_log << "entire set of attr vectors: " << serialized_c_str_all_ser_values << " and is of length: " << strlen(serialized_c_str_all_ser_values) << endl;

    for(int i = 0; i < num_client_procs; i++) {
        int offset = displacement_for_each_proc[i];
        int count = each_proc_ser_values_size[i];
        if(count > 0) {
            vector<T> rec_values;

            //0 rank does not need to deserialize its own attrs
            if(i != 0) {

                //extreme_debug_log << "rank " << i << " count: " << count << " offset: " << offset << endl;
                char serialzed_vals_for_one_proc[count];

                memcpy ( serialzed_vals_for_one_proc, serialized_c_str_all_ser_values + offset, count);

                //extreme_debug_log << "rank " << i << " serialized_c_str: " << (string)serialzed_vals_for_one_proc << endl;
                //extreme_debug_log <<    " serialized_c_str length: " << strlen(serialzed_vals_for_one_proc) << endl;
                //extreme_debug_log << " count: " << count << endl;
                stringstream ss1;
                ss1.write(serialzed_vals_for_one_proc, count);
                //extreme_debug_log << "about to deserialize ss: " << ss1.str() << endl;
                boost::archive::text_iarchive ia(ss1);
                ia >> rec_values;
            }
            else { //i == rank
                rec_values = my_vals;
            }
            all_vals.insert( all_vals.end(), rec_values.begin(), rec_values.end() );
        }
    }
    free(serialized_c_str_all_ser_values);
}

//todo - why not just send the attrs as a byte array???
template <class T>
void gatherv_ser_and_combine(const vector<T> &values, uint32_t num_client_procs, int rank, MPI_Comm comm,
    vector<T> &all_vals
    )
{
    
    //extreme_debug_log << "starting gatherv_ser_and_combine" << endl;   

    int each_proc_ser_values_size[num_client_procs];
    int displacement_for_each_proc[num_client_procs];
    char *serialized_c_str_all_ser_values;

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
    // //extreme_debug_log << "rank " << rank << " object_names ser string is of size " << length_ser_c_str << " serialized_str " << 
    //     serialized_str << endl;
    //extreme_debug_log << "rank " << rank << " ser string is of size " << length_ser_c_str << " str: " << serialized_c_str << endl; 

    // //extreme_debug_log << "rank " << rank << " about to allgather" << endl;

    //debug_log << "rank: " << rank << " about to gather attrs " << endl;
    MPI_Gather(&length_ser_c_str, 1, MPI_INT, each_proc_ser_values_size, 1, MPI_INT, 0, comm);

    int sum = 0;
    // int max_value = 0;
    if (rank == 0 ) {
        for(int i=0; i<num_client_procs; i++) {
            displacement_for_each_proc[i] = sum;
            sum += each_proc_ser_values_size[i];
            if(each_proc_ser_values_size[i] != 0 && rank == 0) {
                //extreme_debug_log << "rank " << i << " has a string of length " << each_proc_ser_values_size[i] << endl;
            }
            // if(displacement_for_each_proc[i] > max_value) {
            //     max_value = displacement_for_each_proc[i];
            // }
        }
        //extreme_debug_log << "sum: " << sum << endl;

        serialized_c_str_all_ser_values = (char *) malloc(sum);

    }

    //extreme_debug_log << "num_bytes is "<< to_string(length_ser_c_str) + " and rank: " << to_string(rank) << endl;
    
    //extreme_debug_log << "rank " << rank << " about to gatherv" << endl;

    MPI_Gatherv(serialized_c_str, length_ser_c_str, MPI_CHAR,
           serialized_c_str_all_ser_values, each_proc_ser_values_size, displacement_for_each_proc,
           MPI_CHAR, 0, comm);

    //extreme_debug_log << "just finished with gather v" << endl;

    free(serialized_c_str);

    if(rank == 0) {
        all_vals.reserve(3*num_client_procs);

        combine_ser( num_client_procs, each_proc_ser_values_size, displacement_for_each_proc, 
        serialized_c_str_all_ser_values, values, all_vals);

        //debug_log << "rank: " << rank << " done with gather attrs " << endl;
    }
}


// int metadata_collective_insert_var_attribute_by_dims_batch (const md_server &server
//                            // vector<uint64_t> &attribute_ids,
//                            ,const vector<md_catalog_var_attribute_entry> &new_attributes,
//                            MPI_Comm comm
//                            )
// {
//  int rank, num_client_procs, return_value;
//  vector<md_catalog_var_attribute_entry> all_attrs;

//  MPI_Comm_rank(comm, &rank);
//     MPI_Comm_size(comm, &num_client_procs);

//     gatherv_ser_and_combine(new_attributes, num_client_procs, rank, comm, all_attrs); 
 
//     // double start_time = get_time();
//     // cout << "rank: " << rank << " is inserting all_attrs of size " << all_attrs.size() << endl;

//     return_value = metadata_insert_var_attribute_by_dims_batch (server
//                            // vector<uint64_t> &attribute_ids,
//                            ,all_attrs
//                            );
//     // if(rank == 0) {
//     //   cout << "rank: " << rank << " time: " << get_time() - start_time << endl;
//     // }


//     return return_value;
// }


int metadata_collective_insert_var_attribute_by_dims_batch (const md_server &server
                           // vector<uint64_t> &attribute_ids,
                           ,const vector<md_catalog_var_attribute_entry> &new_attributes,
                           MPI_Comm comm
                           )
{

    // cout << "doing collective w/o blanks" << endl;
    add_timing_point(INSERT_VAR_ATTRS_COLLECTIVE_START); 

    add_timing_point(GATHER_ATTRS_START); 

    //extreme_debug_log << "am doing batch" << endl;
    // int rank, num_client_procs, return_value;
    int rank, num_client_procs;
    int return_value = RC_OK;
    vector<md_catalog_var_attribute_entry> all_attrs;

    MPI_Comm_rank(comm, &rank);
    MPI_Comm_size(comm, &num_client_procs);
    //debug_log << std::setprecision(16) << get_time() << " rank: " << rank << " start metadata_collective_insert_var_attribute_by_dims_batch for timestep " << new_attributes[0].timestep_id << endl;

 //    if(rank == 0) {
 //     for(int i = 0; i < 5; i++) {
    //      return_value = metadata_insert_var_attribute_by_dims_batch (server, all_attrs);
    //      if(return_value != RC_OK) {
    //          error_log << "error for warmup metadata_insert_var_attribute_by_dims_batch" << endl;
    //      }
    //     }
    // }

    gatherv_ser_and_combine(new_attributes, num_client_procs, rank, comm, all_attrs); 
 
    add_timing_point(GATHER_ATTRS_DONE); 

    // if(rank <= 10) {
    if(rank <= 100) {
    // if(rank <= 175) {
    // if(rank <= 250) {
    // if(rank <= 500) {
    // if(rank == 0) {
        // cout << "am running metadata_insert_var_attribute_by_dims_batch for rank " << rank << " timestep: " << 
        // new_attributes[0].timestep_id << endl;
        // double start_time = get_time();
        // //debug_log << "rank 0 is inserting attrs of size: " << all_attrs.size() << endl;
        //debug_log << "rank " << rank << " is inserting attrs of size: " << all_attrs.size() << endl;
        //debug_log << std::setprecision(16) << get_time() << " rank: " << rank << " metadata_insert_var_attribute_by_dims_batch for timestep " << new_attributes[0].timestep_id << endl;

        return_value = metadata_insert_var_attribute_by_dims_batch (server
                               // vector<uint64_t> &attribute_ids,
                               ,all_attrs
                               );
        // print_var_attribute_list(all_attrs.size(),all_attrs);
        // if(rank == 0) {
        // ////debug_log << "time to complete: " << get_time() - start_time << endl;
        // }
    // }
    // else if (rank < 10) {
 //        vector<md_catalog_timestep_attribute_entry> all_timestep_attrs_to_insert;

 //        return_value = metadata_insert_timestep_attribute_batch (server, all_timestep_attrs_to_insert);
 //        if (return_value != RC_OK) {
 //            error_log << "Error. Was unable to insert blank timestep attributes by batch. Proceeding" << endl;
 //            add_timing_point(ERR_INSERT_TIMESTEP_ATTR_BATCH);
 //        }
    //             //debug
 // //        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    // }
    // else {
        //40 -> 500s
        //30 -> 513
        //50 -> 516
        //75 - 520
        //10 - 530
        //35 - 515
 //        std::this_thread::sleep_for(std::chrono::milliseconds(35));
    }
    //debug_log << "rank: " << rank << " is before barrier for timestep " << new_attributes.at(0).timestep_id << endl;
    // MPI_Barrier(comm);
    //debug_log << "rank: " << rank << " is after barrier for timestep " << new_attributes.at(0).timestep_id << endl;
    add_timing_point(INSERT_VAR_ATTRS_COLLECTIVE_DONE); 

    //debug_log << std::setprecision(16) << get_time() << " rank: " << rank << " done metadata_collective_insert_var_attribute_by_dims_batch for timestep " << new_attributes[0].timestep_id << endl;

    return return_value;
}


template <class T>
void print_attr_data(T attr)
 {

    switch(attr.data_type) {
        case ATTR_DATA_TYPE_INT : {
            long long deserialized_test_int = stoll(attr.data);
            // uint64_t deserialized_test_int;

            // stringstream sso;
            // sso << attr.data;
            // boost::archive::text_iarchive ia(sso);
            // ia >> deserialized_test_int;

            cout << "data: int: " << deserialized_test_int << endl; 
            break;
        }
        case ATTR_DATA_TYPE_REAL : {
            // long double deserialized_test_real;
            long double deserialized_test_real = stold(attr.data);

            // stringstream sso;
            // sso << attr.data;
            // boost::archive::text_iarchive ia(sso);
            // ia >> deserialized_test_real;

            cout << "data: real: " << deserialized_test_real << endl; 
            break;
        }           
        case ATTR_DATA_TYPE_TEXT : {
                // string deserialized_test_string;
            cout << "data: text: " << attr.data << endl; 
            break;
        } 
        case ATTR_DATA_TYPE_BLOB : {
            vector<int> deserialized_vals(2);

            stringstream sso;
            sso << attr.data;
            boost::archive::text_iarchive ia(sso);
            ia >> deserialized_vals;
            cout << "data: blob: val1: " << deserialized_vals.at(0) << " val2: " << deserialized_vals.at(1) << endl; 
            break;
        }    
    }
        // //extreme_debug_log << "serialized var data: " << attr.data << endl;
 }

 void print_var_attr_data(md_catalog_var_attribute_entry attr) {
    print_attr_data(attr);
}
void print_timestep_attr_data(md_catalog_timestep_attribute_entry attr) {
    print_attr_data(attr);
}
void print_run_attr_data(md_catalog_run_attribute_entry attr) {
    print_attr_data(attr);
}
