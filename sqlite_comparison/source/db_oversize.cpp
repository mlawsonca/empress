
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
// #include <vector>
#include <float.h> //needed for DBL_MAX

#include <sqlite_objector_comparison.hh>

// #include <md_client_timing_constants.hh>
// #include <client_timing_constants_write.hh>

#include <sqlite3.h>
#include <sys/resource.h>


sqlite3 * db = NULL;

using namespace std;

static bool extreme_debug_logging = false;
static bool debug_logging = false;
static bool error_logging = true;
static bool testing_logging = false;

static debugLog error_log = debugLog(error_logging);
static debugLog debug_log = debugLog(debug_logging);
static debugLog extreme_debug_log = debugLog(extreme_debug_logging);
static debugLog testing_log = debugLog(testing_logging);

static bool output_timing = true;
static bool output_queries = false;

std::vector<int> catg_of_time_pts;
vector<long double> time_pts;

// static int sql_output_db(sqlite3 *pInMemory, uint64_t job_id, int rank);
// int md_insert_object_batch_stub (const vector<md_insert_object_args> &all_args);
static int sql_output_db(sqlite3 *pInMemory);
// int md_insert_blob (const unsigned char *blob, int num_bytes, int key);
int md_insert_blob (const char *blob, int num_bytes, int key);
static int callback (void * NotUsed, int argc, char ** argv, char ** ColName);
sqlite3_int64 dbsize ();
static int metadata_database_init (bool on_disk);
static int create_reading_indices();

int checkpt_count = 0;

int main(int argc, char **argv) {
    int rc;
    char  *ErrMsg = NULL;

    bool on_disk = false;
    bool write = false;

    if(write) {
	    //set up and return the database
	    rc = metadata_database_init (on_disk);
	    if (rc != RC_OK) {
	        error_log << "Error. Could not init dbs. Error code " << rc << std::endl;
	        //add_timing_point(ERR_DB_SETUP);
	        return RC_ERR;
	    }
	    //add_timing_point(DB_INIT_DONE);


	    // rc = sqlite3_exec (db, "PRAGMA max_page_count = 2900000", callback, 0, &ErrMsg);
	    // if (rc != SQLITE_OK)    {
	    //     fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
	    //     sqlite3_free (ErrMsg);
	    //     sqlite3_close (db);
	    //     return RC_ERR;
	    // }

	    // uint32_t num_bytes = 26500000;
	    // unsigned char *blob = (unsigned char *)malloc(num_bytes);

		//open file
		std::ifstream infile("../source/address.txt");

		//get length of file
		infile.seekg(0, infile.end);
		size_t length = infile.tellg();
		// unsigned char *blob = (unsigned char *)malloc(length);
		char *blob = (char *)malloc(length);
		cout << "length: " << length << endl;

		infile.seekg(0, infile.beg);

		//read file
		infile.read(blob, length);

	    uint64_t max_db_size = 2000000000;
	    uint64_t max_mem_usage = 3400000000;

	    // for(uint32_t timestep=0; timestep < num_timesteps; timestep += 10) {
		uint32_t timestep = 0;
		int key = 0;
		while(true) {
		// while(timestep < 850) {
			if(on_disk && timestep >= 850) {
				break;
			}
			int64_t mem_used = sqlite3_memory_used();
	    	cout << "db_size for timestep " << timestep << ": " << mem_used << endl;
	    	int64_t real_db_size = dbsize();
	    	cout << "dbsize (): " << real_db_size << endl;
			// int64_t mem_highwater = sqlite3_memory_highwater(false);
	    	// cout << "db_highwater for timestep " << timestep << ": " << mem_highwater << endl;
			// if(real_db_size > max_db_size) {
			// 	sql_output_db(db);
			// 	checkpt_count += 1;
			// 	sqlite3_close(db);
			// 	rc = metadata_database_init ();
			// 	if(rc != RC_OK) {
			// 		cout << "error reiniting the db after outputting due to size overload" << endl; 
			// 	}
			// }
	        //add_timing_point(WRITE_TIMESTEP_START);

			struct rusage r_usage;
			getrusage(RUSAGE_SELF,&r_usage);
			// Print the maximum resident set size used (in kilobytes).
			printf("Memory usage: %ld bytes\n",r_usage.ru_maxrss*1000);
			if(r_usage.ru_maxrss*1000 > max_mem_usage) {
				break;
			}

	        debug_log << "beginning writing for timestep " << timestep << endl;
	     	
			// rc = md_insert_blob (blob, num_bytes, key);
			rc = md_insert_blob (blob, length, key);
			if(rc != RC_OK) {
				cout << "error inserting blob" << endl;
				break;
			}
			timestep += 10;
			key += 1;
	        //add_timing_point(WRITE_TIMESTEP_DONE);
	    } // end for(uint32_t timestep=0; timestep < num_timesteps; timestep++)
	}

//-------------------------------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------------------------------        
cleanup:
        
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    // uint64_t db_size1 = sqlite3_memory_used();

  	sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;

    if(debug_logging) {
	    rc = sqlite3_prepare_v2 (db, "select * from blob_catalog", -1, &stmt, &tail); assert (rc == SQLITE_OK);
	    rc = sqlite3_step (stmt); assert (rc == SQLITE_ROW || rc == SQLITE_DONE);
	    while(rc == SQLITE_ROW) {
	    	// cout << "key: " << sqlite3_column_int (stmt, 0) << endl;
	    	rc = sqlite3_step (stmt); assert (rc == SQLITE_ROW || rc == SQLITE_DONE);
	    }
	    rc = sqlite3_finalize (stmt); assert (rc == SQLITE_OK);
	}

    // cout << "db_size for timestep " << timestep << ": " << sqlite3_memory_used() << endl;
 
    debug_log << "got to cleanup7" << endl;

    if(!on_disk && write) {
    	cout << "am outputting the database" << endl;
		rc = sql_output_db(db);
		if(rc != RC_OK) {
			cout << "error outputting the db" << endl;
		}
	}
    sqlite3_close (db);

    if(!on_disk) {
    	cout << "am indexing the database" << endl;
	    string filename = "blob.sqlite_" + to_string(checkpt_count);
		rc = sqlite3_open(filename.c_str(), &db);
		if (rc != SQLITE_OK)
		{   
			fprintf (stderr, "Can't open database: %s\n", sqlite3_errmsg (db));		
			sqlite3_close (db);
			goto cleanup;
		}

		rc = create_reading_indices();
		if(rc == RC_OK) {
			cout << "correctly made the indices" << endl;
		}
	}

    return rc;

}

