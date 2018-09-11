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


#include "OpCreateRunMetaCommon.hh"
#include <server_timing_constants.hh>
#include <sqlite3.h>

extern sqlite3 *db;

using namespace std;
int md_create_run_stub (const md_create_run_args &args,
                        uint64_t &run_id);

// void OpCreateRunMeta::deArchiveMsgFromClient(const std::string &serial_str, md_create_run_args &args) {

//     //log("the archived message is " + serial_str);
//    cout << "am about to dearchive \n";
//    cout << "serial_str: " << serial_str << endl;
//    cout << "serial_str.size(): " << serial_str.size() << endl;
    
//   std::stringstream ss;
//   ss << serial_str;
//   cout << "just put serial_str in ss \n";
//   cout << "ss: " << ss.str() << endl;
//   boost::archive::text_iarchive ia(ss);
//   cout << "just converted to text arhive \n";
//   ia >> args;
//   cout << "Server just dearchived message from client \n";

// }

void OpCreateRunMeta::deArchiveMsgFromClient(message_t *incoming_msg, md_create_run_args &args) {

	std::stringstream ss;
	ss.write(incoming_msg->body, incoming_msg->body_len);
	boost::archive::text_iarchive ia(ss);
	ia >> args;
	// cout << "deserialized text string: " << ss.str() << endl;
	// cout << "deserialized text string length: " << ss.str().size() << endl;

}

std::string OpCreateRunMeta::serializeMsgToClient(uint64_t run_id,
                                                  int return_value) {

	stringstream ss;
	boost::archive::text_oarchive oa(ss);
	oa << run_id;
	oa << return_value;
	//log("the archived message is " + ss.str());


	// cout << "Server just serialized message to client \n";
	return ss.str();
}



WaitingType OpCreateRunMeta::UpdateTarget(OpArgs &args, results_t *results) {
    //log("About to update target");

  message_t *incoming_msg = args.data.msg.ptr;

  switch(state){

    case State::start: {
	    add_timing_point(OP_CREATE_RUN_START);
	    // //log("About to case start");

	    md_create_run_args sql_args;
	    uint64_t run_id;

	    // net::Connect(&peer, incoming_msg->src);

	    peer = args.data.msg.sender;
	    
	    // //log("About to dearchive msg from client");
	    //convert the serialized string back into the args and count pointer
	    // deArchiveMsgFromClient((std::string)incoming_msg->body, sql_args); 

	    deArchiveMsgFromClient(incoming_msg, sql_args);    

	    add_timing_point(OP_CREATE_RUN_DEARCHIVE_MSG_FROM_CLIENT);

	    // //log("About to create run stub");
	    int rc = md_create_run_stub(sql_args, run_id);
	    add_timing_point(OP_CREATE_RUN_MD_CREATE_RUN_STUB);

	    // cout << "run_id: " << run_id << " rc: " << rc << endl;

	    std::string serial_str = serializeMsgToClient(run_id, rc);
	    add_timing_point(OP_CREATE_RUN_SERIALIZE_MSG_FOR_CLIENT);
	    
	    // cout << "got here 3 " << endl;

	    createOutgoingMessage(incoming_msg->src, 
	                          0, //Not expecting a reply
	                          incoming_msg->src_mailbox,
	                          serial_str);
	    add_timing_point(OP_CREATE_RUN_CREATE_MSG_FOR_CLIENT);

	    net::SendMsg(peer, ldo_msg);
	    state=State::done;
	    add_timing_point(OP_CREATE_RUN_SEND_MSG_TO_CLIENT_OP_DONE);
	    return WaitingType::done_and_destroy;
  	}
	case State::done:
    	return WaitingType::done_and_destroy;
  	}
}

//WaitingType OpCreateRunMeta::UpdateOrigin(OpArgs &args, results_t *results) {
WaitingType OpCreateRunMeta::UpdateOrigin(OpArgs &, results_t *) {
    //log("error. its asking me to update origin \n");
	return WaitingType::error;

}

