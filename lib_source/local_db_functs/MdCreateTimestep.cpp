#include <md_local.hh>

extern sqlite3 *db;

using namespace std;

int md_create_timestep_stub (const md_create_timestep_args &args,
                        uint64_t &timestep_id)
{
    int rc;
    int i;
    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;
    // size_t len = 0;

    rc = sqlite3_prepare_v2 (db, "insert into timestep_catalog (id, run_id, path, active, txn_id) values (?, ?, ?, ?, ?)", -1, &stmt, &tail);
    if (rc != SQLITE_OK)
    {
        fprintf (stderr, "Error start of create_timestep_stub: Line: %d SQL error: %s\n", __LINE__, sqlite3_errmsg (db));
        sqlite3_close (db);
        goto cleanup;
    }

    rc = sqlite3_bind_int64 (stmt, 1, args.timestep_id); assert (rc == SQLITE_OK);    
    // rc = sqlite3_bind_null (stmt, 1); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 2, args.run_id); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_text (stmt, 3, strdup(args.path.c_str()), -1, free); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int (stmt, 4, 0); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 5, args.txn_id); assert (rc == SQLITE_OK);

    rc = sqlite3_step (stmt); assert (rc == SQLITE_ROW || rc == SQLITE_DONE);

    timestep_id = (int) sqlite3_last_insert_rowid (db);

    rc = sqlite3_finalize (stmt);

cleanup:

    return rc;
}