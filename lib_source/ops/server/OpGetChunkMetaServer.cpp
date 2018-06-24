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

#include "OpGetChunkMetaCommon.hh"
#include <server_timing_constants.hh>
#include <sqlite3.h>

extern sqlite3 *db;

using namespace std;

int md_get_chunk_stub (const md_get_chunk_args &args,
                       uint32_t &count,
                       std::vector<md_chunk_entry> &chunk_list);

static int get_matching_chunk_count (const md_get_chunk_args &args, uint32_t &count);


void OpGetChunkMeta::deArchiveMsgFromClient(const std::string &serial_str, 
                                             md_get_chunk_args &args) {

    //log("the archived message is " + serial_str);

  std::stringstream ss;
  ss << serial_str;
  boost::archive::text_iarchive ia(ss);
  ia >> args;
}

std::string OpGetChunkMeta::serializeMsgToClient(const std::vector<md_chunk_entry> &chunk_list,
                                                  uint32_t count,
                                                  int return_value) {

  std::stringstream ss;
  boost::archive::text_oarchive oa(ss);
  oa << count;
  if (count > 0) {
    oa << chunk_list;
  }
  oa << return_value;
  //log("the archived message is " + ss.str());


  return ss.str();
}


WaitingType OpGetChunkMeta::UpdateTarget(OpArgs &args, results_t *results) {
  message_t *incoming_msg = args.data.msg.ptr;

  switch(state){

    case State::start: {
    time_pts.push_back(chrono::high_resolution_clock::now());
    catg_of_time_pts.push_back(OP_GET_CHUNK_START);
    md_get_chunk_args var_args;
    std::vector<md_chunk_entry> chunk_list;
    uint32_t count;

    
    peer = args.data.msg.sender;
    

    deArchiveMsgFromClient(incoming_msg->body, var_args);
    time_pts.push_back(chrono::high_resolution_clock::now());
    catg_of_time_pts.push_back(OP_GET_CHUNK_DEARCHIVE_MSG_FROM_CLIENT);

    int rc = md_get_chunk_stub(var_args, count, chunk_list);
    time_pts.push_back(chrono::high_resolution_clock::now());
    catg_of_time_pts.push_back(OP_GET_CHUNK_MD_GET_CHUNK_STUB);


    // for (int i = 0; i < count; i++)
    // {
    //     std::cout << "chunk id: " << chunk_list [i].chunk_id << " connection: " << chunk_list [i].connection << " length: " << chunk_list [i].length_of_chunk << " num_dims: " << chunk_list [i].num_dims;
    //     char v = 'x';
    //     for (int j = 0; j < chunk_list [i].num_dims; j++)
    //     {
    //         std::cout << " " << v << ": (" << chunk_list [i].dims [j].min << "/" << chunk_list [i].dims [j].max << ") ";
    //         v++;
    //     }
    //     std::cout << "\n";
    // }

    std::string serial_str = serializeMsgToClient(chunk_list, count, rc);
    time_pts.push_back(chrono::high_resolution_clock::now());
    catg_of_time_pts.push_back(OP_GET_CHUNK_SERIALIZE_MSG_FOR_CLIENT);

    createOutgoingMessage(incoming_msg->src, 
                          0, //Not expecting a reply
                          incoming_msg->src_mailbox,
                          serial_str);
    time_pts.push_back(chrono::high_resolution_clock::now());
    catg_of_time_pts.push_back(OP_GET_CHUNK_CREATE_MSG_FOR_CLIENT);

    net::SendMsg(peer, ldo_msg);
    state=State::done;
    time_pts.push_back(chrono::high_resolution_clock::now());
    catg_of_time_pts.push_back(OP_GET_CHUNK_SEND_MSG_TO_CLIENT_OP_DONE);
    return WaitingType::done_and_destroy;
  }
  case State::done:
    return WaitingType::done_and_destroy;
  }
}

//WaitingType OpGetChunkMeta::UpdateOrigin(OpArgs &args, results_t *results) {
WaitingType OpGetChunkMeta::UpdateOrigin(OpArgs &, results_t *) {
  
  return WaitingType::error;

}


