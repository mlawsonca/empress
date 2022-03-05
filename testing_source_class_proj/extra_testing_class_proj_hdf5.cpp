

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <my_metadata_client_hdf5.hh>

// #include <client_timing_constants_read_class_proj_hdf5.hh>
#include <client_timing_constants_read_class_proj.hh>

#include <sstream>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

#include <mpi.h>

//includes the print attr functs
#include <testing_harness_helper_functions_class_proj_hdf5.hh>

#include <unordered_set>
// #include <extra_testing_collective_helper_functions.hh>

using namespace std;

extern void add_timing_point(int catg);


// static bool extreme_debug_logging = false;
// static bool debug_logging = false;
// static bool testing_logging = false;
// static bool zero_rank_logging = false;
static bool error_logging  = true;

// static debugLog testing_log = debugLog(testing_logging, zero_rank_logging);
static debugLog error_log = debugLog(error_logging, false);
// static debugLog extreme_debug_log = debugLog(extreme_debug_logging, false);
// static debugLog zero_rank_log = debugLog(true);
// static debugLog debug_log = debugLog(debug_logging, zero_rank_logging);



void do_temporal_analysis ( int rank, uint32_t num_client_procs, 
					const string &run_name, const string &job_id,
					const string &rare_type_name, const string &var_name, 
					MPI_Comm read_comm
 					);


// void do_global_analysis ( int rank, uint32_t num_client_procs,
// 					uint64_t timestep_id, const string &timestep_file_name, const string &type_name, double above_max_val,
// 					MPI_Comm read_comm
//         			);

void do_global_analysis ( int rank, uint32_t num_client_procs,
					const vector<string> &timestep_file_names, const string &type_name, double above_max_val,
					MPI_Comm read_comm
        			);

void do_multivariate_analysis ( int rank, uint32_t num_client_procs,
								const string &timestep_file_name,
								const string &var_name_substr, const string &rare_type_name,
								MPI_Comm read_comm
		                        );


