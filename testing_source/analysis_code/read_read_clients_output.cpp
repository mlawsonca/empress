
#include "../../include/client/md_client_timing_constants.hh"
#include "../../include/client/client_timing_constants_read.hh"
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <iomanip> //set precision
// #include <limits> //numeric_limits
#include <map>
#include "read_testing_output_one_output.hh"
#include <numeric> //for accumulate function

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

static void init(ifstream &file, uint16_t code, struct read_client_config_output &output);
static void read(ifstream &file, uint16_t code, int rank, struct read_client_config_output &output);
static void read_pattern(ifstream &file, int pattern, uint16_t start_code, 
		long double start_time, uint16_t end_code, struct read_client_config_output &output,
		bool type_pattern);
static void ops(ifstream &file, uint16_t code, long double start_time, struct read_client_config_output &output);
static void collective_ops(ifstream &file, uint16_t code, long double start_time, struct read_client_config_output &output);
static void shutdown(ifstream &file, int rank, uint16_t code, struct read_client_config_output &output);

// static int evaluate_all_clock_times_client(ifstream &file, struct read_client_config_output &output);
void add_last_first_timing(read_client_config_output &output);
void add_earliest_start(uint16_t code, long double start_time, read_client_config_output &output);
void add_latest_finish(uint16_t code, long double end_time, read_client_config_output &output);

static const uint16_t FIRST_ERR_CODE = ERR_ARGC;
static const uint16_t FIRST_OP_CODE = MD_ACTIVATE_START;
static const uint16_t FIRST_READ_PATTERN_CODE = READ_PATTERN_1_START;
static const uint16_t LAST_TIMING_CODE = DIRMAN_SHUTDOWN_DONE;

static const uint16_t FIRST_GATHER_CODE = GATHER_ATTR_ENTRIES_START;

// static const uint16_t FIRST_CODE_AFTER_OPS = BOUNDING_BOX_TO_OBJ_NAMES;
static const uint16_t FIRST_CODE_AFTER_OPS = BOUNDING_BOX_TO_OBJ_NAMES_START;

static bool is_error(uint16_t code) {
	return code >= FIRST_ERR_CODE;
}

static bool is_gather(uint16_t code) {
	return (FIRST_GATHER_CODE <= code && code < FIRST_OP_CODE);
}

static bool is_collective_read(uint16_t code) {
	return ( (code % 100 == 30) || (code % 100 == 31) || 
		(code % 100 == 40) || (code % 100 == 41) ||
		(code % 100 == 50) || (code % 100 == 51) );
}

static bool is_op(uint16_t code) {
	return (FIRST_OP_CODE <= code && code < FIRST_CODE_AFTER_OPS);
}

static bool is_obj_name(uint16_t code) {
	return (code == BOUNDING_BOX_TO_OBJ_NAMES);
}
static bool is_objector_start(uint16_t code) {
	return ( code == BOUNDING_BOX_TO_OBJ_NAMES_START );
}
static bool is_objector_done(uint16_t code) {
	return ( code == BOUNDING_BOX_TO_OBJ_NAMES_DONE );
}

static bool is_bounding_box(uint16_t code) {
	return (is_obj_name(code) || is_objector_start(code) || is_objector_done(code));
}

static bool is_gather_or_op(uint16_t code) {
	return (is_op(code) || is_gather(code) );
}

static bool is_gather_start(uint16_t code) {
	return (is_gather(code) && (code % 2 == 0) );
}

// static bool is_gather_done(uint16_t code) {
// 	return (code % 2 == 1);
// }

static bool is_read_pattern(uint16_t code) {
	return( FIRST_READ_PATTERN_CODE <= code && code < FIRST_OP_CODE );
}

static bool is_read_op(uint16_t code) {
	return ( (FIRST_OP_CODE < code && code < FIRST_ERR_CODE) );
}

static bool is_init(uint16_t code) {
	return ( (PROGRAM_START <= code && code < SERVER_SETUP_DONE_INIT_DONE) );
}

static bool is_reading(uint16_t code) {
	return ( (READING_START <= code && code < READING_DONE) || is_gather_or_op(code) 
			   || is_read_pattern(code) || is_bounding_box(code) || is_read_op(code));
}

static bool is_shutdown(uint16_t code) {
	return ( (READING_DONE <= code && code < LAST_TIMING_CODE) || is_op(code));
}

static bool is_hdf5(uint16_t code) {
	return ( (READ_DATA_FOR_ATTR_START <= code && code <= HDF5_READ_DATA_FOR_ATTRS_DONE) );
}

static bool is_hdf5_start(uint16_t code) {
	return ( code == HDF5_READ_DATA_FOR_ATTRS_START );
}

