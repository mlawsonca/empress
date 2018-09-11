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
#include <functional>
#include <iostream>
#include <vector>
#include <string>
#include <numeric>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <map>
#include <math.h>
#include "read_testing_output_one_output.hh"
#include "stats_functions.hh"
#include <stdio.h>

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

uint16_t NUM_CLOCK_PTS_CLIENT = 10;
uint16_t NUM_CLOCK_PTS_SERVER = 6; 
uint16_t NUM_CLOCK_PTS_DIRMAN = 6; 

float DEC_PERCENT_CLIENT_PROCS_USED_TO_READ = .1; 
uint16_t NUM_SERVER_STORAGE_PTS = 9; 

uint16_t num_client_ops = 69;
uint16_t num_server_ops = 68;

int evaluate_all_servers(const string &file_path, struct server_config_output &output);
int evaluate_all_write_clients(const string &file_path, struct write_client_config_output &output);
int evaluate_all_read_clients(const string &file_path, struct read_client_config_output &output);
int evaluate_dirman(const string &file_path, struct dirman_config_output &output);

static int write_output_write_client(const string &results_path, bool first_run_clients, const struct write_client_config_output &output);
static int write_output_read_client(const string &results_path, bool first_run_clients, const struct read_client_config_output &output);
static int write_output_server(const string &results_path, bool first_run_servers, const struct server_config_output &output, const string &output_type);
static int write_output_dirman(const string &results_path, bool first_run_dirman, const struct dirman_config_output &output, const string &output_type);
static int write_op_numbering_guide(const string &results_path);

