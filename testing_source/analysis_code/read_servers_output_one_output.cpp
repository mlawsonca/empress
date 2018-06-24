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

#include "../../include/server/server_timing_constants.hh"
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <iomanip> //set precision
// #include <limits> //numeric_limits
#include <map>
#include "read_testing_output_one_output.hh"

using namespace std;

enum RC
{
    RC_OK = 0,
    RC_ERR = -1
};

static bool extreme_debug_logging = false;
static bool debug_logging = false;
static bool error_logging = true;

static errorLog error_log = errorLog(error_logging);
static debugLog debug_log = debugLog(debug_logging);
static extremeDebugLog extreme_debug_log = extremeDebugLog(extreme_debug_logging);

static void init(ifstream &file, uint16_t code, struct server_config_output &output);
static void ops(ifstream &file, uint16_t code, struct server_config_output &output, long double my_start_time);
static int shutdown(ifstream &file, uint16_t code, struct server_config_output &output);

static int evaluate_clock_times_server(ifstream &file, struct server_config_output &output, std::vector<long double> &start_times);
static int evaluate_storage_sizes(ifstream &file, struct server_config_output &output);

static const uint16_t FIRST_ERR_CODE = ERR_ARGC;
static const uint16_t LAST_INIT_CODE = DIRMAN_SETUP_DONE_INIT_DONE;
static const uint16_t FIRST_OP_CODE = OP_ACTIVATE_VAR_START;
static const uint16_t LAST_TIMING_CODE = DB_CLOSED_SHUTDOWN_DONE; 




static bool is_error(uint16_t code) {
	return code >= FIRST_ERR_CODE;
}

static bool is_op(uint16_t code) {
	return (FIRST_OP_CODE <= code && code < FIRST_ERR_CODE);
}

static bool is_init(uint16_t code) {
	return ( (PROGRAM_START <= code && code < LAST_INIT_CODE) ); 
}

static bool is_shutdown(uint16_t code) {
	return ( (SHUTDOWN_START <= code && code < DB_CLOSED_SHUTDOWN_DONE)); 
}




int evaluate_all_servers(const string &file_path, struct server_config_output &output) {
	
	uint16_t code;
	int rc;
	ifstream file;
	int rank = -1;
	std::vector<long double> my_start_times;

	file.open(file_path);
	if(!file) {
		error_log << "error. failed to open file" << endl;
		return RC_ERR;
	}


	while (file >> code) { 
		if(is_op(code)) {
			extreme_debug_log << "code at top of while loop: " << to_string(code) << endl;
		}
		else { 
			debug_log << "code at top of while loop: " << to_string(code) << endl;
		}

		if(is_error(code)) {
			error_log << "error from in while loop of code: " << code << endl;
			// return 1;
		} 
		else if (code == DB_SIZES) {
		    rc = evaluate_storage_sizes(file, output);
		    if(rc != RC_OK) {
		    	error_log << "server storage size error \n";
		    	goto cleanup;
		    }
		}
		else if (code == CLOCK_TIMES_NEW_PROC) {
			rc = evaluate_clock_times_server(file, output, my_start_times);
			if(rc != RC_OK) {
				error_log << "server clock pts error \n";
				goto cleanup;
			}
		}
		//proc is initing
		//all set up done at 4
		else if(PROGRAM_START <= code && code <= LAST_INIT_CODE ) { 
			debug_log << "initing" << endl;
			//new proc
			rank +=1;
			init(file, code, output);			
		}
		else if(LAST_INIT_CODE < code && code <= LAST_TIMING_CODE) {
			debug_log << "shutdown " << endl;
			shutdown(file, code, output);
			// if(rank == num_server_procs-1) {
			// 	break;
			// }
			// file >> code;
			// error_log << "at end of shutdown again, code: " << code << endl;
			// return rc;
		}
		else if(is_op(code)) {
			ops(file, code, output, my_start_times.at(rank));
		}
		else {
			error_log << "error. code " << code << " didn't fall into the category of error, or starting init, writing, or reading \n";
			return RC_ERR;
		}
	}

cleanup:
	file.close();
	return rc;
}


