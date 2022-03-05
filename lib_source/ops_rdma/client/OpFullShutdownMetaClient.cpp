

#include <OpFullShutdownMetaCommon.hh>

#include <chrono>
#include <OpCoreClient.hh>
using namespace std;

extern std::vector<std::chrono::high_resolution_clock::time_point> time_pts;
extern std::vector<int> catg_of_time_pts;





OpFullShutdownMeta::OpFullShutdownMeta(opbox::net::peer_ptr_t dst) 
    : OpCore(true) {
    peer = dst;
    // add_timing_point(OP_FULL_SHUTDOWN_START);

    createEmptyOutgoingMessage(opbox::net::ConvertPeerToNodeID(dst),
                          GetAssignedMailbox(), 
                          0);
    // add_timing_point(OP_FULL_SHUTDOWN_CREATE_MSG_FOR_SERVER);
  //Work picks up again in Server's state machine

}

WaitingType OpFullShutdownMeta::UpdateOrigin(OpArgs *args) {
  // 
    // cout << "am using my custom UpdateOrigin" << endl;
  switch(state){
  case State::start:
    net::SendMsg(peer, ldo_msg);
    state=State::done;
    // add_timing_point(OP_FULL_SHUTDOWN_SEND_MSG_TO_SERVER_OP_DONE); 

    case State::done:
        return WaitingType::done_and_destroy;
    }

  KFAIL();
    return WaitingType::error;

}

WaitingType OpFullShutdownMeta::handleMessage(bool placeholder, message_t *incoming_msg) {
    OpCore::handleMessage(placeholder, incoming_msg);
}


WaitingType OpFullShutdownMeta::UpdateTarget(OpArgs *args) {
    return WaitingType::done_and_destroy;
}

