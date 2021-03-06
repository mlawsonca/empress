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
#include <OpCatalogRunMetaCommon.hh>


using namespace std;

WaitingType OpCatalogRunMeta::UpdateOrigin(OpArgs &args, results_t *results) {
  message_t *incoming_msg;
  //string user_data;

  switch(state){
  case State::start:
    net::SendMsg(peer, ldo_msg);
    state=State::snd_wait_for_reply;    
    add_timing_point(OP_CATALOG_RUN_SEND_MSG_TO_SERVER);
    
    return WaitingType::waiting_on_cq;

  case State::snd_wait_for_reply:
    assert(args.type == UpdateType::incoming_message &&
           "Sender in snd_wait_for_reply, but event not an incoming message");    
    add_timing_point(OP_CATALOG_RUN_RETURN_MSG_RECEIVED_FROM_SERVER);

    incoming_msg = args.data.msg.ptr;

    //user_data = incoming_msg->body;
    // return_msg_promise.set_value(user_data);
    return_msg_promise.set_value(incoming_msg);

    state=State::done;
    // add_timing_point(OP_CATALOG_RUN_PROMISE_VAL_SET_OP_DONE); 

	case State::done:
    	return WaitingType::done_and_destroy;
  	}

  KFAIL();
	return WaitingType::error;

}


WaitingType OpCatalogRunMeta::UpdateTarget(OpArgs &, results_t *) {
    return WaitingType::done_and_destroy;
}

OpCatalogRunMeta::OpCatalogRunMeta(net::peer_ptr_t dst , md_catalog_run_args args) 
  : state(State::start), ldo_msg(nullptr), Op(true) {
    peer = dst;
    // add_timing_point(OP_CATALOG_RUN_START);

    std::string serial_str = serializeMsgToServer(args);    
    add_timing_point(OP_CATALOG_RUN_SERIALIZE_MSG_FOR_SERVER);

    createOutgoingMessage(net::ConvertPeerToNodeID(dst), 
                      GetAssignedMailbox(), 
                      0, 
                      serial_str);
    // add_timing_point(OP_CATALOG_RUN_CREATE_MSG_FOR_SERVER);
  }


std::future<message_t *> OpCatalogRunMeta::GetFuture() {
  return return_msg_promise.get_future();
}


std::string OpCatalogRunMeta::serializeMsgToServer(const md_catalog_run_args &args) {

  std::stringstream ss;
  boost::archive::text_oarchive oa(ss);
  oa << args;
  //log("the archived message is " + ss.str());

  return ss.str();
}

void OpCatalogRunMeta::deArchiveMsgFromServer(message_t *incoming_msg, 
                                           std::vector<md_catalog_run_entry> &entries,
                                           uint32_t &count,
                                           int &return_value) {

  std::stringstream ss;
  ss.write(incoming_msg->body, incoming_msg->body_len);
  boost::archive::text_iarchive ia(ss);
  // cout << "incoming_msg->body_len: " << incoming_msg->body_len << endl;
  // ia >> new_args4;
  // cout << "deserialized text string: " << ss.str() << endl;
  // cout << "deserialized text string length: " << ss.str().size() << endl;

  // std::stringstream ss;
  // ss << serial_str;
  // boost::archive::text_iarchive ia(ss);
  ia >> count;
  if( count > 0) {
    ia >> entries;
  }
  else {
    entries.clear();
  }
  // cout << "in client, the dearchived message is " << ss.str() << endl;
  // cout << "in client, the dearchived message.size() is: "  << ss.str().size() << endl;
  // std::cout << "entries size is " << entries.size() << endl;
  ia >> return_value;
 

  //log("the archived message is " + serial_str);
}


// void OpCatalogRunMeta::deArchiveMsgFromServer(const std::string &serial_str, 
//                                            std::vector<md_catalog_run_entry> &entries,
//                                            uint32_t &count,
//                                            int &return_value) {

//   // std::stringstream ss;
//   // ss << serial_str;
//   // boost::archive::text_iarchive ia(ss);
//   ia >> count;
//   if( count > 0) {
//     ia >> entries;
//   }
//   else {
//     entries.clear();
//   }
//   cout << "in client, the dearchived message is " << ss.str() << endl;
//   cout << "in client, the dearchived message.size() is: "  << ss.str().size() << endl;
//   // std::cout << "entries size is " << entries.size() << endl;
//   ia >> return_value;
 

//   //log("the archived message is " + serial_str);
// }