//Init - program_start .  server_setup_done_init_done
static void init(ifstream &file, uint16_t code, struct server_config_output &output) {
	double start_time;
	double time_pt;

	if(code == PROGRAM_START) {
		file >> start_time; 
	}
	else {
		error_log << "error, first init code wasn't program start. It was " << code << endl;
		return;
	}
//fix - should be doing something with this
	//note - as is the received code is Program_start
	while(is_init(code)) { 
		file >> code;
		file >> time_pt;				
		extreme_debug_log << "init code: "  << to_string(code) << endl;
	}
	if(code == DIRMAN_SETUP_DONE_INIT_DONE) { //fox		init_time = time_pt - start_time;
		output.init_times.push_back(time_pt - start_time);
	}

	else if (is_error(code)) {
		error_log << "error code " << code << " received " << endl;
	}
	else {
		error_log << "error. code " << code << " received instead of init_done \n";
	}
}


static void ops(ifstream &file, uint16_t code, struct server_config_output &output, long double my_start_time) {
	double start_time;
	file >> start_time;
	double time_pt;
	uint16_t deserialze_code;
	uint16_t db_code;
	uint16_t serialize_code;
	uint16_t end_code;

	double deserialize_done_time;
	double db_done_time;
	double serialize_done_time;
	double end_time;

	vector<vector<double>> *op_piece_times;
		switch (code) {
		//read until code = code + 8
		//for each op: total time = XY00-XY08
		case OP_ACTIVATE_VAR_START:
			deserialze_code = OP_ACTIVATE_VAR_DEARCHIVE_MSG_FROM_CLIENT;
			db_code = OP_ACTIVATE_VAR_MD_ACTIVATE_VAR_STUB;
			serialize_code = OP_ACTIVATE_VAR_SERIALIZE_MSG_FOR_CLIENT;
			end_code = OP_ACTIVATE_VAR_SEND_MSG_TO_CLIENT_OP_DONE;
			op_piece_times = &output.activate_var_times;	
			break;

		case OP_CATALOG_VAR_START:
			deserialze_code = OP_CATALOG_VAR_DEARCHIVE_MSG_FROM_CLIENT;
			db_code = OP_CATALOG_VAR_MD_CATALOG_VAR_STUB;
			serialize_code = OP_CATALOG_VAR_SERIALIZE_MSG_FOR_CLIENT;
			end_code = OP_CATALOG_VAR_SEND_MSG_TO_CLIENT_OP_DONE;
			op_piece_times = &output.catalog_var_times;
			break;

		case OP_CREATE_VAR_START:
			deserialze_code = OP_CREATE_VAR_DEARCHIVE_MSG_FROM_CLIENT;
			db_code = OP_CREATE_VAR_MD_CREATE_VAR_STUB;
			serialize_code = OP_CREATE_VAR_SERIALIZE_MSG_FOR_CLIENT;
			end_code = OP_CREATE_VAR_SEND_MSG_TO_CLIENT_OP_DONE;
			op_piece_times = &output.create_var_times;
			break;

		case OP_GET_CHUNK_LIST_START:
			deserialze_code = OP_GET_CHUNK_LIST_DEARCHIVE_MSG_FROM_CLIENT;
			db_code = OP_GET_CHUNK_LIST_MD_GET_CHUNK_LIST_STUB;
			serialize_code = OP_GET_CHUNK_LIST_SERIALIZE_MSG_FOR_CLIENT;
			end_code = OP_GET_CHUNK_LIST_SEND_MSG_TO_CLIENT_OP_DONE;
			op_piece_times = &output.get_chunk_list_times;
			break;

		case OP_GET_CHUNK_START:
			deserialze_code = OP_GET_CHUNK_DEARCHIVE_MSG_FROM_CLIENT;
			db_code = OP_GET_CHUNK_MD_GET_CHUNK_STUB;
			serialize_code = OP_GET_CHUNK_SERIALIZE_MSG_FOR_CLIENT;
			end_code = OP_GET_CHUNK_SEND_MSG_TO_CLIENT_OP_DONE;
			op_piece_times = &output.get_chunk_times;
			break;


		case OP_INSERT_CHUNK_START:
			deserialze_code = OP_INSERT_CHUNK_DEARCHIVE_MSG_FROM_CLIENT;
			db_code = OP_INSERT_CHUNK_MD_INSERT_CHUNK_STUB;
			serialize_code = OP_INSERT_CHUNK_SERIALIZE_MSG_FOR_CLIENT;
			end_code = OP_INSERT_CHUNK_SEND_MSG_TO_CLIENT_OP_DONE;
			op_piece_times = &output.insert_chunk_times;
			break;

		case OP_PROCESSING_VAR_START:
			deserialze_code = OP_PROCESSING_VAR_DEARCHIVE_MSG_FROM_CLIENT;
			db_code = OP_PROCESSING_VAR_MD_PROCESSING_VAR_STUB;
			serialize_code = OP_PROCESSING_VAR_SERIALIZE_MSG_FOR_CLIENT;
			end_code = OP_PROCESSING_VAR_SEND_MSG_TO_CLIENT_OP_DONE;
			op_piece_times = &output.processing_var_times;
			break;

		case OP_ACTIVATE_TYPE_START:
			deserialze_code = OP_ACTIVATE_TYPE_DEARCHIVE_MSG_FROM_CLIENT;
			db_code = OP_ACTIVATE_TYPE_MD_ACTIVATE_TYPE_STUB;
			serialize_code = OP_ACTIVATE_TYPE_SERIALIZE_MSG_FOR_CLIENT;
			end_code = OP_ACTIVATE_TYPE_SEND_MSG_TO_CLIENT_OP_DONE;
			op_piece_times = &output.activate_type_times;
			break;

		case OP_CATALOG_TYPE_START:
			deserialze_code = OP_CATALOG_TYPE_DEARCHIVE_MSG_FROM_CLIENT;
			db_code = OP_CATALOG_TYPE_MD_CATALOG_TYPE_STUB;
			serialize_code = OP_CATALOG_TYPE_SERIALIZE_MSG_FOR_CLIENT;
			end_code = OP_CATALOG_TYPE_SEND_MSG_TO_CLIENT_OP_DONE;
			op_piece_times = &output.catalog_type_times;
			break;

		case OP_CREATE_TYPE_START:
			deserialze_code = OP_CREATE_TYPE_DEARCHIVE_MSG_FROM_CLIENT;
			db_code = OP_CREATE_TYPE_MD_CREATE_TYPE_STUB;
			serialize_code = OP_CREATE_TYPE_SERIALIZE_MSG_FOR_CLIENT;
			end_code = OP_CREATE_TYPE_SEND_MSG_TO_CLIENT_OP_DONE;
			op_piece_times = &output.create_type_times;
			break;

		case OP_GET_ATTR_LIST_START:
			deserialze_code = OP_GET_ATTR_LIST_DEARCHIVE_MSG_FROM_CLIENT;
			db_code = OP_GET_ATTR_LIST_MD_GET_ATTR_LIST_STUB;
			serialize_code = OP_GET_ATTR_LIST_SERIALIZE_MSG_FOR_CLIENT;
			end_code = OP_GET_ATTR_LIST_SEND_MSG_TO_CLIENT_OP_DONE;
			op_piece_times = &output.get_attr_list_times;
			break;

		case OP_GET_ATTR_START:
			deserialze_code = OP_GET_ATTR_DEARCHIVE_MSG_FROM_CLIENT;
			db_code = OP_GET_ATTR_MD_GET_ATTR_STUB;
			serialize_code = OP_GET_ATTR_SERIALIZE_MSG_FOR_CLIENT;
			end_code = OP_GET_ATTR_SEND_MSG_TO_CLIENT_OP_DONE;
			op_piece_times = &output.get_attr_times;
			break;


		case OP_INSERT_ATTR_START:
			deserialze_code = OP_INSERT_ATTR_DEARCHIVE_MSG_FROM_CLIENT;
			db_code = OP_INSERT_ATTR_MD_INSERT_ATTR_STUB;
			serialize_code = OP_INSERT_ATTR_SERIALIZE_MSG_FOR_CLIENT;
			end_code = OP_INSERT_ATTR_SEND_MSG_TO_CLIENT_OP_DONE;
			op_piece_times = &output.insert_attr_times;
			break;	

		case OP_PROCESSING_TYPE_START:
			deserialze_code = OP_PROCESSING_TYPE_DEARCHIVE_MSG_FROM_CLIENT;
			db_code = OP_PROCESSING_TYPE_MD_PROCESSING_TYPE_STUB;
			serialize_code = OP_PROCESSING_TYPE_SERIALIZE_MSG_FOR_CLIENT;
			end_code = OP_PROCESSING_TYPE_SEND_MSG_TO_CLIENT_OP_DONE;
			op_piece_times = &output.processing_type_times;
			break;

		default: //note - many ops fall in this "other" category for now
			error_log << "error. op code: " << code << " did not match an op start code \n";
			return;
	}

	while(code < end_code) {
		file >> code;
		file >> time_pt;
		if(code == deserialze_code) {
			deserialize_done_time = time_pt;
		}
		else if(code == db_code) {
			db_done_time = time_pt;
		}
		else if(code == serialize_code) {
			serialize_done_time = time_pt;
		}
	}	
	if (code == end_code) {
		end_time = time_pt;
		op_piece_times->at(0).push_back(deserialize_done_time-start_time);
		op_piece_times->at(1).push_back(db_done_time-deserialize_done_time);
		op_piece_times->at(2).push_back(serialize_done_time-db_done_time);
		op_piece_times->at(3).push_back(end_time-db_done_time);
		op_piece_times->at(4).push_back(end_time-start_time);
	}
	else {
		error_log << "error. " << end_code << " end time code never appeared after start time code. Instead code " << code << " appeared \n";
	}			
}

