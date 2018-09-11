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


#include "OpCatalogAllTimestepsWithVarAttributesWithTypeVarSubstrDimsMetaCommon.hh"
#include <server_timing_constants.hh>
#include <sqlite3.h>

extern sqlite3 *db;

using namespace std;

int md_catalog_all_timesteps_with_var_attributes_with_type_var_substr_dims_stub (const md_catalog_all_timesteps_with_var_attributes_with_type_var_substr_dims_args &args,
                           std::vector<md_catalog_timestep_entry> &entries,
                           uint32_t &count);

static int get_matching_timestep_count (const md_catalog_all_timesteps_with_var_attributes_with_type_var_substr_dims_args &args, uint32_t &count);


void OpCatalogAllTimestepsWithVarAttributesWithTypeVarSubstrDimsMeta::deArchiveMsgFromClient(const std::string &serial_str, 
                                             md_catalog_all_timesteps_with_var_attributes_with_type_var_substr_dims_args &args) {

    //log("the archived message is " + serial_str);

	std::stringstream ss;
	ss << serial_str;
	boost::archive::text_iarchive ia(ss);
	ia >> args;
}

std::string OpCatalogAllTimestepsWithVarAttributesWithTypeVarSubstrDimsMeta::serializeMsgToClient(const std::vector<md_catalog_timestep_entry> &entries,
                                                    uint32_t count,
                                                    int return_value) {

	// cout << "in OpCatalogAllTimestepsWithVarAttributesWithTypeVarSubstrDimsMeta, count: " << count << endl;
	// cout << "in OpCatalogAllTimestepsWithVarAttributesWithTypeVarSubstrDimsMeta, attribute_list.size(): " << entries.size() << endl;

	std::stringstream ss;
	boost::archive::text_oarchive oa(ss);
	oa << count;
	if (count > 0) {
		oa << entries;
	}
	oa << return_value;
	//log("the archived message is " + ss.str());


	return ss.str();
}



WaitingType OpCatalogAllTimestepsWithVarAttributesWithTypeVarSubstrDimsMeta::UpdateTarget(OpArgs &args, results_t *results) {
  message_t *incoming_msg = args.data.msg.ptr;

  switch(state){

    case State::start: {
	    add_timing_point(OP_CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_DIMS_START);
	    md_catalog_all_timesteps_with_var_attributes_with_type_var_substr_dims_args sql_args;
	    std::vector<md_catalog_timestep_entry> entries;
	    uint32_t count;
	    
	    peer = args.data.msg.sender;
	    

	    deArchiveMsgFromClient(incoming_msg->body, sql_args);
	    add_timing_point(OP_CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_DIMS_DEARCHIVE_MSG_FROM_CLIENT);

	    int rc = md_catalog_all_timesteps_with_var_attributes_with_type_var_substr_dims_stub(sql_args, entries, count);
	    add_timing_point(OP_CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_DIMS_MD_CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_DIMS_STUB);


	    std::string serial_str = serializeMsgToClient(entries, count, rc);
	    add_timing_point(OP_CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_DIMS_SERIALIZE_MSG_FOR_CLIENT);

	    createOutgoingMessage(incoming_msg->src, 
	                          0, //Not expecting a reply
	                          incoming_msg->src_mailbox,
	                          serial_str);
	    add_timing_point(OP_CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_DIMS_CREATE_MSG_FOR_CLIENT);

	    net::SendMsg(peer, ldo_msg);
	    state=State::done;
	    add_timing_point(OP_CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_DIMS_SEND_MSG_TO_CLIENT_OP_DONE);
	    return WaitingType::done_and_destroy;
	 }
	case State::done:
    	return WaitingType::done_and_destroy;
  	}
}

//WaitingType OpCatalogAllTimestepsWithVarAttributesWithTypeVarSubstrDimsMeta::UpdateOrigin(OpArgs &args, results_t *results) {
WaitingType OpCatalogAllTimestepsWithVarAttributesWithTypeVarSubstrDimsMeta::UpdateOrigin(OpArgs &, results_t *) {
  
	return WaitingType::error;

}


