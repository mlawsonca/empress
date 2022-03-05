#include <md_local.hh>

extern sqlite3 *db;

using namespace std;

extern int callback (void * NotUsed, int argc, char ** argv, char ** ColName);



// int md_create_type_batch_stub (const vector<md_create_type_args> &args,
//                         uint64_t &type_id)
// int md_create_type_batch_stub (const vector<md_create_type_args> &all_args)
// int md_create_type_batch_stub (const vector<md_create_type_args> &all_args,
//                         vector<uint64_t> &type_ids)
int md_create_type_batch_stub (const vector<md_create_type_args> &all_args)
{
    int rc;
    // int i;
    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;
    char * ErrMsg = NULL;

    // int rowid;

    rc = sqlite3_exec (db, "begin;", callback, 0, &ErrMsg);
    if (rc != SQLITE_OK)
    {
        fprintf (stderr, "Error begin md_create_type_batch_stub: Line: %d SQL error: %s\n", __LINE__, ErrMsg);
        sqlite3_free (ErrMsg);
        sqlite3_close (db);
        goto cleanup;
    }

    rc = sqlite3_prepare_v2 (db, "insert into type_catalog (id, run_id, name, version, active, txn_id) values (?, ?, ?, ?, ?, ?)", -1, &stmt, &tail);
    if (rc != SQLITE_OK)
    {
        fprintf (stderr, "Error start of create_type_batch_stub: Line: %d SQL error: %s\n", __LINE__, sqlite3_errmsg (db));
        sqlite3_close (db);
        goto cleanup;
    }

    for(int arg_indx = 0; arg_indx < all_args.size(); arg_indx++) {

    	md_create_type_args args = all_args.at(arg_indx);

	    // rc = sqlite3_bind_int64 (stmt, 1, args.type_id); assert (rc == SQLITE_OK);    
	    rc = sqlite3_bind_null (stmt, 1); assert (rc == SQLITE_OK);
	    rc = sqlite3_bind_int64 (stmt, 2, args.run_id); assert (rc == SQLITE_OK);    
	    rc = sqlite3_bind_text (stmt, 3, strdup(args.name.c_str()), -1, free); assert (rc == SQLITE_OK);
	    rc = sqlite3_bind_int (stmt, 4, args.version); assert (rc == SQLITE_OK);
	    rc = sqlite3_bind_int (stmt, 5, 0); assert (rc == SQLITE_OK);
	    rc = sqlite3_bind_int64 (stmt, 6, args.txn_id); assert (rc == SQLITE_OK);

	    // cout << "run_id: " << args.run_id << " name: " << args.name << " version: " << args.version << " txn_id: " << args.txn_id << endl;

	    rc = sqlite3_step (stmt); assert (rc == SQLITE_ROW || rc == SQLITE_DONE);
	    
    	// uint64_t type_id = (int) sqlite3_last_insert_rowid (db);
    	// type_ids.push_back(type_id);

		rc = sqlite3_reset(stmt); assert (rc == SQLITE_OK);
	}

//printf ("generated new global id:%d\n", *type_id);

    rc = sqlite3_finalize (stmt);

    rc = sqlite3_exec (db, "end;", callback, 0, &ErrMsg);
    if (rc != SQLITE_OK)
    {
        fprintf (stderr, "Error end query md_creat_type_batch_stub: Line: %d SQL error: %s\n", __LINE__, ErrMsg);
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


