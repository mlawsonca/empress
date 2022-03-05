#include <OpCatalogAllTimestepsWithVarMetaCommon.hh>
#include <server_timing_constants_new.hh>
#include <sql_helper_functs.hh>
#include <OpCoreServer.hh>

extern sqlite3 *db;

using namespace std;

int md_catalog_all_timesteps_with_var_stub (md_catalog_all_timesteps_with_var_args &args, std::vector<md_catalog_timestep_entry> &entries, uint32_t &count);


WaitingType OpCatalogAllTimestepsWithVarMeta::handleMessage(bool rdma, message_t *incoming_msg) {
  

    md_catalog_all_timesteps_with_var_args sql_args;
    uint32_t count;    
    std::vector<md_catalog_timestep_entry> entries;

    //convert the serialized string back into the args and count pointer
    deArchiveMsgFromClient(rdma, incoming_msg, sql_args); 
    add_timing_point(OP_CATALOG_ALL_TIMESTEPS_WITH_VAR_DEARCHIVE_MSG_FROM_CLIENT);

    int rc = md_catalog_all_timesteps_with_var_stub(sql_args, entries, count);
    add_timing_point(OP_CATALOG_ALL_TIMESTEPS_WITH_VAR_MD_CATALOG_ALL_TIMESTEPS_WITH_VAR_STUB);

    //log("num of entries is " + std::to_string(entries.size()));

    std::string serial_str = serializeMsgToClient(entries, count, rc);
    add_timing_point(OP_CATALOG_ALL_TIMESTEPS_WITH_VAR_SERIALIZE_MSG_FOR_CLIENT);

    createOutgoingMessage(dst, 
                          0, //Not expecting a reply
                          dst_mailbox,
                          serial_str);
    add_timing_point(OP_CATALOG_ALL_TIMESTEPS_WITH_VAR_CREATE_MSG_FOR_CLIENT);
    
    opbox::net::SendMsg(peer, ldo_msg);
    state=State::done;
    add_timing_point(OP_CATALOG_ALL_TIMESTEPS_WITH_VAR_SEND_MSG_TO_CLIENT_OP_DONE);
    return WaitingType::done_and_destroy;

}



int md_catalog_all_timesteps_with_var_stub (md_catalog_all_timesteps_with_var_args &args, 
                     std::vector<md_catalog_timestep_entry> &entries,
                     uint32_t &count
                     )
{
    int rc;
    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;
    const char * query = "select tmc.* from timestep_catalog tmc "
    "inner join var_catalog vc on tmc.run_id = vc.run_id and tmc.id = vc.timestep_id "
    "where (tmc.txn_id = ? or tmc.active = 1) "
    "and (vc.txn_id = ? or vc.active = 1) "
    "and tmc.run_id = ? "
    "and vc.id = ? "
    "group by tmc.id, tmc.run_id "; 
    // "group by id, run_id "; 

    size_t size = 0;

    // rc = sqlite3_prepare_v2 (db, "select count(distinct timestep_catalog.id timestep_catalog.run_id) from timestep_catalog tmc "
    //   "inner join var_catalog vc on tmc.run_id = vc.run_id and tmc.id = vc.timestep_id "
    //   "where (tmc.txn_id = ? or tmc.active = 1) "
    //   "and (vc.txn_id = ? or vc.active = 1) "
    //   "and tmc.run_id = ? "
    //   "and vc.id = ? ", -1, &stmt, &tail); 
      rc = sqlite3_prepare_v2 (db, "select count(*) from (select distinct tmc.id, tmc.run_id from timestep_catalog tmc "
      "inner join var_catalog vc on tmc.run_id = vc.run_id and tmc.id = vc.timestep_id "
      "where (tmc.txn_id = ? or tmc.active = 1) "
      "and (vc.txn_id = ? or vc.active = 1) "
      "and tmc.run_id = ? "
      "and vc.id = ? ) as internalQuery", -1, &stmt, &tail); 
          if (rc != SQLITE_OK)
        {
            fprintf (stderr, "Error count md_catalog_all_timesteps_with_var_stub: Line: %d SQL error (%d): %s\n", __LINE__, rc, sqlite3_errmsg (db));
            sqlite3_close (db);
            goto cleanup;
        }
    // //assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 1, args.txn_id); //assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 2, args.txn_id); //assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 3, args.run_id); //assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 4, args.var_id); //assert (rc == SQLITE_OK);
    rc = sqlite3_step (stmt); //assert (rc == SQLITE_ROW || rc == SQLITE_DONE);
    count = sqlite3_column_int64 (stmt, 0);
    rc = sqlite3_finalize (stmt); //assert (rc == SQLITE_OK);



//printf ("rows in run_catalog: %d\n", count);
    if (count > 0) {
      entries.reserve(count);

        rc = sqlite3_prepare_v2 (db, query, -1, &stmt, &tail); //assert (rc == SQLITE_OK);
        if (rc != SQLITE_OK)
        {
            fprintf (stderr, "Error md_catalog_all_timesteps_with_var_stub: Line: %d SQL error (%d): %s\n", __LINE__, rc, sqlite3_errmsg (db));
            sqlite3_close (db);
            goto cleanup;
        }
        rc = sqlite3_bind_int64 (stmt, 1, args.txn_id); //assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int64 (stmt, 2, args.txn_id); //assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int64 (stmt, 3, args.run_id); //assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int64 (stmt, 4, args.var_id); //assert (rc == SQLITE_OK);

        rc = sql_retrieve_timesteps(stmt, entries);

        rc = sqlite3_finalize (stmt);  
    }
    

cleanup:

    return rc;
}
