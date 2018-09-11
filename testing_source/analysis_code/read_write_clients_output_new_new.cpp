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



#include "../../include/client/md_client_timing_constants.hh"
#include "../../include/client/client_timing_constants_write.hh"
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <iomanip> //set precision
// #include <limits> //numeric_limits
#include <map>
#include "read_testing_output_one_output.hh"
#include "stats_functions.hh"


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

static void ops(ifstream &file, uint16_t code, long double start_time, 
	struct write_client_config_output &output, long double &total_time);

static void evaluate_pts_of_interest(const vector<pt_of_interest> &pts, struct write_client_config_output &output);

static vector<pt_of_interest> add_all_points_of_interest(uint32_t num_write_client_procs);
static vector<pt_of_interest> add_all_last_first_points_of_interest();

// template<class T1, class T2>
// static void add_pt (std::map<T1, vector<T2>> &my_map, T1 key, T2 value);
static void add_pt (std::vector<timing_pts> &pts, string point_name, long double value);

static const uint16_t FIRST_ERR_CODE = ERR_ARGC;
static const uint16_t FIRST_OP_CODE = MD_ACTIVATE_START;
static const uint16_t LAST_TIMING_CODE = HDF5_CREATE_CHUNKED_VAR_DONE; 
static const uint16_t FIRST_CODE_AFTER_OPS = BOUNDING_BOX_TO_OBJ_NAMES;
// static const uint16_t FIRST_CODE_AFTER_OPS = BOUNDING_BOX_TO_OBJ_NAMES_START;

// static const uint16_t LAST_TIMING_CODE = WRITING_DONE_FOR_ALL_PROCS_START_CLEANUP;

static bool is_error(uint16_t code) {
	return code >= FIRST_ERR_CODE;
}

static bool is_obj_name(uint16_t code) {
	return (code == BOUNDING_BOX_TO_OBJ_NAMES);
}
// static bool is_objector_start(uint16_t code) {
// 	return ( code == BOUNDING_BOX_TO_OBJ_NAMES_START );
// }
// static bool is_objector_done(uint16_t code) {
// 	return ( code == BOUNDING_BOX_TO_OBJ_NAMES_DONE );
// }

// static bool is_bounding_box(uint16_t code) {
// 	return (is_obj_name(code) || is_objector_start(code) || is_objector_done(code));
// }

static bool is_op(uint16_t code) {
	return (FIRST_OP_CODE <= code && code < FIRST_CODE_AFTER_OPS);
}

// static bool is_init(uint16_t code) {
// 	return ( (PROGRAM_START <= code && code < SERVER_SETUP_DONE_INIT_DONE) );
// }

// static bool is_writing(uint16_t code) {
// 	return ( (WRITING_START <= code && code < WRITING_DONE) || is_op(code) || is_obj_name(code) );
// }

// static bool is_shutdown(uint16_t code) {
// 	return ( (WRITING_DONE <= code && code < LAST_TIMING_CODE) || is_op(code) );
// }

static bool is_testing_harness(uint16_t code) {
	// return ( ( PROGRAM_START <= code && code <= LAST_TIMING_CODE) || is_obj_name(code) );
	return ( ( PROGRAM_START <= code && code <= LAST_TIMING_CODE) );
}

// static bool is_writing(uint16_t code) {
// 	// return ( ( PROGRAM_START <= code && code <= LAST_TIMING_CODE) || is_obj_name(code) );
// 	return ( ( WRITE_CHUNK_DATA_START <= code && code <= WRITE_CHUNK_DATA_DONE) );
// }

static bool is_hdf5(uint16_t code) {
	// return ( ( PROGRAM_START <= code && code <= LAST_TIMING_CODE) || is_obj_name(code) );
	return ( ( HDF5_CREATE_TIMESTEP_START <= code && code <= HDF5_CREATE_CHUNKED_VAR_DONE) );
}

// static long double adjust(long double start_time, long double end_time) {
// 	// if (end_time < start_time) {
// 	// 	return (end_time + 3600 - start_time);
// 	// 	extreme_debug_log << "adjusting: start_time: " << start_time << " end_time: " << end_time << endl;
// 	// }
// 	// else {
// 		return end_time - start_time;
// 	// }
// }


