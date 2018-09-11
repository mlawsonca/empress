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


#include "OpCreateVarBatchMetaCommon.hh"
#include <server_timing_constants.hh>
#include <sqlite3.h>

extern sqlite3 *db;

using namespace std;
// int md_create_var_batch_stub (const vector<md_create_var_args> &args,
//                         uint64_t &row_id);
int md_create_var_batch_stub (const vector<md_create_var_args> &args);
extern int callback (void * NotUsed, int argc, char ** argv, char ** ColName);

void OpCreateVarBatchMeta::deArchiveMsgFromClient(const std::string &serial_str, vector<md_create_var_args> &args) {

    //log("the archived message is " + serial_str);
    
	std::stringstream ss;
	ss << serial_str;
	boost::archive::text_iarchive ia(ss);
	ia >> args;

  // ////cout << "Server just dearchived message from client \n";

}

// std::string OpCreateVarBatchMeta::serializeMsgToClient(uint64_t row_id,
//                                                   int return_value) {
std::string OpCreateVarBatchMeta::serializeMsgToClient(int return_value) {
	stringstream ss;
	boost::archive::text_oarchive oa(ss);
	// oa << row_id;
	oa << return_value;
	//log("the archived message is " + ss.str());


	// ////cout << "Server just serialized message to client \n";
	return ss.str();
}



WaitingType OpCreateVarBatchMeta::UpdateTarget(OpArgs &args, results_t *results) {
    //log("About to update target");

  message_t *incoming_msg = args.data.msg.ptr;

  switch(state){

    case State::start: {
	    add_timing_point(OP_CREATE_VAR_BATCH_START);
	    // //log("About to case start");

	    vector<md_create_var_args> sql_args;
	    // uint64_t row_id;

	    // net::Connect(&peer, incoming_msg->src);

	    peer = args.data.msg.sender;
	    
	    // //log("About to dearchive msg from client");
	    //convert the serialized string back into the args and count pointer
	    deArchiveMsgFromClient(incoming_msg->body, sql_args);    
	    add_timing_point(OP_CREATE_VAR_BATCH_DEARCHIVE_MSG_FROM_CLIENT);

	    // //log("About to create var stub");
	    // int rc = md_create_var_batch_stub(sql_args, row_id);
	    int rc = md_create_var_batch_stub(sql_args);	    
	    add_timing_point(OP_CREATE_VAR_BATCH_MD_CREATE_VAR_BATCH_STUB);

	    // std::string serial_str = serializeMsgToClient(row_id, rc);
	    std::string serial_str = serializeMsgToClient(rc);
	    add_timing_point(OP_CREATE_VAR_BATCH_SERIALIZE_MSG_FOR_CLIENT);

	    createOutgoingMessage(incoming_msg->src, 
	                          0, //Not expecting a reply
	                          incoming_msg->src_mailbox,
	                          serial_str);
	    add_timing_point(OP_CREATE_VAR_BATCH_CREATE_MSG_FOR_CLIENT);

	    net::SendMsg(peer, ldo_msg);
	    state=State::done;
	    add_timing_point(OP_CREATE_VAR_BATCH_SEND_MSG_TO_CLIENT_OP_DONE);
	    return WaitingType::done_and_destroy;
	}
	case State::done:
    	return WaitingType::done_and_destroy;
  	}
}

//WaitingType OpCreateVarBatchMeta::UpdateOrigin(OpArgs &args, results_t *results) {
WaitingType OpCreateVarBatchMeta::UpdateOrigin(OpArgs &, results_t *) {
    //log("error. its asking me to update origin \n");
	return WaitingType::error;

}

