#include <OpCreateVarMetaCommon.hh>
#include <server_timing_constants_new.hh>
#include <sql_helper_functs.hh>
#include <OpCoreServer.hh>

extern sqlite3 *db;

using namespace std;
// int md_create_var_stub (const md_create_var_args &args,
//                         uint64_t &row_id);
int md_create_var_stub (const md_create_var_args &args);

WaitingType OpCreateVarMeta::handleMessage(bool rdma, message_t *incoming_msg) {


    md_create_var_args sql_args;
    // uint64_t row_id;

    //convert the serialized string back into the args and count pointer
    deArchiveMsgFromClient(rdma, incoming_msg, sql_args); 
    add_timing_point(OP_CREATE_VAR_DEARCHIVE_MSG_FROM_CLIENT);

    // //log("About to create var stub");
    // int rc = md_create_var_stub(sql_args, row_id);
    int rc = md_create_var_stub(sql_args);
    add_timing_point(OP_CREATE_VAR_MD_CREATE_VAR_STUB);

    // std::string serial_str = serializeMsgToClient(row_id, rc);
    add_timing_point(OP_CREATE_VAR_SERIALIZE_MSG_FOR_CLIENT);

    // createOutgoingMessage(dst, 
    //                       0, //Not expecting a reply
    //                       dst_mailbox,
    //                       serial_str);
    createOutgoingMessage(dst, 
                          0, //Not expecting a reply
                          dst_mailbox,
                          to_string(rc));
    add_timing_point(OP_CREATE_VAR_CREATE_MSG_FOR_CLIENT);

    opbox::net::SendMsg(peer, ldo_msg);
    state=State::done;
    add_timing_point(OP_CREATE_VAR_SEND_MSG_TO_CLIENT_OP_DONE);
    return WaitingType::done_and_destroy;

}


// extern int callback (void * NotUsed, int argc, char ** argv, char ** ColName);


// int md_create_var_stub (const md_create_var_args &args,
//                         uint64_t &row_id)

int md_create_var_stub (const md_create_var_args &args)
{
    int rc;
    int i;
    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;
    // size_t len = 0;

    // char *err_msg = 0;
    // char *sql = "SELECT * FROM var_catalog";
    // cout << "starting var catalog \n";
    // rc = sqlite3_exec(db, sql, callback, 0, &err_msg);
    // cout << "done with var catalog \n";


    rc = sqlite3_prepare_v2 (db, "insert into var_catalog (id, run_id, timestep_id, name, path, version, data_size, active, txn_id, num_dims, d0_min, d0_max, d1_min, d1_max, d2_min, d2_max) values (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)", -1, &stmt, &tail);
    if (rc != SQLITE_OK)
    {
        fprintf (stderr, "Error start of create_var_stub: Line: %d SQL error: %s\n", __LINE__, sqlite3_errmsg (db));
        sqlite3_close (db);
        goto cleanup;
    }

    rc = sqlite3_bind_int64 (stmt, 1, args.var_id); //assert (rc == SQLITE_OK);  
    // cout << "var id on create: " << args.var_id << endl; 
    rc = sqlite3_bind_int64 (stmt, 2, args.run_id); //assert (rc == SQLITE_OK); 
    rc = sqlite3_bind_int64 (stmt, 3, args.timestep_id); //assert (rc == SQLITE_OK);   
    rc = sqlite3_bind_text (stmt, 4, strdup(args.name.c_str()), -1, free); //assert (rc == SQLITE_OK);
    rc = sqlite3_bind_text (stmt, 5, strdup(args.path.c_str()), -1, free); //assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int (stmt, 6, args.version); //assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int (stmt, 7, (int)args.data_size); //assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int (stmt, 8, 0); //assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 9, args.txn_id); //assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int (stmt, 10, args.num_dims); //assert (rc == SQLITE_OK);
    for (i = 0; i < args.num_dims; i++)
    {
        rc = sqlite3_bind_double (stmt, 11 + i * 2, args.dims.at(i).min); //assert (rc == SQLITE_OK);
        rc = sqlite3_bind_double (stmt, 12 + i * 2, args.dims.at(i).max); //assert (rc == SQLITE_OK);
    }

    rc = sqlite3_step (stmt); //assert (rc == SQLITE_ROW || rc == SQLITE_DONE);
    // row_id = (int) sqlite3_last_insert_rowid (db);
//printf ("generated new global id:%d\n", *row_id);

    rc = sqlite3_finalize (stmt);

cleanup:

    return rc;
}