sqlite3_int64 dbsize ()
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


// int main(int argc, char **argv) {
//     int rc;


//     // if (argc != 12) { 
//     //     error_log << "Error. Program should be given 11 arguments. npx_write, npy_write, npz_write, npx_read, npy_read, npz_read, nx, ny, nz, number of timesteps, job_id" << endl;
//     //     // cout << (int)ERR_ARGC << " " << 0 <<  endl;
//     //     return RC_ERR;
//     // }
    

//     uint32_t num_x_procs = 10;
//     uint32_t num_y_procs = 10;
//     uint32_t num_z_procs = 10;   
//     // uint32_t num_read_x_procs = stoul(argv[4],nullptr,0);
//     // uint32_t num_read_y_procs = stoul(argv[5],nullptr,0);
//     // uint32_t num_read_z_procs = stoul(argv[6],nullptr,0);   
//     uint64_t total_x_length = 1250;
//     uint64_t total_y_length = 1600;
//     uint64_t total_z_length = 2500;
//     uint32_t num_timesteps = 2000;

//     uint32_t num_read_x_procs = 1;
//     uint32_t num_read_y_procs = 1;
//     uint32_t num_read_z_procs = 1;

//     uint32_t num_attrs_per_proc = 3;

//     uint64_t job_id = 12345678;

//     extreme_debug_log << "job_id: " << job_id << endl;

//     uint32_t num_client_procs = num_x_procs * num_y_procs * num_z_procs;
//     // uint32_t num_client_procs = 1;
//     // uint32_t num_client_procs = 10000;

//     int rank = 0;

//     //all 10 char plus null character
//     uint32_t num_vars = 10;
//     char var_names[10][11] = {"temperat", "temperat", "pressure", "pressure", "velocity_x", "velocity_y", "velocity_z", "magn_field", "energy_var", "density_vr"};  

//     uint32_t estm_num_time_pts = 200000;
//     catg_of_time_pts.reserve(estm_num_time_pts);
//     time_pts.reserve(estm_num_time_pts);


//     //add_timing_point(PROGRAM_START);


