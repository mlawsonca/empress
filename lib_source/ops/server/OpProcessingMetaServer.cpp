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


#include "OpProcessingMetaCommon.hh"
#include <server_timing_constants.hh>
#include <sqlite3.h>

extern sqlite3 *db;

using namespace std;

int md_processing_stub (const md_processing_args &args);


void OpProcessingMeta::deArchiveMsgFromClient(const std::string &serial_str, md_processing_args &args) {

  //log("the archived message is " + serial_str);

	std::stringstream ss;
	ss << serial_str;
	boost::archive::text_iarchive ia(ss);
	ia >> args;
}

std::string OpProcessingMeta::serializeMsgToClient(int return_value) {

	std::stringstream ss;
	boost::archive::text_oarchive oa(ss);
	oa << return_value;
	//log("the archived message is " + ss.str());


	return ss.str();
}



WaitingType OpProcessingMeta::UpdateTarget(OpArgs &args, results_t *results) {
  message_t *incoming_msg = args.data.msg.ptr;

  switch(state){

    case State::start: {
	    add_timing_point(OP_PROCESSING_START);
	    md_processing_args sql_args;

	    peer = args.data.msg.sender;
	    

	    deArchiveMsgFromClient(incoming_msg->body, sql_args);    
	    add_timing_point(OP_PROCESSING_DEARCHIVE_MSG_FROM_CLIENT);

	    int rc = md_processing_stub(sql_args);
	    add_timing_point(OP_PROCESSING_MD_PROCESSING_STUB);


	    std::string serial_str = serializeMsgToClient(rc);
	    add_timing_point(OP_PROCESSING_SERIALIZE_MSG_FOR_CLIENT);

	    createOutgoingMessage(incoming_msg->src, 
	                          0, //Not expecting a reply
	                          incoming_msg->src_mailbox,
	                          serial_str);
	    add_timing_point(OP_PROCESSING_CREATE_MSG_FOR_CLIENT);

	    net::SendMsg(peer, ldo_msg);
	    state=State::done;
	    add_timing_point(OP_PROCESSING_SEND_MSG_TO_CLIENT_OP_DONE);
	    return WaitingType::done_and_destroy;
  	}
	case State::done:
    	return WaitingType::done_and_destroy;
  	}
}

//WaitingType OpProcessingMeta::UpdateOrigin(OpArgs &args, results_t *results) {
WaitingType OpProcessingMeta::UpdateOrigin(OpArgs &, results_t *) {
  
	return WaitingType::error;

}

int md_processing_stub (const md_processing_args &args)
{
    int rc;
    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;

    string query;

    switch(args.catalog_type) {
    	case RUN_CATALOG : {
    		query = "update run_catalog set active = 2 where txn_id = ?";
    		break;
    	}
    	case TIMESTEP_CATALOG : {
    		query = "update timestep_catalog set active = 2 where txn_id = ?";
    		break;
    	}
    	case VAR_CATALOG : {
    		query = "update var_catalog set active = 2 where txn_id = ?";
    		break;
    	}
    	case TYPE_CATALOG : {
    		query = "update type_catalog set active = 2 where txn_id = ?";
    		break;
    	}
    	case RUN_ATTR_CATALOG : {
    		query = "update run_attribute_catalog set active = 2 where txn_id = ?";
    		break;
    	}
    	case TIMESTEP_ATTR_CATALOG : {
    		query = "update timestep_attribute_catalog set active = 2 where txn_id = ?";
    		break;
    	}
    	case VAR_ATTR_CATALOG : {
    		query = "update var_attribute_catalog set active = 2 where txn_id = ?";
    		break;
    	}
    }

    rc = sqlite3_prepare_v2 (db, query.c_str(), -1, &stmt, &tail); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 1, args.txn_id); assert (rc == SQLITE_OK);
    rc = sqlite3_step (stmt); assert (rc == SQLITE_OK || rc == SQLITE_ROW || rc == SQLITE_DONE);
    rc = sqlite3_finalize (stmt); assert (rc == SQLITE_OK);

    return rc;
}