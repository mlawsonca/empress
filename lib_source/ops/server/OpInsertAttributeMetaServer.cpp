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

#include "OpInsertAttributeMetaCommon.hh"
#include <server_timing_constants.hh>
#include <sqlite3.h>

extern sqlite3 *db;

using namespace std;

int md_insert_attribute_stub (const md_insert_attribute_args &args, uint64_t &attribute_id);

void OpInsertAttributeMeta::deArchiveMsgFromClient(const std::string &serial_str, md_insert_attribute_args &args) {

    //log("the archived message is " + serial_str);

  std::stringstream ss;
  ss << serial_str;
  boost::archive::text_iarchive ia(ss);
  ia >> args;
}

std::string OpInsertAttributeMeta::serializeMsgToClient(int return_value, uint64_t attribute_id) {

  std::stringstream ss;
  boost::archive::text_oarchive oa(ss);
  oa << return_value;
  //log("the archived message is " + ss.str());

  oa << attribute_id;

  return ss.str();
}



WaitingType OpInsertAttributeMeta::UpdateTarget(OpArgs &args, results_t *results) {
    message_t *incoming_msg = args.data.msg.ptr;
    md_insert_attribute_args var_args;
    uint64_t attribute_id;

  switch(state){

    case State::start: {
    time_pts.push_back(chrono::high_resolution_clock::now());
    catg_of_time_pts.push_back(OP_INSERT_ATTR_START);


    peer = args.data.msg.sender;
    

    deArchiveMsgFromClient(incoming_msg->body, var_args);
    time_pts.push_back(chrono::high_resolution_clock::now());
    catg_of_time_pts.push_back(OP_INSERT_ATTR_DEARCHIVE_MSG_FROM_CLIENT);

    int rc = md_insert_attribute_stub(var_args, attribute_id);
    time_pts.push_back(chrono::high_resolution_clock::now());
    catg_of_time_pts.push_back(OP_INSERT_ATTR_MD_INSERT_ATTR_STUB);

    // cout << "according to the server the attribute id is still " << attribute_id <<endl;
    std::string serial_str = serializeMsgToClient(rc, attribute_id);
    time_pts.push_back(chrono::high_resolution_clock::now());
    catg_of_time_pts.push_back(OP_INSERT_ATTR_SERIALIZE_MSG_FOR_CLIENT);

    createOutgoingMessage(incoming_msg->src, 
                          0, //Not expecting a reply
                          incoming_msg->src_mailbox,
                          serial_str);
    time_pts.push_back(chrono::high_resolution_clock::now());
    catg_of_time_pts.push_back(OP_INSERT_ATTR_CREATE_MSG_FOR_CLIENT);

    net::SendMsg(peer, ldo_msg);
    state=State::done;
    time_pts.push_back(chrono::high_resolution_clock::now());
    catg_of_time_pts.push_back(OP_INSERT_ATTR_SEND_MSG_TO_CLIENT_OP_DONE);
    return WaitingType::done_and_destroy;
  }
  case State::done:
    return WaitingType::done_and_destroy;
  }
}

//WaitingType OpInsertAttributeMeta::UpdateOrigin(OpArgs &args, results_t *results) {
WaitingType OpInsertAttributeMeta::UpdateOrigin(OpArgs &, results_t *) {
  
  return WaitingType::error;

}

int md_insert_attribute_stub (const md_insert_attribute_args &args,
                          uint64_t &attribute_id)
{
    int rc;
    // int i;
    // char * ErrMsg = NULL;
    sqlite3_stmt * stmt_index = NULL;
    const char * tail_index = NULL;

    // int rowid;

    rc = sqlite3_prepare_v2 (db, "insert into attribute_data (attribute_id, chunk_id, type_id, data) values (?, ?, ?, ?)", -1, &stmt_index, &tail_index);
    if (rc != SQLITE_OK)
    {
        fprintf (stderr, "Line: %d SQL error: %s\n", __LINE__, sqlite3_errmsg (db));
        sqlite3_close (db);
        goto cleanup;
    }

    rc = sqlite3_bind_null (stmt_index, 1); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int (stmt_index, 2, args.chunk_id); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int (stmt_index, 3, args.type_id); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_text (stmt_index, 4, strdup(args.data.c_str()), -1, free); assert (rc == SQLITE_OK);

    rc = sqlite3_step (stmt_index); assert (rc == SQLITE_OK || rc == SQLITE_DONE);


    attribute_id = (int) sqlite3_last_insert_rowid (db);
    // std::cout << " According to the server, the chunk id is " << attribute_id << std::endl;

    rc = sqlite3_finalize (stmt_index);
//    rowid = sqlite3_last_insert_rowid (db);
//printf ("rowid for chunk: %d\n", rowid);

//printf ("insert chunk completed\n");

//    rc = nssi_put_data (caller, &count, sizeof (uint32_t), data_addr, -1);

cleanup:

    return rc;
}