int main(int argc, char **argv) {
	int rc = RC_OK;

	setbuf(stdout, NULL);



	// string run_type = "remote";
	// string run_type = "local";

	// if(argc != 2) { 
	// 	debug_log << "Error. Usage for " << argv[0] << " is <testing_type>\n";
	// }

	// string testing_type = argv[1];
	vector<string> clusters;
	string run_type = "gscratch";
	// string run_type = "local";
	// string run_type = "remote";

	if(run_type == "gscratch") {
		// clusters.push_back("serrano");
		clusters.push_back("skybridge"); //fix
		clusters.push_back("chama");
		// clusters.push_back("ghost");
		// clusters.push_back("eclipse");
	}
	else {
		// clusters.push_back("chama");
		// clusters.push_back("serrano");
		// clusters.push_back("ghost");	
		// clusters.push_back("skybridge");	
		clusters.push_back("eclipse");
	}

	extreme_debug_log << "got here \n";

	for(int i=0; i<clusters.size(); i++) {
		std::map <string, write_client_config_output> write_client_outputs;
		std::map <string, read_client_config_output> read_client_outputs;
		std::map <string, server_config_output> write_server_outputs;
		std::map <string, server_config_output> read_server_outputs;
		std::map <string, dirman_config_output> write_dirman_outputs;
		std::map <string, dirman_config_output> read_dirman_outputs;
		extreme_debug_log << "clusters.size(): " << clusters.size() << endl;
		extreme_debug_log << "i: " << i << endl;
		string cluster_name = clusters.at(i);
		DIR *runtime_output;
		struct dirent *entry;
		char output_dir[124];
		
		string results_path;

		if (run_type == "remote") {
				int n = sprintf(output_dir, "/ascldap/users/mlawso/sirius/runtime/runtime_%s/run_build",cluster_name.c_str()); 
				results_path = "/ascldap/users/mlawso/sirius/testing_source/analysis_code/analysis";
			// int n = sprintf(output_dir, "/ascldap/users/mlawso/sirius/testing_source/analysis_code/fake_outputs_for_debugging_new"); 
		}
		else if (run_type == "gscratch") {
			int n = sprintf(output_dir, "/gscratch/mlawso/runtime_%s/output/correct_copy_updated",cluster_name.c_str()); 
			results_path = (string)output_dir + "/analysis";
			cout << "results_path: " << results_path << endl;
			// results_path = "/ascldap/users/mlawso/sirius/testing_source/analysis_code/analysis"; //fix
		}
		// else { // run_type == "local"
		// 	// int n = sprintf(output_dir, "/Users/mlawso/sirius/testing_source/analysis_code/fake_outputs_for_debugging_new"); 
		// 	int n = sprintf(output_dir, "/Users/mlawso/sirius/testing_source/analysis_code/log_outputs_serrano_small"); 
		// 				results_path = "/ascldap/users/mlawso/sirius/testing_source/analysis_code/analysis";

		// }
		cout << "reading files from: " << output_dir << endl;
		// int n = sprintf(output_dir, "/Users/margaretlawson/Sublime_Text/sirius/testing_source/analysis");
		// string output_type;

		if( (runtime_output = opendir(output_dir)) ) {
			while(	(entry = readdir(runtime_output)) ) {
				string filename = (string)entry -> d_name;
				size_t found = filename.find_last_of("_");
				string filename_minus_iteration = filename.substr(0,found);

				if( filename.find("testing_harness_new_write") != std::string::npos ) {
					debug_log << "filename: " << filename << endl;
					debug_log << "filename minus iteration: " << filename_minus_iteration << endl;

					if (write_client_outputs.find(filename_minus_iteration) == write_client_outputs.end() ) {
						struct write_client_config_output new_output;
						new_output.filenames.push_back(filename);
						write_client_outputs[filename_minus_iteration] = new_output;
					}
					else {
						write_client_outputs.find(filename_minus_iteration)->second.filenames.push_back(filename);
					}
					debug_log << "count of file params: " << (write_client_outputs.find(filename_minus_iteration)->second).filenames.size() << endl;

				}
				else if( filename.find("testing_harness_new_read") != std::string::npos ) {
					debug_log << "filename: " << filename << endl;
					debug_log << "filename minus iteration: " << filename_minus_iteration << endl;

					if (read_client_outputs.find(filename_minus_iteration) == read_client_outputs.end() ) {
						struct read_client_config_output new_output;
						new_output.filenames.push_back(filename);
						read_client_outputs[filename_minus_iteration] = new_output;
					}
					else {
						read_client_outputs.find(filename_minus_iteration)->second.filenames.push_back(filename);
					}
					debug_log << "count of file params: " << (read_client_outputs.find(filename_minus_iteration)->second).filenames.size() << endl;

				}				
				else if( filename.find("my_metadata_server_write") != std::string::npos ) {
					debug_log << "filename: " << filename << endl;
					debug_log << "filename minus iteration: " << filename_minus_iteration << endl;
					if (write_server_outputs.find(filename_minus_iteration) == write_server_outputs.end() ) {
						struct server_config_output new_output;
						new_output.filenames.push_back(filename);
						write_server_outputs[filename_minus_iteration] = new_output;
					}
					else {
						write_server_outputs.find(filename_minus_iteration)->second.filenames.push_back(filename);
					}
					debug_log << "count of file params: " << (write_server_outputs.find(filename_minus_iteration)->second).filenames.size() << endl;					
				}
				else if( filename.find("my_metadata_server_read") != std::string::npos ) {
					debug_log << "filename: " << filename << endl;
					debug_log << "filename minus iteration: " << filename_minus_iteration << endl;
					if (read_server_outputs.find(filename_minus_iteration) == read_server_outputs.end() ) {
						struct server_config_output new_output;
						new_output.filenames.push_back(filename);
						read_server_outputs[filename_minus_iteration] = new_output;
					}
					else {
						read_server_outputs.find(filename_minus_iteration)->second.filenames.push_back(filename);
					}
					debug_log << "count of file params: " << (read_server_outputs.find(filename_minus_iteration)->second).filenames.size() << endl;					
				}
				else if( filename.find("my_dirman_write") != std::string::npos ) {
					debug_log << "filename: " << filename << endl;
					debug_log << "filename minus iteration: " << filename_minus_iteration << endl;
					if (write_dirman_outputs.find(filename_minus_iteration) == write_dirman_outputs.end() ) {
						struct dirman_config_output new_output;
						new_output.filenames.push_back(filename);
						write_dirman_outputs[filename_minus_iteration] = new_output;
					}
					else {
						write_dirman_outputs.find(filename_minus_iteration)->second.filenames.push_back(filename);

					}
					debug_log << "count of file params: " << (write_dirman_outputs.find(filename_minus_iteration)->second).filenames.size() << endl;		

				}
				else if( filename.find("my_dirman_read") != std::string::npos ) {
					debug_log << "filename: " << filename << endl;
					debug_log << "filename minus iteration: " << filename_minus_iteration << endl;
					if (read_dirman_outputs.find(filename_minus_iteration) == read_dirman_outputs.end() ) {
						struct dirman_config_output new_output;
						new_output.filenames.push_back(filename);
						read_dirman_outputs[filename_minus_iteration] = new_output;
					}
					else {
						read_dirman_outputs.find(filename_minus_iteration)->second.filenames.push_back(filename);

					}
					debug_log << "count of file params: " << (read_dirman_outputs.find(filename_minus_iteration)->second).filenames.size() << endl;		

				}
				else {
					debug_log << "found file that isn't client. name: " << entry -> d_name << endl;
					continue;
				}
				// string write_output =  filename.substr(filename.find(output_type)+output_type.size());
				// write_output =  write_output.substr(0,write_output.find(".log"));
				// debug_log << "write_output: " << write_output << endl;

				// string params_minus_iteration = write_output.substr(0,write_output.size()-2);
				// debug_log << "params_minus_iteration: " << params_minus_iteration << endl;

			}
			debug_log << "done reading files \n";
			closedir(runtime_output);
			debug_log << "got here \n";
		}
		else {
			error_log << "Error. Could not open dir with path " << output_dir << endl;
		}



		//note - currently doens't have options for server proc per node testing, client proc per node testing or txn testing. 
		//for these, please see single config granularity or output per proc versions

	  	for (std::map<string,write_client_config_output>::iterator it=write_client_outputs.begin(); it!=write_client_outputs.end(); ++it) {
	    	string config_params = it->first;
	        write_client_config_output client_config_struct = it->second;

			sscanf(config_params.c_str(), "testing_harness_new_write_%hu_%hu_%hu_%hu_%hu_%hu_%llu", 
					&client_config_struct.config.num_server_procs, &client_config_struct.config.num_server_nodes,
				    &client_config_struct.config.num_write_client_procs, &client_config_struct.config.num_write_client_nodes, 
				    &client_config_struct.config.num_read_client_procs, &client_config_struct.config.num_read_client_nodes, 				    
				    &client_config_struct.config.num_datasets);
			// client_config_struct.config.num_clock_pts = NUM_CLOCK_PTS_CLIENT;
			// client_config_struct.all_clock_times = vector<vector<long double>>(NUM_CLOCK_PTS_CLIENT);
			// // client_config_struct.clock_times_eval = vector<vector<long double>>(6);
			// client_config_struct.clock_times_eval = vector<vector<long double>>(4);
			// client_config_struct.init_times = vector<vector<double>>(6);

			//1000 -> 2400, 3000->8000
			// for (int i = 1000; i<=2400; i+=100) {
			for (short i = 0; i< num_client_ops; i++) {
				client_config_struct.op_times.push_back(std::vector<std::vector<long double>>(5));
			}

			// debug_log << "client_config_struct.all_clock_times.size(): " << client_config_struct.all_clock_times.size() << endl;
			// debug_log << "client_config_struct.clock_times_eval.size(): " << client_config_struct.clock_times_eval.size() << endl;

			debug_log << "num_server_procs: " << client_config_struct.config.num_server_procs << endl;
			debug_log << "num_server_nodes: " << client_config_struct.config.num_server_nodes << endl;
			debug_log << "num_write_client_procs: " << client_config_struct.config.num_write_client_procs << endl;
			debug_log << "num_write_client_nodes: " << client_config_struct.config.num_write_client_nodes << endl;
			debug_log << "num_read_client_procs: " << client_config_struct.config.num_read_client_procs << endl;
			debug_log << "num_read_client_nodes: " << client_config_struct.config.num_read_client_nodes << endl;
			debug_log << "num_datasets: " << client_config_struct.config.num_datasets << endl;
			// debug_log << "num_types: " << client_config_struct.config.num_types << endl;

	      	cout << "write client config_params: " << config_params << " (count: " << client_config_struct.filenames.size() << ")" << endl;

	        for(int i = 0; i<client_config_struct.filenames.size(); i++) {
	        	string filename = client_config_struct.filenames.at(i);
	        	string file_path = (string)output_dir + "/" + filename;
	        	debug_log << "client file_path: " << file_path << endl;
	        	rc = evaluate_all_write_clients(file_path, client_config_struct);
				if (rc != RC_OK) {
					error_log << "Error in evaluate_all_write_clients \n";
					return rc;
				}
	        }
	        write_client_outputs[config_params] = client_config_struct;

		}


	  	for (std::map<string,read_client_config_output>::iterator it=read_client_outputs.begin(); it!=read_client_outputs.end(); ++it) {
	    	string config_params = it->first;
	        read_client_config_output client_config_struct = it->second;

			sscanf(config_params.c_str(), "testing_harness_new_read_%hu_%hu_%hu_%hu_%hu_%hu_%llu", 
					&client_config_struct.config.num_server_procs, &client_config_struct.config.num_server_nodes,
				    &client_config_struct.config.num_write_client_procs, &client_config_struct.config.num_write_client_nodes, 
				    &client_config_struct.config.num_read_client_procs, &client_config_struct.config.num_read_client_nodes, 				    
				    &client_config_struct.config.num_datasets);			
			// client_config_struct.config.num_clock_pts = NUM_CLOCK_PTS_CLIENT;

			// client_config_struct.all_clock_times = vector<vector<long double>>(NUM_CLOCK_PTS_CLIENT);
			// client_config_struct.clock_times_eval = vector<vector<long double>>(6);
			client_config_struct.init_times = vector<vector<long double>>(6);


			//1000 -> 2400, 3000->8000
			// for (int i = 1000; i<=2400; i+=100) {
			for (int i = 0; i< num_client_ops; i++) {
				client_config_struct.op_times.push_back(std::vector<std::vector<long double>>(5));
				client_config_struct.collective_op_times.push_back(std::vector<std::vector<long double>>(4));
			}
			client_config_struct.read_pattern_objector_times = std::vector<std::vector<long double>>(12);
			client_config_struct.read_pattern_times = std::vector<std::vector<long double>>(12);
			client_config_struct.read_pattern_gather_times = std::vector<std::vector<long double>>(12);
			client_config_struct.read_pattern_op_times = std::vector<std::vector<long double>>(12);
			client_config_struct.read_pattern_total_gather_times = std::vector<std::vector<long double>>(12);
			client_config_struct.read_pattern_total_objector_times = std::vector<std::vector<long double>>(12);
			client_config_struct.read_pattern_hdf5_read_times = std::vector<std::vector<long double>>(12);
			client_config_struct.read_pattern_total_hdf5_read_times = std::vector<std::vector<long double>>(12);

			// debug_log << "client_config_struct.all_clock_times.size(): " << client_config_struct.all_clock_times.size() << endl;
			// debug_log << "client_config_struct.clock_times_eval.size(): " << client_config_struct.clock_times_eval.size() << endl;

			debug_log << "num_server_procs: " << client_config_struct.config.num_server_procs << endl;
			debug_log << "num_server_nodes: " << client_config_struct.config.num_server_nodes << endl;
			debug_log << "num_write_client_procs: " << client_config_struct.config.num_write_client_procs << endl;
			debug_log << "num_write_client_nodes: " << client_config_struct.config.num_write_client_nodes << endl;
			debug_log << "num_read_client_procs: " << client_config_struct.config.num_read_client_procs << endl;
			debug_log << "num_read_client_nodes: " << client_config_struct.config.num_read_client_nodes << endl;
			debug_log << "num_datasets: " << client_config_struct.config.num_datasets << endl;
			// debug_log << "num_types: " << client_config_struct.config.num_types << endl;

	      	cout << "read client config_params: " << config_params << " (count: " << client_config_struct.filenames.size() << ")" << endl;

	        for(int i = 0; i<client_config_struct.filenames.size(); i++) {
	        	string filename = client_config_struct.filenames.at(i);
	        	string file_path = (string)output_dir + "/" + filename;
	        	debug_log << "client file_path: " << file_path << endl;

	        	rc = evaluate_all_read_clients(file_path, client_config_struct);
				if (rc != RC_OK) {
					error_log << "Error in evaluate_all_read_clients \n";
					return rc;
				}
	        }
	        read_client_outputs[config_params] = client_config_struct;

		}

	  	for (std::map<string,server_config_output>::iterator it=write_server_outputs.begin(); it!=write_server_outputs.end(); ++it) {
	    	string config_params = it->first;
	        server_config_output server_config_struct = it->second;

			sscanf(config_params.c_str(), "my_metadata_server_write_%hu_%hu_%hu_%hu_%hu_%hu_%llu", 
					&server_config_struct.config.num_server_procs, &server_config_struct.config.num_server_nodes,
				    &server_config_struct.config.num_write_client_procs, &server_config_struct.config.num_write_client_nodes, 
				    &server_config_struct.config.num_read_client_procs, &server_config_struct.config.num_read_client_nodes, 				    
				    &server_config_struct.config.num_datasets);
			server_config_struct.config.num_clock_pts = NUM_CLOCK_PTS_SERVER;
			server_config_struct.config.num_storage_pts = NUM_SERVER_STORAGE_PTS;
			server_config_struct.all_clock_times = vector<vector<long double>>(NUM_CLOCK_PTS_SERVER);
			server_config_struct.all_storage_sizes = vector<vector<uint64_t>>(NUM_SERVER_STORAGE_PTS);
			server_config_struct.clock_times_eval = vector<vector<long double>>(3);
			server_config_struct.init_times = vector<vector<double>>(5);

			for (short i = 0; i< num_server_ops; i++) {
				server_config_struct.op_times.push_back(std::vector<std::vector<double>>(6));
			}

			debug_log << "num_server_procs: " << server_config_struct.config.num_server_procs << endl;
			debug_log << "num_server_nodes: " << server_config_struct.config.num_server_nodes << endl;
			debug_log << "num_write_client_procs: " << server_config_struct.config.num_write_client_procs << endl;
			debug_log << "num_write_client_nodes: " << server_config_struct.config.num_write_client_nodes << endl;
			debug_log << "num_read_client_procs: " << server_config_struct.config.num_read_client_procs << endl;
			debug_log << "num_read_client_nodes: " << server_config_struct.config.num_read_client_nodes << endl;
			debug_log << "num_datasets: " << server_config_struct.config.num_datasets << endl;
			// debug_log << "num_types: " << server_config_struct.config.num_types << endl;
			debug_log << "server_config_struct.all_clock_times.size(): " << server_config_struct.all_clock_times.size() << endl;
			debug_log << "server_config_struct.all_storage_sizes.size(): " << server_config_struct.all_storage_sizes.size() << endl;
			debug_log << "server_config_struct.clock_times_eval.size(): " << server_config_struct.clock_times_eval.size() << endl;

	  	    cout << "write server config_params: " << config_params << " (count: " << server_config_struct.filenames.size() << ")" << endl;

	        for(int i = 0; i<server_config_struct.filenames.size(); i++) {
	        	string filename = server_config_struct.filenames.at(i);
	        	string file_path = (string)output_dir + "/" + filename;
	        	debug_log << "server file_path: " << file_path << endl;
	        	rc = evaluate_all_servers(file_path, server_config_struct);
				if (rc != RC_OK) {
					error_log << "Error in evaluate_all_servers \n";
					return rc;
				}
	        }
	        write_server_outputs[config_params] = server_config_struct;

		}

	  	for (std::map<string,server_config_output>::iterator it=read_server_outputs.begin(); it!=read_server_outputs.end(); ++it) {
	    	string config_params = it->first;
	        server_config_output server_config_struct = it->second;

			sscanf(config_params.c_str(), "my_metadata_server_read_%hu_%hu_%hu_%hu_%hu_%hu_%llu", 
					&server_config_struct.config.num_server_procs, &server_config_struct.config.num_server_nodes,
				    &server_config_struct.config.num_write_client_procs, &server_config_struct.config.num_write_client_nodes, 
				    &server_config_struct.config.num_read_client_procs, &server_config_struct.config.num_read_client_nodes, 				    
				    &server_config_struct.config.num_datasets);
			server_config_struct.config.num_clock_pts = NUM_CLOCK_PTS_SERVER;
			server_config_struct.config.num_storage_pts = NUM_SERVER_STORAGE_PTS;
			server_config_struct.all_clock_times = vector<vector<long double>>(NUM_CLOCK_PTS_SERVER);
			server_config_struct.all_storage_sizes = vector<vector<uint64_t>>(NUM_SERVER_STORAGE_PTS);
			server_config_struct.clock_times_eval = vector<vector<long double>>(3);
			server_config_struct.init_times = vector<vector<double>>(5);

			for (short i = 0; i< num_server_ops; i++) {
				server_config_struct.op_times.push_back(std::vector<std::vector<double>>(6));
			}

			debug_log << "num_server_procs: " << server_config_struct.config.num_server_procs << endl;
			debug_log << "num_server_nodes: " << server_config_struct.config.num_server_nodes << endl;
			debug_log << "num_write_client_procs: " << server_config_struct.config.num_write_client_procs << endl;
			debug_log << "num_write_client_nodes: " << server_config_struct.config.num_write_client_nodes << endl;
			debug_log << "num_read_client_procs: " << server_config_struct.config.num_read_client_procs << endl;
			debug_log << "num_read_client_nodes: " << server_config_struct.config.num_read_client_nodes << endl;
			debug_log << "num_datasets: " << server_config_struct.config.num_datasets << endl;
			// debug_log << "num_types: " << server_config_struct.config.num_types << endl;
			debug_log << "server_config_struct.all_clock_times.size(): " << server_config_struct.all_clock_times.size() << endl;
			debug_log << "server_config_struct.all_storage_sizes.size(): " << server_config_struct.all_storage_sizes.size() << endl;
			debug_log << "server_config_struct.clock_times_eval.size(): " << server_config_struct.clock_times_eval.size() << endl;

	  	    cout << "read server config_params: " << config_params << " (count: " << server_config_struct.filenames.size() << ")" << endl;

	        for(int i = 0; i<server_config_struct.filenames.size(); i++) {
	        	string filename = server_config_struct.filenames.at(i);
	        	string file_path = (string)output_dir + "/" + filename;
	        	debug_log << "server file_path: " << file_path << endl;
	        	rc = evaluate_all_servers(file_path, server_config_struct);
				if (rc != RC_OK) {
					error_log << "Error in evaluate_all_servers \n";
					return rc;
				}
	        }
	        read_server_outputs[config_params] = server_config_struct;

		}

		for (std::map<string,dirman_config_output>::iterator it=write_dirman_outputs.begin(); it!=write_dirman_outputs.end(); ++it) {
	    	string config_params = it->first;
	        dirman_config_output dirman_config_struct = it->second;

			sscanf(config_params.c_str(), "my_dirman_write_%hu_%hu_%hu_%hu_%hu_%hu_%llu", 
				&dirman_config_struct.config.num_server_procs, &dirman_config_struct.config.num_server_nodes,
			    &dirman_config_struct.config.num_write_client_procs, &dirman_config_struct.config.num_write_client_nodes, 
			    &dirman_config_struct.config.num_read_client_procs, &dirman_config_struct.config.num_read_client_nodes, 				    
			    &dirman_config_struct.config.num_datasets);
			dirman_config_struct.config.num_clock_pts = NUM_CLOCK_PTS_DIRMAN;
			dirman_config_struct.all_clock_times = vector<vector<long double>>(NUM_CLOCK_PTS_DIRMAN);
			dirman_config_struct.clock_times_eval = vector<vector<long double>>(2);
			dirman_config_struct.all_time_pts = vector<vector<double>>(NUM_CLOCK_PTS_DIRMAN-1);

			debug_log << "num_server_procs: " << dirman_config_struct.config.num_server_procs << endl;
			debug_log << "num_server_nodes: " << dirman_config_struct.config.num_server_nodes << endl;
			debug_log << "num_write_client_procs: " << dirman_config_struct.config.num_write_client_procs << endl;
			debug_log << "num_write_client_nodes: " << dirman_config_struct.config.num_write_client_nodes << endl;
			debug_log << "num_read_client_procs: " << dirman_config_struct.config.num_read_client_procs << endl;
			debug_log << "num_read_client_nodes: " << dirman_config_struct.config.num_read_client_nodes << endl;
			debug_log << "num_datasets: " << dirman_config_struct.config.num_datasets << endl;
			// debug_log << "num_types: " << dirman_config_struct.config.num_types << endl;
			debug_log << "dirman_config_struct.all_clock_times.size(): " << dirman_config_struct.all_clock_times.size() << endl;
			debug_log << "dirman_config_struct.clock_times_eval.size(): " << dirman_config_struct.clock_times_eval.size() << endl;
			debug_log << "dirman_config_struct.all_time_pts.size(): " << dirman_config_struct.all_time_pts.size() << endl;

	  	  	cout << "write dirman config_params: " << config_params << " (count: " << dirman_config_struct.filenames.size() << ")" << endl;

	        for(int i = 0; i<dirman_config_struct.filenames.size(); i++) {
	        	string filename = dirman_config_struct.filenames.at(i);
	        	string file_path = (string)output_dir + "/" + filename;
	        	debug_log << "dirman file_path: " << file_path << endl;
	        	rc = evaluate_dirman(file_path, dirman_config_struct);
				if (rc != RC_OK) {
					error_log << "Error in evaluate_dirman \n";
					return rc;
				}
	        }
	        write_dirman_outputs[config_params] = dirman_config_struct;

		} 	 	 
	  	
		for (std::map<string,dirman_config_output>::iterator it=read_dirman_outputs.begin(); it!=read_dirman_outputs.end(); ++it) {
	    	string config_params = it->first;
	        dirman_config_output dirman_config_struct = it->second;

			sscanf(config_params.c_str(), "my_dirman_read_%hu_%hu_%hu_%hu_%hu_%hu_%llu", 
				&dirman_config_struct.config.num_server_procs, &dirman_config_struct.config.num_server_nodes,
			    &dirman_config_struct.config.num_write_client_procs, &dirman_config_struct.config.num_write_client_nodes, 
			    &dirman_config_struct.config.num_read_client_procs, &dirman_config_struct.config.num_read_client_nodes, 				    
			    &dirman_config_struct.config.num_datasets);
			dirman_config_struct.config.num_clock_pts = NUM_CLOCK_PTS_DIRMAN;
			dirman_config_struct.all_clock_times = vector<vector<long double>>(NUM_CLOCK_PTS_DIRMAN);
			dirman_config_struct.clock_times_eval = vector<vector<long double>>(2);
			dirman_config_struct.all_time_pts = vector<vector<double>>(NUM_CLOCK_PTS_DIRMAN-1);

			debug_log << "num_server_procs: " << dirman_config_struct.config.num_server_procs << endl;
			debug_log << "num_server_nodes: " << dirman_config_struct.config.num_server_nodes << endl;
			debug_log << "num_write_client_procs: " << dirman_config_struct.config.num_write_client_procs << endl;
			debug_log << "num_write_client_nodes: " << dirman_config_struct.config.num_write_client_nodes << endl;
			debug_log << "num_read_client_procs: " << dirman_config_struct.config.num_read_client_procs << endl;
			debug_log << "num_read_client_nodes: " << dirman_config_struct.config.num_read_client_nodes << endl;
			debug_log << "num_datasets: " << dirman_config_struct.config.num_datasets << endl;
			// debug_log << "num_types: " << dirman_config_struct.config.num_types << endl;
			debug_log << "dirman_config_struct.all_clock_times.size(): " << dirman_config_struct.all_clock_times.size() << endl;
			debug_log << "dirman_config_struct.clock_times_eval.size(): " << dirman_config_struct.clock_times_eval.size() << endl;
			debug_log << "dirman_config_struct.all_time_pts.size(): " << dirman_config_struct.all_time_pts.size() << endl;

	  	  	cout << "read dirman config_params: " << config_params << " (count: " << dirman_config_struct.filenames.size() << ")" << endl;

	        for(int i = 0; i<dirman_config_struct.filenames.size(); i++) {
	        	string filename = dirman_config_struct.filenames.at(i);
	        	string file_path = (string)output_dir + "/" + filename;
	        	debug_log << "dirman file_path: " << file_path << endl;
	        	rc = evaluate_dirman(file_path, dirman_config_struct);
				if (rc != RC_OK) {
					error_log << "Error in evaluate_dirman \n";
					return rc;
				}
	        }
	        read_dirman_outputs[config_params] = dirman_config_struct;

		} 	 	

		bool write_client_first_output = true;
		for (std::map<string,write_client_config_output>::iterator it=write_client_outputs.begin(); it!=write_client_outputs.end(); ++it) {
	    	string config_params = it->first;
	      	write_client_config_output client_config_struct = it->second;
			rc = write_output_write_client(results_path, write_client_first_output, client_config_struct);
			if (rc != RC_OK) {
				error_log << "Error in write_output_write_client \n";
				return rc;
			}
			write_client_first_output = false;

		}

		bool read_client_first_output = true;
		for (std::map<string,read_client_config_output>::iterator it=read_client_outputs.begin(); it!=read_client_outputs.end(); ++it) {
	    	string config_params = it->first;
	      	read_client_config_output client_config_struct = it->second;
			rc = write_output_read_client(results_path, read_client_first_output, client_config_struct);
			if (rc != RC_OK) {
				error_log << "Error in write_output_read_client \n";
				return rc;
			}
			read_client_first_output = false;

		}

		bool write_server_first_output = true;
		for (std::map<string,server_config_output>::iterator it=write_server_outputs.begin(); it!=write_server_outputs.end(); ++it) {
	    	string config_params = it->first;
	      	server_config_output server_config_struct = it->second;
			rc = write_output_server(results_path, write_server_first_output, server_config_struct, "write");
			if (rc != RC_OK) {
				error_log << "Error in write_output_server \n";
				return rc;
			}
			write_server_first_output = false;

		}

		bool read_server_first_output = true;
		for (std::map<string,server_config_output>::iterator it=read_server_outputs.begin(); it!=read_server_outputs.end(); ++it) {
	    	string config_params = it->first;
	      	server_config_output server_config_struct = it->second;
			rc = write_output_server(results_path, read_server_first_output, server_config_struct, "read");
			if (rc != RC_OK) {
				error_log << "Error in read_output_server \n";
				return rc;
			}
			read_server_first_output = false;

		}

		bool write_dirman_first_output = true;
		for (std::map<string,dirman_config_output>::iterator it=write_dirman_outputs.begin(); it!=write_dirman_outputs.end(); ++it) {
	    	string config_params = it->first;
	      	dirman_config_output dirman_config_struct = it->second;
			rc = write_output_dirman(results_path, write_dirman_first_output, dirman_config_struct, "write");
			if (rc != RC_OK) {
				error_log << "Error in write_output_dirman \n";
				return rc;
			}
			write_dirman_first_output = false;

		}

		bool read_dirman_first_output = true;
		for (std::map<string,dirman_config_output>::iterator it=read_dirman_outputs.begin(); it!=read_dirman_outputs.end(); ++it) {
	    	string config_params = it->first;
	      	dirman_config_output dirman_config_struct = it->second;
			rc = write_output_dirman(results_path, read_dirman_first_output, dirman_config_struct, "read");
			if (rc != RC_OK) {
				error_log << "Error in read_output_dirman \n";
				return rc;
			}
			read_dirman_first_output = false;

		}

		rc = write_op_numbering_guide(results_path);
		if (rc != RC_OK) {
			error_log << "Error in write_op_numbering_guide \n";
			return rc;
		}
	}//end for cluster
}

