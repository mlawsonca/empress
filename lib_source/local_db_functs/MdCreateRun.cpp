#include <md_local.hh>

extern sqlite3 *db;

using namespace std;


int md_create_run_stub (const md_create_run_args &args,
                        uint64_t &run_id)
{
    int rc;
    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;

    // size_t len = 0;

    rc = sqlite3_prepare_v2 (db, "insert into run_catalog (id, job_id, name, date, active, txn_id, npx, npy, npz, rank_to_dims_funct, objector_funct) values (?, ?, ?, datetime('now','localtime'), ?, ?, ?, ?, ?, ?, ?)", -1, &stmt, &tail);
    if (rc != SQLITE_OK)
    {
        fprintf (stderr, "Error start of create_run_stub: Line: %d SQL error: %s\n", __LINE__, sqlite3_errmsg (db));
        sqlite3_close (db);
        goto cleanup;
    }

    // cout << "name: " << args.name << " path: " << args.path << " txn_id: " << args.txn_id << 
    // 	" npx: " << args.npx << " npy: " << args.npy << " npz: " << args.npz << endl;

    // cout << "rank_to_dims_funct " << args.rank_to_dims_funct.c_str() << endl;
    // cout << "objector_funct " << args.objector_funct.c_str() << endl;
   
    
    rc = sqlite3_bind_null (stmt, 1); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 2, args.job_id); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_text (stmt, 3, strdup(args.name.c_str()), -1, free); assert (rc == SQLITE_OK);
    // rc = sqlite3_bind_text (stmt, 3, strdup(args.path.c_str()), -1, free); assert (rc == SQLITE_OK);
    // rc = sqlite3_bind_text (stmt, 3, datetime('now','localtime'), -1, free); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int (stmt, 4, 0); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 5, args.txn_id); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int (stmt, 6, args.npx); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int (stmt, 7, args.npy); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int (stmt, 8, args.npz); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_text (stmt, 9, args.rank_to_dims_funct.c_str(), args.rank_to_dims_funct.size() + 1, SQLITE_TRANSIENT); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_text (stmt, 10, args.objector_funct.c_str(), args.objector_funct.size()+1, SQLITE_TRANSIENT); assert (rc == SQLITE_OK);




    // cout << "got here 2 \n";

    run_id = (int) sqlite3_last_insert_rowid (db);
    // cout << "run_id: " << run_id << " \n";

    rc = sqlite3_finalize (stmt);
    // cout << "rc: " << rc << " \n";

cleanup:

    return rc;
}