//     vector<md_dim_bounds> var_dims(3);
//     vector<md_dim_bounds> proc_dims(3);

//     uint32_t version1 = 1;
//     uint32_t version2 = 2;

//     //given in %
//     float type_freqs[] = {25, 5, .1, 100, 100, 20, 2.5, .5, 1, 10};
//     // char type_types[10][3] = {"b", "b", "b", "d", "d", "2p","2p","2p","2d","2d"};
//     char type_types[10][3] = {"b", "b", "b", "d", "d", "s","s","s","2d","2d"};
//     //all 12 char plus null character
//     char type_names[10][13] = {"blob_freq", "blob_ifreq", "blob_rare", "max_val_type", "min_val_type", "note_freq", "note_ifreq", "note_rare", "ranges_type1", "ranges_type2"}; 


//     char run_timestep_type_names[2][9] = {"max_temp", "min_temp"}; 
//     int num_run_timestep_types = 2;
//     uint32_t num_types = 10;

//     extreme_debug_log << "num_type: " << num_types << " num_run_timestep_types: " << num_run_timestep_types << endl;

//     uint64_t x_length_per_proc = total_x_length / num_x_procs; //todo - deal with edge case?
//     uint64_t y_length_per_proc = total_y_length / num_y_procs;
//     uint64_t z_length_per_proc = total_z_length / num_z_procs;

//     uint64_t chunk_size = x_length_per_proc * y_length_per_proc * z_length_per_proc * 8; //8 bytes values

//     uint64_t my_var_id;
//     uint32_t num_dims;
//     uint32_t var_num;
//     uint32_t var_version;

//     struct md_catalog_run_entry temp_run;


//     uint32_t x_pos;
//     uint32_t y_pos;
//     uint32_t z_pos;
//     uint32_t x_offset;
//     uint32_t y_offset;
//     uint32_t z_offset;

//     //add_timing_point(DB_INIT_START);

//     //set up and return the database
//     rc = metadata_database_init ();
//     if (rc != RC_OK) {
//         error_log << "Error. Could not init dbs. Error code " << rc << std::endl;
//         //add_timing_point(ERR_DB_SETUP);
//         return RC_ERR;
//     }
//     //add_timing_point(DB_INIT_DONE);

//     uint64_t db_size0 = sqlite3_memory_used();

//     //reminder - needs to be done for each CLIENT, not server
//     uint64_t run_id = 1;

//     vector<md_insert_object_args> all_args;
//     for(uint32_t timestep=0; timestep < 10; timestep++) {
// 		for (int rank = 0; rank < num_client_procs; rank ++) {
// 		    x_pos = rank % num_x_procs;
// 		    y_pos = (rank / num_x_procs) % num_y_procs; 
// 		    z_pos = ((rank / (num_x_procs * num_y_procs)) % num_z_procs);

// 		    x_offset = x_pos * x_length_per_proc;
// 		    y_offset = y_pos * y_length_per_proc;
// 		    z_offset = z_pos * z_length_per_proc;

// 		    num_dims = 3;
// 		    proc_dims [0].min = x_offset;
// 		    proc_dims [0].max = x_offset + x_length_per_proc-1;
// 		    proc_dims [1].min = y_offset;
// 		    proc_dims [1].max = y_offset + y_length_per_proc -1;
// 		    proc_dims [2].min = z_offset;
// 		    proc_dims [2].max = z_offset + z_length_per_proc -1;

// 	    	for(uint32_t var_indx=0; var_indx < num_vars; var_indx++) {
// 	        //add_timing_point(WRITE_VAR_START);

// 	        //add_timing_point(INSERT_OBJECTS_START);
// 	           	md_insert_object_args obj_args;
// 	            // obj_args.run_id = temp_run.run_id;
// 	            obj_args.timestep_id = timestep;
// 	            obj_args.var_id = var_indx;
// 	            // obj_args.txn_id = temp_var.txn_id;
// 	            // obj_args.num_dims = 3;
// 	        	obj_args.dims = proc_dims;
// 	        	all_args.push_back(obj_args);
// 	        }
// 	    } 
// 	}

//     // for(uint32_t timestep=0; timestep < num_timesteps; timestep += 10) {
// 	uint32_t timestep = 0;
// 	while(true) {
//     	cout << "db_size for timestep " << timestep << ": " << sqlite3_memory_used() << endl;
//         //add_timing_point(WRITE_TIMESTEP_START);
//         debug_log << "beginning writing for timestep " << timestep << endl;
     	
