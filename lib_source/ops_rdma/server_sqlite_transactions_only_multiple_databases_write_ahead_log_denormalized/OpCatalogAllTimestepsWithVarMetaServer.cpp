#include <OpCatalogAllTimestepsWithVarMetaCommon.hh>
#include <server_timing_constants_new.hh>
#include <sql_helper_functs.hh>
#include <OpCoreServer.hh>

extern sqlite3 * get_database(uint64_t txn_id);
extern sqlite3 *main_db;


using namespace std;

int md_catalog_all_timesteps_with_var_stub (sqlite3 *db, const md_catalog_all_timesteps_with_var_args &args, std::vector<md_catalog_timestep_entry> &entries, uint32_t &count);


WaitingType OpCatalogAllTimestepsWithVarMeta::handleMessage(bool rdma, message_t *incoming_msg) {
  
    int rc;
    md_catalog_all_timesteps_with_var_args sql_args;
    uint32_t count;    
    std::vector<md_catalog_timestep_entry> entries;

    //convert the serialized string back into the args and count pointer
    deArchiveMsgFromClient(rdma, incoming_msg, sql_args);
    add_timing_point(OP_CATALOG_ALL_TIMESTEPS_WITH_VAR_DEARCHIVE_MSG_FROM_CLIENT);

    if(sql_args.query_type == COMMITTED) {
        rc = md_catalog_all_timesteps_with_var_stub(main_db, sql_args, entries, count);
    }
    else if(sql_args.query_type == UNCOMMITTED) {
        sqlite3 *db = get_database(sql_args.txn_id);
        rc = md_catalog_all_timesteps_with_var_stub(db, sql_args, entries, count);
    }
    else if(sql_args.query_type == COMMITTED_AND_UNCOMMITTED) {
        uint32_t temp_count;
        
        sqlite3 *db = get_database(sql_args.txn_id);

        rc = md_catalog_all_timesteps_with_var_stub(main_db, sql_args, entries, count);
        if(rc == RC_OK) {
            rc = md_catalog_all_timesteps_with_var_stub(db, sql_args, entries, temp_count);
            count += temp_count;
        }
    }
    else {
        cout << "ERROR. Query type " << sql_args.query_type << " is not defined" << endl;
    }
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



int md_catalog_all_timesteps_with_var_stub (sqlite3 *db, const md_catalog_all_timesteps_with_var_args &args, 
                     std::vector<md_catalog_timestep_entry> &entries,
                     uint32_t &count
                     )
{
    int rc;
    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;
    const char * query = "select tmc.* from timestep_catalog tmc "
    "inner join var_catalog vc on tmc.run_id = vc.run_id and tmc.id = vc.timestep_id "
    "where tmc.run_id = ? "
    "and vc.id = ? "
    "group by tmc.id, tmc.run_id "; 
    // "group by id, run_id "; 

    size_t size = 0;

      rc = sqlite3_prepare_v2 (db, "select count(*) from (select distinct tmc.id, tmc.run_id from timestep_catalog tmc "
      "inner join var_catalog vc on tmc.run_id = vc.run_id and tmc.id = vc.timestep_id "
      "where tmc.run_id = ? "
      "and vc.id = ? ) as internalQuery", -1, &stmt, &tail); 
          if (rc != SQLITE_OK)
        {
            fprintf (stderr, "Error count md_catalog_all_timesteps_with_var_stub: Line: %d SQL error (%d): %s\n", __LINE__, rc, sqlite3_errmsg (db));
            sqlite3_close (db);
            goto cleanup;
        }
    // //assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 1, args.run_id); //assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 2, args.var_id); //assert (rc == SQLITE_OK);
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
        rc = sqlite3_bind_int64 (stmt, 1, args.run_id); //assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int64 (stmt, 2, args.var_id); //assert (rc == SQLITE_OK);

        rc = sql_retrieve_timesteps(stmt, entries);

        rc = sqlite3_finalize (stmt);  
    }
    

cleanup:

    return rc;
}
