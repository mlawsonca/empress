#include <OpFullShutdownMetaCommon.hh>

// extern bool md_shutdown;
extern pthread_cond_t cond1;

using namespace std;

WaitingType OpFullShutdownMeta::UpdateTarget(OpArgs *args) {
    switch(state){
        case State::start: {
            // md_shutdown = true;
            //debug_log << "about to signal shutdown" << endl;
            pthread_cond_signal(&cond1);
            state=State::done;
            return WaitingType::done_and_destroy;
        }
        case State::done:
            return WaitingType::done_and_destroy;
    }
}

//note: is a dummy that should never be called
WaitingType OpFullShutdownMeta::handleMessage(bool rdma, message_t *incoming_msg) {
    // cout << "error am using OpFullShutdownMeta::handleMessage" << endl;

    // md_shutdown = true;
    //debug_log << "about to signal shutdown" << endl;
    pthread_cond_signal(&cond1);

    // add_timing_point(OP_FULL_SHUTDOWN_DONE);
    state=State::done;
    return WaitingType::done_and_destroy;

}

WaitingType OpFullShutdownMeta::UpdateOrigin(OpArgs *) {
  
    return WaitingType::error;

}


