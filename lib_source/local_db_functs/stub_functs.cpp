

int md_activate_stub (const md_activate_args &args)
{
    int rc;
    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;
    string query;

    switch(args.catalog_type) {
    	case RUN_CATALOG : {
    		query = "update run_catalog set active = 1 where txn_id = ?";
    		break;
    	}
    	case TIMESTEP_CATALOG : {
    		query = "update timestep_catalog set active = 1 where txn_id = ?";
    		break;
    	}
    	case VAR_CATALOG : {
    		query = "update var_catalog set active = 1 where txn_id = ?";
    		break;
    	}
    	case TYPE_CATALOG : {
    		query = "update type_catalog set active = 1 where txn_id = ?";
    		break;
    	}
    	case RUN_ATTR_CATALOG : {
    		query = "update run_attribute_catalog set active = 1 where txn_id = ?";
    		break;
    	}
    	case TIMESTEP_ATTR_CATALOG : {
    		query = "update timestep_attribute_catalog set active = 1 where txn_id = ?";
    		break;
    	}
    	case VAR_ATTR_CATALOG : {
    		query = "update var_attribute_catalog set active = 1 where txn_id = ?";
    		break;
    	}
    }

    rc = sqlite3_prepare_v2 (db, query.c_str(), -1, &stmt, &tail); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 1, args.txn_id); assert (rc == SQLITE_OK);
    rc = sqlite3_step (stmt); assert (rc == SQLITE_ROW || rc == SQLITE_DONE);
    rc = sqlite3_finalize (stmt); assert (rc == SQLITE_OK);

cleanup:

    return rc;
}


int md_catalog_all_run_attributes_stub (const md_catalog_all_run_attributes_args &args,
                           std::vector<md_catalog_run_attribute_entry> &attribute_list,
                           uint32_t &count)
{
    int rc;
    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;

    const char * query = "select rac.* from run_attribute_catalog rac "
    "inner join type_catalog tc on rac.type_id = tc.id "
    "where (rac.txn_id = ? or rac.active = 1) "
    "and tc.run_id = ? ";
    // }

    rc = get_matching_attribute_count_catalog_all_run_attributes (args, count); assert (rc == RC_OK);

    if (count > 0) {
        attribute_list.reserve(count);


        rc = sqlite3_prepare_v2 (db, query, -1, &stmt, &tail); 
        if (rc != SQLITE_OK)
        {
            fprintf (stderr, "Error catalog_all_run_attributes_stub: Line: %d SQL error (%d): %s\n", __LINE__, rc, sqlite3_errmsg (db));
            sqlite3_close (db);
            goto cleanup;
        }
        rc = sqlite3_bind_int64 (stmt, 1, args.txn_id); assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int64 (stmt, 2, args.run_id); assert (rc == SQLITE_OK);

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
            // cout << "in server, data type: " << attribute.data_type  << endl;
            switch(attribute.data_type) {
                // case ATTR_DATA_TYPE_NULL : {
                //     break;
                // }        

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
                case ATTR_DATA_TYPE_TEXT : {
                    attribute.data = (char *)sqlite3_column_text (stmt, 5);
                    break;
                }
                case ATTR_DATA_TYPE_BLOB : {
        			int size_blob = sqlite3_column_bytes (stmt, 5);
        			
            		attribute.data.assign((char *)sqlite3_column_text (stmt, 5),(char *)sqlite3_column_text (stmt, 5) + size_blob);
                    // attribute.data = (char *)sqlite3_column_text (stmt, 5);
                    // cout << "run attr in server data str length: " << attribute.data.size() << " str: " << attribute.data << endl;

                    break;
                }
            }            

            // attribute.data = (char *) sqlite3_column_text (stmt, 5);
            // cout << "attr.dims.at(2).min: " << attribute.dims.at(2).min << " attr.dims.at(2).min: " << attribute.dims.at(2).max << endl;

            rc = sqlite3_step (stmt);
            attribute_list.push_back(attribute);
        }

        rc = sqlite3_finalize (stmt);  
    }
        

cleanup:

    return rc;
}



static int get_matching_attribute_count_catalog_all_run_attributes (const md_catalog_all_run_attributes_args &args, uint32_t &count)
{
    int rc;
    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;
    const char * query = "select count (*) from run_attribute_catalog rac "
    "inner join type_catalog tc on rac.type_id = tc.id "
    "where (rac.txn_id = ? or rac.active = 1) "
    "and tc.run_id = ? ";

    rc = sqlite3_prepare_v2 (db, query, -1, &stmt, &tail); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 1, args.txn_id); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 2, args.run_id); assert (rc == SQLITE_OK);
        
    rc = sqlite3_step (stmt); assert (rc == SQLITE_ROW || rc == SQLITE_DONE);
    count = sqlite3_column_int64 (stmt, 0);
    // cout << "in catalog all attrs, count: " << count << endl;
    rc = sqlite3_finalize (stmt); assert (rc == SQLITE_OK);

    return rc;
}

int md_catalog_all_run_attributes_with_type_stub (const md_catalog_all_run_attributes_with_type_args &args,
                           std::vector<md_catalog_run_attribute_entry> &attribute_list,
                           uint32_t &count)
{
    int rc;
    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;

    const char * query = "select rac.* from run_attribute_catalog rac "
    "inner join type_catalog tc on rac.type_id = tc.id "
    "where (rac.txn_id = ? or rac.active = 1) "
    "and rac.type_id = ? "
    "and tc.run_id = ? ";
    // }

    rc = get_matching_attribute_count_catalog_all_run_attributes_with_type (args, count); assert (rc == RC_OK);

    if (count > 0) {
        attribute_list.reserve(count);


        rc = sqlite3_prepare_v2 (db, query, -1, &stmt, &tail); 
        if (rc != SQLITE_OK)
        {
            fprintf (stderr, "Error catalog_all_run_attributes_with_type_stub: Line: %d SQL error (%d): %s\n", __LINE__, rc, sqlite3_errmsg (db));
            sqlite3_close (db);
            goto cleanup;
        }
        rc = sqlite3_bind_int64 (stmt, 1, args.txn_id); assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int64 (stmt, 2, args.type_id); assert (rc == SQLITE_OK);        
        rc = sqlite3_bind_int64 (stmt, 3, args.run_id); assert (rc == SQLITE_OK);

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
                // case ATTR_DATA_TYPE_NULL : {
                //     break;
                // }        

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
                case ATTR_DATA_TYPE_TEXT : {
                    attribute.data = (char *)sqlite3_column_text (stmt, 5);
                    break;
                }
                case ATTR_DATA_TYPE_BLOB : {
        			int size_blob = sqlite3_column_bytes (stmt, 5);
        			
            		attribute.data.assign((char *)sqlite3_column_text (stmt, 5),(char *)sqlite3_column_text (stmt, 5) + size_blob);
                    // attribute.data = (char *)sqlite3_column_text (stmt, 5);
                    break;
                }
            }                        
            // attribute.data = (char *) sqlite3_column_text (stmt, 5);
            // cout << "attr.dims.at(2).min: " << attribute.dims.at(2).min << " attr.dims.at(2).min: " << attribute.dims.at(2).max << endl;

            rc = sqlite3_step (stmt);
            attribute_list.push_back(attribute);
        }

        rc = sqlite3_finalize (stmt);  
    }
        

cleanup:

    return rc;
}



static int get_matching_attribute_count_catalog_all_run_attributes_with_type (const md_catalog_all_run_attributes_with_type_args &args, uint32_t &count)
{
    int rc;
    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;
    const char * query = "select count (*) from run_attribute_catalog rac "
    "inner join type_catalog tc on rac.type_id = tc.id "
    "where (rac.txn_id = ? or rac.active = 1) "
    "and rac.type_id = ? "
    "and tc.run_id = ? ";

    rc = sqlite3_prepare_v2 (db, query, -1, &stmt, &tail); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 1, args.txn_id); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 2, args.type_id); assert (rc == SQLITE_OK);        
    rc = sqlite3_bind_int64 (stmt, 3, args.run_id); assert (rc == SQLITE_OK);
        
    rc = sqlite3_step (stmt); assert (rc == SQLITE_ROW || rc == SQLITE_DONE);
    count = sqlite3_column_int64 (stmt, 0);
    // cout << "in catalog all attrs, count: " << count << endl;
    rc = sqlite3_finalize (stmt); assert (rc == SQLITE_OK);

    return rc;
}