int md_create_run_stub (const md_create_run_args &args,
                        uint64_t &run_id)
{
    int rc;
    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;

    // size_t len = 0;

    rc = sqlite3_prepare_v2 (db, "insert into run_catalog (id, job_id, name, date, active, txn_id, npx, npy, npz, rank_to_dims_funct, objector_funct) values (?, ?, ?, datetime('now','localtime'), ?, ?, ?, ?, ?, ?, ?)", -1, &stmt, &tail);
    if (rc != SQLITE_OK)
    {
        fprintf (stderr, "Error start of create_run_stub: Line: %d SQL error: %s\n", __LINE__, sqlite3_errmsg (db));
        sqlite3_close (db);
        goto cleanup;
    }

    // cout << "got here " << endl;

    // cout << "name: " << args.name << " path: " << args.path << " txn_id: " << args.txn_id << 
    // 	" npx: " << args.npx << " npy: " << args.npy << " npz: " << args.npz << endl;

    // cout << "rank_to_dims_funct " << args.rank_to_dims_funct.c_str() << endl;
    // cout << "objector_funct " << args.objector_funct.c_str() << endl;
   
    
    rc = sqlite3_bind_null (stmt, 1); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 2, args.job_id); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_text (stmt, 3, strdup(args.name.c_str()), -1, free); assert (rc == SQLITE_OK);
    // rc = sqlite3_bind_text (stmt, 3, strdup(args.path.c_str()), -1, free); assert (rc == SQLITE_OK);
    // rc = sqlite3_bind_text (stmt, 3, datetime('now','localtime'), -1, free); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int (stmt, 4, 0); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 5, args.txn_id); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int (stmt, 6, args.npx); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int (stmt, 7, args.npy); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int (stmt, 8, args.npz); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_text (stmt, 9, args.rank_to_dims_funct.c_str(), args.rank_to_dims_funct.size() + 1, free); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_text (stmt, 10, args.objector_funct.c_str(), args.objector_funct.size()+1, free); assert (rc == SQLITE_OK);

   //  char *rank_to_dims_funct = malloc(args.rank_to_dims_funct.size() + 1);
   //  char *objector_funct = malloc(args.objector_funct.size() + 1);
   

  	// archived_msg.copy(&msg->body[0], archived_msg.size());
  	// msg->body[archived_msg.size()] = '\0';

	// rc = sqlite3_bind_blob(stmt, 9, (void*)args.rank_to_dims_funct.c_str(), args.rank_to_dims_funct.size()+1, free);
	// rc = sqlite3_bind_blob(stmt, 10, (void*)args.objector_funct.c_str(), args.objector_funct.size()+1, free);

	// rc = sqlite3_bind_blob(stmt, 9, (void*)&args.rank_to_dims_funct[0], args.rank_to_dims_funct.size()+1, free);
	// rc = sqlite3_bind_blob(stmt, 10, (void*)&args.objector_funct[0], args.objector_funct.size()+1, free);
	// rc = sqlite3_bind_blob(stmt, 9, static_cast<void*>(&args.rank_to_dims_funct[0]), args.rank_to_dims_funct.size()+1, free);
	// rc = sqlite3_bind_blob(stmt, 10, static_cast<void*>(&args.objector_funct[0]), args.objector_funct.size()+1, free);
	

    // cout << "name: " << args.name << " path: " << args.path << " args.txn_id: " << args.txn_id << " args.npx: " << args.npx << " args.npy: " << args.npy << " args.npz: " << args.npz << endl;
    // // cout << "rank_to_dims_funct.size(): " << rank_to_dims_funct.size() << " objector_funct.size(): " << objector_funct.size() << endl;
    // cout << "rank_to_dims_funct: " << args.rank_to_dims_funct << " objector_funct: " << args.objector_funct << endl;

    rc = sqlite3_step (stmt); //assert (rc == SQLITE_OK || rc == SQLITE_DONE);
    if (rc != SQLITE_OK && rc != SQLITE_DONE)
    {
        fprintf (stderr, "Error end of create datset stub: Line: %d SQL error: %s (%d)\n", __LINE__, sqlite3_errmsg (db), rc);
        sqlite3_close (db);
        goto cleanup;
    }

    // cout << "got here 2 \n";

    run_id = (int) sqlite3_last_insert_rowid (db);
    // cout << "run_id: " << run_id << " \n";

    rc = sqlite3_finalize (stmt);
    // cout << "rc: " << rc << " \n";

cleanup:

    return rc;
}