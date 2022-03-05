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

#include <mpi.h>

#include <my_metadata_client.h>
#include <my_metadata_client_lua_functs.h>#include "dirman/DirMan.hh"
#include <sstream>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

#include <hdf5_helper_functions_write.hh>#include <testing_harness_debug_helper_functions.hh>
// #include <md_client_timing_constants.hh>
#include <client_timing_constants_write.hh>

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

static bool extreme_debug_logging = false;
static bool debug_logging = false;
static bool error_logging = true;
static bool testing_logging = false;
static bool zero_rank_logging = false;

debugLog error_log = debugLog(error_logging, zero_rank_logging);
debugLog debug_log = debugLog(debug_logging, zero_rank_logging);
debugLog extreme_debug_log = debugLog(extreme_debug_logging, zero_rank_logging);
debugLog testing_log = debugLog(testing_logging, zero_rank_logging);

static bool do_debug_testing = false;

static bool output_timing = true;
static bool insert_by_batch = true;

static bool do_write_data = true;
static bool hdf5_write_data = true;
static bool write_obj_data = false;
static bool write_obj_data_to_map = false;

static bool output_objector_params = false;
static bool output_obj_names = false;


std::vector<int> catg_of_time_pts;
std::vector<long double> time_pts;

int zero_time_sec;

void add_timing_point(int catg) {
    if (output_timing) {
        struct timeval now;
        gettimeofday(&now, NULL);
        time_pts.push_back( (now.tv_sec - zero_time_sec) + now.tv_usec / 1000000.0);
        catg_of_time_pts.push_back(catg);
    }
}


std::vector<int> all_num_objects_to_fetch;

void add_objector_point(int catg, int num_objects_to_fetch) {
    if (output_timing) {
        add_timing_point(catg);
        extreme_debug_log << "am pushing back " << num_objects_to_fetch << endl;
    }
    all_num_objects_to_fetch.push_back(num_objects_to_fetch);
}

vector<objector_params> all_objector_params;
vector<vector<string>> all_object_names;


std::map <string, vector<double>> data_outputs;


static int setup_dirman(const string &dirman_file_path, const string &dir_path, md_server &dirman, vector<faodel::NameAndNode> &server_procs, int rank, uint32_t num_servers, uint32_t num_clients);
// static void setup_server(md_server &server, int &server_indx, int rank, uint32_t num_servers, const vector<faodel::NameAndNode> &server_procs);
static void setup_server(int rank, uint32_t num_servers, const vector<faodel::NameAndNode> &server_nodes, md_server &server);
// void write_testing(int rank, faodel::DirectoryInfo dir, md_server server, uint64_t num_timesteps, uint32_t chunk_id);

int create_and_activate_run(md_server server, md_catalog_run_entry &run, const string &rank_to_dims_funct_name,
    const string &rank_to_dims_funct_path, const string &objector_funct_name, const string &objector_funct_path);

int create_and_activate_types(md_server server, uint64_t run_id, uint32_t num_types, 
    uint32_t num_run_timestep_types);

int create_vars(md_server server, int rank, uint32_t num_servers, uint32_t num_client_procs,
    md_catalog_var_entry &temp_var, uint32_t var_indx,
    uint64_t total_x_length, uint64_t total_y_length, uint64_t total_z_length,
    vector<md_catalog_var_entry> &all_vars_to_insert
    );

int write_data(int rank, const vector<double> &data_vctr, const md_catalog_run_entry &run, const string &objector_funct_name, 
    uint32_t timestep_num, int64_t timestep_file_id,
    const md_catalog_var_entry &var, int var_indx, const vector<md_dim_bounds> &proc_dims_vctr,
    uint32_t num_run_timestep_types,
    double &timestep_temp_min, double &timestep_temp_max);

int create_var_attrs(md_server server, int rank, const vector<md_dim_bounds> proc_dims_vctr,
    uint32_t timestep_num, uint32_t var_indx, uint32_t num_types, 
    vector<md_catalog_var_attribute_entry> &all_attrs_to_insert,
    double &timestep_temp_min, double &timestep_temp_max
    );

int insert_vars_and_attrs_batch(md_server server, int rank, const vector<md_catalog_var_entry> &all_vars_to_insert,
    const vector<md_catalog_var_attribute_entry> &all_attrs_to_insert, uint32_t num_servers );

int create_timestep_attrs(md_server server, int rank, double timestep_temp_min, double timestep_temp_max,
    uint32_t timestep_num, uint32_t num_types, uint32_t num_run_timestep_types, uint32_t num_client_procs,
    double *all_timestep_temp_mins_for_all_procs, double *all_timestep_temp_maxes_for_all_procs);

int activate_timestep_components(md_server server, uint32_t timestep_num);

int create_run_attrs(md_server server, int rank, double *all_timestep_temp_mins_for_all_procs, 
    double *all_timestep_temp_maxes_for_all_procs, uint64_t run_id, uint32_t num_timesteps, uint32_t num_types
    );

void output_obj_names_and_timing(int rank, uint32_t num_client_procs, const md_catalog_run_entry &temp_run,
    const md_catalog_var_entry &temp_var, uint32_t num_timesteps, uint32_t num_vars
    );

template <class T>
static void make_single_val_data (T val, string &serial_str);
template <class T1, class T2>
static void make_double_val_data (T1 val_0, T2 val_1, string &serial_str);

//note - just for debugging
int debug_testing(md_server server, int rank, int num_servers, vector<md_dim_bounds> dims);
void gather_and_print_output_params(const vector<objector_params> &object_names, int rank, uint32_t num_client_procs);
objector_params get_objector_params ( const md_catalog_run_entry &run, const md_catalog_var_entry &var, 
                            const vector<md_dim_bounds> &bounding_box);

void gather_and_print_object_names(const vector<vector<string>> &all_object_names, int rank, uint32_t num_client_procs, 
                            uint32_t num_timesteps, uint32_t num_vars);
void gatherv_int(vector<int> values, uint32_t num_client_procs, int rank, int *each_proc_num_values,
            int *displacement_for_each_proc, int **all_values);

