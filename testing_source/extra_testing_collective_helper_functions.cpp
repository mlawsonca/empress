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



#include <my_metadata_client.h>

#include <client_timing_constants_read.hh>

#include <sstream>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

#include <mpi.h>

//includes the print attr functs
#include <testing_harness_debug_helper_functions.hh>
#include <hdf5_helper_functions_read.hh>


#include <unordered_set>

using namespace std;

static bool testing_logging = false;
static bool extreme_debug_logging = false;
static bool zero_rank_logging = false;
static bool error_logging  = true;

static debugLog testing_log = debugLog(testing_logging, zero_rank_logging);
static debugLog error_log = debugLog(error_logging, zero_rank_logging);
static debugLog extreme_debug_log = debugLog(extreme_debug_logging, false);
static debugLog zero_rank_log = debugLog(true);

extern bool hdf5_read_data;

extern void add_objector_point(int catg, int num_params_to_fetch);
extern void add_timing_point(int catg);

extern vector<objector_params> all_objector_params;
extern bool output_objector_params;
extern bool output_obj_names;

template <class T1, class T2>
static void make_range_data (T1 min_int, T2 max_int, string &serial_str);

template <class T2>
static void make_single_val_data (T2 above_max_val, string &serial_str);

template <class T>
void gather_attr_entries(vector<T> attr_entries, uint32_t count, int rank, uint32_t num_servers, uint32_t num_client_procs,
		vector<T> &all_attr_entries);

void gather_timestep_entries(vector<md_catalog_timestep_entry> entries, int rank, uint32_t num_servers, uint32_t num_client_procs,
		vector<md_catalog_timestep_entry> &all_entries);

void gather_type_entries(vector<md_catalog_type_entry> entries, int rank, uint32_t num_servers, uint32_t num_client_procs,
		vector<md_catalog_type_entry> &all_entries);

// extern objector_params get_objector_params ( const md_catalog_run_entry &run, const md_catalog_var_entry &var, 
//                             const vector<md_dim_bounds> &bounding_box, OBJECTOR_PARAMS_READ_TYPES read_type );

extern objector_params get_objector_params ( const md_catalog_run_entry &run, const md_catalog_var_entry &var, 
                            const vector<md_dim_bounds> &bounding_box, OBJECTOR_PARAMS_READ_TYPES read_type,
                            const vector<md_dim_bounds> &proc_dims);

extern int add_object_names_offests_and_counts(const md_catalog_run_entry &run, const md_catalog_var_entry &var,
                                    const vector<md_dim_bounds> &proc_dims, uint16_t read_type);

extern vector<md_dim_bounds> get_overlapping_dims_for_bounding_box ( const vector<md_dim_bounds> &bounding_box, 
                            const vector<md_dim_bounds> &proc_dims
                            );


// extern std::map <string, vector<double>> data_outputs;

// int retrieveObjNamesAndDataForAttrCatalog(std::map <string, vector<double>> data_outputs, const md_server &server, int rank,
//                                          const md_catalog_run_entry &run, uint64_t run_id, uint64_t timestep_id,
//                                          uint64_t txn_id,
//                                          vector<md_catalog_var_attribute_entry> attr_entries );



///////////ALL OPS INCLUDING BY NAME/VER WHERE POSSIBLE


