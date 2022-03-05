

#include <OpInsertRunAttributeBatchMetaCommon.hh>

#include <OpCoreClient.hh>
using namespace std;


OpInsertRunAttributeBatchMeta::OpInsertRunAttributeBatchMeta(opbox::net::peer_ptr_t dst , const vector<md_insert_run_attribute_args> &args) 
    : OpCore(true) {
    peer = dst;
    // 
    // add_timing_point(OP_INSERT_RUN_ATTRIBUTE_BATCH_START);

    std::string serial_str = serializeMsgToServer(args);
    add_timing_point(OP_INSERT_RUN_ATTRIBUTE_BATCH_SERIALIZE_MSG_FOR_SERVER);


    createOutgoingMessage(opbox::net::ConvertPeerToNodeID(dst),
                      GetAssignedMailbox(), 
                      0, 
                      serial_str);
    // 
    // add_timing_point(OP_INSERT_RUN_ATTRIBUTE_BATCH_CREATE_MSG_FOR_SERVER); 
  //Work picks up again in Server's state machine

}

WaitingType OpInsertRunAttributeBatchMeta::handleMessage(bool placeholder, message_t *incoming_msg) {
    OpCore::handleMessage(placeholder, incoming_msg);
}
