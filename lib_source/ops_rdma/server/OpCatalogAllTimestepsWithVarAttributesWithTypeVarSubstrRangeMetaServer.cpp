#include <OpCatalogAllTimestepsWithVarAttributesWithTypeVarSubstrRangeMetaCommon.hh>
#include <server_timing_constants_new.hh>
#include <sql_helper_functs.hh>
#include <OpCoreServer.hh>

extern sqlite3 *db;

using namespace std;

template <class T>
int md_catalog_all_timesteps_with_var_attributes_with_type_var_substr_range_stub (const md_catalog_all_timesteps_with_var_attributes_with_type_var_substr_range_args &args,
                           T min, T max,
                           std::vector<md_catalog_timestep_entry> &entries,
                           uint32_t &count);

template <class T>
static int get_matching_timestep_count (const md_catalog_all_timesteps_with_var_attributes_with_type_var_substr_range_args &args, 
    T min, T max, string query_str, uint32_t &count);


WaitingType OpCatalogAllTimestepsWithVarAttributesWithTypeVarSubstrRangeMeta::handleMessage(bool rdma, message_t *incoming_msg) {

    int rc;

    md_catalog_all_timesteps_with_var_attributes_with_type_var_substr_range_args sql_args;
    std::vector<md_catalog_timestep_entry> entries;
    uint32_t count;
    

    deArchiveMsgFromClient(rdma, incoming_msg, sql_args);        
    add_timing_point(OP_CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_RANGE_DEARCHIVE_MSG_FROM_CLIENT);

    // cout << "about to deserialize data: " << sql_args.data << endl;
    stringstream sso;
    sso << sql_args.data;
    boost::archive::text_iarchive ia(sso);

    switch(sql_args.data_type) {
        case ATTR_DATA_TYPE_INT : {
            // cout << "case: int \n";
            uint64_t min_int;
            uint64_t max_int;

            if(sql_args.range_type == DATA_RANGE) {
                // cout << "case: int range \n";

                ia >> min_int;
                ia >> max_int;
            }
            else if(sql_args.range_type == DATA_MAX || sql_args.range_type == DATA_MIN) {
                // cout << "case: int min/max \n";

                ia >> min_int;
                max_int = min_int;
            }

            rc = md_catalog_all_timesteps_with_var_attributes_with_type_var_substr_range_stub(sql_args, 
                min_int, max_int, entries, count);

            break;
        }
        case ATTR_DATA_TYPE_REAL : {
            long double min_real;
            long double max_real;

            if(sql_args.range_type == DATA_RANGE) {
                ia >> min_real;
                ia >> max_real;
                // cout << "min_real: " << min_real << " max_real: " << max_real << endl;
            }
            else if(sql_args.range_type == DATA_MAX || sql_args.range_type == DATA_MIN) {
                ia >> min_real;
                max_real = min_real;
                // cout << "val_real: " << min_real << endl;
            }

            rc = md_catalog_all_timesteps_with_var_attributes_with_type_var_substr_range_stub(sql_args, 
                min_real, max_real, entries, count);

            break;
        }
    }    
    add_timing_point(OP_CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_RANGE_MD_CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_RANGE_STUB);


    std::string serial_str = serializeMsgToClient(entries, count, rc);
    add_timing_point(OP_CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_RANGE_SERIALIZE_MSG_FOR_CLIENT);

    createOutgoingMessage(dst, 
                          0, //Not expecting a reply
                          dst_mailbox,
                          serial_str);
    add_timing_point(OP_CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_RANGE_CREATE_MSG_FOR_CLIENT);

    opbox::net::SendMsg(peer, ldo_msg);
    state=State::done;
    add_timing_point(OP_CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_RANGE_SEND_MSG_TO_CLIENT_OP_DONE);
    return WaitingType::done_and_destroy;

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


    rc = get_matching_timestep_count (args, min, max, query_str, count); //assert (rc == RC_OK);

    if (count > 0) {
        entries.reserve(count);

        rc = sqlite3_prepare_v2 (db, query.c_str(), -1, &stmt, &tail); 
        if (rc != SQLITE_OK)
        {
            fprintf (stderr, "Error catalog_all_timesteps_with_var_attributes_with_type_var_substr_range stub: Line: %d SQL error (%d): %s\n", __LINE__, rc, sqlite3_errmsg (db));
            sqlite3_close (db);
            goto cleanup;
        }
        // rc = sqlite3_prepare_v2 (db, query.c_str(), -1, &stmt, &tail); //assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int64 (stmt, 1, args.txn_id); //assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int64 (stmt, 2, args.txn_id); //assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int64 (stmt, 3, args.type_id); //assert (rc == SQLITE_OK);
        rc = sqlite3_bind_text (stmt, 4, strdup(args.var_name_substr.c_str()), -1, free); //assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int64 (stmt, 5, args.run_id); //assert (rc == SQLITE_OK);

        rc = sqlite3_bind_int (stmt, 6, ATTR_DATA_TYPE_INT); //assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int (stmt, 7, ATTR_DATA_TYPE_REAL); //assert (rc == SQLITE_OK);

        switch(args.data_type) {
            case ATTR_DATA_TYPE_INT : {
                rc = sqlite3_bind_int64 (stmt, (uint64_t)8, min); //assert (rc == SQLITE_OK);
                if(args.range_type == DATA_RANGE) {
                    rc = sqlite3_bind_int64 (stmt, (uint64_t)9, max); //assert (rc == SQLITE_OK);
                }
                break;
            }
            case ATTR_DATA_TYPE_REAL : {
                rc = sqlite3_bind_double (stmt, (long double)8, min); //assert (rc == SQLITE_OK);
                if(args.range_type == DATA_RANGE) {
                    rc = sqlite3_bind_double (stmt, (long double)9, max); //assert (rc == SQLITE_OK);
                }
                break;
            }
        }

        rc = sql_retrieve_timesteps(stmt, entries);

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

    rc = sqlite3_prepare_v2 (db, query.c_str(), -1, &stmt, &tail); //assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 1, args.txn_id); //assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 2, args.txn_id); //assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 3, args.type_id); //assert (rc == SQLITE_OK);
    rc = sqlite3_bind_text (stmt, 4, strdup(args.var_name_substr.c_str()), -1, free); //assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 5, args.run_id); //assert (rc == SQLITE_OK);

    rc = sqlite3_bind_int (stmt, 6, ATTR_DATA_TYPE_INT); //assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int (stmt, 7, ATTR_DATA_TYPE_REAL); //assert (rc == SQLITE_OK);

    switch(args.data_type) {
        case ATTR_DATA_TYPE_INT : {
            rc = sqlite3_bind_int64 (stmt, (uint64_t)8, min); //assert (rc == SQLITE_OK);
            if(args.range_type == DATA_RANGE) {
                rc = sqlite3_bind_int64 (stmt, (uint64_t)9, max); //assert (rc == SQLITE_OK);
            }
            break;
        }
        case ATTR_DATA_TYPE_REAL : {
            rc = sqlite3_bind_double (stmt, (long double)8, min); //assert (rc == SQLITE_OK);
            if(args.range_type == DATA_RANGE) {
                rc = sqlite3_bind_double (stmt, (long double)9, max); //assert (rc == SQLITE_OK);
            }
            break;
        }
    }

    rc = sqlite3_step (stmt); //assert (rc == SQLITE_ROW || rc == SQLITE_DONE);
    count = sqlite3_column_int64 (stmt, 0);
    rc = sqlite3_finalize (stmt); //assert (rc == SQLITE_OK);

    // cout << "count: " << count << endl;

    return rc;
}