int catalog_all_metadata_for_run (const md_server &server, int rank, uint32_t num_servers, uint32_t num_client_procs, uint64_t run_id, 
							const vector<md_dim_bounds> &proc_dims, uint64_t txn_id,
							md_catalog_timestep_entry &timestep0,
							vector<md_catalog_var_entry> &timestep0_vars, vector<md_catalog_type_entry> &type_entries) 
{
	int rc = RC_OK;

  	vector<md_catalog_run_entry> run_entries;
  	vector<md_catalog_timestep_entry> timestep_entries;
    vector<md_catalog_var_entry> var_entries;
    vector<md_catalog_run_attribute_entry> run_attr_entries;
    vector<md_catalog_timestep_attribute_entry> timestep_attr_entries;
    vector<md_catalog_var_attribute_entry> var_attr_entries;

    uint32_t count;

    MPI_Barrier(MPI_COMM_WORLD);
    add_timing_point(CATALOG_RUN_START);
    rc = metadata_catalog_run (server, txn_id, count, run_entries);
    if (rc == RC_OK) {
        if(testing_logging || (zero_rank_logging && rank == 0)) {
            zero_rank_log << "new run catalog for txn_id: " << txn_id << " for server: " << rank << " \n";
            print_run_catalog (count, run_entries);
        }
    }
    else {
        error_log << "Error cataloging the new set of type entries. Proceeding \n";
    }
    add_timing_point(CATALOG_RUN_DONE);

    MPI_Barrier(MPI_COMM_WORLD);
    add_timing_point(CATALOG_TYPE_START);
    rc = metadata_catalog_type (server, run_id, txn_id, count, type_entries);
    if (rc == RC_OK) {
        if(testing_logging || (zero_rank_logging && rank == 0)) {
            zero_rank_log << "new type catalog for run_id " << run_id << " and txn_id: " << txn_id << " for server: " << rank << " \n";
            print_type_catalog (count, type_entries);
        }
    }
    else {
        error_log << "Error cataloging the new set of type entries. Proceeding \n";
    }
    add_timing_point(CATALOG_TYPE_DONE);

    MPI_Barrier(MPI_COMM_WORLD);
   	add_timing_point(CATALOG_ALL_RUN_ATTRIBUTES_START);
    vector<md_catalog_run_attribute_entry> all_run_attr_entries;
    if(rank < num_servers) {
	    rc = metadata_catalog_all_run_attributes ( server, run_id, txn_id, count, run_attr_entries );
	    if (rc == RC_OK) {
	        if(testing_logging || (zero_rank_logging && rank == 0)) {
	     	    zero_rank_log << "run attributes associated with run_id " << run_id << " and txn_id: " << txn_id << " for server: " << rank << " \n";
	            print_run_attribute_list (count, run_attr_entries);
	     	    print_run_attr_data(count, run_attr_entries);
	        }
	    }
	    else {
	        error_log << "Error getting the matching attribute list. Proceeding \n";
	    }
	}
	add_timing_point(CATALOG_ALL_RUN_ATTRIBUTES_DONE);

    MPI_Barrier(MPI_COMM_WORLD); //leave to make timing clearer	
	add_timing_point(CATALOG_ALL_RUN_ATTRIBUTES_COLLECTIVE_START);
    gather_attr_entries(run_attr_entries, count, rank, num_servers, num_client_procs, all_run_attr_entries);
    if(testing_logging || (zero_rank_logging  && rank == 0)) {
  		zero_rank_log << "rank: " << rank << " all run attributes associated with run_id " << run_id << 
  			" note: all_run_attr_entries.size(): " << all_run_attr_entries.size() << endl;
    	print_run_attribute_list (all_run_attr_entries.size(), all_run_attr_entries);
    	print_run_attr_data(all_run_attr_entries.size(), all_run_attr_entries);
    }
    add_timing_point(CATALOG_ALL_RUN_ATTRIBUTES_COLLECTIVE_DONE);

    MPI_Barrier(MPI_COMM_WORLD);
	add_timing_point(CATALOG_TIMESTEP_START);
    rc = metadata_catalog_timestep (server, run_id, txn_id, count, timestep_entries);
    if (rc != RC_OK) {
        error_log << "Error cataloging the post deletion set of timestep entries. Proceeding \n";
    }
    if(testing_logging || (zero_rank_logging && rank == 0)) {
        zero_rank_log << "timestep catalog for run_id " << run_id << " and txn_id: " << txn_id << " for server: " << rank << " \n";
        print_timestep_catalog (count, timestep_entries);
    }
	add_timing_point(CATALOG_TIMESTEP_DONE);

    for (md_catalog_timestep_entry timestep : timestep_entries) {
    	MPI_Barrier(MPI_COMM_WORLD);
        add_timing_point(CATALOG_ALL_TIMESTEP_ATTRIBUTES_START);

       	vector<md_catalog_timestep_attribute_entry> all_timestep_attr_entries;
    	if(rank < num_servers) {
	    	rc = metadata_catalog_all_timestep_attributes ( server, run_id, timestep.timestep_id, txn_id, count, timestep_attr_entries );
		    if (rc == RC_OK) {
		        if(testing_logging || (zero_rank_logging && rank == 0)) {
		        	zero_rank_log << "timestep attributes associated with run_id " << run_id << " and timestep_id " << timestep.timestep_id << " and txn_id: " << txn_id;
		        	zero_rank_log << " for server: " << rank << " \n";		        	
		            print_timestep_attribute_list (count, timestep_attr_entries);
		     	    print_timestep_attr_data(count, timestep_attr_entries);
		        }
		    }
		    else {
		        error_log << "Error getting the matching attribute list. Proceeding \n";
		    }
		}
        add_timing_point(CATALOG_ALL_TIMESTEP_ATTRIBUTES_DONE);

    	MPI_Barrier(MPI_COMM_WORLD); //leave to make timing clearer
		add_timing_point(CATALOG_ALL_TIMESTEP_ATTRIBUTES_COLLECTIVE_START);
	    gather_attr_entries(timestep_attr_entries, count, rank, num_servers, num_client_procs, all_timestep_attr_entries);
	    if(testing_logging || (zero_rank_logging  && rank == 0)) {
	  		zero_rank_log << "rank: " << rank << " all timestep attributes associated with run_id " << run_id << 
	  			" and timestep_id " << timestep.timestep_id << 
	  			" note: all_timestep_attr_entries.size(): " << all_timestep_attr_entries.size() << endl;
	    	print_timestep_attribute_list (all_timestep_attr_entries.size(), all_timestep_attr_entries);
	    	print_timestep_attr_data(all_timestep_attr_entries.size(), all_timestep_attr_entries);
	    }		
        add_timing_point(CATALOG_ALL_TIMESTEP_ATTRIBUTES_COLLECTIVE_DONE);

    	MPI_Barrier(MPI_COMM_WORLD);
        add_timing_point(CATALOG_VAR_START);    	
        rc = metadata_catalog_var(server, run_id, timestep.timestep_id, txn_id, count, var_entries);
        if (rc != RC_OK) {
            error_log << "Error cataloging the post deletion set of var_entries. Proceeding \n";
        }
        if(testing_logging || (zero_rank_logging && rank == 0)) {
	        zero_rank_log << "var catalog for run_id " << run_id << " and timestep_id " << timestep.timestep_id << " and txn_id: " << txn_id << 
	        	" for server: " << rank << " \n";        	
            print_var_catalog (count, var_entries);
        } 
        add_timing_point(CATALOG_VAR_DONE);    	

        for (md_catalog_var_entry var : var_entries) {   	
        	vector<md_catalog_var_attribute_entry> all_var_attr_entries;

        	MPI_Barrier(MPI_COMM_WORLD);
        	add_timing_point(CATALOG_ALL_VAR_ATTRIBUTES_WITH_VAR_BY_ID_START);
        	if(rank < num_servers) {
	            rc = metadata_catalog_all_var_attributes_with_var_by_id (server, run_id, timestep.timestep_id, var.var_id, txn_id, count, var_attr_entries);
	            if (rc == RC_OK) {
	                // if(testing_logging || (zero_rank_logging && rank == 0)) {
	              	 //    zero_rank_log << "var attributes associated with run_id " << run_id << " and timestep_id " << timestep.timestep_id << " and txn_id: " << 
	                // 		txn_id << " and var_id: " << var.var_id << " for server: " << rank << " \n";
	                //     print_var_attribute_list (count, var_attr_entries);
	                //     print_var_attr_data(count, var_attr_entries);
	                // }
	            }
	            else {
	                error_log << "Error getting the matching attribute list. Proceeding \n";
	            }
	        }
        	add_timing_point(CATALOG_ALL_VAR_ATTRIBUTES_WITH_VAR_BY_ID_DONE);    	

        	MPI_Barrier(MPI_COMM_WORLD); //leave to make timing clearer
            add_timing_point(CATALOG_ALL_VAR_ATTRIBUTES_WITH_VAR_BY_ID_COLLECTIVE_START);
	        gather_attr_entries(var_attr_entries, count, rank, num_servers, num_client_procs, all_var_attr_entries);
            if(testing_logging || (zero_rank_logging  && rank == 0)) {
          		zero_rank_log << "rank: " << rank << " all var attributes associated with run_id " << run_id << " and timestep_id " << timestep.timestep_id << 
          			" and var_id: " << var.var_id << " note: all_var_attr_entries.size(): " << all_var_attr_entries.size() << endl;
            	print_var_attribute_list (all_var_attr_entries.size(), all_var_attr_entries);
            	print_var_attr_data(all_var_attr_entries.size(), all_var_attr_entries);
            }

            //this produces too many objects to retrieve
	       //  if (output_objector_params) {
	       //  	count = 0;
		      //   for (md_catalog_var_attribute_entry attr : all_var_attr_entries) {
		      //   	if (dims_overlap ( attr.dims, proc_dims) ) {
		      //   		all_objector_params.push_back( get_objector_params(run_entries.at(0), var, attr.dims) );
		      //   		count += 1;
		      //   	}
		      //   }
		     	// add_objector_point(BOUNDING_BOX_TO_OBJ_NAMES, count);
	       //  }
            add_timing_point(CATALOG_ALL_VAR_ATTRIBUTES_WITH_VAR_BY_ID_COLLECTIVE_DONE);

        }

        if (timestep.timestep_id == 0) {
    		timestep0 = timestep;
    		timestep0_vars = var_entries;
    	}

    }
    return rc;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//TYPE FUNCTS //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int catalog_types_in_timestep_functs ( const md_server &server, int rank, uint32_t num_servers, uint32_t num_client_procs, 
					uint64_t run_id, uint64_t timestep_id, uint64_t var_id,
                    int num_dims, const vector<md_dim_bounds> &dims_to_search, uint64_t txn_id ) 
{

    int rc = RC_OK;
    uint32_t count;
    vector<md_catalog_type_entry> type_entries;

    vector<md_catalog_type_entry> all_type_entries;

    MPI_Barrier(MPI_COMM_WORLD);
    add_timing_point(CATALOG_ALL_TYPES_WITH_VAR_ATTRIBUTES_IN_TIMESTEP_START);
    if (rank < num_servers) {
	    rc = metadata_catalog_all_types_with_var_attributes_in_timestep (server, run_id, timestep_id, txn_id, count, type_entries);

	    if (rc == RC_OK) {
	   	    // if(testing_logging || (zero_rank_logging && rank == 0)) {
		       //  zero_rank_log << "using metadata_catalog_all_types_with_var_attributes_in_timestep funct: "
		       //  	<< " types associated with a var attribute for run_id " << run_id << ", timestep_id " << timestep_id << 
		       //      " and txn_id: " << txn_id << " for server: " << rank << "\n";
	        //     print_type_catalog (count, type_entries);
	        // }
	    }
	    else {
	        error_log << "Error getting the matching type catalog. Proceeding \n";
	    }
	}
	add_timing_point(CATALOG_ALL_TYPES_WITH_VAR_ATTRIBUTES_IN_TIMESTEP_DONE);

    MPI_Barrier(MPI_COMM_WORLD); //leave to make timing clearer
    add_timing_point(CATALOG_ALL_TYPES_WITH_VAR_ATTRIBUTES_IN_TIMESTEP_COLLECTIVE_START);
	gather_type_entries(type_entries, rank, num_servers, num_client_procs, all_type_entries);
	if(testing_logging || (zero_rank_logging  && rank == 0)) {
		zero_rank_log << "rank: " << rank << " all types associated with a var attribute for run_id " << run_id << ", timestep_id " << timestep_id <<
			 " note: all_type_entries.size(): " << all_type_entries.size() << endl;
		print_type_catalog (all_type_entries.size(), all_type_entries);
	}
    add_timing_point(CATALOG_ALL_TYPES_WITH_VAR_ATTRIBUTES_IN_TIMESTEP_COLLECTIVE_DONE);

	all_type_entries.clear();

    MPI_Barrier(MPI_COMM_WORLD);
    add_timing_point(CATALOG_ALL_TYPES_WITH_VAR_ATTRIBUTES_WITH_VAR_IN_TIMESTEP_START);
	if (rank < num_servers) {
	    rc = metadata_catalog_all_types_with_var_attributes_with_var_in_timestep (server, run_id, timestep_id, var_id, txn_id, count, type_entries);

	    if (rc == RC_OK) {
	    	// if(testing_logging || (zero_rank_logging && rank == 0)) {
		    //     zero_rank_log << "using catalog all types with instances on var in timestep funct: types associated with run_id " << run_id << ", timestep_id " << timestep_id << 
		    //        " var_id: " << var_id << " and txn_id: " << txn_id << " for server: " << rank << "\n";
		    //     print_type_catalog (count, type_entries);
	     //    }
	    }
	    else {
	        error_log << "Error getting the matching type catalog. Proceeding \n";
	    }
	}
    add_timing_point(CATALOG_ALL_TYPES_WITH_VAR_ATTRIBUTES_WITH_VAR_IN_TIMESTEP_DONE);

    MPI_Barrier(MPI_COMM_WORLD); //leave to make timing clearer
    add_timing_point(CATALOG_ALL_TYPES_WITH_VAR_ATTRIBUTES_WITH_VAR_IN_TIMESTEP_COLLECTIVE_START);
	gather_type_entries(type_entries, rank, num_servers, num_client_procs, all_type_entries);
	if(testing_logging || (zero_rank_logging  && rank == 0)) {
		zero_rank_log << "rank: " << rank << " all types associated with a var attribute for run_id " << run_id << ", timestep_id " << timestep_id <<
			 " var_id: " << var_id << " note: all_type_entries.size(): " << all_type_entries.size() << endl;
		print_type_catalog (all_type_entries.size(), all_type_entries);
	}
    add_timing_point(CATALOG_ALL_TYPES_WITH_VAR_ATTRIBUTES_WITH_VAR_IN_TIMESTEP_COLLECTIVE_DONE);

	all_type_entries.clear();

    MPI_Barrier(MPI_COMM_WORLD);
    add_timing_point(CATALOG_ALL_TYPES_WITH_VAR_ATTRIBUTES_WITH_VAR_DIMS_IN_TIMESTEP_START);
	if (rank < num_servers) {
	    rc = metadata_catalog_all_types_with_var_attributes_with_var_dims_in_timestep (server, run_id, timestep_id, var_id, txn_id, num_dims, dims_to_search, count, type_entries);

	    if (rc == RC_OK) {
	    	// if(testing_logging || (zero_rank_logging && rank == 0)) {
		    //    zero_rank_log << "using catalog all types with instances on var dims in timestep funct: types associated with run_id " << run_id << ", timestep_id " << timestep_id << 
		    //         " var_id: " << var_id << " and txn_id: " << txn_id << " overlapping with dims";
		    //     for(int j=0; j< num_dims; j++) {
		    //         zero_rank_log << " d" << j << "_min: " << dims_to_search [j].min;
		    //         zero_rank_log << " d" << j << "_max: " << dims_to_search [j].max;               
		    //     }
		    //     zero_rank_log << " for server: " << rank << " \n";
		    //         print_type_catalog (count, type_entries);
	     //    }
	    }
	    else {
	        error_log << "Error getting the matching type catalog. Proceeding \n";
	    }
	}
    add_timing_point(CATALOG_ALL_TYPES_WITH_VAR_ATTRIBUTES_WITH_VAR_DIMS_IN_TIMESTEP_DONE);

    MPI_Barrier(MPI_COMM_WORLD); //leave to make timing clearer
    add_timing_point(CATALOG_ALL_TYPES_WITH_VAR_ATTRIBUTES_WITH_VAR_DIMS_IN_TIMESTEP_COLLECTIVE_START);
	gather_type_entries(type_entries, rank, num_servers, num_client_procs, all_type_entries);
	if(testing_logging || (zero_rank_logging  && rank == 0)) {
		zero_rank_log << "rank: " << rank << " all types associated with a var attribute for run_id " << run_id << ", timestep_id " << timestep_id <<
			 " var_id: " << var_id << " overlapping with dims";
	        for(int j=0; j< num_dims; j++) {
	            zero_rank_log << " d" << j << "_min: " << dims_to_search [j].min;
	            zero_rank_log << " d" << j << "_max: " << dims_to_search [j].max;               
	        }
			zero_rank_log << " note: all_type_entries.size(): " << all_type_entries.size() << endl;
		print_type_catalog (all_type_entries.size(), all_type_entries);
	}
    add_timing_point(CATALOG_ALL_TYPES_WITH_VAR_ATTRIBUTES_WITH_VAR_DIMS_IN_TIMESTEP_COLLECTIVE_DONE);

    return rc;
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//TIMESTEP FUNCTS //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int catalog_timesteps_with_var_or_attr_functs ( const md_server &server, int rank, uint32_t num_servers, uint32_t num_client_procs,
					uint64_t run_id, uint64_t rare_type_id, uint64_t value_type_id, uint64_t var_id,
                    int num_dims, const vector<md_dim_bounds> &dims_to_search, uint64_t txn_id,
                    double min_range, double max_range, double above_max_val, double below_min_val
 					) 
{

    int rc = RC_OK;
    uint32_t count;
    vector<md_catalog_timestep_entry> timestep_entries;


    // //Should be empty now 
    // rc = metadata_catalog_timestep (server, run_id, txn_id, count, timestep_entries);
    // if (rc != RC_OK) {
    //     error_log << "Error set of timestep entries. Proceeding \n";
    // }
    // if(testing_logging || (zero_rank_logging && rank == 0)) {
	    // zero_rank_log << "timestep catalog for run_id " << run_id << " and txn_id " << txn_id << ": \n";
    //     print_timestep_catalog (count, timestep_entries);
    // }

    MPI_Barrier(MPI_COMM_WORLD);
    add_timing_point(CATALOG_ALL_TIMESTEPS_WITH_VAR_START);
    rc = metadata_catalog_all_timesteps_with_var (server, run_id, var_id, txn_id, count, timestep_entries);
    if (rc == RC_OK) {
        if(testing_logging || (zero_rank_logging && rank == 0)) {
	        zero_rank_log << "using catalog_all_timesteps_with_var funct: timesteps associated with run_id " << run_id << ", var_id " << var_id << 
	            " and txn_id: " << txn_id << " for server: " << rank << " \n";        	
            print_timestep_catalog (count, timestep_entries);
        }
    }
    else {
        error_log << "Error getting the matching timestep catalog. Proceeding \n";
    }
    add_timing_point(CATALOG_ALL_TIMESTEPS_WITH_VAR_DONE);

    vector<md_catalog_timestep_entry> all_timestep_entries;

    MPI_Barrier(MPI_COMM_WORLD);
    add_timing_point(CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_START);
    if (rank < num_servers) {
	    rc = metadata_catalog_all_timesteps_with_var_attributes_with_type_var (server, run_id, rare_type_id, var_id, txn_id, count, timestep_entries);

	    if (rc == RC_OK) {
	    	// if(testing_logging || (zero_rank_logging && rank == 0)) {
		    //     zero_rank_log << "using metadata_catalog_all_timesteps_with_var_attributes_with_type_var funct: timesteps associated with run_id " << run_id << ", type_id " << type_id << 
		    //     ", var_id " << var_id << " and txn_id: " << txn_id << " for server: " << rank << " \n";
		    //         print_timestep_catalog (count, timestep_entries);
	     //    }
	    }
	    else {
	        error_log << "Error getting the matching timestep catalog. Proceeding \n";
	    }
	}
    add_timing_point(CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_DONE);

    MPI_Barrier(MPI_COMM_WORLD); //leave to make timing clearer
    add_timing_point(CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_COLLECTIVE_START);
	gather_timestep_entries(timestep_entries, rank, num_servers, num_client_procs, all_timestep_entries);
	if(testing_logging || (zero_rank_logging  && rank == 0)) {
		zero_rank_log << "rank: " << rank << " all timesteps associated with a var attribute for run_id " << run_id << 
			", type_id " << rare_type_id << ", var_id " << var_id <<
			 " note: all_timestep_entries.size(): " << all_timestep_entries.size() << endl;
		print_timestep_catalog (all_timestep_entries.size(), all_timestep_entries);
	}
    add_timing_point(CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_COLLECTIVE_DONE);

    string range_data;
    make_range_data (min_range, max_range, range_data);

	all_timestep_entries.clear();

    MPI_Barrier(MPI_COMM_WORLD);
    add_timing_point(CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_RANGE_START);
    if (rank < num_servers) {
	    rc = metadata_catalog_all_timesteps_with_var_attributes_with_type_var_range (server, run_id, value_type_id, var_id, txn_id, 
	        ATTR_DATA_TYPE_REAL, range_data, count, timestep_entries);

	    if (rc == RC_OK) {
	    	// if(testing_logging || (zero_rank_logging && rank == 0)) {
		    //     zero_rank_log << "using metadata_catalog_all_timesteps_with_var_attributes_with_type_var_range funct: timesteps associated with run_id " << run_id << ", type_id " << type_id << 
		    //     	", var_id " << var_id << " txn_id: " << txn_id << " int attr data and range [" << min_range << "," << max_range << "] for server: " << rank << " \n";
	     //        print_timestep_catalog (count, timestep_entries);
	     //    }
	    }
	    else {
	        error_log << "Error getting the matching timestep catalog. Proceeding \n";
	    }
	}
    add_timing_point(CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_RANGE_DONE);

    MPI_Barrier(MPI_COMM_WORLD); //leave to make timing clearer
    add_timing_point(CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_RANGE_COLLECTIVE_START);
	gather_timestep_entries(timestep_entries, rank, num_servers, num_client_procs, all_timestep_entries);
	if(testing_logging || (zero_rank_logging  && rank == 0)) {
		zero_rank_log << "rank: " << rank << " all timesteps associated with a var attribute for run_id " << run_id << 
			", type_id " << value_type_id << ", var_id " << var_id << " int attr data and range [" << min_range << "," << max_range << "]" << 
			 " note: all_timestep_entries.size(): " << all_timestep_entries.size() << endl;
		print_timestep_catalog (all_timestep_entries.size(), all_timestep_entries);
	}
    add_timing_point(CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_RANGE_COLLECTIVE_DONE);

    string above_max_data;
    make_single_val_data (above_max_val, above_max_data);

	all_timestep_entries.clear();

    MPI_Barrier(MPI_COMM_WORLD);
    add_timing_point(CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_ABOVE_MAX_START);
    if (rank < num_servers) {

	    rc = metadata_catalog_all_timesteps_with_var_attributes_with_type_var_above_max (server, run_id, value_type_id, var_id, txn_id, 
	        ATTR_DATA_TYPE_REAL, above_max_data, count, timestep_entries);

	    if (rc == RC_OK) {
	    	// if(testing_logging || (zero_rank_logging && rank == 0)) {
		    //     zero_rank_log << "using metadata_catalog_all_timesteps_with_var_attributes_with_type_var_range funct: timesteps associated with run_id " << run_id << ", type_id " << type_id << 
		    //     	", var_id " << var_id << " txn_id: " << txn_id << " int attr data greater than or eq to " << above_max_val << "] for server: " << rank << " \n";
	     //        print_timestep_catalog (count, timestep_entries);
	     //    }
	    }
	    else {
	        error_log << "Error getting the matching timestep catalog. Proceeding \n";
	    }
	}
    add_timing_point(CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_ABOVE_MAX_DONE);

    MPI_Barrier(MPI_COMM_WORLD); //leave to make timing clearer
    add_timing_point(CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_ABOVE_MAX_COLLECTIVE_START);
	gather_timestep_entries(timestep_entries, rank, num_servers, num_client_procs, all_timestep_entries);
	if(testing_logging || (zero_rank_logging  && rank == 0)) {
		zero_rank_log << "rank: " << rank << " all timesteps associated with a var attribute for run_id " << run_id << 
			", type_id " << value_type_id << ", var_id " << var_id << " int attr data greater than or eq to " << above_max_val << "]" << 
			 " note: all_timestep_entries.size(): " << all_timestep_entries.size() << endl;
		print_timestep_catalog (all_timestep_entries.size(), all_timestep_entries);
	}
    add_timing_point(CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_ABOVE_MAX_COLLECTIVE_DONE);

    string below_min_data;
    make_single_val_data (below_min_val, below_min_data);

	all_timestep_entries.clear();

    MPI_Barrier(MPI_COMM_WORLD);
    add_timing_point(CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_BELOW_MIN_START);
    if (rank < num_servers) {
	    rc = metadata_catalog_all_timesteps_with_var_attributes_with_type_var_below_min (server, run_id, value_type_id, var_id, txn_id, 
	        ATTR_DATA_TYPE_REAL, below_min_data, count, timestep_entries);

	    if (rc == RC_OK) {
	    	// if(testing_logging || (zero_rank_logging && rank == 0)) {
	     //    	zero_rank_log << "using metadata_catalog_all_timesteps_with_var_attributes_with_type_var_range funct: timesteps associated with run_id " << run_id << ", type_id " << type_id << 
	     //    		", var_id " << var_id << " txn_id: " << txn_id << " int attr data less than or eq to " << below_min_val << "] for server: " << rank << " \n";
	     //        print_timestep_catalog (count, timestep_entries);
	     //    }
	    }
	    else {
	        error_log << "Error getting the matching timestep catalog. Proceeding \n";
	    }
	}
    add_timing_point(CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_BELOW_MIN_DONE);

    MPI_Barrier(MPI_COMM_WORLD); //leave to make timing clearer
    add_timing_point(CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_BELOW_MIN_COLLECTIVE_START);
	gather_timestep_entries(timestep_entries, rank, num_servers, num_client_procs, all_timestep_entries);
	if(testing_logging || (zero_rank_logging  && rank == 0)) {
		zero_rank_log << "rank: " << rank << " all timesteps associated with a var attribute for run_id " << run_id << 
			", type_id " << value_type_id << ", var_id " << var_id << " int attr data less than or eq to " << below_min_val << "]" << 
			 " note: all_timestep_entries.size(): " << all_timestep_entries.size() << endl;
		print_timestep_catalog (all_timestep_entries.size(), all_timestep_entries);
	}
    add_timing_point(CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_BELOW_MIN_COLLECTIVE_DONE);

	all_timestep_entries.clear();

    MPI_Barrier(MPI_COMM_WORLD);
    add_timing_point(CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_DIMS_START);
    if (rank < num_servers) {
	    rc = metadata_catalog_all_timesteps_with_var_attributes_with_type_var_dims (server, run_id, rare_type_id, var_id, txn_id, num_dims, dims_to_search, count, timestep_entries);

	    if (rc == RC_OK) {
	    	// if(testing_logging || (zero_rank_logging && rank == 0)) {
		    //     zero_rank_log << "using metadata_catalog_all_timesteps_with_var_attributes_with_type_var_dims funct: timesteps associated with run_id " << 
		    //     	run_id << ", type_id " << type_id << 
		    //     	", var_id " << var_id << " and txn_id: " << txn_id << " overlapping with dims for server: " << rank << " \n";
		    //     for(int j=0; j< num_dims; j++) {
		    //         zero_rank_log << " d" << j << "_min: " << dims_to_search [j].min;
		    //         zero_rank_log << " d" << j << "_max: " << dims_to_search [j].max;               
		    //     }
	     //        print_timestep_catalog (count, timestep_entries);
	     //    }
	    }
	    else {
	        error_log << "Error getting the matching timestep catalog. Proceeding \n";
	    }
	}
    add_timing_point(CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_DIMS_DONE);

    MPI_Barrier(MPI_COMM_WORLD); //leave to make timing clearer
    add_timing_point(CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_DIMS_COLLECTIVE_START);
	gather_timestep_entries(timestep_entries, rank, num_servers, num_client_procs, all_timestep_entries);
	if(testing_logging || (zero_rank_logging  && rank == 0)) {
		zero_rank_log << "rank: " << rank << " all timesteps associated with a var attribute for run_id " << run_id << 
			", type_id " << rare_type_id << ", var_id " << var_id << " overlapping with dims";
        for(int j=0; j< num_dims; j++) {
            zero_rank_log << " d" << j << "_min: " << dims_to_search [j].min;
            zero_rank_log << " d" << j << "_max: " << dims_to_search [j].max;               
        }
		zero_rank_log << " note: all_timestep_entries.size(): " << all_timestep_entries.size() << endl;
		print_timestep_catalog (all_timestep_entries.size(), all_timestep_entries);
	}
    add_timing_point(CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_DIMS_COLLECTIVE_DONE);

	all_timestep_entries.clear();

    MPI_Barrier(MPI_COMM_WORLD);
    add_timing_point(CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_DIMS_RANGE_START);
	if (rank < num_servers) {
	    rc = metadata_catalog_all_timesteps_with_var_attributes_with_type_var_dims_range (server, run_id, value_type_id, var_id, txn_id, num_dims, dims_to_search, 
	        ATTR_DATA_TYPE_REAL, range_data, count, timestep_entries);

	    if (rc == RC_OK) {
	    	// if(testing_logging || (zero_rank_logging && rank == 0)) {
		    //     zero_rank_log << "using metadata_catalog_all_timesteps_with_var_attributes_with_type_var_dims funct: timesteps associated with run_id " << run_id << ", type_id " << type_id << 
		    //     ", var_id " << var_id << " and txn_id: " << txn_id << " int attr data and range [" << min_range << "," << max_range << "] overlapping with dims";
		    //     for(int j=0; j< num_dims; j++) {
		    //         zero_rank_log << " d" << j << "_min: " << dims_to_search [j].min;
		    //         zero_rank_log << " d" << j << "_max: " << dims_to_search [j].max;               
		    //     }
		    //     zero_rank_log << " for server: " << rank << " \n";
	     //        print_timestep_catalog (count, timestep_entries);
	     //    }
	    }
	    else {
	        error_log << "Error getting the matching timestep catalog. Proceeding \n";
	    }
	}
    add_timing_point(CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_DIMS_RANGE_DONE);

    MPI_Barrier(MPI_COMM_WORLD); //leave to make timing clearer
    add_timing_point(CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_DIMS_RANGE_COLLECTIVE_START);
	gather_timestep_entries(timestep_entries, rank, num_servers, num_client_procs, all_timestep_entries);
	if(testing_logging || (zero_rank_logging  && rank == 0)) {
		zero_rank_log << "rank: " << rank << " all timesteps associated with a var attribute for run_id " << run_id << 
			", type_id " << value_type_id << ", var_id " << var_id << " int attr data and range [" << min_range << "," << max_range << "]" <<
			" overlapping with dims";
        for(int j=0; j< num_dims; j++) {
            zero_rank_log << " d" << j << "_min: " << dims_to_search [j].min;
            zero_rank_log << " d" << j << "_max: " << dims_to_search [j].max;               
        }
		zero_rank_log << " note: all_timestep_entries.size(): " << all_timestep_entries.size() << endl;
		print_timestep_catalog (all_timestep_entries.size(), all_timestep_entries);
	}
    add_timing_point(CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_DIMS_RANGE_COLLECTIVE_DONE);

	all_timestep_entries.clear();

    MPI_Barrier(MPI_COMM_WORLD);
    add_timing_point(CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_DIMS_ABOVE_MAX_START);
	if (rank < num_servers) {
	    rc = metadata_catalog_all_timesteps_with_var_attributes_with_type_var_dims_above_max (server, run_id, value_type_id, var_id, txn_id, num_dims, dims_to_search, 
	        ATTR_DATA_TYPE_REAL, above_max_data, count, timestep_entries);

	    if (rc == RC_OK) {
	    	// if(testing_logging || (zero_rank_logging && rank == 0)) {
		    //     zero_rank_log << "using metadata_catalog_all_timesteps_with_var_attributes_with_type_var_dims funct: timesteps associated with run_id " << run_id << ", type_id " << type_id << 
		    //     ", var_id " << var_id << " and txn_id: " << txn_id << " int attr data greater than or eq to " << above_max_val << " overlapping with dims";
		    //     for(int j=0; j< num_dims; j++) {
		    //         zero_rank_log << " d" << j << "_min: " << dims_to_search [j].min;
		    //         zero_rank_log << " d" << j << "_max: " << dims_to_search [j].max;               
		    //     }
		    //     zero_rank_log << " for server: " << rank << " \n";
	     //        print_timestep_catalog (count, timestep_entries);
		    // }
	    }
	    else {
	        error_log << "Error getting the matching timestep catalog. Proceeding \n";
	    }
	}
    add_timing_point(CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_DIMS_ABOVE_MAX_DONE);

    MPI_Barrier(MPI_COMM_WORLD); //leave to make timing clearer
    add_timing_point(CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_DIMS_ABOVE_MAX_COLLECTIVE_START);
	gather_timestep_entries(timestep_entries, rank, num_servers, num_client_procs, all_timestep_entries);
	if(testing_logging || (zero_rank_logging  && rank == 0)) {
		zero_rank_log << "rank: " << rank << " all timesteps associated with a var attribute for run_id " << run_id << 
			", type_id " << value_type_id << ", var_id " << var_id << " int attr data greater than or eq to " << above_max_val <<
			" overlapping with dims";
        for(int j=0; j< num_dims; j++) {
            zero_rank_log << " d" << j << "_min: " << dims_to_search [j].min;
            zero_rank_log << " d" << j << "_max: " << dims_to_search [j].max;               
        }
		zero_rank_log << " note: all_timestep_entries.size(): " << all_timestep_entries.size() << endl;
		print_timestep_catalog (all_timestep_entries.size(), all_timestep_entries);
	}
    add_timing_point(CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_DIMS_ABOVE_MAX_COLLECTIVE_DONE);

	all_timestep_entries.clear();

    MPI_Barrier(MPI_COMM_WORLD);
    add_timing_point(CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_DIMS_BELOW_MIN_START);
	if (rank < num_servers) {
	    rc = metadata_catalog_all_timesteps_with_var_attributes_with_type_var_dims_below_min (server, run_id, value_type_id, var_id, txn_id, num_dims, dims_to_search, 
	        ATTR_DATA_TYPE_REAL, below_min_data, count, timestep_entries);

	    if (rc == RC_OK) {
	    	// if(testing_logging || (zero_rank_logging && rank == 0)) {
		    //     zero_rank_log << "using metadata_catalog_all_timesteps_with_var_attributes_with_type_var_dims funct: timesteps associated with run_id " << run_id << ", type_id " << type_id << 
		    //     ", var_id " << var_id << " and txn_id: " << txn_id << " int attr data less than or eq to " << below_min_val << " overlapping with dims";
		    //     for(int j=0; j< num_dims; j++) {
		    //         zero_rank_log << " d" << j << "_min: " << dims_to_search [j].min;
		    //         zero_rank_log << " d" << j << "_max: " << dims_to_search [j].max;               
		    //     }
		    //     zero_rank_log << " for server: " << rank << " \n";
	     //        print_timestep_catalog (count, timestep_entries);
	     //    }
	    }
	    else {
	        error_log << "Error getting the matching timestep catalog. Proceeding \n";
	    }
	}
    add_timing_point(CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_DIMS_BELOW_MIN_DONE);

    MPI_Barrier(MPI_COMM_WORLD); //leave to make timing clearer
    add_timing_point(CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_DIMS_BELOW_MIN_COLLECTIVE_START);
	gather_timestep_entries(timestep_entries, rank, num_servers, num_client_procs, all_timestep_entries);
	if(testing_logging || (zero_rank_logging  && rank == 0)) {
		zero_rank_log << "rank: " << rank << " all timesteps associated with a var attribute for run_id " << run_id << 
			", type_id " << value_type_id << ", var_id " << var_id << " int attr data less than or eq to " << below_min_val <<
			" overlapping with dims";
        for(int j=0; j< num_dims; j++) {
            zero_rank_log << " d" << j << "_min: " << dims_to_search [j].min;
            zero_rank_log << " d" << j << "_max: " << dims_to_search [j].max;               
        }
		zero_rank_log << " note: all_timestep_entries.size(): " << all_timestep_entries.size() << endl;
		print_timestep_catalog (all_timestep_entries.size(), all_timestep_entries);
	}
    add_timing_point(CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_DIMS_BELOW_MIN_COLLECTIVE_DONE);

    return rc;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//RUN ATTR FUNCTS //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int catalog_run_attributes_with_type_or_val_functs ( const md_server &server, int rank, uint32_t num_servers, uint32_t num_client_procs,
							uint64_t run_id, uint64_t type_id, uint64_t txn_id,
                            double min_range, double max_range, double above_max_val, double below_min_val
							) 
{

    int rc = RC_OK;
    uint32_t count;
    vector<md_catalog_run_attribute_entry> run_attr_entries;

    // rc = metadata_catalog_all_run_attributes (server, run_id, txn_id, count, run_attr_entries);

    // if (rc == RC_OK) {
    //     if(testing_logging || (zero_rank_logging && rank == 0)) {
	    //     zero_rank_log << "using metadata_catalog_all_run_attributes funct: run var_attr_entries associated with run_id " << run_id << 
	    //         " and txn_id: " << txn_id << " for server: " << rank << " \n";    
    //         print_run_attribute_list (count, run_attr_entries);
    //     }
    //     print_run_attr_data(count, run_attr_entries);
    // }
    // else {
    //     error_log << "Error getting the matching run attr catalog. Proceeding \n";
    // }
   	vector<md_catalog_run_attribute_entry> all_run_attr_entries;

  	MPI_Barrier(MPI_COMM_WORLD);
    add_timing_point(CATALOG_ALL_RUN_ATTRIBUTES_WITH_TYPE_START);
    if(rank < num_servers) {
	    rc = metadata_catalog_all_run_attributes_with_type (server, run_id, type_id, txn_id, count, run_attr_entries);

	    if (rc == RC_OK) {
	   	    // if(testing_logging || (zero_rank_logging && rank == 0)) {
		       //  zero_rank_log << "using metadata_catalog_all_run_attributes_with_type funct: attr_entries associated with run_id " << run_id << 
		       //      " type_id " << type_id << " and txn_id: " << txn_id << " for server: " << rank << " \n";
	        //     print_run_attribute_list (count, run_attr_entries);
	        //     print_run_attr_data(count, run_attr_entries);
	        // }
	    }
	    else {
	        error_log << "Error getting the matching run attr catalog. Proceeding \n";
	    }
	}
    add_timing_point(CATALOG_ALL_RUN_ATTRIBUTES_WITH_TYPE_DONE);

  	MPI_Barrier(MPI_COMM_WORLD); //leave to make timing clearer
    add_timing_point(CATALOG_ALL_RUN_ATTRIBUTES_WITH_TYPE_COLLECTIVE_START);
    gather_attr_entries(run_attr_entries, count, rank, num_servers, num_client_procs, all_run_attr_entries);
    if(testing_logging || (zero_rank_logging  && rank == 0)) {
        zero_rank_log << "rank: " << rank << " all attr_entries associated with run_id " << run_id << 
            " type_id " << type_id << " note: all_run_attr_entries.size(): " << all_run_attr_entries.size() << endl;
    	print_run_attribute_list (all_run_attr_entries.size(), all_run_attr_entries);
    	print_run_attr_data(all_run_attr_entries.size(), all_run_attr_entries);
    }
    add_timing_point(CATALOG_ALL_RUN_ATTRIBUTES_WITH_TYPE_COLLECTIVE_DONE);

    string range_data;
    make_range_data (min_range, max_range, range_data);

	all_run_attr_entries.clear();

  	MPI_Barrier(MPI_COMM_WORLD);
	add_timing_point(CATALOG_ALL_RUN_ATTRIBUTES_WITH_TYPE_RANGE_START);
    if(rank < num_servers) {    
	    rc = metadata_catalog_all_run_attributes_with_type_range (server, run_id, type_id, txn_id, ATTR_DATA_TYPE_REAL, range_data, count, run_attr_entries);

	    if (rc == RC_OK) {
	        // if(testing_logging || (zero_rank_logging && rank == 0)) {
		       //  zero_rank_log << "using metadata_catalog_all_run_attributes_with_type funct: run var_attr_entries associated with run_id " << run_id << 
		       //      " type_id " << type_id << " and txn_id: " << txn_id << " int attr data and range [" << min_range << "," << max_range << "]" <<
		       //      " for server: " << rank << " \n";
	        //     print_run_attribute_list (count, run_attr_entries);
	        //     print_run_attr_data(count, run_attr_entries);
	        // }
	    }
	    else {
	        error_log << "Error getting the matching run attr catalog. Proceeding \n";
	    }
	}
	add_timing_point(CATALOG_ALL_RUN_ATTRIBUTES_WITH_TYPE_RANGE_DONE);

  	MPI_Barrier(MPI_COMM_WORLD); //leave to make timing clearer
	add_timing_point(CATALOG_ALL_RUN_ATTRIBUTES_WITH_TYPE_RANGE_COLLECTIVE_START);
    gather_attr_entries(run_attr_entries, count, rank, num_servers, num_client_procs, all_run_attr_entries);
    if(testing_logging || (zero_rank_logging  && rank == 0)) {
  		zero_rank_log << "rank: " << rank << " all run attributes associated with run_id " << run_id << 
  			" int attr data and range [" << min_range << "," << max_range << "]" <<
  			" note: all_run_attr_entries.size(): " << all_run_attr_entries.size() << endl;
    	print_run_attribute_list (all_run_attr_entries.size(), all_run_attr_entries);
    	print_run_attr_data(all_run_attr_entries.size(), all_run_attr_entries);
    }
	add_timing_point(CATALOG_ALL_RUN_ATTRIBUTES_WITH_TYPE_RANGE_COLLECTIVE_DONE);

    string above_max_data;
    make_single_val_data (above_max_val, above_max_data);

	all_run_attr_entries.clear();

  	MPI_Barrier(MPI_COMM_WORLD);
	add_timing_point(CATALOG_ALL_RUN_ATTRIBUTES_WITH_TYPE_ABOVE_MAX_START);	
    if(rank < num_servers) {    
	    rc = metadata_catalog_all_run_attributes_with_type_above_max (server, run_id, type_id, txn_id, ATTR_DATA_TYPE_REAL, above_max_data, count, run_attr_entries);

	    if (rc == RC_OK) {
        	// if(testing_logging || (zero_rank_logging && rank == 0)) {
		       //  zero_rank_log << "using metadata_catalog_all_run_attributes_with_type funct: run var_attr_entries associated with run_id " << run_id << 
		       //      " type_id " << type_id << " and txn_id: " << txn_id << " int attr data and above_max_val geq " << above_max_val << "] for server: " << rank << " \n";
	        //     print_run_attribute_list (count, run_attr_entries);
	        //     print_run_attr_data(count, run_attr_entries);
	        // }
	    }
	    else {
	        error_log << "Error getting the matching run attr catalog. Proceeding \n";
	    }
	}
	add_timing_point(CATALOG_ALL_RUN_ATTRIBUTES_WITH_TYPE_ABOVE_MAX_DONE);	

  	MPI_Barrier(MPI_COMM_WORLD); //leave to make timing clearer
	add_timing_point(CATALOG_ALL_RUN_ATTRIBUTES_WITH_TYPE_ABOVE_MAX_COLLECTIVE_START);	
    gather_attr_entries(run_attr_entries, count, rank, num_servers, num_client_procs, all_run_attr_entries);
    if(testing_logging || (zero_rank_logging  && rank == 0)) {
  		zero_rank_log << "rank: " << rank << " all run attributes associated with run_id " << run_id << 
  			" int attr data and above_max_val geq " << above_max_val << "]" <<
  			" note: all_run_attr_entries.size(): " << all_run_attr_entries.size() << endl;
    	print_run_attribute_list (all_run_attr_entries.size(), all_run_attr_entries);
    	print_run_attr_data(all_run_attr_entries.size(), all_run_attr_entries);
    }
	add_timing_point(CATALOG_ALL_RUN_ATTRIBUTES_WITH_TYPE_ABOVE_MAX_COLLECTIVE_DONE);	

    string below_min_data;
    make_single_val_data (below_min_val, below_min_data);

	all_run_attr_entries.clear();

  	MPI_Barrier(MPI_COMM_WORLD);
	add_timing_point(CATALOG_ALL_RUN_ATTRIBUTES_WITH_TYPE_BELOW_MIN_START);	
    if(rank < num_servers) {    
	    rc = metadata_catalog_all_run_attributes_with_type_below_min (server, run_id, type_id, txn_id, ATTR_DATA_TYPE_REAL, below_min_data, count, run_attr_entries);

	    if (rc == RC_OK) {
	        // if(testing_logging || (zero_rank_logging && rank == 0)) {
		       //  zero_rank_log << "using metadata_catalog_all_run_attributes_with_type funct: run var_attr_entries associated with run_id " << run_id << 
		       //      " type_id " << type_id << " and txn_id: " << txn_id << " int attr data and below_min_val leq " << below_min_val << "]" <<
		       //      " for server: " << rank << " \n";
	        //     print_run_attribute_list (count, run_attr_entries);
	        //     print_run_attr_data(count, run_attr_entries);
	        // }
	    }
	    else {
	        error_log << "Error getting the matching run attr catalog. Proceeding \n";
	    }
	}
	add_timing_point(CATALOG_ALL_RUN_ATTRIBUTES_WITH_TYPE_BELOW_MIN_DONE);	

  	MPI_Barrier(MPI_COMM_WORLD); //leave to make timing clearer
	add_timing_point(CATALOG_ALL_RUN_ATTRIBUTES_WITH_TYPE_BELOW_MIN_COLLECTIVE_START);	
    gather_attr_entries(run_attr_entries, count, rank, num_servers, num_client_procs, all_run_attr_entries);
    if(testing_logging || (zero_rank_logging  && rank == 0)) {
  		zero_rank_log << "rank: " << rank << " all run attributes associated with run_id " << run_id << 
  			" int attr data and below_min_val leq " << below_min_val << "]" << 
  			" note: all_run_attr_entries.size(): " << all_run_attr_entries.size() << endl;
    	print_run_attribute_list (all_run_attr_entries.size(), all_run_attr_entries);
    	print_run_attr_data(all_run_attr_entries.size(), all_run_attr_entries);
    }
	add_timing_point(CATALOG_ALL_RUN_ATTRIBUTES_WITH_TYPE_BELOW_MIN_COLLECTIVE_DONE);	

    return rc;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//TIMESTEP ATTR FUNCTS /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int catalog_timestep_attributes_with_type_or_val_functs ( const md_server &server, int rank, uint32_t num_servers, uint32_t num_client_procs,
							uint64_t run_id, uint64_t timestep_id, uint64_t type_id, uint64_t txn_id,
                            double min_range, double max_range, double above_max_val, double below_min_val
							) 
{

    int rc = RC_OK;
    uint32_t count;
    vector<md_catalog_timestep_attribute_entry> timestep_attr_entries;

    // rc = metadata_catalog_all_timestep_attributes (server, run_id, timestep_id, txn_id, count, timestep_attr_entries);

    // if (rc == RC_OK) {
    //     if(testing_logging || (zero_rank_logging && rank == 0)) {
	    //     zero_rank_log << "using metadata_catalog_all_timestep_attributes funct: timestep var_attr_entries associated with run_id " << run_id << 
	    //     		" timestep_id " << timestep_id << " and txn_id: " << txn_id << " for server: " << rank << " \n";    
    //         print_timestep_attribute_list (count, timestep_attr_entries);
    //     }
    //     print_timestep_attr_data(count, timestep_attr_entries);
    // }
    // else {
    //     error_log << "Error getting the matching timestep attr catalog. Proceeding \n";
    // }
   	vector<md_catalog_timestep_attribute_entry> all_timestep_attr_entries;

    MPI_Barrier(MPI_COMM_WORLD);
	add_timing_point(CATALOG_ALL_TIMESTEP_ATTRIBUTES_WITH_TYPE_START);	
   	if(rank < num_servers) {
	    rc = metadata_catalog_all_timestep_attributes_with_type (server, run_id, timestep_id, type_id, txn_id, count, timestep_attr_entries);

	    if (rc == RC_OK) {
	   	    // if(testing_logging || (zero_rank_logging && rank == 0)) {
		       //  zero_rank_log << "using metadata_catalog_all_timestep_attributes_with_type funct: timestep var_attr_entries associated with run_id " << run_id << 
		       //  " timestep_id " << timestep_id << " type_id " << type_id << " and txn_id: " << txn_id << " for server: " << rank << " \n";
	        //     print_timestep_attribute_list (count, timestep_attr_entries);
	        // 	print_timestep_attr_data(count, timestep_attr_entries);            
	        // }
	    }
	    else {
	        error_log << "Error getting the matching timestep attr catalog. Proceeding \n";
	    }
	}
	add_timing_point(CATALOG_ALL_TIMESTEP_ATTRIBUTES_WITH_TYPE_DONE);	

    MPI_Barrier(MPI_COMM_WORLD); //leave to make timing clearer
	add_timing_point(CATALOG_ALL_TIMESTEP_ATTRIBUTES_WITH_TYPE_COLLECTIVE_START);	
    gather_attr_entries(timestep_attr_entries, count, rank, num_servers, num_client_procs, all_timestep_attr_entries);
    if(testing_logging || (zero_rank_logging  && rank == 0)) {
  		zero_rank_log << "rank: " << rank << " all timestep attributes associated with run_id " << run_id << 
  			" and timestep_id " << timestep_id << " type_id " << type_id << 
  			" note: all_timestep_attr_entries.size(): " << all_timestep_attr_entries.size() << endl;
    	print_timestep_attribute_list (all_timestep_attr_entries.size(), all_timestep_attr_entries);
    	print_timestep_attr_data(all_timestep_attr_entries.size(), all_timestep_attr_entries);
    }		
	add_timing_point(CATALOG_ALL_TIMESTEP_ATTRIBUTES_WITH_TYPE_COLLECTIVE_DONE);	

    string range_data;
    make_range_data (min_range, max_range, range_data);

	all_timestep_attr_entries.clear();

    MPI_Barrier(MPI_COMM_WORLD);
	add_timing_point(CATALOG_ALL_TIMESTEP_ATTRIBUTES_WITH_TYPE_RANGE_START);	
	if(rank < num_servers) {
	    rc = metadata_catalog_all_timestep_attributes_with_type_range (server, run_id, timestep_id, type_id, txn_id, ATTR_DATA_TYPE_REAL, range_data, count, timestep_attr_entries);
	    if (rc == RC_OK) {
	        // if(testing_logging || (zero_rank_logging && rank == 0)) {
		       //  zero_rank_log << "using metadata_catalog_all_timestep_attributes_with_type funct: timestep var_attr_entries associated with run_id " << run_id << 
		       //  	" timestep_id " << timestep_id << " type_id " << type_id << " and txn_id: " << txn_id << " int attr data and range [" << 
		       //  	min_range << "," << max_range << "] for server: " << rank << " \n";
	        //     print_timestep_attribute_list (count, timestep_attr_entries);
	        // 	print_timestep_attr_data(count, timestep_attr_entries);            
	        // }
	    }
	    else {
	        error_log << "Error getting the matching timestep attr catalog. Proceeding \n";
	    }
	}
	add_timing_point(CATALOG_ALL_TIMESTEP_ATTRIBUTES_WITH_TYPE_RANGE_DONE);	

    MPI_Barrier(MPI_COMM_WORLD); //leave to make timing clearer
	add_timing_point(CATALOG_ALL_TIMESTEP_ATTRIBUTES_WITH_TYPE_RANGE_COLLECTIVE_START);	
    gather_attr_entries(timestep_attr_entries, count, rank, num_servers, num_client_procs, all_timestep_attr_entries);
    if(testing_logging || (zero_rank_logging  && rank == 0)) {
  		zero_rank_log << "rank: " << rank << " all timestep attributes associated with run_id " << run_id << 
  			" and timestep_id " << timestep_id << " type_id " << type_id << 
  			" int attr data and range [" << min_range << "," << max_range << "]" << 
  			" note: all_timestep_attr_entries.size(): " << all_timestep_attr_entries.size() << endl;
    	print_timestep_attribute_list (all_timestep_attr_entries.size(), all_timestep_attr_entries);
    	print_timestep_attr_data(all_timestep_attr_entries.size(), all_timestep_attr_entries);
    }	
	add_timing_point(CATALOG_ALL_TIMESTEP_ATTRIBUTES_WITH_TYPE_RANGE_COLLECTIVE_DONE);	

    string above_max_data;
    make_single_val_data (above_max_val, above_max_data);

	all_timestep_attr_entries.clear();

    MPI_Barrier(MPI_COMM_WORLD);
	add_timing_point(CATALOG_ALL_TIMESTEP_ATTRIBUTES_WITH_TYPE_ABOVE_MAX_START);	
	if(rank < num_servers) {
	    rc = metadata_catalog_all_timestep_attributes_with_type_above_max (server, run_id, timestep_id, type_id, txn_id, ATTR_DATA_TYPE_REAL, above_max_data, count, timestep_attr_entries);
	    if (rc == RC_OK) {
   	     //    if(testing_logging || (zero_rank_logging && rank == 0)) {
		       //  zero_rank_log << "using metadata_catalog_all_timestep_attributes_with_type funct: timestep var_attr_entries associated with run_id " << run_id << 
		       //  " timestep_id " << timestep_id << " type_id " << type_id << " and txn_id: " << txn_id << " int attr data greater than or eq to " << 
		       //  above_max_val << " for server: " << rank << " \n";
	        //     print_timestep_attribute_list (count, timestep_attr_entries);
	        // 	print_timestep_attr_data(count, timestep_attr_entries);            
	        // }
	    }
	    else {
	        error_log << "Error getting the matching timestep attr catalog. Proceeding \n";
	    }
	}
	add_timing_point(CATALOG_ALL_TIMESTEP_ATTRIBUTES_WITH_TYPE_ABOVE_MAX_DONE);	

    MPI_Barrier(MPI_COMM_WORLD); //leave to make timing clearer
	add_timing_point(CATALOG_ALL_TIMESTEP_ATTRIBUTES_WITH_TYPE_ABOVE_MAX_COLLECTIVE_START);	
    gather_attr_entries(timestep_attr_entries, count, rank, num_servers, num_client_procs, all_timestep_attr_entries);
    if(testing_logging || (zero_rank_logging  && rank == 0)) {
  		zero_rank_log << "rank: " << rank << " all timestep attributes associated with run_id " << run_id << 
  			" and timestep_id " << timestep_id << " type_id " << type_id << 
  			" int attr data greater than or eq to " << above_max_val <<
  			" note: all_timestep_attr_entries.size(): " << all_timestep_attr_entries.size() << endl;
    	print_timestep_attribute_list (all_timestep_attr_entries.size(), all_timestep_attr_entries);
    	print_timestep_attr_data(all_timestep_attr_entries.size(), all_timestep_attr_entries);
    }	
	add_timing_point(CATALOG_ALL_TIMESTEP_ATTRIBUTES_WITH_TYPE_ABOVE_MAX_COLLECTIVE_DONE);	

    string below_min_data;
    make_single_val_data (below_min_val, below_min_data);

	all_timestep_attr_entries.clear();

    MPI_Barrier(MPI_COMM_WORLD);
	add_timing_point(CATALOG_ALL_TIMESTEP_ATTRIBUTES_WITH_TYPE_BELOW_MIN_START);	
	if(rank < num_servers) {
	    rc = metadata_catalog_all_timestep_attributes_with_type_below_min (server, run_id, timestep_id, type_id, txn_id, ATTR_DATA_TYPE_REAL, below_min_data, count, timestep_attr_entries);
	    if (rc == RC_OK) {
	   	    // if(testing_logging || (zero_rank_logging && rank == 0)) {
		       //  zero_rank_log << "using metadata_catalog_all_timestep_attributes_with_type funct: timestep var_attr_entries associated with run_id " << run_id << 
		       //  " timestep_id " << timestep_id << " type_id " << type_id << " and txn_id: " << txn_id << " int attr data less than or eq to " << 
		       //  below_min_val << " for server: " << rank << " \n";
	        //     print_timestep_attribute_list (count, timestep_attr_entries);
	        // 	print_timestep_attr_data(count, timestep_attr_entries);            
	        // }
	    }
	    else {
	        error_log << "Error getting the matching timestep attr catalog. Proceeding \n";
	    }
	}
	add_timing_point(CATALOG_ALL_TIMESTEP_ATTRIBUTES_WITH_TYPE_BELOW_MIN_DONE);	

    MPI_Barrier(MPI_COMM_WORLD); //leave to make timing clearer
	add_timing_point(CATALOG_ALL_TIMESTEP_ATTRIBUTES_WITH_TYPE_BELOW_MIN_COLLECTIVE_START);	
    gather_attr_entries(timestep_attr_entries, count, rank, num_servers, num_client_procs, all_timestep_attr_entries);
    if(testing_logging || (zero_rank_logging  && rank == 0)) {
  		zero_rank_log << "rank: " << rank << " all timestep attributes associated with run_id " << run_id << 
  			" and timestep_id " << timestep_id << " type_id " << type_id << 
  			" int attr data less than or eq to " << below_min_val <<
  			" note: all_timestep_attr_entries.size(): " << all_timestep_attr_entries.size() << endl;
    	print_timestep_attribute_list (all_timestep_attr_entries.size(), all_timestep_attr_entries);
    	print_timestep_attr_data(all_timestep_attr_entries.size(), all_timestep_attr_entries);
    }	
	add_timing_point(CATALOG_ALL_TIMESTEP_ATTRIBUTES_WITH_TYPE_BELOW_MIN_COLLECTIVE_DONE);	

    return rc;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//VAR ATTR FUNCTS ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int catalog_all_var_attributes_with_dims ( const md_server &server, int rank, uint32_t num_servers, uint32_t num_client_procs,
							const md_catalog_run_entry &run, const vector<md_catalog_var_entry> &vars, 
							uint64_t run_id, uint64_t timestep_id, 
							uint64_t txn_id, int num_dims, const vector<md_dim_bounds> &dims_to_search,
							const vector<md_dim_bounds> &proc_dims
							) 
{
    int rc = RC_OK;
    uint32_t count;
    vector<md_catalog_var_attribute_entry> var_attr_entries;

	vector<md_catalog_var_attribute_entry> all_var_attr_entries;

	MPI_Barrier(MPI_COMM_WORLD);
	add_timing_point(CATALOG_ALL_VAR_ATTRIBUTES_WITH_DIMS_START);	
	if(rank < num_servers) {
	    rc = metadata_catalog_all_var_attributes_with_dims(server, run_id, timestep_id, txn_id, num_dims, dims_to_search, count, var_attr_entries);

	    if (rc == RC_OK) {
	  	    // if(testing_logging || (zero_rank_logging && rank == 0)) {
		      //   zero_rank_log << "using var var_attr_entries dims funct: attributes associated with run_id " << run_id << ", timestep_id " << 
		      //   	timestep_id << " and txn_id: " << txn_id const << " overlapping with &dims_to_search";
		      //   for(int j=0; j< num_dims; j++) {
		      //       zero_rank_log << " d" << j << "_min: " << dims_to_search [j].min;
		      //       zero_rank_log << " d" << j << "_max: " << dims_to_search [j].max;               
		      //   }
		      //   zero_rank_log << " for server: " << rank << " \n";
	       //      print_var_attribute_list (count, var_attr_entries);
	       //  	print_var_attr_data(count, var_attr_entries);         
	       //  }
	    }
	    else {
	        error_log << "Error getting the matching attribute list. Proceeding \n";
	    }
	}
	add_timing_point(CATALOG_ALL_VAR_ATTRIBUTES_WITH_DIMS_DONE);	

	MPI_Barrier(MPI_COMM_WORLD); //leave to make timing clearer
	add_timing_point(CATALOG_ALL_VAR_ATTRIBUTES_WITH_DIMS_COLLECTIVE_START);	
	gather_attr_entries(var_attr_entries, count, rank, num_servers, num_client_procs, all_var_attr_entries);
	if(testing_logging || (zero_rank_logging  && rank == 0)) {
		zero_rank_log << "rank: " << rank << " all var attributes associated with run_id " << run_id << 
			" and timestep_id " << timestep_id << " overlapping with dims";
        for(int j=0; j< num_dims; j++) {
            zero_rank_log << " d" << j << "_min: " << dims_to_search [j].min << " d" << j << "_max: " << dims_to_search [j].max;              
        } 
        zero_rank_log << " note: all_var_attr_entries.size(): " << all_var_attr_entries.size() << endl;
		print_var_attribute_list (all_var_attr_entries.size(), all_var_attr_entries);
		print_var_attr_data(all_var_attr_entries.size(), all_var_attr_entries);
	}	
	// note - have commented this out since it results in too many objects
    // if (output_objector_params) {
   	//     count = 0;
    //     for (md_catalog_var_attribute_entry attr : all_var_attr_entries) {
    // 		objector_params my_objector_params = get_objector_params(run, vars.at(attr.var_id), attr.dims);
    //         if(my_objector_params.var_id !=attr.var_id) {
    //         	error_log << "attrs var_id: " << attr.var_id << " but given var's id: " << my_objector_params.var_id;
    //         	return RC_ERR;
    //         }
    //         else {
    //         	if (dims_overlap ( attr.dims, proc_dims) ) {
    //             	all_objector_params.push_back(my_objector_params);
    //             	count += 1;
    //             }            	
    //         }
    //     }
    //  	add_objector_point(BOUNDING_BOX_TO_OBJ_NAMES, count);
    // }

	add_timing_point(CATALOG_ALL_VAR_ATTRIBUTES_WITH_DIMS_COLLECTIVE_DONE);	

    // rc = retrieveObjNamesAndDataForAttrCatalog(data_outputs, server, new_run, var.run_id, var.timestep_id, txn_id, var_attr_entries );
    // if (rc != RC_OK) {
    //     error_log << "error. could not perform retrieveObjNamesAndDataForAttrCatalog \n";
    // }

    return rc;
}