static void get_obj_lengths(const md_catalog_var_entry &var, 
                    uint64_t &x_width, uint64_t &last_x_width, uint64_t ndx, uint64_t chunk_size) ;

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

    if (argc != 12) { 
        error_log << "Error. Program should be given 11 arguments. Dirman hexid, npx, npy, npz, nx, ny, nz, number of timesteps, estm num time_pts, num_servers, job_id" << endl;
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

    extreme_debug_log << "job_id: " << job_id << endl;


    //all 10 char plus null character
    uint32_t num_vars = 10;

    catg_of_time_pts.reserve(estm_num_time_pts);
    time_pts.reserve(estm_num_time_pts);
    all_object_names.reserve(100 * num_timesteps * num_vars);
    all_num_objects_to_fetch.reserve(num_timesteps * num_vars);
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

    rc = metadata_init_write();
    if (rc != RC_OK) {
        error_log << "Error initing the md_client. Exiting \n";
        return RC_ERR;
    }

    if (do_debug_testing) {
        rc = metadata_init_read();
        if (rc != RC_OK) {
            error_log << "Error initing the md_client. Exiting \n";
            return RC_ERR;
        }
    }

    add_timing_point(METADATA_CLIENT_INIT_DONE);

    if(rank == 0) {
        extreme_debug_log << "starting" << endl;
    }
    string dir_path="/metadata/testing";
    md_server dirman;

    md_server server;
    // int server_indx;
    // int my_num_clients_per_server;
    // int remainder;

    vector<md_dim_bounds> proc_dims_vctr(3);

    int num_run_timestep_types = 2;
    uint32_t num_types = 10;

    extreme_debug_log << "num_type: " << num_types << " num_run_timestep_types: " << num_run_timestep_types << endl;

    uint32_t x_length_per_proc = total_x_length / num_x_procs; //todo - deal with edge case?
    uint32_t y_length_per_proc = total_y_length / num_y_procs;
    uint32_t z_length_per_proc = total_z_length / num_z_procs;

    uint64_t chunk_vol = x_length_per_proc * y_length_per_proc * z_length_per_proc;

    struct md_catalog_var_entry temp_var;

    uint32_t num_dims;
    uint32_t var_num;
    uint32_t var_version;

    struct md_catalog_run_entry temp_run;
    struct md_catalog_timestep_entry temp_timestep;


    uint32_t x_pos, y_pos, z_pos;
    uint32_t x_offset, y_offset, z_offset;

    double all_timestep_temp_maxes_for_all_procs[num_timesteps];
    double all_timestep_temp_mins_for_all_procs[num_timesteps];

    vector<double> data_vctr;


    // double timestep_temp_maxes[num_timesteps][num_vars] = {0};
    // double timestep_temp_mins[num_timesteps][num_vars] = {0};

    // double timestep_temp_maxes[num_timesteps]= {0};
    // double timestep_temp_mins[num_timesteps] = {0};

    vector<faodel::NameAndNode> server_procs;
    server_procs.resize(num_servers);

    // srand(rank+1);
    extreme_debug_log << "rank: " << rank << " first rgn: " << rand() << endl;

    //just for testing-------------------------------------------

    // long double *all_clock_times;

    // double *total_times; 

    all_objector_params.reserve(num_vars * num_timesteps);
    //--------------------------------------------------------------

    // my_num_clients_per_server = num_client_procs / num_servers; //each has this many at a min
    // remainder = num_client_procs - (num_servers * my_num_clients_per_server);

    // if(server_indx < remainder) {
    //     my_num_clients_per_server +=1;
    // }


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
    proc_dims_vctr [0].min = x_offset;
    proc_dims_vctr [0].max = x_offset + x_length_per_proc-1;
    proc_dims_vctr [1].min = y_offset;
    proc_dims_vctr [1].max = y_offset + y_length_per_proc -1;
    proc_dims_vctr [2].min = z_offset;
    proc_dims_vctr [2].max = z_offset + z_length_per_proc -1;
    // vector<md_dim_bounds> proc_dims_vctr = std::vector<md_dim_bounds>(proc_dims_vctr, proc_dims_vctr + num_dims );

    //length_of_chunk = (dims [0].max - dims [0].min + 1)  * (dims [1].max - dims [1].min + 1)  * (dims [2].max - dims [2].min + 1);
    
    for (int j=0; j<num_dims; j++) {
        extreme_debug_log << "dims [" << to_string(j) << "] min is: " << to_string(proc_dims_vctr [j].min) << endl;
        extreme_debug_log << "dims [" << to_string(j) << "] max is: " << to_string(proc_dims_vctr [j].max) << endl;
    }   


    add_timing_point(INIT_VARS_DONE);

    rc = setup_dirman(argv[1], dir_path, dirman, server_procs, rank, num_servers, num_client_procs);
    if (rc != RC_OK) {
        add_timing_point(ERR_DIRMAN);
        error_log << "Error. Was unable to setup the dirman. Exiting" << endl;
        return RC_ERR;
    }

    add_timing_point(DIRMAN_SETUP_DONE);

    setup_server(rank, num_servers, server_procs, server);

    add_timing_point(SERVER_SETUP_DONE_INIT_DONE);


    MPI_Barrier(MPI_COMM_WORLD);

    //WRITE PROCESS
    //only want one copy of each var and type in each server's db
    add_timing_point(WRITING_START);


    if(write_data) {
        add_timing_point(CREATE_DATA_START);

        data_vctr = vector<double>(chunk_vol);
        /*
         * Initialize data buffer 
         * Just use the same data for each var and timestep
         */

        generate_data_for_proc(y_length_per_proc, z_length_per_proc, rank, proc_dims_vctr, 
            data_vctr, x_length_per_proc, y_length_per_proc, z_length_per_proc, chunk_vol);


        add_timing_point(CREATE_DATA_DONE);
    }

 

    // add_timing_point(DIMS_INIT_DONE);

    temp_run.job_id = job_id;
    temp_run.name = "XGC";
    temp_run.txn_id = 0;
    temp_run.npx = num_x_procs;
    temp_run.npy = num_y_procs;
    temp_run.npz = num_z_procs;


    // rc = stringify_function("PATH_TO_EMPRESS/lib_source/lua/lua_function.lua", "rank_to_bounding_box",  temp_run.rank_to_dims_funct);
    // if (rc != RC_OK) {
    //     error_log << "Error. Was unable to stringify the rank to bounding box function. Exitting \n";
    //     goto cleanup;
    // }

    // rc = stringify_function("PATH_TO_EMPRESS/lib_source/lua/metadata_functs.lua", "boundingBoxToObjectNamesAndCounts",  temp_run.objector_funct);
    // if (rc != RC_OK) {
    //     error_log << "Error. Was unable to stringify the boundingBoxToObjectNamesAndCounts function. Exitting \n";
    //     goto cleanup;
    // }

    string rank_to_dims_funct_name = "rank_to_bounding_box";
    string rank_to_dims_funct_path = "PATH_TO_EMPRESS/lib_source/lua/lua_function.lua";
    string objector_funct_name = "boundingBoxToObjectNamesAndCounts";
    string objector_funct_path = "PATH_TO_EMPRESS/lib_source/lua/metadata_functs.lua";

    // add_timing_point(RUN_INIT_DONE);

    uint64_t run_id = 1;

    //reminder - needs to be done for each CLIENT, not server

    if(write_obj_data) {
        add_timing_point(OBJECTOR_LOAD_START);

        rc = register_objector_funct_write (objector_funct_name, objector_funct_path, job_id);
        if (rc != RC_OK) {
            error_log << "error in registering the objector function in write for " << server.URL << endl;
        }
        else {
            extreme_debug_log << "just registered for server " << server.URL << endl;        
        }

        add_timing_point(OBJECTOR_LOAD_DONE);
    }


    if (rank < num_servers) {
        rc = create_and_activate_run(server, temp_run, rank_to_dims_funct_name, rank_to_dims_funct_path, 
                objector_funct_name, objector_funct_path);
        if (rc != RC_OK) {
            goto cleanup;
        }

        // if(rank == 0 && hdf5_write_data) {
        //     hdf5_create_run(temp_run.name, job_id);
        // }
    }


    temp_run.run_id = run_id; //todo - do we want to make them catalog to get this? or change so registering doesn't require a name

    //reminder - unless you adjust create_type to ask for a type id, if you want the types in the given order (so you know how)
    //type_id corresponds to name/version, you need to insert them in order

    if (rank < num_servers) {
        rc = create_and_activate_types(server, run_id, num_types, num_run_timestep_types);
        if (rc != RC_OK) {
            goto cleanup;
        }
    }

    temp_timestep.run_id = run_id; //todo - will have to change if we want more than 1 run



    // add_timing_point(CREATE_TIMESTEPS_START);
    add_timing_point(CREATE_TIMESTEPS_START);



    for(uint32_t timestep_num=0; timestep_num < num_timesteps; timestep_num++) {
        int64_t timestep_file_id = -1;
        MPI_Barrier(MPI_COMM_WORLD); //want to measure last-first for writing each timestep
        add_timing_point(CREATE_NEW_TIMESTEP_START);

        double timestep_temp_max;
        double timestep_temp_min;

        extreme_debug_log << "rank: " << rank << " timestep_num: " << timestep_num << " (rank / num_servers): " << (rank / num_servers) << endl;

        if( (timestep_num % num_client_procs) == (rank / num_servers)  ) {
            extreme_debug_log << "about to create timestep for timestep_num: " << timestep_num << endl;
            
            temp_timestep.timestep_id = timestep_num;      
            temp_timestep.path = temp_run.name + "/" + to_string(temp_timestep.timestep_id); //todo - do timestep need to use sprintf for this?
            temp_timestep.txn_id = timestep_num; 
            
            // extreme_debug_log << " timestep: " << temp_timestep.timestep_id << 
            //     " rank: " << rank << " num servers: " << num_servers << " num_timesteps: " << num_timesteps << endl;
            // extreme_debug_log << "server hex id: " << server.URL << endl;

            rc = metadata_create_timestep (server, temp_timestep.timestep_id, temp_timestep.run_id, temp_timestep.path, temp_timestep.txn_id);
            if (rc != RC_OK) {
                error_log << "Error. Was unable to create the " << temp_timestep.timestep_id << "th timestep. Exiting" << endl;
                add_timing_point(ERR_CREATE_TIMESTEP);
                goto cleanup;
            }
        }
        if(hdf5_write_data) {
            if(rank == 0) { //have one proc individually create all of the files so they are not all collectives
                hdf5_create_timestep_and_vars(temp_run.name, job_id, timestep_num, num_vars, 
                    total_x_length, total_y_length, total_z_length, proc_dims_vctr);
            }
            hdf5_open_timestep_file_collectively_for_write(temp_run.name, job_id, timestep_num, timestep_file_id);
        }
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        temp_var.run_id = run_id;
        temp_var.timestep_id = timestep_num;
        temp_var.data_size = 8; //double - 64 bit floating point
        temp_var.txn_id = timestep_num; 

        vector<md_catalog_var_attribute_entry> all_attrs_to_insert;
        all_attrs_to_insert.reserve(3 * num_vars);

        vector<md_catalog_var_entry> all_vars_to_insert;
        all_vars_to_insert.reserve(num_vars);
        
        add_timing_point(CREATE_VARS_AND_ATTRS_FOR_NEW_TIMESTEP_START); 

        for(uint32_t var_indx=0; var_indx < num_vars; var_indx++) {

            rc = create_vars(server, rank, num_servers, num_client_procs, temp_var, var_indx,
                    total_x_length, total_y_length, total_z_length, all_vars_to_insert);
            if (rc != RC_OK) {
                goto cleanup;
            }

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

            if (do_write_data) { 

                rc = write_data(rank, data_vctr, temp_run, objector_funct_name, timestep_num, timestep_file_id, temp_var, var_indx, proc_dims_vctr,
                        num_run_timestep_types, timestep_temp_min, timestep_temp_max);
                if (rc != RC_OK) {
                    goto cleanup;
                }
                extreme_debug_log << "finished write_data" << endl;                
            } //end if write data 
            else if (output_objector_params) {
                all_objector_params.push_back( get_objector_params(temp_run, temp_var, proc_dims_vctr) );
                // add_objector_point(BOUNDING_BOX_TO_OBJ_NAMES, 1);
                add_timing_point(BOUNDING_BOX_TO_OBJ_NAMES);
            }
            else if (output_obj_names) {
                std::vector<string> obj_names;

                add_timing_point(BOUNDING_BOX_TO_OBJ_NAMES_START);

                rc = boundingBoxToObjNames(temp_run, temp_var, proc_dims_vctr, obj_names);
                if (rc != RC_OK) {
                    error_log << "Error doing the bounding box to obj names, returning \n";
                    return RC_ERR;
                }
                add_timing_point(BOUNDING_BOX_TO_OBJ_NAMES_DONE);

                all_object_names.push_back(obj_names);
                add_objector_point(BOUNDING_BOX_TO_OBJ_NAMES, obj_names.size());
                // extreme_debug_log << "rank " << rank << " is pushing back obj_names.size(): " << obj_names.size() << endl;
            }

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////            
            srand(rank*10000+1+timestep_num);

            rc = create_var_attrs(server, rank, proc_dims_vctr, timestep_num,
                    var_indx, num_types, all_attrs_to_insert, timestep_temp_min, timestep_temp_max); //if not okay, just proceed

        } //var loop done 

        if(insert_by_batch) {
            rc = insert_vars_and_attrs_batch(server, rank, all_vars_to_insert, 
                    all_attrs_to_insert, num_servers); //if not okay, just proceed
        }
        add_timing_point(CREATE_VARS_AND_ATTRS_FOR_NEW_TIMESTEP_DONE); 

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
        rc = create_timestep_attrs(server, rank, timestep_temp_min, timestep_temp_max, timestep_num,
                num_types, num_run_timestep_types, num_client_procs, all_timestep_temp_mins_for_all_procs,
                all_timestep_temp_maxes_for_all_procs); //if not okay, just proceed

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //reminder - need to barrier so that all instances have been submitted before activating

        if(hdf5_write_data) {
            hdf5_close_timestep_file(timestep_file_id);
        }

        add_timing_point(CREATE_TIMESTEP_DONE); 

        MPI_Barrier(MPI_COMM_WORLD);

        if(rank < num_servers) {
           rc = activate_timestep_components(server, timestep_num);
            if (rc != RC_OK) {
                goto cleanup;
            }          
        }
        add_timing_point(CREATE_AND_ACTIVATE_TIMESTEP_DONE); 
    }
    add_timing_point(CREATE_ALL_TIMESTEPS_DONE); 

    for (int i = 0; i<num_timesteps; i++) {
            debug_log << "rank: " << rank << " timestep: " << i << " max: " << all_timestep_temp_maxes_for_all_procs[i] << endl;
            debug_log << "rank: " << rank << " timestep: " << i << " min: " << all_timestep_temp_mins_for_all_procs[i] << endl;
    }  
    
    if (rank == 0 && num_run_timestep_types > 0) {
        rc = create_run_attrs(server, rank, all_timestep_temp_mins_for_all_procs, all_timestep_temp_maxes_for_all_procs,
                run_id, num_timesteps, num_types);

        if(rc != RC_OK) {
            goto cleanup;
        }
    }

    add_timing_point(WRITING_DONE);

//-------------------------------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------------------------------    

    if(do_debug_testing) {
        MPI_Barrier(MPI_COMM_WORLD);
        // rc = debug_testing(server);
        rc = debug_testing( server, rank, num_servers, proc_dims_vctr);

        if (rc != RC_OK) {
            error_log << "Error with debug testing \n";
        }             
    }


//-------------------------------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------------------------------        
cleanup:
    MPI_Barrier (MPI_COMM_WORLD);
    add_timing_point(WRITING_DONE_FOR_ALL_PROCS_START_CLEANUP);
        
    metadata_finalize_client();

    extreme_debug_log << "num_servers: " << num_servers << endl;

    if( rank < num_servers) {
        metadata_finalize_server(server);
        add_timing_point(SERVER_SHUTDOWN_DONE);
        debug_log << "just finished finalizing" << endl;
        // extreme_debug_log << "just finished finalizing server" << endl;
        if(rank == 0) {
            metadata_finalize_server(dirman);
            add_timing_point(DIRMAN_SHUTDOWN_DONE);
        }
    }

    output_obj_names_and_timing(rank, num_client_procs, temp_run, temp_var, num_timesteps, num_vars);

    
    // free(total_times)
    MPI_Barrier (MPI_COMM_WORLD);
    MPI_Finalize();
    faodel::bootstrap::Finish();
    debug_log << "got to cleanup7" << endl;

    return rc;

}



