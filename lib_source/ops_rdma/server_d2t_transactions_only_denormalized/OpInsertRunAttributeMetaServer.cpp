#include <OpInsertRunAttributeMetaCommon.hh>
#include <server_timing_constants_new.hh>
#include <sql_helper_functs.hh>
#include <OpCoreServer.hh>

extern sqlite3 *db;

using namespace std;

int md_insert_run_attribute_stub (const md_insert_run_attribute_args &args, uint64_t &attribute_id);


WaitingType OpInsertRunAttributeMeta::handleMessage(bool rdma, message_t *incoming_msg) {
    
    md_insert_run_attribute_args sql_args;
    uint64_t attribute_id;

    deArchiveMsgFromClient(rdma, incoming_msg, sql_args);        add_timing_point(OP_INSERT_RUN_ATTRIBUTE_DEARCHIVE_MSG_FROM_CLIENT);

    int rc = md_insert_run_attribute_stub(sql_args, attribute_id);
    add_timing_point(OP_INSERT_RUN_ATTRIBUTE_MD_INSERT_RUN_ATTRIBUTE_STUB);

    // cout << "according to the server the attribute id is still " << attribute_id <<endl;
    std::string serial_str = serializeMsgToClient(attribute_id, rc);
    add_timing_point(OP_INSERT_RUN_ATTRIBUTE_SERIALIZE_MSG_FOR_CLIENT);

    createOutgoingMessage(dst, 
                          0, //Not expecting a reply
                          dst_mailbox,
                          serial_str);
    add_timing_point(OP_INSERT_RUN_ATTRIBUTE_CREATE_MSG_FOR_CLIENT);

    opbox::net::SendMsg(peer, ldo_msg);
    state=State::done;
    add_timing_point(OP_INSERT_RUN_ATTRIBUTE_SEND_MSG_TO_CLIENT_OP_DONE);
    return WaitingType::done_and_destroy;

}




int md_insert_run_attribute_stub (const md_insert_run_attribute_args &args,
                          uint64_t &attribute_id)
{
    int rc;
    int i = 0;
    // int i;
    // char * ErrMsg = NULL;
    sqlite3_stmt * stmt = NULL;
    const char * tail_index = NULL;

    // int rowid;

    rc = sqlite3_prepare_v2 (db, "insert into run_attribute_catalog (id, run_id, type_id, active, txn_id, data_type, data) values (?, ?, ?, ?, ?, ?, ?)", -1, &stmt, &tail_index);
    if (rc != SQLITE_OK)
    {
        fprintf (stderr, "Error at start of insert_run_attribute_stub: Line: %d SQL error: %s\n", __LINE__, sqlite3_errmsg (db));
        sqlite3_close (db);
        goto cleanup;
    }

    rc = sqlite3_bind_null (stmt, 1); //assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 2, args.run_id); //assert (rc == SQLITE_OK);    
    rc = sqlite3_bind_int64 (stmt, 3, args.type_id); //assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int (stmt, 4, 0); //assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 5, args.txn_id); //assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int (stmt, 6, args.data_type); //assert (rc == SQLITE_OK);
    switch(args.data_type) {
        case ATTR_DATA_TYPE_NULL : {
            rc = sqlite3_bind_null (stmt, 7); 
            break;
        }        
        case ATTR_DATA_TYPE_INT : {
            long long deserialized_int = stoll(args.data);
            // stringstream sso;
            // sso << args.data;
            // boost::archive::text_iarchive ia(sso);
         //    ia >> deserialized_int;
            rc = sqlite3_bind_int64 (stmt, 7, deserialized_int); 
            break;
        }
        case ATTR_DATA_TYPE_REAL : {
            long double deserialized_real = stold(args.data);
            // stringstream sso;
            // sso << args.data;
            // boost::archive::text_iarchive ia(sso);
         //    ia >> deserialized_real;
            rc = sqlite3_bind_double (stmt, 7, deserialized_real); 
            break;
        }
        case ATTR_DATA_TYPE_TEXT : {
            rc = sqlite3_bind_text (stmt, 7, strdup(args.data.c_str()), -1, free);
            break;
        }
        case ATTR_DATA_TYPE_BLOB : {
            // rc = sqlite3_bind_text (stmt, 7, args.data.c_str(), args.data.size()+1, free);
            rc = sqlite3_bind_text (stmt, 7, args.data.c_str(), args.data.size()+1, SQLITE_TRANSIENT);
            break;
        }
    }

    rc = sqlite3_step (stmt); //assert (rc == SQLITE_ROW || rc == SQLITE_DONE);


    attribute_id = (int) sqlite3_last_insert_rowid (db);
    // std::cout << " According to the server, the attr id is " << attribute_id << std::endl;

    rc = sqlite3_finalize (stmt);


cleanup:

    return rc;
}