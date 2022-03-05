#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>

#include <3d_read_for_testing.hh>
// #include <md_client_timing_constants.hh>
#include <client_timing_constants_read.hh>
#include <testing_harness_debug_helper_functions.hh> //note - this is just to use while debugging for print functs
#include <my_metadata_client_lua_functs.h> //note- this is just to use while debugging for print functs/using retrieving obj names

#include <hdf5_helper_functions_read.hh>

using namespace std;

// extern std::vector<int> catg_of_time_pts;
// extern std::vector<std::chrono::high_resolution_clock::time_point> time_pts;
extern void add_timing_point(int catg);
extern void add_objector_point(int catg, int num_params_to_fetch);
// extern void add_objector_output_point(int time_catg, int num_params_to_fetch, uint16_t read_type);

static bool extreme_debug_logging = false;
static bool debug_logging = false;
static bool error_logging = true;
static bool testing_logging = false;
static bool zero_rank_logging = false;

static debugLog error_log = debugLog(error_logging);
static debugLog debug_log = debugLog(debug_logging);
static debugLog extreme_debug_log = debugLog(extreme_debug_logging);
static debugLog testing_log = debugLog(testing_logging);

static debugLog zero_rank_log = debugLog(true);

static bool generate_obj_names = false;

extern vector<objector_params> all_objector_params;
extern bool output_objector_params;
extern bool output_obj_names;
extern bool hdf5_read_data;


template <class T>
extern void gather_attr_entries(vector<T> attr_entries, uint32_t count, int rank, uint32_t num_servers, uint32_t num_client_procs,
        vector<T> &all_attr_entries);

extern objector_params get_objector_params ( const md_catalog_run_entry &run, const md_catalog_var_entry &var, 
                            const vector<md_dim_bounds> &bounding_box, OBJECTOR_PARAMS_READ_TYPES read_type );

extern bool dims_overlap(const vector<md_dim_bounds> &attr_dims, const vector<md_dim_bounds> &proc_dims);

extern int add_object_names_offests_and_counts(const md_catalog_run_entry &run, const md_catalog_var_entry &var,
                                    const vector<md_dim_bounds> &proc_dims, uint16_t read_type);

extern vector<md_dim_bounds> get_overlapping_dims_for_bounding_box ( const vector<md_dim_bounds> &bounding_box, 
                            const vector<md_dim_bounds> &proc_dims
                            );

int read_data(int64_t timestep_file_id, const md_catalog_run_entry &run, const md_catalog_var_entry &var, 
			const vector<md_dim_bounds> &proc_dims, OBJECTOR_PARAMS_READ_TYPES catg, uint16_t data_read_start_code,
			uint16_t data_read_done_code
			);

int read_pattern_attrs(md_server server, int rank, const vector<md_dim_bounds> &proc_dims, 
	const md_catalog_run_entry &run, 
	const md_catalog_var_entry &var, uint32_t num_servers, uint32_t num_client_procs, 
	uint64_t txn_id, uint64_t type_id,
	uint16_t catalog_attr_start_code, uint16_t catalog_attr_done_code
	);

// extern void gather_attr_entries(vector<md_catalog_var_attribute_entry> attr_entries, uint32_t count, int rank, uint32_t num_servers, uint32_t num_client_procs,
//         vector<md_catalog_var_attribute_entry> &all_attr_entries);