static int shutdown(ifstream &file, uint16_t code, struct server_config_output &output) {
	uint16_t num_server_procs = output.config.num_server_procs;

	if(code != SHUTDOWN_START) {
		error_log << "error. first shutdown code was " << code << " instead of SHUTDOWN_START" << endl;
		return RC_ERR;
	}
	double time_pt;
	double start_time;
	file >> start_time;
	debug_log << "start_time: " << start_time << endl;
	//fix - do I want to do anything with this?
	while(is_shutdown(code) && !file.eof()){
		file >> code;
		file >> time_pt;
		extreme_debug_log << "code: " << code << endl;
	}
	if(code != DB_CLOSED_SHUTDOWN_DONE) {
		error_log << "error. last shutdown code was " << code << " instead of DB_CLOSED_SHUTDOWN_DONE" << endl;
		return RC_ERR;
	}
	output.shutdown_times.push_back(time_pt - start_time);
	return RC_OK;
}

static int evaluate_storage_sizes(ifstream &file, struct server_config_output &output) {
	uint16_t num_server_storage_pts = output.config.num_storage_pts;
	uint16_t num_server_procs = output.config.num_server_procs;

	for(int i=0; i<num_server_procs; i++) {

		vector<uint64_t> storage_sizes;
		uint64_t storage_size;
		for(int j=0; j<num_server_storage_pts; j++) {
			file >> storage_size;
			extreme_debug_log << " storage pt " << j << ": " << storage_size << endl;
			output.all_storage_sizes.at(j).push_back(storage_size);

			// storage_sizes.push_back(storage_size);
		}
	}
	return RC_OK;
	// debug_log << "last size: " << storage_size << endl;
}

