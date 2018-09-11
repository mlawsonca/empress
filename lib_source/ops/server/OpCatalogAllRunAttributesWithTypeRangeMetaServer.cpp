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


#include "OpCatalogAllRunAttributesWithTypeRangeMetaCommon.hh"
#include <server_timing_constants.hh>
#include <sqlite3.h>

extern sqlite3 *db;

using namespace std;

template <class T>
int md_catalog_all_run_attributes_with_type_range_stub (const md_catalog_all_run_attributes_with_type_range_args &args,
						   T min, T max,
                           std::vector<md_catalog_run_attribute_entry> &attribute_list,
                           uint32_t &count);

template <class T>
static int get_matching_attribute_count (const md_catalog_all_run_attributes_with_type_range_args &args,
	T min, T max, string query_str, uint32_t &count);

void OpCatalogAllRunAttributesWithTypeRangeMeta::deArchiveMsgFromClient(const std::string &serial_str, 
                                             md_catalog_all_run_attributes_with_type_range_args &args) {

    //log("the archived message is " + serial_str);

	std::stringstream ss;
	ss << serial_str;
	boost::archive::text_iarchive ia(ss);
	ia >> args;
}

std::string OpCatalogAllRunAttributesWithTypeRangeMeta::serializeMsgToClient(const std::vector<md_catalog_run_attribute_entry> &attribute_list,
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



WaitingType OpCatalogAllRunAttributesWithTypeRangeMeta::UpdateTarget(OpArgs &args, results_t *results) {
  	message_t *incoming_msg = args.data.msg.ptr;
	int rc;

  switch(state){

    case State::start: {
	    add_timing_point(OP_CATALOG_ALL_RUN_ATTRIBUTES_WITH_TYPE_RANGE_START);
	    md_catalog_all_run_attributes_with_type_range_args sql_args;
	    std::vector<md_catalog_run_attribute_entry> attribute_list;
	    uint32_t count;
	    
	    peer = args.data.msg.sender;
	    

	    deArchiveMsgFromClient(incoming_msg->body, sql_args);
	    add_timing_point(OP_CATALOG_ALL_RUN_ATTRIBUTES_WITH_TYPE_RANGE_DEARCHIVE_MSG_FROM_CLIENT);


		stringstream sso;
		sso << sql_args.data;
		boost::archive::text_iarchive ia(sso);

		switch(sql_args.data_type) {
	        case ATTR_DATA_TYPE_INT : {
	        	uint64_t min_int;
	        	uint64_t max_int;

	        	if(sql_args.range_type == DATA_RANGE) {
	    //	cout << "got here 1 \n";
	        		ia >> min_int;
			    	ia >> max_int;
	        	}
	        	else if(sql_args.range_type == DATA_MAX || sql_args.range_type == DATA_MIN) {
	        		ia >> min_int;
	        		max_int = min_int;
	        	}

	    		rc = md_catalog_all_run_attributes_with_type_range_stub(sql_args, 
	    			min_int, max_int, attribute_list, count);

	            break;
	        }
	        case ATTR_DATA_TYPE_REAL : {
	        	long double min_real;
	        	long double max_real;

	        	if(sql_args.range_type == DATA_RANGE) {
	        		ia >> min_real;
			    	ia >> max_real;
	        	}
	        	else if(sql_args.range_type == DATA_MAX || sql_args.range_type == DATA_MIN) {
	        		ia >> min_real;
	        		max_real = min_real;
	        	}

	    		rc = md_catalog_all_run_attributes_with_type_range_stub(sql_args, 
	    			min_real, max_real, attribute_list, count);

	            break;
	        }
    	}

	    add_timing_point(OP_CATALOG_ALL_RUN_ATTRIBUTES_WITH_TYPE_RANGE_MD_CATALOG_ALL_RUN_ATTRIBUTES_WITH_TYPE_RANGE_STUB);


	    std::string serial_str = serializeMsgToClient(attribute_list, count, rc);
	 // cout << "in catalog all, attr list is of size: " << attribute_list.size() << endl;
	    add_timing_point(OP_CATALOG_ALL_RUN_ATTRIBUTES_WITH_TYPE_RANGE_SERIALIZE_MSG_FOR_CLIENT);

	    createOutgoingMessage(incoming_msg->src, 
	                          0, //Not expecting a reply
	                          incoming_msg->src_mailbox,
	                          serial_str);
	    add_timing_point(OP_CATALOG_ALL_RUN_ATTRIBUTES_WITH_TYPE_RANGE_CREATE_MSG_FOR_CLIENT);

	    net::SendMsg(peer, ldo_msg);
	    state=State::done;
	    add_timing_point(OP_CATALOG_ALL_RUN_ATTRIBUTES_WITH_TYPE_RANGE_SEND_MSG_TO_CLIENT_OP_DONE);
	    return WaitingType::done_and_destroy;
  	}
	case State::done:
    	return WaitingType::done_and_destroy;
  	}
}

//WaitingType OpCatalogAllRunAttributesWithTypeRangeMeta::UpdateOrigin(OpArgs &args, results_t *results) {
WaitingType OpCatalogAllRunAttributesWithTypeRangeMeta::UpdateOrigin(OpArgs &, results_t *) {
  
	return WaitingType::error;

}

template <class T>
int md_catalog_all_run_attributes_with_type_range_stub (const md_catalog_all_run_attributes_with_type_range_args &args,
                           T min, T max,
                           std::vector<md_catalog_run_attribute_entry> &attribute_list,
                           uint32_t &count)
{
    int rc;
    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;

    string query = "select rac.* from run_attribute_catalog rac "
    "inner join type_catalog tc on rac.type_id = tc.id "
    "where (rac.txn_id = ? or rac.active = 1) "
    "and rac.type_id = ? "
    "and tc.run_id = ? "
    "and  (rac.data_type = ? or rac.data_type = ?) ";


    //	cout << "got here 2 \n";

    // const char *query_str;
    string query_str;

    switch(args.range_type) {
    	case DATA_RANGE : {
    		query_str = "and ( ? <= rac.data and rac.data <= ? ) ";
    		break;
    	}
       	case DATA_MAX : {
    		query_str = "and ( ? <= rac.data ) ";
    		break;
    	}
        case DATA_MIN : {
    		query_str = "and ( rac.data <= ? ) ";
    		break;
    	}
    }

	query += query_str;

    //	cout << "got here 3 \n";

    rc = get_matching_attribute_count (args, min, max, query_str, count); assert (rc == RC_OK);

    //	cout << "got here 4 \n";

    if (count > 0) {
        attribute_list.reserve(count);


        rc = sqlite3_prepare_v2 (db, query.c_str(), -1, &stmt, &tail); 
        if (rc != SQLITE_OK)
        {
            fprintf (stderr, "Error catalog_all_run_attributes_with_type_range_stub: Line: %d SQL error (%d): %s\n", __LINE__, rc, sqlite3_errmsg (db));
            sqlite3_close (db);
            goto cleanup;
        }
    //	cout << "got here 5 \n";


        rc = sqlite3_bind_int64 (stmt, 1, args.txn_id); assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int64 (stmt, 2, args.type_id); assert (rc == SQLITE_OK);        
        rc = sqlite3_bind_int64 (stmt, 3, args.run_id); assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int (stmt, 4, ATTR_DATA_TYPE_INT); assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int (stmt, 5, ATTR_DATA_TYPE_REAL); assert (rc == SQLITE_OK);

		switch(args.data_type) {
	        case ATTR_DATA_TYPE_INT : {
	        	    //	cout << "got here 6 \n";

	        	rc = sqlite3_bind_int64 (stmt, 6, (uint64_t)min); assert (rc == SQLITE_OK);
	        	if(args.range_type == DATA_RANGE) {
	        		rc = sqlite3_bind_int64 (stmt, 7, (uint64_t)max); assert (rc == SQLITE_OK);
	        		    //	cout << "got here 7 \n";

	        	}
	            break;
	        }
	        case ATTR_DATA_TYPE_REAL : {
	        	rc = sqlite3_bind_double (stmt, 6, (long double)min); assert (rc == SQLITE_OK);
	        	if(args.range_type == DATA_RANGE) {
	        		rc = sqlite3_bind_double (stmt, 7, (long double)max); assert (rc == SQLITE_OK);
	        	}
	            break;
	        }
	    }

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
            switch(attribute.data_type) {
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
            }                        
         // cout << "attr.dims.at(2).min: " << attribute.dims.at(2).min << " attr.dims.at(2).min: " << attribute.dims.at(2).max << endl;

            rc = sqlite3_step (stmt);
            attribute_list.push_back(attribute);
        }

        rc = sqlite3_finalize (stmt);  
    }
        

cleanup:

    return rc;
}