// problem - some are per run
// template<class T1, class T2>
// uint32_t get_times_per_run( T1 output, const vector<T2> &vct ){
// 	if (vct.size() == 0 || output.filenames.size() == 0) {
// 		extreme_debug_log << "vct.size(): " << vct.size() << endl;
// 		return 0;
// 	}
// 	extreme_debug_log << "vct.size(): " << vct.size() << endl;
// 	extreme_debug_log << "output.filenames.size(): " << output.filenames.size() << endl;
// 	extreme_debug_log << "output.config.num_datasets: " << output.config.num_datasets << endl;
// 	if (vct.size() == output.filenames.size()) {
// 		extreme_debug_log << "are equal, returning" << endl;
// 		return 1;
// 	}
// 	return ( vct.size() / (output.filenames.size() * output.config.num_datasets) );
// }

template <class T>
uint32_t get_times_per_run( write_client_config_output output, const vector<T> &vct ){
	if (vct.size() == 0 || output.filenames.size() == 0) {
		extreme_debug_log << "vct.size(): " << vct.size() << endl;
		return 0;
	}
	return ( vct.size() / (output.filenames.size()) );
}

template <class T>
uint32_t get_times_per_proc( write_client_config_output output, const vector<T> &vct ){
	if (vct.size() == 0 || output.filenames.size() == 0) {
		extreme_debug_log << "vct.size(): " << vct.size() << endl;
		return 0;
	}
	if (vct.size() >= output.config.num_write_client_procs) {
		return (vct.size() / (output.filenames.size() * output.config.num_write_client_procs));
	}
	return 0;
}


	