int catalog_var_attributes_with_var_functs ( const md_server &server, int rank, uint32_t num_servers, uint32_t num_client_procs,
								const md_catalog_run_entry &run, const md_catalog_var_entry &var, 
								int num_dims, const vector<md_dim_bounds> &dims_to_search,
								const vector<md_dim_bounds> &proc_dims, uint64_t txn_id 								
								) 
{

    int rc = RC_OK;
    uint32_t count;
    vector<md_catalog_var_attribute_entry> var_attr_entries;

	vector<md_catalog_var_attribute_entry> all_var_attr_entries;

	MPI_Barrier(MPI_COMM_WORLD);
	add_timing_point(CATALOG_ALL_VAR_ATTRIBUTES_WITH_VAR_BY_NAME_VER_START);	
	if(rank < num_servers) {
	    rc = metadata_catalog_all_var_attributes_with_var_by_name_ver (server, var.run_id, var.timestep_id, var.name, var.version, txn_id, count, var_attr_entries);

	    if (rc == RC_OK) {
   	     //    if(testing_logging || (zero_rank_logging && rank == 0)) {
		       //  zero_rank_log << "using var var_attr_entries by var name ver funct: attributes associated with run_id " << var.run_id << ", timestep_id " << 
		       //      var.timestep_id << " and txn_id: " << txn_id << " and var " << var.name << " ver " << var.version;
		       //  zero_rank_log << " for server: " << rank << " \n";
	        //     print_var_attribute_list (count, var_attr_entries);
	        // 	print_var_attr_data(count, var_attr_entries);         
	        // }
	    }
	    else {
	        error_log << "Error getting the matching attribute list. Proceeding \n";
	    }
	}
	add_timing_point(CATALOG_ALL_VAR_ATTRIBUTES_WITH_VAR_BY_NAME_VER_DONE);	

	MPI_Barrier(MPI_COMM_WORLD); //leave to make timing clearer
	add_timing_point(CATALOG_ALL_VAR_ATTRIBUTES_WITH_VAR_BY_NAME_VER_COLLECTIVE_START);	
	gather_attr_entries(var_attr_entries, count, rank, num_servers, num_client_procs, all_var_attr_entries);
	if(testing_logging || (zero_rank_logging  && rank == 0)) {
			zero_rank_log << "rank: " << rank << " all var attributes associated with run_id " << var.run_id << " and timestep_id " << var.timestep_id << 
				" and var_id: " << var.var_id << " note: all_var_attr_entries.size(): " << all_var_attr_entries.size() << endl;
		print_var_attribute_list (all_var_attr_entries.size(), all_var_attr_entries);
		print_var_attr_data(all_var_attr_entries.size(), all_var_attr_entries);
	}	
	//note - have commented this out since it results in too many objects
    // if (output_objector_params) {
    // 	count = 0;
    //     for (md_catalog_var_attribute_entry attr : all_var_attr_entries) {
    //     	if (dims_overlap ( attr.dims, proc_dims) ) {
    //     		all_objector_params.push_back( get_objector_params(run, var, attr.dims) );
    //     		count += 1;
    //     	}
    //     }
    //  	add_objector_point(BOUNDING_BOX_TO_OBJ_NAMES, count);
    // }	
	add_timing_point(CATALOG_ALL_VAR_ATTRIBUTES_WITH_VAR_BY_NAME_VER_COLLECTIVE_DONE);	

    // rc = retrieveObjNamesAndDataForAttrCatalog(data_outputs, server, new_run, var.run_id, var.timestep_id, txn_id, var_attr_entries );
    // if (rc != RC_OK) {
    //     error_log << "error. could not perform retrieveObjNamesAndDataForAttrCatalog \n";
    // }

	all_var_attr_entries.clear();

	MPI_Barrier(MPI_COMM_WORLD);
	add_timing_point(CATALOG_ALL_VAR_ATTRIBUTES_WITH_VAR_DIMS_BY_NAME_VER_START);	
	if(rank < num_servers) {
	    rc = metadata_catalog_all_var_attributes_with_var_dims_by_name_ver (server, var.run_id, var.timestep_id, var.name, 
	    		var.version, txn_id, num_dims, dims_to_search, count, var_attr_entries);

	    if (rc == RC_OK) {
	        // if(testing_logging || (zero_rank_logging && rank == 0)) {
		       //  zero_rank_log << "using var var_attr_entries by var dims name ver funct: attributes associated with run_id " << var.run_id << ", timestep_id " << 
		       //      var.timestep_id << " and txn_id: " << txn_id << " and var " << var.name << " ver " << var.version << 
		       //      " overlapping with dims";
		       //  for(int j=0; j< num_dims; j++) {
		       //      zero_rank_log << " d" << j << "_min: " << dims_to_search [j].min;
		       //      zero_rank_log << " d" << j << "_max: " << dims_to_search [j].max;               
		       //  }
		       //  zero_rank_log << " for server: " << rank << " \n";
	        //     print_var_attribute_list (count, var_attr_entries);
	        // 	print_var_attr_data(count, var_attr_entries);         
	        // }
	    }
	    else {
	        error_log << "Error getting the matching attribute list. Proceeding \n";
	    }
	}
	add_timing_point(CATALOG_ALL_VAR_ATTRIBUTES_WITH_VAR_DIMS_BY_NAME_VER_DONE);	

	MPI_Barrier(MPI_COMM_WORLD); //leave to make timing clearer
	add_timing_point(CATALOG_ALL_VAR_ATTRIBUTES_WITH_VAR_DIMS_BY_NAME_VER_COLLECTIVE_START);	
	gather_attr_entries(var_attr_entries, count, rank, num_servers, num_client_procs, all_var_attr_entries);
	if(testing_logging || (zero_rank_logging  && rank == 0)) {
		zero_rank_log << "rank: " << rank << " all var attributes associated with run_id " << var.run_id << " and timestep_id " << var.timestep_id << 
			" and var_id: " << var.var_id << " overlapping with dims";
        for(int j=0; j< num_dims; j++) {
            zero_rank_log << " d" << j << "_min: " << dims_to_search [j].min;
            zero_rank_log << " d" << j << "_max: " << dims_to_search [j].max;               
        }			
		zero_rank_log << " note: all_var_attr_entries.size(): " << all_var_attr_entries.size() << endl;
		print_var_attribute_list (all_var_attr_entries.size(), all_var_attr_entries);
		print_var_attr_data(all_var_attr_entries.size(), all_var_attr_entries);
	}	
	//note - have commented this out since it results in too many objects
    // if (output_objector_params) {
    // 	count = 0;
    //     for (md_catalog_var_attribute_entry attr : all_var_attr_entries) {
    // 		if (dims_overlap ( attr.dims, proc_dims) ) {
    //     		all_objector_params.push_back( get_objector_params(run, var, attr.dims) );
    //     		count += 1;
    //     	}
    //     }
    //  	add_objector_point(BOUNDING_BOX_TO_OBJ_NAMES, count);

    // }		
	add_timing_point(CATALOG_ALL_VAR_ATTRIBUTES_WITH_VAR_DIMS_BY_NAME_VER_COLLECTIVE_DONE);	

    // rc = retrieveObjNamesAndDataForAttrCatalog(data_outputs, server, new_run, var.run_id, var.timestep_id, txn_id, var_attr_entries );
    // if (rc != RC_OK) {
    //     error_log << "error. could not perform retrieveObjNamesAndDataForAttrCatalog \n";
    // }

    return rc;
}