// 1. return data for all vars
int read_pattern_1 (const std::vector<md_dim_bounds> &proc_dims, 
                    const md_catalog_run_entry &run,
                    const std::vector<md_catalog_var_entry> &entries, int num_vars, uint64_t txn_id)
{
    
    add_timing_point(READ_PATTERN_1_START);

    int rc = RC_OK;

    for (int j=0; j<3; j++) {
        extreme_debug_log <<  "proc_dims [" << to_string(j) << "] min is: " << to_string(proc_dims [j].min) << endl;
        extreme_debug_log <<  "proc_dims [" << to_string(j) << "] max is: " << to_string(proc_dims [j].max) << endl;
    }    

    // add_timing_point(READ_PATTERN_1_VAR_INIT_DONE_START_READING);
    // add_timing_point(READ_PATTERN_1_VAR_INIT_DONE_START_READING);

    int64_t timestep_file_id;
    if(hdf5_read_data) {
    	hdf5_open_timestep_file_collectively_for_read(run.name, run.job_id, entries.at(0).timestep_id, timestep_file_id);
    }

    //each proc is responsible for a certain area of the sim space
    for(int i=0; i<num_vars; i++) {
       	md_catalog_var_entry var = entries.at(i);   

	   	rc = read_data(timestep_file_id, run, var, proc_dims, PATTERN_1, READ_PATTERN_1_START_READING_VAR_DATA, READ_PATTERN_1_DONE_READING_VAR_DATA);
        
    }
    if(hdf5_read_data) {
    	hdf5_close_timestep_file(timestep_file_id);
    }

    add_timing_point(READ_PATTERN_1_DONE); 

cleanup:
    return rc;
}


// 2. all of 1 var
int read_pattern_2 (int rank, const std::vector<md_dim_bounds> &proc_dims, 
                    int num_servers, int num_client_procs, const md_server &server, bool is_type,
                    uint64_t txn_id, const md_catalog_run_entry &run,
                    const md_catalog_var_entry &var, 
                    const vector<uint64_t> &type_ids
                    )
{
    int rc = RC_OK;
    int index_of_var;

    int64_t timestep_file_id;
    if (hdf5_read_data) {
		hdf5_open_timestep_file_collectively_for_read(run.name, run.job_id, var.timestep_id, timestep_file_id);
    }
    
    add_timing_point(READ_PATTERN_2_START);

    if (!is_type) {
	   	rc = read_data(timestep_file_id, run, var, proc_dims, PATTERN_2, READ_PATTERN_2_START_READING_VAR_DATA, READ_PATTERN_2_DONE_READING_VAR_DATA);
	}
    else { //is_type, the data for the features of interest act as a substitute for reading the whole var

	    for (uint64_t type_id : type_ids) {

			read_pattern_attrs(server, rank, proc_dims, run, var, num_servers, num_client_procs, txn_id, type_id,
				READ_PATTERN_2_START_CATALOGING_VAR_ATTRS, READ_PATTERN_2_DONE_CATALOGING_VAR_ATTRS);	    	

		} //end for (uint64_t type_id : type_ids)
	} //end else (is_type)

	if (hdf5_read_data) {
		hdf5_close_timestep_file(timestep_file_id);

	}

    add_timing_point(READ_PATTERN_2_DONE);

cleanup:

    return rc;
}

// 3. all of a few vars (3 for 3-d, for example)
//todo - note: when you're using the one client per server queries paradigm, none of these patterns need the entire list of servers
int read_pattern_3 (int rank, const std::vector<md_dim_bounds> &proc_dims, int num_servers, 
                    int num_client_procs,
                    const md_server &server,  bool is_type,
                    uint64_t txn_id, const md_catalog_run_entry &run,
                    const std::vector<md_catalog_var_entry> &vars, 
                    const vector<uint64_t> &type_ids
                    ) 
{
    int rc = RC_OK;

    add_timing_point(READ_PATTERN_3_START);

    // for (int j=0; j<3; j++) {
    //     debug_log <<  "proc_dims [" << to_string(j) << "] min is: " << to_string(proc_dims [j].min) << endl;
    //     debug_log <<  "proc_dims [" << to_string(j) << "] max is: " << to_string(proc_dims [j].max) << endl;
    // }    

    int64_t timestep_file_id;
    if(hdf5_read_data) {
    	hdf5_open_timestep_file_collectively_for_read(run.name, run.job_id, vars.at(0).timestep_id, timestep_file_id);
    }

    // add_timing_point(READ_PATTERN_3_VAR_INIT_DONE_START_READING);

    //one proc is in charge of a certain section of the sim space
    //it has to get all info for all vars in that space, this involves asking each server for the info
    for (int i=0; i < vars.size(); i++) {
        md_catalog_var_entry var = vars.at(i);
        if(! is_type ) {

	   		rc = read_data(timestep_file_id, run, var, proc_dims, PATTERN_3, READ_PATTERN_3_START_READING_VAR_DATA, READ_PATTERN_3_DONE_READING_VAR_DATA);
      
	    }
	    else { //is_type, the data for the features of interest act as a substitute for reading the whole var

	        for (uint64_t type_id : type_ids) {
				read_pattern_attrs(server, rank, proc_dims, run, var, num_servers, num_client_procs, txn_id, type_id,
					READ_PATTERN_3_START_CATALOGING_VAR_ATTRS, READ_PATTERN_3_DONE_CATALOGING_VAR_ATTRS);	    	

			} //end for (uint64_t type_id : type_ids)
		}
    }

    if(hdf5_read_data) {
    	hdf5_close_timestep_file(timestep_file_id);
    }


    add_timing_point(READ_PATTERN_3_DONE);


cleanup:
    return rc;
}


