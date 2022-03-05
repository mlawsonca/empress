#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include <mpi.h>
#include <unistd.h>
#include <iostream>
#include <chrono>
#include <fstream>
#include <exception>
#include <sys/stat.h>
#include <sys/time.h>
#include <unordered_map>

// #include <server_timing_constants.hh>
#include <server_timing_constants_new.hh>
#include <my_metadata_server_sqlite_transactions_only.hh>
// #include <libops_sqlite_transactions_only.hh>
#include <libops.hh>

#include "dirman/DirMan.hh"
// 
// #include "opbox/OpBox.hh"
#include "faodel-common/Common.hh"
// 
// #include <sys/resource.h> //needed for get mem usage
// #include <sys/sysinfo.h>

// #define MAX_MEM_USAGE 3400000000
// #define MAX_MEM_USAGE 340000000
// #define MAX_DB_SIZE 2000000000
#define MAX_DB_SIZE 1500000000
// #define MAX_DB_SIZE 200000000

// #define MAX_DB_SIZE 150000000
// #define MAX_DB_SIZE 15000000

pthread_cond_t cond1 = PTHREAD_COND_INITIALIZER;
pthread_mutex_t lock1 = PTHREAD_MUTEX_INITIALIZER;

using namespace std;

std::string default_config_string = R"EOF(
# Select a transport to use for nnti (laptop tries ib if not forced to mpi)

nnti.logger.severity       error
nnti.transport.name     ibverbs
whookie.interfaces       ib0,lo

#config.additional_files.env_name.if_defined   CONFIG
lunasa.lazy_memory_manager   malloc
lunasa.eager_memory_manager  tcmalloc

# Select the type of dirman to use. Currently we only have centralized, which
# just sticks all the directory info on one node (called root). We use roles
# to designate which node is actually the root.
dirman.type        centralized
dirman.host_root   false

# Turn these on if you want to see more debug messages
#bootstrap.debug           true
#whookie.debug           true 
#opbox.debug               true
#dirman.debug             true
#dirman.cache.others.debug true
#dirman.cache.mine.debug   true

)EOF";

sqlite3 *main_db = NULL;
int proc_rank;
bool first_checkpt = true;
int checkpt_count = -1;

// bool debug_logging = false;
bool error_logging = true;

bool md_shutdown = false;

int open_db_count = -1;
// uint64_t checkpt_count = 0;
// int checkpt_type;
// md_write_type write_type;
md_server_type server_type;
md_db_index_type index_type;
md_db_checkpoint_type checkpt_type;


struct db_conn {
    sqlite3 *db;
    bool transaction_open = false;

    db_conn(sqlite3 *my_db) {
        db = my_db;
    }

    db_conn() {

    }

    // db_conn(sqlite3 *my_db, bool trans_open) {
    //     db = my_db;
    //     transaction_open = trans_open;
    // }
};

unordered_map<uint64_t,db_conn> txn_id_to_db_connection{};
// unordered_map<uint64_t,sqlite3 *> txn_id_to_db_connection{};
// unordered_map<uint64_t,vector<bool>> txn_id_to_transaction_states{};

// uint32_t MAX_EAGER_MSG_SIZE = 8192;
// uint32_t MAX_EAGER_MSG_SIZE = 4096;
// uint32_t MAX_EAGER_MSG_SIZE = 2048;
// uint32_t MAX_EAGER_MSG_SIZE = 1024;
uint32_t MAX_EAGER_MSG_SIZE;

static bool output_timing = true;

static debugLog error_log = debugLog(error_logging);
// static debugLog debug_log = debugLog(debug_logging);

//link to testing_debug.cpp in cmake to do debug tests
// static bool do_debug_testing = false;

std::vector<std::chrono::high_resolution_clock::time_point> time_pts;
std::vector<int> catg_of_time_pts;
std::vector<uint64_t> db_checkpoint_sizes;

static void add_timing_point(chrono::high_resolution_clock::time_point time_pt, int catg) {
    if (output_timing) {
        time_pts.push_back(time_pt);
        catg_of_time_pts.push_back(catg);
    }
}


void add_timing_point(int catg) {
    if (output_timing) {
        time_pts.push_back(chrono::high_resolution_clock::now());
        catg_of_time_pts.push_back(catg);
    }
}

static void add_db_output_timing_point(int catg, uint64_t size) {
    if (output_timing) {
        time_pts.push_back(chrono::high_resolution_clock::now());
        catg_of_time_pts.push_back(catg);
        db_checkpoint_sizes.push_back(size);
    }
}

static int db_reset(sqlite3 **db, uint64_t job_id);

int callback (void * NotUsed, int argc, char ** argv, char ** ColName)
{
    int i;
    for (i = 0; i < argc; i++)
    {
        printf ("%s = %s\n", ColName [i], argv [i] ? argv [i] : "NULL");
    }
    printf ("\n");
    return 0;
}


// bool db_output_is_whole(md_db_checkpoint_type checkpt_type) {
//   return(checkpt_type != DB_COPY_AND_DELETE && checkpt_type != DB_COPY_AND_RESET);
// }
bool db_output_is_whole() {
    return(checkpt_count <= 0);
}
//===========================================================================
// static int metadata_database_init(bool load_db, uint64_t job_id, int rank)


//argv[0] = name of program, argv[1]=directory manager path
int main (int argc, char **argv)
{
    char name[100];
    gethostname(name, sizeof(name));
    //debug_log << name << endl;
    
    if (argc != 8)
    {
        cout << 0 << " " << (int)ERR_ARGC << endl;
        // error_log << "Usage: " << argv[0] << " <md_dirman file path>, estm_num_time_pts, sql_load_db(0 or 1), job_id, server_type, index_type, checkpt type \n";
        error_log << "Usage: " << argv[0] << " <md_dirman node name>, estm_num_time_pts, sql_load_db(0 or 1), job_id, server_type, index_type, checkpt type \n";
        return RC_ERR;
    }

    string dirman_file_path = argv[1];
    // string dirman_node_name = argv[1];
    uint32_t estm_num_time_pts = stoul(argv[2], nullptr,0); 
    bool load_db = atoi(argv[3]);
    // bool output_db = atoi(argv[4]);
    uint64_t job_id = stoull(argv[4],nullptr,0);
    // uint64_t num_timesteps = stoull(argv[5],nullptr,0);
    // uint32_t num_timesteps_per_checkpt = stoul(argv[6],nullptr,0);
    // md_db_checkpoint_type checkpt_type = (md_db_checkpoint_type)stoul(argv[7],nullptr,0);
    // md_write_type write_type = (md_write_type)stoul(argv[5],nullptr,0);
    // write_type = (md_write_type)stoul(argv[5],nullptr,0);
    server_type = (md_server_type)stoul(argv[5],nullptr,0);
    index_type = (md_db_index_type)stoul(argv[6],nullptr,0);

    // write_type = (md_write_type)stoul(argv[5],nullptr,0);
    // md_db_checkpoint_type checkpt_type = (md_db_checkpoint_type)stoul(argv[7],nullptr,0);
    checkpt_type = (md_db_checkpoint_type)stoul(argv[7],nullptr,0);

    if(checkpt_type == DB_INCR_OUTPUT_AND_DELETE || checkpt_type == DB_INCR_OUTPUT_AND_RESET 
        || server_type == SERVER_DEDICATED_ON_DISK || server_type == SERVER_LOCAL_ON_DISK) {
        checkpt_count = 0;
    }

    catg_of_time_pts.reserve(estm_num_time_pts);
    time_pts.reserve(estm_num_time_pts);

    int num_wall_time_pts = 6;
    int num_db_pts = 9;
    struct timeval start, mpi_init_done, register_done, db_init_done, dirman_init_done, finalize;
    vector<long double> clock_times;
    clock_times.reserve(num_wall_time_pts);

    gettimeofday(&start, NULL);
    int zero_time_sec = 86400 * (start.tv_sec / 86400);
    clock_times.push_back( (start.tv_sec - zero_time_sec) + start.tv_usec / 1000000.0);
    chrono::high_resolution_clock::time_point start_time = chrono::high_resolution_clock::now();
    add_timing_point(start_time, PROGRAM_START);

    int rc;
    int rank;
    int num_procs;
    string dir_path="/metadata/testing";
    uint64_t db_sizes[num_db_pts];
    opbox::net::Attrs nnti_attrs;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
    proc_rank = rank;

    gettimeofday(&mpi_init_done, NULL);
    clock_times.push_back( (mpi_init_done.tv_sec - zero_time_sec) + mpi_init_done.tv_usec / 1000000.0);
    add_timing_point(MPI_INIT_DONE);


    rc = register_ops();

    gettimeofday(&register_done, NULL);
    clock_times.push_back( (register_done.tv_sec - zero_time_sec) + register_done.tv_usec / 1000000.0);
    add_timing_point(REGISTER_OPS_DONE);


    rc = sqlite3_config(SQLITE_CONFIG_URI);
    if (rc != SQLITE_OK)    {
        fprintf (stdout, "Error setting sqlite3_config. Line: %d SQL error: %s\n", __LINE__, sqlite3_errmsg (main_db));
        sqlite3_close (main_db);
        goto cleanup;
    }
    //set up and return the database
    // rc = metadata_database_init(load_db, job_id, rank);
    // rc = metadata_database_init(load_db, job_id, server_type);
    rc = metadata_database_init(&main_db, load_db, false, job_id, false);
    if (rc != RC_OK) {
        error_log << "Error. Could not init dbs. Error code " << rc << std::endl;
        time_pts.push_back(chrono::high_resolution_clock::now());
        catg_of_time_pts.push_back(ERR_DB_SETUP);
        goto cleanup;
    }


    gettimeofday(&db_init_done, NULL);
    clock_times.push_back( (db_init_done.tv_sec - zero_time_sec) + db_init_done.tv_usec / 1000000.0);
    // add_timing_point(DB_SETUP_DONE);

    // db_sizes[0] = get_db_size();
    db_sizes[0] = get_db_size(main_db);
    //debug_log << "mem after setup and index creation: " << db_sizes[0] << endl;

    rc = setup_dirman(dirman_file_path, dir_path, rank);
    // rc = setup_dirman(dirman_node_name, dir_path, rank);
    if (rc != RC_OK) {
        time_pts.push_back(chrono::high_resolution_clock::now());
        catg_of_time_pts.push_back(ERR_DIRMAN);
        error_log << "Error. Could not init dirman. Error code " << rc << std::endl;
        goto cleanup;
    }
    opbox::net::GetAttrs(&nnti_attrs);
    MAX_EAGER_MSG_SIZE = nnti_attrs.max_eager_size;
    //debug_log << "MAX_EAGER_MSG_SIZE: " << MAX_EAGER_MSG_SIZE << endl;

    gettimeofday(&dirman_init_done, NULL);
    clock_times.push_back( (dirman_init_done.tv_sec - zero_time_sec) + dirman_init_done.tv_usec / 1000000.0);
    add_timing_point(DIRMAN_SETUP_DONE_INIT_DONE);


    //debug_log << "starting ops" << endl;
    // while(!md_shutdown) {
    //    cout << "about to sleep" << endl;
    //    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    //    cout << "done sleeping" << endl;
    //    //debug_log << "id: " << std::this_thread::get_id() << endl;
    //  }

    //debug_log << "am waiting for cond" << endl;
    pthread_cond_wait(&cond1, &lock1);
    //debug_log << "am continuing after cond" << endl;

cleanup:
    // main_db = txn_id_to_db_connection[job_id].db; //just in case its changed

    //debug_log << "Shutting down" << endl;
    gettimeofday(&finalize, NULL);
    clock_times.push_back( (finalize.tv_sec - zero_time_sec) + finalize.tv_usec / 1000000.0);
    add_timing_point(SHUTDOWN_START);

    // db_sizes[1] = get_db_size();
    db_sizes[1] = get_db_size(main_db);

    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;


    rc = sqlite3_prepare_v2 (main_db, "select count (*) from run_catalog", -1, &stmt, &tail); assert (rc == SQLITE_OK);
    rc = sqlite3_step (stmt); assert (rc == SQLITE_ROW || rc == SQLITE_DONE);
    db_sizes[2] = sqlite3_column_int (stmt, 0);
    rc = sqlite3_finalize (stmt); assert (rc == SQLITE_OK);

    rc = sqlite3_prepare_v2 (main_db, "select count (*) from timestep_catalog", -1, &stmt, &tail); assert (rc == SQLITE_OK);
    rc = sqlite3_step (stmt); assert (rc == SQLITE_ROW || rc == SQLITE_DONE);
    db_sizes[3] = sqlite3_column_int (stmt, 0);
    rc = sqlite3_finalize (stmt); assert (rc == SQLITE_OK);

    rc = sqlite3_prepare_v2 (main_db, "select count (*) from var_catalog", -1, &stmt, &tail); assert (rc == SQLITE_OK);
    rc = sqlite3_step (stmt); assert (rc == SQLITE_ROW || rc == SQLITE_DONE);
    db_sizes[4] = sqlite3_column_int (stmt, 0);
    rc = sqlite3_finalize (stmt); assert (rc == SQLITE_OK);

    rc = sqlite3_prepare_v2 (main_db, "select count (*) from type_catalog", -1, &stmt, &tail); assert (rc == SQLITE_OK);
    rc = sqlite3_step (stmt); assert (rc == SQLITE_ROW || rc == SQLITE_DONE);
    db_sizes[5] = sqlite3_column_int (stmt, 0);
    rc = sqlite3_finalize (stmt); assert (rc == SQLITE_OK);

    rc = sqlite3_prepare_v2 (main_db, "select count (*) from run_attribute_catalog", -1, &stmt, &tail); assert (rc == SQLITE_OK);
    rc = sqlite3_step (stmt); assert (rc == SQLITE_ROW || rc == SQLITE_DONE);
    db_sizes[6] = sqlite3_column_int (stmt, 0);
    rc = sqlite3_finalize (stmt); assert (rc == SQLITE_OK);

    rc = sqlite3_prepare_v2 (main_db, "select count (*) from timestep_attribute_catalog", -1, &stmt, &tail); assert (rc == SQLITE_OK);
    rc = sqlite3_step (stmt); assert (rc == SQLITE_ROW || rc == SQLITE_DONE);
    db_sizes[7] = sqlite3_column_int (stmt, 0);
    rc = sqlite3_finalize (stmt); assert (rc == SQLITE_OK);

    rc = sqlite3_prepare_v2 (main_db, "select count (*) from var_attribute_catalog", -1, &stmt, &tail); assert (rc == SQLITE_OK);
    rc = sqlite3_step (stmt); assert (rc == SQLITE_ROW || rc == SQLITE_DONE);
    db_sizes[8] = sqlite3_column_int (stmt, 0);
    rc = sqlite3_finalize (stmt); assert (rc == SQLITE_OK);   

    //debug_log << "sizeof main_db: " << db_sizes[1] << endl;
    //debug_log << "num runs: " << db_sizes[2] << " num timesteps: " << db_sizes[3] << " num vars: " << db_sizes[4];
    //debug_log << " num types: " << db_sizes[5] << " num run attributes: " << db_sizes[6] << " num timestep attributes: " << db_sizes[7];
    //debug_log << " num var attributes: " << db_sizes[8] << endl;

    add_timing_point(DB_ANALYSIS_DONE);

    // if(output_db) {
    //   rc = sql_output_db(main_db, job_id, rank);
    //   if (rc != SQLITE_OK)
    //   {   
    //       fprintf (stderr, "Can't output database for job_id:%llu: %s\n",job_id, sqlite3_errmsg (main_db));       
    //txn_id_to_db_connection.erase(job_id);    
    //sqlite3_close (main_db);
    //       goto cleanup;
    //   }
    // }
    txn_id_to_db_connection.erase(job_id);
    sqlite3_close (main_db);
    add_timing_point(DB_CLOSED_SHUTDOWN_DONE);

    //note - if we knew for a fact that the main_db would fit in mem, could do this before closing
    if(index_type == WRITE_DELAYED_INDEX && db_output_is_whole()) {
        sqlite3_stmt * stmt = NULL;
        const char * tail = NULL;

        char  *ErrMsg = NULL;

        // string filename = get_filename_base(job_id, checkpt_type) + to_string(checkpt_count);
        string filename = get_filename_base(job_id) + to_string(checkpt_count);
        //debug_log << "trying to open " << filename.c_str() << endl;
        rc = sqlite3_open(filename.c_str(), &main_db);
        if (rc != SQLITE_OK)
        {   
            fprintf (stderr, "Can't open database: %s\n", sqlite3_errmsg (main_db));     
            txn_id_to_db_connection.erase(job_id);
            sqlite3_close (main_db);
            goto cleanup;
        }
        //makes sure that we wont run out of space for the temp files (used for creating the indices)
        rc = sqlite3_exec (main_db, "PRAGMA temp_store = FILE", callback, 0, 0);
        if (rc != SQLITE_OK)    {
            fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, sqlite3_errmsg (main_db));
            sqlite3_close (main_db);
            goto cleanup;
        }

        rc = sqlite3_exec (main_db, "PRAGMA temp_store_directory = 'FILL_IN_WITH_DESIRED_VALUE", callback, 0, 0);
        if (rc != SQLITE_OK)    {
            fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, sqlite3_errmsg (main_db));
            sqlite3_close (main_db);
            goto cleanup;
        }

        rc = create_reading_indices(main_db);
        txn_id_to_db_connection.erase(job_id);
        sqlite3_close (main_db);
    }
    else if (!db_output_is_whole()) {
        //debug_log << "main_db isn't whole. checkpt_type: " << checkpt_type << endl;
        // rc = db_compact_and_output(job_id, checkpt_type);
        rc = db_compact_and_output(main_db, job_id);
    }

    if(output_timing) {
        int num_time_pts = time_pts.size();
        int *catg_of_time_pts_ary = &catg_of_time_pts[0];
        double *total_times = (double *) malloc(sizeof(double) * num_time_pts);

        //debug_log << "rank: " << rank <<" my main_db checkpt size: " << db_checkpoint_sizes.size() << endl;

        uint64_t *db_checkpoint_sizes_ary;
 
        if(db_checkpoint_sizes.size() > 0) {
            //debug_log << "rank: " << rank << " my actual main_db size: " << db_checkpoint_sizes[0] << endl;
            db_checkpoint_sizes_ary = &db_checkpoint_sizes[0];
        }
         uint64_t *all_db_checkpoint_sizes_buf;

        //needed by rank 0
        int displacement_for_each_proc[num_procs];
        int *each_proc_num_time_pts;
        int *all_catg_time_pts_buf;
        double *all_time_pts_buf;
        uint64_t *all_db_sizes;
        long double *all_clock_times;

        if(rank == 0) {
            each_proc_num_time_pts = (int *) malloc(num_procs * sizeof(int));
            all_db_sizes = (uint64_t *) malloc(num_procs * num_db_pts * sizeof(uint64_t));
            all_clock_times = (long double *) malloc(num_procs * num_wall_time_pts * sizeof(long double));
        }

        //debug_log << "time_pts.size(): " << time_pts.size() << endl;
        MPI_Gather(&num_time_pts, 1, MPI_INT, each_proc_num_time_pts, 1, MPI_INT, 0, MPI_COMM_WORLD); 

        MPI_Gather(db_sizes, num_db_pts, MPI_UNSIGNED_LONG, all_db_sizes, num_db_pts, MPI_UNSIGNED_LONG, 0, MPI_COMM_WORLD); 

        MPI_Gather(&clock_times[0], num_wall_time_pts, MPI_LONG_DOUBLE, all_clock_times, num_wall_time_pts, MPI_LONG_DOUBLE, 0, MPI_COMM_WORLD); 

        int sum = 0;
        if(rank == 0) {
            for(int i=0; i<num_procs; i++) {
                displacement_for_each_proc[i] = sum;
                sum += each_proc_num_time_pts[i];
            }
            all_time_pts_buf = (double *) malloc(sum * sizeof(double));
            all_catg_time_pts_buf = (int *) malloc(sum * sizeof(int));
            //debug_log << "num_procs: " << num_procs << " db_checkpoint_sizes.size(): " << db_checkpoint_sizes.size() << endl;
            all_db_checkpoint_sizes_buf = (uint64_t *) malloc(num_procs * db_checkpoint_sizes.size() * sizeof(uint64_t));

            //debug_log << "sum: " + to_string(sum) << endl;
               //debug_log << "estm num_time_pts: " << to_string(estm_num_time_pts) << endl;
        }
        for(int i=0; i< time_pts.size(); i++) {
            std::chrono::duration<double, std::nano> fp_ns = time_pts.at(i) - start_time;
            total_times[i] = fp_ns.count();
        }

        MPI_Gatherv(total_times, num_time_pts, MPI_DOUBLE, all_time_pts_buf, each_proc_num_time_pts, displacement_for_each_proc, MPI_DOUBLE, 0, MPI_COMM_WORLD); 

        MPI_Gatherv(catg_of_time_pts_ary, num_time_pts, MPI_INT, all_catg_time_pts_buf, each_proc_num_time_pts, displacement_for_each_proc, MPI_INT, 0, MPI_COMM_WORLD); 

        // if(db_checkpoint_sizes.size() > 0) {
        //debug_log << "db_checkpoint_sizes.size():" << db_checkpoint_sizes.size() << endl;

        MPI_Gather(db_checkpoint_sizes_ary, db_checkpoint_sizes.size(), MPI_UINT64_T, all_db_checkpoint_sizes_buf, db_checkpoint_sizes.size(), MPI_UINT64_T, 0, MPI_COMM_WORLD); 

        // }
        if (rank == 0) {
            //prevent it from buffering the printf statements
            setbuf(stdout, NULL);
            std::cout << "begin timing output" << endl;

            cout << (int)DB_SIZES << " ";
            for(int i=0; i<(num_db_pts*num_procs); i++) {
                std::cout << all_db_sizes[i] << " ";
                if (i%20 == 0 && i!=0) {
                    std::cout << std::endl;
                }
            }
            for(int i = 0; i<(num_wall_time_pts * num_procs); i++) {
                if(i%num_wall_time_pts == 0) {
                    std::cout << (int)CLOCK_TIMES_NEW_PROC << " ";
                }
                if(all_clock_times[i] != 0) {
                    printf("%.6Lf ", all_clock_times[i]);
                }
                if (i%20 == 0 && i!=0) {
                    std::cout << std::endl;
                } 
            }
            std::cout << (int)CLOCK_TIMES_DONE << " ";

            int checkpt_indx = 0;
            //prevent it buffering the 
            for(int i=0; i<sum; i++) {
                if( is_db_output_start(all_catg_time_pts_buf[i])) {
                    printf("%4d %10.0f %llu", all_catg_time_pts_buf[i], all_time_pts_buf[i], all_db_checkpoint_sizes_buf[checkpt_indx]);
                    //only increment for the incremental since one always follows the original output
                    checkpt_indx += 1;
                }
                else {
                    printf("%4d %10.0f ", all_catg_time_pts_buf[i], all_time_pts_buf[i]);
                }
               // std::cout << all_time_pts_buf[i] << " " << all_catg_time_pts_buf[i] << " ";
               if (i%20 == 0 && i!=0) {
                   std::cout << std::endl;
               }
           }
           std::cout << std::endl;

            free(each_proc_num_time_pts);
            free(all_db_sizes);
            free(all_clock_times);
            free(all_time_pts_buf);
            free(all_catg_time_pts_buf);
            free(all_db_checkpoint_sizes_buf);
        }
        free(total_times);
    }
   
    MPI_Barrier(MPI_COMM_WORLD);
    faodel::bootstrap::Finish();
    //debug_log << "about to MPI_Finalize \n";
    MPI_Finalize();
    //debug_log << "just mpi finalized \n";
    //debug_log << "returning \n";
    return rc;
}