int md_catalog_all_timesteps_with_var_attributes_with_type_var_substr_dims_stub (const md_catalog_all_timesteps_with_var_attributes_with_type_var_substr_dims_args &args,
                           std::vector<md_catalog_timestep_entry> &entries,
                           uint32_t &count)
{
    int rc;
    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;
    string query;

    if(args.num_dims == 1) {
    	query = "select tmc.* from timestep_catalog tmc "
	    "inner join type_catalog tc on tc.run_id = tmc.run_id "
	    "inner join var_attribute_catalog vac on vac.timestep_id = tmc.id and vac.type_id = tc.id "
	    "inner join var_catalog vc on vac.timestep_id = vc.timestep_id and vac.var_id = vc.id "
	    "where (vac.txn_id = ? or vac.active = 1) "
	    "and (tmc.txn_id = ? or tmc.active = 1) "
	    "and vac.type_id = ? "
	    "and vc.name like ? "
	    "and tmc.run_id = ? "
	    "and ? <= vac.d0_max and ? >= vac.d0_min "
	    "group by tmc.id, tmc.run_id";
	}
	else if(args.num_dims == 2) {
    	query = "select tmc.* from timestep_catalog tmc "
	    "inner join type_catalog tc on tc.run_id = tmc.run_id "
	    "inner join var_attribute_catalog vac on vac.timestep_id = tmc.id and vac.type_id = tc.id "
	    "inner join var_catalog vc on vac.timestep_id = vc.timestep_id and vac.var_id = vc.id "
	    "where (vac.txn_id = ? or vac.active = 1) "
	    "and (tmc.txn_id = ? or tmc.active = 1) "
	    "and vac.type_id = ? "
	    "and vc.name like ? "
	    "and tmc.run_id = ? "
	    "and ? <= vac.d0_max and ? >= vac.d0_min "
	    "and ? <= vac.d1_max and ? >= vac.d1_min "
	    "group by tmc.id, tmc.run_id";
	}
	else if(args.num_dims == 3) {
    	query = "select tmc.* from timestep_catalog tmc "
	    "inner join type_catalog tc on tc.run_id = tmc.run_id "
	    "inner join var_attribute_catalog vac on vac.timestep_id = tmc.id and vac.type_id = tc.id "
	    "inner join var_catalog vc on vac.timestep_id = vc.timestep_id and vac.var_id = vc.id "
	    "where (vac.txn_id = ? or vac.active = 1) "
	    "and (tmc.txn_id = ? or tmc.active = 1) "
	    "and vac.type_id = ? "
	    "and vc.name like ? "
	    "and tmc.run_id = ? "
	    "and ? <= vac.d0_max and ? >= vac.d0_min "
	    "and ? <= vac.d1_max and ? >= vac.d1_min "
	    "and ? <= vac.d2_max and ? >= vac.d2_min "
	    "group by tmc.id, tmc.run_id";
	}
	else {
		count = 0;
		return rc;
	}	

    rc = get_matching_timestep_count (args, count); assert (rc == RC_OK);

    if (count > 0) {
        entries.reserve(count);

        rc = sqlite3_prepare_v2 (db, query.c_str(), -1, &stmt, &tail); 
        if (rc != SQLITE_OK)
        {
            fprintf (stderr, "Error catalog_all_timesteps_with_var_attributes_with_type_var_substr_dims stub: Line: %d SQL error (%d): %s\n", __LINE__, rc, sqlite3_errmsg (db));
            sqlite3_close (db);
            goto cleanup;
        }
        // rc = sqlite3_prepare_v2 (db, query.c_str(), -1, &stmt, &tail); assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int64 (stmt, 1, args.txn_id); assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int64 (stmt, 2, args.txn_id); assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int64 (stmt, 3, args.type_id); assert (rc == SQLITE_OK);
    	rc = sqlite3_bind_text (stmt, 4, strdup(args.var_name_substr.c_str()), -1, free); assert (rc == SQLITE_OK);
    	// cout << "var_name_substr: " << args.var_name_substr << endl;
        rc = sqlite3_bind_int64 (stmt, 5, args.run_id); assert (rc == SQLITE_OK);

        for (int j = 0; j < args.num_dims; j++)
        {
            rc = sqlite3_bind_int64 (stmt, 6 + (j * 2), args.dims.at(j).min);
            rc = sqlite3_bind_int64 (stmt, 7 + (j * 2), args.dims.at(j).max);
        }

        rc = sqlite3_step (stmt); assert (rc == SQLITE_OK || rc == SQLITE_ROW);

        while (rc == SQLITE_ROW)
        {
            md_catalog_timestep_entry entry;

            entry.timestep_id = sqlite3_column_int64 (stmt, 0);
            entry.run_id = sqlite3_column_int64 (stmt, 1);
            entry.path = (char *)sqlite3_column_text (stmt, 2);
            entry.active = sqlite3_column_int (stmt, 3);
            entry.txn_id = sqlite3_column_int64 (stmt, 4);
            // cout << "attr.dims.at(2).min: " << attribute.dims.at(2).min << " attr.dims.at(2).min: " << attribute.dims.at(2).max << endl;

            rc = sqlite3_step (stmt);
            entries.push_back(entry);
        }

        rc = sqlite3_finalize (stmt);  
    }
        

cleanup:

    return rc;
}