static bool is_hdf5_done(uint16_t code) {
	return ( code == HDF5_READ_DATA_FOR_ATTRS_DONE );
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



int evaluate_all_read_clients(const string &file_path, struct read_client_config_output &output) {
		
	uint16_t code;
	int rc = RC_OK;
	ifstream file;
	int rank = -1;

	// vector<long double> all_clock_times;

	file.open(file_path);
	if(!file) {
		error_log << "error. failed to open file " << file_path << endl;
		return RC_ERR;
	}

	//igore the outputted objector params 			
    //don't start reading until the timing information begins 			
    std::string line; 			
    while (std::getline(file, line)) 			
    { 			
        if( line.find("begin timing output") != std::string::npos) { 			
            break; 			
        } 			
    }

	while (file >> code) { 
		debug_log << "code at top of while loop: " << to_string(code) << endl;

		if(is_error(code)) {
			error_log << "error from in while loop of code: " << code << endl;
			// return 1;
		} 
		// else if(code == CLOCK_TIMES_NEW_PROC) {
		// 	debug_log << "code in eval all read clients: " << to_string(code) << endl;
		// 	rc = evaluate_all_clock_times_client(file, output);
		// 	if(rc != RC_OK) {
		// 		error_log << "client clock pts error \n";
		// 		goto cleanup;
		// 	}
		// }
		//proc is initing
		//all set up done at 4
		else if(PROGRAM_START <= code && code < READING_START ) { 
			debug_log << "initing" << endl;
			//new proc
			rank +=1;
			init(file, code, output);			
		}
		//reading
		//reading time: reading_done - reading_start
		//total reading time: reading_done_for_all_procs_start_cleanup - earliest reading start
		else if(READING_START <= code && code < READING_DONE) {
			debug_log << "reading " << endl;
			read(file, code, rank, output);
		}

		else if(READING_DONE <= code && code <= LAST_TIMING_CODE) {
			debug_log << "shutdown " << endl;
			shutdown(file, rank, code, output);
		}
		else {
			error_log << "error. code " << code << " didn't fall into the category of error, or starting init, writing, or reading \n";
			return RC_ERR;
		}
	}


cleanup:
	add_last_first_timing(output);

	file.close();
	return rc;
}

static void init(ifstream &file, uint16_t code, struct read_client_config_output &output) {
	long double start_time;
	long double time_pt;

	long double mpi_init_done_time;
	long double md_client_init_done_time;
	long double init_var_done_time;
	long double dirman_setup_done_time;

	if(code == PROGRAM_START) {
		file >> start_time;
		add_earliest_start(code, start_time, output);
	}
	else {
		error_log << "error, first init code wasn't program start. It was " << code << endl;
		return;
	}
	//note - as is the received code is Program_start
	while(is_init(code)) { 
		file >> code;
		file >> time_pt;				
		extreme_debug_log << "init code: "  << to_string(code) << endl;
		switch(code) {
			case MPI_INIT_DONE:
				output.init_times.at(0).push_back(adjust(start_time, time_pt)) ;
				mpi_init_done_time = time_pt;
				break;

			case MD_CLIENT_INIT_DONE:
				output.init_times.at(1).push_back(adjust(mpi_init_done_time,time_pt));
				md_client_init_done_time = time_pt;
				break;

			case INIT_VARS_DONE:
				output.init_times.at(2).push_back(adjust(md_client_init_done_time, time_pt));
				init_var_done_time = time_pt;
				break;

			case DIRMAN_SETUP_DONE:
				output.init_times.at(3).push_back(adjust(init_var_done_time,time_pt));
				dirman_setup_done_time = time_pt;
				break;
		}
	}
	if(code == SERVER_SETUP_DONE_INIT_DONE) {
		//init time: time_pt - start_time 
		output.init_times.at(4).push_back(adjust(dirman_setup_done_time,time_pt));
		output.init_times.at(5).push_back(adjust(start_time,time_pt));
	
		add_latest_finish(PROGRAM_START, time_pt, output);
	}
	else if (is_error(code)) {
		error_log << "error code " << code << " received " << endl;
	}
	else {
		error_log << "error. code " << code << " received instead of init_done \n";
	}
}

static void read_init(ifstream &file, uint16_t code, int rank, struct read_client_config_output &output) {
	long double time_pt;
	uint16_t start_code = code;

	while(!file.eof() && is_reading(code) && code != FIND_VARS_DONE) {
		file >> code;
		file >> time_pt;
		// md_log("time_pt: " + to_string(time_pt));
		//read patterns
		// md_log("in reading, code: " + to_string(code));
		if(READING_START <= code && code < FIRST_READ_PATTERN_CODE) {
			switch (code) {
				//fix
				// case BCAST_CATALOGS_DONE:
				// 	extreme_debug_log << "done bcasting catalog" << endl; 
				// 	// output.init_times.push_back(time_pt-read_start);
				// 	reading_patterns_start = time_pt;
				// 	break;
				
			}
		}
		else if (is_op(code)) {
			if (is_collective_read(code)) {
				collective_ops(file, code, time_pt, output);
			}
			else {
				ops(file, code, time_pt, output);
			}
		}
		// else if (is_obj_name(code)) { 	
		//     //fix 
		//     cout << "read init. code is_obj_name " << endl;			
		// } 
		else if (code == CATALOG_RUN_DONE) {
		}
		else if(is_error(code)) {
			error_log << "there was an error code: " << code << " for time_pt: " << time_pt << endl;
			// return;
		}
		else {
			error_log << "error. received code " << code << " which did not match one of readings expected categories \n";
		}
	}
	if (code == FIND_VARS_DONE) {
		add_latest_finish(start_code, time_pt, output);

	}
	debug_log << "done initing read" << endl;
	debug_log << "time_pt: " << time_pt << endl;
}

static void read(ifstream &file, uint16_t init_code, int rank, struct read_client_config_output &output) {
	long double read_start;
	// long double reading_patterns_start;
	// long double read_type_patterns_start;
	long double extra_testing_start;
	uint16_t num_server_procs = output.config.num_server_procs;

	file >> read_start;

	long double time_pt;
	uint16_t code = init_code;

	add_earliest_start(code, read_start, output);

	read_init(file, init_code, rank, output);

	bool servers_shutdown = false;

	long double read_all_type_patterns_time = 0;
	long double extra_testing_time;
	long double read_all_patterns_time = 0;

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
					//fix
					// output.init_times.push_back(time_pt-read_start);
					// reading_patterns_start = time_pt;
					break;

				case READING_PATTERNS_DONE:
					extreme_debug_log << "done reading patterns" << endl; 
					for(int pattern = 1; pattern <= 6; pattern++) {
						int pattern_indx = pattern - 1;
						extreme_debug_log << "got here 1\n";
						read_all_patterns_time += output.read_pattern_times[pattern_indx].at(output.read_pattern_times[pattern_indx].size()-1);
						extreme_debug_log << "got here 2\n";
					}
					extreme_debug_log << "read_all_patterns_time: " << read_all_patterns_time << endl;
					output.read_all_patterns_times.push_back(read_all_patterns_time);
					// output.read_pattern_times.push_back(time_pt - reading_patterns_start);
					// read_type_patterns_start = time_pt;	
	
					break;

				case READING_TYPE_PATTERNS_DONE: {
						extreme_debug_log << "done reading type patterns" << endl;
					 // 	long double pattern2t, pattern3t;
						// pattern2t = output.read_pattern_times[7].at(output.read_pattern_times[7].size()-1);
						// pattern3t = output.read_pattern_times[8].at(output.read_pattern_times[8].size()-1);
						// read_all_type_patterns_time = pattern2t + pattern3t;
						// output.read_all_type_patterns_times.push_back(read_all_type_patterns_time);
						long double type_pattern_2_times = 0;
						long double type_pattern_3_times = 0;					
						for(int pattern = 1; pattern <= 6; pattern++) {
							int pattern_indx = pattern + 6 - 1;
							extreme_debug_log << "got here 1\n";
							extreme_debug_log << "pattern_indx: " << pattern_indx << " output.read_pattern_times.size(): " << output.read_pattern_times[pattern_indx].size() << endl;
							extreme_debug_log << "pattern_indx: " << pattern_indx << " output.read_pattern_total_hdf5_read_times.size(): " << output.read_pattern_total_hdf5_read_times[pattern_indx].size() << endl;

							read_all_type_patterns_time += output.read_pattern_times[pattern_indx].back();
							if(pattern < 4) {
								type_pattern_2_times += output.read_pattern_times[pattern_indx].back();
							}
							else {
								type_pattern_3_times += output.read_pattern_times[pattern_indx].back();
							}
							extreme_debug_log << "got here 2\n";
						}
						output.read_all_type_patterns_times.push_back(read_all_type_patterns_time);
						output.read_all_type_pattern_2_times.push_back(type_pattern_2_times);
						output.read_all_type_pattern_3_times.push_back(type_pattern_3_times);
						break;
					}

					//fix - don't think I'll ever need this, right?
					// output.read_all_pattern_times.push_back(read_all_patterns_time + read_all_type_patterns_time);	


					// output.read_type_pattern_times.push_back(time_pt - read_type_patterns_start);
					// output.read_all_pattern_times.push_back(time_pt - read_start);	
					// extra_testing_start = time_pt;	

				case EXTRA_TESTING_START:
					extra_testing_start = time_pt;
					break;	

				case EXTRA_TESTING_DONE:
					extreme_debug_log << "done with extra testing" << endl; 
					extra_testing_time = time_pt - extra_testing_start;
					output.extra_testing_times.push_back(extra_testing_time);
					output.reading_times.push_back(read_all_patterns_time + read_all_type_patterns_time + extra_testing_time);	

					break;

				// case READING_DONE:
				// 	break;
			}
		}
		else if(is_read_pattern(code)) {
			add_earliest_start(code, time_pt, output);

			switch (code) {
				case READ_PATTERN_1_START:
					read_pattern(file, 1, code, 
						time_pt, READ_PATTERN_1_DONE, output, false);
					break;

				case READ_PATTERN_2_START:
					read_pattern(file, 2, code, 
						time_pt, READ_PATTERN_2_DONE, output, false);
					break;	

				case READ_PATTERN_2_TYPES_START:		
					read_pattern(file, 2, code, 
						time_pt, READ_PATTERN_2_DONE, output, true);
					break;	

				case READ_PATTERN_3_START: 
					read_pattern(file, 3, code, 
						time_pt, READ_PATTERN_3_DONE, output, false);
					break;	

				case READ_PATTERN_3_TYPES_START:
					read_pattern(file, 3, code, 
						time_pt, READ_PATTERN_3_DONE, output, true);
					break;	

				case READ_PATTERN_4_START:
					read_pattern(file, 4, code, 
						time_pt, READ_PATTERN_4_DONE, output, false);
					break;	

				case READ_PATTERN_5_START:
					read_pattern(file, 5, code, 
						time_pt, READ_PATTERN_5_DONE, output, false);
					break;	

				case READ_PATTERN_6_START:
					read_pattern(file, 6, code, 
						time_pt, READ_PATTERN_6_DONE, output, false);
					break;	

				default:
					error_log << "error. code did not match a pattern start. code: " << code << " \n";
			}
		}
		//ops
		else if(is_read_op(code) && !is_collective_read(code)) {
			add_earliest_start(code, time_pt, output);
			uint16_t start_code = code;
		
			file >> code;
			file >> time_pt;

			if(is_op(code)) {
				ops(file, code, time_pt, output);
				extreme_debug_log << "just returned from ops \n";
				file >> code;
				file >> time_pt;
			}
			// else {
			// 	error_log << "error. expecting op code received " << code << " instead" << endl;
			// 	return;
			// }
			if(is_read_op(code) && code % 2 == 1) {
				add_latest_finish(start_code, time_pt, output);
			}
			else {
				error_log << "error. was expecting a read finish code but received " << code << " instead" << endl;
			}
		}
		else if(is_read_op(code) && is_collective_read(code)) {
			add_earliest_start(code, time_pt, output);
			extreme_debug_log << "code: " << code << " is collective op" << endl;
			collective_ops(file, code, time_pt, output);
		}
		// else if (is_op(code)) {
		// 	if (is_collective_read(code)) {
		// 		collective_ops(file, code, time_pt, output);
		// 	}
		// 	else {
		// 		ops(file, code, time_pt, output);
		// 	}
		// }
		// else if (is_obj_name(code)) { 	
		//     //fix 			
		// } 
		else if(is_error(code)) {
			error_log << "there was an error code: " << code << " for time_pt: " << time_pt << endl;
			// return;
		}
		else {
			error_log << "error. received code " << code << " which did not match one of readings expected categories \n";
		}
	}
	debug_log << "done reading file" << endl;
	debug_log << "time_pt: " << time_pt << endl;
}