//doesn't quite work anymore. gets the hostname->ip slightly wrong
// static int setup_dirman(const string &dirman_hostname, const string &dir_path, int rank, int dirman_port)
// {
//     bool dirman_initted = false;
//     int rc = RC_ERR;
//     bool ok;

//     faodel::DirectoryInfo dir;
//     opbox::net::peer_ptr_t peer;

//     faodel::nodeid_t dirman_nodeid(dirman_hostname, to_string(dirman_port));
//     //debug_log << "dirman_hostname: " << dirman_hostname << endl;
//     //debug_log << "dirman port: " << dirman_nodeid.GetPort() << endl;
//     //debug_log << "dirman ip: " << dirman_nodeid.GetIP() << endl;
//     string dirman_hexid = dirman_nodeid.GetHex();
//     //debug_log << "dirman hexid: " + dirman_hexid << endl;

//     faodel::Configuration config(default_config_string);
//     config.Append("dirman.root_node", dirman_hexid.c_str());
//     config.Append("whookie.port", to_string(1991+rank));
//     config.AppendFromReferences();

//     faodel::bootstrap::Start(config, dirman::bootstrap);

//     do {
//         std::this_thread::sleep_for(std::chrono::milliseconds(10));
//         rc = opbox::net::Connect(&peer, dirman_nodeid);
//         //debug_log << "could not connect. trying again" << endl;
//     } while (rc != RC_OK);

//     //debug_log << "about to join dir" << endl;
//     ok = dirman::JoinDirWithoutName(faodel::ResourceURL(dir_path), &dir); 
//     // assert(ok && "Could not join the directory?");
//     if(ok) return RC_OK;
//     else return RC_ERR;
// }


static int setup_dirman(const string &dirman_file_path, const string &dir_path, int rank)
{
   struct stat buffer;
    bool dirman_initted = false;
    std::ifstream file;
    char dirman_hexid_c_str[14];
    string dirman_hexid;

    bool ok;
    int rc;

    if(rank == 0) { 
        while (!dirman_initted) {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            dirman_initted = (stat (dirman_file_path.c_str(), &buffer) == 0);
            //debug_log << "dirman not initted yet \n";
        }
        file.open(dirman_file_path);
        if(!file) {
            return RC_ERR;
        }
        while( file.peek() == std::ifstream::traits_type::eof() ) {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            //debug_log << "dirman file empty \n";
        }
        file >> dirman_hexid;
        // dirman_hexid_c_str = (char *)dirman_hexid.c_str();
        dirman_hexid.copy(dirman_hexid_c_str,13);
        dirman_hexid_c_str[13]='\0';
        //debug_log << "just got the hexid: " << dirman_hexid << endl;
    }
    MPI_Bcast(dirman_hexid_c_str, 14, MPI_CHAR, 0, MPI_COMM_WORLD);

    //debug_log << "just got the hexid from bcast: " << dirman_hexid_c_str << endl;

    //Add the directory manager's URL to the config string so the clients know
    //Where to access the directory 
    faodel::Configuration config(default_config_string);
    config.Append("dirman.root_node", dirman_hexid_c_str);
    config.Append("whookie.port", to_string(1991+rank)); //note if you up number of servers you'll want to up this
    config.AppendFromReferences();

    //debug_log << "just configged" << endl; 
    faodel::bootstrap::Start(config, dirman::bootstrap);

    string myHexID = opbox::GetMyID().GetHex();
    //debug_log << "my nodeid is:   "  << myHexID << endl;
    //-------------------------------------------
    //TODO: This connect is temporarily necessary
    faodel::nodeid_t dirman_nodeid((string)dirman_hexid_c_str);

    //debug_log << "Dirman node ID " << dirman_nodeid.GetHex() << " " << dirman_nodeid.GetIP() << " port " << dirman_nodeid.GetPort() << endl;
    opbox::net::peer_ptr_t peer;
    rc = opbox::net::Connect(&peer, dirman_nodeid);
    assert(rc==0 && "could not connect");
    //-------------------------------------------
    
    faodel::DirectoryInfo dir;
    faodel::nodeid_t myid = opbox::GetMyID();
    //debug_log << "AppServer ID   " << myid.GetHex() << endl;
    //Have all nodes join the directory
    ok = dirman::JoinDirWithoutName(faodel::ResourceURL(dir_path), &dir);
    assert(ok && "Could not join the directory?");

    //debug_log << "App Server Info: '" << dir.info << "ReferenceNode: " << dir.GetReferenceNode().GetHex() << " NumberChildren: " << to_string(dir.members.size()) << endl;

    return rc;
}