template <class T>
int md_catalog_all_run_attributes_with_type_range_stub (const md_catalog_all_run_attributes_with_type_range_args &args,
                           T min, T max,
                           std::vector<md_catalog_run_attribute_entry> &attribute_list,
                           uint32_t &count)
{
    int rc;
    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;

    string query = "select rac.* from run_attribute_catalog rac "
    "inner join type_catalog tc on rac.type_id = tc.id "
    "where (rac.txn_id = ? or rac.active = 1) "
    "and rac.type_id = ? "
    "and tc.run_id = ? "
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

    //	cout << "got here 3 \n";

    rc = get_matching_attribute_count_catalog_all_run_attributes_with_type_range_stub (args, min, max, query_str, count); assert (rc == RC_OK);

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
static int get_matching_attribute_count_catalog_all_run_attributes_with_type_range_stub (const md_catalog_all_run_attributes_with_type_range_args &args, 
	T min, T max, string query_str, uint32_t &count)
{
    int rc;
    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;
    string query = "select count (*) from run_attribute_catalog rac "
    "inner join type_catalog tc on rac.type_id = tc.id "
    "where (rac.txn_id = ? or rac.active = 1) "
    "and rac.type_id = ? "
    "and tc.run_id = ? "
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

	    	rc = sqlite3_bind_int64 (stmt, 6, min_int); assert (rc == SQLITE_OK);
	    	if(args.range_type == DATA_RANGE) {
	    		rc = sqlite3_bind_int64 (stmt, 7, max_int); assert (rc == SQLITE_OK);
	    		        //	cout << "got here 4.8 \n";

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
    count = sqlite3_column_int64 (stmt, 0);
 // cout << "in catalog all attrs, count: " << count << endl;
    rc = sqlite3_finalize (stmt); assert (rc == SQLITE_OK);

    return rc;
}


int md_catalog_all_timestep_attributes_stub (const md_catalog_all_timestep_attributes_args &args,
                           std::vector<md_catalog_timestep_attribute_entry> &attribute_list,
                           uint32_t &count)
{
    int rc;
    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;

    const char * query = "select tac.* from timestep_attribute_catalog tac "
    "inner join type_catalog tc on tac.type_id = tc.id "
    "where (tac.txn_id = ? or tac.active = 1) "
    "and tc.run_id = ? "
    "and tac.timestep_id = ? ";

    rc = get_matching_attribute_count_catalog_all_timestep_attributes (args, count); assert (rc == RC_OK);

    if (count > 0) {
        attribute_list.reserve(count);


        rc = sqlite3_prepare_v2 (db, query, -1, &stmt, &tail); 
        if (rc != SQLITE_OK)
        {
            fprintf (stderr, "Error catalog_all_timestep_attributes stub: Line: %d SQL error (%d): %s\n", __LINE__, rc, sqlite3_errmsg (db));
            sqlite3_close (db);
            goto cleanup;
        }
        rc = sqlite3_bind_int64 (stmt, 1, args.txn_id); assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int64 (stmt, 2, args.run_id); assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int64 (stmt, 3, args.timestep_id); assert (rc == SQLITE_OK);

        rc = sqlite3_step (stmt); assert (rc == SQLITE_ROW || rc == SQLITE_DONE);

        // cout << "starting \n";
        while (rc == SQLITE_ROW)
        {
            md_catalog_timestep_attribute_entry attribute;

            attribute.attribute_id = sqlite3_column_int64 (stmt, 0);
            attribute.timestep_id = sqlite3_column_int64 (stmt, 1);
            attribute.type_id = sqlite3_column_int64 (stmt, 2);
            attribute.active = sqlite3_column_int (stmt, 3);
            attribute.txn_id = sqlite3_column_int64 (stmt, 4);            
            attribute.data_type = (attr_data_type) sqlite3_column_int (stmt, 5);
            switch(attribute.data_type) {
                // case ATTR_DATA_TYPE_NULL : {
                //     break;
                // }        

                case ATTR_DATA_TYPE_INT : {
                    stringstream ss;
                    boost::archive::text_oarchive oa(ss);
                    oa << (uint64_t)sqlite3_column_int64 (stmt, 6);
                    attribute.data = ss.str();
                    break;
                }
                case ATTR_DATA_TYPE_REAL : {
                    stringstream ss;
                    boost::archive::text_oarchive oa(ss);                    
                    oa << (long double)sqlite3_column_double (stmt, 6);
                    attribute.data = ss.str();
                    break;
                }
                case ATTR_DATA_TYPE_TEXT : {
                    attribute.data = (char *)sqlite3_column_text (stmt, 6);
                    break;
                }
                case ATTR_DATA_TYPE_BLOB : {
        			int size_blob = sqlite3_column_bytes (stmt, 6);
        			
            		attribute.data.assign((char *)sqlite3_column_text (stmt, 6),(char *)sqlite3_column_text (stmt, 6) + size_blob);
                    // cout << "timestep attr in server data str length: " << attribute.data.size() << " str: " << attribute.data << endl;
                    // attribute.data = (char *)sqlite3_column_text (stmt, 6);
                    break;
                }
            }                        
            // attribute.data = (char *) sqlite3_column_text (stmt, 6);
            // cout << "attr.dims.at(2).min: " << attribute.dims.at(2).min << " attr.dims.at(2).min: " << attribute.dims.at(2).max << endl;

            rc = sqlite3_step (stmt);
            attribute_list.push_back(attribute);
        }

        rc = sqlite3_finalize (stmt);  
    }
        

cleanup:

    return rc;
}



static int get_matching_attribute_count_catalog_all_timestep_attributes (const md_catalog_all_timestep_attributes_args &args, uint32_t &count)
{
    int rc;
    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;
    const char * query = "select count (*) from timestep_attribute_catalog tac "
    "inner join type_catalog tc on tac.type_id = tc.id "
    "where (tac.txn_id = ? or tac.active = 1) "
    "and tc.run_id = ? "
    "and tac.timestep_id = ? ";

    rc = sqlite3_prepare_v2 (db, query, -1, &stmt, &tail); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 1, args.txn_id); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 2, args.run_id); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 3, args.timestep_id); assert (rc == SQLITE_OK);

    rc = sqlite3_step (stmt); assert (rc == SQLITE_ROW || rc == SQLITE_DONE);
    count = sqlite3_column_int64 (stmt, 0);
    // cout << "in catalog all attrs, count: " << count << endl;
    rc = sqlite3_finalize (stmt); assert (rc == SQLITE_OK);

    return rc;
}

int md_catalog_all_timestep_attributes_with_type_stub (const md_catalog_all_timestep_attributes_with_type_args &args,
                           std::vector<md_catalog_timestep_attribute_entry> &attribute_list,
                           uint32_t &count)
{
    int rc;
    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;

    const char * query = "select tac.* from timestep_attribute_catalog tac "
    "inner join type_catalog tc on tac.type_id = tc.id "
    "where (tac.txn_id = ? or tac.active = 1) "
    "and tc.run_id = ? "
    "and tac.timestep_id = ? "
    "and tac.type_id = ? ";

    rc = get_matching_attribute_count_catalog_all_timestep_attributes_with_type (args, count); assert (rc == RC_OK);

    if (count > 0) {
        attribute_list.reserve(count);


        rc = sqlite3_prepare_v2 (db, query, -1, &stmt, &tail); 
        if (rc != SQLITE_OK)
        {
            fprintf (stderr, "Error catalog_all_timestep_attributes_with_type stub: Line: %d SQL error (%d): %s\n", __LINE__, rc, sqlite3_errmsg (db));
            sqlite3_close (db);
            goto cleanup;
        }
        rc = sqlite3_bind_int64 (stmt, 1, args.txn_id); assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int64 (stmt, 2, args.run_id); assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int64 (stmt, 3, args.timestep_id); assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int64 (stmt, 4, args.type_id); assert (rc == SQLITE_OK);

        rc = sqlite3_step (stmt); assert (rc == SQLITE_ROW || rc == SQLITE_DONE);

        // cout << "starting \n";
        while (rc == SQLITE_ROW)
        {
            md_catalog_timestep_attribute_entry attribute;

            attribute.attribute_id = sqlite3_column_int64 (stmt, 0);
            attribute.timestep_id = sqlite3_column_int64 (stmt, 1);
            attribute.type_id = sqlite3_column_int64 (stmt, 2);
            attribute.active = sqlite3_column_int (stmt, 3);
            attribute.txn_id = sqlite3_column_int64 (stmt, 4);            
            attribute.data_type = (attr_data_type) sqlite3_column_int (stmt, 5);
            switch(attribute.data_type) {
                // case ATTR_DATA_TYPE_NULL : {
                //     break;
                // }        

                case ATTR_DATA_TYPE_INT : {
                    stringstream ss;
                    boost::archive::text_oarchive oa(ss);
                    oa << (uint64_t)sqlite3_column_int64 (stmt, 6);
                    attribute.data = ss.str();
                    break;
                }
                case ATTR_DATA_TYPE_REAL : {
                    stringstream ss;
                    boost::archive::text_oarchive oa(ss);                    
                    oa << (long double)sqlite3_column_double (stmt, 6);
                    attribute.data = ss.str();
                    break;
                }
                case ATTR_DATA_TYPE_TEXT : {
                    attribute.data = (char *)sqlite3_column_text (stmt, 6);
                    break;
                }
                case ATTR_DATA_TYPE_BLOB : {
        			int size_blob = sqlite3_column_bytes (stmt, 6);
        			
            		attribute.data.assign((char *)sqlite3_column_text (stmt, 6),(char *)sqlite3_column_text (stmt, 6) + size_blob);
                    // attribute.data = (char *)sqlite3_column_text (stmt, 6);
                    break;
                }
            }                        
            // attribute.data = (char *) sqlite3_column_text (stmt, 6);
            // cout << "attr.dims.at(2).min: " << attribute.dims.at(2).min << " attr.dims.at(2).min: " << attribute.dims.at(2).max << endl;

            rc = sqlite3_step (stmt);
            attribute_list.push_back(attribute);
        }

        rc = sqlite3_finalize (stmt);  
    }
        

cleanup:

    return rc;
}