int retrieveObjNamesAndDataForAttrCatalog(const std::map <string, vector<double>> &data_outputs, const md_catalog_run_entry &run, 
                                         const vector<md_catalog_var_entry> &var_entries,
                                         uint64_t run_id, uint64_t timestep_id,
                                         uint64_t txn_id,
                                         const vector<md_catalog_var_attribute_entry> &attr_entries );

// int retrieveObjNamesAndDataForAttrCatalog(const std::map <string, vector<double>> &data_outputs, const md_catalog_run_entry &run, 
//                                          const vector<md_catalog_var_entry> &var_entries,
//                                          uint64_t run_id, uint64_t timestep_id,
//                                          uint64_t txn_id,
//                                          const vector<md_catalog_var_attribute_entry> &attr_entries,
//                                          vector<string> &obj_names, vector<uint64_t> &offsets_and_counts
//                                          );

int debug_testing(md_server server, int rank, int num_servers, vector<md_dim_bounds> dims) {
    int rc;
    uint32_t count;
    vector<md_catalog_var_attribute_entry> var_attr_entries;
    vector<md_catalog_run_attribute_entry> run_attr_entries;
    vector<md_catalog_timestep_attribute_entry> timestep_attr_entries;

    vector<md_catalog_run_entry> run_entries;
    vector<md_catalog_timestep_entry> timestep_entries;
    vector<md_catalog_var_entry> var_entries;
    vector<md_catalog_type_entry> type_entries;

    uint64_t txn_id = -1;

    rc = metadata_catalog_run (server, txn_id, count, run_entries);
    if (rc != RC_OK) {
        error_log << "Error cataloging the post deletion set of run entries. Proceeding" << endl;
    }
    if (rank < num_servers) {
        testing_log << "run catalog for txn_id " << txn_id << ": \n";
        print_run_catalog (count, run_entries);
    }

    if (rank < num_servers) {
        for (md_catalog_run_entry run : run_entries) {
            rc = metadata_catalog_all_run_attributes ( server, run.run_id, txn_id, count, run_attr_entries );
            if (rc == RC_OK) {
                testing_log << "attributes associated with run_id " << run.run_id;
                testing_log << "\n";

                print_run_attribute_list (count, run_attr_entries);
                print_run_attr_data(count, run_attr_entries);
            }
            else {
                error_log << "Error getting the matching attribute list. Proceeding" << endl;
            }
            testing_log << "\n";
        }
    }

    for (md_catalog_run_entry run : run_entries) {
        rc = metadata_catalog_type (server, run.run_id, txn_id, count, type_entries);
        if (rc == RC_OK) {
            if (rank < num_servers) {
                testing_log << "new type catalog for run_id " << run.run_id << ": \n";
                print_type_catalog (count, type_entries);
            }
        }
        else {
            error_log << "Error cataloging the new set of type entries. Proceeding" << endl;
        }
    }

    for (md_catalog_run_entry run : run_entries) {

        rc = metadata_catalog_timestep (server, run.run_id, txn_id, count, timestep_entries);
        if (rc != RC_OK) {
            error_log << "Error cataloging the post deletion set of timestep entries. Proceeding" << endl;
        }
        if (rank < num_servers) {
            testing_log << "timestep catalog for run_id " << run.run_id << ": \n";
            print_timestep_catalog (count, timestep_entries);
        }

        for (md_catalog_timestep_entry timestep : timestep_entries) {

            if (rank < num_servers) {
                rc = metadata_catalog_all_timestep_attributes(server, timestep.run_id, timestep.timestep_id, txn_id, count, timestep_attr_entries);
                if (rc == RC_OK) {
                    testing_log << "attributes associated with run id: " << timestep.run_id << " and timestep: " << timestep.timestep_id << "\n";

                    print_timestep_attribute_list (count, timestep_attr_entries);
                    print_timestep_attr_data(count, timestep_attr_entries);
                }
                else {
                    error_log << "Error getting the matching timestep attribute list. Proceeding" << endl;
                }
            }


            rc = metadata_catalog_var (server, timestep.run_id, timestep.timestep_id, run.txn_id, count, var_entries);

            if (rc == RC_OK) {
                if (rank < num_servers) {
                    testing_log << "vars associated with run_id " << timestep.run_id << "and timestep " << timestep.timestep_id <<  ": \n";
                    print_var_catalog (count, var_entries);
                }
            }
            else {
                error_log << "Error cataloging the new set of var entries. Proceeding" << endl;
            }

            // rc = metadata_catalog_all_var_attributes (server, timestep.run_id, timestep.timestep_id, txn_id, count, var_attr_entries);
            // if (rc == RC_OK) {
            //     testing_log << "using var attrs funct: attributes associated with run_id " << timestep.run_id << 
            //         " and timestep_id " << timestep.timestep_id ;
            //     testing_log << "\n";

            //     print_var_attribute_list (count, var_attr_entries);
            //     print_var_attr_data(count, var_attr_entries);
            // }
            // else {
            //     error_log << "Error getting the matching attribute list. Proceeding" << endl;
            // }
            rc = metadata_catalog_all_var_attributes_with_dims(server, timestep.run_id, timestep.timestep_id, 
                timestep.txn_id, dims.size(), dims, count, var_attr_entries);

            if (rc == RC_OK) {
                testing_log << "using var attrs dims funct: attributes associated with run_id " << timestep.run_id << 
                    ", timestep_id " << timestep.timestep_id << " and txn_id: " << timestep.txn_id << " overlapping with vect_dims";
                for(int j=0; j< dims.size(); j++) {
                    testing_log << " d" << j << "_min: " << dims [j].min;
                    testing_log << " d" << j << "_max: " << dims [j].max;               
                }
                testing_log << " \n";
                print_var_attribute_list (count, var_attr_entries);

                print_var_attr_data(count, var_attr_entries);
            }
            else {
                error_log << "Error getting the matching attribute list. Proceeding" << endl;
            }
            testing_log << "\n";

            // vector<string> obj_names;
            // vector<uint64_t> offsets_and_counts;            

            // rc = retrieveObjNamesAndDataForAttrCatalog(data_outputs, run, var_entries, run.run_id, timestep.timestep_id, txn_id, var_attr_entries, obj_names, offsets_and_counts );
            if(do_write_data) {
                rc = retrieveObjNamesAndDataForAttrCatalog(data_outputs, run, var_entries, run.run_id, timestep.timestep_id, txn_id, var_attr_entries );
                if (rc != RC_OK) {
                    error_log << "error. could not perform retrieveObjNamesAndDataForAttrCatalog \n";
                }
            }
        } //for each timestep
    } //for each run
    return rc;
}