void extra_testing(const string &run_name, const string &job_id, int rank, uint32_t num_client_procs,
					uint32_t num_timesteps,
					const vector<string> &var_names, const vector<string> &type_names, 
					const vector<string> &run_timestep_type_names, MPI_Comm read_comm ) 
{
    //testing_log << "starting extra testing" << endl;

    double above_max_val = 9.9 * pow(10, 9);
    string var_substr1 = "press";

    uint64_t timestep0_id = 0;
 	string timestep0_file_name = run_name + "/" + job_id + "/" + to_string(timestep0_id);

 	vector<string> all_timestep_file_names;
 	all_timestep_file_names.reserve(num_timesteps);

    for (int timestep_id = 0; timestep_id < num_timesteps; timestep_id++) {
    	string FILENAME = run_name + "/" + job_id + "/" + to_string(timestep_id);
    	all_timestep_file_names.push_back(FILENAME);
    }

	do_temporal_analysis ( rank, num_client_procs, run_name, job_id,
					type_names.at(2), var_names.at(8), read_comm
 					);

	do_global_analysis ( rank, num_client_procs, all_timestep_file_names, run_timestep_type_names.at(0), above_max_val, read_comm);


	do_multivariate_analysis ( rank, num_client_procs, timestep0_file_name, var_substr1, type_names.at(6), read_comm); 
 
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//TIMESTEP FUNCTS //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void do_temporal_analysis ( int rank, uint32_t num_client_procs, 
					const string &run_name, const string &job_id,
					const string &rare_type_name, const string &var_name, 
					MPI_Comm read_comm
 					) 
{

    vector<string> timestep_file_names;

    MPI_Barrier(read_comm);
    add_timing_point(CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_START);
    // add_timing_point(CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_COLLECTIVE_START);
	if(rank == 0) {
		vector<string> all_timestep_file_names;
		hid_t run_file_id;

    	open_run_for_read (run_name, job_id, run_file_id);
        metadata_catalog_timestep (run_file_id, all_timestep_file_names);
     //    if(testing_logging) {
	    //     testing_log << "rank: " << rank << " timestep catalog: \n";
	    //     print_timestep_catalog (all_timestep_file_names);
	    // }
		close_run (run_file_id);

    	metadata_catalog_all_timesteps_with_var_attributes_with_type_var (all_timestep_file_names, rare_type_name, var_name, timestep_file_names);
  //       if(testing_logging) {	
		// 	testing_log << "rank: " << rank << " all timesteps associated with a var attribute type_name " <<
		// 		 rare_type_name << ", var_name " << var_name << endl;
		// 	print_timestep_catalog (timestep_file_names);	
		// }
	}
    add_timing_point(CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_DONE);
    
    MPI_Barrier(read_comm); //leave to make timing clearer
    add_timing_point(CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_COLLECTIVE_START);
	bcast_entries(timestep_file_names, rank, num_client_procs, read_comm);
    add_timing_point(CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_COLLECTIVE_DONE);

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//RUN ATTR FUNCTS //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//global analysis
void do_global_analysis ( int rank, uint32_t num_client_procs,
					const vector<string> &timestep_file_names, const string &type_name, 
					double above_max_val, MPI_Comm read_comm
        			) 
{

    vector<non_var_attribute_str> attr_entries;
    md_timestep_entry timestep;

    MPI_Barrier(read_comm);
	add_timing_point(CATALOG_ALL_TIMESTEP_ATTRIBUTES_WITH_TYPE_ABOVE_MAX_START);	
	// add_timing_point(CATALOG_ALL_TIMESTEP_ATTRIBUTES_WITH_TYPE_COLLECTIVE_START);	
	if(rank == 0) {
	    vector<non_var_attribute_str> all_attr_entries;

		// open_timestep_file_and_timestep_attr_table_for_read(timestep_file_name, 
		// 		timestep.file_id, timestep.timestep_attr_table_id);

  //   	metadata_catalog_all_timestep_attributes_with_type (timestep.timestep_attr_table_id, type_name, all_attr_entries);
  // 		//testing_log << "rank: " << rank << " all timestep attributes associated with timestep " << timestep_id << " and type_name " << type_name << endl;

    	metadata_catalog_all_timestep_attributes_with_type (timestep_file_names, type_name, all_attr_entries);
  		//testing_log << "rank: " << rank << " all timestep attributes associated with type_name " << type_name << endl;

  		for(int i = 0; i < all_attr_entries.size(); i++) {
  			double value = std::stod (all_attr_entries.at(i).data);
  			//extreme_debug_log << "value: " << value << " above_max_val: " << above_max_val << endl;
  			if(value > above_max_val) {
  				attr_entries.push_back(all_attr_entries.at(i));
  			}
  		}
  		// if(testing_logging) {
    // 		print_timestep_attribute_list (attr_entries);
    // 	}

    	// close_timestep_file_and_attr_table(timestep.file_id, timestep.timestep_attr_table_id);
	}
	add_timing_point(CATALOG_ALL_TIMESTEP_ATTRIBUTES_WITH_TYPE_ABOVE_MAX_DONE);	

    MPI_Barrier(read_comm); //leave to make timing clearer
	add_timing_point(CATALOG_ALL_TIMESTEP_ATTRIBUTES_WITH_TYPE_ABOVE_MAX_COLLECTIVE_START);	
	bcast_entries(attr_entries, rank, num_client_procs, read_comm);
	// add_timing_point(CATALOG_ALL_TIMESTEP_ATTRIBUTES_WITH_TYPE_COLLECTIVE_DONE);	
	add_timing_point(CATALOG_ALL_TIMESTEP_ATTRIBUTES_WITH_TYPE_ABOVE_MAX_COLLECTIVE_DONE);	

}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//SUBSTR FUNCTS ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void do_multivariate_analysis ( int rank, uint32_t num_client_procs,
								const string &timestep_file_name,
								const string &var_name_substr, const string &rare_type_name,
								MPI_Comm read_comm
		                        ) 
{

    vector<var_attribute_str> attr_entries;
    md_timestep_entry timestep;

    MPI_Barrier(read_comm);
	// add_timing_point(CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_COLLECTIVE_START);
	add_timing_point(CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_START);		
	if(rank == 0) {
 		open_timestep_file_and_var_attr_table_for_read(timestep_file_name, 
									timestep.file_id, timestep.var_attr_table_id);

    	metadata_catalog_all_var_attributes_with_type_var_substr (timestep.var_attr_table_id, rare_type_name, var_name_substr, attr_entries);
		// if(testing_logging) {
		// 	testing_log << "rank: " << rank << " all var attributes associated with timestep " << 
		// 		timestep_file_name.substr(timestep_file_name.find_last_of("/")+1)<< 
		// 		" type: " << rare_type_name << 
		// 		" and var_name_substr " << var_name_substr << endl;
		// 	print_var_attribute_list (attr_entries);
		// }
 		close_timestep_file_and_attr_table(timestep.file_id, timestep.var_attr_table_id);
	}
	add_timing_point(CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_DONE);	
	
	MPI_Barrier(read_comm); //leave to make timing clearer
	add_timing_point(CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_COLLECTIVE_START);		
	bcast_entries(attr_entries, rank, num_client_procs, read_comm);
	add_timing_point(CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_COLLECTIVE_DONE);	

	// add_timing_point(CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_COLLECTIVE_DONE);	
}



