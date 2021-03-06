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


#include "OpCatalogAllTypesWithVarAttributesWithVarSubstrInTimestepMetaCommon.hh"
#include <server_timing_constants.hh>
#include <sqlite3.h>

extern sqlite3 *db;

using namespace std;

int md_catalog_all_types_with_var_attributes_with_var_substr_in_timestep_stub (md_catalog_all_types_with_var_attributes_with_var_substr_in_timestep_args &args, 
                          std::vector<md_catalog_type_entry> &entries,
                          uint32_t &count);


void OpCatalogAllTypesWithVarAttributesWithVarSubstrInTimestepMeta::deArchiveMsgFromClient(const std::string &serial_str, md_catalog_all_types_with_var_attributes_with_var_substr_in_timestep_args &args) {

    //log("the archived message is " + serial_str);

	std::stringstream ss;
	ss << serial_str;
	boost::archive::text_iarchive ia(ss);
	ia >> args;

}

std::string OpCatalogAllTypesWithVarAttributesWithVarSubstrInTimestepMeta::serializeMsgToClient(const std::vector<md_catalog_type_entry> &entries,
                                                    uint32_t count,
                                                    int return_value) {

	// cout << "in OpCatalogAllTypesWithVarAttributesWithVarSubstrInTimestepMeta, count: " << count << endl;
	// cout << "in OpCatalogAllTypesWithVarAttributesWithVarSubstrInTimestepMeta, attribute_list.size(): " << entries.size() << endl;
	std::stringstream ss;
	boost::archive::text_oarchive oa(ss);
	oa << count;
	if(count > 0) {
		oa << entries;
	}
	// std::// cout << "server still thinks entries size is " << entries.size() << std::endl;
	oa << return_value;
	//log("the archived message is " + ss.str());


	return ss.str();
}

WaitingType OpCatalogAllTypesWithVarAttributesWithVarSubstrInTimestepMeta::UpdateTarget(OpArgs &args, results_t *results) {
  message_t *incoming_msg = args.data.msg.ptr;

  switch(state){

    case State::start: {
	    add_timing_point(OP_CATALOG_ALL_TYPES_WITH_VAR_ATTRIBUTES_WITH_VAR_SUBSTR_IN_TIMESTEP_START);
	    md_catalog_all_types_with_var_attributes_with_var_substr_in_timestep_args sql_args;
	    std::vector<md_catalog_type_entry> entries;
	    uint32_t count;

	    peer = args.data.msg.sender;


	    //convert the serialized string back into the args and count pointer
	    deArchiveMsgFromClient(incoming_msg->body, sql_args);    
	    add_timing_point(OP_CATALOG_ALL_TYPES_WITH_VAR_ATTRIBUTES_WITH_VAR_SUBSTR_IN_TIMESTEP_DEARCHIVE_MSG_FROM_CLIENT);

	    int rc = md_catalog_all_types_with_var_attributes_with_var_substr_in_timestep_stub(sql_args, entries, count);
	    add_timing_point(OP_CATALOG_ALL_TYPES_WITH_VAR_ATTRIBUTES_WITH_VAR_SUBSTR_IN_TIMESTEP_MD_CATALOG_ALL_TYPES_WITH_VAR_ATTRIBUTES_WITH_VAR_SUBSTR_IN_TIMESTEP_STUB);

	    // std::// cout << "num of entries is " << entries.size() << std::endl;

	    std::string serial_str = serializeMsgToClient(entries, count, rc);
	    add_timing_point(OP_CATALOG_ALL_TYPES_WITH_VAR_ATTRIBUTES_WITH_VAR_SUBSTR_IN_TIMESTEP_SERIALIZE_MSG_FOR_CLIENT);

	    createOutgoingMessage(incoming_msg->src, 
	                          0, //Not expecting a reply
	                          incoming_msg->src_mailbox,
	                          serial_str);
	    add_timing_point(OP_CATALOG_ALL_TYPES_WITH_VAR_ATTRIBUTES_WITH_VAR_SUBSTR_IN_TIMESTEP_CREATE_MSG_FOR_CLIENT);
	    
	    net::SendMsg(peer, ldo_msg);
	    state=State::done;
	    add_timing_point(OP_CATALOG_ALL_TYPES_WITH_VAR_ATTRIBUTES_WITH_VAR_SUBSTR_IN_TIMESTEP_SEND_MSG_TO_CLIENT_OP_DONE);
	    return WaitingType::done_and_destroy;
	}
	case State::done:
    	return WaitingType::done_and_destroy;
  	}
}
//WaitingType OpCatalogAllTypesWithVarAttributesWithVarSubstrInTimestepMeta::UpdateOrigin(OpArgs &args, results_t *results) {
WaitingType OpCatalogAllTypesWithVarAttributesWithVarSubstrInTimestepMeta::UpdateOrigin(OpArgs &, results_t *) {
  
	return WaitingType::error;

}

