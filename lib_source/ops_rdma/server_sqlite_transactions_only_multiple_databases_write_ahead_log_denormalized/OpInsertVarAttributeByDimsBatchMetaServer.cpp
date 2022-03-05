#include <OpInsertVarAttributeByDimsBatchMetaCommon.hh>
#include <server_timing_constants_new.hh>
#include <sql_helper_functs.hh>
#include <OpCoreServer.hh>
#include <sys/time.h> //needed for timeval

#include <sched.h> //needed for temp debuging

#include "opbox/ops/OpHelpers.hh" //needed for AllEventsCallback

extern sqlite3 * get_database_for_write(uint64_t txn_id);
extern sqlite3 *main_db;
extern int callback (void * NotUsed, int argc, char ** argv, char ** ColName);

extern sqlite3_int64 get_db_size();

using namespace std;


// double get_time() {
//     struct timeval now;
//     gettimeofday(&now, NULL);
//     if(start_time == 0) {
//         start_time = now.tv_sec + now.tv_usec / 1000000.0;
//         return (start_time);
//     }
//     else {
//         return ((now.tv_sec + now.tv_usec / 1000000.0) - start_time);
//     }
// }
// double get_time() {
//     struct timeval now;
//     gettimeofday(&now, NULL);
//     return (now.tv_sec + now.tv_usec / 1000000.0);
// }
// int md_insert_var_attribute_by_dims_batch_stub (sqlite3 *db, const vector<md_insert_var_attribute_by_dims_args> &args, vector<uint64_t> &attribute_ids);
int md_insert_var_attribute_by_dims_batch_stub (sqlite3 *db, const vector<md_insert_var_attribute_by_dims_args> &all_args);
// extern int callback (void * NotUsed, int argc, char ** argv, char ** ColName);


// WaitingType OpInsertVarAttributeByDimsBatchMeta::UpdateTarget(OpArgs *args) {
    
//     vector<md_insert_var_attribute_by_dims_args> sql_args;
//     // vector<uint64_t> attribute_ids;

//   switch(state){

//     case State::start: {
//         add_timing_point(OP_INSERT_VAR_ATTRIBUTE_BY_DIMS_BATCH_START);


//         message_t *incoming_msg =  args->ExpectMessageOrDie<message_t *>(&peer);

        

//         deArchiveMsgFromClient(false, incoming_msg, sql_args);
//         add_timing_point(OP_INSERT_VAR_ATTRIBUTE_BY_DIMS_BATCH_DEARCHIVE_MSG_FROM_CLIENT);

//         // rc = md_insert_var_attribute_by_dims_batch_stub(db, sql_args, attribute_ids);
//         rc = md_insert_var_attribute_by_dims_batch_stub(db, sql_args);
//         add_timing_point(OP_INSERT_VAR_ATTRIBUTE_BY_DIMS_BATCH_MD_INSERT_VAR_ATTRIBUTE_BY_DIMS_BATCH_STUB);

//         // //cout << "according to the server the attribute id is still " << attribute_id <<endl;
//         // std::string serial_str = serializeMsgToClient(rc, attribute_ids);
//         // std::string serial_str = serializeMsgToClient(rc);    
//         add_timing_point(OP_INSERT_VAR_ATTRIBUTE_BY_DIMS_BATCH_SERIALIZE_MSG_FOR_CLIENT);

//         createOutgoingMessage(incoming_msg->src, 
//                               0, //Not expecting a reply
//                               incoming_msg->src_mailbox,
//                               to_string(rc));
//         add_timing_point(OP_INSERT_VAR_ATTRIBUTE_BY_DIMS_BATCH_CREATE_MSG_FOR_CLIENT);

//         opbox::net::SendMsg(peer, ldo_msg);
//         state=State::done;
//         add_timing_point(OP_INSERT_VAR_ATTRIBUTE_BY_DIMS_BATCH_SEND_MSG_TO_CLIENT_OP_DONE);
//         return WaitingType::done_and_destroy;
//     }
//     case State::done:
//         return WaitingType::done_and_destroy;
//     }
// }

