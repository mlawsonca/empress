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


#include "OpCreateTimestepMetaCommon.hh"
#include <server_timing_constants.hh>
#include <sqlite3.h>

extern sqlite3 *db;

using namespace std;
int md_create_timestep_stub (const md_create_timestep_args &args,
                        uint64_t &timestep_id);

void OpCreateTimestepMeta::deArchiveMsgFromClient(const std::string &serial_str, md_create_timestep_args &args) {

    //log("the archived message is " + serial_str);
    
	std::stringstream ss;
	ss << serial_str;
	boost::archive::text_iarchive ia(ss);
	ia >> args;

  // cout << "Server just dearchived message from client \n";

}

std::string OpCreateTimestepMeta::serializeMsgToClient(uint64_t timestep_id,
                                                  int return_value) {

	stringstream ss;
	boost::archive::text_oarchive oa(ss);
	oa << timestep_id;
	oa << return_value;
	//log("the archived message is " + ss.str());


	// cout << "Server just serialized message to client \n";
	return ss.str();
}



WaitingType OpCreateTimestepMeta::UpdateTarget(OpArgs &args, results_t *results) {
    //log("About to update target");

  message_t *incoming_msg = args.data.msg.ptr;

  switch(state){

    case State::start: {
	    add_timing_point(OP_CREATE_TIMESTEP_START);
	    // //log("About to case start");

	    md_create_timestep_args sql_args;
	    uint64_t timestep_id;

	    // net::Connect(&peer, incoming_msg->src);

	    peer = args.data.msg.sender;
	    
	    // //log("About to dearchive msg from client");
	    //convert the serialized string back into the args and count pointer
	    deArchiveMsgFromClient(incoming_msg->body, sql_args);    
	    add_timing_point(OP_CREATE_TIMESTEP_DEARCHIVE_MSG_FROM_CLIENT);

	    // //log("About to create timestep stub");
	    int rc = md_create_timestep_stub(sql_args, timestep_id);
	    add_timing_point(OP_CREATE_TIMESTEP_MD_CREATE_TIMESTEP_STUB);

	    std::string serial_str = serializeMsgToClient(timestep_id, rc);
	    add_timing_point(OP_CREATE_TIMESTEP_SERIALIZE_MSG_FOR_CLIENT);

	    createOutgoingMessage(incoming_msg->src, 
	                          0, //Not expecting a reply
	                          incoming_msg->src_mailbox,
	                          serial_str);
	    add_timing_point(OP_CREATE_TIMESTEP_CREATE_MSG_FOR_CLIENT);

	    net::SendMsg(peer, ldo_msg);
	    state=State::done;
	    add_timing_point(OP_CREATE_TIMESTEP_SEND_MSG_TO_CLIENT_OP_DONE);
	    return WaitingType::done_and_destroy;
  	}
	case State::done:
    	return WaitingType::done_and_destroy;
  	}
}

//WaitingType OpCreateTimestepMeta::UpdateOrigin(OpArgs &args, results_t *results) {
WaitingType OpCreateTimestepMeta::UpdateOrigin(OpArgs &, results_t *) {
    //log("error. its asking me to update origin \n");
	return WaitingType::error;

}

int md_create_timestep_stub (const md_create_timestep_args &args,
                        uint64_t &timestep_id)
{
    int rc;
    int i;
    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;
    // size_t len = 0;

    rc = sqlite3_prepare_v2 (db, "insert into timestep_catalog (id, run_id, path, active, txn_id) values (?, ?, ?, ?, ?)", -1, &stmt, &tail);
    if (rc != SQLITE_OK)
    {
        fprintf (stderr, "Error start of create_timestep_stub: Line: %d SQL error: %s\n", __LINE__, sqlite3_errmsg (db));
        sqlite3_close (db);
        goto cleanup;
    }

    rc = sqlite3_bind_int64 (stmt, 1, args.timestep_id); assert (rc == SQLITE_OK);    
    // rc = sqlite3_bind_null (stmt, 1); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 2, args.run_id); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_text (stmt, 3, strdup(args.path.c_str()), -1, free); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int (stmt, 4, 0); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 5, args.txn_id); assert (rc == SQLITE_OK);

    rc = sqlite3_step (stmt); //assert (rc == SQLITE_OK || rc == SQLITE_DONE);
    if (rc != SQLITE_OK && rc != SQLITE_DONE)
    {
        fprintf (stderr, "Error end of create_timestep_stub: Line: %d SQL error: %s (%d)\n", __LINE__, sqlite3_errmsg (db), rc);
        sqlite3_close (db);
        goto cleanup;
    }

    timestep_id = (int) sqlite3_last_insert_rowid (db);

    rc = sqlite3_finalize (stmt);

cleanup:

    return rc;
}