#include <md_local.hh>

extern sqlite3 *db;

using namespace std;


static int get_matching_attribute_count (const md_catalog_all_run_attributes_args &args, uint32_t &count);

int md_catalog_all_run_attributes_stub (const md_catalog_all_run_attributes_args &args,
                           std::vector<md_catalog_run_attribute_entry> &attribute_list,
                           uint32_t &count)
{
    int rc;
    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;

    // const char * query = "select rac.* from run_attribute_catalog rac "
    // "inner join type_catalog tc on rac.type_id = tc.id "
    // "where (rac.txn_id = ? or rac.active = 1) "
    // "and tc.run_id = ? ";
    const char * query = "select rac.* from run_attribute_catalog rac "
    "where (rac.txn_id = ? or rac.active = 1) "
    "and rac.run_id = ? ";
    // }

    rc = get_matching_attribute_count (args, count); assert (rc == RC_OK);

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



static int get_matching_attribute_count (const md_catalog_all_run_attributes_args &args, uint32_t &count)
{
    int rc;
    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;
    // const char * query = "select count (*) from run_attribute_catalog rac "
    // "inner join type_catalog tc on rac.type_id = tc.id "
    // "where (rac.txn_id = ? or rac.active = 1) "
    // "and tc.run_id = ? ";
    const char * query = "select count (*) from run_attribute_catalog rac "
    "where (rac.txn_id = ? or rac.active = 1) "
    "and rac.run_id = ? ";

    rc = sqlite3_prepare_v2 (db, query, -1, &stmt, &tail); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 1, args.txn_id); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 2, args.run_id); assert (rc == SQLITE_OK);
        
    rc = sqlite3_step (stmt); assert (rc == SQLITE_ROW || rc == SQLITE_DONE);
    count = sqlite3_column_int64 (stmt, 0);
    // cout << "in catalog all attrs, count: " << count << endl;
    rc = sqlite3_finalize (stmt); assert (rc == SQLITE_OK);

    return rc;
}
