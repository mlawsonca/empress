#include <OpCatalogVarMetaCommon.hh>
#include <server_timing_constants_new.hh>
#include <sql_helper_functs.hh>
#include <OpCoreServer.hh>

extern sqlite3 * get_database(uint64_t txn_id);
extern sqlite3 *main_db;


using namespace std;

int md_catalog_var_stub (sqlite3 *db, const md_catalog_var_args &args, std::vector<md_catalog_var_entry> &entries, uint32_t &count);
// extern int callback (void * NotUsed, int argc, char ** argv, char ** ColName);


WaitingType OpCatalogVarMeta::handleMessage(bool rdma, message_t *incoming_msg) {
  
int rc;
    
    md_catalog_var_args sql_args;
    uint32_t count;    
    std::vector<md_catalog_var_entry> entries;

    //convert the serialized string back into the args and count pointer
    deArchiveMsgFromClient(rdma, incoming_msg, sql_args);
    add_timing_point(OP_CATALOG_VAR_DEARCHIVE_MSG_FROM_CLIENT);

    if(sql_args.query_type == COMMITTED) {
        rc = md_catalog_var_stub(main_db, sql_args, entries, count);
    }
    else if(sql_args.query_type == UNCOMMITTED) {
        sqlite3 *db = get_database(sql_args.txn_id);
        rc = md_catalog_var_stub(db, sql_args, entries, count);
    }
    else if(sql_args.query_type == COMMITTED_AND_UNCOMMITTED) {
        uint32_t temp_count;
        
        sqlite3 *db = get_database(sql_args.txn_id);

        rc = md_catalog_var_stub(main_db, sql_args, entries, count);
        if(rc == RC_OK) {
            rc = md_catalog_var_stub(db, sql_args, entries, temp_count);
            count += temp_count;
        }
    }
    else {
        cout << "ERROR. Query type " << sql_args.query_type << " is not defined" << endl;
    }
    add_timing_point(OP_CATALOG_VAR_MD_CATALOG_VAR_STUB);

    //log("num of entries is " + std::to_string(entries.size()));
    if(entries.size()>0) {
        //log("in md_catalog_var_stub the first entry has name " + entries.at(0).name);

    }

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
            sqlite3_close (db);
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



