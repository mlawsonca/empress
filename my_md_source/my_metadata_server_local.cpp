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

#include <my_metadata_args.h>
// #include <libops.hh>

// #include "dirman/DirMan.hh"
// 
// #include "opbox/OpBox.hh"
#include "faodel-common/Common.hh"
// 
#include <sqlite3.h>

//will limit it to 200 MB since we don't want to use too much of RAM
//and the SQLite usage will exceed the actual DB size
#define MAX_DB_SIZE 200000000


// #include <md_client_timing_constants_local.hh>
#include <server_timing_constants_new.hh>


using namespace std;

//dummy, will never be used since we aren't sending any messages to servers across the network
uint32_t MAX_EAGER_MSG_SIZE = 2048;

extern bool output_timing;
extern sqlite3 *db;
// sqlite3 *db;

int init_db_size;
bool first_checkpt = true;
int checkpt_count = -1;
bool first_init = true;

//add back in once sent
extern void add_timing_point(int catg);
extern void add_db_output_timing_point(int catg, uint64_t size);
bool use_mpi = true;


static bool extreme_debug_logging = false;
static bool debug_logging = false;
static bool error_logging = true;
static debugLog error_log = debugLog(error_logging);
static debugLog debug_log = debugLog(debug_logging);
static debugLog extreme_debug_log = debugLog(extreme_debug_logging);


md_server_type server_type;
md_db_index_type index_type;
md_db_checkpoint_type checkpt_type;


#ifndef MD_DB_TYPE_ENUM
#define MD_DB_TYPE_ENUM
enum md_db_type : unsigned short
{
    DB_IN_MEM = 0,
    DB_PREV = 1,
    DB_OUTPUT = 2,
};
#endif //MD_DB_CHECKPOINT_TYPE_ENUM

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

int md_catalog_num_procs (uint64_t job_id, uint64_t &num_write_procs );


// int setup_dirman(const string &dirman_file_path, const string &app_name, int proc_rank);

// int sql_output_db(uint64_t job_id, int proc_rank);
int sql_load_db(uint64_t job_id, int proc_rank);
int sql_load_multi_db(uint64_t job_id, uint32_t num_read_procs, int proc_rank);

static int db_copy(string filename);
static int db_incr_output(string filename);
static int add_new_to_db_checkpoint_new_file(string output_filename, string prev_filename);
static int delete_db();
static bool is_db_output_start(uint16_t code);
// static int db_reset(int proc_rank, uint64_t job_id, md_server_type server_type, md_db_index_type index_type);
static int db_reset(int proc_rank, uint64_t job_id);
static int get_seq_values(md_db_type db_type, uint64_t &last_run_id, uint64_t &last_type_id, 
                    uint64_t &last_run_attr_id,uint64_t &last_timestep_attr_id, uint64_t &last_var_attr_id,  
                    bool &contains_types, bool &contains_run_attrs, bool &contains_timestep_attrs, 
                    bool &contains_var_attrs
                );
static int update_seq_values(uint64_t last_run_id, uint64_t last_type_id, uint64_t last_run_attr_id, uint64_t last_timestep_attr_id, 
                    uint64_t last_var_attr_id, bool contains_types, bool contains_run_attrs,
                    bool contains_timestep_attrs, bool contains_var_attrs
                    );
static int get_last_timestep(md_db_type db_type, uint64_t last_run_id, uint64_t &last_timestep_id);
static int insert_new(uint64_t last_run_id, uint64_t last_timestep_id, uint64_t last_type_id, 
                    uint64_t last_run_attr_id, uint64_t last_timestep_attr_id, 
                    uint64_t last_var_attr_id, bool contains_types, bool contains_run_attrs,
                    bool contains_timestep_attrs, bool contains_var_attrs
                    );
// static int db_compact_and_output(int proc_rank, uint64_t job_id, md_db_index_type index_type, md_db_checkpoint_type checkpt_type);
static int db_compact_and_output(int proc_rank, uint64_t job_id);
static int register_ops ();
static int create_writing_indices();
static int create_reading_indices();
// static sqlite3_int64 get_db_size ();
sqlite3_int64 get_db_size ();

static void gather_db_statistics( uint64_t **all_db_sizes, int proc_rank, int num_procs);
// static string get_filename_base(int proc_rank, uint64_t job_id, md_db_checkpoint_type checkpt_type);
static string get_filename_base(int proc_rank, uint64_t job_id);
static int db_delete();
static int db_incr_output_delete(string filename);

bool db_output_is_whole() {
    return(checkpt_count == 0);
}

//===========================================================================
// int metadata_database_close() {
//  sqlite3_close (db);
// }

void metadata_init_server(md_server_type my_server_type, md_db_index_type my_index_type, md_db_checkpoint_type my_checkpt_type)
{
    server_type = my_server_type;
    index_type = my_index_type;
    checkpt_type = my_checkpt_type;

    if(checkpt_type == DB_INCR_OUTPUT_AND_DELETE || checkpt_type == DB_INCR_OUTPUT_AND_RESET || server_type == SERVER_LOCAL_ON_DISK) {
        checkpt_count = 0;
    }
}
        

// int metadata_database_init (bool load_multiple_db, uint64_t job_id, int proc_rank, uint32_t num_read_procs, md_server_type server_type)
int metadata_database_init (bool load_multiple_db, uint64_t job_id, int proc_rank, uint32_t num_read_procs)
{
    int rc;
    char  *ErrMsg = NULL;
    
    // rc = sqlite3_open (":memory:", &db);

    add_timing_point(DB_SETUP_START);
    if(server_type != SERVER_LOCAL_ON_DISK) {
        rc = sqlite3_open (":memory:", &db);
        // cout << "am opening in memory db" << endl;
    }
    else {
        string filename = to_string(job_id) + "_" + to_string(proc_rank) + "_0_disk";
        rc = sqlite3_open (filename.c_str(), &db);
        // cout << "am opening db on disk" << endl;
    }
    if (rc != SQLITE_OK)
    {   
        fprintf (stderr, "Can't open database: %s\n", sqlite3_errmsg (db));        
        sqlite3_close (db);
        goto cleanup;
    }

    if (load_multiple_db) {
        rc = sql_load_multi_db(job_id, num_read_procs, proc_rank);
        if (rc != SQLITE_OK)
        {   
            fprintf (stderr, "Can't load multiple databases for job_id:%llu: %s\n",job_id, sqlite3_errmsg (db));        
            sqlite3_close (db);
            goto cleanup;
        }
    }
    else {
        rc = sql_load_db(job_id, proc_rank);
        if (rc != SQLITE_OK)
        {   
            fprintf (stderr, "Can't load database for job_id:%llu: %s\n",job_id, sqlite3_errmsg (db));        
            sqlite3_close (db);
            goto cleanup;
        }
    }

    add_timing_point(DB_SETUP_DONE);

    init_db_size = sqlite3_memory_used();


cleanup:
    if(rc != RC_OK) {
        add_timing_point(ERR_DB_SETUP);
    }
    return rc;
}


