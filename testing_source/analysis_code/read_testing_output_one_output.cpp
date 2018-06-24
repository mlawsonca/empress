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

static errorLog error_log = errorLog(error_logging);
static debugLog debug_log = debugLog(debug_logging);
static extremeDebugLog extreme_debug_log = extremeDebugLog(extreme_debug_logging);

uint16_t NUM_CLOCK_PTS_CLIENT = 9;
uint16_t NUM_CLOCK_PTS_SERVER = 6; 
uint16_t NUM_CLOCK_PTS_DIRMAN = 6; 

float DEC_PERCENT_CLIENT_PROCS_USED_TO_READ = .1; //fix
uint16_t NUM_SERVER_STORAGE_PTS = 8; //fix - command line param?

std::map <string, client_config_output> client_outputs;
std::map <string, server_config_output> server_outputs;
std::map <string, dirman_config_output> dirman_outputs;


int evaluate_all_servers(const string &file_path, struct server_config_output &output);
int evaluate_all_clients(const string &file_path, struct client_config_output &output);
int evaluate_dirman(const string &file_path, struct dirman_config_output &output);

static int write_output_client(const string &results_path, bool first_run_clients, const struct client_config_output &output);
static int write_output_server(const string &results_path, bool first_run_servers, const struct server_config_output &output);
static int write_output_dirman(const string &results_path, bool first_run_dirman, const struct dirman_config_output &output);

