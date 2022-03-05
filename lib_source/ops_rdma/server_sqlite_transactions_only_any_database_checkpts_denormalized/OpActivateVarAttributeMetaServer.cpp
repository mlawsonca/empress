#include <OpActivateVarAttributeMetaCommon.hh>
#include <server_timing_constants_new.hh>
#include <sql_helper_functs.hh>
#include <OpCoreServer.hh>

extern sqlite3 * get_database_and_status(uint64_t job_id, md_catalog_type md_catalog_type, bool &savept_active);
extern void close_database(uint64_t job_id);
extern int callback (void * NotUsed, int argc, char ** argv, char ** ColName);

using namespace std;
int md_activate_var_attribute_stub (const md_activate_args &args);


WaitingType OpActivateVarAttributeMeta::handleMessage(bool rdma, message_t *incoming_msg) {
  

    md_activate_args args;

    deArchiveMsgFromClient(rdma, incoming_msg, args); 

    add_timing_point(OP_ACTIVATE_VAR_ATTRIBUTE_DEARCHIVE_MSG_FROM_CLIENT);

    int rc = md_activate_var_attribute_stub(args);
    add_timing_point(OP_ACTIVATE_VAR_ATTRIBUTE_MD_ACTIVATE_VAR_ATTRIBUTE_STUB);

    // std::string serial_str = serializeMsgToClient(rc);
    add_timing_point(OP_ACTIVATE_VAR_ATTRIBUTE_SERIALIZE_MSG_FOR_CLIENT);

    createOutgoingMessage(dst, 
                          0, //Not expecting a reply
                          dst_mailbox,
                          to_string(rc));
    add_timing_point(OP_ACTIVATE_VAR_ATTRIBUTE_CREATE_MSG_FOR_CLIENT);
    
    opbox::net::SendMsg(peer, ldo_msg);
    state=State::done;
    add_timing_point(OP_ACTIVATE_VAR_ATTRIBUTE_SEND_MSG_TO_CLIENT_OP_DONE);
    return WaitingType::done_and_destroy;

}



int md_activate_var_attribute_stub (const md_activate_args &args)
{
    int rc = RC_OK;
    char * ErrMsg = NULL;
    bool savept_active;

    sqlite3 *db = get_database_and_status(args.job_id, VAR_ATTR_CATALOG, savept_active);

    if(savept_active) {
        //cout << "about to release var_attribute" << endl;

        //note - will fail if no savepoint exists of that name
        rc = sqlite3_exec (db, "release var_attribute;", callback, 0, &ErrMsg);
        if (rc != SQLITE_OK)
        {
            fprintf (stderr, "Error md_activate_var_attribute_stub: Line: %d SQL error: %s\n", __LINE__, ErrMsg);
            sqlite3_free (ErrMsg);
            close_database(args.job_id);
        }
    }
cleanup:

    return rc;
}