// int metadata_database_init (int proc_rank, uint64_t job_id, md_server_type server_type, md_db_index_type index_type)
int metadata_database_init (int proc_rank, uint64_t job_id)
{
    //setup the database
    int rc;
    char  *ErrMsg = NULL;
    
    // rc = sqlite3_open (":memory:", &db);

    add_timing_point(DB_SETUP_START);
    if(server_type != SERVER_LOCAL_ON_DISK) {
        rc = sqlite3_open (":memory:", &db);
        // cout << "am opening in memory db" << endl;
    }
    else { //SERVER_LOCAL_ON_DISK
        string filename = to_string(job_id) + "_" + to_string(proc_rank) + "_0_disk";
        rc = sqlite3_open (filename.c_str(), &db);
        // cout << "am opening db on disk" << endl;
    }
    if (rc != SQLITE_OK)
    {   
        fprintf (stderr, "Can't open database: %s\n", sqlite3_errmsg (db));        
        sqlite3_close (db);
        goto cleanup;
    }


    if(server_type != SERVER_LOCAL_SQLITE_TRANSACTION_MANAGEMENT_WAL && server_type != SERVER_DEDICATED_SQLITE_TRANSACTION_MANAGEMENT_DB_STREAMS) {
        rc = sqlite3_exec (db, "create table run_catalog (id integer primary key autoincrement not null, job_id int, name varchar (50) collate nocase, date varchar (50), active int, txn_id int, npx int, npy int, npz int, rank_to_dims_funct varchar (1000), objector_funct varchar (6000))", callback, 0, &ErrMsg);
        if (rc != SQLITE_OK)    {
            fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
            sqlite3_free (ErrMsg);
            sqlite3_close (db);
            goto cleanup;
        }

        rc = sqlite3_exec (db, "create table timestep_catalog (id integer not null, run_id int not null, path varchar (50) collate nocase, active int, txn_id int, primary key(id, run_id) )", callback, 0, &ErrMsg);
        if (rc != SQLITE_OK)    {
            fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
            sqlite3_free (ErrMsg);
            sqlite3_close (db);
            goto cleanup;
        }

        //todo - should just dataset be active instead of var?
        // rc = sqlite3_exec (db, "create table var_catalog (id integer primary key autoincrement not null, dataset_id int, name varchar (50), path varchar (50), version int, data_size int, num_dims int, d0_min int, d0_max int, d1_min int, d1_max int, d2_min int, d2_max int)", callback, 0, &ErrMsg);
        rc = sqlite3_exec (db, "create table var_catalog (id integer not null, run_id int not null, timestep_id int not null, name varchar (50) collate nocase, path varchar (50) collate nocase, version int, data_size int, active int, txn_id int, num_dims int, d0_min int, d0_max int, d1_min int, d1_max int, d2_min int, d2_max int, primary key(id, run_id, timestep_id) )", callback, 0, &ErrMsg);
        
        if (rc != SQLITE_OK)    {
            fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
            sqlite3_free (ErrMsg);
            sqlite3_close (db);
            goto cleanup;
        }

        // rc = sqlite3_exec (db, "create table type_catalog (id integer primary key autoincrement not null, dataset_id int, name varchar (50), version int, active int, txn_id int)", callback, 0, &ErrMsg);
        rc = sqlite3_exec (db, "create table type_catalog (id integer primary key autoincrement not null, run_id int not null, name varchar (50) collate nocase, version int, active int, txn_id int )", callback, 0, &ErrMsg);
        if (rc != SQLITE_OK)    {
            fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
            sqlite3_free (ErrMsg);
            sqlite3_close (db);
            goto cleanup;
        }

        rc = sqlite3_exec (db, "create table run_attribute_catalog (id integer primary key autoincrement not null, run_id int not null, type_id int not null, active int, txn_id int, data_type int, data none )", callback, 0, &ErrMsg);
        if (rc != SQLITE_OK)    {
            fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
            sqlite3_free (ErrMsg);
            sqlite3_close (db);
            goto cleanup;
        }

        rc = sqlite3_exec (db, "create table timestep_attribute_catalog (id integer primary key autoincrement not null, timestep_id not null, type_id int not null, active int, txn_id int, data_type int, data none )", callback, 0, &ErrMsg);
        if (rc != SQLITE_OK)    {
            fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
            sqlite3_free (ErrMsg);
            sqlite3_close (db);
            goto cleanup;
        }

        rc = sqlite3_exec (db, "create table var_attribute_catalog (id integer primary key autoincrement not null, timestep_id not null, type_id int not null, var_id int not null, active int, txn_id int, num_dims int not null, d0_min int, d0_max int, d1_min int, d1_max int, d2_min int, d2_max int, data_type int, data none )", callback, 0, &ErrMsg);
        if (rc != SQLITE_OK)    {
            fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
            sqlite3_free (ErrMsg);
            sqlite3_close (db);
            goto cleanup;
        }
       
        rc = create_writing_indices();
    }
    else { //SERVER_LOCAL_SQLITE_TRANSACTION_MANAGEMENT_WAL
        rc = sqlite3_exec (db, "create table run_catalog (id integer primary key autoincrement not null, job_id int, name varchar (50) collate nocase, date varchar (50), npx int, npy int, npz int, rank_to_dims_funct varchar (1000), objector_funct varchar (6000))", callback, 0, &ErrMsg);
        if (rc != SQLITE_OK)    {
            fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
            sqlite3_free (ErrMsg);
            sqlite3_close (db);
            goto cleanup;
        }

        rc = sqlite3_exec (db, "create table timestep_catalog (id integer not null, run_id int not null, path varchar (50) collate nocase, primary key(id, run_id) )", callback, 0, &ErrMsg);
        if (rc != SQLITE_OK)    {
            fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
            sqlite3_free (ErrMsg);
            sqlite3_close (db);
            goto cleanup;
        }

        // rc = sqlite3_exec (db, "create table var_catalog (id integer primary key autoincrement not null, dataset_id int, name varchar (50), path varchar (50), version int, data_size int, num_dims int, d0_min int, d0_max int, d1_min int, d1_max int, d2_min int, d2_max int)", callback, 0, &ErrMsg);
        rc = sqlite3_exec (db, "create table var_catalog (id integer not null, run_id int not null, timestep_id int not null, name varchar (50) collate nocase, path varchar (50) collate nocase, version int, data_size int, num_dims int, d0_min int, d0_max int, d1_min int, d1_max int, d2_min int, d2_max int, primary key(id, run_id, timestep_id) )", callback, 0, &ErrMsg);
        
        if (rc != SQLITE_OK)    {
            fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
            sqlite3_free (ErrMsg);
            sqlite3_close (db);
            goto cleanup;
        }

        // rc = sqlite3_exec (db, "create table type_catalog (id integer primary key autoincrement not null, dataset_id int, name varchar (50), version int)", callback, 0, &ErrMsg);
        rc = sqlite3_exec (db, "create table type_catalog (id integer primary key autoincrement not null, run_id int not null, name varchar (50) collate nocase, version int )", callback, 0, &ErrMsg);
        if (rc != SQLITE_OK)    {
            fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
            sqlite3_free (ErrMsg);
            sqlite3_close (db);
            goto cleanup;
        }

        rc = sqlite3_exec (db, "create table run_attribute_catalog (id integer primary key autoincrement not null, run_id int not null, type_id int not null, data_type int, data none )", callback, 0, &ErrMsg);
        if (rc != SQLITE_OK)    {
            fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
            sqlite3_free (ErrMsg);
            sqlite3_close (db);
            goto cleanup;
        }

        rc = sqlite3_exec (db, "create table timestep_attribute_catalog (id integer primary key autoincrement not null, timestep_id not null, type_id int not null, data_type int, data none )", callback, 0, &ErrMsg);
        if (rc != SQLITE_OK)    {
            fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
            sqlite3_free (ErrMsg);
            sqlite3_close (db);
            goto cleanup;
        }

        rc = sqlite3_exec (db, "create table var_attribute_catalog (id integer primary key autoincrement not null, timestep_id not null, type_id int not null, var_id int not null, num_dims int not null, d0_min int, d0_max int, d1_min int, d1_max int, d2_min int, d2_max int, data_type int, data none )", callback, 0, &ErrMsg);
        if (rc != SQLITE_OK)    {
            fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
            sqlite3_free (ErrMsg);
            sqlite3_close (db);
            goto cleanup;
        }
    }
    if(index_type == WRITE_INDEX) {
        rc = create_reading_indices();
    }

    if (proc_rank > 0 && checkpt_count < 0) {
        uint64_t new_seq_start = 1000000*proc_rank;

        string update_id_query, update_id_query2;

        vector<string> table_seqs_to_update = {"run_attribute_catalog", "timestep_attribute_catalog", "var_attribute_catalog"};

        rc = sqlite3_exec (db, "BEGIN TRANSACTION", callback, 0, &ErrMsg);
        if (rc != SQLITE_OK)    {
            fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
            sqlite3_free (ErrMsg);
            sqlite3_close (db);
            goto cleanup;
        }

        for(int i = 0; i < table_seqs_to_update.size(); i++) {
            char query[248];
            char query2[248];

            string catalog_name = table_seqs_to_update.at(i);
    

            int n = sprintf(query, "UPDATE SQLITE_SEQUENCE SET seq = %llu WHERE name = '%s'",new_seq_start, catalog_name.c_str());
            extreme_debug_log << "query: " << query << endl;

            rc = sqlite3_exec (db, query, callback, 0, &ErrMsg);
            if (rc != SQLITE_OK)    {
                fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
                sqlite3_free (ErrMsg);
                sqlite3_close (db);
                goto cleanup;
            }
            extreme_debug_log << "got here" << endl;

            n = sprintf(query2, "INSERT INTO sqlite_sequence (name,seq) SELECT '%s', %llu WHERE NOT EXISTS "
                   "(SELECT changes() AS change FROM sqlite_sequence WHERE change <> 0)", catalog_name.c_str(), new_seq_start);
            extreme_debug_log << "query2: " << query2 << endl;

            rc = sqlite3_exec (db, query2, callback, 0, &ErrMsg);
            if (rc != SQLITE_OK)    {
                fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
                sqlite3_free (ErrMsg);
                sqlite3_close (db);
                goto cleanup;
            }
        }

        rc = sqlite3_exec (db, "COMMIT", callback, 0, &ErrMsg);


  //       if (rc != SQLITE_OK)    {
  //           fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
  //           sqlite3_free (ErrMsg);
  //           sqlite3_close (db);
  //           goto cleanup;
  //       }
  //       extreme_debug_log << "update_id_query: " << update_id_query << endl; 
        

//          update_id_query = "UPDATE SQLITE_SEQUENCE SET seq = " + to_string(1000000*proc_rank) + " WHERE name = 'timestep_attribute_catalog'";
        // rc = sqlite3_exec (db, update_id_query.c_str(), callback, 0, &ErrMsg);
  //       if (rc != SQLITE_OK)    {
  //           fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
  //           sqlite3_free (ErrMsg);
  //           sqlite3_close (db);
  //           goto cleanup;
  //       }

//          update_id_query = "UPDATE SQLITE_SEQUENCE SET seq = " + to_string(1000000*proc_rank) + " WHERE name = 'var_attribute_catalog'";
        // rc = sqlite3_exec (db, update_id_query.c_str(), callback, 0, &ErrMsg);
  //       if (rc != SQLITE_OK)    {
  //           fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
  //           sqlite3_free (ErrMsg);
  //           sqlite3_close (db);
  //           goto cleanup;
  //       }

      //   extreme_debug_log << "about to select" << endl;
        // rc = sqlite3_exec (db, "SELECT * FROM SQLITE_SEQUENCE", callback, 0, &ErrMsg);
      //   if (rc != SQLITE_OK)    {
      //       fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
      //       sqlite3_free (ErrMsg);
      //       sqlite3_close (db);
      //       goto cleanup;
      //   }
      //   extreme_debug_log << "done selecting" << endl;

    }
    

    add_timing_point(DB_SETUP_DONE);

    init_db_size = sqlite3_memory_used();

cleanup:
    return rc;
}

