#include <OpProcessingMetaCommon.hh>
#include <server_timing_constants_new.hh>
#include <sql_helper_functs.hh>
#include <OpCoreServer.hh>

extern sqlite3 * get_database_and_status(uint64_t job_id, md_catalog_type md_catalog_type, bool &savept_active);
extern void close_database(uint64_t job_id);
extern int callback (void * NotUsed, int argc, char ** argv, char ** ColName);

using namespace std;

int md_processing_stub (const md_processing_args &args);


WaitingType OpProcessingMeta::handleMessage(bool rdma, message_t *incoming_msg) {
  
    int rc;
    md_processing_args args;

    deArchiveMsgFromClient(rdma, incoming_msg, args); 
    add_timing_point(OP_PROCESSING_DEARCHIVE_MSG_FROM_CLIENT);

    rc = md_processing_stub(args);
    add_timing_point(OP_PROCESSING_MD_PROCESSING_STUB);


    // std::string serial_str = serializeMsgToClient(rc);
    add_timing_point(OP_PROCESSING_SERIALIZE_MSG_FOR_CLIENT);

    createOutgoingMessage(dst, 
                          0, //Not expecting a reply
                          dst_mailbox,
                          to_string(rc));
    add_timing_point(OP_PROCESSING_CREATE_MSG_FOR_CLIENT);

    opbox::net::SendMsg(peer, ldo_msg);
    state=State::done;
    add_timing_point(OP_PROCESSING_SEND_MSG_TO_CLIENT_OP_DONE);
    return WaitingType::done_and_destroy;

}




int md_processing_stub (const md_processing_args &args)
{
    int rc = RC_OK;
    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;
    char * ErrMsg = NULL;
    bool savept_active;

    // string query;
    char *query;
    sqlite3 *db = get_database_and_status(args.job_id, args.catalog_type, savept_active);

    if(savept_active) {
        // cout << "savept is active" << endl;
        switch(args.catalog_type) {
            case RUN_CATALOG : {
                query = "rollback to run;";
                break;
            }
            case TIMESTEP_CATALOG : {
                query = "rollback to timestep;";
                break;
            }
            case VAR_CATALOG : {
                query = "rollback to var;";
                break;
            }
            case TYPE_CATALOG : {
                query = "rollback to type;";
                break;
            }
            case RUN_ATTR_CATALOG : {
                query = "rollback to run_attribute;";
                break;
            }
            case TIMESTEP_ATTR_CATALOG : {
                query = "rollback to timestep_attribute;";
                break;
            }
            case VAR_ATTR_CATALOG : {
                query = "rollback to var_attribute;";
                break;
            }
        }

        rc = sqlite3_exec (db, query, callback, 0, &ErrMsg);
        if (rc != SQLITE_OK)
        {
            fprintf (stderr, "Error md_processing_stub: Line: %d SQL error: %s\n", __LINE__, ErrMsg);
            sqlite3_free (ErrMsg);
            close_database(args.job_id);
        }
    }
    // else {
    //     cout << "savept is not active" << endl;
    // }
    return rc;
}