// static int metadata_database_init(bool load_db, uint64_t job_id, md_server_type server_type)
static int metadata_database_init(sqlite3 **db, bool load_db, bool auxiliary_db, uint64_t job_id, bool db_reset)
{
    //setup the database
    int rc;
    char  *ErrMsg = NULL;
    //debug_log << "metadata_database_init: beginning db: " << &db << endl;

    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;
    uint64_t last_run_id, last_type_id, last_run_attr_id, last_timestep_attr_id, last_var_attr_id;
    bool contains_types, contains_run_attrs, contains_timestep_attrs, contains_var_attrs;


    if(auxiliary_db) {
        string db_name = "file:md" + to_string(job_id) + "?mode=memory&cache=shared";
        rc = sqlite3_open_v2(db_name.c_str(), db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_URI, NULL);
        //debug_log << "metadata_database_init: opening: " << db_name << endl;
        // rc = sqlite3_open("file:memdb1?mode=memory&cache=shared", &db);
    }
    else {
        // rc = sqlite3_open(":memory:", db); 
        rc = sqlite3_open_v2("file::memory:", db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_URI, NULL); 
        // rc = sqlite3_open_v2(":memory:", db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_URI, NULL); 

    }
    if (rc != SQLITE_OK)
    {   
        fprintf (stderr, "Can't open database: %s\n", sqlite3_errmsg (*db));     
        sqlite3_close (*db);
        goto cleanup;
    }

    add_timing_point(DB_SETUP_START);

    //note - doing this creates a locking problem
    // rc = sqlite3_open_v2("file::memory:?cache=shared", db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_URI, NULL);
    // // rc = sqlite3_open(":memory:", db);    
    // if (rc != SQLITE_OK)
    // {   
    //     fprintf (stderr, "Can't open database: %s\n", sqlite3_errmsg (*db));     
    //     sqlite3_close (*db);
    //     goto cleanup;
    // }

    // rc = sqlite3_prepare_v2 (*db, "SELECT name FROM sqlite_master WHERE type='table';",  -1, &stmt, &tail);
    // rc = sqlite3_step (stmt); assert (rc == SQLITE_ROW || rc == SQLITE_DONE);
    // while (rc == SQLITE_ROW) {
    //     string name = (char *)sqlite3_column_text (stmt, 0);
    //     //debug_log << "have already created table: " << name << endl;
    //     rc = sqlite3_step (stmt);
    // }
    // rc = sqlite3_finalize(stmt);

    // rc = get_seq_values(*db, DB_IN_MEM, last_run_id, last_type_id, last_run_attr_id,
    //                 last_timestep_attr_id, last_var_attr_id, contains_types, contains_run_attrs,
    //                 contains_timestep_attrs, contains_var_attrs );

    
    // if(contains_run_attrs) {
    //     //debug_log << "last_run_attr_id: " << last_run_attr_id;
    // }
    // if(contains_timestep_attrs) {
    //     //debug_log <<  " last_timestep_attr_id: " << last_timestep_attr_id;
    // }
    // if(contains_timestep_attrs) {
    //     //debug_log <<  " last_var_attr_id: " << last_var_attr_id;
    // }
    // if(contains_types) {
    //     //debug_log << " last_type_id: " << last_type_id << endl;
    // }
    // //debug_log << " last_run_id: " << last_run_id << " last_timestep_id: " << last_timestep_id << endl;



    txn_id_to_db_connection[job_id] = db_conn(*db);
    //debug_log "metadata_database_init: initting job_id: " << job_id << " to " << *db << endl;

    if(load_db) {
        // rc = sql_load_db(*db, job_id, rank);
        rc = sql_load_db(*db, job_id);
        while (rc == SQLITE_LOCKED || rc == SQLITE_BUSY) {
            if (rc == SQLITE_LOCKED) {
                error_log << "rank: " << proc_rank << " db is locked. am waiting" << endl;
            }
            else {
                 error_log << "rank: " << proc_rank << "db is busy. am waiting" << endl;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        if (rc != SQLITE_OK)
        {   
            fprintf (stderr, "Can't load database for job_id:%llu: %s\n",job_id, sqlite3_errmsg (*db));      
            txn_id_to_db_connection.erase(job_id);
            sqlite3_close (*db);
            goto cleanup;
        }

    }
    else {
        //debug_log << "about to create run_catalog" << endl;

        rc = sqlite3_exec (*db, "create table run_catalog (id integer primary key autoincrement not null, job_id int, name varchar (50) collate nocase, date varchar (50), npx int, npy int, npz int, rank_to_dims_funct varchar (1000), objector_funct varchar (6000))", callback, 0, &ErrMsg);
        if (rc != SQLITE_OK)    {
            fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
            sqlite3_free (ErrMsg);
            txn_id_to_db_connection.erase(job_id);
            sqlite3_close (*db);
            goto cleanup;
        }

        rc = sqlite3_exec (*db, "create table timestep_catalog (id integer not null, run_id int not null, path varchar (50) collate nocase, primary key(id, run_id) )", callback, 0, &ErrMsg);
        if (rc != SQLITE_OK)    {
            fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
            sqlite3_free (ErrMsg);
            txn_id_to_db_connection.erase(job_id);
            sqlite3_close (*db);
            goto cleanup;
        }

        // rc = sqlite3_exec (*db, "create table var_catalog (id integer primary key autoincrement not null, dataset_id int, name varchar (50), path varchar (50), version int, data_size int, num_dims int, d0_min int, d0_max int, d1_min int, d1_max int, d2_min int, d2_max int)", callback, 0, &ErrMsg);
        rc = sqlite3_exec (*db, "create table var_catalog (id integer not null, run_id int not null, timestep_id int not null, name varchar (50) collate nocase, path varchar (50) collate nocase, version int, data_size int, num_dims int, d0_min int, d0_max int, d1_min int, d1_max int, d2_min int, d2_max int, primary key(id, run_id, timestep_id) )", callback, 0, &ErrMsg);
        
        if (rc != SQLITE_OK)    {
            fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
            sqlite3_free (ErrMsg);
            txn_id_to_db_connection.erase(job_id);
            sqlite3_close (*db);
            goto cleanup;
        }

        rc = sqlite3_exec (*db, "create table type_catalog (id integer primary key autoincrement not null, run_id int not null, name varchar (50) collate nocase, version int )", callback, 0, &ErrMsg);
        if (rc != SQLITE_OK)    {
            fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
            sqlite3_free (ErrMsg);
            txn_id_to_db_connection.erase(job_id);
            sqlite3_close (*db);
            goto cleanup;
        }

        rc = sqlite3_exec (*db, "create table run_attribute_catalog (id integer primary key autoincrement not null, run_id int not null, type_id int not null, data_type int, data none )", callback, 0, &ErrMsg);
        if (rc != SQLITE_OK)    {
            fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
            sqlite3_free (ErrMsg);
            txn_id_to_db_connection.erase(job_id);
            sqlite3_close (*db);
            goto cleanup;
        }

        rc = sqlite3_exec (*db, "create table timestep_attribute_catalog (id integer primary key autoincrement not null, run_id not null, timestep_id not null, type_id int not null, data_type int, data none )", callback, 0, &ErrMsg);
        if (rc != SQLITE_OK)    {
            fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
            sqlite3_free (ErrMsg);
            txn_id_to_db_connection.erase(job_id);
            sqlite3_close (*db);
            goto cleanup;
        }

        if(index_type == INDEX_RTREE) {
            rc = sqlite3_exec (*db, "create virtual table var_attribute_catalog using rtree(id, d0_min, d0_max, d1_min, d1_max, d2_min, d2_max, +run_id not null, +timestep_id not null, +type_id int not null, +var_id int not null, +num_dims int not null, +data_type int, +data none )", callback, 0, &ErrMsg);
            if (rc != SQLITE_OK)    {
                fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
                sqlite3_free (ErrMsg);
                txn_id_to_db_connection.erase(job_id);
                sqlite3_close (*db);
                goto cleanup;
            }
        }
        else {
            rc = sqlite3_exec (*db, "create table var_attribute_catalog (id integer primary key autoincrement not null, run_id not null, timestep_id not null, type_id int not null, var_id int not null, num_dims int not null, d0_min int, d0_max int, d1_min int, d1_max int, d2_min int, d2_max int, data_type int, data none )", callback, 0, &ErrMsg);
            if (rc != SQLITE_OK)    {
                fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
                sqlite3_free (ErrMsg);
                txn_id_to_db_connection.erase(job_id);
                sqlite3_close (*db);
                goto cleanup;
            }
        }
        if(index_type != WRITE_DELAYED_INDEX) {
            rc = create_reading_indices(*db);
            if(rc != RC_OK) {
                txn_id_to_db_connection.erase(job_id); 
            }
        }
        
    } //end of if(!load_*db) 
    //makes sure that each temporary/transaction *db uses distinct ids
    //if db_reset, we do not need to adjust the sequence values since this is done in db_reset
   if (open_db_count > 0 && !db_reset) { 
        //debug_log << "am initing the seq start for txn_id: " << job_id << endl;
        uint64_t multiplier = 1000000000;
        uint64_t new_seq_start = multiplier*open_db_count;

        string update_id_query, update_id_query2;

        vector<string> table_seqs_to_update = { "run_catalog", "type_catalog", "run_attribute_catalog", "timestep_attribute_catalog", "var_attribute_catalog"};

        rc = sqlite3_exec (*db, "BEGIN TRANSACTION", callback, 0, &ErrMsg);
        if (rc != SQLITE_OK)    {
            fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
            sqlite3_free (ErrMsg);
            sqlite3_close (*db);
            goto cleanup;
        }

        for(int i = 0; i < table_seqs_to_update.size(); i++) {
            char query[248];
            char query2[248];

            string catalog_name = table_seqs_to_update.at(i);
    

            int n = sprintf(query, "UPDATE SQLITE_SEQUENCE SET seq = %llu WHERE name = '%s'",new_seq_start, catalog_name.c_str());
            //debug_log << "query: " << query << endl;

            rc = sqlite3_exec (*db, query, callback, 0, &ErrMsg);
            if (rc != SQLITE_OK)    {
                fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
                sqlite3_free (ErrMsg);
                sqlite3_close (*db);
                goto cleanup;
            }
            //debug_log << "got here" << endl;

            n = sprintf(query2, "INSERT INTO sqlite_sequence (name,seq) SELECT '%s', %llu WHERE NOT EXISTS "
                   "(SELECT changes() AS change FROM sqlite_sequence WHERE change <> 0)", catalog_name.c_str(), new_seq_start);
            //debug_log << "query2: " << query2 << endl;

            rc = sqlite3_exec (*db, query2, callback, 0, &ErrMsg);
            if (rc != SQLITE_OK)    {
                fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
                sqlite3_free (ErrMsg);
                sqlite3_close (*db);
                goto cleanup;
            }
        }

        rc = sqlite3_exec (*db, "COMMIT", callback, 0, &ErrMsg);
    }


    //debug_log << "metadata_database_init: end db: " << &db << endl;

    open_db_count += 1;

    add_timing_point(DB_SETUP_DONE);

cleanup:
    return rc;
}


static int create_reading_indices(sqlite3 *db)
{
    int rc;
    char  *ErrMsg = NULL;
    add_timing_point(DB_CREATE_READING_INDICES_START);


    // //INDICES/////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // //TXN-ACTIVE/////////////////////////////////////////////////////////////////////////////////////////////////////////////       
    //   rc = sqlite3_exec (db, "create index rc_txn_id_active on run_catalog (txn_id, active)", callback, 0, &ErrMsg);
    //   if (rc != SQLITE_OK)   {
    //       fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
    //       sqlite3_free (ErrMsg);
    // sqlite3_close (db);
    //       goto cleanup;
    //   }

    //   rc = sqlite3_exec (db, "create index tmc_txn_id_active on timestep_attribute_catalog (txn_id, active)", callback, 0, &ErrMsg);
    //   if (rc != SQLITE_OK)   {
    //       fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
    //       sqlite3_free (ErrMsg);
    // sqlite3_close (db);
    //       goto cleanup;
    //   }

    //   rc = sqlite3_exec (db, "create index vc_txn_id_active on var_catalog (txn_id, active)", callback, 0, &ErrMsg);
    //   if (rc != SQLITE_OK)   {
    //       fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
    //       sqlite3_free (ErrMsg);
    // sqlite3_close (db);
    //       goto cleanup;
    //   }


    //   rc = sqlite3_exec (db, "create index tc_txn_id_active on type_catalog (txn_id, active)", callback, 0, &ErrMsg);
    //   if (rc != SQLITE_OK)   {
    //       fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
    //       sqlite3_free (ErrMsg);
    // sqlite3_close (db);
    //       goto cleanup;
    //   }

    //   rc = sqlite3_exec (db, "create index rac_txn_id_active on run_attribute_catalog (txn_id, active)", callback, 0, &ErrMsg);
    //   if (rc != SQLITE_OK)   {
    //       fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
    //       sqlite3_free (ErrMsg);
    // sqlite3_close (db);
    //       goto cleanup;
    //   }

    //   rc = sqlite3_exec (db, "create index tac_txn_id_active on timestep_attribute_catalog (txn_id, active)", callback, 0, &ErrMsg);
    //   if (rc != SQLITE_OK)   {
    //       fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
    //       sqlite3_free (ErrMsg);
    // sqlite3_close (db);
    //       goto cleanup;
    //   }

    //   rc = sqlite3_exec (db, "create index vac_txn_id_active on var_attribute_catalog (txn_id, active)", callback, 0, &ErrMsg);
    //   if (rc != SQLITE_OK)   {
    //       fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
    //       sqlite3_free (ErrMsg);
    // sqlite3_close (db);
    //       goto cleanup;
    //   }

    // //ACTIVE/////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //   rc = sqlite3_exec (db, "create index rc_active on run_catalog (active)", callback, 0, &ErrMsg);
    //   if (rc != SQLITE_OK)   {
    //       fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
    //       sqlite3_free (ErrMsg);
    // sqlite3_close (db);
    //       goto cleanup;
    //   }

    //   rc = sqlite3_exec (db, "create index tmc_active on timestep_catalog (active)", callback, 0, &ErrMsg);
    //   if (rc != SQLITE_OK)   {
    //       fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
    //       sqlite3_free (ErrMsg);
    // sqlite3_close (db);
    //       goto cleanup;
    //   }

    //   rc = sqlite3_exec (db, "create index vc_active on var_catalog (active)", callback, 0, &ErrMsg);
    //   if (rc != SQLITE_OK)   {
    //       fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
    //       sqlite3_free (ErrMsg);
    // sqlite3_close (db);
    //       goto cleanup;
    //   }

    //   rc = sqlite3_exec (db, "create index tc_active on type_catalog (active)", callback, 0, &ErrMsg);
    //   if (rc != SQLITE_OK)   {
    //       fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
    //       sqlite3_free (ErrMsg);
    // sqlite3_close (db);
    //       goto cleanup;
    //   }

    //   rc = sqlite3_exec (db, "create index rac_active on run_attribute_catalog (active)", callback, 0, &ErrMsg);
    //   if (rc != SQLITE_OK)   {
    //       fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
    //       sqlite3_free (ErrMsg);
    // sqlite3_close (db);
    //       goto cleanup;
    //   }

    //   rc = sqlite3_exec (db, "create index tac_active on timestep_attribute_catalog (active)", callback, 0, &ErrMsg);
    //   if (rc != SQLITE_OK)   {
    //       fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
    //       sqlite3_free (ErrMsg);
    // sqlite3_close (db);
    //       goto cleanup;
    //   }

    //   rc = sqlite3_exec (db, "create index vac_active on var_attribute_catalog (active)", callback, 0, &ErrMsg);
    //   if (rc != SQLITE_OK)   {
    //       fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
    //       sqlite3_free (ErrMsg);
    // sqlite3_close (db);
    //       goto cleanup;
    //   }

    //DIMS/////////////////////////////////////////////////////////////////////////////////////////////////////////////

        // rc = sqlite3_exec (db, "create index vac_dims on var_attribute_catalog (d0_min, d0_max, d1_min, d1_max, d2_min, d2_max)", callback, 0, &ErrMsg);
        // if (rc != SQLITE_OK) {
        //   fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
        //   sqlite3_free (ErrMsg);
        // sqlite3_close (db);
        //   goto cleanup;
        // }

    //MISC/////////////////////////////////////////////////////////////////////////////////////////////////////////////
    rc = sqlite3_exec (db, "create index tc_all_cols on type_catalog(run_id, name, version)", callback, 0, &ErrMsg);
    if (rc != SQLITE_OK)    {
        fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
        sqlite3_free (ErrMsg);
        sqlite3_close (db);
        goto cleanup;
    }

    rc = sqlite3_exec (db, "create index rac_all_cols on run_attribute_catalog(run_id, type_id, data_type)", callback, 0, &ErrMsg);
    if (rc != SQLITE_OK)    {
        fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
        sqlite3_free (ErrMsg);
        sqlite3_close (db);
        goto cleanup;
    }

    rc = sqlite3_exec (db, "create index tac_all_cols on timestep_attribute_catalog(type_id, timestep_id, data_type)", callback, 0, &ErrMsg);
    if (rc != SQLITE_OK)    {
        fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
        sqlite3_free (ErrMsg);
        sqlite3_close (db);
        goto cleanup;
    }

    rc = sqlite3_exec (db, "create index vc_run_timestep on var_catalog (timestep_id, run_id)", callback, 0, &ErrMsg);
    if (rc != SQLITE_OK)    {
        fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
        sqlite3_free (ErrMsg);
        sqlite3_close (db);
        goto cleanup;
    }

    // rc = sqlite3_exec (db, "create index vc_timestep on var_catalog (timestep_id)", callback, 0, &ErrMsg);
    // if (rc != SQLITE_OK) {
    //   fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
    //   sqlite3_free (ErrMsg);
    // sqlite3_close (db);
    //   goto cleanup;
    // }

    rc = sqlite3_exec (db, "create index tmc_run on timestep_catalog (run_id)", callback, 0, &ErrMsg);
    if (rc != SQLITE_OK)    {
        fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
        sqlite3_free (ErrMsg);
        sqlite3_close (db);
        goto cleanup;
    }


    if(index_type != INDEX_RTREE) {
        rc = sqlite3_exec (db, "create index vac_all_cols on var_attribute_catalog (run_id, timestep_id, type_id, var_id, d0_min)", callback, 0, &ErrMsg);
        if (rc != SQLITE_OK)    {
            fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
            sqlite3_free (ErrMsg);
            sqlite3_close (db);
            goto cleanup;
        }

        rc = sqlite3_exec (db, "create index vac_type_id_d0 on var_attribute_catalog(type_id, d0_min)", callback, 0, &ErrMsg);
        if (rc != SQLITE_OK)    {
            fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
            sqlite3_free (ErrMsg);
            sqlite3_close (db);
            goto cleanup;
        }

        rc = sqlite3_exec (db, "create index vac_varid_d0 on var_attribute_catalog(var_id, d0_min)", callback, 0, &ErrMsg);
        if (rc != SQLITE_OK)    {
            fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
            sqlite3_free (ErrMsg);
            sqlite3_close (db);
            goto cleanup;
        }
    }
    else {
        // rc = sqlite3_exec (db, "create index vac_all_cols on var_attribute_catalog (timestep_id, type_id, var_id)", callback, 0, &ErrMsg);
        // if (rc != SQLITE_OK)    {
        //     fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
        //     sqlite3_free (ErrMsg);
        // sqlite3_close (db);
        //     goto cleanup;
        // } 

        // rc = sqlite3_exec (db, "create index vac_type_id_d0 on var_attribute_catalog(type_id)", callback, 0, &ErrMsg);
        // if (rc != SQLITE_OK)    {
        //     fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
        //     sqlite3_free (ErrMsg);
        // sqlite3_close (db);
        //     goto cleanup;
        // }

        // rc = sqlite3_exec (db, "create index vac_varid_d0 on var_attribute_catalog(var_id)", callback, 0, &ErrMsg);
        // if (rc != SQLITE_OK)    {
        //     fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
        //     sqlite3_free (ErrMsg);
        // sqlite3_close (db);
        //     goto cleanup;
        // }
    }



    // rc = sqlite3_exec (db, "create index tac_type_id on timestep_attribute_catalog(type_id)", callback, 0, &ErrMsg);
    // if (rc != SQLITE_OK) {
    //   fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
    //   sqlite3_free (ErrMsg);
    // sqlite3_close (db);
    //   goto cleanup;
    // }

     add_timing_point(DB_CREATE_READING_INDICES_DONE);

cleanup:
    return rc;
}

//reminder: you get a performance boost if you register the ops before launching opbox/gutties
static int register_ops () {
    //just needed for the sqlite only transaction test
    // opbox::RegisterOp<OpAbortTransactionMeta>();
    // opbox::RegisterOp<OpBeginTransactionMeta>();
    // opbox::RegisterOp<OpCommitTransactionMeta>();

    opbox::RegisterOp<OpCheckpointDatabaseMeta>();
    opbox::RegisterOp<OpInsertTimestepAttributeBatchMeta>();
    opbox::RegisterOp<OpInsertRunAttributeBatchMeta>();
    opbox::RegisterOp<OpCreateVarBatchMeta>();
    opbox::RegisterOp<OpCreateTypeBatchMeta>();
    opbox::RegisterOp<OpInsertVarAttributeByDimsBatchMeta>();
    opbox::RegisterOp<OpDeleteAllVarsWithSubstrMeta>();
    opbox::RegisterOp<OpCatalogAllVarAttributesWithVarSubstrDimsMeta>();
    opbox::RegisterOp<OpCatalogAllVarAttributesWithTypeVarSubstrDimsMeta>();
    opbox::RegisterOp<OpCatalogAllVarAttributesWithTypeVarSubstrMeta>();
    opbox::RegisterOp<OpCatalogAllVarAttributesWithTypeVarSubstrRangeMeta>();
    opbox::RegisterOp<OpCatalogAllVarAttributesWithTypeVarSubstrDimsRangeMeta>();
    opbox::RegisterOp<OpCatalogAllTypesWithVarAttributesWithVarSubstrInTimestepMeta>();
    opbox::RegisterOp<OpCatalogAllTypesWithVarAttributesWithVarSubstrDimsInTimestepMeta>();
    opbox::RegisterOp<OpCatalogAllTimestepsWithVarSubstrMeta>();
    opbox::RegisterOp<OpCatalogAllTimestepsWithVarAttributesWithTypeVarSubstrRangeMeta>();
    opbox::RegisterOp<OpCatalogAllTimestepsWithVarAttributesWithTypeVarSubstrMeta>();
    opbox::RegisterOp<OpCatalogAllTimestepsWithVarAttributesWithTypeVarSubstrDimsRangeMeta>();
    opbox::RegisterOp<OpCatalogAllTimestepsWithVarAttributesWithTypeVarSubstrDimsMeta>();
    opbox::RegisterOp<OpCatalogAllVarAttributesWithVarSubstrMeta>();
    opbox::RegisterOp<OpProcessingMeta>();
    // opbox::RegisterOp<OpActivateMeta>();
    opbox::RegisterOp<OpActivateRunAttributeMeta>();
    opbox::RegisterOp<OpActivateTimestepAttributeMeta>();
    opbox::RegisterOp<OpActivateTimestepMeta>();
    opbox::RegisterOp<OpActivateVarMeta>();
    opbox::RegisterOp<OpActivateRunMeta>();
    opbox::RegisterOp<OpActivateTypeMeta>();  
    opbox::RegisterOp<OpActivateVarAttributeMeta>();
    opbox::RegisterOp<OpCatalogAllVarAttributesWithTypeVarDimsRangeMeta>();
    opbox::RegisterOp<OpCatalogAllVarAttributesWithTypeVarRangeMeta>();
    opbox::RegisterOp<OpCatalogAllTimestepAttributesWithTypeRangeMeta>();
    opbox::RegisterOp<OpCatalogAllTimestepsWithVarAttributesWithTypeVarDimsRangeMeta>();
    opbox::RegisterOp<OpCatalogAllTimestepsWithVarAttributesWithTypeVarRangeMeta>();
    opbox::RegisterOp<OpCatalogAllRunAttributesWithTypeRangeMeta>();
    opbox::RegisterOp<OpCatalogAllTimestepAttributesWithTypeMeta>();
    opbox::RegisterOp<OpCatalogAllTypesWithVarAttributesWithVarDimsInTimestepMeta>();
    opbox::RegisterOp<OpCatalogAllTypesWithVarAttributesWithVarInTimestepMeta>();
    opbox::RegisterOp<OpCatalogAllTypesWithVarAttributesInTimestepMeta>();
    opbox::RegisterOp<OpCatalogAllTimestepsWithVarMeta>();
    opbox::RegisterOp<OpCatalogAllTimestepsWithVarAttributesWithTypeVarDimsMeta>();
    opbox::RegisterOp<OpCatalogAllTimestepsWithVarAttributesWithTypeVarMeta>();
    opbox::RegisterOp<OpCatalogAllRunAttributesWithTypeMeta>();
    opbox::RegisterOp<OpCatalogAllTimestepAttributesMeta>();
    opbox::RegisterOp<OpCatalogAllRunAttributesMeta>();
    opbox::RegisterOp<OpDeleteTimestepByIdMeta>();
    opbox::RegisterOp<OpCatalogTimestepMeta>();
    opbox::RegisterOp<OpInsertTimestepAttributeMeta>();
    opbox::RegisterOp<OpInsertRunAttributeMeta>();
    opbox::RegisterOp<OpCreateTimestepMeta>();
    opbox::RegisterOp<OpDeleteVarByNamePathVerMeta>();
    opbox::RegisterOp<OpDeleteVarByIdMeta>();
    opbox::RegisterOp<OpDeleteTypeByNameVerMeta>();
    opbox::RegisterOp<OpDeleteTypeByIdMeta>();
    opbox::RegisterOp<OpDeleteRunByIdMeta>();
    opbox::RegisterOp<OpCatalogAllVarAttributesWithVarDimsByNameVerMeta>();
    opbox::RegisterOp<OpCatalogAllVarAttributesWithVarDimsByIdMeta>();
    opbox::RegisterOp<OpCatalogAllVarAttributesWithVarByNameVerMeta>();
    opbox::RegisterOp<OpCatalogAllVarAttributesWithVarByIdMeta>();
    opbox::RegisterOp<OpCatalogAllVarAttributesWithTypeVarDimsByNameVerMeta>();
    opbox::RegisterOp<OpCatalogAllVarAttributesWithTypeVarDimsByIdMeta>();
    opbox::RegisterOp<OpCatalogAllVarAttributesWithTypeVarByNameVerMeta>();
    opbox::RegisterOp<OpCatalogAllVarAttributesWithTypeVarByIdMeta>();
    opbox::RegisterOp<OpCatalogAllVarAttributesWithTypeDimsByNameVerMeta>();
    opbox::RegisterOp<OpCatalogAllVarAttributesWithTypeDimsByIdMeta>();
    opbox::RegisterOp<OpCatalogAllVarAttributesWithTypeByNameVerMeta>();
    opbox::RegisterOp<OpCatalogAllVarAttributesWithTypeByIdMeta>();
    opbox::RegisterOp<OpCatalogAllVarAttributesWithDimsMeta>();
    opbox::RegisterOp<OpCatalogAllVarAttributesMeta>();
    opbox::RegisterOp<OpCatalogRunMeta>();
    opbox::RegisterOp<OpCatalogTypeMeta>(); 
    opbox::RegisterOp<OpCatalogVarMeta>();
    opbox::RegisterOp<OpCreateRunMeta>();
    opbox::RegisterOp<OpCreateTypeMeta>();
    opbox::RegisterOp<OpCreateVarMeta>();
    opbox::RegisterOp<OpFullShutdownMeta>();  
    opbox::RegisterOp<OpInsertVarAttributeByDimsMeta>();

    return RC_OK;
}


static bool is_db_output_start(uint16_t code)
{
    return ( DB_COMPACT_DONE < code && code < OP_ACTIVATE_RUN_START && code%2 == 0 );
}



static int sql_load_db(sqlite3 *db, uint64_t job_id)
{
    int rc;
    sqlite3 *pFile;        /* Database connection  */
    sqlite3_backup *pBackup;  /* Backup object used to copy data */

    add_timing_point(DB_LOAD_START);

    string filename = to_string(job_id) + "_" + to_string(proc_rank);
    /* Open the database file. Exit early if this fails
      ** for any reason. */
    rc = sqlite3_open(filename.c_str(), &pFile);
    if( rc==SQLITE_OK ){  
        /* Set up the backup procedure to copy from the the main database of connection db
        to the "main" database of the connection pFile.
        ** If something goes wrong, pBackup will be set to NULL and an error
        ** code and message left in connection pFile.
        */
        // pBackup = sqlite3_backup_init(pTo, "main", pFrom, "main");
        pBackup = sqlite3_backup_init(db, "main", pFile, "main");

        /* If the backup object is successfully created, call backup_step()
        ** to copy data from pInMemory to pFile . Then call backup_finish()
        ** to release resources associated with the pBackup object.  If an
        ** error occurred, then an error code and message will be left in
        ** connection pTo. If no error occurred, then the error code belonging
        ** to pFile is set to SQLITE_OK.
        */
        if( pBackup ){
          (void)sqlite3_backup_step(pBackup, -1);
          (void)sqlite3_backup_finish(pBackup);
        }
        // rc = sqlite3_errcode(pTo);
        rc = sqlite3_errcode(db);
    }

    /* Close the database connection opened on database file zFilename
    ** and return the result of this function. */
    (void)sqlite3_close(pFile);

    add_timing_point(DB_LOAD_DONE);

    return rc;

}

static int db_copy(sqlite3 *db, string filename)
{
    int rc;
    sqlite3 *pFile;        /* Database connection  */
    sqlite3_backup *pBackup;  /* Backup object used to copy data */

     // string filename = to_string(job_id) + "_" + to_string(proc_rank);
    /* Open the database file. Exit early if this fails
      ** for any reason. */
    rc = sqlite3_open(filename.c_str(), &pFile);
    // //debug_log << "just opened file: " << filename << " for checkpointing" << endl;
    if( rc==SQLITE_OK ){  
        /* Set up the backup procedure to copy from the the main database of connection pInMemory
        to the "main" database of the connection pFile.
        ** If something goes wrong, pBackup will be set to NULL and an error
        ** code and message left in connection pFile.
        */
        // pBackup = sqlite3_backup_init(pTo, "main", pFrom, "main");
        pBackup = sqlite3_backup_init(pFile, "main", db, "main");


        /* If the backup object is successfully created, call backup_step()
        ** to copy data from pInMemory to pFile . Then call backup_finish()
        ** to release resources associated with the pBackup object.  If an
        ** error occurred, then an error code and message will be left in
        ** connection pTo. If no error occurred, then the error code belonging
        ** to pFile is set to SQLITE_OK.
        */
        if( pBackup ){
          (void)sqlite3_backup_step(pBackup, -1);
          (void)sqlite3_backup_finish(pBackup);
        }
        // rc = sqlite3_errcode(pTo);
        rc = sqlite3_errcode(pFile);
    }

    /* Close the database connection opened on database file zFilename
    ** and return the result of this function. */
    (void)sqlite3_close(pFile);

    return rc;
}


// // void get_mem_usage(double& vm_usage, double& resident_set)
// void get_mem_usage(double& resident_set)
// {
//  // vm_usage  = 0.0;
//  resident_set = 0.0;

//  // 'file' stat seems to give the most reliable results
//  //
//  ifstream stat_stream("/proc/self/stat",ios_base::in);

//  // dummy vars for leading entries in stat that we don't care about
//  //
//  string pid, comm, state, ppid, pgrp, session, tty_nr;
//  string tpgid, flags, minflt, cminflt, majflt, cmajflt;
//  string utime, stime, cutime, cstime, priority, nice;
//  string O, itrealvalue, starttime;

//  // the two fields we want
//  //
//  unsigned long vsize;
//  long rss;

//  stat_stream >> pid >> comm >> state >> ppid >> pgrp >> session >> tty_nr
//              >> tpgid >> flags >> minflt >> cminflt >> majflt >> cmajflt
//              >> utime >> stime >> cutime >> cstime >> priority >> nice
//              >> O >> itrealvalue >> starttime >> vsize >> rss; // don't care about the rest

//  stat_stream.close();

//  // long page_size_kb = sysconf(_SC_PAGE_SIZE) / 1024; // in case x86-64 is configured to use 2MB pages
//  long page_size = sysconf(_SC_PAGE_SIZE) ; // in case x86-64 is configured to use 2MB pages
//  // vm_usage  = vsize / 1024.0;
//  // resident_set = rss * page_size_kb;
//  resident_set = rss * page_size;
// }

// uint64_t get_mem_usage()
// {
//   struct rusage r_usage;
//   getrusage(RUSAGE_SELF,&r_usage);
//   // Print the maximum resident r_usage.ru_maxrss*1000  <set size used (in bytes).
//   cout << "mem_usage: " << r_usage.ru_maxrss*1000 << " db_size: " << get_db_size() << " alloc: " << sqlite3_memory_used() << endl;
        
//   // printf("Usage ix: %ld\n", r_usage.ru_ixrss);
//   // printf("Usage is: %ld\n", r_usage.ru_isrss);
//   // printf("Usage id: %ld\n", r_usage.ru_idrss);

//   // double vm_usage, resident_set;
//   // get_mem_usage(vm_usage, resident_set);
//   //debug_log << "vm_usage: " << vm_usage << endl;
//   //debug_log << "resident_set: " << resident_set << endl;

//   double resident_set;
//   get_mem_usage(resident_set);
//   cout << "resident_set: " << resident_set << endl;

//  //  struct sysinfo info;

//  // sysinfo(&info);

//  //  cout << "totalram: " << info.totalram << endl;
//  //  cout << "freeram: " << info.freeram << endl;
//  //  cout << "mem_unit: " << info.mem_unit << endl;

//  // //    struct sysinfo {
//  // //        long uptime;            /* Seconds since boot */
//  // //        unsigned long loads[3];  /* 1, 5, and 15 minute load averages */
//  // //        unsigned long totalram;  /* Total usable main memory size */
//  // //        unsigned long freeram;   /* Available memory size */
//  // //        unsigned long sharedram; /* Amount of shared memory */
//  // //        unsigned long bufferram; /* Memory used by buffers */
//  // //        unsigned long totalswap;  Total swap space size 
//  // //        unsigned long freeswap;  /* swap space still available */
//  // //        unsigned short procs;  /* Number of current processes */
//  // //        unsigned long totalhigh; /* Total high memory size */
//  // //        unsigned long freehigh;  /* Available high memory size */
//  // //        unsigned int mem_unit;   /* Memory unit size in bytes */
//  // //        char _f[20-2*sizeof(long)-sizeof(int)]; /* Padding to 64 bytes */
//  // //    };

//  // // and the sizes are given as multiples of mem_unit bytes.

//   // return (r_usage.ru_maxrss*1000 );
//   return resident_set;
// }



int db_checkpoint_copy(uint64_t job_id)
{
    int rc;

    //debug_log "about to start outputting the db" << endl;

    //debug_log << "about to start outputting the db" << endl;
    //we want both the timing and the amount of memory used by the DB, so we can see
    //how the size affects the time
    uint64_t db_size = get_db_size(main_db);
    add_db_output_timing_point(DB_CHECKPT_COPY_START, db_size);

    if(first_checkpt) {
        checkpt_count += 1;
        first_checkpt = false;
    }
    string filename = to_string(job_id) + "_" + to_string(proc_rank) + "_" + to_string(checkpt_count);

    rc = db_copy(main_db, filename);

    //debug_log << "mem highwater: " << sqlite3_memory_highwater(true) << endl;

    // if(get_mem_usage() > MAX_MEM_USAGE) {
    if(db_size > MAX_DB_SIZE) {
        //debug_log "going to reset the db" << endl;
        // get_mem_usage();
        rc = db_reset(&main_db, job_id);
        if(rc != RC_OK) {
            error_log << "error resetting the db in db_checkpoint_copy" << endl;
            return rc;
        }
        // checkpt_count += 1;
        first_checkpt = true;
        rc = RC_DB_RESET;
    }

    add_timing_point(DB_CHECKPT_COPY_DONE);

    return rc;

}

//question - would we be better off just making a second write of all md -> writing it also to a "yet to be committed" db?

// int sql_output_incremental_db(uint64_t job_id, int rank)
int db_checkpoint_incremental_output(uint64_t job_id)

{
    int rc;
    uint64_t db_size =  get_db_size(main_db);
    add_db_output_timing_point(DB_CHECKPT_INCREMENTAL_OUTPUT_START, db_size);

    //debug_log << "about to start outputing new db" << endl;

    if(first_checkpt) {
        checkpt_count += 1;
        string filename = to_string(job_id) + "_" + to_string(proc_rank) + "_incr_" + to_string(checkpt_count);

        rc = db_copy(main_db, filename);
        first_checkpt = false;
        //debug_log << "just did original output. rc: " << rc << endl;
    }
    else {
        string filename = to_string(job_id) + "_" + to_string(proc_rank) + "_incr_" + to_string(checkpt_count);
        //debug_log << "about to do incr_output for filename: " << filename << endl;
        rc = db_incr_output(main_db, filename);
        if(rc != RC_OK) {
            txn_id_to_db_connection.erase(job_id);
            return rc;
        }
        //debug_log << "just did incremental output. rc: " << rc << endl;
    }

    // if(get_mem_usage() > MAX_MEM_USAGE) {
    if(db_size > MAX_DB_SIZE) {
        // get_mem_usage();
        rc = db_reset(&main_db, job_id);
        if(rc != RC_OK) {
            error_log << "error resetting the db in db_checkpoint_copy" << endl;
            return rc;
        }
        // checkpt_count += 1;
        first_checkpt = true;

        rc = RC_DB_RESET;
    }

    // //debug_log << "about to return" << endl;

    add_timing_point(DB_CHECKPT_INCREMENTAL_OUTPUT_DONE);

    return rc;

}

int db_checkpoint_copy_and_delete(uint64_t job_id)
{

    add_db_output_timing_point(DB_CHECKPT_COPY_AND_DELETE_START, get_db_size(main_db));
    int rc;
    checkpt_count += 1;

    string filename = to_string(job_id) + "_" + to_string(proc_rank) + "_del_" + to_string(checkpt_count);

    rc = db_copy(main_db, filename);
    if(rc != RC_OK) {
        error_log << "error copying the db in db_checkpoint_copy_and_delete" << endl;
        return rc;
    }
    rc = db_delete(main_db);
    if(rc != RC_OK) {
        error_log << "error in db_delete in db_checkpoint_copy_and_delete" << endl;
        txn_id_to_db_connection.erase(job_id);
        return rc;
    }

    add_timing_point(DB_CHECKPT_COPY_AND_DELETE_DONE);

    return rc;
};

int db_checkpoint_copy_and_reset(uint64_t job_id)
{   
    add_db_output_timing_point(DB_CHECKPT_COPY_AND_RESET_START, get_db_size(main_db));

    int rc;
    checkpt_count += 1;

    string filename = to_string(job_id) + "_" + to_string(proc_rank) + "_reset_" + to_string(checkpt_count);


    rc = db_copy(main_db, filename);
    if(rc != RC_OK) {
        error_log << "error copying the db in db_checkpoint_copy_and_delete" << endl;
        return rc;
    }

    rc = db_reset(&main_db, job_id);
    if(rc != RC_OK) {
        error_log << "error resetting the db in db_checkpoint_copy_and_delete" << endl;
        return rc;
    }
    
    add_timing_point(DB_CHECKPT_COPY_AND_RESET_DONE);
    return rc;

};

int db_checkpoint_incremental_output_and_delete(uint64_t job_id)
{
    add_db_output_timing_point(DB_CHECKPT_INCREMENTAL_OUTPUT_AND_DELETE_START, get_db_size(main_db));

    int rc;
    // string filename = to_string(job_id) + "_" + to_string(proc_rank) + "_incr_del_" + to_string(checkpt_count);
    string filename = to_string(job_id) + "_" + to_string(proc_rank) + "_incr_del_0";

    if(first_checkpt) {
        rc = db_copy(main_db, filename);
        first_checkpt = false;
        //debug_log << "just did original output. rc: " << rc << endl;
    }
    else {
        rc = db_incr_output_delete(main_db, filename);
        if(rc != RC_OK) {
            txn_id_to_db_connection.erase(job_id);
            return rc;
        }
        //debug_log << "just did incremental output. rc: " << rc << endl;
    }
    rc = db_delete(main_db);
    if(rc != RC_OK) {
        error_log << "error in db_delete in db_checkpoint_incremental_output_and_delete" << endl;
        txn_id_to_db_connection.erase(job_id);
        return rc;
    }

    add_timing_point(DB_CHECKPT_INCREMENTAL_OUTPUT_AND_DELETE_DONE);
    return rc;

};

int db_checkpoint_incremental_output_and_reset(uint64_t job_id)
{   
    add_db_output_timing_point(DB_CHECKPT_INCREMENTAL_OUTPUT_AND_RESET_START, get_db_size(main_db));

    int rc;
    // string filename = to_string(job_id) + "_" + to_string(proc_rank) + "_incr_reset_" + to_string(checkpt_count);
    string filename = to_string(job_id) + "_" + to_string(proc_rank) + "_incr_reset_0";

    if(first_checkpt) {
        rc = db_copy(main_db, filename);
        first_checkpt = false;
        //debug_log << "just did original output. rc: " << rc << endl;
    }
    else {
        rc = db_incr_output_delete(main_db, filename);
        if(rc != RC_OK) {
            txn_id_to_db_connection.erase(job_id);
            return rc;
        }
        //debug_log << "just did incremental output. rc: " << rc << endl;
    }
    rc = db_reset(&main_db, job_id);
    if(rc != RC_OK) {
        error_log << "error copying the db in db_checkpoint_copy_and_delete" << endl;
        return rc;
    }

    add_timing_point(DB_CHECKPT_INCREMENTAL_OUTPUT_AND_RESET_DONE);
    return rc;
};



///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

//note: these are exactly the same as the one above except that they fetch the db for the job_id
//instead of just using main db. if we want to use these, need to make sure to set main db
//whenver database_init is called
// int db_checkpoint_copy(uint64_t job_id)
// {
//     int rc;

//     //debug_log << "about to start outputting the db" << endl;

//     //debug_log << "about to start outputting the db" << endl;
//     sqlite3 *db = txn_id_to_db_connection[job_id].db;
//     //we want both the timing and the amount of memory used by the DB, so we can see
//     //how the size affects the time
//     uint64_t db_size = get_db_size(db);
//     add_db_output_timing_point(DB_CHECKPT_COPY_START, db_size);

//     if(first_checkpt) {
//         checkpt_count += 1;
//         first_checkpt = false;
//     }
//     string filename = to_string(job_id) + "_" + to_string(proc_rank) + "_" + to_string(checkpt_count);

//     rc = db_copy(db, filename);

//     //debug_log << "mem highwater: " << sqlite3_memory_highwater(true) << endl;

//     // if(get_mem_usage() > MAX_MEM_USAGE) {
//     if(db_size > MAX_DB_SIZE) {
//         //debug_log << "going to reset the db" << endl;
//         // get_mem_usage();
//         rc = db_reset(db, job_id);
//         if(rc != RC_OK) {
//             error_log << "error resetting the db in db_checkpoint_copy" << endl;
//             return rc;
//         }
//         // checkpt_count += 1;
//         first_checkpt = true;
//         rc = RC_DB_RESET;
//     }

//     add_timing_point(DB_CHECKPT_COPY_DONE);

//     return rc;

// }

// //question - would we be better off just making a second write of all md -> writing it also to a "yet to be committed" db?

// // int sql_output_incremental_db(uint64_t job_id, int rank)
// int db_checkpoint_incremental_output(uint64_t job_id)

// {
//     int rc;
//     sqlite3 *db = txn_id_to_db_connection[job_id].db;
//     uint64_t db_size =  get_db_size(db);
//     add_db_output_timing_point(DB_CHECKPT_INCREMENTAL_OUTPUT_START, db_size);

//     //debug_log << "about to start outputing new db" << endl;

//     if(first_checkpt) {
//         checkpt_count += 1;
//         string filename = to_string(job_id) + "_" + to_string(proc_rank) + "_incr_" + to_string(checkpt_count);

//         rc = db_copy(db, filename);
//         first_checkpt = false;
//         //debug_log << "just did original output. rc: " << rc << endl;
//     }
//     else {
//         string filename = to_string(job_id) + "_" + to_string(proc_rank) + "_incr_" + to_string(checkpt_count);
//         //debug_log << "about to do incr_output for filename: " << filename << endl;
//         rc = db_incr_output(db, filename);
//         if(rc != RC_OK) {
//             txn_id_to_db_connection.erase(job_id);
//             return rc;
//         }
//         //debug_log << "just did incremental output. rc: " << rc << endl;
//     }

//     // if(get_mem_usage() > MAX_MEM_USAGE) {
//     if(db_size > MAX_DB_SIZE) {
//         // get_mem_usage();
//         rc = db_reset(db, job_id);
//         if(rc != RC_OK) {
//             error_log << "error resetting the db in db_checkpoint_copy" << endl;
//             return rc;
//         }
//         // checkpt_count += 1;
//         first_checkpt = true;

//         rc = RC_DB_RESET;
//     }

//     // //debug_log << "about to return" << endl;

//     add_timing_point(DB_CHECKPT_INCREMENTAL_OUTPUT_DONE);

//     return rc;

// }

// int db_checkpoint_copy_and_delete(uint64_t job_id)
// {

//     sqlite3 *db = txn_id_to_db_connection[job_id].db;
//     add_db_output_timing_point(DB_CHECKPT_COPY_AND_DELETE_START, get_db_size(db));
//     int rc;
//     checkpt_count += 1;

//     string filename = to_string(job_id) + "_" + to_string(proc_rank) + "_del_" + to_string(checkpt_count);

//     rc = db_copy(db, filename);
//     if(rc != RC_OK) {
//         error_log << "error copying the db in db_checkpoint_copy_and_delete" << endl;
//         return rc;
//     }
//     rc = db_delete(db);
//     if(rc != RC_OK) {
//         error_log << "error in db_delete in db_checkpoint_copy_and_delete" << endl;
//         txn_id_to_db_connection.erase(job_id);
//         return rc;
//     }

//     add_timing_point(DB_CHECKPT_COPY_AND_DELETE_DONE);

//     return rc;
// };

// int db_checkpoint_copy_and_reset(uint64_t job_id)
// {   
//     sqlite3 *db = txn_id_to_db_connection[job_id].db;
//     add_db_output_timing_point(DB_CHECKPT_COPY_AND_RESET_START, get_db_size(db));

//     int rc;
//     checkpt_count += 1;

//     string filename = to_string(job_id) + "_" + to_string(proc_rank) + "_reset_" + to_string(checkpt_count);


//     rc = db_copy(db, filename);
//     if(rc != RC_OK) {
//         error_log << "error copying the db in db_checkpoint_copy_and_delete" << endl;
//         return rc;
//     }

//     rc = db_reset(db, job_id);
//     if(rc != RC_OK) {
//         error_log << "error resetting the db in db_checkpoint_copy_and_delete" << endl;
//         return rc;
//     }
    
//     add_timing_point(DB_CHECKPT_COPY_AND_RESET_DONE);
//     return rc;

// };

// int db_checkpoint_incremental_output_and_delete(uint64_t job_id)
// {
//     sqlite3 *db = txn_id_to_db_connection[job_id].db;
//     add_db_output_timing_point(DB_CHECKPT_INCREMENTAL_OUTPUT_AND_DELETE_START, get_db_size(db));

//     int rc;
//     // string filename = to_string(job_id) + "_" + to_string(proc_rank) + "_incr_del_" + to_string(checkpt_count);
//     string filename = to_string(job_id) + "_" + to_string(proc_rank) + "_incr_del_0";

//     if(first_checkpt) {
//         rc = db_copy(db, filename);
//         first_checkpt = false;
//         //debug_log << "just did original output. rc: " << rc << endl;
//     }
//     else {
//         rc = db_incr_output_delete(db, filename);
//         if(rc != RC_OK) {
//             txn_id_to_db_connection.erase(job_id);
//             return rc;
//         }
//         //debug_log << "just did incremental output. rc: " << rc << endl;
//     }
//     rc = db_delete(db);
//     if(rc != RC_OK) {
//         error_log << "error in db_delete in db_checkpoint_incremental_output_and_delete" << endl;
//         txn_id_to_db_connection.erase(job_id);
//         return rc;
//     }

//     add_timing_point(DB_CHECKPT_INCREMENTAL_OUTPUT_AND_DELETE_DONE);
//     return rc;

// };

// int db_checkpoint_incremental_output_and_reset(uint64_t job_id)
// {   
//     sqlite3 *db = txn_id_to_db_connection[job_id].db;
//     add_db_output_timing_point(DB_CHECKPT_INCREMENTAL_OUTPUT_AND_RESET_START, get_db_size(db));

//     int rc;
//     // string filename = to_string(job_id) + "_" + to_string(proc_rank) + "_incr_reset_" + to_string(checkpt_count);
//     string filename = to_string(job_id) + "_" + to_string(proc_rank) + "_incr_reset_0";

//     if(first_checkpt) {
//         rc = db_copy(db, filename);
//         first_checkpt = false;
//         //debug_log << "just did original output. rc: " << rc << endl;
//     }
//     else {
//         rc = db_incr_output_delete(db, filename);
//         if(rc != RC_OK) {
//             txn_id_to_db_connection.erase(job_id);
//             return rc;
//         }
//         //debug_log << "just did incremental output. rc: " << rc << endl;
//     }
//     rc = db_reset(db, job_id);
//     if(rc != RC_OK) {
//         error_log << "error copying the db in db_checkpoint_copy_and_delete" << endl;
//         return rc;
//     }

//     add_timing_point(DB_CHECKPT_INCREMENTAL_OUTPUT_AND_RESET_DONE);
//     return rc;
// };
///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

// int sql_output_incremental_db_new_file(uint64_t job_id)

// {
//   int rc;

//   add_db_output_timing_point(DB_OUTPUT_INCREMENTAL_NEW_FILE_START, get_db_size());

//  string output_filename = to_string(job_id) + "_" + to_string(proc_rank) + "_incr_" + to_string(checkpt_count);

//   //debug_log << "about to start outputing new db incrementally to a new file" << endl;
//   //debug_log << "output_filename: " << output_filename << endl;

//   if(first_checkpt) {
//      rc = do_db_checkpoint(output_filename);
//      first_checkpt = false;
//      //debug_log << "just did original output. rc: " << rc << endl;
//   }
//   else {
//      string prev_filename = to_string(job_id) + "_" + to_string(proc_rank) + "_incr_" + to_string(checkpt_count-1);
//      //debug_log << "prev_filename: " << prev_filename << endl;

//      rc = db_incr_output_new_file(output_filename, prev_filename);
//      //debug_log << "just did incremental output. rc: " << rc << endl;
//   }

//  // //debug_log << "about to return" << endl;
//  checkpt_count += 1;

//   // add_db_output_timing_point(DB_OUTPUT_INCREMENTAL_DONE, get_db_size());

//   return rc;

// }


static int db_incr_output(sqlite3 *db, string filename)
{
    int rc;
    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;

    char  *ErrMsg = NULL;

    uint64_t last_run_attr_id, last_timestep_attr_id, last_timestep_id, last_run_id, last_type_id;
    uint64_t last_var_attr_id;
    bool contains_run_attrs = false, contains_timestep_attrs = false, contains_var_attrs = false, contains_types = false;

    //note - we are going to assume for now that we are only writing 1 run at a time
    //keep track of the last run attr id, the last timestep attr id, and the last timestep number for the given run

    // timestep_id = (int) sqlite3_last_insert_rowid (db);


    // string filename_temp = to_string(job_id) + "_" + to_string(proc_rank) + "_incr";

    rc = sqlite3_prepare_v2 (db, "attach database ? as db_output", -1, &stmt, &tail);
    if (rc != SQLITE_OK)
    {
        fprintf (stderr, "Error start of attach db: Line: %d SQL error: %s\n", __LINE__, sqlite3_errmsg (db));
        sqlite3_close (db);
        return rc;
    }
    rc = sqlite3_bind_text (stmt, 1, strdup(filename.c_str()), -1, free); assert (rc == SQLITE_OK);
    rc = sqlite3_step (stmt); assert (rc == SQLITE_ROW || rc == SQLITE_DONE);

    rc = sqlite3_finalize(stmt); assert (rc == SQLITE_OK);

    rc = sqlite3_exec (db, "begin;", callback, 0, &ErrMsg);
    if (rc != SQLITE_OK)
    {
        fprintf (stderr, "Error begin db_delete Line: %d SQL error: %s\n", __LINE__, sqlite3_errmsg (db));
        sqlite3_free (ErrMsg);
        sqlite3_close (db);
        return rc;
    }

    rc = get_seq_values(db, DB_OUTPUT, last_run_id, last_type_id, last_run_attr_id,
                    last_timestep_attr_id, last_var_attr_id, contains_types, contains_run_attrs,
                    contains_timestep_attrs, contains_var_attrs );

    rc = get_last_timestep(db, DB_OUTPUT, last_run_id, last_timestep_id);

    rc = insert_new(db, last_run_id, last_timestep_id, last_type_id,
                    last_run_attr_id, last_timestep_attr_id, last_var_attr_id,
                    contains_types, contains_run_attrs, contains_timestep_attrs, contains_var_attrs
                    );

    rc = sqlite3_exec (db, "end;", callback, 0, &ErrMsg);
    if (rc != SQLITE_OK)
    {
        fprintf (stderr, "Error begin db_delete Line: %d SQL error: %s\n", __LINE__, sqlite3_errmsg (db));
        sqlite3_free (ErrMsg);
        sqlite3_close (db);
        return rc;
    }
    rc = sqlite3_exec (db, "detach database db_output", callback, 0, &ErrMsg);
    if (rc != SQLITE_OK)    {
        fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
        sqlite3_free (ErrMsg);
        sqlite3_close (db);
        return rc;
    }
    
    // if(contains_run_attrs) {
    //     //debug_log << "last_run_attr_id: " << last_run_attr_id;
    // }
    // if(contains_timestep_attrs) {
    //     //debug_log <<  " last_timestep_attr_id: " << last_timestep_attr_id;
    // }
    // if(contains_timestep_attrs) {
    //     //debug_log <<  " last_var_attr_id: " << last_var_attr_id;
    // }
    // if(contains_types) {
    //     //debug_log << " last_type_id: " << last_type_id << endl;
    // }
    // //debug_log << " last_run_id: " << last_run_id << " last_timestep_id: " << last_timestep_id << endl;

    return rc;

}

// static int db_incr_output_new_file(sqlite3 *db, string output_filename, string prev_filename)
// {
//     int rc;
//     sqlite3_stmt * stmt = NULL;
//     const char * tail = NULL;

//     char  *ErrMsg = NULL;

//     uint64_t last_run_attr_id, last_timestep_attr_id, last_timestep_id, last_run_id, last_type_id;
//     uint64_t last_var_attr_id;
//     bool contains_run_attrs = false, contains_timestep_attrs = false, contains_var_attrs = false, contains_types = false;

//     //note - we are going to assume for now that we are only writing 1 run at a time
//     //keep track of the last run attr id, the last timestep attr id, and the last timestep number for the given run

//     // timestep_id = (int) sqlite3_last_insert_rowid (db);


//     // string filename_temp = to_string(job_id) + "_" + to_string(proc_rank) + "_incr";

//     rc = sqlite3_prepare_v2 (db, "attach database ? as db_prev", -1, &stmt, &tail);
//     if (rc != SQLITE_OK)
//     {
//         fprintf (stderr, "Error start of attach db: Line: %d SQL error: %s\n", __LINE__, sqlite3_errmsg (db));
//         sqlite3_close (db);
//         return rc;
//     }
//     rc = sqlite3_bind_text (stmt, 1, strdup(prev_filename.c_str()), -1, free); assert (rc == SQLITE_OK);
//     rc = sqlite3_step (stmt); assert (rc == SQLITE_ROW || rc == SQLITE_DONE);
//     rc = sqlite3_finalize(stmt); assert (rc == SQLITE_OK);

//     //note - if we were comfortable keeping all of these as global vars
//     rc = get_seq_values(DB_PREV, last_run_id, last_type_id, last_run_attr_id,
//                     last_timestep_attr_id, last_var_attr_id, contains_types, contains_run_attrs,
//                     contains_timestep_attrs, contains_var_attrs );

//     rc = get_last_timestep(DB_PREV, last_run_id, last_timestep_id);
//     //debug_log << "last_timestep_id: " << last_timestep_id << endl;


//     rc = sqlite3_exec (db, "detach database db_prev", callback, 0, &ErrMsg);
//     if (rc != SQLITE_OK)    {
//         fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
//         sqlite3_free (ErrMsg);
//         sqlite3_close (db);
//         return rc;
//     }
//     //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//     //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//     // INSERT INTO X.TABLE SELECT * FROM Y.TABLE;

//     //need to output run attrs, timestep attrs, new timesteps, vars for new timesteps, new var attrs, types

//     rc = sqlite3_prepare_v2 (db, "attach database ? as db_output", -1, &stmt, &tail);
//     if (rc != SQLITE_OK)
//     {
//         fprintf (stderr, "Error start of attach db: Line: %d SQL error: %s\n", __LINE__, sqlite3_errmsg (db));
//         sqlite3_close (db);
//         return rc;
//     }
//     rc = sqlite3_bind_text (stmt, 1, strdup(output_filename.c_str()), -1, free); assert (rc == SQLITE_OK);
//     rc = sqlite3_step (stmt); assert (rc == SQLITE_ROW || rc == SQLITE_DONE);
//     rc = sqlite3_finalize(stmt); assert (rc == SQLITE_OK);


//     rc = insert_new(last_run_id, last_timestep_id, last_type_id,
//                     last_run_attr_id, last_timestep_attr_id, last_var_attr_id,
//                     contains_types, contains_run_attrs, contains_timestep_attrs, contains_var_attrs
//                     );


//     rc = sqlite3_exec (db, "detach database db_output", callback, 0, &ErrMsg);
//     if (rc != SQLITE_OK)    {
//         fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
//         sqlite3_free (ErrMsg);
//         sqlite3_close (db);
//         return rc;
//     }
    
//     // if(contains_run_attrs) {
//     //     //debug_log << "last_run_attr_id: " << last_run_attr_id;
//     // }
//     // if(contains_timestep_attrs) {
//     //     //debug_log <<  " last_timestep_attr_id: " << last_timestep_attr_id;
//     // }
//     // if(contains_timestep_attrs) {
//     //     //debug_log <<  " last_var_attr_id: " << last_var_attr_id;
//     // }
//     // if(contains_types) {
//     //     //debug_log << " last_type_id: " << last_type_id << endl;
//     // }
//     // //debug_log << " last_run_id: " << last_run_id << " last_timestep_id: " << last_timestep_id << endl;

//     return rc;

// }

static int db_delete(sqlite3 *db)
{
    int rc;
    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;
    char  *ErrMsg = NULL;

    // rc = sqlite3_exec (db, "delete from run_catalog", -1, 0, &ErrMsg); assert(rc == SQLITE_OK);
    // rc = sqlite3_exec (db, "delete from timestep_catalog", -1, 0, &ErrMsg); assert(rc == SQLITE_OK);
    // rc = sqlite3_exec (db, "delete from var_catalog", -1, 0, &ErrMsg); assert(rc == SQLITE_OK);
    // rc = sqlite3_exec (db, "delete from type_catalog", -1, 0, &ErrMsg); assert(rc == SQLITE_OK);
    // rc = sqlite3_exec (db, "delete from run_attribute_catalog", -1, 0, &ErrMsg); assert(rc == SQLITE_OK);
    // rc = sqlite3_exec (db, "delete from timestep_attribute_catalog", -1, 0, &ErrMsg); assert(rc == SQLITE_OK);
    // rc = sqlite3_exec (db, "delete from var_attribute_catalog", -1, 0, &ErrMsg); assert(rc == SQLITE_OK);

    // vector<string> catalogs = {"run_catalog", "timestep_catalog", "var_catalog", "type_catalog",
    //   "run_attribute_catalog", "timestep_attribute_catalog", "var_attribute_catalog"};

    rc = sqlite3_exec (db, "begin;", callback, 0, &ErrMsg);
    if (rc != SQLITE_OK)
    {
        fprintf (stderr, "Error begin db_delete Line: %d SQL error: %s\n", __LINE__, sqlite3_errmsg (db));
        sqlite3_free (ErrMsg);
        sqlite3_close (db);
        goto cleanup;
    }

    rc = sqlite3_prepare_v2 (db, "delete from run_catalog", -1, &stmt, &tail); assert(rc == SQLITE_OK);
    rc = sqlite3_step (stmt); assert (rc == SQLITE_ROW || rc == SQLITE_DONE);
    rc = sqlite3_finalize (stmt); assert (rc == SQLITE_OK);

    rc = sqlite3_prepare_v2 (db, "delete from timestep_catalog", -1, &stmt, &tail); assert(rc == SQLITE_OK);
    rc = sqlite3_step (stmt); assert (rc == SQLITE_ROW || rc == SQLITE_DONE);
    rc = sqlite3_finalize (stmt); assert (rc == SQLITE_OK);

    rc = sqlite3_prepare_v2 (db, "delete from var_catalog", -1, &stmt, &tail); assert(rc == SQLITE_OK);
    rc = sqlite3_step (stmt); assert (rc == SQLITE_ROW || rc == SQLITE_DONE);
    rc = sqlite3_finalize (stmt); assert (rc == SQLITE_OK);

    rc = sqlite3_prepare_v2 (db, "delete from type_catalog", -1, &stmt, &tail); assert(rc == SQLITE_OK);
    rc = sqlite3_step (stmt); assert (rc == SQLITE_ROW || rc == SQLITE_DONE);
    rc = sqlite3_finalize (stmt); assert (rc == SQLITE_OK);

    rc = sqlite3_prepare_v2 (db, "delete from run_attribute_catalog", -1, &stmt, &tail); assert(rc == SQLITE_OK);
    rc = sqlite3_step (stmt); assert (rc == SQLITE_ROW || rc == SQLITE_DONE);
    rc = sqlite3_finalize (stmt); assert (rc == SQLITE_OK);

    rc = sqlite3_prepare_v2 (db, "delete from timestep_attribute_catalog", -1, &stmt, &tail); assert(rc == SQLITE_OK);
    rc = sqlite3_step (stmt); assert (rc == SQLITE_ROW || rc == SQLITE_DONE);
    rc = sqlite3_finalize (stmt); assert (rc == SQLITE_OK);

    rc = sqlite3_prepare_v2 (db, "delete from var_attribute_catalog", -1, &stmt, &tail); assert(rc == SQLITE_OK);
    rc = sqlite3_step (stmt); assert (rc == SQLITE_ROW || rc == SQLITE_DONE);
    rc = sqlite3_finalize (stmt); assert (rc == SQLITE_OK);

    // rc = sqlite3_prepare_v2 (db, "delete from run_catalog", -1, &stmt, &tail); assert(rc == SQLITE_OK);
    // rc = sqlite3_prepare_v2 (db, "delete from timestep_catalog", -1, &stmt, &tail); assert(rc == SQLITE_OK);
    // rc = sqlite3_prepare_v2 (db, "delete from var_catalog", -1, &stmt, &tail); assert(rc == SQLITE_OK);
    // rc = sqlite3_prepare_v2 (db, "delete from type_catalog", -1, &stmt, &tail); assert(rc == SQLITE_OK);
    // rc = sqlite3_prepare_v2 (db, "delete from run_attribute_catalog", -1, &stmt, &tail); assert(rc == SQLITE_OK);
    // rc = sqlite3_prepare_v2 (db, "delete from timestep_attribute_catalog", -1, &stmt, &tail); assert(rc == SQLITE_OK);
    // rc = sqlite3_prepare_v2 (db, "delete from var_attribute_catalog", -1, &stmt, &tail); assert(rc == SQLITE_OK);
    // rc = sqlite3_step (stmt); assert (rc == SQLITE_ROW || rc == SQLITE_DONE);
    // rc = sqlite3_finalize (stmt); assert (rc == SQLITE_OK);
    rc = sqlite3_exec (db, "end;", callback, 0, &ErrMsg);
    if (rc != SQLITE_OK)
    {
        fprintf (stderr, "Error end delete_all_vars_with_substr_stub: Line: %d SQL error: %s\n", __LINE__, ErrMsg);
        sqlite3_free (ErrMsg);
        sqlite3_close (db);
        goto cleanup;
    }


cleanup:
    if (rc != SQLITE_OK)
    {
        rc = sqlite3_exec (db, "rollback;", callback, 0, &ErrMsg);
    }
    return rc;
}

static int db_reset(sqlite3 **db, uint64_t job_id)
{
    add_timing_point(DB_RESET_START);

    int rc;
    // sqlite3_stmt * stmt = NULL;
 // const char * tail = NULL;
    // char  *ErrMsg = NULL;

    uint64_t last_run_attr_id, last_timestep_attr_id, last_timestep_id, last_run_id, last_type_id;
    uint64_t last_var_attr_id;
    bool contains_run_attrs = false, contains_timestep_attrs = false, contains_var_attrs = false, contains_types = false;

    rc = get_seq_values(*db, DB_IN_MEM, last_run_id, last_type_id, last_run_attr_id,
                    last_timestep_attr_id, last_var_attr_id, contains_types, contains_run_attrs,
                    contains_timestep_attrs, contains_var_attrs );
    if(rc != RC_OK) {
        error_log << "error in get_seq_values in db_reset" << endl;
        txn_id_to_db_connection.erase(job_id);
        return rc;
    }
    txn_id_to_db_connection.erase(job_id);
    sqlite3_close (*db);

    // rc = metadata_database_init(false, job_id, server_type);
    rc = metadata_database_init(db, false, false, job_id, true);
    if(rc != RC_OK) {
        error_log << "error in metadata_database_init in db_reset" << endl;
        return rc;
    }

    rc = update_seq_values(*db, last_run_id, last_type_id, last_run_attr_id, last_timestep_attr_id, 
                    last_var_attr_id, contains_types, contains_run_attrs,
                    contains_timestep_attrs, contains_var_attrs
                    );
    if(rc != RC_OK) {
        error_log << "error in update_seq_values in db_reset" << endl;
        return rc;
    }
    add_timing_point(DB_RESET_DONE);

    return rc;
}


static int get_seq_values(sqlite3 *db, md_db_type db_type, uint64_t &last_run_id, uint64_t &last_type_id, 
                    uint64_t &last_run_attr_id,uint64_t &last_timestep_attr_id, uint64_t &last_var_attr_id,  
                    bool &contains_types, bool &contains_run_attrs, bool &contains_timestep_attrs, 
                    bool &contains_var_attrs
                )
{
    int rc;
    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;

    switch(db_type) {
        case DB_IN_MEM : {
            rc = sqlite3_prepare_v2 (db, "select * from sqlite_sequence", -1, &stmt, &tail);
            if (rc != SQLITE_OK)
            {
                fprintf (stderr, "Error start of get_seq_values for db_in_mem. Line: %d SQL error: %s\n", __LINE__, sqlite3_errmsg (db));
                sqlite3_close (db);
                return rc;
            }
            break;
        }
        case DB_PREV : {
            rc = sqlite3_prepare_v2 (db, "select * from db_prev.sqlite_sequence", -1, &stmt, &tail);
            if (rc != SQLITE_OK)
            {
                fprintf (stderr, "Error start of get_seq_values for db_prev. Line: %d SQL error: %s\n", __LINE__, sqlite3_errmsg (db));
                sqlite3_close (db);
                return rc;
            }
            break;      
        }
        case DB_OUTPUT : {
            rc = sqlite3_prepare_v2 (db, "select * from db_output.sqlite_sequence", -1, &stmt, &tail);
            if (rc != SQLITE_OK)
            {
                fprintf (stderr, "Error start of get_seq_values for db_output. Line: %d SQL error: %s\n", __LINE__, sqlite3_errmsg (db));
                sqlite3_close (db);
                return rc;
            }
            break;
        }
    }

    rc = sqlite3_step (stmt); assert (rc == SQLITE_ROW || rc == SQLITE_DONE);
    while (rc == SQLITE_ROW) {
        string name = (char *)sqlite3_column_text (stmt, 0);
        if(!name.compare("run_catalog")) {
            last_run_id = (uint64_t)sqlite3_column_int64 (stmt, 1);
        }
        else if(!name.compare("type_catalog")) {
            contains_types = true;
            last_type_id = (uint64_t)sqlite3_column_int64 (stmt, 1);
        }       
        else if(!name.compare("var_attribute_catalog")) {
            contains_var_attrs = true;
            last_var_attr_id = (uint64_t)sqlite3_column_int64 (stmt, 1);
        }
        else if(!name.compare("timestep_attribute_catalog")) {
            contains_timestep_attrs = true;
            last_timestep_attr_id = (uint64_t)sqlite3_column_int64 (stmt, 1);
        }
        else if(!name.compare("run_attribute_catalog")) {
            contains_run_attrs = true;
            last_run_attr_id = (uint64_t)sqlite3_column_int64 (stmt, 1);
        }
        rc = sqlite3_step (stmt);

    }
    rc = sqlite3_finalize (stmt); assert (rc == SQLITE_OK); 
    return rc;
}

static int update_seq_values(sqlite3 *db, uint64_t last_run_id, uint64_t last_type_id, uint64_t last_run_attr_id, uint64_t last_timestep_attr_id, 
                    uint64_t last_var_attr_id, bool contains_types, bool contains_run_attrs,
                    bool contains_timestep_attrs, bool contains_var_attrs
                    )
{
    int rc;
    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;

    string catalog_name = "catalog";

    vector<string> catalog_names = {"run_catalog", "type_catalog", "run_attribute_catalog", "timestep_attribute_catalog", "var_attribute_catalog"};
    vector<bool> contains = {true, contains_types, contains_run_attrs, contains_timestep_attrs, contains_var_attrs};
    vector<uint64_t> ids = {last_run_id, last_type_id, last_run_attr_id, last_timestep_attr_id, last_var_attr_id};

    rc = sqlite3_prepare_v2 (db, "insert into sqlite_sequence (name,seq) values (?, ?)", -1, &stmt, &tail); assert(rc == SQLITE_OK);

    for(int i = 0; i < catalog_names.size(); i++) {
        if(contains.at(i)) {
            rc = sqlite3_bind_text (stmt, 1, strdup(catalog_names.at(i).c_str()), -1, free); assert (rc == SQLITE_OK);
            rc = sqlite3_bind_int64 (stmt, 2, ids.at(i)); assert (rc == SQLITE_OK); 
            rc = sqlite3_step (stmt); assert (rc == SQLITE_ROW || rc == SQLITE_DONE);
            rc = sqlite3_reset(stmt); assert (rc == SQLITE_OK);
        }
    }
    rc = sqlite3_finalize (stmt); assert (rc == SQLITE_OK);

    return rc;
}

static int get_run_id(sqlite3 *db, uint64_t job_id, uint64_t &last_run_id )
{

    int rc;
    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;

    rc = sqlite3_prepare_v2 (db, "select id from run_catalog where job_id = ? limit 1", -1, &stmt, &tail);
    if (rc != SQLITE_OK)
    {
        fprintf (stderr, "Error start of select get_run_id. Line: %d SQL error: %s\n", __LINE__, sqlite3_errmsg (db));
        txn_id_to_db_connection.erase(job_id);
        sqlite3_close (db);
        return rc;
    }
    rc = sqlite3_bind_int64 (stmt, 1, job_id); assert (rc == SQLITE_OK);

    rc = sqlite3_step (stmt); assert (rc == SQLITE_ROW || rc == SQLITE_DONE);
    if (rc == SQLITE_ROW) {
        last_run_id = (uint64_t)sqlite3_column_int64 (stmt, 0);
        rc = sqlite3_step (stmt); assert (rc == SQLITE_ROW || rc == SQLITE_DONE);
    }
    rc = sqlite3_finalize (stmt); assert (rc == SQLITE_OK); 

    return rc;
}


static int get_last_timestep(sqlite3 *db, md_db_type db_type, uint64_t last_run_id, uint64_t &last_timestep_id)
{

    int rc;
    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;


    switch(db_type) {
        case DB_IN_MEM : {
            rc = sqlite3_prepare_v2 (db, "select max(id) from timestep_catalog where run_id = ?", -1, &stmt, &tail);
            if (rc != SQLITE_OK)
            {
                fprintf (stderr, "Error start of get_last_timestep for db_in_mem. Line: %d SQL error: %s\n", __LINE__, sqlite3_errmsg (db));
                sqlite3_close (db);
                return rc;
            }
            break;
        }
        case DB_PREV : {
            rc = sqlite3_prepare_v2 (db, "select max(id) from db_prev.timestep_catalog where run_id = ?", -1, &stmt, &tail);
            if (rc != SQLITE_OK)
            {
                fprintf (stderr, "Error start of get_last_timestep for db_prev. Line: %d SQL error: %s\n", __LINE__, sqlite3_errmsg (db));
                sqlite3_close (db);
                return rc;
            }
            break;
        }
        case DB_OUTPUT : {
            rc = sqlite3_prepare_v2 (db, "select max(id) from db_output.timestep_catalog where run_id = ?", -1, &stmt, &tail);
            if (rc != SQLITE_OK)
            {
                fprintf (stderr, "Error start of get_last_timestep for db_output. Line: %d SQL error: %s\n", __LINE__, sqlite3_errmsg (db));
                sqlite3_close (db);
                return rc;
            }
            break;
        }
    }
    rc = sqlite3_bind_int64 (stmt, 1, last_run_id); assert (rc == SQLITE_OK);

    rc = sqlite3_step (stmt); assert (rc == SQLITE_ROW || rc == SQLITE_DONE);
    if (rc == SQLITE_ROW) {
        last_timestep_id = (uint64_t)sqlite3_column_int64 (stmt, 0);
        rc = sqlite3_step (stmt); assert (rc == SQLITE_ROW || rc == SQLITE_DONE);
    }
    rc = sqlite3_finalize (stmt); assert (rc == SQLITE_OK); 

    return rc;
}

static int insert_new(sqlite3 *db, uint64_t last_run_id, uint64_t last_timestep_id, uint64_t last_type_id, 
                    uint64_t last_run_attr_id, uint64_t last_timestep_attr_id, 
                    uint64_t last_var_attr_id, bool contains_types, bool contains_run_attrs,
                    bool contains_timestep_attrs, bool contains_var_attrs
                    )
{

    int rc;
    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;

  if(contains_run_attrs) {
        rc = sqlite3_prepare_v2 (db, "insert into db_output.run_attribute_catalog select * from run_attribute_catalog where id > ?", -1, &stmt, &tail); assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int64 (stmt, 1, last_run_attr_id); assert (rc == SQLITE_OK);
    }
    else {
        rc = sqlite3_prepare_v2 (db, "insert into db_output.run_attribute_catalog select * from run_attribute_catalog", -1, &stmt, &tail); assert (rc == SQLITE_OK);
    }
    rc = sqlite3_step (stmt); assert (rc == SQLITE_ROW || rc == SQLITE_DONE);
    rc = sqlite3_finalize (stmt);  assert (rc == SQLITE_OK); 


    if(contains_timestep_attrs) {
        rc = sqlite3_prepare_v2 (db, "insert into db_output.timestep_attribute_catalog select * from timestep_attribute_catalog where id > ?", -1, &stmt, &tail); assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int64 (stmt, 1, last_timestep_attr_id); assert (rc == SQLITE_OK);
    }
    else {
        rc = sqlite3_prepare_v2 (db, "insert into db_output.timestep_attribute_catalog select * from timestep_attribute_catalog", -1, &stmt, &tail); assert (rc == SQLITE_OK);
    }
    rc = sqlite3_step (stmt); assert (rc == SQLITE_ROW || rc == SQLITE_DONE);
    rc = sqlite3_finalize (stmt); assert (rc == SQLITE_OK);  

    if(contains_var_attrs) {
        rc = sqlite3_prepare_v2 (db, "insert into db_output.var_attribute_catalog select * from var_attribute_catalog where id > ?", -1, &stmt, &tail); assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int64 (stmt, 1, last_var_attr_id); assert (rc == SQLITE_OK);
    }
    else {
        rc = sqlite3_prepare_v2 (db, "insert into db_output.var_attribute_catalog select * from var_attribute_catalog", -1, &stmt, &tail); assert (rc == SQLITE_OK);
    }
    rc = sqlite3_step (stmt); assert (rc == SQLITE_ROW || rc == SQLITE_DONE);
    rc = sqlite3_finalize (stmt); assert (rc == SQLITE_OK); 


    if(contains_types) {
        rc = sqlite3_prepare_v2 (db, "insert into db_output.type_catalog select * from type_catalog where id > ?", -1, &stmt, &tail); assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int64 (stmt, 1, last_type_id); assert (rc == SQLITE_OK);
    }
    else {
        rc = sqlite3_prepare_v2 (db, "insert into db_output.type_catalog select * from type_catalog", -1, &stmt, &tail); assert (rc == SQLITE_OK);      
    }
    rc = sqlite3_step (stmt); assert (rc == SQLITE_ROW || rc == SQLITE_DONE);
    rc = sqlite3_finalize (stmt); assert (rc == SQLITE_OK); 

    //non-unique single number ids
    rc = sqlite3_prepare_v2 (db, "insert into db_output.timestep_catalog select * from timestep_catalog where id > ? and run_id = ?", -1, &stmt, &tail); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 1, last_timestep_id); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 2, last_run_id); assert (rc == SQLITE_OK);
    rc = sqlite3_step (stmt); assert (rc == SQLITE_ROW || rc == SQLITE_DONE);
    rc = sqlite3_finalize (stmt); assert (rc == SQLITE_OK); 

    rc = sqlite3_prepare_v2 (db, "insert into db_output.var_catalog select * from var_catalog where timestep_id > ? and run_id = ?", -1, &stmt, &tail); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 1, last_timestep_id); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 2, last_run_id); assert (rc == SQLITE_OK);   
    rc = sqlite3_step (stmt); assert (rc == SQLITE_ROW || rc == SQLITE_DONE);
    rc = sqlite3_finalize (stmt); assert (rc == SQLITE_OK); 

    return rc;
}


