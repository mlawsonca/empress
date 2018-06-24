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

#include "OpCatalogVarMetaCommon.hh"
#include <server_timing_constants.hh>
#include <sqlite3.h>

extern sqlite3 *db;

using namespace std;

int md_catalog_var_stub (md_catalog_args &args, uint32_t &count, std::vector<md_catalog_var_entry> &entries);


void OpCatalogVarMeta::deArchiveMsgFromClient(const std::string &serial_str, md_catalog_args &args) {

    //log("the archived message is " + serial_str);

  std::stringstream ss;
  ss << serial_str;
  boost::archive::text_iarchive ia(ss);
  ia >> args;

}

std::string OpCatalogVarMeta::serializeMsgToClient(const std::vector<md_catalog_var_entry> &entries,
                                                uint32_t count,
                                                int return_value) {
  std::stringstream ss;
  boost::archive::text_oarchive oa(ss);
  oa << count;
  if(count > 0) {
    oa << entries;
  }
  oa << return_value;
  //log("the archived message is " + ss.str());


  return ss.str();
}


WaitingType OpCatalogVarMeta::UpdateTarget(OpArgs &args, results_t *results) {
  message_t *incoming_msg = args.data.msg.ptr;

  switch(state){

    case State::start: {
    time_pts.push_back(chrono::high_resolution_clock::now());
    catg_of_time_pts.push_back(OP_CATALOG_VAR_START);
    md_catalog_args var_args;
    uint32_t count;    
    std::vector<md_catalog_var_entry> entries;

    peer = args.data.msg.sender;


    //convert the serialized string back into the args and count pointer
    deArchiveMsgFromClient(incoming_msg->body, var_args);
    time_pts.push_back(chrono::high_resolution_clock::now());
    catg_of_time_pts.push_back(OP_CATALOG_VAR_DEARCHIVE_MSG_FROM_CLIENT);

    int rc = md_catalog_var_stub(var_args, count, entries);
    time_pts.push_back(chrono::high_resolution_clock::now());
    catg_of_time_pts.push_back(OP_CATALOG_VAR_MD_CATALOG_VAR_STUB);

    //log("num of entries is " + std::to_string(entries.size()));
    if(entries.size()>0) {
        //log("in md_catalog_var_stub the first entry has name " + entries.at(0).name);

    }

    std::string serial_str = serializeMsgToClient(entries, count, rc);
    time_pts.push_back(chrono::high_resolution_clock::now());
    catg_of_time_pts.push_back(OP_CATALOG_VAR_SERIALIZE_MSG_FOR_CLIENT);

    createOutgoingMessage(incoming_msg->src, 
                          0, //Not expecting a reply
                          incoming_msg->src_mailbox,
                          serial_str);
    time_pts.push_back(chrono::high_resolution_clock::now());
    catg_of_time_pts.push_back(OP_CATALOG_VAR_CREATE_MSG_FOR_CLIENT);
    
    net::SendMsg(peer, ldo_msg);
    state=State::done;
    time_pts.push_back(chrono::high_resolution_clock::now());
    catg_of_time_pts.push_back(OP_CATALOG_VAR_SEND_MSG_TO_CLIENT_OP_DONE);
    return WaitingType::done_and_destroy;
  }
  case State::done:
    return WaitingType::done_and_destroy;
  }
}
//WaitingType OpCatalogVarMeta::UpdateOrigin(OpArgs &args, results_t *results) {
WaitingType OpCatalogVarMeta::UpdateOrigin(OpArgs &, results_t *) {
  
  return WaitingType::error;

}

int md_catalog_var_stub (md_catalog_args &args, 
                     uint32_t &count,
                     std::vector<md_catalog_var_entry> &entries)
{
    int rc;
    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;
    const char * query = "select id, name, path, version, active, txn_id, num_dims, d0_min, d0_max, d1_min, d1_max, d2_min, d2_max from var_catalog where txn_id = ? or active = 1 order by path, name, version";
    size_t size = 0;

    rc = sqlite3_prepare_v2 (db, "select count (*) from var_catalog where txn_id = ? or active = 1", -1, &stmt, &tail); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int (stmt, 1, args.txn_id); assert (rc == SQLITE_OK);
    rc = sqlite3_step (stmt); assert (rc == SQLITE_OK || rc == SQLITE_ROW);
    count = sqlite3_column_int (stmt, 0);
    rc = sqlite3_finalize (stmt); assert (rc == SQLITE_OK);

//printf ("rows in var_catalog: %d\n", count);
    if (count > 0) {
      entries.reserve(count);

        rc = sqlite3_prepare_v2 (db, query, -1, &stmt, &tail); assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int (stmt, 1, args.txn_id); assert (rc == SQLITE_OK);
        rc = sqlite3_step (stmt); assert (rc == SQLITE_ROW || rc == SQLITE_OK);

        while (rc == SQLITE_ROW)
        {
            int j = 0;

            md_catalog_var_entry entry;


            entry.var_id = sqlite3_column_int (stmt, 0);
            entry.name = (char *)sqlite3_column_text (stmt, 1);
            entry.path = (char *)sqlite3_column_text (stmt, 2);
            entry.version = sqlite3_column_int (stmt, 3);
            entry.active = sqlite3_column_int (stmt, 4);
            entry.txn_id = sqlite3_column_int (stmt, 5);
            entry.num_dims = sqlite3_column_int (stmt, 6);
            entry.dims.reserve(entry.num_dims);
    //printf ("id: %d name: %s path: %s version: %d num_dims: %d ", id, entry [i].name, entry [i].path, entry [i].version, entry [i].num_dims);


            char v = 'x';
            while (j < entry.num_dims)
            {
                md_dim_bounds bounds;
                bounds.min =  sqlite3_column_int (stmt, 7 + (j * 2));
                bounds.max = sqlite3_column_int (stmt, 8 + (j * 2));
                entry.dims.push_back(bounds);
    //printf ("%c: (%d/%d) ", v++, entry [i].dim [j].min, entry [i].dim [j].max);
                j++;
            }
    //printf ("\n");

            rc = sqlite3_step (stmt);
            entries.push_back(entry);
        }

        rc = sqlite3_finalize (stmt);  
    }
    

cleanup:

    return rc;
}