int evaluate_all_write_clients(const string &file_path, struct write_client_config_output &output) {
		
	uint16_t code;
	int rc = RC_OK;
	ifstream file;
	int rank = -1;
	long double time_pt;


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

	vector<pt_of_interest> all_points_of_interest = add_all_points_of_interest(output.config.num_write_client_procs);
	long double op_time_since_last_testing_harness_pt;
	// long double hdf5_time_since_last_testing_harness_pt;

	while (file >> code) { 
		file >> time_pt;
		debug_log << "code at top of while loop: " << to_string(code) << endl;

		if(is_error(code)) {
			error_log << "error from in while loop of code: " << code << endl;
			// return 1;
		} 
		else if(is_op(code)) {
			long double total_op_time;
			extreme_debug_log << "starting an op \n";
			ops(file, code, time_pt, output, total_op_time);
			extreme_debug_log << "just completed an op" << endl;
			op_time_since_last_testing_harness_pt += total_op_time;
			// cout << "total_op_time: " << total_op_time << endl;
		}
		// else if(is_hdf5(code)) {
		// 	long double start_pt = time_pt;
		// 	uint16_t start_code = code;
		// 	file >> code;
		// 	file >> time_pt;
		// 	if(code != start_code+1) {
		// 		error_log << "error. expected code " << start_code + 1 << " to appear "
		// 			<< " after code " << start_code << " but instead " << code << " appeared" << endl;
		// 	}
		// 	else {
		// 		hdf5_time_since_last_testing_harness_pt += (time_pt - start_pt);
		// 	}
		// }
		else if (is_testing_harness(code)) {
			if(code == PROGRAM_START) {
				rank += 1;

				for (pt_of_interest &pt : all_points_of_interest) {
					pt.open = false;
				}
			}
			for (pt_of_interest &pt : all_points_of_interest) {
				extreme_debug_log << "pt: " << pt.name << " open: " << pt.open << endl;
				if (pt.open) {
					if (pt.op_times.at(rank).size() == 0) {
						error_log << "error. pt: " << pt.name << " has 0 size for ops" << endl;
					}
					extreme_debug_log << "code: " << code << " pt " << pt.name << " is still open" << endl;
					extreme_debug_log << "op time: " << pt.op_times.at(rank).back() << endl;
					pt.op_times.at(rank).at(pt.op_times.at(rank).size()-1) += op_time_since_last_testing_harness_pt;
					// pt.hdf5_times.at(rank).at(pt.hdf5_times.at(rank).size()-1) += hdf5_time_since_last_testing_harness_pt;
					extreme_debug_log << "op_time_since_last_testing_harness_pt: " << op_time_since_last_testing_harness_pt << endl;
					extreme_debug_log << "op time: " << pt.op_times.at(rank).back() << endl;

					if (code == pt.end_code) {
						extreme_debug_log << "code: " << " is end point for " << pt.name << endl;
						pt.open = false;
						pt.end_times.at(rank).push_back(time_pt);
					}
				}
				else if (code == pt.start_code) {
					pt.open = true;
					extreme_debug_log << "code: " << " is start point for " << pt.name << endl;
					extreme_debug_log << "pt.open: " << pt.open << endl;
					pt.op_times.at(rank).push_back(0);
					// pt.hdf5_times.at(rank).push_back(0);
					pt.start_times.at(rank).push_back(time_pt);
				}
			}

			op_time_since_last_testing_harness_pt = 0;
			// hdf5_time_since_last_testing_harness_pt = 0;
		}
		else {
			error_log << "error in write client rank " << rank << ". code: " << code << " did not fit into one of the expected categories" << endl;
		}

		
	}
	evaluate_pts_of_interest(all_points_of_interest, output);


cleanup:
	file.close();
	return rc;
}