static void read_pattern(ifstream &file, int pattern, uint16_t code, 
		long double start_time, uint16_t end_code, struct read_client_config_output &output,
		bool type_pattern)
{
	long double time_pt;
	uint16_t start_code = code;
	uint16_t second_end_code = end_code;

	long double op_start_time;

	bool uses_op = false;

	bool uses_hdf5;
	long double hdf5_start_time;

	int pattern_indx = pattern - 1;
	if (type_pattern) {
		pattern_indx = 6;
		if(pattern == 3) {
			pattern_indx += 3;
		}
		second_end_code = start_code + 1;
	}

	long double adj_start_time;

	vector<long double> all_objector_times;
	vector<long double> all_gather_times;
	vector<long double> all_hdf5_read_times;

	// uint32_t var
	uint32_t type_pattern_3_var = 0;

	// long double pattern_2_new_type_start_time = 0;
	// long double pattern_3_new_type_start_time = 0;
	extreme_debug_log << "starting new pattern " << pattern_indx << endl;
	while(code != end_code && !is_error(code) && file >> code) {
		extreme_debug_log << "code: " << code << endl;
		file >> time_pt;
		if(code == start_code + 3 - 10) {
			// if (pattern == 2 || pattern == 3 && type_pattern_3_count == 2) {
				adj_start_time = time_pt;
				extreme_debug_log << "adj_start_time: " << adj_start_time << endl;
			// }
		}
		if(is_op(code)) {
			op_start_time = time_pt;
			uses_op = true;
			if (is_collective_read(code)) {
				collective_ops(file, code, time_pt, output);
			}
			else {
				ops(file, code, time_pt, output);
			}
			extreme_debug_log << "just returned from ops in pattern " << pattern_indx << " \n";
		}
		else if (is_gather_start(code)) { 
			//starts gathering once the md query completes
			if (uses_op) {
				output.read_pattern_op_times.at(pattern_indx).push_back(adjust(op_start_time,time_pt));
			}

			long double gather_start_time = time_pt;
			uint16_t gather_start_code = code;

			file >> code;
			file >> time_pt;
			if(code == (gather_start_code + 1) ) {
				output.read_pattern_gather_times.at(pattern_indx).push_back(adjust(gather_start_time,time_pt));
				all_gather_times.push_back(adjust(gather_start_time,time_pt));
			}
			else {
				error_log << "error. saw gather start code: " << gather_start_code << 
						" but did not see gather end code. instead saw: " << code << endl;
				return;
			}

		}
		else if (is_obj_name(code)) { 	
		    //fix 			
		}
		else if(is_objector_start(code)) {
			long double objector_start = time_pt;

			file >> code;
			file >> time_pt;
			if(is_objector_done(code)) {
				extreme_debug_log << "objector_start: " << objector_start << " time_pt: " << time_pt << endl;
				output.read_pattern_objector_times.at(pattern_indx).push_back(adjust(objector_start,time_pt));
				extreme_debug_log << "just added objector point: " << output.read_pattern_objector_times.at(pattern_indx).back() << endl;
				all_objector_times.push_back(adjust(objector_start,time_pt));

				// if(type_pattern && pattern == 2){
				// 	if(pattern_2_new_type_start_time == 0) {
				// 		error_log << "error. pattern 2 type time is 0" << endl;
				// 	}
				// 	else {
				// 		output.read_pattern_type_2_times.push_back(time_pt - pattern_2_new_type_start_time);
				// 	}
				// 	pattern_2_new_type_start_time = 0;
				// }
				// else if(type_pattern && pattern == 3){
				// 	if(pattern_3_new_type_start_time == 0) {
				// 		error_log << "error. pattern 3 type time is 0" << endl;
				// 	}
				// 	else {
				// 		output.read_pattern_type_3_times.push_back(time_pt - pattern_3_new_type_start_time);
				// 	}
				// 	pattern_3_new_type_start_time = 0;
				// }
			}
		}
		else if (is_hdf5_start(code)) {
			hdf5_start_time = time_pt;
			extreme_debug_log << "hdf5_start_time: " << hdf5_start_time << endl;
			while(is_hdf5(code) && !is_hdf5_done(code) && file >> code) {
				file >> time_pt;
			}
			if(is_hdf5_done(code)) {
				output.read_pattern_hdf5_read_times.at(pattern_indx).push_back(adjust(hdf5_start_time,time_pt));
				all_hdf5_read_times.push_back(adjust(hdf5_start_time, time_pt));

				if(type_pattern) {
					if(pattern == 2 || pattern == 3 && type_pattern_3_var == 0) {
						extreme_debug_log << "all_gather_times.size() start: " << all_gather_times.size() << endl; 
						extreme_debug_log << "output.read_pattern_total_gather_times.at(pattern_indx).size(): " << output.read_pattern_total_gather_times.at(pattern_indx).size() << endl;
						if(all_gather_times.size() > 0) {
							output.read_pattern_total_gather_times.at(pattern_indx).push_back(std::accumulate(all_gather_times.begin(), all_gather_times.end(), 0.0));
							all_gather_times.clear();
						}
						if(all_objector_times.size() > 0) {
							output.read_pattern_total_objector_times.at(pattern_indx).push_back(std::accumulate(all_objector_times.begin(), all_objector_times.end(), 0.0));
							all_objector_times.clear();
						}
						if(all_hdf5_read_times.size() > 0) {
							output.read_pattern_total_hdf5_read_times.at(pattern_indx).push_back(std::accumulate(all_hdf5_read_times.begin(), all_hdf5_read_times.end(), 0.0));
							all_hdf5_read_times.clear();
						}
						output.read_pattern_times.at(pattern_indx).push_back(adjust(adj_start_time,time_pt));
						extreme_debug_log << "all_gather_times.size() end: " << all_gather_times.size() << endl; 
						extreme_debug_log << "output.read_pattern_total_gather_times.at(pattern_indx).size(): " << output.read_pattern_total_gather_times.at(pattern_indx).size() << endl;

					}
					else if (pattern == 3) {
						extreme_debug_log << "output.read_pattern_total_gather_times.at(pattern_indx).size() start: " << output.read_pattern_total_gather_times.at(pattern_indx).size() << endl;
						if(all_gather_times.size() > 0) {
							output.read_pattern_total_gather_times.at(pattern_indx).back() += (std::accumulate(all_gather_times.begin(), all_gather_times.end(), 0.0));
							all_gather_times.clear();
						}
						if(all_objector_times.size() > 0) {
							output.read_pattern_total_objector_times.at(pattern_indx).back() += (std::accumulate(all_objector_times.begin(), all_objector_times.end(), 0.0));
							all_objector_times.clear();
						}
						if(all_hdf5_read_times.size() > 0) {
							output.read_pattern_total_hdf5_read_times.at(pattern_indx).back() += (std::accumulate(all_hdf5_read_times.begin(), all_hdf5_read_times.end(), 0.0));
							all_hdf5_read_times.clear();
						}
						output.read_pattern_times.at(pattern_indx).back() += (adjust(adj_start_time,time_pt));
						extreme_debug_log << "output.read_pattern_total_gather_times.at(pattern_indx).size() end: " << output.read_pattern_total_gather_times.at(pattern_indx).size() << endl;

					}
					// type_pattern_3_count = 0;
					extreme_debug_log << "time_pt: " << time_pt << " adj_start_time: " << adj_start_time << endl;

					extreme_debug_log << "pattern time: " << time_pt - adj_start_time << endl;
					pattern_indx += 1;
					if(pattern_indx == 12) {
						pattern_indx = 9;
						type_pattern_3_var += 1;
					}

				}
		    }	
		    else {
				error_log << "error. expected to see hdf5 done code but instead saw " << code << endl;
		    }
		}
		else if(code > end_code) {
			error_log << "error in read pattern " << pattern << ". code: " << code << " and is not of expected type" << endl;
		}
		// else if(code == READ_PATTERN_2_START_CATALOGING_VAR_ATTRS) {
		// 	pattern_2_new_type_start_time = time_pt;
		// }
		// else if(code == READ_PATTERN_3_START_CATALOGING_VAR_ATTRS) {
		// 	pattern_3_new_type_start_time = time_pt;
		// }
	}
	if(code == end_code) {
		if(!type_pattern ) {
			output.read_pattern_times.at(pattern_indx).push_back(adjust(start_time,time_pt));
		}
		extreme_debug_log << "found my end so am returning" << endl;
	}
	else {
		error_log << "error. pattern " << pattern << " end time code never appeared after start time code. Instead code " << code << " appeared \n";
	}	
	file >> code;
	file >> time_pt;
	if(code != second_end_code) {
		error_log << "error. pattern "<<  pattern << " second end time code never appeared after start time code. Instead code " << code << " appeared \n";
		return;
	}
	add_latest_finish(start_code, time_pt, output);


	extreme_debug_log << "output.read_pattern_total_gather_times.at(pattern_indx).size() end1: " << output.read_pattern_total_gather_times.at(pattern_indx).size() << endl;

	if(all_gather_times.size() > 1) {
		output.read_pattern_total_gather_times.at(pattern_indx).push_back(std::accumulate(all_gather_times.begin(), all_gather_times.end(), 0.0));
	}
	if(all_objector_times.size() > 1) {
		output.read_pattern_total_objector_times.at(pattern_indx).push_back(std::accumulate(all_objector_times.begin(), all_objector_times.end(), 0.0));
	}
	if(all_hdf5_read_times.size() > 1 ){
		output.read_pattern_total_hdf5_read_times.at(pattern_indx).push_back(std::accumulate(all_hdf5_read_times.begin(), all_hdf5_read_times.end(), 0.0));
	}
	extreme_debug_log << "output.read_pattern_total_gather_times.at(pattern_indx).size() end2: " << output.read_pattern_total_gather_times.at(pattern_indx).size() << endl;

}

