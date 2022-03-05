#include <OpCheckpointDatabaseMetaCommon.hh>

#include <OpCoreClient.hh>
// OpCheckpointDatabaseMeta::OpCheckpointDatabaseMeta(opbox::net::peer_ptr_t dst , uint64_t job_id, int checkpt_type) 
//     : OpCore(true) {
//     peer = dst;
//     // add_timing_point(OP_CHECKPOINT_DATABASE_START);

//     createOutgoingMessage(opbox::net::ConvertPeerToNodeID(dst),
//                       GetAssignedMailbox(), 
//                       0, 
//                       std::to_string(job_id)+"/"+std::to_string(checkpt_type));
//     // add_timing_point(OP_CHECKPOINT_DATABASE_CREATE_MSG_FOR_SERVER);
//   //Work picks up again in Server's state machine
// }

WaitingType OpCheckpointDatabaseMeta::handleMessage(bool placeholder, message_t *incoming_msg) {
    OpCore::handleMessage(placeholder, incoming_msg);
}



OpCheckpointDatabaseMeta::OpCheckpointDatabaseMeta(opbox::net::peer_ptr_t dst , const md_checkpoint_database_args &args) 
    : OpCore(true) {
    peer = dst;
    // add_timing_point(OP_CHECKPOINT_DATABASE_START);

    std::string serial_str = serializeMsgToServer(args);    
    add_timing_point(OP_CHECKPOINT_DATABASE_SERIALIZE_MSG_FOR_SERVER);

    createOutgoingMessage(opbox::net::ConvertPeerToNodeID(dst),
                      GetAssignedMailbox(), 
                      0, 
                      serial_str);
    // add_timing_point(OP_CHECKPOINT_DATABASE_CREATE_MSG_FOR_SERVER);
  //Work picks up again in Server's state machine
}

