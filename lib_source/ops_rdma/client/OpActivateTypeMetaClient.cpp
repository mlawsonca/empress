#include <OpActivateTypeMetaCommon.hh>#include <OpCoreClient.hh>
using namespace std;

OpActivateTypeMeta::OpActivateTypeMeta(opbox::net::peer_ptr_t dst, const md_activate_args &args) 
    : OpCore(true) {
    peer = dst;
    // add_timing_point(OP_ACTIVATE_TYPE_START); 

    std::string serial_str = serializeMsgToServer(args);
    add_timing_point(OP_ACTIVATE_TYPE_SERIALIZE_MSG_FOR_SERVER);



    createOutgoingMessage(opbox::net::ConvertPeerToNodeID(dst),
                      GetAssignedMailbox(), 
                      0, 
                      serial_str);
    // add_timing_point(OP_ACTIVATE_TYPE_CREATE_MSG_FOR_SERVER);
  //Work picks up again in Server's state machine
}

WaitingType OpActivateTypeMeta::handleMessage(bool placeholder, message_t *incoming_msg) {
    OpCore::handleMessage(placeholder, incoming_msg);
}