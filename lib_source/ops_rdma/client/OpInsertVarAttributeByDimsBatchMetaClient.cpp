

#include <OpInsertVarAttributeByDimsBatchMetaCommon.hh>

#include <OpCoreClient.hh>
using namespace std;


// const unsigned int OpInsertVarAttributeByDimsBatchMeta::op_id = const_hash("OpInsertVarAttributeByDimsBatchMeta");
// const string OpInsertVarAttributeByDimsBatchMeta::op_name = "OpInsertVarAttributeByDimsBatchMeta";   


// OpInsertVarAttributeByDimsBatchMeta::OpInsertVarAttributeByDimsBatchMeta(op_create_as_target_t t) 
//   : state(State::start), ldo_msg(), Op(t) {
//   //No work to do - done in Client's state machine
// }

// OpInsertVarAttributeByDimsBatchMeta::~OpInsertVarAttributeByDimsBatchMeta() {
//   //   if((state == State::start) && (ldo_msg != nullptr)) {
//   //   net::ReleaseMessage(ldo_msg);
//   // }
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

//   std::string OpInsertVarAttributeByDimsBatchMeta::GetStateName() const {
//   switch(state){
//   case State::start:              return "Start";
//   case State::snd_wait_for_reply: return "Sender-WaitForReply";
//   case State::done:               return "Done";
//   }
//   KFAIL();
// }


// WaitingType OpInsertVarAttributeByDimsBatchMeta::UpdateOrigin(OpArgs *args) {

//   message_t *incoming_msg;
//   // char *user_data;

//   switch(state){
//   case State::start:

//     opbox::net::SendMsg(peer, std::move(ldo_msg));
//     // net::SendMsg(peer, ldo_msg);
//     state=State::snd_wait_for_reply;
//     add_timing_point(OP_INSERT_VAR_ATTRIBUTE_BY_DIMS_BATCH_SEND_MSG_TO_SERVER);
//     return WaitingType::waiting_on_cq;

//   case State::snd_wait_for_reply:
//     //assert(args->type == UpdateType::incoming_message &&
//            "Sender in snd_wait_for_reply, but event not an incoming message");
//     add_timing_point(OP_INSERT_VAR_ATTRIBUTE_BY_DIMS_BATCH_RETURN_MSG_RECEIVED_FROM_SERVER); 
    
//     incoming_msg = args->ExpectMessageOrDie<message_t *>();
    
//     // user_data = incoming_msg->body;
//     // return_msg_promise.set_value(user_data);
//     return_msg_promise.set_value(incoming_msg->body);
//     state=State::done;
//     // 
//     // add_timing_point(OP_INSERT_VAR_ATTRIBUTE_BY_DIMS_BATCH_PROMISE_VAL_SET_OP_DONE);  

//     case State::done:
//         return WaitingType::done_and_destroy;
//     }

//   KFAIL();
//     return WaitingType::error;

// }


// WaitingType OpInsertVarAttributeByDimsBatchMeta::UpdateTarget(OpArgs *args) {
//     return WaitingType::done_and_destroy;
// }

// // OpInsertVarAttributeByDimsBatchMeta::OpInsertVarAttributeByDimsBatchMeta(opbox::net::peer_ptr_t dst , const vector<md_insert_var_attribute_by_dims_args> &args) 
// // : state(State::start), ldo_msg(), Op(true) {
// //     peer = dst;
// //     // 
// //     // add_timing_point(OP_INSERT_VAR_ATTRIBUTE_BY_DIMS_BATCH_START);

// //     std::string serial_str = serializeMsgToServer(args);
// //     // cout << "message body: " << serial_str << endl;

// //     // allocate an Lunasa DataObject (LDO) to be the target of RDMA operations
// //     lunasa::DataObject msg = lunasa::DataObject(0, serial_str.size()+1, lunasa::DataObject::AllocatorType::eager);
// //     // copy the message into the RDMA target
// //     memcpy(msg.GetDataPtr(), serial_str.c_str(), serial_str.size()+1);
// //     ping_ldo = msg;

// //     add_timing_point(OP_INSERT_VAR_ATTRIBUTE_BY_DIMS_BATCH_SERIALIZE_MSG_FOR_SERVER);


// //     createOutgoingMessage(opbox::net::ConvertPeerToNodeID(dst),
// //                       GetAssignedMailbox(), 
// //                       0, 
// //                       ping_ldo);

// //     // createOutgoingMessage(opbox::net::ConvertPeerToNodeID(dst),
// //     //                   GetAssignedMailbox(), 
// //     //                   0, 
// //     //                   serial_str);
// //     // 
// //     // add_timing_point(OP_INSERT_VAR_ATTRIBUTE_BY_DIMS_BATCH_CREATE_MSG_FOR_SERVER); 
// //   //Work picks up again in Server's state machine

// //   }

// OpInsertVarAttributeByDimsBatchMeta::OpInsertVarAttributeByDimsBatchMeta(opbox::net::peer_ptr_t dst , const vector<md_insert_var_attribute_by_dims_args> &args) 
// : state(State::start), ldo_msg(), Op(true) {
//     peer = dst;
//     // 
//     // add_timing_point(OP_INSERT_VAR_ATTRIBUTE_BY_DIMS_BATCH_START);

//     std::string serial_str = serializeMsgToServer(args);
//     add_timing_point(OP_INSERT_VAR_ATTRIBUTE_BY_DIMS_BATCH_SERIALIZE_MSG_FOR_SERVER);


//     createOutgoingMessage(opbox::net::ConvertPeerToNodeID(dst),
//                       GetAssignedMailbox(), 
//                       0, 
//                       serial_str);
//     // 
//     // add_timing_point(OP_INSERT_VAR_ATTRIBUTE_BY_DIMS_BATCH_CREATE_MSG_FOR_SERVER); 
//   //Work picks up again in Server's state machine

