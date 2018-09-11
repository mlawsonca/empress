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
#include <OpFullShutdownMetaCommon.hh>

#include <chrono>

using namespace std;

extern std::vector<std::chrono::high_resolution_clock::time_point> time_pts;
extern std::vector<int> catg_of_time_pts;


WaitingType OpFullShutdownMeta::UpdateOrigin(OpArgs &args, results_t *results) {
  // 

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


WaitingType OpFullShutdownMeta::UpdateTarget(OpArgs &, results_t *) {
    return WaitingType::done_and_destroy;
}

OpFullShutdownMeta::OpFullShutdownMeta(net::peer_ptr_t dst) 
  : state(State::start), ldo_msg(nullptr), Op(true) {
    peer = dst;
    // add_timing_point(OP_FULL_SHUTDOWN_START);

    createOutgoingMessage(net::ConvertPeerToNodeID(dst), 
                          GetAssignedMailbox(), 
                          0);
    // add_timing_point(OP_FULL_SHUTDOWN_CREATE_MSG_FOR_SERVER);
  //Work picks up again in Server's state machine

  }



