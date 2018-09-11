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
#include "eval_testing_output_hdf5.hh"
#include "stats_functions.hh"
#include <stdio.h>
#include <assert.h>


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

float DEC_PERCENT_CLIENT_PROCS_USED_TO_READ = .1; 
// uint16_t NUM_SERVER_STORAGE_PTS = 9; 

// uint16_t num_client_ops = 69;
// uint16_t num_server_ops = 68;
uint16_t num_md_functs = 39;

int evaluate_all_write_clients(const string &file_path, struct client_config_output &output);
int evaluate_all_read_clients(const string &file_path, struct client_config_output &output);

static int write_output_write_client(const string &results_path, bool first_run_clients, const struct client_config_output &output);
static int write_output_read_client(const string &results_path, bool first_run_clients, const struct client_config_output &output);
static int write_op_numbering_guide(const string &results_path);

void output_stats_with_string(char *buffer, client_config_output output,
		 vector<long double> time_pts, string str);
void output_stats_with_int(char *buffer, client_config_output output,
		 vector<long double> time_pts, int val);
void output_stats_with_two_ints(char *buffer, client_config_output output,
		 vector<long double> time_pts, int val1, int val2);
void output_stats_with_three_ints(char *buffer, client_config_output output,
		 vector<long double> time_pts, int val1, int val2, int val3);

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
		clusters.push_back("skybridge");
		clusters.push_back("chama");
		// clusters.push_back("ghost");
	}
	else {
		// clusters.push_back("chama");
		clusters.push_back("ghost");
	}

	extreme_debug_log << "got here \n";

	for(int i=0; i<clusters.size(); i++) {
		std::map <string, client_config_output> write_client_outputs;
		std::map <string, client_config_output> read_client_outputs;
		extreme_debug_log << "clusters.size(): " << clusters.size() << endl;
		extreme_debug_log << "i: " << i << endl;
		string cluster_name = clusters.at(i);
		DIR *runtime_output;
		struct dirent *entry;
		char output_dir[124];

		string results_path;

		if (run_type == "remote") {
				// int n = sprintf(output_dir, "/ascldap/users/mlawso/sirius/runtime/runtime_skybridge/run_build");
			int n = sprintf(output_dir, "/ascldap/users/mlawso/sirius/runtime/runtime_%s/run_build", cluster_name.c_str()); 
			results_path = "/ascldap/users/mlawso/sirius/testing_source_hdf5/analysis_code/analysis_hdf5";
		}
		else if (run_type == "gscratch") {
			int n = sprintf(output_dir, "/gscratch/mlawso/runtime_%s/output_hdf5/correct_copy",cluster_name.c_str()); 
			results_path = (string)output_dir + "/analysis_hdf5";
		}
		// else { // run_type == "local" //fix
		// 	// int n = sprintf(output_dir, "/Users/mlawso/sirius/testing_source/analysis_code/fake_outputs_for_debugging_new"); 
		// 	int n = sprintf(output_dir, "/Users/mlawso/sirius/testing_source_hdf5/analysis_code/log_outputs_serrano_small"); 

		// }

		cout << "reading files from: " << output_dir << endl;
		cout << "results_path: " << results_path << endl;


		if( (runtime_output = opendir(output_dir)) ) {
			while(	(entry = readdir(runtime_output)) ) {
				string filename = (string)entry -> d_name;
				size_t found = filename.find_last_of("_");
				string filename_minus_iteration = filename.substr(0,found);
				// filename_minus_iteration = filename_minus_iteration.substr(0,filename_minus_iteration.find(".txt")-2);

				if( filename.find("testing_harness_write_hdf5") != std::string::npos ) {
					debug_log << "filename: " << filename << endl;
					debug_log << "filename minus iteration: " << filename_minus_iteration << endl;

					if (write_client_outputs.find(filename_minus_iteration) == write_client_outputs.end() ) {
						struct client_config_output new_output;
						new_output.filenames.push_back(filename);
						write_client_outputs[filename_minus_iteration] = new_output;
					}
					else {
						write_client_outputs.find(filename_minus_iteration)->second.filenames.push_back(filename);
					}
					debug_log << "count of file params: " << (write_client_outputs.find(filename_minus_iteration)->second).filenames.size() << endl;

				}
				else if( filename.find("testing_harness_read_hdf5") != std::string::npos ) {
					debug_log << "filename: " << filename << endl;
					debug_log << "filename minus iteration: " << filename_minus_iteration << endl;

					if (read_client_outputs.find(filename_minus_iteration) == read_client_outputs.end() ) {
						struct client_config_output new_output;
						new_output.filenames.push_back(filename);
						read_client_outputs[filename_minus_iteration] = new_output;
					}
					else {
						read_client_outputs.find(filename_minus_iteration)->second.filenames.push_back(filename);
					}
					debug_log << "count of file params: " << (read_client_outputs.find(filename_minus_iteration)->second).filenames.size() << endl;

				}				
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

	  	for (std::map<string,client_config_output>::iterator it=write_client_outputs.begin(); it!=write_client_outputs.end(); ++it) {
	    	string config_params = it->first;
	        client_config_output client_config_struct = it->second;

			sscanf(config_params.c_str(), "testing_harness_write_hdf5_%hu_%hu_%hu_%hu_%llu", 
				    &client_config_struct.config.num_write_client_procs, &client_config_struct.config.num_write_client_nodes, 
				    &client_config_struct.config.num_read_client_procs, &client_config_struct.config.num_read_client_nodes, 				    
				    &client_config_struct.config.num_datasets);

			// client_config_struct.op_times.reserve(num_md_functs);
			client_config_struct.op_times = vector<vector<long double>>(num_md_functs);

			//1000 -> 2400, 3000->8000
			// for (int i = 1000; i<=2400; i+=100) {
			// for (short i = 0; i< num_client_ops; i++) { //fix
			// 	client_config_struct.op_times.push_back(std::vector<std::vector<long double>>(5));
			// }

			debug_log << "num_write_client_procs: " << client_config_struct.config.num_write_client_procs << endl;
			debug_log << "num_write_client_nodes: " << client_config_struct.config.num_write_client_nodes << endl;
			debug_log << "num_read_client_procs: " << client_config_struct.config.num_read_client_procs << endl;
			debug_log << "num_read_client_nodes: " << client_config_struct.config.num_read_client_nodes << endl;
			debug_log << "num_datasets: " << client_config_struct.config.num_datasets << endl;
			// debug_log << "num_types: " << client_config_struct.config.num_types << endl;

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


	  	for (std::map<string,client_config_output>::iterator it=read_client_outputs.begin(); it!=read_client_outputs.end(); ++it) {
	    	string config_params = it->first;
	        client_config_output client_config_struct = it->second;

			sscanf(config_params.c_str(), "testing_harness_read_hdf5_%hu_%hu_%hu_%hu_%llu", 
				    &client_config_struct.config.num_write_client_procs, &client_config_struct.config.num_write_client_nodes, 
				    &client_config_struct.config.num_read_client_procs, &client_config_struct.config.num_read_client_nodes, 				    
				    &client_config_struct.config.num_datasets);			

			// client_config_struct.op_times.reserve(num_md_functs);
			client_config_struct.op_times = vector<vector<long double>>(num_md_functs);

			//1000 -> 2400, 3000->8000
			// for (int i = 1000; i<=2400; i+=100) {
			// for (int i = 0; i< num_client_ops; i++) { //fix
			// 	client_config_struct.op_times.push_back(std::vector<std::vector<long double>>(5));
			// 	client_config_struct.collective_op_times.push_back(std::vector<std::vector<long double>>(3));
			// }

			// debug_log << "client_config_struct.all_clock_times.size(): " << client_config_struct.all_clock_times.size() << endl;
			// debug_log << "client_config_struct.clock_times_eval.size(): " << client_config_struct.clock_times_eval.size() << endl;

			debug_log << "num_write_client_procs: " << client_config_struct.config.num_write_client_procs << endl;
			debug_log << "num_write_client_nodes: " << client_config_struct.config.num_write_client_nodes << endl;
			debug_log << "num_read_client_procs: " << client_config_struct.config.num_read_client_procs << endl;
			debug_log << "num_read_client_nodes: " << client_config_struct.config.num_read_client_nodes << endl;
			debug_log << "num_datasets: " << client_config_struct.config.num_datasets << endl;
			// debug_log << "num_types: " << client_config_struct.config.num_types << endl;

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
	  
		bool write_client_first_output = true;
		for (std::map<string,client_config_output>::iterator it=write_client_outputs.begin(); it!=write_client_outputs.end(); ++it) {
	    	string config_params = it->first;
	      	client_config_output client_config_struct = it->second;
	      	cout << "write client config_params: " << config_params << " (count: " << client_config_struct.filenames.size() << ")" << endl;
			rc = write_output_write_client(results_path, write_client_first_output, client_config_struct);
			if (rc != RC_OK) {
				error_log << "Error in write_output_write_client \n";
				return rc;
			}
			write_client_first_output = false;

		}

		bool read_client_first_output = true;
		for (std::map<string,client_config_output>::iterator it=read_client_outputs.begin(); it!=read_client_outputs.end(); ++it) {
	    	string config_params = it->first;
	      	client_config_output client_config_struct = it->second;
	     	cout << "read client config_params: " << config_params << " (count: " << client_config_struct.filenames.size() << ")" <<  endl;
			rc = write_output_read_client(results_path, read_client_first_output, client_config_struct);
			if (rc != RC_OK) {
				error_log << "Error in write_output_read_client \n";
				return rc;
			}
			read_client_first_output = false;

		}

		rc = write_op_numbering_guide(results_path); //fix
		if (rc != RC_OK) {
			error_log << "Error in write_op_numbering_guide \n";
			return rc;
		}
	}//end for cluster
}


template <class T>
uint32_t get_times_per_run( client_config_output output, const vector<T> &vct ){
	if (vct.size() == 0 || output.filenames.size() == 0) {
		extreme_debug_log << "vct.size(): " << vct.size() << endl;
		return 0;
	}
	return ( vct.size() / (output.filenames.size()) );
}

template <class T>
uint32_t get_times_per_proc( client_config_output output, const vector<T> &vct ){
	if (vct.size() == 0 || output.filenames.size() == 0) {
		extreme_debug_log << "vct.size(): " << vct.size() << endl;
		return 0;
	}
	if (vct.size() >= output.config.num_write_client_procs) {
		return (vct.size() / (output.filenames.size() * output.config.num_write_client_procs));
	}
	return 0;
}

static int write_output_write_client(const string &results_path, bool first_run_clients, const struct client_config_output &output) {
	//note: these will in essence be key value tables with key: config params

	int rc = RC_OK;						

	debug_log << "about to start writing client output \n";

	int num_ops = output.op_times.size();

	ofstream op_fs, op_breakdown_fs, section_fs, last_first_fs;

	string op_results_path = results_path + "/write_client_ops.csv"; 
	// string op_breakdown_path = results_path + "/write_client_op_breakdown.csv";
	// string section_results_path = results_path + "/write_client_sections.csv";
	string last_first_results_path = results_path + "/write_client_last_first_sections.csv";

	debug_log << "output.filenames.at(0): " << output.filenames.at(0) << endl;

	if(first_run_clients) { 
		debug_log << "first run \n";
		// debug_log << "am first run client about to open " << op_results_path << " for writing" << endl;

		op_fs.open(op_results_path, std::fstream::out); assert(op_fs);
		// op_breakdown_fs.open(op_breakdown_path, std::fstream::out); assert(op_breakdown_fs);
		// section_fs.open(section_results_path, std::fstream::out); assert(section_fs);
		last_first_fs.open(last_first_results_path, std::fstream::out); assert(last_first_fs);


		op_fs << "op,num_write_client_procs,num_write_client_nodes,"
			  << "num_read_client_procs,num_read_client_nodes,"
		      << "num_datasets,num data pts,pts per run,pts per proc,max,min,avg,variance,std deviation,median" << endl;

		// op_breakdown_fs << "op,num_write_client_procs,num_write_client_nodes,"
		// 	  << "num_read_client_procs,num_read_client_nodes,"
		//       << "num_datasets,num data pts,avg serialize time,avg send time,avg received return msg time,avg deserialize time,avg total time" << endl;


		// // section_fs << "section categories: " << endl;
		// //section_fs << "categories: " << endl;
		// //section_fs << "0: total time, 1:total op time" << endl << endl;


		// // section_fs << "0: total init time, 1:total writing basic metadata time, "
		// // 		   << "2: writing custom metadata times, 3:total writing time" << endl;
		// // section_fs << endl;
		// section_fs << "name,category,num_write_client_procs,num_write_client_nodes,"
		// 		   << "num_read_client_procs,num_read_client_nodes,"
		//            << "num_datasets,num data pts,max,min,avg,variance,std deviation,median" << endl;

		last_first_fs << "name,num_write_client_procs,num_write_client_nodes,"
			  		  << "num_read_client_procs,num_read_client_nodes,"
				 	  << "num_datasets,num data pts,pts per run,pts per proc,max,min,avg,variance,std deviation,median" << endl;

	}
	else {
		op_fs.open(op_results_path, std::fstream::app); assert(op_fs);
		// debug_log << "am NOT first run client about to open " << op_results_path << " for appending" << endl;
		// op_breakdown_fs.open(op_breakdown_path, std::fstream::app); assert(op_breakdown_fs);
		// section_fs.open(section_results_path, std::fstream::app); assert(section_fs);
		last_first_fs.open(last_first_results_path, std::fstream::app); assert(last_first_fs);
	}

	char buffer[512];

	for(int op_index = 0; op_index < num_ops; op_index++) {
		// vector<vector<long double>> op_pieces_times = output.op_times[op_index];
		// vector<long double> total_op_times = op_pieces_times.at(op_pieces_times.size()-1);
		vector<long double> total_op_times = output.op_times[op_index];
		debug_log << "write client - num time pts for op " << op_index << ": " << total_op_times.size() << endl;
		// debug_log << "write client - num categories for op breakdown " << op_index << ": " << op_pieces_times.size() << endl;

		if(total_op_times.size() > 0) {

			output_stats_with_int(buffer, output, total_op_times, op_index*100 + 1000);
			op_fs << buffer << endl;
			// if(op_pieces_times.size() == 5) {
			// 	long double avg_ser_time = get_mean(op_pieces_times.at(0));
			// 	long double avg_send_time = get_mean(op_pieces_times.at(1));
			// 	long double avg_rec_ret_time = get_mean(op_pieces_times.at(2));
			// 	long double avg_deser_time = get_mean(op_pieces_times.at(3));
			// 	long double avg_total_time = get_mean(op_pieces_times.at(4));

			// 	sprintf(buffer, "%d,%d,%d,%d,%d,%d,%d,%llu,%lu,%Lf,%Lf,%Lf,%Lf,%Lf", 
			// 		op_index*100 + 1000, output.config.num_server_procs, output.config.num_server_nodes, 
			// 		output.config.num_write_client_procs, output.config.num_write_client_nodes, 
			// 		output.config.num_read_client_procs, output.config.num_read_client_nodes, 
			// 		output.config.num_datasets, 
			// 	 	total_op_times.size(), 
			// 	 	avg_ser_time, avg_send_time, avg_rec_ret_time, avg_deser_time, avg_total_time);
			// 	op_breakdown_fs << buffer << endl;
			// }
			// else {
			// 	error_log << "error. op " << op_index << " had " << op_pieces_times.size() << " time categories \n";
			// }
		}
	}


	// extreme_debug_log << "output.per_proc_times.size(): " << output.per_proc_times.size() << endl;
	// for (int i = 0; i < output.per_proc_times.size(); i++) {
 //    	string pt_name = output.per_proc_times.at(i).name;
 //      	vector<long double> timing_points = output.per_proc_times.at(i).time_pts;
 //      	extreme_debug_log << "timing_points.size(): " << timing_points.size() << endl;
	// 	if(timing_points.size() > 0) {	
	// 		output_stats_with_string(buffer, output, timing_points, pt_name); 
	// 		section_fs << buffer << endl;
	// 	}

	// 	int indx = 0;
	// 	while(indx < output.per_proc_op_times.size()) {
	// 		if (output.per_proc_op_times.at(indx).name == pt_name ){
	// 			break;
	// 		}
	// 		indx++;
	// 	}

	// 	if (indx != output.per_proc_op_times.size()) {
	// 		vector<long double> op_times = output.per_proc_op_times.at(indx).time_pts;

	// 		if(op_times.size() > 0) {	
	// 			output_stats_with_string(buffer, output, op_times, pt_name+"_OP"); 
	// 			section_fs << buffer << endl;
	// 		}
 //      	}
	// }


	extreme_debug_log << "output.last_first_times.size(): " << output.last_first_times.size() << endl;
	for (int i = 0; i < output.last_first_times.size(); i++) {
    	string pt_name = output.last_first_times.at(i).name;
      	vector<long double> timing_points = output.last_first_times.at(i).time_pts;
      	extreme_debug_log << "timing_points.size(): " << timing_points.size() << endl;
		if(timing_points.size() > 0) {	
			output_stats_with_string(buffer, output, timing_points, pt_name); 
			last_first_fs << buffer << endl;
		}	

		int indx = 0;
		while(indx < output.per_proc_op_times.size()) {
			if (output.per_proc_op_times.at(indx).name == pt_name ){
				break;
			}
			indx++;
		}

		if (indx != output.per_proc_op_times.size()) {
			vector<long double> op_times = output.per_proc_op_times.at(indx).time_pts;
			extreme_debug_log << "pt: " << pt_name << " size: " << op_times.size() << endl;
			if(op_times.size() > 0) {	
				output_stats_with_string(buffer, output, op_times, pt_name+"_OP"); 
				last_first_fs << buffer << endl;
			}
      	}
	}


	//means there is a line between timing info for different configs
	op_fs << endl;
	// op_breakdown_fs << endl;
	// section_fs << endl;
	last_first_fs << endl;

cleanup:
	op_fs.close();
	// op_breakdown_fs.close();
	// section_fs.close();
	last_first_fs.close();

	return rc;
}

std::vector<string> op_names = {
	"CATALOG_ALL_RUN_ATTRIBUTES",
    "CATALOG_ALL_RUN_ATTRIBUTES_WITH_TYPE",
    "CATALOG_ALL_TIMESTEP_ATTRIBUTES",
    "CATALOG_ALL_TIMESTEP_ATTRIBUTES_WITH_TYPE",
    "CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_DIMS",
    "CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR",
    "CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_DIMS",
    "CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR",
    "CATALOG_ALL_TIMESTEPS_WITH_VAR",
    "CATALOG_ALL_TIMESTEPS_WITH_VAR_SUBSTR",
    "CATALOG_ALL_TYPES_WITH_VAR_ATTRIBUTES_IN_TIMESTEP",
    "CATALOG_ALL_TYPES_WITH_VAR_ATTRIBUTES_WITH_VAR_DIMS_IN_TIMESTEP",
    "CATALOG_ALL_TYPES_WITH_VAR_ATTRIBUTES_WITH_VAR_IN_TIMESTEP",
    "CATALOG_ALL_TYPES_WITH_VAR_ATTRIBUTES_WITH_VAR_SUBSTR_DIMS_IN_TIMESTEP",
    "CATALOG_ALL_TYPES_WITH_VAR_ATTRIBUTES_WITH_VAR_SUBSTR_IN_TIMESTEP",
    "CATALOG_ALL_VAR_ATTRIBUTES",
    "CATALOG_ALL_VAR_ATTRIBUTES_WITH_DIMS",
    "CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_DIMS",
    "CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE",
    "CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_DIMS",
    "CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR",
    "CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_DIMS",
    "CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR",
    "CATALOG_ALL_VAR_ATTRIBUTES_WITH_VAR_DIMS",
    "CATALOG_ALL_VAR_ATTRIBUTES_WITH_VAR",
    "CATALOG_ALL_VAR_ATTRIBUTES_WITH_VAR_SUBSTR_DIMS",
    "CATALOG_ALL_VAR_ATTRIBUTES_WITH_VAR_SUBSTR",
    "CATALOG_TIMESTEP",
    "CATALOG_VAR",
    "CREATE_CHUNKED_VAR",
    "CREATE_RUN",
    "CREATE_TIMESTEP",
    "CREATE_VAR",
    "INSERT_RUN_ATTRIBUTE_BATCH",
    "INSERT_RUN_ATTRIBUTE",
    "INSERT_TIMESTEP_ATTRIBUTE_BATCH",
    "INSERT_TIMESTEP_ATTRIBUTE",
    "INSERT_VAR_ATTRIBUTE_BATCH",
    "INSERT_VAR_ATTRIBUTE"
};


//fix
static int write_output_read_client(const string &results_path, bool first_run_clients, const struct client_config_output &output) {
	//note: these will in essence be key value tables with key: config params

	int rc = RC_OK;						

	debug_log << "about to start writing client output \n";

	int num_ops = output.op_times.size();

	ofstream op_fs, op_breakdown_fs, section_fs, last_first_fs;

	string op_results_path = results_path + "/read_client_ops.csv"; 
	// string op_breakdown_path = results_path + "/read_client_op_breakdown.csv";
	// string section_results_path = results_path + "/read_client_sections.csv";
	string last_first_results_path = results_path + "/read_client_last_first_sections.csv";

	debug_log << "output.filenames.at(0): " << output.filenames.at(0) << endl;

	if(first_run_clients) { 
		debug_log << "first run \n";
		// debug_log << "am first run client about to open " << op_results_path << " for writing" << endl;

		op_fs.open(op_results_path, std::fstream::out); assert(op_fs);
		// op_breakdown_fs.open(op_breakdown_path, std::fstream::out); assert(op_breakdown_fs);
		// section_fs.open(section_results_path, std::fstream::out); assert(section_fs);
		last_first_fs.open(last_first_results_path, std::fstream::out); assert(last_first_fs);


		op_fs << "op,num_write_client_procs,num_write_client_nodes,"
			  << "num_read_client_procs,num_read_client_nodes,"
		      << "num_datasets,num data pts,pts per run,pts per proc,max,min,avg,variance,std deviation,median" << endl;

		// op_breakdown_fs << "op,num_write_client_procs,num_write_client_nodes,"
		// 	  << "num_read_client_procs,num_read_client_nodes,"
		//       << "num_datasets,num data pts,avg serialize time,avg send time,avg received return msg time,avg deserialize time,avg total time" << endl;


		// // section_fs << "section categories: " << endl;
		// //section_fs << "categories: " << endl;
		// //section_fs << "0: total time, 1:total op time" << endl << endl;


		// // section_fs << "0: total init time, 1:total writing basic metadata time, "
		// // 		   << "2: writing custom metadata times, 3:total writing time" << endl;
		// // section_fs << endl;
		// section_fs << "name,category,num_write_client_procs,num_write_client_nodes,"
		// 		   << "num_read_client_procs,num_read_client_nodes,"
		//            << "num_datasets,num data pts,max,min,avg,variance,std deviation,median" << endl;

		last_first_fs << "name,num_write_client_procs,num_write_client_nodes,"
			  		  << "num_read_client_procs,num_read_client_nodes,"
				 	  << "num_datasets,num data pts,pts per run,pts per proc,max,min,avg,variance,std deviation,median" << endl;

	}
	else {
		op_fs.open(op_results_path, std::fstream::app); assert(op_fs);
		// debug_log << "am NOT first run client about to open " << op_results_path << " for appending" << endl;
		// op_breakdown_fs.open(op_breakdown_path, std::fstream::app); assert(op_breakdown_fs);
		// section_fs.open(section_results_path, std::fstream::app); assert(section_fs);
		last_first_fs.open(last_first_results_path, std::fstream::app); assert(last_first_fs);
	}

	char buffer[512];

	for(int op_index = 0; op_index < num_ops; op_index++) {
		// vector<vector<long double>> op_pieces_times = output.op_times[op_index];
		// vector<long double> total_op_times = op_pieces_times.at(op_pieces_times.size()-1);
		vector<long double> total_op_times = output.op_times[op_index];
		debug_log << "write client - num time pts for op " << op_index << ": " << total_op_times.size() << endl;
		// debug_log << "write client - num categories for op breakdown " << op_index << ": " << op_pieces_times.size() << endl;

		if(total_op_times.size() > 0) {

			output_stats_with_int(buffer, output, total_op_times, op_index*100 + 1000);
			op_fs << buffer << endl;
			// if(op_pieces_times.size() == 5) {
			// 	long double avg_ser_time = get_mean(op_pieces_times.at(0));
			// 	long double avg_send_time = get_mean(op_pieces_times.at(1));
			// 	long double avg_rec_ret_time = get_mean(op_pieces_times.at(2));
			// 	long double avg_deser_time = get_mean(op_pieces_times.at(3));
			// 	long double avg_total_time = get_mean(op_pieces_times.at(4));

			// 	sprintf(buffer, "%d,%d,%d,%d,%d,%d,%d,%llu,%lu,%Lf,%Lf,%Lf,%Lf,%Lf", 
			// 		op_index*100 + 1000, output.config.num_server_procs, output.config.num_server_nodes, 
			// 		output.config.num_write_client_procs, output.config.num_write_client_nodes, 
			// 		output.config.num_read_client_procs, output.config.num_read_client_nodes, 
			// 		output.config.num_datasets, 
			// 	 	total_op_times.size(), 
			// 	 	avg_ser_time, avg_send_time, avg_rec_ret_time, avg_deser_time, avg_total_time);
			// 	op_breakdown_fs << buffer << endl;
			// }
			// else {
			// 	error_log << "error. op " << op_index << " had " << op_pieces_times.size() << " time categories \n";
			// }
		}
	}


	// extreme_debug_log << "output.per_proc_times.size(): " << output.per_proc_times.size() << endl;
	// for (int i = 0; i < output.per_proc_times.size(); i++) {
 //    	string pt_name = output.per_proc_times.at(i).name;
 //      	vector<long double> timing_points = output.per_proc_times.at(i).time_pts;
 //      	extreme_debug_log << "timing_points.size(): " << timing_points.size() << endl;
	// 	if(timing_points.size() > 0) {	
	// 		output_stats_with_string(buffer, output, timing_points, pt_name); 
	// 		section_fs << buffer << endl;
	// 	}

	// 	int indx = 0;
	// 	while(indx < output.per_proc_op_times.size()) {
	// 		if (output.per_proc_op_times.at(indx).name == pt_name ){
	// 			break;
	// 		}
	// 		indx++;
	// 	}

	// 	if (indx != output.per_proc_op_times.size()) {
	// 		vector<long double> op_times = output.per_proc_op_times.at(indx).time_pts;

	// 		if(op_times.size() > 0) {	
	// 			output_stats_with_string(buffer, output, op_times, pt_name+"_OP"); 
	// 			section_fs << buffer << endl;
	// 		}
 //      	}
	// }


	extreme_debug_log << "output.last_first_times.size(): " << output.last_first_times.size() << endl;
	for (int i = 0; i < output.last_first_times.size(); i++) {
    	string pt_name = output.last_first_times.at(i).name;
      	vector<long double> timing_points = output.last_first_times.at(i).time_pts;
      	extreme_debug_log << "timing_points.size(): " << timing_points.size() << endl;
		if(timing_points.size() > 0) {
			int indx = pt_name.find("CATALOG");	
			if( indx != string::npos && indx == 0 ) {
				int op_index = 0;
				while(op_index < op_names.size()) {
					if(op_names.at(op_index) == pt_name) {
						output_stats_with_int(buffer, output, timing_points, op_index*100 + 1010);
						last_first_fs << buffer << endl;
						break;
					}
					op_index++;
				}
				if (op_index == op_names.size()) {
					cout << "error. could not find " << pt_name << " in op names list" << endl;
				}
			}
			else {
				output_stats_with_string(buffer, output, timing_points, pt_name); 
				last_first_fs << buffer << endl;
			}
		}	

		int indx = 0;
		while(indx < output.per_proc_op_times.size()) {
			if (output.per_proc_op_times.at(indx).name == pt_name ){
				break;
			}
			indx++;
		}

		if (indx != output.per_proc_op_times.size()) {
			vector<long double> op_times = output.per_proc_op_times.at(indx).time_pts;
			extreme_debug_log << "pt: " << pt_name << " size: " << op_times.size() << endl;
			if(op_times.size() > 0) {	
				output_stats_with_string(buffer, output, op_times, pt_name+"_OP"); 
				last_first_fs << buffer << endl;
			}
      	}
	}


	//means there is a line between timing info for different configs
	op_fs << endl;
	// op_breakdown_fs << endl;
	// section_fs << endl;
	last_first_fs << endl;

cleanup:
	op_fs.close();
	// op_breakdown_fs.close();
	// section_fs.close();
	last_first_fs.close();

	return rc;
}


static int write_op_numbering_guide(const string &results_path) {
	int rc = RC_OK;

	debug_log << "about to start writing op numbering guide \n";

	string file_path = results_path + "/op_numbering_guide.csv";
	ofstream file_path_fs;

	file_path_fs.open(file_path, std::fstream::out); assert (file_path_fs);
	file_path_fs << "op number, op name" << endl;

	char buffer[512];


	if(op_names.size() != num_md_functs) {
		error_log << "Error. Was expecting " << num_md_functs << " but received " << op_names.size() << " instead" << endl;
	}

	for(int i=0; i<op_names.size(); i++) {
		int op_num = i*100 + 1000;

		sprintf(buffer, "%d,%s", 
				op_num, op_names.at(i).c_str());
		file_path_fs << buffer << endl;		
	}


cleanup:
	file_path_fs.close();

	return rc;
}


void output_stats_with_string(char *buffer, client_config_output output,
		 vector<long double> time_pts, string str)
{
	long double max = get_max(time_pts);
	long double min = get_min(time_pts);
	long double avg = get_mean(time_pts);
	long double variance = get_variance(time_pts);
	long double std_dev = get_std_dev(time_pts);
	long double median = get_median(time_pts);

	// sprintf(buffer, "%-35s,%d,%d,%d,%d,%llu,%lu,%lu,%lu,%Lf,%Lf,%Lf,%Lf,%Lf,%Lf", 
	sprintf(buffer, "%35s,%d,%d,%d,%d,%llu,%lu,%lu,%lu,%Lf,%Lf,%Lf,%Lf,%Lf,%Lf", 
			str.c_str(), output.config.num_write_client_procs, output.config.num_write_client_nodes, 
			output.config.num_read_client_procs, output.config.num_read_client_nodes, 
			output.config.num_datasets, time_pts.size(), get_times_per_run(output, time_pts),
					get_times_per_proc(output, time_pts), max, min, avg, variance, std_dev, median);
}

void output_stats_with_int(char *buffer, client_config_output output,
		 vector<long double> time_pts, int val)
{
	long double max = get_max(time_pts);
	long double min = get_min(time_pts);
	long double avg = get_mean(time_pts);
	long double variance = get_variance(time_pts);
	long double std_dev = get_std_dev(time_pts);
	long double median = get_median(time_pts);

	sprintf(buffer, "%d,%d,%d,%d,%d,%llu,%lu,%lu,%lu,%Lf,%Lf,%Lf,%Lf,%Lf,%Lf", 
			val, output.config.num_write_client_procs, output.config.num_write_client_nodes, 
			output.config.num_read_client_procs, output.config.num_read_client_nodes, 
			output.config.num_datasets, time_pts.size(), get_times_per_run(output, time_pts),
					get_times_per_proc(output, time_pts), max, min, avg, variance, std_dev, median);
}

void output_stats_with_two_ints(char *buffer, client_config_output output,
		 vector<long double> time_pts, int val1, int val2)
{
	long double max = get_max(time_pts);
	long double min = get_min(time_pts);
	long double avg = get_mean(time_pts);
	long double variance = get_variance(time_pts);
	long double std_dev = get_std_dev(time_pts);
	long double median = get_median(time_pts);

	sprintf(buffer, "%d,%d,%d,%d,%d,%d,%llu,%lu,%lu,%lu,%Lf,%Lf,%Lf,%Lf,%Lf,%Lf", 
			val1, val2, output.config.num_write_client_procs, output.config.num_write_client_nodes, 
			output.config.num_read_client_procs, output.config.num_read_client_nodes, 
			output.config.num_datasets, time_pts.size(), get_times_per_run(output, time_pts),
					get_times_per_proc(output, time_pts), max, min, avg, variance, std_dev, median);
}

void output_stats_with_three_ints(char *buffer, client_config_output output,
		 vector<long double> time_pts, int val1, int val2, int val3)
{
	long double max = get_max(time_pts);
	long double min = get_min(time_pts);
	long double avg = get_mean(time_pts);
	long double variance = get_variance(time_pts);
	long double std_dev = get_std_dev(time_pts);
	long double median = get_median(time_pts);

	sprintf(buffer, "%d,%d,%d,%d,%d,%d,%d,%llu,%lu,%lu,%lu,%Lf,%Lf,%Lf,%Lf,%Lf,%Lf", 
			val1, val2, val3, output.config.num_write_client_procs, output.config.num_write_client_nodes, 
			output.config.num_read_client_procs, output.config.num_read_client_nodes, 
			output.config.num_datasets, time_pts.size(), get_times_per_run(output, time_pts),
					get_times_per_proc(output, time_pts), max, min, avg, variance, std_dev, median);
}