//   }


// //Means for passing back the result
// std::future<string> OpInsertVarAttributeByDimsBatchMeta::GetFuture() {
//   return return_msg_promise.get_future();
// }


// std::string OpInsertVarAttributeByDimsBatchMeta::serializeMsgToServer(const vector<md_insert_var_attribute_by_dims_args> &args) {

//   std::stringstream ss;
//   boost::archive::text_oarchive oa(ss);
//   oa << args;
//   //log("the archived message is " + ss.str());

//   return ss.str();
// }




// WaitingType OpInsertVarAttributeByDimsBatchMeta::UpdateOrigin(OpArgs *args) {

//   message_t *incoming_msg;
//   // char *user_data;

//   switch(state){
//   case State::start:

//     opbox::net::SendMsg(peer, std::move(ldo_msg));
//     // net::SendMsg(peer, ldo_msg);
//     state=State::snd_wait_for_reply;
//     add_timing_point(OP_INSERT_VAR_ATTRIBUTE_BY_DIMS_BATCH_SEND_MSG_TO_SERVER);
//     return WaitingType::waiting_on_cq;

//   case State::snd_wait_for_reply:
//     //assert(args->type == UpdateType::incoming_message &&
//            "Sender in snd_wait_for_reply, but event not an incoming message");
//     add_timing_point(OP_INSERT_VAR_ATTRIBUTE_BY_DIMS_BATCH_RETURN_MSG_RECEIVED_FROM_SERVER); 
    
//     incoming_msg = args->ExpectMessageOrDie<message_t *>();
    
//     // user_data = incoming_msg->body;
//     // return_msg_promise.set_value(user_data);
//     return_msg_promise.set_value(incoming_msg->body);
//     state=State::done;
//     // 
//     // add_timing_point(OP_INSERT_VAR_ATTRIBUTE_BY_DIMS_BATCH_PROMISE_VAL_SET_OP_DONE);  

//     case State::done:
//         return WaitingType::done_and_destroy;
//     }

//   KFAIL();
//     return WaitingType::error;

// }


// WaitingType OpInsertVarAttributeByDimsBatchMeta::UpdateTarget(OpArgs *args) {
//     return WaitingType::done_and_destroy;
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

// OpInsertVarAttributeByDimsBatchMeta::OpInsertVarAttributeByDimsBatchMeta(opbox::net::peer_ptr_t dst , const vector<md_insert_var_attribute_by_dims_args> &args) 
// : state(State::start), ldo_msg(), OpCore(true) {
//     peer = dst;
//     // 
//     // add_timing_point(OP_INSERT_VAR_ATTRIBUTE_BY_DIMS_BATCH_START);

//     std::string serial_str = serializeMsgToServer(args);
//     add_timing_point(OP_INSERT_VAR_ATTRIBUTE_BY_DIMS_SERIALIZE_MSG_FOR_SERVER);


//     createOutgoingMessage(opbox::net::ConvertPeerToNodeID(dst),
//                       GetAssignedMailbox(), 
//                       0, 
//                       serial_str);
//     // 
//     // add_timing_point(OP_INSERT_VAR_ATTRIBUTE_BY_DIMS_BATCH_CREATE_MSG_FOR_SERVER); 
//   //Work picks up again in Server's state machine

//   }


// //Means for passing back the result
// std::future<string> OpInsertVarAttributeByDimsBatchMeta::GetFuture() {
//   return return_msg_promise.get_future();
// }


// std::string OpInsertVarAttributeByDimsBatchMeta::serializeMsgToServer(const vector<md_insert_var_attribute_by_dims_args> &args) {

//   std::stringstream ss;
//   boost::archive::text_oarchive oa(ss);
//   oa << args;
//   //log("the archived message is " + ss.str());

//   return ss.str();
// }


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


OpInsertVarAttributeByDimsBatchMeta::OpInsertVarAttributeByDimsBatchMeta(opbox::net::peer_ptr_t dst , const vector<md_insert_var_attribute_by_dims_args> &args) 
    : OpCore(true) {
    peer = dst;


  //   createOutgoingMessage(net::ConvertPeerToNodeID(dst),
  //                                  GetAssignedMailbox(),  
  //                                  0,
  //                                  args,
  //                                  true
  //                       );
  // //   // 
    // add_timing_point(OP_INSERT_VAR_ATTRIBUTE_BY_DIMS_BATCH_START);

    std::string serial_str = serializeMsgToServer(args);
    add_timing_point(OP_INSERT_VAR_ATTRIBUTE_BY_DIMS_BATCH_SERIALIZE_MSG_FOR_SERVER);


    createOutgoingMessage(opbox::net::ConvertPeerToNodeID(dst), 
                      GetAssignedMailbox(), 
                      0, 
                      serial_str);
    // 
    // add_timing_point(OP_INSERT_VAR_ATTRIBUTE_BY_DIMS_BATCH_CREATE_MSG_FOR_SERVER); 
  //Work picks up again in Server's state machine

}

WaitingType OpInsertVarAttributeByDimsBatchMeta::handleMessage(bool placeholder, message_t *incoming_msg) {
    OpCore::handleMessage(placeholder, incoming_msg);
}


// std::string OpInsertVarAttributeByDimsBatchMeta::serializeMsgToServer(const vector<md_insert_var_attribute_by_dims_args> &args) {
//   std::stringstream ss;
//   boost::archive::text_oarchive oa(ss);
//   oa << args;
//   //log("the archived message is " + ss.str());

//   return ss.str();
// }

