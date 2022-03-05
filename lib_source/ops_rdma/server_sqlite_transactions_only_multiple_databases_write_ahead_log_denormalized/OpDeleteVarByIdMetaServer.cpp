#include <OpDeleteVarByIdMetaCommon.hh>
#include <server_timing_constants_new.hh>
#include <sql_helper_functs.hh>
#include <OpCoreServer.hh>

extern sqlite3 * get_database(uint64_t txn_id);
extern sqlite3 *main_db;


using namespace std;

int md_delete_var_by_id_stub (sqlite3 *db, const md_delete_var_by_id_args &args);
extern int callback (void * NotUsed, int argc, char ** argv, char ** ColName);


WaitingType OpDeleteVarByIdMeta::handleMessage(bool rdma, message_t *incoming_msg) {
  
    int rc;
    md_delete_var_by_id_args sql_args;

    
    deArchiveMsgFromClient(rdma, incoming_msg, sql_args);
    add_timing_point(OP_DELETE_VAR_BY_ID_DEARCHIVE_MSG_FROM_CLIENT);

    if(sql_args.query_type == COMMITTED) {
        rc = md_delete_var_by_id_stub(main_db, sql_args);
    }
    else if(sql_args.query_type == UNCOMMITTED) {
        sqlite3 *db = get_database(sql_args.txn_id);
        rc = md_delete_var_by_id_stub(db, sql_args);
    }
    else if(sql_args.query_type == COMMITTED_AND_UNCOMMITTED) {        
        sqlite3 *db = get_database(sql_args.txn_id);

        rc = md_delete_var_by_id_stub(main_db, sql_args);
        if(rc == RC_OK) {
            rc = md_delete_var_by_id_stub(db, sql_args);
        }
    }
    else {
        cout << "ERROR. Query type " << sql_args.query_type << " is not defined" << endl;
    }
    add_timing_point(OP_DELETE_VAR_BY_ID_MD_DELETE_VAR_BY_ID_STUB);

    // std::string serial_str = serializeMsgToClient(rc);
    add_timing_point(OP_DELETE_VAR_BY_ID_SERIALIZE_MSG_FOR_CLIENT);

    createOutgoingMessage(dst, 
                          0, //Not expecting a reply
                          dst_mailbox,
                          to_string(rc));
    add_timing_point(OP_DELETE_VAR_BY_ID_CREATE_MSG_FOR_CLIENT);
 
    opbox::net::SendMsg(peer, ldo_msg);
    state=State::done;
    add_timing_point(OP_DELETE_VAR_BY_ID_SEND_MSG_TO_CLIENT_OP_DONE);
    return WaitingType::done_and_destroy;

}





int md_delete_var_by_id_stub (sqlite3 *db, const md_delete_var_by_id_args &args)
{
    int rc = RC_OK;
    char * ErrMsg = NULL;
    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;

    const char * query1 = "delete from var_attribute_catalog "
    "where var_id = ? "
    "and run_id = ? "
    "and timestep_id = ? ";
    const char * query2 = "delete from var_catalog where id = ? and run_id = ? and timestep_id = ? ";



    // rc = sqlite3_exec (db, "begin;", callback, 0, &ErrMsg);
    // if (rc != SQLITE_OK)
    // {
    //     fprintf (stderr, "Error begin delete_var_by_id_stub: Line: %d SQL error: %s\n", __LINE__, ErrMsg);
    //     sqlite3_free (ErrMsg);
    //     sqlite3_close (db);
    //     goto cleanup;
    // }
    rc = sqlite3_exec (db, "savepoint savept_delete;", callback, 0, &ErrMsg);
    if (rc != SQLITE_OK)
    {
        fprintf (stderr, "Error begin delete_timestep_by_id_stub: Line: %d SQL error: %s\n", __LINE__, ErrMsg);
        sqlite3_free (ErrMsg);
        sqlite3_close (db);
        goto cleanup;
    }
    
    rc = sqlite3_prepare_v2 (db, query1, -1, &stmt, &tail);
    if (rc != SQLITE_OK)
    {
        fprintf (stderr, "Error first query delete_var_by_id_stub: Line: %d SQL error: %s\n", __LINE__, sqlite3_errmsg (db));
        sqlite3_close (db);
        goto cleanup;
    }

    rc = sqlite3_bind_int64 (stmt, 1, args.var_id); //assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 2, args.run_id); //assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 3, args.timestep_id); //assert (rc == SQLITE_OK);
    rc = sqlite3_step (stmt); //assert (rc == SQLITE_ROW || rc == SQLITE_DONE);

    rc = sqlite3_finalize (stmt);

    rc = sqlite3_prepare_v2 (db, query2, -1, &stmt, &tail);
    if (rc != SQLITE_OK)
    {
        fprintf (stderr, "Error second query delete_var_by_id_stub: Line: %d SQL error: %s\n", __LINE__, sqlite3_errmsg (db));
        sqlite3_close (db);
        goto cleanup;
    }

    rc = sqlite3_bind_int64 (stmt, 1, args.var_id); //assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 2, args.run_id); //assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 3, args.timestep_id); //assert (rc == SQLITE_OK);
    rc = sqlite3_step (stmt); //assert (rc == SQLITE_ROW || rc == SQLITE_DONE);

    rc = sqlite3_finalize (stmt);

    // rc = sqlite3_exec (db, "end;", callback, 0, &ErrMsg);
    // if (rc != SQLITE_OK)
    // {
    //     fprintf (stderr, "Error end delete_var_by_id_stub: Line: %d SQL error: %s\n", __LINE__, ErrMsg);
    //     sqlite3_free (ErrMsg);
    //     sqlite3_close (db);
    //     goto cleanup;
    // }

cleanup:
    // if (rc != SQLITE_OK)
    // {
    //     rc = sqlite3_exec (db, "rollback;", callback, 0, &ErrMsg);
    // }
    if (rc == SQLITE_OK)
    {
        rc = sqlite3_exec (db, "release savept_delete;", callback, 0, &ErrMsg);
    }
    else {
        rc = sqlite3_exec (db, "rollback to savept_delete;", callback, 0, &ErrMsg);
    }

    return rc;
}