static int insert_all_new(sqlite3 *db)
{

    int rc;
    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;

    rc = sqlite3_prepare_v2 (db, "insert into db_output.run_attribute_catalog select * from run_attribute_catalog", -1, &stmt, &tail); assert (rc == SQLITE_OK);
    rc = sqlite3_step (stmt); assert (rc == SQLITE_ROW || rc == SQLITE_DONE);
    rc = sqlite3_finalize (stmt); assert (rc == SQLITE_OK); 

    rc = sqlite3_prepare_v2 (db, "insert into db_output.timestep_attribute_catalog select * from timestep_attribute_catalog", -1, &stmt, &tail); assert (rc == SQLITE_OK);
    rc = sqlite3_step (stmt); assert (rc == SQLITE_ROW || rc == SQLITE_DONE);
    rc = sqlite3_finalize (stmt); assert (rc == SQLITE_OK); 

    rc = sqlite3_prepare_v2 (db, "insert into db_output.var_attribute_catalog select * from var_attribute_catalog", -1, &stmt, &tail); assert (rc == SQLITE_OK);
    rc = sqlite3_step (stmt); assert (rc == SQLITE_ROW || rc == SQLITE_DONE);
    rc = sqlite3_finalize (stmt); assert (rc == SQLITE_OK); 

    rc = sqlite3_prepare_v2 (db, "insert into db_output.type_catalog select * from type_catalog", -1, &stmt, &tail); assert (rc == SQLITE_OK);    
    rc = sqlite3_step (stmt); assert (rc == SQLITE_ROW || rc == SQLITE_DONE);
    rc = sqlite3_finalize (stmt); assert (rc == SQLITE_OK); 

    //non-unique single number ids
    rc = sqlite3_prepare_v2 (db, "insert into db_output.timestep_catalog select * from timestep_catalog", -1, &stmt, &tail); assert (rc == SQLITE_OK);
    rc = sqlite3_step (stmt); assert (rc == SQLITE_ROW || rc == SQLITE_DONE);
    rc = sqlite3_finalize (stmt); assert (rc == SQLITE_OK); 

    rc = sqlite3_prepare_v2 (db, "insert into db_output.var_catalog select * from var_catalog", -1, &stmt, &tail); assert (rc == SQLITE_OK);
    rc = sqlite3_step (stmt); assert (rc == SQLITE_ROW || rc == SQLITE_DONE);
    rc = sqlite3_finalize (stmt); assert (rc == SQLITE_OK); 

    return rc;
}

