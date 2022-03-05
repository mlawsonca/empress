#include <md_local.hh>

extern sqlite3 *db;

using namespace std;


int md_catalog_type_stub (md_catalog_type_args &args, 
                          std::vector<md_catalog_type_entry> &entries,
                          uint32_t &count)
{
    int rc;
    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;
    const char * query = "select * from type_catalog where "
  "(txn_id = ? or active = 1) "
  "and run_id = ? "
  // "order by name, version";
  "order by id";

    size_t size = 0;

    rc = sqlite3_prepare_v2 (db, "select count (*) from type_catalog where "
      "(txn_id = ? or active = 1) and run_id = ? ", -1, &stmt, &tail); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 1, args.txn_id); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 2, args.run_id); assert (rc == SQLITE_OK);

    rc = sqlite3_step (stmt); assert (rc == SQLITE_ROW || rc == SQLITE_DONE);
    count = sqlite3_column_int64 (stmt, 0);
    rc = sqlite3_finalize (stmt); assert (rc == SQLITE_OK);

    // char *err_msg = 0;
    // char *sql = "SELECT * FROM type_catalog";
    // cout << "starting type catalog \n";
    // rc = sqlite3_exec(db, sql, callback, 0, &err_msg);
    // cout << "done with type catalog \n";

    if (count > 0) {
        entries.reserve(count);

        rc = sqlite3_prepare_v2 (db, query, -1, &stmt, &tail); assert (rc == SQLITE_OK);
        if (rc != SQLITE_OK)
        {
            fprintf (stderr, "Error md_catalog_type_stub: Line: %d SQL error (%d): %s\n", __LINE__, rc, sqlite3_errmsg (db));
            sqlite3_close (db);
            goto cleanup;
        }
        rc = sqlite3_bind_int64 (stmt, 1, args.txn_id); assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int64 (stmt, 2, args.run_id); assert (rc == SQLITE_OK);

        rc = sqlite3_step (stmt); assert (rc == SQLITE_ROW || rc == SQLITE_DONE);

        // std::cout << "Server thinks there are " << count << " types \n";

        while (rc == SQLITE_ROW)
        {
            md_catalog_type_entry entry;


            entry.type_id = sqlite3_column_int64 (stmt, 0);
            entry.run_id = sqlite3_column_int64 (stmt, 1);
            entry.name = (char *)sqlite3_column_text (stmt, 2);
            entry.version = sqlite3_column_int (stmt, 3);
            entry.active = sqlite3_column_int (stmt, 4);
            entry.txn_id = sqlite3_column_int64 (stmt, 5);

    //printf ("id: %d name: %s path: %s version: %d num_dims: %d ", id, entry [i].name, entry [i].path, entry [i].version, entry [i].num_dims);
            // std::cout << "in md_catalog_type_stub the given entry has name " << entry.name << std::endl;
    //printf ("\n");

            rc = sqlite3_step (stmt);
            entries.push_back(entry);

        }

        rc = sqlite3_finalize (stmt);   
    }
//printf ("rows in var_catalog: %d\n", count);
       

cleanup:

    return rc;
}
