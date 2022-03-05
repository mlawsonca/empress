

#include <OpDeleteVarByNamePathVerMetaCommon.hh>

#include <OpCoreClient.hh>
using namespace std;



OpDeleteVarByNamePathVerMeta::OpDeleteVarByNamePathVerMeta(opbox::net::peer_ptr_t dst, const md_delete_var_by_name_path_ver_args &args) 
    : OpCore(true) {
    peer = dst;
    // add_timing_point(OP_DELETE_VAR_BY_NAME_PATH_VER_START);

    // cout << "Client about to serialize a message to server \n";

    std::string serial_str = serializeMsgToServer(args);    
    add_timing_point(OP_DELETE_VAR_BY_NAME_PATH_VER_SERIALIZE_MSG_FOR_SERVER);
    // cout << "Client about to create outgoing message to server \n";

    createOutgoingMessage(opbox::net::ConvertPeerToNodeID(dst),
                      GetAssignedMailbox(), 
                      0, 
                      serial_str);

    // add_timing_point(OP_DELETE_VAR_BY_NAME_PATH_VER_CREATE_MSG_FOR_SERVER);
  //Work picks up again in Server's state machine
  // cout << "Client just dispatched message to server \n";

}

WaitingType OpDeleteVarByNamePathVerMeta::handleMessage(bool placeholder, message_t *incoming_msg) {
    OpCore::handleMessage(placeholder, incoming_msg);
}
