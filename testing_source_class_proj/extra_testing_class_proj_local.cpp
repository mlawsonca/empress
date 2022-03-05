

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>
#include <sstream>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <unordered_set>

//includes the print attr functs
#include <testing_harness_debug_helper_functions.hh>
#include <my_metadata_client_local.hh>
#include <client_timing_constants_read_class_proj.hh>
#include <read_helper_functions.hh>

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


template <class T1, class T2>
static void make_range_data (T1 min_int, T2 max_int, string &serial_str);

template <class T2>
static void make_single_val_data (T2 above_max_val, string &serial_str);

template <class T>
void gather_attr_entries(vector<T> attr_entries, uint32_t 
	nt, int rank, uint32_t num_client_procs,
		vector<T> &all_attr_entries, MPI_Comm comm);

void gather_timestep_entries(vector<md_catalog_timestep_entry> entries, int rank, uint32_t num_client_procs,
		vector<md_catalog_timestep_entry> &all_entries, MPI_Comm comm);

// extern std::map <string, vector<double>> data_outputs;

// void find_dims_to_search(const vector<md_dim_bounds> &var_dims, vector<md_dim_bounds> &dims_to_search, 
// 					int dims_funct_count);

int catalog_entries (int rank, uint64_t run_id, 
							uint64_t txn_id,
							md_catalog_timestep_entry &timestep0,
							vector<md_catalog_var_entry> &timestep0_vars,
							vector<md_catalog_type_entry> &type_entries
							);

int do_temporal_analysis ( int rank, uint32_t num_client_procs,
					uint64_t run_id, uint64_t rare_type_id, uint64_t var_id,
              		uint64_t txn_id, MPI_Comm comm
 					);

// int do_global_analysis ( int rank, uint32_t num_client_procs,
// 							uint64_t run_id, uint64_t timestep_id, uint64_t type_id, uint64_t txn_id, 
// 							double above_max_val, MPI_Comm comm
// 							);
int do_global_analysis ( int rank, uint32_t num_client_procs,
							uint64_t run_id, uint64_t type_id, uint64_t txn_id, 
							double above_max_val, MPI_Comm comm
							);

int do_multivariate_analysis ( int rank, uint32_t num_client_procs,
													uint64_t run_id, uint64_t timestep_id,
													const string &var_name_substr, uint64_t rare_type_id,
                                                    uint64_t txn_id, MPI_Comm comm
                                                    );


