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



#ifndef READTESTINGOUTPUTONEOUTPUT_HH
#define READTESTINGOUTPUTONEOUTPUT_HH

// #include <stdint.h>
#include <map>

struct testing_config {
	uint16_t num_clock_pts;
	uint16_t num_server_procs;
	uint16_t num_server_nodes;
	uint16_t num_write_client_procs;
	uint16_t num_write_client_nodes;
	uint16_t num_read_client_procs;
	uint16_t num_read_client_nodes;
	uint64_t num_datasets;
	// uint16_t num_types;

	uint16_t num_storage_pts; // just used by server
}; 

struct pt_of_interest {
	bool open;
	bool last_first;
	uint16_t start_code;
	uint16_t end_code;
	std::string name;
	std::vector<std::vector<long double>> start_times;
	std::vector<std::vector<long double>> end_times;
	std::vector<std::vector<long double>> op_times;


	pt_of_interest ( std::string point_name, uint16_t start, uint16_t end, uint32_t num_clients,
			 bool last_frst = false) {
		open = false;
		start_code = start;
		end_code = end;
		name = point_name;
		last_first = last_frst;
		start_times = std::vector<std::vector<long double>>(num_clients);
		end_times = std::vector<std::vector<long double>>(num_clients);
		op_times = std::vector<std::vector<long double>>(num_clients);
		// hdf5_times = std::vector<std::vector<long double>>(num_clients);
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


struct write_client_config_output {
	testing_config config; 

	std::vector<std::string> filenames;

	// struct once_per_run_times {
	// 	std::vector<double> load_objector_times;
	// 	std::vector<double> create_run_times;
	// 	std::vector<double> create_types_times;
	// 	std::vector<double> total_times;
	// };
	// std::map<std::string, std::vector<timing_pt>> per_proc_times;

	//indicates, for each testing harness timing pt, how much of that time was spent on ops
	// std::map<std::string, std::vector<long double>> per_proc_op_times;
	// std::map<std::string, std::vector<long double>> per_proc_times;


	// indicates, for each testing harness timing pt, how much of that time was spent on ops
	std::vector<timing_pts> per_proc_op_times;
	// std::vector<timing_pts> per_proc_hdf5_times;
	std::vector<timing_pts> per_proc_times;
	std::vector<timing_pts> last_first_times;

	// std::map<std::string, std::vector<long double>> last_first_times;

	// //sections
	// std::vector<std::vector<double>> init_times;
	// std::vector<double> writing_create_times;
	// std::vector<double> writing_insert_times;
	// std::vector<double> writing_times;
	// std::vector<double> writing_create_type_times;
	// std::vector<double> writing_insert_var_attrs_times;
	// std::vector<double> writing_insert_run_and_timestep_attrs_times;

	// std::vector<double> write_basic_md_total_objector_times;
	// std::vector<double> write_basic_md_total_op_times;

	// std::vector<double> objector_times;

	std::vector<std::vector<std::vector<long double>>> op_times;

	// //time of day
	// std::vector< std::vector<long double>> all_clock_times;
	// std::vector< std::vector<long double>> clock_times_eval;

};

struct read_client_config_output {
	testing_config config; 

	std::vector<std::string> filenames;


	//sections
	std::vector<std::vector<long double>> init_times;

	//note: this is how long it takes to get through the read pattern section, 
	//regardless of if there are types or not
	std::vector<long double> read_all_patterns_times;
	std::vector<long double> read_all_type_patterns_times;
	std::vector<long double> read_all_type_pattern_2_times;
	std::vector<long double> read_all_type_pattern_3_times;
	std::vector<long double> extra_testing_times;
	std::vector<long double> reading_times;

	//read patterns

	std::vector<std::vector<long double>> read_pattern_times;
	std::vector<std::vector<long double>> read_pattern_objector_times;
	std::vector<std::vector<long double>> read_pattern_gather_times;
	std::vector<std::vector<long double>> read_pattern_op_times;
	std::vector<std::vector<long double>> read_pattern_total_gather_times;
	std::vector<std::vector<long double>> read_pattern_total_objector_times;
	std::vector<std::vector<long double>> read_pattern_hdf5_read_times;
	std::vector<std::vector<long double>> read_pattern_total_hdf5_read_times;

	std::vector<long double> read_pattern_type_2_times;
	std::vector<long double> read_pattern_type_3_times;



	// std::vector<double> read_pattern_4_type_times;
	// std::vector<double> read_pattern_5_type_times;
	// std::vector<double> read_pattern_6_type_times;

	std::vector<std::vector<std::vector<long double>>> op_times;
	std::vector<std::vector<std::vector<long double>>> collective_op_times;

	std::map <uint16_t, long double> earliest_starts;
	std::map <uint16_t, long double> latest_finishes;
		// //ops

	//time of day
	// std::vector< std::vector<long double>> all_clock_times;
	std::vector< std::vector<long double>> clock_times_eval;
	std::vector<uint16_t> clock_times_eval_catgs;

};

struct server_config_output {

	testing_config config; 

	std::vector<std::string> filenames;

	//sections 
	std::vector<std::vector<double>> init_times;
	std::vector<double> run_times;
	std::vector<double> shutdown_times;
	std::vector<double> total_run_times;
	std::vector<double> db_load_times;
	std::vector<double> db_output_times;

	// //ops
	std::vector<std::vector<std::vector<double>>> op_times;

	//time of day
	std::vector< std::vector<long double>> all_clock_times;
	std::vector< std::vector<long double>> clock_times_eval;

	//db sizes and estimates
	std::vector< std::vector<uint64_t>> all_storage_sizes;
};


struct dirman_config_output {
	testing_config config; 

	std::vector<std::string> filenames;

	std::vector< std::vector<double>> all_time_pts;

	std::vector< std::vector<long double>> clock_times_eval;
	std::vector< std::vector<long double>> all_clock_times;

};
   
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


#endif //READTESTINGOUTPUTONEOUTPUT_HH