int create_and_activate_run(md_server server, md_catalog_run_entry &run, const string &rank_to_dims_funct_name,
    const string &rank_to_dims_funct_path, const string &objector_funct_name, const string &objector_funct_path)
{
    int rc = RC_OK;

    add_timing_point(CREATE_NEW_RUN_START);

    rc = metadata_create_run  ( server, run.run_id, run.job_id, run.name, run.txn_id, run.npx, run.npy, run.npz,
                        rank_to_dims_funct_name, rank_to_dims_funct_path, objector_funct_name, objector_funct_path);
    if (rc != RC_OK) {
        error_log << "Error. Was unable to create the first run. Exitting \n";
        add_timing_point(ERR_CREATE_RUN);
        return rc;
    }

    rc = metadata_activate_run  ( server, run.txn_id);
    if (rc != RC_OK) {
        error_log << "Error. Was unable to activate the run. Exitting \n";
        add_timing_point(ERR_ACTIVATE_RUN);
        return rc;
    }

    add_timing_point(CREATE_NEW_RUN_DONE);

    return rc;
}

int create_and_activate_types(md_server server, uint64_t run_id, uint32_t num_types, 
    uint32_t num_run_timestep_types) 
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

        extreme_debug_log << "type_indx: " << type_indx << endl;

        temp_type.run_id = run_id;
        if(is_run_type) {
            temp_type.name = run_timestep_type_names[type_indx_adj];
        }
        else {
            temp_type.name = type_names[type_indx_adj];
        }
        temp_type.version = 0; 
        temp_type.txn_id = run_id;
        // add_timing_point(TYPE_INIT_DONE);

        uint64_t temp_type_id;

        if(insert_by_batch) {
            all_types_to_insert.push_back(temp_type);
        }
        else {
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
        rc = metadata_create_type_batch (server, first_type_id, all_types_to_insert);
        if (rc != RC_OK) {
            add_timing_point(ERR_CREATE_TYPE_BATCH);
            error_log << "Error. Was unable to insert the types by batch. Proceeding" << endl;
            return rc;
        }
    }

    rc = metadata_activate_type  ( server, run_id);
    if (rc != RC_OK) {
        error_log << "Error. Was unable to activate the types. Exitting \n";
        add_timing_point(ERR_ACTIVATE_TYPE);
        return rc;
    }

    add_timing_point(CREATE_TYPES_DONE);

    return rc;
}

int create_vars(md_server server, int rank, uint32_t num_servers, uint32_t num_client_procs,
    md_catalog_var_entry &temp_var, uint32_t var_indx,
    uint64_t total_x_length, uint64_t total_y_length, uint64_t total_z_length,
    vector<md_catalog_var_entry> &all_vars_to_insert
    )
{
    int rc = RC_OK;

    char var_names[10][11] = {"temperat", "temperat", "pressure", "pressure", "velocity_x", "velocity_y", "velocity_z", "magn_field", "energy_var", "density_vr"};  
 
    uint32_t version1 = 1;
    uint32_t version2 = 2;

    vector<md_dim_bounds> var_dims(3);

    add_timing_point(CREATE_VAR_AND_ATTRS_FOR_NEW_VAR_START);

    extreme_debug_log << "var_indx: " << var_indx << " rank: " << rank << endl;

    extreme_debug_log << " var_indx: " << var_indx << 
        " rank: " << rank << " num servers: " << num_servers << endl;

    // uint64_t var_id = my_num_clients_per_server * var_indx + (rank / num_servers);

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
        debug_log << "rank just pushed back var with id: " << temp_var.var_id << endl;
        all_vars_to_insert.push_back(temp_var);

    }
    else if (!insert_by_batch) {
        if ( (var_indx % num_client_procs) == (rank / num_servers) )  { 
            extreme_debug_log << "var_indx: " << var_indx << endl;
       
            rc = metadata_create_var (server, temp_var_id, temp_var);
            if (rc != RC_OK) {
                error_log << "Error. Was unable to create the temp var. Exiting" << endl;
                error_log << "Failed var_id " << to_string(temp_var_id) << endl;
                add_timing_point(ERR_CREATE_VAR);
                return rc;
            }

            extreme_debug_log << "rank " << rank << " and server hexid: " << server.URL <<
                     " Finished creating var. new var_id " << var_indx << endl;
        }
        else {
            extreme_debug_log << "var id is greater than num vars so am skipping create \n";
        }
    }

    return rc;
}

