

#include <OpCreateTypeBatchMetaCommon.hh>

#include <OpCoreClient.hh>
using namespace std;


OpCreateTypeBatchMeta::OpCreateTypeBatchMeta(opbox::net::peer_ptr_t dst, const vector<md_create_type_args> &args) 
    : OpCore(true) {
    peer = dst;
    // add_timing_point(OP_CREATE_TYPE_BATCH_START);

    // cout << "Client about to serialize a message to server \n";

    std::string serial_str = serializeMsgToServer(args);    
    add_timing_point(OP_CREATE_TYPE_BATCH_SERIALIZE_MSG_FOR_SERVER);
    // cout << "Client about to create outgoing message to server \n";

    createOutgoingMessage(opbox::net::ConvertPeerToNodeID(dst),
                      GetAssignedMailbox(), 
                      0, 
                      serial_str);
    // add_timing_point(OP_CREATE_TYPE_BATCH_CREATE_MSG_FOR_SERVER);
  //Work picks up again in Server's state machine
  // cout << "Client just dispatched message to server \n";

}

WaitingType OpCreateTypeBatchMeta::handleMessage(bool placeholder, message_t *incoming_msg) {
    OpCore::handleMessage(placeholder, incoming_msg);
}
