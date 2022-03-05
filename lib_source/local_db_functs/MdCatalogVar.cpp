#include <md_local.hh>

extern sqlite3 *db;

using namespace std;


int md_catalog_var_stub (md_catalog_var_args &args, 
                     std::vector<md_catalog_var_entry> &entries,
                     uint32_t &count)
{
    int rc;
    //fix - do I need to check if datasets is active?
    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;

    const char * query = "select * from var_catalog where (txn_id = ? or active = 1) and run_id = ? and timestep_id = ? "
    "order by id";

    // "order by path, name, version";

    rc = sqlite3_prepare_v2 (db, "select count (*) from var_catalog where (txn_id = ? or active = 1) and run_id = ? and timestep_id = ?  ", -1, &stmt, &tail); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 1, args.txn_id); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 2, args.run_id); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 3, args.timestep_id); assert (rc == SQLITE_OK);
    rc = sqlite3_step (stmt); assert (rc == SQLITE_ROW || rc == SQLITE_DONE);
    count = sqlite3_column_int64 (stmt, 0);
    rc = sqlite3_finalize (stmt); assert (rc == SQLITE_OK);

    // char *err_msg = 0;
    // char *sql = "SELECT * FROM var_catalog";
    // rc = sqlite3_exec(db, sql, callback, 0, &err_msg);

    // cout << "txn_id: " << args.txn_id << " run_id: " << args.run_id << " timestep_id: " << args.timestep_id << endl;
    // cout << "count: " << count << endl;

    if (count > 0) {
      	entries.reserve(count);

        rc = sqlite3_prepare_v2 (db, query, -1, &stmt, &tail); 
        if (rc != SQLITE_OK)
        {
            fprintf (stderr, "Error md_catalog_var_stub: Line: %d SQL error (%d): %s\n", __LINE__, rc, sqlite3_errmsg (db));
            sqlite3_close (db);
            goto cleanup;
        }
        rc = sqlite3_bind_int64 (stmt, 1, args.txn_id); assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int64 (stmt, 2, args.run_id); assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int64 (stmt, 3, args.timestep_id); assert (rc == SQLITE_OK);
        rc = sqlite3_step (stmt); assert (rc == SQLITE_ROW || rc == SQLITE_DONE);

        while (rc == SQLITE_ROW)
        {

            md_catalog_var_entry entry;
            
            entry.var_id = sqlite3_column_int64 (stmt, 0);
            // cout << "var_id: " << entry.var_id << endl;
            entry.run_id = sqlite3_column_int64 (stmt, 1);
            entry.timestep_id = sqlite3_column_int64 (stmt, 2);
            entry.name = (char *)sqlite3_column_text (stmt, 3);
            entry.path = (char *)sqlite3_column_text (stmt, 4);
            entry.version = sqlite3_column_int (stmt, 5);
            entry.data_size = sqlite3_column_int (stmt, 6);
            entry.active = sqlite3_column_int (stmt, 7);
            entry.txn_id = sqlite3_column_int64 (stmt, 8);
            entry.num_dims = sqlite3_column_int (stmt, 9);
            entry.dims.reserve(entry.num_dims);
            int j = 0;
            while (j < entry.num_dims)
            {
                md_dim_bounds bounds;
                bounds.min =  sqlite3_column_double (stmt, 10 + (j * 2));
                bounds.max = sqlite3_column_double (stmt, 11 + (j * 2));
                entry.dims.push_back(bounds);
                //printf ("%c: (%d/%d) ", v++, entry [i].dim [j].min, entry [i].dim [j].max);
                j++;
            }

            //printf ("id: %d name: %s path: %s version: %d num_dims: %d ", id, entry [i].name, entry [i].path, entry [i].version, entry [i].num_dims);

            //printf ("\n");

            rc = sqlite3_step (stmt);
            entries.push_back(entry);
        }

        rc = sqlite3_finalize (stmt);  


    // char *err_msg = 0;
    // char *sql = "SELECT * FROM var_catalog";
    // cout << "starting var catalog, count: " << count << " \n";
    // rc = sqlite3_exec(db, sql, callback, 0, &err_msg);
    // cout << "done with var catalog \n";
    }
    

cleanup:

    return rc;
}



