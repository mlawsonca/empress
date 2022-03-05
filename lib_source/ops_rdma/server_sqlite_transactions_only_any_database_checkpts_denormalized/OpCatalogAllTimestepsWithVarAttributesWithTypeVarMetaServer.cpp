#include <OpCatalogAllTimestepsWithVarAttributesWithTypeVarMetaCommon.hh>
#include <server_timing_constants_new.hh>
#include <sql_helper_functs.hh>
#include <OpCoreServer.hh>

extern sqlite3 * get_database(uint64_t job_id, md_query_type query_type, bool &trans_active);
extern void close_database(uint64_t job_id);
extern int callback (void * NotUsed, int argc, char ** argv, char ** ColName);


using namespace std;

int md_catalog_all_timesteps_with_var_attributes_with_type_var_stub (sqlite3 *db, const md_catalog_all_timesteps_with_var_attributes_with_type_var_args &args,
                           std::vector<md_catalog_timestep_entry> &entries,
                           uint32_t &count);

static int get_matching_timestep_count (sqlite3 *db, const md_catalog_all_timesteps_with_var_attributes_with_type_var_args &args, uint32_t &count);



WaitingType OpCatalogAllTimestepsWithVarAttributesWithTypeVarMeta::handleMessage(bool rdma, message_t *incoming_msg) {
  
    int rc;
    md_catalog_all_timesteps_with_var_attributes_with_type_var_args args;
    std::vector<md_catalog_timestep_entry> entries;
    uint32_t count = 0;
    bool trans_active;

    deArchiveMsgFromClient(rdma, incoming_msg, args);
    add_timing_point(OP_CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_DEARCHIVE_MSG_FROM_CLIENT);

    sqlite3 *db = get_database(args.job_id, args.query_type, trans_active);
    if(!trans_active) {
        rc = md_catalog_all_timesteps_with_var_attributes_with_type_var_stub(db, args, entries, count);
    }
    else {
        rc = RC_DB_BUSY;
    }  
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





int md_catalog_all_timesteps_with_var_attributes_with_type_var_stub (sqlite3 *db, const md_catalog_all_timesteps_with_var_attributes_with_type_var_args &args,
                           std::vector<md_catalog_timestep_entry> &entries,
                           uint32_t &count)
{
    int rc;
    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;
    const char * query = "select tmc.* from timestep_catalog tmc "
    "inner join var_attribute_catalog vac on vac.timestep_id = tmc.id and vac.run_id = tmc.run_id "
    "where vac.type_id = ? "
    "and vac.var_id = ? "
    "and tmc.run_id = ? "
    "group by tmc.id, tmc.run_id";

    // char *err_msg = 0;
    // char * sql = "SELECT * FROM var_attribute_catalog where type_id=3 and var_id=8";
    // cout << "starting var_attribute_catalog \n";
    // rc = sqlite3_exec(db, sql, callback, 0, &err_msg);
    // cout << "done with var_attribute_catalog \n";

    // char *sql = "SELECT * FROM timestep_catalog";
    // cout << "starting timestep_catalog \n";
    // rc = sqlite3_exec(db, sql, callback, 0, &err_msg);
    // cout << "done with timestep_catalog \n";

    // sql = "SELECT * FROM timestep_catalog";
    // cout << "starting timestep_catalog \n";
    // rc = sqlite3_exec(db, sql, callback, 0, &err_msg);
    // cout << "done with timestep_catalog \n";

    rc = get_matching_timestep_count (db, args, count); //assert (rc == RC_OK);

    if (count > 0) {
        entries.reserve(count);

        rc = sqlite3_prepare_v2 (db, query, -1, &stmt, &tail); 
        if (rc != SQLITE_OK)
        {
            fprintf (stderr, "Error catalog_all_timesteps_with_var_attributes_with_type_var stub: Line: %d SQL error (%d): %s\n", __LINE__, rc, sqlite3_errmsg (db));
            close_database(args.job_id);
            goto cleanup;
        }
        // rc = sqlite3_prepare_v2 (db, query, -1, &stmt, &tail); //assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int64 (stmt, 1, args.type_id); //assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int64 (stmt, 2, args.var_id); //assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int64 (stmt, 3, args.run_id); //assert (rc == SQLITE_OK);

        rc = sql_retrieve_timesteps(stmt, entries);

        rc = sqlite3_finalize (stmt);  
    }
        

cleanup:

    return rc;
}



static int get_matching_timestep_count (sqlite3 *db, const md_catalog_all_timesteps_with_var_attributes_with_type_var_args &args, uint32_t &count)
{
    int rc;
    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;
    const char * query = "select count(*) from (select distinct tmc.id, tmc.run_id from timestep_catalog tmc "
    "inner join var_attribute_catalog vac on vac.timestep_id = tmc.id and vac.run_id = tmc.run_id "
    "where vac.type_id = ? "
    "and vac.var_id = ? "
    "and tmc.run_id = ? ) as internalQuery";

    rc = sqlite3_prepare_v2 (db, query, -1, &stmt, &tail); //assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 1, args.type_id); //assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 2, args.var_id); //assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 3, args.run_id); //assert (rc == SQLITE_OK);

    rc = sqlite3_step (stmt); //assert (rc == SQLITE_ROW || rc == SQLITE_DONE);
    // sqlite3_extended_result_codes(db, 1);
    // string msg =  "rc: " + to_string(rc) + ", errMsg: " + sqlite3_errmsg (db);
    // cout << "debug info: " << msg.c_str() << endl;
    // cout << "type_id: " <<  args.type_id << endl;
    // cout << "var_id: " <<  args.var_id << endl;
    // cout << "run_id: " << args.run_id << endl;

    count = sqlite3_column_int64 (stmt, 0);
    rc = sqlite3_finalize (stmt); //assert (rc == SQLITE_OK);

    // cout << "count: " << count << endl;

    return rc;
}
