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

int START = 0;
int MPI_INIT_DONE = 1;
int DIRMAN_SETUP_DONE = 2;
int REGISTER_OPS_DONE = 3;
int GENERATE_CONTACT_INFO_DONE = 4;
int FINALIZE = 5;

int CLOCK_TIMES_START = 98;
int CLOCK_TIMES_END = 99;

int ERR_GENERATE_CONTACT_INFO = 10000;

static bool extreme_debug_logging = false;
static bool debug_logging = false;
static bool error_logging = true;

static errorLog error_log = errorLog(error_logging);
static debugLog debug_log = debugLog(debug_logging);
static extremeDebugLog extreme_debug_log = extremeDebugLog(extreme_debug_logging);


static int evaluate_clock_times(ifstream &file, uint16_t code, struct dirman_config_output &output); 

static const uint16_t FIRST_ERR_CODE = ERR_GENERATE_CONTACT_INFO; 
static const uint16_t LAST_TIMING_CODE = FINALIZE; 

static bool is_error(uint16_t code) {
	return code >= FIRST_ERR_CODE;
}


int evaluate_dirman(const string &file_path, struct dirman_config_output &output) {
	
	uint16_t code;
	double time_pt;
	double start_time;
	int rc;
	ifstream file;
	vector<double> time_pts;

	file.open(file_path);
	if(!file) {
		error_log << "error. failed to open file" << endl;
		return RC_ERR;
	}

	rc = evaluate_clock_times(file, code, output);
	if(rc == RC_ERR) {
		goto cleanup;
	}
	file >> start_time;
	// output.all_time_pts.at(0).push_back(time_pt);
	// debug_log << "i: " << 0 << " output.all_time_pts.at(0).size(): " << output.all_time_pts.at(0).size() << endl;

	//don't want to record that start time was 0.00000

	for(int i=1; i<=LAST_TIMING_CODE; i++) {
		file >> code;
		file >> time_pt;
		if(code != i) {
			error_log << "error. the dirman's " << i << "th time point was not of type " << i << endl;
			goto cleanup;
		}
		if(code == GENERATE_CONTACT_INFO_DONE) {
			output.clock_times_eval.at(0).push_back(time_pt-start_time);
		}
		else if(code == FINALIZE) {
			output.clock_times_eval.at(1).push_back(time_pt-start_time);
		}

		output.all_time_pts.at(i-1).push_back(time_pt);
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
		error_log << "error. dirman did not start with clock times \n";
		return RC_ERR;
	}
	// while(time_pt != CLOCK_TIMES_END && !file.eof() ) {
	vector<long double> clock_times;

	while(time_pt != START && !file.eof() ) {
		clock_times.push_back(time_pt);
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
