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
#include <libops.hh>

#include <sqlite3.h>

using namespace std;


sqlite3 * db = NULL;
int proc_rank;
// int checkpt_count = 0;

bool debug_logging = false;
bool error_logging = true;

//link to testing_debug.cpp in cmake to do debug tests
// static bool do_debug_testing = false;


struct debug_log {
  template<typename T> debug_log& operator << (const T& x) {
   if(debug_logging) {
      std::cout << x;
    }
    return *this;
  }
  debug_log& operator<<(std::ostream& (*manipulator)(std::ostream&)) {
    if(debug_logging) {
      std::cout << manipulator;
    }
    return *this;
  }
} debug_log;

// struct extreme_debug_log {
//   template<typename T> extreme_debug_log& operator << (const T& x) {
//    if(md_extreme_debug_logging) {
//       std::cout << x;
//     }
//     return *this;
//   }
//   extreme_debug_log& operator<<(std::ostream& (*manipulator)(std::ostream&)) {
//     if(md_extreme_debug_logging) {
//       std::cout << manipulator;
//     }
//     return *this;
//   }
// } extreme_debug_log;


struct error_log {
  template<typename T> error_log& operator << (const T& x) {
    if(error_logging) {
      std::cout << x;
    }
    return *this;
  }
  error_log& operator<<(std::ostream& (*manipulator)(std::ostream&)) {
    if(error_logging) {
      std::cout << manipulator;
    }
     return *this;
  }
} error_log;

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


static int metadata_database_init ();
static int setup_dirman(const string &dirman_file_path, const string &app_name, int rank);

static int sql_load_db(sqlite3 *pInMemory, uint64_t job_id, int rank);

