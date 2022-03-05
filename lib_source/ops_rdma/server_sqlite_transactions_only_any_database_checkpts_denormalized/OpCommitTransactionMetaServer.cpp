#include <OpCommitTransactionMetaCommon.hh>
#include <server_timing_constants_new.hh>
#include <sql_helper_functs.hh>

extern sqlite3 *db;

extern sqlite3 * get_database_for_transaction_end(uint64_t job_id, bool &trans_active);
extern void close_database(uint64_t job_id);
extern int callback (void * NotUsed, int argc, char ** argv, char ** ColName);

using namespace std;

WaitingType OpCommitTransactionMeta::handleMessage(bool rdma, message_t *incoming_msg) {
  
    int rc;
    char * ErrMsg = NULL;
    bool trans_active;
    uint64_t database_to_query = stoull(incoming_msg->body);
    sqlite3 *db = get_database_for_transaction_end(database_to_query, trans_active);

    // cout << "OpCommitTransactionMeta: about to commit transaction" << endl;

    rc = sqlite3_exec (db, "end;", callback, 0, &ErrMsg);
    if (rc != SQLITE_OK)
    {
        fprintf (stderr, "Error OpCommitTransactionMeta: Line: %d SQL error: %s\n", __LINE__, ErrMsg);
        sqlite3_free (ErrMsg);
        close_database(database_to_query);
    }
    add_timing_point(OP_COMMIT_TRANSACTION_MD_COMMIT_TRANSACTION_STUB);

    createOutgoingMessage(incoming_msg->src, 
                          0, //Not expecting a reply
                          incoming_msg->src_mailbox,
                          to_string(rc));



    opbox::net::SendMsg(peer, ldo_msg);
    state=State::done;
    add_timing_point(OP_COMMIT_TRANSACTION_SEND_MSG_TO_CLIENT_OP_DONE);
    return WaitingType::done_and_destroy;

}
