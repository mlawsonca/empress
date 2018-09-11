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


#ifndef EVALTESTINGOUTPUT_HH
#define EVALTESTINGOUTPUT_HH

// #include <stdint.h>
#include <map>

struct testing_config {
	uint16_t num_write_client_procs;
	uint16_t num_write_client_nodes;
	uint16_t num_read_client_procs;
	uint16_t num_read_client_nodes;
	uint64_t num_datasets;
	// uint16_t num_types;

	// uint16_t num_storage_pts; // just used by server
}; 

struct pt_of_interest {
	uint16_t start_code;
	uint16_t end_code;
	std::string name;

	pt_of_interest ( std::string point_name, uint16_t start, uint16_t end) {
		start_code = start;
		end_code = end;
		name = point_name;
	}
};

// struct timing_pt {
// 	std::string point_name;
// 	long double time;
// 	long double total_op_time;

// 	timing_pt ( std::string name, long double time_pt, long double op_time) {
// 		point_name = name;
// 		time = time_pt;
// 		total_op_time = op_time;
// 	}
// };

struct timing_pts {
	std::string name;
	std::vector<long double> time_pts;
	// vector<long double> total_op_time_pts;

	timing_pts ( std::string pt_name, std::vector<long double> pts) {
		name = pt_name;
		time_pts = pts;
	}
};


struct client_config_output {
	testing_config config; 

	std::vector<std::string> filenames;

	std::vector<timing_pts> per_proc_op_times;
	std::vector<timing_pts> per_proc_times;
	std::vector<timing_pts> last_first_times;

	std::vector<std::vector<long double>> op_times;

	// std::vector<std::vector<std::vector<long double>>> op_times;
};

// struct read_client_config_output {
// 	testing_config config; 

// 	std::vector<std::string> filenames;

// 	std::vector<timing_pts> per_proc_op_times;
// 	std::vector<timing_pts> per_proc_times;
// 	std::vector<timing_pts> last_first_times;

// 	std::vector<std::vector<std::vector<long double>>> op_times;
// };


   
//for debug testing purposes
struct debugLog {
  private:
    bool on;

  public:
  debugLog(bool turn_on) {
    on = turn_on;
  }
  void turn_on_logging() {
    on = true;
  }
  void turn_off_logging() {
    on = false;
  }
  template<typename T> debugLog& operator << (const T& x) {
   if(on) {
      std::cout << x;
    }
    return *this;
  }
  debugLog& operator<<(std::ostream& (*manipulator)(std::ostream&)) {
   if(on) {
      std::cout << manipulator;
    }
     return *this;
  }
};


#endif //EVALTESTINGOUTPUT_HH