#include <OpCreateTypeMetaCommon.hh>
#include <server_timing_constants_new.hh>
#include <sql_helper_functs.hh>
#include <OpCoreServer.hh>

extern sqlite3 * get_database_for_write(uint64_t txn_id);
extern sqlite3 *main_db;
extern int callback (void * NotUsed, int argc, char ** argv, char ** ColName);


using namespace std;

int md_create_type_stub (sqlite3 *db, const md_create_type_args &args,
                        uint64_t &type_id);



WaitingType OpCreateTypeMeta::handleMessage(bool rdma, message_t *incoming_msg) {

    int rc;
    md_create_type_args sql_args;
    uint64_t type_id;
    
    // std::cout << "About to dearchive msg from client \n";
    //convert the serialized string back into the args and count pointer
    deArchiveMsgFromClient(rdma, incoming_msg, sql_args);
    add_timing_point(OP_CREATE_TYPE_DEARCHIVE_MSG_FROM_CLIENT);

    // sqlite3 *db = get_database(sql_args.txn_id);    
    sqlite3 *db = get_database_for_write(sql_args.txn_id);

    // std::cout << "About to create var stub \n";
    rc = md_create_type_stub(db, sql_args, type_id);
    add_timing_point(OP_CREATE_TYPE_MD_CREATE_TYPE_STUB);

    std::string serial_str = serializeMsgToClient(type_id, rc);
    add_timing_point(OP_CREATE_TYPE_SERIALIZE_MSG_FOR_CLIENT);

    createOutgoingMessage(dst, 
                          0, //Not expecting a reply
                          dst_mailbox,
                          serial_str);
    add_timing_point(OP_CREATE_TYPE_CREATE_MSG_FOR_CLIENT);

    opbox::net::SendMsg(peer, ldo_msg);
    state=State::done;
    add_timing_point(OP_CREATE_TYPE_SEND_MSG_TO_CLIENT_OP_DONE);
    return WaitingType::done_and_destroy;

}


int md_create_type_stub (sqlite3 *db, const md_create_type_args &args,
                        uint64_t &type_id)
{
    int rc;
    // int i;
    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;
    // size_t len = 0;
    char * ErrMsg = NULL;

    // cout << "create type" << endl;



    // rc = sqlite3_exec (db, "savepoint type;", callback, 0, &ErrMsg);
    // if (rc != SQLITE_OK)
    // {
    //     fprintf (stderr, "Error md_create_type_stub: Line: %d SQL error: %s\n", __LINE__, ErrMsg);
    //     sqlite3_free (ErrMsg);
    //     sqlite3_close (db);
    // }

    rc = sqlite3_prepare_v2 (db, "insert into type_catalog (id, run_id, name, version) values (?, ?, ?, ?)", -1, &stmt, &tail);
    if (rc != SQLITE_OK)
    {
        fprintf (stderr, "Error start of create_type_stub: Line: %d SQL error: %s\n", __LINE__, sqlite3_errmsg (db));
        sqlite3_close (db);
        goto cleanup;
    }

    
    // rc = sqlite3_bind_int64 (stmt, 1, args.type_id); //assert (rc == SQLITE_OK);    
    rc = sqlite3_bind_null (stmt, 1); //assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 2, args.run_id); //assert (rc == SQLITE_OK);    
    rc = sqlite3_bind_text (stmt, 3, strdup(args.name.c_str()), -1, free); //assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int (stmt, 4, args.version); //assert (rc == SQLITE_OK);

    // cout << "run_id: " << args.run_id << " name: " << args.name << " version: " << args.version << < endl;

    rc = sqlite3_step (stmt); //assert (rc == SQLITE_ROW || rc == SQLITE_DONE);
    type_id = (int) sqlite3_last_insert_rowid (db);
//printf ("generated new global id:%d\n", *type_id);

    rc = sqlite3_finalize (stmt);


cleanup:

    return rc;
}

