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

#include "OpGetChunkListMetaCommon.hh"
#include <server_timing_constants.hh>
#include <sqlite3.h>

extern sqlite3 *db;

using namespace std;

int md_get_chunk_list_stub (const md_get_chunk_list_args &args,
                            std::vector<md_chunk_entry> &entries,
                            uint32_t &count);

static int get_chunk_list_count (const md_get_chunk_list_args &args, uint32_t &count);


void OpGetChunkListMeta::deArchiveMsgFromClient(const std::string &serial_str, 
                                                 md_get_chunk_list_args &args) {

    //log("the archived message is " + serial_str);

  std::stringstream ss;
  ss << serial_str;
  boost::archive::text_iarchive ia(ss);
  ia >> args;
}

std::string OpGetChunkListMeta::serializeMsgToClient(const std::vector<md_chunk_entry> &entries,
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


WaitingType OpGetChunkListMeta::UpdateTarget(OpArgs &args, results_t *results) {
  message_t *incoming_msg = args.data.msg.ptr;

  switch(state){

    case State::start: {
    time_pts.push_back(chrono::high_resolution_clock::now());
    catg_of_time_pts.push_back(OP_GET_CHUNK_LIST_START);
    md_get_chunk_list_args var_args;
    std::vector<md_chunk_entry> entries;
    uint32_t count;

    peer = args.data.msg.sender;
    

    deArchiveMsgFromClient(incoming_msg->body, var_args);
    time_pts.push_back(chrono::high_resolution_clock::now());
    catg_of_time_pts.push_back(OP_GET_CHUNK_LIST_DEARCHIVE_MSG_FROM_CLIENT);

    int rc = md_get_chunk_list_stub(var_args, entries, count);
    time_pts.push_back(chrono::high_resolution_clock::now());
    catg_of_time_pts.push_back(OP_GET_CHUNK_LIST_MD_GET_CHUNK_LIST_STUB);

    if (entries.size() > 0) {
        //log("the first entry has connection " + entries.at(0).connection);

    }

    std::string serial_str = serializeMsgToClient(entries, count, rc);
    time_pts.push_back(chrono::high_resolution_clock::now());
    catg_of_time_pts.push_back(OP_GET_CHUNK_LIST_SERIALIZE_MSG_FOR_CLIENT);

    createOutgoingMessage(incoming_msg->src, 
                          0, //Not expecting a reply
                          incoming_msg->src_mailbox,
                          serial_str);
    time_pts.push_back(chrono::high_resolution_clock::now());
    catg_of_time_pts.push_back(OP_GET_CHUNK_LIST_CREATE_MSG_FOR_CLIENT);

    net::SendMsg(peer, ldo_msg);
    state=State::done;
    time_pts.push_back(chrono::high_resolution_clock::now());
    catg_of_time_pts.push_back(OP_GET_CHUNK_LIST_SEND_MSG_TO_CLIENT_OP_DONE);
    return WaitingType::done_and_destroy;
  }
  case State::done:
    return WaitingType::done_and_destroy;
  }
}

//WaitingType OpGetChunkListMeta::UpdateOrigin(OpArgs &args, results_t *results) {
WaitingType OpGetChunkListMeta::UpdateOrigin(OpArgs &, results_t *) {
  
  return WaitingType::error;

}



int md_get_chunk_list_stub (const md_get_chunk_list_args &args,
                            std::vector<md_chunk_entry> &entries,
                            uint32_t &count)
{
    int rc;
    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;
    const char * query = "select cd.chunk_id, cd.length, cd.connection, vc.num_dims, cd.d0_min, cd.d0_max, cd.d1_min, cd.d1_max, cd.d2_min, cd.d2_max from chunk_data cd, var_catalog vc where vc.name = ? and vc.path = ? and vc.version = ? and vc.id = cd.var_id and (vc.txn_id = ? or vc.active = 1)";

    rc = get_chunk_list_count (args, count);
//printf ("CL chunks: %d\n", count);

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
        rc = sqlite3_bind_text (stmt, 1, strdup(args.name.c_str()), -1, free); assert (rc == SQLITE_OK);
        rc = sqlite3_bind_text (stmt, 2, strdup(args.path.c_str()), -1, free); assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int (stmt, 3, args.var_version); assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int (stmt, 4, args.txn_id); assert (rc == SQLITE_OK);
        rc = sqlite3_step (stmt);
//printf ("CL rc: %d\n", rc);

        int j = 0;
        while (rc == SQLITE_ROW)
        {
            // int id;
            md_chunk_entry entry;

//printf ("CL: entry: %d\n", j);

            entry.chunk_id = sqlite3_column_int (stmt, 0);
            entry.length_of_chunk = sqlite3_column_int (stmt, 1);
            entry.connection = (char *) sqlite3_column_text (stmt, 2);
            entry.num_dims = sqlite3_column_int (stmt, 3);

//printf ("CL num_dims: %d\n", entries [j].num_dims);


//printf ("CL id: %d length: %ld connection: %s num_dims: %d ", id, entries [j].length_of_chunk, entries [j].connection, entries [j].num_dims);
            // char v = 'x';
            int i = 0;
            entry.dims.reserve(entry.num_dims);

            while (i < entry.num_dims)
            {
                md_dim_bounds bounds;
                bounds.min =  sqlite3_column_int (stmt, 4 + (i * 2));
                bounds.max = sqlite3_column_int (stmt, 5 + (i * 2));
                entry.dims.push_back(bounds);

//printf ("%c: (%d/%d) ", v++, entries [j].dim [i].min, entries [j].dim [i].max);
                i++;
            }
//printf ("\n");

            rc = sqlite3_step (stmt);
            entries.push_back(entry);

            j++;
        }
        rc = sqlite3_finalize (stmt); assert (rc == SQLITE_OK);
    }

cleanup:

    return rc;
}

static int get_chunk_list_count (const md_get_chunk_list_args &args, uint32_t &count)
{
    int rc;
    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;
    const char * query = "select count (*) from chunk_data cd, var_catalog vc where vc.name = ? and vc.path = ? and vc.version = ? and vc.id = cd.var_id and (vc.txn_id = ? or vc.active = 1)";

    rc = sqlite3_prepare_v2 (db, query, -1, &stmt, &tail);
    if (rc != SQLITE_OK)
    {
        fprintf (stderr, "Line: %d SQL error: %s\n", __LINE__, sqlite3_errmsg (db));
        sqlite3_close (db);
        goto cleanup;
    }
    rc = sqlite3_bind_text (stmt, 1, strdup(args.name.c_str()), -1, free); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_text (stmt, 2, strdup(args.path.c_str()), -1, free); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int (stmt, 3, args.var_version); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int (stmt, 4, args.txn_id); assert (rc == SQLITE_OK);

    rc = sqlite3_step (stmt); assert (rc == SQLITE_OK || rc == SQLITE_ROW);
    count = sqlite3_column_int (stmt, 0);
//printf ("database says %u chunks\n", *count);
    rc = sqlite3_finalize (stmt); assert (rc == SQLITE_OK);

cleanup:
    return rc;
}