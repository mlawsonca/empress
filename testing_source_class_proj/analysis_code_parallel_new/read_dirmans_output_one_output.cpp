#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <iomanip> //set precision
// #include <limits> //numeric_limits
#include "read_testing_output_const_streaming.hh"


using namespace std;

// enum RC
// {
//     RC_OK = 0,
//     RC_ERR = -1
// };

int START = 0;
int MPI_INIT_DONE = 1;
int DIRMAN_SETUP_DONE = 2;
int REGISTER_OPS_DONE = 3;
int GENERATE_CONTACT_INFO_DONE = 4;
int FINALIZE = 5;

int CLOCK_TIMES_START = 98;
// int CLOCK_TIMES_END = 99;

int ERR_GENERATE_CONTACT_INFO = 10000;

static bool extreme_debug_logging = false;
static bool debug_logging = false;
static bool error_logging = true;

static debugLog error_log = debugLog(error_logging);
static debugLog debug_log = debugLog(debug_logging);
static debugLog extreme_debug_log = debugLog(extreme_debug_logging);


static int evaluate_clock_times(ifstream &file, uint16_t code, struct dirman_config_output &output); 

static const uint16_t FIRST_ERR_CODE = ERR_GENERATE_CONTACT_INFO; 
static const uint16_t LAST_TIMING_CODE = FINALIZE; 

static bool is_error(uint16_t code) {
	return code >= FIRST_ERR_CODE;
}

static long double adjust(long double start_time, long double end_time) {
	// if (end_time < start_time) {
	// 	return (end_time + 3600 - start_time);
	// 	debug_log << "adjusting: start_time: " << start_time << " end_time: " << end_time << endl;
	// }
	// else {
		return end_time - start_time;
	// }
}

int evaluate_dirman(const string &file_path, struct dirman_config_output &output) {
	
	uint16_t code;
	double time_pt;
	double start_time;
	int rc = RC_OK;
	ifstream file;
	double prev_time_pt;

	file.open(file_path);
	if(!file) {
		error_log << "error. failed to open file " << file_path << endl;
		return RC_ERR;
	}
   	// std::string line; 			
   	// std::getline(file, line); //throws away the error line
    // if( line.find("<ERROR> [ibverbs_transport]") == std::string::npos) { 			
    //     error_log << "line 0 for file path " << file_path << " does not contain <ERROR> [ibverbs_transport]. " <<
    //     	"the line is: " << line << endl;
    // } 	
	
	extreme_debug_log << "about to evaluate_clock_times \n";
	rc = evaluate_clock_times(file, code, output);
	extreme_debug_log << "done with evaluate_clock_times \n";
	if(rc == RC_ERR) {
		goto cleanup;
	}
	file >> start_time;
	// output.all_time_pts.at(0).push_back(time_pt);
	// debug_log << "i: " << 0 << " output.all_time_pts.at(0).size(): " << output.all_time_pts.at(0).size() << endl;

	prev_time_pt = start_time;
	for(int i=1; i<=LAST_TIMING_CODE; i++) {
		file >> code;
		file >> time_pt;
		if(code != i) {
			error_log << "error. the dirman's " << i << "th time point was not of type " << i << endl;
			goto cleanup;
		}
		if(code == GENERATE_CONTACT_INFO_DONE) {
			output.clock_times_eval.at(0).push_back(adjust(start_time,time_pt));
		}
		else if(code == FINALIZE) {
			output.clock_times_eval.at(1).push_back(adjust(start_time,time_pt));
		}

		output.all_time_pts.at(i-1).push_back(adjust(prev_time_pt,time_pt));
		prev_time_pt = time_pt;
		extreme_debug_log << "i-1: " << i << " output.all_time_pts.at(i-1).size(): " << output.all_time_pts.at(i-1).size() << endl;
	}

cleanup:	
	file.close();
	return rc;
}

static int evaluate_clock_times(ifstream &file, uint16_t code, struct dirman_config_output &output) {
	long double time_pt;

	file >> code;
	file >> time_pt;

	uint16_t clock_catg = 0;

	if(code != CLOCK_TIMES_START) {
		error_log << "error. dirman did not start with clock times. instead saw code: " << code << " \n";
		return RC_ERR;
	}
	// while(time_pt != CLOCK_TIMES_END && !file.eof() ) {
	vector<long double> clock_times;

	while(time_pt != START && !file.eof() ) {
		if(clock_times.size() == output.config.num_clock_pts) {
			error_log << "found time_pt: " << time_pt << " and category: " << clock_catg << " when expecting " <<
						output.config.num_clock_pts << " clock points \n";
			return -1;
		}
		clock_times.push_back(time_pt);
		extreme_debug_log << "clock_catg: " << clock_catg << " time_pt: " << time_pt << endl;
		output.all_clock_times.at(clock_catg).push_back(time_pt);
		file >> time_pt;
		clock_catg++;
	}
	if(time_pt != START) {
		error_log << "error. reached end of file before finding START. last code found: " << time_pt << endl;
		return RC_ERR;
	}
	// if(time_pt != CLOCK_TIMES_END) {
	// 	error_log << "error. reached end of file before finding CLOCK_TIMES_END. last code found: " << time_pt << endl;
	// 	return RC_ERR;
	// }
	debug_log << "last time_pt: " << time_pt << endl;
	return RC_OK;
}


