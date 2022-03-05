#include <OpCreateRunMetaCommon.hh>
#include <server_timing_constants_new.hh>
#include <sql_helper_functs.hh>
#include <OpCoreServer.hh>

extern sqlite3 * get_database_for_write(uint64_t job_id, md_catalog_type catalog_type);
extern void close_database(uint64_t job_id);
extern int callback (void * NotUsed, int argc, char ** argv, char ** ColName);


using namespace std;
int md_create_run_stub (sqlite3 *db, const md_create_run_args &args,
                        uint64_t &run_id);




WaitingType OpCreateRunMeta::handleMessage(bool rdma, message_t *incoming_msg) {
    
    int rc;
    md_create_run_args args;
    uint64_t run_id;

    // cout << "about to dearchive message" << endl;
    deArchiveMsgFromClient(rdma, incoming_msg, args);
    // cout << "database_to_query: " << args.job_id << endl;
    
    // cout << "db: " << db << endl;
    // cout << "db addr: " << &db << endl;

    // cout << "about to do DB query" << endl;
    add_timing_point(OP_CREATE_RUN_DEARCHIVE_MSG_FROM_CLIENT);
    // sqlite3 *db = get_database(args.job_id, args.query_type, trans_active);

    // cout << "OpCreateRunMeta: about to get database for job_id: " << args.job_id << endl;
    sqlite3 *db = get_database_for_write(args.job_id, RUN_CATALOG);

    // //log("About to create run stub");
    rc = md_create_run_stub(db, args, run_id);
    // cout << "run_id: " << run_id << endl;
    add_timing_point(OP_CREATE_RUN_MD_CREATE_RUN_STUB);

    // cout << "run_id: " << run_id << " rc: " << rc << endl;

    std::string serial_str = serializeMsgToClient(run_id, rc);
    add_timing_point(OP_CREATE_RUN_SERIALIZE_MSG_FOR_CLIENT);
    
    // cout << "got here 3 " << endl;

    createOutgoingMessage(dst, 
                          0, //Not expecting a reply
                          dst_mailbox,
                          serial_str);
    add_timing_point(OP_CREATE_RUN_CREATE_MSG_FOR_CLIENT);

    opbox::net::SendMsg(peer, ldo_msg);
    state=State::done;
    add_timing_point(OP_CREATE_RUN_SEND_MSG_TO_CLIENT_OP_DONE);
    return WaitingType::done_and_destroy;
}





