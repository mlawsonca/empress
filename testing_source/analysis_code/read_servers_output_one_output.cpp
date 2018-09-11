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

static debugLog error_log = debugLog(error_logging);
static debugLog debug_log = debugLog(debug_logging);
static debugLog extreme_debug_log = debugLog(extreme_debug_logging);

static void init(ifstream &file, uint16_t code, struct server_config_output &output, double &start_time, double &init_done);
static void ops(ifstream &file, uint16_t code, struct server_config_output &output);
static int shutdown(ifstream &file, uint16_t code, struct server_config_output &output, double &start_time, double &shutdown_done);

static int evaluate_clock_times_server(ifstream &file, struct server_config_output &output);
static int evaluate_storage_sizes(ifstream &file, struct server_config_output &output);
static int evaluate_db_interaction_server(ifstream &file, struct server_config_output &output, uint16_t code);

static const uint16_t FIRST_ERR_CODE = ERR_ARGC;
static const uint16_t LAST_INIT_CODE = DIRMAN_SETUP_DONE_INIT_DONE;
static const uint16_t FIRST_OP_CODE = OP_ACTIVATE_START;
static const uint16_t LAST_TIMING_CODE = DB_CLOSED_SHUTDOWN_DONE; 


static bool is_error(uint16_t code) {
	return code >= FIRST_ERR_CODE;
}

static bool is_op(uint16_t code) {
	return (FIRST_OP_CODE <= code && code < FIRST_ERR_CODE);
}

static bool is_db_load(uint16_t code) {
	return ( (DB_LOAD_START <= code && code <= DB_LOAD_DONE)); 
}

static bool is_init(uint16_t code) {
	return ( (PROGRAM_START <= code && code < LAST_INIT_CODE) || is_db_load(code) ); 
}

static bool is_db_output(uint16_t code) {
	return ( (DB_OUTPUT_START <= code && code <= DB_OUTPUT_DONE)); 
}

static bool is_shutdown(uint16_t code) {
	return ( (SHUTDOWN_START <= code && code < DB_CLOSED_SHUTDOWN_DONE) || is_db_output(code)); 
}


static bool is_db_load_or_output(uint16_t code) {
	return ( (DB_LOAD_START <= code && code <= DB_OUTPUT_DONE)); 
}



static long double adjust(long double start_time, long double end_time) {
	// if (end_time < start_time) {
	// 	return (end_time + 3600 - start_time);
	// 	cout << "adjusting: start_time: " << start_time << " end_time: " << end_time << endl;
	// }
	// else {
		return end_time - start_time;
	// }
}

