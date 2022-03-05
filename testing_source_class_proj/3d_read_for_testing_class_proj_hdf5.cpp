#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include <vector>

#include <math.h> //needed for pow()

#include <hdf5.h>



// #include <md_client_timing_constants.hh>
#include <3d_read_for_testing_class_proj_hdf5.hh>
// #include <client_timing_constants_read_class_proj_hdf5.hh>
#include <client_timing_constants_read_class_proj.hh>
#include <testing_harness_helper_functions_class_proj_hdf5.hh>
// #include <testing_harness_helper_functions_hdf5.hh>

using namespace std;

extern void add_timing_point(int catg);

// static bool extreme_debug_logging = false;
// static bool debug_logging = false;
// static bool error_logging = true;
// static bool testing_logging = false;

// static debugLog error_log = debugLog(error_logging);
// static debugLog //debug_log = debugLog(debug_logging);
// static debugLog //extreme_debug_log = debugLog(extreme_debug_logging);
// static debugLog //testing_log = debugLog(testing_logging);

// extern debugLog testing_log;
extern debugLog error_log;
// extern debugLog extreme_debug_log;
// extern debugLog debug_log;

// extern bool testing_logging;


void read_pattern_1 (int rank, string timestep_file_name, const vector<string> &var_names,
					const vector<string> &type_names, uint32_t num_client_procs, MPI_Comm read_comm  )
{
   	add_timing_point(READ_PATTERN_1_START);

	for(string var_name : var_names ) {
	    for (string type_name : type_names) {

			read_attrs(rank, num_client_procs, timestep_file_name, var_name, type_name,
					READ_PATTERN_1_START_CATALOGING_VAR_ATTRS, READ_PATTERN_1_DONE_CATALOGING_VAR_ATTRS, read_comm);
		} //end for (uint64_t type_id : type_ids)
	}
    add_timing_point(READ_PATTERN_1_DONE); 
}



void read_pattern_2 (int rank, string timestep_file_name, const vector<string> &type_names,
					string var_name, uint32_t num_client_procs, MPI_Comm read_comm  )
{
   	add_timing_point(READ_PATTERN_2_START);

    for (string type_name : type_names) {
		read_attrs(rank, num_client_procs, timestep_file_name, var_name, type_name,
				READ_PATTERN_2_START_CATALOGING_VAR_ATTRS, READ_PATTERN_2_DONE_CATALOGING_VAR_ATTRS, read_comm);

	} //end for (uint64_t type_id : type_ids)
    add_timing_point(READ_PATTERN_2_DONE); 
}



// 3. all of a few vars (3 for 3-d, for example)
void read_pattern_3 (int rank, string timestep_file_name, const vector<string> &type_names,
				const std::vector<string> &var_names, uint32_t num_client_procs, 
				MPI_Comm read_comm )
{
    
    add_timing_point(READ_PATTERN_3_START);

	for(string var_name : var_names ) {
	    for (string type_name : type_names) {

			read_attrs(rank, num_client_procs, timestep_file_name, var_name, type_name,
					READ_PATTERN_3_START_CATALOGING_VAR_ATTRS, READ_PATTERN_3_DONE_CATALOGING_VAR_ATTRS, read_comm);
		} //end for (uint64_t type_id : type_ids)
	}

    add_timing_point(READ_PATTERN_3_DONE);
}

// 4. 1 plane in each dimension for 1 variable
// note: this can only be used for 3 dimensional variables


void read_pattern_4 (int rank, string timestep_file_name, const vector<string> &type_names,
					string var_name, hsize_t *var_dims, uint32_t num_client_procs, MPI_Comm read_comm) 
{
    
    add_timing_point(READ_PATTERN_4_START);


    for (string type_name : type_names) {

	    for (int plane = 0; plane < 3; plane ++) {
			md_dim_bounds query_dims;
			query_dims.d0_min = 0;
			query_dims.d0_max = var_dims[0] - 1;
			query_dims.d1_min = 0;
			query_dims.d1_max = var_dims[1] - 1;	
			query_dims.d2_min = 0;
			query_dims.d2_max = var_dims[2] - 1;

			if(plane == 0) {
				query_dims.d0_min = var_dims[plane] / 2;
				query_dims.d0_max = query_dims.d0_min;
			}
			else if(plane == 1) {
				query_dims.d1_min = var_dims[plane] / 2;
				query_dims.d1_max = query_dims.d1_min;
			}
			else {
				query_dims.d2_min = var_dims[plane] / 2;
				query_dims.d2_max = query_dims.d2_min;	
			}

			read_attrs(rank, num_client_procs, timestep_file_name, var_name, type_name, query_dims,
					READ_PATTERN_4_START_CATALOGING_VAR_ATTRS, READ_PATTERN_4_DONE_CATALOGING_VAR_ATTRS, read_comm);	    	

		} //end read planes for given type
	} //end for (uint64_t type_id : type_ids)

    add_timing_point(READ_PATTERN_4_DONE); 
}



