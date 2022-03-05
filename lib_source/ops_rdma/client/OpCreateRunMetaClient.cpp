

#include <OpCreateRunMetaCommon.hh>

#include <OpCoreClient.hh>
using namespace std;


OpCreateRunMeta::OpCreateRunMeta(opbox::net::peer_ptr_t dst, const md_create_run_args &args) 
    : OpCore(true) {
    peer = dst;
    // add_timing_point(OP_CREATE_RUN_START);

    //cout << "OpCreateRunMeta: about to serializeMsgToServer \n";

    std::string serial_str = serializeMsgToServer(args);    
    add_timing_point(OP_CREATE_RUN_SERIALIZE_MSG_FOR_SERVER);
    //cout << "OpCreateRunMeta: about to createOutgoingMessage \n";
    // //cout << "serial_str in client: " << serial_str << endl;

    createOutgoingMessage(opbox::net::ConvertPeerToNodeID(dst),
                      GetAssignedMailbox(), 
                      0, 
                      serial_str);

    // add_timing_point(OP_CREATE_RUN_CREATE_MSG_FOR_SERVER);
  //Work picks up again in Server's state machine
    //cout << "OpCreateRunMeta: done with createOutgoingMessage \n";

}

WaitingType OpCreateRunMeta::handleMessage(bool placeholder, message_t *incoming_msg) {
    OpCore::handleMessage(placeholder, incoming_msg);
}

