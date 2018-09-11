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


#include "OpCatalogRunMetaCommon.hh"
#include <server_timing_constants.hh>
#include <sqlite3.h>

extern sqlite3 *db;

using namespace std;

int md_catalog_run_stub (md_catalog_run_args &args, uint32_t &count, std::vector<md_catalog_run_entry> &entries);


void OpCatalogRunMeta::deArchiveMsgFromClient(const std::string &serial_str, md_catalog_run_args &args) {

    //log("the archived message is " + serial_str);

	std::stringstream ss;
	ss << serial_str;
	boost::archive::text_iarchive ia(ss);
	ia >> args;
}

std::string OpCatalogRunMeta::serializeMsgToClient(const std::vector<md_catalog_run_entry> &entries,
                                                uint32_t count,
                                                int return_value) {
	std::stringstream ss;
	boost::archive::text_oarchive oa(ss);
	oa << count;
	if(count > 0) {
		oa << entries;
	}
	oa << return_value;
	//log("the archived message is " + ss.str());
	// cout << "archived message: " << ss.str() << endl;
	// cout << "archived message.size(): " << ss.str().size() << endl;

	return ss.str();
}


WaitingType OpCatalogRunMeta::UpdateTarget(OpArgs &args, results_t *results) {
  message_t *incoming_msg = args.data.msg.ptr;

  switch(state){

    case State::start: {
	    add_timing_point(OP_CATALOG_RUN_START);
	    md_catalog_run_args sql_args;
	    uint32_t count;    
	    std::vector<md_catalog_run_entry> entries;

	    peer = args.data.msg.sender;


	    //convert the serialized string back into the args and count pointer
	    deArchiveMsgFromClient(incoming_msg->body, sql_args);    
	    add_timing_point(OP_CATALOG_RUN_DEARCHIVE_MSG_FROM_CLIENT);

	    int rc = md_catalog_run_stub(sql_args, count, entries);
	    add_timing_point(OP_CATALOG_RUN_MD_CATALOG_RUN_STUB);

	    //log("num of entries is " + std::to_string(entries.size()));
	    if(entries.size()>0) {
	        //log("in md_catalog_run_stub the first entry has name " + entries.at(0).name);

	    }

	    std::string serial_str = serializeMsgToClient(entries, count, rc);
	    add_timing_point(OP_CATALOG_RUN_SERIALIZE_MSG_FOR_CLIENT);

	    createOutgoingMessage(incoming_msg->src, 
	                          0, //Not expecting a reply
	                          incoming_msg->src_mailbox,
	                          serial_str);
	    add_timing_point(OP_CATALOG_RUN_CREATE_MSG_FOR_CLIENT);
	    
	    net::SendMsg(peer, ldo_msg);
	    state=State::done;
	    add_timing_point(OP_CATALOG_RUN_SEND_MSG_TO_CLIENT_OP_DONE);
	    return WaitingType::done_and_destroy;
  	}
	case State::done:
    	return WaitingType::done_and_destroy;
  	}
}
//WaitingType OpCatalogRunMeta::UpdateOrigin(OpArgs &args, results_t *results) {
WaitingType OpCatalogRunMeta::UpdateOrigin(OpArgs &, results_t *) {
  
	return WaitingType::error;

}

// extern int callback (void * NotUsed, int argc, char ** argv, char ** ColName);

int md_catalog_run_stub (md_catalog_run_args &args, 
                     uint32_t &count,
                     std::vector<md_catalog_run_entry> &entries)
{
    int rc;
    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;
    // const char * query = "select * from run_catalog where txn_id = ? or active = 1 order by name, id";
    const char * query = "select * from run_catalog where txn_id = ? or active = 1 order by id";
    size_t size = 0;

    rc = sqlite3_prepare_v2 (db, "select count (*) from run_catalog where txn_id = ? or active = 1", -1, &stmt, &tail); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 1, args.txn_id); assert (rc == SQLITE_OK);
    rc = sqlite3_step (stmt); assert (rc == SQLITE_OK || rc == SQLITE_ROW);
    count = sqlite3_column_int64 (stmt, 0);
    rc = sqlite3_finalize (stmt); assert (rc == SQLITE_OK);