// 		rc = md_insert_object_batch_stub (all_args);
// 		if(rc != RC_OK) {
// 			break;
// 		}
// 		timestep += 10;
//         //add_timing_point(WRITE_TIMESTEP_DONE);
//     } // end for(uint32_t timestep=0; timestep < num_timesteps; timestep++)

// //-------------------------------------------------------------------------------------------------------------------------------------------
// //-------------------------------------------------------------------------------------------------------------------------------------------        
// cleanup:
        
// /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
//     uint64_t db_size1 = sqlite3_memory_used();


//   	sqlite3_stmt * stmt = NULL;
//     const char * tail = NULL;

//     rc = sqlite3_prepare_v2 (db, "select count (*) from oid_catalog", -1, &stmt, &tail); assert (rc == SQLITE_OK);
//     rc = sqlite3_step (stmt); assert (rc == SQLITE_ROW || rc == SQLITE_DONE);
//     uint64_t db_size2 = sqlite3_column_int (stmt, 0);
//     rc = sqlite3_finalize (stmt); assert (rc == SQLITE_OK);

//     cout << "num_objects: " << db_size2 << endl;

//     debug_log << "got to cleanup7" << endl;

// 	// rc = sql_output_db(db, job_id, rank);

//     // sqlite3_close (db);

//     return rc;

// }






static int callback (void * NotUsed, int argc, char ** argv, char ** ColName)
{
    int i;
    for (i = 0; i < argc; i++)
    {
        printf ("%s = %s\n", ColName [i], argv [i] ? argv [i] : "NULL");
    }
    printf ("\n");
    return 0;
}


//===========================================================================
static int metadata_database_init (bool on_disk)
{
    //setup the database
    int rc;
    char  *ErrMsg = NULL;
    
    // rc = sqlite3_open (":memory:", &db);

    string filename;
    if(on_disk) {
	    filename = "blob_on_disk";
	}
	else {
		filename = ":memory:";
	}
	rc = sqlite3_open(filename.c_str(), &db);
    if (rc != SQLITE_OK)
    {   
        fprintf (stderr, "Can't open database: %s\n", sqlite3_errmsg (db));        
        sqlite3_close (db);
        goto cleanup;
    }

    // rc = sqlite3_exec (db, "create table oid_catalog (id integer primary key autoincrement not null, timestep_id not null, var_id int not null, d0_min int, d0_max int, d1_min int, d1_max int, d2_min int, d2_max int, name varchar (50) )", callback, 0, &ErrMsg);
    // if (rc != SQLITE_OK)    {
    //     fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
    //     sqlite3_free (ErrMsg);
    //     sqlite3_close (db);
    //     goto cleanup;
    // }
    // rc = sqlite3_exec (db, "create table blob_catalog (key int, bytes blob )", callback, 0, &ErrMsg);
    // if (rc != SQLITE_OK)    {
    //     fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
    //     sqlite3_free (ErrMsg);
    //     sqlite3_close (db);
    //     goto cleanup;
    // }
    rc = sqlite3_exec (db, "create table blob_catalog (key int, bytes text )", callback, 0, &ErrMsg);
    if (rc != SQLITE_OK)    {
        fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
        sqlite3_free (ErrMsg);
        sqlite3_close (db);
        goto cleanup;
    }

cleanup:
    return rc;
}

//to use to insert objects into the table
// int md_insert_blob (const unsigned char *blob, int num_bytes, int key)
int md_insert_blob (const char *blob, int num_bytes, int key)
{
    int rc;
    int i = 0;
    sqlite3_stmt * stmt_index = NULL;
    const char * tail_index = NULL;

    rc = sqlite3_prepare_v2 (db, "insert into blob_catalog (key, bytes) values (?, ?)", -1, &stmt_index, &tail_index);
    if (rc != SQLITE_OK)
    {
        fprintf (stderr, "Error at start of md_insert_blob: Line: %d SQL error: %s\n", __LINE__, sqlite3_errmsg (db));
        sqlite3_close (db);
        goto cleanup;
    }

    rc = sqlite3_bind_int (stmt_index, 1, key); assert (rc == SQLITE_OK);
    // rc = sqlite3_bind_blob(stmt_index, 2, blob, num_bytes, SQLITE_STATIC); assert (rc == SQLITE_OK);
   	rc = sqlite3_bind_text (stmt_index, 2, blob, num_bytes, SQLITE_TRANSIENT);
    rc = sqlite3_step (stmt_index); assert (rc == SQLITE_ROW || rc == SQLITE_DONE);

    rc = sqlite3_finalize (stmt_index);


cleanup:

    return rc;
}

