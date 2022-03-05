#include <OpActivateVarAttributeMetaCommon.hh>#include <OpCoreClient.hh>
using namespace std;



OpActivateVarAttributeMeta::OpActivateVarAttributeMeta(opbox::net::peer_ptr_t dst, const md_activate_args &args) 
    : OpCore(true) {
    peer = dst;
    // add_timing_point(OP_ACTIVATE_VAR_ATTRIBUTE_START);

    std::string serial_str = serializeMsgToServer(args);    
    add_timing_point(OP_ACTIVATE_VAR_ATTRIBUTE_SERIALIZE_MSG_FOR_SERVER);

    createOutgoingMessage(opbox::net::ConvertPeerToNodeID(dst),
                      GetAssignedMailbox(), 
                      0, 
                      serial_str);
    // add_timing_point(OP_ACTIVATE_VAR_ATTRIBUTE_CREATE_MSG_FOR_SERVER);
  //Work picks up again in Server's state machine
}

WaitingType OpActivateVarAttributeMeta::handleMessage(bool placeholder, message_t *incoming_msg) {
    OpCore::handleMessage(placeholder, incoming_msg);
}
