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


#include "OpCatalogAllRunAttributesMetaCommon.hh"
#include <server_timing_constants.hh>
#include <sqlite3.h>

extern sqlite3 *db;

using namespace std;

int md_catalog_all_run_attributes_stub (const md_catalog_all_run_attributes_args &args,
                           std::vector<md_catalog_run_attribute_entry> &attribute_list,
                           uint32_t &count);

static int get_matching_attribute_count (const md_catalog_all_run_attributes_args &args, uint32_t &count);


void OpCatalogAllRunAttributesMeta::deArchiveMsgFromClient(const std::string &serial_str, 
                                             md_catalog_all_run_attributes_args &args) {

    //log("the archived message is " + serial_str);

	std::stringstream ss;
	ss << serial_str;
	boost::archive::text_iarchive ia(ss);
	ia >> args;
}

std::string OpCatalogAllRunAttributesMeta::serializeMsgToClient(const std::vector<md_catalog_run_attribute_entry> &attribute_list,
                                                    uint32_t count,
                                                    int return_value) {

	std::stringstream ss;
	boost::archive::text_oarchive oa(ss);
	oa << count;
	if (count > 0) {
	oa << attribute_list;
	}
	oa << return_value;
	//log("the archived message is " + ss.str());


	return ss.str();
}



WaitingType OpCatalogAllRunAttributesMeta::UpdateTarget(OpArgs &args, results_t *results) {
  message_t *incoming_msg = args.data.msg.ptr;

  switch(state){

    case State::start: {
	    add_timing_point(OP_CATALOG_ALL_RUN_ATTRIBUTES_START);
	    md_catalog_all_run_attributes_args sql_args;
	    std::vector<md_catalog_run_attribute_entry> attribute_list;
	    uint32_t count;
	    
	    peer = args.data.msg.sender;
	    

	    deArchiveMsgFromClient(incoming_msg->body, sql_args);
	    add_timing_point(OP_CATALOG_ALL_RUN_ATTRIBUTES_DEARCHIVE_MSG_FROM_CLIENT);

	    int rc = md_catalog_all_run_attributes_stub(sql_args, attribute_list, count);
	    add_timing_point(OP_CATALOG_ALL_RUN_ATTRIBUTES_MD_CATALOG_ALL_RUN_ATTRIBUTES_STUB);


	    std::string serial_str = serializeMsgToClient(attribute_list, count, rc);
	    // cout << "in catalog all, attr list is of size: " << attribute_list.size() << endl;
	    add_timing_point(OP_CATALOG_ALL_RUN_ATTRIBUTES_SERIALIZE_MSG_FOR_CLIENT);

	    createOutgoingMessage(incoming_msg->src, 
	                          0, //Not expecting a reply
	                          incoming_msg->src_mailbox,
	                          serial_str);
	    add_timing_point(OP_CATALOG_ALL_RUN_ATTRIBUTES_CREATE_MSG_FOR_CLIENT);

	    net::SendMsg(peer, ldo_msg);
	    state=State::done;
	    add_timing_point(OP_CATALOG_ALL_RUN_ATTRIBUTES_SEND_MSG_TO_CLIENT_OP_DONE);
	    return WaitingType::done_and_destroy;
  	}
	case State::done:
		return WaitingType::done_and_destroy;
	}
}

//WaitingType OpCatalogAllRunAttributesMeta::UpdateOrigin(OpArgs &args, results_t *results) {
WaitingType OpCatalogAllRunAttributesMeta::UpdateOrigin(OpArgs &, results_t *) {
  
	return WaitingType::error;

}