static int write_output_write_client(const string &results_path, bool first_run_clients, const struct write_client_config_output &output) {
	//note: these will in essence be key value tables with key: config params

	int rc = RC_OK;
	// vector<vector<vector<double>>> all_total_op_times{output.activate_var_times, output.catalog_var_times,output.create_var_times,output.get_chunk_list_times,
	// 								    output.get_chunk_times,output.insert_chunk_times,output.processing_var_times,output.activate_type_times,
	// 									output.catalog_type_times,output.create_type_times,output.get_attr_list_times,output.get_attr_times,
	// 									output.insert_attr_times,output.processing_type_times,output.shutdown_times};

	// vector<vector<double>> all_timing_points{output.init_times.at(5),output.writing_create_times,
	// 					output.writing_insert_times,output.writing_times};

	// vector<vector<double>> all_timing_points{output.init_times.at(0),output.init_times.at(1),output.init_times.at(2),
	// 									output.init_times.at(3),output.init_times.at(4),output.init_times.at(5),
	// 									output.write_basic_md_total_op_times,output.objector_times, output.write_basic_md_total_objector_times,
	// 									output.writing_create_times,output.writing_create_type_times,
	// 									output.writing_insert_var_attrs_times, output.writing_insert_run_and_timestep_attrs_times,
	// 									output.writing_insert_times,output.writing_times};

	// vector<vector<double>> init_breakdown_times{output.init_times.at(5),output.init_times.at(0),output.init_times.at(1),output.init_times.at(2),
	// 									output.init_times.at(3),output.init_times.at(4)};

	// vector<vector<double>> write_basic_md_breakdown_times{output.writing_create_times,
	// 							output.write_basic_md_total_op_times,output.objector_times, 
	// 							output.write_basic_md_total_objector_times};

	// vector<vector<double>> write_custom_md_breakdown_times{output.writing_insert_times,									
	// 									output.writing_create_type_times, output.writing_insert_var_attrs_times, 
	// 									output.writing_insert_run_and_timestep_attrs_times};
	
	// vector<vector<vector<double>>> all_breakdown_times = {init_breakdown_times, 
	// 										write_basic_md_breakdown_times,write_custom_md_breakdown_times};
										

	debug_log << "about to start writing client output \n";

	int num_ops = output.op_times.size();
	// int num_sections = all_timing_points.size();
	// int prev_indx = 0;

	string op_results_path = results_path + "/write_client_ops.csv";
	ofstream op_fs;
	string op_breakdown_path = results_path + "/write_client_op_breakdown.csv";
	ofstream op_breakdown_fs;
	string section_results_path = results_path + "/write_client_sections.csv";
	ofstream section_fs;

	// string section_breakdown_path = results_path + "/write_client_sections_breakdown.csv";
	// ofstream section_breakdown_fs;

	// string compare_total_op_times_path = results_path + "/client_compare_total_op_times.csv";
	// ofstream compare_total_op_times_fs;
	// string clock_results_path = results_path + "/client_clocks.csv";
	// ofstream clock_fs;
	// string clock_eval_results_path = results_path + "/write_client_clocks_eval.csv";
	// ofstream clock_eval_fs;
	string last_first_results_path = results_path + "/write_client_last_first_sections.csv";
	ofstream last_first_fs;

	debug_log << "output.filenames.at(0): " << output.filenames.at(0) << endl;

	if(first_run_clients) { 
		debug_log << "first run \n";
		debug_log << "am first run client about to open " << op_results_path << " for writing" << endl;

		op_fs.open(op_results_path, std::fstream::out);
		op_breakdown_fs.open(op_breakdown_path, std::fstream::out);
		section_fs.open(section_results_path, std::fstream::out);
		// section_breakdown_fs.open(section_breakdown_path, std::fstream::out);
		// clock_fs.open(clock_results_path, std::fstream::out);
		// clock_eval_fs.open(clock_eval_results_path, std::fstream::out);
		last_first_fs.open(last_first_results_path, std::fstream::out);

		// compare_total_op_times_fs.open(compare_total_op_times_path, std::fstream::out);


		op_fs << "op,Write Clients per Server,num_server_nodes,num_write_client_procs,num_write_client_nodes,"
			  << "num_read_client_procs,num_read_client_nodes,"
		      << "num_datasets,num data pts,pts per run,pts per proc,max,min,avg,variance,std deviation,median" << endl;

		op_breakdown_fs << "op,Write Clients per Server,num_server_nodes,num_write_client_procs,num_write_client_nodes,"
			  << "num_read_client_procs,num_read_client_nodes,"
		      << "num_datasets,num data pts,pts per run,pts per proc,avg serialize time,avg send time,avg received return msg time,avg deserialize time,avg total time" << endl;


		// section_fs << "section categories: " << endl;
		section_fs << "#categories: " << endl;
		section_fs << "#0: total time, 1:total op time" << endl << endl;


		// section_fs << "0: total init time, 1:total writing basic metadata time, "
		// 		   << "2: writing custom metadata times, 3:total writing time" << endl;
		// section_fs << endl;
		section_fs << "name,category,Write Clients per Server,num_server_nodes,num_write_client_procs,num_write_client_nodes,"
				   << "num_read_client_procs,num_read_client_nodes,"
		           << "num_datasets,num data pts,pts per run,pts per proc,max,min,avg,variance,std deviation,median" << endl;

		// section_breakdown_fs << "section breakdowns: " << endl;
		// section_breakdown_fs << "init: 0:total init time, 1:mpi init time, 2:md_client init time, 3:init vars time, 4: dirman setup time, "
		// 		   			 << "5: server setup time \n" << endl;

		// section_breakdown_fs << "write basic md: 6:total time, 7:writing basic md total op times, 8:avg objector time, "
		// 		   << "9:total objector time \n" << endl;

		// section_breakdown_fs << "write custom md: 10:total time, 11:create types time, 12:insert var attrs time, 13:insert run and timestep attrs time\n"
		// 		   << endl;
		// section_breakdown_fs << "section,Write Clients per Server,num_server_nodes,num_write_client_procs,num_write_client_nodes,"
		// 		   << "num_read_client_procs,num_read_client_nodes,"
		//            << "num_datasets,num data pts,max,min,avg,variance,std deviation,median" << endl;

		// clock_eval_fs << "categories: " << endl;
		// clock_eval_fs << "0:full init time, 1:write basic md time, 2:write var attr time, 3:total write custom md time(insert types and var,timestep and run attrs)" << endl;
		// // clock_eval_fs << "categories: " << endl;
		// // clock_eval_fs << "0:full init time, 1:write basic md time, 2:write custom md time, 3:total write time (with types),"
		// // 			  << "4:total run time without types, 5:total run time with types" << endl;
		// clock_eval_fs << endl;
		last_first_fs << "name,Write Clients per Server,num_server_nodes,num_write_client_procs,num_write_client_nodes,"
			  		  << "num_read_client_procs,num_read_client_nodes,"
				 	  << "num_datasets,num data pts,pts per run,pts per proc,max,min,avg,variance,std deviation,median" << endl;

	}
	else {
		op_fs.open(op_results_path, std::fstream::app);
		debug_log << "am NOT first run client about to open " << op_results_path << " for appending" << endl;

		op_breakdown_fs.open(op_breakdown_path, std::fstream::app);
		section_fs.open(section_results_path, std::fstream::app);
		// section_breakdown_fs.open(section_breakdown_path, std::fstream::app);
		// clock_fs.open(clock_results_path, std::fstream::app);
		// compare_total_op_times_fs.open(section_results_path, std::fstream::app);
		last_first_fs.open(last_first_results_path, std::fstream::app);
	}

	if(!op_fs) {
		error_log << "error. failed to open ops results file: " << op_results_path << endl;
		rc = RC_ERR;
		goto cleanup;
	}
	else if(!op_breakdown_fs) {
		error_log << "error. failed to open ops breakdown file" << endl;
		rc = RC_ERR;
		goto cleanup;
	}
	else if(!section_fs) {
		error_log << "error. failed to open section results file" << endl;
		rc = RC_ERR;
		goto cleanup;	
	}

	else if(!last_first_fs) {
		error_log << "error. failed to open last first results file" << endl;
		rc = RC_ERR;
		goto cleanup;	
	}
	char buffer[512];


	for(int op_index = 0; op_index < num_ops; op_index++) {
		vector<vector<long double>> op_pieces_times = output.op_times[op_index];
		vector<long double> total_op_times = op_pieces_times.at(op_pieces_times.size()-1);
		debug_log << "write client - num time pts for op " << op_index << ": " << total_op_times.size() << endl;
		debug_log << "write client - num categories for op breakdown " << op_index << ": " << op_pieces_times.size() << endl;

		if(total_op_times.size() > 0) {
			// extreme_debug_log << "test. total_op_times[0] orig: " << total_op_times[0] << endl;
			// for(int i = 0; i < total_op_times.size(); i++) {
			// 	total_op_times[i] = (total_op_times[i] / pow(10, 9));
			// }
			// extreme_debug_log << "test. total_op_times[0] new: " << total_op_times[0] << endl;

			long double max = get_max(total_op_times);
			long double min = get_min(total_op_times);
			long double avg = get_mean(total_op_times);
			long double variance = get_variance(total_op_times);
			long double std_dev = get_std_dev(total_op_times);
			long double median = get_median(total_op_times);
			extreme_debug_log << "total op time pts max: " << max;

			sprintf(buffer, "%d,%d,%d,%d,%d,%d,%d,%llu,%lu,%lu,%lu,%Lf,%Lf,%Lf,%Lf,%Lf,%Lf", 
					op_index*100 + 1000, output.config.num_write_client_procs / output.config.num_server_procs,
					output.config.num_server_nodes, 
					output.config.num_write_client_procs, output.config.num_write_client_nodes, 
					output.config.num_read_client_procs, output.config.num_read_client_nodes, 
					output.config.num_datasets, total_op_times.size(), get_times_per_run(output, total_op_times),
					get_times_per_proc(output, total_op_times),
					max, min, avg, variance, std_dev, median);
			extreme_debug_log << "just wrote buffer" << endl;
			op_fs << buffer << endl;

			if(op_pieces_times.size() == 5) {
				// for(int i = 0; i < 5; i++) {
				// 	// for (int j = 0; j < op_pieces_times.at(i).size(); j++) {
				// 	// 	// if(j == 0) {
				// 	// 	// 	extreme_debug_log << "test. top_pieces_times.at(" << i << ").at(0) orig: " << op_pieces_times.at(i)[0] << endl;
				// 	// 	// }
				// 	// 	op_pieces_times.at(i).at(j) = op_pieces_times.at(i).at(j) / pow(10, 9);
				// 	// 	// if(j == 0) {
				// 	// 	// 	extreme_debug_log << "test. top_pieces_times.at(" << i << ").at(0) new: " << op_pieces_times.at(i)[0] << endl;
				// 	// 	// }
				// 	// }
				// }
				long double avg_ser_time = get_mean(op_pieces_times.at(0));
				long double avg_send_time = get_mean(op_pieces_times.at(1));
				long double avg_rec_ret_time = get_mean(op_pieces_times.at(2));
				long double avg_deser_time = get_mean(op_pieces_times.at(3));
				long double avg_total_time = get_mean(op_pieces_times.at(4));
				extreme_debug_log << "avg_ser_time mean: " << avg_ser_time;

				sprintf(buffer, "%d,%d,%d,%d,%d,%d,%d,%llu,%lu,%lu,%lu,%Lf,%Lf,%Lf,%Lf,%Lf", 
					op_index*100 + 1000, output.config.num_write_client_procs / output.config.num_server_procs,
					output.config.num_server_nodes, 
					output.config.num_write_client_procs, output.config.num_write_client_nodes, 
					output.config.num_read_client_procs, output.config.num_read_client_nodes, 
					output.config.num_datasets, 
				 	total_op_times.size(), get_times_per_run(output, total_op_times), 
				 	get_times_per_proc(output, total_op_times),
				 	avg_ser_time, avg_send_time, avg_rec_ret_time, avg_deser_time, avg_total_time);
				
				extreme_debug_log << "just wrote buffer" << endl;
				op_breakdown_fs << buffer << endl;
			}
			else {
				error_log << "error. op " << op_index << " had " << op_pieces_times.size() << " time categories \n";
			}
		}
	}


	cout << "output.per_proc_times.size(): " << output.per_proc_times.size() << endl;
	for (int i = 0; i < output.per_proc_times.size(); i++) {
    	string pt_name = output.per_proc_times.at(i).name;
      	vector<long double> timing_points = output.per_proc_times.at(i).time_pts;
      	cout << "timing_points.size(): " << timing_points.size() << endl;
		if(timing_points.size() > 0) {	
			// extreme_debug_log << "test. timing_points[0] orig: " << timing_points[0] << endl;
			// extreme_debug_log << "test. timing_points[0] new: " << timing_points[0] << endl;

			long double max = get_max(timing_points);
			long double min = get_min(timing_points);
			long double avg = get_mean(timing_points);
			long double variance = get_variance(timing_points);
			long double std_dev = get_std_dev(timing_points);
			long double median = get_median(timing_points);
			extreme_debug_log << "time pts max: " << max;

			sprintf(buffer, "%30s,%d,%d,%d,%d,%d,%d,%d,%llu,%lu,%lu,%lu,%Lf,%Lf,%Lf,%Lf,%Lf,%Lf", 
					pt_name.c_str(), 0, output.config.num_write_client_procs / output.config.num_server_procs,
					output.config.num_server_nodes, 
					output.config.num_write_client_procs, output.config.num_write_client_nodes, 
					output.config.num_read_client_procs, output.config.num_read_client_nodes, 
					output.config.num_datasets, timing_points.size(), get_times_per_run(output, timing_points),
					get_times_per_proc(output, timing_points),
					max, min, avg, variance, std_dev, median);
			extreme_debug_log << "just wrote buffer" << endl;
			section_fs << buffer << endl;
		}

		int indx = 0;
		while(indx < output.per_proc_op_times.size()) {
			if (output.per_proc_op_times.at(indx).name == pt_name ){
				break;
			}
			indx++;
		}
		// if (indx == output.per_proc_op_times.size()) {
  //     		error_log << "error. found " << pt_name << " in proc times but " << 
  //     			"not in proc op times" << endl;
  //     	}
  //     	else {
		if (indx != output.per_proc_op_times.size()) {
			vector<long double> op_times = output.per_proc_op_times.at(indx).time_pts;

			if(op_times.size() > 0) {	
				// extreme_debug_log << "test. timing_points[0] orig: " << timing_points[0] << endl;
				// extreme_debug_log << "test. timing_points[0] new: " << timing_points[0] << endl;

				long double max = get_max(op_times);
				long double min = get_min(op_times);
				long double avg = get_mean(op_times);
				long double variance = get_variance(op_times);
				long double std_dev = get_std_dev(op_times);
				long double median = get_median(op_times);
				extreme_debug_log << "op max: " << max;

				sprintf(buffer, "%30s,%d,%d,%d,%d,%d,%d,%d,%llu,%lu,%lu,%lu,%Lf,%Lf,%Lf,%Lf,%Lf,%Lf", 
						pt_name.c_str(), 1, output.config.num_write_client_procs / output.config.num_server_procs,
						output.config.num_server_nodes, 
						output.config.num_write_client_procs, output.config.num_write_client_nodes, 
						output.config.num_read_client_procs, output.config.num_read_client_nodes, 
						output.config.num_datasets, op_times.size(), get_times_per_run(output, op_times),
						get_times_per_proc(output, op_times),
						max, min, avg, variance, std_dev, median);
				extreme_debug_log << "just wrote buffer" << endl;
				section_fs << buffer << endl;
			}
      	}
	}


	cout << "output.last_first_times.size(): " << output.last_first_times.size() << endl;
	for (int i = 0; i < output.last_first_times.size(); i++) {
    	string pt_name = output.last_first_times.at(i).name;
      	vector<long double> timing_points = output.last_first_times.at(i).time_pts;
      	cout << "timing_points.size(): " << timing_points.size() << endl;
		if(timing_points.size() > 0) {	
			// extreme_debug_log << "test. timing_points[0] orig: " << timing_points[0] << endl;
			// extreme_debug_log << "test. timing_points[0] new: " << timing_points[0] << endl;

			long double max = get_max(timing_points);
			long double min = get_min(timing_points);
			long double avg = get_mean(timing_points);
			long double variance = get_variance(timing_points);
			long double std_dev = get_std_dev(timing_points);
			long double median = get_median(timing_points);

			extreme_debug_log << "max: " << max << " min: " << min << " times per timestep: " <<  get_times_per_run(output, timing_points) << endl;

			sprintf(buffer, "%30s,%d,%d,%d,%d,%d,%d,%llu,%lu,%lu,%lu,%Lf,%Lf,%Lf,%Lf,%Lf,%Lf", 
					pt_name.c_str(), output.config.num_write_client_procs / output.config.num_server_procs,
					output.config.num_server_nodes, 
					output.config.num_write_client_procs, output.config.num_write_client_nodes, 
					output.config.num_read_client_procs, output.config.num_read_client_nodes, 
					output.config.num_datasets, timing_points.size(), get_times_per_run(output, timing_points),
					get_times_per_proc(output, timing_points),
					max, min, avg, variance, std_dev, median);
			extreme_debug_log << "just wrote buffer" << endl;

			last_first_fs << buffer << endl;
		}	
	}


	//means there is a line between timing info for different configs
	op_fs << endl;
	op_breakdown_fs << endl;
	section_fs << endl;
	// section_breakdown_fs << endl;
	// clock_eval_fs << endl;
	last_first_fs << endl;

cleanup:
	if(op_fs.is_open()) {
		op_fs.close();
	}
	if(op_breakdown_fs.is_open()) {
		op_breakdown_fs.close();
	}
	if(section_fs.is_open()) {
		section_fs.close();
	}
	// if(section_breakdown_fs.is_open()) {
	// 	section_breakdown_fs.close();
	// }
	// if(compare_ops_fs.is_open()) {
	// 	compare_ops_fs.close();
	// }	
	// if(clock_fs.is_open()) {
	// 	clock_fs.close();
	// }
	// }	
	// if(clock_eval_fs.is_open()) {
	// 	clock_eval_fs.close();
	// }
	if(last_first_fs.is_open()) {
		last_first_fs.close();
	}

	return rc;
}




