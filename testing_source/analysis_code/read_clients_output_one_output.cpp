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

#include "../../include/client/client_timing_constants.hh"
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

static void init(ifstream &file, uint16_t code, struct client_config_output &output);
static void writing(ifstream &file, uint16_t code, struct client_config_output &output, long double my_start_time);
static void read(ifstream &file, uint16_t code, int rank, struct client_config_output &output, long double my_start_time); //fix - why does it need num_server_procs
static void read_pattern_1(ifstream &file, double start_time, struct client_config_output &output, long double my_start_time);
static void read_pattern_2(ifstream &file, double start_time, struct client_config_output &output, long double my_start_time);
static void read_pattern_3(ifstream &file, double start_time, struct client_config_output &output, long double my_start_time);
static void read_pattern_4(ifstream &file, double start_time, struct client_config_output &output, long double my_start_time);
static void read_pattern_5(ifstream &file, double start_time, struct client_config_output &output, long double my_start_time);
static void read_pattern_6(ifstream &file, double start_time, struct client_config_output &output, long double my_start_time);
static void ops(ifstream &file, uint16_t code, double start_time, struct client_config_output &output, long double my_start_time);
static void shutdown(ifstream &file, int rank, uint16_t code, struct client_config_output &output, long double my_start_time);

static int evaluate_all_clock_times_client(ifstream &file, struct client_config_output &output, vector<long double> &start_times);

static const uint16_t FIRST_ERR_CODE = ERR_ARGC;
static const uint16_t FIRST_OP_CODE = MD_ACTIVATE_VAR_START;
static const uint16_t FIRST_READ_PATTERN_CODE = READ_PATTERN_1_START;
static const uint16_t LAST_TIMING_CODE = DIRMAN_SHUTDOWN_DONE;

static bool is_error(uint16_t code) {
	return code >= FIRST_ERR_CODE;
}

static bool is_op(uint16_t code) {
	return (FIRST_OP_CODE <= code && code < FIRST_ERR_CODE);
}

static bool is_read_pattern(uint16_t code) {
	return( FIRST_READ_PATTERN_CODE <= code && code < FIRST_OP_CODE );
}

static bool is_init(uint16_t code) {
	return ( (PROGRAM_START <= code && code < SERVER_SETUP_DONE_INIT_DONE) );
}

static bool is_writing(uint16_t code) {
	return ( (WRITING_START <= code && code < WRITING_DONE) || is_op(code));
}

static bool is_reading(uint16_t code) {
	return ( (READING_START <= code && code < READING_DONE) || is_op(code) || is_read_pattern(code));
}

static bool is_shutdown(uint16_t code) {
	return ( (READING_DONE <= code && code < LAST_TIMING_CODE) || is_op(code));
}