static int evaluate_clock_times_server(ifstream &file, struct server_config_output &output, std::vector<long double> &my_start_times) {
	uint16_t num_clock_pts = output.config.num_clock_pts;
	uint16_t num_server_procs = output.config.num_server_procs;

	long double time_pt;
	uint16_t code = CLOCK_TIMES_NEW_PROC;
	long double earliest_start = 1000000;
	long double latest_init_done = 0;
	long double latest_shutdown_done = 0;


	for(int i=0; i<num_server_procs; i++) {
		// debug_log << "i: " << i << endl;
		if(i != 0) {
			file >> code;
			if(code != CLOCK_TIMES_NEW_PROC) {
				error_log << "error. directly after server storage sizes was not the clock times. code: " << code << endl;
				return RC_ERR;
			}
		}
		for(int j=0; j<num_clock_pts; j++) {
			file >> time_pt;
			if(j==0) {
				if(time_pt < earliest_start) {
					earliest_start = time_pt;
				}
				my_start_times.push_back(time_pt);
			}	
			else if(j == (num_clock_pts-2) && time_pt > latest_init_done) {
				//the last time point is finalize
				latest_init_done = time_pt;
			}
			else if(j == (num_clock_pts-1) && time_pt > latest_shutdown_done) {
				//the last time point is finalize
				latest_shutdown_done = time_pt;
			}
			extreme_debug_log << "time_pt: " << time_pt << endl;
			output.all_clock_times.at(j).push_back(time_pt);
		}
	}
	output.clock_times_eval.at(0).push_back(latest_init_done-earliest_start);
	output.clock_times_eval.at(1).push_back(latest_shutdown_done-earliest_start);	

	file >> code;
	if(code != CLOCK_TIMES_DONE) {
		error_log << "error. last server clock code was " << code << " instead of CLOCK_TIMES_DONE" << endl;
		return RC_ERR;
	}
	debug_log << "last time_pt: " << time_pt << endl;
	return RC_OK;
}