static int write_output_read_client(const string &results_path, bool first_run_clients, const struct read_client_config_output &output) {
	//note: these will in essence be key value tables with key: config params

	int rc = RC_OK;

	vector<vector<vector<long double>>> all_read_pattern_times = {output.read_pattern_op_times, 
				output.read_pattern_gather_times, output.read_pattern_total_gather_times,
				output.read_pattern_objector_times, output.read_pattern_total_objector_times, 
				output.read_pattern_hdf5_read_times, output.read_pattern_total_hdf5_read_times,
				output.read_pattern_times};


	vector<vector<long double>> all_type_read_pattern_times = {output.read_pattern_type_2_times, output.read_pattern_type_3_times};

	// vector<vector<double>> all_timing_points{output.init_times.at(0),output.init_times.at(1),output.init_times.at(2),
	// 									output.init_times.at(3),output.init_times.at(4),output.init_times.at(5),
	// 									output.read_pattern_times,output.read_type_pattern_times,
	// 									output.read_all_pattern_times,output.extra_testing_times,output.reading_times};
	vector< vector<long double>> all_timing_points{output.init_times.at(0),output.init_times.at(1),output.init_times.at(2),
										output.init_times.at(3),output.init_times.at(4),output.init_times.at(5),
										output.read_all_patterns_times,output.read_all_type_patterns_times,
										output.read_all_type_pattern_2_times, output.read_all_type_pattern_3_times,
										output.extra_testing_times,output.reading_times};

	debug_log << "about to start read client output \n";

	int num_read_patterns = 12;
	int num_ops = output.op_times.size();
	int num_sections = all_timing_points.size();

	string pattern_results_path = results_path + "/read_client_patterns.csv";
	ofstream pattern_fs;
	string op_results_path = results_path + "/read_client_ops.csv";
	ofstream op_fs;
	string op_breakdown_path = results_path + "/read_client_op_breakdown.csv";
	ofstream op_breakdown_fs;
	string collective_op_results_path = results_path + "/read_client_collective_ops.csv";
	ofstream collective_op_fs;
	string section_results_path = results_path + "/read_client_sections.csv";
	ofstream section_fs;
	// string compare_total_op_times_path = results_path + "/client_compare_total_op_times.csv";
	// ofstream compare_total_op_times_fs;
	// string clock_results_path = results_path + "/client_clocks.csv";
	// ofstream clock_fs;
	string clock_eval_results_path = results_path + "/read_client_clocks_eval.csv";
	ofstream clock_eval_fs;

	debug_log << "output.filenames.at(0): " << output.filenames.at(0) << endl;

	vector<double> freqs = {.25, .05, .001};

	if(first_run_clients) { 
		debug_log << "first run \n";
		pattern_fs.open(pattern_results_path, std::fstream::out);
		op_fs.open(op_results_path, std::fstream::out);
		op_breakdown_fs.open(op_breakdown_path, std::fstream::out);
		section_fs.open(section_results_path, std::fstream::out);

		// clock_fs.open(clock_results_path, std::fstream::out);
		clock_eval_fs.open(clock_eval_results_path, std::fstream::out);
		collective_op_fs.open(collective_op_results_path, std::fstream::out);
		// compare_total_op_times_fs.open(compare_total_op_times_path, std::fstream::out);


		pattern_fs << "#is_type: " << endl;
		pattern_fs << "#0:no 1:yes" << endl;
		pattern_fs << endl;
		pattern_fs << "#category: " << endl;
		pattern_fs << "#0:op time, 1:avg gather time, 2:sum gather time, 3:avg objector time, 4:sum objector time, "
				   << "5:avg hdf5 read time, 6:total hdf5 read time, 7:total time" << endl;	
		pattern_fs << endl;
		pattern_fs << "pattern,is_type,Selectivity,category,Write Clients per Server,num_server_nodes,num_write_client_procs,num_write_client_nodes,"
			  	   << "num_read_client_procs,num_read_client_nodes,"
		           << "num_datasets,num data pts,max,min,avg,variance,std deviation,median" << endl;


		op_fs << "op,Write Clients per Server,num_server_nodes,num_write_client_procs,num_write_client_nodes,"
			  << "num_read_client_procs,num_read_client_nodes,"
		      << "num_datasets,num data pts,max,min,avg,variance,std deviation,median" << endl;

		collective_op_fs << "timing categories: " << endl;
		collective_op_fs << "0:gather time, 1:objector time, 2: hdf5 read time, 3:total collective op time" << endl;
		collective_op_fs << endl;
		collective_op_fs << "op,category,Write Clients per Server,num_server_nodes,num_write_client_procs,num_write_client_nodes,"
			  << "num_read_client_procs,num_read_client_nodes,"
		      << "num_datasets,num data pts,max,min,avg,variance,std deviation,median" << endl;

		op_breakdown_fs << "op,Write Clients per Server,num_server_nodes,num_write_client_procs,num_write_client_nodes,"
			  << "num_read_client_procs,num_read_client_nodes,"
		      << "num_datasets,num data pts,avg serialize time,avg send time,avg received return msg time,avg deserialize time,avg total time" << endl;


		section_fs << "#section categories: " << endl;
		section_fs << "#0:mpi init time, 1:md_client init time, 2:init vars time, 3: dirman setup time, "
				   << "4: server setup time, 5: total init time, 6:total for 6 read patterns (with no type), "
				   << "7:total for read patterns (2&3) w. types, 8:all type pattern 2 times, 9:all type pattern 3 times,"
				   << "10:extra testing time, "
				   << "11:total read time" << endl;
		section_fs << endl;
		section_fs << "section,Write Clients per Server,num_server_nodes,num_write_client_procs,num_write_client_nodes,"
				   << "num_read_client_procs,num_read_client_nodes,"
		           << "num_datasets,num data pts,max,min,avg,variance,std deviation,median" << endl;

			 	//start, mpi init done, register done, db_init done, dirman_init_done, finalize
		// clock_fs << "categories: " << endl;
		// clock_fs << "0:start, 1:mpi init done, 2:register ops done, 3:dirman init done, 4:server setup done"
		// 	     << "5:writing create done, 6:writing insert done, 7:read patterns done, 8:extra testing done" << endl;
		// clock_fs << endl;
		// clock_fs << "category,Write Clients per Server,num_server_nodes,num_write_client_procs,num_write_client_nodes,"
		           	  // << "num_read_client_procs,num_read_client_nodes,"
		// 		 << "num_datasets,max,min,avg,variance,std deviation,median" << endl;

		// clock_eval_fs << "categories: " << endl;
		// clock_eval_fs << "0:full init time, 1:full read pattern time, 2:extra testing time, "
		// 			  << "3:total read time, "
		// 			  << "4:full run time without extra, 5:full run time with extra" << endl;
		// clock_eval_fs << endl;
		clock_eval_fs << "#Numbering guide:" << endl;
		clock_eval_fs << "#0: init time" << endl;
		clock_eval_fs << "#50: read init time" << endl;
		clock_eval_fs << "#100: pattern 1" << endl;
		clock_eval_fs << "#200: pattern 2" << endl;
		clock_eval_fs << "#210: pattern 2 with types" << endl;
		// clock_eval_fs << "#210: pattern 2 with type 0" << endl;
		// clock_eval_fs << "#211: pattern 2 with type 1" << endl;
		// clock_eval_fs << "#212: pattern 2 with type 2" << endl;
		clock_eval_fs << "#300: pattern 3" << endl;
		clock_eval_fs << "#310: pattern 3 with types" << endl;
		// clock_eval_fs << "#310: pattern 3 with type 0" << endl;
		// clock_eval_fs << "#311: pattern 3 with type 1" << endl;
		// clock_eval_fs << "#312: pattern 3 with types 2" << endl;
		clock_eval_fs << "#400: pattern 4" << endl;
		clock_eval_fs << "#500: pattern 5" << endl;
		clock_eval_fs << "#600: pattern 6" << endl;
		clock_eval_fs << endl;
		clock_eval_fs << "#For numbers XX00, see the op numbering guide (op_numbering_guide.csv)" << endl;
		clock_eval_fs << endl;
		clock_eval_fs << "#XX10 - ABOVE_MAX op instead of range" << endl;
		clock_eval_fs << "#XX20 - BELOW_MIN op instead of range" << endl;
		clock_eval_fs << "#XX30 - Gather" << endl;
		clock_eval_fs << "#XX40 - Gather for ABOVE_MAX op instead of range" << endl;
		clock_eval_fs << "#XX50 - Gather for BELOW_MIN op instead of range" << endl;
		clock_eval_fs << endl;



		clock_eval_fs << "category,Write Clients per Server,num_server_nodes,num_write_client_procs,num_write_client_nodes,"
				      << "num_read_client_procs,num_read_client_nodes,"
				 	  << "num_datasets,num data pts,max,min,avg,variance,std deviation,median" << endl;

	}
	else {
		pattern_fs.open(pattern_results_path, std::fstream::app);
		op_fs.open(op_results_path, std::fstream::app);
		op_breakdown_fs.open(op_breakdown_path, std::fstream::app);
		collective_op_fs.open(collective_op_results_path, std::fstream::app);		
		section_fs.open(section_results_path, std::fstream::app);
		// clock_fs.open(clock_results_path, std::fstream::app);
		// compare_total_op_times_fs.open(section_results_path, std::fstream::app);
		clock_eval_fs.open(clock_eval_results_path, std::fstream::app);
	}

	
	if(!pattern_fs) {
		error_log << "error. failed to open pattern results file" << endl;
		rc = RC_ERR;
		goto cleanup;	
	}
	else if(!op_fs) {
		error_log << "error. failed to open ops results file: " << op_results_path << endl;
		rc = RC_ERR;
		goto cleanup;
	}
	else if(!collective_op_fs) {
		error_log << "error. failed to open collective op results file" << endl;
		rc = RC_ERR;
		goto cleanup;
	}
	else if(!op_breakdown_fs) {
		error_log << "error. failed to open ops breakdown file" << endl;
		rc = RC_ERR;
		goto cleanup;
	}
	else if(!section_fs) {
		error_log << "error. failed to open section results file" << endl;
		rc = RC_ERR;
		goto cleanup;	
	}
	// else if(!compare_total_op_times_fs) {
	// 	error_log << "error. failed to open total ops results file" << endl;
	// 	rc = RC_ERR;
	// 	goto cleanup;	
	// }
	// else if(!clock_fs) {
	// 	debug_log << "error. failed to open clock results file" << endl;
	// 	rc = RC_ERR;
	// 	goto cleanup;	
	// }
	else if(!clock_eval_fs) {
		error_log << "error. failed to open clock results file" << endl;
		rc = RC_ERR;
		goto cleanup;	
	}
	char buffer[1024];


	for(int pattern_indx=0; pattern_indx<num_read_patterns; pattern_indx++) {
		for(int category = 0; category < all_read_pattern_times.size(); category++) {
			vector<long double> read_pattern_times = all_read_pattern_times.at(category).at(pattern_indx);

			debug_log << "pattern size for pattern " << pattern_indx + 1 << ": " << read_pattern_times.size() << endl;

			if(read_pattern_times.size() > 0) {
				long double max = get_max(read_pattern_times);
				long double min = get_min(read_pattern_times);
				long double avg = get_mean(read_pattern_times);
				long double variance = get_variance(read_pattern_times);
				long double std_dev = get_std_dev(read_pattern_times);
				long double median = get_median(read_pattern_times);
				extreme_debug_log << "pattern: " << pattern_indx+1 << " category: " << category << endl;
				extreme_debug_log << "max: " << max << " min: " << min << " avg: " << avg << " variance: " << variance << endl;

				// for (long double val : read_pattern_times ) {
				// 	extreme_debug_log << "val: " << val << endl;
				// }

					// debug_log << "my rank: " << rank << "and time pt index: " << i << " and pt: " << output->read_pattern_1_times.size() << endl;
				int pattern = pattern_indx%6+1;
				bool is_type = pattern_indx/6;
				float freq = 1;
				if(is_type) {
					freq = freqs[ (pattern-1) % 3];
				}

				sprintf(buffer, "%d,%d,%f,%d,%d,%d,%d,%d,%d,%d,%llu,%lu,%Lf,%Lf,%Lf,%Lf,%Lf,%Lf", 
						pattern,is_type,freq,category,output.config.num_write_client_procs / output.config.num_server_procs,
						output.config.num_server_nodes, 
						output.config.num_write_client_procs, output.config.num_write_client_nodes, 
						output.config.num_read_client_procs, output.config.num_read_client_nodes, 
						output.config.num_datasets, read_pattern_times.size(), max, min, avg, variance, std_dev, median);	

				pattern_fs << buffer << endl;
			}
		}
	}

	// for(int category = 0; category < all_type_read_pattern_times.size(); category++) {
	// 	bool is_type = true;
	// 	for(int i = 0; i < 3; i++) {

	// 		vector<long double> this_read_pattern_times = all_type_read_pattern_times.at(category);
	// 		vector<long double> read_pattern_times;

	// 		for(int j = i; j < this_read_pattern_times.size(); j+=3) {
	// 			read_pattern_times.push_back(this_read_pattern_times.at(j));
	// 		}

	// 		// debug_log << "pattern size for pattern " << pattern_indx + 1 << ": " << read_pattern_times.size() << endl;

	// 		if(read_pattern_times.size() > 0) {
	// 			long double max = get_max(read_pattern_times);
	// 			long double min = get_min(read_pattern_times);
	// 			long double avg = get_mean(read_pattern_times);
	// 			long double variance = get_variance(read_pattern_times);
	// 			long double std_dev = get_std_dev(read_pattern_times);
	// 			long double median = get_median(read_pattern_times);
	// 			// extreme_debug_log << "pattern: " << pattern_indx+1 << " category: " << category << endl;
	// 			extreme_debug_log << "max: " << max << " min: " << min << " avg: " << avg << " variance: " << variance << endl;

	// 			// for (long double val : read_pattern_times ) {
	// 			// 	extreme_debug_log << "val: " << val << endl;
	// 			// }
	// 				// debug_log << "my rank: " << rank << "and time pt index: " << i << " and pt: " << output->read_pattern_1_times.size() << endl;
	// 			// int pattern = pattern_indx%6+1;
	// 			int pattern = category + 1;

	// 			sprintf(buffer, "%d,%d,%f,%d,%d,%d,%d,%d,%d,%d,%llu,%lu,%Lf,%Lf,%Lf,%Lf,%Lf,%Lf", 
	// 					pattern,is_type,freqs[i],category,output.config.num_write_client_procs / output.config.num_server_procs,
	// output.config.num_server_nodes, 
	// 					output.config.num_write_client_procs, output.config.num_write_client_nodes, 
	// 					output.config.num_read_client_procs, output.config.num_read_client_nodes, 
	// 					output.config.num_datasets, read_pattern_times.size(), max, min, avg, variance, std_dev, median);	

	// 			pattern_fs << buffer << endl;

	// 		}
	// 		// if (!is_type) {
	// 		// 	break;
	// 		// }
	// 	}
	// }

	for(int op_index=0; op_index < num_ops; op_index++) {
		vector<vector<long double>> op_pieces_times = output.op_times[op_index];
		vector<long double> total_op_times = op_pieces_times.at(op_pieces_times.size()-1);
		debug_log << "read client - num time pts for op " << op_index << ": " << total_op_times.size() << endl;
		debug_log << "read client - num categories for op breakdown " << op_index << ": " << op_pieces_times.size() << endl;

		if(total_op_times.size() > 0) {
			long double max = get_max(total_op_times);
			long double min = get_min(total_op_times);
			long double avg = get_mean(total_op_times);
			long double variance = get_variance(total_op_times);
			long double std_dev = get_std_dev(total_op_times);
			long double median = get_median(total_op_times);
			sprintf(buffer, "%d,%d,%d,%d,%d,%d,%d,%llu,%lu,%Lf,%Lf,%Lf,%Lf,%Lf,%Lf", 
					op_index*100 + 1000, output.config.num_write_client_procs / output.config.num_server_procs,
					output.config.num_server_nodes, 
					output.config.num_write_client_procs, output.config.num_write_client_nodes, 
					output.config.num_read_client_procs, output.config.num_read_client_nodes, 
					output.config.num_datasets, total_op_times.size(), max, min, avg, variance, std_dev, median);
			op_fs << buffer << endl;

			if(op_pieces_times.size() == 5) {
				long double avg_ser_time = get_mean(op_pieces_times.at(0));
				long double avg_send_time = get_mean(op_pieces_times.at(1));
				long double avg_rec_ret_time = get_mean(op_pieces_times.at(2));
				long double avg_deser_time = get_mean(op_pieces_times.at(3));
				long double avg_total_time = get_mean(op_pieces_times.at(4));

				sprintf(buffer, "%d,%d,%d,%d,%d,%d,%d,%llu,%lu,%Lf,%Lf,%Lf,%Lf,%Lf", 
					op_index*100 + 1000, output.config.num_write_client_procs / output.config.num_server_procs,
					output.config.num_server_nodes, 
					output.config.num_write_client_procs, output.config.num_write_client_nodes, 
					output.config.num_read_client_procs, output.config.num_read_client_nodes, 
					output.config.num_datasets, op_pieces_times.at(4).size(), 
					avg_ser_time, avg_send_time, avg_rec_ret_time, avg_deser_time, avg_total_time);

				op_breakdown_fs << buffer << endl;
			}
			else {
				error_log << "error. op " << op_index << " had " << op_pieces_times.size() << " time categories \n";
			}
		}
	}

	for(int op_index=0; op_index < num_ops; op_index++) {
		vector<vector<long double>> op_pieces_times = output.collective_op_times[op_index];
		debug_log << "read client - num time points for collective op " << op_index << ": " << op_pieces_times.at(0).size() << endl;
		debug_log << "read client - num cateogries for collective op breakdown " << op_index << ": " << op_pieces_times.size() << endl;

		for (int category = 0; category < op_pieces_times.size(); category++) {
			vector<long double> category_times = op_pieces_times.at(category);

			if(category_times.size() > 0) {
				long double max = get_max(category_times);
				long double min = get_min(category_times);
				long double avg = get_mean(category_times);
				long double variance = get_variance(category_times);
				long double std_dev = get_std_dev(category_times);
				long double median = get_median(category_times);
				sprintf(buffer, "%d,%d,%d,%d,%d,%d,%d,%d,%llu,%lu,%Lf,%Lf,%Lf,%Lf,%Lf,%Lf", 
						op_index*100 + 1000, category, output.config.num_write_client_procs / output.config.num_server_procs,
						output.config.num_server_nodes, 
						output.config.num_write_client_procs, output.config.num_write_client_nodes, 
						output.config.num_read_client_procs, output.config.num_read_client_nodes, 
						output.config.num_datasets, 
						category_times.size(), max, min, avg, variance, std_dev, median);
				collective_op_fs << buffer << endl;
			}
		}
	}

	for(int section_index=0; section_index<num_sections; section_index++) {
		vector<long double> timing_points = all_timing_points.at(section_index);
		debug_log << "size for section " << section_index << ": " << timing_points.size() << endl;

		// for (long double time_pt : timing_points) {
		// 	extreme_debug_log << "time_pt: " << time_pt << endl;
		// }

		if(timing_points.size() > 0) {	
			long double max = get_max(timing_points);
			extreme_debug_log << "max: " << max << endl;
			long double min = get_min(timing_points);
			long double avg = get_mean(timing_points);
			long double variance = get_variance(timing_points);
			long double std_dev = get_std_dev(timing_points);
			long double median = get_median(timing_points);

			sprintf(buffer, "%d,%d,%d,%d,%d,%d,%d,%llu,%lu,%Lf,%Lf,%Lf,%Lf,%Lf,%Lf", 
					section_index, output.config.num_write_client_procs / output.config.num_server_procs,
					output.config.num_server_nodes, 
					output.config.num_write_client_procs, output.config.num_write_client_nodes, 
					output.config.num_read_client_procs, output.config.num_read_client_nodes, 
					output.config.num_datasets, timing_points.size(), max, min, avg, variance, std_dev, median);
			section_fs << buffer << endl;
		}
	}

	// for(int i=0; i<output.all_clock_times.size(); i++) {
	// 	vector<long double> clock_times = output.all_clock_times.at(i);
	// 	debug_log << "clock times for category " << i << ": " << clock_times.size() << endl;
	// 	if(clock_times.size() > 0) {
	// 		long double max = get_max(clock_times);
	// 		long double min = get_min(clock_times);
	// 		long double avg = get_mean(clock_times);
	// 		long double variance = get_variance(clock_times);
	// 		long double std_dev = get_std_dev(clock_times);
	// 		long double median = get_median(clock_times);
	// 		sprintf(buffer, "%d,%d,%d,%d,%d,%d,%d,%llu,%d,%Lf,%Lf,%Lf,%Lf,%Lf,%Lf\n", 
					// i, output.config.num_write_client_procs / output.config.num_server_procs,
					// output.config.num_server_nodes, 
					// output.config.num_write_client_procs, output.config.num_write_client_nodes, 
					// output.config.num_read_client_procs, output.config.num_read_client_nodes, 
					// output.config.num_datasets, 
					// clock_times.size(), max, min, avg, variance, std_dev, median);
	// 		clock_fs << buffer;
	// 	}
	// }

	//struct timeval start, mpi_init_done, register_ops_done, db_init done, dirman_init_done /
	debug_log << "output.clock_times_eval.size(): " << output.clock_times_eval.size() << endl;
	for(int i=0; i<output.clock_times_eval.size(); i++) {
		vector<long double> clock_times_eval = output.clock_times_eval.at(i);
		debug_log << "clock times eval for category " << i << ": " << clock_times_eval.size() << endl;
		if(clock_times_eval.size() > 0) {
			long double max = get_max(clock_times_eval);
			long double min = get_min(clock_times_eval);
			long double avg = get_mean(clock_times_eval);
			long double variance = get_variance(clock_times_eval);
			long double std_dev = get_std_dev(clock_times_eval);
			long double median = get_median(clock_times_eval);
			// extreme_debug_log << "output.clock_times_eval_catgs.at(i): " << output.clock_times_eval_catgs.at(i) << endl;
			sprintf(buffer, "%d,%d,%d,%d,%d,%d,%d,%llu,%lu,%Lf,%Lf,%Lf,%Lf,%Lf,%Lf", 
					output.clock_times_eval_catgs.at(i), output.config.num_write_client_procs / output.config.num_server_procs,
					output.config.num_server_nodes, 
					output.config.num_write_client_procs, output.config.num_write_client_nodes, 
					output.config.num_read_client_procs, output.config.num_read_client_nodes, 
					output.config.num_datasets, clock_times_eval.size(), max, min, avg, variance, std_dev, median);
			clock_eval_fs << buffer << endl;
			// cout << "just wrote buffer: " << buffer << endl;
		}
	}

	//means there is a line between timing info for different configs
	pattern_fs << endl;
	op_fs << endl;
	op_breakdown_fs << endl;
	section_fs << endl;
	clock_eval_fs << endl;
	collective_op_fs << endl;

cleanup:
	if(pattern_fs.is_open()) {
		pattern_fs.close();
	}
	if(op_fs.is_open()) {
		op_fs.close();
	}
	if(op_breakdown_fs.is_open()) {
		op_breakdown_fs.close();
	}
	if(section_fs.is_open()) {
		section_fs.close();
	}
	// if(compare_ops_fs.is_open()) {
	// 	compare_ops_fs.close();
	// }	
	// if(clock_fs.is_open()) {
	// 	clock_fs.close();
	// }
	// }	
	if(clock_eval_fs.is_open()) {
		clock_eval_fs.close();
	}
	if(collective_op_fs.is_open()) {
		collective_op_fs.close();
	}
	return rc;
}


