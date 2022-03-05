#include <md_local.hh>

extern sqlite3 *db;

using namespace std;

template <class T>
static int get_matching_attribute_count (const md_catalog_all_run_attributes_with_type_range_args &args, 
	T min, T max, string query_str, uint32_t &count);


int md_catalog_all_run_attributes_with_type_range_stub (const md_catalog_all_run_attributes_with_type_range_args &args,
                           std::vector<md_catalog_run_attribute_entry> &attribute_list,
                           uint32_t &count)
{
    int rc;
    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;

    // string query = "select rac.* from run_attribute_catalog rac "
    // "inner join type_catalog tc on rac.type_id = tc.id "
    // "where (rac.txn_id = ? or rac.active = 1) "
    // "and rac.type_id = ? "
    // "and tc.run_id = ? "
    // "and  (rac.data_type = ? or rac.data_type = ?) ";

    string query = "select rac.* from run_attribute_catalog rac "
    "where (rac.txn_id = ? or rac.active = 1) "
    "and rac.type_id = ? "
    "and rac.run_id = ? "
    "and  (rac.data_type = ? or rac.data_type = ?) ";

    //	cout << "got here 2 \n";

    // const char *query_str;
    string query_str;

    switch(args.range_type) {
    	case DATA_RANGE : {
    		query_str = "and ( ? <= rac.data and rac.data <= ? ) ";
    		break;
    	}
       	case DATA_MAX : {
    		query_str = "and ( ? <= rac.data ) ";
    		break;
    	}
        case DATA_MIN : {
    		query_str = "and ( rac.data <= ? ) ";
    		break;
    	}
    }

	query += query_str;


	uint64_t min_int, max_int;
	long double min_real, max_real;

    switch(args.data_type) {
    	case ATTR_DATA_TYPE_INT : {
			get_data_int( args.data, args.range_type, min_int, max_int);
			rc = get_matching_attribute_count (args, min_int, max_int, query_str, count); assert (rc == RC_OK);
    		break;
    	}
       	case ATTR_DATA_TYPE_REAL : {
			get_data_real( args.data, args.range_type, min_real, max_real);
			rc = get_matching_attribute_count (args, min_real, max_real, query_str, count); assert (rc == RC_OK);
    		break;
    	}
    }

    //	cout << "got here 3 \n";


    //	cout << "got here 4 \n";

    if (count > 0) {
        attribute_list.reserve(count);


        rc = sqlite3_prepare_v2 (db, query.c_str(), -1, &stmt, &tail); 
        if (rc != SQLITE_OK)
        {
            fprintf (stderr, "Error catalog_all_run_attributes_with_type_range_stub: Line: %d SQL error (%d): %s\n", __LINE__, rc, sqlite3_errmsg (db));
            sqlite3_close (db);
            goto cleanup;
        }
    //	cout << "got here 5 \n";


        rc = sqlite3_bind_int64 (stmt, 1, args.txn_id); assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int64 (stmt, 2, args.type_id); assert (rc == SQLITE_OK);        
        rc = sqlite3_bind_int64 (stmt, 3, args.run_id); assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int (stmt, 4, ATTR_DATA_TYPE_INT); assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int (stmt, 5, ATTR_DATA_TYPE_REAL); assert (rc == SQLITE_OK);

		switch(args.data_type) {
	        case ATTR_DATA_TYPE_INT : {
	        	    //	cout << "got here 6 \n";

	        	rc = sqlite3_bind_int64 (stmt, 6, min_int); assert (rc == SQLITE_OK);
	        	if(args.range_type == DATA_RANGE) {
	        		rc = sqlite3_bind_int64 (stmt, 7, max_int); assert (rc == SQLITE_OK);
	        		    //	cout << "got here 7 \n";

	        	}
	            break;
	        }
	        case ATTR_DATA_TYPE_REAL : {
	        	rc = sqlite3_bind_double (stmt, 6, min_real); assert (rc == SQLITE_OK);
	        	if(args.range_type == DATA_RANGE) {
	        		rc = sqlite3_bind_double (stmt, 7, max_real); assert (rc == SQLITE_OK);
	        	}
	            break;
	        }
	    }

        rc = sqlite3_step (stmt); assert (rc == SQLITE_ROW || rc == SQLITE_DONE);

     // cout << "starting \n";
        while (rc == SQLITE_ROW)
        {
            md_catalog_run_attribute_entry attribute;

            attribute.attribute_id = sqlite3_column_int64 (stmt, 0);
            attribute.type_id = sqlite3_column_int64 (stmt, 1);
            attribute.active = sqlite3_column_int (stmt, 2);
            attribute.txn_id = sqlite3_column_int64 (stmt, 3);            
            attribute.data_type = (attr_data_type) sqlite3_column_int (stmt, 4);
            switch(attribute.data_type) {
                case ATTR_DATA_TYPE_INT : {
                    stringstream ss;
                    boost::archive::text_oarchive oa(ss);
                    oa << (uint64_t)sqlite3_column_int64 (stmt, 5);
                    attribute.data = ss.str();
                    break;
                }
                case ATTR_DATA_TYPE_REAL : {
                    stringstream ss;
                    boost::archive::text_oarchive oa(ss);                    
                    oa << (long double)sqlite3_column_double (stmt, 5);
                    attribute.data = ss.str();
                    break;
                }
            }                        
         // cout << "attr.dims.at(2).min: " << attribute.dims.at(2).min << " attr.dims.at(2).min: " << attribute.dims.at(2).max << endl;

            rc = sqlite3_step (stmt);
            attribute_list.push_back(attribute);
        }

        rc = sqlite3_finalize (stmt);  
    }
        

cleanup:

    return rc;
}


