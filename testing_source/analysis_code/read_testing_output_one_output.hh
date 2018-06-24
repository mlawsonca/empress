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


struct client_config_output {
	struct testing_config {
		uint16_t num_clock_pts;
		uint16_t num_server_procs;
		uint16_t num_server_nodes;
		uint16_t num_client_procs;
		uint16_t num_client_nodes;
		uint16_t num_read_procs;
		uint64_t num_datasets;
		uint16_t num_types;
	} config; 

	std::vector<std::string> filenames;

	//sections
	std::vector<double> init_times;
	std::vector<double> writing_create_times;
	std::vector<double> writing_insert_times;
	std::vector<double> writing_times;
	//note: this is how long it takes to get through the read pattern section, 
	//regardless of if there are types or not
	std::vector<double> read_init_times;
	std::vector<double> read_pattern_times;
	std::vector<double> read_type_pattern_times;
	std::vector<double> read_all_pattern_times;
	std::vector<double> extra_testing_times;
	std::vector<double> reading_times;

	//read patterns
	std::vector<double> read_pattern_1_times;
	std::vector<double> read_pattern_2_times;
	std::vector<double> read_pattern_3_times;
	std::vector<double> read_pattern_4_times;
	std::vector<double> read_pattern_5_times;
	std::vector<double> read_pattern_6_times;
	std::vector<double> read_pattern_1_type_times;
	std::vector<double> read_pattern_2_type_times;
	std::vector<double> read_pattern_3_type_times;
	std::vector<double> read_pattern_4_type_times;
	std::vector<double> read_pattern_5_type_times;
	std::vector<double> read_pattern_6_type_times;

	//ops
	std::vector<std::vector<double>> activate_var_times; 
	std::vector<std::vector<double>> catalog_var_times; 
	std::vector<std::vector<double>> create_var_times; 
	//note - no longer testing delete
	std::vector<std::vector<double>> get_chunk_list_times; 
	std::vector<std::vector<double>> get_chunk_times; 
	std::vector<std::vector<double>> insert_chunk_times; 
	std::vector<std::vector<double>> processing_var_times; 
	std::vector<std::vector<double>> activate_type_times; 
	std::vector<std::vector<double>> catalog_type_times; 
	std::vector<std::vector<double>> create_type_times;
	//note - no longer testing delete
	std::vector<std::vector<double>> get_attr_list_times;
	std::vector<std::vector<double>> get_attr_times;
	std::vector<std::vector<double>> insert_attr_times;
	std::vector<std::vector<double>> processing_type_times;
	std::vector<std::vector<double>> shutdown_times;

	//time of day
	std::vector< std::vector<long double> > all_clock_times;
	std::vector< std::vector<long double> > clock_times_eval;

};



struct server_config_output {
	struct testing_config {
		uint16_t num_storage_pts;
		uint16_t num_clock_pts;
		uint16_t num_server_procs;
		uint16_t num_server_nodes;
		uint16_t num_client_procs;
		uint16_t num_client_nodes;
		uint16_t num_read_procs;
		uint64_t num_datasets;
		uint16_t num_types;
	} config; 

	std::vector<std::string> filenames;

	//sections 
	std::vector<double> init_times;
	std::vector<double> shutdown_times;

	//ops
	std::vector<std::vector<double>> activate_var_times; 
	std::vector<std::vector<double>> catalog_var_times; 
	std::vector<std::vector<double>> create_var_times; 
	//note - no longer testing delete
	std::vector<std::vector<double>> get_chunk_list_times; 
	std::vector<std::vector<double>> get_chunk_times; 
	std::vector<std::vector<double>> insert_chunk_times; 
	std::vector<std::vector<double>> processing_var_times; 
	std::vector<std::vector<double>> activate_type_times; 
	std::vector<std::vector<double>> catalog_type_times; 
	std::vector<std::vector<double>> create_type_times;
	//note - no longer testing delete
	std::vector<std::vector<double>> get_attr_list_times;
	std::vector<std::vector<double>> get_attr_times;
	std::vector<std::vector<double>> insert_attr_times;
	std::vector<std::vector<double>> processing_type_times;

	//time of day
	std::vector< std::vector<long double> > all_clock_times;
	std::vector< std::vector<long double> > clock_times_eval;

	//db sizes and estimates
	std::vector< std::vector<uint64_t> > all_storage_sizes;
};


struct dirman_config_output {
	struct testing_config {
		uint16_t num_storage_pts;
		uint16_t num_clock_pts;
		uint16_t num_server_procs;
		uint16_t num_server_nodes;
		uint16_t num_client_procs;
		uint16_t num_client_nodes;
		uint16_t num_read_procs;
		uint64_t num_datasets;
		uint16_t num_types;
	} config; 

	std::vector<std::string> filenames;

	std::vector< std::vector<double> > all_time_pts;

	std::vector< std::vector<long double> > clock_times_eval;
	std::vector< std::vector<long double> > all_clock_times;

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

struct extremeDebugLog {
  private:
    bool on;

  public:
  extremeDebugLog(bool turn_on) {
    on = turn_on;
  }
  void turn_on_logging() {
    on = true;
  }
  void turn_off_logging() {
    on = false;
  }
  template<typename T> extremeDebugLog& operator << (const T& x) {
   if(on) {
      std::cout << x;
    }
    return *this;
  }
  extremeDebugLog& operator<<(std::ostream& (*manipulator)(std::ostream&)) {
   if(on) {
      std::cout << manipulator;
    }
     return *this;
  }
};

struct errorLog {
  private:
    bool on;

  public:
  errorLog(bool turn_on) {
    on = turn_on;
  }
  void turn_on_logging() {
    on = true;
  }
  void turn_off_logging() {
    on = false;
  }
  template<typename T> errorLog& operator << (const T& x) {
   if(on) {
      std::cout << x;
    }
    return *this;
  }
  errorLog& operator<<(std::ostream& (*manipulator)(std::ostream&)) {
   if(on) {
      std::cout << manipulator;
    }
     return *this;
  }
};

#endif //READTESTINGOUTPUTONEOUTPUT_HH