// extern int callback (void * NotUsed, int argc, char ** argv, char ** ColName);

int md_catalog_all_types_with_var_attributes_with_var_substr_in_timestep_stub (md_catalog_all_types_with_var_attributes_with_var_substr_in_timestep_args &args, 
                          std::vector<md_catalog_type_entry> &entries,
                          uint32_t &count)
{
    int rc;
    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;
    const char * query = "select tc.* from type_catalog tc "
      "inner join var_attribute_catalog vac on tc.id = vac.type_id "
      "inner join var_catalog vc on vac.timestep_id = vc.timestep_id and vac.var_id = vc.id "
      "where (vac.txn_id = ? or vac.active = 1) "
      "and (tc.txn_id = ? or tc.active = 1) "
      "and tc.run_id = ? "
      "and vac.timestep_id = ? "
      "and vc.name like ? "
      "group by tc.id";

    size_t size = 0;

    rc = sqlite3_prepare_v2 (db, "select count(*) from (select distinct tc.id from type_catalog tc "
      "inner join var_attribute_catalog vac on tc.id = vac.type_id "
      "inner join var_catalog vc on vac.timestep_id = vc.timestep_id and vac.var_id = vc.id "
      "where (vac.txn_id = ? or vac.active = 1) "
      "and (tc.txn_id = ? or tc.active = 1) "
      "and tc.run_id = ? "
      "and vac.timestep_id = ? "
      "and vc.name like ? ) as internalQuery", -1, &stmt, &tail); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 1, args.txn_id); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 2, args.txn_id); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 3, args.run_id); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 4, args.timestep_id); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_text (stmt, 5, strdup(args.var_name_substr.c_str()), -1, free); assert (rc == SQLITE_OK);

    rc = sqlite3_step (stmt); assert (rc == SQLITE_OK || rc == SQLITE_ROW);
    count = sqlite3_column_int64 (stmt, 0);
    rc = sqlite3_finalize (stmt); assert (rc == SQLITE_OK);

    // char *err_msg = 0;
    // char *sql = "SELECT * FROM type_catalog";
    // cout << "starting type catalog \n";
    // rc = sqlite3_exec(db, sql, callback, 0, &err_msg);
    // cout << "done with type catalog \n";

    if (count > 0) {
        entries.reserve(count);

        rc = sqlite3_prepare_v2 (db, query, -1, &stmt, &tail); assert (rc == SQLITE_OK);
        if (rc != SQLITE_OK)
        {
            fprintf (stderr, "Error md_catalog_all_types_with_var_attributes_with_var_substr_in_timestep_stub: Line: %d SQL error (%d): %s\n", __LINE__, rc, sqlite3_errmsg (db));
            sqlite3_close (db);
            goto cleanup;
        }
        rc = sqlite3_bind_int64 (stmt, 1, args.txn_id); assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int64 (stmt, 2, args.txn_id); assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int64 (stmt, 3, args.run_id); assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int64 (stmt, 4, args.timestep_id); assert (rc == SQLITE_OK);
    	rc = sqlite3_bind_text (stmt, 5, strdup(args.var_name_substr.c_str()), -1, free); assert (rc == SQLITE_OK);


        rc = sqlite3_step (stmt); assert (rc == SQLITE_ROW || rc == SQLITE_OK);

        // std::// cout << "Server thinks there are " << count << " types \n";

        while (rc == SQLITE_ROW)
        {
            md_catalog_type_entry entry;


            entry.type_id = sqlite3_column_int64 (stmt, 0);
            entry.run_id = sqlite3_column_int64 (stmt, 1);
            entry.name = (char *)sqlite3_column_text (stmt, 2);
            entry.version = sqlite3_column_int (stmt, 3);
            entry.active = sqlite3_column_int (stmt, 4);
            entry.txn_id = sqlite3_column_int64 (stmt, 5);

    //printf ("id: %d name: %s path: %s version: %d num_dims: %d ", id, entry [i].name, entry [i].path, entry [i].version, entry [i].num_dims);
            // std::// cout << "in md_catalog_all_types_with_var_attributes_with_var_substr_in_timestep_stub the given entry has name " << entry.name << std::endl;
    //printf ("\n");

            rc = sqlite3_step (stmt);
            entries.push_back(entry);

        }

        rc = sqlite3_finalize (stmt);   
    }
//printf ("rows in var_catalog: %d\n", count);
       

cleanup:

    return rc;
}