//printf ("rows in run_catalog: %d\n", count);
    if (count > 0) {
      entries.reserve(count);


    // char *err_msg = 0;
    // char *sql = "SELECT * FROM run_catalog";
    // cout << "starting run catalog \n";
    // rc = sqlite3_exec(db, sql, callback, 0, &err_msg);
    // cout << "done with run catalog \n";

        rc = sqlite3_prepare_v2 (db, query, -1, &stmt, &tail); assert (rc == SQLITE_OK);
        if (rc != SQLITE_OK)
        {
            fprintf (stderr, "Error md_catalog_run_stub: Line: %d SQL error (%d): %s\n", __LINE__, rc, sqlite3_errmsg (db));
            sqlite3_close (db);
            goto cleanup;
        }
        rc = sqlite3_bind_int64 (stmt, 1, args.txn_id); assert (rc == SQLITE_OK);
        rc = sqlite3_step (stmt); assert (rc == SQLITE_ROW || rc == SQLITE_OK);

        while (rc == SQLITE_ROW)
        {
            int j = 0;

            md_catalog_run_entry entry;


            entry.run_id = sqlite3_column_int64 (stmt, 0);
            entry.job_id = sqlite3_column_int64 (stmt, 1);
            entry.name = (char *)sqlite3_column_text (stmt, 2);
            // entry.path = (char *)sqlite3_column_text (stmt, 2);
            entry.date = (char *)sqlite3_column_text (stmt, 3);
            entry.active = sqlite3_column_int (stmt, 4);
            entry.txn_id = sqlite3_column_int64 (stmt, 5);
            entry.npx = sqlite3_column_int (stmt, 6);
            entry.npy = sqlite3_column_int (stmt, 7);
            entry.npz = sqlite3_column_int (stmt, 8);


            int size_rank_funct = sqlite3_column_bytes (stmt, 9);
            entry.rank_to_dims_funct.assign((char *)sqlite3_column_text (stmt, 9),(char *)sqlite3_column_text (stmt, 9) + size_rank_funct);
            // cout << "entry.rank_to_dims_funct.size(): " << entry.rank_to_dims_funct.size() << endl;
            int size_obj_funct = sqlite3_column_bytes (stmt, 10);
            entry.objector_funct.assign((char *)sqlite3_column_text (stmt, 10),(char *)sqlite3_column_text (stmt, 10) + size_obj_funct);
           	// cout << "objector_funct.size(): " << entry.objector_funct.size() << endl;

           	// entry.rank_to_dims_funct = (string)sqlite3_column_blob (stmt, 8);

     		// entry.rank_to_dims_funct = (char *)sqlite3_column_blob (stmt, 8);
       //      cout << "entry.rank_to_dims_funct.size(): " << entry.rank_to_dims_funct.size() << endl;
       //      entry.objector_funct = (char *)sqlite3_column_blob (stmt, 9);
       // //     	cout << "objector_funct.size(): " << entry.objector_funct.size() << endl;

       //      string &s = *(static_cast<std::string*>(sqlite3_column_blob (stmt, 8)));
       //                  string &s2 = *(static_cast<std::string*>(sqlite3_column_blob (stmt, 9)));

          //   int size_rank_funct = sqlite3_column_bytes (stmt, 8);
          //   // memcpy(entry.rank_to_dims_funct, sqlite3_column_blob(stmt, 8), size_rank_funct);
          //   entry.rank_to_dims_funct.assign(&((char *)sqlite3_column_blob(stmt, 8))[0], &(char *)(sqlite3_column_blob(stmt, 8))[0] + size_rank_funct);

          //   int size_obj_funct = sqlite3_column_bytes (stmt, 9);
          //   entry.objector_funct.assign(&((char *)sqlite3_column_blob(stmt, 9))[0], &(char *)(sqlite3_column_blob(stmt, 9))[0] + size_obj_funct);

          //   // memcpy(entry.objector_funct, sqlite3_column_blob(stmt, 9), size_obj_funct);	
         	// // entry.rank_to_dims_funct = (char *)sqlite3_column_text (stmt, 8);
          //   cout << "entry.rank_to_dims_funct.size(): " << entry.rank_to_dims_funct.size() << endl;
          // //   entry.objector_funct = (char *)sqlite3_column_text (stmt, 9);
          //  	cout << "objector_funct.size(): " << entry.objector_funct << endl;

     		// entry.rank_to_dims_funct = *(static_cast<std::string*>(sqlite3_column_blob (stmt, 8)));
       //      cout << "entry.rank_to_dims_funct.size(): " << entry.rank_to_dims_funct.size() << endl;
       //      entry.objector_funct = *(static_cast<std::string*>(sqlite3_column_blob (stmt, 9)));
       //     	cout << "objector_funct.size(): " << entry.objector_funct.size() << endl;

            // entry.rank_to_dims_funct = (char *)sqlite3_column_text (stmt, 8);
            // cout << "entry.rank_to_dims_funct.size(): " << entry.rank_to_dims_funct.size() << endl;
            // entry.objector_funct = (char *)sqlite3_column_text (stmt, 9);
           	// cout << "objector_funct.size(): " << entry.objector_funct << endl;

            // entry.rank_to_dims_funct = (string)sqlite3_column_text (stmt, 8);
            // cout << "entry.rank_to_dims_funct.size(): " << entry.rank_to_dims_funct.size() << endl;
            // entry.objector_funct = (string)sqlite3_column_text (stmt, 9);
           	// cout << "objector_funct.size(): " << entry.objector_funct.size() << endl;
    //printf ("id: %d name: %s path: %s version: %d num_dims: %d ", id, entry [i].name, entry [i].path, entry [i].version, entry [i].num_dims);
    //printf ("\n");
            rc = sqlite3_step (stmt);
            entries.push_back(entry);
        }

        rc = sqlite3_finalize (stmt);  
    }
    

cleanup:

    return rc;
}