template <class T>
static int get_matching_attribute_count (const md_catalog_all_run_attributes_with_type_range_args &args, 
	T min, T max, string query_str, uint32_t &count)
{
    int rc;
    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;
    // string query = "select count (*) from run_attribute_catalog rac "
    // "inner join type_catalog tc on rac.type_id = tc.id "
    // "where (rac.txn_id = ? or rac.active = 1) "
    // "and rac.type_id = ? "
    // "and tc.run_id = ? "
    // "and  (rac.data_type = ? or rac.data_type = ?) " + query_str;// cout << "query: " << query << endl;
    string query = "select count (*) from run_attribute_catalog rac "
    "where (rac.txn_id = ? or rac.active = 1) "
    "and rac.type_id = ? "
    "and rac.run_id = ? "
    "and  (rac.data_type = ? or rac.data_type = ?) " + query_str;// cout << "query: " << query << endl;

    rc = sqlite3_prepare_v2 (db, query.c_str(), -1, &stmt, &tail); assert (rc == SQLITE_OK);

        //	cout << "got here 4.5 \n";

    rc = sqlite3_bind_int64 (stmt, 1, args.txn_id); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 2, args.type_id); assert (rc == SQLITE_OK);        
    rc = sqlite3_bind_int64 (stmt, 3, args.run_id); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int (stmt, 4, ATTR_DATA_TYPE_INT); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int (stmt, 5, ATTR_DATA_TYPE_REAL); assert (rc == SQLITE_OK);// cout << "min: " << min << " max: " << max << endl;

	switch(args.data_type) {
	    case ATTR_DATA_TYPE_INT : {
	    	        //	cout << "got here 4.6 \n";

	    	rc = sqlite3_bind_int64 (stmt, 6, (uint64_t)min); assert (rc == SQLITE_OK);
	    	if(args.range_type == DATA_RANGE) {
	    		rc = sqlite3_bind_int64 (stmt, 7, (uint64_t)max); assert (rc == SQLITE_OK);
	    		        //	cout << "got here 4.8 \n";

	    	}
	        break;
	    }
	    case ATTR_DATA_TYPE_REAL : {
	    	rc = sqlite3_bind_double (stmt, 6, (long double)min); assert (rc == SQLITE_OK);
	    	if(args.range_type == DATA_RANGE) {
	    		rc = sqlite3_bind_double (stmt, 7, (long double)max); assert (rc == SQLITE_OK);
	    	}
	        break;
	    }
	}
    rc = sqlite3_step (stmt); assert (rc == SQLITE_ROW || rc == SQLITE_DONE);
    count = sqlite3_column_int64 (stmt, 0);
 // cout << "in catalog all attrs, count: " << count << endl;
    rc = sqlite3_finalize (stmt); assert (rc == SQLITE_OK);

    return rc;
}
