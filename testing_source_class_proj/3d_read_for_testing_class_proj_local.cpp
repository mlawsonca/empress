#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>

#include <3d_read_for_testing_class_proj_local.hh>
// #include <md_client_timing_constants.hh>
#include <client_timing_constants_read_class_proj.hh>
#include <testing_harness_debug_helper_functions.hh> //note - this is just to use while debugging for print functs
#include <read_helper_functions.hh>


using namespace std;

// extern std::vector<int> catg_of_time_pts;
// extern std::vector<std::chrono::high_resolution_clock::time_point> time_pts;
extern void add_timing_point(int catg);
extern void add_objector_point(int catg, int num_params_to_fetch);
// extern void add_objector_output_point(int time_catg, int num_params_to_fetch, uint16_t read_type);

// static bool extreme_debug_logging = false;
// static bool debug_logging = false;
// static bool testing_logging = false;
// static bool zero_rank_logging = false;
static bool error_logging = true;

static debugLog error_log = debugLog(error_logging);
// static debugLog debug_log = debugLog(debug_logging);
// static debugLog extreme_debug_log = debugLog(extreme_debug_logging);
// static debugLog testing_log = debugLog(testing_logging);
// static debugLog zero_rank_log = debugLog(true);


// template <class T>
// extern void gather_attr_entries(vector<T> attr_entries, uint32_t count, int rank, uint32_t num_client_procs,
//         vector<T> &all_attr_entries, MPI_Comm read_comm);


int read_pattern_attrs(int rank,
	const md_catalog_var_entry &var, uint32_t num_client_procs, 
	uint64_t txn_id, uint64_t type_id,
	uint16_t catalog_attr_start_code, uint16_t catalog_attr_done_code,
	MPI_Comm read_comm
	);

int read_pattern_attrs(int rank, const vector<md_dim_bounds> &dims,
	const md_catalog_var_entry &var, uint32_t num_client_procs,
	uint64_t txn_id, uint64_t type_id,
	uint16_t catalog_attr_start_code, uint16_t catalog_attr_done_code,
	MPI_Comm read_comm
	);

int do_read(int rank, const vector<md_dim_bounds> &dims,
	const md_catalog_var_entry &var, uint32_t num_client_procs,
	uint64_t txn_id, uint64_t type_id,
	MPI_Comm read_comm);

// static void print_all_var_attrs(int rank, 
// 				const md_catalog_var_entry &var, uint32_t num_client_procs,
// 				uint64_t txn_id
// 				);

static void convert_dim_bounds_to_counts(const vector<md_dim_bounds> &dims, uint64_t *counts);


// 1. return data for all vars
int read_pattern_1 (int rank, const std::vector<md_dim_bounds> &proc_dims, 
                    int num_client_procs,
                    uint64_t txn_id, const md_catalog_run_entry &run, 
                    const std::vector<md_catalog_var_entry> &entries,
                    const std::vector<uint64_t> &type_ids, MPI_Comm read_comm
                    )
{
    
    add_timing_point(READ_PATTERN_1_START);

    int rc = RC_OK;

    // for (int j=0; j<3; j++) {
        //extreme_debug_log <<  "proc_dims [" << to_string(j) << "] min is: " << to_string(proc_dims [j].min) << endl;
        //extreme_debug_log <<  "proc_dims [" << to_string(j) << "] max is: " << to_string(proc_dims [j].max) << endl;
    // }    

    for(int i=0; i<entries.size(); i++) {
       	md_catalog_var_entry var = entries.at(i); 

	    for (uint64_t type_id : type_ids) {

			read_pattern_attrs(rank, var, num_client_procs, txn_id, type_id,
				READ_PATTERN_1_START_CATALOGING_VAR_ATTRS, READ_PATTERN_1_DONE_CATALOGING_VAR_ATTRS, read_comm);	    	

		} //end for (uint64_t type_id : type_ids)
	}
        

    add_timing_point(READ_PATTERN_1_DONE); 

cleanup:
    return rc;
}


