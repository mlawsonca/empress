/* 
 * Copyright 2018 National Technology & Engineering Solutions of
 * Sandia, LLC (NTESS). Under the terms of Contract DE-NA0003525 with NTESS,
 * the U.S. Government retains certain rights in this software.
 *
 * The MIT License (MIT)
 * 
 * Copyright (c) 2018 Sandia Corporation
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */


#include <md_client_timing_constants.hh>
#include <OpCreateVarMetaCommon.hh>


using namespace std;

WaitingType OpCreateVarMeta::UpdateOrigin(OpArgs &args, results_t *results) {
  

  message_t *incoming_msg;
  string user_data;

  switch(state){
  case State::start:

    net::SendMsg(peer, ldo_msg);
    state=State::snd_wait_for_reply;    
    add_timing_point(OP_CREATE_VAR_SEND_MSG_TO_SERVER);
    return WaitingType::waiting_on_cq;

  case State::snd_wait_for_reply:
  
    assert(args.type == UpdateType::incoming_message &&
           "Sender in snd_wait_for_reply, but event not an incoming message");    
    add_timing_point(OP_CREATE_VAR_RETURN_MSG_RECEIVED_FROM_SERVER);
    
    incoming_msg = args.data.msg.ptr;

    user_data = incoming_msg->body;
    return_msg_promise.set_value(user_data);
    state=State::done;
    // add_timing_point(OP_CREATE_VAR_PROMISE_VAL_SET_OP_DONE);

	case State::done:
    	return WaitingType::done_and_destroy;
  	}

  KFAIL();
	return WaitingType::error;

}


WaitingType OpCreateVarMeta::UpdateTarget(OpArgs &, results_t *) {
      //log("error. its asking me to update target \n");

    return WaitingType::done_and_destroy;
}

OpCreateVarMeta::OpCreateVarMeta(net::peer_ptr_t dst, md_create_var_args args) 
  : state(State::start), ldo_msg(nullptr), Op(true) {
    peer = dst;
    // add_timing_point(OP_CREATE_VAR_START);

    // cout << "Client about to serialize a message to server \n";

    std::string serial_str = serializeMsgToServer(args);    
    add_timing_point(OP_CREATE_VAR_SERIALIZE_MSG_FOR_SERVER);
    // cout << "Client about to create outgoing message to server \n";

    createOutgoingMessage(net::ConvertPeerToNodeID(dst), 
                      GetAssignedMailbox(), 
                      0, 
                      serial_str);

    // add_timing_point(OP_CREATE_VAR_CREATE_MSG_FOR_SERVER);
  //Work picks up again in Server's state machine
  // cout << "Client just dispatched message to server \n";

  }


//Means for passing back the result
std::future<std::string> OpCreateVarMeta::GetFuture() {
  return return_msg_promise.get_future();
}


std::string OpCreateVarMeta::serializeMsgToServer(const md_create_var_args &args) {

  std::stringstream ss;
  boost::archive::text_oarchive oa(ss);
  oa << args;
  //log("the archived message is " + ss.str());

  // cout << "Client just serialized message to server \n";

  return ss.str();
}


void OpCreateVarMeta::deArchiveMsgFromServer(const std::string &serial_str,
                                             uint64_t &var_id,
                                             int &return_value) {

  std::stringstream ss;
  ss << serial_str;
  boost::archive::text_iarchive ia(ss);
  ia >> var_id;
  ia >> return_value;
  //log("the archived message is " + serial_str);



  // cout << "Client just dearchived message from server \n";

}