// //WaitingType OpInsertVarAttributeByDimsBatchMeta::UpdateOrigin(OpArgs *args) {
// WaitingType OpInsertVarAttributeByDimsBatchMeta::UpdateOrigin(OpArgs *) {
  
//     return WaitingType::error;

// }

// void OpInsertVarAttributeByDimsBatchMeta::createOutgoingMessage(faodel::nodeid_t dst , 
//                                    const mailbox_t &src_mailbox, 
//                                    const mailbox_t &dst_mailbox,
//                                    const std::string &archived_msg){  

//   ldo_msg = opbox::net::NewMessage( sizeof(message_t)+archived_msg.size()+1);
//   message_t *msg = ldo_msg.GetDataPtr<message_t *>();
//   msg->src           = opbox::net::GetMyID();
//   msg->dst           = dst;
//   msg->src_mailbox   = src_mailbox;
//   msg->dst_mailbox   = dst_mailbox;
//   msg->op_id         = OpInsertVarAttributeByDimsBatchMeta::op_id;
//   msg->body_len      = archived_msg.size()+1;
//   archived_msg.copy(&msg->body[0], archived_msg.size());
//   msg->body[archived_msg.size()] = '\0';
//   }

// OpInsertVarAttributeByDimsBatchMeta::OpInsertVarAttributeByDimsBatchMeta(op_create_as_target_t t) 
//   : state(State::start), ldo_msg(), OpCore(t) {
//   //No work to do - done in Client's state machine
// }

// OpInsertVarAttributeByDimsBatchMeta::~OpInsertVarAttributeByDimsBatchMeta() {
//   //   if((state == State::start) && (ldo_msg != nullptr)) {
//   //   net::ReleaseMessage(ldo_msg);
//   // }
// }

// std::string OpInsertVarAttributeByDimsBatchMeta::GetStateName() const {
//   switch(state){
//   case State::start:              return "Start";
//   case State::snd_wait_for_reply: return "Sender-WaitForReply";
//   case State::done:               return "Done";
//   }
//   KFAIL();
// }

// void OpInsertVarAttributeByDimsBatchMeta::deArchiveMsgFromClient(bool rdma, message_t *incoming_msg, vector<md_insert_var_attribute_by_dims_args> &all_args) {

//     std::stringstream ss;
//     // cout << "message body: " << incoming_msg->body << endl;
//     ss.write(incoming_msg->body, incoming_msg->body_len);
//     boost::archive::text_iarchive ia(ss);
//     ia >> all_args;
//     // //cout << "deserialized text string: " << ss.str() << endl;
//     // //cout << "deserialized text string length: " << ss.str().size() << endl;

// }


WaitingType OpInsertVarAttributeByDimsBatchMeta::handleMessage(bool rdma, message_t *incoming_msg) {
    
    int rc;
    vector<md_insert_var_attribute_by_dims_args> sql_args;
    // vector<uint64_t> attribute_ids;

    deArchiveMsgFromClient(rdma, incoming_msg, sql_args);
    // sqlite3 *db = get_database(sql_args[0].txn_id);
    sqlite3 *db = get_database_for_write(sql_args[0].txn_id);

    add_timing_point(OP_INSERT_VAR_ATTRIBUTE_BY_DIMS_BATCH_DEARCHIVE_MSG_FROM_CLIENT);

    // rc = md_insert_var_attribute_by_dims_batch_stub(db, sql_args, attribute_ids);
     rc = md_insert_var_attribute_by_dims_batch_stub(db, sql_args);
    add_timing_point(OP_INSERT_VAR_ATTRIBUTE_BY_DIMS_BATCH_MD_INSERT_VAR_ATTRIBUTE_BY_DIMS_BATCH_STUB);

    // //cout << "according to the server the attribute id is still " << attribute_id <<endl;
    // std::string serial_str = serializeMsgToClient(attribute_ids, rc);
    // std::string serial_str = serializeMsgToClient(rc);    
    add_timing_point(OP_INSERT_VAR_ATTRIBUTE_BY_DIMS_BATCH_SERIALIZE_MSG_FOR_CLIENT);

    createOutgoingMessage(dst, 
                          0, //Not expecting a reply
                          dst_mailbox,
                          to_string(rc));
    add_timing_point(OP_INSERT_VAR_ATTRIBUTE_BY_DIMS_BATCH_CREATE_MSG_FOR_CLIENT);

    opbox::net::SendMsg(peer, ldo_msg);
    state=State::done;
    add_timing_point(OP_INSERT_VAR_ATTRIBUTE_BY_DIMS_BATCH_SEND_MSG_TO_CLIENT_OP_DONE);
    return WaitingType::done_and_destroy;

}