// 4. 1 plane in each dimension for 1 variable
// int read_pattern_4 (int rank, int num_servers, const std::vector<md_server> &servers, 
//                     bool is_type, int num_x_procs, int num_y_procs, int nx, int ny, int nz, 
//                     uint64_t txn_id, const md_catalog_run_entry &run, const md_catalog_var_entry &var)
int read_pattern_4 (int rank, int num_x_procs, int num_y_procs, int nx, int ny, int nz, 
                    uint64_t txn_id, const md_catalog_run_entry &run, const md_catalog_var_entry &var)
{
    
    add_timing_point(READ_PATTERN_4_START);

    int rc = RC_OK;

   	uint32_t my_x_dim, my_y_dim;
    uint64_t x_offset, y_offset;

    int64_t timestep_file_id, var_id, var_data_space;

    unsigned long long var_dims[var.num_dims];

    convert_dim_bounds_to_counts(var.dims, var_dims);

    if(hdf5_read_data) {
    	hdf5_open_timestep_file_collectively_for_read(run.name, run.job_id, var.timestep_id, timestep_file_id);

		hdf5_open_var(timestep_file_id, var.name, var.version, var_id); 

    	var_data_space = hdf5_get_dataspace(var_id);

    }

    vector<md_dim_bounds> plane_dims(3);

    for (int plane = 0; plane < 3; plane ++) {
    	plane_dims[plane].min = var_dims[plane] / 2;
    	plane_dims[plane].max = plane_dims[plane].min;

	    int x_dim;
    	int y_dim;

	    if(plane == 0 ) {
	    	x_dim = 1;
	    }
	    else { 
	    	x_dim = 0;
	    }
	    if(plane == 2) {
	    	y_dim = 1;
	    }
	    else {
	    	y_dim = 2;
	    }

	    my_x_dim = var_dims[x_dim] / num_x_procs;
	    my_y_dim = var_dims[y_dim] / num_y_procs;

	    x_offset = (rank % num_x_procs) * my_x_dim;
    	y_offset = (rank/num_x_procs) * my_y_dim;

    	plane_dims[x_dim].min = x_offset;
    	plane_dims[x_dim].max = x_offset + my_x_dim - 1;

    	plane_dims[y_dim].min = y_offset;
    	plane_dims[y_dim].max = y_offset + my_y_dim - 1;

	    // plane_vol = my_x_dim * my_y_dim;
   
	    debug_log << "read pattern 4 plane " << plane << endl;
	    for (int j=0; j<3; j++) {
	        debug_log <<  "dims [" << to_string(j) << "] min is: " << plane_dims[j].min << endl;
	        debug_log <<  "dims [" << to_string(j) << "] max is: " << plane_dims[j].max << endl;
	    }  

    	// add_timing_point(READ_PATTERN_4_PLANE_TO_FIND_INIT_DONE_START_READING_VAR_DATA); 

	    extreme_debug_log << "my_x_dim: " << my_x_dim << endl;
	    extreme_debug_log << "my_y_dim: " << my_y_dim << endl;
	    extreme_debug_log << "x_offset: " << x_offset << endl;
	    extreme_debug_log << "y_offset: " << y_offset << endl;

	   	rc = read_data(timestep_file_id, run, var, plane_dims, PATTERN_4, READ_PATTERN_4_START_READING_VAR_DATA, READ_PATTERN_4_DONE_READING_VAR_DATA);

	}

	if (hdf5_read_data) {
	 	// Close/release resources.
	    hdf5_close_dataspace(var_data_space); 
		hdf5_close_var(var_id);
    	hdf5_close_timestep_file(timestep_file_id);
	}

    add_timing_point(READ_PATTERN_4_DONE); 
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
cleanup:
    debug_log << "initiating cleanup" << endl;

    return rc;
}