int md_catalog_all_run_attributes_stub (const md_catalog_all_run_attributes_args &args,
                           std::vector<md_catalog_run_attribute_entry> &attribute_list,
                           uint32_t &count)
{
    int rc;
    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;

    const char * query = "select rac.* from run_attribute_catalog rac "
    "inner join type_catalog tc on rac.type_id = tc.id "
    "where (rac.txn_id = ? or rac.active = 1) "
    "and tc.run_id = ? ";
    // }

    rc = get_matching_attribute_count (args, count); assert (rc == RC_OK);

    if (count > 0) {
        attribute_list.reserve(count);


        rc = sqlite3_prepare_v2 (db, query, -1, &stmt, &tail); 
        if (rc != SQLITE_OK)
        {
            fprintf (stderr, "Error catalog_all_run_attributes_stub: Line: %d SQL error (%d): %s\n", __LINE__, rc, sqlite3_errmsg (db));
            sqlite3_close (db);
            goto cleanup;
        }
        rc = sqlite3_bind_int64 (stmt, 1, args.txn_id); assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int64 (stmt, 2, args.run_id); assert (rc == SQLITE_OK);

        rc = sqlite3_step (stmt); assert (rc == SQLITE_OK || rc == SQLITE_ROW);

        // cout << "starting \n";
        while (rc == SQLITE_ROW)
        {
            md_catalog_run_attribute_entry attribute;

            attribute.attribute_id = sqlite3_column_int64 (stmt, 0);
            attribute.type_id = sqlite3_column_int64 (stmt, 1);
            attribute.active = sqlite3_column_int (stmt, 2);
            attribute.txn_id = sqlite3_column_int64 (stmt, 3);    
            attribute.data_type = (attr_data_type) sqlite3_column_int (stmt, 4);
            // cout << "in server, data type: " << attribute.data_type  << endl;
            switch(attribute.data_type) {
                // case ATTR_DATA_TYPE_NULL : {
                //     break;
                // }        

                case ATTR_DATA_TYPE_INT : {
                    stringstream ss;
                    boost::archive::text_oarchive oa(ss);
                    oa << (uint64_t)sqlite3_column_int64 (stmt, 5);
                    attribute.data = ss.str();
                    break;
                }
                case ATTR_DATA_TYPE_REAL : {
                    stringstream ss;
                    boost::archive::text_oarchive oa(ss);                    
                    oa << (long double)sqlite3_column_double (stmt, 5);
                    attribute.data = ss.str();
                    break;
                }
                case ATTR_DATA_TYPE_TEXT : {
                    attribute.data = (char *)sqlite3_column_text (stmt, 5);
                    break;
                }
                case ATTR_DATA_TYPE_BLOB : {
        			int size_blob = sqlite3_column_bytes (stmt, 5);
        			
            		attribute.data.assign((char *)sqlite3_column_text (stmt, 5),(char *)sqlite3_column_text (stmt, 5) + size_blob);
                    // attribute.data = (char *)sqlite3_column_text (stmt, 5);
                    // cout << "run attr in server data str length: " << attribute.data.size() << " str: " << attribute.data << endl;

                    break;
                }
            }            

            // attribute.data = (char *) sqlite3_column_text (stmt, 5);
            // cout << "attr.dims.at(2).min: " << attribute.dims.at(2).min << " attr.dims.at(2).min: " << attribute.dims.at(2).max << endl;

            rc = sqlite3_step (stmt);
            attribute_list.push_back(attribute);
        }

        rc = sqlite3_finalize (stmt);  
    }
        

cleanup:

    return rc;
}



static int get_matching_attribute_count (const md_catalog_all_run_attributes_args &args, uint32_t &count)
{
    int rc;
    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;
    const char * query = "select count (*) from run_attribute_catalog rac "
    "inner join type_catalog tc on rac.type_id = tc.id "
    "where (rac.txn_id = ? or rac.active = 1) "
    "and tc.run_id = ? ";

    rc = sqlite3_prepare_v2 (db, query, -1, &stmt, &tail); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 1, args.txn_id); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 2, args.run_id); assert (rc == SQLITE_OK);
        
    rc = sqlite3_step (stmt); assert (rc == SQLITE_OK || rc == SQLITE_ROW);
    count = sqlite3_column_int64 (stmt, 0);
    // cout << "in catalog all attrs, count: " << count << endl;
    rc = sqlite3_finalize (stmt); assert (rc == SQLITE_OK);

    return rc;
}
