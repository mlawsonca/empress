#include <OpCatalogAllTimestepsWithVarAttributesWithTypeVarMetaCommon.hh>
#include <server_timing_constants_new.hh>
#include <sql_helper_functs.hh>
#include <OpCoreServer.hh>

extern sqlite3 *db;

using namespace std;

int md_catalog_all_timesteps_with_var_attributes_with_type_var_stub (const md_catalog_all_timesteps_with_var_attributes_with_type_var_args &args,
                           std::vector<md_catalog_timestep_entry> &entries,
                           uint32_t &count);

static int get_matching_timestep_count (const md_catalog_all_timesteps_with_var_attributes_with_type_var_args &args, uint32_t &count);



WaitingType OpCatalogAllTimestepsWithVarAttributesWithTypeVarMeta::handleMessage(bool rdma, message_t *incoming_msg) {
  

    md_catalog_all_timesteps_with_var_attributes_with_type_var_args sql_args;
    std::vector<md_catalog_timestep_entry> entries;
    uint32_t count;
    

    deArchiveMsgFromClient(rdma, incoming_msg, sql_args);        
    add_timing_point(OP_CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_DEARCHIVE_MSG_FROM_CLIENT);

    int rc = md_catalog_all_timesteps_with_var_attributes_with_type_var_stub(sql_args, entries, count);
    add_timing_point(OP_CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_MD_CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_STUB);


    std::string serial_str = serializeMsgToClient(entries, count, rc);
    add_timing_point(OP_CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_SERIALIZE_MSG_FOR_CLIENT);

    createOutgoingMessage(dst, 
                          0, //Not expecting a reply
                          dst_mailbox,
                          serial_str);
    add_timing_point(OP_CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_CREATE_MSG_FOR_CLIENT);

    opbox::net::SendMsg(peer, ldo_msg);
    state=State::done;
    add_timing_point(OP_CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_SEND_MSG_TO_CLIENT_OP_DONE);
    return WaitingType::done_and_destroy;

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

    rc = get_matching_timestep_count (args, count); //assert (rc == RC_OK);

    if (count > 0) {
        entries.reserve(count);

        rc = sqlite3_prepare_v2 (db, query, -1, &stmt, &tail); 
        if (rc != SQLITE_OK)
        {
            fprintf (stderr, "Error catalog_all_timesteps_with_var_attributes_with_type_var stub: Line: %d SQL error (%d): %s\n", __LINE__, rc, sqlite3_errmsg (db));
            sqlite3_close (db);
            goto cleanup;
        }
        // rc = sqlite3_prepare_v2 (db, query, -1, &stmt, &tail); //assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int64 (stmt, 1, args.txn_id); //assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int64 (stmt, 2, args.txn_id); //assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int64 (stmt, 3, args.type_id); //assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int64 (stmt, 4, args.var_id); //assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int64 (stmt, 5, args.run_id); //assert (rc == SQLITE_OK);

        rc = sql_retrieve_timesteps(stmt, entries);

        rc = sqlite3_finalize (stmt);  
    }
        

cleanup:

    return rc;
}



static int get_matching_timestep_count (const md_catalog_all_timesteps_with_var_attributes_with_type_var_args &args, uint32_t &count)
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

    rc = sqlite3_prepare_v2 (db, query, -1, &stmt, &tail); //assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 1, args.txn_id); //assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 2, args.txn_id); //assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 3, args.type_id); //assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 4, args.var_id); //assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 5, args.run_id); //assert (rc == SQLITE_OK);

    rc = sqlite3_step (stmt); //assert (rc == SQLITE_ROW || rc == SQLITE_DONE);
     // sqlite3_extended_result_codes(db, 1);
    // string msg =  "rc: " + to_string(rc) + ", errMsg: " + sqlite3_errmsg (db);
    // cout << "debug info: " << msg.c_str() << endl;
    // cout << "txn_id: " <<  args.txn_id << endl;
    // cout << "type_id: " <<  args.type_id << endl;
    // cout << "var_id: " <<  args.var_id << endl;
    // cout << "run_id: " << args.run_id << endl;

    count = sqlite3_column_int64 (stmt, 0);
    rc = sqlite3_finalize (stmt); //assert (rc == SQLITE_OK);

    // cout << "count: " << count << endl;

    return rc;
}
