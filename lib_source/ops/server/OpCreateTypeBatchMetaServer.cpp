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


#include "OpCreateTypeBatchMetaCommon.hh"
#include <server_timing_constants.hh>
#include <sqlite3.h>

extern sqlite3 *db;

using namespace std;

// int md_create_type_batch_stub (const vector<md_create_type_args> &all_args,
//                         uint64_t &type_id);
// int md_create_type_batch_stub (const vector<md_create_type_args> &all_args);
int md_create_type_batch_stub (const vector<md_create_type_args> &all_args
							,vector<uint64_t> &type_ids
							);

extern int callback (void * NotUsed, int argc, char ** argv, char ** ColName);

WaitingType OpCreateTypeBatchMeta::UpdateTarget(OpArgs &args, results_t *results) {
    // std::cout << "About to update target \n";

  message_t *incoming_msg = args.data.msg.ptr;

  switch(state){

    case State::start: {
	    add_timing_point(OP_CREATE_TYPE_BATCH_START);
	    // std::cout << "About to case start \n";

	    vector<md_create_type_args> sql_args;
	    // uint64_t type_id;
	    vector<uint64_t> type_ids;
	    type_ids.reserve(sql_args.size());

	    // net::Connect(&peer, incoming_msg->src);

	    peer = args.data.msg.sender;
	    
	    // std::cout << "About to dearchive msg from client \n";
	    //convert the serialized string back into the args and count pointer
	    deArchiveMsgFromClient(incoming_msg->body, sql_args);    
	    add_timing_point(OP_CREATE_TYPE_BATCH_DEARCHIVE_MSG_FROM_CLIENT);

	    // std::cout << "About to create var stub \n";
	    // int rc = md_create_type_batch_stub(sql_args, type_id);
	    // int rc = md_create_type_batch_stub(sql_args);	    
	    int rc = md_create_type_batch_stub(sql_args, type_ids);
	    add_timing_point(OP_CREATE_TYPE_BATCH_MD_CREATE_TYPE_BATCH_STUB);

	    std::string serial_str = serializeMsgToClient(type_ids, rc);
	    // std::string serial_str = serializeMsgToClient(rc);
	    // std::string serial_str = serializeMsgToClient(type_id, rc);
	    add_timing_point(OP_CREATE_TYPE_BATCH_SERIALIZE_MSG_FOR_CLIENT);

	    createOutgoingMessage(incoming_msg->src, 
	                          0, //Not expecting a reply
	                          incoming_msg->src_mailbox,
	                          serial_str);
	    add_timing_point(OP_CREATE_TYPE_BATCH_CREATE_MSG_FOR_CLIENT);

	    net::SendMsg(peer, ldo_msg);
	    state=State::done;
	    add_timing_point(OP_CREATE_TYPE_BATCH_SEND_MSG_TO_CLIENT_OP_DONE);
	    return WaitingType::done_and_destroy;
	}
	case State::done:
    	return WaitingType::done_and_destroy;
  	}
}

//WaitingType OpCreateTypeBatchMeta::UpdateOrigin(OpArgs &args, results_t *results) {
WaitingType OpCreateTypeBatchMeta::UpdateOrigin(OpArgs &, results_t *) {
    //log("error. its asking me to update origin \n");
	return WaitingType::error;

}

// int md_create_type_batch_stub (const vector<md_create_type_args> &args,
//                         uint64_t &type_id)
// int md_create_type_batch_stub (const vector<md_create_type_args> &all_args)
int md_create_type_batch_stub (const vector<md_create_type_args> &all_args,
                        vector<uint64_t> &type_ids)
{
    int rc;
    // int i;
    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;
    char * ErrMsg = NULL;

    // int rowid;

    rc = sqlite3_exec (db, "begin;", callback, 0, &ErrMsg);
    if (rc != SQLITE_OK)
    {
        fprintf (stderr, "Error begin md_create_type_batch_stub: Line: %d SQL error: %s\n", __LINE__, ErrMsg);
        sqlite3_free (ErrMsg);
        sqlite3_close (db);
        goto cleanup;
    }

    rc = sqlite3_prepare_v2 (db, "insert into type_catalog (id, run_id, name, version, active, txn_id) values (?, ?, ?, ?, ?, ?)", -1, &stmt, &tail);
    if (rc != SQLITE_OK)
    {
        fprintf (stderr, "Error start of create_type_batch_stub: Line: %d SQL error: %s\n", __LINE__, sqlite3_errmsg (db));
        sqlite3_close (db);
        goto cleanup;
    }

    for(int arg_indx = 0; arg_indx < all_args.size(); arg_indx++) {

    	md_create_type_args args = all_args.at(arg_indx);

	    // rc = sqlite3_bind_int64 (stmt, 1, args.type_id); assert (rc == SQLITE_OK);    
	    rc = sqlite3_bind_null (stmt, 1); assert (rc == SQLITE_OK);
	    rc = sqlite3_bind_int64 (stmt, 2, args.run_id); assert (rc == SQLITE_OK);    
	    rc = sqlite3_bind_text (stmt, 3, strdup(args.name.c_str()), -1, free); assert (rc == SQLITE_OK);
	    rc = sqlite3_bind_int (stmt, 4, args.version); assert (rc == SQLITE_OK);
	    rc = sqlite3_bind_int (stmt, 5, 0); assert (rc == SQLITE_OK);
	    rc = sqlite3_bind_int64 (stmt, 6, args.txn_id); assert (rc == SQLITE_OK);

	    // cout << "run_id: " << args.run_id << " name: " << args.name << " version: " << args.version << " txn_id: " << args.txn_id << endl;

	    rc = sqlite3_step (stmt); assert (rc == SQLITE_OK || rc == SQLITE_DONE);
	    
    	uint64_t type_id = (int) sqlite3_last_insert_rowid (db);
    	type_ids.push_back(type_id);

		rc = sqlite3_reset(stmt); assert (rc == SQLITE_OK);
	}

//printf ("generated new global id:%d\n", *type_id);

    rc = sqlite3_finalize (stmt);

    rc = sqlite3_exec (db, "end;", callback, 0, &ErrMsg);
    if (rc != SQLITE_OK)
    {
        fprintf (stderr, "Error end query md_creat_type_batch_stub: Line: %d SQL error: %s\n", __LINE__, ErrMsg);
        sqlite3_free (ErrMsg);
        sqlite3_close (db);
        goto cleanup;
    }


cleanup:
    if (rc != SQLITE_OK)
    {
        rc = sqlite3_exec (db, "rollback;", callback, 0, &ErrMsg);
    }
    return rc;
}


void OpCreateTypeBatchMeta::deArchiveMsgFromClient(const std::string &serial_str, vector<md_create_type_args> &all_args) {

    //log("the archived message is " + serial_str);

	std::stringstream ss;
	ss << serial_str;
	boost::archive::text_iarchive ia(ss);
	ia >> all_args;

  // cout << "Server just dearchived message from client \n";

}

// std::string OpCreateTypeBatchMeta::serializeMsgToClient(uint64_t type_id,
//                                                   int return_value) {
// std::string OpCreateTypeBatchMeta::serializeMsgToClient(int return_value) {
std::string OpCreateTypeBatchMeta::serializeMsgToClient(const vector<uint64_t> &type_ids
												,int return_value
												) 
{

	stringstream ss;
	boost::archive::text_oarchive oa(ss);
	// oa << type_id;
	oa << type_ids;
	oa << return_value;
	//log("the archived message is " + ss.str());


	// cout << "Server just serialized message to client \n";
	return ss.str();
}