int md_create_run_stub (sqlite3 *db, const md_create_run_args &args,
                        uint64_t &run_id)
{
    int rc;
    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;

    char * ErrMsg = NULL;
    char *sql;

    // cout << "about to start md_create_run_stub" << endl;


    // sql = "SELECT * FROM run_catalog";
    // cout << "at beginning, starting run_catalog \n";
    // rc = sqlite3_exec(db, sql, callback, 0, &ErrMsg);
    // cout << "done with run_catalog \n";

    //cout << "creating savepoint run" << endl;

    //pulled this code inside get_database
    // rc = sqlite3_exec (db, "savepoint run;", callback, 0, &ErrMsg);
    // if (rc != SQLITE_OK)
    // {
    //     fprintf (stderr, "Error md_create_run_stub: Line: %d SQL error: %s\n", __LINE__, ErrMsg);
    //     sqlite3_free (ErrMsg);
    //     close_database(args.job_id);
    // }

    // size_t len = 0;
 
    // cout << "about to sqlite3_prepare_v2" << endl;
    // cout << "error_msg: " << sqlite3_errmsg (db) << endl;

    rc = sqlite3_prepare_v2 (db, "insert into run_catalog (id, job_id, name, date, npx, npy, npz, rank_to_dims_funct, objector_funct) values (?, ?, ?, datetime('now','localtime'), ?, ?, ?, ?, ?)", -1, &stmt, &tail);
    if (rc != SQLITE_OK)
    {
        fprintf (stderr, "Error start of create_run_stub: Line: %d SQL error: %s\n", __LINE__, sqlite3_errmsg (db));
        close_database(args.job_id);
        goto cleanup;
    }

    // cout << "got here " << endl;

    // // cout << "name: " << args.name << " path: " << args.path <<
    // cout << "name: " << args.name << " job_id: " << args.job_id <<
    //  " npx: " << args.npx << " npy: " << args.npy << " npz: " << args.npz << endl;

    // cout << "rank_to_dims_funct " << args.rank_to_dims_funct.c_str() << endl;
    // cout << "objector_funct " << args.objector_funct.c_str() << endl;
   
    
    rc = sqlite3_bind_null (stmt, 1); //assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 2, args.job_id); //assert (rc == SQLITE_OK);
    rc = sqlite3_bind_text (stmt, 3, strdup(args.name.c_str()), -1, free); //assert (rc == SQLITE_OK);
    // rc = sqlite3_bind_text (stmt, 3, strdup(args.path.c_str()), -1, free); //assert (rc == SQLITE_OK);
    // rc = sqlite3_bind_text (stmt, 3, datetime('now','localtime'), -1, free); //assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int (stmt, 4, args.npx); //assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int (stmt, 5, args.npy); //assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int (stmt, 6, args.npz); //assert (rc == SQLITE_OK);
    if(args.rank_to_dims_funct.size() > 0) {
        rc = sqlite3_bind_text (stmt, 7, args.rank_to_dims_funct.c_str(), args.rank_to_dims_funct.size() + 1, SQLITE_TRANSIENT); //assert (rc == SQLITE_OK);
    }
    if(args.objector_funct.size() > 0) {
        rc = sqlite3_bind_text (stmt, 8, args.objector_funct.c_str(), args.objector_funct.size()+1, SQLITE_TRANSIENT); //assert (rc == SQLITE_OK);
    }

   //  char *rank_to_dims_funct = malloc(args.rank_to_dims_funct.size() + 1);
   //  char *objector_funct = malloc(args.objector_funct.size() + 1);
   

    // archived_msg.copy(&msg->body[0], archived_msg.size());
    // msg->body[archived_msg.size()] = '\0';

    // rc = sqlite3_bind_blob(stmt, 9, (void*)args.rank_to_dims_funct.c_str(), args.rank_to_dims_funct.size()+1, free);
    // rc = sqlite3_bind_blob(stmt, 10, (void*)args.objector_funct.c_str(), args.objector_funct.size()+1, free);

    // rc = sqlite3_bind_blob(stmt, 9, (void*)&args.rank_to_dims_funct[0], args.rank_to_dims_funct.size()+1, free);
    // rc = sqlite3_bind_blob(stmt, 10, (void*)&args.objector_funct[0], args.objector_funct.size()+1, free);
    // rc = sqlite3_bind_blob(stmt, 9, static_cast<void*>(&args.rank_to_dims_funct[0]), args.rank_to_dims_funct.size()+1, free);
    // rc = sqlite3_bind_blob(stmt, 10, static_cast<void*>(&args.objector_funct[0]), args.objector_funct.size()+1, free);
    

    // cout << "name: " << args.name << " path: " << args.path << << " args.npx: " << args.npx << " args.npy: " << args.npy << " args.npz: " << args.npz << endl;
    // // cout << "rank_to_dims_funct.size(): " << rank_to_dims_funct.size() << " objector_funct.size(): " << objector_funct.size() << endl;
    // cout << "rank_to_dims_funct: " << args.rank_to_dims_funct << " objector_funct: " << args.objector_funct << endl;

    rc = sqlite3_step (stmt); //assert (rc == SQLITE_ROW || rc == SQLITE_DONE);
    // cout << "got here 2 \n";


    // sql = "SELECT * FROM run_catalog";
    // cout << "at end, starting run_catalog \n";
    // rc = sqlite3_exec(db, sql, callback, 0, &ErrMsg);
    // cout << "done with run_catalog \n";



    run_id = (int) sqlite3_last_insert_rowid (db);
    // cout << "OpCreateRunMeta: run_id: " << run_id << " \n";

    rc = sqlite3_finalize (stmt);
    // cout << "rc: " << rc << " \n";

cleanup:

    return rc;
}