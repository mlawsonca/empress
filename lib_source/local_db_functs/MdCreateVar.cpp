#include <md_local.hh>

extern sqlite3 *db;

using namespace std;



int md_create_var_stub (const md_create_var_args &args,
                        uint64_t &row_id)
{
    int rc;
    int i;
    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;
    // size_t len = 0;

    // char *err_msg = 0;
    // char *sql = "SELECT * FROM var_catalog";
    // cout << "starting var catalog \n";
    // rc = sqlite3_exec(db, sql, callback, 0, &err_msg);
    // cout << "done with var catalog \n";


    rc = sqlite3_prepare_v2 (db, "insert into var_catalog (id, run_id, timestep_id, name, path, version, data_size, active, txn_id, num_dims, d0_min, d0_max, d1_min, d1_max, d2_min, d2_max) values (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)", -1, &stmt, &tail);
    if (rc != SQLITE_OK)
    {
        fprintf (stderr, "Error start of create_var_stub: Line: %d SQL error: %s\n", __LINE__, sqlite3_errmsg (db));
        sqlite3_close (db);
        goto cleanup;
    }

    rc = sqlite3_bind_int64 (stmt, 1, args.var_id); assert (rc == SQLITE_OK);  
    // cout << "var id on create: " << args.var_id << endl; 
    rc = sqlite3_bind_int64 (stmt, 2, args.run_id); assert (rc == SQLITE_OK); 
    rc = sqlite3_bind_int64 (stmt, 3, args.timestep_id); assert (rc == SQLITE_OK);   
    rc = sqlite3_bind_text (stmt, 4, strdup(args.name.c_str()), -1, free); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_text (stmt, 5, strdup(args.path.c_str()), -1, free); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int (stmt, 6, args.version); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int (stmt, 7, (int)args.data_size); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int (stmt, 8, 0); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 9, args.txn_id); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int (stmt, 10, args.num_dims); assert (rc == SQLITE_OK);
    for (i = 0; i < args.num_dims; i++)
    {
        rc = sqlite3_bind_double (stmt, 11 + i * 2, args.dims.at(i).min); assert (rc == SQLITE_OK);
        rc = sqlite3_bind_double (stmt, 12 + i * 2, args.dims.at(i).max); assert (rc == SQLITE_OK);
    }

    rc = sqlite3_step (stmt); assert (rc == SQLITE_ROW || rc == SQLITE_DONE);

    row_id = (int) sqlite3_last_insert_rowid (db);
//printf ("generated new global id:%d\n", *row_id);

    rc = sqlite3_finalize (stmt);

cleanup:

    return rc;
}