int catalog_var_attributes_with_type_functs ( const md_server &server, int rank, uint32_t num_servers, uint32_t num_client_procs,
									const md_catalog_run_entry &run, const vector<md_catalog_var_entry> &vars,
									uint64_t timestep_id, const md_catalog_type_entry &type,
									const vector<md_dim_bounds> &proc_dims, uint64_t txn_id 								
									) 
{

	int rc = RC_OK;

	uint32_t count;
    vector<md_catalog_var_attribute_entry> var_attr_entries;
	vector<md_catalog_var_attribute_entry> all_var_attr_entries;

	MPI_Barrier(MPI_COMM_WORLD);
	add_timing_point(CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_BY_NAME_VER_START);	
	if(rank < num_servers) {
	    rc = metadata_catalog_all_var_attributes_with_type_by_name_ver (server, type.run_id, timestep_id, type.name, type.version, type.txn_id, count, var_attr_entries);

	    if (rc == RC_OK) {
	   	    // if(testing_logging || (zero_rank_logging && rank == 0)) {
		       //  zero_rank_log << "using var var_attr_entries type name ver funct: attributes associated with run_id " << type.run_id << ", timestep_id " << 
		       //      timestep_id << " and txn_id: " << type.txn_id << " and type " << type.name << " ver " << type.version;
		       //  zero_rank_log << " for server: " << rank << " \n";
	        //     print_var_attribute_list (count, var_attr_entries);
	        // 	print_var_attr_data(count, var_attr_entries);         
	        // }
	    }
	    else {
	        error_log << "Error getting the matching attribute list. Proceeding \n";
	    }
	}
	add_timing_point(CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_BY_NAME_VER_DONE);	

	MPI_Barrier(MPI_COMM_WORLD); //leave to make timing clearer
	add_timing_point(CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_BY_NAME_VER_COLLECTIVE_START);	
	gather_attr_entries(var_attr_entries, count, rank, num_servers, num_client_procs, all_var_attr_entries);
	if(testing_logging || (zero_rank_logging  && rank == 0)) {
			zero_rank_log << "rank: " << rank << " all var attributes associated with run_id " << type.run_id << " and timestep_id " << timestep_id << 
				" and type_id: " << type.type_id << " note: all_var_attr_entries.size(): " << all_var_attr_entries.size() << endl;
		print_var_attribute_list (all_var_attr_entries.size(), all_var_attr_entries);
		print_var_attr_data(all_var_attr_entries.size(), all_var_attr_entries);
	}	
    if (output_objector_params) {
    	count = 0;
        for (md_catalog_var_attribute_entry attr : all_var_attr_entries) {
        	if (dims_overlap ( attr.dims, proc_dims) ) {
        		objector_params my_objector_params = get_objector_params(run, vars.at(attr.var_id), attr.dims, CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_BY_NAME_VER, proc_dims);
        		extreme_debug_log << "found a bb for rank " << rank << endl;
            	all_objector_params.push_back(my_objector_params);
            	count += 1;
            }        
            // if(my_objector_params.var_id !=attr.var_id) {
            // 	error_log << "attrs var_id: " << attr.var_id << " but given var's id: " << my_objector_params.var_id;
            // 	return RC_ERR;
            // }
        }
		add_objector_point(BOUNDING_BOX_TO_OBJ_NAMES, count);
    }	
	else if (output_obj_names) {
	    for (md_catalog_var_attribute_entry attr : all_var_attr_entries) {
	   		if (dims_overlap ( attr.dims, proc_dims) ) {
	            rc = add_object_names_offests_and_counts(run, vars.at(attr.var_id), get_overlapping_dims_for_bounding_box(attr.dims, proc_dims), CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_BY_NAME_VER);  
	            if (rc != RC_OK) {
	                error_log << "error in add_object_names_offests_and_counts \n";
	                return rc;
	            }
	            // NOTE: THIS IS WHERE DATA WOULD BE READ
	       		extreme_debug_log << "for rank: " << rank << " found a matching bb" << endl;
	        }
	    }
	}
	else if (hdf5_read_data) {
	    for (md_catalog_var_attribute_entry attr : all_var_attr_entries) {
	   		if (dims_overlap ( attr.dims, proc_dims) ) {
	            rc = add_object_names_offests_and_counts(run, vars.at(attr.var_id), get_overlapping_dims_for_bounding_box(attr.dims, proc_dims), CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_BY_NAME_VER);  
	            if (rc != RC_OK) {
	                error_log << "error in add_object_names_offests_and_counts \n";
	                return rc;
	            }
	            // NOTE: THIS IS WHERE DATA WOULD BE READ
	       		extreme_debug_log << "for rank: " << rank << " found a matching bb" << endl;
	        }
	    }
		read_data_for_attrs(run, vars, all_var_attr_entries, proc_dims);

	}


	add_timing_point(CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_BY_NAME_VER_COLLECTIVE_DONE);	

    // rc = retrieveObjNamesAndDataForAttrCatalog(data_outputs, server, new_run, var.run_id, var.timestep_id, txn_id, var_attr_entries );
    // if (rc != RC_OK) {
    //     error_log << "error. could not perform retrieveObjNamesAndDataForAttrCatalog \n";
    // }

	return rc;
}

int catalog_var_attributes_with_type_dims_functs ( const md_server &server, int rank, uint32_t num_servers, uint32_t num_client_procs,
												const md_catalog_run_entry &run, const vector<md_catalog_var_entry> &vars,
												uint64_t timestep_id, int num_dims, 
                                                const vector<md_dim_bounds> &dims_to_search, const md_catalog_type_entry &type,
                                                const vector<md_dim_bounds> &proc_dims, uint64_t txn_id 
                                                ) 
{
    int rc = RC_OK;
    uint32_t count;
    vector<md_catalog_var_attribute_entry> var_attr_entries;
	vector<md_catalog_var_attribute_entry> all_var_attr_entries;

	rc = catalog_var_attributes_with_type_functs ( server, rank, num_servers, num_client_procs, run, vars, 
									timestep_id, type, proc_dims, txn_id );
	if (rc != RC_OK) {
	    error_log << "Error with catalog_var_attributes_with_type_functs. Proceeding \n";
	}
	

	MPI_Barrier(MPI_COMM_WORLD);
	add_timing_point(CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_DIMS_BY_NAME_VER_START);	
	if(rank < num_servers) {
	    rc = metadata_catalog_all_var_attributes_with_type_dims_by_name_ver (server, type.run_id, timestep_id, 
	    		type.name, type.version, type.txn_id, num_dims, dims_to_search, count, var_attr_entries);

	    if (rc == RC_OK) {
	 	    // if(testing_logging || (zero_rank_logging && rank == 0)) {
		     //    zero_rank_log << "using var var_attr_entries type dims name ver funct: attributes associated with run_id " << type.run_id << ", timestep_id " << 
		     //        timestep_id << " and txn_id: " << type.txn_id << " and type " << type.name << " ver " << type.version << 
		     //        " overlapping with dims";
		     //    for(int j=0; j< num_dims; j++) {
		     //        zero_rank_log << " d" << j << "_min: " << dims_to_search [j].min;
		     //        zero_rank_log << " d" << j << "_max: " << dims_to_search [j].max;               
		     //    }
		     //    zero_rank_log << " for server: " << rank << " \n";
	      //       print_var_attribute_list (count, var_attr_entries);
	      //   	print_var_attr_data(count, var_attr_entries);         
	      //   }
	    }
	    else {
	        error_log << "Error getting the matching attribute list. Proceeding \n";
	    }
	}
	add_timing_point(CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_DIMS_BY_NAME_VER_DONE);	

	MPI_Barrier(MPI_COMM_WORLD); //leave to make timing clearer
	add_timing_point(CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_DIMS_BY_NAME_VER_COLLECTIVE_START);	
	gather_attr_entries(var_attr_entries, count, rank, num_servers, num_client_procs, all_var_attr_entries);
	if(testing_logging || (zero_rank_logging  && rank == 0)) {
		zero_rank_log << "rank: " << rank << " all var attributes associated with run_id " << type.run_id << " and timestep_id " << timestep_id << 
			" and type_id: " << type.type_id << " overlapping with dims";
        for(int j=0; j< num_dims; j++) {
            zero_rank_log << " d" << j << "_min: " << dims_to_search [j].min;
            zero_rank_log << " d" << j << "_max: " << dims_to_search [j].max;               
        }
		zero_rank_log <<	" note: all_var_attr_entries.size(): " << all_var_attr_entries.size() << endl;
		print_var_attribute_list (all_var_attr_entries.size(), all_var_attr_entries);
		print_var_attr_data(all_var_attr_entries.size(), all_var_attr_entries);
	}	
    if (output_objector_params) {
    	count = 0;
        for (md_catalog_var_attribute_entry attr : all_var_attr_entries) {
	        if (dims_overlap ( attr.dims, proc_dims) ) {
        		extreme_debug_log << "found a bb for rank " << rank << endl;
          		objector_params my_objector_params = get_objector_params(run, vars.at(attr.var_id), attr.dims, CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_DIMS_BY_NAME_VER, proc_dims);
        		all_objector_params.push_back(my_objector_params);
        		count += 1;
        	}
            // if(my_objector_params.var_id !=attr.var_id) {
            // 	error_log << "attrs var_id: " << attr.var_id << " but given var's id: " << my_objector_params.var_id;
            // 	return RC_ERR;
            // }
        }
     	add_objector_point(BOUNDING_BOX_TO_OBJ_NAMES, count);
    }	
	else if (output_obj_names) {
	    for (md_catalog_var_attribute_entry attr : all_var_attr_entries) {
	   		if (dims_overlap ( attr.dims, proc_dims) ) {
	            rc = add_object_names_offests_and_counts(run, vars.at(attr.var_id), get_overlapping_dims_for_bounding_box(attr.dims, proc_dims), CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_DIMS_BY_NAME_VER);  
	            if (rc != RC_OK) {
	                error_log << "error in add_object_names_offests_and_counts \n";
	                return rc;
	            }
	            // NOTE: THIS IS WHERE DATA WOULD BE READ
	       		extreme_debug_log << "for rank: " << rank << " found a matching bb" << endl;
	        }
	    }
	}
	else if (hdf5_read_data) {
		read_data_for_attrs(run, vars, all_var_attr_entries, proc_dims);
	}
	add_timing_point(CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_DIMS_BY_NAME_VER_COLLECTIVE_DONE);	

    // rc = retrieveObjNamesAndDataForAttrCatalog(data_outputs, server, new_run, var.run_id, var.timestep_id, txn_id, var_attr_entries );
    // if (rc != RC_OK) {
    //     error_log << "error. could not perform retrieveObjNamesAndDataForAttrCatalog \n";
    // }

    return rc;
}

int catalog_var_attributes_with_type_var ( const md_server &server, int rank, uint32_t num_servers, uint32_t num_client_procs,
										const md_catalog_run_entry &run, 
										const md_catalog_type_entry &type, const vector<md_catalog_var_entry> &vars, 
										const md_catalog_var_entry &var,
                                        uint64_t txn_id, const vector<md_dim_bounds> &proc_dims
                                        ) 
{

    int rc = RC_OK;
    uint32_t count;
    vector<md_catalog_var_attribute_entry> var_attr_entries;

	vector<md_catalog_var_attribute_entry> all_var_attr_entries;

	MPI_Barrier(MPI_COMM_WORLD);
	add_timing_point(CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_BY_NAME_VER_START);	
	if(rank < num_servers) {
	    rc = metadata_catalog_all_var_attributes_with_type_var_by_name_ver(server, var.run_id, var.timestep_id, type.name, type.version, 
	            var.name, var.version, txn_id, count, var_attr_entries);

	    if (rc == RC_OK) {
	   	    // if(testing_logging || (zero_rank_logging && rank == 0)) {
		       //  zero_rank_log << "using var var_attr_entries with type var by name ver funct: attributes associated with run_id " << var.run_id << ", timestep_id " << 
		       //      var.timestep_id << " and txn_id: " << txn_id << " and type " << type.name << " ver " << type.version <<
		       //      " and var " << var.name << " ver " << var.version;
		       //  zero_rank_log << " for server: " << rank << " \n";
	        //     print_var_attribute_list (count, var_attr_entries);
	        // 	print_var_attr_data(count, var_attr_entries);         
	        // }
	    }
	    else {
	        error_log << "Error getting the matching attribute list. Proceeding \n";
	    }
	}
	add_timing_point(CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_BY_NAME_VER_DONE);	

	MPI_Barrier(MPI_COMM_WORLD); //leave to make timing clearer
	add_timing_point(CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_BY_NAME_VER_COLLECTIVE_START);	
	gather_attr_entries(var_attr_entries, count, rank, num_servers, num_client_procs, all_var_attr_entries);
	if(testing_logging || (zero_rank_logging  && rank == 0)) {
			zero_rank_log << "rank: " << rank << " all var attributes associated with run_id " << var.run_id << " and timestep_id " << var.timestep_id << 
				" and type_id: " << type.type_id <<
				" and var_id: " << var.var_id << " note: all_var_attr_entries.size(): " << all_var_attr_entries.size() << endl;
		print_var_attribute_list (all_var_attr_entries.size(), all_var_attr_entries);
		print_var_attr_data(all_var_attr_entries.size(), all_var_attr_entries);
	}	
    if (output_objector_params) {
    	count = 0;
        for (md_catalog_var_attribute_entry attr : all_var_attr_entries) {
        	if (dims_overlap ( attr.dims, proc_dims) ) {
            		extreme_debug_log << "found a bb for rank " << rank << endl;
	        	all_objector_params.push_back( get_objector_params(run, var, attr.dims, CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_BY_NAME_VER, proc_dims) );
	        	count += 1;
	        }
        }
	 	add_objector_point(BOUNDING_BOX_TO_OBJ_NAMES, count);

    }	
	else if (output_obj_names) {
	    for (md_catalog_var_attribute_entry attr : all_var_attr_entries) {
	   		if (dims_overlap ( attr.dims, proc_dims) ) {
	            rc = add_object_names_offests_and_counts(run, var, get_overlapping_dims_for_bounding_box(attr.dims, proc_dims), CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_BY_NAME_VER);  
	            if (rc != RC_OK) {
	                error_log << "error in add_object_names_offests_and_counts \n";
	                return rc;
	            }
	            // NOTE: THIS IS WHERE DATA WOULD BE READ
	       		extreme_debug_log << "for rank: " << rank << " found a matching bb" << endl;
	        }
	    }
	}
	else if (hdf5_read_data) {
		read_data_for_attrs(run, vars, all_var_attr_entries, proc_dims);
	}
	add_timing_point(CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_BY_NAME_VER_COLLECTIVE_DONE);	

	return rc;
}