// 2. all of 1 var
int read_pattern_2 (int rank, const std::vector<md_dim_bounds> &proc_dims, 
                    int num_client_procs,
                    uint64_t txn_id, const md_catalog_run_entry &run,
                    const md_catalog_var_entry &var, 
                    const std::vector<uint64_t> &type_ids, MPI_Comm read_comm
                    )
{

    int rc = RC_OK;
    int index_of_var;

    add_timing_point(READ_PATTERN_2_START);

    for (uint64_t type_id : type_ids) {

		read_pattern_attrs(rank, var, num_client_procs, txn_id, type_id,
			READ_PATTERN_2_START_CATALOGING_VAR_ATTRS, READ_PATTERN_2_DONE_CATALOGING_VAR_ATTRS, read_comm);	    	

	} //end for (uint64_t type_id : type_ids)

    add_timing_point(READ_PATTERN_2_DONE);

cleanup:

    return rc;
}

// 3. all of a few vars (3 for 3-d, for example)
int read_pattern_3 (int rank, const std::vector<md_dim_bounds> &proc_dims, 
                    int num_client_procs,
                    uint64_t txn_id, const md_catalog_run_entry &run,
                    const std::vector<md_catalog_var_entry> &vars, 
                    const std::vector<uint64_t> &type_ids, MPI_Comm read_comm
                    ) 
{
    int rc = RC_OK;

    add_timing_point(READ_PATTERN_3_START);

    // for (int j=0; j<3; j++) {
    //     //debug_log <<  "proc_dims [" << to_string(j) << "] min is: " << to_string(proc_dims [j].min) << endl;
    //     //debug_log <<  "proc_dims [" << to_string(j) << "] max is: " << to_string(proc_dims [j].max) << endl;
    // }    
    // add_timing_point(READ_PATTERN_3_VAR_INIT_DONE_START_READING);

    for (int i=0; i < vars.size(); i++) {
        md_catalog_var_entry var = vars.at(i);

        for (uint64_t type_id : type_ids) {
			read_pattern_attrs(rank, var, num_client_procs, txn_id, type_id,
				READ_PATTERN_3_START_CATALOGING_VAR_ATTRS, READ_PATTERN_3_DONE_CATALOGING_VAR_ATTRS, read_comm);	    	

		} //end for (uint64_t type_id : type_ids)
    }

    add_timing_point(READ_PATTERN_3_DONE);


cleanup:
    return rc;
}


// 4. 1 plane in each dimension for 1 variable
int read_pattern_4 (int rank, int num_x_procs, int num_y_procs, 
                    int num_client_procs,
                    uint64_t txn_id, const md_catalog_run_entry &run,
                    const md_catalog_var_entry &var, 
                    const std::vector<uint64_t> &type_ids, MPI_Comm read_comm
                    ) 
{
   
    add_timing_point(READ_PATTERN_4_START);

    int rc = RC_OK;

   	// uint32_t my_x_dim, my_y_dim;
    // uint64_t x_offset, y_offset;

    // //figure out nx, ny, nz for the given var
    uint64_t var_dims[var.num_dims];
    convert_dim_bounds_to_counts(var.dims, var_dims);

    for (uint64_t type_id : type_ids) {

	    for (int plane = 0; plane < 3; plane ++) {
	      	//want to read the md for the entire plane
    		vector<md_dim_bounds> plane_dims = var.dims;
	    	plane_dims[plane].min = var_dims[plane] / 2;
	    	plane_dims[plane].max = plane_dims[plane].min;

		    // int x_dim;
	    	// int y_dim;

		    // if(plane == 0 ) {
		    // 	x_dim = 1;
		    // }
		    // else { 
		    // 	x_dim = 0;
		    // }
		    // if(plane == 2) {
		    // 	y_dim = 1;
		    // }
		    // else {
		    // 	y_dim = 2;
		    // }

		    // my_x_dim = var_dims[x_dim] / num_x_procs;
		    // my_y_dim = var_dims[y_dim] / num_y_procs;

		    // //note - would need to be adjusted for vars that don't start at 0
		    // x_offset = (rank % num_x_procs) * my_x_dim;
	    	// y_offset = (rank/num_x_procs) * my_y_dim;

	    	// plane_dims[x_dim].min = x_offset;
	    	// plane_dims[x_dim].max = x_offset + my_x_dim - 1;

	    	// plane_dims[y_dim].min = y_offset;
	    	// plane_dims[y_dim].max = y_offset + my_y_dim - 1;

		    // // plane_vol = my_x_dim * my_y_dim;
	   
		    // //debug_log << "read pattern 4 plane " << plane << endl;
		    // for (int j=0; j<3; j++) {
		    //     //debug_log <<  "dims [" << to_string(j) << "] min is: " << plane_dims[j].min << endl;
		    //     //debug_log <<  "dims [" << to_string(j) << "] max is: " << plane_dims[j].max << endl;
		    // }  

	    	// // add_timing_point(READ_PATTERN_4_PLANE_TO_FIND_INIT_DONE_START_READING_VAR_DATA); 

		    // //extreme_debug_log << "my_x_dim: " << my_x_dim << endl;
		    // //extreme_debug_log << "my_y_dim: " << my_y_dim << endl;
		    // //extreme_debug_log << "x_offset: " << x_offset << endl;
		    // //extreme_debug_log << "y_offset: " << y_offset << endl;

			rc = read_pattern_attrs(rank, plane_dims, var, num_client_procs, txn_id, type_id,
				READ_PATTERN_4_START_CATALOGING_VAR_ATTRS, READ_PATTERN_4_DONE_CATALOGING_VAR_ATTRS, read_comm);	    	

		} //end read planes for given type
	} //end for (uint64_t type_id : type_ids)

    add_timing_point(READ_PATTERN_4_DONE); 
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
cleanup:
    //debug_log << "initiating cleanup" << endl;

    return rc;
}