//READ_PATTERN_1_START
//READ_PATTERN_1_DONE


static void ops(ifstream &file, uint16_t code, long double start_time, struct read_client_config_output &output) {
	long double time_pt;

	if(code % 100 != 0) {
		error_log << "error. was expecting a code divisible by 100 but received " << code << " instead" << endl;
		return;
	}

	long double serialize_done_time;
	long double send_done_time;
	long double rec_return_msg_done_time;
	long double end_time;

	unsigned short serialze_code = code + 2;
	unsigned short send_code = code + 4;
	unsigned short rec_return_msg_code = code + 5;
	unsigned short end_code = code + 7;

	extreme_debug_log << "end_code: " << to_string(end_code) << " note this is in extreme debug logging" << endl;
	// extreme_debug_log << "am debug logging \n";
	while(code != end_code && !is_error(code) && file >> code) {
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
	extreme_debug_log << "code in ops: " << code << endl;
	int op_indx = (code - 1000) / 100;
	if (code == end_code) {
		end_time = time_pt;
		extreme_debug_log << "is end code: " << code << endl;

		if(end_code != MD_FULL_SHUTDOWN_OP_DONE) {
			output.op_times[op_indx].at(0).push_back(adjust(start_time,serialize_done_time));
			output.op_times[op_indx].at(1).push_back(adjust(serialize_done_time,send_done_time));
			output.op_times[op_indx].at(2).push_back(adjust(send_done_time,rec_return_msg_done_time));
			output.op_times[op_indx].at(3).push_back(adjust(rec_return_msg_done_time,end_time));
			output.op_times[op_indx].at(4).push_back(adjust(start_time,end_time));	
			extreme_debug_log << "just finished pushing back " << endl;
		
			// op_piece_times->at(0).push_back(serialize_done_time-start_time);
			// op_piece_times->at(1).push_back(send_done_time-serialize_done_time);
			// op_piece_times->at(2).push_back(rec_return_msg_done_time-send_done_time);
			// op_piece_times->at(3).push_back(end_time-rec_return_msg_done_time);
			// op_piece_times->at(4).push_back(end_time-start_time);
		}
		else {
			output.op_times[op_indx].at(4).push_back(adjust(start_time,end_time));			
			// op_piece_times->at(4).push_back(end_time-start_time);
		}
	}
	else {
		error_log << "error. " << " end time code (" <<  to_string(end_code) << ") never appeared after start time code. Instead code " << to_string(code) << " appeared \n";
	}			
	extreme_debug_log << "got to bottom of ops" << endl;

}

static void collective_ops(ifstream &file, uint16_t code, long double start_time, struct read_client_config_output &output) 
{
	long double time_pt;

	long double gather_start_time;
	long double gather_end_time;
	bool gathering = false;

	uint16_t start_code = code;
	long double end_time;
	unsigned short collective_done_code = code + 1;

	unsigned short gather_end_code;

	long double objector_start_time;
	long double objector_end_time;

	long double hdf5_start_time;
	long double hdf5_end_time;

	bool uses_objector = false;
	bool uses_gather = false;
	bool uses_hdf5 = false;

	// unsigned short op_start_cde = code - 20;

	extreme_debug_log << "end_code: " << to_string(collective_done_code) << " note this is in extreme debug logging" << endl;
	// extreme_debug_log << "am debug logging \n";
	while(code != collective_done_code && !is_error(code) && file >> code) {
		file >> time_pt;
		if( is_op(code) && !is_collective_read(code) ) {
			ops(file, code, time_pt, output);
		}
		else if ( is_gather(code) ) {
			uses_gather = true;
			if (gathering) {
				gather_end_time = time_pt;
				gathering = false;
			}
			else { //not gathering
				gather_start_time = time_pt;
				gather_end_code = code + 1;
				gathering = true;
			}

		}
		else if (is_obj_name(code)) { 	
		    //fix 
		} 
		else if (is_objector_start(code)) {
			objector_start_time = time_pt;
			extreme_debug_log << "objector_start_time: " << objector_start_time << endl;
			file >> code;
			file >> time_pt;
			if(is_objector_done(code)) {
	    		objector_end_time = time_pt;
	    		extreme_debug_log << "objector_end_time: " << objector_end_time << endl;
	    		uses_objector = true;
		    }		
		    else {
		    	error_log << "error. expected to see objector done code but instead saw: " << code << endl;
		    }

		}
		else if (is_hdf5_start(code)) {
			hdf5_start_time = time_pt;
			extreme_debug_log << "hdf5_start_time: " << hdf5_start_time << endl;
			while(is_hdf5(code) && !is_hdf5_done(code) && file >> code) {
				file >> time_pt;
			}
			if(is_hdf5_done(code)) {
	    		hdf5_end_time = time_pt;
	    		extreme_debug_log << "hdf5_end_time: " << hdf5_end_time << endl;
	    		uses_hdf5 = true;
		    }	
		    else {
				error_log << "error. expected to see hdf5 done code but instead saw " << code << endl;
		    }
		}
		else if (code != collective_done_code){
			error_log << "error in collective ops. expecting op code or gather code and instead saw " << code << endl;
		}
	}
	if (gathering) {
		error_log << "error. saw gather start code but not end code " << endl;
	}

	extreme_debug_log << "code in ops: " << code << endl;
	int op_indx = (code - 11000) / 100;
	if (code == collective_done_code) {
		end_time = time_pt;
		extreme_debug_log << "is end code: " << code << endl;

		extreme_debug_log << "op_indx: " << op_indx << " output.collective_op_times.size(): " <<
				output.collective_op_times.size() <<
				" output.collective_op_times[op_indx].size(): " <<
				output.collective_op_times[op_indx].size() << endl;

		if(uses_gather) {
			output.collective_op_times[op_indx].at(0).push_back(adjust(gather_start_time,gather_end_time));
			extreme_debug_log << "got here 1\n";
		}
		if(uses_objector) {
			output.collective_op_times[op_indx].at(1).push_back(adjust(objector_start_time,objector_end_time));
			extreme_debug_log << "got here 2\n";
		}
		if(uses_hdf5) {
			output.collective_op_times[op_indx].at(2).push_back(adjust(hdf5_start_time,hdf5_end_time));
			extreme_debug_log << "got here 3\n";	
		}
		output.collective_op_times[op_indx].at(3).push_back(adjust(start_time,end_time));
		extreme_debug_log << "just finished pushing back " << endl;

		add_latest_finish(start_code, time_pt, output);
	}
	else {
		error_log << "error. end time code (" << to_string(collective_done_code) << ") never appeared after gather start time code. Instead code " << to_string(code) << " appeared \n";
	}			
	extreme_debug_log << "got to bottom of ops" << endl;

}

static void shutdown(ifstream &file, int rank, uint16_t code, struct read_client_config_output &output) {
	long double time_pt;
	long double start_time;
	uint16_t num_server_procs = output.config.num_server_procs;

	file >> start_time;
	if(code == READING_DONE_FOR_ALL_PROCS_START_CLEANUP && rank >= num_server_procs) {
		return;
	}
	//todo - do I want to do anything with this?
	while(is_shutdown(code) && !file.eof()){
		file >> code;
		file >> time_pt;
		if(code == SERVER_SHUTDOWN_DONE && rank !=0) {
			debug_log << "rank: " << rank << " and got to server shutdown done " << endl;
			break;
		}
		else if(is_op(code)) {
			ops(file, code, time_pt, output);
		}
	}
}



// static int evaluate_all_clock_times_client(ifstream &file, struct read_client_config_output &output) {
// 	long double time_pt;	
// 	bool new_proc;
// 	uint16_t num_client_procs = output.config.num_read_client_procs;
// 	uint16_t num_clock_pts = output.config.num_clock_pts;
// 	uint16_t num_server_procs = output.config.num_server_procs;

// 	extreme_debug_log << "num_client_procs: " << num_client_procs << endl;
// 	extreme_debug_log << "num_server_procs: " << num_server_procs << endl;

// 	long double earliest_start = 1000000;
// 	long double earliest_init_done = 1000000;
// 	long double earliest_read_pattern_done = 1000000;

// 	long double latest_init_done = 0;
// 	long double latest_read_pattern_done = 0;
// 	long double latest_extra_testing_done = 0;

// 	long double read_start_time;
// 	long double read_end_time;
// 	long double read_time;
// 	// long double longest_read_time = 0;

// 	for(int rank=0; rank<num_client_procs; rank++) {
// 		uint16_t clock_catg = 0;

// 		// extreme_debug_log << "i: " << i << endl;
// 		file >> time_pt;
// 		extreme_debug_log << "time_pt: " << time_pt << endl;

//     //start:0, mpi_init_done:1, register_ops_done:2, dirman_init_done:3, server_setup_done:4, pattern_read_end:5, read_end:6
// 		while(time_pt != CLOCK_TIMES_NEW_PROC && time_pt != CLOCK_TIMES_DONE) {
// 			output.all_clock_times.at(clock_catg).push_back(time_pt);
// 			switch(clock_catg) {
// 				case 0: //start
// 					if(time_pt < earliest_start) {
// 						earliest_start = time_pt;
// 					}
// 					break;
// 				case 4: //server setup done
// 					read_start_time = time_pt;
// 					if(time_pt < earliest_init_done) {
// 						earliest_init_done = time_pt;
// 					}
// 					if(time_pt > latest_init_done) {
// 						extreme_debug_log << "time_pt: " << time_pt << endl;
// 						latest_init_done = time_pt;
// 					}
// 					break;
// 				case 5: //read pattern done
// 					read_end_time = time_pt;
// 					if(time_pt < earliest_read_pattern_done) {
// 						earliest_read_pattern_done = time_pt;
// 					}
// 					if(time_pt > latest_read_pattern_done) {
// 						latest_read_pattern_done = time_pt;
// 					}
// 					break;
// 				case 6: //extra testing done
// 					read_end_time = time_pt;
// 					if(rank < num_server_procs && time_pt > latest_extra_testing_done) {
// 						latest_extra_testing_done = time_pt; 
// 					}
// 					break;
// 			}
// 			file >> time_pt;
// 			extreme_debug_log << "time_pt: " << time_pt << endl;

// 			clock_catg++;
// 		}
// 		// //this is where I'm leaving off (plus see errors.txt)
// 		// 	// -> do I need any of this?
// 		// read_time = read_end_time - read_start_time;
// 		// if(read_time > longest_read_time) {
// 		// 	longest_read_time = read_time;
// 		// }
// 		// output.all_clock_times.push_back(clock_times);
// 	}
// 	long double earliest_read_pattern_start = earliest_init_done;
// 	long double earliest_extra_testing_start = earliest_read_pattern_done;
// // (0) total init time (0:start -> 4:server setup done)
// // (1) total read pattern time (4:server setup done -> 5: read patterns done)
// // (2) total extra testing time ( 5: read patterns done -> 6: extra testing done)
// // (3) total read time (4:server setup done -> 6: extra testing done)
// // (4) total run time without extra (0:start -> 5: read patterns done)
// // (5) total run time with extra (0:start -> 6: extra testing done)
// 		//skip (6)
// // (6) longest single read time (4:server setup done-> 5: read patterns done or 6: extra testing done if type )

// 	extreme_debug_log << "earliest start: " << earliest_start << endl;
// 	extreme_debug_log << "earliest_init_done: " << earliest_init_done << endl;
// 	extreme_debug_log << "earliest_read_pattern_done: " << earliest_read_pattern_done << endl;
// 	extreme_debug_log << "latest_init_done: " << latest_init_done << endl;
// 	extreme_debug_log << "latest_read_pattern_done: " << latest_read_pattern_done << endl;
// 	extreme_debug_log << "latest_extra_testing_done: " << latest_extra_testing_done << endl;

// 	output.clock_times_eval.at(0).push_back(latest_init_done-earliest_start);
// 	output.clock_times_eval.at(1).push_back(latest_read_pattern_done-earliest_read_pattern_start);
// 	// if(output.config.num_types > 0) {
// 		output.clock_times_eval.at(2).push_back(latest_extra_testing_done-earliest_extra_testing_start);
// 		output.clock_times_eval.at(3).push_back(latest_extra_testing_done-earliest_init_done);
// 	// }
// 	output.clock_times_eval.at(4).push_back(latest_read_pattern_done-earliest_start);
// 	// if(output.config.num_types > 0) {
// 		output.clock_times_eval.at(5).push_back(latest_extra_testing_done-earliest_start);
// 	// }
// 	// output.clock_times_eval.at(6).push_back(longest_read_time);

// 	debug_log << "last time_pt: " << time_pt << endl;
// 	return RC_OK;
// }

void add_earliest_start(uint16_t code, long double start_time, read_client_config_output &output) {
	map<uint16_t,long double>::iterator i = output.earliest_starts.find(code);

	if ( i == output.earliest_starts.end() || i->second > start_time) {
		output.earliest_starts[code] = start_time;
	}
}

void add_latest_finish(uint16_t code, long double end_time, read_client_config_output &output) {
	map<uint16_t,long double>::iterator i = output.latest_finishes.find(code);

	if ( i == output.latest_finishes.end() || i->second < end_time) {
		output.latest_finishes[code] = end_time;
	}
}


void add_last_first_timing(read_client_config_output &output) {
	int indx = 0;
	bool first_file = false;
	if(output.clock_times_eval_catgs.size() == 0) {
		first_file = true;
	}
	for (std::map<uint16_t,long double>::iterator start_it=output.earliest_starts.begin(); start_it!=output.earliest_starts.end(); ++start_it) {
		debug_log << start_it->first << " => " << start_it->second << '\n';	

		map<uint16_t,long double>::iterator end_it = output.latest_finishes.find(start_it->first);
		if ( end_it != output.latest_finishes.end() ) {
			debug_log << "indx: " << indx << endl;
			extreme_debug_log << "for category: " << start_it->first << " earliest start: " << start_it->second << " latest finish: " << end_it->second  << endl;
			if(output.clock_times_eval.size() <= indx) {
				output.clock_times_eval.push_back(vector<long double>());
			}
			extreme_debug_log << "output.clock_times_eval.size(): " << output.clock_times_eval.size() << " indx: " << indx << endl;
			output.clock_times_eval.at(indx).push_back(end_it->second - start_it->second);
			extreme_debug_log << "just pushed back: " << end_it->second - start_it->second << " for category " << start_it->first << endl;
			if (first_file) {
				if ( start_it->first > 10000) {
					output.clock_times_eval_catgs.push_back(start_it->first - 10000);
				}
				else {
					output.clock_times_eval_catgs.push_back(start_it->first);								
				}
			}
			else {
				if (start_it->first != output.clock_times_eval_catgs.at(indx) && start_it->first -10000 != output.clock_times_eval_catgs.at(indx))  {
					error_log << "error. was expecting catg: " << output.clock_times_eval_catgs.at(indx) <<
						" but instead saw: " << start_it->first << endl;
				}
			}
			indx += 1;
		}
		else {
			error_log << "error. found code " << start_it->first << " as an earliest start but not a latest finish" << endl;
		}
	}
	output.earliest_starts.clear();
	output.latest_finishes.clear();
}