int evaluate_all_clients(const string &file_path, struct client_config_output &output) {
		
	uint16_t code;
	int rc;
	ifstream file;
	int rank = -1;
	vector<long double> my_start_times;

	file.open(file_path);
	if(!file) {
		error_log << "error. failed to open file" << endl;
		return RC_ERR;
	}

	while (file >> code) { 
		debug_log << "code at top of while loop: " << to_string(code) << endl;

		if(is_error(code)) {
			error_log << "error from in while loop of code: " << code << endl;
			// return 1;
		} 
		else if(code == CLOCK_TIMES_NEW_PROC) {
			debug_log << "code: " << to_string(code) << endl;
			rc = evaluate_all_clock_times_client(file, output, my_start_times);
			if(rc != RC_OK) {
				error_log << "client clock pts error \n";
				goto cleanup;
			}
		}
		//proc is initing
		//all set up done at 4
		else if(PROGRAM_START <= code && code < WRITING_START ) { 
			debug_log << "initing" << endl;
			//new proc
			rank +=1;
			init(file, code, output);			
		}
		//writing
		//total writing time: writing_start - writing_done
		else if(WRITING_START <= code  && code < WRITING_DONE) {
			writing(file, code, output, my_start_times.at(rank));
			debug_log << "code returned from writing: " << to_string(code) << endl;
		}
		//reading
		//reading time: reading_done - reading_start
		//total reading time: reading_done_for_all_procs_start_cleanup - earliest reading start
		//fix - reading should collect all of these start and "finished" times to find the total
		else if(READING_START <= code && code < READING_DONE) {
			debug_log << "reading " << endl;
			//fix - do we want to pass the code?
			read(file, code, rank, output, my_start_times.at(rank));
		}

		else if(READING_DONE <= code && code <= LAST_TIMING_CODE) {
			debug_log << "shutdown " << endl;
			shutdown(file, rank, code, output, my_start_times.at(rank));
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

static void init(ifstream &file, uint16_t code, struct client_config_output &output) {
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
	if(code == SERVER_SETUP_DONE_INIT_DONE) {
		//init time: time_pt - start_time //fix
		output.init_times.push_back(time_pt - start_time);
	}
	else if (is_error(code)) {
		error_log << "error code " << code << " received " << endl;
	}
	else {
		error_log << "error. code " << code << " received instead of init_done \n";
	}
}

//Writing  - WRITING_START . WRITING_DONE
static void writing(ifstream &file, uint16_t code, struct client_config_output &output, long double my_start_time) {
	double start_time;
	file >> start_time;
	double insert_start_time;

	double time_pt;
	if( code != WRITING_START ) {
		error_log << "error. Writing start not found before process began writing \n";
		while(is_writing(code) && !file.eof()) {
			file>> code;
			file >> time_pt;
		}
		return;
	}

	while(is_writing(code) && !file.eof()) {
		file>> code;
		file >> time_pt;

		if(is_op(code)) {
			extreme_debug_log << "starting an op \n";
			ops(file, code, time_pt, output, my_start_time);
			extreme_debug_log << "just completed an op" << endl;
		}
		else if(code == CREATE_TYPES_DONE) {
			output.writing_create_times.push_back(time_pt - start_time);
			insert_start_time = time_pt;
		}
	}
	if(code == WRITING_DONE) { //started reading
		// debug_log << "start_time: " << start_time << " end read time pt: " << last_time_pt << endl;
		output.writing_times.push_back(time_pt - start_time);
		output.writing_insert_times.push_back(time_pt - insert_start_time);
	}
	else if(is_error(code)){
		error_log << "error. issued an error code " << code << "while reading \n"; // note - this will be changing		
	} 
	else {
		error_log << "error. reading done never came after writing start. code is " << code << " \n"; // note - this will be changing
	}
	// md_log("code in writing: " + to_string(code)); 
}


static void read(ifstream &file, uint16_t init_code, int rank, struct client_config_output &output, long double my_start_time) {
	//fix - read should check if this proc is actually reading
	//fix - read shoud keep track of the earliest start time and latest finishin
		//fix - needs to know start time
	double read_start;
	double reading_patterns_start;
	double read_type_patterns_start;
	double extra_testing_start;
	uint16_t num_server_procs = output.config.num_server_procs;


	file >> read_start;



	double time_pt;
	uint16_t code = init_code;
	bool servers_shutdown = false;

	while(!file.eof() && is_reading(code)) {
		file >> code;
		file >> time_pt;
		// md_log("time_pt: " + to_string(time_pt));
		//read patterns
		// md_log("in reading, code: " + to_string(code));
		if(READING_START <= code && code <= READING_DONE) {
			switch (code) {
				case BCAST_CATALOGS_DONE:
					extreme_debug_log << "done bcasting catalog" << endl; 
					output.read_init_times.push_back(time_pt-read_start);
					reading_patterns_start = time_pt;
					break;

				case READING_PATTERNS_DONE:
					extreme_debug_log << "done reading patterns" << endl; 
					output.read_pattern_times.push_back(time_pt - reading_patterns_start);
					read_type_patterns_start = time_pt;	
			
					break;

				case READING_TYPE_PATTERNS_DONE:
					extreme_debug_log << "done reading type patterns" << endl; 
					output.read_type_pattern_times.push_back(time_pt - read_type_patterns_start);
					output.read_all_pattern_times.push_back(time_pt - read_start);	
					extra_testing_start = time_pt;	
					if(rank >= num_server_procs) {
						output.reading_times.push_back(time_pt - read_start);	
					}			
					break;

				case EXTRA_TESTING_DONE:
					if(rank < num_server_procs) {
						extreme_debug_log << "done with extra testing" << endl; 
						output.extra_testing_times.push_back(time_pt - extra_testing_start);
					}
					break;

				case READING_DONE:
					if(rank < num_server_procs || output.config.num_types == 0) {
						extreme_debug_log << "done reading" << endl; 
						output.reading_times.push_back(time_pt - read_start);
					}
					break;
			}
		}
		else if(is_read_pattern(code)) {
			switch (code) {
				case READ_PATTERN_1_START:
					read_pattern_1(file, time_pt, output, my_start_time);
					break;

				case READ_PATTERN_2_START:
					read_pattern_2(file, time_pt, output, my_start_time);	
					break;			

				case READ_PATTERN_3_START:
					read_pattern_3(file, time_pt, output, my_start_time);	
					break;	

				case READ_PATTERN_4_START:
					read_pattern_4(file, time_pt, output, my_start_time);	
					break;	

				case READ_PATTERN_5_START:
					read_pattern_5(file, time_pt, output, my_start_time);	
					break;	

				case READ_PATTERN_6_START:
					read_pattern_6(file, time_pt, output, my_start_time);	
					break;

				default:
					error_log << "error. code did not match a pattern start \n";
				//fix - if it isn't one of these, should issue a waring	
			}
		}
		//ops
		else if(is_op(code)) {
			ops(file, code, time_pt, output, my_start_time);
		}
		else if(is_error(code)) {
			error_log << "there was an error code: " << code << endl;
			// return;
		}
		else {
			error_log << "error. received code " << code << " which did not match one of readings expected categories \n";
		}
	}
	debug_log << "done reading file" << endl;
	debug_log << "time_pt: " << time_pt << endl;
}

//total pattern 1 time: 108-100
static void read_pattern_1(ifstream &file, double start_time, struct client_config_output &output, long double my_start_time) {
	uint16_t code = READ_PATTERN_1_START;
	double time_pt;
	bool type = false;

	while(code != READ_PATTERN_1_DONE_READING && !is_error(code)) {
		file >> code;
		file >> time_pt;
		//is type
		if(code == READ_PATTERN_1_START_GET_ATTRS) {
			type = true;
		}
		else if(is_op(code)) {
			ops(file, code, time_pt, output, my_start_time);
		}
	}
	if(code == READ_PATTERN_1_DONE_READING) {
		if(type) {
			output.read_pattern_1_type_times.push_back(time_pt - start_time);
		}
		else {
			output.read_pattern_1_times.push_back(time_pt - start_time);
		}
	}
	else {
		error_log << "error. pattern 1 end time code never appeared after start time code. Instead code " << code << " appeared \n";
	}	
}

static void read_pattern_2(ifstream &file, double start_time, struct client_config_output &output, long double my_start_time) {
	uint16_t code = READ_PATTERN_2_START;
	double time_pt;
	bool type = false;

	while(code != READ_PATTERN_2_DONE_READING && !is_error(code)) {
		file >> code;
		file >> time_pt;
		//is type
		if(code == READ_PATTERN_2_START_GET_ATTRS) {
			type = true;
		}
		else if(is_op(code)) {
			ops(file, code, time_pt, output, my_start_time);
		}
	}
	if (code == READ_PATTERN_2_DONE_READING) {
		if(type) {
			output.read_pattern_2_type_times.push_back(time_pt - start_time);
		}
		else {
			output.read_pattern_2_times.push_back(time_pt - start_time);
		}
	}
	else {
		error_log << "error. pattern 2 end time code never appeared after start time code. Instead code " << code << " appeared \n";
	}	
}

static void read_pattern_3(ifstream &file, double start_time, struct client_config_output &output, long double my_start_time) {
	uint16_t code = READ_PATTERN_3_START;

	double time_pt;
	bool type = false;

	while(code != READ_PATTERN_3_DONE_READING && !is_error(code)) {
		file >> code;
		file >> time_pt;
		//is type
		if(code == READ_PATTERN_3_START_GET_ATTRS) {
			type = true;
		}
		else if(is_op(code)) {
			ops(file, code, time_pt, output, my_start_time);
		}
	}
	if (code == READ_PATTERN_3_DONE_READING) {
		if(type) {
			output.read_pattern_3_type_times.push_back(time_pt - start_time);
		}
		else {
			output.read_pattern_3_times.push_back(time_pt - start_time);
		}	
	}
	else {
		error_log << "error. pattern 3 end time code never appeared after start time code. Instead code " << code << " appeared \n";
	}	
}

//total pattern 4 time: 408-400
static void read_pattern_4(ifstream &file, double start_time, struct client_config_output &output, long double my_start_time) {

	uint16_t code = READ_PATTERN_4_START;
	double time_pt;
	bool type = false;

	while(code != READ_PATTERN_4_DONE_READING && !is_error(code)) {
		file >> code;
		file >> time_pt;
		//is type
		if(code == READ_PATTERN_4_START_GET_ATTRS) {
			type = true;
		}
		else if(is_op(code)) {
			ops(file, code, time_pt, output, my_start_time);
		}
	}
	if (code == READ_PATTERN_4_DONE_READING) {
		if(type) {
			output.read_pattern_4_type_times.push_back(time_pt - start_time);
		}
		else {
			output.read_pattern_4_times.push_back(time_pt - start_time);
		}	
	}
	else {
		error_log << "error. pattern 4 end time code never appeared after start time code. Instead code " << code << " appeared \n";
	}	
}

//total pattern 5 time: 507-500
static void read_pattern_5(ifstream &file, double start_time, struct client_config_output &output, long double my_start_time) {

	uint16_t code = READ_PATTERN_5_START;
	double time_pt;
	bool type = false;

	while(code != READ_PATTERN_5_DONE_READING && !is_error(code)) {
		file >> code;
		file >> time_pt;
		//is type
		if(code == READ_PATTERN_5_START_GET_ATTRS) {
			type = true;
		}
		else if(is_op(code)) {
			ops(file, code, time_pt, output, my_start_time);
		}
	}
	if(code == READ_PATTERN_5_DONE_READING) {
		if(type) {
			output.read_pattern_5_type_times.push_back(time_pt - start_time);
		}
		else {
			output.read_pattern_5_times.push_back(time_pt - start_time);
		}	
	}
	else {
		error_log << "error. pattern 5 end time code never appeared after start time code. Instead code " << code << " appeared \n";
	}	
}

//total pattern 6 time: 608-600
static void read_pattern_6(ifstream &file, double start_time, struct client_config_output &output, long double my_start_time) {

	uint16_t code = READ_PATTERN_6_START;
	double time_pt;
	bool type = false;

	while(code != READ_PATTERN_6_DONE_READING && !is_error(code)) {
		file >> code;
		file >> time_pt;
		//is type
		if(code == READ_PATTERN_6_START_GET_ATTRS) {
			type = true;
		}
		else if(is_op(code)) {
			ops(file, code, time_pt, output, my_start_time);
		}
	}
	if(code == READ_PATTERN_6_DONE_READING) {
		if(type) {
			output.read_pattern_6_type_times.push_back(time_pt - start_time);
		}
		else {
			output.read_pattern_6_times.push_back(time_pt - start_time);
		}	
	}
	else {
		error_log << "error. pattern 6 end time code never appeared after start time code. Instead code " << code << " appeared \n";
	}		
}

static void ops(ifstream &file, uint16_t code, double start_time, struct client_config_output &output, long double my_start_time) {
	double time_pt;
	uint16_t serialze_code;
	uint16_t send_code;
	uint16_t rec_return_msg_code;
	uint16_t end_code;
	vector<vector<double>> *op_piece_times;

	double serialize_done_time;
	double send_done_time;
	double rec_return_msg_done_time;
	double end_time;

	switch (code) {
		//read until code = code + 8
		//for each op: total time = XY00-XY08
		case MD_ACTIVATE_VAR_START:
			serialze_code = OP_ACTIVATE_VAR_SERIALIZE_MSG_FOR_SERVER;
			send_code = OP_ACTIVATE_VAR_SEND_MSG_TO_SERVER;
			rec_return_msg_code = OP_ACTIVATE_VAR_RETURN_MSG_RECEIVED_FROM_SERVER;
			end_code = MD_ACTIVATE_VAR_DEARCHIVE_RESPONSE_OP_DONE;
			op_piece_times = &output.activate_var_times;	
			extreme_debug_log << "just assigned op type as activate var" << endl;
			break;

		case MD_CATALOG_VAR_START:
			serialze_code = OP_CATALOG_VAR_SERIALIZE_MSG_FOR_SERVER;
			send_code = OP_CATALOG_VAR_SEND_MSG_TO_SERVER;
			rec_return_msg_code = OP_CATALOG_VAR_RETURN_MSG_RECEIVED_FROM_SERVER;
			end_code = MD_CATALOG_VAR_DEARCHIVE_RESPONSE_OP_DONE;
			op_piece_times = &output.catalog_var_times;
			extreme_debug_log << "just assigned op type as catalog " << endl;
			break;

		case MD_CREATE_VAR_START:
			serialze_code = OP_CREATE_VAR_SERIALIZE_MSG_FOR_SERVER;
			send_code = OP_CREATE_VAR_SEND_MSG_TO_SERVER;
			rec_return_msg_code = OP_CREATE_VAR_RETURN_MSG_RECEIVED_FROM_SERVER;
			end_code = MD_CREATE_VAR_DEARCHIVE_RESPONSE_OP_DONE;
			op_piece_times = &output.create_var_times;
			extreme_debug_log << "just assigned op type as create " << endl;
			break;

		case MD_GET_CHUNK_LIST_START:
			serialze_code = OP_GET_CHUNK_LIST_SERIALIZE_MSG_FOR_SERVER;
			send_code = OP_GET_CHUNK_LIST_SEND_MSG_TO_SERVER;
			rec_return_msg_code = OP_GET_CHUNK_LIST_RETURN_MSG_RECEIVED_FROM_SERVER;
			end_code = MD_GET_CHUNK_LIST_DEARCHIVE_RESPONSE_OP_DONE;
			op_piece_times = &output.get_chunk_list_times;
			extreme_debug_log << "just assigned op type as get chunk list " << endl;
			break;

		case MD_GET_CHUNK_START:
			serialze_code = OP_GET_CHUNK_SERIALIZE_MSG_FOR_SERVER;
			send_code = OP_GET_CHUNK_SEND_MSG_TO_SERVER;
			rec_return_msg_code = OP_GET_CHUNK_RETURN_MSG_RECEIVED_FROM_SERVER;
			end_code = MD_GET_CHUNK_DEARCHIVE_RESPONSE_OP_DONE;
			op_piece_times = &output.get_chunk_times;
			extreme_debug_log << "just assigned op type as get chunk " << endl;
			break;

		case MD_INSERT_CHUNK_START:
			serialze_code = OP_INSERT_CHUNK_SERIALIZE_MSG_FOR_SERVER;
			send_code = OP_INSERT_CHUNK_SEND_MSG_TO_SERVER;
			rec_return_msg_code = OP_INSERT_CHUNK_RETURN_MSG_RECEIVED_FROM_SERVER;
			end_code = MD_INSERT_CHUNK_DEARCHIVE_RESPONSE_OP_DONE;
			op_piece_times = &output.insert_chunk_times;
			extreme_debug_log << "just assigned op type as insert chunk " << endl;
			break;

		case MD_PROCESSING_VAR_START:
			serialze_code = OP_PROCESSING_VAR_SERIALIZE_MSG_FOR_SERVER;
			send_code = OP_PROCESSING_VAR_SEND_MSG_TO_SERVER;
			rec_return_msg_code = OP_PROCESSING_VAR_RETURN_MSG_RECEIVED_FROM_SERVER;
			end_code = MD_PROCESSING_VAR_DEARCHIVE_RESPONSE_OP_DONE;
			op_piece_times = &output.processing_var_times;
			extreme_debug_log << "just assigned op type as process var " << endl;
			break;

		case MD_ACTIVATE_TYPE_START:
			serialze_code = OP_ACTIVATE_TYPE_SERIALIZE_MSG_FOR_SERVER;
			send_code = OP_ACTIVATE_TYPE_SEND_MSG_TO_SERVER;
			rec_return_msg_code = OP_ACTIVATE_TYPE_RETURN_MSG_RECEIVED_FROM_SERVER;
			end_code = MD_ACTIVATE_TYPE_DEARCHIVE_RESPONSE_OP_DONE;
			op_piece_times = &output.activate_type_times;
			extreme_debug_log << "just assigned op type as activate type " << endl;
			break;

		case MD_CATALOG_TYPE_START:
			serialze_code = OP_CATALOG_TYPE_SERIALIZE_MSG_FOR_SERVER;
			send_code = OP_CATALOG_TYPE_SEND_MSG_TO_SERVER;
			rec_return_msg_code = OP_CATALOG_TYPE_RETURN_MSG_RECEIVED_FROM_SERVER;			
			end_code = MD_CATALOG_TYPE_DEARCHIVE_RESPONSE_OP_DONE;
			op_piece_times = &output.catalog_type_times;
			extreme_debug_log << "just assigned op type as catalog type " << endl;
			break;

		case MD_CREATE_TYPE_START:
			serialze_code = OP_CREATE_TYPE_SERIALIZE_MSG_FOR_SERVER;
			send_code = OP_CREATE_TYPE_SEND_MSG_TO_SERVER;
			rec_return_msg_code = OP_CREATE_TYPE_RETURN_MSG_RECEIVED_FROM_SERVER;
			end_code = MD_CREATE_TYPE_DEARCHIVE_RESPONSE_OP_DONE;
			op_piece_times = &output.create_type_times;
			extreme_debug_log << "just assigned op type as create type " << endl;
			break;

		case MD_GET_ATTR_LIST_START:
			serialze_code = OP_GET_ATTR_LIST_SERIALIZE_MSG_FOR_SERVER;
			send_code = OP_GET_ATTR_LIST_SEND_MSG_TO_SERVER;
			rec_return_msg_code = OP_GET_ATTR_LIST_RETURN_MSG_RECEIVED_FROM_SERVER;
			end_code = MD_GET_ATTR_LIST_DEARCHIVE_RESPONSE_OP_DONE;
			op_piece_times = &output.get_attr_list_times;
			extreme_debug_log << "just assigned op type as get attr list " << endl;
			break;

		case MD_GET_ATTR_START:
			serialze_code = OP_GET_ATTR_SERIALIZE_MSG_FOR_SERVER;
			send_code = OP_GET_ATTR_SEND_MSG_TO_SERVER;
			rec_return_msg_code = OP_GET_ATTR_RETURN_MSG_RECEIVED_FROM_SERVER;
			end_code = MD_GET_ATTR_DEARCHIVE_RESPONSE_OP_DONE;
			op_piece_times = &output.get_attr_times;
			extreme_debug_log << "just assigned op type as get attr " << endl;
			break;

		case MD_INSERT_ATTR_START:
			serialze_code = OP_INSERT_ATTR_SERIALIZE_MSG_FOR_SERVER;
			send_code = OP_INSERT_ATTR_SEND_MSG_TO_SERVER;
			rec_return_msg_code = OP_INSERT_ATTR_RETURN_MSG_RECEIVED_FROM_SERVER;
			end_code = MD_INSERT_ATTR_DEARCHIVE_RESPONSE_OP_DONE;
			op_piece_times = &output.insert_attr_times;
			extreme_debug_log << "just assigned op type as insert attr " << endl;
			break;	

		case MD_PROCESSING_TYPE_START:
			serialze_code = OP_PROCESSING_TYPE_SERIALIZE_MSG_FOR_SERVER;
			send_code = OP_PROCESSING_TYPE_SEND_MSG_TO_SERVER;
			rec_return_msg_code = OP_PROCESSING_TYPE_RETURN_MSG_RECEIVED_FROM_SERVER;
			end_code = MD_PROCESSING_TYPE_DEARCHIVE_RESPONSE_OP_DONE;
			op_piece_times = &output.processing_type_times;
			extreme_debug_log << "just assigned op type as processing type " << endl;
			break;

		case MD_FULL_SHUTDOWN_START:
			end_code = MD_FULL_SHUTDOWN_OP_DONE;
			op_piece_times = &output.shutdown_times;
			extreme_debug_log << "just assigned op type as shutdown " << endl;
			break;

		default: //note - many ops fall in this "other" category for now
			error_log << "error. op code: " << to_string(code) << " did not match an op start code \n";
			return;
	}
	extreme_debug_log << "end_code: " << to_string(end_code) << " note this is in extreme debug logging" << endl;
	// extreme_debug_log << "am debug logging \n";
	while(code < end_code) {
		file >> code;
		file >> time_pt;
		if(code == serialze_code) {
			serialize_done_time = time_pt;
		}
		else if(code == send_code) {
			send_done_time = time_pt;
		}
		else if(code == rec_return_msg_code) {
			rec_return_msg_done_time = time_pt;
		}
	}	
	if (code == end_code) {
		end_time = time_pt;
		if(end_code != MD_FULL_SHUTDOWN_OP_DONE) {
			op_piece_times->at(0).push_back(serialize_done_time-start_time);
			op_piece_times->at(1).push_back(send_done_time-serialize_done_time);
			op_piece_times->at(2).push_back(rec_return_msg_done_time-send_done_time);
			op_piece_times->at(3).push_back(end_time-rec_return_msg_done_time);
			op_piece_times->at(4).push_back(end_time-start_time);
		}
		else {
			op_piece_times->at(4).push_back(end_time-start_time);
		}
		//note -> need to fix this so time_pts are vector of vector of vector, should init them to number of pieces, then acces them and not just do push back

	}
	else {
		error_log << "error. " << to_string(end_code) << " end time code never appeared after start time code. Instead code " << to_string(code) << " appeared \n";
	}			
}

static void shutdown(ifstream &file, int rank, uint16_t code, struct client_config_output &output, long double my_start_time) {
	double time_pt;
	double start_time;
	uint16_t num_server_procs = output.config.num_server_procs;

	file >> start_time;
	if(code == READING_DONE_FOR_ALL_PROCS_START_CLEANUP && rank >= num_server_procs) {
		return;
	}
	//fix - do I want to do anything with this?
	while(is_shutdown(code) && !file.eof()){
		file >> code;
		file >> time_pt;
		if(code == SERVER_SHUTDOWN_DONE && rank !=0) {
			debug_log << "rank: " << rank << " and got to server shutdown done " << endl;
			break;
		}
		else if(is_op(code)) {
			ops(file, code, time_pt, output, my_start_time);
		}
	}
}

static int evaluate_all_clock_times_client(ifstream &file, struct client_config_output &output, vector<long double> &my_start_times) {
	long double time_pt;	
	bool new_proc;
	uint16_t num_client_procs = output.config.num_client_procs;
	uint16_t num_clock_pts = output.config.num_clock_pts;
	uint16_t num_server_procs = output.config.num_server_procs;

	extreme_debug_log << "num_client_procs: " << num_client_procs << endl;
	extreme_debug_log << "num_server_procs: " << num_server_procs << endl;

	long double earliest_start = 1000000;
	long double earliest_init_done = 1000000;
	long double earliest_write_done = 1000000;
	long double earliest_read_pattern_done = 1000000;
	long double latest_init_done = 0;
	long double latest_write_done = 0;
	long double latest_read_pattern_done = 0;
	long double latest_extra_testing_done = 0;

	long double write_start_time;
	long double write_end_time;
	long double write_time;
	long double longest_write_time = 0;

	for(int rank=0; rank<num_client_procs; rank++) {
		uint16_t clock_catg = 0;

		// extreme_debug_log << "i: " << i << endl;
		file >> time_pt;
		extreme_debug_log << "time_pt: " << time_pt << endl;

    //start:0, mpi_init_done:1, register_ops_done:2, dirman_init_done, server_setup_done, write_var_and_type_end, write_end, pattern_read_end, read_end;

		while(time_pt != CLOCK_TIMES_NEW_PROC && time_pt != CLOCK_TIMES_DONE) {
			output.all_clock_times.at(clock_catg).push_back(time_pt);
			switch(clock_catg) {
				case 0: //start
					if(time_pt < earliest_start) {
						earliest_start = time_pt;
					}
					my_start_times.push_back(time_pt);
					break;
				case 4: //server setup done
					write_start_time = time_pt;
					if(time_pt < earliest_init_done) {
						earliest_init_done = time_pt;
					}
					if(time_pt > latest_init_done) {
						extreme_debug_log << "time_pt: " << time_pt << endl;
						latest_init_done = time_pt;
					}
					break;
				case 6: //writing insert done
					write_end_time = time_pt;
					if(time_pt < earliest_write_done) {
						earliest_write_done = time_pt;
					}
					if(time_pt > latest_write_done) {
						latest_write_done = time_pt;
					}
					break;
				case 7: //read pattern done
					if(time_pt < earliest_read_pattern_done) {
						earliest_read_pattern_done = time_pt;
					}
					if(time_pt > latest_read_pattern_done) {
						latest_read_pattern_done = time_pt;
					}
					break;
				case 8: //extra testing done
					if(rank < num_server_procs && time_pt > latest_extra_testing_done) {
						latest_extra_testing_done = time_pt; 
					}
					break;
			}
			file >> time_pt;
			extreme_debug_log << "time_pt: " << time_pt << endl;

			clock_catg++;
		}
		write_time = write_end_time - write_start_time;
		if(write_time > longest_write_time) {
			longest_write_time = write_time;
		}
		// output.all_clock_times.push_back(clock_times);
	}
	long double earliest_write_start = earliest_init_done;
	long double earliest_read_pattern_start = earliest_write_done;
	long double earliest_extra_testing_start = earliest_read_pattern_done;
// (0) total init time (0:start 0> 4server setup done)
// (1) total write time (4:server setup done->6 writing insert done)
// (2) total read pattern time (6 writing insert done -> 7 read patterns done)
// (3) total read time (6 writing insert done -> 8 extra testing done)
// (4) total run time without extra (0:start -> 7: read patterns done)
// (5) total run time with extra (0:start -> 8: extra testing done)
	extreme_debug_log << "earliest start: " << earliest_start << endl;
	extreme_debug_log << "earliest_init_done: " << earliest_init_done << endl;
	extreme_debug_log << "earliest_write_done: " << earliest_write_done << endl;
	extreme_debug_log << "earliest_read_pattern_done: " << earliest_read_pattern_done << endl;
	extreme_debug_log << "latest_init_done: " << latest_init_done << endl;
	extreme_debug_log << "latest_write_done: " << latest_write_done << endl;
	extreme_debug_log << "latest_read_pattern_done: " << latest_read_pattern_done << endl;
	extreme_debug_log << "latest_extra_testing_done: " << latest_extra_testing_done << endl;

	output.clock_times_eval.at(0).push_back(latest_init_done-earliest_start);
	// output.clock_times_eval.at(1).push_back(latest_write_done-earliest_init_done);
	output.clock_times_eval.at(1).push_back(longest_write_time);
	output.clock_times_eval.at(2).push_back(latest_read_pattern_done-latest_write_done);
	if(output.config.num_types > 0) {
		output.clock_times_eval.at(3).push_back(latest_extra_testing_done-earliest_write_done);
	}
	output.clock_times_eval.at(4).push_back(latest_read_pattern_done-earliest_start);
	if(output.config.num_types > 0) {
		output.clock_times_eval.at(5).push_back(latest_extra_testing_done-earliest_start);
	}

	debug_log << "last time_pt: " << time_pt << endl;
	return RC_OK;
}