static int create_writing_indices() {
    int rc;
    char  *ErrMsg = NULL;
    add_timing_point(DB_CREATE_WRITING_INDICES_START);

        rc = sqlite3_exec (db, "create index rc_txn_id on run_catalog (txn_id)", callback, 0, &ErrMsg);
    if (rc != SQLITE_OK)    {
        fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
        sqlite3_free (ErrMsg);
        sqlite3_close (db);
        goto cleanup;
    }

    rc = sqlite3_exec (db, "create index tmc_txn_id on timestep_attribute_catalog (txn_id)", callback, 0, &ErrMsg);
    if (rc != SQLITE_OK)    {
        fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
        sqlite3_free (ErrMsg);
        sqlite3_close (db);
        goto cleanup;
    }

    rc = sqlite3_exec (db, "create index vc_txn_id on var_catalog (txn_id)", callback, 0, &ErrMsg);
    if (rc != SQLITE_OK)    {
        fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
        sqlite3_free (ErrMsg);
        sqlite3_close (db);
        goto cleanup;
    }

    rc = sqlite3_exec (db, "create index tc_txn_id on type_catalog (txn_id)", callback, 0, &ErrMsg);
    if (rc != SQLITE_OK)    {
        fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
        sqlite3_free (ErrMsg);
        sqlite3_close (db);
        goto cleanup;
    }

    rc = sqlite3_exec (db, "create index rac_txn_id on run_attribute_catalog (txn_id)", callback, 0, &ErrMsg);
    if (rc != SQLITE_OK)    {
        fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
        sqlite3_free (ErrMsg);
        sqlite3_close (db);
        goto cleanup;
    }

    rc = sqlite3_exec (db, "create index tac_txn_id on timestep_attribute_catalog (txn_id)", callback, 0, &ErrMsg);
    if (rc != SQLITE_OK)    {
        fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
        sqlite3_free (ErrMsg);
        sqlite3_close (db);
        goto cleanup;
    }

    rc = sqlite3_exec (db, "create index vac_txn_id on var_attribute_catalog (txn_id)", callback, 0, &ErrMsg);
    if (rc != SQLITE_OK)    {
        fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
        sqlite3_free (ErrMsg);
        sqlite3_close (db);
        goto cleanup;
    }
    add_timing_point(DB_CREATE_WRITING_INDICES_DONE);

cleanup:
    return rc;
}

