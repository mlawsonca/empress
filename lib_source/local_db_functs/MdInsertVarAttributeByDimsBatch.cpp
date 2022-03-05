#include <md_local.hh>

extern sqlite3 *db;

using namespace std;

// int md_insert_var_attribute_by_dims_batch_stub (const vector<md_insert_var_attribute_by_dims_args> &args, vector<uint64_t> &attribute_ids);
extern int callback (void * NotUsed, int argc, char ** argv, char ** ColName);


// int md_insert_var_attribute_by_dims_batch_stub (const vector<md_insert_var_attribute_by_dims_args> &args,
//                           vector<uint64_t> &attribute_ids)
int md_insert_var_attribute_by_dims_batch_stub (const vector<md_insert_var_attribute_by_dims_args> &all_args)
{
    int rc;
    // int i;
    // char * ErrMsg = NULL;
    sqlite3_stmt * stmt = NULL;
    const char * tail_index = NULL;
    char * ErrMsg = NULL;

    // int rowid;

    // char *err_msg = 0;
    // char *sql = "SELECT * FROM var_attribute_catalog";
    
    rc = sqlite3_exec (db, "begin;", callback, 0, &ErrMsg);
    if (rc != SQLITE_OK)
    {
        fprintf (stderr, "Error begin md_insert_var_attribute_by_dims_batch_stub: Line: %d SQL error: %s\n", __LINE__, ErrMsg);
        sqlite3_free (ErrMsg);
        sqlite3_close (db);
        goto cleanup;
    }

    rc = sqlite3_prepare_v2 (db, "insert into var_attribute_catalog (id, timestep_id, type_id, var_id, active, txn_id, num_dims, d0_min, d0_max, d1_min, d1_max, d2_min, d2_max, data_type, data) values (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)", -1, &stmt, &tail_index);
    if (rc != SQLITE_OK)
    {
        fprintf (stderr, "Error at start of insert_var_attribute_stub: Line: %d SQL error: %s\n", __LINE__, sqlite3_errmsg (db));
        sqlite3_close (db);
        goto cleanup;
    }

    //cout << " all_args.size(): " <<  all_args.size() << endl;
    for(int arg_indx = 0; arg_indx < all_args.size(); arg_indx++) {
    	// //cout << "got to top of loop" << endl;

    	int dim = 0;


    	md_insert_var_attribute_by_dims_args args = all_args.at(arg_indx);

    	//cout << " args.timestep_id: " <<  args.timestep_id << endl;
    	//cout << " args.type_id: " <<  args.type_id << endl;
    	//cout << " args.var_id: " <<  args.var_id << endl;
    	//cout << " args.txn_id: " <<  args.txn_id << endl;
    	//cout << " args.num_dims: " <<  args.num_dims << endl;


	    rc = sqlite3_bind_null (stmt, 1); assert (rc == SQLITE_OK);
	    rc = sqlite3_bind_int64 (stmt, 2, args.timestep_id); assert (rc == SQLITE_OK);   
	    rc = sqlite3_bind_int64 (stmt, 3, args.type_id); assert (rc == SQLITE_OK);
	    rc = sqlite3_bind_int64 (stmt, 4, args.var_id); assert (rc == SQLITE_OK);
	    rc = sqlite3_bind_int (stmt, 5, 0); assert (rc == SQLITE_OK);
	    rc = sqlite3_bind_int64 (stmt, 6, args.txn_id); assert (rc == SQLITE_OK);
	    rc = sqlite3_bind_int (stmt, 7, args.num_dims); assert (rc == SQLITE_OK);
	    while(dim < args.num_dims) {
	        //cout << " args.d[" << dim << "].min: " <<  args.dims.at(dim).min << endl;
	        //cout << " args.d[" << dim << "].max: " <<  args.dims.at(dim).max << endl;

	        rc = sqlite3_bind_double (stmt, 8 + dim * 2, args.dims.at(dim).min); assert (rc == SQLITE_OK);
	        rc = sqlite3_bind_double (stmt, 9 + dim * 2, args.dims.at(dim).max); assert (rc == SQLITE_OK);
	        dim++;
	    }
		//if num dims < 3 explicitly bind null to prevent bleed over from prev call
	    while(dim < 3) {
	    	rc = sqlite3_bind_null (stmt, 8 + dim * 2); 
	    	rc = sqlite3_bind_null (stmt, 9 + dim * 2); 
	  	    dim++;
	    }
	    rc = sqlite3_bind_int (stmt, 14, args.data_type); assert (rc == SQLITE_OK);
	 	//cout << " args.data_type: " <<  args.data_type << endl;

	    switch(args.data_type) {
	        case ATTR_DATA_TYPE_NULL : {
	            rc = sqlite3_bind_null (stmt, 15); 
	            break;
	        }        
	        case ATTR_DATA_TYPE_INT : {
	            uint64_t deserialized_int;
	        	stringstream sso;
	        	sso << args.data;
	        	boost::archive::text_iarchive ia(sso);
	        	// //cout << "serialized int data on insertion: " << args.data << endl;
	            ia >> deserialized_int;
	            // //cout << "deserialized_int: " << deserialized_int << endl;
	            rc = sqlite3_bind_int64 (stmt, 15, deserialized_int); 
	            break;
	        }
	        case ATTR_DATA_TYPE_REAL : {
	            long double deserialized_real;
	        	stringstream sso;
	        	sso << args.data;
	        	boost::archive::text_iarchive ia(sso);
	            ia >> deserialized_real;
	            //cout << "deserialized_real: " << deserialized_real << endl;
	            rc = sqlite3_bind_double (stmt, 15, deserialized_real); 
	            break;
	        }
	        case ATTR_DATA_TYPE_TEXT : {
	            //cout << "text: " << args.data.c_str() << endl;
	            rc = sqlite3_bind_text (stmt, 15, strdup(args.data.c_str()), -1, free);
	            break;
	        }
	        case ATTR_DATA_TYPE_BLOB : {
	            //cout << "blob: " << args.data.c_str() << endl;
	            //cout << "blob: " << args.data << " str size: " << args.data.size() << endl;
	            // rc = sqlite3_bind_text (stmt, 15, strdup(args.data.c_str()), args.data.size()+1, free);
	            rc = sqlite3_bind_text (stmt, 15, args.data.c_str(), args.data.size()+1, SQLITE_TRANSIENT);

	            // rc = sqlite3_bind_blob64 (stmt, 15, args.data, -1, free); 
	            break;
	        }
	    }
	    // //cout << "got to bottom of bind \n";
	    assert (rc == SQLITE_OK);

	    rc = sqlite3_step (stmt); assert (rc == SQLITE_ROW || rc == SQLITE_DONE);
	    // //cout << "just stepped \n";

	    rc = sqlite3_reset(stmt); assert (rc == SQLITE_OK);
	    // //cout << "just reset. rc: " << rc << " \n";

	    // rc = sqlite3_clear_bindings(stmt);
	    // //cout << "just cleared bindings. rc: " << rc << " \n";

	    // attribute_id = (int) sqlite3_last_insert_rowid (db);
	    // attribute_ids.push_back(attribute_id);
	}
	// //cout << "about to finalize \n";

    rc = sqlite3_finalize (stmt);
	// //cout << "just finalized \n";

    rc = sqlite3_exec (db, "end;", callback, 0, &ErrMsg);
    if (rc != SQLITE_OK)
    {
        fprintf (stderr, "Error end query md_insert_var_attribute_by_dims_batch_stub: Line: %d SQL error: %s\n", __LINE__, ErrMsg);
        sqlite3_free (ErrMsg);
        sqlite3_close (db);
        goto cleanup;
    }




    //cout << "starting var catalog \n";
    // rc = sqlite3_exec(db, sql, callback, 0, &err_msg);
    //cout << "done with var catalog \n";
	//cout << "just execed \n";


cleanup:
    if (rc != SQLITE_OK)
    {
        rc = sqlite3_exec (db, "rollback;", callback, 0, &ErrMsg);
    }
    return rc;
}