int catalog_var_attributes_with_type_var_or_val_functs ( const md_server &server, int rank, uint32_t num_servers, uint32_t num_client_procs,
										const md_catalog_run_entry &run, const md_catalog_type_entry &rare_type,
										const md_catalog_type_entry &value_type, const vector<md_catalog_var_entry> &vars,
										const md_catalog_var_entry &var,
                                        uint64_t txn_id, int num_dims, const vector<md_dim_bounds> &dims_to_search,
                                      	double min_range, double max_range, double above_max_val, double below_min_val,
                                      	const vector<md_dim_bounds> &proc_dims
                                        ) 
{

    int rc = RC_OK;
    uint32_t count;
    vector<md_catalog_var_attribute_entry> var_attr_entries;

	vector<md_catalog_var_attribute_entry> all_var_attr_entries;

	rc = catalog_var_attributes_with_type_var (server, rank, num_servers, num_client_procs, run, 
										rare_type, vars, var, txn_id, proc_dims); 
    if (rc != RC_OK) {
        error_log << "Error with catalog_var_attributes_with_type_var. Proceeding \n";
    }

    // rc = retrieveObjNamesAndDataForAttrCatalog(data_outputs, server, new_run, var.run_id, var.timestep_id, txn_id, var_attr_entries );
    // if (rc != RC_OK) {
    //     error_log << "error. could not perform retrieveObjNamesAndDataForAttrCatalog \n";
    // }
    string range_data;
    make_range_data (min_range, max_range, range_data);

    MPI_Barrier(MPI_COMM_WORLD);
	add_timing_point(CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_RANGE_START);	
	if(rank < num_servers) {
	    rc = metadata_catalog_all_var_attributes_with_type_var_range (server, var.timestep_id, value_type.type_id, var.var_id, txn_id, 
	        ATTR_DATA_TYPE_REAL, range_data, count, var_attr_entries);

	    if (rc == RC_OK) {
	        // if(testing_logging || (zero_rank_logging && rank == 0)) {
		       //  zero_rank_log << "using var var_attr_entries with type var by id funct: attributes associated with run_id " << var.run_id << ", timestep_id " << 
		       //  var.timestep_id << " and txn_id: " << txn_id << " and type " << value_type.name << " ver " << value_type.version <<
		       //  " and var " << var.name << " ver " << var.version << " int attr data and range [" << min_range << "," << max_range << "]" <<
		       //  " for server: " << rank << " \n";
	        //     print_var_attribute_list (count, var_attr_entries);
	        // 	print_var_attr_data(count, var_attr_entries);         
	        // }
	    }
	    else {
	        error_log << "Error getting the matching attribute list. Proceeding \n";
	    }
	}
	add_timing_point(CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_RANGE_DONE);	

    MPI_Barrier(MPI_COMM_WORLD); //leave to make timing clearer
	add_timing_point(CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_RANGE_COLLECTIVE_START);	
	gather_attr_entries(var_attr_entries, count, rank, num_servers, num_client_procs, all_var_attr_entries);
	if(testing_logging || (zero_rank_logging  && rank == 0)) {
			zero_rank_log << "rank: " << rank << " all var attributes associated with run_id " << var.run_id << " and timestep_id " << var.timestep_id << 
				" and type_id: " << value_type.type_id <<
				" and var_id: " << var.var_id << " int attr data and range [" << min_range << "," << max_range << "]" <<
				" note: all_var_attr_entries.size(): " << all_var_attr_entries.size() << endl;
		print_var_attribute_list (all_var_attr_entries.size(), all_var_attr_entries);
		print_var_attr_data(all_var_attr_entries.size(), all_var_attr_entries);
	}	
    if (output_objector_params) {
    	count = 0;
        for (md_catalog_var_attribute_entry attr : all_var_attr_entries) {
        	if (dims_overlap ( attr.dims, proc_dims) ) {
            		extreme_debug_log << "found a bb for rank " << rank << endl;
	        	all_objector_params.push_back( get_objector_params(run, var, attr.dims, CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_RANGE, proc_dims) );
	        	count += 1;
	        }
        }
     	add_objector_point(BOUNDING_BOX_TO_OBJ_NAMES, count);
    }		
	else if (output_obj_names) {
	    for (md_catalog_var_attribute_entry attr : all_var_attr_entries) {
	   		if (dims_overlap ( attr.dims, proc_dims) ) {
	            rc = add_object_names_offests_and_counts(run, var, get_overlapping_dims_for_bounding_box(attr.dims, proc_dims), CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_RANGE);  
	            if (rc != RC_OK) {
	                error_log << "error in add_object_names_offests_and_counts \n";
	                return rc;
	            }
	            // NOTE: THIS IS WHERE DATA WOULD BE READ
	       		extreme_debug_log << "for rank: " << rank << " found a matching bb" << endl;
	        }
	    }
	}
	else if (hdf5_read_data) {
		read_data_for_attrs(run, vars, all_var_attr_entries, proc_dims);
	}
	add_timing_point(CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_RANGE_COLLECTIVE_DONE);	

    string above_max_data;
    //uint64_t new_attr.data = 12;
    make_single_val_data (above_max_val, above_max_data);

	all_var_attr_entries.clear();

    MPI_Barrier(MPI_COMM_WORLD);
	add_timing_point(CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_ABOVE_MAX_START);	
	if(rank < num_servers) {
	    rc = metadata_catalog_all_var_attributes_with_type_var_above_max (server, var.timestep_id, value_type.type_id, var.var_id, txn_id, 
	        ATTR_DATA_TYPE_REAL, above_max_data, count, var_attr_entries);

	    if (rc == RC_OK) {
	        // if(testing_logging || (zero_rank_logging && rank == 0)) {
		       //  zero_rank_log << "using var var_attr_entries with type var by id funct: attributes associated with run_id " << var.run_id << ", timestep_id " << 
		       //  var.timestep_id << " and txn_id: " << txn_id << " and type " << value_type.name << " ver " << value_type.version <<
		       //  " and var " << var.name << " ver " << var.version << " int attr data with above_max_val geq " << above_max_val << 
		       //  " for server: " << rank << "\n";
	        //     print_var_attribute_list (count, var_attr_entries);
	        // 	print_var_attr_data(count, var_attr_entries);         
	        // }
	    }
	    else {
	        error_log << "Error getting the matching attribute list. Proceeding \n";
	    }
	}
	add_timing_point(CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_ABOVE_MAX_DONE);	

    MPI_Barrier(MPI_COMM_WORLD); //leave to make timing clearer
	add_timing_point(CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_ABOVE_MAX_COLLECTIVE_START);		
	gather_attr_entries(var_attr_entries, count, rank, num_servers, num_client_procs, all_var_attr_entries);
	if(testing_logging || (zero_rank_logging  && rank == 0)) {
			zero_rank_log << "rank: " << rank << " all var attributes associated with run_id " << var.run_id << " and timestep_id " << var.timestep_id << 
				" and type_id: " << value_type.type_id <<
				" and var_id: " << var.var_id << " int attr data with above_max_val geq " << above_max_val <<
				" note: all_var_attr_entries.size(): " << all_var_attr_entries.size() << endl;
		print_var_attribute_list (all_var_attr_entries.size(), all_var_attr_entries);
		print_var_attr_data(all_var_attr_entries.size(), all_var_attr_entries);
	}	
    if (output_objector_params) {
    	count = 0;
        for (md_catalog_var_attribute_entry attr : all_var_attr_entries) {
        	if (dims_overlap ( attr.dims, proc_dims) ) {
	        	all_objector_params.push_back( get_objector_params(run, var, attr.dims, CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_ABOVE_MAX, proc_dims) );
            		extreme_debug_log << "found a bb for rank " << rank << endl;
	        	count += 1;
	        }
        }
     	add_objector_point(BOUNDING_BOX_TO_OBJ_NAMES, count);

    }		
	else if (output_obj_names) {
	    for (md_catalog_var_attribute_entry attr : all_var_attr_entries) {
	   		if (dims_overlap ( attr.dims, proc_dims) ) {
	            rc = add_object_names_offests_and_counts(run, var, get_overlapping_dims_for_bounding_box(attr.dims, proc_dims), CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_ABOVE_MAX);  
	            if (rc != RC_OK) {
	                error_log << "error in add_object_names_offests_and_counts \n";
	                return rc;
	            }
	            // NOTE: THIS IS WHERE DATA WOULD BE READ
	       		extreme_debug_log << "for rank: " << rank << " found a matching bb" << endl;
	        }
	    }
	}
	else if (hdf5_read_data) {
		read_data_for_attrs(run, vars, all_var_attr_entries, proc_dims);
	}
	add_timing_point(CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_ABOVE_MAX_COLLECTIVE_DONE);	

    string below_min_data;
    make_single_val_data (below_min_val, below_min_data);

	all_var_attr_entries.clear();

    MPI_Barrier(MPI_COMM_WORLD);
	add_timing_point(CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_BELOW_MIN_START);	
	if(rank < num_servers) {
	    rc = metadata_catalog_all_var_attributes_with_type_var_below_min (server, var.timestep_id, value_type.type_id, var.var_id, txn_id, 
	        ATTR_DATA_TYPE_REAL, below_min_data, count, var_attr_entries);

	    if (rc == RC_OK) {
	 	    // if(testing_logging || (zero_rank_logging && rank == 0)) {
		     //    zero_rank_log << "using var var_attr_entries with type var by id funct: attributes associated with run_id " << var.run_id << ", timestep_id " << 
		     //    var.timestep_id << " and txn_id: " << txn_id << " and type " << value_type.name << " ver " << value_type.version <<
		     //    " and var " << var.name << " ver " << var.version << " int attr data with below_min_val leq " << below_min_val << 
		     //    " for server: " << rank << "\n";
	      //       print_var_attribute_list (count, var_attr_entries);
	      //   	print_var_attr_data(count, var_attr_entries);         
	      //   }
	    }
	    else {
	        error_log << "Error getting the matching attribute list. Proceeding \n";
	    }
	}
	add_timing_point(CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_BELOW_MIN_DONE);	

    MPI_Barrier(MPI_COMM_WORLD); //leave to make timing clearer
	add_timing_point(CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_BELOW_MIN_COLLECTIVE_START);	
	gather_attr_entries(var_attr_entries, count, rank, num_servers, num_client_procs, all_var_attr_entries);
	if(testing_logging || (zero_rank_logging  && rank == 0)) {
			zero_rank_log << "rank: " << rank << " all var attributes associated with run_id " << var.run_id << " and timestep_id " << var.timestep_id << 
				" and type_id: " << value_type.type_id <<
				" and var_id: " << var.var_id << " int attr data with below_min_val leq " << below_min_val <<
				" note: all_var_attr_entries.size(): " << all_var_attr_entries.size() << endl;
		print_var_attribute_list (all_var_attr_entries.size(), all_var_attr_entries);
		print_var_attr_data(all_var_attr_entries.size(), all_var_attr_entries);
	}	
    if (output_objector_params) {
    	count = 0;
        for (md_catalog_var_attribute_entry attr : all_var_attr_entries) {
        	if (dims_overlap ( attr.dims, proc_dims) ) {
	        	all_objector_params.push_back( get_objector_params(run, var, attr.dims, CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_BELOW_MIN, proc_dims) );
            		extreme_debug_log << "found a bb for rank " << rank << endl;
	        	count += 1;
	        }
        }
     	add_objector_point(BOUNDING_BOX_TO_OBJ_NAMES, count);
    }	
	else if (output_obj_names) {
	    for (md_catalog_var_attribute_entry attr : all_var_attr_entries) {
	   		if (dims_overlap ( attr.dims, proc_dims) ) {
	            rc = add_object_names_offests_and_counts(run, var, get_overlapping_dims_for_bounding_box(attr.dims, proc_dims), CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_BELOW_MIN);  
	            if (rc != RC_OK) {
	                error_log << "error in add_object_names_offests_and_counts \n";
	                return rc;
	            }
	            // NOTE: THIS IS WHERE DATA WOULD BE READ
	       		extreme_debug_log << "for rank: " << rank << " found a matching bb" << endl;
	        }
	    }
	}
	else if (hdf5_read_data) {
		read_data_for_attrs(run, vars, all_var_attr_entries, proc_dims);
	}
	add_timing_point(CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_BELOW_MIN_COLLECTIVE_DONE);	

    // rc = retrieveObjNamesAndDataForAttrCatalog(data_outputs, server, new_run, var.run_id, var.timestep_id, txn_id, var_attr_entries );
    // if (rc != RC_OK) {
    //     error_log << "error. could not perform retrieveObjNamesAndDataForAttrCatalog \n";
    // }


	all_var_attr_entries.clear();

    MPI_Barrier(MPI_COMM_WORLD);
	add_timing_point(CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_DIMS_BY_NAME_VER_START);	
	if(rank < num_servers) {
	    rc = metadata_catalog_all_var_attributes_with_type_var_dims_by_name_ver (server, var.run_id, var.timestep_id, rare_type.name, rare_type.version, 
	            var.name, var.version, txn_id, num_dims, dims_to_search, count, var_attr_entries);

	    if (rc == RC_OK) {
	        // if(testing_logging || (zero_rank_logging && rank == 0)) {
		       //  zero_rank_log << "using var var_attr_entries with type var dims by name ver funct: attributes associated with run_id " << var.run_id << ", timestep_id " << 
		       //      var.timestep_id << " and txn_id: " << txn_id << " and type " << type.name << " ver " << type.version <<
		       //      " and var " << var.name << " ver " << var.version << " overlapping with dims";
		       //  for(int j=0; j< num_dims; j++) {
		       //      zero_rank_log << " d" << j << "_min: " << dims_to_search [j].min;
		       //      zero_rank_log << " d" << j << "_max: " << dims_to_search [j].max;               
		       //  }
		       //  zero_rank_log << " for server: " << rank << " \n";
	        //     print_var_attribute_list (count, var_attr_entries);
	        // 	print_var_attr_data(count, var_attr_entries);         
	        // }
	    }
	    else {
	        error_log << "Error getting the matching attribute list. Proceeding \n";
	    }
	}
	add_timing_point(CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_DIMS_BY_NAME_VER_DONE);	

    MPI_Barrier(MPI_COMM_WORLD); //leave to make timing clearer
	add_timing_point(CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_DIMS_BY_NAME_VER_COLLECTIVE_START);	
	gather_attr_entries(var_attr_entries, count, rank, num_servers, num_client_procs, all_var_attr_entries);
	if(testing_logging || (zero_rank_logging  && rank == 0)) {
		zero_rank_log << "rank: " << rank << " all var attributes associated with run_id " << var.run_id << " and timestep_id " << var.timestep_id << 
			" and type_id: " << rare_type.type_id << " and var_id: " << var.var_id << " overlapping with dims:"; 
        for(int j=0; j< num_dims; j++) {
            zero_rank_log << " d" << j << "_min: " << dims_to_search [j].min;
            zero_rank_log << " d" << j << "_max: " << dims_to_search [j].max;               
        }
		zero_rank_log << " note: all_var_attr_entries.size(): " << all_var_attr_entries.size() << endl;
		print_var_attribute_list (all_var_attr_entries.size(), all_var_attr_entries);
		print_var_attr_data(all_var_attr_entries.size(), all_var_attr_entries);
	}	
    if (output_objector_params) {
    	count = 0;
        for (md_catalog_var_attribute_entry attr : all_var_attr_entries) {
        	if (dims_overlap ( attr.dims, proc_dims) ) {
            		extreme_debug_log << "found a bb for rank " << rank << endl;
	        	all_objector_params.push_back( get_objector_params(run, var, attr.dims, CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_DIMS_BY_NAME_VER, proc_dims) );
	        	count += 1;
	        }
        }
     	add_objector_point(BOUNDING_BOX_TO_OBJ_NAMES, count);
    }		
	else if (output_obj_names) {
	    for (md_catalog_var_attribute_entry attr : all_var_attr_entries) {
	   		if (dims_overlap ( attr.dims, proc_dims) ) {
	            rc = add_object_names_offests_and_counts(run, var, get_overlapping_dims_for_bounding_box(attr.dims, proc_dims), CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_DIMS_BY_NAME_VER);  
	            if (rc != RC_OK) {
	                error_log << "error in add_object_names_offests_and_counts \n";
	                return rc;
	            }
	            // NOTE: THIS IS WHERE DATA WOULD BE READ
	       		extreme_debug_log << "for rank: " << rank << " found a matching bb" << endl;
	        }
	    }
	}
	else if (hdf5_read_data) {
		read_data_for_attrs(run, vars, all_var_attr_entries, proc_dims);
	}			
	add_timing_point(CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_DIMS_BY_NAME_VER_COLLECTIVE_DONE);	

    // rc = retrieveObjNamesAndDataForAttrCatalog(data_outputs, server, new_run, var.run_id, var.timestep_id, txn_id, var_attr_entries );
    // if (rc != RC_OK) {
    //     error_log << "error. could not perform retrieveObjNamesAndDataForAttrCatalog \n";
    // }

	all_var_attr_entries.clear();

    MPI_Barrier(MPI_COMM_WORLD);
	add_timing_point(CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_DIMS_RANGE_START);	
	if(rank < num_servers) {
	    rc = metadata_catalog_all_var_attributes_with_type_var_dims_range (server, var.timestep_id, value_type.type_id, var.var_id, txn_id , 
	        num_dims, dims_to_search, ATTR_DATA_TYPE_REAL, range_data, count, var_attr_entries);

	    if (rc == RC_OK) {
	        // if(testing_logging || (zero_rank_logging && rank == 0)) {
		       //  zero_rank_log << "using var var_attr_entries with type var by id funct: attributes associated with run_id " << var.run_id << ", timestep_id " << 
		       //  var.timestep_id << " and txn_id: " << txn_id << " and type " << value_type.name << " ver " << value_type.version <<
		       //  " and var " << var.name << " ver " << var.version << " int attr data and range [" << min_range << "," << max_range << "]" <<
		       //  " overlapping with dims";
		       //  for(int j=0; j< num_dims; j++) {
		       //      zero_rank_log << " d" << j << "_min: " << dims_to_search [j].min;
		       //      zero_rank_log << " d" << j << "_max: " << dims_to_search [j].max;               
		       //  }
		       //  zero_rank_log << " for server: " << rank << " \n";
	        //     print_var_attribute_list (count, var_attr_entries);
	        // 	print_var_attr_data(count, var_attr_entries);         
	        // }
	    }
	    else {
	        error_log << "Error getting the matching attribute list. Proceeding \n";
	    }
	}
	add_timing_point(CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_DIMS_RANGE_DONE);	

    MPI_Barrier(MPI_COMM_WORLD); //leave to make timing clearer
	add_timing_point(CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_DIMS_RANGE_COLLECTIVE_START);	
	gather_attr_entries(var_attr_entries, count, rank, num_servers, num_client_procs, all_var_attr_entries);
	if(testing_logging || (zero_rank_logging  && rank == 0)) {
		zero_rank_log << "rank: " << rank << " all var attributes associated with run_id " << var.run_id << " and timestep_id " << var.timestep_id << 
			" and type_id: " << value_type.type_id << " and var_id: " << var.var_id <<
			" int attr data and range [" << min_range << "," << max_range << "] overlapping with dims";
        for(int j=0; j< num_dims; j++) {
            zero_rank_log << " d" << j << "_min: " << dims_to_search [j].min;
            zero_rank_log << " d" << j << "_max: " << dims_to_search [j].max;               
        }
		zero_rank_log << " note: all_var_attr_entries.size(): " << all_var_attr_entries.size() << endl;
		print_var_attribute_list (all_var_attr_entries.size(), all_var_attr_entries);
		print_var_attr_data(all_var_attr_entries.size(), all_var_attr_entries);
	}	
    if (output_objector_params) {
    	count = 0;
        for (md_catalog_var_attribute_entry attr : all_var_attr_entries) {
        	if (dims_overlap ( attr.dims, proc_dims) ) {
            		extreme_debug_log << "found a bb for rank " << rank << endl;
	        	all_objector_params.push_back( get_objector_params(run, var, attr.dims, CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_DIMS_RANGE, proc_dims) );
	        	count += 1;
	        }
        }
     	add_objector_point(BOUNDING_BOX_TO_OBJ_NAMES, count);
    }	
	else if (output_obj_names) {
	    for (md_catalog_var_attribute_entry attr : all_var_attr_entries) {
	   		if (dims_overlap ( attr.dims, proc_dims) ) {
	            rc = add_object_names_offests_and_counts(run, var, get_overlapping_dims_for_bounding_box(attr.dims, proc_dims), CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_DIMS_RANGE);  
	            if (rc != RC_OK) {
	                error_log << "error in add_object_names_offests_and_counts \n";
	                return rc;
	            }
	            // NOTE: THIS IS WHERE DATA WOULD BE READ
	       		extreme_debug_log << "for rank: " << rank << " found a matching bb" << endl;
	        }
	    }
	}
	else if (hdf5_read_data) {
		read_data_for_attrs(run, vars, all_var_attr_entries, proc_dims);
	}		
	add_timing_point(CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_DIMS_RANGE_COLLECTIVE_DONE);	

    // rc = retrieveObjNamesAndDataForAttrCatalog(data_outputs, server, new_run, var.run_id, var.timestep_id, txn_id, var_attr_entries );
    // if (rc != RC_OK) {
    //     error_log << "error. could not perform retrieveObjNamesAndDataForAttrCatalog \n";
    // }

	all_var_attr_entries.clear();

    MPI_Barrier(MPI_COMM_WORLD);
	add_timing_point(CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_DIMS_ABOVE_MAX_START);	
	if(rank < num_servers) {
	    rc = metadata_catalog_all_var_attributes_with_type_var_dims_above_max (server, var.timestep_id, value_type.type_id, var.var_id, txn_id , 
	        num_dims, dims_to_search, ATTR_DATA_TYPE_REAL, above_max_data, count, var_attr_entries);

	    if (rc == RC_OK) {
	        // if(testing_logging || (zero_rank_logging && rank == 0)) {
		       //  zero_rank_log << "using var var_attr_entries with type var by id funct: attributes associated with run_id " << var.run_id << ", timestep_id " << 
		       //  var.timestep_id << " and txn_id: " << txn_id << " and type " << type.name << " ver " << type.version <<
		       //  " and var " << var.name << " ver " << var.version << " int attr data and above_max_val geq " << above_max_val <<
		       //  " overlapping with dims";
		       //  for(int j=0; j< num_dims; j++) {
		       //      zero_rank_log << " d" << j << "_min: " << dims_to_search [j].min;
		       //      zero_rank_log << " d" << j << "_max: " << dims_to_search [j].max;               
		       //  }
		       //  zero_rank_log << " for server: " << rank << " \n";
	        //     print_var_attribute_list (count, var_attr_entries);
	        // 	print_var_attr_data(count, var_attr_entries);         
	        // }
	    }
	    else {
	        error_log << "Error getting the matching attribute list. Proceeding \n";
	    }
	}
	add_timing_point(CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_DIMS_ABOVE_MAX_DONE);	

    MPI_Barrier(MPI_COMM_WORLD); //leave to make timing clearer
	add_timing_point(CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_DIMS_ABOVE_MAX_COLLECTIVE_START);	
	gather_attr_entries(var_attr_entries, count, rank, num_servers, num_client_procs, all_var_attr_entries);
	if(testing_logging || (zero_rank_logging  && rank == 0)) {
		zero_rank_log << "rank: " << rank << " all var attributes associated with run_id " << var.run_id << " and timestep_id " << var.timestep_id << 
			" and type_id: " << value_type.type_id << " and var_id: " << var.var_id <<
			" int attr data and above_max_val geq " << above_max_val << " overlapping with dims";
        for(int j=0; j< num_dims; j++) {
            zero_rank_log << " d" << j << "_min: " << dims_to_search [j].min;
            zero_rank_log << " d" << j << "_max: " << dims_to_search [j].max;               
        }
		zero_rank_log << " note: all_var_attr_entries.size(): " << all_var_attr_entries.size() << endl;
		print_var_attribute_list (all_var_attr_entries.size(), all_var_attr_entries);
		print_var_attr_data(all_var_attr_entries.size(), all_var_attr_entries);
	}		
    if (output_objector_params) {
    	count = 0;
        for (md_catalog_var_attribute_entry attr : all_var_attr_entries) {
        	if (dims_overlap ( attr.dims, proc_dims) ) {
            		extreme_debug_log << "found a bb for rank " << rank << endl;
	        	all_objector_params.push_back( get_objector_params(run, var, attr.dims, CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_DIMS_ABOVE_MAX, proc_dims) );
	        	count += 1;
	        }
        }
     	add_objector_point(BOUNDING_BOX_TO_OBJ_NAMES, count);
    }		
	else if (output_obj_names) {
	    for (md_catalog_var_attribute_entry attr : all_var_attr_entries) {
	   		if (dims_overlap ( attr.dims, proc_dims) ) {
	            rc = add_object_names_offests_and_counts(run, var, get_overlapping_dims_for_bounding_box(attr.dims, proc_dims), CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_DIMS_ABOVE_MAX);  
	            if (rc != RC_OK) {
	                error_log << "error in add_object_names_offests_and_counts \n";
	                return rc;
	            }
	            // NOTE: THIS IS WHERE DATA WOULD BE READ
	       		extreme_debug_log << "for rank: " << rank << " found a matching bb" << endl;
	        }
	    }
	}
	else if (hdf5_read_data) {
		read_data_for_attrs(run, vars, all_var_attr_entries, proc_dims);
	}
	add_timing_point(CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_DIMS_ABOVE_MAX_COLLECTIVE_DONE);	

    // rc = retrieveObjNamesAndDataForAttrCatalog(data_outputs, server, new_run, var.run_id, var.timestep_id, txn_id, var_attr_entries );
    // if (rc != RC_OK) {
    //     error_log << "error. could not perform retrieveObjNamesAndDataForAttrCatalog \n";
    // }

	all_var_attr_entries.clear();

    MPI_Barrier(MPI_COMM_WORLD);
	add_timing_point(CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_DIMS_BELOW_MIN_START);	
	if(rank < num_servers) {
	    rc = metadata_catalog_all_var_attributes_with_type_var_dims_below_min(server, var.timestep_id, value_type.type_id, var.var_id, txn_id , 
	        num_dims, dims_to_search, ATTR_DATA_TYPE_REAL, below_min_data, count, var_attr_entries);

	    if (rc == RC_OK) {
	 	   //  if(testing_logging || (zero_rank_logging && rank == 0)) {
		    //     zero_rank_log << "using var var_attr_entries with type var by id funct: attributes associated with run_id " << var.run_id << ", timestep_id " << 
		    //     var.timestep_id << " and txn_id: " << txn_id << " and type " << type.name << " ver " << type.version <<
		    //     " and var " << var.name << " ver " << var.version << " int attr data and below_min_val leq " << below_min_val <<
		    //     " overlapping with dims";
		    //     for(int j=0; j< num_dims; j++) {
		    //         zero_rank_log << " d" << j << "_min: " << dims_to_search [j].min;
		    //         zero_rank_log << " d" << j << "_max: " << dims_to_search [j].max;               
		    //     }
		    //     zero_rank_log << " for server: " << rank << " \n";
	     //        print_var_attribute_list (count, var_attr_entries);
	     //    	print_var_attr_data(count, var_attr_entries);         
		    // }
	    }
	    else {
	        error_log << "Error getting the matching attribute list. Proceeding \n";
	    }
	}
	add_timing_point(CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_DIMS_BELOW_MIN_DONE);	

    MPI_Barrier(MPI_COMM_WORLD); //leave to make timing clearer
	add_timing_point(CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_DIMS_BELOW_MIN_COLLECTIVE_START);	
	gather_attr_entries(var_attr_entries, count, rank, num_servers, num_client_procs, all_var_attr_entries);
	if(testing_logging || (zero_rank_logging  && rank == 0)) {
		zero_rank_log << "rank: " << rank << " all var attributes associated with run_id " << var.run_id << " and timestep_id " << var.timestep_id << 
			" and type_id: " << value_type.type_id << " and var_id: " << var.var_id <<
			" int attr data and below_min_val leq " << below_min_val << " overlapping with dims";
        for(int j=0; j< num_dims; j++) {
            zero_rank_log << " d" << j << "_min: " << dims_to_search [j].min;
            zero_rank_log << " d" << j << "_max: " << dims_to_search [j].max;               
        }
		zero_rank_log << " note: all_var_attr_entries.size(): " << all_var_attr_entries.size() << endl;
		print_var_attribute_list (all_var_attr_entries.size(), all_var_attr_entries);
		print_var_attr_data(all_var_attr_entries.size(), all_var_attr_entries);
	}	
    if (output_objector_params) {
    	count = 0;
        for (md_catalog_var_attribute_entry attr : all_var_attr_entries) {
        	if (dims_overlap ( attr.dims, proc_dims) ) {
            		extreme_debug_log << "found a bb for rank " << rank << endl;
	        	all_objector_params.push_back( get_objector_params(run, var, attr.dims, CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_DIMS_BELOW_MIN, proc_dims) );
	        	count += 1;
	        }
        }
     	add_objector_point(BOUNDING_BOX_TO_OBJ_NAMES, count);
    }
	else if (output_obj_names) {
	    for (md_catalog_var_attribute_entry attr : all_var_attr_entries) {
	   		if (dims_overlap ( attr.dims, proc_dims) ) {
	            rc = add_object_names_offests_and_counts(run, var, get_overlapping_dims_for_bounding_box(attr.dims, proc_dims), CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_DIMS_BELOW_MIN);  
	            if (rc != RC_OK) {
	                error_log << "error in add_object_names_offests_and_counts \n";
	                return rc;
	            }
	            // NOTE: THIS IS WHERE DATA WOULD BE READ
	       		extreme_debug_log << "for rank: " << rank << " found a matching bb" << endl;
	        }
	    }
	}		
	else if (hdf5_read_data) {
		read_data_for_attrs(run, vars, all_var_attr_entries, proc_dims);
	}	
	add_timing_point(CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_DIMS_BELOW_MIN_COLLECTIVE_DONE);	

    // rc = retrieveObjNamesAndDataForAttrCatalog(data_outputs, server, new_run, var.run_id, var.timestep_id, txn_id, var_attr_entries );
    // if (rc != RC_OK) {
    //     error_log << "error. could not perform retrieveObjNamesAndDataForAttrCatalog \n";
    // }

    return rc;
}





////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//SUBSTR FUNCTS ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int catalog_types_with_var_substr_in_timestep_functs ( const md_server &server, int rank, uint32_t num_servers, uint32_t num_client_procs,
					uint64_t run_id, uint64_t timestep_id, const string &var_substr,
                    int num_dims, const vector<md_dim_bounds> &dims_to_search, uint64_t txn_id ) {

    int rc = RC_OK;
    uint32_t count;
    vector<md_catalog_type_entry> type_entries;

    vector<md_catalog_type_entry> all_type_entries;

    MPI_Barrier(MPI_COMM_WORLD);
	add_timing_point(CATALOG_ALL_TYPES_WITH_VAR_ATTRIBUTES_WITH_VAR_SUBSTR_IN_TIMESTEP_START);	
    if (rank < num_servers) {
	    rc = metadata_catalog_all_types_with_var_attributes_with_var_substr_in_timestep (server, run_id, timestep_id, var_substr, txn_id, count, type_entries);

	    if (rc == RC_OK) {
        	// if(testing_logging || (zero_rank_logging && rank == 0)) {
		       //  zero_rank_log << "using catalog all types with instances on var str in timestep funct: types associated with run_id " << run_id << 
		       //  	", timestep_id " << timestep_id << " and var_name_substr " << var_substr << " and txn_id: " << txn_id << 
		       //  	" for server: " << rank << " \n";
	        //     print_type_catalog (count, type_entries);
	        // }
	    }
	    else {
	        error_log << "Error getting the matching type catalog. Proceeding \n";
	    }
	}
	add_timing_point(CATALOG_ALL_TYPES_WITH_VAR_ATTRIBUTES_WITH_VAR_SUBSTR_IN_TIMESTEP_DONE);	

    MPI_Barrier(MPI_COMM_WORLD); //leave to make timing clearer
	add_timing_point(CATALOG_ALL_TYPES_WITH_VAR_ATTRIBUTES_WITH_VAR_SUBSTR_IN_TIMESTEP_COLLECTIVE_START);	
	gather_type_entries(type_entries, rank, num_servers, num_client_procs, all_type_entries);
	if(testing_logging || (zero_rank_logging  && rank == 0)) {
		zero_rank_log << "rank: " << rank << " all types associated with a var attribute for run_id " << run_id << ", timestep_id " << timestep_id <<
			 " and var_name_substr " << var_substr <<
			 " note: all_type_entries.size(): " << all_type_entries.size() << endl;
		print_type_catalog (all_type_entries.size(), all_type_entries);
	}
	add_timing_point(CATALOG_ALL_TYPES_WITH_VAR_ATTRIBUTES_WITH_VAR_SUBSTR_IN_TIMESTEP_COLLECTIVE_DONE);	


	all_type_entries.clear();

    MPI_Barrier(MPI_COMM_WORLD);
	add_timing_point(CATALOG_ALL_TYPES_WITH_VAR_ATTRIBUTES_WITH_VAR_SUBSTR_DIMS_IN_TIMESTEP_START);	
	if (rank < num_servers) {
	    rc = metadata_catalog_all_types_with_var_attributes_with_var_substr_dims_in_timestep (server, run_id, timestep_id, var_substr, txn_id, num_dims, dims_to_search, count, type_entries);
	    if (rc == RC_OK) {
	        // if(testing_logging || (zero_rank_logging && rank == 0)) {
		       //  zero_rank_log << "using catalog all types with instances on var str dims in timestep funct: types associated with run_id " << 
		       // 		run_id << ", timestep_id " << timestep_id << 
		       //      " and var_name_substr " << var_substr << " and txn_id: " << txn_id << " overlapping with dims";
		       //  for(int j=0; j< num_dims; j++) {
		       //      zero_rank_log << " d" << j << "_min: " << dims_to_search [j].min;
		       //      zero_rank_log << " d" << j << "_max: " << dims_to_search [j].max;               
		       //  }
		       //  zero_rank_log << " for server: " << rank << " \n";
	        //     print_type_catalog (count, type_entries);
	        // }
	    }
	    else {
	        error_log << "Error getting the matching type catalog. Proceeding \n";
	    }
	}
	add_timing_point(CATALOG_ALL_TYPES_WITH_VAR_ATTRIBUTES_WITH_VAR_SUBSTR_DIMS_IN_TIMESTEP_DONE);	

    MPI_Barrier(MPI_COMM_WORLD); //leave to make timing clearer
	add_timing_point(CATALOG_ALL_TYPES_WITH_VAR_ATTRIBUTES_WITH_VAR_SUBSTR_DIMS_IN_TIMESTEP_COLLECTIVE_START);	
	gather_type_entries(type_entries, rank, num_servers, num_client_procs, all_type_entries);
	if(testing_logging || (zero_rank_logging  && rank == 0)) {
		zero_rank_log << "rank: " << rank << " all types associated with a var attribute for run_id " << run_id << ", timestep_id " << timestep_id <<
			 " and var_name_substr " << var_substr << " overlapping with dims";
		for(int j=0; j< num_dims; j++) {
            zero_rank_log << " d" << j << "_min: " << dims_to_search [j].min;
            zero_rank_log << " d" << j << "_max: " << dims_to_search [j].max;               
        }
		zero_rank_log << " note: all_type_entries.size(): " << all_type_entries.size() << endl;
		print_type_catalog (all_type_entries.size(), all_type_entries);
	}
	add_timing_point(CATALOG_ALL_TYPES_WITH_VAR_ATTRIBUTES_WITH_VAR_SUBSTR_DIMS_IN_TIMESTEP_COLLECTIVE_DONE);	

    return rc;
}


