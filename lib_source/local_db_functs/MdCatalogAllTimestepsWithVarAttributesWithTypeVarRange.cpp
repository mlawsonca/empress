#include <md_local.hh>

extern sqlite3 *db;

using namespace std;


template <class T>
static int get_matching_timestep_count (const md_catalog_all_timesteps_with_var_attributes_with_type_var_range_args &args, 
	T min, T max, string query_str, uint32_t &count);


int md_catalog_all_timesteps_with_var_attributes_with_type_var_range_stub (const md_catalog_all_timesteps_with_var_attributes_with_type_var_range_args &args,
                           std::vector<md_catalog_timestep_entry> &entries,
                           uint32_t &count)
{
    int rc;
    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;
    string query = "select tmc.* from timestep_catalog tmc "
    "inner join type_catalog tc on tc.run_id = tmc.run_id "
    "inner join var_attribute_catalog vac on vac.timestep_id = tmc.id and vac.type_id = tc.id "
    "where (vac.txn_id = ? or vac.active = 1) "
    "and (tmc.txn_id = ? or tmc.active = 1) "
    "and vac.type_id = ? "
    "and vac.var_id = ? "
    "and tmc.run_id = ? "
    "and  (vac.data_type = ? or vac.data_type = ?) ";

    string query_str;

    switch(args.range_type) {
    	case DATA_RANGE : {
    		query_str = "and ( ? <= vac.data and vac.data <= ? ) "
    			"group by tmc.id, tmc.run_id";
    		break;
    	}
       	case DATA_MAX : {
    		query_str = "and ( ? <= vac.data ) "
    			"group by tmc.id, tmc.run_id";
    		break;
    	}
        case DATA_MIN : {
    		query_str = "and ( vac.data <= ? ) "
    		    "group by tmc.id, tmc.run_id";
    		break;
    	}
    }
   	query += query_str;

	uint64_t min_int, max_int;
	long double min_real, max_real;

    switch(args.data_type) {
    	case ATTR_DATA_TYPE_INT : {
			get_data_int( args.data, args.range_type, min_int, max_int);
			rc = get_matching_timestep_count (args, min_int, max_int, query_str, count); assert (rc == RC_OK);
    		// cout << "case is int. max: " << max_int << " min: " << min_int << endl;
    		break;
    	}
       	case ATTR_DATA_TYPE_REAL : {
			get_data_real( args.data, args.range_type, min_real, max_real);
			rc = get_matching_timestep_count (args, min_real, max_real, query_str, count); assert (rc == RC_OK);
    		// cout << "case is real. max: " << max_real << " min: " << min_real << endl;
    		// cout << "args.data: " << args.data << endl;
    		break;
    	}
    }



    // cout << "in op, count: " << count << endl;
    // rc = get_matching_timestep_count (args, min, max, query_str, count); assert (rc == RC_OK);

    if (count > 0) {
        entries.reserve(count);

        rc = sqlite3_prepare_v2 (db, query.c_str(), -1, &stmt, &tail); 
        if (rc != SQLITE_OK)
        {
            fprintf (stderr, "Error catalog_all_timesteps_with_var_attributes_with_type_var_range stub: Line: %d SQL error (%d): %s\n", __LINE__, rc, sqlite3_errmsg (db));
            sqlite3_close (db);
            goto cleanup;
        }
        // rc = sqlite3_prepare_v2 (db, query.c_str(), -1, &stmt, &tail); assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int64 (stmt, 1, args.txn_id); assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int64 (stmt, 2, args.txn_id); assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int64 (stmt, 3, args.type_id); assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int64 (stmt, 4, args.var_id); assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int64 (stmt, 5, args.run_id); assert (rc == SQLITE_OK);

	    rc = sqlite3_bind_int (stmt, 6, ATTR_DATA_TYPE_INT); assert (rc == SQLITE_OK);
	    rc = sqlite3_bind_int (stmt, 7, ATTR_DATA_TYPE_REAL); assert (rc == SQLITE_OK);

		switch(args.data_type) {
	        case ATTR_DATA_TYPE_INT : {
	        	rc = sqlite3_bind_int64 (stmt, 8, min_int); assert (rc == SQLITE_OK);
	        	if(args.range_type == DATA_RANGE) {
	        		rc = sqlite3_bind_int64 (stmt, 9, max_int); assert (rc == SQLITE_OK);
	        	}
	            break;
	        }
	        case ATTR_DATA_TYPE_REAL : {
	        	rc = sqlite3_bind_double (stmt, 8, min_real); assert (rc == SQLITE_OK);
	    	    if(args.range_type == DATA_RANGE) {
	        		rc = sqlite3_bind_double (stmt, 9, max_real); assert (rc == SQLITE_OK);
	        	}
	            break;
	        }
	    }

        rc = sqlite3_step (stmt); assert (rc == SQLITE_ROW || rc == SQLITE_DONE);

        while (rc == SQLITE_ROW)
        {
            md_catalog_timestep_entry entry;

            entry.timestep_id = sqlite3_column_int64 (stmt, 0);
            entry.run_id = sqlite3_column_int64 (stmt, 1);
            entry.path = (char *)sqlite3_column_text (stmt, 2);
            entry.active = sqlite3_column_int (stmt, 3);
            entry.txn_id = sqlite3_column_int64 (stmt, 4);
            // cout << "attr.dims.at(2).min: " << attribute.dims.at(2).min << " attr.dims.at(2).min: " << attribute.dims.at(2).max << endl;

            rc = sqlite3_step (stmt);
            entries.push_back(entry);
        }

        rc = sqlite3_finalize (stmt);  
    }
        

cleanup:

    return rc;
}


