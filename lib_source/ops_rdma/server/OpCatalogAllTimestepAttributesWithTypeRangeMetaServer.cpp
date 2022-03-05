#include <OpCatalogAllTimestepAttributesWithTypeRangeMetaCommon.hh>
#include <server_timing_constants_new.hh>
#include <sql_helper_functs.hh>
#include <OpCoreServer.hh>

extern sqlite3 *db;

using namespace std;

template <class T>
int md_catalog_all_timestep_attributes_with_type_range_stub (const md_catalog_all_timestep_attributes_with_type_range_args &args,
                           T min, T max,
                           std::vector<md_catalog_timestep_attribute_entry> &attribute_list,
                           uint32_t &count);

template <class T>
static int get_matching_attribute_count (const md_catalog_all_timestep_attributes_with_type_range_args &args, 
    T min, T max, string query, uint32_t &count);


WaitingType OpCatalogAllTimestepAttributesWithTypeRangeMeta::handleMessage(bool rdma, message_t *incoming_msg) {
  
    int rc;

    md_catalog_all_timestep_attributes_with_type_range_args sql_args;
    std::vector<md_catalog_timestep_attribute_entry> attribute_list;
    uint32_t count;
    

    deArchiveMsgFromClient(rdma, incoming_msg, sql_args);        
    add_timing_point(OP_CATALOG_ALL_TIMESTEP_ATTRIBUTES_WITH_TYPE_RANGE_DEARCHIVE_MSG_FROM_CLIENT);


    stringstream sso;
    sso << sql_args.data;
    boost::archive::text_iarchive ia(sso);

    switch(sql_args.data_type) {
        case ATTR_DATA_TYPE_INT : {
            uint64_t min_int;
            uint64_t max_int;

            if(sql_args.range_type == DATA_RANGE) {
    //    cout << "got here 1 \n";
                ia >> min_int;
                ia >> max_int;
            }
            else if(sql_args.range_type == DATA_MAX || sql_args.range_type == DATA_MIN) {
                ia >> min_int;
                max_int = min_int;
            }

            rc = md_catalog_all_timestep_attributes_with_type_range_stub(sql_args, 
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

            rc = md_catalog_all_timestep_attributes_with_type_range_stub(sql_args, 
                min_real, max_real, attribute_list, count);

            break;
        }
    }    
    add_timing_point(OP_CATALOG_ALL_TIMESTEP_ATTRIBUTES_WITH_TYPE_RANGE_MD_CATALOG_ALL_TIMESTEP_ATTRIBUTES_WITH_TYPE_RANGE_STUB);


    std::string serial_str = serializeMsgToClient(attribute_list, count, rc);
    // cout << "in catalog all, attr list is of size: " << attribute_list.size() << endl;
    add_timing_point(OP_CATALOG_ALL_TIMESTEP_ATTRIBUTES_WITH_TYPE_RANGE_SERIALIZE_MSG_FOR_CLIENT);

    createOutgoingMessage(dst, 
                          0, //Not expecting a reply
                          dst_mailbox,
                          serial_str);
    add_timing_point(OP_CATALOG_ALL_TIMESTEP_ATTRIBUTES_WITH_TYPE_RANGE_CREATE_MSG_FOR_CLIENT);

    opbox::net::SendMsg(peer, ldo_msg);
    state=State::done;
    add_timing_point(OP_CATALOG_ALL_TIMESTEP_ATTRIBUTES_WITH_TYPE_RANGE_SEND_MSG_TO_CLIENT_OP_DONE);
    return WaitingType::done_and_destroy;

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

    string query;

    if(args.timestep_id == ALL_TIMESTEPS) {
        query = "select tac.* from timestep_attribute_catalog tac "
        "inner join type_catalog tc on tac.type_id = tc.id "
        "where (tac.txn_id = ? or tac.active = 1) "
        "and tc.run_id = ? "
        "and tac.type_id = ? "
        "and  (tac.data_type = ? or tac.data_type = ?) ";
    }
    else {
        query = "select tac.* from timestep_attribute_catalog tac "
        "inner join type_catalog tc on tac.type_id = tc.id "
        "where (tac.txn_id = ? or tac.active = 1) "
        "and tc.run_id = ? "
        "and tac.timestep_id = ? "
        "and tac.type_id = ? "
        "and  (tac.data_type = ? or tac.data_type = ?) ";        
    }

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

    rc = get_matching_attribute_count (args, min, max, query_str, count); //assert (rc == RC_OK);

    if (count > 0) {
        attribute_list.reserve(count);


        rc = sqlite3_prepare_v2 (db, query.c_str(), -1, &stmt, &tail); //assert (rc == SQLITE_OK);
        int index = 3;
        rc = sqlite3_bind_int64 (stmt, 1, args.txn_id); //assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int64 (stmt, 2, args.run_id); //assert (rc == SQLITE_OK);
        if(args.timestep_id != ALL_TIMESTEPS) {
               rc = sqlite3_bind_int64 (stmt, 3, args.timestep_id); //assert (rc == SQLITE_OK);
               index += 1;
           }
        rc = sqlite3_bind_int64 (stmt, index, args.type_id); //assert (rc == SQLITE_OK);

        rc = sqlite3_bind_int (stmt, index+1, ATTR_DATA_TYPE_INT); //assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int (stmt, index+2, ATTR_DATA_TYPE_REAL); //assert (rc == SQLITE_OK);

        switch(args.data_type) {
            case ATTR_DATA_TYPE_INT : {
                rc = sqlite3_bind_int64 (stmt, index+3, (uint64_t)min); //assert (rc == SQLITE_OK);
                if(args.range_type == DATA_RANGE) {
                    rc = sqlite3_bind_int64 (stmt, index+4, (uint64_t)max); //assert (rc == SQLITE_OK);
                }
                break;
            }
            case ATTR_DATA_TYPE_REAL : {
                rc = sqlite3_bind_double (stmt, index+3, (long double)min); //assert (rc == SQLITE_OK);
                if(args.range_type == DATA_RANGE) {
                    rc = sqlite3_bind_double (stmt, index+4, (long double)max); //assert (rc == SQLITE_OK);
                }
                break;
            }
        }

        rc = sql_retrieve_timestep_attrs(stmt, attribute_list);

        rc = sqlite3_finalize (stmt);  
    }
        

cleanup:

    return rc;
}


template <class T>
static int get_matching_attribute_count (const md_catalog_all_timestep_attributes_with_type_range_args &args, 
    T min, T max, string query_str, uint32_t &count)
{
    int rc;
    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;
    string query;

    if(args.timestep_id == ALL_TIMESTEPS) {
        query = "select count (*) from timestep_attribute_catalog tac "
        "inner join type_catalog tc on tac.type_id = tc.id "
        "where (tac.txn_id = ? or tac.active = 1) "
        "and tc.run_id = ? "
        "and tac.type_id = ? "
        "and  (tac.data_type = ? or tac.data_type = ?) " + query_str;
    }
    else {
        query = "select count (*) from timestep_attribute_catalog tac "
        "inner join type_catalog tc on tac.type_id = tc.id "
        "where (tac.txn_id = ? or tac.active = 1) "
        "and tc.run_id = ? "
        "and tac.timestep_id = ? "    
        "and tac.type_id = ? "
        "and  (tac.data_type = ? or tac.data_type = ?) " + query_str;        
    }

    rc = sqlite3_prepare_v2 (db, query.c_str(), -1, &stmt, &tail); //assert (rc == SQLITE_OK);
    int index = 3;
    rc = sqlite3_bind_int64 (stmt, 1, args.txn_id); //assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 2, args.run_id); //assert (rc == SQLITE_OK);
    if(args.timestep_id != ALL_TIMESTEPS) {
           rc = sqlite3_bind_int64 (stmt, 3, args.timestep_id); //assert (rc == SQLITE_OK);
           index += 1;
       }
    rc = sqlite3_bind_int64 (stmt, index, args.type_id); //assert (rc == SQLITE_OK);

    rc = sqlite3_bind_int (stmt, index+1, ATTR_DATA_TYPE_INT); //assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int (stmt, index+2, ATTR_DATA_TYPE_REAL); //assert (rc == SQLITE_OK);

    switch(args.data_type) {
        case ATTR_DATA_TYPE_INT : {
            rc = sqlite3_bind_int64 (stmt, index+3, (uint64_t)min); //assert (rc == SQLITE_OK);
            if(args.range_type == DATA_RANGE) {
                rc = sqlite3_bind_int64 (stmt, index+4, (uint64_t)max); //assert (rc == SQLITE_OK);
            }
            break;
        }
        case ATTR_DATA_TYPE_REAL : {
            rc = sqlite3_bind_double (stmt, index+3, (long double)min); //assert (rc == SQLITE_OK);
            if(args.range_type == DATA_RANGE) {
                rc = sqlite3_bind_double (stmt, index+4, (long double)max); //assert (rc == SQLITE_OK);
            }
            break;
        }
    }

    rc = sqlite3_step (stmt); //assert (rc == SQLITE_ROW || rc == SQLITE_DONE);
    count = sqlite3_column_int64 (stmt, 0);
    // cout << "in catalog all attrs, count: " << count << endl;
    rc = sqlite3_finalize (stmt); //assert (rc == SQLITE_OK);

    return rc;
}
