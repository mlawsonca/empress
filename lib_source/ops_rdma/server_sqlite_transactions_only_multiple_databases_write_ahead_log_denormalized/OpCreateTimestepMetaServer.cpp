#include <OpCreateTimestepMetaCommon.hh>
#include <server_timing_constants_new.hh>
#include <sql_helper_functs.hh>
#include <OpCoreServer.hh>

extern sqlite3 * get_database_for_write(uint64_t txn_id);
extern sqlite3 *main_db;
extern int callback (void * NotUsed, int argc, char ** argv, char ** ColName);


using namespace std;
// int md_create_timestep_stub (sqlite3 *db, const md_create_timestep_args &args,
//                         uint64_t &timestep_id);

int md_create_timestep_stub (sqlite3 *db, const md_create_timestep_args &args);


WaitingType OpCreateTimestepMeta::handleMessage(bool rdma, message_t *incoming_msg) {

    int rc;
    md_create_timestep_args sql_args;
    uint64_t timestep_id;

    deArchiveMsgFromClient(rdma, incoming_msg, sql_args);
    add_timing_point(OP_CREATE_TIMESTEP_DEARCHIVE_MSG_FROM_CLIENT);

    // sqlite3 *db = get_database(sql_args.txn_id);
    sqlite3 *db = get_database_for_write(sql_args.txn_id);

    // //log("About to create timestep stub");
    // rc = md_create_timestep_stub(db, sql_args, timestep_id);
    rc = md_create_timestep_stub(db, sql_args);
    add_timing_point(OP_CREATE_TIMESTEP_MD_CREATE_TIMESTEP_STUB);

    // std::string serial_str = serializeMsgToClient(timestep_id, rc);
    add_timing_point(OP_CREATE_TIMESTEP_SERIALIZE_MSG_FOR_CLIENT);

    // createOutgoingMessage(dst, 
    //                       0, //Not expecting a reply
    //                       dst_mailbox,
    //                       serial_str);
    createOutgoingMessage(dst, 
                          0, //Not expecting a reply
                          dst_mailbox,
                          to_string(rc));
    add_timing_point(OP_CREATE_TIMESTEP_CREATE_MSG_FOR_CLIENT);

    opbox::net::SendMsg(peer, ldo_msg);
    state=State::done;
    add_timing_point(OP_CREATE_TIMESTEP_SEND_MSG_TO_CLIENT_OP_DONE);
    return WaitingType::done_and_destroy;

}





// int md_create_timestep_stub (sqlite3 *db, const md_create_timestep_args &args,
//                         uint64_t &timestep_id)
int md_create_timestep_stub (sqlite3 *db, const md_create_timestep_args &args)
{
    int rc;
    int i;
    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;
    // size_t len = 0;

    char * ErrMsg = NULL;

    // rc = sqlite3_exec (db, "savepoint timestep;", callback, 0, &ErrMsg);
    // if (rc != SQLITE_OK)
    // {
    //     fprintf (stderr, "Error md_create_timestep_stub: Line: %d SQL error: %s\n", __LINE__, ErrMsg);
    //     sqlite3_free (ErrMsg);
    //     sqlite3_close (db);
    // }

    rc = sqlite3_prepare_v2 (db, "insert into timestep_catalog (id, run_id, path) values (?, ?, ?)", -1, &stmt, &tail);
    if (rc != SQLITE_OK)
    {
        fprintf (stderr, "Error start of create_timestep_stub: Line: %d SQL error: %s\n", __LINE__, sqlite3_errmsg (db));
        sqlite3_close (db);
        goto cleanup;
    }

    rc = sqlite3_bind_int64 (stmt, 1, args.timestep_id); //assert (rc == SQLITE_OK);    
    // rc = sqlite3_bind_null (stmt, 1); //assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 2, args.run_id); //assert (rc == SQLITE_OK);
    rc = sqlite3_bind_text (stmt, 3, strdup(args.path.c_str()), -1, free); //assert (rc == SQLITE_OK);
    
    rc = sqlite3_step (stmt); //assert (rc == SQLITE_ROW || rc == SQLITE_DONE);
    // timestep_id = (int) sqlite3_last_insert_rowid (db);

    rc = sqlite3_finalize (stmt);

cleanup:

    return rc;
}