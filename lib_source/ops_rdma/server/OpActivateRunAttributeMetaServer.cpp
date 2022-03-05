#include <OpActivateRunAttributeMetaCommon.hh>
#include <server_timing_constants_new.hh>
#include <sql_helper_functs.hh>
#include <OpCoreServer.hh>

extern sqlite3 *db;

using namespace std;
int md_activate_run_attribute_stub (const md_activate_args &args);


WaitingType OpActivateRunAttributeMeta::handleMessage(bool rdma, message_t *incoming_msg) {
  
    md_activate_args sql_args;

    deArchiveMsgFromClient(rdma, incoming_msg, sql_args); 
    add_timing_point(OP_ACTIVATE_RUN_ATTRIBUTE_DEARCHIVE_MSG_FROM_CLIENT);

    int rc = md_activate_run_attribute_stub(sql_args);
    add_timing_point(OP_ACTIVATE_RUN_ATTRIBUTE_MD_ACTIVATE_RUN_ATTRIBUTE_STUB);

    // std::string serial_str = serializeMsgToClient(rc);
    add_timing_point(OP_ACTIVATE_RUN_ATTRIBUTE_SERIALIZE_MSG_FOR_CLIENT);


    createOutgoingMessage(dst, 
                          0, //Not expecting a reply
                          dst_mailbox,
                          to_string(rc));
    add_timing_point(OP_ACTIVATE_RUN_ATTRIBUTE_CREATE_MSG_FOR_CLIENT);
    
    opbox::net::SendMsg(peer, ldo_msg);
    state=State::done;
    add_timing_point(OP_ACTIVATE_RUN_ATTRIBUTE_SEND_MSG_TO_CLIENT_OP_DONE);
    return WaitingType::done_and_destroy;

}

int md_activate_run_attribute_stub (const md_activate_args &args)
{
    int rc;
    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;
    const char * query = "update run_attribute_catalog set active = 1 where txn_id = ?";

    rc = sqlite3_prepare_v2 (db, query, -1, &stmt, &tail); //assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 1, args.txn_id); //assert (rc == SQLITE_OK);
    rc = sqlite3_step (stmt); //assert (rc == SQLITE_ROW || rc == SQLITE_DONE);
    rc = sqlite3_finalize (stmt); //assert (rc == SQLITE_OK);

cleanup:

    return rc;
}