static void evaluate_pts_of_interest(const vector<pt_of_interest> &all_points_of_interest, struct write_client_config_output &output)
{
	debug_log << "about to begin eval" << endl;

	for(pt_of_interest pt : all_points_of_interest) {
		for(int rank = 0; rank < output.config.num_write_client_procs; rank++) {
			vector<long double> start_times = pt.start_times.at(rank);
			vector<long double> end_times = pt.end_times.at(rank);
			vector<long double> op_times = pt.op_times.at(rank);
			// vector<long double> hdf5_times = pt.hdf5_times.at(rank);

			if(start_times.size() == 0 || end_times.size() == 0) {
				continue;
			}
			else {
				if(start_times.size() != end_times.size()) {
					error_log << "error. for rank: " << rank << " for start code: " << pt.start_code << 
						" size: " << start_times.size() << " for end code: " << pt.end_code <<
						" size: " << end_times.size() << endl;
					continue;
				}		
			}

			for(int i = 0; i < start_times.size(); i++) {
				long double start_time = start_times.at(i);
				long double end_time = end_times.at(i);
				long double total_time = end_time - start_time;
				long double op_time = op_times.at(i);
				// long double hdf5_time = hdf5_times.at(i);

				extreme_debug_log << "am adding to map total time: " << total_time << endl;
				extreme_debug_log << "for rank: " << rank << " start time: " << start_time << " end time: " << end_time << endl; 
				add_pt (output.per_proc_times, pt.name, total_time) ;

				if(op_time > 0) {
					add_pt( output.per_proc_op_times, pt.name, op_time);
				}
				// if(hdf5_time > 0) {
				// 	add_pt( output.per_proc_hdf5_times, pt.name, hdf5_time);
				// }
				else {
					extreme_debug_log << "for code: " << pt.start_code << " to code " << pt.end_code << " op_times sum == 0" << endl;
				}
				extreme_debug_log << "output.per_proc_times.size: " << output.per_proc_times.size() << endl;
				if(total_time < op_time) {
					cout << "error. for rank: " << rank << " for start code: " << pt.start_code << " and end code: " << pt.end_code <<
						" total_time: " << total_time << " and op_time: " << op_time << endl;
				}
				// if(total_time < hdf5_times) {
				// 	cout << "error. for rank: " << rank << " for start code: " << pt.start_code << " and end code: " << pt.end_code <<
				// 		" total_time: " << total_time << " and hdf5_time: " << hdf5_time << endl;
				// }
			}
		}
	}

	for(pt_of_interest pt : all_points_of_interest) {
		if (! pt.last_first) {
			continue;
		}

		uint32_t size = pt.start_times.at(0).size();

		vector<long double> first_starts = pt.start_times.at(0);
		vector<long double> last_finishes = pt.end_times.at(0);

		for(int rank = 0; rank < output.config.num_write_client_procs; rank++) {
			if(pt.start_times.at(rank).size() == 0 || pt.end_times.at(rank).size() == 0) {
				continue;
			}
			if(pt.start_times.at(rank).size() != size || pt.end_times.at(rank).size() != size) {
				error_log << "error. for last first pt of interest: " << pt.name << 
					" for rank: " << rank << 
					" start times size: " << pt.start_times.at(rank).size() <<
					" end times size: " << pt.end_times.at(rank).size() << endl;
			}
			for(int i = 0; i<pt.start_times.at(rank).size(); i++) {
				long double start_time = pt.start_times.at(rank).at(i);
				long double end_time = pt.end_times.at(rank).at(i);
				if(start_time < first_starts.at(i)) {
					first_starts.at(i) = start_time;
				}
				if(end_time > last_finishes.at(i)) {
					last_finishes.at(i) = end_time;
				}				

			}
		}

		for(int i = 0; i < first_starts.size(); i++) {
			long double total_time = last_finishes.at(i) - first_starts.at(i);
			// extreme_debug_log << "for last_first first_start: " << first_start << " last finish: " << last_finish << endl;
			add_pt (output.last_first_times, pt.name, total_time );
		}
		// extreme_debug_log << "start_times 0: " << start_times.at(0).at(0) << endl;
		// extreme_debug_log << "end_times 0: " << end_times.at(0).at(0) << endl;
	
		// long double first_start = get_min_vv(start_times);
		// long double last_finish = get_max_vv(end_times);


		// long double total_time = last_finish - first_start;
		// extreme_debug_log << "for last_first first_start: " << first_start << " last finish: " << last_finish << endl;

		// add_pt (output.last_first_times, pt.name, total_time );

	}
	

}

