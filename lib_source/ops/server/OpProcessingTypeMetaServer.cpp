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

#include "OpProcessingTypeMetaCommon.hh"
#include <server_timing_constants.hh>
#include <sqlite3.h>

extern sqlite3 *db;

using namespace std;

int md_processing_type_stub (const md_processing_type_args &args);

void OpProcessingTypeMeta::deArchiveMsgFromClient(const std::string &serial_str, md_processing_type_args &args) {

    //log("the archived message is " + serial_str);

  std::stringstream ss;
  ss << serial_str;
  boost::archive::text_iarchive ia(ss);
  ia >> args;
}

std::string OpProcessingTypeMeta::serializeMsgToClient(int return_value) {

  std::stringstream ss;
  boost::archive::text_oarchive oa(ss);
  oa << return_value;
  //log("the archived message is " + ss.str());


  return ss.str();
}


WaitingType OpProcessingTypeMeta::UpdateTarget(OpArgs &args, results_t *results) {
  message_t *incoming_msg = args.data.msg.ptr;

  switch(state){

    case State::start: {
    time_pts.push_back(chrono::high_resolution_clock::now());
    catg_of_time_pts.push_back(OP_PROCESSING_TYPE_START);
    md_processing_type_args type_args;

    peer = args.data.msg.sender;
    

    deArchiveMsgFromClient(incoming_msg->body, type_args);
    time_pts.push_back(chrono::high_resolution_clock::now());
    catg_of_time_pts.push_back(OP_PROCESSING_TYPE_DEARCHIVE_MSG_FROM_CLIENT);

    int rc = md_processing_type_stub(type_args);
    time_pts.push_back(chrono::high_resolution_clock::now());
    catg_of_time_pts.push_back(OP_PROCESSING_TYPE_MD_PROCESSING_TYPE_STUB);


    std::string serial_str = serializeMsgToClient(rc);
    time_pts.push_back(chrono::high_resolution_clock::now());
    catg_of_time_pts.push_back(OP_PROCESSING_TYPE_SERIALIZE_MSG_FOR_CLIENT);

    createOutgoingMessage(incoming_msg->src, 
                          0, //Not expecting a reply
                          incoming_msg->src_mailbox,
                          serial_str);
    time_pts.push_back(chrono::high_resolution_clock::now());
    catg_of_time_pts.push_back(OP_PROCESSING_TYPE_CREATE_MSG_FOR_CLIENT);

    net::SendMsg(peer, ldo_msg);
    state=State::done;
    time_pts.push_back(chrono::high_resolution_clock::now());
    catg_of_time_pts.push_back(OP_PROCESSING_TYPE_SEND_MSG_TO_CLIENT_OP_DONE);
    return WaitingType::done_and_destroy;
  }
  case State::done:
    return WaitingType::done_and_destroy;
  }
}

//WaitingType OpProcessingTypeMeta::UpdateOrigin(OpArgs &args, results_t *results) {
WaitingType OpProcessingTypeMeta::UpdateOrigin(OpArgs &, results_t *) {
  
  return WaitingType::error;

}

int md_processing_type_stub (const md_processing_type_args &args)
{
    int rc;
    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;
    const char * query = "update type_catalog set active = 2 where txn_id = ?";

    rc = sqlite3_prepare_v2 (db, query, -1, &stmt, &tail); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int (stmt, 1, args.txn_id); assert (rc == SQLITE_OK);
    rc = sqlite3_step (stmt); assert (rc == SQLITE_OK || rc == SQLITE_ROW || rc == SQLITE_DONE);
    rc = sqlite3_finalize (stmt); assert (rc == SQLITE_OK);

    return rc;
}