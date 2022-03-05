#include <OpCoreCommon.hh>

using namespace std;


// OpCore::OpCore(opbox::net::peer_ptr_t dst , const op_args &args) 
//     : OpCore(true) {

//     peer = dst;
//     // 
//     // add_timing_point(getOpTimingConstant() + START);

//     //serialize the message
//     std::stringstream ss;
//     boost::archive::text_oarchive oa(ss);
//     oa << args;
//     std::string serial_str = ss.str();

//     add_timing_point(getOpTimingConstant() + SERIALIZE_MSG_FOR_SERVER);

//     createOutgoingMessage(opbox::net::ConvertPeerToNodeID(dst),
//                       GetAssignedMailbox(), 
//                       0, 
//                       serial_str);


//     // add_timing_point(getOpTimingConstant() + CREATE_MSG_FOR_SERVER); 
//   //Work picks up again in Server's state machine

// }

WaitingType OpCore::UpdateOrigin(OpArgs *args) {

    message_t *incoming_msg;
    string user_data;

    switch(state){
        case State::start:
            //cout << "about to State::start" << endl;
            net::SendMsg(peer, ldo_msg);
            //cout << "sent message" << endl;

            state=State::snd_wait_for_reply;    
            add_timing_point(getOpTimingConstant() + SEND_MSG_TO_SERVER);
            //cout << "added timing point " << (getOpTimingConstant() + SEND_MSG_TO_SERVER) << endl;
            return WaitingType::waiting_on_cq;

            // return handleMessageFromServer(incoming_msg);

            // user_data = "dummy";
            // return_msg_promise.set_value(user_data);
            // state=State::done;
            // return WaitingType::done_and_destroy;
            // return handleMessage(false, incoming_msg);

        case State::snd_wait_for_reply:
            add_timing_point(getOpTimingConstant() + RETURN_MSG_RECEIVED_FROM_SERVER);
            //cout << "about to State::snd_wait_for_reply" << endl;

            //assert(args->type == UpdateType::incoming_message && "Sender in snd_wait_for_reply, but event not an incoming message");    

            incoming_msg = args->ExpectMessageOrDie<message_t *>();

            //cout << "about to handleMessageFromServer" << endl;
            return handleMessageFromServer(incoming_msg);
            // add_timing_point(getOpTimingConstant() + PROMISE_VAL_SET_OP_DONE); 

        case State::get_wait_complete:
            add_timing_point(getOpTimingConstant() + RDMA_GET_FINISHED);

            //cout << "about to State::get_wait_complete" << endl;
            user_data.assign(shout_ldo.GetDataPtr<char *>(), shout_ldo.GetDataSize());
            return_msg_promise.set_value(user_data);

            state=State::done;
            return WaitingType::done_and_destroy;

        case State::done:
            //cout << "about to State::done" << endl;
            return WaitingType::done_and_destroy;
    }

    KFAIL();
    return WaitingType::error;

}

//note - dummy function, should never get called
WaitingType OpCore::handleMessage(bool placeholder, message_t *incoming_msg)
{
    return WaitingType::error;
}

//note - we use this instead of handleMessage since it causes errors with compiler -O3, -O2, -O1
// WaitingType OpCore::handleMessageFromServer(message_t *incoming_msg)
// {
//     string user_data;
//     // user_data = "dummy";
//     user_data.assign(incoming_msg->body, incoming_msg->body + incoming_msg->body_len);
//     //cout << "user_data: " << user_data << endl;
//     return_msg_promise.set_value(user_data);
//     state=State::done;
//     return WaitingType::done_and_destroy;


// }

//note - we keep the placeholder there because the server side of the function uses the bool
WaitingType OpCore::handleMessageFromServer(message_t *incoming_msg)
{
    string user_data;
    //cout << "msg: " << incoming_msg->body << endl;
    //cout << "incoming_msg->body[1]: " << incoming_msg->body[1] << endl;
    //cout << "msg length: " << incoming_msg->body_len << endl;

    if(incoming_msg->body[0] == '0') { //not rdma
        //assign everythign but the rdma flag big
        //this produces an incorrect result
        // user_data.assign(incoming_msg->body[1], incoming_msg->body[1] + incoming_msg->body_len-1);
        //looks like this doens't properly handle embedded nulls
        // user_data.assign(incoming_msg->body, 1, incoming_msg->body_len-1);
        user_data.assign(&incoming_msg->body[1], &incoming_msg->body[1] + incoming_msg->body_len-1);
        //cout << "user_data: " << user_data << endl;
        return_msg_promise.set_value(user_data);
        state=State::done;
        return WaitingType::done_and_destroy;
    }
    else {
        // save a copy of the NBR for later use, don't grab the rdma flag bit
        memcpy(&nbr, &incoming_msg->body[1], sizeof(opbox::net::NetBufferRemote));
        //cout << "nbr: " << nbr.str() << endl;

        // this is the initiator buffer for the get and the put
        shout_ldo = lunasa::DataObject(0, nbr.GetLength(), lunasa::DataObject::AllocatorType::eager);
        //cout << "done assigning shout_ldo" << endl;

        // get the ping message from the origin process.
        // AllEventsCallback() is a convenience class that will redirect
        // all events generated by the get to this operation's Update()
        // method.
        opbox::net::Get(peer, &nbr, shout_ldo, AllEventsCallback(this));
        //cout << "done with opbox::net::Get" << endl;

        add_timing_point(getOpTimingConstant() + RDMA_GET_START);
        state=State::get_wait_complete;
        return WaitingType::waiting_on_cq;
    }

}



WaitingType OpCore::UpdateTarget(OpArgs *args) {
    return WaitingType::done_and_destroy;
}



//Means for passing back the result
std::future<string>  OpCore::GetFuture() {
  return return_msg_promise.get_future();
}

// WaitingType OpCore::handleMessage(bool rdma, message_t *incoming_msg) {
// }