static int create_reading_indices()
{
    int rc;
    char  *ErrMsg = NULL;
    add_timing_point(DB_CREATE_READING_INDICES_START);


    // //INDICES/////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // //TXN-ACTIVE/////////////////////////////////////////////////////////////////////////////////////////////////////////////       
    //     rc = sqlite3_exec (db, "create index rc_txn_id_active on run_catalog (txn_id, active)", callback, 0, &ErrMsg);
    //     if (rc != SQLITE_OK)    {
    //         fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
    //         sqlite3_free (ErrMsg);
    //         sqlite3_close (db);
    //         goto cleanup;
    //     }

    //     rc = sqlite3_exec (db, "create index tmc_txn_id_active on timestep_attribute_catalog (txn_id, active)", callback, 0, &ErrMsg);
    //     if (rc != SQLITE_OK)    {
    //         fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
    //         sqlite3_free (ErrMsg);
    //         sqlite3_close (db);
    //         goto cleanup;
    //     }

    //     rc = sqlite3_exec (db, "create index vc_txn_id_active on var_catalog (txn_id, active)", callback, 0, &ErrMsg);
    //     if (rc != SQLITE_OK)    {
    //         fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
    //         sqlite3_free (ErrMsg);
    //         sqlite3_close (db);
    //         goto cleanup;
    //     }


    //     rc = sqlite3_exec (db, "create index tc_txn_id_active on type_catalog (txn_id, active)", callback, 0, &ErrMsg);
    //     if (rc != SQLITE_OK)    {
    //         fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
    //         sqlite3_free (ErrMsg);
    //         sqlite3_close (db);
    //         goto cleanup;
    //     }

    //     rc = sqlite3_exec (db, "create index rac_txn_id_active on run_attribute_catalog (txn_id, active)", callback, 0, &ErrMsg);
    //     if (rc != SQLITE_OK)    {
    //         fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
    //         sqlite3_free (ErrMsg);
    //         sqlite3_close (db);
    //         goto cleanup;
    //     }

    //     rc = sqlite3_exec (db, "create index tac_txn_id_active on timestep_attribute_catalog (txn_id, active)", callback, 0, &ErrMsg);
    //     if (rc != SQLITE_OK)    {
    //         fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
    //         sqlite3_free (ErrMsg);
    //         sqlite3_close (db);
    //         goto cleanup;
    //     }

    //     rc = sqlite3_exec (db, "create index vac_txn_id_active on var_attribute_catalog (txn_id, active)", callback, 0, &ErrMsg);
    //     if (rc != SQLITE_OK)    {
    //         fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
    //         sqlite3_free (ErrMsg);
    //         sqlite3_close (db);
    //         goto cleanup;
    //     }

    // //ACTIVE/////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //     rc = sqlite3_exec (db, "create index rc_active on run_catalog (active)", callback, 0, &ErrMsg);
    //     if (rc != SQLITE_OK)    {
    //         fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
    //         sqlite3_free (ErrMsg);
    //         sqlite3_close (db);
    //         goto cleanup;
    //     }

    //     rc = sqlite3_exec (db, "create index tmc_active on timestep_catalog (active)", callback, 0, &ErrMsg);
    //     if (rc != SQLITE_OK)    {
    //         fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
    //         sqlite3_free (ErrMsg);
    //         sqlite3_close (db);
    //         goto cleanup;
    //     }

    //     rc = sqlite3_exec (db, "create index vc_active on var_catalog (active)", callback, 0, &ErrMsg);
    //     if (rc != SQLITE_OK)    {
    //         fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
    //         sqlite3_free (ErrMsg);
    //         sqlite3_close (db);
    //         goto cleanup;
    //     }

    //     rc = sqlite3_exec (db, "create index tc_active on type_catalog (active)", callback, 0, &ErrMsg);
    //     if (rc != SQLITE_OK)    {
    //         fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
    //         sqlite3_free (ErrMsg);
    //         sqlite3_close (db);
    //         goto cleanup;
    //     }

    //     rc = sqlite3_exec (db, "create index rac_active on run_attribute_catalog (active)", callback, 0, &ErrMsg);
    //     if (rc != SQLITE_OK)    {
    //         fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
    //         sqlite3_free (ErrMsg);
    //         sqlite3_close (db);
    //         goto cleanup;
    //     }

    //     rc = sqlite3_exec (db, "create index tac_active on timestep_attribute_catalog (active)", callback, 0, &ErrMsg);
    //     if (rc != SQLITE_OK)    {
    //         fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
    //         sqlite3_free (ErrMsg);
    //         sqlite3_close (db);
    //         goto cleanup;
    //     }

    //     rc = sqlite3_exec (db, "create index vac_active on var_attribute_catalog (active)", callback, 0, &ErrMsg);
    //     if (rc != SQLITE_OK)    {
    //         fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
    //         sqlite3_free (ErrMsg);
    //         sqlite3_close (db);
    //         goto cleanup;
    //     }

    //DIMS/////////////////////////////////////////////////////////////////////////////////////////////////////////////

        // rc = sqlite3_exec (db, "create index vac_dims on var_attribute_catalog (d0_min, d0_max, d1_min, d1_max, d2_min, d2_max)", callback, 0, &ErrMsg);
        // if (rc != SQLITE_OK)    {
        //     fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
        //     sqlite3_free (ErrMsg);
        //     sqlite3_close (db);
        //     goto cleanup;
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

    rc = sqlite3_exec (db, "create index vac_all_cols on var_attribute_catalog (timestep_id, type_id, var_id, d0_min)", callback, 0, &ErrMsg);
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
    // if (rc != SQLITE_OK)    {
    //     fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
    //     sqlite3_free (ErrMsg);
    //     sqlite3_close (db);
    //     goto cleanup;
    // }

    rc = sqlite3_exec (db, "create index tmc_run on timestep_catalog (run_id)", callback, 0, &ErrMsg);
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

    // rc = sqlite3_exec (db, "create index tac_type_id on timestep_attribute_catalog(type_id)", callback, 0, &ErrMsg);
    // if (rc != SQLITE_OK)    {
    //     fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
    //     sqlite3_free (ErrMsg);
    //     sqlite3_close (db);
    //     goto cleanup;
    // }

    rc = sqlite3_exec (db, "create index vac_varid_d0 on var_attribute_catalog(var_id, d0_min)", callback, 0, &ErrMsg);
    if (rc != SQLITE_OK)    {
        fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
        sqlite3_free (ErrMsg);
        sqlite3_close (db);
        goto cleanup;
    }
     add_timing_point(DB_CREATE_READING_INDICES_DONE);

cleanup:
    return rc;
}

// int metadata_finalize_server(int proc_rank, uint64_t job_id, md_db_index_type index_type, md_db_checkpoint_type checkpt_type)
int metadata_finalize_server(int proc_rank, uint64_t job_id)
{   

    int rc = RC_OK;
    add_timing_point(SHUTDOWN_START);

    int num_procs = 1;
    if(use_mpi) {
        MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
    }


    int num_db_pts = 9;
    uint64_t *all_db_sizes;

    if(output_timing) {
        gather_db_statistics(&all_db_sizes, proc_rank, num_procs);
    }
    sqlite3_close (db);
    add_timing_point(DB_CLOSED_SHUTDOWN_DONE);
    //note - if we knew for a fact that the db would fit in mem, could do this before closing
    if(index_type == WRITE_DELAYED_INDEX && db_output_is_whole()) {
        // string filename = get_filename_base(proc_rank, job_id, checkpt_type) + to_string(checkpt_count);
        string filename = get_filename_base(proc_rank, job_id) + to_string(checkpt_count);
        rc = sqlite3_open(filename.c_str(), &db);
        rc = sqlite3_exec (db, "PRAGMA temp_store = FILE", callback, 0, 0);
        if (rc != SQLITE_OK)    {
            fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, sqlite3_errmsg (db));
            sqlite3_close (db);
            return RC_ERR;
        }
        rc = create_reading_indices();
        sqlite3_close (db);
    }
    else if (!db_output_is_whole()) {
        //debug_log << "db isn't whole. checkpt_type: " << checkpt_type << endl;
        // rc = db_compact_and_output(proc_rank, job_id, index_type, checkpt_type);
        rc = db_compact_and_output(proc_rank, job_id);
    }

    if (proc_rank == 0 && output_timing) {
        //prevent it from buffering the printf statements

        cout << (int)DB_SIZES << " ";
        for(int i=0; i<(num_db_pts*num_procs); i++) {
            std::cout << all_db_sizes[i] << " ";
            if (i%20 == 0 && i!=0) {
                std::cout << std::endl;
            }
        }
        std::cout << std::endl;
    }
    return rc;
}