// template<class T1, class T2>
static void add_pt (std::vector<timing_pts> &pts, string point_name, long double value) {
	int i = 0;
	// extreme_debug_log << "point_name: " << point_name << endl;
	while (i < pts.size()) {
		if(pts.at(i).name == point_name) {
			// extreme_debug_log << "yes. " << pts.at(i).name << " == " << point_name << endl;
			pts.at(i).time_pts.push_back(value);
			break;
		}
		i++;
	}
	if(i == pts.size()) {
		// extreme_debug_log << "i: " << i << " value: " << value << endl;
		vector<long double> time_pts = {value};
		pts.push_back( timing_pts( point_name, time_pts));
	}
}



// template<class T1, class T2>
// static void add_pt (std::map<T1, vector<T2>> &my_map, T1 key, T2 value) {
// 	if (my_map.find(key) == my_map.end() ) {
// 		vector<T2> new_val = {value};
// 		my_map[key] = new_val;
// 	}
// 	else {
// 		my_map.find(key)->second.push_back(value);
// 	}
// }

static vector<pt_of_interest> add_all_points_of_interest(uint32_t num_write_client_procs) {
	vector<pt_of_interest> all_points_of_interest = {	

    	pt_of_interest("TOTAL_RUN_TIME", PROGRAM_START, WRITING_DONE, num_write_client_procs, true), //problem
		pt_of_interest("TOTAL_INIT_TIME", PROGRAM_START, SERVER_SETUP_DONE_INIT_DONE, num_write_client_procs, true),
		pt_of_interest("MPI_INIT_TIME", PROGRAM_START, MPI_INIT_DONE, num_write_client_procs,num_write_client_procs),
		// pt_of_interest("METADATA_CLIENT_INIT_TIME", MPI_INIT_DONE, METADATA_CLIENT_INIT_DONE, num_write_client_procs,num_write_client_procs),
		pt_of_interest("DIRMAN_INIT_TIME", INIT_VARS_DONE, DIRMAN_SETUP_DONE,num_write_client_procs),
		pt_of_interest("SERVER_INIT_TIME", DIRMAN_SETUP_DONE, SERVER_SETUP_DONE_INIT_DONE,num_write_client_procs),
		pt_of_interest("TOTAL_WRITE_TIME", WRITING_START, WRITING_DONE, num_write_client_procs, true), //problem		
		pt_of_interest("PER_RUN_CREATE_TIME", WRITING_START, CREATE_TYPES_DONE, num_write_client_procs, true), //problem		
		// pt_of_interest("OBJECTOR_LOAD_TIME", OBJECTOR_LOAD_START, OBJECTOR_LOAD_DONE,num_write_client_procs),
		pt_of_interest("CREATE_RUN_TIME", CREATE_NEW_RUN_START, CREATE_NEW_RUN_DONE,num_write_client_procs),
		pt_of_interest("CREATE_TYPES_TIME", CREATE_TYPES_START, CREATE_TYPES_DONE,num_write_client_procs),
		pt_of_interest("TOTAL_TIMESTEPS_CREATE_TIME",  CREATE_TIMESTEPS_START, CREATE_ALL_TIMESTEPS_DONE, num_write_client_procs, true), //problem
		pt_of_interest("TOTAL_TIMESTEP_CREATE_TIME", CREATE_NEW_TIMESTEP_START,CREATE_AND_ACTIVATE_TIMESTEP_DONE, num_write_client_procs, true), //problem
		// pt_of_interest("CREATE_OBJS_AND_STORE_IN_MAP_TIME", CREATE_OBJS_AND_STORE_IN_MAP_START, CREATE_OBJS_AND_STORE_IN_MAP_DONE,num_write_client_procs),
		pt_of_interest("WRITE_CHUNK_DATA_TIME", WRITE_CHUNK_DATA_START, WRITE_CHUNK_DATA_DONE,num_write_client_procs),
		pt_of_interest("CREATE_DATA_TIME", CREATE_DATA_START, CREATE_DATA_DONE,num_write_client_procs),
		// pt_of_interest("FIND_CHUNK_MIN_AND_MAX_TIME", CHUNK_MAX_MIN_FIND_START, CHUNK_MAX_MIN_FIND_DONE,num_write_client_procs),
		// pt_of_interest("OBJECTOR_TIME", BOUNDING_BOX_TO_OBJ_NAMES_START, BOUNDING_BOX_TO_OBJ_NAMES_DONE,num_write_client_procs),
		pt_of_interest("CREATE_VAR_ATTRS_TIME", CREATE_VAR_ATTRS_FOR_NEW_VAR_START, CREATE_VAR_ATTRS_FOR_NEW_VAR_DONE,num_write_client_procs),
		pt_of_interest("CREATE_TIMESTEP_ATTRS_TIME", CREATE_TIMESTEP_ATTRS_START, CREATE_TIMESTEP_ATTRS_DONE,num_write_client_procs),
		pt_of_interest("CREATE_RUN_ATTRS_TIME", CREATE_RUN_ATTRS_START, CREATE_RUN_ATTRS_DONE,num_write_client_procs)
		//fix - do I want to do anything with shutdown?
	};

	return all_points_of_interest;
}