static int get_matching_attribute_count_catalog_all_timestep_attributes_with_type (const md_catalog_all_timestep_attributes_with_type_args &args, uint32_t &count)
{
    int rc;
    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;
    const char * query = "select count (*) from timestep_attribute_catalog tac "
    "inner join type_catalog tc on tac.type_id = tc.id "
    "where (tac.txn_id = ? or tac.active = 1) "
    "and tc.run_id = ? "
    "and tac.timestep_id = ? "    
    "and tac.type_id = ? ";

    rc = sqlite3_prepare_v2 (db, query, -1, &stmt, &tail); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 1, args.txn_id); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 2, args.run_id); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 3, args.timestep_id); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 4, args.type_id); assert (rc == SQLITE_OK);

    rc = sqlite3_step (stmt); assert (rc == SQLITE_ROW || rc == SQLITE_DONE);
    count = sqlite3_column_int64 (stmt, 0);
    // cout << "in catalog all attrs, count: " << count << endl;
    rc = sqlite3_finalize (stmt); assert (rc == SQLITE_OK);

    return rc;
}

template <class T>
int md_catalog_all_timestep_attributes_with_type_range_stub (const md_catalog_all_timestep_attributes_with_type_range_args &args,
                           T min, T max,
                           std::vector<md_catalog_timestep_attribute_entry> &attribute_list,
                           uint32_t &count)
{
    int rc;
    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;

    string query = "select tac.* from timestep_attribute_catalog tac "
    "inner join type_catalog tc on tac.type_id = tc.id "
    "where (tac.txn_id = ? or tac.active = 1) "
    "and tc.run_id = ? "
    "and tac.timestep_id = ? "
    "and tac.type_id = ? "
    "and  (tac.data_type = ? or tac.data_type = ?) ";

    string query_str;

    switch(args.range_type) {
    	case DATA_RANGE : {
    		query_str = "and ( ? <= tac.data and tac.data <= ? ) ";
    		break;
    	}
       	case DATA_MAX : {
    		query_str = "and ( ? <= tac.data ) ";
    		break;
    	}
        case DATA_MIN : {
    		query_str = "and ( tac.data <= ? ) ";
    		break;
    	}
    }
	query += query_str;

    rc = get_matching_attribute_count_catalog_all_timestep_attributes_with_type_range (args, min, max, query_str, count); assert (rc == RC_OK);

    if (count > 0) {
        attribute_list.reserve(count);


        rc = sqlite3_prepare_v2 (db, query.c_str(), -1, &stmt, &tail); 
        if (rc != SQLITE_OK)
        {
            fprintf (stderr, "Error catalog_all_timestep_attributes_with_type_range stub: Line: %d SQL error (%d): %s\n", __LINE__, rc, sqlite3_errmsg (db));
            sqlite3_close (db);
            goto cleanup;
        }
        rc = sqlite3_bind_int64 (stmt, 1, args.txn_id); assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int64 (stmt, 2, args.run_id); assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int64 (stmt, 3, args.timestep_id); assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int64 (stmt, 4, args.type_id); assert (rc == SQLITE_OK);

	    rc = sqlite3_bind_int (stmt, 5, ATTR_DATA_TYPE_INT); assert (rc == SQLITE_OK);
	    rc = sqlite3_bind_int (stmt, 6, ATTR_DATA_TYPE_REAL); assert (rc == SQLITE_OK);

		switch(args.data_type) {
	        case ATTR_DATA_TYPE_INT : {
	        	rc = sqlite3_bind_int64 (stmt, 7, min_int); assert (rc == SQLITE_OK);
				if(args.range_type == DATA_RANGE) {
	        		rc = sqlite3_bind_int64 (stmt, 8, max_int); assert (rc == SQLITE_OK);
	    		}
	            break;
	        }
	        case ATTR_DATA_TYPE_REAL : {
	        	rc = sqlite3_bind_double (stmt, 7, min_real); assert (rc == SQLITE_OK);
	        	if(args.range_type == DATA_RANGE) {
	        		rc = sqlite3_bind_double (stmt, 8, max_real); assert (rc == SQLITE_OK);
	    		}
	            break;
	        }
	    }

        rc = sqlite3_step (stmt); assert (rc == SQLITE_ROW || rc == SQLITE_DONE);

        // cout << "starting \n";
        while (rc == SQLITE_ROW)
        {
            md_catalog_timestep_attribute_entry attribute;

            attribute.attribute_id = sqlite3_column_int64 (stmt, 0);
            attribute.timestep_id = sqlite3_column_int64 (stmt, 1);
            attribute.type_id = sqlite3_column_int64 (stmt, 2);
            attribute.active = sqlite3_column_int (stmt, 3);
            attribute.txn_id = sqlite3_column_int64 (stmt, 4);            
            attribute.data_type = (attr_data_type) sqlite3_column_int (stmt, 5);
            switch(attribute.data_type) {       
                case ATTR_DATA_TYPE_INT : {
                    stringstream ss;
                    boost::archive::text_oarchive oa(ss);
                    oa << (uint64_t)sqlite3_column_int64 (stmt, 6);
                    attribute.data = ss.str();
                    break;
                }
                case ATTR_DATA_TYPE_REAL : {
                    stringstream ss;
                    boost::archive::text_oarchive oa(ss);                    
                    oa << (long double)sqlite3_column_double (stmt, 6);
                    attribute.data = ss.str();
                    break;
                }
            }                        
            // attribute.data = (char *) sqlite3_column_text (stmt, 6);
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
static int get_matching_attribute_count_catalog_all_timestep_attributes_with_type_range (const md_catalog_all_timestep_attributes_with_type_range_args &args, 
	T min, T max, string query_str, uint32_t &count)
{
    int rc;
    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;
    string query = "select count (*) from timestep_attribute_catalog tac "
    "inner join type_catalog tc on tac.type_id = tc.id "
    "where (tac.txn_id = ? or tac.active = 1) "
    "and tc.run_id = ? "
    "and tac.timestep_id = ? "    
    "and tac.type_id = ? "
    "and  (tac.data_type = ? or tac.data_type = ?) " + query_str;

    rc = sqlite3_prepare_v2 (db, query.c_str(), -1, &stmt, &tail); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 1, args.txn_id); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 2, args.run_id); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 3, args.timestep_id); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 4, args.type_id); assert (rc == SQLITE_OK);

    rc = sqlite3_bind_int (stmt, 5, ATTR_DATA_TYPE_INT); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int (stmt, 6, ATTR_DATA_TYPE_REAL); assert (rc == SQLITE_OK);


	switch(args.data_type) {
	    case ATTR_DATA_TYPE_INT : {
	    	rc = sqlite3_bind_int64 (stmt, 7, min_int); assert (rc == SQLITE_OK);
			if(args.range_type == DATA_RANGE) {
	    		rc = sqlite3_bind_int64 (stmt, 8, max_int); assert (rc == SQLITE_OK);
			}
	        break;
	    }
	    case ATTR_DATA_TYPE_REAL : {
	    	rc = sqlite3_bind_double (stmt, 7, min_real); assert (rc == SQLITE_OK);
	    	if(args.range_type == DATA_RANGE) {
	    		rc = sqlite3_bind_double (stmt, 8, max_real); assert (rc == SQLITE_OK);
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

int md_catalog_all_timesteps_with_var_attributes_with_type_var_dims_stub (const md_catalog_all_timesteps_with_var_attributes_with_type_var_dims_args &args,
                           std::vector<md_catalog_timestep_entry> &entries,
                           uint32_t &count)
{
    int rc;
    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;

    string query;
    if (args.num_dims == 1) {
    	query = "select tmc.* from timestep_catalog tmc "
	    "inner join type_catalog tc on tc.run_id = tmc.run_id "
	    "inner join var_attribute_catalog vac on vac.timestep_id = tmc.id and vac.type_id = tc.id "
	    "where (vac.txn_id = ? or vac.active = 1) "
	    "and (tmc.txn_id = ? or tmc.active = 1) "
	    "and vac.type_id = ? "
	    "and vac.var_id = ? "
	    "and tmc.run_id = ? "
	    "and ? <= vac.d0_max and ? >= vac.d0_min "
	    "group by tmc.id, tmc.run_id";
    }
    else if(args.num_dims == 2) {
    	query = "select tmc.* from timestep_catalog tmc "
	    "inner join type_catalog tc on tc.run_id = tmc.run_id "
	    "inner join var_attribute_catalog vac on vac.timestep_id = tmc.id and vac.type_id = tc.id "
	    "where (vac.txn_id = ? or vac.active = 1) "
	    "and (tmc.txn_id = ? or tmc.active = 1) "
	    "and vac.type_id = ? "
	    "and vac.var_id = ? "
	    "and tmc.run_id = ? "
	    "and ? <= vac.d0_max and ? >= vac.d0_min "
	    "and ? <= vac.d1_max and ? >= vac.d1_min "
	    "group by tmc.id, tmc.run_id";
    }
    else if(args.num_dims == 3) {
    	query = "select tmc.* from timestep_catalog tmc "
	    "inner join type_catalog tc on tc.run_id = tmc.run_id "
	    "inner join var_attribute_catalog vac on vac.timestep_id = tmc.id and vac.type_id = tc.id "
	    "where (vac.txn_id = ? or vac.active = 1) "
	    "and (tmc.txn_id = ? or tmc.active = 1) "
	    "and vac.type_id = ? "
	    "and vac.var_id = ? "
	    "and tmc.run_id = ? "
	    "and ? <= vac.d0_max and ? >= vac.d0_min "
	    "and ? <= vac.d1_max and ? >= vac.d1_min "
    	"and ? <= vac.d2_max and ? >= vac.d2_min "; 
	    "group by tmc.id, tmc.run_id";    	
    }
    else {
    	count = 0;
    	return rc;
    }

    rc = get_matching_timestep_count (args, count); assert (rc == RC_OK);

    if (count > 0) {
        entries.reserve(count);

        // rc = sqlite3_prepare_v2 (db, query.c_str())(str, -1, &stmt, &tail); 
        // if (rc != SQLITE_OK)
        // {
        //     fprintf (stderr, "Error catalog_all_timesteps_with_var_attributes_with_type_var_dims stub: Line: %d SQL error (%d): %s\n", __LINE__, rc, sqlite3_errmsg (db));
        //     sqlite3_close (db);
        //     goto cleanup;
        // }
        rc = sqlite3_prepare_v2 (db, query.c_str(), -1, &stmt, &tail); assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int64 (stmt, 1, args.txn_id); assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int64 (stmt, 2, args.txn_id); assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int64 (stmt, 3, args.type_id); assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int64 (stmt, 4, args.var_id); assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int64 (stmt, 5, args.run_id); assert (rc == SQLITE_OK);

        for (int j = 0; j < args.num_dims; j++)
        {
            rc = sqlite3_bind_double (stmt, 6 + (j * 2), args.dims.at(j).min);
            rc = sqlite3_bind_double (stmt, 7 + (j * 2), args.dims.at(j).max);
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



static int get_matching_timestep_count_catalog_all_timesteps_with_var_attributes_with_type_var_dims (const md_catalog_all_timesteps_with_var_attributes_with_type_var_dims_args &args, uint32_t &count)
{
    int rc;
    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;

    string query;

    if(args.num_dims == 1) {
	  	query = "select count(*) from (select distinct tmc.id, tmc.run_id from timestep_catalog tmc "
	    "inner join type_catalog tc on tc.run_id = tmc.run_id "
	    "inner join var_attribute_catalog vac on vac.timestep_id = tmc.id and vac.type_id = tc.id "
	    "where (vac.txn_id = ? or vac.active = 1) "
	    "and (tmc.txn_id = ? or tmc.active = 1) "
	    "and vac.type_id = ? "
	    "and vac.var_id = ? "
	    "and tmc.run_id = ? "
	    "and ? <= vac.d0_max and ? >= vac.d0_min ) as internalQuery"; 
	}
	else if(args.num_dims == 2) {   
	  	query = "select count(*) from (select distinct tmc.id, tmc.run_id from timestep_catalog tmc "
	    "inner join type_catalog tc on tc.run_id = tmc.run_id "
	    "inner join var_attribute_catalog vac on vac.timestep_id = tmc.id and vac.type_id = tc.id "
	    "where (vac.txn_id = ? or vac.active = 1) "
	    "and (tmc.txn_id = ? or tmc.active = 1) "
	    "and vac.type_id = ? "
	    "and vac.var_id = ? "
	    "and tmc.run_id = ? "
	    "and ? <= vac.d0_max and ? >= vac.d0_min "
	    "and ? <= vac.d1_max and ? >= vac.d1_min ) as internalQuery"; 
	}
	else if(args.num_dims == 3) {   
	  	query = "select count(*) from (select distinct tmc.id, tmc.run_id from timestep_catalog tmc "
	    "inner join type_catalog tc on tc.run_id = tmc.run_id "
	    "inner join var_attribute_catalog vac on vac.timestep_id = tmc.id and vac.type_id = tc.id "
	    "where (vac.txn_id = ? or vac.active = 1) "
	    "and (tmc.txn_id = ? or tmc.active = 1) "
	    "and vac.type_id = ? "
	    "and vac.var_id = ? "
	    "and tmc.run_id = ? "
	    "and ? <= vac.d0_max and ? >= vac.d0_min "
	    "and ? <= vac.d1_max and ? >= vac.d1_min "
	    "and ? <= vac.d2_max and ? >= vac.d2_min ) as internalQuery"; 
	}

    rc = sqlite3_prepare_v2 (db, query.c_str(), -1, &stmt, &tail); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 1, args.txn_id); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 2, args.txn_id); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 3, args.type_id); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 4, args.var_id); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 5, args.run_id); assert (rc == SQLITE_OK);

    for (int j = 0; j < args.num_dims; j++)
    {
        rc = sqlite3_bind_double (stmt, 6 + (j * 2), args.dims.at(j).min);
        rc = sqlite3_bind_double (stmt, 7 + (j * 2), args.dims.at(j).max);
    }

    rc = sqlite3_step (stmt); assert (rc == SQLITE_ROW || rc == SQLITE_DONE);
    count = sqlite3_column_int64 (stmt, 0);
    rc = sqlite3_finalize (stmt); assert (rc == SQLITE_OK);
//printf ("matching attr count: %d\n", *count);

    return rc;
}

template <class T>
int md_catalog_all_timesteps_with_var_attributes_with_type_var_dims_range_stub (const md_catalog_all_timesteps_with_var_attributes_with_type_var_dims_range_args &args,
                           T min, T max,
                           std::vector<md_catalog_timestep_entry> &entries,
                           uint32_t &count)
{
    int rc;
    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;
    string query;

    if(args.num_dims == 1) {
	    query = "select tmc.* from timestep_catalog tmc "
	    "inner join type_catalog tc on tc.run_id = tmc.run_id "
	    "inner join var_attribute_catalog vac on vac.timestep_id = tmc.id and vac.type_id = tc.id "
	    "where (vac.txn_id = ? or vac.active = 1) "
	    "and (tmc.txn_id = ? or tmc.active = 1) "
	    "and vac.type_id = ? "
	    "and vac.var_id = ? "
	    "and tmc.run_id = ? "
	    "and ? <= vac.d0_max and ? >= vac.d0_min "
	    "and (vac.data_type = ? or vac.data_type = ?) ";
	}
	else if(args.num_dims == 2) {
		query = "select tmc.* from timestep_catalog tmc "
	    "inner join type_catalog tc on tc.run_id = tmc.run_id "
	    "inner join var_attribute_catalog vac on vac.timestep_id = tmc.id and vac.type_id = tc.id "
	    "where (vac.txn_id = ? or vac.active = 1) "
	    "and (tmc.txn_id = ? or tmc.active = 1) "
	    "and vac.type_id = ? "
	    "and vac.var_id = ? "
	    "and tmc.run_id = ? "
	    "and ? <= vac.d0_max and ? >= vac.d0_min "
	    "and ? <= vac.d1_max and ? >= vac.d1_min "
	    "and (vac.data_type = ? or vac.data_type = ?) ";
	}
	else if(args.num_dims == 3) {
		query = "select tmc.* from timestep_catalog tmc "
	    "inner join type_catalog tc on tc.run_id = tmc.run_id "
	    "inner join var_attribute_catalog vac on vac.timestep_id = tmc.id and vac.type_id = tc.id "
	    "where (vac.txn_id = ? or vac.active = 1) "
	    "and (tmc.txn_id = ? or tmc.active = 1) "
	    "and vac.type_id = ? "
	    "and vac.var_id = ? "
	    "and tmc.run_id = ? "
	    "and ? <= vac.d0_max and ? >= vac.d0_min "
	    "and ? <= vac.d1_max and ? >= vac.d1_min "
	    "and ? <= vac.d2_max and ? >= vac.d2_min "
	    "and (vac.data_type = ? or vac.data_type = ?) ";
	}
	else {
		count = 0;
		return rc;
	}	

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


    rc = get_matching_timestep_count (args, min, max, query_str, count); assert (rc == RC_OK);

    if (count > 0) {
        entries.reserve(count);

        rc = sqlite3_prepare_v2 (db, query.c_str(), -1, &stmt, &tail); 
        if (rc != SQLITE_OK)
        {
            fprintf (stderr, "Error catalog_all_timesteps_with_var_attributes_with_type_var_dims_range stub: Line: %d SQL error (%d): %s\n", __LINE__, rc, sqlite3_errmsg (db));
            sqlite3_close (db);
            goto cleanup;
        }
        // rc = sqlite3_prepare_v2 (db, query.c_str(), -1, &stmt, &tail); assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int64 (stmt, 1, args.txn_id); assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int64 (stmt, 2, args.txn_id); assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int64 (stmt, 3, args.type_id); assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int64 (stmt, 4, args.var_id); assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int64 (stmt, 5, args.run_id); assert (rc == SQLITE_OK);

        for (int j = 0; j < args.num_dims; j++)
        {
            rc = sqlite3_bind_double (stmt, 6 + (j * 2), args.dims.at(j).min);
            rc = sqlite3_bind_double (stmt, 7 + (j * 2), args.dims.at(j).max);
        }

	    rc = sqlite3_bind_int (stmt, 12, ATTR_DATA_TYPE_INT); assert (rc == SQLITE_OK);
	    rc = sqlite3_bind_int (stmt, 13, ATTR_DATA_TYPE_REAL); assert (rc == SQLITE_OK);

		switch(args.data_type) {
	        case ATTR_DATA_TYPE_INT : {
	        	rc = sqlite3_bind_int64 (stmt, 14, min_int); assert (rc == SQLITE_OK);
	        	if(args.range_type == DATA_RANGE) {
	        		rc = sqlite3_bind_int64 (stmt, 15, max_int); assert (rc == SQLITE_OK);
	        	}
	            break;
	        }
	        case ATTR_DATA_TYPE_REAL : {
	        	rc = sqlite3_bind_double (stmt, 14, min_real); assert (rc == SQLITE_OK);
	        	if(args.range_type == DATA_RANGE) {
	        		rc = sqlite3_bind_double (stmt, 15, max_real); assert (rc == SQLITE_OK);
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
static int get_matching_timestep_count_catalog_all_timesteps_with_var_attributes_with_type_var_dims_range (const md_catalog_all_timesteps_with_var_attributes_with_type_var_dims_range_args &args, 
	T min, T max, string query_str, uint32_t &count)
{
    int rc;
    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;
    string query;

    if(args.num_dims == 1) {
	    query = "select count(*) from (select distinct tmc.id, tmc.run_id from timestep_catalog tmc "
	    "inner join type_catalog tc on tc.run_id = tmc.run_id "
	    "inner join var_attribute_catalog vac on vac.timestep_id = tmc.id and vac.type_id = tc.id "
	    "where (vac.txn_id = ? or vac.active = 1) "
	    "and (tmc.txn_id = ? or tmc.active = 1) "
	    "and vac.type_id = ? "
	    "and vac.var_id = ? "
	    "and tmc.run_id = ? "
	    "and ? <= vac.d0_max and ? >= vac.d0_min "
	    "and (vac.data_type = ? or vac.data_type = ?) " + query_str + " ) as internalQuery"; 
	}
	else if(args.num_dims == 2) {
    	query = "select count(*) from (select distinct tmc.id, tmc.run_id from timestep_catalog tmc "
	    "inner join type_catalog tc on tc.run_id = tmc.run_id "
	    "inner join var_attribute_catalog vac on vac.timestep_id = tmc.id and vac.type_id = tc.id "
	    "where (vac.txn_id = ? or vac.active = 1) "
	    "and (tmc.txn_id = ? or tmc.active = 1) "
	    "and vac.type_id = ? "
	    "and vac.var_id = ? "
	    "and tmc.run_id = ? "
	    "and ? <= vac.d0_max and ? >= vac.d0_min "
	    "and ? <= vac.d1_max and ? >= vac.d1_min "
	    "and (vac.data_type = ? or vac.data_type = ?) " + query_str + " ) as internalQuery"; 
	}
	else if(args.num_dims == 3) {
    	query = "select count(*) from (select distinct tmc.id, tmc.run_id from timestep_catalog tmc "
	    "inner join type_catalog tc on tc.run_id = tmc.run_id "
	    "inner join var_attribute_catalog vac on vac.timestep_id = tmc.id and vac.type_id = tc.id "
	    "where (vac.txn_id = ? or vac.active = 1) "
	    "and (tmc.txn_id = ? or tmc.active = 1) "
	    "and vac.type_id = ? "
	    "and vac.var_id = ? "
	    "and tmc.run_id = ? "
	    "and ? <= vac.d0_max and ? >= vac.d0_min "
	    "and ? <= vac.d1_max and ? >= vac.d1_min "
	    "and ? <= vac.d2_max and ? >= vac.d2_min "
	    "and (vac.data_type = ? or vac.data_type = ?) " + query_str + " ) as internalQuery"; 
	}

    rc = sqlite3_prepare_v2 (db, query.c_str(), -1, &stmt, &tail); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 1, args.txn_id); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 2, args.txn_id); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 3, args.type_id); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 4, args.var_id); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 5, args.run_id); assert (rc == SQLITE_OK);

    for (int j = 0; j < args.num_dims; j++)
    {
        rc = sqlite3_bind_double (stmt, 6 + (j * 2), args.dims.at(j).min);
        rc = sqlite3_bind_double (stmt, 7 + (j * 2), args.dims.at(j).max);
    }

    rc = sqlite3_bind_int (stmt, 12, ATTR_DATA_TYPE_INT); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int (stmt, 13, ATTR_DATA_TYPE_REAL); assert (rc == SQLITE_OK);

	switch(args.data_type) {
        case ATTR_DATA_TYPE_INT : {
        	rc = sqlite3_bind_int64 (stmt, 14, min_int); assert (rc == SQLITE_OK);
        	if(args.range_type == DATA_RANGE) {
        		rc = sqlite3_bind_int64 (stmt, 15, max_int); assert (rc == SQLITE_OK);
        	}
            break;
        }
        case ATTR_DATA_TYPE_REAL : {
        	rc = sqlite3_bind_double (stmt, 14, min_real); assert (rc == SQLITE_OK);
    	    if(args.range_type == DATA_RANGE) {
        		rc = sqlite3_bind_double (stmt, 15, max_real); assert (rc == SQLITE_OK);
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

int md_catalog_all_timesteps_with_var_attributes_with_type_var_stub (const md_catalog_all_timesteps_with_var_attributes_with_type_var_args &args,
                           std::vector<md_catalog_timestep_entry> &entries,
                           uint32_t &count)
{
    int rc;
    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;
    const char * query = "select tmc.* from timestep_catalog tmc "
    "inner join type_catalog tc on tc.run_id = tmc.run_id "
    "inner join var_attribute_catalog vac on vac.timestep_id = tmc.id and vac.type_id = tc.id "
    "where (vac.txn_id = ? or vac.active = 1) "
    "and (tmc.txn_id = ? or tmc.active = 1) "
    "and vac.type_id = ? "
    "and vac.var_id = ? "
    "and tmc.run_id = ? "
    "group by tmc.id, tmc.run_id";

    rc = get_matching_timestep_count_catalog_all_timesteps_with_var_attributes_with_type_var (args, count); assert (rc == RC_OK);

    if (count > 0) {
        entries.reserve(count);

        rc = sqlite3_prepare_v2 (db, query, -1, &stmt, &tail); 
        if (rc != SQLITE_OK)
        {
            fprintf (stderr, "Error catalog_all_timesteps_with_var_attributes_with_type_var stub: Line: %d SQL error (%d): %s\n", __LINE__, rc, sqlite3_errmsg (db));
            sqlite3_close (db);
            goto cleanup;
        }
        // rc = sqlite3_prepare_v2 (db, query, -1, &stmt, &tail); assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int64 (stmt, 1, args.txn_id); assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int64 (stmt, 2, args.txn_id); assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int64 (stmt, 3, args.type_id); assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int64 (stmt, 4, args.var_id); assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int64 (stmt, 5, args.run_id); assert (rc == SQLITE_OK);

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



static int get_matching_timestep_count_catalog_all_timesteps_with_var_attributes_with_type_var (const md_catalog_all_timesteps_with_var_attributes_with_type_var_args &args, uint32_t &count)
{
    int rc;
    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;
    const char * query = "select count(*) from (select distinct tmc.id, tmc.run_id from timestep_catalog tmc "
    "inner join type_catalog tc on tc.run_id = tmc.run_id "
    "inner join var_attribute_catalog vac on vac.timestep_id = tmc.id and vac.type_id = tc.id "
    "where (vac.txn_id = ? or vac.active = 1) "
    "and (tmc.txn_id = ? or tmc.active = 1) "
    "and vac.type_id = ? "
    "and vac.var_id = ? "
    "and tmc.run_id = ? ) as internalQuery";

    rc = sqlite3_prepare_v2 (db, query, -1, &stmt, &tail); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 1, args.txn_id); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 2, args.txn_id); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 3, args.type_id); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 4, args.var_id); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 5, args.run_id); assert (rc == SQLITE_OK);

    rc = sqlite3_step (stmt); assert (rc == SQLITE_ROW || rc == SQLITE_DONE);
    count = sqlite3_column_int64 (stmt, 0);
    rc = sqlite3_finalize (stmt); assert (rc == SQLITE_OK);

    // cout << "count: " << count << endl;

    return rc;
}

template <class T>
int md_catalog_all_timesteps_with_var_attributes_with_type_var_range_stub (const md_catalog_all_timesteps_with_var_attributes_with_type_var_range_args &args,
                           T min, T max,
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


    rc = get_matching_timestep_count_catalog_all_timesteps_with_var_attributes_with_type_var_range (args, min, max, query_str, count); assert (rc == RC_OK);

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
	        	rc = sqlite3_bind_int64 (stmt, 8, (uint64_t)min); assert (rc == SQLITE_OK);
	        	if(args.range_type == DATA_RANGE) {
	        		rc = sqlite3_bind_int64 (stmt, (uint64_t)9, max); assert (rc == SQLITE_OK);
	        	}
	            break;
	        }
	        case ATTR_DATA_TYPE_REAL : {
	        	rc = sqlite3_bind_double (stmt, (long double)8, min); assert (rc == SQLITE_OK);
	    	    if(args.range_type == DATA_RANGE) {
	        		rc = sqlite3_bind_double (stmt, (long double)9, max); assert (rc == SQLITE_OK);
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
static int get_matching_timestep_count_catalog_all_timesteps_with_var_attributes_with_type_var_range (const md_catalog_all_timesteps_with_var_attributes_with_type_var_range_args &args, 
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
        		rc = sqlite3_bind_int64 (stmt, (uint64_t)9, max); assert (rc == SQLITE_OK);
        	}
            break;
        }
        case ATTR_DATA_TYPE_REAL : {
        	rc = sqlite3_bind_double (stmt, (long double)8, min); assert (rc == SQLITE_OK);
    	    if(args.range_type == DATA_RANGE) {
        		rc = sqlite3_bind_double (stmt, (long double)9, max); assert (rc == SQLITE_OK);
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

int md_catalog_all_timesteps_with_var_attributes_with_type_var_substr_dims_stub (const md_catalog_all_timesteps_with_var_attributes_with_type_var_substr_dims_args &args,
                           std::vector<md_catalog_timestep_entry> &entries,
                           uint32_t &count)
{
    int rc;
    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;
    string query;

    if(args.num_dims == 1) {
    	query = "select tmc.* from timestep_catalog tmc "
	    "inner join type_catalog tc on tc.run_id = tmc.run_id "
	    "inner join var_attribute_catalog vac on vac.timestep_id = tmc.id and vac.type_id = tc.id "
	    "inner join var_catalog vc on vac.timestep_id = vc.timestep_id and vac.var_id = vc.id "
	    "where (vac.txn_id = ? or vac.active = 1) "
	    "and (tmc.txn_id = ? or tmc.active = 1) "
	    "and vac.type_id = ? "
	    "and vc.name like ? "
	    "and tmc.run_id = ? "
	    "and ? <= vac.d0_max and ? >= vac.d0_min "
	    "group by tmc.id, tmc.run_id";
	}
	else if(args.num_dims == 2) {
    	query = "select tmc.* from timestep_catalog tmc "
	    "inner join type_catalog tc on tc.run_id = tmc.run_id "
	    "inner join var_attribute_catalog vac on vac.timestep_id = tmc.id and vac.type_id = tc.id "
	    "inner join var_catalog vc on vac.timestep_id = vc.timestep_id and vac.var_id = vc.id "
	    "where (vac.txn_id = ? or vac.active = 1) "
	    "and (tmc.txn_id = ? or tmc.active = 1) "
	    "and vac.type_id = ? "
	    "and vc.name like ? "
	    "and tmc.run_id = ? "
	    "and ? <= vac.d0_max and ? >= vac.d0_min "
	    "and ? <= vac.d1_max and ? >= vac.d1_min "
	    "group by tmc.id, tmc.run_id";
	}
	else if(args.num_dims == 3) {
    	query = "select tmc.* from timestep_catalog tmc "
	    "inner join type_catalog tc on tc.run_id = tmc.run_id "
	    "inner join var_attribute_catalog vac on vac.timestep_id = tmc.id and vac.type_id = tc.id "
	    "inner join var_catalog vc on vac.timestep_id = vc.timestep_id and vac.var_id = vc.id "
	    "where (vac.txn_id = ? or vac.active = 1) "
	    "and (tmc.txn_id = ? or tmc.active = 1) "
	    "and vac.type_id = ? "
	    "and vc.name like ? "
	    "and tmc.run_id = ? "
	    "and ? <= vac.d0_max and ? >= vac.d0_min "
	    "and ? <= vac.d1_max and ? >= vac.d1_min "
	    "and ? <= vac.d2_max and ? >= vac.d2_min "
	    "group by tmc.id, tmc.run_id";
	}
	else {
		count = 0;
		return rc;
	}	

    rc = get_matching_timestep_count_catalog_all_timesteps_with_var_attributes_with_type_var_substr_dims (args, count); assert (rc == RC_OK);

    if (count > 0) {
        entries.reserve(count);

        rc = sqlite3_prepare_v2 (db, query.c_str(), -1, &stmt, &tail); 
        if (rc != SQLITE_OK)
        {
            fprintf (stderr, "Error catalog_all_timesteps_with_var_attributes_with_type_var_substr_dims stub: Line: %d SQL error (%d): %s\n", __LINE__, rc, sqlite3_errmsg (db));
            sqlite3_close (db);
            goto cleanup;
        }
        // rc = sqlite3_prepare_v2 (db, query.c_str(), -1, &stmt, &tail); assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int64 (stmt, 1, args.txn_id); assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int64 (stmt, 2, args.txn_id); assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int64 (stmt, 3, args.type_id); assert (rc == SQLITE_OK);
    	rc = sqlite3_bind_text (stmt, 4, strdup(args.var_name_substr.c_str()), -1, free); assert (rc == SQLITE_OK);
    	// cout << "var_name_substr: " << args.var_name_substr << endl;
        rc = sqlite3_bind_int64 (stmt, 5, args.run_id); assert (rc == SQLITE_OK);

        for (int j = 0; j < args.num_dims; j++)
        {
            rc = sqlite3_bind_double (stmt, 6 + (j * 2), args.dims.at(j).min);
            rc = sqlite3_bind_double (stmt, 7 + (j * 2), args.dims.at(j).max);
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



static int get_matching_timestep_count_catalog_all_timesteps_with_var_attributes_with_type_var_substr_dims (const md_catalog_all_timesteps_with_var_attributes_with_type_var_substr_dims_args &args, uint32_t &count)
{
    int rc;
    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;
    string query;

    if(args.num_dims == 1) {
		query = "select count(*) from (select distinct tmc.id, tmc.run_id from timestep_catalog tmc "
	    "inner join type_catalog tc on tc.run_id = tmc.run_id "
	    "inner join var_attribute_catalog vac on vac.timestep_id = tmc.id and vac.type_id = tc.id "
	    "inner join var_catalog vc on vac.timestep_id = vc.timestep_id and vac.var_id = vc.id "
	    "where (vac.txn_id = ? or vac.active = 1) "
	    "and (tmc.txn_id = ? or tmc.active = 1) "
	    "and vac.type_id = ? "
	    "and vc.name like ? "
	    "and tmc.run_id = ? "
	    "and ? <= vac.d0_max and ? >= vac.d0_min ) as internalQuery"; 
	}
	else if(args.num_dims == 2) {
		query = "select count(*) from (select distinct tmc.id, tmc.run_id from timestep_catalog tmc "
	    "inner join type_catalog tc on tc.run_id = tmc.run_id "
	    "inner join var_attribute_catalog vac on vac.timestep_id = tmc.id and vac.type_id = tc.id "
	    "inner join var_catalog vc on vac.timestep_id = vc.timestep_id and vac.var_id = vc.id "
	    "where (vac.txn_id = ? or vac.active = 1) "
	    "and (tmc.txn_id = ? or tmc.active = 1) "
	    "and vac.type_id = ? "
	    "and vc.name like ? "
	    "and tmc.run_id = ? "
	    "and ? <= vac.d0_max and ? >= vac.d0_min "
	    "and ? <= vac.d1_max and ? >= vac.d1_min ) as internalQuery"; 
	}
	else if(args.num_dims == 3) {
		query = "select count(*) from (select distinct tmc.id, tmc.run_id from timestep_catalog tmc "
	    "inner join type_catalog tc on tc.run_id = tmc.run_id "
	    "inner join var_attribute_catalog vac on vac.timestep_id = tmc.id and vac.type_id = tc.id "
	    "inner join var_catalog vc on vac.timestep_id = vc.timestep_id and vac.var_id = vc.id "
	    "where (vac.txn_id = ? or vac.active = 1) "
	    "and (tmc.txn_id = ? or tmc.active = 1) "
	    "and vac.type_id = ? "
	    "and vc.name like ? "
	    "and tmc.run_id = ? "
	    "and ? <= vac.d0_max and ? >= vac.d0_min "
	    "and ? <= vac.d1_max and ? >= vac.d1_min "
	    "and ? <= vac.d2_max and ? >= vac.d2_min ) as internalQuery"; 
	}
    

    rc = sqlite3_prepare_v2 (db, query.c_str(), -1, &stmt, &tail); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 1, args.txn_id); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 2, args.txn_id); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 3, args.type_id); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_text (stmt, 4, strdup(args.var_name_substr.c_str()), -1, free); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 5, args.run_id); assert (rc == SQLITE_OK);

    for (int j = 0; j < args.num_dims; j++)
    {
        rc = sqlite3_bind_double (stmt, 6 + (j * 2), args.dims.at(j).min);
        rc = sqlite3_bind_double (stmt, 7 + (j * 2), args.dims.at(j).max);
    }

    rc = sqlite3_step (stmt); assert (rc == SQLITE_ROW || rc == SQLITE_DONE);
    count = sqlite3_column_int64 (stmt, 0);
    rc = sqlite3_finalize (stmt); assert (rc == SQLITE_OK);
//printf ("matching attr count: %d\n", *count);

    return rc;
}

template <class T>
int md_catalog_all_timesteps_with_var_attributes_with_type_var_substr_dims_range_stub (const md_catalog_all_timesteps_with_var_attributes_with_type_var_substr_dims_range_args &args,
                           T min, T max,
                           std::vector<md_catalog_timestep_entry> &entries,
                           uint32_t &count)
{
    int rc;
    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;

    string query_str;
    string query;

    if(args.num_dims == 1) {
    	query = "select tmc.* from timestep_catalog tmc "
	    "inner join type_catalog tc on tc.run_id = tmc.run_id "
	    "inner join var_attribute_catalog vac on vac.timestep_id = tmc.id and vac.type_id = tc.id "
	    "inner join var_catalog vc on vac.timestep_id = vc.timestep_id and vac.var_id = vc.id "
	    "where (vac.txn_id = ? or vac.active = 1) "
	    "and (tmc.txn_id = ? or tmc.active = 1) "
	    "and vac.type_id = ? "
	    "and vc.name like ? "
	    "and tmc.run_id = ? "
	    "and ? <= vac.d0_max and ? >= vac.d0_min "
    	"and  (vac.data_type = ? or vac.data_type = ?) ";
	}
	else if(args.num_dims == 2) {
    	query = "select tmc.* from timestep_catalog tmc "
	    "inner join type_catalog tc on tc.run_id = tmc.run_id "
	    "inner join var_attribute_catalog vac on vac.timestep_id = tmc.id and vac.type_id = tc.id "
	    "inner join var_catalog vc on vac.timestep_id = vc.timestep_id and vac.var_id = vc.id "
	    "where (vac.txn_id = ? or vac.active = 1) "
	    "and (tmc.txn_id = ? or tmc.active = 1) "
	    "and vac.type_id = ? "
	    "and vc.name like ? "
	    "and tmc.run_id = ? "
	    "and ? <= vac.d0_max and ? >= vac.d0_min "
	    "and ? <= vac.d1_max and ? >= vac.d1_min "
    	"and  (vac.data_type = ? or vac.data_type = ?) ";
	}
	else if(args.num_dims == 3) {
    	query = "select tmc.* from timestep_catalog tmc "
	    "inner join type_catalog tc on tc.run_id = tmc.run_id "
	    "inner join var_attribute_catalog vac on vac.timestep_id = tmc.id and vac.type_id = tc.id "
	    "inner join var_catalog vc on vac.timestep_id = vc.timestep_id and vac.var_id = vc.id "
	    "where (vac.txn_id = ? or vac.active = 1) "
	    "and (tmc.txn_id = ? or tmc.active = 1) "
	    "and vac.type_id = ? "
	    "and vc.name like ? "
	    "and tmc.run_id = ? "
	    "and ? <= vac.d0_max and ? >= vac.d0_min "
	    "and ? <= vac.d1_max and ? >= vac.d1_min "
	    "and ? <= vac.d2_max and ? >= vac.d2_min "
    	"and  (vac.data_type = ? or vac.data_type = ?) ";
	}
	else {
		count = 0;
		return rc;
	}	

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


    rc = get_matching_timestep_count_catalog_all_timesteps_with_var_attributes_with_type_var_substr_dims_range (args, min, max, query_str, count); assert (rc == RC_OK);

    if (count > 0) {
        entries.reserve(count);

        rc = sqlite3_prepare_v2 (db, query.c_str(), -1, &stmt, &tail); 
        if (rc != SQLITE_OK)
        {
            fprintf (stderr, "Error catalog_all_timesteps_with_var_attributes_with_type_var_dims_substr_range stub: Line: %d SQL error (%d): %s\n", __LINE__, rc, sqlite3_errmsg (db));
            sqlite3_close (db);
            goto cleanup;
        }
        // rc = sqlite3_prepare_v2 (db, query.c_str(), -1, &stmt, &tail); assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int64 (stmt, 1, args.txn_id); assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int64 (stmt, 2, args.txn_id); assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int64 (stmt, 3, args.type_id); assert (rc == SQLITE_OK);
    	rc = sqlite3_bind_text (stmt, 4, strdup(args.var_name_substr.c_str()), -1, free); assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int64 (stmt, 5, args.run_id); assert (rc == SQLITE_OK);

        for (int j = 0; j < args.num_dims; j++)
        {
            rc = sqlite3_bind_double (stmt, 6 + (j * 2), args.dims.at(j).min);
            rc = sqlite3_bind_double (stmt, 7 + (j * 2), args.dims.at(j).max);
        }

	    rc = sqlite3_bind_int (stmt, 12, ATTR_DATA_TYPE_INT); assert (rc == SQLITE_OK);
	    rc = sqlite3_bind_int (stmt, 13, ATTR_DATA_TYPE_REAL); assert (rc == SQLITE_OK);

		switch(args.data_type) {
	        case ATTR_DATA_TYPE_INT : {
	        	rc = sqlite3_bind_int64 (stmt, 14, min_int); assert (rc == SQLITE_OK);
	        	if(args.range_type == DATA_RANGE) {
	        		rc = sqlite3_bind_int64 (stmt, 15, max_int); assert (rc == SQLITE_OK);
	        	}
	            break;
	        }
	        case ATTR_DATA_TYPE_REAL : {
	        	rc = sqlite3_bind_double (stmt, 14, min_real); assert (rc == SQLITE_OK);
	        	if(args.range_type == DATA_RANGE) {
	        		rc = sqlite3_bind_double (stmt, 15, max_real); assert (rc == SQLITE_OK);
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
static int get_matching_timestep_count_catalog_all_timesteps_with_var_attributes_with_type_var_substr_dims_range (const md_catalog_all_timesteps_with_var_attributes_with_type_var_substr_dims_range_args &args, 
	T min, T max, string query_str, uint32_t &count)
{
    int rc;
    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;
    string query;

    if(args.num_dims == 1) {
		query = "select count(*) from (select distinct tmc.id, tmc.run_id from timestep_catalog tmc "
	    "inner join type_catalog tc on tc.run_id = tmc.run_id "
	    "inner join var_attribute_catalog vac on vac.timestep_id = tmc.id and vac.type_id = tc.id "
	    "inner join var_catalog vc on vac.timestep_id = vc.timestep_id and vac.var_id = vc.id "
	    "where (vac.txn_id = ? or vac.active = 1) "
	    "and (tmc.txn_id = ? or tmc.active = 1) "
	    "and vac.type_id = ? "
	    "and vc.name like ? "
	    "and tmc.run_id = ? "
	    "and ? <= vac.d0_max and ? >= vac.d0_min "
	    "and  (vac.data_type = ? or vac.data_type = ?) " + query_str + " ) as internalQuery"; 
 
	}
	else if(args.num_dims == 2) {
		query = "select count(*) from (select distinct tmc.id, tmc.run_id from timestep_catalog tmc "
	    "inner join type_catalog tc on tc.run_id = tmc.run_id "
	    "inner join var_attribute_catalog vac on vac.timestep_id = tmc.id and vac.type_id = tc.id "
	    "inner join var_catalog vc on vac.timestep_id = vc.timestep_id and vac.var_id = vc.id "
	    "where (vac.txn_id = ? or vac.active = 1) "
	    "and (tmc.txn_id = ? or tmc.active = 1) "
	    "and vac.type_id = ? "
	    "and vc.name like ? "
	    "and tmc.run_id = ? "
	    "and ? <= vac.d0_max and ? >= vac.d0_min "
	    "and ? <= vac.d1_max and ? >= vac.d1_min "
	    "and  (vac.data_type = ? or vac.data_type = ?) " + query_str + " ) as internalQuery"; 
 
	}
	else if(args.num_dims == 3) {
		query = "select count(*) from (select distinct tmc.id, tmc.run_id from timestep_catalog tmc "
	    "inner join type_catalog tc on tc.run_id = tmc.run_id "
	    "inner join var_attribute_catalog vac on vac.timestep_id = tmc.id and vac.type_id = tc.id "
	    "inner join var_catalog vc on vac.timestep_id = vc.timestep_id and vac.var_id = vc.id "
	    "where (vac.txn_id = ? or vac.active = 1) "
	    "and (tmc.txn_id = ? or tmc.active = 1) "
	    "and vac.type_id = ? "
	    "and vc.name like ? "
	    "and tmc.run_id = ? "
	    "and ? <= vac.d0_max and ? >= vac.d0_min "
	    "and ? <= vac.d1_max and ? >= vac.d1_min "
	    "and ? <= vac.d2_max and ? >= vac.d2_min "
	    "and  (vac.data_type = ? or vac.data_type = ?) " + query_str + " ) as internalQuery"; 
 
	}
    
    rc = sqlite3_prepare_v2 (db, query.c_str(), -1, &stmt, &tail); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 1, args.txn_id); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 2, args.txn_id); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 3, args.type_id); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_text (stmt, 4, strdup(args.var_name_substr.c_str()), -1, free); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 5, args.run_id); assert (rc == SQLITE_OK);

    for (int j = 0; j < args.num_dims; j++)
    {
        rc = sqlite3_bind_double (stmt, 6 + (j * 2), args.dims.at(j).min);
        rc = sqlite3_bind_double (stmt, 7 + (j * 2), args.dims.at(j).max);
    }

    rc = sqlite3_bind_int (stmt, 12, ATTR_DATA_TYPE_INT); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int (stmt, 13, ATTR_DATA_TYPE_REAL); assert (rc == SQLITE_OK);

	switch(args.data_type) {
        case ATTR_DATA_TYPE_INT : {
        	rc = sqlite3_bind_int64 (stmt, 14, min_int); assert (rc == SQLITE_OK);
        	if(args.range_type == DATA_RANGE) {
        		rc = sqlite3_bind_int64 (stmt, 15, max_int); assert (rc == SQLITE_OK);
        	}
            break;
        }
        case ATTR_DATA_TYPE_REAL : {
        	rc = sqlite3_bind_double (stmt, 14, min_real); assert (rc == SQLITE_OK);
    	    if(args.range_type == DATA_RANGE) {
        		rc = sqlite3_bind_double (stmt, 15, max_real); assert (rc == SQLITE_OK);
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

int md_catalog_all_timesteps_with_var_attributes_with_type_var_substr_stub (const md_catalog_all_timesteps_with_var_attributes_with_type_var_substr_args &args,
                           std::vector<md_catalog_timestep_entry> &entries,
                           uint32_t &count)
{
    int rc;
    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;
    const char * query = "select tmc.* from timestep_catalog tmc "
    "inner join type_catalog tc on tc.run_id = tmc.run_id "
    "inner join var_attribute_catalog vac on vac.timestep_id = tmc.id and vac.type_id = tc.id "
    "inner join var_catalog vc on vac.timestep_id = vc.timestep_id and vac.var_id = vc.id "
    "where (vac.txn_id = ? or vac.active = 1) "
    "and (tmc.txn_id = ? or tmc.active = 1) "
    "and vac.type_id = ? "
    "and vc.name like ? "
    "and tmc.run_id = ? "
    "group by tmc.id, tmc.run_id";

    rc = get_matching_timestep_count_catalog_all_timesteps_with_var_attributes_with_type_var_substr (args, count); assert (rc == RC_OK);

    if (count > 0) {
        entries.reserve(count);

        rc = sqlite3_prepare_v2 (db, query, -1, &stmt, &tail); 
        if (rc != SQLITE_OK)
        {
            fprintf (stderr, "Error catalog_all_timesteps_with_var_attributes_with_type_var_substr stub: Line: %d SQL error (%d): %s\n", __LINE__, rc, sqlite3_errmsg (db));
            sqlite3_close (db);
            goto cleanup;
        }
        // rc = sqlite3_prepare_v2 (db, query, -1, &stmt, &tail); assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int64 (stmt, 1, args.txn_id); assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int64 (stmt, 2, args.txn_id); assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int64 (stmt, 3, args.type_id); assert (rc == SQLITE_OK);
    	rc = sqlite3_bind_text (stmt, 4, strdup(args.var_name_substr.c_str()), -1, free); assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int64 (stmt, 5, args.run_id); assert (rc == SQLITE_OK);

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



static int get_matching_timestep_count_catalog_all_timesteps_with_var_attributes_with_type_var_substr (const md_catalog_all_timesteps_with_var_attributes_with_type_var_substr_args &args, uint32_t &count)
{
    int rc;
    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;
    const char * query = "select count(*) from (select distinct tmc.id, tmc.run_id from timestep_catalog tmc "
    "inner join type_catalog tc on tc.run_id = tmc.run_id "
    "inner join var_attribute_catalog vac on vac.timestep_id = tmc.id and vac.type_id = tc.id "
    "inner join var_catalog vc on vac.timestep_id = vc.timestep_id and vac.var_id = vc.id "
    "where (vac.txn_id = ? or vac.active = 1) "
    "and (tmc.txn_id = ? or tmc.active = 1) "
    "and vac.type_id = ? "
    "and vc.name like ? "
    "and tmc.run_id = ? ) as internalQuery";

    rc = sqlite3_prepare_v2 (db, query, -1, &stmt, &tail); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 1, args.txn_id); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 2, args.txn_id); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 3, args.type_id); assert (rc == SQLITE_OK);
   	rc = sqlite3_bind_text (stmt, 4, strdup(args.var_name_substr.c_str()), -1, free); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 5, args.run_id); assert (rc == SQLITE_OK);

    rc = sqlite3_step (stmt); assert (rc == SQLITE_ROW || rc == SQLITE_DONE);
    count = sqlite3_column_int64 (stmt, 0);
    rc = sqlite3_finalize (stmt); assert (rc == SQLITE_OK);

    // cout << "count: " << count << endl;

    return rc;
}

template <class T>
int md_catalog_all_timesteps_with_var_attributes_with_type_var_substr_range_stub (const md_catalog_all_timesteps_with_var_attributes_with_type_var_substr_range_args &args,
                           T min, T max,
                           std::vector<md_catalog_timestep_entry> &entries,
                           uint32_t &count)
{
    int rc;
    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;
    string query = "select tmc.* from timestep_catalog tmc "
    "inner join type_catalog tc on tc.run_id = tmc.run_id "
    "inner join var_attribute_catalog vac on vac.timestep_id = tmc.id and vac.type_id = tc.id "
    "inner join var_catalog vc on vac.timestep_id = vc.timestep_id and vac.var_id = vc.id "
    "where (vac.txn_id = ? or vac.active = 1) "
    "and (tmc.txn_id = ? or tmc.active = 1) "
    "and vac.type_id = ? "
    "and vc.name like ? "
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


    rc = get_matching_timestep_count (args, min, max, query_str, count); assert (rc == RC_OK);

    if (count > 0) {
        entries.reserve(count);

        rc = sqlite3_prepare_v2 (db, query.c_str(), -1, &stmt, &tail); 
        if (rc != SQLITE_OK)
        {
            fprintf (stderr, "Error catalog_all_timesteps_with_var_attributes_with_type_var_substr_range stub: Line: %d SQL error (%d): %s\n", __LINE__, rc, sqlite3_errmsg (db));
            sqlite3_close (db);
            goto cleanup;
        }
        // rc = sqlite3_prepare_v2 (db, query.c_str(), -1, &stmt, &tail); assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int64 (stmt, 1, args.txn_id); assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int64 (stmt, 2, args.txn_id); assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int64 (stmt, 3, args.type_id); assert (rc == SQLITE_OK);
    	rc = sqlite3_bind_text (stmt, 4, strdup(args.var_name_substr.c_str()), -1, free); assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int64 (stmt, 5, args.run_id); assert (rc == SQLITE_OK);

	    rc = sqlite3_bind_int (stmt, 6, ATTR_DATA_TYPE_INT); assert (rc == SQLITE_OK);
	    rc = sqlite3_bind_int (stmt, 7, ATTR_DATA_TYPE_REAL); assert (rc == SQLITE_OK);

		switch(args.data_type) {
	        case ATTR_DATA_TYPE_INT : {
	        	rc = sqlite3_bind_int64 (stmt, 8, (uint64_t)min); assert (rc == SQLITE_OK);
	        	if(args.range_type == DATA_RANGE) {
	        		rc = sqlite3_bind_int64 (stmt, (uint64_t)9, max); assert (rc == SQLITE_OK);
	        	}
	            break;
	        }
	        case ATTR_DATA_TYPE_REAL : {
	        	rc = sqlite3_bind_double (stmt, (long double)8, min); assert (rc == SQLITE_OK);
	    	    if(args.range_type == DATA_RANGE) {
	        		rc = sqlite3_bind_double (stmt, (long double)9, max); assert (rc == SQLITE_OK);
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
static int get_matching_timestep_count (const md_catalog_all_timesteps_with_var_attributes_with_type_var_substr_range_args &args, 
	T min, T max, string query_str, uint32_t &count)
{
    int rc;
    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;

	string query = "select count(*) from (select distinct tmc.id, tmc.run_id from timestep_catalog tmc "
    "inner join type_catalog tc on tc.run_id = tmc.run_id "
    "inner join var_attribute_catalog vac on vac.timestep_id = tmc.id and vac.type_id = tc.id "
    "inner join var_catalog vc on vac.timestep_id = vc.timestep_id and vac.var_id = vc.id "
    "where (vac.txn_id = ? or vac.active = 1) "
    "and (tmc.txn_id = ? or tmc.active = 1) "
    "and vac.type_id = ? "
    "and vc.name like ? "
    "and tmc.run_id = ? "
    "and  (vac.data_type = ? or vac.data_type = ?) " + query_str + " ) as internalQuery";

    rc = sqlite3_prepare_v2 (db, query.c_str(), -1, &stmt, &tail); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 1, args.txn_id); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 2, args.txn_id); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 3, args.type_id); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_text (stmt, 4, strdup(args.var_name_substr.c_str()), -1, free); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 5, args.run_id); assert (rc == SQLITE_OK);

    rc = sqlite3_bind_int (stmt, 6, ATTR_DATA_TYPE_INT); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int (stmt, 7, ATTR_DATA_TYPE_REAL); assert (rc == SQLITE_OK);

	switch(args.data_type) {
        case ATTR_DATA_TYPE_INT : {
        	rc = sqlite3_bind_int64 (stmt, 8, (uint64_t)min); assert (rc == SQLITE_OK);
        	if(args.range_type == DATA_RANGE) {
        		rc = sqlite3_bind_int64 (stmt, (uint64_t)9, max); assert (rc == SQLITE_OK);
        	}
            break;
        }
        case ATTR_DATA_TYPE_REAL : {
        	rc = sqlite3_bind_double (stmt, (long double)8, min); assert (rc == SQLITE_OK);
    	    if(args.range_type == DATA_RANGE) {
        		rc = sqlite3_bind_double (stmt, (long double)9, max); assert (rc == SQLITE_OK);
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