// int md_insert_var_attribute_by_dims_batch_stub (sqlite3 *db, const vector<md_insert_var_attribute_by_dims_args> &args,
//                           vector<uint64_t> &attribute_ids)
int md_insert_var_attribute_by_dims_batch_stub (sqlite3 *db, const vector<md_insert_var_attribute_by_dims_args> &all_args)
{
    int rc = RC_OK;
    // int i;
    // char * ErrMsg = NULL;
    sqlite3_stmt * stmt = NULL;
    const char * tail_index = NULL;
    char * ErrMsg = NULL;

    // rc = sqlite3_exec (db, "savepoint var_attribute;", callback, 0, &ErrMsg);
    // if (rc != SQLITE_OK)
    // {
    //     fprintf (stderr, "Error md_insert_var_attribute_by_dims_batch_stub: Line: %d SQL error: %s\n", __LINE__, ErrMsg);
    //     sqlite3_free (ErrMsg);
    //     sqlite3_close (db);
    // }

    // int rowid;

    // cout << "in md_insert_var_attribute_by_dims_batch_stub, run_id: " << all_args[0].run_id << endl;

    
    if(all_args.size() > 0) {
        // cout << "beginning" << endl;
        //todo - test - how does adjusting this to be prepare change things?    
        // rc = sqlite3_exec (db, "begin;", 0, 0, &ErrMsg);
        // if (rc != SQLITE_OK)
        // {
        //     fprintf (stderr, "Error begin md_insert_var_attribute_by_dims_batch_stub: Line: %d SQL error: %s\n", __LINE__, ErrMsg);
        //     sqlite3_free (ErrMsg);
        //     sqlite3_close (db);
        //     goto cleanup;
        // }

        rc = sqlite3_prepare_v2 (db, "insert into var_attribute_catalog (id, run_id, timestep_id, type_id, var_id, num_dims, d0_min, d0_max, d1_min, d1_max, d2_min, d2_max, data_type, data) values (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)", -1, &stmt, &tail_index);
        if (rc != SQLITE_OK)
        {
            fprintf (stderr, "Error at start of insert_var_attribute_stub: Line: %d SQL error: %s\n", __LINE__, sqlite3_errmsg (db));
            sqlite3_close (db);
            goto cleanup;
        }
    }


    for(int arg_indx = 0; arg_indx < all_args.size(); arg_indx++) {
        int dim = 0;

        md_insert_var_attribute_by_dims_args args = all_args.at(arg_indx);

        rc = sqlite3_bind_null (stmt, 1); //assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int64 (stmt, 2, args.run_id); //assert (rc == SQLITE_OK);   
        rc = sqlite3_bind_int64 (stmt, 3, args.timestep_id); //assert (rc == SQLITE_OK);   
        rc = sqlite3_bind_int64 (stmt, 4, args.type_id); //assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int64 (stmt, 5, args.var_id); //assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int (stmt, 6, args.num_dims); //assert (rc == SQLITE_OK);
        while(dim < args.num_dims) {

            rc = sqlite3_bind_double (stmt, 7 + dim * 2, args.dims.at(dim).min); //assert (rc == SQLITE_OK);
            rc = sqlite3_bind_double (stmt, 8 + dim * 2, args.dims.at(dim).max); //assert (rc == SQLITE_OK);
            dim++;
        }
        //if num dims < 3 explicitly bind null to prevent bleed over from prev call
        while(dim < 3) {
            rc = sqlite3_bind_null (stmt, 7 + dim * 2); 
            rc = sqlite3_bind_null (stmt, 8 + dim * 2); 
            dim++;
        }
        rc = sqlite3_bind_int (stmt, 13, args.data_type); //assert (rc == SQLITE_OK);

        switch(args.data_type) {
            case ATTR_DATA_TYPE_NULL : {
                rc = sqlite3_bind_null (stmt, 14); 
                break;
            }        
            case ATTR_DATA_TYPE_INT : {
                long long deserialized_int = stoll(args.data);
                rc = sqlite3_bind_int64 (stmt, 14, deserialized_int); 
                break;
            }
            case ATTR_DATA_TYPE_REAL : {
                long double deserialized_real = stold(args.data);
                rc = sqlite3_bind_double (stmt, 14, deserialized_real); 
                break;
            }
            case ATTR_DATA_TYPE_TEXT : {
                rc = sqlite3_bind_text (stmt, 14, strdup(args.data.c_str()), -1, free);
                // rc = sqlite3_bind_text (stmt, 15, strdup(args.data.c_str()), -1, SQLITE_TRANSIENT);
                // rc = sqlite3_bind_text (stmt, 15, args.data.c_str(), -1, SQLITE_STATIC);
                // rc = sqlite3_bind_text (stmt, 15, args.data.c_str(), -1, SQLITE_TRANSIENT);
                // rc = sqlite3_bind_text (stmt, 15, args.data.c_str(), args.data.size()+1, SQLITE_TRANSIENT);
                // rc = sqlite3_bind_text (stmt, 15, args.data.c_str(), args.data.size(), SQLITE_STATIC);
                // rc = sqlite3_bind_text (stmt, 15, all_args.at(arg_indx).data.c_str(), -1, SQLITE_STATIC);



                break;
            }
            case ATTR_DATA_TYPE_BLOB : {
                //cout << "blob: " << args.data.c_str() << endl;
                //cout << "blob: " << args.data << " str size: " << args.data.size() << endl;
                // rc = sqlite3_bind_text (stmt, 15, strdup(args.data.c_str()), args.data.size()+1, free);
                rc = sqlite3_bind_text (stmt, 14, args.data.c_str(), args.data.size()+1, SQLITE_TRANSIENT);
                // rc = sqlite3_bind_text (stmt, 15, args.data.c_str(), args.data.size(), SQLITE_STATIC);
                // rc = sqlite3_bind_text (stmt, 15, all_args.at(arg_indx).data.c_str(), all_args.at(arg_indx).data.size()+1, SQLITE_STATIC);

                // rc = sqlite3_bind_blob64 (stmt, 15, args.data, -1, free); 
                break;
            }
        }
        //assert (rc == SQLITE_OK);

        rc = sqlite3_step (stmt); //assert (rc == SQLITE_ROW || rc == SQLITE_DONE);

        // //cout << "just stepped \n";
        //todo - how does clearing bindings affect things?
        //todo - do we need to be freeing things from c_str()?

        rc = sqlite3_reset(stmt); //assert (rc == SQLITE_OK);
       
    }
    // done_insert_time = get_time();
    // cout << "about to finalize" << endl;
    // cout << "id: " << std::this_thread::get_id() << endl;

    if(all_args.size() > 0) {
        rc = sqlite3_finalize (stmt);
    }



cleanup:
    // if (rc != SQLITE_OK)
    // {
    //     rc = sqlite3_exec (db, "rollback;", NULL, 0, &ErrMsg);
    // }
    return rc;
}