int extra_testing (int rank, uint32_t num_client_procs, MPI_Comm read_comm) 
{
    int rc;

    uint32_t count;
    //testing_log << "starting extra testing" << endl;


    uint64_t txn_id = -1;

    double above_max_val = 9.9 * pow(10, 9);

    string var_substr1 = "press";

    //extreme_debug_log << "rank: " << rank << endl;

    uint64_t run_id = 1;
    uint64_t timestep_id0 = 0;
    uint64_t type_id0 = 3;
    uint64_t var_id0 = 8;
    uint64_t type_id1 = 11;
    uint64_t type_id2 = 7;


    rc = do_temporal_analysis ( rank, num_client_procs,
    				run_id, type_id0, var_id0,
                    txn_id, read_comm
                    );
    if (rc != RC_OK) {
        error_log << "Error with do_temporal_analysis" << endl;
    }
	// dims_funct_count += 1;

    rc = do_global_analysis ( rank, num_client_procs,
    						run_id, type_id1, txn_id,
                            above_max_val, read_comm
                            );
    if (rc != RC_OK) {
        error_log << "Error with do_global_analysis" << endl;
    }

    rc = do_multivariate_analysis ( rank,  num_client_procs,
    												run_id, timestep_id0,
                                                    var_substr1, type_id2,
                                                    txn_id, read_comm
                                                    );
    if (rc != RC_OK) {
        error_log << "Error with catalog_var_attributes_with_type_var_name_substr_or_val_functs" << endl;
    }
	// dims_funct_count += 1;


    return rc;
 
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//TIMESTEP FUNCTS //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//temporal analysis
// int catalog_timesteps_with_var_or_attr_functs ( int rank, uint32_t num_client_procs,
// 					uint64_t run_id, uint64_t rare_type_id, uint64_t var_id,
//                     int num_dims, const vector<md_dim_bounds> &dims_to_search, uint64_t txn_id,
//  					) 
int do_temporal_analysis ( int rank, uint32_t num_client_procs,
					uint64_t run_id, uint64_t rare_type_id, uint64_t var_id,
              		uint64_t txn_id, MPI_Comm comm
 					) 
{

    int rc = RC_OK;
    uint32_t count;
    vector<md_catalog_timestep_entry> timestep_entries;


    // //Should be empty now 
    // rc = metadata_catalog_timestep (run_id, txn_id, count, timestep_entries);
    // if (rc != RC_OK) {
    //     error_log << "Error set of timestep entries. Proceeding" << endl;
    // }
    // if(testing_logging || (zero_rank_logging && rank == 0)) {
	    // zero_rank_log << "timestep catalog for run_id " << run_id << " and txn_id " << txn_id << ": \n";
    //     print_timestep_catalog (count, timestep_entries);
    // }

    vector<md_catalog_timestep_entry> all_timestep_entries;

    MPI_Barrier(comm);
    add_timing_point(CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_START);
    rc = metadata_catalog_all_timesteps_with_var_attributes_with_type_var (run_id, rare_type_id, var_id, txn_id, count, timestep_entries);

    if (rc == RC_OK) {
    	// if(testing_logging || (zero_rank_logging && rank == 0)) {
	    //     zero_rank_log << "using metadata_catalog_all_timesteps_with_var_attributes_with_type_var funct: timesteps associated with run_id " << run_id << ", type_id " << type_id << 
	    //     ", var_id " << var_id << " and txn_id: " << txn_id << " for rank: " << rank << " \n";
	    //         print_timestep_catalog (count, timestep_entries);
     //    }
    }
    else {
        error_log << "Error getting the matching timestep catalog. Proceeding" << endl;
    }
    add_timing_point(CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_DONE);

    MPI_Barrier(comm); //leave to make timing clearer
    add_timing_point(CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_COLLECTIVE_START);
	gather_timestep_entries(timestep_entries, rank, num_client_procs, all_timestep_entries, comm);
	// if(testing_logging || (zero_rank_logging  && rank == 0)) {
	// 	zero_rank_log << "rank: " << rank << " all timesteps associated with a var attribute for run_id " << run_id << 
	// 		", type_id " << rare_type_id << ", var_id " << var_id <<
	// 		 " note: all_timestep_entries.size(): " << all_timestep_entries.size() << endl;
	// 	print_timestep_catalog (all_timestep_entries.size(), all_timestep_entries);
	// }
    add_timing_point(CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_COLLECTIVE_DONE);

	// all_timestep_entries.clear();
   
 //    MPI_Barrier(comm);
 //    add_timing_point(CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_DIMS_START);
//     rc = metadata_catalog_all_timesteps_with_var_attributes_with_type_var_dims (run_id, rare_type_id, var_id, txn_id, num_dims, dims_to_search, count, timestep_entries);

//     if (rc == RC_OK) {
//     	// if(testing_logging || (zero_rank_logging && rank == 0)) {
// 	    //     zero_rank_log << "using metadata_catalog_all_timesteps_with_var_attributes_with_type_var_dims funct: timesteps associated with run_id " << 
// 	    //     	run_id << ", type_id " << type_id << 
// 	    //     	", var_id " << var_id << " and txn_id: " << txn_id << " overlapping with dims for rank: " << rank << " \n";
// 	    //     for(int j=0; j< num_dims; j++) {
// 	    //         zero_rank_log << " d" << j << "_min: " << dims_to_search [j].min;
// 	    //         zero_rank_log << " d" << j << "_max: " << dims_to_search [j].max;               
// 	    //     }
//      //        print_timestep_catalog (count, timestep_entries);
//      //    }
//     }
//     else {
//         error_log << "Error getting the matching timestep catalog. Proceeding" << endl;
//     }
 //    add_timing_point(CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_DIMS_DONE);

 //    MPI_Barrier(comm); //leave to make timing clearer
 //    add_timing_point(CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_DIMS_COLLECTIVE_START);
	// gather_timestep_entries(timestep_entries, rank, num_client_procs, all_timestep_entries, comm);
	// if(testing_logging || (zero_rank_logging  && rank == 0)) {
	// 	zero_rank_log << "rank: " << rank << " all timesteps associated with a var attribute for run_id " << run_id << 
	// 		", type_id " << rare_type_id << ", var_id " << var_id << " overlapping with dims";
 //        for(int j=0; j< num_dims; j++) {
 //            zero_rank_log << " d" << j << "_min: " << dims_to_search [j].min;
 //            zero_rank_log << " d" << j << "_max: " << dims_to_search [j].max;               
 //        }
	// 	zero_rank_log << " note: all_timestep_entries.size(): " << all_timestep_entries.size() << endl;
	// 	print_timestep_catalog (all_timestep_entries.size(), all_timestep_entries);
	// }
 //    add_timing_point(CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_DIMS_COLLECTIVE_DONE);


    return rc;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//RUN ATTR FUNCTS //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//global analysis
// int do_global_analysis ( int rank, uint32_t num_client_procs,
// 							uint64_t run_id, uint64_t timestep_id, uint64_t type_id, uint64_t txn_id, double above_max_val,
// 							MPI_Comm comm
// 							) 
int do_global_analysis ( int rank, uint32_t num_client_procs,
							uint64_t run_id, uint64_t type_id, uint64_t txn_id, double above_max_val,
							MPI_Comm comm
							) 
{

    int rc = RC_OK;
    uint32_t count;
    // vector<md_catalog_run_attribute_entry> run_attr_entries;
   	// vector<md_catalog_run_attribute_entry> all_run_attr_entries;

    vector<md_catalog_timestep_attribute_entry> timestep_attr_entries;
    vector<md_catalog_timestep_attribute_entry> all_timestep_attr_entries;


 //  	MPI_Barrier(comm);
 //    add_timing_point(CATALOG_ALL_RUN_ATTRIBUTES_WITH_TYPE_START);
//     rc = metadata_catalog_all_run_attributes_with_type (run_id, type_id, txn_id, count, run_attr_entries);

//     if (rc == RC_OK) {
//    	    // if(testing_logging || (zero_rank_logging && rank == 0)) {
// 	       //  zero_rank_log << "using metadata_catalog_all_run_attributes_with_type funct: attr_entries associated with run_id " << run_id << 
// 	       //      " type_id " << type_id << " and txn_id: " << txn_id << " for rank: " << rank << " \n";
//         //     print_run_attribute_list (count, run_attr_entries);
//         //     print_run_attr_data(count, run_attr_entries);
//         // }
//     }
//     else {
//         error_log << "Error getting the matching run attr catalog. Proceeding" << endl;
//     }
 //    add_timing_point(CATALOG_ALL_RUN_ATTRIBUTES_WITH_TYPE_DONE);

 //  	MPI_Barrier(comm); //leave to make timing clearer
 //    add_timing_point(CATALOG_ALL_RUN_ATTRIBUTES_WITH_TYPE_COLLECTIVE_START);
 //    gather_attr_entries(run_attr_entries, count, rank, num_client_procs, all_run_attr_entries);
 //    if(testing_logging || (zero_rank_logging  && rank == 0)) {
 //        zero_rank_log << "rank: " << rank << " all attr_entries associated with run_id " << run_id << 
 //            " type_id " << type_id << " note: all_run_attr_entries.size(): " << all_run_attr_entries.size() << endl;
 //    	print_run_attribute_list (all_run_attr_entries.size(), all_run_attr_entries);
 //    	print_run_attr_data(all_run_attr_entries.size(), all_run_attr_entries);
 //    }
 //    add_timing_point(CATALOG_ALL_RUN_ATTRIBUTES_WITH_TYPE_COLLECTIVE_DONE);


    string above_max_data;
    make_single_val_data (above_max_val, above_max_data);

    MPI_Barrier(comm);
	add_timing_point(CATALOG_ALL_TIMESTEP_ATTRIBUTES_WITH_TYPE_ABOVE_MAX_START);	
    rc = metadata_catalog_all_timestep_attributes_with_type_above_max (run_id, type_id, txn_id, ATTR_DATA_TYPE_REAL, above_max_data, count, timestep_attr_entries);
    if (rc != RC_OK) {
	    	error_log << "Error metadata_catalog_all_timestep_attributes_with_type_above_max. Proceeding" << endl;
    }
	add_timing_point(CATALOG_ALL_TIMESTEP_ATTRIBUTES_WITH_TYPE_ABOVE_MAX_DONE);	

    MPI_Barrier(comm); //leave to make timing clearer
	add_timing_point(CATALOG_ALL_TIMESTEP_ATTRIBUTES_WITH_TYPE_ABOVE_MAX_COLLECTIVE_START);	
    gather_attr_entries(timestep_attr_entries, count, rank, num_client_procs, all_timestep_attr_entries, comm);
    // if(testing_logging || (zero_rank_logging  && rank == 0)) {
  		// zero_rank_log << "rank: " << rank << " all timestep attributes associated with run_id " << run_id << 
  		// 	" and type_id " << type_id << 
  		// 	" int attr data greater than or eq to " << above_max_val <<
  		// 	" note: all_timestep_attr_entries.size(): " << all_timestep_attr_entries.size() << endl;
    // 	print_timestep_attribute_list (all_timestep_attr_entries.size(), all_timestep_attr_entries);
    // 	// print_timestep_attr_data(all_timestep_attr_entries.size(), all_timestep_attr_entries);
    // }	
	add_timing_point(CATALOG_ALL_TIMESTEP_ATTRIBUTES_WITH_TYPE_ABOVE_MAX_COLLECTIVE_DONE);	

  
    return rc;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//SUBSTR FUNCTS ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    

// int do_multivariate_analysis ( int rank, uint32_t num_client_procs,
// 													uint64_t run_id, uint64_t timestep_id,
// 													const string &var_name_substr, uint64_t rare_type_id,
//                                                     int num_dims, const vector<md_dim_bounds> &dims_to_search,
//                                                     uint64_t txn_id
//                                                     ) 
int do_multivariate_analysis ( int rank, uint32_t num_client_procs,
													uint64_t run_id, uint64_t timestep_id,
													const string &var_name_substr, uint64_t rare_type_id,
                                                    uint64_t txn_id, MPI_Comm comm
                                                    ) 
{

    int rc = RC_OK;
    uint32_t count;
    vector<md_catalog_var_attribute_entry> var_attr_entries;

	vector<md_catalog_var_attribute_entry> all_var_attr_entries;

	MPI_Barrier(comm);
	add_timing_point(CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_START);	
    rc = metadata_catalog_all_var_attributes_with_type_var_substr (run_id, timestep_id, rare_type_id, var_name_substr, txn_id, count, var_attr_entries);
    if (rc == RC_OK) {
        // if(testing_logging || (zero_rank_logging && rank == 0)) {
	       //  zero_rank_log << "using var var_attr_entries with type var by name ver funct: attributes associated with run_id " << run_id << ", timestep_id " << 
	       //  	timestep_id << " and txn_id: " << txn_id << " and type_id " << type_id <<
	       //  	" and var_substr " << var_name_substr;
	       //  zero_rank_log << " for rank: " << rank << " \n";
        //     print_var_attribute_list (count, var_attr_entries);
        //     print_var_attr_data(count, var_attr_entries);
        // }
    }
    else {
        error_log << "Error metadata_catalog_all_var_attributes_with_type_var_substr. Proceeding" << endl;
    }
	add_timing_point(CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_DONE);	

	MPI_Barrier(comm); //leave to make timing clearer
	add_timing_point(CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_COLLECTIVE_START);	
	gather_attr_entries(var_attr_entries, count, rank, num_client_procs, all_var_attr_entries, comm);
	// if(testing_logging || (zero_rank_logging  && rank == 0)) {
	// 	zero_rank_log << "rank: " << rank << " all var attributes associated with run_id " << run_id << " and timestep_id " << timestep_id << 
	// 		" type: " << rare_type_id << 
	// 		" and var_substr " << var_name_substr << " note: all_var_attr_entries.size(): " << all_var_attr_entries.size() << endl;
	// 	print_var_attribute_list (all_var_attr_entries.size(), all_var_attr_entries);
	// 	// print_var_attr_data(all_var_attr_entries.size(), all_var_attr_entries);
	// }

	add_timing_point(CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_COLLECTIVE_DONE);	

	// all_var_attr_entries.clear();

	
	// MPI_Barrier(comm);	
	// add_timing_point(CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_DIMS_START);	
//     rc = metadata_catalog_all_var_attributes_with_type_var_substr_dims (run_id, timestep_id, rare_type_id, 
//     	var_name_substr, txn_id, num_dims, dims_to_search, count, var_attr_entries);
//     if (rc == RC_OK) {
//         // if(testing_logging || (zero_rank_logging && rank == 0)) {
// 	       //  zero_rank_log << "using metadata_catalog_all_var_attributes_with_type_var_substr_dims funct: " << 
//     	   		//	"attributes associated with run_id " << run_id << ", timestep_id " << timestep_id << " and txn_id: " << txn_id << " and type_id " << rare_type_id <<
// 	       //  " and var_substr " << var_name_substr << " overlapping with dims";
// 	       //  for(int j=0; j< num_dims; j++) {
// 	       //      zero_rank_log << " d" << j << "_min: " << dims_to_search [j].min;
// 	       //      zero_rank_log << " d" << j << "_max: " << dims_to_search [j].max;               
// 	       //  }
// 	       //  zero_rank_log << " for rank: " << rank << " \n";
//         //     print_var_attribute_list (count, var_attr_entries);
//         //     print_var_attr_data(count, var_attr_entries);
//         // }
//     }
//     else {
//         error_log << "Error metadata_catalog_all_var_attributes_with_type_var_substr_dims" << " \n";
//     }
	// add_timing_point(CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_DIMS_DONE);	

	// MPI_Barrier(comm); //leave to make timing clearer	
	// add_timing_point(CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_DIMS_COLLECTIVE_START);	
	// gather_attr_entries(var_attr_entries, count, rank, num_client_procs, all_var_attr_entries, comm);
	// if(testing_logging || (zero_rank_logging  && rank == 0)) {
	// 	zero_rank_log << "rank: " << rank << " all var attributes associated with run_id " << run_id << " and timestep_id " << timestep_id << 
	// 		" type: " << rare_type_id << 
	// 		" and var_substr " << var_name_substr << " overlapping with dims";
 //        for(int j=0; j< num_dims; j++) {
 //            zero_rank_log << " d" << j << "_min: " << dims_to_search [j].min;
 //            zero_rank_log << " d" << j << "_max: " << dims_to_search [j].max;               
 //        }
	// 	zero_rank_log << " note: all_var_attr_entries.size(): " << all_var_attr_entries.size() << endl;
	// 	print_var_attribute_list (all_var_attr_entries.size(), all_var_attr_entries);
	// 	print_var_attr_data(all_var_attr_entries.size(), all_var_attr_entries);
	// }

	// add_timing_point(CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_DIMS_COLLECTIVE_DONE);	

	// all_var_attr_entries.clear();


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


// template <class T>
// void gather_attr_entries(vector<T> attr_entries, uint32_t count, int rank, uint32_t num_client_procs,
// 		vector<T> &all_attr_entries, MPI_Comm comm) {

// 	add_timing_point(GATHER_ATTR_ENTRIES_START);

// 	//extreme_debug_log.set_rank(rank);

// 	int length_ser_c_str = 0;
// 	char *serialized_c_str;
// 	int each_proc_ser_attr_entries_size[num_client_procs];
// 	int displacement_for_each_proc[num_client_procs];

// 	char *serialized_c_str_all_attr_entries;		

// 	//extreme_debug_log << "rank " << rank << " attr_entries.size(): " <<attr_entries.size() << endl;
// 	//extreme_debug_log << "num_client_procs: " << num_client_procs << endl;
// 	if (attr_entries.size() > 0) {
// 	    stringstream ss;
// 		boost::archive::text_oarchive oa(ss);
// 		// //extreme_debug_log << "rank " << rank << " about to try serializing at the beginning" << endl;
// 	    oa << attr_entries;
// 	  	// //extreme_debug_log << "rank " << rank << " just finished serializing at the beginning" << endl;
// 	    string serialized_str = ss.str();
// 	    length_ser_c_str = serialized_str.size() + 1;
// 	    serialized_c_str = (char *) malloc(length_ser_c_str);
// 	    serialized_str.copy(serialized_c_str, serialized_str.size());
// 	    serialized_c_str[serialized_str.size()]='\0';
// 		// //extreme_debug_log << "rank " << rank << " attrs ser string is of size " << length_ser_c_str << " serialized_str " << 
// 		// 	serialized_str << " serialized_c_str: " << serialized_c_str << endl;
// 		//extreme_debug_log << "rank " << rank << " attrs ser string is of size " << length_ser_c_str << " serialized_str " << 
// 			// serialized_str << endl;
// 	}
		
// 	//extreme_debug_log << "rank " << rank << " about to allgather my length_ser_c_str: " << length_ser_c_str << endl;

// 	MPI_Allgather(&length_ser_c_str, 1, MPI_INT, each_proc_ser_attr_entries_size, 1, MPI_INT, comm);

// 	//extreme_debug_log << "rank " << rank << " done with allgather" << endl;

//     int sum = 0;
    
//     for(int i=0; i<num_client_procs; i++) {
//     	//extreme_debug_log << "i: " << i << " sum: " << sum << endl;
//         displacement_for_each_proc[i] = sum;
//         //extreme_debug_log << "each_proc_ser_attr_entries_size[i]: " << each_proc_ser_attr_entries_size[i] << endl;
//         sum += each_proc_ser_attr_entries_size[i];
//         // if(each_proc_ser_attr_entries_size[i] != 0 && rank == 0) {
//         	//extreme_debug_log << "rank " << i << " has a string of length " << each_proc_ser_attr_entries_size[i] << endl;
//         // }
//     }

//     serialized_c_str_all_attr_entries = (char *) malloc(sum);
//     // //extreme_debug_log << "num_bytes is "<< to_string(length_ser_c_str) + " and rank: " << to_string(rank) << endl;
    
//    	// //extreme_debug_log << "rank " << rank << " about to allgatherv" << endl;

//  	MPI_Allgatherv(serialized_c_str, length_ser_c_str, MPI_CHAR,
//            serialized_c_str_all_attr_entries, each_proc_ser_attr_entries_size, displacement_for_each_proc,
//            MPI_CHAR, comm);

// 	// //extreme_debug_log << "rank " << rank << " done with allgatherv" << endl;
// 	// //extreme_debug_log << "entire set of attr vectors: " << serialized_c_str_all_attr_entries << " and is of length: " << strlen(serialized_c_str_all_attr_entries) << endl;

// 	for(int i = 0; i < num_client_procs; i++) {
// 		int offset = displacement_for_each_proc[i];
// 		int count = each_proc_ser_attr_entries_size[i];
// 		if(count > 0) {
// 			if(i != rank) {
// 				//extreme_debug_log << "rank " << rank << " count: " << count << " offset: " << offset << endl;
// 				char serialzed_attr_entries_for_one_proc[count];

// 				memcpy ( serialzed_attr_entries_for_one_proc, serialized_c_str_all_attr_entries + offset, count);
// 				vector<T> rec_attr_entries;

// 				//extreme_debug_log << "rank " << rank << " serialized_c_str: " << (string)serialzed_attr_entries_for_one_proc << endl;
// 				//extreme_debug_log <<	" serialized_c_str length: " << strlen(serialzed_attr_entries_for_one_proc) << 
// 					// " count: " << count << endl;
// 		        stringstream ss1;
// 		        ss1.write(serialzed_attr_entries_for_one_proc, count);
// 		        //extreme_debug_log << "about to deserialize ss: " << ss1.str() << endl;
// 		        boost::archive::text_iarchive ia(ss1);
// 		        ia >> rec_attr_entries;
// 		      	//extreme_debug_log << "rank " << rank << " received attr_entries.size(): " << attr_entries.size() << endl;

// 		      	all_attr_entries.insert(all_attr_entries.end(), rec_attr_entries.begin(), rec_attr_entries.end());

// 		    }
// 		    else { //i == rank
// 		    	all_attr_entries.insert(all_attr_entries.end(), attr_entries.begin(), attr_entries.end());
// 		    }
//         }
// 	}

// 	if(length_ser_c_str > 0) {
// 	    free(serialized_c_str);
// 	}
//     free(serialized_c_str_all_attr_entries);

// 	add_timing_point(GATHER_ATTR_ENTRIES_DONE);
// }

void gather_timestep_entries(vector<md_catalog_timestep_entry> entries, int rank, uint32_t num_client_procs,
		vector<md_catalog_timestep_entry> &all_entries, MPI_Comm comm) {

	add_timing_point(GATHER_TIMESTEP_ENTRIES_START);

	//extreme_debug_log.set_rank(rank);

	int length_ser_c_str = 0;
	char *serialized_c_str;
	int each_proc_ser_entries_size[num_client_procs];
	int displacement_for_each_proc[num_client_procs];

	char *serialized_c_str_all_entries;		

	//extreme_debug_log << "rank " << rank << " entries.size(): " <<entries.size() << endl;

	if (entries.size() > 0) {
	    stringstream ss;
		boost::archive::text_oarchive oa(ss);
		// //extreme_debug_log << "rank " << rank << " about to try serializing at the beginning" << endl;
	    oa << entries;
	  	// //extreme_debug_log << "rank " << rank << " just finished serializing at the beginning" << endl;
	    string serialized_str = ss.str();
	    length_ser_c_str = serialized_str.size() + 1;
	    serialized_c_str = (char *) malloc(length_ser_c_str);
	    serialized_str.copy(serialized_c_str, serialized_str.size());
	    serialized_c_str[serialized_str.size()]='\0';
		// //extreme_debug_log << "rank " << rank << " attrs ser string is of size " << length_ser_c_str << " serialized_str " << 
		// 	serialized_str << " serialized_c_str: " << serialized_c_str << endl;
		//extreme_debug_log << "rank " << rank << " attrs ser string is of size " << length_ser_c_str << " serialized_str " << 
			// serialized_str << endl;
	}
		
	// //extreme_debug_log << "rank " << rank << " about to allgather" << endl;

	MPI_Allgather(&length_ser_c_str, 1, MPI_INT, each_proc_ser_entries_size, 1, MPI_INT, comm);

	// //extreme_debug_log << "rank " << rank << " about with allgather" << endl;

    int sum = 0;
    
    for(int i=0; i<num_client_procs; i++) {
        displacement_for_each_proc[i] = sum;
        sum += each_proc_ser_entries_size[i];
        // if(each_proc_ser_entries_size[i] != 0 && rank == 0) {
        	//extreme_debug_log << "rank " << i << " has a string of length " << each_proc_ser_entries_size[i] << endl;
        // }
    }
    // if (rank == 0 ) {
    	//extreme_debug_log << "sum: " << sum << endl;
    // }


    serialized_c_str_all_entries = (char *) malloc(sum);
    // //extreme_debug_log << "num_bytes is "<< to_string(length_ser_c_str) + " and rank: " << to_string(rank) << endl;
    
   	// //extreme_debug_log << "rank " << rank << " about to allgatherv" << endl;

 	MPI_Allgatherv(serialized_c_str, length_ser_c_str, MPI_CHAR,
           serialized_c_str_all_entries, each_proc_ser_entries_size, displacement_for_each_proc,
           MPI_CHAR, comm);

	// //extreme_debug_log << "rank " << rank << " done with allgatherv" << endl;
	// //extreme_debug_log << "entire set of attr vectors: " << serialized_c_str_all_entries << " and is of length: " << strlen(serialized_c_str_all_entries) << endl;

 	std::unordered_set<uint64_t> set_ids;

	for(int i = 0; i < num_client_procs; i++) {
		int offset = displacement_for_each_proc[i];
		int count = each_proc_ser_entries_size[i];
		if(count > 0) {
			vector<md_catalog_timestep_entry> new_entries;

			if(i != rank) {
				//extreme_debug_log << "rank " << rank << " count: " << count << " offset: " << offset << endl;
				char serialzed_entries_for_one_proc[count];

				memcpy ( serialzed_entries_for_one_proc, serialized_c_str_all_entries + offset, count);

				//extreme_debug_log << "rank " << rank << " serialized_c_str: " << (string)serialzed_entries_for_one_proc << endl;
				//extreme_debug_log <<	" serialized_c_str length: " << strlen(serialzed_entries_for_one_proc) << 
					// " count: " << count << endl;
		        stringstream ss1;
		        ss1.write(serialzed_entries_for_one_proc, count);
		        //extreme_debug_log << "about to deserialize ss: " << ss1.str() << endl;
		        boost::archive::text_iarchive ia(ss1);
		        ia >> new_entries;
		      	//extreme_debug_log << "rank " << rank << " received entries.size(): " << entries.size() << endl;
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