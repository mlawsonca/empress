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


#include <OpCreateRunMetaCommon.hh>

using namespace std;


const unsigned int OpCreateRunMeta::op_id = const_hash("OpCreateRunMeta");
const string OpCreateRunMeta::op_name = "OpCreateRunMeta";   

OpCreateRunMeta::OpCreateRunMeta(op_create_as_target_t t) 
  : state(State::start), ldo_msg(nullptr), Op(t) {
  //No work to do - done in Client's state machine
}

OpCreateRunMeta::~OpCreateRunMeta() {
    if((state == State::start) && (ldo_msg != nullptr)) {
    net::ReleaseMessage(ldo_msg);
  }
}


void OpCreateRunMeta::createOutgoingMessage(gutties::nodeid_t dst , 
                                   const mailbox_t &src_mailbox, 
                                   const mailbox_t &dst_mailbox,
                                   std::string &archived_msg){  

  ldo_msg = net::NewMessage( sizeof(message_t)+archived_msg.size()+1);
  message_t *msg = static_cast<message_t *>(ldo_msg->dataPtr());
  msg->src           = net::GetMyID();
  msg->dst           = dst;
  msg->src_mailbox   = src_mailbox;
  msg->dst_mailbox   = dst_mailbox;
  msg->op_id         = OpCreateRunMeta::op_id;
  msg->body_len      = archived_msg.size()+1;
  // cout << "archived_msg.size(): " << archived_msg.size() << endl;
  // cout << "archived_msg: " << archived_msg << endl;
// std::copy(archived_msg.begin(), archived_msg.end(), &msg->body[0]);
  archived_msg.copy(&msg->body[0], archived_msg.size());
  msg->body[archived_msg.size()] = '\0';
//   cout << "msg->body: " << msg->body << endl;

// cout << "str_body: " << (string) msg->body << endl;
//   // cout << "archived_msg.c_str(): " << archived_msg.c_str() << endl;

// 	strncpy(&msg->body[0], archived_msg.c_str(), archived_msg.size()); 
// 	   cout << "strncpy msg->body: " << msg->body << endl;
// 	cout << "strncpy str_body: " << (string) msg->body << endl;
	// &msg->body[0] = (char*)malloc(archived_msg.size());

	// memcpy(msg->body, archived_msg.data(), archived_msg.size()); 
	//    cout << "memcpy msg->body: " << msg->body << endl;

 //   // compare data after copy
 //   if(memcmp(msg->body, archived_msg.data(), archived_msg.size()) == 0)
 //   {
 //    cout<<"Copy Data 1 Succeed"<<endl;
 //   }

	// memcpy(&msg->body[0], archived_msg.data(), archived_msg.size()); 
	//    cout << "new memcpy msg->body: " << msg->body << endl;

   // compare data after copy
   // if(memcmp(&msg->body[0], archived_msg.data(), archived_msg.size()) == 0)
   // {
   //  cout<<"Copy Data 2 Succeed"<<endl;
   // }

}

 
std::string OpCreateRunMeta::GetStateName() const {
  switch(state){
  case State::start:              return "Start";
  case State::snd_wait_for_reply: return "Sender-WaitForReply";
  case State::done:               return "Done";
  }
  KFAIL();
}

// void OpCreateRunMeta::log(const std::string &s)  {
//   if(debug) cout << "("<< op_name << "): "<< s<<endl;
// }