int catalog_timesteps_with_var_substr_or_val_functs ( const md_server &server, int rank, uint32_t num_servers, uint32_t num_client_procs,
					uint64_t run_id, uint64_t type_id, 
					const string &var_name_substr, int num_dims, const vector<md_dim_bounds> &dims_to_search, uint64_t txn_id,
					 double min_range, double max_range, double above_max_val, double below_min_val
					 ) 
{

    int rc = RC_OK;
    uint32_t count;
    vector<md_catalog_timestep_entry> timestep_entries;
 
    MPI_Barrier(MPI_COMM_WORLD);
	add_timing_point(CATALOG_ALL_TIMESTEPS_WITH_VAR_SUBSTR_START);	
    rc = metadata_catalog_all_timesteps_with_var_substr (server, run_id, var_name_substr, txn_id, count, timestep_entries);
    if (rc == RC_OK) {
        // if(testing_logging || (zero_rank_logging && rank == 0)) {
	       //  zero_rank_log << "using catalog_all_timesteps_with_var_substr funct: timesteps associated with run_id " << run_id << 
		      //   ", var_name_substr " << var_name_substr << " and txn_id: " << txn_id << 
		      //   " for server: " << rank << " \n";
        //     print_timestep_catalog (count, timestep_entries);
        // }
    }
    else {
        error_log << "Error getting the matching timestep catalog. Proceeding \n";
    }
	add_timing_point(CATALOG_ALL_TIMESTEPS_WITH_VAR_SUBSTR_DONE);	


    MPI_Barrier(MPI_COMM_WORLD);
	add_timing_point(CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_START);	
    vector<md_catalog_timestep_entry> all_timestep_entries;
    if (rank < num_servers) {
	    rc = metadata_catalog_all_timesteps_with_var_attributes_with_type_var_substr (server, run_id, type_id, var_name_substr, txn_id, count, timestep_entries);

	    if (rc == RC_OK) {
	    //     if(testing_logging || (zero_rank_logging && rank == 0)) {
		   //      zero_rank_log << "using metadata_catalog_all_timesteps_with_var_attributes_with_type_var_substr funct: timesteps associated with run_id " << 
		   //      run_id << ", type_id " << type_id << 
		   //      ", var_name_substr " << var_name_substr << " and txn_id: " << txn_id << 
		   //      " for server: " << rank << " \n";
	    //         print_timestep_catalog (count, timestep_entries);
	    //     }
	    }
	    else {
	        error_log << "Error getting the matching timestep catalog. Proceeding \n";
	    }
	}
	add_timing_point(CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_DONE);	

    MPI_Barrier(MPI_COMM_WORLD); //leave to make timing clearer
	add_timing_point(CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_COLLECTIVE_START);	
	gather_timestep_entries(timestep_entries, rank, num_servers, num_client_procs, all_timestep_entries);
	if(testing_logging || (zero_rank_logging  && rank == 0)) {
		zero_rank_log << "rank: " << rank << " all timesteps associated with a var attribute for run_id " << run_id << 
			", type_id " << type_id << ", var_name_substr " << var_name_substr <<
			" note: all_timestep_entries.size(): " << all_timestep_entries.size() << endl;
		print_timestep_catalog (all_timestep_entries.size(), all_timestep_entries);
	}
	add_timing_point(CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_COLLECTIVE_DONE);	

    string range_data;
    make_range_data (min_range, max_range, range_data);

	all_timestep_entries.clear();

    MPI_Barrier(MPI_COMM_WORLD);
	add_timing_point(CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_RANGE_START);	
	if (rank < num_servers) {
	    rc = metadata_catalog_all_timesteps_with_var_attributes_with_type_var_substr_range (server, run_id, type_id, var_name_substr, txn_id, 
	        ATTR_DATA_TYPE_REAL, range_data, count, timestep_entries);
	    if (rc == RC_OK) {
	        // if(testing_logging || (zero_rank_logging && rank == 0)) {
		       //  zero_rank_log << "using metadata_catalog_all_timesteps_with_var_attributes_with_type_var_substr_range funct: timesteps associated with run_id " << run_id << ", type_id " << type_id << 
		       //  ", var_name_substr " << var_name_substr << " txn_id: " << txn_id << " int attr data and range [" << min_range << "," << max_range << "]" <<
		       //  " for server: " << rank << " \n";
	        //     print_timestep_catalog (count, timestep_entries);
	        // }
	    }
	    else {
	        error_log << "Error getting the matching timestep catalog. Proceeding \n";
	    }
	}
	add_timing_point(CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_RANGE_DONE);	

    MPI_Barrier(MPI_COMM_WORLD); //leave to make timing clearer
	add_timing_point(CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_RANGE_COLLECTIVE_START);	
	gather_timestep_entries(timestep_entries, rank, num_servers, num_client_procs, all_timestep_entries);
	if(testing_logging || (zero_rank_logging  && rank == 0)) {
		zero_rank_log << "rank: " << rank << " all timesteps associated with a var attribute for run_id " << run_id << 
			", type_id " << type_id << ", var_name_substr " << var_name_substr << 
			" int attr data and range [" << min_range << "," << max_range << "]" <<
			" note: all_timestep_entries.size(): " << all_timestep_entries.size() << endl;
		print_timestep_catalog (all_timestep_entries.size(), all_timestep_entries);
	}
	add_timing_point(CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_RANGE_COLLECTIVE_DONE);	

    string above_max_data;
    make_single_val_data (above_max_val, above_max_data);

	all_timestep_entries.clear();

    MPI_Barrier(MPI_COMM_WORLD);
	add_timing_point(CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_ABOVE_MAX_START);	
	if(rank < num_servers) {
	    rc = metadata_catalog_all_timesteps_with_var_attributes_with_type_var_substr_above_max (server, run_id, type_id, var_name_substr, txn_id, 
	        ATTR_DATA_TYPE_REAL, above_max_data, count, timestep_entries);

	    if (rc == RC_OK) {
	        // if(testing_logging || (zero_rank_logging && rank == 0)) {	   
		       //  zero_rank_log << "using metadata_catalog_all_timesteps_with_var_attributes_with_type_var_substr_above_max funct: timesteps associated with run_id " << run_id << ", type_id " << type_id << 
		       //  ", var_name_substr " << var_name_substr << " txn_id: " << txn_id << " int attr data greater than or eq to " << above_max_val << "]" <<
		       //  " for server: " << rank << " \n";
	        //     print_timestep_catalog (count, timestep_entries);
	        // }
	    }
	    else {
	        error_log << "Error getting the matching timestep catalog. Proceeding \n";
	    }
	}
	add_timing_point(CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_ABOVE_MAX_DONE);	

    MPI_Barrier(MPI_COMM_WORLD); //leave to make timing clearer
	add_timing_point(CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_ABOVE_MAX_COLLECTIVE_START);	
	gather_timestep_entries(timestep_entries, rank, num_servers, num_client_procs, all_timestep_entries);
	if(testing_logging || (zero_rank_logging  && rank == 0)) {
		zero_rank_log << "rank: " << rank << " all timesteps associated with a var attribute for run_id " << run_id << 
			", type_id " << type_id << ", var_name_substr " << var_name_substr << 
			" int attr data greater than or eq to " << above_max_val << "]" <<
			" note: all_timestep_entries.size(): " << all_timestep_entries.size() << endl;
		print_timestep_catalog (all_timestep_entries.size(), all_timestep_entries);
	}
	add_timing_point(CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_ABOVE_MAX_COLLECTIVE_DONE);	

    string below_min_data;
    make_single_val_data (below_min_val, below_min_data);

	all_timestep_entries.clear();

    MPI_Barrier(MPI_COMM_WORLD);
	add_timing_point(CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_BELOW_MIN_START);	
	if(rank < num_servers) {
	    rc = metadata_catalog_all_timesteps_with_var_attributes_with_type_var_substr_below_min (server, run_id, type_id, var_name_substr, txn_id, 
	        ATTR_DATA_TYPE_REAL, below_min_data, count, timestep_entries);

	    if (rc == RC_OK) {
	        // if(testing_logging || (zero_rank_logging && rank == 0)) {	    
		       //  zero_rank_log << "using metadata_catalog_all_timesteps_with_var_attributes_with_type_var_substr_below_min funct: timesteps associated with run_id " << run_id << ", type_id " << type_id << 
		       //  ", var_name_substr " << var_name_substr << " txn_id: " << txn_id << " int attr data less than or eq to " << below_min_val << "]" <<
		       //  " for server: " << rank << " \n";
	        //     print_timestep_catalog (count, timestep_entries);
	        // }
	    }
	    else {
	        error_log << "Error getting the matching timestep catalog. Proceeding \n";
	    }
	}
	add_timing_point(CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_BELOW_MIN_DONE);	

    MPI_Barrier(MPI_COMM_WORLD); //leave to make timing clearer
	add_timing_point(CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_BELOW_MIN_COLLECTIVE_START);	
	gather_timestep_entries(timestep_entries, rank, num_servers, num_client_procs, all_timestep_entries);
	if(testing_logging || (zero_rank_logging  && rank == 0)) {
		zero_rank_log << "rank: " << rank << " all timesteps associated with a var attribute for run_id " << run_id << 
			", type_id " << type_id << ", var_name_substr " << var_name_substr << 
			" int attr data less than or eq to " << below_min_val << "]" <<
			" note: all_timestep_entries.size(): " << all_timestep_entries.size() << endl;
		print_timestep_catalog (all_timestep_entries.size(), all_timestep_entries);
	}
	add_timing_point(CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_BELOW_MIN_COLLECTIVE_DONE);	


	all_timestep_entries.clear();

    MPI_Barrier(MPI_COMM_WORLD);
	add_timing_point(CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_DIMS_START);	
	if(rank < num_servers) {
	    rc = metadata_catalog_all_timesteps_with_var_attributes_with_type_var_substr_dims (server, run_id, type_id, var_name_substr, txn_id, num_dims, dims_to_search, count, timestep_entries);
	    if (rc == RC_OK) {
	        // if(testing_logging || (zero_rank_logging && rank == 0)) {	    	
		       //  zero_rank_log << "using metadata_catalog_all_timesteps_with_var_attributes_with_type_var_substr_dims funct: timesteps associated with run_id " << run_id << ", type_id " << type_id << 
		       //  ", var_name_substr " << var_name_substr << " and txn_id: " << txn_id << " overlapping with dims";
		       //  for(int j=0; j< num_dims; j++) {
		       //      zero_rank_log << " d" << j << "_min: " << dims_to_search [j].min;
		       //      zero_rank_log << " d" << j << "_max: " << dims_to_search [j].max;               
		       //  }
		       //  zero_rank_log << " for server: " << rank << " \n";
	        //     print_timestep_catalog (count, timestep_entries);
	        // }
	    }
	    else {
	        error_log << "Error getting the matching timestep catalog. Proceeding \n";
	    }
	}
	add_timing_point(CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_DIMS_DONE);	

    MPI_Barrier(MPI_COMM_WORLD); //leave to make timing clearer
	add_timing_point(CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_DIMS_COLLECTIVE_START);	
	gather_timestep_entries(timestep_entries, rank, num_servers, num_client_procs, all_timestep_entries);
	if(testing_logging || (zero_rank_logging  && rank == 0)) {
		zero_rank_log << "rank: " << rank << " all timesteps associated with a var attribute for run_id " << run_id << 
			", type_id " << type_id << ", var_name_substr " << var_name_substr << 
			" overlapping with dims";
        for(int j=0; j< num_dims; j++) {
            zero_rank_log << " d" << j << "_min: " << dims_to_search [j].min;
            zero_rank_log << " d" << j << "_max: " << dims_to_search [j].max;               
        }
		zero_rank_log << " note: all_timestep_entries.size(): " << all_timestep_entries.size() << endl;
		print_timestep_catalog (all_timestep_entries.size(), all_timestep_entries);
	}
	add_timing_point(CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_DIMS_COLLECTIVE_DONE);	

	all_timestep_entries.clear();

    MPI_Barrier(MPI_COMM_WORLD);
	add_timing_point(CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_DIMS_RANGE_START);	
	if (rank < num_servers) {
	    rc = metadata_catalog_all_timesteps_with_var_attributes_with_type_var_substr_dims_range (server, run_id, type_id, var_name_substr, txn_id, num_dims, dims_to_search, 
	        ATTR_DATA_TYPE_REAL, range_data, count, timestep_entries);

	    if (rc == RC_OK) {
	        // if(testing_logging || (zero_rank_logging && rank == 0)) {	    	
		       //  zero_rank_log << "using metadata_catalog_all_timesteps_with_var_attributes_with_type_var_substr_dims funct: timesteps associated with run_id " << run_id << ", type_id " << type_id << 
		       //  ", var_name_substr " << var_name_substr << " and txn_id: " << txn_id << " int attr data and range [" << min_range << "," << max_range << "] overlapping with dims";
		       //  for(int j=0; j< num_dims; j++) {
		       //      zero_rank_log << " d" << j << "_min: " << dims_to_search [j].min;
		       //      zero_rank_log << " d" << j << "_max: " << dims_to_search [j].max;               
		       //  }
		       //  zero_rank_log << " for server: " << rank << " \n";
	        //     print_timestep_catalog (count, timestep_entries);
	        // }
	    }
	    else {
	        error_log << "Error getting the matching timestep catalog. Proceeding \n";
	    }
	}
	add_timing_point(CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_DIMS_RANGE_DONE);	

    MPI_Barrier(MPI_COMM_WORLD); //leave to make timing clearer
	add_timing_point(CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_DIMS_RANGE_COLLECTIVE_START);	
	gather_timestep_entries(timestep_entries, rank, num_servers, num_client_procs, all_timestep_entries);
	if(testing_logging || (zero_rank_logging  && rank == 0)) {
		zero_rank_log << "rank: " << rank << " all timesteps associated with a var attribute for run_id " << run_id << 
			", type_id " << type_id << ", var_name_substr " << var_name_substr << 
			" int attr data and range [" << min_range << "," << max_range << "]" << 
			" overlapping with dims";
        for(int j=0; j< num_dims; j++) {
            zero_rank_log << " d" << j << "_min: " << dims_to_search [j].min;
            zero_rank_log << " d" << j << "_max: " << dims_to_search [j].max;               
        }
		zero_rank_log << " note: all_timestep_entries.size(): " << all_timestep_entries.size() << endl;
		print_timestep_catalog (all_timestep_entries.size(), all_timestep_entries);
	}
	add_timing_point(CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_DIMS_RANGE_COLLECTIVE_DONE);	

	all_timestep_entries.clear();

    MPI_Barrier(MPI_COMM_WORLD);
	add_timing_point(CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_DIMS_ABOVE_MAX_START);	
	if (rank < num_servers) {	
	    rc = metadata_catalog_all_timesteps_with_var_attributes_with_type_var_substr_dims_above_max (server, run_id, type_id, var_name_substr, txn_id, num_dims, dims_to_search, 
	        ATTR_DATA_TYPE_REAL, above_max_data, count, timestep_entries);

	    if (rc == RC_OK) {
	        // if(testing_logging || (zero_rank_logging && rank == 0)) {
		       //  zero_rank_log << "using metadata_catalog_all_timesteps_with_var_attributes_with_type_var_substr_dims funct: timesteps associated with run_id " << run_id << ", type_id " << type_id << 
		       //  ", var_name_substr " << var_name_substr << " and txn_id: " << txn_id << " int attr data greater than or eq to " << above_max_val << " overlapping with dims";
		       //  for(int j=0; j< num_dims; j++) {
		       //      zero_rank_log << " d" << j << "_min: " << dims_to_search [j].min;
		       //      zero_rank_log << " d" << j << "_max: " << dims_to_search [j].max;               
		       //  }
		       //  zero_rank_log << " for server: " << rank << " \n";
	        //     print_timestep_catalog (count, timestep_entries);
	        // }
	    }
	    else {
	        error_log << "Error getting the matching timestep catalog. Proceeding \n";
	    }
	}
	add_timing_point(CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_DIMS_ABOVE_MAX_DONE);	

    MPI_Barrier(MPI_COMM_WORLD); //leave to make timing clearer
	add_timing_point(CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_DIMS_ABOVE_MAX_COLLECTIVE_START);	
	gather_timestep_entries(timestep_entries, rank, num_servers, num_client_procs, all_timestep_entries);
	if(testing_logging || (zero_rank_logging  && rank == 0)) {
		zero_rank_log << "rank: " << rank << " all timesteps associated with a var attribute for run_id " << run_id << 
			", type_id " << type_id << ", var_name_substr " << var_name_substr << 
			" int attr data greater than or eq to " << above_max_val <<
			" overlapping with dims";
        for(int j=0; j< num_dims; j++) {
            zero_rank_log << " d" << j << "_min: " << dims_to_search [j].min;
            zero_rank_log << " d" << j << "_max: " << dims_to_search [j].max;               
        }
		zero_rank_log << " note: all_timestep_entries.size(): " << all_timestep_entries.size() << endl;
		print_timestep_catalog (all_timestep_entries.size(), all_timestep_entries);
	}
	add_timing_point(CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_DIMS_ABOVE_MAX_COLLECTIVE_DONE);	

	all_timestep_entries.clear();

    MPI_Barrier(MPI_COMM_WORLD);
	add_timing_point(CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_DIMS_BELOW_MIN_START);	
	if (rank < num_servers) {	
	    rc = metadata_catalog_all_timesteps_with_var_attributes_with_type_var_substr_dims_below_min (server, run_id, type_id, var_name_substr, txn_id, num_dims, dims_to_search, 
	        ATTR_DATA_TYPE_REAL, below_min_data, count, timestep_entries);

	    if (rc == RC_OK) {
	        // if(testing_logging || (zero_rank_logging && rank == 0)) {
		       //  zero_rank_log << "using metadata_catalog_all_timesteps_with_var_attributes_with_type_var_substr_dims funct: timesteps associated with run_id " << run_id << ", type_id " << type_id << 
		       //  ", var_name_substr " << var_name_substr << " and txn_id: " << txn_id << " int attr data less than or eq to " << below_min_val << " overlapping with dims";
		       //  for(int j=0; j< num_dims; j++) {
		       //      zero_rank_log << " d" << j << "_min: " << dims_to_search [j].min;
		       //      zero_rank_log << " d" << j << "_max: " << dims_to_search [j].max;               
		       //  }
		       //  zero_rank_log << " for server: " << rank << " \n";
	        //     print_timestep_catalog (count, timestep_entries);
	        // }
	    }
	    else {
	        error_log << "Error getting the matching timestep catalog. Proceeding \n";
	    }
	}
	add_timing_point(CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_DIMS_BELOW_MIN_DONE);	

    MPI_Barrier(MPI_COMM_WORLD); //leave to make timing clearer
	add_timing_point(CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_DIMS_BELOW_MIN_COLLECTIVE_START);	
	gather_timestep_entries(timestep_entries, rank, num_servers, num_client_procs, all_timestep_entries);
	if(testing_logging || (zero_rank_logging  && rank == 0)) {
		zero_rank_log << "rank: " << rank << " all timesteps associated with a var attribute for run_id " << run_id << 
			", type_id " << type_id << ", var_name_substr " << var_name_substr << 
			" int attr data less than or eq to " << below_min_val << 
			" overlapping with dims";
        for(int j=0; j< num_dims; j++) {
            zero_rank_log << " d" << j << "_min: " << dims_to_search [j].min;
            zero_rank_log << " d" << j << "_max: " << dims_to_search [j].max;               
        }
		zero_rank_log << " note: all_timestep_entries.size(): " << all_timestep_entries.size() << endl;
		print_timestep_catalog (all_timestep_entries.size(), all_timestep_entries);
	}
	add_timing_point(CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_DIMS_BELOW_MIN_COLLECTIVE_DONE);	

    return rc;
}

int catalog_var_attributes_with_var_name_substr_functs ( const md_server &server, int rank, uint32_t num_servers, uint32_t num_client_procs,
											const md_catalog_run_entry &run, const vector<md_catalog_var_entry> &vars,
											uint64_t run_id, uint64_t timestep_id,
											const string &var_name_substr, 
											int num_dims, const vector<md_dim_bounds> &dims_to_search, uint64_t txn_id,
											const vector<md_dim_bounds> &proc_dims 
											) 
{

    int rc = RC_OK;
    uint32_t count;
    vector<md_catalog_var_attribute_entry> var_attr_entries;

	vector<md_catalog_var_attribute_entry> all_var_attr_entries;

    MPI_Barrier(MPI_COMM_WORLD);
	add_timing_point(CATALOG_ALL_VAR_ATTRIBUTES_WITH_VAR_SUBSTR_START);	
	if (rank < num_servers) {
	    rc = metadata_catalog_all_var_attributes_with_var_substr (server, run_id, timestep_id, var_name_substr, txn_id, count, var_attr_entries);

	    if (rc == RC_OK) {
  	      //   if(testing_logging || (zero_rank_logging && rank == 0)) {
		       //  zero_rank_log << "using var var_attr_entries by var substr funct: attributes associated with run_id " << run_id << ", timestep_id " << timestep_id << 
		       //  	" and txn_id: " << txn_id << " and var_name_substr " << var_name_substr << " for server: " << rank << " \n";
	        //     print_var_attribute_list (count, var_attr_entries);
	        // 	print_var_attr_data(count, var_attr_entries);         
	        // }
	    }
	    else {
	        error_log << "Error metadata_catalog_all_var_attributes. Proceeding \n";
	    }
	}
	add_timing_point(CATALOG_ALL_VAR_ATTRIBUTES_WITH_VAR_SUBSTR_DONE);	

    MPI_Barrier(MPI_COMM_WORLD); //leave to make timing clearer
	add_timing_point(CATALOG_ALL_VAR_ATTRIBUTES_WITH_VAR_SUBSTR_COLLECTIVE_START);	
	gather_attr_entries(var_attr_entries, count, rank, num_servers, num_client_procs, all_var_attr_entries);
	if(testing_logging || (zero_rank_logging  && rank == 0)) {
			zero_rank_log << "rank: " << rank << " all var attributes associated with run_id " << run_id << " and timestep_id " << timestep_id << 
				" and var_name_substr " << var_name_substr << " note: all_var_attr_entries.size(): " << all_var_attr_entries.size() << endl;
		print_var_attribute_list (all_var_attr_entries.size(), all_var_attr_entries);
		print_var_attr_data(all_var_attr_entries.size(), all_var_attr_entries);
	}
	// note - have commented this out since it results in too many objects
 //    if (output_objector_params) {
 //    	count = 0;
 //        for (md_catalog_var_attribute_entry attr : all_var_attr_entries) {
 //    		objector_params my_objector_params = get_objector_params(run, vars.at(attr.var_id), attr.dims);
 //            if(my_objector_params.var_id !=attr.var_id) {
 //            	error_log << "attrs var_id: " << attr.var_id << " but given var's id: " << my_objector_params.var_id;
 //            	return RC_ERR;
 //            }
 //            else {
	//         	if (dims_overlap ( attr.dims, proc_dims) ) {
 //                	all_objector_params.push_back(my_objector_params);
 //       				count += 1;
 //       			}
 //       		}
 //       	}
 //       	add_objector_point(BOUNDING_BOX_TO_OBJ_NAMES, count);
 //    }		
	add_timing_point(CATALOG_ALL_VAR_ATTRIBUTES_WITH_VAR_SUBSTR_COLLECTIVE_DONE);	

	all_var_attr_entries.clear();

    MPI_Barrier(MPI_COMM_WORLD);
	add_timing_point(CATALOG_ALL_VAR_ATTRIBUTES_WITH_VAR_SUBSTR_DIMS_START);	
	if (rank < num_servers) {
	    rc = metadata_catalog_all_var_attributes_with_var_substr_dims (server, run_id, timestep_id, var_name_substr, txn_id, num_dims, dims_to_search, count, var_attr_entries);
	    if (rc == RC_OK) {
	        // if(testing_logging || (zero_rank_logging && rank == 0)) {
		       //  zero_rank_log << "using var var_attr_entries by var dims name ver funct: attributes associated with run_id " << run_id << ", timestep_id " << timestep_id << 
		       //  	" and txn_id: " << txn_id << " and var_name_substr " << var_name_substr << " overlapping with dims";
		       //  for(int j=0; j< num_dims; j++) {
		       //      zero_rank_log << " d" << j << "_min: " << dims_to_search [j].min;
		       //      zero_rank_log << " d" << j << "_max: " << dims_to_search [j].max;               
		       //  }
		       //  zero_rank_log << " for server: " << rank << " \n";
	        //     print_var_attribute_list (count, var_attr_entries);
	        // 	print_var_attr_data(count, var_attr_entries);         
	        // }
	    }
	    else {
	        error_log << "Error metadata_catalog_all_var_attributes_with_var_substr_dims. Proceeding \n";
	    }
	}
	add_timing_point(CATALOG_ALL_VAR_ATTRIBUTES_WITH_VAR_SUBSTR_DIMS_DONE);	

    MPI_Barrier(MPI_COMM_WORLD); //leave to make timing clearer	
	add_timing_point(CATALOG_ALL_VAR_ATTRIBUTES_WITH_VAR_SUBSTR_DIMS_COLLECTIVE_START);	
	gather_attr_entries(var_attr_entries, count, rank, num_servers, num_client_procs, all_var_attr_entries);
	if(testing_logging || (zero_rank_logging  && rank == 0)) {
		zero_rank_log << "rank: " << rank << " all var attributes associated with run_id " << run_id << " and timestep_id " << timestep_id << 
			" and var_name_substr " << var_name_substr << " overlapping with dims";
        for(int j=0; j< num_dims; j++) {
            zero_rank_log << " d" << j << "_min: " << dims_to_search [j].min;
            zero_rank_log << " d" << j << "_max: " << dims_to_search [j].max;               
        }
		zero_rank_log << " note: all_var_attr_entries.size(): " << all_var_attr_entries.size() << endl;
		print_var_attribute_list (all_var_attr_entries.size(), all_var_attr_entries);
		print_var_attr_data(all_var_attr_entries.size(), all_var_attr_entries);
	}
	// note - have commented this out since it results in too many objects
  //  if (output_objector_params) {
  //       count = 0;
  //       for (md_catalog_var_attribute_entry attr : all_var_attr_entries) {
  //   		objector_params my_objector_params = get_objector_params(run, vars.at(attr.var_id), attr.dims);
  //           if(my_objector_params.var_id !=attr.var_id) {
  //           	error_log << "attrs var_id: " << attr.var_id << " but given var's id: " << my_objector_params.var_id;
  //           	return RC_ERR;
  //           }
  //           else {
	 //        	if (dims_overlap ( attr.dims, proc_dims) ) {
  //               	all_objector_params.push_back(my_objector_params);
  //      				count += 1;
  //      			}
  //      		}
		// }
  //      	add_objector_point(BOUNDING_BOX_TO_OBJ_NAMES, count);
  //   }		
	add_timing_point(CATALOG_ALL_VAR_ATTRIBUTES_WITH_VAR_SUBSTR_DIMS_COLLECTIVE_DONE);	

    return rc;
}

