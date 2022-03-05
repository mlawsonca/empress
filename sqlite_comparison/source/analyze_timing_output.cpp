

#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <numeric>
#include <algorithm>

#include <sqlite_objector_comparison.hh>

using namespace std;

static bool debug_logging = false;
static debugLog debug_log = debugLog(debug_logging);

int output_analysis(uint64_t num_clients, uint64_t num_timesteps, uint64_t num_iterations,
	 bool first_output, const vector<int> &db_sizes, const vector<long double> &all_write_objector_times,
	 const vector<vector<long double>> &write_objector_times, const vector<long double> &all_sql_insert_object_times,
	 const vector<vector<long double>> &sql_insert_object_time, const vector<vector<long double>> &all_read_objector_times,
	 const vector<vector<vector<long double>>> &read_objector_times, const vector<vector<long double>> &all_sql_query_object_times, 
	 const vector<vector<vector<long double>>> &sql_query_object_times
	 );

int analyze_and_output_analysis(uint64_t num_clients, uint64_t num_timesteps, int num_iterations,
					bool first_output);

template <class T>
T get_sum(vector<T> v) {
    T sum = std::accumulate(v.begin(), v.end(), 0.0);
    return sum;
}

template <class T>
T get_mean(vector<T> v) {
 
    return (get_sum(v) / v.size());
}

template <class T>
T get_max(vector<T> v)  {
   return *max_element(v.begin(),v.end());
}

template <class T>
T get_min(vector<T> v)  {
   return *min_element(v.begin(),v.end());
}


int main() {

	int num_iterations = 10;
	// int num_iterations = 1;
	uint64_t num_timesteps = 3;


	vector<uint64_t> all_num_clients = {1000, 2000, 4000, 8000, 16000};
	// vector<uint64_t> all_num_clients = {1000};

	bool first_output = true;

	for(uint64_t num_clients : all_num_clients ) {
		analyze_and_output_analysis(num_clients, num_timesteps, num_iterations, first_output);
		if(num_clients == 1000) {
			first_output = false;
		}
	}
}

