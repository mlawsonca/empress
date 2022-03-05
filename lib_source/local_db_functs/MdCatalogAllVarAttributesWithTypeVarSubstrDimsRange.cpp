#include <md_local.hh>

extern sqlite3 *db;

using namespace std;


template <class T1>
static int get_matching_attribute_count (const md_catalog_all_var_attributes_with_type_var_substr_dims_range_args &args, 
	T1 min, T1 max, string query_str, uint32_t &count);



// template <class T>
int md_catalog_all_var_attributes_with_type_var_substr_dims_range_stub (const md_catalog_all_var_attributes_with_type_var_substr_dims_range_args &args,
                           // T min, T max,
                           std::vector<md_catalog_var_attribute_entry> &attribute_list,
                           uint32_t &count)
{
    int rc;
    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;
    string query;

    if (args.num_dims == 1) {
	   	query = "select vac.* from var_attribute_catalog vac "
		    "inner join var_catalog vc on vac.timestep_id = vc.timestep_id and vac.var_id = vc.id "
		    "inner join type_catalog tc on vac.type_id = tc.id and vc.run_id = tc.run_id "
		    "where (vac.txn_id = ? or vac.active = 1) "
		    "and vac.type_id = ? "
		    "and vc.name like ? "
		    "and vac.timestep_id = ? "
	 		"and ? <= vac.d0_max and ? >= vac.d0_min "
	    	"and  (vac.data_type = ? or vac.data_type = ?) ";
    }
    else if(args.num_dims == 2) {
   		query = "select vac.* from var_attribute_catalog vac "
		    "inner join var_catalog vc on vac.timestep_id = vc.timestep_id and vac.var_id = vc.id "
		    "inner join type_catalog tc on vac.type_id = tc.id and vc.run_id = tc.run_id "
		    "where (vac.txn_id = ? or vac.active = 1) "
		    "and vac.type_id = ? "
		    "and vc.name like ? "
		    "and vac.timestep_id = ? "
	 		"and ? <= vac.d0_max and ? >= vac.d0_min "
			"and ? <= vac.d1_max and ? >= vac.d1_min "
	    	"and  (vac.data_type = ? or vac.data_type = ?) ";
    }
    else if(args.num_dims == 3) {
   		query = "select vac.* from var_attribute_catalog vac "
		    "inner join var_catalog vc on vac.timestep_id = vc.timestep_id and vac.var_id = vc.id "
		    "inner join type_catalog tc on vac.type_id = tc.id and vc.run_id = tc.run_id "
		    "where (vac.txn_id = ? or vac.active = 1) "
		    "and vac.type_id = ? "
		    "and vc.name like ? "
		    "and vac.timestep_id = ? "
	 		"and ? <= vac.d0_max and ? >= vac.d0_min "
			"and ? <= vac.d1_max and ? >= vac.d1_min "
			"and ? <= vac.d2_max and ? >= vac.d2_min "
	    	"and  (vac.data_type = ? or vac.data_type = ?) ";
    }
    else {
    	count = 0;
    	return rc;
    }

    string query_str;

    switch(args.range_type) {
    	case DATA_RANGE : {
    		query_str = "and ( ? <= vac.data and vac.data <= ? ) ";
    		break;
    	}
       	case DATA_MAX : {
    		query_str = "and ( ? <= vac.data ) ";
    		break;
    	}
        case DATA_MIN : {
    		query_str = "and ( vac.data <= ? ) ";
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
    // rc = get_matching_attribute_count (args, min, max, query_str, count); assert (rc == RC_OK);

    if (count > 0) {
        attribute_list.reserve(count);

        rc = sqlite3_prepare_v2 (db, query.c_str(), -1, &stmt, &tail); 
        if (rc != SQLITE_OK)
        {
            fprintf (stderr, "Error catalog_all_var_attributes_with_type_var_substr_dims_range stub: Line: %d SQL error (%d): %s\n", __LINE__, rc, sqlite3_errmsg (db));
            sqlite3_close (db);
            goto cleanup;
        }
        rc = sqlite3_bind_int64 (stmt, 1, args.txn_id); assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int64 (stmt, 2, args.type_id); assert (rc == SQLITE_OK);
    	rc = sqlite3_bind_text (stmt, 3, strdup(args.var_name_substr.c_str()), -1, free); assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int64 (stmt, 4, args.timestep_id); assert (rc == SQLITE_OK);

        for (int j = 0; j < args.num_dims; j++)
        {
            rc = sqlite3_bind_double (stmt, 5 + (j * 2), args.dims.at(j).min);
            rc = sqlite3_bind_double (stmt, 6 + (j * 2), args.dims.at(j).max);
        }

	    rc = sqlite3_bind_int (stmt, 11, ATTR_DATA_TYPE_INT); assert (rc == SQLITE_OK);
	    rc = sqlite3_bind_int (stmt, 12, ATTR_DATA_TYPE_REAL); assert (rc == SQLITE_OK);

		switch(args.data_type) {
	        case ATTR_DATA_TYPE_INT : {
	        	rc = sqlite3_bind_int64 (stmt, 13, min_int); assert (rc == SQLITE_OK);
	    		if(args.range_type == DATA_RANGE) {
	        		rc = sqlite3_bind_int64 (stmt, 14, max_int); assert (rc == SQLITE_OK);
	        	}
	            break;
	        }
	        case ATTR_DATA_TYPE_REAL : {
	        	rc = sqlite3_bind_double (stmt, 13, min_real); assert (rc == SQLITE_OK);
	    		if(args.range_type == DATA_RANGE) {
	        		rc = sqlite3_bind_double (stmt, 14, max_real); assert (rc == SQLITE_OK);
	        	}
	            break;
	        }
	    }

        rc = sqlite3_step (stmt); assert (rc == SQLITE_ROW || rc == SQLITE_DONE);

        while (rc == SQLITE_ROW)
        {
            md_catalog_var_attribute_entry attribute;

            attribute.attribute_id = sqlite3_column_int64 (stmt, 0);
            attribute.timestep_id = sqlite3_column_int64 (stmt, 1);
            attribute.type_id = sqlite3_column_int64 (stmt, 2);
            attribute.var_id = sqlite3_column_int64 (stmt, 3);
            attribute.active = sqlite3_column_int (stmt, 4);
            attribute.txn_id = sqlite3_column_int64 (stmt, 5);
            attribute.num_dims = sqlite3_column_int (stmt, 6);
            attribute.dims.reserve(3);
            // attribute.dims.reserve(attribute.num_dims);
            int j = 0;
            // while (j < attribute.num_dims)
            while (j < 3)
            {
                md_dim_bounds bounds;
                bounds.min =  sqlite3_column_double (stmt, 7 + (j * 2));
                bounds.max = sqlite3_column_double (stmt, 8 + (j * 2));
                attribute.dims.push_back(bounds);
                j++;
            }
            attribute.data_type = (attr_data_type) sqlite3_column_int (stmt, 13);                        
			switch(attribute.data_type) {
				case ATTR_DATA_TYPE_INT : {
					stringstream ss;
				    boost::archive::text_oarchive oa(ss);
					oa << (uint64_t)sqlite3_column_int64 (stmt, 14);
					attribute.data = ss.str();
        			// cout << "serialized int data on catalog: " << attribute.data << endl;
					break;
				}
				case ATTR_DATA_TYPE_REAL : {
					stringstream ss;
				    boost::archive::text_oarchive oa(ss);
					oa << (long double)sqlite3_column_double (stmt, 14);
					attribute.data = ss.str();
					break;
				}
			}    
            // attribute.data = (char *) sqlite3_column_text (stmt, 14);
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
static int get_matching_attribute_count (const md_catalog_all_var_attributes_with_type_var_substr_dims_range_args &args,
	T min, T max, string query_str, uint32_t &count)
{
    int rc;
    sqlite3_stmt * stmt = NULL;

    string query;
    
    if (args.num_dims == 1) {
   		query = "select count (*) from var_attribute_catalog vac "
		    "inner join var_catalog vc on vac.timestep_id = vc.timestep_id and vac.var_id = vc.id "
		    "inner join type_catalog tc on vac.type_id = tc.id and vc.run_id = tc.run_id "
		    "where (vac.txn_id = ? or vac.active = 1) "
		    "and vac.type_id = ? "
		    "and vc.name like ? "
		    "and vac.timestep_id = ? "
	 		"and ? <= vac.d0_max and ? >= vac.d0_min "
	    	"and  (vac.data_type = ? or vac.data_type = ?) " + query_str;
    }
    else if(args.num_dims == 2) {
   		query = "select count (*) from var_attribute_catalog vac "
		    "inner join var_catalog vc on vac.timestep_id = vc.timestep_id and vac.var_id = vc.id "
		    "inner join type_catalog tc on vac.type_id = tc.id and vc.run_id = tc.run_id "
		    "where (vac.txn_id = ? or vac.active = 1) "
		    "and vac.type_id = ? "
		    "and vc.name like ? "
		    "and vac.timestep_id = ? "
	 		"and ? <= vac.d0_max and ? >= vac.d0_min "
			"and ? <= vac.d1_max and ? >= vac.d1_min "
	    	"and  (vac.data_type = ? or vac.data_type = ?) " + query_str;
    }
    else if(args.num_dims == 3) {
   		query = "select count (*) from var_attribute_catalog vac "
		    "inner join var_catalog vc on vac.timestep_id = vc.timestep_id and vac.var_id = vc.id "
		    "inner join type_catalog tc on vac.type_id = tc.id and vc.run_id = tc.run_id "
		    "where (vac.txn_id = ? or vac.active = 1) "
		    "and vac.type_id = ? "
		    "and vc.name like ? "
		    "and vac.timestep_id = ? "
	 		"and ? <= vac.d0_max and ? >= vac.d0_min "
			"and ? <= vac.d1_max and ? >= vac.d1_min "
			"and ? <= vac.d2_max and ? >= vac.d2_min "
	    	"and  (vac.data_type = ? or vac.data_type = ?) " + query_str;
    }

    const char * tail = NULL;
	

    rc = sqlite3_prepare_v2 (db, query.c_str(), -1, &stmt, &tail); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 1, args.txn_id); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 2, args.type_id); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_text (stmt, 3, strdup(args.var_name_substr.c_str()), -1, free); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 4, args.timestep_id); assert (rc == SQLITE_OK);

    for (int j = 0; j < args.num_dims; j++)
    {
        rc = sqlite3_bind_double (stmt, 5 + (j * 2), args.dims.at(j).min);
        rc = sqlite3_bind_double (stmt, 6 + (j * 2), args.dims.at(j).max);
    }

    rc = sqlite3_bind_int (stmt, 11, ATTR_DATA_TYPE_INT); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int (stmt, 12, ATTR_DATA_TYPE_REAL); assert (rc == SQLITE_OK);

	switch(args.data_type) {
        case ATTR_DATA_TYPE_INT : {
        	rc = sqlite3_bind_int64 (stmt, 13, (uint64_t)min); assert (rc == SQLITE_OK);
    		if(args.range_type == DATA_RANGE) {
        		rc = sqlite3_bind_int64 (stmt, 14, (uint64_t)max); assert (rc == SQLITE_OK);
        	}
            break;
        }
        case ATTR_DATA_TYPE_REAL : {
        	rc = sqlite3_bind_double (stmt, 13, (long double)min); assert (rc == SQLITE_OK);
    		if(args.range_type == DATA_RANGE) {
        		rc = sqlite3_bind_double (stmt, 14, (long double)max); assert (rc == SQLITE_OK);
        	}
            break;
        }
    }

    rc = sqlite3_step (stmt); assert (rc == SQLITE_ROW || rc == SQLITE_DONE);
    count = sqlite3_column_int64 (stmt, 0);
    rc = sqlite3_finalize (stmt); assert (rc == SQLITE_OK);
//printf ("matching attr count: %d\n", *count);

    return rc;
}