// 5. an arbitrary rectangular subset representing a cubic area of interest
int read_pattern_5 (int rank, int num_x_procs, int num_y_procs, int num_z_procs, 
	                int num_client_procs,
                    uint64_t txn_id, const md_catalog_run_entry &run,
                    const md_catalog_var_entry &var, 
                    const std::vector<uint64_t> &type_ids, MPI_Comm read_comm
                    )
{    

    add_timing_point(READ_PATTERN_5_START);

    int rc = RC_OK;

 //    int my_x_dim;
 //    int my_y_dim;
 //    int my_z_dim;

 //    int x_offset;
 //    int y_offset;
 //    int z_offset;

 //    // int64_t timestep_file_id;

 //    //figure out nx, ny, nz for the given var
 //    uint64_t var_dims[var.num_dims];
 //    convert_dim_bounds_to_counts(var.dims, var_dims);

 //    //extreme_debug_log << "num_x_procs: " << num_x_procs << endl;
 //    //extreme_debug_log << "num_y_procs: " << num_y_procs << endl;
 //    //extreme_debug_log << "num_z_procs: " << num_z_procs << endl;
 //    //extreme_debug_log << "nx: " << var_dims[0] << endl;
 //    //extreme_debug_log << "ny: " << var_dims[1] << endl;
 //    //extreme_debug_log << "nz: " << var_dims[2] << endl;

 //    my_x_dim = var_dims[0] / num_x_procs / 2;
 //    my_y_dim = var_dims[1] / num_y_procs / 2;
 //    my_z_dim = var_dims[2] / num_z_procs / 2;

	// //note - would need to be adjusted for vars that don't start at 0
 //    x_offset = (rank % num_x_procs) * my_x_dim;
 //    y_offset = ((rank/num_x_procs) % num_y_procs) * my_y_dim;
 //    z_offset = (rank/(num_x_procs*num_y_procs) % num_z_procs) * my_z_dim;

 //    //extreme_debug_log << "my_x_dim: " << my_x_dim << endl;
 //    //extreme_debug_log << "my_y_dim: " << my_y_dim << endl;
 //    //extreme_debug_log << "my_z_dim: " << my_z_dim << endl;
 //    //extreme_debug_log << "x_offset: " << x_offset << endl;
 //    //extreme_debug_log << "y_offset: " << y_offset << endl;
 //    //extreme_debug_log << "z_offset: " << z_offset << endl;

 //    uint32_t num_dims = 3;
 //    vector<md_dim_bounds> dims(num_dims);
 //    dims [0].min = var_dims[0] / 4 + x_offset;
 //    dims [0].max = var_dims[0] / 4 + x_offset + my_x_dim - 1;
 //    dims [1].min = var_dims[1] / 4 + y_offset;
 //    dims [1].max = var_dims[1] / 4 + y_offset + my_y_dim -1;
 //    dims [2].min = var_dims[2] / 4 + z_offset;
 //    dims [2].max = var_dims[2] / 4 + z_offset + my_z_dim -1;

 //    //debug_log << "read pattern 5" << endl;
 //    for (int j=0; j<3; j++) {
 //        //debug_log <<  "dims [" << to_string(j) << "] min is: " << to_string(dims [j].min) << endl;
 //        //debug_log <<  "dims [" << to_string(j) << "] max is: " << to_string(dims [j].max) << endl;
 //    }  

    //figure out nx, ny, nz for the given var
    uint64_t var_dims[var.num_dims];
    convert_dim_bounds_to_counts(var.dims, var_dims);

    int x_width = var_dims[0] / 2;
    int y_width = var_dims[1] / 2;
    int z_width = var_dims[2] / 2;

    uint32_t num_dims = 3;
    vector<md_dim_bounds> dims(num_dims);
    dims [0].min = var_dims[0] / 4 ;
    dims [0].max = dims[0].min + x_width - 1;
    dims [1].min = var_dims[1] / 4;
    dims [1].max = dims[1].min + y_width -1;
    dims [2].min = var_dims[2] / 4;
    dims [2].max = dims[2].min + z_width -1;

    //debug_log << "read pattern 5" << endl;
    // for (int j=0; j<3; j++) {
        //debug_log <<  "dims [" << to_string(j) << "] min is: " << to_string(dims [j].min) << endl;
        //debug_log <<  "dims [" << to_string(j) << "] max is: " << to_string(dims [j].max) << endl;
    // }  

    for (uint64_t type_id : type_ids) {
		rc = read_pattern_attrs(rank, dims, var, num_client_procs, txn_id, type_id,
			READ_PATTERN_5_START_CATALOGING_VAR_ATTRS, READ_PATTERN_5_DONE_CATALOGING_VAR_ATTRS, read_comm);
	}

    add_timing_point(READ_PATTERN_5_DONE);

cleanup:
    //debug_log << "initiating cleanup" << endl;

    return rc;
}