int analyze_and_output_analysis(uint64_t num_clients, uint64_t num_timesteps, int num_iterations,
					bool first_output) 
{

	ifstream file;

	uint16_t code;
	long double time_pt;

	long double start_time;
	long double init_time;

	vector<int> db_sizes;


	vector<long double> all_write_objector_times;
	vector<vector<long double>> write_objector_times(num_iterations);
	vector<long double> all_sql_insert_object_times;
	vector<vector<long double>> sql_insert_object_times(num_iterations);

	vector<vector<long double>> all_read_objector_times(6);
	vector<vector<vector<long double>>> read_objector_times;

	vector<vector<long double>> all_sql_query_object_times(6);
	vector<vector<vector<long double>>> sql_query_object_times;

	for(int i = 0; i < num_iterations; i++) {
		read_objector_times.push_back(vector<vector<long double>>(6));
		sql_query_object_times.push_back(vector<vector<long double>>(6));		
	}
	debug_log << "read_objector_times.size(): " << read_objector_times.size() << endl;
	debug_log << "read_objector_times[0].size(): " << read_objector_times[0].size() << endl;


	for(int iteration = 0; iteration < num_iterations; iteration++) {
		debug_log << "iteration: " << iteration << endl;

		file.open("../output_new/sqlite_objector_comparison_" + to_string(num_clients) +
				"_" + to_string(num_timesteps) + "_" + to_string(iteration)+".log");
		if(!file) {
			cout << "error. failed to open file " << "../output" + to_string(iteration)+".txt" << endl;
			return RC_ERR;
		}
	    std::string line; 			
	    while (std::getline(file, line)) 			
	    { 			
	        if( line.find("begin timing output") != std::string::npos) { 			
	            break; 			
	        } 	
	    }


		bool writing = false;
		bool reading = false;

		while (file >> code) { 
			if(code == DB_SIZES) {
				for(int i = 0; i < 3; i++) {
					int size;
					file >> size;
					debug_log << "size: " << size << endl;
					if(iteration == 0) {
						db_sizes.push_back(size);
					}
				}
				file >> code;
			}
			if (code == PROGRAM_START) {
				file >> time_pt;
				start_time = time_pt;

				while (file >> code && code != LUA_INIT_DONE ) {
					file >> time_pt;
				}
				if(code == LUA_INIT_DONE) {
					writing = true;
					if(time_pt < start_time) {
						cout << "time_pt: " << time_pt << " start_time: " << start_time <<
							" so am pushing back " << time_pt + 3600 - start_time << endl;
						init_time = time_pt + 3600 - start_time;
					}
					else {
						init_time = time_pt - start_time;
					}
				}
			}
			else {
				cout << "code wasnt db_sizes or start. instead was: " << code << endl;
				file >> time_pt;
			}
			while(file >> code) {
				file >> time_pt;
				if(code == BOUNDING_BOX_TO_OBJ_NAMES_START) {
					long double bb_start_time = time_pt;
					file >> code;
					file >> time_pt;
					if(code == BOUNDING_BOX_TO_OBJ_NAMES_DONE) {
						// debug_log << "BOUNDING_BOX_TO_OBJ_NAMES_DONE write" << endl;
						if(time_pt < bb_start_time) {
							cout << "time_pt: " << time_pt << " bb_start_time: " << bb_start_time <<
								" so am pushing back " << time_pt + 3600 - bb_start_time << endl;
							write_objector_times.at(iteration).push_back(time_pt + 3600 - bb_start_time);
							all_write_objector_times.push_back(time_pt + 3600 - bb_start_time);
						}
						else {
							write_objector_times.at(iteration).push_back(time_pt - bb_start_time);
							all_write_objector_times.push_back(time_pt - bb_start_time);
						}

						// debug_log << "BOUNDING_BOX_TO_OBJ_NAMES actually done" << endl;


					}
					else {
						cout << "error. BOUNDING_BOX_TO_OBJ_NAMES_DONE did not appear after start. instead " << code << " appeared \n";
						return RC_ERR;
					}
				}
				else if(code == READING_START) {
					writing = false;
					reading = true;
					break;
					//todo ?
				}
				else if(code == INSERT_OBJECT_START) {
					long double sql_start_time = time_pt;
					file >> code;
					file >> time_pt;
					if(code == INSERT_OBJECT_DONE) {
						// debug_log << "INSERT_OBJECT_DONE" << endl;
						if(time_pt < sql_start_time) {
							cout << "time_pt: " << time_pt << " sql_start_time: " << sql_start_time <<
								" so am pushing back " << time_pt + 3600 - sql_start_time << endl;
							sql_insert_object_times.at(iteration).push_back(time_pt + 3600 - sql_start_time);
							all_sql_insert_object_times.push_back(time_pt + 3600 - sql_start_time);
						}
						else {
							sql_insert_object_times.at(iteration).push_back(time_pt - sql_start_time);
							all_sql_insert_object_times.push_back(time_pt - sql_start_time);
						}
					}
					else {
						cout << "error. INSERT_OBJECT_DONE did not appear after start. instead " << code << " appeared \n";
					}
				}		
			}
			while(file >> code) {
				file >> time_pt;
				// if (code % 100 != 0 && code != READING_PATTERNS_DONE) {
				// 	cout << "error. was expecting a read pattern start code but instead saw " << code << endl;
				// 	return RC_ERR;
				// }
				while (code % 100 != 0 && file >> code) {
					file >> time_pt;
				}

				uint16_t start_code = code;
				long double pattern_start_time = time_pt;
				int pattern_indx = (code / 100) - 1;
				uint16_t end_code = code + 1; 
				while (file >> code) {
					file >> time_pt;
					if(code == BOUNDING_BOX_TO_OBJ_NAMES_START) {
						long double bb_start_time = time_pt;
						file >> code;
						file >> time_pt;
						if(code == BOUNDING_BOX_TO_OBJ_NAMES_DONE) {
							// debug_log << "BOUNDING_BOX_TO_OBJ_NAMES_DONE read" << endl;

							if(time_pt < bb_start_time) {
								cout << "time_pt: " << time_pt << " bb_start_time: " << bb_start_time <<
									" so am pushing back " << time_pt + 3600 - bb_start_time << endl;
								read_objector_times.at(iteration).at(pattern_indx).push_back(time_pt + 3600 - bb_start_time);
								all_read_objector_times.at(pattern_indx).push_back(time_pt + 3600 - bb_start_time);
							}
							else {
								read_objector_times.at(iteration).at(pattern_indx).push_back(time_pt - bb_start_time);
								all_read_objector_times.at(pattern_indx).push_back(time_pt - bb_start_time);
							}
						}
						else {
							cout << "error. BOUNDING_BOX_TO_OBJ_NAMES_DONE did not appear after start. instead " << code << " appeared \n";
							return RC_ERR;
						}
					}
					if(code == CATALOG_ALL_OBJECTS_WITH_VAR_DIMS_START) {
						long double sql_start_time = time_pt;
						file >> code;
						file >> time_pt;
						if(code == CATALOG_ALL_OBJECTS_WITH_VAR_DIMS_DONE) {

							if(time_pt < sql_start_time) {
								cout << "time_pt: " << time_pt << " sql_start_time: " << sql_start_time <<
									" so am pushing back " << time_pt + 3600 - sql_start_time << endl;
								sql_query_object_times.at(iteration).at(pattern_indx).push_back(time_pt + 3600 - sql_start_time);
								all_sql_query_object_times.at(pattern_indx).push_back(time_pt + 3600 - sql_start_time);
							}
							else {
								sql_query_object_times.at(iteration).at(pattern_indx).push_back(time_pt - sql_start_time);
								all_sql_query_object_times.at(pattern_indx).push_back(time_pt - sql_start_time);
							}
							// debug_log << "CATALOG_ALL_OBJECTS_WITH_VAR_DIMS_DONE" << endl;

						}
						else {
							cout << "error. CATALOG_ALL_OBJECTS_WITH_VAR_DIMS_DONE did not appear after start. instead " << code << " appeared \n";
							return RC_ERR;
						}
					}
					if (code == end_code) {
						break;
					}
				}

			}
		}
		file.close();
	}

	output_analysis(num_clients, num_timesteps, num_iterations, first_output, db_sizes, 
		all_write_objector_times, write_objector_times, all_sql_insert_object_times, sql_insert_object_times,
		all_read_objector_times, read_objector_times, all_sql_query_object_times, sql_query_object_times);
}