// 5. an arbitrary rectangular subset representing a cubic area of interest
// int read_pattern_5 (int rank, int num_servers, const std::vector<md_server> &servers, 
//                     bool is_type, int num_x_procs, int num_y_procs, int num_z_procs, 
//                     int nx, int ny, int nz, uint64_t txn_id, const md_catalog_run_entry &run,
//                     const md_catalog_var_entry &var)
int read_pattern_5 (int rank, int num_x_procs, int num_y_procs, int num_z_procs, 
                    int nx, int ny, int nz, uint64_t txn_id, const md_catalog_run_entry &run,
                    const md_catalog_var_entry &var)
{
    add_timing_point(READ_PATTERN_5_START);

    int rc = RC_OK;

    int my_x_dim;
    int my_y_dim;
    int my_z_dim;

    int x_offset;
    int y_offset;
    int z_offset;

    int64_t timestep_file_id;

    if(hdf5_read_data) {
    	hdf5_open_timestep_file_collectively_for_read(run.name, run.job_id, var.timestep_id, timestep_file_id);
    }


    extreme_debug_log << "num_x_procs: " << num_x_procs << endl;
    extreme_debug_log << "num_y_procs: " << num_y_procs << endl;
    extreme_debug_log << "num_z_procs: " << num_z_procs << endl;
    extreme_debug_log << "nx: " << nx << endl;
    extreme_debug_log << "ny: " << ny << endl;
    extreme_debug_log << "nz: " << nz << endl;

    my_x_dim = nx / num_x_procs / 2;
    my_y_dim = ny / num_y_procs / 2;
    my_z_dim = nz / num_z_procs / 2;
    x_offset = (rank % num_x_procs) * my_x_dim;
    y_offset = ((rank/num_x_procs) % num_y_procs) * my_y_dim;
    z_offset = (rank/(num_x_procs*num_y_procs) % num_z_procs) * my_z_dim;

    extreme_debug_log << "my_x_dim: " << my_x_dim << endl;
    extreme_debug_log << "my_y_dim: " << my_y_dim << endl;
    extreme_debug_log << "my_z_dim: " << my_z_dim << endl;
    extreme_debug_log << "x_offset: " << x_offset << endl;
    extreme_debug_log << "y_offset: " << y_offset << endl;
    extreme_debug_log << "z_offset: " << z_offset << endl;

    uint32_t num_dims = 3;
    vector<md_dim_bounds> dims(num_dims);
    dims [0].min = nx / 4 + x_offset;
    dims [0].max = nx / 4 + x_offset + my_x_dim - 1;
    dims [1].min = ny / 4 + y_offset;
    dims [1].max = ny / 4 + y_offset + my_y_dim -1;
    dims [2].min = nz / 4 + z_offset;
    dims [2].max = nz / 4 + z_offset + my_z_dim -1;

    debug_log << "read pattern 5" << endl;
    for (int j=0; j<3; j++) {
        debug_log <<  "dims [" << to_string(j) << "] min is: " << to_string(dims [j].min) << endl;
        debug_log <<  "dims [" << to_string(j) << "] max is: " << to_string(dims [j].max) << endl;
    }  

	rc = read_data(timestep_file_id, run, var, dims, PATTERN_5, READ_PATTERN_5_START_READING_VAR_DATA, READ_PATTERN_5_DONE_READING_VAR_DATA);



    if(hdf5_read_data) {
		hdf5_close_timestep_file(timestep_file_id);
    }

    add_timing_point(READ_PATTERN_5_DONE);

cleanup:
    debug_log << "initiating cleanup" << endl;

    return rc;
}