template <class T>
static int get_matching_timestep_count (const md_catalog_all_timesteps_with_var_attributes_with_type_var_range_args &args, 
	T min, T max, string query_str, uint32_t &count)
{
    int rc;
    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;
    string query = "select count(*) from (select distinct tmc.id, tmc.run_id from timestep_catalog tmc "
    "inner join type_catalog tc on tc.run_id = tmc.run_id "
    "inner join var_attribute_catalog vac on vac.timestep_id = tmc.id and vac.type_id = tc.id "
    "where (vac.txn_id = ? or vac.active = 1) "
    "and (tmc.txn_id = ? or tmc.active = 1) "
    "and vac.type_id = ? "
    "and vac.var_id = ? "
    "and tmc.run_id = ? "
    "and  (vac.data_type = ? or vac.data_type = ?) " + query_str + " ) as internalQuery";

    rc = sqlite3_prepare_v2 (db, query.c_str(), -1, &stmt, &tail); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 1, args.txn_id); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 2, args.txn_id); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 3, args.type_id); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 4, args.var_id); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 5, args.run_id); assert (rc == SQLITE_OK);

    rc = sqlite3_bind_int (stmt, 6, ATTR_DATA_TYPE_INT); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int (stmt, 7, ATTR_DATA_TYPE_REAL); assert (rc == SQLITE_OK);

	switch(args.data_type) {
        case ATTR_DATA_TYPE_INT : {
        	rc = sqlite3_bind_int64 (stmt, 8, (uint64_t)min); assert (rc == SQLITE_OK);
        	if(args.range_type == DATA_RANGE) {
        		rc = sqlite3_bind_int64 (stmt, 9, (uint64_t)max); assert (rc == SQLITE_OK);
        	}
            break;
        }
        case ATTR_DATA_TYPE_REAL : {
        	rc = sqlite3_bind_double (stmt, 8, (long double)min); assert (rc == SQLITE_OK);
    	    if(args.range_type == DATA_RANGE) {
        		rc = sqlite3_bind_double (stmt, 9, (long double)max); assert (rc == SQLITE_OK);
        	}
            break;
        }
    }

    rc = sqlite3_step (stmt); assert (rc == SQLITE_ROW || rc == SQLITE_DONE);
    count = sqlite3_column_int64 (stmt, 0);
    rc = sqlite3_finalize (stmt); assert (rc == SQLITE_OK);

    // cout << "count: " << count << endl;

    return rc;
}