static int write_output_server(const string &results_path, bool first_run_servers, 
					const struct server_config_output &output, const string &output_type) {
	//note: these will in essence be key value tables with key: config params

	debug_log << "got here \n";
	int rc = RC_OK;

	debug_log << "about to start writing server output \n";

	int num_ops = output.op_times.size();
	debug_log << "num ops: " << num_ops << endl;

	vector <vector<double>> all_timing_points{output.init_times.at(0),output.init_times.at(1),output.init_times.at(2),
										output.init_times.at(3),output.init_times.at(4),
										output.run_times,output.shutdown_times,output.total_run_times};

	int num_sections = all_timing_points.size();

	bool writing = false;

	if(output_type == "write") {
		writing = true;
	}

	string op_results_path = results_path + "/" + output_type + "_server_ops.csv";
	ofstream op_fs;
	string op_breakdown_path = results_path + "/" + output_type + "_server_op_breakdown.csv";
	ofstream op_breakdown_fs;
	string section_results_path = results_path + "/" + output_type +"_server_sections.csv";
	ofstream section_fs;
	string storage_results_path = results_path + "/" + output_type +"_server_storage.csv";
	ofstream storage_fs;
	string clock_eval_results_path = results_path + "/" + output_type +"_server_clocks_eval.csv";
	ofstream clock_eval_fs;

	 vector<string> storage_pts_names = {"DB Initial", "DB Final", "Num Runs", "Num Timesteps",
						"Num Vars", "Num Types", "Num Run Attrs", "Num Timestep Attrs", "Num Var Attrs"};

	if(first_run_servers) {
		op_fs.open(op_results_path, std::fstream::out);
		op_breakdown_fs.open(op_breakdown_path, std::fstream::out);
		section_fs.open(section_results_path, std::fstream::out);
		if(writing) {
			storage_fs.open(storage_results_path, std::fstream::out);
		}	
		clock_eval_fs.open(clock_eval_results_path, std::fstream::out);	

		op_fs << "op,Write Clients per Server,num_server_nodes,num_write_client_procs,num_write_client_nodes,"
			  << "num_read_client_procs,num_read_client_nodes,"
		      << "num_datasets,num data pts,max,min,avg,variance,std deviation,median" << endl;

		op_breakdown_fs << "op,Write Clients per Server,num_server_nodes,num_write_client_procs,num_write_client_nodes,"
			  << "num_read_client_procs,num_read_client_nodes,"
		      << "num_datasets,num data pts,avg deserialize time,avg db time,avg serialize time,"
		      << "avg create msg,avg ser-create-send msg,avg total time" << endl;

		section_fs << "#section categories: " << endl;
		section_fs << "#0:mpi init time, 1:register ops time, 2: db setup time, 3: dirman setup time, "
				   << "4: total init time, 5:run time, 6:shutdown time, 7:total run time" << endl;
		section_fs << endl;
		section_fs << "section,Write Clients per Server,num_server_nodes,num_write_client_procs,num_write_client_nodes,"
				   << "num_read_client_procs,num_read_client_nodes,"
		           << "num_datasets,num data pts,max,min,avg,variance,std deviation,median" << endl;

		if(writing) {
			// storage_fs << "#categories: " << endl;
			// storage_fs << "#0:init size, 1:final size, 2:num runs, 3:num timesteps, 4:num vars, 5:num types, "
			// 		   << "6:num run attrs, 7:num timestep attrs, 8: num var attrs << endl;
			// storage_fs << endl;
			storage_fs << "Storage Type,Clients per Server,num_server_nodes,num_write_client_procs,num_write_client_nodes,"
					   << "num_read_client_procs,num_read_client_nodes,"
					   << "num_datasets,num data pts,max,min,avg,variance,std deviation,median,avg total per iteration" << endl;
	    }
		clock_eval_fs << "#categories: " << endl;
		clock_eval_fs << "#0:full init time, 1:full run (minus init)time, 2:full run time" << endl;
		clock_eval_fs << endl;
		clock_eval_fs << "category,Write Clients per Server,num_server_nodes,num_write_client_procs,num_write_client_nodes,"
				      << "num_read_client_procs,num_read_client_nodes,"
				 	  << "num_datasets,num data pts,max,min,avg,variance,std deviation,median" << endl;
	}
	else {
		op_fs.open(op_results_path, std::fstream::app);
		op_breakdown_fs.open(op_breakdown_path, std::fstream::app);
		section_fs.open(section_results_path, std::fstream::app);
		if (writing) {
			storage_fs.open(storage_results_path, std::fstream::app);
		}
		clock_eval_fs.open(clock_eval_results_path, std::fstream::app);	
	}
	//start, mpi init done, register done, db_init done, dirman_init_done, finalize

	

	if(!op_fs) {
		error_log << "error. failed to open ops results file: " << op_results_path << endl;
		rc = RC_ERR;
		goto cleanup;
	}
	else if(!op_breakdown_fs) {
		error_log << "error. failed to open ops breakdown file" << endl;
		rc = RC_ERR;
		goto cleanup;
	}
	else if(!section_fs) {
		error_log << "error. failed to open section results file" << endl;
		rc = RC_ERR;
		goto cleanup;
	}
	else if(writing && !storage_fs) {
		error_log << "error. failed to open storage results file" << endl;
		rc = RC_ERR;
		goto cleanup;
	}
	else if(!clock_eval_fs) {
		error_log << "error. failed to open clock results file" << endl;
		rc = RC_ERR;
		goto cleanup;
	}
	char buffer[512];


	for(int op_index=0; op_index < num_ops; op_index++) {
		vector<vector<double>> op_pieces_times = output.op_times[op_index];
		for(int i = 0; i < op_pieces_times.size(); i++) {
			for (int j = 0; j < op_pieces_times.at(i).size(); j++) {
				op_pieces_times.at(i).at(j) = op_pieces_times.at(i).at(j) / pow(10, 9);
			}
		}	
		vector<double> total_op_times = op_pieces_times.at(op_pieces_times.size()-1);
		debug_log << "write server - num time pts for op " << op_index << ": " << total_op_times.size() << endl;
		debug_log << "write server - num categories for op breakdown " << op_index << ": " << op_pieces_times.size() << endl;


		if(total_op_times.size() > 0) {
			double max = get_max(total_op_times);
			double min = get_min(total_op_times);
			double avg = get_mean(total_op_times);
			double variance = get_variance(total_op_times);
			double std_dev = get_std_dev(total_op_times);
			double median = get_median(total_op_times);
			sprintf(buffer, "%d,%d,%d,%d,%d,%d,%d,%llu,%lu,%lf,%lf,%lf,%lf,%lf,%lf", 
					op_index*100 + 1000, output.config.num_write_client_procs / output.config.num_server_procs,
					output.config.num_server_nodes, 
					output.config.num_write_client_procs, output.config.num_write_client_nodes, 
					output.config.num_read_client_procs, output.config.num_read_client_nodes, 
					output.config.num_datasets, total_op_times.size(), max, min, avg, variance, std_dev, median);
			op_fs << buffer << endl;

			if(op_pieces_times.size() == 6) {
				double avg_deser_time = get_mean(op_pieces_times.at(0));
				double avg_db_time = get_mean(op_pieces_times.at(1));
				double avg_ser_time = get_mean(op_pieces_times.at(2));
				double avg_create_time = get_mean(op_pieces_times.at(3));
				double avg_ser_create_send_time = get_mean(op_pieces_times.at(4));
				double avg_total_time = get_mean(op_pieces_times.at(5));
				sprintf(buffer, "%d,%d,%d,%d,%d,%d,%d,%llu,%lu,%lf,%lf,%lf,%lf,%lf,%lf", 
					op_index*100 + 1000, output.config.num_write_client_procs / output.config.num_server_procs,
					output.config.num_server_nodes, 
					output.config.num_write_client_procs, output.config.num_write_client_nodes, 
					output.config.num_read_client_procs, output.config.num_read_client_nodes, 
					output.config.num_datasets, op_pieces_times.at(5).size(), 
					avg_deser_time, avg_db_time, avg_ser_time, avg_create_time, avg_ser_create_send_time, avg_total_time);
				op_breakdown_fs << buffer << endl;
			}
			else {
				error_log << "error. op " << op_index << " had " << op_pieces_times.size() << " time categories \n";
			}
		}
	}

	for(int section_index=0; section_index<num_sections; section_index++) {
		vector<double> timing_points = all_timing_points.at(section_index);
		for(int i = 0; i < timing_points.size(); i++) {
			timing_points[i] = (timing_points[i] / pow(10, 9));
		}
		if(timing_points.size() > 0) {
			double max = get_max(timing_points);
			double min = get_min(timing_points);
			double avg = get_mean(timing_points);
			double variance = get_variance(timing_points);
			double std_dev = get_std_dev(timing_points);
			double median = get_median(timing_points);

			sprintf(buffer, "%d,%d,%d,%d,%d,%d,%d,%llu,%lu,%lf,%lf,%lf,%lf,%lf,%lf", 
					section_index, output.config.num_write_client_procs / output.config.num_server_procs,
					output.config.num_server_nodes, 
					output.config.num_write_client_procs, output.config.num_write_client_nodes, 
					output.config.num_read_client_procs, output.config.num_read_client_nodes, 
					output.config.num_datasets, timing_points.size(), max, min, avg, variance, std_dev, median);
			section_fs << buffer << endl;
		}
	}

	if (writing) {
		for(int i=0; i<output.all_storage_sizes.size(); i++) {
			vector<uint64_t> storage_sizes = output.all_storage_sizes.at(i);
			debug_log << "storage sizes for category " << i << ": " << storage_sizes.size() << endl;
			if(storage_sizes.size() > 0) {
				uint64_t max = get_max(storage_sizes);
				uint64_t min = get_min(storage_sizes);
				uint64_t avg = get_mean(storage_sizes);
				uint64_t variance = get_variance(storage_sizes);
				uint64_t std_dev = get_std_dev(storage_sizes);
				uint64_t median = get_median(storage_sizes);
				// if(i >= 6) { //num attrs (run, timestep or var)
					//sum all of the #s across all of the iterations, then multiply by (#of servers/total #servers across all iterations) 
					uint64_t total = output.config.num_server_procs * std::accumulate(storage_sizes.begin(), storage_sizes.end(), 0.0) / storage_sizes.size();
					debug_log << "i: " << i << " total: " << total << endl;
		
					sprintf(buffer, "%s,%d,%d,%d,%d,%d,%d,%llu,%lu,%llu,%llu,%llu,%llu,%llu,%llu,%llu", 
						storage_pts_names.at(i).c_str(), output.config.num_write_client_procs / output.config.num_server_procs,
						output.config.num_server_nodes, 
						output.config.num_write_client_procs, output.config.num_write_client_nodes, 
						output.config.num_read_client_procs, output.config.num_read_client_nodes, 
						output.config.num_datasets, 
						storage_sizes.size(), max, min, avg, variance, std_dev, median, total);	
				// }
				// else {
				// 	sprintf(buffer, "%s,%d,%d,%d,%d,%d,%d,%llu,%lu,%llu,%llu,%llu,%llu,%llu,%llu", 
				// 		storage_pts_names.at(i).c_str(), output.config.num_write_client_procs / output.config.num_server_procs,
				output.config.num_server_nodes, 
				// 		output.config.num_write_client_procs, output.config.num_write_client_nodes, 
				// 		output.config.num_read_client_procs, output.config.num_read_client_nodes, 
				// 		output.config.num_datasets, 
				// 		storage_sizes.size(), max, min, avg, variance, std_dev, median, );	
				// }
				storage_fs << buffer << endl;
			}
		}
	}

	//struct timeval start, mpi_init_done, register_ops_done, db_init done, dirman_init_done /
	for(int i=0; i<output.clock_times_eval.size(); i++) {
		vector<long double> clock_times_eval = output.clock_times_eval.at(i);
		debug_log << "clock times eval for category " << i << ": " << clock_times_eval.size() << endl;
		if(clock_times_eval.size() > 0) {
			long double max = get_max(clock_times_eval);
			long double min = get_min(clock_times_eval);
			long double avg = get_mean(clock_times_eval);
			long double variance = get_variance(clock_times_eval);
			long double std_dev = get_std_dev(clock_times_eval);
			long double median = get_median(clock_times_eval);
			sprintf(buffer, "%d,%d,%d,%d,%d,%d,%d,%llu,%lu,%Lf,%Lf,%Lf,%Lf,%Lf,%Lf",
					i, output.config.num_write_client_procs / output.config.num_server_procs,
					output.config.num_server_nodes, 
					output.config.num_write_client_procs, output.config.num_write_client_nodes, 
					output.config.num_read_client_procs, output.config.num_read_client_nodes, 
					output.config.num_datasets, clock_times_eval.size(), max, min, avg, variance, std_dev, median);	
			clock_eval_fs << buffer << endl;
		}
	}

	//means there is a line between timing info for different configs
	op_fs << endl;
	op_breakdown_fs << endl;
	section_fs << endl;
	if (writing) {
		storage_fs << endl;	
	}
	clock_eval_fs << endl;

cleanup:
	if(op_fs.is_open()) {
		op_fs.close();
	}
	if(op_breakdown_fs.is_open()) {
		op_breakdown_fs.close();
	}
	if(section_fs.is_open()) {
		section_fs.close();
	}
	if(writing && storage_fs.is_open()) {
		storage_fs.close();
	}
	if(clock_eval_fs.is_open()) {
		clock_eval_fs.close();
	}
	return rc;
}


