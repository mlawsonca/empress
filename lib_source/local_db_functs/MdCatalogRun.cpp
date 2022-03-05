#include <md_local.hh>

extern sqlite3 *db;

using namespace std;


// extern int callback (void * NotUsed, int argc, char ** argv, char ** ColName);

int md_catalog_run_stub (md_catalog_run_args &args, 
                     std::vector<md_catalog_run_entry> &entries,
                     uint32_t &count)
{
    int rc;
    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;
    // const char * query = "select * from run_catalog where txn_id = ? or active = 1 order by name, id";
    const char * query = "select * from run_catalog where txn_id = ? or active = 1 order by id";
    size_t size = 0;

    rc = sqlite3_prepare_v2 (db, "select count (*) from run_catalog where txn_id = ? or active = 1", -1, &stmt, &tail); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 1, args.txn_id); assert (rc == SQLITE_OK);
    rc = sqlite3_step (stmt); assert (rc == SQLITE_ROW || rc == SQLITE_DONE);
    count = sqlite3_column_int64 (stmt, 0);
    rc = sqlite3_finalize (stmt); assert (rc == SQLITE_OK);

//printf ("rows in run_catalog: %d\n", count);
    if (count > 0) {
      entries.reserve(count);


    // char *err_msg = 0;
    // char *sql = "SELECT * FROM run_catalog";
    // cout << "starting run catalog \n";
    // rc = sqlite3_exec(db, sql, callback, 0, &err_msg);
    // cout << "done with run catalog \n";

        rc = sqlite3_prepare_v2 (db, query, -1, &stmt, &tail); assert (rc == SQLITE_OK);
        if (rc != SQLITE_OK)
        {
            fprintf (stderr, "Error md_catalog_run_stub: Line: %d SQL error (%d): %s\n", __LINE__, rc, sqlite3_errmsg (db));
            sqlite3_close (db);
            goto cleanup;
        }
        rc = sqlite3_bind_int64 (stmt, 1, args.txn_id); assert (rc == SQLITE_OK);
        rc = sqlite3_step (stmt); assert (rc == SQLITE_ROW || rc == SQLITE_DONE);

        while (rc == SQLITE_ROW)
        {
            int j = 0;

            md_catalog_run_entry entry;


            entry.run_id = sqlite3_column_int64 (stmt, 0);
            entry.job_id = sqlite3_column_int64 (stmt, 1);
            entry.name = (char *)sqlite3_column_text (stmt, 2);
            // entry.path = (char *)sqlite3_column_text (stmt, 2);
            entry.date = (char *)sqlite3_column_text (stmt, 3);
            entry.active = sqlite3_column_int (stmt, 4);
            entry.txn_id = sqlite3_column_int64 (stmt, 5);
            entry.npx = sqlite3_column_int (stmt, 6);
            entry.npy = sqlite3_column_int (stmt, 7);
            entry.npz = sqlite3_column_int (stmt, 8);


            int size_rank_funct = sqlite3_column_bytes (stmt, 9);
            entry.rank_to_dims_funct.assign((char *)sqlite3_column_text (stmt, 9),(char *)sqlite3_column_text (stmt, 9) + size_rank_funct);
            // cout << "entry.rank_to_dims_funct.size(): " << entry.rank_to_dims_funct.size() << endl;
            int size_obj_funct = sqlite3_column_bytes (stmt, 10);
            entry.objector_funct.assign((char *)sqlite3_column_text (stmt, 10),(char *)sqlite3_column_text (stmt, 10) + size_obj_funct);
           	// cout << "objector_funct.size(): " << entry.objector_funct.size() << endl;

           	// entry.rank_to_dims_funct = (string)sqlite3_column_blob (stmt, 8);

     		// entry.rank_to_dims_funct = (char *)sqlite3_column_blob (stmt, 8);
       //      cout << "entry.rank_to_dims_funct.size(): " << entry.rank_to_dims_funct.size() << endl;
       //      entry.objector_funct = (char *)sqlite3_column_blob (stmt, 9);
       // //     	cout << "objector_funct.size(): " << entry.objector_funct.size() << endl;

       //      string &s = *(static_cast<std::string*>(sqlite3_column_blob (stmt, 8)));
       //                  string &s2 = *(static_cast<std::string*>(sqlite3_column_blob (stmt, 9)));

          //   int size_rank_funct = sqlite3_column_bytes (stmt, 8);
          //   // memcpy(entry.rank_to_dims_funct, sqlite3_column_blob(stmt, 8), size_rank_funct);
          //   entry.rank_to_dims_funct.assign(&((char *)sqlite3_column_blob(stmt, 8))[0], &(char *)(sqlite3_column_blob(stmt, 8))[0] + size_rank_funct);

          //   int size_obj_funct = sqlite3_column_bytes (stmt, 9);
          //   entry.objector_funct.assign(&((char *)sqlite3_column_blob(stmt, 9))[0], &(char *)(sqlite3_column_blob(stmt, 9))[0] + size_obj_funct);

          //   // memcpy(entry.objector_funct, sqlite3_column_blob(stmt, 9), size_obj_funct);	
         	// // entry.rank_to_dims_funct = (char *)sqlite3_column_text (stmt, 8);
          //   cout << "entry.rank_to_dims_funct.size(): " << entry.rank_to_dims_funct.size() << endl;
          // //   entry.objector_funct = (char *)sqlite3_column_text (stmt, 9);
          //  	cout << "objector_funct.size(): " << entry.objector_funct << endl;

     		// entry.rank_to_dims_funct = *(static_cast<std::string*>(sqlite3_column_blob (stmt, 8)));
       //      cout << "entry.rank_to_dims_funct.size(): " << entry.rank_to_dims_funct.size() << endl;
       //      entry.objector_funct = *(static_cast<std::string*>(sqlite3_column_blob (stmt, 9)));
       //     	cout << "objector_funct.size(): " << entry.objector_funct.size() << endl;

            // entry.rank_to_dims_funct = (char *)sqlite3_column_text (stmt, 8);
            // cout << "entry.rank_to_dims_funct.size(): " << entry.rank_to_dims_funct.size() << endl;
            // entry.objector_funct = (char *)sqlite3_column_text (stmt, 9);
           	// cout << "objector_funct.size(): " << entry.objector_funct << endl;

            // entry.rank_to_dims_funct = (string)sqlite3_column_text (stmt, 8);
            // cout << "entry.rank_to_dims_funct.size(): " << entry.rank_to_dims_funct.size() << endl;
            // entry.objector_funct = (string)sqlite3_column_text (stmt, 9);
           	// cout << "objector_funct.size(): " << entry.objector_funct.size() << endl;
    //printf ("id: %d name: %s path: %s version: %d num_dims: %d ", id, entry [i].name, entry [i].path, entry [i].version, entry [i].num_dims);
    //printf ("\n");
            rc = sqlite3_step (stmt);
            entries.push_back(entry);
        }

        rc = sqlite3_finalize (stmt);  
    }
    

cleanup:

    return rc;
}