int md_get_chunk_stub (const md_get_chunk_args &args,
                       uint32_t &count,
                       std::vector<md_chunk_entry> &chunk_list)
{
    int rc;
    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;
    const char * query = "select vd.chunk_id, vd.connection, vd.length, vc.num_dims, vd.d0_min, vd.d0_max, vd.d1_min, vd.d1_max, vd.d2_min, vd.d2_max from chunk_data vd, var_catalog vc where "
"vc.name = ? and vc.path = ? and vc.version = ? "
"and (vc.txn_id = ? or vc.active = 1) "
"and vc.id = vd.var_id "
"and ? <= vd.d0_max and ? >= vd.d0_min "
"and ? <= vd.d1_max and ? >= vd.d1_min "
"and ? <= vd.d2_max and ? >= vd.d2_min ";
    // const char * query = "select id, name, path, version, active, txn_id, num_dims, d0_min, d0_max, d1_min, d1_max, d2_min, d2_max from var_catalog where "
    // "vc.name = ? and vc.path = ? and vc.version = ? "

    // "and ? <= d0_max and ? >= d0_min "
    // "and ? <= d1_max and ? >= d1_min "
    // "and ? <= d2_max and ? >= d2_min ";

    // std::cout << "var name is " << args.name.c_str() << std::endl;
    // std::cout << "var var_version is " << args.var_version << std::endl;
    // std::cout << "var path is " << args.path.c_str() << std::endl;
    // std::cout << "var txn_id is " << args.txn_id << std::endl;
    // std::cout << "var num_dims is " << args.num_dims << std::endl;

    // for (int j = 0; j < args.num_dims; j++)
    // {
    //     std::cout << "var dim " << j << " min is " << args.dims.at(j).min<< std::endl;
    //     std::cout << "var dim " << j << " max is " << args.dims.at(j).max<< std::endl;
    // }

    rc = get_matching_chunk_count (args, count); assert (rc == RC_OK);

    // std::cout << "matching chunk count is " << count << std::endl;


    if (count > 0) {
        chunk_list.reserve(count);

        rc = sqlite3_prepare_v2 (db, query, -1, &stmt, &tail); 
        if (rc != SQLITE_OK)
        {
            fprintf (stderr, "Line: %d SQL error (%d): %s\n", __LINE__, rc, sqlite3_errmsg (db));
            sqlite3_close (db);
            goto cleanup;
        }    
        rc = sqlite3_bind_text (stmt, 1, args.name.c_str(), -1, free); assert (rc == SQLITE_OK);
        rc = sqlite3_bind_text (stmt, 2, 
    args.path.c_str(), -1, free); assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int (stmt, 3, args.var_version); assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int (stmt, 4, args.txn_id); assert (rc == SQLITE_OK);
        for (int j = 0; j < args.num_dims; j++)
        {
            rc = sqlite3_bind_int (stmt, 5 + (j * 2), args.dims.at(j).min);
            rc = sqlite3_bind_int (stmt, 6 + (j * 2), args.dims.at(j).max);
        }
        rc = sqlite3_step (stmt); assert (rc == SQLITE_OK || rc == SQLITE_ROW);

        while (rc == SQLITE_ROW)
        {
    //printf ("writing item: %d\n", i);
            md_chunk_entry chunk;

            chunk.chunk_id = sqlite3_column_int (stmt, 0);
            chunk.connection = (char *) sqlite3_column_text (stmt, 1);
            chunk.length_of_chunk = sqlite3_column_int (stmt, 2);
            chunk.num_dims = sqlite3_column_int (stmt, 3);
            chunk.dims.reserve(chunk.num_dims);

    //printf ("GC id: %d name: %s path: %s version: %d ", id, args.name, args.path, args.var_version);
            int j = 0;
            // char v = 'x';

            while (j < chunk.num_dims)
            {
                md_dim_bounds bounds;
                bounds.min =  sqlite3_column_int (stmt, 4 + (j * 2));
                bounds.max = sqlite3_column_int (stmt, 5 + (j * 2));
                chunk.dims.push_back(bounds);
    //printf ("%c: (%d/%d) ", v++, chunk_list [i].dim [j].min, chunk_list [i].dim [j].max);
                j++;
            }
    //printf ("\n");

            rc = sqlite3_step (stmt);
            chunk_list.push_back(chunk);
        }

        rc = sqlite3_finalize (stmt);
    }
cleanup:

    return rc;
}

//note: this appears in get_chunk_count also
static int get_matching_chunk_count (const md_get_chunk_args &args, uint32_t &count)
{
    int rc;
    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;
    const char * query = "select count (*) from chunk_data vd, var_catalog vc where "
"vc.name = ? and vc.path = ? and vc.version = ? "
"and (vc.txn_id = ? or vc.active = 1) "
"and vc.id = vd.var_id "
"and ? <= vd.d0_max and ? >= vd.d0_min "
"and ? <= vd.d1_max and ? >= vd.d1_min "
"and ? <= vd.d2_max and ? >= vd.d2_min ";

    rc = sqlite3_prepare_v2 (db, query, -1, &stmt, &tail); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_text (stmt, 1, strdup(args.name.c_str()), -1, free); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_text (stmt, 2, strdup(args.path.c_str()), -1, free); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int (stmt, 3, args.var_version); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int (stmt, 4, args.txn_id); assert (rc == SQLITE_OK);
//printf ("looking for name: %s path: %s version: %d ", args->name, args->path, args->var_version);
    for (int j = 0; j < args.num_dims; j++)
    {
        rc = sqlite3_bind_int (stmt, 5 + (j * 2), args.dims.at(j).min);
        rc = sqlite3_bind_int (stmt, 6 + (j * 2), args.dims.at(j).max);
//printf ("(%d:%d) ", dims [j].min, dims [j].max);
    }
//printf ("\n");
    rc = sqlite3_step (stmt); assert (rc == SQLITE_OK || rc == SQLITE_ROW);
    count = sqlite3_column_int (stmt, 0);
    rc = sqlite3_finalize (stmt); assert (rc == SQLITE_OK);
//printf ("matching chunk count: %d\n", *count);

    return rc;
}