static int db_incr_output_delete(sqlite3 *db, string filename)
{
    int rc;
    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;

    char  *ErrMsg = NULL;

    uint64_t last_run_attr_id, last_timestep_attr_id, last_timestep_id, last_run_id, last_type_id;
    uint64_t last_var_attr_id;
    bool contains_run_attrs = false, contains_timestep_attrs = false, contains_var_attrs = false, contains_types = false;


    rc = sqlite3_prepare_v2 (db, "attach database ? as db_output", -1, &stmt, &tail);
    if (rc != SQLITE_OK)
    {
        fprintf (stderr, "Error start of attach db: Line: %d SQL error: %s\n", __LINE__, sqlite3_errmsg (db));
        sqlite3_close (db);
        return rc;
    }
    rc = sqlite3_bind_text (stmt, 1, strdup(filename.c_str()), -1, free); assert (rc == SQLITE_OK);
    rc = sqlite3_step (stmt); assert (rc == SQLITE_ROW || rc == SQLITE_DONE);

    rc = sqlite3_finalize(stmt); assert (rc == SQLITE_OK);

    rc = sqlite3_exec (db, "begin;", callback, 0, &ErrMsg);
    if (rc != SQLITE_OK)
    {
        fprintf (stderr, "Error db_incr_output_delete: Line: %d SQL error: %s\n", __LINE__, sqlite3_errmsg (db));
        sqlite3_free (ErrMsg);
        sqlite3_close (db);
        return rc;
    }

    rc = insert_all_new(db);

    rc = sqlite3_exec (db, "end;", callback, 0, &ErrMsg);
    if (rc != SQLITE_OK)
    {
        fprintf (stderr, "Error db_incr_output_delete: Line: %d SQL error: %s\n", __LINE__, sqlite3_errmsg (db));
        sqlite3_free (ErrMsg);
        sqlite3_close (db);
        return rc;
    }

    rc = sqlite3_exec (db, "detach database db_output", callback, 0, &ErrMsg);
    if (rc != SQLITE_OK)    {
        fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
        sqlite3_free (ErrMsg);
        sqlite3_close (db);
        return rc;
    }
    
    // if(contains_run_attrs) {
        //debug_log << "last_run_attr_id: " << last_run_attr_id;
    // }
    // if(contains_timestep_attrs) {
        //debug_log <<  " last_timestep_attr_id: " << last_timestep_attr_id;
    // }
    // if(contains_timestep_attrs) {
        //debug_log <<  " last_var_attr_id: " << last_var_attr_id;
    // }
    // if(contains_types) {
        //debug_log << " last_type_id: " << last_type_id << endl;
    // }
    //debug_log << " last_run_id: " << last_run_id << " last_timestep_id: " << last_timestep_id << endl;

    return rc;

}


