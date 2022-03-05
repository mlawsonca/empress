#include <OpDeleteRunByIdMetaCommon.hh>
#include <server_timing_constants_new.hh>
#include <sql_helper_functs.hh>
#include <OpCoreServer.hh>

extern sqlite3 * get_database(uint64_t job_id);
extern void close_database(uint64_t job_id);




using namespace std;

int md_delete_run_by_id_stub (sqlite3 *db, const md_delete_run_by_id_args &args);
extern int callback (void * NotUsed, int argc, char ** argv, char ** ColName);


WaitingType OpDeleteRunByIdMeta::handleMessage(bool rdma, message_t *incoming_msg) {
  
    int rc;
    md_delete_run_by_id_args args;

    deArchiveMsgFromClient(rdma, incoming_msg, args);
    add_timing_point(OP_DELETE_RUN_BY_ID_DEARCHIVE_MSG_FROM_CLIENT);

    sqlite3 *db = get_database(args.job_id);
    rc = md_delete_run_by_id_stub(db, args);

    add_timing_point(OP_DELETE_RUN_BY_ID_MD_DELETE_RUN_BY_ID_STUB);

    // std::string serial_str = serializeMsgToClient(rc);
    add_timing_point(OP_DELETE_RUN_BY_ID_SERIALIZE_MSG_FOR_CLIENT);

    createOutgoingMessage(dst, 
                          0, //Not expecting a reply
                          dst_mailbox,
                          to_string(rc));
    add_timing_point(OP_DELETE_RUN_BY_ID_CREATE_MSG_FOR_CLIENT);
 
    opbox::net::SendMsg(peer, ldo_msg);
    state=State::done;
    add_timing_point(OP_DELETE_RUN_BY_ID_SEND_MSG_TO_CLIENT_OP_DONE);
    return WaitingType::done_and_destroy;

}