int evaluate_all_servers(const string &file_path, struct server_config_output &output) {
	
	uint16_t code;
	int rc = RC_OK;
	ifstream file;
	int rank = -1;

	double start_time;
	double init_done;
	double shutdown_start;
	double shutdown_done;

	extreme_debug_log << "got to the top of evaluate_all_servers \n";

	file.open(file_path);
	if(!file) {
		error_log << "error. failed to open file " << file_path << endl;
		return RC_ERR;
	}
 //   	std::string line; 
 //   	int i = 0;			
 //    while (i < output.config.num_server_procs) { 	
 //    	std::getline(file, line);
	//     // if( line.find("<ERROR> [ibverbs_transport]") == std::string::npos) { 			
	//     //     error_log << "line " << i << " for file path " << file_path << " does not contain <ERROR> [ibverbs_transport]. " <<
	//     //     	"the line is: " << line << endl;
 // 	   //  } 	
	//     i += 1;
	// }		

	while (file >> code) { 
		if(is_op(code)) {
			extreme_debug_log << "is op - code at top of while loop: " << to_string(code) << endl;
		}
		else { 
			debug_log << "is NOT op - code at top of while loop: " << to_string(code) << endl;
		}

		if(is_error(code)) {
			error_log << "error from in while loop of code: " << code << endl;
			// return 1;
		} 
		else if (code == DB_SIZES) {
			extreme_debug_log << "about to evaluate_storage_sizes"<< endl;
	
		    rc = evaluate_storage_sizes(file, output);
		    if(rc != RC_OK) {
		    	error_log << "server storage size error \n";
		    	goto cleanup;
		    }
		   	extreme_debug_log << "done with evaluate_storage_sizes"<< endl;
		}
		else if (code == CLOCK_TIMES_NEW_PROC) {
			extreme_debug_log << "about to evaluate_clock_times_server"<< endl;

			rc = evaluate_clock_times_server(file, output);
			if(rc != RC_OK) {
				error_log << "server clock pts error \n";
				goto cleanup;
			}
			extreme_debug_log << "done with evaluate_clock_times_server"<< endl;

		}
		//proc is initing
		//all set up done at 4
		else if(PROGRAM_START <= code && code <= LAST_INIT_CODE ) { 
			debug_log << "initing" << endl;
			//new proc
			rank +=1;
			init(file, code, output, start_time, init_done);
			debug_log << "done initing" << endl;
			
		}
		else if(LAST_INIT_CODE < code && code <= LAST_TIMING_CODE) {
			debug_log << "shutdown " << endl;
			shutdown(file, code, output, shutdown_start, shutdown_done);

			output.run_times.push_back(shutdown_start - init_done);
			output.total_run_times.push_back(shutdown_done - start_time);

			debug_log << "done with shutdown" << endl;
	
			// if(rank == num_server_procs-1) {
			// 	break;
			// }
			// file >> code;
			// error_log << "at end of shutdown again, code: " << code << endl;
			// return rc;
		}
		else if(is_op(code)) {
			extreme_debug_log << "about to ops"<< endl;

			ops(file, code, output);			

			extreme_debug_log << "done with ops"<< endl;

		}
		else if(is_db_load_or_output(code)) {
			rc = evaluate_db_interaction_server(file, output, code);

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
static void init(ifstream &file, uint16_t code, struct server_config_output &output, double &start_time, double &init_done) {
	double time_pt;

	if(code == PROGRAM_START) {
		file >> start_time; 
	}
	else {
		error_log << "error, first init code wasn't program start. It was " << code << endl;
		return;
	}

    double mpi_init_done_time;
    double register_ops_done_time;
    double db_setup_done_time;

	//note - as is the received code is Program_start
	while(is_init(code)) { 
		file >> code;
		file >> time_pt;				
		extreme_debug_log << "init code: "  << to_string(code) << endl;
		switch(code) {
			case MPI_INIT_DONE:
				output.init_times.at(0).push_back(time_pt - start_time);
				mpi_init_done_time = time_pt;
				break;

			case REGISTER_OPS_DONE:
				output.init_times.at(1).push_back(time_pt - mpi_init_done_time);
				register_ops_done_time = time_pt;
				break;

			case DB_SETUP_DONE:
				output.init_times.at(2).push_back(time_pt - register_ops_done_time);
				db_setup_done_time = time_pt;
				break;

		}
	}
	if(code == DIRMAN_SETUP_DONE_INIT_DONE) {
		output.init_times.at(3).push_back(time_pt - db_setup_done_time);
		output.init_times.at(4).push_back(time_pt - start_time);
		init_done = time_pt; 		
	}

	else if (is_error(code)) {
		error_log << "error code " << code << " received " << endl;
	}
	else {
		error_log << "error. code " << code << " received instead of init_done \n";
	}
}


static void ops(ifstream &file, uint16_t code, struct server_config_output &output) {
	double start_time;
	file >> start_time;
	double time_pt;

	uint16_t deserialze_code = code + 1;
	uint16_t db_code = code + 2;
	uint16_t serialize_code = code + 3;
	uint16_t create_msg_code = code + 4;
	uint16_t end_code = code + 5;

	double deserialize_done_time;
	double db_done_time;
	double serialize_done_time;
	double create_msg_done_time;
	double end_time;


	while(code < end_code && file >> code) {
		file >> time_pt;
		extreme_debug_log << "code: " << code << endl;
		if(code == deserialze_code) {
			deserialize_done_time = time_pt;
		}
		else if(code == db_code) {
			db_done_time = time_pt;
		}
		else if(code == serialize_code) {
			serialize_done_time = time_pt;
		}
		else if(code == create_msg_code) {
			create_msg_done_time = time_pt;
		}
	}	
	int op_indx = (code - 1000) / 100;
	if (code == end_code) {
		end_time = time_pt;
		output.op_times[op_indx].at(0).push_back(deserialize_done_time-start_time);
		output.op_times[op_indx].at(1).push_back(db_done_time-deserialize_done_time);
		output.op_times[op_indx].at(2).push_back(serialize_done_time-db_done_time);
		output.op_times[op_indx].at(3).push_back(create_msg_done_time-serialize_done_time);
		output.op_times[op_indx].at(4).push_back(end_time-db_done_time);
		output.op_times[op_indx].at(5).push_back(end_time-start_time);
	}
	else {
		error_log << "error. " << end_code << " end time code never appeared after start time code. Instead code " << code << " appeared \n";
	}			
}

static int shutdown(ifstream &file, uint16_t code, struct server_config_output &output, double &start_time, double &shutdown_done) {
	uint16_t num_server_procs = output.config.num_server_procs;

	if(code != SHUTDOWN_START) {
		error_log << "error. first shutdown code was " << code << " instead of SHUTDOWN_START" << endl;
		return RC_ERR;
	}
	double time_pt;
	file >> start_time;
	debug_log << "start_time: " << start_time << endl;
	//todo - do I want to do anything with this?
	while(is_shutdown(code) && !file.eof()){
		file >> code;
		file >> time_pt;
		extreme_debug_log << "code: " << code << endl;
	}
	if(code != DB_CLOSED_SHUTDOWN_DONE) {
		error_log << "error. last shutdown code was " << code << " instead of DB_CLOSED_SHUTDOWN_DONE" << endl;
		return RC_ERR;
	}
	shutdown_done = time_pt;
	output.shutdown_times.push_back(shutdown_done - start_time);
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

static int evaluate_clock_times_server(ifstream &file, struct server_config_output &output) {
	uint16_t num_clock_pts = output.config.num_clock_pts;
	uint16_t num_server_procs = output.config.num_server_procs;

	long double time_pt;
	uint16_t code = CLOCK_TIMES_NEW_PROC;
	long double earliest_start = 1000000;
	long double latest_init_done = 0;
	long double latest_shutdown_done = 0;
	long double earliest_init_done = 1000000;


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
			}	
			else if(j == (num_clock_pts-2) ) {
				if (time_pt > latest_init_done) {
				//the last time point is finalize
					latest_init_done = time_pt;
				}
				if(time_pt < earliest_init_done) {
					earliest_init_done = time_pt;
				}
			}
			else if(j == (num_clock_pts-1) && time_pt > latest_shutdown_done) {
				//the last time point is finalize
				latest_shutdown_done = time_pt;
			}
			extreme_debug_log << "time_pt: " << time_pt << endl;
			output.all_clock_times.at(j).push_back(time_pt);
		}
	}
	output.clock_times_eval.at(0).push_back(adjust(earliest_start,latest_init_done));
	output.clock_times_eval.at(1).push_back(adjust(earliest_init_done,latest_shutdown_done));	
	output.clock_times_eval.at(2).push_back(adjust(earliest_start,latest_shutdown_done));	

	file >> code;
	if(code != CLOCK_TIMES_DONE) {
		error_log << "error. last server clock code was " << code << " instead of CLOCK_TIMES_DONE" << endl;
		return RC_ERR;
	}
	debug_log << "last time_pt: " << time_pt << endl;
	return RC_OK;
}

static int evaluate_db_interaction_server(ifstream &file, struct server_config_output &output, uint16_t code) {
	double start_pt, end_pt;
	file >> start_pt;

	if(code == DB_LOAD_START) {
		file >> code;
		if(code == DB_LOAD_DONE) {
			file >> end_pt;
			output.db_load_times.push_back(end_pt-start_pt);
		}
		else {
			error_log << "error. was expecting DB_LOAD_DONE but instead saw " << code << endl;
			return RC_ERR;
		}

	}
	else if(code == DB_OUTPUT_START) {
		file >> code;
		if(code == DB_OUTPUT_DONE) {
			file >> end_pt;
			output.db_output_times.push_back(end_pt-start_pt);
		}
		else {
			error_log << "error. was expecting DB_OUTPUT_DONE but instead saw " << code << endl;
			return RC_ERR;
		}

	}
	else {
		error_log << "error. was expecting DB start pt but instead saw " << code << endl;
		return RC_ERR;
	}
	return RC_OK;
}