int md_catalog_num_timesteps (sqlite3 *db, uint64_t job_id, uint64_t &num_timesteps )
{
    int rc;
    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;
    const char * query = "select npx*npy*npz from run_catalog where job_id = ? limit 1";
    size_t size = 0;


    uint32_t count;
    rc = sqlite3_prepare_v2 (db, "select count (*) from run_catalog where job_id = ? ", -1, &stmt, &tail); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 1, job_id); assert (rc == SQLITE_OK);
    rc = sqlite3_step (stmt); assert (rc == SQLITE_ROW || rc == SQLITE_DONE);
    count = sqlite3_column_int64 (stmt, 0);
    rc = sqlite3_finalize (stmt); assert (rc == SQLITE_OK);

    if (count == 0) {
        error_log << "error. no runs matched job_id: " << job_id << endl;
        return RC_ERR;
    }

    rc = sqlite3_prepare_v2 (db, query, -1, &stmt, &tail); assert (rc == SQLITE_OK);
    if (rc != SQLITE_OK)
    {
        fprintf (stderr, "Error md_catalog_run_stub: Line: %d SQL error (%d): %s\n", __LINE__, rc, sqlite3_errmsg (db));
        txn_id_to_db_connection.erase(job_id);
        sqlite3_close (db);
        goto cleanup;
    }
    rc = sqlite3_bind_int64 (stmt, 1, job_id); assert (rc == SQLITE_OK);
    rc = sqlite3_step (stmt); assert (rc == SQLITE_ROW || rc == SQLITE_DONE);

    
    num_timesteps = sqlite3_column_int64 (stmt, 0);

    rc = sqlite3_finalize (stmt);  
    

