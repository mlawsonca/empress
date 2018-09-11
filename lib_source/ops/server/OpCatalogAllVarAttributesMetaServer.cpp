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


#include "OpCatalogAllVarAttributesMetaCommon.hh"
#include <server_timing_constants.hh>
#include <sqlite3.h>

extern sqlite3 *db;

using namespace std;

int md_catalog_all_var_attributes_stub (const md_catalog_all_var_attributes_args &args,
						   std::vector<md_catalog_var_attribute_entry> &attribute_list,
						   uint32_t &count);

static int get_matching_attribute_count (const md_catalog_all_var_attributes_args &args, uint32_t &count);


void OpCatalogAllVarAttributesMeta::deArchiveMsgFromClient(const std::string &serial_str, 
											 md_catalog_all_var_attributes_args &args) {

	//log("the archived message is " + serial_str);

	std::stringstream ss;
	ss << serial_str;
	boost::archive::text_iarchive ia(ss);
	ia >> args;
}

std::string OpCatalogAllVarAttributesMeta::serializeMsgToClient(const std::vector<md_catalog_var_attribute_entry> &attribute_list,
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



WaitingType OpCatalogAllVarAttributesMeta::UpdateTarget(OpArgs &args, results_t *results) {
  message_t *incoming_msg = args.data.msg.ptr;

  switch(state){

	case State::start: {
		add_timing_point(OP_CATALOG_ALL_VAR_ATTRIBUTES_START);
		md_catalog_all_var_attributes_args sql_args;
		std::vector<md_catalog_var_attribute_entry> attribute_list;
		uint32_t count;
		
		peer = args.data.msg.sender;
		

		deArchiveMsgFromClient(incoming_msg->body, sql_args);
		add_timing_point(OP_CATALOG_ALL_VAR_ATTRIBUTES_DEARCHIVE_MSG_FROM_CLIENT);

		int rc = md_catalog_all_var_attributes_stub(sql_args, attribute_list, count);
		add_timing_point(OP_CATALOG_ALL_VAR_ATTRIBUTES_MD_CATALOG_ALL_VAR_ATTRIBUTES_STUB);


		std::string serial_str = serializeMsgToClient(attribute_list, count, rc);
		// cout << "in catalog all, attr list is of size: " << attribute_list.size() << endl;
		add_timing_point(OP_CATALOG_ALL_VAR_ATTRIBUTES_SERIALIZE_MSG_FOR_CLIENT);

		createOutgoingMessage(incoming_msg->src, 
							  0, //Not expecting a reply
							  incoming_msg->src_mailbox,
							  serial_str);
		add_timing_point(OP_CATALOG_ALL_VAR_ATTRIBUTES_CREATE_MSG_FOR_CLIENT);

		net::SendMsg(peer, ldo_msg);
		state=State::done;
		add_timing_point(OP_CATALOG_ALL_VAR_ATTRIBUTES_SEND_MSG_TO_CLIENT_OP_DONE);
		return WaitingType::done_and_destroy;
	}
	case State::done:
		return WaitingType::done_and_destroy;
	}
}

//WaitingType OpCatalogAllVarAttributesMeta::UpdateOrigin(OpArgs &args, results_t *results) {
WaitingType OpCatalogAllVarAttributesMeta::UpdateOrigin(OpArgs &, results_t *) {
  
	return WaitingType::error;

}

// extern int callback (void * NotUsed, int argc, char ** argv, char ** ColName);


int md_catalog_all_var_attributes_stub (const md_catalog_all_var_attributes_args &args,
						   std::vector<md_catalog_var_attribute_entry> &attribute_list,
						   uint32_t &count)
{
	int rc;
	sqlite3_stmt * stmt = NULL;
	const char * tail = NULL;

	const char * query = "select vac.* from var_attribute_catalog vac "
	"inner join type_catalog tc on vac.type_id = tc.id "
	"where (vac.txn_id = ? or vac.active = 1) "
	"and tc.run_id = ? "
	"and vac.timestep_id = ? ";

	// char *err_msg = 0;
	// char *sql = "SELECT * FROM var_attribute_catalog";
	// fprintf(stdout, "Starting attr operation\n");
	// rc = sqlite3_exec(db, sql, callback, 0, &err_msg);
	// if (rc == SQLITE_OK) {
	//     fprintf(stdout, "Attr Operation done successfully\n");
	// }

	// err_msg = 0;
	// sql = "SELECT * FROM type_catalog";
	// rc = sqlite3_exec(db, sql, callback, 0, &err_msg);
	// if (rc == SQLITE_OK) {
	//     fprintf(stdout, "Type Operation done successfully\n");
	// }

	rc = get_matching_attribute_count (args, count); assert (rc == RC_OK);

	if (count > 0) {
		attribute_list.reserve(count);


		rc = sqlite3_prepare_v2 (db, query, -1, &stmt, &tail); 
		if (rc != SQLITE_OK)
		{
			fprintf (stderr, "Error catalog_all_var_attributes stub: Line: %d SQL error (%d): %s\n", __LINE__, rc, sqlite3_errmsg (db));
			sqlite3_close (db);
			goto cleanup;
		}
		rc = sqlite3_bind_int64 (stmt, 1, args.txn_id); assert (rc == SQLITE_OK);
		rc = sqlite3_bind_int64 (stmt, 2, args.run_id); assert (rc == SQLITE_OK);
		rc = sqlite3_bind_int64 (stmt, 3, args.timestep_id); assert (rc == SQLITE_OK);

		rc = sqlite3_step (stmt); assert (rc == SQLITE_OK || rc == SQLITE_ROW);

		// cout << "starting \n";
		while (rc == SQLITE_ROW)
		{
			md_catalog_var_attribute_entry attribute;

			attribute.attribute_id = sqlite3_column_int64 (stmt, 0);
			// cout << "attr id: " << attribute.attribute_id << endl;
			// attribute.timestep_id = args.timestep_id;
			attribute.timestep_id = sqlite3_column_int64 (stmt, 1);
			attribute.type_id = sqlite3_column_int64 (stmt, 2);
			attribute.var_id = sqlite3_column_int64 (stmt, 3);
			attribute.active = sqlite3_column_int (stmt, 4);
			attribute.txn_id = sqlite3_column_int64 (stmt, 5);            
			attribute.num_dims = sqlite3_column_int (stmt, 6);
			attribute.dims.reserve(3);
			// attribute.dims.reserve(attribute.num_dims);
			int j = 0;
			// while (j < attribute.num_dims)
			while (j < 3)
			{
				md_dim_bounds bounds;
				bounds.min =  sqlite3_column_int64 (stmt, 7 + (j * 2));
				bounds.max = sqlite3_column_int64 (stmt, 8 + (j * 2));
				attribute.dims.push_back(bounds);
				j++;
			}
			attribute.data_type = (attr_data_type) sqlite3_column_int (stmt, 13);
			switch(attribute.data_type) {
				// case ATTR_DATA_TYPE_NULL : {
				//     break;
				// }        

				case ATTR_DATA_TYPE_INT : {
					stringstream ss;
				    boost::archive::text_oarchive oa(ss);
					oa << (uint64_t)sqlite3_column_int64 (stmt, 14);
					attribute.data = ss.str();
        			// cout << "serialized int data on catalog: " << attribute.data << endl;
					break;
				}
				case ATTR_DATA_TYPE_REAL : {
					stringstream ss;
				    boost::archive::text_oarchive oa(ss);
					oa << (long double)sqlite3_column_double (stmt, 14);
					attribute.data = ss.str();
					break;
				}
				case ATTR_DATA_TYPE_TEXT : {
					attribute.data = (char *)sqlite3_column_text (stmt, 14);
					break;
				}
                case ATTR_DATA_TYPE_BLOB : {
        			int size_blob = sqlite3_column_bytes (stmt, 14);
        			
            		attribute.data.assign((char *)sqlite3_column_text (stmt, 14),(char *)sqlite3_column_text (stmt, 14) + size_blob);
            		// cout << "var attr in server data str length: " << attribute.data.size() << " str: " << attribute.data << endl;

                    // attribute.data = (char *)sqlite3_column_text (stmt, 14);
                    break;
                }
			}                        
			// attribute.data = (char *) sqlite3_column_text (stmt, 14);
			// cout << "attr.dims.at(2).min: " << attribute.dims.at(2).min << " attr.dims.at(2).min: " << attribute.dims.at(2).max << endl;


			rc = sqlite3_step (stmt);
			attribute_list.push_back(attribute);
		}

		rc = sqlite3_finalize (stmt);  
	}
		

cleanup:

	return rc;
}



static int get_matching_attribute_count (const md_catalog_all_var_attributes_args &args, uint32_t &count)
{
	int rc;
	sqlite3_stmt * stmt = NULL;
	const char * tail = NULL;
	const char * query = "select count (*) from var_attribute_catalog vac "
	"inner join type_catalog tc on vac.type_id = tc.id "
	"where (vac.txn_id = ? or vac.active = 1) "
	"and tc.run_id = ? "
	"and vac.timestep_id = ? ";

	rc = sqlite3_prepare_v2 (db, query, -1, &stmt, &tail); assert (rc == SQLITE_OK);
	rc = sqlite3_bind_int64 (stmt, 1, args.txn_id); assert (rc == SQLITE_OK);
	rc = sqlite3_bind_int64 (stmt, 2, args.run_id); assert (rc == SQLITE_OK);
	rc = sqlite3_bind_int64 (stmt, 3, args.timestep_id); assert (rc == SQLITE_OK);
		
	rc = sqlite3_step (stmt); assert (rc == SQLITE_OK || rc == SQLITE_ROW);
	count = sqlite3_column_int64 (stmt, 0);
	// cout << "in catalog all attrs, count: " << count << endl;
	rc = sqlite3_finalize (stmt); assert (rc == SQLITE_OK);

	return rc;
}