// 6. an arbitrary area on an orthogonal plane (decomposition dimensions) aka partial plane
int read_pattern_6 (int rank, int num_x_procs, int num_y_procs, 
                    int num_client_procs,
                    uint64_t txn_id, const md_catalog_run_entry &run,
                    const md_catalog_var_entry &var, 
                    const std::vector<uint64_t> &type_ids, MPI_Comm read_comm
                    ) 
{
    
    add_timing_point(READ_PATTERN_6_START);

    int rc = RC_OK;

    // int my_x_dim;
    // int my_y_dim;

    // int x_offset;
    // int y_offset;

    int x_width, y_width;


    //figure out nx, ny, nz for the given var
    uint64_t var_dims[var.num_dims];
    convert_dim_bounds_to_counts(var.dims, var_dims);

  	vector<md_dim_bounds> plane_dims(3);

  	for (uint64_t type_id : type_ids) {
		for (int plane = 0; plane < 3; plane ++) {
		    plane_dims[2-plane].min = var_dims[2-plane] / 4;
		    plane_dims [2-plane].max = plane_dims[2-plane].min;

		    int x_dim;
	    	int y_dim;

		    if(plane == 0 ) {
		    	x_dim = 1;
		    }
		    else { 
		    	x_dim = 2;
		    }
		    if(plane == 2) {
		    	y_dim = 1;
		    }
		    else {
		    	y_dim = 0;
		    }

		    // my_x_dim = var_dims[x_dim] / num_x_procs / 2;
		    // my_y_dim = var_dims[y_dim] / num_y_procs / 2;

		    // //note - would need to be adjusted for vars that don't start at 0
		    // x_offset = (rank % num_x_procs) * my_x_dim;
	    	// y_offset = (rank/num_x_procs) * my_y_dim;


		    // plane_dims [x_dim].min = var_dims[x_dim]/4 + x_offset;
		    // plane_dims [x_dim].max = plane_dims[x_dim].min + my_x_dim - 1;

		    // plane_dims [y_dim].min = var_dims[y_dim]/4 + y_offset;
		    // plane_dims [y_dim].max = plane_dims[y_dim].min + my_y_dim - 1;

		    x_width = var_dims[x_dim] / 2;
		    y_width = var_dims[y_dim] / 2;

		    plane_dims [x_dim].min = var_dims[x_dim]/4;
		    plane_dims [x_dim].max = plane_dims[x_dim].min + x_width - 1;

		    plane_dims [y_dim].min = var_dims[y_dim]/4;
		    plane_dims [y_dim].max = plane_dims[y_dim].min + y_width - 1;

		    // plane_vol = my_x_dim * my_y_dim;
	   
		    //debug_log << "read pattern 6 plane " << plane << endl;
		    // for (int j=0; j<3; j++) {
		        //debug_log <<  "dims [" << to_string(j) << "] min is: " << plane_dims [j].min << endl;
		        //debug_log <<  "dims [" << to_string(j) << "] max is: " << plane_dims [j].max << endl;
		    // }  



			// add_timing_point(READ_PATTERN_6_START_CATALOGING_VAR_ATTRS);
			// rc = read_pattern_attrs(rank, plane_dims, var, num_client_procs, txn_id, type_id,
			// 		read_comm);
	 		// do_read(rank, plane_dims, var, num_client_procs, txn_id, type_id, read_comm);
			// add_timing_point(READ_PATTERN_6_DONE_CATALOGING_VAR_ATTRS);
			rc = read_pattern_attrs(rank, plane_dims, var, num_client_procs, txn_id, type_id,
				READ_PATTERN_6_START_CATALOGING_VAR_ATTRS, READ_PATTERN_6_DONE_CATALOGING_VAR_ATTRS, read_comm);
		}
	} //end for (uint64_t type_id : type_ids)

    add_timing_point(READ_PATTERN_6_DONE);
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

cleanup:
    //debug_log << "initiating cleanup" << endl;
    return rc;
}