// 6. an arbitrary area on an orthogonal plane (decomposition dimensions) aka partial plane
// int read_pattern_6 (int rank, int num_servers, const std::vector<md_server> &servers, 
//                     bool is_type, int num_x_procs, int num_y_procs, int nx, int ny, int nz, 
//                     uint64_t txn_id, const md_catalog_run_entry &run, const md_catalog_var_entry &var)
int read_pattern_6 (int rank, int num_x_procs, int num_y_procs, int nx, int ny, int nz, 
                    uint64_t txn_id, const md_catalog_run_entry &run, const md_catalog_var_entry &var)
{
    
    add_timing_point(READ_PATTERN_6_START);

    int rc = RC_OK;

    int my_x_dim;
    int my_y_dim;

    int x_offset;
    int y_offset;


    int64_t timestep_file_id, var_id, var_data_space;
    unsigned long long var_dims[var.num_dims];

    convert_dim_bounds_to_counts(var.dims, var_dims);

    if(hdf5_read_data) {
    	hdf5_open_timestep_file_collectively_for_read(run.name, run.job_id, var.timestep_id, timestep_file_id);

		hdf5_open_var(timestep_file_id, var.name, var.version, var_id); 

    	var_data_space = hdf5_get_dataspace(var_id);
    }

  	vector<md_dim_bounds> plane_dims(3);

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

	    my_x_dim = var_dims[x_dim] / num_x_procs / 2;
	    my_y_dim = var_dims[y_dim] / num_y_procs / 2;

	    x_offset = (rank % num_x_procs) * my_x_dim;
    	y_offset = (rank/num_x_procs) * my_y_dim;


	    plane_dims [x_dim].min = var_dims[x_dim]/4 + x_offset;
	    plane_dims [x_dim].max = plane_dims[x_dim].min + my_x_dim - 1;

	    plane_dims [y_dim].min = var_dims[y_dim]/4 + y_offset;
	    plane_dims [y_dim].max = plane_dims[y_dim].min + my_y_dim - 1;

	    // plane_vol = my_x_dim * my_y_dim;
   
	    debug_log << "read pattern 6 plane " << plane << endl;
	    for (int j=0; j<3; j++) {
	        debug_log <<  "dims [" << to_string(j) << "] min is: " << plane_dims [j].min << endl;
	        debug_log <<  "dims [" << to_string(j) << "] max is: " << plane_dims [j].max << endl;
	    }  

	   	rc = read_data(timestep_file_id, run, var, plane_dims, PATTERN_6, READ_PATTERN_6_START_READING_VAR_DATA, READ_PATTERN_6_DONE_READING_VAR_DATA);
	}

	if (hdf5_read_data) {
	    hdf5_close_dataspace(var_data_space); 
		hdf5_close_var(var_id);
    	hdf5_close_timestep_file(timestep_file_id);
    }

    add_timing_point(READ_PATTERN_6_DONE);
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

cleanup:
    debug_log << "initiating cleanup" << endl;
    return rc;
}


int read_data(int64_t timestep_file_id, const md_catalog_run_entry &run, const md_catalog_var_entry &var, 
			const vector<md_dim_bounds> &proc_dims, OBJECTOR_PARAMS_READ_TYPES catg, uint16_t data_read_start_code,
			uint16_t data_read_done_code
			) 
{
	int rc = RC_OK;

    if(generate_obj_names) {
        add_timing_point(BOUNDING_BOX_TO_OBJ_NAMES_START);

        std::vector<std::string> obj_names;
        std::vector<uint64_t> offsets_and_counts;

        rc = boundingBoxToObjNamesAndCounts (run, var, proc_dims, obj_names, offsets_and_counts);
        if (rc != RC_OK) {
            error_log << "error in boundingBoxToObjNamesAndCounts \n";
            return rc;
        }
        //note - this is where the data should then be retrieved

        add_timing_point(BOUNDING_BOX_TO_OBJ_NAMES_DONE);
    }
	else if (output_objector_params) {
		all_objector_params.push_back( get_objector_params(run, var, proc_dims, catg) );

        add_objector_point(BOUNDING_BOX_TO_OBJ_NAMES, 1);
    }
    else if (output_obj_names) {
        rc = add_object_names_offests_and_counts(run, var, proc_dims, catg);  
        if (rc != RC_OK) {
            error_log << "error in add_object_names_offests_and_counts \n";
            return rc;
        }
        // NOTE: THIS IS WHERE DATA WOULD BE READ
    }
    else if(hdf5_read_data) {
        add_timing_point(data_read_start_code);

       	hdf5_read_hyperslab(timestep_file_id, var, proc_dims);

        add_timing_point(data_read_done_code);
    }
   	return rc;
}



