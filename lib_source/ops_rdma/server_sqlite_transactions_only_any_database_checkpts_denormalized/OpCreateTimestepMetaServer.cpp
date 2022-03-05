#include <OpCreateTimestepMetaCommon.hh>
#include <server_timing_constants_new.hh>
#include <sql_helper_functs.hh>
#include <OpCoreServer.hh>

extern sqlite3 * get_database_for_write(uint64_t job_id, md_catalog_type catalog_type);
extern void close_database(uint64_t job_id);


extern int callback (void * NotUsed, int argc, char ** argv, char ** ColName);


using namespace std;
// int md_create_timestep_stub (sqlite3 *db, const md_create_timestep_args &args,
//                         uint64_t &timestep_id);

int md_create_timestep_stub (sqlite3 *db, const md_create_timestep_args &args);

WaitingType OpCreateTimestepMeta::handleMessage(bool rdma, message_t *incoming_msg) {

    int rc;
    md_create_timestep_args args;
    // uint64_t timestep_id;

    deArchiveMsgFromClient(rdma, incoming_msg, args);
    add_timing_point(OP_CREATE_TIMESTEP_DEARCHIVE_MSG_FROM_CLIENT);

    // sqlite3 *db = get_database(args.job_id, args.query_type, trans_active);

    sqlite3 *db = get_database_for_write(args.job_id, TIMESTEP_CATALOG);

    // //log("About to create timestep stub");
    // rc = md_create_timestep_stub(db, args, timestep_id);
    rc = md_create_timestep_stub(db, args);
    add_timing_point(OP_CREATE_TIMESTEP_MD_CREATE_TIMESTEP_STUB);

    // std::string serial_str = serializeMsgToClient(timestep_id, rc);
    add_timing_point(OP_CREATE_TIMESTEP_SERIALIZE_MSG_FOR_CLIENT);

    // createOutgoingMessage(dst, 
    //                       0, //Not expecting a reply
    //                       dst_mailbox,
    //                       serial_str);
    createOutgoingMessage(dst, 
                          0, //Not expecting a reply
                          dst_mailbox,
                          to_string(rc));
    add_timing_point(OP_CREATE_TIMESTEP_CREATE_MSG_FOR_CLIENT);

    opbox::net::SendMsg(peer, ldo_msg);
    state=State::done;
    add_timing_point(OP_CREATE_TIMESTEP_SEND_MSG_TO_CLIENT_OP_DONE);
    return WaitingType::done_and_destroy;

}





// int md_create_timestep_stub (sqlite3 *db, const md_create_timestep_args &args,
//                         uint64_t &timestep_id)

int md_create_timestep_stub (sqlite3 *db, const md_create_timestep_args &args)
{
    int rc;
    int i;
    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;
    // size_t len = 0;
    char *err_msg = 0;
    char *sql;

    char * ErrMsg = NULL;

    //cout << "creating savepoint timestep" << endl;

    //pulled this code inside get_database
    // rc = sqlite3_exec (db, "savepoint timestep;", callback, 0, &ErrMsg);
    // if (rc != SQLITE_OK)
    // {
    //     fprintf (stderr, "Error md_create_timestep_stub: Line: %d SQL error: %s\n", __LINE__, ErrMsg);
    //     sqlite3_free (ErrMsg);
    //     close_database(args.job_id);
    // }

    rc = sqlite3_prepare_v2 (db, "insert into timestep_catalog (id, run_id, path) values (?, ?, ?)", -1, &stmt, &tail);
    if (rc != SQLITE_OK)
    {
        fprintf (stderr, "Error start of create_timestep_stub: Line: %d SQL error: %s\n", __LINE__, sqlite3_errmsg (db));
        close_database(args.job_id);
        goto cleanup;
    }

    rc = sqlite3_bind_int64 (stmt, 1, args.timestep_id); //assert (rc == SQLITE_OK);    
    // rc = sqlite3_bind_null (stmt, 1); //assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 2, args.run_id); //assert (rc == SQLITE_OK);
    rc = sqlite3_bind_text (stmt, 3, strdup(args.path.c_str()), -1, free); //assert (rc == SQLITE_OK);
    
    // cout << "timestep_id: " << args.timestep_id << endl;
    // cout << "run_id: " << args.run_id << endl;

    // sql = "SELECT * FROM timestep_catalog";
    // cout << "starting timestep_catalog \n";
    // rc = sqlite3_exec(db, sql, callback, 0, &err_msg);
    // cout << "done with timestep_catalog \n";


    rc = sqlite3_step (stmt); 
    if (rc != SQLITE_ROW && rc != SQLITE_DONE)
    {
        fprintf (stderr, "Error md_create_timestep_stub: Line: %d rc: %d SQL error: %s\n", __LINE__, rc, sqlite3_errmsg (db));
        close_database(args.job_id);
        goto cleanup;
    }
    //assert (rc == SQLITE_ROW || rc == SQLITE_DONE);
    // timestep_id = (int) sqlite3_last_insert_rowid (db);

    rc = sqlite3_finalize (stmt);

cleanup:

    return rc;
}