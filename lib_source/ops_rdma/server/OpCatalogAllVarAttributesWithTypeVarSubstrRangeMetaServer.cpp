#include <OpCatalogAllVarAttributesWithTypeVarSubstrRangeMetaCommon.hh>
#include <server_timing_constants_new.hh>
#include <sql_helper_functs.hh>
#include <OpCoreServer.hh>

extern sqlite3 *db;

using namespace std;

template <class T>
int md_catalog_all_var_attributes_with_type_var_substr_range_stub (const md_catalog_all_var_attributes_with_type_var_substr_range_args &args,
                           T min, T max,
                           std::vector<md_catalog_var_attribute_entry> &attribute_list,
                           uint32_t &count);

template <class T1>
static int get_matching_attribute_count (const md_catalog_all_var_attributes_with_type_var_substr_range_args &args, 
    T1 min, T1 max, string query_str, uint32_t &count);


WaitingType OpCatalogAllVarAttributesWithTypeVarSubstrRangeMeta::handleMessage(bool rdma, message_t *incoming_msg) {

    int rc;

    md_catalog_all_var_attributes_with_type_var_substr_range_args sql_args;
    std::vector<md_catalog_var_attribute_entry> attribute_list;
    uint32_t count;
    

    deArchiveMsgFromClient(rdma, incoming_msg, sql_args);        
    add_timing_point(OP_CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_RANGE_DEARCHIVE_MSG_FROM_CLIENT);

    stringstream sso;
    sso << sql_args.data;
    boost::archive::text_iarchive ia(sso);

    switch(sql_args.data_type) {
        case ATTR_DATA_TYPE_INT : {
            uint64_t min_int;
            uint64_t max_int;

            if(sql_args.range_type == DATA_RANGE) {
    //    // cout << "got here 1 \n";
                ia >> min_int;
                ia >> max_int;
            }
            else if(sql_args.range_type == DATA_MAX || sql_args.range_type == DATA_MIN) {
                ia >> min_int;
                max_int = min_int;
            }

            rc = md_catalog_all_var_attributes_with_type_var_substr_range_stub(sql_args, 
                min_int, max_int, attribute_list, count);

            break;
        }
        case ATTR_DATA_TYPE_REAL : {
            long double min_real;
            long double max_real;

            if(sql_args.range_type == DATA_RANGE) {
                ia >> min_real;
                ia >> max_real;
            }
            else if(sql_args.range_type == DATA_MAX || sql_args.range_type == DATA_MIN) {
                ia >> min_real;
                max_real = min_real;
            }

            rc = md_catalog_all_var_attributes_with_type_var_substr_range_stub(sql_args, 
                min_real, max_real, attribute_list, count);

            break;
        }
    }    

    add_timing_point(OP_CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_RANGE_MD_CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_RANGE_STUB);


    std::string serial_str = serializeMsgToClient(attribute_list, count, rc);
    // cout << "in server, rc: " << rc << endl;
    add_timing_point(OP_CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_RANGE_SERIALIZE_MSG_FOR_CLIENT);

    createOutgoingMessage(dst, 
                          0, //Not expecting a reply
                          dst_mailbox,
                          serial_str);
    add_timing_point(OP_CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_RANGE_CREATE_MSG_FOR_CLIENT);

    opbox::net::SendMsg(peer, ldo_msg);
    state=State::done;
    add_timing_point(OP_CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_RANGE_SEND_MSG_TO_CLIENT_OP_DONE);
    return WaitingType::done_and_destroy;

}