int read_pattern_attrs(int rank, 
	const md_catalog_var_entry &var, uint32_t num_client_procs,
	uint64_t txn_id, uint64_t type_id,
	uint16_t catalog_attr_start_code, uint16_t catalog_attr_done_code,
	MPI_Comm read_comm
	)
{
	int rc = RC_OK;

	// int new_num_client_procs;
	// MPI_Comm_size(read_comm, &new_num_client_procs);
	// //extreme_debug_log << "num_client_procs: " << num_client_procs << " new_num_client_procs: " << new_num_client_procs << endl;

    add_timing_point(catalog_attr_start_code);

    std::vector<md_catalog_var_attribute_entry> var_attr_entries;
    std::vector<md_catalog_var_attribute_entry> all_var_attr_entries;

    uint32_t num_var_attr_entries = 0;
	//extreme_debug_log << "rank: " << rank << " about to catalog" << endl;
	rc = metadata_catalog_all_var_attributes_with_type_var_by_id (var.timestep_id, 
		type_id, var.var_id, txn_id, num_var_attr_entries, var_attr_entries); 

	//extreme_debug_log << "rank: " << rank << " just finished the catalog" << endl;

    if (rc != RC_OK) {
        error_log << "error metadata_catalog_all_var_attributes_with_type_var_dims_by_id" << endl;
        add_timing_point(ERR_CATALOG_ATTR);
    }
    // else {
    	//extreme_debug_log << "rank: " << rank << " catalog went okay" << endl;

        // if(testing_logging) {
        //     testing_log << "using var attr by type var id funct: var_attr_entries associated with with run_id " << var.run_id <<
        //     	", var.timestep_id " << var.timestep_id << ", type_id " << type_id << 
        //         " and var " << var.name << " ver " << var.version << 
        //         " and rank: " << rank <<  "\n";
        //     print_var_attribute_list (num_var_attr_entries, var_attr_entries);
        //     // print_var_attr_data(num_var_attr_entries, var_attr_entries);
        // }
    // }
    //extreme_debug_log << "rank: " << rank << " about to gather_attr_entries" << endl; 
    gather_attr_entries(var_attr_entries, num_var_attr_entries, rank, num_client_procs, all_var_attr_entries, read_comm);
    // if(zero_rank_logging && rank == 0) {
    //     zero_rank_log << "rank: " << rank << " all var var_attr_entries associated with run_id " << var.run_id << 
    //     	" and var.timestep_id " << var.timestep_id << ", type_id " << type_id <<
    //         " and var_id: " << var.var_id << " note: all_var_attr_entries.size(): " << all_var_attr_entries.size() << endl;
    //     print_var_attribute_list (all_var_attr_entries.size(), all_var_attr_entries);
    // }
    add_timing_point(catalog_attr_done_code);
    //extreme_debug_log << "rank: " << rank << " catalog_attr_done_code" << endl; 

    return rc;
}