static int write_output_dirman(const string &results_path, bool first_run_dirman, 
							const struct dirman_config_output &output, const string &output_type) {
	//note: these will in essence be key value tables with key: rank with the addition of num clients, num servers, and num client and server nodes in addition

	// int num_time_pts = all_time_pts.size();
	int rc = RC_OK;

	debug_log << "about to start writing dirmans output \n";


	string time_pts_results_path = results_path + "/" + output_type + "_dirman_time_pts.csv";
	ofstream time_pts_fs;
	// string clock_results_path = results_path + "/dirman_clocks.csv";
	// ofstream clock_fs;
	string clock_eval_results_path = results_path + "/" + output_type + "_dirman_clocks_eval.csv";
	ofstream clock_eval_fs;

	if(first_run_dirman) {
		time_pts_fs.open(time_pts_results_path, std::fstream::out);
		time_pts_fs << "#categories: " << endl;
		time_pts_fs << "#0:mpi init done, 1:dirman setup done, 2:register op done, 3:generate contact info done, 4:run time" << endl;
		time_pts_fs << endl;
		time_pts_fs << "code,Write Clients per Server,num_server_nodes,num_write_client_procs,num_write_client_nodes,"
			  		<< "num_read_client_procs,num_read_client_nodes,"
		            << "num_datasets,num data pts,max,min,avg,variance,std deviation,median" << endl;

	 //    //struct timeval start, mpi_init_done, dirman_init_done, register_done, generate_done, finalize;
		// clock_fs.open(clock_results_path, std::fstream::out);	
		// clock_fs << "categories: " << endl;
		// clock_fs << "0:start, 1:mpi init done, 2:dirman init done, 3:register ops done, 4:generate contact info done, 5:finalize" << endl;
		// clock_fs << endl;
		// clock_fs << "category,Write Clients per Server,num_server_nodes,num_write_client_procs,num_write_client_nodes,"
		            	  // << "num_read_client_procs,num_read_client_nodes,"
		// 		 << "num_datasets,max,min,avg,variance,std deviation,median" << endl;

	    //struct timeval start, mpi_init_done, dirman_init_done, register_done, generate_done, finalize;
		clock_eval_fs.open(clock_eval_results_path, std::fstream::out);	
		clock_eval_fs << "#categories: " << endl;
		clock_eval_fs << "#0:full init time, 1:full runtime" << endl;
		clock_eval_fs << endl;
		clock_eval_fs << "category,Write Clients per Server,num_server_nodes,num_write_client_procs,num_write_client_nodes,"
			  		  << "num_read_client_procs,num_read_client_nodes,"
				 	  << "num_datasets,num data pts,max,min,avg,variance,std deviation,median" << endl;
	}
	else {
		time_pts_fs.open(time_pts_results_path, std::fstream::app);
		// clock_fs.open(clock_results_path, std::fstream::app);
		clock_eval_fs.open(clock_eval_results_path, std::fstream::app);
	}

	if(!time_pts_fs) {
		error_log << "error. failed to open time pts results file" << endl;
		rc = RC_ERR;
		goto cleanup;
	}
	// else if(!clock_fs) {
	// 	debug_log << "error. failed to open clock results file" << endl;
	// 	rc = RC_ERR;
	// 	goto cleanup;
	// }
	else if(!clock_eval_fs) {
		error_log << "error. failed to open clock eval results file" << endl;
		rc = RC_ERR;
		goto cleanup;	
	}
	char buffer[512];



	for(int i=0; i<output.all_time_pts.size(); i++) {
		vector<double> time_pts = output.all_time_pts.at(i);
		debug_log << "num time pts for category " << i << ": " << time_pts.size() << endl;
		for(int i = 0; i < time_pts.size(); i++) {
			time_pts[i] = (time_pts[i] / pow(10, 9));
		}
		if(time_pts.size() > 0) {
			double max = get_max(time_pts);
			double min = get_min(time_pts);
			double avg = get_mean(time_pts);
			double variance = get_variance(time_pts);
			double std_dev = get_std_dev(time_pts);
			double median = get_median(time_pts);
			sprintf(buffer, "%d,%d,%d,%d,%d,%d,%d,%llu,%lu,%lf,%lf,%lf,%lf,%lf,%lf", 
					i, output.config.num_write_client_procs / output.config.num_server_procs,
					output.config.num_server_nodes, 
					output.config.num_write_client_procs, output.config.num_write_client_nodes, 
					output.config.num_read_client_procs, output.config.num_read_client_nodes, 
					output.config.num_datasets, time_pts.size(), max, min, avg, variance, std_dev, median);
			time_pts_fs << buffer << endl;		
		}
	}


	// for(int i=0; i<output.all_clock_times.size(); i++) {
	// 	vector<long double> clock_times = output.all_clock_times.at(i);
	// 	debug_log << "clock times for category " << i << ": " << clock_times.size() << endl;
	// 	if(clock_times.size() > 0) {
	// 		long double max = get_max(clock_times);
	// 		long double min = get_min(clock_times);
	// 		long double avg = get_mean(clock_times);
	// 		long double variance = get_variance(clock_times);
	// 		long double std_dev = get_std_dev(clock_times);
	// 		long double median = get_median(clock_times);
	// 		sprintf(buffer, "%d,%d,%d,%d,%d,%d,%d,%llu,%d,%Lf,%Lf,%Lf,%Lf,%Lf,%Lf\n", 
					// i, output.config.num_write_client_procs / output.config.num_server_procs,
					// output.config.num_server_nodes, 
					// output.config.num_write_client_procs, output.config.num_write_client_nodes, 
					// output.config.num_read_client_procs, output.config.num_read_client_nodes, 
					// output.config.num_datasets, 
					// clock_times.size(), max, min, avg, variance, std_dev, median);
	// 		clock_fs << buffer;
	// 	}
	// }


	//struct timeval start, mpi_init_done, register_ops_done, db_init done, dirman_init_done /
	for(int i=0; i<output.clock_times_eval.size(); i++) {
		vector<long double> clock_times_eval = output.clock_times_eval.at(i);
		for(int i = 0; i < clock_times_eval.size(); i++) {
			clock_times_eval[i] = (clock_times_eval[i] / pow(10, 9));
		}	
		debug_log << "clock times eval for category " << i << ": " << clock_times_eval.size() << endl;
		if(clock_times_eval.size() > 0) {
			extreme_debug_log << "clock_times[0]: " << clock_times_eval.at(0) << endl;
			long double max = get_max(clock_times_eval);
			long double min = get_min(clock_times_eval);
			long double avg = get_mean(clock_times_eval);
			long double variance = get_variance(clock_times_eval);
			long double std_dev = get_std_dev(clock_times_eval);
			long double median = get_median(clock_times_eval);
			sprintf(buffer, "%d,%d,%d,%d,%d,%d,%d,%llu,%lu,%Lf,%Lf,%Lf,%Lf,%Lf,%Lf",					
					i, output.config.num_write_client_procs / output.config.num_server_procs,
					output.config.num_server_nodes, 
					output.config.num_write_client_procs, output.config.num_write_client_nodes, 
					output.config.num_read_client_procs, output.config.num_read_client_nodes, 
					output.config.num_datasets, clock_times_eval.size(), max, min, avg, variance, std_dev, median);	
			clock_eval_fs << buffer << endl;
		}
	}


	//means there is a line between timing info for different configs
	time_pts_fs << endl;
	clock_eval_fs << endl;

cleanup:
	if(time_pts_fs.is_open()) {
		time_pts_fs.close();
	}
	// if(clock_fs.is_open()) {
	// 	clock_fs.close();
	// }
	if(clock_eval_fs.is_open()) {
		clock_eval_fs.close();
	}
	return rc;
}


