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


#include "OpDeleteAllVarsWithSubstrMetaCommon.hh"
#include <server_timing_constants.hh>
#include <sqlite3.h>

extern sqlite3 *db;

using namespace std;

int md_delete_all_vars_with_substr_stub (const md_delete_all_vars_with_substr_args &args);
extern int callback (void * NotUsed, int argc, char ** argv, char ** ColName);


void OpDeleteAllVarsWithSubstrMeta::deArchiveMsgFromClient(const std::string &serial_str, md_delete_all_vars_with_substr_args &args) {

    //log("the archived message is " + serial_str);

	std::stringstream ss;
	ss << serial_str;
	boost::archive::text_iarchive ia(ss);
	ia >> args;
}

std::string OpDeleteAllVarsWithSubstrMeta::serializeMsgToClient(int return_value) {

	std::stringstream ss;
	boost::archive::text_oarchive oa(ss);
	oa << return_value;
	//log("the archived message is " + ss.str());


	return ss.str();
}


WaitingType OpDeleteAllVarsWithSubstrMeta::UpdateTarget(OpArgs &args, results_t *results) {
  message_t *incoming_msg = args.data.msg.ptr;

  switch(state){

    case State::start: {
	    add_timing_point(OP_DELETE_ALL_VARS_WITH_SUBSTR_START);
	    md_delete_all_vars_with_substr_args sql_args;

	    peer = args.data.msg.sender;

	    deArchiveMsgFromClient(incoming_msg->body, sql_args);    
	    add_timing_point(OP_DELETE_ALL_VARS_WITH_SUBSTR_DEARCHIVE_MSG_FROM_CLIENT);

	    int rc = md_delete_all_vars_with_substr_stub(sql_args);
	    add_timing_point(OP_DELETE_ALL_VARS_WITH_SUBSTR_MD_DELETE_ALL_VARS_WITH_SUBSTR_STUB);

	    std::string serial_str = serializeMsgToClient(rc);
	    add_timing_point(OP_DELETE_ALL_VARS_WITH_SUBSTR_SERIALIZE_MSG_FOR_CLIENT);

	    createOutgoingMessage(incoming_msg->src, 
	                          0, //Not expecting a reply
	                          incoming_msg->src_mailbox,
	                          serial_str);
	    add_timing_point(OP_DELETE_ALL_VARS_WITH_SUBSTR_CREATE_MSG_FOR_CLIENT);
	 
	    net::SendMsg(peer, ldo_msg);
	    state=State::done;
	    add_timing_point(OP_DELETE_ALL_VARS_WITH_SUBSTR_SEND_MSG_TO_CLIENT_OP_DONE);
	    return WaitingType::done_and_destroy;
  	}
	case State::done:
    	return WaitingType::done_and_destroy;
  	}
}

//WaitingType OpDeleteAllVarsWithSubstrMeta::UpdateOrigin(OpArgs &args, results_t *results) {
WaitingType OpDeleteAllVarsWithSubstrMeta::UpdateOrigin(OpArgs &, results_t *) {
  
	return WaitingType::error;

}


int md_delete_all_vars_with_substr_stub (const md_delete_all_vars_with_substr_args &args)
{
    int rc = RC_OK;
    char * ErrMsg = NULL;
    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;

    const char * query1 = "delete from var_attribute_catalog where var_attribute_catalog.id in (select vac.id from var_attribute_catalog vac "
    "inner join type_catalog tc on vac.type_id = tc.id "
    "inner join var_catalog vc on vac.var_id = vc.id and vac.timestep_id = vc.timestep_id and tc.run_id = vc.run_id "
    "where vc.run_id = ? "
    "and vc.timestep_id = ? "
    "and vc.name like ? ) ";

    const char * query2 = "delete from var_catalog where "
    "run_id = ? "
    "and timestep_id = ? "
    "and name like ? ";


    // cout << "deleting var where run_id: " << args.run_id << " timestep_id: " << args.timestep_id << " name: " << 
    //     args.name << " path: " << args.path << " ver: " << args.version << endl;

    rc = sqlite3_exec (db, "begin;", callback, 0, &ErrMsg);
    if (rc != SQLITE_OK)
    {
        fprintf (stderr, "Error begin delete_all_vars_with_substr_stub Line: %d SQL error: %s\n", __LINE__, sqlite3_errmsg (db));
        sqlite3_free (ErrMsg);
        sqlite3_close (db);
        goto cleanup;
    }

    rc = sqlite3_prepare_v2 (db, query1, -1, &stmt, &tail);
    if (rc != SQLITE_OK)
    {
        fprintf (stderr, "Error first query delete_all_vars_with_substr_stub: Line: %d SQL error: %s\n", __LINE__, ErrMsg);
        sqlite3_close (db);
        goto cleanup;
    }

    rc = sqlite3_bind_int64 (stmt, 1, args.run_id); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 2, args.timestep_id); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_text (stmt, 3, strdup(args.var_name_substr.c_str()), -1, free); assert (rc == SQLITE_OK);
    // cout << "args.var_name_substr: " << args.var_name_substr << endl;

    rc = sqlite3_step (stmt); assert (rc == SQLITE_OK || rc == SQLITE_DONE);

    rc = sqlite3_finalize (stmt);

    rc = sqlite3_prepare_v2 (db, query2, -1, &stmt, &tail);
    if (rc != SQLITE_OK)
    {
        fprintf (stderr, "Error second query delete_all_vars_with_substr_stub: Line: %d SQL error: %s\n", __LINE__, sqlite3_errmsg (db));
        sqlite3_close (db);
        goto cleanup;
    }

    rc = sqlite3_bind_int64 (stmt, 1, args.run_id); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 2, args.timestep_id); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_text (stmt, 3, strdup(args.var_name_substr.c_str()), -1, free); assert (rc == SQLITE_OK);
    
    rc = sqlite3_step (stmt); assert (rc == SQLITE_OK || rc == SQLITE_DONE);

    rc = sqlite3_finalize (stmt);

    rc = sqlite3_exec (db, "end;", callback, 0, &ErrMsg);
    if (rc != SQLITE_OK)
    {
        fprintf (stderr, "Error end delete_all_vars_with_substr_stub: Line: %d SQL error: %s\n", __LINE__, ErrMsg);
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

