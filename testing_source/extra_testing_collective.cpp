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



#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <math.h>

// #include <my_metadata_client.h>
#include <extra_testing_collective_helper_functions.hh>
#include <client_timing_constants_read.hh>

using namespace std;

extern void add_timing_point(int catg);

static bool extreme_debug_logging = false;
static bool debug_logging = false;
static bool error_logging = true;
static bool testing_logging = false;
static bool zero_rank_logging = false;
static debugLog error_log = debugLog(error_logging);
static debugLog debug_log = debugLog(debug_logging);
static debugLog extreme_debug_log = debugLog(extreme_debug_logging);
static debugLog testing_log = debugLog(testing_logging);

void find_dims_to_search(const vector<md_dim_bounds> &var_dims, vector<md_dim_bounds> &dims_to_search, 
					int dims_funct_count);

int extra_testing_collective(const md_server &server, int rank, uint32_t num_servers, uint32_t num_client_procs, const md_catalog_run_entry &run, 
                  const vector<md_dim_bounds> &chunk_dims) 
{
    int rc;

    uint32_t count;
    // vector<md_catalog_run_entry> run_entries;
    vector<md_catalog_timestep_entry> timestep_entries;
    vector<md_catalog_var_entry> var_entries;
    vector<md_catalog_type_entry> type_entries;
    vector<md_catalog_run_attribute_entry> run_attr_entries;
    vector<md_catalog_timestep_attribute_entry> timestep_attr_entries;
    vector<md_catalog_var_attribute_entry> var_attr_entries;

    testing_log << "starting extra testing" << endl;



    // uint32_t num_var_entries;
    // timestep0_vars.at(0);
    // // type_entries.at(3);
    // md_catalog_type_entry type;
    // uint32_t num_type_entries;
    // uint32_t num_attrs;

    uint64_t txn_id = -1;

    md_catalog_timestep_entry timestep0;
    vector<md_catalog_var_entry> timestep0_vars;


    int num_dims = 3;
    // vector<md_dim_bounds> dims_to_search(3);

    // dims_to_search[0].min = 50;
    // dims_to_search[0].max = 99;
    // dims_to_search[1].min = 50;
    // dims_to_search[1].max = 99;
    // dims_to_search[2].min = 50;
    // dims_to_search[2].max = 99;

    vector<md_dim_bounds> dims_to_search(3);

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

    rc = catalog_all_metadata_for_run ( server, rank, num_servers, num_client_procs, run.run_id, 
    						chunk_dims, txn_id, timestep0, timestep0_vars, type_entries);
    if (rc != RC_OK) {
        error_log << "Error with catalog_all_metadata_for_run" << endl;
    }

    if (rank < num_servers) {
    	find_dims_to_search(timestep0_vars.at(6).dims, dims_to_search, dims_funct_count);
    }
    rc = catalog_types_in_timestep_functs ( server, rank, num_servers, num_client_procs,
    				run.run_id, timestep0.timestep_id, timestep0_vars.at(6).var_id, 
                    num_dims, dims_to_search,  txn_id );
    if (rc != RC_OK) {
        error_log << "Error with catalog_types_in_timestep_functs" << endl;
    }
	dims_funct_count += 1;

    if (rank < num_servers) {
    	find_dims_to_search(timestep0_vars.at(8).dims, dims_to_search, dims_funct_count);
    }

    rc = catalog_timesteps_with_var_or_attr_functs ( server, rank, num_servers, num_client_procs,
    				run.run_id, type_entries.at(2).type_id, type_entries.at(3).type_id, timestep0_vars.at(8).var_id,
                    num_dims, dims_to_search, txn_id,
                    min_range, max_range, above_max_val, below_min_val
                    );
    if (rc != RC_OK) {
        error_log << "Error with catalog_timesteps_with_var_or_attr_functs" << endl;
    }
	dims_funct_count += 1;

    rc = catalog_run_attributes_with_type_or_val_functs ( server, rank, num_servers, num_client_procs, 
    						run.run_id, type_entries.at(10).type_id, txn_id,
                            min_range, max_range, above_max_val, below_min_val
                            );
    if (rc != RC_OK) {
        error_log << "Error with catalog_run_attributes_with_type_or_val_functs" << endl;
    }

    rc = catalog_timestep_attributes_with_type_or_val_functs ( server, rank, num_servers, num_client_procs,
    						 run.run_id, timestep0.timestep_id, type_entries.at(11).type_id, txn_id,
                            -max_range, -min_range, -below_min_val, -above_max_val
                            );
    if (rc != RC_OK) {
        error_log << "Error with catalog_timestep_attributes_with_type_or_val_functs" << endl;
    }

    if (rank < num_servers) {
    	find_dims_to_search(timestep0_vars.at(7).dims, dims_to_search, dims_funct_count);
    }
    rc = catalog_all_var_attributes_with_dims ( server, rank, num_servers, num_client_procs, run, 
    						timestep0_vars, run.run_id, timestep0.timestep_id, 
                            txn_id, num_dims, dims_to_search, chunk_dims 
                            );
    if (rc != RC_OK) {
        error_log << "Error with catalog_all_var_attributes_with_dims" << endl;
    }
	dims_funct_count += 1;

    if (rank < num_servers) {
    	find_dims_to_search(timestep0_vars.at(9).dims, dims_to_search, dims_funct_count);
    }
    rc = catalog_var_attributes_with_var_functs ( server, rank, num_servers, num_client_procs, run, timestep0_vars.at(9), 
                                num_dims, dims_to_search, chunk_dims, txn_id 
                                );
    if (rc != RC_OK) {
        error_log << "Error with catalog_var_attributes_with_var_functs" << endl;
    }
	dims_funct_count += 1;

    if (rank < num_servers) {
    	find_dims_to_search(timestep0_vars.at(4).dims, dims_to_search, dims_funct_count);
    }
    rc = catalog_var_attributes_with_type_dims_functs ( server, rank, num_servers, num_client_procs, 
    									run, timestep0_vars,
    									timestep0.timestep_id, num_dims, 
                                        dims_to_search, type_entries.at(1), chunk_dims, txn_id 
                                        );
    if (rc != RC_OK) {
        error_log << "Error with catalog_var_attributes_with_type_dims_functs" << endl;
    }
	dims_funct_count += 1;

    if (rank < num_servers) {
    	find_dims_to_search(timestep0_vars.at(5).dims, dims_to_search, dims_funct_count);
    }
    //since using type = min, flip the values
    rc = catalog_var_attributes_with_type_var_or_val_functs ( server, rank, num_servers, num_client_procs,
    									run, type_entries.at(7),
    									type_entries.at(4), timestep0_vars, timestep0_vars.at(5),
                                        txn_id, num_dims, dims_to_search,
                                        -max_range, -min_range, -below_min_val, -above_max_val,  chunk_dims
                                        );
    if (rc != RC_OK) {
        error_log << "Error with catalog_var_attributes_with_type_var_or_val_functs" << endl;
    }
	dims_funct_count += 1;

    if (rank < num_servers) {
    	find_dims_to_search(timestep0_vars.at(0).dims, dims_to_search, dims_funct_count);
    }
    rc = catalog_types_with_var_substr_in_timestep_functs ( server, rank, num_servers, num_client_procs,
    										run.run_id, timestep0.timestep_id, var_substr0,
						                    num_dims, dims_to_search, txn_id 
						                    );
    if (rc != RC_OK) {
        error_log << "Error with catalog_types_with_var_substr_in_timestep_functs" << endl;
    }
	dims_funct_count += 1;

    if (rank < num_servers) {
    	find_dims_to_search(timestep0_vars.at(0).dims, dims_to_search, dims_funct_count);
    }
    rc = catalog_timesteps_with_var_substr_or_val_functs ( server, rank, num_servers, num_client_procs,
    				 run.run_id, type_entries.at(3).type_id, 
                     var_substr0, num_dims, dims_to_search, txn_id,
                     min_range, max_range, above_max_val, below_min_val
                     );
    if (rc != RC_OK) {
        error_log << "Error with catalog_timesteps_with_var_substr_or_val_functs" << endl;
    }
	dims_funct_count += 1;

    if (rank < num_servers) {
    	find_dims_to_search(timestep0_vars.at(2).dims, dims_to_search, dims_funct_count);
    }
    rc = catalog_var_attributes_with_var_name_substr_functs ( server, rank, num_servers, num_client_procs,
    										run, timestep0_vars,
    										run.run_id, timestep0.timestep_id,
                                            var_substr1, 
                                            num_dims, dims_to_search, txn_id, chunk_dims
                                            );
    if (rc != RC_OK) {
        error_log << "Error with catalog_var_attributes_with_var_name_substr_functs" << endl;
    }
	dims_funct_count += 1;

    if (rank < num_servers) {
    	find_dims_to_search(timestep0_vars.at(2).dims, dims_to_search, dims_funct_count);
    }
    rc = catalog_var_attributes_with_type_var_name_substr_or_val_functs ( server, rank,  num_servers, num_client_procs,
    												run, timestep0_vars,
    												run.run_id, timestep0.timestep_id,
                                                    var_substr1, type_entries.at(6).type_id,
                                                    type_entries.at(3).type_id, num_dims, dims_to_search,
                                                    txn_id,
                                                    min_range, max_range, above_max_val, below_min_val, chunk_dims
                                                    );
    if (rc != RC_OK) {
        error_log << "Error with catalog_var_attributes_with_type_var_name_substr_or_val_functs" << endl;
    }
	dims_funct_count += 1;

	//have moved it so tiering test is done is 3D read
 //    //type: note_freq
	// rc = catalog_var_attributes_with_type_var ( server, rank, num_servers, num_client_procs, 
	// 											run, type_entries.at(5), timestep0_vars.at(1), txn_id, chunk_dims );
 //    if (rc != RC_OK) {
 //        error_log << "Error with catalog_var_attributes_with_type_var for type_id: " << type_entries.at(5).type_id << endl;
 //    }

 //    //type: note_ifreq
	// rc = catalog_var_attributes_with_type_var ( server, rank, num_servers, num_client_procs, 
	// 											run, type_entries.at(6), timestep0_vars.at(1), txn_id, chunk_dims );
 //    if (rc != RC_OK) {
 //        error_log << "Error with catalog_var_attributes_with_type_var for type_id: " << type_entries.at(6).type_id << endl;
 //    }

 //    //type: note_rare
	// rc = catalog_var_attributes_with_type_var ( server, rank, num_servers, num_client_procs, 
	// 											run, type_entries.at(7), timestep0_vars.at(1), txn_id, chunk_dims );

 //    if (rc != RC_OK) {
 //        error_log << "Error with catalog_var_attributes_with_type_var for type_id: " << type_entries.at(7).type_id << endl;
 //    }

    return rc;
 
}


void find_dims_to_search(const vector<md_dim_bounds> &var_dims, vector<md_dim_bounds> &dims_to_search, 
							int dims_funct_count) {

	int num_dims_functs = 10;

	int plane_dim = dims_funct_count % 3;

	for (int dim = 0; dim < var_dims.size(); dim++) {
		if (dim == plane_dim) {
			dims_to_search[dim].min = (dims_funct_count + 1) * (var_dims[dim].max - var_dims[dim].min + 1) / (num_dims_functs + 2)  ;
			dims_to_search[dim].max = dims_to_search[dim].min;			
		}
		else  {
			dims_to_search[dim].min = var_dims[dim].min ;
			dims_to_search[dim].max = var_dims[dim].max ;	
		}
	}
	extreme_debug_log << "dims_funct_count: " << dims_funct_count << " dims_to_search: ";
	for(int j=0; j< var_dims.size(); j++) {
        extreme_debug_log << " d" << j << "_min: " << dims_to_search [j].min;
        extreme_debug_log << " d" << j << "_max: " << dims_to_search [j].max;               
    }
    extreme_debug_log << endl;
}