template <class T>
static int get_matching_attribute_count (const md_catalog_all_run_attributes_with_type_range_args &args, 
	T min, T max, string query_str, uint32_t &count)
{
    int rc;
    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;
    string query = "select count (*) from run_attribute_catalog rac "
    "inner join type_catalog tc on rac.type_id = tc.id "
    "where (rac.txn_id = ? or rac.active = 1) "
    "and rac.type_id = ? "
    "and tc.run_id = ? "
    "and  (rac.data_type = ? or rac.data_type = ?) " + query_str;// cout << "query: " << query << endl;

    rc = sqlite3_prepare_v2 (db, query.c_str(), -1, &stmt, &tail); assert (rc == SQLITE_OK);

        //	cout << "got here 4.5 \n";

    rc = sqlite3_bind_int64 (stmt, 1, args.txn_id); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 2, args.type_id); assert (rc == SQLITE_OK);        
    rc = sqlite3_bind_int64 (stmt, 3, args.run_id); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int (stmt, 4, ATTR_DATA_TYPE_INT); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int (stmt, 5, ATTR_DATA_TYPE_REAL); assert (rc == SQLITE_OK);// cout << "min: " << min << " max: " << max << endl;

	switch(args.data_type) {
	    case ATTR_DATA_TYPE_INT : {
	    	        //	cout << "got here 4.6 \n";

	    	rc = sqlite3_bind_int64 (stmt, 6, (uint64_t)min); assert (rc == SQLITE_OK);
	    	if(args.range_type == DATA_RANGE) {
	    		rc = sqlite3_bind_int64 (stmt, 7, (uint64_t)max); assert (rc == SQLITE_OK);
	    		        //	cout << "got here 4.8 \n";

	    	}
	        break;
	    }
	    case ATTR_DATA_TYPE_REAL : {
	    	rc = sqlite3_bind_double (stmt, 6, (long double)min); assert (rc == SQLITE_OK);
	    	if(args.range_type == DATA_RANGE) {
	    		rc = sqlite3_bind_double (stmt, 7, (long double)max); assert (rc == SQLITE_OK);
	    	}
	        break;
	    }
	}
    rc = sqlite3_step (stmt); assert (rc == SQLITE_OK || rc == SQLITE_ROW);
    count = sqlite3_column_int64 (stmt, 0);
 // cout << "in catalog all attrs, count: " << count << endl;
    rc = sqlite3_finalize (stmt); assert (rc == SQLITE_OK);

    return rc;
}
