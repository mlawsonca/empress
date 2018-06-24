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

#include "OpGetAttributeMetaCommon.hh"
#include <server_timing_constants.hh>
#include <sqlite3.h>

extern sqlite3 *db;

using namespace std;

int md_get_attribute_stub (const md_get_attribute_args &args,
                           std::vector<md_attribute_entry> &attribute_list,
                           uint32_t &count);

static int get_matching_attribute_count (const md_get_attribute_args &args, uint32_t &count);


void OpGetAttributeMeta::deArchiveMsgFromClient(const std::string &serial_str, 
                                             md_get_attribute_args &args) {

    //log("the archived message is " + serial_str);

  std::stringstream ss;
  ss << serial_str;
  boost::archive::text_iarchive ia(ss);
  ia >> args;
}

std::string OpGetAttributeMeta::serializeMsgToClient(const std::vector<md_attribute_entry> &attribute_list,
                                                    uint32_t count,
                                                    int return_value) {

  std::stringstream ss;
  boost::archive::text_oarchive oa(ss);
  oa << count;
  if (count > 0) {
   oa << attribute_list;
  }
  oa << return_value;
  //log("the archived message is " + ss.str());


  return ss.str();
}



WaitingType OpGetAttributeMeta::UpdateTarget(OpArgs &args, results_t *results) {
  message_t *incoming_msg = args.data.msg.ptr;

  switch(state){

    case State::start: {
    time_pts.push_back(chrono::high_resolution_clock::now());
    catg_of_time_pts.push_back(OP_GET_ATTR_START);
    md_get_attribute_args var_args;
    std::vector<md_attribute_entry> attribute_list;
    uint32_t count;
    
    peer = args.data.msg.sender;
    

    deArchiveMsgFromClient(incoming_msg->body, var_args);
    time_pts.push_back(chrono::high_resolution_clock::now());
    catg_of_time_pts.push_back(OP_GET_ATTR_DEARCHIVE_MSG_FROM_CLIENT);

    int rc = md_get_attribute_stub(var_args, attribute_list, count);
    time_pts.push_back(chrono::high_resolution_clock::now());
    catg_of_time_pts.push_back(OP_GET_ATTR_MD_GET_ATTR_STUB);


    std::string serial_str = serializeMsgToClient(attribute_list, count, rc);
    time_pts.push_back(chrono::high_resolution_clock::now());
    catg_of_time_pts.push_back(OP_GET_ATTR_SERIALIZE_MSG_FOR_CLIENT);

    createOutgoingMessage(incoming_msg->src, 
                          0, //Not expecting a reply
                          incoming_msg->src_mailbox,
                          serial_str);
    time_pts.push_back(chrono::high_resolution_clock::now());
    catg_of_time_pts.push_back(OP_GET_ATTR_CREATE_MSG_FOR_CLIENT);

    net::SendMsg(peer, ldo_msg);
    state=State::done;
    time_pts.push_back(chrono::high_resolution_clock::now());
    catg_of_time_pts.push_back(OP_GET_ATTR_SEND_MSG_TO_CLIENT_OP_DONE);
    return WaitingType::done_and_destroy;
  }
  case State::done:
    return WaitingType::done_and_destroy;
  }
}

//WaitingType OpGetAttributeMeta::UpdateOrigin(OpArgs &args, results_t *results) {
WaitingType OpGetAttributeMeta::UpdateOrigin(OpArgs &, results_t *) {
  
  return WaitingType::error;

}


int md_get_attribute_stub (const md_get_attribute_args &args,
                           std::vector<md_attribute_entry> &attribute_list,
                           uint32_t &count)
{
    int rc;
    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;
    const char * query = "select ad.attribute_id, ad.chunk_id, ad.type_id, tc.version, ad.data " 
    "from attribute_data ad, type_catalog tc where "
    "(tc.txn_id = ? or tc.active = 1) "
    "and tc.type_id = ad.type_id "
    "and ad.chunk_id = ? ";

    rc = get_matching_attribute_count (args, count); assert (rc == RC_OK);

    if (count > 0) {
        attribute_list.reserve(count);

        rc = sqlite3_prepare_v2 (db, query, -1, &stmt, &tail); 
        if (rc != SQLITE_OK)
        {
            fprintf (stderr, "Line: %d SQL error (%d): %s\n", __LINE__, rc, sqlite3_errmsg (db));
            sqlite3_close (db);
            goto cleanup;
        }
        rc = sqlite3_bind_int (stmt, 1, args.txn_id); assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int (stmt, 2, args.chunk_id); assert (rc == SQLITE_OK);

        rc = sqlite3_step (stmt); assert (rc == SQLITE_OK || rc == SQLITE_ROW);

        while (rc == SQLITE_ROW)
        {
    //printf ("writing item: %d\n", i);

            md_attribute_entry attribute;

            attribute.attribute_id = sqlite3_column_int (stmt, 0);
            attribute.chunk_id = sqlite3_column_int (stmt, 1);
            attribute.type_id = sqlite3_column_int (stmt, 2);
            attribute.type_version = sqlite3_column_int (stmt, 3);
            attribute.data = (char *) sqlite3_column_text (stmt, 4);

    //printf ("GC id: %d name: %s path: %s version: %d ", id, args.name, args.path, args.var_version);
    //printf ("\n");

            rc = sqlite3_step (stmt);
            attribute_list.push_back(attribute);
        }

        rc = sqlite3_finalize (stmt);  
    }
        

cleanup:

    return rc;
}

//note: this appears in get_attribute_count also
static int get_matching_attribute_count (const md_get_attribute_args &args, uint32_t &count)
{
    int rc;
    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;
    const char * query = "select count (*) from attribute_data ad, type_catalog tc where "
"(tc.txn_id = ? or tc.active = 1) "
"and tc.type_id = ad.type_id "
"and ad.chunk_id = ? ";

    rc = sqlite3_prepare_v2 (db, query, -1, &stmt, &tail); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int (stmt, 1, args.txn_id); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int (stmt, 2, args.chunk_id); assert (rc == SQLITE_OK);

//printf ("looking for name: %s path: %s version: %d ", args->name, args->path, args->var_version);
//printf ("\n");
    rc = sqlite3_step (stmt); assert (rc == SQLITE_OK || rc == SQLITE_ROW);
    count = sqlite3_column_int (stmt, 0);
    rc = sqlite3_finalize (stmt); assert (rc == SQLITE_OK);
//printf ("matching attribute count: %d\n", *count);

    return rc;
}