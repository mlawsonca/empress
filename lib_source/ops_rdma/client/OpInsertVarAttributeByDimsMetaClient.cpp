

#include <OpInsertVarAttributeByDimsMetaCommon.hh>

#include <OpCoreClient.hh>
using namespace std;


OpInsertVarAttributeByDimsMeta::OpInsertVarAttributeByDimsMeta(opbox::net::peer_ptr_t dst , const md_insert_var_attribute_by_dims_args &args) 
    : OpCore(true) {
    peer = dst;
    // 
    // add_timing_point(OP_INSERT_VAR_ATTRIBUTE_BY_DIMS_START);

    std::string serial_str = serializeMsgToServer(args);
    add_timing_point(OP_INSERT_VAR_ATTRIBUTE_BY_DIMS_SERIALIZE_MSG_FOR_SERVER);


    createOutgoingMessage(opbox::net::ConvertPeerToNodeID(dst),
                      GetAssignedMailbox(), 
                      0, 
                      serial_str);
    // 
    // add_timing_point(OP_INSERT_VAR_ATTRIBUTE_BY_DIMS_CREATE_MSG_FOR_SERVER); 
  //Work picks up again in Server's state machine

}

WaitingType OpInsertVarAttributeByDimsMeta::handleMessage(bool placeholder, message_t *incoming_msg) {
    OpCore::handleMessage(placeholder, incoming_msg);
}
