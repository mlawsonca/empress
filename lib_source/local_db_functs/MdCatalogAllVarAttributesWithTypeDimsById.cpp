#include <md_local.hh>

extern sqlite3 *db;

using namespace std;


static int get_matching_attribute_count (const md_catalog_all_var_attributes_with_type_dims_by_id_args &args, uint32_t &count);



int md_catalog_all_var_attributes_with_type_dims_by_id_stub (const md_catalog_all_var_attributes_with_type_dims_by_id_args &args,
						   std::vector<md_catalog_var_attribute_entry> &attribute_list,
						   uint32_t &count)
{
	int rc;
	sqlite3_stmt * stmt = NULL;
	const char * tail = NULL;
	string query;

    if(args.num_dims == 1) {
		query = "select vac.* from var_attribute_catalog vac where "
		"(vac.txn_id = ? or vac.active = 1) "
		"and (vac.type_id = ?) "
		"and vac.timestep_id = ? "  
		"and ? <= vac.d0_max and ? >= vac.d0_min ";
	}
	else if(args.num_dims == 2) {
		query = "select vac.* from var_attribute_catalog vac where "
		"(vac.txn_id = ? or vac.active = 1) "
		"and (vac.type_id = ?) "
		"and vac.timestep_id = ? "  
		"and ? <= vac.d0_max and ? >= vac.d0_min "
		"and ? <= vac.d1_max and ? >= vac.d1_min ";
	}
	else if(args.num_dims == 3) {
		query = "select vac.* from var_attribute_catalog vac where "
		"(vac.txn_id = ? or vac.active = 1) "
		"and (vac.type_id = ?) "
		"and vac.timestep_id = ? "  
		"and ? <= vac.d0_max and ? >= vac.d0_min "
		"and ? <= vac.d1_max and ? >= vac.d1_min "
		"and ? <= vac.d2_max and ? >= vac.d2_min ";
	}
	else {
		count = 0;
		return rc;
	}	


	rc = get_matching_attribute_count (args, count); assert (rc == RC_OK);

	if (count > 0) {
		attribute_list.reserve(count);

		rc = sqlite3_prepare_v2 (db, query.c_str(), -1, &stmt, &tail); 
		if (rc != SQLITE_OK)
		{
			fprintf (stderr, "Error catalog_all_var_attributes_with_type_dims_by_id stub: Line: %d SQL error (%d): %s\n", __LINE__, rc, sqlite3_errmsg (db));
			sqlite3_close (db);
			goto cleanup;
		}
		rc = sqlite3_bind_int64 (stmt, 1, args.txn_id); assert (rc == SQLITE_OK);
		rc = sqlite3_bind_int64 (stmt, 2, args.type_id); assert (rc == SQLITE_OK);
		rc = sqlite3_bind_int64 (stmt, 3, args.timestep_id); assert (rc == SQLITE_OK);

		for (int j = 0; j < args.num_dims; j++)
		{
			rc = sqlite3_bind_double (stmt, 4 + (j * 2), args.dims.at(j).min);
			rc = sqlite3_bind_double (stmt, 5 + (j * 2), args.dims.at(j).max);
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
				// case ATTR_DATA_TYPE_NULL : {
				//     break;
				// }        

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
				case ATTR_DATA_TYPE_TEXT : {
					attribute.data = (char *)sqlite3_column_text (stmt, 14);
					break;
				}
                case ATTR_DATA_TYPE_BLOB : {
        			int size_blob = sqlite3_column_bytes (stmt, 14);
        			
            		attribute.data.assign((char *)sqlite3_column_text (stmt, 14),(char *)sqlite3_column_text (stmt, 14) + size_blob);
                    // attribute.data = (char *)sqlite3_column_text (stmt, 14);
                    break;
                }
			}   							 
			// attribute.data = (char *) sqlite3_column_text (stmt, 14);
			//  cout << "attr.dims.at(2).min: " << attribute.dims.at(2).min << " attr.dims.at(2).min: " << attribute.dims.at(2).max << endl;



			rc = sqlite3_step (stmt);
			attribute_list.push_back(attribute);
		}

		rc = sqlite3_finalize (stmt);  
	}
		

cleanup:

	return rc;
}



static int get_matching_attribute_count (const md_catalog_all_var_attributes_with_type_dims_by_id_args &args, uint32_t &count)
{
	int rc;
	sqlite3_stmt * stmt = NULL;
	const char * tail = NULL;
    string query;

    if (args.num_dims == 1) {
		query = "select count (*) from var_attribute_catalog vac where "
			"(vac.txn_id = ? or vac.active = 1) "
			"and (vac.type_id = ?) "
			"and vac.timestep_id = ? "  
			"and ? <= vac.d0_max and ? >= vac.d0_min ";  
    }
    else if(args.num_dims == 2) {
		query = "select count (*) from var_attribute_catalog vac where "
			"(vac.txn_id = ? or vac.active = 1) "
			"and (vac.type_id = ?) "
			"and vac.timestep_id = ? "  
			"and ? <= vac.d0_max and ? >= vac.d0_min "
			"and ? <= vac.d1_max and ? >= vac.d1_min ";  
    }
    else if(args.num_dims == 3) {
		query = "select count (*) from var_attribute_catalog vac where "
			"(vac.txn_id = ? or vac.active = 1) "
			"and (vac.type_id = ?) "
			"and vac.timestep_id = ? "  
			"and ? <= vac.d0_max and ? >= vac.d0_min "
			"and ? <= vac.d1_max and ? >= vac.d1_min "
			"and ? <= vac.d2_max and ? >= vac.d2_min ";  
    }


	rc = sqlite3_prepare_v2 (db, query.c_str(), -1, &stmt, &tail); assert (rc == SQLITE_OK);
	rc = sqlite3_bind_int64 (stmt, 1, args.txn_id); assert (rc == SQLITE_OK);
	rc = sqlite3_bind_int64 (stmt, 2, args.type_id); assert (rc == SQLITE_OK);
	rc = sqlite3_bind_int64 (stmt, 3, args.timestep_id); assert (rc == SQLITE_OK);

	//printf ("looking for name: %s path: %s version: %d ", args->name, args->path, args->var_version);
	for (int j = 0; j < args.num_dims; j++)
	{
		rc = sqlite3_bind_double (stmt, 4 + (j * 2), args.dims.at(j).min);
		rc = sqlite3_bind_double (stmt, 5 + (j * 2), args.dims.at(j).max);
//printf ("(%d:%d) ", dims [j].min, dims [j].max);
	}
//printf ("\n");
	rc = sqlite3_step (stmt); assert (rc == SQLITE_ROW || rc == SQLITE_DONE);
	count = sqlite3_column_int64 (stmt, 0);
	rc = sqlite3_finalize (stmt); assert (rc == SQLITE_OK);
//printf ("matching attr count: %d\n", *count);

	return rc;
}