static int get_matching_timestep_count (const md_catalog_all_timesteps_with_var_attributes_with_type_var_substr_dims_args &args, uint32_t &count)
{
    int rc;
    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;
    string query;

    if(args.num_dims == 1) {
		query = "select count(*) from (select distinct tmc.id, tmc.run_id from timestep_catalog tmc "
	    "inner join type_catalog tc on tc.run_id = tmc.run_id "
	    "inner join var_attribute_catalog vac on vac.timestep_id = tmc.id and vac.type_id = tc.id "
	    "inner join var_catalog vc on vac.timestep_id = vc.timestep_id and vac.var_id = vc.id "
	    "where (vac.txn_id = ? or vac.active = 1) "
	    "and (tmc.txn_id = ? or tmc.active = 1) "
	    "and vac.type_id = ? "
	    "and vc.name like ? "
	    "and tmc.run_id = ? "
	    "and ? <= vac.d0_max and ? >= vac.d0_min ) as internalQuery"; 
	}
	else if(args.num_dims == 2) {
		query = "select count(*) from (select distinct tmc.id, tmc.run_id from timestep_catalog tmc "
	    "inner join type_catalog tc on tc.run_id = tmc.run_id "
	    "inner join var_attribute_catalog vac on vac.timestep_id = tmc.id and vac.type_id = tc.id "
	    "inner join var_catalog vc on vac.timestep_id = vc.timestep_id and vac.var_id = vc.id "
	    "where (vac.txn_id = ? or vac.active = 1) "
	    "and (tmc.txn_id = ? or tmc.active = 1) "
	    "and vac.type_id = ? "
	    "and vc.name like ? "
	    "and tmc.run_id = ? "
	    "and ? <= vac.d0_max and ? >= vac.d0_min "
	    "and ? <= vac.d1_max and ? >= vac.d1_min ) as internalQuery"; 
	}
	else if(args.num_dims == 3) {
		query = "select count(*) from (select distinct tmc.id, tmc.run_id from timestep_catalog tmc "
	    "inner join type_catalog tc on tc.run_id = tmc.run_id "
	    "inner join var_attribute_catalog vac on vac.timestep_id = tmc.id and vac.type_id = tc.id "
	    "inner join var_catalog vc on vac.timestep_id = vc.timestep_id and vac.var_id = vc.id "
	    "where (vac.txn_id = ? or vac.active = 1) "
	    "and (tmc.txn_id = ? or tmc.active = 1) "
	    "and vac.type_id = ? "
	    "and vc.name like ? "
	    "and tmc.run_id = ? "
	    "and ? <= vac.d0_max and ? >= vac.d0_min "
	    "and ? <= vac.d1_max and ? >= vac.d1_min "
	    "and ? <= vac.d2_max and ? >= vac.d2_min ) as internalQuery"; 
	}
    

    rc = sqlite3_prepare_v2 (db, query.c_str(), -1, &stmt, &tail); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 1, args.txn_id); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 2, args.txn_id); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 3, args.type_id); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_text (stmt, 4, strdup(args.var_name_substr.c_str()), -1, free); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 5, args.run_id); assert (rc == SQLITE_OK);

    for (int j = 0; j < args.num_dims; j++)
    {
        rc = sqlite3_bind_int64 (stmt, 6 + (j * 2), args.dims.at(j).min);
        rc = sqlite3_bind_int64 (stmt, 7 + (j * 2), args.dims.at(j).max);
    }

    rc = sqlite3_step (stmt); assert (rc == SQLITE_OK || rc == SQLITE_ROW);
    count = sqlite3_column_int64 (stmt, 0);
    rc = sqlite3_finalize (stmt); assert (rc == SQLITE_OK);
//printf ("matching attr count: %d\n", *count);

    return rc;
}