static void gather_db_statistics( uint64_t **all_sizes, int proc_rank, int num_procs)
{
    int rc;

    int num_db_pts = 9;
    uint64_t db_sizes[num_db_pts];
    db_sizes[0] = init_db_size;

    db_sizes[1] = sqlite3_memory_used();

    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;


    rc = sqlite3_prepare_v2 (db, "select count (*) from run_catalog", -1, &stmt, &tail); assert (rc == SQLITE_OK);
    rc = sqlite3_step (stmt); assert (rc == SQLITE_ROW || rc == SQLITE_DONE);
    db_sizes[2] = sqlite3_column_int (stmt, 0);
    rc = sqlite3_finalize (stmt); assert (rc == SQLITE_OK);

    rc = sqlite3_prepare_v2 (db, "select count (*) from timestep_catalog", -1, &stmt, &tail); assert (rc == SQLITE_OK);
    rc = sqlite3_step (stmt); assert (rc == SQLITE_ROW || rc == SQLITE_DONE);
    db_sizes[3] = sqlite3_column_int (stmt, 0);
    rc = sqlite3_finalize (stmt); assert (rc == SQLITE_OK);

    rc = sqlite3_prepare_v2 (db, "select count (*) from var_catalog", -1, &stmt, &tail); assert (rc == SQLITE_OK);
    rc = sqlite3_step (stmt); assert (rc == SQLITE_ROW || rc == SQLITE_DONE);
    db_sizes[4] = sqlite3_column_int (stmt, 0);
    rc = sqlite3_finalize (stmt); assert (rc == SQLITE_OK);

    rc = sqlite3_prepare_v2 (db, "select count (*) from type_catalog", -1, &stmt, &tail); assert (rc == SQLITE_OK);
    rc = sqlite3_step (stmt); assert (rc == SQLITE_ROW || rc == SQLITE_DONE);
    db_sizes[5] = sqlite3_column_int (stmt, 0);
    rc = sqlite3_finalize (stmt); assert (rc == SQLITE_OK);

    rc = sqlite3_prepare_v2 (db, "select count (*) from run_attribute_catalog", -1, &stmt, &tail); assert (rc == SQLITE_OK);
    rc = sqlite3_step (stmt); assert (rc == SQLITE_ROW || rc == SQLITE_DONE);
    db_sizes[6] = sqlite3_column_int (stmt, 0);
    rc = sqlite3_finalize (stmt); assert (rc == SQLITE_OK);

    rc = sqlite3_prepare_v2 (db, "select count (*) from timestep_attribute_catalog", -1, &stmt, &tail); assert (rc == SQLITE_OK);
    rc = sqlite3_step (stmt); assert (rc == SQLITE_ROW || rc == SQLITE_DONE);
    db_sizes[7] = sqlite3_column_int (stmt, 0);
    rc = sqlite3_finalize (stmt); assert (rc == SQLITE_OK);

    rc = sqlite3_prepare_v2 (db, "select count (*) from var_attribute_catalog", -1, &stmt, &tail); assert (rc == SQLITE_OK);
    rc = sqlite3_step (stmt); assert (rc == SQLITE_ROW || rc == SQLITE_DONE);
    db_sizes[8] = sqlite3_column_int (stmt, 0);
    rc = sqlite3_finalize (stmt); assert (rc == SQLITE_OK);   

    debug_log << "sizeof db: " << db_sizes[1] << endl;
    debug_log << "num runs: " << db_sizes[2] << " num timesteps: " << db_sizes[3] << " num vars: " << db_sizes[4] << 
        " num types: " << db_sizes[5] << " num run attributes: " << db_sizes[6] << " num timestep attributes: " << db_sizes[7] <<  
        " num var attributes: " << db_sizes[8] << endl;

    //needed by proc_rank 0
    uint64_t *all_db_sizes;

    if(proc_rank == 0) {
        all_db_sizes = (uint64_t *) malloc(num_procs * num_db_pts * sizeof(uint64_t));
    }

    if(use_mpi) {
        MPI_Gather(db_sizes, num_db_pts, MPI_UINT64_T, all_db_sizes, num_db_pts, MPI_UINT64_T, 0, MPI_COMM_WORLD);
    }

    add_timing_point(DB_ANALYSIS_DONE);

    *all_sizes = all_db_sizes;

}



int sql_load_db(uint64_t job_id, int proc_rank)
{
    int rc;
    sqlite3 *pFile;           /* Database connection  */
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
        // rc = sqlite3_errcode(pTo);
        rc = sqlite3_errcode(db);
    }

    /* Close the database connection opened on database file zFilename
    ** and return the result of this function. */
    (void)sqlite3_close(pFile);

    add_timing_point(DB_LOAD_DONE);

    return rc;

}


int sql_load_multi_db(uint64_t job_id, uint32_t num_read_procs, int proc_rank)
{
    int rc;
    sqlite3 *pFile;           /* Database connection  */
    sqlite3_backup *pBackup;  /* Backup object used to copy data */

    uint64_t num_write_procs;

    add_timing_point(DB_LOAD_MULTI_START);


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
        // rc = sqlite3_errcode(pTo);
        rc = sqlite3_errcode(db);
    }

    /* Close the database connection opened on database file zFilename
    ** and return the result of this function. */
    (void)sqlite3_close(pFile);


    rc = md_catalog_num_procs (job_id, num_write_procs );
    if (rc != RC_OK)
    {   
        error_log << "error. was unable to catalog the number of procs" << endl;
        return rc;
    }
    debug_log << "num_write_procs: " << num_write_procs << endl;


    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;
    char  *ErrMsg = NULL;

    rc = sqlite3_prepare_v2 (db, "attach database ? as db_temp", -1, &stmt, &tail);
    if (rc != SQLITE_OK)
    {
        fprintf (stderr, "Error start of create_run_stub: Line: %d SQL error: %s\n", __LINE__, sqlite3_errmsg (db));
        sqlite3_close (db);
        return rc;
    }

    for(int write_rank = proc_rank + num_read_procs; write_rank < num_write_procs; write_rank += num_read_procs) {

        string filename_temp = to_string(job_id) + "_" + to_string(write_rank);
        debug_log << "filename: " << strdup(filename_temp.c_str()) << endl;

        rc = sqlite3_bind_text (stmt, 1, strdup(filename_temp.c_str()), -1, free); assert (rc == SQLITE_OK);
        rc = sqlite3_step (stmt); assert (rc == SQLITE_ROW || rc == SQLITE_DONE);
        debug_log << "about to reset" << endl;
        rc = sqlite3_reset(stmt); assert (rc == SQLITE_OK);

        rc = sqlite3_exec (db, "insert into run_attribute_catalog select * from db_temp.run_attribute_catalog", callback, 0, &ErrMsg);
        if (rc != SQLITE_OK)    {
            fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
            sqlite3_free (ErrMsg);
            sqlite3_close (db);
            return rc;
        }

        rc = sqlite3_exec (db, "insert into timestep_attribute_catalog select * from db_temp.timestep_attribute_catalog", callback, 0, &ErrMsg);
        if (rc != SQLITE_OK)    {
            fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
            sqlite3_free (ErrMsg);
            sqlite3_close (db);
            return rc;
        }

        rc = sqlite3_exec (db, "insert into var_attribute_catalog select * from db_temp.var_attribute_catalog", callback, 0, &ErrMsg);
        if (rc != SQLITE_OK)    {
            fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
            sqlite3_free (ErrMsg);
            sqlite3_close (db);
            return rc;
        }

        rc = sqlite3_exec (db, "detach database db_temp", callback, 0, &ErrMsg);
        if (rc != SQLITE_OK)    {
            fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
            sqlite3_free (ErrMsg);
            sqlite3_close (db);
            return rc;
        }
    }
    
    rc = sqlite3_finalize(stmt); assert (rc == SQLITE_OK);

    debug_log << "about to return" << endl;

    add_timing_point(DB_LOAD_MULTI_DONE);

    return rc;

}