int do_read(int rank, const vector<md_dim_bounds> &dims,
	const md_catalog_var_entry &var, uint32_t num_client_procs,
	uint64_t txn_id, uint64_t type_id,
	MPI_Comm read_comm) 
{
	int rc = RC_OK;
    std::vector<md_catalog_var_attribute_entry> var_attr_entries;
    std::vector<md_catalog_var_attribute_entry> all_var_attr_entries;

    uint32_t num_var_attr_entries = 0;
	//extreme_debug_log << "rank: " << rank << " about to catalog" << endl;

	rc = metadata_catalog_all_var_attributes_with_type_var_dims_by_id (var.timestep_id, 
		type_id, var.var_id, txn_id, dims.size(), dims, num_var_attr_entries, var_attr_entries); 

	//extreme_debug_log << "rank: " << rank << " just finished the catalog" << endl;

    if (rc != RC_OK) {
        add_timing_point(ERR_CATALOG_ATTR);
        error_log << "error metadata_catalog_all_var_attributes_with_type_var_dims_by_id" << endl;
    }
    // else {
    	//extreme_debug_log << "rank: " << rank << " catalog went okay" << endl;

        // if(testing_logging) {
            // testing_log << "using var attr by type var dims id funct: var_attr_entries associated with with run_id " << var.run_id <<
        //     	", var.timestep_id " << var.timestep_id << ", type_id " << type_id << 
        //         " and var " << var.name << " ver " << var.version << 
        //         " and rank: " << rank <<  "\n";
        //     print_var_attribute_list (num_var_attr_entries, var_attr_entries);
        //     // print_var_attr_data(num_var_attr_entries, var_attr_entries);
        // }
    // }
    //extreme_debug_log << "rank: " << rank << " about to gather_attr_entries" << endl; 
	gather_attr_entries(var_attr_entries, num_var_attr_entries, rank, num_client_procs, all_var_attr_entries, read_comm);
 //    if(zero_rank_logging && rank == 0) {
	// 	zero_rank_log << "rank: " << rank << " all var attributes associated with run_id " << var.run_id << " and timestep_id " << var.timestep_id << 
	// 		" and type_id: " << type_id << " and var_id: " << var.var_id << " overlapping with dims:"; 
 //        for(int j=0; j< dims.size(); j++) {
 //            zero_rank_log << " d" << j << "_min: " << dims [j].min;
 //            zero_rank_log << " d" << j << "_max: " << dims [j].max;               
 //        }
	// 	zero_rank_log << " note: all_var_attr_entries.size(): " << all_var_attr_entries.size() << endl;
	// 	print_var_attribute_list (all_var_attr_entries.size(), all_var_attr_entries);
	// 	// print_var_attr_data(all_var_attr_entries.size(), all_var_attr_entries);
	// }	
	return rc;
}

int read_pattern_attrs(int rank, const vector<md_dim_bounds> &dims,
	const md_catalog_var_entry &var, uint32_t num_client_procs,
	uint64_t txn_id, uint64_t type_id,
	uint16_t catalog_attr_start_code, uint16_t catalog_attr_done_code,
	MPI_Comm read_comm
	)
{

    add_timing_point(catalog_attr_start_code);

	int rc = do_read(rank, dims, var, num_client_procs, txn_id, type_id, read_comm);

    add_timing_point(catalog_attr_done_code);
    //extreme_debug_log << "rank: " << rank << " catalog_attr_done_code" << endl; 

    return rc;
}

static void convert_dim_bounds_to_counts(const vector<md_dim_bounds> &dims, uint64_t *counts)
{	
	uint32_t num_dims = dims.size();

	if(num_dims >= 1) {
		counts[0] = dims[0].max - dims[0].min + 1;
		if(num_dims >= 2) {
			counts[1] = dims[1].max - dims[1].min + 1;
			if(num_dims >= 2) {
				counts[2] = dims[2].max - dims[2].min + 1;	
			}	
		} 
	}
}