cleanup:

    return rc;
}


int db_compact(sqlite3 *db, string filename_base, uint64_t job_id, string compacted_filename)
{
    int rc;
    sqlite3 *pFile;        /* Database connection  */
    sqlite3_backup *pBackup;  /* Backup object used to copy data */

    uint64_t run_id;
    // uint64_t num_timesteps;

    add_timing_point(DB_COMPACT_START);

    //note - if we knew the total size would be less than the max avail, could do this in memory
    rc = sqlite3_open (compacted_filename.c_str(), &db);
    if (rc != SQLITE_OK)
    {   
        fprintf (stderr, "Can't open database: %s\n in db_compact", sqlite3_errmsg (db));       
        txn_id_to_db_connection.erase(job_id);
        sqlite3_close (db);
        return RC_ERR;
    }

    string filename = filename_base + "0";

    /* Open the database file. Exit early if this fails
      ** for any reason. */

    //debug_log << "about to open: " << filename << endl;
    rc = sqlite3_open(filename.c_str(), &pFile);
    if( rc==SQLITE_OK ){  
        /* Set up the backup procedure to copy from the the main database of connection db
        to the "main" database of the connection pFile.
        ** If something goes wrong, pBackup will be set to NULL and an error
        ** code and message left in connection pFile.
        */
        // pBackup = sqlite3_backup_init(pTo, "main", pFrom, "main");
        pBackup = sqlite3_backup_init(db, "main", pFile, "main");

        /* If the backup object is successfully created, call backup_step()
        ** to copy data from db to pFile . Then call backup_finish()
        ** to release resources associated with the pBackup object.  If an
        ** error occurred, then an error code and message will be left in
        ** connection pTo. If no error occurred, then the error code belonging
        ** to pFile is set to SQLITE_OK.
        */
        if( pBackup ){
          (void)sqlite3_backup_step(pBackup, -1);
          (void)sqlite3_backup_finish(pBackup);
        }
        else {
            error_log << "error with sqlite3_backup_init in db_compact" << endl;
        }
        // rc = sqlite3_errcode(pTo);
        rc = sqlite3_errcode(db);
        //debug_log << "rc: " << rc << " in db_compact" << endl;
    }
    else {
        error_log << "error opening the file in db_compact" << endl;
    }

    /* Close the database connection opened on database file zFilename
    ** and return the result of this function. */
    (void)sqlite3_close(pFile);


    rc = get_run_id(db, job_id, run_id );
    //debug_log << "run_id: " << run_id << endl;


    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;
    char  *ErrMsg = NULL;

    rc = sqlite3_prepare_v2 (db, "attach database ? as db_input", -1, &stmt, &tail);
    if (rc != SQLITE_OK)
    {
        fprintf (stderr, "Error start of db_compact: Line: %d SQL error: %s\n", __LINE__, sqlite3_errmsg (db));
        txn_id_to_db_connection.erase(job_id);
        sqlite3_close (db);
        return rc;
    }

    for(int checkpt_num = 1; checkpt_num <= checkpt_count; checkpt_num += 1) {

        string filename_temp = filename_base + to_string(checkpt_num);
        //debug_log << "filename: " << strdup(filename_temp.c_str()) << endl;

        rc = sqlite3_bind_text (stmt, 1, strdup(filename_temp.c_str()), -1, free); assert (rc == SQLITE_OK);
        rc = sqlite3_step (stmt); assert (rc == SQLITE_ROW || rc == SQLITE_DONE);
        //debug_log << "about to reset" << endl;

        rc = sqlite3_exec (db, "begin;", callback, 0, &ErrMsg);
        if (rc != SQLITE_OK)
        {
            fprintf (stderr, "Error begin db_delete Line: %d SQL error: %s\n", __LINE__, sqlite3_errmsg (db));
            sqlite3_free (ErrMsg);
            sqlite3_close (db);
            return rc;
        }

        rc = sqlite3_exec (db, "insert into timestep_catalog select * from db_input.timestep_catalog", callback, 0, &ErrMsg);
        if (rc != SQLITE_OK)    {
            fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
            sqlite3_free (ErrMsg);
            txn_id_to_db_connection.erase(job_id);
            sqlite3_close (db);
            return rc;
        }

        rc = sqlite3_exec (db, "insert into var_catalog select * from db_input.var_catalog", callback, 0, &ErrMsg);
        if (rc != SQLITE_OK)    {
            fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
            sqlite3_free (ErrMsg);
            txn_id_to_db_connection.erase(job_id);
            sqlite3_close (db);
            return rc;
        }

        //note - we would assume that no types would be inserted part way through the run 
        // (we assume all posibilites are inserted at the beginning), but will leave this here to allow for the possibility
        rc = sqlite3_exec (db, "insert into type_catalog select * from db_input.type_catalog", callback, 0, &ErrMsg);
        if (rc != SQLITE_OK)    {
            fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
            sqlite3_free (ErrMsg);
            txn_id_to_db_connection.erase(job_id);
            sqlite3_close (db);
            return rc;
        }

        rc = sqlite3_exec (db, "insert into run_attribute_catalog select * from db_input.run_attribute_catalog", callback, 0, &ErrMsg);
        if (rc != SQLITE_OK)    {
            fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
            sqlite3_free (ErrMsg);
            txn_id_to_db_connection.erase(job_id);
            sqlite3_close (db);
            return rc;
        }

        rc = sqlite3_exec (db, "insert into timestep_attribute_catalog select * from db_input.timestep_attribute_catalog", callback, 0, &ErrMsg);
        if (rc != SQLITE_OK)    {
            fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
            sqlite3_free (ErrMsg);
            txn_id_to_db_connection.erase(job_id);
            sqlite3_close (db);
            return rc;
        }

        rc = sqlite3_exec (db, "insert into var_attribute_catalog select * from db_input.var_attribute_catalog", callback, 0, &ErrMsg);
        if (rc != SQLITE_OK)    {
            fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
            sqlite3_free (ErrMsg);
            txn_id_to_db_connection.erase(job_id);
            sqlite3_close (db);
            return rc;
        }

        rc = sqlite3_exec (db, "end;", callback, 0, &ErrMsg);
        if (rc != SQLITE_OK)
        {
            fprintf (stderr, "Error begin db_delete Line: %d SQL error: %s\n", __LINE__, sqlite3_errmsg (db));
            sqlite3_free (ErrMsg);
            sqlite3_close (db);
            return rc;
        }

        rc = sqlite3_exec (db, "detach database db_input", callback, 0, &ErrMsg);
        if (rc != SQLITE_OK)    {
            fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
            sqlite3_free (ErrMsg);
            txn_id_to_db_connection.erase(job_id);
            sqlite3_close (db);
            return rc;
        }

        rc = sqlite3_reset(stmt); assert (rc == SQLITE_OK);
    }
    
    rc = sqlite3_finalize(stmt); assert (rc == SQLITE_OK);

    //debug_log << "about to return" << endl;

    add_timing_point(DB_COMPACT_DONE);

    return rc;

}