int read_pattern_attrs(md_server server, int rank, const vector<md_dim_bounds> &proc_dims, 
	const md_catalog_run_entry &run, 
	const md_catalog_var_entry &var, uint32_t num_servers, uint32_t num_client_procs,
	uint64_t txn_id, uint64_t type_id,
	uint16_t catalog_attr_start_code, uint16_t catalog_attr_done_code
	)
{
	int rc = RC_OK;

    add_timing_point(catalog_attr_start_code);

    std::vector<md_catalog_var_attribute_entry> var_attr_entries;
    vector<md_catalog_var_attribute_entry> all_var_attr_entries;

    uint32_t num_var_attr_entries = 0;
    if(rank < num_servers) {

    	rc = metadata_catalog_all_var_attributes_with_type_var_by_id (server, var.timestep_id, 
    		type_id, var.var_id, txn_id, num_var_attr_entries, var_attr_entries); 

        if (rc != RC_OK) {
            add_timing_point(ERR_CATALOG_ATTR);
        }
        else {
            // if(testing_logging) {
            //     testing_log << "using var attr by var dims id funct: var_attr_entries associated with with run_id " << var.run_id <<
            //     	", var.timestep_id " << var.timestep_id << ", type_id " << type_id << 
            //         " and var " << var.name << " ver " << var.version << 
            //         " and rank: " << rank <<  "\n";
            //     print_var_attribute_list (num_var_attr_entries, var_attr_entries);
            //     print_var_attr_data(num_var_attr_entries, var_attr_entries);
            // }
        }
    }
    gather_attr_entries(var_attr_entries, num_var_attr_entries, rank, num_servers, num_client_procs, all_var_attr_entries);
    if(zero_rank_logging && rank == 0) {
        zero_rank_log << "rank: " << rank << " all var var_attr_entries associated with run_id " << var.run_id << 
        	" and var.timestep_id " << var.timestep_id << ", type_id " << type_id <<
            " and var_id: " << var.var_id << " note: all_var_attr_entries.size(): " << all_var_attr_entries.size() << endl;
        print_var_attribute_list (all_var_attr_entries.size(), all_var_attr_entries);
        print_var_attr_data(all_var_attr_entries.size(), all_var_attr_entries);
    }
    add_timing_point(catalog_attr_done_code);

    if (output_objector_params) {
    	num_var_attr_entries = 0;
        for (md_catalog_var_attribute_entry attr : all_var_attr_entries) {
       		if (dims_overlap ( attr.dims, proc_dims) ) {
       			extreme_debug_log << "for rank: " << rank << " found a matching bb" << endl;
				all_objector_params.push_back( get_objector_params(run, var, attr.dims, PATTERN_2_ATTRS) );
				num_var_attr_entries += 1;
			}
        }
        add_objector_point(BOUNDING_BOX_TO_OBJ_NAMES, num_var_attr_entries);	        
    }
    else if (output_obj_names) {
        for (md_catalog_var_attribute_entry attr : all_var_attr_entries) {
       		if (dims_overlap ( attr.dims, proc_dims) ) {
	            rc = add_object_names_offests_and_counts(run, var, get_overlapping_dims_for_bounding_box(attr.dims, proc_dims), PATTERN_2_ATTRS);  
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
		read_data_for_attrs(run, var, all_var_attr_entries, proc_dims);
    }

    return rc;
}
