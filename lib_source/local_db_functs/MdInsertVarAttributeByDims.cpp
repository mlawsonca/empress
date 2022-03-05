#include <md_local.hh>

extern sqlite3 *db;

using namespace std;


int md_insert_var_attribute_by_dims_stub (const md_insert_var_attribute_by_dims_args &args,
                          uint64_t &attribute_id)
{
    int rc;
    int i = 0;
    // int i;
    // char * ErrMsg = NULL;
    sqlite3_stmt * stmt = NULL;
    const char * tail_index = NULL;

    // int rowid;

    rc = sqlite3_prepare_v2 (db, "insert into var_attribute_catalog (id, timestep_id, type_id, var_id, active, txn_id, num_dims, d0_min, d0_max, d1_min, d1_max, d2_min, d2_max, data_type, data) values (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)", -1, &stmt, &tail_index);
    if (rc != SQLITE_OK)
    {
        fprintf (stderr, "Error at start of insert_var_attribute_stub: Line: %d SQL error: %s\n", __LINE__, sqlite3_errmsg (db));
        sqlite3_close (db);
        goto cleanup;
    }

    rc = sqlite3_bind_null (stmt, 1); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 2, args.timestep_id); assert (rc == SQLITE_OK);   
    rc = sqlite3_bind_int64 (stmt, 3, args.type_id); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 4, args.var_id); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int (stmt, 5, 0); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 6, args.txn_id); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int (stmt, 7, args.num_dims); assert (rc == SQLITE_OK);
    while(i < args.num_dims) {
        rc = sqlite3_bind_double (stmt, 8 + i * 2, args.dims.at(i).min); assert (rc == SQLITE_OK);
        rc = sqlite3_bind_double (stmt, 9 + i * 2, args.dims.at(i).max); assert (rc == SQLITE_OK);
        i++;
    }
    rc = sqlite3_bind_int (stmt, 14, args.data_type); assert (rc == SQLITE_OK);
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
        	// cout << "serialized int data on insertion: " << args.data << endl;
            ia >> deserialized_int;
            // cout << "deserialized_int: " << deserialized_int << endl;
            rc = sqlite3_bind_int64 (stmt, 15, deserialized_int); 
            break;
        }
        case ATTR_DATA_TYPE_REAL : {
            long double deserialized_real;
        	stringstream sso;
        	sso << args.data;
        	boost::archive::text_iarchive ia(sso);
            ia >> deserialized_real;
            // cout << "deserialized_real: " << deserialized_real << endl;
            rc = sqlite3_bind_double (stmt, 15, deserialized_real); 
            break;
        }
        case ATTR_DATA_TYPE_TEXT : {
            // cout << "text: " << args.data.c_str() << endl;
            rc = sqlite3_bind_text (stmt, 15, strdup(args.data.c_str()), -1, free);
            break;
        }
        case ATTR_DATA_TYPE_BLOB : {
            // cout << "blob: " << args.data.c_str() << endl;
            // cout << "blob: " << args.data << " str size: " << args.data.size() << endl;
            // rc = sqlite3_bind_text (stmt, 15, strdup(args.data.c_str()), args.data.size()+1, free);
            rc = sqlite3_bind_text (stmt, 15, args.data.c_str(), args.data.size()+1, SQLITE_TRANSIENT);

            // rc = sqlite3_bind_blob64 (stmt, 15, args.data, -1, free); 
            break;
        }
    }
    assert (rc == SQLITE_OK);

    rc = sqlite3_step (stmt); assert (rc == SQLITE_ROW || rc == SQLITE_DONE);


    attribute_id = (int) sqlite3_last_insert_rowid (db);
    // std::cout << " According to the server, the attr id is " << attribute_id << std::endl;

    rc = sqlite3_finalize (stmt);


cleanup:

    return rc;
}