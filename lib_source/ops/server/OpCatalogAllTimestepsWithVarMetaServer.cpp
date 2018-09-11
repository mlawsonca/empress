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


#include "OpCatalogAllTimestepsWithVarMetaCommon.hh"
#include <server_timing_constants.hh>
#include <sqlite3.h>

extern sqlite3 *db;

using namespace std;

int md_catalog_all_timesteps_with_var_stub (md_catalog_all_timesteps_with_var_args &args, uint32_t &count, std::vector<md_catalog_timestep_entry> &entries);


void OpCatalogAllTimestepsWithVarMeta::deArchiveMsgFromClient(const std::string &serial_str, md_catalog_all_timesteps_with_var_args &args) {

    //log("the archived message is " + serial_str);

	std::stringstream ss;
	ss << serial_str;
	boost::archive::text_iarchive ia(ss);
	ia >> args;

}

std::string OpCatalogAllTimestepsWithVarMeta::serializeMsgToClient(const std::vector<md_catalog_timestep_entry> &entries,
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


WaitingType OpCatalogAllTimestepsWithVarMeta::UpdateTarget(OpArgs &args, results_t *results) {
  message_t *incoming_msg = args.data.msg.ptr;

  switch(state){

    case State::start: {
	    add_timing_point(OP_CATALOG_ALL_TIMESTEPS_WITH_VAR_START);
	    md_catalog_all_timesteps_with_var_args sql_args;
	    uint32_t count;    
	    std::vector<md_catalog_timestep_entry> entries;

	    peer = args.data.msg.sender;


	    //convert the serialized string back into the args and count pointer
	    deArchiveMsgFromClient(incoming_msg->body, sql_args);    
	    add_timing_point(OP_CATALOG_ALL_TIMESTEPS_WITH_VAR_DEARCHIVE_MSG_FROM_CLIENT);

	    int rc = md_catalog_all_timesteps_with_var_stub(sql_args, count, entries);
	    add_timing_point(OP_CATALOG_ALL_TIMESTEPS_WITH_VAR_MD_CATALOG_ALL_TIMESTEPS_WITH_VAR_STUB);

	    //log("num of entries is " + std::to_string(entries.size()));

	    std::string serial_str = serializeMsgToClient(entries, count, rc);
	    add_timing_point(OP_CATALOG_ALL_TIMESTEPS_WITH_VAR_SERIALIZE_MSG_FOR_CLIENT);

	    createOutgoingMessage(incoming_msg->src, 
	                          0, //Not expecting a reply
	                          incoming_msg->src_mailbox,
	                          serial_str);
	    add_timing_point(OP_CATALOG_ALL_TIMESTEPS_WITH_VAR_CREATE_MSG_FOR_CLIENT);
	    
	    net::SendMsg(peer, ldo_msg);
	    state=State::done;
	    add_timing_point(OP_CATALOG_ALL_TIMESTEPS_WITH_VAR_SEND_MSG_TO_CLIENT_OP_DONE);
	    return WaitingType::done_and_destroy;
	}
	case State::done:
    	return WaitingType::done_and_destroy;
  	}
}
//WaitingType OpCatalogAllTimestepsWithVarMeta::UpdateOrigin(OpArgs &args, results_t *results) {
WaitingType OpCatalogAllTimestepsWithVarMeta::UpdateOrigin(OpArgs &, results_t *) {
  
	return WaitingType::error;

}

int md_catalog_all_timesteps_with_var_stub (md_catalog_all_timesteps_with_var_args &args, 
                     uint32_t &count,
                     std::vector<md_catalog_timestep_entry> &entries)
{
    int rc;
    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;
    const char * query = "select tmc.* from timestep_catalog tmc "
    "inner join var_catalog vc on tmc.run_id = vc.run_id and tmc.id = vc.timestep_id "
    "where (tmc.txn_id = ? or tmc.active = 1) "
    "and (vc.txn_id = ? or vc.active = 1) "
    "and tmc.run_id = ? "
    "and vc.id = ? "
    "group by tmc.id, tmc.run_id "; 
    // "group by id, run_id "; 

    size_t size = 0;

    // rc = sqlite3_prepare_v2 (db, "select count(distinct timestep_catalog.id timestep_catalog.run_id) from timestep_catalog tmc "
    //   "inner join var_catalog vc on tmc.run_id = vc.run_id and tmc.id = vc.timestep_id "
    //   "where (tmc.txn_id = ? or tmc.active = 1) "
    //   "and (vc.txn_id = ? or vc.active = 1) "
    //   "and tmc.run_id = ? "
    //   "and vc.id = ? ", -1, &stmt, &tail); 
      rc = sqlite3_prepare_v2 (db, "select count(*) from (select distinct tmc.id, tmc.run_id from timestep_catalog tmc "
      "inner join var_catalog vc on tmc.run_id = vc.run_id and tmc.id = vc.timestep_id "
      "where (tmc.txn_id = ? or tmc.active = 1) "
      "and (vc.txn_id = ? or vc.active = 1) "
      "and tmc.run_id = ? "
      "and vc.id = ? ) as internalQuery", -1, &stmt, &tail); 
          if (rc != SQLITE_OK)
        {
            fprintf (stderr, "Error count md_catalog_all_timesteps_with_var_stub: Line: %d SQL error (%d): %s\n", __LINE__, rc, sqlite3_errmsg (db));
            sqlite3_close (db);
            goto cleanup;
        }
    // assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 1, args.txn_id); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 2, args.txn_id); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 3, args.run_id); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 4, args.var_id); assert (rc == SQLITE_OK);
    rc = sqlite3_step (stmt); assert (rc == SQLITE_OK || rc == SQLITE_ROW);
    count = sqlite3_column_int64 (stmt, 0);
    rc = sqlite3_finalize (stmt); assert (rc == SQLITE_OK);



//printf ("rows in run_catalog: %d\n", count);
    if (count > 0) {
      entries.reserve(count);

        rc = sqlite3_prepare_v2 (db, query, -1, &stmt, &tail); assert (rc == SQLITE_OK);
        if (rc != SQLITE_OK)
        {
            fprintf (stderr, "Error md_catalog_all_timesteps_with_var_stub: Line: %d SQL error (%d): %s\n", __LINE__, rc, sqlite3_errmsg (db));
            sqlite3_close (db);
            goto cleanup;
        }
        rc = sqlite3_bind_int64 (stmt, 1, args.txn_id); assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int64 (stmt, 2, args.txn_id); assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int64 (stmt, 3, args.run_id); assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int64 (stmt, 4, args.var_id); assert (rc == SQLITE_OK);
        rc = sqlite3_step (stmt); assert (rc == SQLITE_ROW || rc == SQLITE_OK);

        while (rc == SQLITE_ROW)
        {
            int j = 0;

            md_catalog_timestep_entry entry;


            entry.timestep_id = sqlite3_column_int64 (stmt, 0);
            entry.run_id = sqlite3_column_int64 (stmt, 1);
            entry.path = (char *)sqlite3_column_text (stmt, 2);
            entry.active = sqlite3_column_int (stmt, 3);
            entry.txn_id = sqlite3_column_int64 (stmt, 4);

    //printf ("id: %d name: %s path: %s version: %d num_dims: %d ", id, entry [i].name, entry [i].path, entry [i].version, entry [i].num_dims);
    //printf ("\n");
            rc = sqlite3_step (stmt);
            entries.push_back(entry);
        }

        rc = sqlite3_finalize (stmt);  
    }
    

cleanup:

    return rc;
}