int catalog_var_attributes_with_type_var_name_substr_or_val_functs ( const md_server &server, int rank, uint32_t num_servers, uint32_t num_client_procs,
													const md_catalog_run_entry &run, const vector<md_catalog_var_entry> &vars,
													uint64_t run_id, uint64_t timestep_id,
													const string &var_name_substr, uint64_t rare_type_id,
                                                    uint64_t val_type_id, int num_dims, const vector<md_dim_bounds> &dims_to_search,
                                                    uint64_t txn_id,
                                                    double min_range, double max_range, double above_max_val, double below_min_val,
                                                    const vector<md_dim_bounds> &proc_dims
                                                    ) 
{

    int rc = RC_OK;
    uint32_t count;
    vector<md_catalog_var_attribute_entry> var_attr_entries;

	vector<md_catalog_var_attribute_entry> all_var_attr_entries;

	MPI_Barrier(MPI_COMM_WORLD);
	add_timing_point(CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_START);	
	if(rank < num_servers) {
	    rc = metadata_catalog_all_var_attributes_with_type_var_substr (server, run_id, timestep_id, rare_type_id, var_name_substr, txn_id, count, var_attr_entries);
	    if (rc == RC_OK) {
	        // if(testing_logging || (zero_rank_logging && rank == 0)) {
		       //  zero_rank_log << "using var var_attr_entries with type var by name ver funct: attributes associated with run_id " << run_id << ", timestep_id " << 
		       //  	timestep_id << " and txn_id: " << txn_id << " and type_id " << type_id <<
		       //  	" and var_substr " << var_name_substr;
		       //  zero_rank_log << " for server: " << rank << " \n";
	        //     print_var_attribute_list (count, var_attr_entries);
	        //     print_var_attr_data(count, var_attr_entries);
	        // }
	    }
	    else {
	        error_log << "Error metadata_catalog_all_var_attributes_with_type_var_substr. Proceeding \n";
	    }
	}
	add_timing_point(CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_DONE);	

	MPI_Barrier(MPI_COMM_WORLD); //leave to make timing clearer
	add_timing_point(CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_COLLECTIVE_START);	
	gather_attr_entries(var_attr_entries, count, rank, num_servers, num_client_procs, all_var_attr_entries);
	if(testing_logging || (zero_rank_logging  && rank == 0)) {
		zero_rank_log << "rank: " << rank << " all var attributes associated with run_id " << run_id << " and timestep_id " << timestep_id << 
			" type: " << rare_type_id << 
			" and var_substr " << var_name_substr << " note: all_var_attr_entries.size(): " << all_var_attr_entries.size() << endl;
		print_var_attribute_list (all_var_attr_entries.size(), all_var_attr_entries);
		print_var_attr_data(all_var_attr_entries.size(), all_var_attr_entries);
	}
   if (output_objector_params) {
        count = 0;
        for (md_catalog_var_attribute_entry attr : all_var_attr_entries) {
        	if (dims_overlap ( attr.dims, proc_dims) ) {
        		extreme_debug_log << "found a bb for rank " << rank << endl;
    			objector_params my_objector_params = get_objector_params(run, vars.at(attr.var_id), attr.dims, CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR, proc_dims);
            	all_objector_params.push_back(my_objector_params);
   				count += 1;
   			}
            // if(my_objector_params.var_id !=attr.var_id) {
            // 	error_log << "attrs var_id: " << attr.var_id << " but given var's id: " << my_objector_params.var_id;
            // 	return RC_ERR;
            // }
		}
       	add_objector_point(BOUNDING_BOX_TO_OBJ_NAMES, count);
    }	
	else if (output_obj_names) {
	    for (md_catalog_var_attribute_entry attr : all_var_attr_entries) {
	   		if (dims_overlap ( attr.dims, proc_dims) ) {
	            rc = add_object_names_offests_and_counts(run, vars.at(attr.var_id), get_overlapping_dims_for_bounding_box(attr.dims, proc_dims), CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR);  
	            if (rc != RC_OK) {
	                error_log << "error in add_object_names_offests_and_counts \n";
	                return rc;
	            }
	            // NOTE: THIS IS WHERE DATA WOULD BE READ
	       		extreme_debug_log << "for rank: " << rank << " found a matching bb" << endl;
	        }
	    }
	}	
	else if (hdf5_read_data) {
		read_data_for_attrs(run, vars, all_var_attr_entries, proc_dims);
	}
	add_timing_point(CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_COLLECTIVE_DONE);	

    string range_data;
    make_range_data (min_range, max_range, range_data);

	all_var_attr_entries.clear();

	MPI_Barrier(MPI_COMM_WORLD);
	add_timing_point(CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_RANGE_START);	
	if (rank < num_servers) {
	    rc = metadata_catalog_all_var_attributes_with_type_var_substr_range (server, timestep_id, val_type_id, var_name_substr, txn_id, 
	        ATTR_DATA_TYPE_REAL, range_data, count, var_attr_entries);
	    if (rc == RC_OK) {
	        // if(testing_logging || (zero_rank_logging && rank == 0)) {
		       //  zero_rank_log << "using var var_attr_entries with type var by id funct: attributes associated with run_id " << run_id << ", timestep_id " << 
		       //  timestep_id << " and txn_id: " << txn_id << " and type_id " << val_type_id <<
		       //  " and var_substr " << var_name_substr << " int attr data and range [" << min_range << "," << max_range << "]" <<
		       //  " for server: " << rank << " \n";
	        //     print_var_attribute_list (count, var_attr_entries);
	        //     print_var_attr_data(count, var_attr_entries);
	        // }
	    }
	    else {
	        error_log << "Error metadata_catalog_all_var_attributes_with_type_var_substr_range. Proceeding \n";
	    }
	}
	add_timing_point(CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_RANGE_DONE);	

	MPI_Barrier(MPI_COMM_WORLD); //leave to make timing clearer
	add_timing_point(CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_RANGE_COLLECTIVE_START);	
	gather_attr_entries(var_attr_entries, count, rank, num_servers, num_client_procs, all_var_attr_entries);
	if(testing_logging || (zero_rank_logging  && rank == 0)) {
		zero_rank_log << "rank: " << rank << " all var attributes associated with run_id " << run_id << " and timestep_id " << timestep_id << 
			" type: " << val_type_id << 
			" and var_substr " << var_name_substr << " int attr data and range [" << min_range << "," << max_range << "]" <<
			" note: all_var_attr_entries.size(): " << all_var_attr_entries.size() << endl;
		print_var_attribute_list (all_var_attr_entries.size(), all_var_attr_entries);
		print_var_attr_data(all_var_attr_entries.size(), all_var_attr_entries);
	}
   if (output_objector_params) {
        count = 0;
        for (md_catalog_var_attribute_entry attr : all_var_attr_entries) {
           	if (dims_overlap ( attr.dims, proc_dims) ) {
        		extreme_debug_log << "found a bb for rank " << rank << endl;
    			objector_params my_objector_params = get_objector_params(run, vars.at(attr.var_id), attr.dims, CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_RANGE, proc_dims);
            	all_objector_params.push_back(my_objector_params);
   				count += 1;
   			}
            // if(my_objector_params.var_id !=attr.var_id) {
            // 	error_log << "attrs var_id: " << attr.var_id << " but given var's id: " << my_objector_params.var_id;
            // 	return RC_ERR;
            // }
		}
       	add_objector_point(BOUNDING_BOX_TO_OBJ_NAMES, count);
    }		
	else if (output_obj_names) {
	    for (md_catalog_var_attribute_entry attr : all_var_attr_entries) {
	   		if (dims_overlap ( attr.dims, proc_dims) ) {
	            rc = add_object_names_offests_and_counts(run, vars.at(attr.var_id), get_overlapping_dims_for_bounding_box(attr.dims, proc_dims), CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_RANGE);  
	            if (rc != RC_OK) {
	                error_log << "error in add_object_names_offests_and_counts \n";
	                return rc;
	            }
	            // NOTE: THIS IS WHERE DATA WOULD BE READ
	       		extreme_debug_log << "for rank: " << rank << " found a matching bb" << endl;
	        }
	    }
	}    
	else if (hdf5_read_data) {
		read_data_for_attrs(run, vars, all_var_attr_entries, proc_dims);
	}
	add_timing_point(CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_RANGE_COLLECTIVE_DONE);	

    string above_max_data;
    make_single_val_data (above_max_val, above_max_data);

	all_var_attr_entries.clear();

	MPI_Barrier(MPI_COMM_WORLD);
	add_timing_point(CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_ABOVE_MAX_START);	
	if (rank < num_servers) {    
	    rc = metadata_catalog_all_var_attributes_with_type_var_substr_above_max (server, timestep_id, val_type_id, var_name_substr, txn_id, 
	        ATTR_DATA_TYPE_REAL, above_max_data, count, var_attr_entries);

	    if (rc == RC_OK) {
	        // if(testing_logging || (zero_rank_logging && rank == 0)) {
		       //  zero_rank_log << "using metadata_catalog_all_var_attributes_with_type_var_substr_above_max funct: attributes associated with run_id " << run_id << ", timestep_id " << 
		       //  	timestep_id << " and txn_id: " << txn_id << " and type_id " << type_id <<
			      //   " and var_substr " << var_name_substr << " int attr data with val geq " << above_max_val << 
			      //   " for server: " << rank << "\n";
	        //     print_var_attribute_list (count, var_attr_entries);
	        //     print_var_attr_data(count, var_attr_entries);
	        // }
	    }
	    else {
	        error_log << "Error metadata_catalog_all_var_attributes_with_type_var_substr_above_max. Proceeding \n";
	    } 
	}
	add_timing_point(CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_ABOVE_MAX_DONE);	

	MPI_Barrier(MPI_COMM_WORLD); //leave to make timing clearer	
	add_timing_point(CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_ABOVE_MAX_COLLECTIVE_START);	
	gather_attr_entries(var_attr_entries, count, rank, num_servers, num_client_procs, all_var_attr_entries);
	if(testing_logging || (zero_rank_logging  && rank == 0)) {
		zero_rank_log << "rank: " << rank << " all var attributes associated with run_id " << run_id << " and timestep_id " << timestep_id << 
			" type: " << val_type_id << 
			" and var_substr " << var_name_substr << " int attr data with val geq " << above_max_val << 
			" note: all_var_attr_entries.size(): " << all_var_attr_entries.size() << endl;
		print_var_attribute_list (all_var_attr_entries.size(), all_var_attr_entries);
		print_var_attr_data(all_var_attr_entries.size(), all_var_attr_entries);
	}
   if (output_objector_params) {
        count = 0;
        for (md_catalog_var_attribute_entry attr : all_var_attr_entries) {
    		if (dims_overlap ( attr.dims, proc_dims) ) {
        		extreme_debug_log << "found a bb for rank " << rank << endl;
    			objector_params my_objector_params = get_objector_params(run, vars.at(attr.var_id), attr.dims, CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_ABOVE_MAX, proc_dims);
            	all_objector_params.push_back(my_objector_params);
            	count += 1;
            }    		
            // if(my_objector_params.var_id !=attr.var_id) {
            // 	error_log << "attrs var_id: " << attr.var_id << " but given var's id: " << my_objector_params.var_id;
            // 	return RC_ERR;
            // }
		}
       	add_objector_point(BOUNDING_BOX_TO_OBJ_NAMES, count);
    }	
	else if (output_obj_names) {
	    for (md_catalog_var_attribute_entry attr : all_var_attr_entries) {
	   		if (dims_overlap ( attr.dims, proc_dims) ) {
	            rc = add_object_names_offests_and_counts(run, vars.at(attr.var_id), get_overlapping_dims_for_bounding_box(attr.dims, proc_dims), CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_ABOVE_MAX);  
	            if (rc != RC_OK) {
	                error_log << "error in add_object_names_offests_and_counts \n";
	                return rc;
	            }
	            // NOTE: THIS IS WHERE DATA WOULD BE READ
	       		extreme_debug_log << "for rank: " << rank << " found a matching bb" << endl;
	        }
	    }
	}	
	else if (hdf5_read_data) {
		read_data_for_attrs(run, vars, all_var_attr_entries, proc_dims);
	}
	add_timing_point(CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_ABOVE_MAX_COLLECTIVE_DONE);	

    string below_min_data;
    make_single_val_data (below_min_val, below_min_data);

	all_var_attr_entries.clear();

	MPI_Barrier(MPI_COMM_WORLD);	
	add_timing_point(CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_BELOW_MIN_START);	
	if (rank < num_servers) {    
	    rc = metadata_catalog_all_var_attributes_with_type_var_substr_below_min (server, timestep_id, val_type_id, var_name_substr, txn_id, 
	        ATTR_DATA_TYPE_REAL, below_min_data, count, var_attr_entries);

	    if (rc == RC_OK) {
	 	    // if(testing_logging || (zero_rank_logging && rank == 0)) {
		     //    zero_rank_log << "using metadata_catalog_all_var_attributes_with_type_var_substr_below_min funct: attributes associated with run_id " << run_id << ", timestep_id " << 
		     //    	timestep_id << " and txn_id: " << txn_id << " and type_id " << val_type_id <<
		     //    	" and var_substr " << var_name_substr << " int attr data with val leq " << below_min_val << 
	      //   		" for server: " << rank << "\n";
	      //       print_var_attribute_list (count, var_attr_entries);
	      //       print_var_attr_data(count, var_attr_entries);
	      //   }
	    }
	    else {
	        error_log << "Error metadata_catalog_all_var_attributes_with_type_var_substr_below_min. Proceeding \n";
	    }
	}
	add_timing_point(CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_BELOW_MIN_DONE);	

	MPI_Barrier(MPI_COMM_WORLD); //leave to make timing clearer	
	add_timing_point(CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_BELOW_MIN_COLLECTIVE_START);	
	gather_attr_entries(var_attr_entries, count, rank, num_servers, num_client_procs, all_var_attr_entries);
	if(testing_logging || (zero_rank_logging  && rank == 0)) {
		zero_rank_log << "rank: " << rank << " all var attributes associated with run_id " << run_id << " and timestep_id " << timestep_id << 
			" type: " << val_type_id << 
			" and var_substr " << var_name_substr << " int attr data with val leq " << below_min_val << 
			" note: all_var_attr_entries.size(): " << all_var_attr_entries.size() << endl;
		print_var_attribute_list (all_var_attr_entries.size(), all_var_attr_entries);
		print_var_attr_data(all_var_attr_entries.size(), all_var_attr_entries);
	}
   if (output_objector_params) {
        count = 0;
        for (md_catalog_var_attribute_entry attr : all_var_attr_entries) {
        	if (dims_overlap ( attr.dims, proc_dims) ) {
	    		extreme_debug_log << "found a bb for rank " << rank << endl;
    			objector_params my_objector_params = get_objector_params(run, vars.at(attr.var_id), attr.dims, CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_BELOW_MIN, proc_dims);	        	
	        	all_objector_params.push_back(my_objector_params);
				count += 1;
			}   		            
            // if(my_objector_params.var_id !=attr.var_id) {
            // 	error_log << "attrs var_id: " << attr.var_id << " but given var's id: " << my_objector_params.var_id;
            // 	return RC_ERR;
            // }
		}
       	add_objector_point(BOUNDING_BOX_TO_OBJ_NAMES, count);
    }		
	else if (output_obj_names) {
	    for (md_catalog_var_attribute_entry attr : all_var_attr_entries) {
	   		if (dims_overlap ( attr.dims, proc_dims) ) {
	            rc = add_object_names_offests_and_counts(run, vars.at(attr.var_id), get_overlapping_dims_for_bounding_box(attr.dims, proc_dims), CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_BELOW_MIN);  
	            if (rc != RC_OK) {
	                error_log << "error in add_object_names_offests_and_counts \n";
	                return rc;
	            }
	            // NOTE: THIS IS WHERE DATA WOULD BE READ
	       		extreme_debug_log << "for rank: " << rank << " found a matching bb" << endl;
	        }
	    }
	}
	else if (hdf5_read_data) {
		read_data_for_attrs(run, vars, all_var_attr_entries, proc_dims);
	}
	add_timing_point(CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_BELOW_MIN_COLLECTIVE_DONE);	

	all_var_attr_entries.clear();

	MPI_Barrier(MPI_COMM_WORLD);	
	add_timing_point(CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_DIMS_START);	
	if (rank < num_servers) {
	    rc = metadata_catalog_all_var_attributes_with_type_var_substr_dims (server, run_id, timestep_id, rare_type_id, 
	    	var_name_substr, txn_id, num_dims, dims_to_search, count, var_attr_entries);
	    if (rc == RC_OK) {
	        // if(testing_logging || (zero_rank_logging && rank == 0)) {
		       //  zero_rank_log << "using metadata_catalog_all_var_attributes_with_type_var_substr_dims funct: " << 
	    	   		//	"attributes associated with run_id " << run_id << ", timestep_id " << timestep_id << " and txn_id: " << txn_id << " and type_id " << rare_type_id <<
		       //  " and var_substr " << var_name_substr << " overlapping with dims";
		       //  for(int j=0; j< num_dims; j++) {
		       //      zero_rank_log << " d" << j << "_min: " << dims_to_search [j].min;
		       //      zero_rank_log << " d" << j << "_max: " << dims_to_search [j].max;               
		       //  }
		       //  zero_rank_log << " for server: " << rank << " \n";
	        //     print_var_attribute_list (count, var_attr_entries);
	        //     print_var_attr_data(count, var_attr_entries);
	        // }
	    }
	    else {
	        error_log << "Error metadata_catalog_all_var_attributes_with_type_var_substr_dims" << " \n";
	    }
	}
	add_timing_point(CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_DIMS_DONE);	

	MPI_Barrier(MPI_COMM_WORLD); //leave to make timing clearer	
	add_timing_point(CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_DIMS_COLLECTIVE_START);	
	gather_attr_entries(var_attr_entries, count, rank, num_servers, num_client_procs, all_var_attr_entries);
	if(testing_logging || (zero_rank_logging  && rank == 0)) {
		zero_rank_log << "rank: " << rank << " all var attributes associated with run_id " << run_id << " and timestep_id " << timestep_id << 
			" type: " << rare_type_id << 
			" and var_substr " << var_name_substr << " overlapping with dims";
        for(int j=0; j< num_dims; j++) {
            zero_rank_log << " d" << j << "_min: " << dims_to_search [j].min;
            zero_rank_log << " d" << j << "_max: " << dims_to_search [j].max;               
        }
		zero_rank_log << " note: all_var_attr_entries.size(): " << all_var_attr_entries.size() << endl;
		print_var_attribute_list (all_var_attr_entries.size(), all_var_attr_entries);
		print_var_attr_data(all_var_attr_entries.size(), all_var_attr_entries);
	}
   if (output_objector_params) {
        count = 0;
        for (md_catalog_var_attribute_entry attr : all_var_attr_entries) {
        	if (dims_overlap ( attr.dims, proc_dims) ) {
        		extreme_debug_log << "found a bb for rank " << rank << endl;
    			objector_params my_objector_params = get_objector_params(run, vars.at(attr.var_id), attr.dims, CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_DIMS, proc_dims);
            	all_objector_params.push_back(my_objector_params);
   				count += 1;
   			}    		
            // if(my_objector_params.var_id !=attr.var_id) {
            // 	error_log << "attrs var_id: " << attr.var_id << " but given var's id: " << my_objector_params.var_id;
            // 	return RC_ERR;
            // }
		}
       	add_objector_point(BOUNDING_BOX_TO_OBJ_NAMES, count);
    }
	else if (output_obj_names) {
	    for (md_catalog_var_attribute_entry attr : all_var_attr_entries) {
	   		if (dims_overlap ( attr.dims, proc_dims) ) {
	            rc = add_object_names_offests_and_counts(run, vars.at(attr.var_id), get_overlapping_dims_for_bounding_box(attr.dims, proc_dims), CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_DIMS);  
	            if (rc != RC_OK) {
	                error_log << "error in add_object_names_offests_and_counts \n";
	                return rc;
	            }
	            // NOTE: THIS IS WHERE DATA WOULD BE READ
	       		extreme_debug_log << "for rank: " << rank << " found a matching bb" << endl;
	        }
	    }
	}
	else if (hdf5_read_data) {
		read_data_for_attrs(run, vars, all_var_attr_entries, proc_dims);
	}	
	add_timing_point(CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_DIMS_COLLECTIVE_DONE);	

	all_var_attr_entries.clear();

	MPI_Barrier(MPI_COMM_WORLD);	
	add_timing_point(CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_DIMS_RANGE_START);	
	if (rank < num_servers) {
	    rc = metadata_catalog_all_var_attributes_with_type_var_substr_dims_range (server, timestep_id, val_type_id, var_name_substr, txn_id , 
	        num_dims, dims_to_search, ATTR_DATA_TYPE_REAL, range_data, count, var_attr_entries);
	    if (rc == RC_OK) {
	        // if(testing_logging || (zero_rank_logging && rank == 0)) {
		       //  zero_rank_log << "using metadata_catalog_all_var_attributes_with_type_var_substr_dims_range funct: attributes associated with run_id " << run_id << ", timestep_id " << 
			      //   timestep_id << " and txn_id: " << txn_id << " and type_id " << val_type_id <<
			      //   " and var_substr " << var_name_substr << " int attr data and range [" << min_range << "," << max_range << "]" <<
			      //   " overlapping with dims";
		       //  for(int j=0; j< num_dims; j++) {
		       //      zero_rank_log << " d" << j << "_min: " << dims_to_search [j].min;
		       //      zero_rank_log << " d" << j << "_max: " << dims_to_search [j].max;               
		       //  }
		       //  zero_rank_log << " for server: " << rank << " \n";
	        //     print_var_attribute_list (count, var_attr_entries);
	        //     print_var_attr_data(count, var_attr_entries);
	        // }
	    }
	    else {
	        error_log << "Error metadata_catalog_all_var_attributes_with_type_var_substr_dims_range. Proceeding \n";
	    }
	}
	add_timing_point(CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_DIMS_RANGE_DONE);	

	MPI_Barrier(MPI_COMM_WORLD); //leave to make timing clearer	
	add_timing_point(CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_DIMS_RANGE_COLLECTIVE_START);	
	gather_attr_entries(var_attr_entries, count, rank, num_servers, num_client_procs, all_var_attr_entries);
	if(testing_logging || (zero_rank_logging  && rank == 0)) {
		zero_rank_log << "rank: " << rank << " all var attributes associated with run_id " << run_id << " and timestep_id " << timestep_id << 
			" type: " << val_type_id << 
			" and var_substr " << var_name_substr << " int attr data and range [" << min_range << "," << max_range << "]" <<
			" overlapping with dims";
        for(int j=0; j< num_dims; j++) {
            zero_rank_log << " d" << j << "_min: " << dims_to_search [j].min;
            zero_rank_log << " d" << j << "_max: " << dims_to_search [j].max;               
        }
		zero_rank_log << " note: all_var_attr_entries.size(): " << all_var_attr_entries.size() << endl;
		print_var_attribute_list (all_var_attr_entries.size(), all_var_attr_entries);
		print_var_attr_data(all_var_attr_entries.size(), all_var_attr_entries);
	}
    if (output_objector_params) {
    	count = 0;
        for (md_catalog_var_attribute_entry attr : all_var_attr_entries) {
        	if (dims_overlap ( attr.dims, proc_dims) ) {
        		extreme_debug_log << "found a bb for rank " << rank << endl;    			
    			objector_params my_objector_params = get_objector_params(run, vars.at(attr.var_id), attr.dims, CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_DIMS_RANGE, proc_dims );
            	all_objector_params.push_back(my_objector_params);
   				count += 1;
   			}
            // if(my_objector_params.var_id !=attr.var_id) {
            // 	error_log << "attrs var_id: " << attr.var_id << " but given var's id: " << my_objector_params.var_id;
            // 	return RC_ERR;
            // }
		}
       	add_objector_point(BOUNDING_BOX_TO_OBJ_NAMES, count);
    }
	else if (output_obj_names) {
	    for (md_catalog_var_attribute_entry attr : all_var_attr_entries) {
	   		if (dims_overlap ( attr.dims, proc_dims) ) {
	            rc = add_object_names_offests_and_counts(run, vars.at(attr.var_id), get_overlapping_dims_for_bounding_box(attr.dims, proc_dims), CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_DIMS_RANGE);  
	            if (rc != RC_OK) {
	                error_log << "error in add_object_names_offests_and_counts \n";
	                return rc;
	            }
	            // NOTE: THIS IS WHERE DATA WOULD BE READ
	       		extreme_debug_log << "for rank: " << rank << " found a matching bb" << endl;
	        }
	    }
	}	
	else if (hdf5_read_data) {
		read_data_for_attrs(run, vars, all_var_attr_entries, proc_dims);
	}	
	add_timing_point(CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_DIMS_RANGE_COLLECTIVE_DONE);	

	all_var_attr_entries.clear();

	MPI_Barrier(MPI_COMM_WORLD);	
	add_timing_point(CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_DIMS_ABOVE_MAX_START);	
	if (rank < num_servers) {
	    rc = metadata_catalog_all_var_attributes_with_type_var_substr_dims_above_max (server, timestep_id, val_type_id, var_name_substr, txn_id , 
	        num_dims, dims_to_search, ATTR_DATA_TYPE_REAL, above_max_data, count, var_attr_entries);
	    if (rc == RC_OK) {
	        // if(testing_logging || (zero_rank_logging && rank == 0)) {	    	
		       //  zero_rank_log << "using metadata_catalog_all_var_attributes_with_type_var_substr_dims_above_max funct: attributes associated with run_id " << run_id << ", timestep_id " << 
		       //  	timestep_id << " and txn_id: " << txn_id << " and type_id " << type_id <<
			      //   " and var_substr " << var_name_substr << " int attr data and val geq " << above_max_val <<
			      //   " overlapping with dims";
		       //  for(int j=0; j< num_dims; j++) {
		       //      zero_rank_log << " d" << j << "_min: " << dims_to_search [j].min;
		       //      zero_rank_log << " d" << j << "_max: " << dims_to_search [j].max;               
		       //  }
		       //  zero_rank_log << " for server: " << rank << " \n";
	        //     print_var_attribute_list (count, var_attr_entries);
	        //     print_var_attr_data(count, var_attr_entries);
	        // }
	    }
	    else {
	        error_log << "Error metadata_catalog_all_var_attributes_with_type_var_substr_dims_above_max. Proceeding \n";
	    }
	}
	add_timing_point(CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_DIMS_ABOVE_MAX_DONE);	

	MPI_Barrier(MPI_COMM_WORLD); //leave to make timing clearer	
	add_timing_point(CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_DIMS_ABOVE_MAX_COLLECTIVE_START);	
	gather_attr_entries(var_attr_entries, count, rank, num_servers, num_client_procs, all_var_attr_entries);
	if(testing_logging || (zero_rank_logging  && rank == 0)) {
		zero_rank_log << "rank: " << rank << " all var attributes associated with run_id " << run_id << " and timestep_id " << timestep_id << 
			" type: " << val_type_id << 
			" and var_substr " << var_name_substr << " int attr data and val geq " << above_max_val <<
			" overlapping with dims";
        for(int j=0; j< num_dims; j++) {
            zero_rank_log << " d" << j << "_min: " << dims_to_search [j].min;
            zero_rank_log << " d" << j << "_max: " << dims_to_search [j].max;               
        }
		zero_rank_log << " note: all_var_attr_entries.size(): " << all_var_attr_entries.size() << endl;
		print_var_attribute_list (all_var_attr_entries.size(), all_var_attr_entries);
		print_var_attr_data(all_var_attr_entries.size(), all_var_attr_entries);
	}   
   if (output_objector_params) {
        count = 0;
        for (md_catalog_var_attribute_entry attr : all_var_attr_entries) {
        	if (dims_overlap ( attr.dims, proc_dims) ) {
        		extreme_debug_log << "found a bb for rank " << rank << endl;
    			objector_params my_objector_params = get_objector_params(run, vars.at(attr.var_id), attr.dims, CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_DIMS_ABOVE_MAX, proc_dims );
            	all_objector_params.push_back(my_objector_params);
   				count += 1;
   			}    		
            // if(my_objector_params.var_id !=attr.var_id) {
            // 	error_log << "attrs var_id: " << attr.var_id << " but given var's id: " << my_objector_params.var_id;
            // 	return RC_ERR;
            // }
		}
       	add_objector_point(BOUNDING_BOX_TO_OBJ_NAMES, count);
    }
	else if (output_obj_names) {
	    for (md_catalog_var_attribute_entry attr : all_var_attr_entries) {
	   		if (dims_overlap ( attr.dims, proc_dims) ) {
	            rc = add_object_names_offests_and_counts(run, vars.at(attr.var_id), get_overlapping_dims_for_bounding_box(attr.dims, proc_dims), CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_DIMS_ABOVE_MAX);  
	            if (rc != RC_OK) {
	                error_log << "error in add_object_names_offests_and_counts \n";
	                return rc;
	            }
	            // NOTE: THIS IS WHERE DATA WOULD BE READ
	       		extreme_debug_log << "for rank: " << rank << " found a matching bb" << endl;
	        }
	    }
	}
	else if (hdf5_read_data) {
		read_data_for_attrs(run, vars, all_var_attr_entries, proc_dims);
	}		 
	add_timing_point(CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_DIMS_ABOVE_MAX_COLLECTIVE_DONE);	

	all_var_attr_entries.clear();

	MPI_Barrier(MPI_COMM_WORLD);	
	add_timing_point(CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_DIMS_BELOW_MIN_START);	
	if (rank < num_servers) {
	    rc = metadata_catalog_all_var_attributes_with_type_var_substr_dims_below_min (server, timestep_id, val_type_id, var_name_substr, txn_id , 
	        num_dims, dims_to_search, ATTR_DATA_TYPE_REAL, below_min_data, count, var_attr_entries);
	    if (rc == RC_OK) {
	        // if(testing_logging || (zero_rank_logging && rank == 0)) {
		       //  zero_rank_log << "using metadata_catalog_all_var_attributes_with_type_var_substr_dims_below_min funct: attributes associated with run_id " << run_id << ", timestep_id " << 
			      //   timestep_id << " and txn_id: " << txn_id << " and type_id " << type_id <<
			      //   " and var_substr " << var_name_substr << " int attr data and val leq " << below_min_val <<
			      //   " overlapping with dims";
		       //  for(int j=0; j< num_dims; j++) {
		       //      zero_rank_log << " d" << j << "_min: " << dims_to_search [j].min;
		       //      zero_rank_log << " d" << j << "_max: " << dims_to_search [j].max;               
		       //  }
		       //  zero_rank_log << " for server: " << rank << " \n";
	        //     print_var_attribute_list (count, var_attr_entries);
	        //     print_var_attr_data(count, var_attr_entries);
	        // }
	    }
	    else {
	        error_log << "Error metadata_catalog_all_var_attributes_with_type_var_substr_dims_below_min. Proceeding \n";
	    }
	}
	add_timing_point(CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_DIMS_BELOW_MIN_DONE);

	MPI_Barrier(MPI_COMM_WORLD); //leave to make timing clearer	
	add_timing_point(CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_DIMS_BELOW_MIN_COLLECTIVE_START);	
	gather_attr_entries(var_attr_entries, count, rank, num_servers, num_client_procs, all_var_attr_entries);
	if(testing_logging || (zero_rank_logging  && rank == 0)) {
		zero_rank_log << "rank: " << rank << " all var attributes associated with run_id " << run_id << " and timestep_id " << timestep_id << 
			" type: " << val_type_id << 
			" and var_substr " << var_name_substr << " int attr data and val leq " << below_min_val <<
			" overlapping with dims";
        for(int j=0; j< num_dims; j++) {
            zero_rank_log << " d" << j << "_min: " << dims_to_search [j].min;
            zero_rank_log << " d" << j << "_max: " << dims_to_search [j].max;               
        }
		zero_rank_log << " note: all_var_attr_entries.size(): " << all_var_attr_entries.size() << endl;
		print_var_attribute_list (all_var_attr_entries.size(), all_var_attr_entries);
		print_var_attr_data(all_var_attr_entries.size(), all_var_attr_entries);
	}    
   if (output_objector_params) {
        count = 0;
        for (md_catalog_var_attribute_entry attr : all_var_attr_entries) {
        	if (dims_overlap ( attr.dims, proc_dims) ) {
        		extreme_debug_log << "found a bb for rank " << rank << endl;
    			objector_params my_objector_params = get_objector_params(run, vars.at(attr.var_id), attr.dims, CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_DIMS_BELOW_MIN, proc_dims);
            	all_objector_params.push_back(my_objector_params);
   				count += 1;
   			}    		
            // if(my_objector_params.var_id !=attr.var_id) {
            // 	error_log << "attrs var_id: " << attr.var_id << " but given var's id: " << my_objector_params.var_id;
            // 	return RC_ERR;
            // }
		}
       	add_objector_point(BOUNDING_BOX_TO_OBJ_NAMES, count);
    }		
	else if (output_obj_names) {
	    for (md_catalog_var_attribute_entry attr : all_var_attr_entries) {
	   		if (dims_overlap ( attr.dims, proc_dims) ) {
	            rc = add_object_names_offests_and_counts(run, vars.at(attr.var_id), get_overlapping_dims_for_bounding_box(attr.dims, proc_dims), CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_DIMS_BELOW_MIN);  
	            if (rc != RC_OK) {
	                error_log << "error in add_object_names_offests_and_counts \n";
	                return rc;
	            }
	            // NOTE: THIS IS WHERE DATA WOULD BE READ
	       		extreme_debug_log << "for rank: " << rank << " found a matching bb" << endl;
	        }
	    }
	}
	else if (hdf5_read_data) {
		read_data_for_attrs(run, vars, all_var_attr_entries, proc_dims);
	}
	add_timing_point(CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_DIMS_BELOW_MIN_COLLECTIVE_DONE);	

    return rc;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//helper functions//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template <class T1, class T2>
static void make_range_data (T1 min_int, T2 max_int, string &serial_str) {
    stringstream ss;
    boost::archive::text_oarchive oa(ss);
    oa << min_int;
    oa << max_int;
    serial_str = ss.str();
}

template <class T2>
static void make_single_val_data (T2 above_max_val, string &serial_str) {
    stringstream ss;
    boost::archive::text_oarchive oa(ss);
    oa << above_max_val;
    serial_str = ss.str();
}

// string to_upper (string above_max_val) {
//     string new_str = above_max_val;
//     for (int i =0; i<above_max_val.size(); i++) {
//         new_str[i] = toupper(new_str[i]);
//     }
//     return new_str;
// }


template <class T>
void gather_attr_entries(vector<T> attr_entries, uint32_t count, int rank, uint32_t num_servers, uint32_t num_client_procs,
		vector<T> &all_attr_entries) {

	add_timing_point(GATHER_ATTR_ENTRIES_START);

	extreme_debug_log.set_rank(rank);

	int length_ser_c_str = 0;
	char *serialized_c_str;
	int each_proc_ser_attr_entries_size[num_client_procs];
	int displacement_for_each_proc[num_client_procs];

	char *serialized_c_str_all_attr_entries;		

	extreme_debug_log << "rank " << rank << " attr_entries.size(): " <<attr_entries.size() << endl;

	if (rank < num_servers && attr_entries.size() > 0) {
	    stringstream ss;
		boost::archive::text_oarchive oa(ss);
		// extreme_debug_log << "rank " << rank << " about to try serializing at the beginning" << endl;
	    oa << attr_entries;
	  	// extreme_debug_log << "rank " << rank << " just finished serializing at the beginning" << endl;
	    string serialized_str = ss.str();
	    length_ser_c_str = serialized_str.size() + 1;
	    serialized_c_str = (char *) malloc(length_ser_c_str);
	    serialized_str.copy(serialized_c_str, serialized_str.size());
	    serialized_c_str[serialized_str.size()]='\0';
		// extreme_debug_log << "rank " << rank << " attrs ser string is of size " << length_ser_c_str << " serialized_str " << 
		// 	serialized_str << " serialized_c_str: " << serialized_c_str << endl;
		extreme_debug_log << "rank " << rank << " attrs ser string is of size " << length_ser_c_str << " serialized_str " << 
			serialized_str << endl;
	}
		
	// extreme_debug_log << "rank " << rank << " about to allgather" << endl;

	MPI_Allgather(&length_ser_c_str, 1, MPI_INT, each_proc_ser_attr_entries_size, 1, MPI_INT, MPI_COMM_WORLD);

	// extreme_debug_log << "rank " << rank << " about with allgather" << endl;

    int sum = 0;
    
    for(int i=0; i<num_client_procs; i++) {
        displacement_for_each_proc[i] = sum;
        sum += each_proc_ser_attr_entries_size[i];
        if(each_proc_ser_attr_entries_size[i] != 0 && rank == 0) {
        	extreme_debug_log << "rank " << i << " has a string of length " << each_proc_ser_attr_entries_size[i] << endl;
        }
    }
    if (rank == 0 ) {
    	extreme_debug_log << "sum: " << sum << endl;
    }


    serialized_c_str_all_attr_entries = (char *) malloc(sum);
    // extreme_debug_log << "num_bytes is "<< to_string(length_ser_c_str) + " and rank: " << to_string(rank) << endl;
    
   	// extreme_debug_log << "rank " << rank << " about to allgatherv" << endl;

 	MPI_Allgatherv(serialized_c_str, length_ser_c_str, MPI_CHAR,
           serialized_c_str_all_attr_entries, each_proc_ser_attr_entries_size, displacement_for_each_proc,
           MPI_CHAR, MPI_COMM_WORLD);

	// extreme_debug_log << "rank " << rank << " done with allgatherv" << endl;
	// extreme_debug_log << "entire set of attr vectors: " << serialized_c_str_all_attr_entries << " and is of length: " << strlen(serialized_c_str_all_attr_entries) << endl;

	for(int i = 0; i < num_servers; i++) {
		int offset = displacement_for_each_proc[i];
		int count = each_proc_ser_attr_entries_size[i];
		if(count > 0) {
			if(i != rank) {
				extreme_debug_log << "rank " << rank << " count: " << count << " offset: " << offset << endl;
				char serialzed_attr_entries_for_one_proc[count];

				memcpy ( serialzed_attr_entries_for_one_proc, serialized_c_str_all_attr_entries + offset, count);
				vector<T> rec_attr_entries;

				extreme_debug_log << "rank " << rank << " serialized_c_str: " << (string)serialzed_attr_entries_for_one_proc << endl;
				extreme_debug_log <<	" serialized_c_str length: " << strlen(serialzed_attr_entries_for_one_proc) << 
					" count: " << count << endl;
		        stringstream ss1;
		        ss1.write(serialzed_attr_entries_for_one_proc, count);
		        extreme_debug_log << "about to deserialize ss: " << ss1.str() << endl;
		        boost::archive::text_iarchive ia(ss1);
		        ia >> rec_attr_entries;
		      	extreme_debug_log << "rank " << rank << " received attr_entries.size(): " << attr_entries.size() << endl;

		      	all_attr_entries.insert(all_attr_entries.end(), rec_attr_entries.begin(), rec_attr_entries.end());

		    }
		    else { //i == rank
		    	all_attr_entries.insert(all_attr_entries.end(), attr_entries.begin(), attr_entries.end());
		    }
        }
	}

	if(length_ser_c_str > 0) {
	    free(serialized_c_str);
	}
    free(serialized_c_str_all_attr_entries);

	add_timing_point(GATHER_ATTR_ENTRIES_DONE);
}

void gather_timestep_entries(vector<md_catalog_timestep_entry> entries, int rank, uint32_t num_servers, uint32_t num_client_procs,
		vector<md_catalog_timestep_entry> &all_entries) {

	add_timing_point(GATHER_TIMESTEP_ENTRIES_START);

	extreme_debug_log.set_rank(rank);

	int length_ser_c_str = 0;
	char *serialized_c_str;
	int each_proc_ser_entries_size[num_client_procs];
	int displacement_for_each_proc[num_client_procs];

	char *serialized_c_str_all_entries;		

	extreme_debug_log << "rank " << rank << " entries.size(): " <<entries.size() << endl;

	if (rank < num_servers && entries.size() > 0) {
	    stringstream ss;
		boost::archive::text_oarchive oa(ss);
		// extreme_debug_log << "rank " << rank << " about to try serializing at the beginning" << endl;
	    oa << entries;
	  	// extreme_debug_log << "rank " << rank << " just finished serializing at the beginning" << endl;
	    string serialized_str = ss.str();
	    length_ser_c_str = serialized_str.size() + 1;
	    serialized_c_str = (char *) malloc(length_ser_c_str);
	    serialized_str.copy(serialized_c_str, serialized_str.size());
	    serialized_c_str[serialized_str.size()]='\0';
		// extreme_debug_log << "rank " << rank << " attrs ser string is of size " << length_ser_c_str << " serialized_str " << 
		// 	serialized_str << " serialized_c_str: " << serialized_c_str << endl;
		extreme_debug_log << "rank " << rank << " attrs ser string is of size " << length_ser_c_str << " serialized_str " << 
			serialized_str << endl;
	}
		
	// extreme_debug_log << "rank " << rank << " about to allgather" << endl;

	MPI_Allgather(&length_ser_c_str, 1, MPI_INT, each_proc_ser_entries_size, 1, MPI_INT, MPI_COMM_WORLD);

	// extreme_debug_log << "rank " << rank << " about with allgather" << endl;

    int sum = 0;
    
    for(int i=0; i<num_client_procs; i++) {
        displacement_for_each_proc[i] = sum;
        sum += each_proc_ser_entries_size[i];
        if(each_proc_ser_entries_size[i] != 0 && rank == 0) {
        	extreme_debug_log << "rank " << i << " has a string of length " << each_proc_ser_entries_size[i] << endl;
        }
    }
    if (rank == 0 ) {
    	extreme_debug_log << "sum: " << sum << endl;
    }


    serialized_c_str_all_entries = (char *) malloc(sum);
    // extreme_debug_log << "num_bytes is "<< to_string(length_ser_c_str) + " and rank: " << to_string(rank) << endl;
    
   	// extreme_debug_log << "rank " << rank << " about to allgatherv" << endl;

 	MPI_Allgatherv(serialized_c_str, length_ser_c_str, MPI_CHAR,
           serialized_c_str_all_entries, each_proc_ser_entries_size, displacement_for_each_proc,
           MPI_CHAR, MPI_COMM_WORLD);

	// extreme_debug_log << "rank " << rank << " done with allgatherv" << endl;
	// extreme_debug_log << "entire set of attr vectors: " << serialized_c_str_all_entries << " and is of length: " << strlen(serialized_c_str_all_entries) << endl;

 	std::unordered_set<uint64_t> set_ids;

	for(int i = 0; i < num_servers; i++) {
		int offset = displacement_for_each_proc[i];
		int count = each_proc_ser_entries_size[i];
		if(count > 0) {
			vector<md_catalog_timestep_entry> new_entries;

			if(i != rank) {
				extreme_debug_log << "rank " << rank << " count: " << count << " offset: " << offset << endl;
				char serialzed_entries_for_one_proc[count];

				memcpy ( serialzed_entries_for_one_proc, serialized_c_str_all_entries + offset, count);

				extreme_debug_log << "rank " << rank << " serialized_c_str: " << (string)serialzed_entries_for_one_proc << endl;
				extreme_debug_log <<	" serialized_c_str length: " << strlen(serialzed_entries_for_one_proc) << 
					" count: " << count << endl;
		        stringstream ss1;
		        ss1.write(serialzed_entries_for_one_proc, count);
		        extreme_debug_log << "about to deserialize ss: " << ss1.str() << endl;
		        boost::archive::text_iarchive ia(ss1);
		        ia >> new_entries;
		      	extreme_debug_log << "rank " << rank << " received entries.size(): " << entries.size() << endl;
		    }
		    else {
		    	new_entries = entries;
		    }
			for( md_catalog_timestep_entry entry : new_entries) {
				//entry.run_id not in ids
				if ( set_ids.find(entry.timestep_id) == set_ids.end() ) {
					all_entries.push_back(entry);
					set_ids.insert(entry.timestep_id);
				}
			}	
        } //end if(count > 0)
	}

	if(length_ser_c_str > 0) {
	    free(serialized_c_str);
	}
    free(serialized_c_str_all_entries);

	add_timing_point(GATHER_TIMESTEP_ENTRIES_DONE);
}

void gather_type_entries(vector<md_catalog_type_entry> entries, int rank, uint32_t num_servers, uint32_t num_client_procs,
		vector<md_catalog_type_entry> &all_entries) {

	add_timing_point(GATHER_TYPE_ENTRIES_START);

	extreme_debug_log.set_rank(rank);

	int length_ser_c_str = 0;
	char *serialized_c_str;
	int each_proc_ser_entries_size[num_client_procs];
	int displacement_for_each_proc[num_client_procs];

	char *serialized_c_str_all_entries;		

	extreme_debug_log << "rank " << rank << " entries.size(): " <<entries.size() << endl;

	if (rank < num_servers && entries.size() > 0) {
	    stringstream ss;
		boost::archive::text_oarchive oa(ss);
		// extreme_debug_log << "rank " << rank << " about to try serializing at the beginning" << endl;
	    oa << entries;
	  	// extreme_debug_log << "rank " << rank << " just finished serializing at the beginning" << endl;
	    string serialized_str = ss.str();
	    length_ser_c_str = serialized_str.size() + 1;
	    serialized_c_str = (char *) malloc(length_ser_c_str);
	    serialized_str.copy(serialized_c_str, serialized_str.size());
	    serialized_c_str[serialized_str.size()]='\0';
		// extreme_debug_log << "rank " << rank << " attrs ser string is of size " << length_ser_c_str << " serialized_str " << 
		// 	serialized_str << " serialized_c_str: " << serialized_c_str << endl;
		extreme_debug_log << "rank " << rank << " attrs ser string is of size " << length_ser_c_str << " serialized_str " << 
			serialized_str << endl;
	}
		
	// extreme_debug_log << "rank " << rank << " about to allgather" << endl;

	MPI_Allgather(&length_ser_c_str, 1, MPI_INT, each_proc_ser_entries_size, 1, MPI_INT, MPI_COMM_WORLD);

	// extreme_debug_log << "rank " << rank << " about with allgather" << endl;

    int sum = 0;
    
    for(int i=0; i<num_client_procs; i++) {
        displacement_for_each_proc[i] = sum;
        sum += each_proc_ser_entries_size[i];
        if(each_proc_ser_entries_size[i] != 0 && rank == 0) {
        	extreme_debug_log << "rank " << i << " has a string of length " << each_proc_ser_entries_size[i] << endl;
        }
    }
    if (rank == 0 ) {
    	extreme_debug_log << "sum: " << sum << endl;
    }


    serialized_c_str_all_entries = (char *) malloc(sum);
    // extreme_debug_log << "num_bytes is "<< to_string(length_ser_c_str) + " and rank: " << to_string(rank) << endl;
    
   	// extreme_debug_log << "rank " << rank << " about to allgatherv" << endl;

 	MPI_Allgatherv(serialized_c_str, length_ser_c_str, MPI_CHAR,
           serialized_c_str_all_entries, each_proc_ser_entries_size, displacement_for_each_proc,
           MPI_CHAR, MPI_COMM_WORLD);

	// extreme_debug_log << "rank " << rank << " done with allgatherv" << endl;
	// extreme_debug_log << "entire set of attr vectors: " << serialized_c_str_all_entries << " and is of length: " << strlen(serialized_c_str_all_entries) << endl;

 	std::unordered_set<uint64_t> set_ids;

	for(int i = 0; i < num_servers; i++) {
		int offset = displacement_for_each_proc[i];
		int count = each_proc_ser_entries_size[i];
		if(count > 0) {
			vector<md_catalog_type_entry> new_entries;

			if(i != rank) {
				extreme_debug_log << "rank " << rank << " count: " << count << " offset: " << offset << endl;
				char serialzed_entries_for_one_proc[count];

				memcpy ( serialzed_entries_for_one_proc, serialized_c_str_all_entries + offset, count);

				extreme_debug_log << "rank " << rank << " serialized_c_str: " << (string)serialzed_entries_for_one_proc << endl;
				extreme_debug_log <<	" serialized_c_str length: " << strlen(serialzed_entries_for_one_proc) << 
					" count: " << count << endl;
		        stringstream ss1;
		        ss1.write(serialzed_entries_for_one_proc, count);
		        extreme_debug_log << "about to deserialize ss: " << ss1.str() << endl;
		        boost::archive::text_iarchive ia(ss1);
		        ia >> new_entries;
		      	extreme_debug_log << "rank " << rank << " received entries.size(): " << entries.size() << endl;
		    }
		    else {
		    	new_entries = entries;
		    }
			for( md_catalog_type_entry entry : new_entries) {
				//entry.run_id not in ids
				// extreme_debug_log << "new_entries.size(): " << new_entries.size() << endl;
				// extreme_debug_log << "entry.type_id: " << entry.type_id << endl;
				if ( set_ids.find(entry.type_id) == set_ids.end() ) {
					all_entries.push_back(entry);
					set_ids.insert(entry.type_id);
				}
			}	
        } //end if(count > 0)
	}

	if(length_ser_c_str > 0) {
	    free(serialized_c_str);
	}
    free(serialized_c_str_all_entries);

	add_timing_point(GATHER_TYPE_ENTRIES_DONE);
}

