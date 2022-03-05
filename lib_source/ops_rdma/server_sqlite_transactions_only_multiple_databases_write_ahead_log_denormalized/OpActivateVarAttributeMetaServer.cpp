#include <OpActivateVarAttributeMetaCommon.hh>
#include <server_timing_constants_new.hh>
#include <sql_helper_functs.hh>
#include <OpCoreServer.hh>

extern sqlite3 * get_database_for_read(uint64_t txn_id);
extern sqlite3 *main_db;
extern int callback (void * NotUsed, int argc, char ** argv, char ** ColName);

using namespace std;
int md_activate_var_attribute_stub (const md_activate_args &args);

WaitingType OpActivateVarAttributeMeta::handleMessage(bool rdma, message_t *incoming_msg) {
  
    int rc;
    md_activate_args sql_args;

    deArchiveMsgFromClient(rdma, incoming_msg, sql_args); 

    
    add_timing_point(OP_ACTIVATE_VAR_ATTRIBUTE_DEARCHIVE_MSG_FROM_CLIENT);

    rc = md_activate_var_attribute_stub(sql_args);
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
    int rc;
    char * ErrMsg = NULL;
    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;

    string aux_file_name = "'file:md" + to_string(args.txn_id) + "?mode=memory&cache=shared'";
    string query = "ATTACH " + aux_file_name + " AS aux_db";

    sqlite3 *db = get_database_for_read(args.txn_id);

    // rc = sqlite3_exec (db, "release var_attribute;", callback, 0, &ErrMsg);
    // if (rc != SQLITE_OK)
    // {
    //     fprintf (stderr, "Error md_activate_var_attribute_stub: Line: %d SQL error: %s\n", __LINE__, ErrMsg);
    //     sqlite3_free (ErrMsg);
    //     sqlite3_close (db);
    // }

    rc = sqlite3_exec (main_db, query.c_str(), callback, 0, &ErrMsg);

    rc = sqlite3_prepare_v2 (main_db, "insert into var_attribute_catalog select * from aux_db.var_attribute_catalog", -1, &stmt, &tail); //assert (rc == SQLITE_OK);
    // rc = sqlite3_prepare_v2 (main_db, "insert into var_attribute_catalog(timestep_id, type_id, var_id, num_dims, d0_min, d0_max, d1_min, d1_max, d2_min, d2_max, data_type, data) select timestep_id, type_id, var_id, num_dims, d0_min, d0_max, d1_min, d1_max, d2_min, d2_max, data_type, data from aux_db.var_attribute_catalog", -1, &stmt, &tail); //assert (rc == SQLITE_OK);
    rc = sqlite3_step (stmt); //assert (rc == SQLITE_ROW || rc == SQLITE_DONE);
    rc = sqlite3_finalize (stmt);  //assert (rc == SQLITE_OK); 


    rc = sqlite3_exec (main_db, "DELETE FROM aux_db.var_attribute_catalog", callback, 0, &ErrMsg);

    rc = sqlite3_exec (main_db, "DETACH aux_db;", callback, 0, &ErrMsg);
    if (rc != SQLITE_OK)
    {
        fprintf (stderr, "Error md_activate_var_attribute_stub: Line: %d SQL error: %s\n", __LINE__, ErrMsg);
        sqlite3_free (ErrMsg);
        sqlite3_close (main_db);
    }

cleanup:

    return rc;
}


