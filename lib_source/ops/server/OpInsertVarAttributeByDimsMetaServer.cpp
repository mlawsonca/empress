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


#include "OpInsertVarAttributeByDimsMetaCommon.hh"
#include <server_timing_constants.hh>
#include <sqlite3.h>

extern sqlite3 *db;

using namespace std;

int md_insert_var_attribute_by_dims_stub (const md_insert_var_attribute_by_dims_args &args, uint64_t &attribute_id);

// void OpInsertVarAttributeByDimsMeta::deArchiveMsgFromClient(const std::string &serial_str, md_insert_var_attribute_by_dims_args &args) {

//     //log("the archived message is " + serial_str);

// 	std::stringstream ss;
// 	ss << serial_str;
// 	boost::archive::text_iarchive ia(ss);
// 	ia >> args;
// }

void OpInsertVarAttributeByDimsMeta::deArchiveMsgFromClient(message_t *incoming_msg, md_insert_var_attribute_by_dims_args &args) {

	std::stringstream ss;
	ss.write(incoming_msg->body, incoming_msg->body_len);
	boost::archive::text_iarchive ia(ss);
	ia >> args;
	// cout << "deserialized text string: " << ss.str() << endl;
	// cout << "deserialized text string length: " << ss.str().size() << endl;

}

std::string OpInsertVarAttributeByDimsMeta::serializeMsgToClient(int return_value, uint64_t attribute_id) {

	std::stringstream ss;
	boost::archive::text_oarchive oa(ss);
	oa << return_value;
	//log("the archived message is " + ss.str());

	oa << attribute_id;

	return ss.str();
}



WaitingType OpInsertVarAttributeByDimsMeta::UpdateTarget(OpArgs &args, results_t *results) {
    message_t *incoming_msg = args.data.msg.ptr;
    md_insert_var_attribute_by_dims_args sql_args;
    uint64_t attribute_id;

  switch(state){

    case State::start: {
	    add_timing_point(OP_INSERT_VAR_ATTRIBUTE_BY_DIMS_START);


	    peer = args.data.msg.sender;
	    

	    deArchiveMsgFromClient(incoming_msg, sql_args);
	    add_timing_point(OP_INSERT_VAR_ATTRIBUTE_BY_DIMS_DEARCHIVE_MSG_FROM_CLIENT);

	    int rc = md_insert_var_attribute_by_dims_stub(sql_args, attribute_id);
	    add_timing_point(OP_INSERT_VAR_ATTRIBUTE_BY_DIMS_MD_INSERT_VAR_ATTRIBUTE_BY_DIMS_STUB);

	    // cout << "according to the server the attribute id is still " << attribute_id <<endl;
	    std::string serial_str = serializeMsgToClient(rc, attribute_id);
	    add_timing_point(OP_INSERT_VAR_ATTRIBUTE_BY_DIMS_SERIALIZE_MSG_FOR_CLIENT);

	    createOutgoingMessage(incoming_msg->src, 
	                          0, //Not expecting a reply
	                          incoming_msg->src_mailbox,
	                          serial_str);
	    add_timing_point(OP_INSERT_VAR_ATTRIBUTE_BY_DIMS_CREATE_MSG_FOR_CLIENT);

	    net::SendMsg(peer, ldo_msg);
	    state=State::done;
	    add_timing_point(OP_INSERT_VAR_ATTRIBUTE_BY_DIMS_SEND_MSG_TO_CLIENT_OP_DONE);
	    return WaitingType::done_and_destroy;
  	}
	case State::done:
    	return WaitingType::done_and_destroy;
  	}
}

//WaitingType OpInsertVarAttributeByDimsMeta::UpdateOrigin(OpArgs &args, results_t *results) {
WaitingType OpInsertVarAttributeByDimsMeta::UpdateOrigin(OpArgs &, results_t *) {
  
	return WaitingType::error;

}

int md_insert_var_attribute_by_dims_stub (const md_insert_var_attribute_by_dims_args &args,
                          uint64_t &attribute_id)
{
    int rc;
    int i = 0;
    // int i;
    // char * ErrMsg = NULL;
    sqlite3_stmt * stmt = NULL;
    const char * tail_index = NULL;

    // int rowid;

    rc = sqlite3_prepare_v2 (db, "insert into var_attribute_catalog (id, timestep_id, type_id, var_id, active, txn_id, num_dims, d0_min, d0_max, d1_min, d1_max, d2_min, d2_max, data_type, data) values (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)", -1, &stmt, &tail_index);
    if (rc != SQLITE_OK)
    {
        fprintf (stderr, "Error at start of insert_var_attribute_stub: Line: %d SQL error: %s\n", __LINE__, sqlite3_errmsg (db));
        sqlite3_close (db);
        goto cleanup;
    }

    rc = sqlite3_bind_null (stmt, 1); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 2, args.timestep_id); assert (rc == SQLITE_OK);   
    rc = sqlite3_bind_int64 (stmt, 3, args.type_id); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 4, args.var_id); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int (stmt, 5, 0); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 6, args.txn_id); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int (stmt, 7, args.num_dims); assert (rc == SQLITE_OK);
    while(i < args.num_dims) {
        rc = sqlite3_bind_int64 (stmt, 8 + i * 2, args.dims.at(i).min); assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int64 (stmt, 9 + i * 2, args.dims.at(i).max); assert (rc == SQLITE_OK);
        i++;
    }
    rc = sqlite3_bind_int (stmt, 14, args.data_type); assert (rc == SQLITE_OK);
    switch(args.data_type) {
        case ATTR_DATA_TYPE_NULL : {
            rc = sqlite3_bind_null (stmt, 15); 
            break;
        }        
        case ATTR_DATA_TYPE_INT : {
            uint64_t deserialized_int;
        	stringstream sso;
        	sso << args.data;
        	boost::archive::text_iarchive ia(sso);
        	// cout << "serialized int data on insertion: " << args.data << endl;
            ia >> deserialized_int;
            // cout << "deserialized_int: " << deserialized_int << endl;
            rc = sqlite3_bind_int64 (stmt, 15, deserialized_int); 
            break;
        }
        case ATTR_DATA_TYPE_REAL : {
            long double deserialized_real;
        	stringstream sso;
        	sso << args.data;
        	boost::archive::text_iarchive ia(sso);
            ia >> deserialized_real;
            // cout << "deserialized_real: " << deserialized_real << endl;
            rc = sqlite3_bind_double (stmt, 15, deserialized_real); 
            break;
        }
        case ATTR_DATA_TYPE_TEXT : {
            // cout << "text: " << args.data.c_str() << endl;
            rc = sqlite3_bind_text (stmt, 15, strdup(args.data.c_str()), -1, free);
            break;
        }
        case ATTR_DATA_TYPE_BLOB : {
            // cout << "blob: " << args.data.c_str() << endl;
            // cout << "blob: " << args.data << " str size: " << args.data.size() << endl;
            // rc = sqlite3_bind_text (stmt, 15, strdup(args.data.c_str()), args.data.size()+1, free);
            rc = sqlite3_bind_text (stmt, 15, args.data.c_str(), args.data.size()+1, free);

            // rc = sqlite3_bind_blob64 (stmt, 15, args.data, -1, free); 
            break;
        }
    }
    assert (rc == SQLITE_OK);

    rc = sqlite3_step (stmt); assert (rc == SQLITE_OK || rc == SQLITE_DONE);


    attribute_id = (int) sqlite3_last_insert_rowid (db);
    // std::cout << " According to the server, the attr id is " << attribute_id << std::endl;

    rc = sqlite3_finalize (stmt);


cleanup:

    return rc;
}