int write_data(int rank, const vector<double> &data_vctr, const md_catalog_run_entry &run, const string &objector_funct_name, 
    uint32_t timestep_num, int64_t timestep_file_id,
    const md_catalog_var_entry &var, int var_indx, const vector<md_dim_bounds> &proc_dims_vctr,
    uint32_t num_run_timestep_types,
    double &timestep_temp_min, double &timestep_temp_max)
{
    int rc = RC_OK;

    if(write_obj_data_to_map) {
        add_timing_point(CREATE_OBJS_AND_STORE_IN_MAP_START);

        rc = write_output(data_outputs, rank, run, objector_funct_name, var, proc_dims_vctr, data_vctr);
        if (rc != RC_OK) {
            error_log << "Error. Was unable write the output for rank " << rank << " \n";
            add_timing_point(ERR_CREATE_OBJ_NAMES_AND_DATA); 
            return rc;
        }
        add_timing_point(CREATE_OBJS_AND_STORE_IN_MAP_DONE);
    }
    else if (hdf5_write_data) {

        hdf5_write_chunk_data(timestep_file_id, var, proc_dims_vctr, rank, data_vctr);

    }
    if(var_indx == 0 && num_run_timestep_types > 0) {
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
        timestep_temp_min -= 100*timestep_num;
        timestep_temp_max += 100*timestep_num;
        add_timing_point(CHUNK_MAX_MIN_FIND_DONE);

    } //end if(var_indx == 0 && num_run_timestep_types > 0)

    return rc;
}