// static vector<pt_of_interest> add_all_last_first_points_of_interest() {
// 	vector<pt_of_interest> all_points_of_interest = {	
// 		pt_of_interest("TOTAL_INIT_TIME", PROGRAM_START, SERVER_SETUP_DONE_INIT_DONE,num_write_client_procs),
// 		pt_of_interest("PER_RUN_CREATE_TIME", WRITING_START, CREATE_TYPES_DONE,num_write_client_procs), //problem
// 		pt_of_interest("TOTAL_TIMESTEPS_CREATE_TIME",  CREATE_TIMESTEPS_START, CREATE_ALL_TIMESTEPS_DONE,num_write_client_procs), //problem
// 		pt_of_interest("TOTAL_TIMESTEP_CREATE_TIME", CREATE_NEW_TIMESTEP_START,CREATE_AND_ACTIVATE_TIMESTEP_DONE,num_write_client_procs), //problem
// 		pt_of_interest("TOTAL_WRITE_TIME", WRITING_START, WRITING_DONE,num_write_client_procs), //problem
// 		pt_of_interest("TOTAL_RUN_TIME", PROGRAM_START, WRITING_DONE) //problem
// 	};

// 	return all_points_of_interest;
// }



static void ops(ifstream &file, uint16_t code, long double start_time, struct write_client_config_output &output, long double &total_time) {
	long double time_pt;

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
	while(code < end_code && file >> code) {
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
	int op_indx = (code - 1000) / 100;
	if (code == end_code) {
		end_time = time_pt;
		total_time = end_time - start_time;
		if(end_code != MD_FULL_SHUTDOWN_OP_DONE) {
			output.op_times[op_indx].at(0).push_back(serialize_done_time-start_time);
			output.op_times[op_indx].at(1).push_back(send_done_time-serialize_done_time);
			output.op_times[op_indx].at(2).push_back(rec_return_msg_done_time-send_done_time);
			output.op_times[op_indx].at(3).push_back(end_time-rec_return_msg_done_time);
			output.op_times[op_indx].at(4).push_back(end_time-start_time);			
			// op_piece_times->at(0).push_back(serialize_done_time-start_time);
			// op_piece_times->at(1).push_back(send_done_time-serialize_done_time);
			// op_piece_times->at(2).push_back(rec_return_msg_done_time-send_done_time);
			// op_piece_times->at(3).push_back(end_time-rec_return_msg_done_time);
			// op_piece_times->at(4).push_back(end_time-start_time);
		}
		else {
			output.op_times[op_indx].at(4).push_back(end_time-start_time);			
			// op_piece_times->at(4).push_back(end_time-start_time);
		}
	}
	else {
		error_log << "error. " << to_string(end_code) << " end time code never appeared after start time code. Instead code " << to_string(code) << " appeared \n";
	}			
}