//===========================================================================
static int metadata_database_init (uint64_t job_id, int rank)
{
    //setup the database
    int rc;
    char  *ErrMsg = NULL;
    
    rc = sqlite3_open (":memory:", &db);
    
    if (rc != SQLITE_OK)
    {   
        fprintf (stderr, "Can't open database: %s\n", sqlite3_errmsg (db));        
        sqlite3_close (db);
        goto cleanup;
    }

    rc = sql_load_db(db, job_id, rank);
    while (rc == SQLITE_LOCKED || rc == SQLITE_BUSY) {
        if (rc == SQLITE_LOCKED) {
            error_log << "rank: " << rank << " db is locked. am waiting" << endl;
        }
        else {
             error_log << "rank: " << rank << "db is busy. am waiting" << endl;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    if (rc != SQLITE_OK)
    {   
        fprintf (stderr, "Can't load database for job_id:%llu: %s\n",job_id, sqlite3_errmsg (db));        
        sqlite3_close (db);
        goto cleanup;
    }


cleanup:
    return rc;
}




//argv[0] = name of program, argv[1]=directory manager path
int main (int argc, char **argv)
{
    char name[100];
    gethostname(name, sizeof(name));
    debug_log << name << endl;
    
    if (argc != 2)
    {
        // cout << 0 << " " << (int)ERR_ARGC << endl;
        error_log << "Usage: " << argv[0] << " job_id \n";
        return RC_ERR;
    }

    uint64_t job_id = atoi(argv[1]);

    int rc;
    int rank;
    int num_procs;

    int num_db_pts = 8;
    uint64_t db_sizes[num_db_pts];

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
    proc_rank = rank;

    //set up and return the database
    rc = metadata_database_init (job_id, rank);
    if (rc != RC_OK) {
        error_log << "Error. Could not init dbs. Error code " << rc << std::endl;
        goto cleanup;
    }


    // db_sizes[0] = sqlite3_memory_used();
    debug_log << "mem after setup and index creation: " << db_sizes[0] << endl;

  
cleanup:
    debug_log << "Shutting down" << endl;

    db_sizes[0] = sqlite3_memory_used();

    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;


    rc = sqlite3_prepare_v2 (db, "select count (*) from run_catalog", -1, &stmt, &tail); assert (rc == SQLITE_OK);
    rc = sqlite3_step (stmt); assert (rc == SQLITE_ROW || rc == SQLITE_DONE);
    db_sizes[1] = sqlite3_column_int (stmt, 0);
    rc = sqlite3_finalize (stmt); assert (rc == SQLITE_OK);

    rc = sqlite3_prepare_v2 (db, "select count (*) from timestep_catalog", -1, &stmt, &tail); assert (rc == SQLITE_OK);
    rc = sqlite3_step (stmt); assert (rc == SQLITE_ROW || rc == SQLITE_DONE);
    db_sizes[2] = sqlite3_column_int (stmt, 0);
    rc = sqlite3_finalize (stmt); assert (rc == SQLITE_OK);

    rc = sqlite3_prepare_v2 (db, "select count (*) from var_catalog", -1, &stmt, &tail); assert (rc == SQLITE_OK);
    rc = sqlite3_step (stmt); assert (rc == SQLITE_ROW || rc == SQLITE_DONE);
    db_sizes[3] = sqlite3_column_int (stmt, 0);
    rc = sqlite3_finalize (stmt); assert (rc == SQLITE_OK);

    rc = sqlite3_prepare_v2 (db, "select count (*) from type_catalog", -1, &stmt, &tail); assert (rc == SQLITE_OK);
    rc = sqlite3_step (stmt); assert (rc == SQLITE_ROW || rc == SQLITE_DONE);
    db_sizes[4] = sqlite3_column_int (stmt, 0);
    rc = sqlite3_finalize (stmt); assert (rc == SQLITE_OK);

    rc = sqlite3_prepare_v2 (db, "select count (*) from run_attribute_catalog", -1, &stmt, &tail); assert (rc == SQLITE_OK);
    rc = sqlite3_step (stmt); assert (rc == SQLITE_ROW || rc == SQLITE_DONE);
    db_sizes[5] = sqlite3_column_int (stmt, 0);
    rc = sqlite3_finalize (stmt); assert (rc == SQLITE_OK);

    rc = sqlite3_prepare_v2 (db, "select count (*) from timestep_attribute_catalog", -1, &stmt, &tail); assert (rc == SQLITE_OK);
    rc = sqlite3_step (stmt); assert (rc == SQLITE_ROW || rc == SQLITE_DONE);
    db_sizes[6] = sqlite3_column_int (stmt, 0);
    rc = sqlite3_finalize (stmt); assert (rc == SQLITE_OK);

    rc = sqlite3_prepare_v2 (db, "select count (*) from var_attribute_catalog", -1, &stmt, &tail); assert (rc == SQLITE_OK);
    rc = sqlite3_step (stmt); assert (rc == SQLITE_ROW || rc == SQLITE_DONE);
    db_sizes[7] = sqlite3_column_int (stmt, 0);
    rc = sqlite3_finalize (stmt); assert (rc == SQLITE_OK);   

    cout << "sizeof db: " << db_sizes[0] << endl;
    cout << "num runs: " << db_sizes[1] << " num timesteps: " << db_sizes[2] << " num vars: " << db_sizes[3] << 
        " num types: " << db_sizes[4] << " num run attributes: " << db_sizes[5] << " num timestep attributes: " << db_sizes[6] <<  
        " num var attributes: " << db_sizes[7] << endl;

    //add_timing_point(DB_ANALYSIS_DONE);

    // if(output_db) {
    //     rc = sql_output_db(db, job_id, rank);
    //     if (rc != SQLITE_OK)
    //     {   
    //         fprintf (stderr, "Can't output database for job_id:%llu: %s\n",job_id, sqlite3_errmsg (db));        
    //         sqlite3_close (db);
    //         goto cleanup;
    //     }
    // }

    sqlite3_close (db);

    //add_timing_point(DB_CLOSED_SHUTDOWN_DONE);

	    
    // uint64_t *all_db_sizes;

    // if(rank == 0) {
    //     all_db_sizes = (uint64_t *) malloc(num_procs * num_db_pts * sizeof(uint64_t));
    // }
    // MPI_Gather(db_sizes, num_db_pts, MPI_UNSIGNED_LONG, all_db_sizes, num_db_pts, MPI_UNSIGNED_LONG, 0, MPI_COMM_WORLD); 
   
    // if (rank == 0) {
    //     //prevent it from buffering the printf statements
    //     setbuf(stdout, NULL);

    //     cout << "DB_SIZES ";
    //     for(int i=0; i<(num_db_pts*num_procs); i++) {
    //         std::cout << all_db_sizes[i] << " ";
    //         if (i%20 == 0 && i!=0) {
    //             std::cout << std::endl;
    //         }	       
    //     }
    //     std::cout << std::endl;

    //     free(all_db_sizes);
    // }

    MPI_Barrier(MPI_COMM_WORLD);
    debug_log << "about to MPI_Finalize \n";
    MPI_Finalize();
    debug_log << "just mpi finalized \n";
    debug_log << "returning \n";
    return rc;
}



static int sql_load_db(sqlite3 *pInMemory, uint64_t job_id, int rank)
{
    int rc;
    sqlite3 *pFile;           /* Database connection  */
    sqlite3_backup *pBackup;  /* Backup object used to copy data */

    //add_timing_point(DB_LOAD_START);

    string filename = to_string(job_id) + "_" + to_string(rank);
    /* Open the database file. Exit early if this fails
      ** for any reason. */
    rc = sqlite3_open(filename.c_str(), &pFile);
    if( rc==SQLITE_OK ){  
        /* Set up the backup procedure to copy from the the main database of connection pInMemory
        to the "main" database of the connection pFile.
        ** If something goes wrong, pBackup will be set to NULL and an error
        ** code and message left in connection pFile.
        */
        // pBackup = sqlite3_backup_init(pTo, "main", pFrom, "main");
        pBackup = sqlite3_backup_init(pInMemory, "main", pFile, "main");

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
        rc = sqlite3_errcode(pInMemory);
    }

    /* Close the database connection opened on database file zFilename
    ** and return the result of this function. */
    (void)sqlite3_close(pFile);

    //add_timing_point(DB_LOAD_DONE);

    return rc;

}