template <class T>
int md_catalog_all_var_attributes_with_type_var_substr_range_stub (const md_catalog_all_var_attributes_with_type_var_substr_range_args &args,
                           T min, T max,
                           std::vector<md_catalog_var_attribute_entry> &attribute_list,
                           uint32_t &count)
{
    int rc;
    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;
    
       string query = "select vac.* from var_attribute_catalog vac "
    "inner join var_catalog vc on vac.timestep_id = vc.timestep_id and vac.var_id = vc.id "
    "inner join type_catalog tc on vac.type_id = tc.id and vc.run_id = tc.run_id "
    "where (vac.txn_id = ? or vac.active = 1) "
    "and vac.type_id = ? "
    "and vc.name like ? "
    "and vac.timestep_id = ? "
    "and  (vac.data_type = ? or vac.data_type = ?) ";

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

    // cout << "got here \n";

    rc = get_matching_attribute_count (args, min, max, query_str, count); //assert (rc == RC_OK);

    if (count > 0) {
        attribute_list.reserve(count);

        rc = sqlite3_prepare_v2 (db, query.c_str(), -1, &stmt, &tail); 
        if (rc != SQLITE_OK)
        {
            fprintf (stderr, "Error catalog_all_var_attributes_with_type_var_substr_range stub: Line: %d SQL error (%d): %s\n", __LINE__, rc, sqlite3_errmsg (db));
            sqlite3_close (db);
            goto cleanup;
        }
        rc = sqlite3_bind_int64 (stmt, 1, args.txn_id); //assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int64 (stmt, 2, args.type_id); //assert (rc == SQLITE_OK);
        rc = sqlite3_bind_text (stmt, 3, strdup(args.var_name_substr.c_str()), -1, free); //assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int64 (stmt, 4, args.timestep_id); //assert (rc == SQLITE_OK);

        rc = sqlite3_bind_int (stmt, 5, ATTR_DATA_TYPE_INT); //assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int (stmt, 6, ATTR_DATA_TYPE_REAL); //assert (rc == SQLITE_OK);

        switch(args.data_type) {
            case ATTR_DATA_TYPE_INT : {
                rc = sqlite3_bind_int64 (stmt, 7, (uint64_t)min); //assert (rc == SQLITE_OK);
                if(args.range_type == DATA_RANGE) {
                    rc = sqlite3_bind_int64 (stmt, 8, (uint64_t)max); //assert (rc == SQLITE_OK);
                }
                break;
            }
            case ATTR_DATA_TYPE_REAL : {
                rc = sqlite3_bind_double (stmt, 7, (long double)min); //assert (rc == SQLITE_OK);
                   if(args.range_type == DATA_RANGE) {
                    rc = sqlite3_bind_double (stmt, 8, (long double)max); //assert (rc == SQLITE_OK);
                }
                break;
            }
        }

        rc = sql_retrieve_var_attrs(stmt, attribute_list);

        // cout << "before finalize, rc: " << rc << endl;
        rc = sqlite3_finalize (stmt); 
        // cout << "after finalize, rc: " << rc << endl;
    }
        
    // cout << "at end of stub rc: " << rc << endl;

cleanup:

    return rc;
}


template <class T>
static int get_matching_attribute_count (const md_catalog_all_var_attributes_with_type_var_substr_range_args &args, 
    T min, T max, string query_str, uint32_t &count)
{
    int rc;
    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;
       string query = "select count (*) from var_attribute_catalog vac "
    "inner join var_catalog vc on vac.timestep_id = vc.timestep_id and vac.var_id = vc.id "
    "inner join type_catalog tc on vac.type_id = tc.id and vc.run_id = tc.run_id "
    "where (vac.txn_id = ? or vac.active = 1) "
    "and vac.type_id = ? "
    "and vc.name like ? "
    "and vac.timestep_id = ? "
    "and  (vac.data_type = ? or vac.data_type = ?) " + query_str;

    rc = sqlite3_prepare_v2 (db, query.c_str(), -1, &stmt, &tail); //assert (rc == SQLITE_OK);
    if (rc != SQLITE_OK)
    {
        fprintf (stderr, "Error get_matching_attribute_count stub: Line: %d SQL error (%d): %s\n", __LINE__, rc, sqlite3_errmsg (db));
        sqlite3_close (db);
        return rc;
    }
    rc = sqlite3_bind_int64 (stmt, 1, args.txn_id); //assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 2, args.type_id); //assert (rc == SQLITE_OK);
    rc = sqlite3_bind_text (stmt, 3, strdup(args.var_name_substr.c_str()), -1, free); //assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 4, args.timestep_id); //assert (rc == SQLITE_OK);

    rc = sqlite3_bind_int (stmt, 5, ATTR_DATA_TYPE_INT); //assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int (stmt, 6, ATTR_DATA_TYPE_REAL); //assert (rc == SQLITE_OK);

    switch(args.data_type) {
        case ATTR_DATA_TYPE_INT : {
            rc = sqlite3_bind_int64 (stmt, 7, (uint64_t)min); //assert (rc == SQLITE_OK);
            if(args.range_type == DATA_RANGE) {
                rc = sqlite3_bind_int64 (stmt, 8, (uint64_t)max); //assert (rc == SQLITE_OK);
            }
            break;
        }
        case ATTR_DATA_TYPE_REAL : {
            rc = sqlite3_bind_double (stmt, 7, (long double)min); //assert (rc == SQLITE_OK);
               if(args.range_type == DATA_RANGE) {
                rc = sqlite3_bind_double (stmt, 8, (long double)max); //assert (rc == SQLITE_OK);
            }
            break;
        }
    }

    rc = sqlite3_step (stmt); //assert (rc == SQLITE_ROW || rc == SQLITE_DONE);
    count = sqlite3_column_int64 (stmt, 0);
    rc = sqlite3_finalize (stmt); //assert (rc == SQLITE_OK);

    return rc;
}