static int create_reading_indices()
{
	int rc;
	char  *ErrMsg = NULL;
	sqlite3_extended_result_codes(db, 1);

	rc = sqlite3_exec (db, "create index index_blobs on blob_catalog(key, bytes)", 0, 0, &ErrMsg);
	if (rc != SQLITE_OK)	{
		fprintf (stdout, "Line: %d SQL error: %s. rc: %d\n", __LINE__, ErrMsg, rc);
		sqlite3_free (ErrMsg);
		sqlite3_close (db);
		goto cleanup;
	}

	

cleanup:
	return rc;
}

// //to use to insert objects into the table
// int md_insert_object_stub (const md_insert_object_args &args,
//                           uint64_t &object_id)
// {
//     int rc;
//     int i = 0;
//     sqlite3_stmt * stmt_index = NULL;
//     const char * tail_index = NULL;

//     rc = sqlite3_prepare_v2 (db, "insert into oid_catalog (id, timestep_id, var_id, d0_min, d0_max, d1_min, d1_max, d2_min, d2_max) values (?, ?, ?, ?, ?, ?, ?, ?, ?)", -1, &stmt_index, &tail_index);
//     if (rc != SQLITE_OK)
//     {
//         fprintf (stderr, "Error at start of insert_var_object_stub: Line: %d SQL error: %s\n", __LINE__, sqlite3_errmsg (db));
//         sqlite3_close (db);
//         goto cleanup;
//     }

//     rc = sqlite3_bind_null (stmt_index, 1); assert (rc == SQLITE_OK);
//     rc = sqlite3_bind_int64 (stmt_index, 2, args.timestep_id); assert (rc == SQLITE_OK);   
//     rc = sqlite3_bind_int64 (stmt_index, 3, args.var_id); assert (rc == SQLITE_OK);
//     while(i < 3) {
//         rc = sqlite3_bind_int64 (stmt_index, 4 + i * 2, args.dims.at(i).min); assert (rc == SQLITE_OK);
//         rc = sqlite3_bind_int64 (stmt_index, 5 + i * 2, args.dims.at(i).max); assert (rc == SQLITE_OK);
//         i++;
//     }

//     assert (rc == SQLITE_OK);
//     rc = sqlite3_step (stmt_index); assert (rc == SQLITE_ROW || rc == SQLITE_DONE);


//     object_id = (int) sqlite3_last_insert_rowid (db);

//     rc = sqlite3_finalize (stmt_index);


// cleanup:

//     return rc;
// }

// //to use to insert objects into the table
// int md_insert_object_batch_stub (const vector<md_insert_object_args> &all_args)
// {
//     int rc;
//     int i = 0;
//     sqlite3_stmt * stmt_index = NULL;
//     const char * tail_index = NULL;
//     char * ErrMsg = NULL;

//     rc = sqlite3_exec (db, "begin;", callback, 0, &ErrMsg);
//     if (rc != SQLITE_OK)
//     {
//         fprintf (stderr, "Error begin md_insert_object_batch_stub: Line: %d SQL error: %s\n", __LINE__, ErrMsg);
//         sqlite3_free (ErrMsg);
//         sqlite3_close (db);
//         goto cleanup;
//     }

//     rc = sqlite3_prepare_v2 (db, "insert into oid_catalog (id, timestep_id, var_id, d0_min, d0_max, d1_min, d1_max, d2_min, d2_max) values (?, ?, ?, ?, ?, ?, ?, ?, ?)", -1, &stmt_index, &tail_index);
//     if (rc != SQLITE_OK)
//     {
//         fprintf (stderr, "Error at start of insert_var_object_stub: Line: %d SQL error: %s\n", __LINE__, sqlite3_errmsg (db));
//         sqlite3_close (db);
//         goto cleanup;
//     }

//     for(int arg_indx = 0; arg_indx < all_args.size(); arg_indx++) {
//     	md_insert_object_args args = all_args.at(arg_indx);

