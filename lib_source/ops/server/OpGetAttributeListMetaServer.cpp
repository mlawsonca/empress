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

#include "OpGetAttributeListMetaCommon.hh"
#include <server_timing_constants.hh>
#include <sqlite3.h>

extern sqlite3 *db;

using namespace std;

int md_get_attribute_list_stub (const md_get_attribute_list_args &args,
                                std::vector<md_attribute_entry> &entries,
                                uint32_t &count);

static int get_attribute_list_count (const md_get_attribute_list_args &args, uint32_t &count);


void OpGetAttributeListMeta::deArchiveMsgFromClient(const std::string &serial_str, 
                                                 md_get_attribute_list_args &args) {

  //log("the archived message is " + serial_str);

  std::stringstream ss;
  ss << serial_str;
  boost::archive::text_iarchive ia(ss);
  ia >> args;
  // //log("server thinks type version is " + std::to_string((int) args.type_version));

}

std::string OpGetAttributeListMeta::serializeMsgToClient(const std::vector<md_attribute_entry> &entries,
                                                         uint32_t count,
                                                         int return_value) {

  std::stringstream ss;
  boost::archive::text_oarchive oa(ss);
  oa << count;
  if (count > 0) {
    oa << entries;  
  }
  oa << return_value;
  //log("the archived message is " + ss.str());


  return ss.str();
}



WaitingType OpGetAttributeListMeta::UpdateTarget(OpArgs &args, results_t *results) {
  message_t *incoming_msg = args.data.msg.ptr;

  switch(state){

    case State::start: {
    time_pts.push_back(chrono::high_resolution_clock::now());
    catg_of_time_pts.push_back(OP_GET_ATTR_LIST_START);
    md_get_attribute_list_args var_args;
    std::vector<md_attribute_entry> entries;
    uint32_t count;

    peer = args.data.msg.sender;
    

    deArchiveMsgFromClient(incoming_msg->body, var_args);
    time_pts.push_back(chrono::high_resolution_clock::now());
    catg_of_time_pts.push_back(OP_GET_ATTR_LIST_DEARCHIVE_MSG_FROM_CLIENT);

        //log("Type name is " + var_args.type_name);
        //log("Type version is " + std::to_string((int) var_args.type_version));
        //log("txn_id is " + std::to_string((int) var_args.txn_id));


    int rc = md_get_attribute_list_stub(var_args, entries, count);
    time_pts.push_back(chrono::high_resolution_clock::now());
    catg_of_time_pts.push_back(OP_GET_ATTR_LIST_MD_GET_ATTR_LIST_STUB);

    std::string serial_str = serializeMsgToClient(entries, count, rc);
    time_pts.push_back(chrono::high_resolution_clock::now());
    catg_of_time_pts.push_back(OP_GET_ATTR_LIST_SERIALIZE_MSG_FOR_CLIENT);

    createOutgoingMessage(incoming_msg->src, 
                          0, //Not expecting a reply
                          incoming_msg->src_mailbox,
                          serial_str);
    time_pts.push_back(chrono::high_resolution_clock::now());
    catg_of_time_pts.push_back(OP_GET_ATTR_LIST_CREATE_MSG_FOR_CLIENT);

    net::SendMsg(peer, ldo_msg);
    state=State::done;
    time_pts.push_back(chrono::high_resolution_clock::now());
    catg_of_time_pts.push_back(OP_GET_ATTR_LIST_SEND_MSG_TO_CLIENT_OP_DONE);
    return WaitingType::done_and_destroy;
  }
  case State::done:
    return WaitingType::done_and_destroy;
  }
}

//WaitingType OpGetAttributeListMeta::UpdateOrigin(OpArgs &args, results_t *results) {
WaitingType OpGetAttributeListMeta::UpdateOrigin(OpArgs &, results_t *) {
  
  return WaitingType::error;

}



int md_get_attribute_list_stub (const md_get_attribute_list_args &args,
                                std::vector<md_attribute_entry> &entries,
                                uint32_t &count)
{
    int rc;
    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;
    const char * query = "select ad.attribute_id, ad.chunk_id, ad.type_id, ad.data from attribute_data ad, type_catalog tc where tc.name = ? and tc.version = ? and tc.type_id = ad.type_id and (tc.txn_id = ? or tc.active = 1)";

    rc = get_attribute_list_count (args, count);
//printf ("CL attributes: %d\n", count);

    if (count > 0)
    {
        entries.reserve(count);

        rc = sqlite3_prepare_v2 (db, query, -1, &stmt, &tail);
        if (rc != SQLITE_OK)
        {
            fprintf (stderr, "Line: %d SQL error: %s\n", __LINE__, sqlite3_errmsg (db));
            sqlite3_close (db);
            goto cleanup;
        }
        rc = sqlite3_bind_text (stmt, 1, strdup(args.type_name.c_str()), -1, free); assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int (stmt, 2, args.type_version); assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int (stmt, 3, args.txn_id); assert (rc == SQLITE_OK);
        rc = sqlite3_step (stmt);

//printf ("CL rc: %d\n", rc);

        int j = 0;
        while (rc == SQLITE_ROW)
        {
            // int id;
            md_attribute_entry entry;

//printf ("CL: entry: %d\n", j);

            entry.attribute_id = sqlite3_column_int (stmt, 0);
            entry.chunk_id = sqlite3_column_int (stmt, 1);
            entry.type_id = sqlite3_column_int (stmt, 2);
            entry.data = (char *) sqlite3_column_text (stmt, 3);

//printf ("CL num_dims: %d\n", entries [j].num_dims);

//printf ("CL id: %d length: %ld connection: %s num_dims: %d ", id, entries [j].length_of_attribute, entries [j].connection, entries [j].num_dims);
           
            rc = sqlite3_step (stmt);
            entries.push_back(entry);

            j++;
        }
        rc = sqlite3_finalize (stmt); assert (rc == SQLITE_OK);
    }

cleanup:

    return rc;
}

static int get_attribute_list_count (const md_get_attribute_list_args &args, uint32_t &count)
{
    int rc;
    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;
    const char * query = "select count (*) from attribute_data ad, type_catalog tc where tc.name = ? and tc.version = ? and tc.type_id = ad.type_id and (tc.txn_id = ? or tc.active = 1)";

    rc = sqlite3_prepare_v2 (db, query, -1, &stmt, &tail);
    if (rc != SQLITE_OK)
    {
        fprintf (stderr, "Line: %d SQL error: %s\n", __LINE__, sqlite3_errmsg (db));
        sqlite3_close (db);
        goto cleanup;
    }
    rc = sqlite3_bind_text (stmt, 1, strdup(args.type_name.c_str()), -1, free); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int (stmt, 2, args.type_version); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int (stmt, 3, args.txn_id); assert (rc == SQLITE_OK);

    rc = sqlite3_step (stmt); assert (rc == SQLITE_OK || rc == SQLITE_ROW);
    count = sqlite3_column_int (stmt, 0);
//printf ("database says %u attributes\n", *count);
    rc = sqlite3_finalize (stmt); assert (rc == SQLITE_OK);

cleanup:
    return rc;
}