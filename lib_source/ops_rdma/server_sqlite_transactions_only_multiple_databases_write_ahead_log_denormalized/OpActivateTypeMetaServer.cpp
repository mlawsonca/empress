#include <OpActivateTypeMetaCommon.hh>
#include <server_timing_constants_new.hh>
#include <sql_helper_functs.hh>
#include <OpCoreServer.hh>

extern sqlite3 * get_database_for_read(uint64_t txn_id);
extern sqlite3 *main_db;
extern int callback (void * NotUsed, int argc, char ** argv, char ** ColName);

using namespace std;
int md_activate_type_stub (const md_activate_args &args);




WaitingType OpActivateTypeMeta::handleMessage(bool rdma, message_t *incoming_msg) {
    
    int rc;
    md_activate_args sql_args;

    deArchiveMsgFromClient(rdma, incoming_msg, sql_args); 

    
    add_timing_point(OP_ACTIVATE_TYPE_DEARCHIVE_MSG_FROM_CLIENT);

    rc = md_activate_type_stub(sql_args);
    add_timing_point(OP_ACTIVATE_TYPE_MD_ACTIVATE_TYPE_STUB);

    // std::string serial_str = serializeMsgToClient(rc);
    add_timing_point(OP_ACTIVATE_TYPE_SERIALIZE_MSG_FOR_CLIENT);

    createOutgoingMessage(dst, 
                          0, //Not expecting a reply
                          dst_mailbox,
                          to_string(rc));
    add_timing_point(OP_ACTIVATE_TYPE_CREATE_MSG_FOR_CLIENT);
    
    opbox::net::SendMsg(peer, ldo_msg);
    state=State::done;
    add_timing_point(OP_ACTIVATE_TYPE_SEND_MSG_TO_CLIENT_OP_DONE);
    return WaitingType::done_and_destroy;

}


int md_activate_type_stub (const md_activate_args &args)
{
    int rc;
    char * ErrMsg = NULL;
    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;

// ATTACH DATABASE 'file:memdb1?mode=memory&cache=shared' AS aux1;

    string aux_file_name = "'file:md" + to_string(args.txn_id) + "?mode=memory&cache=shared'";

    // string aux_file_name = "temp";
    // string aux_file_name = "file:temp?mode=memory&cache=shared";
    string query = "ATTACH DATABASE " + aux_file_name + " AS aux_db";

    // cout << "aux_file_name: " << aux_file_name << endl;

    sqlite3 *db = get_database_for_read(args.txn_id);

    // rc = sqlite3_exec (db, "release type;", callback, 0, &ErrMsg);
    // if (rc != SQLITE_OK)
    // {
    //     fprintf (stderr, "Error md_activate_type_stub: Line: %d SQL error: %s\n", __LINE__, ErrMsg);
    //     sqlite3_free (ErrMsg);
    //     sqlite3_close (db);
    // }

    rc = sqlite3_exec (main_db, query.c_str(), callback, 0, &ErrMsg);
    if (rc != SQLITE_OK)
    {
        fprintf (stderr, "Error md_activate_type_stub: Line: %d SQL error: %s\n", __LINE__, ErrMsg);
        sqlite3_free (ErrMsg);
        sqlite3_close (main_db);
    }


    // rc = sqlite3_prepare_v2 (main_db, "insert into type_catalog (id, run_id, name, version) values (?, ?, ?, ?)", -1, &stmt, &tail);
    // if (rc != SQLITE_OK)
    // {
    //     fprintf (stderr, "Error start of create_type_stub: Line: %d SQL error: %s\n", __LINE__, sqlite3_errmsg (main_db));
    //     sqlite3_close (main_db);
    //     goto cleanup;
    // }
    // // rc = sqlite3_bind_int64 (stmt, 1, args.type_id); //assert (rc == SQLITE_OK);    
    // rc = sqlite3_bind_null (stmt, 1); //assert (rc == SQLITE_OK);
    // rc = sqlite3_bind_int64 (stmt, 2, 8); //assert (rc == SQLITE_OK);    
    // rc = sqlite3_bind_text (stmt, 3, "temp", -1, free); //assert (rc == SQLITE_OK);
    // rc = sqlite3_bind_int (stmt, 4, 0); //assert (rc == SQLITE_OK);

    // rc = sqlite3_step (stmt); //assert (rc == SQLITE_ROW || rc == SQLITE_DONE);
    // cout << "confirmed, main_db not write locked" << endl;
    // rc = sqlite3_finalize (stmt);  //assert (rc == SQLITE_OK);



    rc = sqlite3_prepare_v2 (main_db, "insert into type_catalog select * from aux_db.type_catalog", -1, &stmt, &tail); 
    // rc = sqlite3_prepare_v2 (main_db, "insert into type_catalog(run_id, name, version) select run_id, name, version from aux_db.type_catalog", -1, &stmt, &tail); 
    // if (rc != SQLITE_OK)
    // {
    //     fprintf (stderr, "Error md_activate_type_stub: Line: %d SQL error (%d): %s\n", __LINE__, rc, sqlite3_errmsg (main_db));
    //     sqlite3_free (ErrMsg);
    //     sqlite3_close (main_db);
    // }
    //assert (rc == SQLITE_OK);
    rc = sqlite3_step (stmt); 
    // if (rc != SQLITE_ROW && rc != SQLITE_DONE)
    // {
    //     fprintf (stderr, "Error md_activate_type_stub: Line: %d SQL error (%d): %s\n", __LINE__, rc, sqlite3_errmsg (main_db));
    //     sqlite3_free (ErrMsg);
    //     sqlite3_close (main_db);
    // }
    //assert (rc == SQLITE_ROW || rc == SQLITE_DONE);
    rc = sqlite3_finalize (stmt);  //assert (rc == SQLITE_OK); 


    rc = sqlite3_exec (main_db, "DELETE FROM aux_db.type_catalog", callback, 0, &ErrMsg);

    rc = sqlite3_exec (main_db, "DETACH DATABASE aux_db;", callback, 0, &ErrMsg);
    if (rc != SQLITE_OK)
    {
        fprintf (stderr, "Error md_activate_type_stub: Line: %d SQL error: %s\n", __LINE__, ErrMsg);
        sqlite3_free (ErrMsg);
        sqlite3_close (main_db);
    }

cleanup:

    return rc;
}
