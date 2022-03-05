#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <math.h>

// #include <my_metadata_client.h>
#include <extra_testing_collective_helper_functions_hdf5.hh>
#include <client_timing_constants_read_hdf5.hh>

using namespace std;

extern void add_timing_point(int catg);

// static bool extreme_debug_logging = false;
// static bool debug_logging = false;
// static bool testing_logging = false;
// static bool zero_rank_logging = true;
// debugLog debug_log = debugLog(debug_logging, zero_rank_logging);
// debugLog extreme_debug_log = debugLog(extreme_debug_logging, false);
// debugLog testing_log = debugLog(testing_logging, zero_rank_logging);

extern debugLog testing_log;
extern debugLog error_log;
extern debugLog extreme_debug_log;
extern debugLog debug_log;

md_dim_bounds get_dims_to_search(md_var_entry var, int dims_funct_count);

// void get_dims_to_search(const vector<md_dim_bounds> &var_dims, vector<md_dim_bounds> &dims_to_search, 
// 					int dims_funct_count);

// int extra_testing_collective(const md_server &int rank, uint32_t uint32_t num_client_procs, const md_catalog_run_entry &run, 
//                   const vector<md_dim_bounds> &chunk_dims) 
int extra_testing_collective(string run_name, string job_id, int rank, uint32_t num_client_procs, md_dim_bounds chunk_dims, 
					vector<string> var_names_with_vers, vector<string> type_names) 
{

    // vector<md_catalog_run_entry> run_entries;
    vector<string> timestep_file_names;
    vector<md_var_entry> var_entries;
    vector<non_var_attribute_str> run_attr_entries;
    vector<non_var_attribute_str> timestep_attr_entries;
    vector<var_attribute_str> var_attr_entries;

    testing_log << "starting extra testing" << endl;

    string timestep0_file_name;

    int dims_funct_count = 0;

    double min_range = 9.5 * pow(10, 9);
    double max_range = 9.9 * pow(10, 9);
    double above_max_val = 9.9 * pow(10, 9);
    // double below_min_val = -pow(10, 9);
    // double below_min_val = -above_max_val;
    double below_min_val = 5 * pow(10, 8);

    string var_substr0 = "temp";
    string var_substr1 = "press";

    extreme_debug_log << "rank: " << rank << endl;


	catalog_all_metadata_for_run (rank, num_client_procs, run_name, job_id,
							chunk_dims, timestep_file_names, timestep0_file_name, var_entries);

	vector<md_var_entry> vars_sorted(var_names_with_vers.size());
	for(int i = 0; i < var_names_with_vers.size(); i++) {
		string var_name = var_names_with_vers.at(i);
		for(md_var_entry var : var_entries ) {
			if (var.var_name == var_name) {
				vars_sorted[i] = var;
				break;
			}
		}
	}

    size_t found = timestep0_file_name.find_last_of("/");
    // cout << "timestep0_file_name: " << timestep0_file_name << endl;
    // cout << "timestep_file_name.substr(found+1): " << timestep0_file_name.substr(found+1) << endl;
   	uint64_t timestep0_id = stoull(timestep0_file_name.substr(found+1));


	catalog_types_in_timestep_functs ( rank, num_client_procs, timestep0_id,
				timestep0_file_name, vars_sorted.at(6).var_name,
                    get_dims_to_search(vars_sorted.at(6), dims_funct_count) );

	dims_funct_count += 1;


	catalog_timesteps_with_var_or_attr_functs ( rank, num_client_procs,
					timestep_file_names, type_names.at(2), vars_sorted.at(8).var_name,
    				get_dims_to_search(vars_sorted.at(8), dims_funct_count) ); 


	dims_funct_count += 1;

	catalog_run_attributes_with_type_or_val_functs ( rank, num_client_procs,
							run_name, job_id, type_names.at(10) );


	catalog_timestep_attributes_with_type_or_val_functs ( rank, num_client_procs, 
			timestep0_id, timestep0_file_name, type_names.at(11) ); 


	catalog_all_var_attributes_with_dims ( rank, num_client_procs,
			timestep0_id, timestep0_file_name, get_dims_to_search(vars_sorted.at(7), dims_funct_count),
			chunk_dims );

	dims_funct_count += 1;


	catalog_var_attributes_with_var_functs ( rank, num_client_procs,
			timestep0_id, timestep0_file_name, vars_sorted.at(9).var_name,
    		get_dims_to_search(vars_sorted.at(9), dims_funct_count), chunk_dims );  

	dims_funct_count += 1;




	catalog_var_attributes_with_type_dims_functs ( rank, num_client_procs,
			timestep0_id, timestep0_file_name, get_dims_to_search(vars_sorted.at(4), dims_funct_count),
            type_names.at(1), chunk_dims );

	dims_funct_count += 1;



	catalog_var_attributes_with_type_var_or_val_functs ( rank, num_client_procs,
			timestep0_id, timestep0_file_name, type_names.at(7), vars_sorted.at(5).var_name,
			get_dims_to_search(vars_sorted.at(5), dims_funct_count), chunk_dims); 

	dims_funct_count += 1;



	catalog_types_with_var_substr_in_timestep_functs ( rank, num_client_procs,
			timestep0_id, timestep0_file_name, var_substr0, 
			get_dims_to_search(vars_sorted.at(0), dims_funct_count) ); 

    	
	dims_funct_count += 1;



	catalog_timesteps_with_var_substr_or_val_functs ( rank, num_client_procs,
			timestep0_id, timestep_file_names, type_names.at(3), var_substr0,
			get_dims_to_search(vars_sorted.at(0), dims_funct_count) ); 

	dims_funct_count += 1;



	catalog_var_attributes_with_var_name_substr_functs ( rank, num_client_procs,
			timestep0_id, timestep0_file_name, var_substr1, 
			get_dims_to_search(vars_sorted.at(2), dims_funct_count), chunk_dims ); 

	dims_funct_count += 1;



	catalog_var_attributes_with_type_var_name_substr_or_val_functs ( rank, num_client_procs,
			timestep0_id, timestep0_file_name, var_substr1, type_names.at(6),
		    get_dims_to_search(vars_sorted.at(2), dims_funct_count), chunk_dims); 

	dims_funct_count += 1;

	//have moved it so tiering test is done is 3D read
 //    //type: note_freq
	// catalog_var_attributes_with_type_var ( rank, num_client_procs, 
	// 											run, type_names.at(5), vars_sorted.at(1), chunk_dims );
 //    if (rc != RC_OK) {
 //    }

 //    //type: note_ifreq
	// catalog_var_attributes_with_type_var ( rank, num_client_procs, 
	// 											run, type_names.at(6), vars_sorted.at(1), chunk_dims );
 //    if (rc != RC_OK) {
 //    }

 //    //type: note_rare
	// catalog_var_attributes_with_type_var ( rank, num_client_procs, 
	// 											run, type_names.at(7), vars_sorted.at(1), chunk_dims );

 //    if (rc != RC_OK) {
 //    }
 
}


md_dim_bounds get_dims_to_search(md_var_entry var, int dims_funct_count) {

	int num_dims_functs = 10;

	int plane_dim = dims_funct_count % 3;

	hsize_t search_mins[var.num_dims];
	hsize_t search_maxs[var.num_dims];

	for (int dim = 0; dim < var.num_dims; dim++) {
		if (dim == plane_dim) {
			search_mins[dim] = (dims_funct_count + 1) * (var.dims[dim] ) / (num_dims_functs + 2)  ;
			search_maxs[dim] = search_mins[dim];			
		}
		else  {
			search_mins[dim] =  0;
			search_maxs[dim] = var.dims[dim] - 1;	
		}
	}

	extreme_debug_log << "dims_funct_count: " << dims_funct_count << " dims_to_search: ";
	for(int j=0; j< var.num_dims; j++) {
        extreme_debug_log << " d" << j << "_min: " << search_mins [j];
        extreme_debug_log << " d" << j << "_max: " << search_maxs [j];               
    }
    extreme_debug_log << endl;

	return md_dim_bounds(var.num_dims, search_mins[0], search_maxs[0], 
		search_mins[1], search_maxs[1], search_mins[2], search_maxs[2]);
}