// 	    rc = sqlite3_bind_null (stmt_index, 1); assert (rc == SQLITE_OK);
// 	    rc = sqlite3_bind_int64 (stmt_index, 2, args.timestep_id); assert (rc == SQLITE_OK);   
// 	    rc = sqlite3_bind_int64 (stmt_index, 3, args.var_id); assert (rc == SQLITE_OK);
// 	    while(i < 3) {
// 	        rc = sqlite3_bind_int64 (stmt_index, 4 + i * 2, args.dims.at(i).min); assert (rc == SQLITE_OK);
// 	        rc = sqlite3_bind_int64 (stmt_index, 5 + i * 2, args.dims.at(i).max); assert (rc == SQLITE_OK);
// 	        i++;
// 	    }
//     	assert (rc == SQLITE_OK);
//     	rc = sqlite3_step (stmt_index); assert (rc == SQLITE_ROW || rc == SQLITE_DONE);
// 	    rc = sqlite3_reset(stmt_index); assert (rc == SQLITE_OK);
// 	}

//     rc = sqlite3_finalize (stmt_index);


//     rc = sqlite3_exec (db, "end;", callback, 0, &ErrMsg);
//     if (rc != SQLITE_OK)
//     {
//         fprintf (stderr, "Error end query md_insert_var_attribute_by_dims_batch_stub: Line: %d SQL error: %s\n", __LINE__, ErrMsg);
//         sqlite3_free (ErrMsg);
//         sqlite3_close (db);
//         goto cleanup;
//     }


// cleanup:

//     return rc;
// }

static int sql_output_db(sqlite3 *pInMemory)
{
    int rc;
    sqlite3 *pFile;           /* Database connection  */
    sqlite3_backup *pBackup;  /* Backup object used to copy data */

    /* Open the database file. Exit early if this fails
      ** for any reason. */
    string filename = "blob.sqlite_" + to_string(checkpt_count);
    rc = sqlite3_open(filename.c_str(), &pFile);
    if( rc==SQLITE_OK ){  
        /* Set up the backup procedure to copy from the the main database of connection pInMemory
        to the "main" database of the connection pFile.
        ** If something goes wrong, pBackup will be set to NULL and an error
        ** code and message left in connection pFile.
        */
        // pBackup = sqlite3_backup_init(pTo, "main", pFrom, "main");
        pBackup = sqlite3_backup_init(pFile, "main", pInMemory, "main");


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
        cout << "errcode: " << rc << endl;
    }

    /* Close the database connection opened on database file zFilename
    ** and return the result of this function. */
    (void)sqlite3_close(pFile);
    return rc;

}

// static int sql_output_db(sqlite3 *pInMemory, uint64_t job_id, int rank)
// {
//     int rc;
//     sqlite3 *pFile;           /* Database connection  */
//     sqlite3_backup *pBackup;  /* Backup object used to copy data */

//     string filename = to_string(job_id) + "_" + to_string(rank);
//     /* Open the database file. Exit early if this fails
//       ** for any reason. */
//     rc = sqlite3_open(filename.c_str(), &pFile);
//     if( rc==SQLITE_OK ){  
//         /* Set up the backup procedure to copy from the the main database of connection pInMemory
//         to the "main" database of the connection pFile.
//         ** If something goes wrong, pBackup will be set to NULL and an error
//         ** code and message left in connection pFile.
//         */
//         // pBackup = sqlite3_backup_init(pTo, "main", pFrom, "main");
//         pBackup = sqlite3_backup_init(pFile, "main", pInMemory, "main");


//         /* If the backup object is successfully created, call backup_step()
//         ** to copy data from pInMemory to pFile . Then call backup_finish()
//         ** to release resources associated with the pBackup object.  If an
//         ** error occurred, then an error code and message will be left in
//         ** connection pTo. If no error occurred, then the error code belonging
//         ** to pFile is set to SQLITE_OK.
//         */
//         if( pBackup ){
//           (void)sqlite3_backup_step(pBackup, -1);
//           (void)sqlite3_backup_finish(pBackup);
//         }
//         // rc = sqlite3_errcode(pTo);
//         rc = sqlite3_errcode(pFile);
//     }

//     /* Close the database connection opened on database file zFilename
//     ** and return the result of this function. */
//     (void)sqlite3_close(pFile);
//     return rc;

// }