int output_analysis(uint64_t num_clients, uint64_t num_timesteps, uint64_t num_iterations,
	 bool first_output, const vector<int> &db_sizes, const vector<long double> &all_write_objector_times,
	 const vector<vector<long double>> &write_objector_times, const vector<long double> &all_sql_insert_object_times,
	 const vector<vector<long double>> &sql_insert_object_times, const vector<vector<long double>> &all_read_objector_times,
	 const vector<vector<vector<long double>>> &read_objector_times, const vector<vector<long double>> &all_sql_query_object_times, 
	 const vector<vector<vector<long double>>> &sql_query_object_times
	 ) 
{


 // 	if(first_output) {
	// 	read_fs << "number of runs used to compile statistics: " << num_iterations << endl;
	// 	read_fs << "simulating global simulation for " << num_clients << " procs each " <<
	// 				  << "with chunk size: 125x160x250" endl;
	// }

	char buffer[512];

	ofstream storage_sizes_fs;
	if(first_output) {
		storage_sizes_fs.open("../analysis/storage_sizes.csv", std::fstream::out);
	}
	else {
		storage_sizes_fs.open("../analysis/storage_sizes.csv", std::fstream::app);
	}
	if(!storage_sizes_fs) {
		cout << "error. failed to open storage_sizes_fs " << endl;
		return RC_ERR;
	}

	if(first_output) {
		storage_sizes_fs << "num_procs,num_timesteps,num_iterations,sqlite initial table size,sqlite final table size,num objects in table" << endl;
	}

	sprintf(buffer, "%d,%d,%d,%d,%d,%d", 
			num_clients, num_timesteps, num_iterations, db_sizes.at(0), db_sizes.at(1), db_sizes.at(2));

	storage_sizes_fs << buffer << endl;
	storage_sizes_fs.close();


	ofstream write_fs;
	if(first_output) {
		write_fs.open("../analysis/write_times.csv", std::fstream::out);
	}
	else {
		write_fs.open("../analysis/write_times.csv", std::fstream::app);
	}
	if(!write_fs) {
		cout << "error. failed to open write_fs " << endl;
		return RC_ERR;
	}


	vector<long double> sum_objector_times;
	vector<long double> sum_sqlite_times;
	for(int iteration = 0; iteration < num_iterations; iteration++) {
		// debug_log << "get sum" << endl;
		sum_objector_times.push_back(get_sum(write_objector_times.at(iteration)));
		sum_sqlite_times.push_back(get_sum(sql_insert_object_times.at(iteration)));
	}

	if(first_output) {
		// write_fs << "#categories: 0:objector, 1:sql";
		// write_fs << endl;
		write_fs << "Run Type,num_procs,num_timesteps,num_iterations,num time points per run,"
						  << "avg write time,max write time,min write time,"
						  << "avg total write time per run" << endl;
	}

	sprintf(buffer, "%s,%d,%d,%d,%d,%Lf,%Lf,%Lf,%Lf", 
			"Objector", num_clients, num_timesteps, num_iterations, write_objector_times.at(0).size(),
			get_mean(all_write_objector_times), get_max(all_write_objector_times), 
			get_min(all_write_objector_times), get_mean(sum_objector_times) );

	write_fs << buffer << endl;

	sprintf(buffer, "%s,%d,%d,%d,%d,%Lf,%Lf,%Lf,%Lf", 
			"SQLite", num_clients, num_timesteps, num_iterations, sql_insert_object_times.at(0).size(),
			get_mean(all_sql_insert_object_times), get_max(all_sql_insert_object_times), 
			get_min(all_sql_insert_object_times),  get_mean(sum_sqlite_times) );

	write_fs << buffer << endl << endl;
	write_fs.close();



	ofstream read_fs;
	if(first_output) {
		read_fs.open("../analysis/read_times.csv", std::fstream::out);
	}
	else {
		read_fs.open("../analysis/read_times.csv", std::fstream::app);		
	}
	if(!read_fs) {
		cout << "error. failed to open analysis file " << endl;
		return RC_ERR;
	}

	if(first_output) {
		// read_fs << "#categories: 0:objector, 1:sql";
		// read_fs << endl;
		read_fs << "Run Type,pattern,num_procs,num_timesteps,num_iterations,num time points per run,"
						  << "avg read time,max read time,min read time,"
						  << "avg total read time per run" << endl;

	}

	for(int pattern_indx = 0; pattern_indx < 6; pattern_indx++) {
		vector<long double> sum_objector_times;
		vector<long double> sum_sqlite_times;
		for(int iteration = 0; iteration < num_iterations; iteration++) {
			// debug_log << "get sum2" << endl;

			sum_objector_times.push_back(get_sum(read_objector_times.at(iteration).at(pattern_indx)));
			sum_sqlite_times.push_back(get_sum(sql_query_object_times.at(iteration).at(pattern_indx)));
		}

		sprintf(buffer, "%s,%d,%d,%d,%d,%d,%Lf,%Lf,%Lf,%Lf", 
				"Objector", pattern_indx+1, num_clients, num_timesteps, num_iterations, 
				read_objector_times.at(0).at(pattern_indx).size(), 
				get_mean(all_read_objector_times.at(pattern_indx)),
				get_max(all_read_objector_times.at(pattern_indx)),
				get_min(all_read_objector_times.at(pattern_indx)),
				get_mean(sum_objector_times) );

		read_fs << buffer << endl;



		sprintf(buffer, "%s,%d,%d,%d,%d,%d,%Lf,%Lf,%Lf,%Lf", 
				"SQLite", pattern_indx+1, num_clients, num_timesteps, num_iterations, 
				sql_query_object_times.at(0).at(pattern_indx).size(), 
				get_mean(all_sql_query_object_times.at(pattern_indx)),
				get_max(all_sql_query_object_times.at(pattern_indx)),
				get_min(all_sql_query_object_times.at(pattern_indx)),
				get_mean(sum_sqlite_times) );

		read_fs << buffer << endl;
			// debug_log << "get mean done 2" << endl;

	}
	read_fs << endl;
	read_fs.close();


	return RC_OK;
}