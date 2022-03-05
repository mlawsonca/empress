#include <md_local.hh>

extern sqlite3 *db;

using namespace std;

// int md_insert_run_attribute_batch_stub (const vector<md_insert_run_attribute_args> &args, uint64_t &attribute_id);
extern int callback (void * NotUsed, int argc, char ** argv, char ** ColName);


// int md_insert_run_attribute_batch_stub (const vector<md_insert_run_attribute_args> &args,
//                           uint64_t &attribute_id)
int md_insert_run_attribute_batch_stub (const vector<md_insert_run_attribute_args> &all_args)
{
    int rc;
    int i = 0;
    sqlite3_stmt * stmt = NULL;
    const char * tail_index = NULL;
    char * ErrMsg = NULL;


    rc = sqlite3_exec (db, "begin;", callback, 0, &ErrMsg);
    if (rc != SQLITE_OK)
    {
        fprintf (stderr, "Error begin md_insert_run_attribute_batch_stub: Line: %d SQL error: %s\n", __LINE__, ErrMsg);
        sqlite3_free (ErrMsg);
        sqlite3_close (db);
        goto cleanup;
    }

    rc = sqlite3_prepare_v2 (db, "insert into run_attribute_catalog (id, run_id, type_id, active, txn_id, data_type, data) values (?, ?, ?, ?, ?, ?, ?)", -1, &stmt, &tail_index);
    if (rc != SQLITE_OK)
    {
        fprintf (stderr, "Error at start of insert_run_attribute_batch_stub: Line: %d SQL error: %s\n", __LINE__, sqlite3_errmsg (db));
        sqlite3_close (db);
        goto cleanup;
    }

    for(int arg_indx = 0; arg_indx < all_args.size(); arg_indx++) {

    	md_insert_run_attribute_args args = all_args.at(arg_indx);

	    rc = sqlite3_bind_null (stmt, 1); assert (rc == SQLITE_OK);
	    rc = sqlite3_bind_int64 (stmt, 2, args.run_id); assert (rc == SQLITE_OK);	    
	    rc = sqlite3_bind_int64 (stmt, 3, args.type_id); assert (rc == SQLITE_OK);
	    rc = sqlite3_bind_int (stmt, 4, 0); assert (rc == SQLITE_OK);
	    rc = sqlite3_bind_int64 (stmt, 5, args.txn_id); assert (rc == SQLITE_OK);
	    rc = sqlite3_bind_int (stmt, 6, args.data_type); assert (rc == SQLITE_OK);
	    switch(args.data_type) {
	        case ATTR_DATA_TYPE_NULL : {
	            rc = sqlite3_bind_null (stmt, 7); 
	            break;
	        }        
	        case ATTR_DATA_TYPE_INT : {
	            uint64_t deserialized_int;
	        	stringstream sso;
	        	sso << args.data;
	        	boost::archive::text_iarchive ia(sso);
	            ia >> deserialized_int;
	            rc = sqlite3_bind_int64 (stmt, 7, deserialized_int); 
	            break;
	        }
	        case ATTR_DATA_TYPE_REAL : {
	            long double deserialized_real;
	        	stringstream sso;
	        	sso << args.data;
	        	boost::archive::text_iarchive ia(sso);
	            ia >> deserialized_real;
	            rc = sqlite3_bind_double (stmt, 7, deserialized_real); 
	            break;
	        }
	        case ATTR_DATA_TYPE_TEXT : {
	            rc = sqlite3_bind_text (stmt, 7, strdup(args.data.c_str()), -1, free);
	            break;
	        }
	        case ATTR_DATA_TYPE_BLOB : {
	            rc = sqlite3_bind_text (stmt, 7, args.data.c_str(), args.data.size()+1, SQLITE_TRANSIENT);
	            // rc = sqlite3_bind_blob64 (stmt, 6, args.data, -1, free); 
	            break;
	        }
	    }

	    rc = sqlite3_step (stmt); assert (rc == SQLITE_ROW || rc == SQLITE_DONE);

		rc = sqlite3_reset(stmt); assert (rc == SQLITE_OK);
	    // attribute_id = (int) sqlite3_last_insert_rowid (db);
	    // std::cout << " According to the server, the attr id is " << attribute_id << std::endl;
	}

    rc = sqlite3_finalize (stmt);

    rc = sqlite3_exec (db, "end;", callback, 0, &ErrMsg);
    if (rc != SQLITE_OK)
    {
        fprintf (stderr, "Error end query md_insert_run_attribute_batch_stub: Line: %d SQL error: %s\n", __LINE__, ErrMsg);
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