// static int db_compact_and_output(uint64_t job_id, md_db_checkpoint_type checkpt_type)
static int db_compact_and_output(sqlite3 *db, uint64_t job_id)
{
    int rc;

    add_timing_point(DB_COMPACT_AND_OUTPUT_START);

    string filename_compacted = to_string(job_id) + "_" + to_string(proc_rank) + "_compact";
    // string filename_base = get_filename_base(job_id, checkpt_type);
    string filename_base = get_filename_base(job_id);

    rc = db_compact(db, filename_base, job_id, filename_compacted);
    if(rc != RC_OK) {
        error_log << "error in db_compact in db_compact_and_output" << endl;
    }
    //want to make sure to create the indices at the end of writing if they haven't already been created
    if(index_type == WRITE_DELAYED_INDEX) {
        rc = sqlite3_exec (db, "PRAGMA temp_store = FILE", callback, 0, 0);
        if (rc != SQLITE_OK)    {
            fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, sqlite3_errmsg (db));
            sqlite3_close (db);
            return RC_ERR;
        }

        rc = sqlite3_exec (db, "PRAGMA temp_store_directory = 'FILL_IN_WITH_DESIRED_VALUE", callback, 0, 0);
        if (rc != SQLITE_OK)    {
            fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, sqlite3_errmsg (db));
            sqlite3_close (db);
            return RC_ERR;
        }
  
        rc = create_reading_indices(db);
        if(rc != RC_OK) {
            txn_id_to_db_connection.erase(job_id); 
        }
    }

    // rc = db_copy(filename_compacted);
    // if(rc != RC_OK) {
    //   error_log << "error in db_copy in db_compact_and_output" << endl;
    // }
    txn_id_to_db_connection.erase(job_id);
    sqlite3_close (db);

    add_timing_point(DB_COMPACT_AND_OUTPUT_DONE);

    return rc;
}

sqlite3_int64 get_db_size (sqlite3 *db)
{
    int rc;
    sqlite3_int64 n1, n2;
    
    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;

    rc = sqlite3_prepare_v2 (db, "PRAGMA page_count", -1, &stmt, &tail); assert (rc == SQLITE_OK);
    rc = sqlite3_step (stmt); assert (rc == SQLITE_ROW || rc == SQLITE_DONE);
    n1 = sqlite3_column_int64 (stmt, 0);
    rc = sqlite3_finalize (stmt); assert (rc == SQLITE_OK);

    rc = sqlite3_prepare_v2 (db, "PRAGMA page_size", -1, &stmt, &tail); assert (rc == SQLITE_OK);
    rc = sqlite3_step (stmt); assert (rc == SQLITE_ROW || rc == SQLITE_DONE);
    n2 = sqlite3_column_int64 (stmt, 0);
    rc = sqlite3_finalize (stmt); assert (rc == SQLITE_OK);

    // PRAGMA page_count * PRAGMA page_size;
    return n1 * n2;
}

// static string get_filename_base(uint64_t job_id, md_db_checkpoint_type checkpt_type)
static string get_filename_base(uint64_t job_id)
{
    string filename_base;


    if(server_type == SERVER_DEDICATED_ON_DISK || server_type == SERVER_LOCAL_ON_DISK) {
        //debug_log << "true, server is on disk" << endl;
        return (to_string(job_id) + "_" + to_string(proc_rank) + "_disk_");
    }

    //debug_log << "server is NOT on disk" << endl;

    switch(checkpt_type) {
        case DB_COPY : {
            filename_base = to_string(job_id) + "_" + to_string(proc_rank) + "_";
            break;
        }
        case DB_INCR_OUTPUT : {
            filename_base = to_string(job_id) + "_" + to_string(proc_rank) + "_incr_";
            break;
        }
        case DB_COPY_AND_DELETE : {
            filename_base = to_string(job_id) + "_" + to_string(proc_rank) + "_del_";

            break;
        }
        case DB_COPY_AND_RESET : {
            filename_base = to_string(job_id) + "_" + to_string(proc_rank) + "_reset_";
            break;
        }
        case DB_INCR_OUTPUT_AND_DELETE : {
            filename_base = to_string(job_id) + "_" + to_string(proc_rank) + "_incr_del_";

            break;
        }
        case DB_INCR_OUTPUT_AND_RESET : {
            filename_base = to_string(job_id) + "_" + to_string(proc_rank) + "_incr_reset_";
            break;
        }

    }
    return filename_base;
}


// void get_database(uint64_t txn_id, sqlite3 **db, bool &transaction_open) {
//     std::unordered_map<uint64_t,sqlite3 *>::const_iterator got = txn_id_to_db_connection.find (txn_id);

//     if ( got == txn_id_to_db_connection.end() ) {
//         //debug_log << "could not find database: " << txn_id << " so am adding it" << endl;
//     //not found
//         metadata_database_init(db, false, true, txn_id);
//         txn_id_to_db_connection[txn_id] = db_conn(db);
//         return db;
//     }
//     else { //found
//         //debug_log << "found database " << txn_id << endl;
//         *db = (got->second).db;
//         transaction_open = (got->second).transaction_open;
//     }
// }


//reminder: since sqlite does entire db locking, if we want to activate md (moving it from the WAL to
//the permanent db, we need to end all transactions)
sqlite3 * get_database_for_read(uint64_t txn_id) {
    int rc;
    char * ErrMsg = NULL;
    sqlite3 *db;

    std::unordered_map<uint64_t,db_conn>::iterator got = txn_id_to_db_connection.find (txn_id);

    if ( got == txn_id_to_db_connection.end() ) {
        //debug_log << "could not find database: " << txn_id << " so am adding it" << endl;
    //not found
        metadata_database_init(&db, false, true, txn_id, false);
        txn_id_to_db_connection[txn_id] = db_conn(db);
        return db;
    }
    else { //found
        db = (got -> second).db;

        if((got -> second).transaction_open) {
            rc = sqlite3_exec (db, "end;", callback, 0, &ErrMsg);
            if (rc != SQLITE_OK)
            {
                fprintf (stderr, "Error end query get_database_for_read: Line: %d SQL error: %s\n", __LINE__, ErrMsg);
                sqlite3_free (ErrMsg);
                sqlite3_close (db);
                return NULL;
            }
            (got -> second).transaction_open = false;
        }
       
        //debug_log << "found database " << txn_id << endl;
        return db;
    }
    // return txn_id_to_db_connection[txn_id];
}


sqlite3 * get_database_for_write(uint64_t txn_id) {
    int rc;
    char * ErrMsg = NULL;
    sqlite3 *db;

    std::unordered_map<uint64_t,db_conn>::iterator got = txn_id_to_db_connection.find (txn_id);

    if ( got == txn_id_to_db_connection.end() ) {
        //debug_log << "could not find database: " << txn_id << " so am adding it" << endl;
    //not found
        metadata_database_init(&db, false, true, txn_id, false);
        // txn_id_to_db_connection[txn_id] = db_conn(db);

        //wrap the write in a transaction
        rc = sqlite3_exec (db, "begin;", callback, 0, &ErrMsg);
        if (rc != SQLITE_OK)
        {
            fprintf (stderr, "Error end query get_database_for_write: Line: %d SQL error: %s\n", __LINE__, ErrMsg);
            sqlite3_free (ErrMsg);
            sqlite3_close (db);
            return NULL;
        }
        txn_id_to_db_connection[txn_id].transaction_open = true;
        return db;
    }
    else { //found
        db = (got -> second).db;

        if(!(got -> second).transaction_open) {
             rc = sqlite3_exec (db, "begin;", callback, 0, &ErrMsg);
            if (rc != SQLITE_OK)
            {
                fprintf (stderr, "Error end query get_database_for_write: Line: %d SQL error: %s\n", __LINE__, ErrMsg);
                sqlite3_free (ErrMsg);
                sqlite3_close (db);
                return NULL;
            }
            (got -> second).transaction_open = true;
        }
       
        //debug_log << "found database " << txn_id << endl;
        return db;
    }
    // return txn_id_to_db_connection[txn_id];
}

sqlite3 * get_database(uint64_t txn_id) {
    //debug_log << "txn_id_to_db_connection.size(): " << txn_id_to_db_connection.size() << endl;
    int rc;
    char * ErrMsg = NULL;
    sqlite3 *db;

    std::unordered_map<uint64_t,db_conn>::const_iterator got = txn_id_to_db_connection.find (txn_id);

    if ( got == txn_id_to_db_connection.end() ) {
        //debug_log << "could not find database: " << txn_id << " so am adding it" << endl;
    //not found
        metadata_database_init(&db, false, true, txn_id, false);
        txn_id_to_db_connection[txn_id] = db_conn(db);
        return db;
    }
    else { //found
        //debug_log << "found database " << txn_id << endl;
        return (got -> second).db;
    }
    // return txn_id_to_db_connection[txn_id];
}

// void get_database(uint64_t txn_id, sqlite3 **db) {
//     std::unordered_map<uint64_t,sqlite3 *>::const_iterator got = txn_id_to_db_connection.find (txn_id);

//     if ( got == txn_id_to_db_connection.end() ) {
//         //debug_log << "could not find database: " << txn_id << " so am adding it" << endl;
//     //not found
//         metadata_database_init(db, false, true, txn_id);
//         txn_id_to_db_connection[txn_id] = db_conn(db);
//         return db;
//     }
//     else { //found
//         //debug_log << "found database " << txn_id << endl;
//         *db = (got->second).db;
//     }
// }

// sqlite3 * get_database(uint64_t txn_id) {
//     //debug_log << "txn_id_to_db_connection.size(): " << txn_id_to_db_connection.size() << endl;
//     sqlite3 *db;

//     std::unordered_map<uint64_t,sqlite3 *>::const_iterator got = txn_id_to_db_connection.find (txn_id);

//     if ( got == txn_id_to_db_connection.end() ) {
//         //debug_log << "could not find database: " << txn_id << " so am adding it" << endl;
//     //not found
//         metadata_database_init(&db, false, true, txn_id);
//         txn_id_to_db_connection[txn_id] = db;
//         return db;
//     }
//     else { //found
//         //debug_log << "found database " << txn_id << endl;
//         return got -> second;
//     }

//     // return txn_id_to_db_connection[txn_id];
// }

// void start_transaction(uint64_t txn_id) {
//     vector<bool> transaction_states(7); //number of transaction savepoints
//     transaction_states[RUN_TRANSACTION_STARTED] = true;
//     txn_id_to_transaction_states[txn_id] = transaction_states; 

// }

// void add_transaction_savepoint(uint64_t txn_id, md_server_transaction_savepoints savepoint) {
//     txn_id_to_transaction_states[txn_id][savepoint] = true;
// }

// void release_transaction_savepoint(uint64_t txn_id, md_server_transaction_savepoints savepoint) {
//     txn_id_to_transaction_states[txn_id][savepoint] = false;
// }

// bool get_transaction_state(uint64_t txn_id, md_server_transaction_savepoints savepoint) {
//     return txn_id_to_transaction_states[txn_id][savepoint];
// }

