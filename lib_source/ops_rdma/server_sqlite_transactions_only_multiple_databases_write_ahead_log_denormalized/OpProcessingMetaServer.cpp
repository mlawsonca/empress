#include <OpProcessingMetaCommon.hh>
#include <server_timing_constants_new.hh>
#include <sql_helper_functs.hh>
#include <OpCoreServer.hh>

extern sqlite3 * get_database(uint64_t txn_id);
extern sqlite3 *main_db;
extern int callback (void * NotUsed, int argc, char ** argv, char ** ColName);

using namespace std;

int md_processing_stub (sqlite3 *db, const md_processing_args &args);


WaitingType OpProcessingMeta::handleMessage(bool rdma, message_t *incoming_msg) {
  
    int rc;
    md_processing_args sql_args;

    deArchiveMsgFromClient(rdma, incoming_msg, sql_args); 
    add_timing_point(OP_PROCESSING_DEARCHIVE_MSG_FROM_CLIENT);

    sqlite3 *db = get_database(sql_args.txn_id);

    rc = md_processing_stub(db, sql_args);
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




int md_processing_stub (sqlite3 *db, const md_processing_args &args)
{
    int rc;
    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;
    char * ErrMsg = NULL;

    // string query;
    char *query;

    switch(args.catalog_type) {
        case RUN_CATALOG : {
            // query = "rollback to run;";
            query = "DELETE FROM run_catalog";
            break;
        }
        case TIMESTEP_CATALOG : {
            // query = "rollback to timestep;";
            query = "DELETE FROM timestep_catalog";
            break;
        }
        case VAR_CATALOG : {
            // query = "rollback to var;";
            query = "DELETE FROM var_catalog";
            break;
        }
        case TYPE_CATALOG : {
            // query = "rollback to type;";
            query = "DELETE FROM type_catalog";
            break;
        }
        case RUN_ATTR_CATALOG : {
            // query = "rollback to run attribute;";
            query = "DELETE FROM run_attribute_catalog";
            break;
        }
        case TIMESTEP_ATTR_CATALOG : {
            // query = "rollback to timestep attribute;";
            query = "DELETE FROM timestep_attribute_catalog";
            break;
        }
        case VAR_ATTR_CATALOG : {
            // query = "rollback to var attribute;";
            query = "DELETE FROM var_attribute_catalog";
            break;
        }
    }

    rc = sqlite3_exec (db, query, callback, 0, &ErrMsg);
    if (rc != SQLITE_OK)
    {
        fprintf (stderr, "Error md_activate_var_stub: Line: %d SQL error: %s\n", __LINE__, ErrMsg);
        sqlite3_free (ErrMsg);
        sqlite3_close (db);
    }
    return rc;
}