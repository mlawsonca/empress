#include <OpInsertVarAttributeByDimsBatchMetaCommon.hh>
#include <server_timing_constants_new.hh>
#include <sql_helper_functs.hh>
#include <OpCoreServer.hh>
#include <sys/time.h> //needed for timeval

#include <sched.h> //needed for temp debuging

#include "opbox/ops/OpHelpers.hh" //needed for AllEventsCallback

extern sqlite3 *db;
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
// int md_insert_var_attribute_by_dims_batch_stub (const vector<md_insert_var_attribute_by_dims_args> &args, vector<uint64_t> &attribute_ids);
int md_insert_var_attribute_by_dims_batch_stub (const vector<md_insert_var_attribute_by_dims_args> &all_args);
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

//         // int rc = md_insert_var_attribute_by_dims_batch_stub(sql_args, attribute_ids);
//         int rc = md_insert_var_attribute_by_dims_batch_stub(sql_args);
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
    
    vector<md_insert_var_attribute_by_dims_args> sql_args;
    // vector<uint64_t> attribute_ids;

    deArchiveMsgFromClient(rdma, incoming_msg, sql_args);        
    add_timing_point(OP_INSERT_VAR_ATTRIBUTE_BY_DIMS_BATCH_DEARCHIVE_MSG_FROM_CLIENT);

    // int rc = md_insert_var_attribute_by_dims_batch_stub(sql_args, attribute_ids);
     int rc = md_insert_var_attribute_by_dims_batch_stub(sql_args);
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