static int write_op_numbering_guide(const string &results_path) {
	int rc = RC_OK;

	debug_log << "about to start writing op numbering guide \n";


	std::vector<string> op_names = {
		"Activate",

		"Catalog All Timesteps with Variable",
		"Catalog All Timesteps with Variable Whose Name Contains Substring",
		"Catalog All Application Runs",
		"Catalog All Timesteps for Run",
		"Catalog All Variables for Timestep",

		"Create Application Run",
		"Create Timestep for Run",
		"Create Variable for Timestep in Run",
		"Create Variables for Timestep in Run in Batch",

		"Delete All Variables in Timestep Whose Name Contains Substring",
		"Delete All Metadata For Run (ID)",
		"Delete All Metadata For Timestep (ID)",
		"Delete All Metadata For Variable (ID)",
		"Delete All Metadata For Variable (Name, Path, Version)",

		"Deactivate/Processing",

		"Catalog All Attributes on Run",
		"Catalog All Attributes on Run With Type",
		"Catalog All Attributes on Run With Type and Value",
		"Catalog All Attributes on Timestep",
		"Catalog All Attributes on Timestep With Type",
		"Catalog All Attributes on Timestep With Type and Value",

		"Catalog All Timesteps with Variable Attributes With Type and Variable",
		"Catalog All Timesteps with Variable Attributes With Type and Variable in Dimensions",
		"Catalog All Timesteps with Variable Attributes With Type, Variable, in Dimensions and with Value",
		"Catalog All Timesteps with Variable Attributes With Type, Variable, and Value",
		"Catalog All Timesteps with Variable Attributes With Type, and Variable Whose Name Contains Substring",
		"Catalog All Timesteps with Variable Attributes With Type, and Variable Whose Name Contains Substring in Dimensions",
		"Catalog All Timesteps with Variable Attributes With Type, and Variable Whose Name Contains Substring in Dimensions and with Value",
		"Catalog All Timesteps with Variable Attributes With Type, and Variable Whose Name Contains Substring with Value",

		"Catalog All Types with An Instance on Any Variable in Timestep",
		"Catalog All Types with An Instance on Variable in Dimensions in Timestep",
		"Catalog All Types with An Instance on Variable in Timestep",
		"Catalog All Types with An Instance on a Variable Whose Name Contains Substring in Dimensions in Timestep",
		"Catalog All Types with An Instance on a Variable Whose Name Contains Substring in Timestep",

		"Catalog All Variable Attributes in Timestep",
		"Catalog All Variable Attributes in Dimensions in Timestep",
		"Catalog All Variable Attributes of Type (ID) in Timestep",
		"Catalog All Variable Attributes of Type (Name and Version) in Timestep",
		"Catalog All Variable Attributes of Type (ID) in Dimensions",
		"Catalog All Variable Attributes of Type (Name and Version) in Dimensions in Timestep",
		"Catalog All Variable Attributes of Type (ID) on Variable (ID) in Timestep",
		"Catalog All Variable Attributes of Type (Name and Version) on Variable (Name and Version) in Timestep",
		"Catalog All Variable Attributes of Type (ID) on Variable (ID) in Dimensions in Timestep",
		"Catalog All Variable Attributes of Type (Name and Version) on Variable (Name and Version) in Dimensions in Timestep",
		"Catalog All Variable Attributes of Type (ID) on Variable (ID) in Dimensions with Value in Timestep",
		"Catalog All Variable Attributes of Type (ID) on Variable (ID) with Value in Timestep",
		"Catalog All Variable Attributes of Type (ID) on Variable Whose Name Contains Substring in Timestep",
		"Catalog All Variable Attributes of Type (ID) on Variable Whose Name Contains Substring in Dimensions in Timestep",
		"Catalog All Variable Attributes of Type (ID) on Variable Whose Name Contains Substring in Dimensions with Value in Timestep",
		"Catalog All Variable Attributes of Type (ID) on Variable Whose Name Contains Substring with Value in Timestep",
		"Catalog All Variable Attributes on Variable (ID) in Timestep",
		"Catalog All Variable Attributes on Variable (Name and Version) in Timestep",
		"Catalog All Variable Attributes on Variable (ID) in Dimensions in Timestep",
		"Catalog All Variable Attributes on Variable (Name and Version) in Dimensions in Timestep",
		"Catalog All Variable Attributes on Variable Whose Name Contains Substring in Timestep",
		"Catalog All Variable Attributes on Variable Whose Name Contains Substring in Dimensions in Timestep",

		"Catalog All Types in Run",
		"Create Type",
		"Create Types in Batch",

		"Delete All Metadata For Type (ID)",
		"Delete All Metadata For Type (Name and Version)",

		"Create Attribute on Run",
		"Create Attributes on in Batch Run",
		"Create Attribute on Timestep",
		"Create Attributes on Timestep in Batch",
		"Create Attribute on Variable",
		"Create Attributes on Variable in Batch",

		"Shutdown",
	};

	string file_path = results_path + "/op_numbering_guide.csv";
	ofstream file_path_fs;


	file_path_fs.open(file_path, std::fstream::out);
	file_path_fs << "op number, op name" << endl;

	if(!file_path_fs) {
		error_log << "error. failed to op numbering guide file" << endl;
		rc = RC_ERR;
		goto cleanup;
	}

	char buffer[512];



	if(op_names.size() != num_client_ops) {
		error_log << "Error. Was expecting " << num_client_ops << " but received " << op_names.size() << " instead" << endl;
	}

	for(int i=0; i<op_names.size(); i++) {
		int op_num = i*100 + 1000;

		sprintf(buffer, "%d,%s", 
				op_num, op_names.at(i).c_str());
		file_path_fs << buffer << endl;		
	}


cleanup:
	if(file_path_fs.is_open()) {
		file_path_fs.close();
	}

	return rc;
}