// 5. an arbitrary rectangular subset representing a cubic area of interest
// note - this is currently just set up for 3d vars
void read_pattern_5 (int rank, string timestep_file_name, const vector<string> &type_names,
					string var_name, hsize_t *var_dims, uint32_t num_client_procs, MPI_Comm read_comm)
{    

    add_timing_point(READ_PATTERN_5_START);

    int x_width = var_dims[0] / 2;
    int y_width = var_dims[1] / 2;
    int z_width = var_dims[2] / 2;

    uint32_t num_dims = 3;
   	md_dim_bounds query_dims;
    query_dims.d0_min = var_dims[0] / 4 ;
    query_dims.d0_max = query_dims.d0_min + x_width - 1;
    query_dims.d1_min = var_dims[1] / 4;
    query_dims.d1_max = query_dims.d1_min + y_width -1;
    query_dims.d2_min = var_dims[2] / 4;
    query_dims.d2_max = query_dims.d2_min + z_width -1;


    for (string type_name : type_names) {
		read_attrs(rank, num_client_procs, timestep_file_name, var_name, type_name, query_dims,
				READ_PATTERN_5_START_CATALOGING_VAR_ATTRS, READ_PATTERN_5_DONE_CATALOGING_VAR_ATTRS, read_comm);
	}

    add_timing_point(READ_PATTERN_5_DONE);

}

// 6. an arbitrary area on an orthogonal plane (decomposition dimensions) aka partial plane
// note: this can only be used for 3 dimensional variables
void read_pattern_6 (int rank, string timestep_file_name, const vector<string> &type_names,
					string var_name, hsize_t *var_dims, uint32_t num_client_procs, MPI_Comm read_comm) 
{
    
    add_timing_point(READ_PATTERN_6_START);

    int x_width, y_width;
    int num_dims = 3;
    hsize_t mins[num_dims];
    hsize_t maxes[num_dims];


  	for (string type_name : type_names) {
		for (int plane = 0; plane < num_dims; plane ++) {
		    mins[2-plane] = var_dims[2-plane] / 4;
		    maxes[2-plane] = mins[2-plane];

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
		    x_width = var_dims[x_dim] / 2;
		    y_width = var_dims[y_dim] / 2;

		    mins[x_dim] = var_dims[x_dim]/4;
		    maxes[x_dim] = mins[x_dim] + x_width - 1;

		    mins[y_dim] = var_dims[y_dim]/4;
		    maxes[y_dim] = mins[y_dim] + y_width - 1;

		    md_dim_bounds query_dims = md_dim_bounds(num_dims, mins[0], maxes[0], mins[1], maxes[1], mins[2], maxes[2]);

			read_attrs(rank, num_client_procs, timestep_file_name, var_name, type_name, query_dims,
					READ_PATTERN_6_START_CATALOGING_VAR_ATTRS, READ_PATTERN_6_DONE_CATALOGING_VAR_ATTRS, read_comm);
		}
	} //end for (uint64_t type_id : type_ids)

    add_timing_point(READ_PATTERN_6_DONE);

}



void read_attrs(int rank, uint32_t num_client_procs, string timestep_file_name, 
			string var_name, string type_name,
			uint16_t catalog_attr_start_code, uint16_t catalog_attr_done_code,
			MPI_Comm read_comm
			)
{

  	add_timing_point(catalog_attr_start_code);

  	md_timestep_entry timestep;

	vector<var_attribute_str> attr_entries;
	if(rank == 0) {
		open_timestep_file_and_var_attr_table_for_read(timestep_file_name, 
		timestep.file_id, timestep.var_attr_table_id);

	    metadata_catalog_all_var_attributes_with_type_var (timestep.var_attr_table_id, type_name, var_name, attr_entries);
		// if(testing_logging) {

		// 	testing_log << "rank: " << rank << " all var attributes associated with timestep " << 
		// 		timestep_file_name.substr(timestep_file_name.find_last_of("/")+1)<< 
		// 			" and type_name: " << type_name <<
		// 			" and var_name: " << var_name << endl;
		// 	print_var_attribute_list (attr_entries);
		// }
		close_timestep_file_and_attr_table(timestep.file_id, timestep.var_attr_table_id);
	}
	bcast_entries(attr_entries, rank, num_client_procs, read_comm);
    add_timing_point(catalog_attr_done_code);
}

void read_attrs(int rank, uint32_t num_client_procs, string timestep_file_name, 
			string var_name, string type_name, md_dim_bounds query_dims,
			uint16_t catalog_attr_start_code, uint16_t catalog_attr_done_code,
			MPI_Comm read_comm
			)
{

  	add_timing_point(catalog_attr_start_code);

  	md_timestep_entry timestep;

	vector<var_attribute_str> attr_entries;
	if(rank == 0) {
		open_timestep_file_and_var_attr_table_for_read(timestep_file_name, 
		timestep.file_id, timestep.var_attr_table_id);

	    metadata_catalog_all_var_attributes_with_type_var_dims (timestep.var_attr_table_id, type_name, var_name, query_dims, attr_entries);
		// if(testing_logging) {			
		// 	testing_log << "rank: " << rank << " all var attributes associated with timestep " << 
		// 		timestep_file_name.substr(timestep_file_name.find_last_of("/")+1)<< 
		// 		// found -= 2; //we don't want to count the checkpoint type as something separate
		// 			" and type_name: " << type_name <<
		// 			" and var_name: " << var_name <<
		// 			" and dims: ";
		// 	print_md_dim_bounds(query_dims);
		// 		// //testing_log << endl;
		// 	print_var_attribute_list (attr_entries);
		// }
		close_timestep_file_and_attr_table(timestep.file_id, timestep.var_attr_table_id);
	}
	bcast_entries(attr_entries, rank, num_client_procs, read_comm);
    add_timing_point(catalog_attr_done_code);
}