// int md_insert_var_attribute_by_dims_batch_stub (const vector<md_insert_var_attribute_by_dims_args> &args,
//                           vector<uint64_t> &attribute_ids)
int md_insert_var_attribute_by_dims_batch_stub (const vector<md_insert_var_attribute_by_dims_args> &all_args)
{
    int rc = RC_OK;
    // int i;
    // char * ErrMsg = NULL;
    sqlite3_stmt * stmt = NULL;
    const char * tail_index = NULL;
    char * ErrMsg = NULL;

    double start_time, start_insert_time, done_insert_time,  done_time, bind_done_time, step_done_time, reset_done_time;
    // double start_insert_time, done_insert_time, done_time;

    // int rowid;

    // start_time = 0;
    // double all_time = 0;
    // double bind_time = 0;

    // char *err_msg = 0;
    // char *sql = "SELECT * FROM var_attribute_catalog";
    // start_time = get_time();
    // cout << "beginning. time: " << get_time() << endl;
    
    if(all_args.size() > 0) {
        // cout << "beginning" << endl;
        //todo - test - how does adjusting this to be prepare change things?    
        rc = sqlite3_exec (db, "begin;", 0, 0, &ErrMsg);
        if (rc != SQLITE_OK)
        {
            fprintf (stderr, "Error begin md_insert_var_attribute_by_dims_batch_stub: Line: %d SQL error: %s\n", __LINE__, ErrMsg);
            sqlite3_free (ErrMsg);
            sqlite3_close (db);
            goto cleanup;
        }

        rc = sqlite3_prepare_v2 (db, "insert into var_attribute_catalog (id, timestep_id, type_id, var_id, active, txn_id, num_dims, d0_min, d0_max, d1_min, d1_max, d2_min, d2_max, data_type, data) values (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)", -1, &stmt, &tail_index);
        if (rc != SQLITE_OK)
        {
            fprintf (stderr, "Error at start of insert_var_attribute_stub: Line: %d SQL error: %s\n", __LINE__, sqlite3_errmsg (db));
            sqlite3_close (db);
            goto cleanup;
        }
    }

    // start_insert_time = get_time();

    // cout << " all_args.size(): " <<  all_args.size() << endl;
    // cout << "id: " << std::this_thread::get_id() << endl;
    // cout << "time: " << get_time() << endl;

    // cout << "timestep: " <<  all_args.at(0).timestep_id << " cpu: " << sched_getcpu() << endl;

    for(int arg_indx = 0; arg_indx < all_args.size(); arg_indx++) {

        // cout << "timestep: " <<  all_args.at(0).timestep_id << " cpu: " << sched_getcpu() << endl;

        // //cout << "got to top of loop" << endl;
        // cout << "loop start time: " << get_time()-start_time << endl;
        // double loop_start_time = get_time();
        int dim = 0;

        // int cpu = sched_getcpu();

        md_insert_var_attribute_by_dims_args args = all_args.at(arg_indx);

        //cout << " args.timestep_id: " <<  args.timestep_id << endl;
        //cout << " args.type_id: " <<  args.type_id << endl;
        //cout << " args.var_id: " <<  args.var_id << endl;
        //cout << " args.txn_id: " <<  args.txn_id << endl;
        //cout << " args.num_dims: " <<  args.num_dims << endl;


        rc = sqlite3_bind_null (stmt, 1); //assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int64 (stmt, 2, args.timestep_id); //assert (rc == SQLITE_OK);   
        rc = sqlite3_bind_int64 (stmt, 3, args.type_id); //assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int64 (stmt, 4, args.var_id); //assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int (stmt, 5, 0); //assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int64 (stmt, 6, args.txn_id); //assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int (stmt, 7, args.num_dims); //assert (rc == SQLITE_OK);
        while(dim < args.num_dims) {
            //cout << " args.d[" << dim << "].min: " <<  args.dims.at(dim).min << endl;
            //cout << " args.d[" << dim << "].max: " <<  args.dims.at(dim).max << endl;

            rc = sqlite3_bind_double (stmt, 8 + dim * 2, args.dims.at(dim).min); //assert (rc == SQLITE_OK);
            rc = sqlite3_bind_double (stmt, 9 + dim * 2, args.dims.at(dim).max); //assert (rc == SQLITE_OK);
            dim++;
        }
        //if num dims < 3 explicitly bind null to prevent bleed over from prev call
        while(dim < 3) {
            rc = sqlite3_bind_null (stmt, 8 + dim * 2); 
            rc = sqlite3_bind_null (stmt, 9 + dim * 2); 
              dim++;
        }
        rc = sqlite3_bind_int (stmt, 14, args.data_type); //assert (rc == SQLITE_OK);
         //cout << " args.data_type: " <<  args.data_type << endl;

        switch(args.data_type) {
            case ATTR_DATA_TYPE_NULL : {
                rc = sqlite3_bind_null (stmt, 15); 
                break;
            }        
            case ATTR_DATA_TYPE_INT : {
                long long deserialized_int = stoll(args.data);
                // long long deserialized_int;
                // stringstream sso;
                // sso << args.data;
                // boost::archive::text_iarchive ia(sso);
                // // //cout << "serialized int data on insertion: " << args.data << endl;
             //    ia >> deserialized_int;
             //    //cout << "deserialized_int: " << deserialized_int << endl;
                rc = sqlite3_bind_int64 (stmt, 15, deserialized_int); 
                break;
            }
            case ATTR_DATA_TYPE_REAL : {
                long double deserialized_real = stold(args.data);
                // long double deserialized_real;
                // stringstream sso;
                // sso << args.data;
                // boost::archive::text_iarchive ia(sso);
             //    ia >> deserialized_real;
             //    // cout << "deserialized_real: " << deserialized_real << endl;
                rc = sqlite3_bind_double (stmt, 15, deserialized_real); 
                break;
            }
            case ATTR_DATA_TYPE_TEXT : {
                // char *data = strdup(args.data.c_str());
             //    rc = sqlite3_bind_text (stmt, 15, data, -1, SQLITE_TRANSIENT);
             //    free(data);
                //cout << "text: " << args.data.c_str() << endl;
                //todo - how does adjusting this change things?
                rc = sqlite3_bind_text (stmt, 15, strdup(args.data.c_str()), -1, free);
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
                //enables us to have embedded nulls in the text
                rc = sqlite3_bind_text (stmt, 15, args.data.c_str(), args.data.size()+1, SQLITE_TRANSIENT);
                // rc = sqlite3_bind_text (stmt, 15, args.data.c_str(), args.data.size(), SQLITE_STATIC);
                // rc = sqlite3_bind_text (stmt, 15, all_args.at(arg_indx).data.c_str(), all_args.at(arg_indx).data.size()+1, SQLITE_STATIC);

                // rc = sqlite3_bind_blob64 (stmt, 15, args.data, -1, free); 
                break;
            }
        }
        // cout << "loop bind done time: " << get_time()-start_time << endl;
        // //cout << "got to bottom of bind \n";
        // bind_done_time = get_time();
        //assert (rc == SQLITE_OK);
        // bind_time += bind_done_time-loop_start_time;


        rc = sqlite3_step (stmt); 
        if(rc != SQLITE_ROW && rc != SQLITE_DONE) {
            sqlite3_extended_result_codes(db, 1);
            cout << "rc: " << rc << ", errMsg: " << sqlite3_errmsg (db) << endl;
            cout << "db size: " << get_db_size() << endl;
        }
        //assert (rc == SQLITE_ROW || rc == SQLITE_DONE);
        rc = sqlite3_reset(stmt); //assert (rc == SQLITE_OK);
        // reset_done_time = get_time();
        // reset_time += get_time()-step_done_time;

        // if(arg_indx==0) {
        //     cout << "bind time, step time, reset time" << endl;
        // }
        // cout << bind_done_time-loop_start_time << ", " << step_done_time-bind_done_time << ", " << reset_done_time-step_done_time << endl;

        // //cout << "just reset. rc: " << rc << " \n";

        // rc = sqlite3_clear_bindings(stmt);
        // //cout << "just cleared bindings. rc: " << rc << " \n";
        // cout << "loop done time: " << get_time()-start_time << endl;
        // all_time += get_time()-loop_start_time;
        // attribute_id = (int) sqlite3_last_insert_rowid (db);
        // attribute_ids.push_back(attribute_id);
    }
    // done_insert_time = get_time();
    // cout << "about to finalize" << endl;
    // cout << "id: " << std::this_thread::get_id() << endl;

    if(all_args.size() > 0) {
        // cout << "ending" << endl;
        rc = sqlite3_finalize (stmt);
        // //cout << "just finalized \n";

        rc = sqlite3_exec (db, "end;", 0, 0, &ErrMsg);
        if (rc != SQLITE_OK)
        {
            fprintf (stderr, "Error end query md_insert_var_attribute_by_dims_batch_stub: Line: %d SQL error: %s\n", __LINE__, ErrMsg);
            sqlite3_free (ErrMsg);
            sqlite3_close (db);
            goto cleanup;
        }
        // cout << "all_args.size(): " << all_args.size() << " insert time: " << done_insert_time - start_time << endl;
        // cout << "all_args.size(): " << all_args.size() << " total time: " << get_time() - start_time << endl;
        // cout << "bind time: " << bind_time << endl;
        // cout << "all_time time: " << all_time << endl;
    }

    // cout << "done finalizing" << endl;
 //    cout << "all_args.size(): " << all_args.size() << " total time: " << done_time - start_time << " insert time: " << done_insert_time-start_insert_time << endl;


    //cout << "starting var catalog \n";
    // rc = sqlite3_exec(db, sql, callback, 0, &err_msg);
    //cout << "done with var catalog \n";


cleanup:
    if (rc != SQLITE_OK)
    {
        rc = sqlite3_exec (db, "rollback;", NULL, 0, &ErrMsg);
    }
    return rc;
}