int create_var_attrs(md_server server, int rank, const vector<md_dim_bounds> proc_dims_vctr,
    uint32_t timestep_num, uint32_t var_indx, uint32_t num_types, 
    vector<md_catalog_var_attribute_entry> &all_attrs_to_insert,
    double &timestep_temp_min, double &timestep_temp_max
    )
{
    int rc = RC_OK;

    //given in %
    float type_freqs[] = {25, 5, .1, 100, 100, 20, 2.5, .5, 1, 10};
    // char type_types[10][3] = {"b", "b", "b", "d", "d", "2p","2p","2p","2d","2d"};
    char type_types[10][3] = {"b", "b", "b", "d", "d", "s","s","s","2d","2d"};

    add_timing_point(CREATE_VAR_ATTRS_FOR_NEW_VAR_START); 

    for(uint32_t type_indx=0; type_indx<num_types; type_indx++) {
        // std::chrono::high_resolution_clock::time_point create_attr_start_time = chrono::high_resolution_clock::now();

        float freq = type_freqs[type_indx];
        uint32_t odds = 100 / freq;
        uint32_t val = rand() % odds; 
        extreme_debug_log << "rank: " << to_string(rank) << " and val: " << to_string(val) << " and odds: " << to_string(odds) << endl;
        if(val == 0) { //makes sure we have the desired frequency for each type
            add_timing_point(CREATE_NEW_VAR_ATTR_START);

            uint64_t attr_id;
            md_catalog_var_attribute_entry temp_attr;
            temp_attr.timestep_id = timestep_num;
            temp_attr.type_id = type_indx+1; //note - if we change the id system this will change
            temp_attr.var_id = var_indx; //todo - are we okay with this?
            temp_attr.txn_id = timestep_num;
            temp_attr.num_dims = 3;
            // temp_attr.dims = std::vector<md_dim_bounds>(proc_dims_vctr, proc_dims_vctr + num_dims ); 
            temp_attr.dims = proc_dims_vctr; //todo - you wont want this to be the entire chunk for all attrs

            if(strcmp(type_types[type_indx], "b") == 0) {
                bool flag = rank % 2;
                make_single_val_data(flag, temp_attr.data);
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
                make_single_val_data(val, temp_attr.data);
                temp_attr.data_type = ATTR_DATA_TYPE_REAL;  
                //if write data the chunk min and max for var 0 will be based on the actual data values (this is done above)
                if(!do_write_data && var_indx == 0 )  { 
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
            add_timing_point(VAR_ATTR_INIT_DONE);

            if(insert_by_batch) {
                all_attrs_to_insert.push_back(temp_attr);
                // if(all_attrs_to_insert.size() < 5) {
                //     extreme_debug_log << "for rank: " << rank << " the " << all_attrs_to_insert.size() << "th attr has val: " << temp_attr.data << endl;
                // }
            }
            else {
                // rc = metadata_insert_var_attribute_by_dims (server, attr_id, temp_attr);
                rc = metadata_insert_var_attribute_by_dims (server, attr_id, temp_attr);
                if (rc != RC_OK) {
                    error_log << "Error. Was unable to insert the attribute for type indx" << type_indx << "var indx " << var_indx <<". Proceeding" << endl;
                    add_timing_point(ERR_INSERT_VAR_ATTR);
                }
                extreme_debug_log << "rank: " << rank << "attr.data: " << temp_attr.data << endl;
            }

           add_timing_point(CREATE_NEW_VAR_ATTR_DONE);
        }//val = 0 done
    } //type loop done
    add_timing_point(CREATE_VAR_ATTRS_FOR_NEW_VAR_DONE);

    return rc;
}

int insert_vars_and_attrs_batch(md_server server, int rank, const vector<md_catalog_var_entry> &all_vars_to_insert,
    const vector<md_catalog_var_attribute_entry> &all_attrs_to_insert, uint32_t num_servers )
{
    int rc = RC_OK;

    if(rank < num_servers) {
        debug_log << "about to insert all vars of size: " << all_vars_to_insert.size() << endl;
        rc = metadata_create_var_batch (server, all_vars_to_insert);
        if (rc != RC_OK) {
            error_log << "Error. Was unable to insert the batch vars. Proceeding" << endl;
            add_timing_point(ERR_CREATE_VAR_BATCH);
        }
    }

    debug_log << "about to insert all attrs of size: " << all_attrs_to_insert.size() << endl;
    rc = metadata_insert_var_attribute_by_dims_batch (server, all_attrs_to_insert);
    if (rc != RC_OK) {
        error_log << "Error. Was unable to insert the batch attributes. Proceeding" << endl;
        add_timing_point(ERR_INSERT_VAR_ATTR_BATCH);
    }

    return rc;
}

int create_timestep_attrs(md_server server, int rank, double timestep_temp_min, double timestep_temp_max,
    uint32_t timestep_num,
    uint32_t num_types, uint32_t num_run_timestep_types, uint32_t num_client_procs,
    double *all_timestep_temp_mins_for_all_procs, double *all_timestep_temp_maxes_for_all_procs)
{
    int rc = RC_OK;
    add_timing_point(CREATE_TIMESTEP_ATTRS_START);

    if(rank == 0 && num_run_timestep_types > 0) {

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

        uint64_t temp_max_id;
        md_catalog_timestep_attribute_entry temp_max;
        temp_max.timestep_id = timestep_num;
        temp_max.type_id = num_types+1;
        temp_max.txn_id = timestep_num;
        make_single_val_data(timestep_var_max, temp_max.data);
        temp_max.data_type = ATTR_DATA_TYPE_REAL;

        uint64_t temp_min_id;
        md_catalog_timestep_attribute_entry temp_min;
        temp_min.timestep_id = timestep_num;
        temp_min.type_id = num_types+2;
        temp_min.txn_id = timestep_num;
        make_single_val_data(timestep_var_min, temp_min.data);
        temp_min.data_type = ATTR_DATA_TYPE_REAL;

        if(insert_by_batch) {
            vector<md_catalog_timestep_attribute_entry> all_timestep_attrs_to_insert = {temp_max, temp_min};

            rc = metadata_insert_timestep_attribute_batch (server, all_timestep_attrs_to_insert);
            if (rc != RC_OK) {
                error_log << "Error. Was unable to insert timestep attributes by batch. Proceeding" << endl;
                add_timing_point(ERR_INSERT_TIMESTEP_ATTR_BATCH);
            }
        }
        else {
            rc = metadata_insert_timestep_attribute (server, temp_max_id, temp_max);
            if (rc != RC_OK) {
                error_log << "Error. Was unable to insert the new timestep attribute. Proceeding" << endl;
                add_timing_point(ERR_INSERT_TIMESTEP_ATTR);
            }
            debug_log << "New timestep attribute id is " << to_string(temp_max_id) << endl;

            rc = metadata_insert_timestep_attribute (server, temp_min_id, temp_min);
            if (rc != RC_OK) {
                error_log << "Error. Was unable to insert the new timestep attribute. Proceeding" << endl;
                add_timing_point(ERR_INSERT_TIMESTEP_ATTR);
            }
            debug_log << "New timestep attribute id is " << to_string(temp_min_id) << endl;
        }
    }
    else if (num_run_timestep_types > 0) {
        MPI_Gather(&timestep_temp_max, 1, MPI_DOUBLE, NULL, NULL, MPI_DOUBLE, 0, MPI_COMM_WORLD); 
        MPI_Gather(&timestep_temp_min, 1, MPI_DOUBLE, NULL, NULL, MPI_DOUBLE, 0, MPI_COMM_WORLD); 
    }

    add_timing_point(CREATE_TIMESTEP_ATTRS_DONE);

    return rc;
}

int activate_timestep_components(md_server server, uint32_t timestep_num)
{
    int rc; 

    rc = metadata_activate_timestep  ( server, timestep_num);
    if (rc != RC_OK) {
        error_log << "Error. Was unable to activate the " << timestep_num << "th timestep. Exitting \n";
        add_timing_point(ERR_ACTIVATE_TIMESTEP);
        return rc;
    }

    rc = metadata_activate_var ( server, timestep_num);
    if (rc != RC_OK) {
        error_log << "Error. Was unable to activate the vars for the " << timestep_num << "th timestep. Exitting \n";
        add_timing_point(ERR_ACTIVATE_VAR);
        return rc;
    }

    rc = metadata_activate_var_attribute (server, timestep_num);
    if (rc != RC_OK) {
        error_log << "Error. Was unable to activate the types. Exitting \n";
        add_timing_point(ERR_ACTIVATE_VAR_ATTR);
        return rc;
    }

    rc = metadata_activate_timestep_attribute  ( server, timestep_num);
    if (rc != RC_OK) {
        error_log << "Error. Was unable to activate the timestep attrs. Exitting \n";
        add_timing_point(ERR_ACTIVATE_TIMESTEP_ATTR);
        return rc;
    }

    return rc;
}

int create_run_attrs(md_server server, int rank, double *all_timestep_temp_mins_for_all_procs, 
    double *all_timestep_temp_maxes_for_all_procs, uint64_t run_id, uint32_t num_timesteps, uint32_t num_types
    )
{
    int rc = RC_OK;

    add_timing_point(CREATE_RUN_ATTRS_START);

    double run_temp_max = -1 * RAND_MAX;
    double run_temp_min = RAND_MAX;

    add_timing_point(TIMESTEP_MAX_MIN_FIND_START);

    for(int timestep = 0; timestep < num_timesteps; timestep++) {
        if( all_timestep_temp_maxes_for_all_procs[timestep] > run_temp_max) {
            run_temp_max = all_timestep_temp_maxes_for_all_procs[timestep];
        }
        if( all_timestep_temp_mins_for_all_procs[timestep] < run_temp_min) {
            run_temp_min = all_timestep_temp_mins_for_all_procs[timestep];
        }    
    }
    add_timing_point(TIMESTEP_MAX_MIN_FIND_DONE);

    uint64_t temp_max_id;
    md_catalog_run_attribute_entry temp_max;
    temp_max.run_id = run_id;
    temp_max.type_id = num_types+1;
    temp_max.txn_id = run_id;
    make_single_val_data(run_temp_max, temp_max.data);
    testing_log << "run temperature max: " << run_temp_max << endl;
    temp_max.data_type = ATTR_DATA_TYPE_REAL;

    uint64_t temp_min_id;
    md_catalog_run_attribute_entry temp_min;
    temp_min.run_id = run_id;
    temp_min.type_id = num_types+2;
    temp_min.txn_id = run_id;
    make_single_val_data(run_temp_min, temp_min.data);
    testing_log << "run temperature min: " << run_temp_min << endl;
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
        debug_log << "New run attribute id is " << to_string(temp_max_id) << endl;

        rc = metadata_insert_run_attribute (server, temp_min_id, temp_min);
        if (rc != RC_OK) {
            error_log << "Error. Was unable to insert the new run attribute. Proceeding" << endl;
            add_timing_point(ERR_INSERT_RUN_ATTR);
        }
        debug_log << "New run attribute id is " << to_string(temp_min_id) << endl;
    }
    rc = metadata_activate_run_attribute  ( server, run_id);
    if (rc != RC_OK) {
        error_log << "Error. Was unable to activate the run attrs. Exitting \n";
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
    // faodel::NameAndNode *server_ary;

    if(rank == 0) { 
        struct stat buffer;
        bool dirman_initted = false;
        std::ifstream file;
        string dirman_hexid;
        faodel::DirectoryInfo dir;

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


        faodel::Configuration config(default_config_string);
        config.Append("dirman.root_node", dirman_hexid);
        config.Append("whookie.port", to_string(3000+rank)); //note if you up number of servers you'll want to up this
        config.AppendFromReferences();
        debug_log << "just configged" << endl; 
        faodel::bootstrap::Start(config, opbox::bootstrap);

        //Add the directory manager's URL to the config string so the clients know
        //Where to access the directory 
        //-------------------------------------------
        //TODO: This connect is temporarily necessary
        faodel::nodeid_t dirman_nodeid(dirman_hexid);
        extreme_debug_log << "Dirman node ID " << dirman_nodeid.GetHex() << " " << dirman_nodeid.GetIP() << " port " << dirman_nodeid.GetPort() << endl;
        dirman.name_and_node.node = dirman_nodeid;
        extreme_debug_log << "about to connect peer to node" << endl;
        rc = opbox::net::Connect(&dirman.peer_ptr, dirman.name_and_node.node);
        assert(rc==RC_OK && "could not connect");
        extreme_debug_log << "just connected" << endl;
        //-------------------------------------------
        extreme_debug_log << "app name is " << dir_path << endl;
        ok = dirman::GetRemotefaodel::DirectoryInfo(faodel::ResourceURL(dir_path), &dir);
        assert(ok && "Could not get info about the directory?");
        extreme_debug_log << "just got directory info" << endl;

        while( dir.members.size() < num_servers) {
            debug_log << "dir.members.size() < num_servers. dir.members.size(): "  << dir.members.size() << " num_servers: " << num_servers << endl;

            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            ok = dirman::GetRemotefaodel::DirectoryInfo(faodel::ResourceURL(dir_path), &dir);
            extreme_debug_log << "and now dir.members.size() < num_servers. dir.members.size(): "  << dir.members.size() << " num_servers: " << num_servers << endl;

            assert(ok && "Could not get info about the directory?");
            extreme_debug_log << "just got directory info" << endl;
        }
        debug_log << "dir init is done" << endl;
        server_procs = dir.members;
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
        extreme_debug_log << "serialized_c_str: " << (string)serialized_c_str << endl;
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
        debug_log << "just configged" << endl; 
        faodel::bootstrap::Start(config, opbox::bootstrap);

    } 
    free(serialized_c_str);


    for (int i=0; i<num_servers; i++) {
        extreme_debug_log << "server " << i << " in ary has hexid " << server_procs[i].node.GetHex() << endl;
    }



    // server_procs = temp(server_ary, server_ary +(int)num_servers);
    extreme_debug_log << "done initing dirman" << endl;
    return RC_OK;
 } 



static void setup_server(int rank, uint32_t num_servers, 
        const vector<faodel::NameAndNode> &server_nodes, md_server &server) {

    int server_indx = rank % num_servers; 

    server.name_and_node = server_nodes[server_indx];
    opbox::net::Connect(&server.peer_ptr, server.name_and_node.node);
    server.URL = server.name_and_node.node.GetHex();
    extreme_debug_log << "server.URL: " << server.URL << endl;
    extreme_debug_log << "server_indx: " << server_indx << endl;
}


void output_obj_names_and_timing(int rank, uint32_t num_client_procs, const md_catalog_run_entry &temp_run,
    const md_catalog_var_entry &temp_var, uint32_t num_timesteps, uint32_t num_vars
    )
{
    long double *all_time_pts_buf;

    if (output_objector_params) {
        gather_and_print_output_params(all_objector_params, rank, num_client_procs);
    }
    else if (output_obj_names) {
        uint64_t x_width, last_x_width;

        if(rank == 0) {
            uint64_t ndx = ( temp_var.dims[0].max - temp_var.dims[0].min + 1 ) / temp_run.npx;
            uint64_t ndy = ( temp_var.dims[1].max - temp_var.dims[1].min + 1 ) / temp_run.npy;
            uint64_t ndz = ( temp_var.dims[2].max - temp_var.dims[2].min + 1 ) / temp_run.npz;
            uint64_t chunk_volume = ndx * ndy * ndz;
            uint64_t chunk_size = chunk_volume * temp_var.data_size;
            get_obj_lengths(temp_var, x_width, last_x_width, ndx, chunk_size);
            uint64_t regular_object_volume = x_width * ndy * ndz;
            uint64_t last_object_volume = last_x_width * ndy * ndz;
            printf("chunk size: ndx: %d, ndy: %d, ndz: %d. total chunk volume %d\n", ndx,ndy,ndz,chunk_volume);
            printf("obj size: ndx: %d, ndy: %d, ndz: %d. for last object in chunk ndx: %d\n", 
                    x_width,ndy,ndz,last_x_width);
            printf("regular object volume: %d, regular object size: %d. last object volume: %d, last object size: %d \n\n",
                regular_object_volume,regular_object_volume*temp_var.data_size,last_object_volume,last_object_volume*temp_var.data_size);
        }
        gather_and_print_object_names(all_object_names, rank, num_client_procs, num_timesteps, num_vars);
    }

    if(output_timing) {

        double accum2 = std::accumulate(all_num_objects_to_fetch.begin(), all_num_objects_to_fetch.end(), 0.0);
        if( all_object_names.size() != accum2 ) {
            error_log << "error. all_num_objects_to_fetch.size(): " << all_object_names.size() << " sum(all_object_names): " <<
                accum2 << endl;
        }
        else {
            debug_log << "all_num_objects_to_fetch.size() == accum2 == " << accum2 << endl;
        }

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

            int obj_count = 0;
            int objector_output_count = 0;
            for(int i=0; i<sum; i++) {
                if (output_obj_names  && all_catg_time_pts_buf[i] == BOUNDING_BOX_TO_OBJ_NAMES) {
                    printf("%d %d ", all_catg_time_pts_buf[i], num_objects_to_fetch_for_all_procs[objector_output_count]);
                    objector_output_count += 1;
                }
                else if (output_objector_params && all_catg_time_pts_buf[i] == BOUNDING_BOX_TO_OBJ_NAMES) {
                    printf("%d %d ", all_catg_time_pts_buf[i], 1);
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
            free(num_objects_to_fetch_for_all_procs);

        }
    }
}


template <class T>
static void make_single_val_data (T val, string &serial_str) {
    extreme_debug_log << "about to make single val data \n";
    stringstream ss;
    boost::archive::text_oarchive oa(ss);
    oa << val;
    serial_str = ss.str();
}

template <class T1, class T2>
static void make_double_val_data (T1 val_0, T2 val_1, string &serial_str) {
    extreme_debug_log << "about to make double val data \n";
    stringstream ss;
    boost::archive::text_oarchive oa(ss);
    oa << val_0;
    oa << val_1;
    serial_str = ss.str();
}

void gather_and_print_output_params(const vector<objector_params> &all_objector_params, int rank, uint32_t num_client_procs) {

    extreme_debug_log.set_rank(rank);

    int length_ser_c_str = 0;
    char *serialized_c_str;
    int each_proc_ser_objector_params_size[num_client_procs];
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
    extreme_debug_log << "rank " << rank << " params ser string is of size " << length_ser_c_str << " serialized_str " << 
        serialized_str << endl;
        
    // extreme_debug_log << "rank " << rank << " about to allgather" << endl;

    MPI_Gather(&length_ser_c_str, 1, MPI_INT, each_proc_ser_objector_params_size, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // extreme_debug_log << "rank " << rank << " about with allgather" << endl;

    int num_iterations;
    int max_value = 0;

    int sum = 0;
    
    if (rank == 0 ) {
        for(int i=0; i<num_client_procs; i++) {
            displacement_for_each_proc[i] = sum;
            sum += each_proc_ser_objector_params_size[i];
            if(each_proc_ser_objector_params_size[i] != 0 && rank == 0) {
                extreme_debug_log << "rank " << i << " has a string of length " << each_proc_ser_objector_params_size[i] << endl;
            }
            if (each_proc_ser_objector_params_size[i] > max_value) {
                max_value = each_proc_ser_objector_params_size[i];
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
                "var_x1, var_x2, var_y1, var_y2, var_z1, var_z2              \n";
        for(int i = 0; i < num_client_procs; i++) {
            int offset = displacement_for_each_proc[i];
            int count = each_proc_ser_objector_params_size[i];
            if(count > 0) {
                vector<objector_params> rec_objector_params;

                if(i != rank) {

                    extreme_debug_log << "rank " << rank << " count: " << count << " offset: " << offset << endl;
                    char serialzed_objector_params_for_one_proc[count];

                    memcpy ( serialzed_objector_params_for_one_proc, serialized_c_str_all_objector_params + offset, count);
                    objector_params rec_objector_params;

                    extreme_debug_log << "rank " << rank << " serialized_c_str: " << (string)serialzed_objector_params_for_one_proc << endl;
                    extreme_debug_log <<    " serialized_c_str length: " << strlen(serialzed_objector_params_for_one_proc) << 
                        " count: " << count << endl;
                    stringstream ss1;
                    ss1.write(serialzed_objector_params_for_one_proc, count);
                    extreme_debug_log << "about to deserialize ss: " << ss1.str() << endl;
                    boost::archive::text_iarchive ia(ss1);
                    ia >> rec_objector_params;
                }
                else { //i == rank
                    rec_objector_params = all_objector_params;
                }
                extreme_debug_log << "num entries in rec_objector_params: " << rec_objector_params.size() << endl;

               // cout <<"rank         get_counts  false  job_id            sim_name          timestep      " <<
               //          "ndx              ndy              ndz              " <<
               //          "ceph_obj_size           var_name                   var_version      data_size     x1              " <<
               //          "y1              z1              x2              y2              z2              " <<
               //          "var_x1              var_x2              var_y1              var_y2              var_z1              var_z2              \n";

                
                int len =  to_string(max( rec_objector_params.at(0).var_dims[0].max, max(rec_objector_params.at(0).var_dims[1].max, rec_objector_params.at(0).var_dims[2].max) )).length();

                int len2 =  to_string(num_client_procs - 1).length();

                for( objector_params object_name : rec_objector_params) {                    
                    // function boundingBoxToObjectNamesAndCounts(get_counts, job_id, sim_name, timestep, ndx, ndy, ndz, ceph_obj_size, var_name, var_version, data_size, x1, y1, z1, x2, y2, z2, var_x1, var_x2, var_y1, var_y2, var_z1, var_z2)

                    //todo change %s to have max num digits if ever start using more than 1 run name
                    //todo change %llu to have max num digits if ever start using multi digit timestep ids
                    //todo change %lu to have max num digits if ever start using multi digit var versions
                    printf("%*d, false, %8llu, %s, %llu, %llu, %llu, %llu, "
                        "8000000, %10s, %lu, 8, %*llu, %*llu, %*llu, %*llu, %*llu, %*llu, " 
                        "%*llu, %*llu, %*llu, %*llu, %*llu, %*llu \n", 
                        len2, i, object_name.job_id, object_name.run_name.c_str(), object_name.timestep_id, 
                        object_name.ndx, object_name.ndy, object_name.ndz, object_name.var_name.c_str(), object_name.var_version, len,
                        object_name.bounding_box[0].min, len, object_name.bounding_box[1].min, len, object_name.bounding_box[2].min, 
                        len, object_name.bounding_box[0].max, len, object_name.bounding_box[1].max, len, object_name.bounding_box[2].max, 
                        len, object_name.var_dims[0].min, len, object_name.var_dims[0].max, len, object_name.var_dims[1].min, 
                        len, object_name.var_dims[1].max, len, object_name.var_dims[2].min, len, object_name.var_dims[2].max);
                    // printf("rank: %6d, get_counts: false, job_id: %8llu, sim_name: %6s, timestep: %2llu, ndx: %10llu, ndy: %10llu, ndz: %10llu,"
                    //     "ceph_obj_size: 8000000, var_name: %14s, var_version: %2lu, data_size: %1d, x1: %10llu, y1: %10llu, z1: %10llu, x2: %10llu, y2: %10llu, z2: %10llu," 
                    //     "var_x1: %10llu, var_x2: %10llu, var_y1: %10llu, var_y2: %10llu, var_z1: %10llu, var_z2: %10llu \n", 
                    //     i, object_name.job_id, object_name.run_name.c_str(), object_name.timestep_id, 
                    //     object_name.ndx, object_name.ndy, object_name.ndz, object_name.var_name.c_str(), object_name.var_version, 8,
                    //     object_name.bounding_box[0].min, object_name.bounding_box[1].min, object_name.bounding_box[2].min, 
                    //     object_name.bounding_box[0].max, object_name.bounding_box[1].max, object_name.bounding_box[2].max, 
                    //     object_name.var_dims[0].min, object_name.var_dims[0].max, object_name.var_dims[1].min, 
                    //     object_name.var_dims[1].max, object_name.var_dims[2].min, object_name.var_dims[2].max);
                }
            }
            cout << endl;
        }
        free(serialized_c_str_all_objector_params);
    }

    if(length_ser_c_str > 0) {
        free(serialized_c_str);
    }
}


objector_params get_objector_params ( const md_catalog_run_entry &run, const md_catalog_var_entry &var, 
                            const vector<md_dim_bounds> &bounding_box) {

    objector_params object_names;

    object_names.run_id = run.run_id;
    object_names.job_id = run.job_id;
    object_names.run_name = run.name;
    object_names.timestep_id = var.timestep_id;
    object_names.ndx = ( var.dims[0].max - var.dims[0].min + 1 ) / run.npx;
    object_names.ndy = ( var.dims[1].max - var.dims[1].min + 1) / run.npy;
    object_names.ndz = ( var.dims[2].max - var.dims[2].min + 1 ) / run.npz;
    object_names.var_id = var.var_id;
    object_names.var_name = var.name;
    object_names.var_version = var.version;      
    object_names.var_dims = var.dims;
    object_names.bounding_box = bounding_box;
    // object_names.ceph_obj_size = 8000000

    return object_names;
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
    // if (rank == 1) {
    //     cout << "serialized_str: " << serialized_str << endl;
    // }

    int sum = 0;
    // int max_value = 0;
    if (rank == 0 ) {
        for(int i=0; i<num_client_procs; i++) {
            displacement_for_each_proc[i] = sum;
            sum += each_proc_ser_values_size[i];
            if(each_proc_ser_values_size[i] != 0 && rank == 0) {
                extreme_debug_log << "rank " << i << " has a string of length " << each_proc_ser_values_size[i] << endl;
            }
            // if(displacement_for_each_proc[i] > max_value) {
            //     max_value = displacement_for_each_proc[i];
            // }
        }
        extreme_debug_log << "sum: " << sum << endl;

       *serialized_c_str_all_ser_values = (char *) malloc(sum);
    }

    extreme_debug_log << "num_bytes is "<< to_string(length_ser_c_str) + " and rank: " << to_string(rank) << endl;
    
    extreme_debug_log << "rank " << rank << " about to gatherv" << endl;

    MPI_Gatherv(serialized_c_str, length_ser_c_str, MPI_CHAR,
           *serialized_c_str_all_ser_values, each_proc_ser_values_size, displacement_for_each_proc,
           MPI_CHAR, 0, MPI_COMM_WORLD);

    extreme_debug_log << "just finished with gather v" << endl;

    free(serialized_c_str);
}

void gather_and_print_object_names(const vector<vector<string>> &all_object_names, int rank, uint32_t num_client_procs, 
                            uint32_t num_timesteps, uint32_t num_vars)
{
    extreme_debug_log.set_rank(rank);


   uint16_t num_vars_per_run = num_vars * num_timesteps;
   vector<vector<vector<string>>> all_rec_obj_names(num_vars_per_run);

    for (int k = 0; k < all_object_names.size(); k++) {
        int each_proc_ser_object_names_size[num_client_procs];
        int displacement_for_each_proc[num_client_procs];
        char *serialized_c_str_all_object_names;

        vector<string> obj_names = all_object_names.at(k);
        if (obj_names.size() == 0) {
            cout << "error. rank: " << rank << "obj_names.size(): " << obj_names.size() << endl;
        }

        gatherv_ser(obj_names, num_client_procs, rank, each_proc_ser_object_names_size, displacement_for_each_proc, 
        &serialized_c_str_all_object_names); 
 
        // extreme_debug_log << "rank " << rank << " done with allgatherv" << endl;
        // extreme_debug_log << "entire set of attr vectors: " << serialized_c_str_all_object_names << " and is of length: " << strlen(serialized_c_str_all_object_names) << endl;
        if(rank == 0) {

            for(int i = 0; i < num_client_procs; i++) {
                int offset = displacement_for_each_proc[i];
                int count = each_proc_ser_object_names_size[i];
                if(count > 0) {
                    vector<string> rec_obj_names;

                    if(i != rank) {

                        extreme_debug_log << "rank " << rank << " count: " << count << " offset: " << offset << endl;
                        char serialzed_object_names_for_one_proc[count];

                        memcpy ( serialzed_object_names_for_one_proc, serialized_c_str_all_object_names + offset, count);

                        extreme_debug_log << "rank " << rank << " serialized_c_str: " << (string)serialzed_object_names_for_one_proc << endl;
                        extreme_debug_log <<    " serialized_c_str length: " << strlen(serialzed_object_names_for_one_proc) << 
                            " count: " << count << endl;
                        stringstream ss1;
                        ss1.write(serialzed_object_names_for_one_proc, count);
                        extreme_debug_log << "about to deserialize ss: " << ss1.str() << endl;
                        boost::archive::text_iarchive ia(ss1);
                        ia >> rec_obj_names;
                    }
                    else { //i == rank
                        rec_obj_names = all_object_names.at(k);
                    }
                    all_rec_obj_names.at(k).push_back(rec_obj_names);
                }
            }
            free(serialized_c_str_all_object_names);
        }
    }

    if (rank == 0) {

        int len2 =  to_string(num_client_procs - 1).length();

       cout <<"rank, object_name\n";
        for (int i = 0; i < all_rec_obj_names.size(); i++) {
            int var_id = i % num_vars;
            int timestep_id = i / num_vars;
            //rank, timestep, var
            printf("\nobj names for TIMESTEP %d VAR %d:\n", timestep_id, var_id);

            vector<vector<string>> obj_names = all_rec_obj_names.at(i);

            for (int rank = 0; rank < obj_names.size(); rank++) {
                for (string obj_name : obj_names.at(rank)) {
                    printf("%*d, %s\n", len2, rank, obj_name.c_str());
                }
                // cout << endl;
            }
        }
        if(output_timing) {
            cout << "begin timing output" << endl;
        }
    }

}

static void get_obj_lengths(const md_catalog_var_entry &var, 
                    uint64_t &x_width, uint64_t &last_x_width, uint64_t ndx, uint64_t chunk_size) 
{

    uint64_t ceph_obj_size = 8000000; //todo 

    extreme_debug_log << "chunk size: " << chunk_size << endl;
    uint32_t num_objs_per_chunk = round(chunk_size / ceph_obj_size);
    if(num_objs_per_chunk <= 0) {
        num_objs_per_chunk = 1;
    }
    extreme_debug_log << "num_objs_per_chunk: " << num_objs_per_chunk << endl;
    x_width = round(ndx / num_objs_per_chunk);
    if(x_width <= 0) {
        x_width = 1;
    }
    extreme_debug_log << "x_width: " << x_width << endl;
    last_x_width = ndx - x_width * (num_objs_per_chunk-1);
    if(last_x_width <= 0) {
        num_objs_per_chunk = num_objs_per_chunk + floor( (last_x_width-1) / x_width);
        last_x_width = ndx - x_width * (num_objs_per_chunk-1);
    }
}