int md_delete_run_by_id_stub (sqlite3 *db, const md_delete_run_by_id_args &args)
{
    int rc = RC_OK;
    char * ErrMsg = NULL;
    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;
    //fix - would we ever want to delete a run but not its vars/types/attrs?
    // const char * query1 = "delete from run_attribute_catalog where run_attribute_catalog.id in (select rac.id from run_attribute_catalog rac "
    //     "inner join type_catalog tc on rac.type_id = tc.id "
    //     "where tc.run_id = ? ) ";
    // const char * query1 = "delete from run_attribute_catalog where run_attribute_catalog.id in (select rac.id from run_attribute_catalog rac "
    //     "where rac.run_id = ? ) ";
    const char * query1 = "delete from run_attribute_catalog where run_id = ? ";

    const char * query2 = "delete from timestep_attribute_catalog where run_id = ? ";

    const char * query3 = "delete from var_attribute_catalog where run_id = ? ";

    const char * query4 = "delete from type_catalog where run_id = ? ";

    const char * query5 = "delete from var_catalog where run_id = ? ";

    const char * query6 = "delete from timestep_catalog where run_id = ? ";

    const char * query7 = "delete from run_catalog where id = ?  ";

    // rc = sqlite3_exec (db, "begin;", callback, 0, &ErrMsg);
    // if (rc != SQLITE_OK)
    // {
    //     fprintf (stderr, "Error begin delete_run_by_id_stub: Line: %d SQL error: %s\n", __LINE__, ErrMsg);
    //     sqlite3_free (ErrMsg);
    //     close_database(args.job_id);
    //     goto cleanup;
    // }

    rc = sqlite3_prepare_v2 (db, query1, -1, &stmt, &tail);
    if (rc != SQLITE_OK)
    {
        fprintf (stderr, "Error first query delete_run_by_id_stub: Line: %d SQL error: %s\n", __LINE__, ErrMsg);
        sqlite3_free (ErrMsg);
        close_database(args.job_id);
        goto cleanup;
    }

    rc = sqlite3_bind_int64 (stmt, 1, args.run_id); //assert (rc == SQLITE_OK);
    rc = sqlite3_step (stmt); //assert (rc == SQLITE_ROW || rc == SQLITE_DONE);

    rc = sqlite3_finalize (stmt);

    rc = sqlite3_prepare_v2 (db, query2, -1, &stmt, &tail);
    if (rc != SQLITE_OK)
    {
        fprintf (stderr, "Error second query delete_run_by_id_stub: Line: %d SQL error: %s\n", __LINE__, ErrMsg);
        sqlite3_free (ErrMsg);
        close_database(args.job_id);
        goto cleanup;
    }

    rc = sqlite3_bind_int64 (stmt, 1, args.run_id); //assert (rc == SQLITE_OK);
    rc = sqlite3_step (stmt); //assert (rc == SQLITE_ROW || rc == SQLITE_DONE);

    rc = sqlite3_finalize (stmt);

    rc = sqlite3_prepare_v2 (db, query3, -1, &stmt, &tail);
    if (rc != SQLITE_OK)
    {
        fprintf (stderr, "Error third query delete_run_by_id_stub: Line: %d SQL error: %s\n", __LINE__, ErrMsg);
        sqlite3_free (ErrMsg);
        close_database(args.job_id);
        goto cleanup;
    }

    rc = sqlite3_bind_int64 (stmt, 1, args.run_id); //assert (rc == SQLITE_OK);
    rc = sqlite3_step (stmt); //assert (rc == SQLITE_ROW || rc == SQLITE_DONE);

    rc = sqlite3_finalize (stmt);

    rc = sqlite3_prepare_v2 (db, query4, -1, &stmt, &tail);
    if (rc != SQLITE_OK)
    {
        fprintf (stderr, "Error fourth query delete_run_by_id_stub: Line: %d SQL error: %s\n", __LINE__, ErrMsg);
        sqlite3_free (ErrMsg);
        close_database(args.job_id);
        goto cleanup;
    }

    rc = sqlite3_bind_int64 (stmt, 1, args.run_id); //assert (rc == SQLITE_OK);
    rc = sqlite3_step (stmt); //assert (rc == SQLITE_ROW || rc == SQLITE_DONE);

    rc = sqlite3_finalize (stmt);

    rc = sqlite3_prepare_v2 (db, query5, -1, &stmt, &tail);
    if (rc != SQLITE_OK)
    {
        fprintf (stderr, "Error fifth query delete_run_by_id_stub: Line: %d SQL error: %s\n", __LINE__, ErrMsg);
        sqlite3_free (ErrMsg);
        close_database(args.job_id);
        goto cleanup;
    }

    rc = sqlite3_bind_int64 (stmt, 1, args.run_id); //assert (rc == SQLITE_OK);
    rc = sqlite3_step (stmt); //assert (rc == SQLITE_ROW || rc == SQLITE_DONE);

    rc = sqlite3_finalize (stmt);

    rc = sqlite3_prepare_v2 (db, query6, -1, &stmt, &tail);
    if (rc != SQLITE_OK)
    {
        fprintf (stderr, "Error sixth query delete_run_by_id_stub: Line: %d SQL error: %s\n", __LINE__, ErrMsg);
        sqlite3_free (ErrMsg);
        close_database(args.job_id);
        goto cleanup;
    }

    rc = sqlite3_bind_int64 (stmt, 1, args.run_id); //assert (rc == SQLITE_OK);
    rc = sqlite3_step (stmt); //assert (rc == SQLITE_ROW || rc == SQLITE_DONE);

    rc = sqlite3_finalize (stmt);

    rc = sqlite3_prepare_v2 (db, query7, -1, &stmt, &tail);
    if (rc != SQLITE_OK)
    {
        fprintf (stderr, "Error seventh query delete_run_by_id_stub: Line: %d SQL error: %s\n", __LINE__, ErrMsg);
        sqlite3_free (ErrMsg);
        close_database(args.job_id);
        goto cleanup;
    }

    rc = sqlite3_bind_int64 (stmt, 1, args.run_id); //assert (rc == SQLITE_OK);
    rc = sqlite3_step (stmt); //assert (rc == SQLITE_ROW || rc == SQLITE_DONE);

    rc = sqlite3_finalize (stmt);


    // rc = sqlite3_exec (db, "end;", callback, 0, &ErrMsg);
    // if (rc != SQLITE_OK)
    // {
    //     fprintf (stderr, "Error end query delete_run_by_id_stub: Line: %d SQL error: %s\n", __LINE__, ErrMsg);
    //     sqlite3_free (ErrMsg);
    //     close_database(args.job_id);
    //     goto cleanup;
    // }

cleanup:
    // if (rc != SQLITE_OK)
    // {
    //     rc = sqlite3_exec (db, "rollback;", callback, 0, &ErrMsg);
    // }

    return rc;
}