int main(int argc, char **argv) {
	// if(argc != 2) { 
	// 	debug_log << "Error. Usage for " << argv[0] << " is <testing_type>\n";
	// }

	// string testing_type = argv[1];
	string testing_type = "fixed_proc_per_node_testing"; //fix
	string results_path = "/gscratch/mlawso/runtime_serrano/output/fixed_proc_per_node_testing/post_op_speedup_copy/analysis";  //fix
	// string results_path = "/Users/margaretlawson/Sublime_Text/sirius/testing_source/analysis";
	vector<string> clusters;
	// clusters.push_back("serrano");
	clusters.push_back("serrano"); //fix

	for(int i=0; i<clusters.size(); i++) {
		extreme_debug_log << "clusters.size(): " << clusters.size() << endl;
		extreme_debug_log << "i: " << i << endl;
		string cluster_name = clusters.at(i);
		DIR *runtime_output;
		struct dirent *entry;
		char output_dir[124];
		int n = sprintf(output_dir, "/gscratch/mlawso/runtime_serrano/output/fixed_proc_per_node_testing/post_op_speedup_copy"); //fix
		// int n = sprintf(output_dir, "/Users/margaretlawson/Sublime_Text/sirius/testing_source/analysis");

		// string output_type;

		//fix
		// int n = sprintf(output_dir, "/ascldap/users/mlawso/sirius/runtime/runtime_skybridge/run_build");
		// int n = sprintf(output_dir, "/gscratch/mlawso/runtime_%s/output/%s", cluster_name.c_str(), testing_type.c_str());
		if( (runtime_output = opendir(output_dir)) ) {
			while(	(entry = readdir(runtime_output)) ) {
				string filename = (string)entry -> d_name;
				//fix -> should this filter out log2?
				string filename_minus_iteration = filename.substr(0,filename.find(".log")-2);

				if( filename.find("testing_harness") != std::string::npos ) {
					debug_log << "filename: " << filename << endl;
					debug_log << "filename minus iteration: " << filename_minus_iteration << endl;

					if (client_outputs.find(filename_minus_iteration) == client_outputs.end() ) {
						struct client_config_output new_output;
						new_output.filenames.push_back(filename);
						client_outputs[filename_minus_iteration] = new_output;
					}
					else {
						client_outputs.find(filename_minus_iteration)->second.filenames.push_back(filename);
					}
					debug_log << "count of file params: " << (client_outputs.find(filename_minus_iteration)->second).filenames.size() << endl;

				}
				else if( filename.find("my_metadata_server") != std::string::npos ) {
					debug_log << "filename: " << filename << endl;
					debug_log << "filename minus iteration: " << filename_minus_iteration << endl;
					if (server_outputs.find(filename_minus_iteration) == server_outputs.end() ) {
						struct server_config_output new_output;
						new_output.filenames.push_back(filename);
						server_outputs[filename_minus_iteration] = new_output;
					}
					else {
						server_outputs.find(filename_minus_iteration)->second.filenames.push_back(filename);
					}
					debug_log << "count of file params: " << (server_outputs.find(filename_minus_iteration)->second).filenames.size() << endl;					
				}
				else if( filename.find("my_dirman") != std::string::npos ) {
					debug_log << "filename: " << filename << endl;
					debug_log << "filename minus iteration: " << filename_minus_iteration << endl;
					if (dirman_outputs.find(filename_minus_iteration) == dirman_outputs.end() ) {
						struct dirman_config_output new_output;
						new_output.filenames.push_back(filename);
						dirman_outputs[filename_minus_iteration] = new_output;
					}
					else {
						dirman_outputs.find(filename_minus_iteration)->second.filenames.push_back(filename);

					}
					debug_log << "count of file params: " << (dirman_outputs.find(filename_minus_iteration)->second).filenames.size() << endl;		

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
			error_log << "Error. Could not open dir \n";
		}



		//note - currently doens't have options for server proc per node testing, client proc per node testing or txn testing. 
		//for these, please see single config granularity or output per proc versions

	  	for (std::map<string,client_config_output>::iterator it=client_outputs.begin(); it!=client_outputs.end(); ++it) {
	    	string config_params = it->first;
	        client_config_output client_config_struct = it->second;

			sscanf(config_params.c_str(), "testing_harness_%hu_%hu_%hu_%hu_%llu_%hu", &client_config_struct.config.num_server_procs, &client_config_struct.config.num_server_nodes,
				   &client_config_struct.config.num_client_procs, &client_config_struct.config.num_client_nodes, &client_config_struct.config.num_datasets, &client_config_struct.config.num_types);
			client_config_struct.config.num_clock_pts = NUM_CLOCK_PTS_CLIENT;
			uint16_t num_read_procs = DEC_PERCENT_CLIENT_PROCS_USED_TO_READ * client_config_struct.config.num_client_procs;
			if(num_read_procs != (DEC_PERCENT_CLIENT_PROCS_USED_TO_READ * client_config_struct.config.num_client_procs)  ) {
				num_read_procs +=1; //note - this will be fixed in the testing code
			}
			client_config_struct.config.num_read_procs = num_read_procs;
			client_config_struct.all_clock_times = vector<vector<long double>>(NUM_CLOCK_PTS_CLIENT);
			client_config_struct.clock_times_eval = vector<vector<long double>>(6);
			client_config_struct.activate_var_times = vector<vector<double>>(5);
			client_config_struct.catalog_var_times = vector<vector<double>>(5);
			client_config_struct.create_var_times = vector<vector<double>>(5);
			client_config_struct.get_chunk_list_times = vector<vector<double>>(5);
			client_config_struct.get_chunk_times = vector<vector<double>>(5);
			client_config_struct.insert_chunk_times = vector<vector<double>>(5);
			client_config_struct.insert_chunk_times = vector<vector<double>>(5);
			client_config_struct.processing_var_times = vector<vector<double>>(5);
			client_config_struct.activate_type_times = vector<vector<double>>(5);
			client_config_struct.catalog_type_times = vector<vector<double>>(5);
			client_config_struct.create_type_times = vector<vector<double>>(5);
			client_config_struct.get_attr_list_times = vector<vector<double>>(5);
			client_config_struct.get_attr_times = vector<vector<double>>(5);
			client_config_struct.insert_attr_times = vector<vector<double>>(5);
			client_config_struct.processing_type_times = vector<vector<double>>(5);
			client_config_struct.shutdown_times = vector<vector<double>>(5);

			debug_log << "client_config_struct.all_clock_times.size(): " << client_config_struct.all_clock_times.size() << endl;
			debug_log << "client_config_struct.clock_times_eval.size(): " << client_config_struct.clock_times_eval.size() << endl;

			debug_log << "num_server_procs: " << client_config_struct.config.num_server_procs << endl;
			debug_log << "num_server_nodes: " << client_config_struct.config.num_server_nodes << endl;
			debug_log << "num_client_procs: " << client_config_struct.config.num_client_procs << endl;
			debug_log << "num_client_nodes: " << client_config_struct.config.num_client_nodes << endl;
			debug_log << "num_datasets: " << client_config_struct.config.num_datasets << endl;
			debug_log << "num_types: " << client_config_struct.config.num_types << endl;

	        for(int i = 0; i<client_config_struct.filenames.size(); i++) {
	        	string filename = client_config_struct.filenames.at(i);
	        	string file_path = (string)output_dir + "/" + filename;
	        	debug_log << "client file_path: " << file_path << endl;
	        	evaluate_all_clients(file_path, client_config_struct);

	        }
	        client_outputs[config_params] = client_config_struct;

		}

	  	for (std::map<string,server_config_output>::iterator it=server_outputs.begin(); it!=server_outputs.end(); ++it) {
	    	string config_params = it->first;
	        server_config_output server_config_struct = it->second;

			sscanf(config_params.c_str(), "my_metadata_server_%hu_%hu_%hu_%hu_%llu_%hu", &server_config_struct.config.num_server_procs, &server_config_struct.config.num_server_nodes,
				   &server_config_struct.config.num_client_procs, &server_config_struct.config.num_client_nodes, &server_config_struct.config.num_datasets, &server_config_struct.config.num_types);
			server_config_struct.config.num_clock_pts = NUM_CLOCK_PTS_SERVER;
			server_config_struct.config.num_storage_pts = NUM_SERVER_STORAGE_PTS;
			uint16_t num_read_procs = DEC_PERCENT_CLIENT_PROCS_USED_TO_READ * server_config_struct.config.num_client_procs;
			if(num_read_procs != (DEC_PERCENT_CLIENT_PROCS_USED_TO_READ * server_config_struct.config.num_client_procs)  ) {
				num_read_procs +=1; //note - this will be fixed in the testing code
			}
			server_config_struct.config.num_read_procs = num_read_procs;
			server_config_struct.all_clock_times = vector<vector<long double>>(NUM_CLOCK_PTS_SERVER);
			server_config_struct.all_storage_sizes = vector<vector<uint64_t>>(NUM_SERVER_STORAGE_PTS);
			server_config_struct.clock_times_eval = vector<vector<long double>>(2);
			server_config_struct.activate_var_times = vector<vector<double>>(5);
			server_config_struct.catalog_var_times = vector<vector<double>>(5);
			server_config_struct.create_var_times = vector<vector<double>>(5);
			server_config_struct.get_chunk_list_times = vector<vector<double>>(5);
			server_config_struct.get_chunk_times = vector<vector<double>>(5);
			server_config_struct.insert_chunk_times = vector<vector<double>>(5);
			server_config_struct.insert_chunk_times = vector<vector<double>>(5);
			server_config_struct.processing_var_times = vector<vector<double>>(5);
			server_config_struct.activate_type_times = vector<vector<double>>(5);
			server_config_struct.catalog_type_times = vector<vector<double>>(5);
			server_config_struct.create_type_times = vector<vector<double>>(5);
			server_config_struct.get_attr_list_times = vector<vector<double>>(5);
			server_config_struct.get_attr_times = vector<vector<double>>(5);
			server_config_struct.insert_attr_times = vector<vector<double>>(5);
			server_config_struct.processing_type_times = vector<vector<double>>(5);


			debug_log << "num_server_procs: " << server_config_struct.config.num_server_procs << endl;
			debug_log << "num_server_nodes: " << server_config_struct.config.num_server_nodes << endl;
			debug_log << "num_client_procs: " << server_config_struct.config.num_client_procs << endl;
			debug_log << "num_client_nodes: " << server_config_struct.config.num_client_nodes << endl;
			debug_log << "num_datasets: " << server_config_struct.config.num_datasets << endl;
			debug_log << "num_types: " << server_config_struct.config.num_types << endl;
			debug_log << "server_config_struct.all_clock_times.size(): " << server_config_struct.all_clock_times.size() << endl;
			debug_log << "server_config_struct.all_storage_sizes.size(): " << server_config_struct.all_storage_sizes.size() << endl;
			debug_log << "server_config_struct.clock_times_eval.size(): " << server_config_struct.clock_times_eval.size() << endl;

	        for(int i = 0; i<server_config_struct.filenames.size(); i++) {
	        	string filename = server_config_struct.filenames.at(i);
	        	string file_path = (string)output_dir + "/" + filename;
	        	debug_log << "server file_path: " << file_path << endl;
	        	evaluate_all_servers(file_path, server_config_struct);

	        }
	        server_outputs[config_params] = server_config_struct;

		}

		for (std::map<string,dirman_config_output>::iterator it=dirman_outputs.begin(); it!=dirman_outputs.end(); ++it) {
	    	string config_params = it->first;
	        dirman_config_output dirman_config_struct = it->second;

			sscanf(config_params.c_str(), "my_dirman_%hu_%hu_%hu_%hu_%llu_%hu", &dirman_config_struct.config.num_server_procs, &dirman_config_struct.config.num_server_nodes,
				   &dirman_config_struct.config.num_client_procs, &dirman_config_struct.config.num_client_nodes, &dirman_config_struct.config.num_datasets, &dirman_config_struct.config.num_types);
			dirman_config_struct.config.num_clock_pts = NUM_CLOCK_PTS_DIRMAN;
			uint16_t num_read_procs = DEC_PERCENT_CLIENT_PROCS_USED_TO_READ * dirman_config_struct.config.num_client_procs;
			if(num_read_procs != (DEC_PERCENT_CLIENT_PROCS_USED_TO_READ * dirman_config_struct.config.num_client_procs)  ) {
				num_read_procs +=1; //note - this will be fixed in the testing code
			}
			dirman_config_struct.config.num_read_procs = num_read_procs;
			dirman_config_struct.all_clock_times = vector<vector<long double>>(NUM_CLOCK_PTS_DIRMAN);
			dirman_config_struct.clock_times_eval = vector<vector<long double>>(2);
			dirman_config_struct.all_time_pts = vector<vector<double>>(NUM_CLOCK_PTS_DIRMAN-1);


			debug_log << "num_server_procs: " << dirman_config_struct.config.num_server_procs << endl;
			debug_log << "num_server_nodes: " << dirman_config_struct.config.num_server_nodes << endl;
			debug_log << "num_client_procs: " << dirman_config_struct.config.num_client_procs << endl;
			debug_log << "num_client_nodes: " << dirman_config_struct.config.num_client_nodes << endl;
			debug_log << "num_datasets: " << dirman_config_struct.config.num_datasets << endl;
			debug_log << "num_types: " << dirman_config_struct.config.num_types << endl;
			debug_log << "dirman_config_struct.all_clock_times.size(): " << dirman_config_struct.all_clock_times.size() << endl;
			debug_log << "dirman_config_struct.clock_times_eval.size(): " << dirman_config_struct.clock_times_eval.size() << endl;
			debug_log << "dirman_config_struct.all_time_pts.size(): " << dirman_config_struct.all_time_pts.size() << endl;

	        for(int i = 0; i<dirman_config_struct.filenames.size(); i++) {
	        	string filename = dirman_config_struct.filenames.at(i);
	        	string file_path = (string)output_dir + "/" + filename;
	        	debug_log << "dirman file_path: " << file_path << endl;
	        	evaluate_dirman(file_path, dirman_config_struct);

	        }
	        dirman_outputs[config_params] = dirman_config_struct;

		} 	 	 
	  	

		bool client_first_output = true;
		for (std::map<string,client_config_output>::iterator it=client_outputs.begin(); it!=client_outputs.end(); ++it) {
	    	string config_params = it->first;
	      	client_config_output client_config_struct = it->second;
			write_output_client(results_path, client_first_output, client_config_struct);
			client_first_output = false;
			debug_log << "output.read_pattern_1_times.size() in write output: " << client_config_struct.read_pattern_1_times.size() << endl;

		}

		bool server_first_output = true;
		for (std::map<string,server_config_output>::iterator it=server_outputs.begin(); it!=server_outputs.end(); ++it) {
	    	string config_params = it->first;
	      	server_config_output server_config_struct = it->second;
			write_output_server(results_path, server_first_output, server_config_struct);
			server_first_output = false;

		}

		bool dirman_first_output = true;
		for (std::map<string,dirman_config_output>::iterator it=dirman_outputs.begin(); it!=dirman_outputs.end(); ++it) {
	    	string config_params = it->first;
	      	dirman_config_output dirman_config_struct = it->second;
			write_output_dirman(results_path, dirman_first_output, dirman_config_struct);
			dirman_first_output = false;

		}
		//note - need to add dirman back in 
	}//end for cluster


}



static int write_output_client(const string &results_path, bool first_run_clients, const struct client_config_output &output) {
	//note: these will in essence be key value tables with key: config params

	int rc;

	vector< vector<double>> all_read_patterns_times{output.read_pattern_1_times,output.read_pattern_2_times,output.read_pattern_3_times,
											   output.read_pattern_4_times,output.read_pattern_5_times,output.read_pattern_6_times,
											   output.read_pattern_1_type_times,output.read_pattern_2_type_times,output.read_pattern_3_type_times,
											   output.read_pattern_4_type_times,output.read_pattern_5_type_times,output.read_pattern_1_type_times};

	vector<vector< vector<double>>> all_total_op_times{output.activate_var_times, output.catalog_var_times,output.create_var_times,output.get_chunk_list_times,
									    output.get_chunk_times,output.insert_chunk_times,output.processing_var_times,output.activate_type_times,
										output.catalog_type_times,output.create_type_times,output.get_attr_list_times,output.get_attr_times,
										output.insert_attr_times,output.processing_type_times,output.shutdown_times};

	vector< vector<double>> all_section_times{output.init_times,output.writing_create_times,output.writing_insert_times,output.writing_times,
								  output.read_init_times,output.read_pattern_times,output.read_type_pattern_times,output.read_all_pattern_times,output.extra_testing_times,output.reading_times};

	// vector <string> op_names = {"activate var", "catalog var", "create var", "get chunk", "get chunk list", "insert chunk",
	// 							"processing var", "activate type", "catalog type", "create type"}
	debug_log << "about to start writing client output \n";

	int num_read_patterns = all_read_patterns_times.size();
	int num_ops = all_total_op_times.size();
	int num_sections = all_section_times.size();

	string pattern_results_path = results_path + "/client_patterns.csv";
	ofstream pattern_fs;
	string op_results_path = results_path + "/client_ops.csv";
	ofstream op_fs;
	string op_breakdown_path = results_path + "/client_op_breakdown.csv";
	ofstream op_breakdown_fs;
	string section_results_path = results_path + "/client_sections.csv";
	ofstream section_fs;

	// string compare_total_op_times_path = results_path + "/client_compare_total_op_times.csv";
	// ofstream compare_total_op_times_fs;
	// string clock_results_path = results_path + "/client_clocks.csv";
	// ofstream clock_fs;
	string clock_eval_results_path = results_path + "/client_clocks_eval.csv";
	ofstream clock_eval_fs;

	debug_log << "output.filenames.at(0): " << output.filenames.at(0) << endl;

	if(first_run_clients) { 
		debug_log << "first run \n";
		pattern_fs.open(pattern_results_path, std::fstream::out);
		op_fs.open(op_results_path, std::fstream::out);
		op_breakdown_fs.open(op_breakdown_path, std::fstream::out);
		section_fs.open(section_results_path, std::fstream::out);
		// clock_fs.open(clock_results_path, std::fstream::out);
		clock_eval_fs.open(clock_eval_results_path, std::fstream::out);

		// compare_total_op_times_fs.open(compare_total_op_times_path, std::fstream::out);

		pattern_fs << "is_type: " << endl;
		pattern_fs << "0:no 1:yes" << endl;
		pattern_fs << endl;
		pattern_fs << "pattern,is_type,rank,num_server_procs,num_server_nodes,num_client_procs,num_client_nodes,"
		           << "num_read_procs,num_datasets,num_types,num data pts,max,min,avg,variance,std deviation,median" << endl;

		op_fs << "op categories: " << endl;
		op_fs << "0:activate var, 1:catalog var, 2:create var, 3:get chunk list, 4:get chunk, 5:insert chunk, 6:processing var, "
			  << "7:activate type, 8:catalog type, 9:create type, 10:get attr list, 11:get attr, 12:insert attr, 13:processing type "
			  << "14:shutdown" << endl;
		op_fs << endl;
		op_fs << "op,num_server_procs,num_server_nodes,num_client_procs,num_client_nodes,num_read_procs,"
		      << "num_datasets,num_types,num data pts,max,min,avg,variance,std deviation,median" << endl;

		op_breakdown_fs << "op categories: " << endl;
		op_breakdown_fs << "0:activate var, 1:catalog var, 2:create var, 3:get chunk list, 4:get chunk, 5:insert chunk, 6:processing var, "
			  << "7:activate type, 8:catalog type, 9:create type, 10:get attr list, 11:get attr, 12:insert attr, 13:processing type "
			  << "14:shutdown" << endl;
		op_breakdown_fs << endl;
		op_breakdown_fs << "op,num_server_procs,num_server_nodes,num_client_procs,num_client_nodes,num_read_procs,"
		      << "num_datasets,num_types,num data pts,avg serialize time,avg send time,avg received return msg time,avg deserialize time,avg total time" << endl;

		section_fs << "section categories: " << endl;
		section_fs << "0:init time, 1:writing create time, 2:writing insert time, 3:writing time,4:read init time, 5:read pattern time, 6:read type pattern time, 7:total read pattern time, 8:extra testing time, 9:total reading time" << endl;
		section_fs << endl;
		section_fs << "section,num_server_procs,num_server_nodes,num_client_procs,num_client_nodes,"
		           << "num_read_procs,num_datasets,num_types,num data pts,max,min,avg,variance,std deviation,median" << endl;

			 	//start, mpi init done, register done, db_init done, dirman_init_done, finalize
		// clock_fs << "categories: " << endl;
		// clock_fs << "0:start, 1:mpi init done, 2:register ops done, 3:dirman init done, 4:server setup done"
		// 	     << "5:writing create done, 6:writing insert done, 7:read patterns done, 8:extra testing done" << endl;
		// clock_fs << endl;
		// clock_fs << "category,num_server_procs,num_server_nodes,num_client_procs,num_client_nodes,num_read_procs,"
		// 		 << "num_datasets,num_types,max,min,avg,variance,std deviation,median" << endl;

		clock_eval_fs << "categories: " << endl;
		clock_eval_fs << "0:full init time, 1:full write time, 2:full read pattern time, 3:full read time, "
					  << "4:full run time without extra, 5:full run time with extra" << endl;
		clock_eval_fs << endl;
		clock_eval_fs << "category,num_server_procs,num_server_nodes,num_client_procs,num_client_nodes,"
				 	  << "num_read_procs,num_datasets,num_types,num data pts,max,min,avg,variance,std deviation,median" << endl;

	}
	else {
		pattern_fs.open(pattern_results_path, std::fstream::app);
		op_fs.open(op_results_path, std::fstream::app);
		op_breakdown_fs.open(op_breakdown_path, std::fstream::app);
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
		error_log << "error. failed to open ops results file" << endl;
		rc = RC_ERR;
		goto cleanup;
	}
	else if(!op_breakdown_fs) {
		error_log << "error. failed to open ops breakdown file" << endl;
		rc = RC_ERR;
		goto cleanup;
	}
	else if(!section_fs) {
		error_log << "error. failed to open ops results file" << endl;
		rc = RC_ERR;
		goto cleanup;	
	}
	// else if(!compare_total_op_times_fs) {
	// 	error_log << "error. failed to open ops results file" << endl;
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
	char buffer[512];


	for(int i=0; i<num_read_patterns; i++) {
		vector<double> read_pattern_times = all_read_patterns_times.at(i);
		debug_log << "pattern size for pattern " << i << ": " << read_pattern_times.size() << endl;

		if(read_pattern_times.size() > 0) {
			double max = get_max(read_pattern_times);
			double min = get_min(read_pattern_times);
			double avg = get_mean(read_pattern_times);
			double variance = get_variance(read_pattern_times);
			double std_dev = get_std_dev(read_pattern_times);
			double median = get_median(read_pattern_times);
				// debug_log << "my rank: " << rank << "and time pt index: " << i << " and pt: " << output->read_pattern_1_times.size() << endl;
			int pattern = i%6+1;
			bool is_type = i/6;
			sprintf(buffer, "%d,%d,%d,%d,%d,%d,%d,%llu,%d,%lu,%lf,%lf,%lf,%lf,%lf,%lf\n", pattern,is_type,output.config.num_server_procs, output.config.num_server_nodes, 
				   output.config.num_client_procs, output.config.num_client_nodes, output.config.num_read_procs, output.config.num_datasets, output.config.num_types, 
				   read_pattern_times.size(),max, min, avg, variance, std_dev, median);

			pattern_fs << buffer;
		}

	}

	for(int op_index=0; op_index<num_ops; op_index++) {
		vector<vector<double>> op_pieces_times = all_total_op_times.at(op_index);
		vector<double> total_op_times = op_pieces_times.at(op_pieces_times.size()-1);
		debug_log << "size for op " << op_index << ": " << total_op_times.size() << endl;
		debug_log << "num pieces of ops " << op_index << ": " << op_pieces_times.size() << endl;

		if(total_op_times.size() > 0) {
			double max = get_max(total_op_times);
			double min = get_min(total_op_times);
			double avg = get_mean(total_op_times);
			double variance = get_variance(total_op_times);
			double std_dev = get_std_dev(total_op_times);
			double median = get_median(total_op_times);
			sprintf(buffer, "%d,%d,%d,%d,%d,%d,%llu,%d,%lu,%lf,%lf,%lf,%lf,%lf,%lf\n", op_index, output.config.num_server_procs, output.config.num_server_nodes, 
				   output.config.num_client_procs, output.config.num_client_nodes, output.config.num_read_procs, output.config.num_datasets, 
				   output.config.num_types, total_op_times.size(), max, min, avg, variance, std_dev, median);
			op_fs << buffer;

			if(op_pieces_times.size() == 5) {
				double avg_ser_time = get_mean(op_pieces_times.at(0));
				double avg_send_time = get_mean(op_pieces_times.at(1));
				double avg_rec_ret_time = get_mean(op_pieces_times.at(2));
				double avg_deser_time = get_mean(op_pieces_times.at(3));
				double avg_total_time = get_mean(op_pieces_times.at(4));

				sprintf(buffer, "%d,%d,%d,%d,%d,%d,%llu,%d,%lu,%lf,%lf,%lf,%lf,%lf\n", op_index, output.config.num_server_procs, output.config.num_server_nodes, 
					   output.config.num_client_procs, output.config.num_client_nodes, output.config.num_read_procs, output.config.num_datasets, 
					   output.config.num_types, total_op_times.size(), avg_ser_time, avg_send_time, avg_rec_ret_time, avg_deser_time, avg_total_time);
				op_breakdown_fs << buffer;
			}
			else {
				error_log << "error. op " << op_index << " had " << op_pieces_times.size() << " time categories \n";
			}
		}
	}

	for(int section_index=0; section_index<num_sections; section_index++) {
		vector<double> section_times = all_section_times.at(section_index);
		debug_log << "size for section " << section_index << ": " << section_times.size() << endl;

		if(section_times.size() > 0) {	
			double max = get_max(section_times);
			double min = get_min(section_times);
			double avg = get_mean(section_times);
			double variance = get_variance(section_times);
			double std_dev = get_std_dev(section_times);
			double median = get_median(section_times);

			sprintf(buffer, "%d,%d,%d,%d,%d,%d,%llu,%d,%lu,%lf,%lf,%lf,%lf,%lf,%lf\n", section_index, output.config.num_server_procs, output.config.num_server_nodes, 
				output.config.num_client_procs, output.config.num_client_nodes, output.config.num_read_procs, output.config.num_datasets, 
				output.config.num_types, section_times.size(), max, min, avg, variance, std_dev, median);
			// sprintf(buffer, "%d,%d,%d,%d,%d,%d,%llu,%d,%lu,%lf,%lf,%lf,%lf,%lf,%lf\n", section_index, output.config.num_server_procs, output.config.num_server_nodes, 
			// 	output.config.num_client_procs, output.config.num_client_nodes, output.config.num_read_procs, output.config.num_datasets, 
			// 	output.config.num_types, section_times.size(), max, min, avg, variance, std_dev, median);
			section_fs << buffer;
		}
	}

	// for(int i=0; i<output.all_clock_times.size(); i++) {
	// 	vector<long double> clock_times = output.all_clock_times.at(i);
	// 	debug_log << "clock times for category " << i << ": " << clock_times.size() << endl;
	// 	if(clock_times.size() > 0) {
	// 		long double max = get_long_max(clock_times);
	// 		long double min = get_long_min(clock_times);
	// 		long double avg = get_long_mean(clock_times);
	// 		long double variance = get_long_variance(clock_times);
	// 		long double std_dev = get_long_std_dev(clock_times);
	// 		long double median = get_long_median(clock_times);
	// 		sprintf(buffer, "%d,%d,%d,%d,%d,%d,%llu,%d,%Lf,%Lf,%Lf,%Lf,%Lf,%Lf\n", i, output.config.num_server_procs, output.config.num_server_nodes, 
	// 				output.config.num_client_procs, output.config.num_client_nodes, output.config.num_read_procs, output.config.num_datasets, output.config.num_types,  max, min, avg, variance, std_dev, median);	
	// 		clock_fs << buffer;
	// 	}
	// }

	//struct timeval start, mpi_init_done, register_ops_done, db_init done, dirman_init_done /
	for(int i=0; i<output.clock_times_eval.size(); i++) {
		vector<long double> clock_times_eval = output.clock_times_eval.at(i);
		debug_log << "clock times eval for category " << i << ": " << clock_times_eval.size() << endl;
		if(clock_times_eval.size() > 0) {
			long double max = get_long_max(clock_times_eval);
			long double min = get_long_min(clock_times_eval);
			long double avg = get_long_mean(clock_times_eval);
			long double variance = get_long_variance(clock_times_eval);
			long double std_dev = get_long_std_dev(clock_times_eval);
			long double median = get_long_median(clock_times_eval);
			sprintf(buffer, "%d,%d,%d,%d,%d,%d,%llu,%d,%lu,%Lf,%Lf,%Lf,%Lf,%Lf,%Lf\n", i, output.config.num_server_procs, output.config.num_server_nodes, 
					output.config.num_client_procs, output.config.num_client_nodes, output.config.num_read_procs, output.config.num_datasets, 
					output.config.num_types, clock_times_eval.size(), max, min, avg, variance, std_dev, median);	
			clock_eval_fs << buffer;
		}
	}

	//write clock times
	//struct timeval start, mpi_init_done, register_ops_done, db_init done, dirman_init_done //fix

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
	return rc;
}


static int write_output_server(const string &results_path, bool first_run_servers, const struct server_config_output &output) {
	//note: these will in essence be key value tables with key: config params

	debug_log << "got here \n";
	int rc;

	vector<vector< vector<double>>> 	all_total_op_times{output.activate_var_times, output.catalog_var_times,output.create_var_times,output.get_chunk_list_times,
									    output.get_chunk_times,output.insert_chunk_times,output.processing_var_times,output.activate_type_times,
										output.catalog_type_times,output.create_type_times,output.get_attr_list_times,output.get_attr_times,
										output.insert_attr_times,output.processing_type_times};

	// vector <string> op_names = {"activate var", "catalog var", "create var", "get chunk", "get chunk list", "insert chunk",
	// 							"processing var", "activate type", "catalog type", "create type"}
	debug_log << "about to start writing server output \n";

	int num_ops = all_total_op_times.size();
	debug_log << "num ops: " << num_ops << endl;

	vector <vector<double>> all_section_times{output.init_times,output.shutdown_times};

	int num_sections = all_section_times.size();

	string op_results_path = results_path + "/server_ops.csv";
	ofstream op_fs;
	string op_breakdown_path = results_path + "/server_op_breakdown.csv";
	ofstream op_breakdown_fs;
	string section_results_path = results_path + "/server_sections.csv";
	ofstream section_fs;
	string storage_results_path = results_path + "/server_storage.csv";
	ofstream storage_fs;
	string clock_eval_results_path = results_path + "/server_clocks_eval.csv";
	ofstream clock_eval_fs;

	if(first_run_servers) {
		op_fs.open(op_results_path, std::fstream::out);
		op_breakdown_fs.open(op_breakdown_path, std::fstream::out);
		section_fs.open(section_results_path, std::fstream::out);
		storage_fs.open(storage_results_path, std::fstream::out);	
		clock_eval_fs.open(clock_eval_results_path, std::fstream::out);	

		op_fs << "op categories: " << endl;
		op_fs << "0:activate var, 1:catalog var, 2:create var, 3:get chunk list, 4:get chunk, 5:insert chunk, 6:processing var, "
			  << "7:activate type, 8:catalog type, 9:create type, 10:get attr list, 11:get attr, 12:insert attr, 13:processing type "
			  << endl;
		op_fs << endl;
		op_fs << "op,num_server_procs,num_server_nodes,num_client_procs,num_client_nodes,num_read_procs,"
		      << "num_datasets,num_types,num data pts,max,min,avg,variance,std deviation,median" << endl;

		// compare_total_op_times_fs.open(compare_total_op_times_path, std::fstream::out);
		op_breakdown_fs << "op categories: " << endl;
		op_breakdown_fs << "0:activate var, 1:catalog var, 2:create var, 3:get chunk list, 4:get chunk, 5:insert chunk, 6:processing var, "
			  << "7:activate type, 8:catalog type, 9:create type, 10:get attr list, 11:get attr, 12:insert attr, 13:processing type "
			  << "14:shutdown" << endl;
		op_breakdown_fs << endl;
		op_breakdown_fs << "op,num_server_procs,num_server_nodes,num_client_procs,num_client_nodes,num_read_procs,"
		      << "num_datasets,num_types,num data pts,avg deserialize time,avg db time,avg serialize time,avg send msg,avg total time" << endl;

		section_fs << "section categories: " << endl;
		section_fs << "0:init time, 1:shutdown time" << endl;
		section_fs << endl;
		section_fs << "section,num_server_procs,num_server_nodes,num_client_procs,num_client_nodes,"
		           << "num_read_procs,num data pts,max,min,avg,variance,std deviation,median" << endl;

		storage_fs << "categories: " << endl;
		storage_fs << "0:init size, 1:final size, 2:number of vars, 3:number of chunks, 4:number of types, 5:number of attribtues, "
				   << "6:estm size needed, 7:estm size needed with seralization" << endl;
		storage_fs << endl;
		storage_fs << "category,num_server_procs,num_server_nodes,num_client_procs,num_client_nodes,"
				   << "num_read_procs,num data pts,max,min,avg,variance,std deviation,median,avg total per iteration" << endl;

		clock_eval_fs << "categories: " << endl;
		clock_eval_fs << "0:full init time, 1:full run time" << endl;
		clock_eval_fs << endl;
		clock_eval_fs << "category,num_server_procs,num_server_nodes,num_client_procs,num_client_nodes,"
				 	  << "num_read_procs,num_datasets,num_types,num data pts,max,min,avg,variance,std deviation,median" << endl;
	}
	else {
		op_fs.open(op_results_path, std::fstream::app);
		op_breakdown_fs.open(op_breakdown_path, std::fstream::app);
		section_fs.open(section_results_path, std::fstream::app);
		storage_fs.open(storage_results_path, std::fstream::app);
		clock_eval_fs.open(clock_eval_results_path, std::fstream::app);	
	}
	//start, mpi init done, register done, db_init done, dirman_init_done, finalize

	

	if(!op_fs) {
		error_log << "error. failed to open ops results file" << endl;
		rc = RC_ERR;
		goto cleanup;
	}
	else if(!op_breakdown_fs) {
		error_log << "error. failed to open ops breakdown file" << endl;
		rc = RC_ERR;
		goto cleanup;
	}
	else if(!section_fs) {
		error_log << "error. failed to open ops results file" << endl;
		rc = RC_ERR;
		goto cleanup;
	}
	else if(!storage_fs) {
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


	for(int op_index=0; op_index<num_ops; op_index++) {
		vector<vector<double>> op_pieces_times = all_total_op_times.at(op_index);
		vector<double> total_op_times = op_pieces_times.at(op_pieces_times.size()-1);
		debug_log << "size for op " << op_index << ": " << total_op_times.size() << endl;
		debug_log << "num pieces of ops " << op_index << ": " << op_pieces_times.size() << endl;

		if(total_op_times.size() > 0) {
			double max = get_max(total_op_times);
			double min = get_min(total_op_times);
			double avg = get_mean(total_op_times);
			double variance = get_variance(total_op_times);
			double std_dev = get_std_dev(total_op_times);
			double median = get_median(total_op_times);
			sprintf(buffer, "%d,%d,%d,%d,%d,%d,%llu,%d,%lu,%lf,%lf,%lf,%lf,%lf,%lf\n", op_index, output.config.num_server_procs, output.config.num_server_nodes, 
				   output.config.num_client_procs, output.config.num_client_nodes, output.config.num_read_procs, output.config.num_datasets, 
				   output.config.num_types, total_op_times.size(), max, min, avg, variance, std_dev, median);
			op_fs << buffer;

			if(op_pieces_times.size() == 5) {
				double avg_ser_time = get_mean(op_pieces_times.at(0));
				double avg_send_time = get_mean(op_pieces_times.at(1));
				double avg_rec_ret_time = get_mean(op_pieces_times.at(2));
				double avg_deser_time = get_mean(op_pieces_times.at(3));
				double avg_total_time = get_mean(op_pieces_times.at(4));

				sprintf(buffer, "%d,%d,%d,%d,%d,%d,%llu,%d,%lu,%lf,%lf,%lf,%lf,%lf\n", op_index, output.config.num_server_procs, output.config.num_server_nodes, 
					   output.config.num_client_procs, output.config.num_client_nodes, output.config.num_read_procs, output.config.num_datasets, 
					   output.config.num_types, total_op_times.size(), avg_ser_time, avg_send_time, avg_rec_ret_time, avg_deser_time, avg_total_time);
				op_breakdown_fs << buffer;
			}
			else {
				error_log << "error. op " << op_index << " had " << op_pieces_times.size() << " time categories \n";
			}
		}
	}

	for(int section_index=0; section_index<num_sections; section_index++) {
		vector<double> section_times = all_section_times.at(section_index);
		if(section_times.size() > 0) {
			double max = get_max(section_times);
			double min = get_min(section_times);
			double avg = get_mean(section_times);
			double variance = get_variance(section_times);
			double std_dev = get_std_dev(section_times);
			double median = get_median(section_times);

			sprintf(buffer, "%d,%d,%d,%d,%d,%d,%llu,%d,%lu,%lf,%lf,%lf,%lf,%lf,%lf\n", section_index, output.config.num_server_procs, output.config.num_server_nodes, 
				output.config.num_client_procs, output.config.num_client_nodes, output.config.num_read_procs, output.config.num_datasets, 
				output.config.num_types, section_times.size(), max, min, avg, variance, std_dev, median);
			section_fs << buffer;
		}
	}

	for(int i=0; i<output.all_storage_sizes.size(); i++) {
		vector<uint64_t> storage_sizes = output.all_storage_sizes.at(i);
		debug_log << "storage sizes for category " << i << ": " << storage_sizes.size() << endl;
		if(storage_sizes.size() > 0) {
			uint64_t max = get_uint64_t_max(storage_sizes);
			uint64_t min = get_uint64_t_min(storage_sizes);
			uint64_t avg = get_uint64_t_mean(storage_sizes);
			uint64_t variance = get_uint64_t_variance(storage_sizes);
			uint64_t std_dev = get_uint64_t_std_dev(storage_sizes);
			uint64_t median = get_uint64_t_median(storage_sizes);
			if(i == 3 || i == 5) { //num chunks or num attr
				uint64_t total = output.config.num_server_procs * std::accumulate(storage_sizes.begin(), storage_sizes.end(), 0.0) / storage_sizes.size();
				sprintf(buffer, "%d,%d,%d,%d,%d,%d,%llu,%d,%lu,%llu,%llu,%llu,%llu,%llu,%llu,%llu\n", i, output.config.num_server_procs, output.config.num_server_nodes, 
						output.config.num_client_procs, output.config.num_client_nodes, output.config.num_read_procs, output.config.num_datasets, 
						output.config.num_types, storage_sizes.size(), max, min, avg, variance, std_dev, median, total);	
			}
			else {
				sprintf(buffer, "%d,%d,%d,%d,%d,%d,%llu,%d,%lu,%llu,%llu,%llu,%llu,%llu,%llu\n", i, output.config.num_server_procs, output.config.num_server_nodes, 
						output.config.num_client_procs, output.config.num_client_nodes, output.config.num_read_procs, output.config.num_datasets, 
						output.config.num_types, storage_sizes.size(), max, min, avg, variance, std_dev, median);	
			}
			storage_fs << buffer;
		}
	}

	//struct timeval start, mpi_init_done, register_ops_done, db_init done, dirman_init_done /
	for(int i=0; i<output.clock_times_eval.size(); i++) {
		vector<long double> clock_times_eval = output.clock_times_eval.at(i);
		debug_log << "clock times eval for category " << i << ": " << clock_times_eval.size() << endl;
		if(clock_times_eval.size() > 0) {
			long double max = get_long_max(clock_times_eval);
			long double min = get_long_min(clock_times_eval);
			long double avg = get_long_mean(clock_times_eval);
			long double variance = get_long_variance(clock_times_eval);
			long double std_dev = get_long_std_dev(clock_times_eval);
			long double median = get_long_median(clock_times_eval);
			sprintf(buffer, "%d,%d,%d,%d,%d,%d,%llu,%d,%lu,%Lf,%Lf,%Lf,%Lf,%Lf,%Lf\n", i, output.config.num_server_procs, output.config.num_server_nodes, 
					output.config.num_client_procs, output.config.num_client_nodes, output.config.num_read_procs, output.config.num_datasets, 
					output.config.num_types, clock_times_eval.size(), max, min, avg, variance, std_dev, median);	
			clock_eval_fs << buffer;
		}
	}

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
	if(storage_fs.is_open()) {
		storage_fs.close();
	}
	if(clock_eval_fs.is_open()) {
		clock_eval_fs.close();
	}
	return rc;
}


static int write_output_dirman(const string &results_path, bool first_run_dirman, const struct dirman_config_output &output) {
	//note: these will in essence be key value tables with key: rank with the addition of num clients, num servers, and num client and server nodes in addition

	// int num_time_pts = all_time_pts.size();
	int rc;

	debug_log << "about to start writing dirmans output \n";


	string time_pts_results_path = results_path + "/dirman_time_pts.csv";
	ofstream time_pts_fs;
	// string clock_results_path = results_path + "/dirman_clocks.csv";
	// ofstream clock_fs;
	string clock_eval_results_path = results_path + "/dirman_clocks_eval.csv";
	ofstream clock_eval_fs;

	if(first_run_dirman) {

		time_pts_fs.open(time_pts_results_path, std::fstream::out);
		time_pts_fs << "categories: " << endl;
		time_pts_fs << "1:mpi init done, 2:dirman setup done, 3:register ops done, 4:generate contact info done, 5:finalize" << endl;
		time_pts_fs << endl;
		time_pts_fs << "code,num_server_procs,num_server_nodes,num_client_procs,num_client_nodes,num_read_procs,"
		            << "num_datasets,num_types,num data pts,max,min,avg,variance,std deviation,median" << endl;

	 //    //struct timeval start, mpi_init_done, dirman_init_done, register_done, generate_done, finalize;
		// clock_fs.open(clock_results_path, std::fstream::out);	
		// clock_fs << "categories: " << endl;
		// clock_fs << "0:start, 1:mpi init done, 2:dirman init done, 3:register ops done, 4:generate contact info done, 5:finalize" << endl;
		// clock_fs << endl;
		// clock_fs << "category,num_server_procs,num_server_nodes,num_client_procs,num_client_nodes,num_read_procs,"
		// 		 << "num_datasets,num_types,max,min,avg,variance,std deviation,median" << endl;

	    //struct timeval start, mpi_init_done, dirman_init_done, register_done, generate_done, finalize;
		clock_eval_fs.open(clock_eval_results_path, std::fstream::out);	
		clock_eval_fs << "categories: " << endl;
		clock_eval_fs << "0:full init time, 1:full runtime" << endl;
		clock_eval_fs << endl;
		clock_eval_fs << "category,num_server_procs,num_server_nodes,num_client_procs,num_client_nodes,num_read_procs,"
				 	  << "num_datasets,num_types,num data pts,max,min,avg,variance,std deviation,median" << endl;
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
		if(time_pts.size() > 0) {
			double max = get_max(time_pts);
			double min = get_min(time_pts);
			double avg = get_mean(time_pts);
			double variance = get_variance(time_pts);
			double std_dev = get_std_dev(time_pts);
			double median = get_median(time_pts);
			sprintf(buffer, "%d,%d,%d,%d,%d,%d,%llu,%d,%lu,%lf,%lf,%lf,%lf,%lf,%lf\n", i, output.config.num_server_procs, output.config.num_server_nodes, 
				output.config.num_client_procs, output.config.num_client_nodes, output.config.num_read_procs, output.config.num_datasets, 
				output.config.num_types, time_pts.size(), max, min, avg, variance, std_dev, median);
			time_pts_fs << buffer;		
		}
	}


	// for(int i=0; i<output.all_clock_times.size(); i++) {
	// 	vector<long double> clock_times = output.all_clock_times.at(i);
	// 	debug_log << "clock times for category " << i << ": " << clock_times.size() << endl;
	// 	if(clock_times.size() > 0) {
	// 		long double max = get_long_max(clock_times);
	// 		long double min = get_long_min(clock_times);
	// 		long double avg = get_long_mean(clock_times);
	// 		long double variance = get_long_variance(clock_times);
	// 		long double std_dev = get_long_std_dev(clock_times);
	// 		long double median = get_long_median(clock_times);
	// 		sprintf(buffer, "%d,%d,%d,%d,%d,%d,%llu,%d,%Lf,%Lf,%Lf,%Lf,%Lf,%Lf\n", i, output.config.num_server_procs, output.config.num_server_nodes, 
	// 				output.config.num_client_procs, output.config.num_client_nodes, output.config.num_read_procs, output.config.num_datasets, output.config.num_types,  max, min, avg, variance, std_dev, median);	
	// 		clock_fs << buffer;
	// 	}
	// }


	//struct timeval start, mpi_init_done, register_ops_done, db_init done, dirman_init_done /
	for(int i=0; i<output.clock_times_eval.size(); i++) {
		vector<long double> clock_times_eval = output.clock_times_eval.at(i);
		debug_log << "clock times eval for category " << i << ": " << clock_times_eval.size() << endl;
		if(clock_times_eval.size() > 0) {
			long double max = get_long_max(clock_times_eval);
			long double min = get_long_min(clock_times_eval);
			long double avg = get_long_mean(clock_times_eval);
			long double variance = get_long_variance(clock_times_eval);
			long double std_dev = get_long_std_dev(clock_times_eval);
			long double median = get_long_median(clock_times_eval);
			sprintf(buffer, "%d,%d,%d,%d,%d,%d,%llu,%d,%lu,%Lf,%Lf,%Lf,%Lf,%Lf,%Lf\n", i, output.config.num_server_procs, output.config.num_server_nodes, 
					output.config.num_client_procs, output.config.num_client_nodes, output.config.num_read_procs, output.config.num_datasets, 
					output.config.num_types, clock_times_eval.size(), max, min, avg, variance, std_dev, median);	
			clock_eval_fs << buffer;
		}
	}



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