int md_catalog_num_procs (uint64_t job_id, uint64_t &num_write_procs )
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
        sqlite3_close (db);
        goto cleanup;
    }
    rc = sqlite3_bind_int64 (stmt, 1, job_id); assert (rc == SQLITE_OK);
    rc = sqlite3_step (stmt); assert (rc == SQLITE_ROW || rc == SQLITE_DONE);

    
    num_write_procs = sqlite3_column_int64 (stmt, 0);

    rc = sqlite3_finalize (stmt);  
    

cleanup:

    return rc;
}




static int db_copy(string filename)
{
    int rc;
    sqlite3 *pFile;           /* Database connection  */
    sqlite3_backup *pBackup;  /* Backup object used to copy data */

     // string filename = to_string(job_id) + "_" + to_string(proc_rank);
    /* Open the database file. Exit early if this fails
      ** for any reason. */
    rc = sqlite3_open(filename.c_str(), &pFile);
    // debug_log << "just opened file: " << filename << " for checkpointing" << endl;
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



// int db_checkpoint_copy(int proc_rank, uint64_t job_id, md_server_type server_type, md_db_index_type index_type)
int db_checkpoint_copy(int proc_rank, uint64_t job_id)
{
    int rc;
    //debug_log << "about to start outputting the db" << endl;

    //we want both the timing and the amount of memory used by the DB, so we can see
    //how the size affects the time
    uint64_t db_size = get_db_size();
    add_db_output_timing_point(DB_CHECKPT_COPY_START, db_size);

    if(first_checkpt) {
        checkpt_count += 1;
        first_checkpt = false;
    }
    string filename = to_string(job_id) + "_" + to_string(proc_rank) + "_" + to_string(checkpt_count);

    rc = db_copy(filename);

    // if(get_mem_usage() > MAX_MEM_USAGE) {
    if(db_size > MAX_DB_SIZE) {
        // get_mem_usage();
        // rc = db_reset(proc_rank, job_id, server_type, index_type);
        rc = db_reset(proc_rank, job_id);
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

// int db_checkpoint_incremental_output( int proc_rank, uint64_t job_id, md_server_type server_type, md_db_index_type index_type)
int db_checkpoint_incremental_output( int proc_rank, uint64_t job_id)
{
    int rc;
    uint64_t db_size =  get_db_size();
    add_db_output_timing_point(DB_CHECKPT_INCREMENTAL_OUTPUT_START, db_size);

    //debug_log << "about to start outputing new db" << endl;

    if(first_checkpt) {
        checkpt_count += 1;
        string filename = to_string(job_id) + "_" + to_string(proc_rank) + "_incr_" + to_string(checkpt_count);

        rc = db_copy(filename);
        first_checkpt = false;
        //debug_log << "just did original output. rc: " << rc << endl;
    }
    else {
        string filename = to_string(job_id) + "_" + to_string(proc_rank) + "_incr_" + to_string(checkpt_count);
        //debug_log << "about to do incr_output for filename: " << filename << endl;
        rc = db_incr_output(filename);
        //debug_log << "just did incremental output. rc: " << rc << endl;
    }

    // if(get_mem_usage() > MAX_MEM_USAGE) {
    if(db_size > MAX_DB_SIZE) {
        // get_mem_usage();
        // rc = db_reset(proc_rank, job_id, server_type, index_type);
        rc = db_reset(proc_rank, job_id);
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

int db_checkpoint_copy_and_delete(int proc_rank, uint64_t job_id)
{
    add_db_output_timing_point(DB_CHECKPT_COPY_AND_DELETE_START, get_db_size());

    int rc;
    checkpt_count += 1;

    string filename = to_string(job_id) + "_" + to_string(proc_rank) + "_del_" + to_string(checkpt_count);

    rc = db_copy(filename);
    if(rc != RC_OK) {
        error_log << "error copying the db in db_checkpoint_copy_and_delete" << endl;
        return rc;
    }
    rc = db_delete();
    if(rc != RC_OK) {
        error_log << "error in db_delete in db_checkpoint_copy_and_delete" << endl;
        return rc;
    }

    add_timing_point(DB_CHECKPT_COPY_AND_DELETE_DONE);

    return rc;
};


// int db_checkpoint_copy_and_reset(int proc_rank, uint64_t job_id, md_server_type server_type, md_db_index_type index_type)
int db_checkpoint_copy_and_reset(int proc_rank, uint64_t job_id)
{   
    add_db_output_timing_point(DB_CHECKPT_COPY_AND_RESET_START, get_db_size());

    int rc;
    checkpt_count += 1;

    string filename = to_string(job_id) + "_" + to_string(proc_rank) + "_reset_" + to_string(checkpt_count);


    rc = db_copy(filename);
    if(rc != RC_OK) {
        error_log << "error copying the db in db_checkpoint_copy_and_delete" << endl;
        return rc;
    }

    // rc = db_reset(proc_rank, job_id, server_type, index_type);
    rc = db_reset(proc_rank, job_id);
    if(rc != RC_OK) {
        error_log << "error resetting the db in db_checkpoint_copy_and_delete" << endl;
        return rc;
    }
    
    add_timing_point(DB_CHECKPT_COPY_AND_RESET_DONE);
    return rc;

};

int db_checkpoint_incremental_output_and_delete(int proc_rank, uint64_t job_id)
{
    add_db_output_timing_point(DB_CHECKPT_INCREMENTAL_OUTPUT_AND_DELETE_START, get_db_size());

    int rc;
    // string filename = to_string(job_id) + "_" + to_string(proc_rank) + "_incr_del_" + to_string(checkpt_count);
    string filename = to_string(job_id) + "_" + to_string(proc_rank) + "_incr_del_0";

    if(first_checkpt) {
        rc = db_copy(filename);
        first_checkpt = false;
        //debug_log << "just did original output. rc: " << rc << endl;
    }
    else {
        rc = db_incr_output_delete(filename);
        //debug_log << "just did incremental output. rc: " << rc << endl;
    }
    rc = db_delete();
    if(rc != RC_OK) {
        error_log << "error in db_delete in db_checkpoint_incremental_output_and_delete" << endl;
        return rc;
    }

    add_timing_point(DB_CHECKPT_INCREMENTAL_OUTPUT_AND_DELETE_DONE);
    return rc;

};


// int db_checkpoint_incremental_output_and_reset(int proc_rank, uint64_t job_id, md_server_type server_type, md_db_index_type index_type)
int db_checkpoint_incremental_output_and_reset(int proc_rank, uint64_t job_id)
{   
    add_db_output_timing_point(DB_CHECKPT_INCREMENTAL_OUTPUT_AND_RESET_START, get_db_size());

    int rc;
    // string filename = to_string(job_id) + "_" + to_string(proc_rank) + "_incr_reset_" + to_string(checkpt_count);
    string filename = to_string(job_id) + "_" + to_string(proc_rank) + "_incr_reset_0";

    if(first_checkpt) {
        rc = db_copy(filename);
        first_checkpt = false;
        //debug_log << "just did original output. rc: " << rc << endl;
    }
    else {
        rc = db_incr_output_delete(filename);
        //debug_log << "just did incremental output. rc: " << rc << endl;
    }
    // rc = db_reset(proc_rank, job_id, server_type, index_type);
    rc = db_reset(proc_rank, job_id);
    if(rc != RC_OK) {
        error_log << "error copying the db in db_checkpoint_copy_and_delete" << endl;
        return rc;
    }

    add_timing_point(DB_CHECKPT_INCREMENTAL_OUTPUT_AND_RESET_DONE);
    return rc;
};



static int db_incr_output(string filename)
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


    rc = get_seq_values(DB_OUTPUT, last_run_id, last_type_id, last_run_attr_id,
                    last_timestep_attr_id, last_var_attr_id, contains_types, contains_run_attrs,
                    contains_timestep_attrs, contains_var_attrs );

    rc = get_last_timestep(DB_OUTPUT, last_run_id, last_timestep_id);

    rc = insert_new(last_run_id, last_timestep_id, last_type_id,
                    last_run_attr_id, last_timestep_attr_id, last_var_attr_id,
                    contains_types, contains_run_attrs, contains_timestep_attrs, contains_var_attrs
                    );

    rc = sqlite3_exec (db, "detach database db_output", callback, 0, &ErrMsg);
    if (rc != SQLITE_OK)    {
        fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
        sqlite3_free (ErrMsg);
        sqlite3_close (db);
        return rc;
    }
    
    // if(contains_run_attrs) {
    //  //debug_log << "last_run_attr_id: " << last_run_attr_id;
    // }
    // if(contains_timestep_attrs) {
    //  //debug_log <<  " last_timestep_attr_id: " << last_timestep_attr_id;
    // }
    // if(contains_timestep_attrs) {
    //  //debug_log <<  " last_var_attr_id: " << last_var_attr_id;
    // }
    // if(contains_types) {
    //  //debug_log << " last_type_id: " << last_type_id << endl;
    // }
    // //debug_log << " last_run_id: " << last_run_id << " last_timestep_id: " << last_timestep_id << endl;

    return rc;

}

static int db_incr_output_new_file(string output_filename, string prev_filename)
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

    rc = sqlite3_prepare_v2 (db, "attach database ? as db_prev", -1, &stmt, &tail);
    if (rc != SQLITE_OK)
    {
        fprintf (stderr, "Error start of attach db: Line: %d SQL error: %s\n", __LINE__, sqlite3_errmsg (db));
        sqlite3_close (db);
        return rc;
    }
    rc = sqlite3_bind_text (stmt, 1, strdup(prev_filename.c_str()), -1, free); assert (rc == SQLITE_OK);
    rc = sqlite3_step (stmt); assert (rc == SQLITE_ROW || rc == SQLITE_DONE);
    rc = sqlite3_finalize(stmt); assert (rc == SQLITE_OK);

    //note - if we were comfortable keeping all of these as global vars
    rc = get_seq_values(DB_PREV, last_run_id, last_type_id, last_run_attr_id,
                    last_timestep_attr_id, last_var_attr_id, contains_types, contains_run_attrs,
                    contains_timestep_attrs, contains_var_attrs );

    rc = get_last_timestep(DB_PREV, last_run_id, last_timestep_id);
    //debug_log << "last_timestep_id: " << last_timestep_id << endl;


    rc = sqlite3_exec (db, "detach database db_prev", callback, 0, &ErrMsg);
    if (rc != SQLITE_OK)    {
        fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
        sqlite3_free (ErrMsg);
        sqlite3_close (db);
        return rc;
    }
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // INSERT INTO X.TABLE SELECT * FROM Y.TABLE;

    //need to output run attrs, timestep attrs, new timesteps, vars for new timesteps, new var attrs, types

    rc = sqlite3_prepare_v2 (db, "attach database ? as db_output", -1, &stmt, &tail);
    if (rc != SQLITE_OK)
    {
        fprintf (stderr, "Error start of attach db: Line: %d SQL error: %s\n", __LINE__, sqlite3_errmsg (db));
        sqlite3_close (db);
        return rc;
    }
    rc = sqlite3_bind_text (stmt, 1, strdup(output_filename.c_str()), -1, free); assert (rc == SQLITE_OK);
    rc = sqlite3_step (stmt); assert (rc == SQLITE_ROW || rc == SQLITE_DONE);
    rc = sqlite3_finalize(stmt); assert (rc == SQLITE_OK);


    rc = insert_new(last_run_id, last_timestep_id, last_type_id,
                    last_run_attr_id, last_timestep_attr_id, last_var_attr_id,
                    contains_types, contains_run_attrs, contains_timestep_attrs, contains_var_attrs
                    );


    rc = sqlite3_exec (db, "detach database db_output", callback, 0, &ErrMsg);
    if (rc != SQLITE_OK)    {
        fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
        sqlite3_free (ErrMsg);
        sqlite3_close (db);
        return rc;
    }
    
    // if(contains_run_attrs) {
    //  //debug_log << "last_run_attr_id: " << last_run_attr_id;
    // }
    // if(contains_timestep_attrs) {
    //  //debug_log <<  " last_timestep_attr_id: " << last_timestep_attr_id;
    // }
    // if(contains_timestep_attrs) {
    //  //debug_log <<  " last_var_attr_id: " << last_var_attr_id;
    // }
    // if(contains_types) {
    //  //debug_log << " last_type_id: " << last_type_id << endl;
    // }
    // //debug_log << " last_run_id: " << last_run_id << " last_timestep_id: " << last_timestep_id << endl;

    return rc;

}

static int db_delete()
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

// static int db_reset(int proc_rank, uint64_t job_id, md_server_type server_type, md_db_index_type index_type)
int db_reset(int proc_rank, uint64_t job_id)
{
    add_timing_point(DB_RESET_START);

    int rc;
    // sqlite3_stmt * stmt = NULL;
 //    const char * tail = NULL;
    // char  *ErrMsg = NULL;

    uint64_t last_run_attr_id, last_timestep_attr_id, last_timestep_id, last_run_id, last_type_id;
    uint64_t last_var_attr_id;
    bool contains_run_attrs = false, contains_timestep_attrs = false, contains_var_attrs = false, contains_types = false;

    rc = get_seq_values(DB_IN_MEM, last_run_id, last_type_id, last_run_attr_id,
                    last_timestep_attr_id, last_var_attr_id, contains_types, contains_run_attrs,
                    contains_timestep_attrs, contains_var_attrs );
    if(rc != RC_OK) {
        error_log << "error in get_seq_values in db_reset" << endl;
        return rc;
    }
    sqlite3_close (db);

    // rc = metadata_database_init(proc_rank, job_id, server_type, index_type);
    rc = metadata_database_init(proc_rank, job_id);
    if(rc != RC_OK) {
        error_log << "error in metadata_database_init in db_reset" << endl;
        return rc;
    }

    rc = update_seq_values(last_run_id, last_type_id, last_run_attr_id, last_timestep_attr_id, 
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


static int get_seq_values(md_db_type db_type, uint64_t &last_run_id, uint64_t &last_type_id, 
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

static int update_seq_values(uint64_t last_run_id, uint64_t last_type_id, uint64_t last_run_attr_id, uint64_t last_timestep_attr_id, 
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

static int get_run_id(uint64_t job_id, uint64_t &last_run_id )
{

    int rc;
    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;

    rc = sqlite3_prepare_v2 (db, "select id from run_catalog where job_id = ? limit 1", -1, &stmt, &tail);
    if (rc != SQLITE_OK)
    {
        fprintf (stderr, "Error start of select get_run_id. Line: %d SQL error: %s\n", __LINE__, sqlite3_errmsg (db));
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


static int get_last_timestep(md_db_type db_type, uint64_t last_run_id, uint64_t &last_timestep_id)
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

static int insert_new(uint64_t last_run_id, uint64_t last_timestep_id, uint64_t last_type_id, 
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


static int insert_all_new()
{

    int rc;
    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;

    rc = sqlite3_prepare_v2 (db, "insert into db_output.run_attribute_catalog select * from run_attribute_catalog", -1, &stmt, &tail); 
    if(rc != SQLITE_OK) {
        sqlite3_extended_result_codes(db, 1);
        cout << "rc: " << rc << ", errMsg: " << sqlite3_errmsg (db) << endl;
    }
    assert (rc == SQLITE_OK);
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

static int db_incr_output_delete(string filename)
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

    rc = insert_all_new();

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


int md_catalog_num_timesteps (uint64_t job_id, uint64_t &num_timesteps )
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


int db_compact(string filename_base, uint64_t job_id, string compacted_filename)
{
    int rc;
    sqlite3 *pFile;           /* Database connection  */
    sqlite3_backup *pBackup;  /* Backup object used to copy data */

    uint64_t run_id;
    // uint64_t num_timesteps;

    add_timing_point(DB_COMPACT_START);

    //note - if we knew the total size would be less than the max avail, could do this in memory
    rc = sqlite3_open (compacted_filename.c_str(), &db);
    if (rc != SQLITE_OK)
    {   
        fprintf (stderr, "Can't open database: %s\n in db_compact", sqlite3_errmsg (db));        
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


    rc = get_run_id(job_id, run_id );
    //debug_log << "run_id: " << run_id << endl;


    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;
    char  *ErrMsg = NULL;

    rc = sqlite3_prepare_v2 (db, "attach database ? as db_input", -1, &stmt, &tail);
    if (rc != SQLITE_OK)
    {
        fprintf (stderr, "Error start of create_run_stub: Line: %d SQL error: %s\n", __LINE__, sqlite3_errmsg (db));
        sqlite3_close (db);
        return rc;
    }

    for(int checkpt_num = 1; checkpt_num <= checkpt_count; checkpt_num += 1) {

        string filename_temp = filename_base + to_string(checkpt_num);
        //debug_log << "filename: " << strdup(filename_temp.c_str()) << endl;

        rc = sqlite3_bind_text (stmt, 1, strdup(filename_temp.c_str()), -1, free); assert (rc == SQLITE_OK);
        rc = sqlite3_step (stmt); assert (rc == SQLITE_ROW || rc == SQLITE_DONE);
        //debug_log << "about to reset" << endl;
        rc = sqlite3_reset(stmt); assert (rc == SQLITE_OK);


        rc = sqlite3_exec (db, "insert into timestep_catalog select * from db_input.timestep_catalog", callback, 0, &ErrMsg);
        if (rc != SQLITE_OK)    {
            fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
            sqlite3_free (ErrMsg);
            sqlite3_close (db);
            return rc;
        }

        rc = sqlite3_exec (db, "insert into var_catalog select * from db_input.var_catalog", callback, 0, &ErrMsg);
        if (rc != SQLITE_OK)    {
            fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
            sqlite3_free (ErrMsg);
            sqlite3_close (db);
            return rc;
        }

        //note - we would assume that no types would be inserted part way through the run 
        // (we assume all posibilites are inserted at the beginning), but will leave this here to allow for the possibility
        rc = sqlite3_exec (db, "insert into type_catalog select * from db_input.type_catalog", callback, 0, &ErrMsg);
        if (rc != SQLITE_OK)    {
            fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
            sqlite3_free (ErrMsg);
            sqlite3_close (db);
            return rc;
        }

        rc = sqlite3_exec (db, "insert into run_attribute_catalog select * from db_input.run_attribute_catalog", callback, 0, &ErrMsg);
        if (rc != SQLITE_OK)    {
            fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
            sqlite3_free (ErrMsg);
            sqlite3_close (db);
            return rc;
        }

        rc = sqlite3_exec (db, "insert into timestep_attribute_catalog select * from db_input.timestep_attribute_catalog", callback, 0, &ErrMsg);
        if (rc != SQLITE_OK)    {
            fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
            sqlite3_free (ErrMsg);
            sqlite3_close (db);
            return rc;
        }

        rc = sqlite3_exec (db, "insert into var_attribute_catalog select * from db_input.var_attribute_catalog", callback, 0, &ErrMsg);
        if (rc != SQLITE_OK)    {
            fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
            sqlite3_free (ErrMsg);
            sqlite3_close (db);
            return rc;
        }

        rc = sqlite3_exec (db, "detach database db_input", callback, 0, &ErrMsg);
        if (rc != SQLITE_OK)    {
            fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
            sqlite3_free (ErrMsg);
            sqlite3_close (db);
            return rc;
        }
    }
    
    rc = sqlite3_finalize(stmt); assert (rc == SQLITE_OK);

    //debug_log << "about to return" << endl;

    add_timing_point(DB_COMPACT_DONE);

    return rc;

}

// static int db_compact_and_output(int proc_rank, uint64_t job_id, md_db_index_type index_type, md_db_checkpoint_type checkpt_type)
static int db_compact_and_output(int proc_rank, uint64_t job_id)

{
    int rc;

    add_timing_point(DB_COMPACT_AND_OUTPUT_START);

    string filename_compacted = to_string(job_id) + "_" + to_string(proc_rank) + "_compact";
    // string filename_base = get_filename_base(proc_rank, job_id, checkpt_type);
    string filename_base = get_filename_base(proc_rank, job_id);

    rc = db_compact(filename_base, job_id, filename_compacted);
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
        rc = create_reading_indices();
    }

    // rc = db_copy(filename_compacted);
    // if(rc != RC_OK) {
    //     error_log << "error in db_copy in db_compact_and_output" << endl;
    // }
    sqlite3_close (db);

    add_timing_point(DB_COMPACT_AND_OUTPUT_DONE);

    return rc;
}

// static sqlite3_int64 get_db_size ()
sqlite3_int64 get_db_size ()
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

// static string get_filename_base(int proc_rank, uint64_t job_id, md_db_checkpoint_type checkpt_type)
static string get_filename_base(int proc_rank, uint64_t job_id)
{
    string filename_base;
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

int metadata_checkpoint_database(int proc_rank, uint64_t job_id)
{
    int return_value;
    
    switch(checkpt_type) {
        case DB_COPY: {
            // return_value = db_checkpoint_copy(proc_rank, job_id, write_type);
            return_value = db_checkpoint_copy(proc_rank, job_id);
            break;
        }
        case DB_INCR_OUTPUT: {
            // return_value = db_checkpoint_incremental_output(proc_rank, job_id, write_type);
            return_value = db_checkpoint_incremental_output(proc_rank, job_id);
            break;
        }           
        case DB_COPY_AND_DELETE: {
            return_value = db_checkpoint_copy_and_delete(proc_rank, job_id);
            break;
        }               
        case DB_COPY_AND_RESET: {
            // return_value = db_checkpoint_copy_and_reset(proc_rank, job_id, write_type);
            return_value = db_checkpoint_copy_and_reset(proc_rank, job_id);
            break;
        }               
        case DB_INCR_OUTPUT_AND_DELETE: {
            return_value = db_checkpoint_incremental_output_and_delete(proc_rank, job_id);
            break;
        }               
        case DB_INCR_OUTPUT_AND_RESET: {
            // return_value = db_checkpoint_incremental_output_and_reset(proc_rank, job_id, write_type);
            return_value = db_checkpoint_incremental_output_and_reset(proc_rank, job_id);
            break;
        }
    }
    return return_value;
}

