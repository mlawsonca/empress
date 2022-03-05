#include <OpCatalogVarMetaCommon.hh>
#include <server_timing_constants_new.hh>
#include <sql_helper_functs.hh>
#include <OpCoreServer.hh>

extern sqlite3 * get_database(uint64_t job_id, md_query_type query_type, bool &trans_active);
extern void close_database(uint64_t job_id);




using namespace std;

int md_catalog_var_stub (sqlite3 *db, const md_catalog_var_args &args, std::vector<md_catalog_var_entry> &entries, uint32_t &count);
// extern int callback (void * NotUsed, int argc, char ** argv, char ** ColName);


WaitingType OpCatalogVarMeta::handleMessage(bool rdma, message_t *incoming_msg) {
  
int rc;
    
    md_catalog_var_args args;
    std::vector<md_catalog_var_entry> entries;
    uint32_t count = 0;    
    bool trans_active;

    deArchiveMsgFromClient(rdma, incoming_msg, args);
    add_timing_point(OP_CATALOG_VAR_DEARCHIVE_MSG_FROM_CLIENT);

    sqlite3 *db = get_database(args.job_id, args.query_type, trans_active);
    if(!trans_active) {
        rc = md_catalog_var_stub(db, args, entries, count);
    }
    else {
        rc = RC_DB_BUSY;
    }

    add_timing_point(OP_CATALOG_VAR_MD_CATALOG_VAR_STUB);

    //log("num of entries is " + std::to_string(entries.size()));
    // if(entries.size()>0) {
    //     //log("in md_catalog_var_stub the first entry has name " + entries.at(0).name);

    // }

    std::string serial_str = serializeMsgToClient(entries, count, rc);
    add_timing_point(OP_CATALOG_VAR_SERIALIZE_MSG_FOR_CLIENT);

    createOutgoingMessage(dst, 
                          0, //Not expecting a reply
                          dst_mailbox,
                          serial_str);
    add_timing_point(OP_CATALOG_VAR_CREATE_MSG_FOR_CLIENT);
    
    opbox::net::SendMsg(peer, ldo_msg);
    state=State::done;
    add_timing_point(OP_CATALOG_VAR_SEND_MSG_TO_CLIENT_OP_DONE);
    return WaitingType::done_and_destroy;

}




int md_catalog_var_stub (sqlite3 *db, const md_catalog_var_args &args, 
                     std::vector<md_catalog_var_entry> &entries,
                     uint32_t &count
                     )
{
    int rc;
    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;

    const char * query = "select * from var_catalog where run_id = ? and timestep_id = ? "
    "order by id";

    // "order by path, name, version";

    rc = sqlite3_prepare_v2 (db, "select count (*) from var_catalog where run_id = ? and timestep_id = ?  ", -1, &stmt, &tail); //assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 1, args.run_id); //assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 2, args.timestep_id); //assert (rc == SQLITE_OK);
    rc = sqlite3_step (stmt); //assert (rc == SQLITE_ROW || rc == SQLITE_DONE);
    count = sqlite3_column_int64 (stmt, 0);
    rc = sqlite3_finalize (stmt); //assert (rc == SQLITE_OK);

    // char *err_msg = 0;
    // char *sql = "SELECT * FROM var_catalog";
    // rc = sqlite3_exec(db, sql, callback, 0, &err_msg);

    // cout << " run_id: " << args.run_id << " timestep_id: " << args.timestep_id << endl;
    // cout << "count: " << count << endl;

    if (count > 0) {
        entries.reserve(count);

        rc = sqlite3_prepare_v2 (db, query, -1, &stmt, &tail); 
        if (rc != SQLITE_OK)
        {
            fprintf (stderr, "Error md_catalog_var_stub: Line: %d SQL error (%d): %s\n", __LINE__, rc, sqlite3_errmsg (db));
            close_database(args.job_id);
            goto cleanup;
        }
        rc = sqlite3_bind_int64 (stmt, 1, args.run_id); //assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int64 (stmt, 2, args.timestep_id); //assert (rc == SQLITE_OK);
    
        rc = sql_retrieve_vars(stmt, entries);

        rc = sqlite3_finalize (stmt);  


    // char *err_msg = 0;
    // char *sql = "SELECT * FROM var_catalog";
    // cout << "starting var catalog, count: " << count << " \n";
    // rc = sqlite3_exec(db, sql, callback, 0, &err_msg);
    // cout << "done with var catalog \n";
    }
    

cleanup:

    return rc;
}