// int md_create_var_batch_stub (const vector<md_create_var_args> &args,
//                         uint64_t &row_id)
int md_create_var_batch_stub (const vector<md_create_var_args> &all_args)
{
    int rc;
    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;
    char * ErrMsg = NULL;

    // char *err_msg = 0;
    // char *sql = "SELECT * FROM var_catalog";
    // ////cout << "starting var catalog \n";
    // rc = sqlite3_exec(db, sql, callback, 0, &err_msg);
    // ////cout << "done with var catalog \n";

    rc = sqlite3_exec (db, "begin;", callback, 0, &ErrMsg);
    if (rc != SQLITE_OK)
    {
        fprintf (stderr, "Error begin md_create_var_batch_stub: Line: %d SQL error: %s\n", __LINE__, ErrMsg);
        sqlite3_free (ErrMsg);
        sqlite3_close (db);
        goto cleanup;
    }

    rc = sqlite3_prepare_v2 (db, "insert into var_catalog (id, run_id, timestep_id, name, path, version, data_size, active, txn_id, num_dims, d0_min, d0_max, d1_min, d1_max, d2_min, d2_max) values (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)", -1, &stmt, &tail);
    if (rc != SQLITE_OK)
    {
        fprintf (stderr, "Error start of create_var_batch_stub: Line: %d SQL error: %s\n", __LINE__, sqlite3_errmsg (db));
        sqlite3_close (db);
        goto cleanup;
    }

    for(int arg_indx = 0; arg_indx < all_args.size(); arg_indx++) {
    	md_create_var_args args = all_args.at(arg_indx);
    	// cout << "var_id: " << args.var_id << endl;
    	// cout << "run_id: " << args.run_id << endl;
    	// cout << "timestep_id: " << args.timestep_id << endl;

	    rc = sqlite3_bind_int64 (stmt, 1, args.var_id); assert (rc == SQLITE_OK);  
	    // ////cout << "var id on create: " << args.var_id << endl; 
	    rc = sqlite3_bind_int64 (stmt, 2, args.run_id); assert (rc == SQLITE_OK); 
	    rc = sqlite3_bind_int64 (stmt, 3, args.timestep_id); assert (rc == SQLITE_OK);   
	    rc = sqlite3_bind_text (stmt, 4, strdup(args.name.c_str()), -1, free); assert (rc == SQLITE_OK);
	    rc = sqlite3_bind_text (stmt, 5, strdup(args.path.c_str()), -1, free); assert (rc == SQLITE_OK);
	    rc = sqlite3_bind_int (stmt, 6, args.version); assert (rc == SQLITE_OK);
	    rc = sqlite3_bind_int (stmt, 7, (int)args.data_size); assert (rc == SQLITE_OK);
	    rc = sqlite3_bind_int (stmt, 8, 0); assert (rc == SQLITE_OK);
	    rc = sqlite3_bind_int64 (stmt, 9, args.txn_id); assert (rc == SQLITE_OK);
	    rc = sqlite3_bind_int (stmt, 10, args.num_dims); assert (rc == SQLITE_OK);
	    
    	int i = 0;
	    while( i < args.num_dims)
	    {
	    	////cout << "dim - i: " << i << endl;
	        rc = sqlite3_bind_int64 (stmt, 11 + i * 2, args.dims.at(i).min); assert (rc == SQLITE_OK);
	        rc = sqlite3_bind_int64 (stmt, 12 + i * 2, args.dims.at(i).max); assert (rc == SQLITE_OK);
	        ////cout << args.dims.at(i).min << endl;
	        ////cout << args.dims.at(i).max << endl;

	        i++;
	    }
	    //explicitly bind null to any remaining dims so that there isn't any overflow from the prev insert
	    while(i < 3) {
	    	////cout << "null - i: " << i << endl;
	    	rc = sqlite3_bind_null (stmt, 11 + i * 2); assert (rc == SQLITE_OK);
	    	rc = sqlite3_bind_null (stmt, 12 + i * 2); assert (rc == SQLITE_OK);
	  	    i++;
	    }	    

	    rc = sqlite3_step (stmt); 
	    // ////cout << "rc: " << rc << endl;
        if (rc != SQLITE_OK && rc != SQLITE_DONE)
	    {
	    	// cout << "rc: " << rc << endl;
	        fprintf (stderr, "Error step md_create_var_batch_stub: Line: %d SQL error: %s\n", __LINE__, ErrMsg);
	        sqlite3_free (ErrMsg);
	        sqlite3_close (db);
	        goto cleanup;
	    }
	    assert (rc == SQLITE_OK || rc == SQLITE_DONE);
	    
    	// type_id = (int) sqlite3_last_insert_rowid (db);

		rc = sqlite3_reset(stmt); assert (rc == SQLITE_OK);

	    // row_id = (int) sqlite3_last_insert_rowid (db);
	}
//printf ("generated new global id:%d\n", *row_id);

    rc = sqlite3_finalize (stmt); 
    // if (rc != SQLITE_OK)
    // {
    // 	cout << "rc: " << rc << endl;
   	// }
    assert (rc == SQLITE_OK);

    rc = sqlite3_exec (db, "end;", callback, 0, &ErrMsg);
    if (rc != SQLITE_OK)
    {
    	// cout << "rc: " << rc << endl;
        fprintf (stderr, "Error end query md_create_var_batch_stub: Line: %d SQL error: %s\n", __LINE__, ErrMsg);
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
