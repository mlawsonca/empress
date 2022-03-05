#include <OpCreateVarMetaCommon.hh>
#include <server_timing_constants_new.hh>
#include <sql_helper_functs.hh>
#include <OpCoreServer.hh>

extern sqlite3 * get_database_for_write(uint64_t job_id, md_catalog_type catalog_type);
extern void close_database(uint64_t job_id);


extern int callback (void * NotUsed, int argc, char ** argv, char ** ColName);


using namespace std;
// int md_create_var_stub (sqlite3 *db, const md_create_var_args &args,
//                         uint64_t &row_id);
int md_create_var_stub (sqlite3 *db, const md_create_var_args &args);

WaitingType OpCreateVarMeta::handleMessage(bool rdma, message_t *incoming_msg) {

    int rc;
    md_create_var_args args;
    // uint64_t row_id;

    deArchiveMsgFromClient(rdma, incoming_msg, args);
    add_timing_point(OP_CREATE_VAR_DEARCHIVE_MSG_FROM_CLIENT);

    // sqlite3 *db = get_database(args.job_id, args.query_type, trans_active);
    
    sqlite3 *db = get_database_for_write(args.job_id, VAR_CATALOG);

    // //log("About to create var stub");
    // rc = md_create_var_stub(db, args, row_id);
    rc = md_create_var_stub(db, args);
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


// int md_create_var_stub (sqlite3 *db, const md_create_var_args &args,
//                         uint64_t &row_id)
int md_create_var_stub (sqlite3 *db, const md_create_var_args &args)
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
    char * ErrMsg = NULL;

    //cout << "creating savepoint var" << endl;

    //pulled this code inside get_database
    // rc = sqlite3_exec (db, "savepoint var;", callback, 0, &ErrMsg);
    // if (rc != SQLITE_OK)
    // {
    //     fprintf (stderr, "Error md_create_var_stub: Line: %d SQL error: %s\n", __LINE__, ErrMsg);
    //     sqlite3_free (ErrMsg);
    //     close_database(args.job_id);
    // }

    rc = sqlite3_prepare_v2 (db, "insert into var_catalog (id, run_id, timestep_id, name, path, version, data_size, num_dims, d0_min, d0_max, d1_min, d1_max, d2_min, d2_max) values (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)", -1, &stmt, &tail);
    if (rc != SQLITE_OK)
    {
        fprintf (stderr, "Error start of create_var_stub: Line: %d SQL error: %s\n", __LINE__, sqlite3_errmsg (db));
        close_database(args.job_id);
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
    rc = sqlite3_bind_int (stmt, 8, args.num_dims); //assert (rc == SQLITE_OK);
    for (i = 0; i < args.num_dims; i++)
    {
        rc = sqlite3_bind_double (stmt, 9 + i * 2, args.dims.at(i).min); //assert (rc == SQLITE_OK);
        rc = sqlite3_bind_double (stmt, 10 + i * 2, args.dims.at(i).max); //assert (rc == SQLITE_OK);
    }

    rc = sqlite3_step (stmt); //assert (rc == SQLITE_ROW || rc == SQLITE_DONE);

    // row_id = (int) sqlite3_last_insert_rowid (db);
//printf ("generated new global id:%d\n", *row_id);

    rc = sqlite3_finalize (stmt);

cleanup:

    return rc;
}