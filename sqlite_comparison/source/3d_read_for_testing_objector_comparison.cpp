

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>

#include <sqlite_objector_comparison.hh>


// #include <3d_read_for_testing_objector_comparison.hh>
// #include <client_timing_constants.hh>
// #include <client_timing_constants_read.hh>

using namespace std;

// extern std::vector<int> catg_of_time_pts;
// extern std::vector<std::chrono::high_resolution_clock::time_point> time_pts;
extern void add_timing_point(uint16_t catg);
// extern void add_objector_point(int catg, int num_params_to_fetch);
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

static bool generate_object_names = true;

void print_obj_names(const vector<string> obj_names) {
	if(testing_logging) {
		cout << "beginning printing obj names \n";		
		for (string obj_name : obj_names) {
			cout << obj_name << endl;
		}
		cout << endl;
	}
}

void print_obj_list(const vector<md_catalog_object_entry> objs) {
	if(testing_logging) {
		cout << "beginning printing obj list \n";
		for (md_catalog_object_entry obj : objs) {
			cout << obj.object_name << endl;
		}
		cout << endl;
	}
}

int call_md_catalog_all_objects_with_var_dims_stub (const md_catalog_var_entry &var, const vector<md_dim_bounds> &dims)
{
	int rc;

	md_catalog_all_objects_with_var_dims_args args;
	// args.run_id = var.run_id;
	args.timestep_id = var.timestep_id;
	args.var_id = var.var_id;
	// args.num_dims = dims.size();
	args.dims = dims;

	std::vector<md_catalog_object_entry> object_list;
	uint32_t count;

	rc = md_catalog_all_objects_with_var_dims_stub (args, object_list, count);
	if (rc != RC_OK) {
		error_log << "error with md_catalog_all_objects_with_var_dims_stub \n";
	}
	else {
		print_obj_list(object_list);
	}

	return rc;
											
}


// 1. return data for all vars
int read_pattern_1 (const std::vector<md_dim_bounds> &proc_dims, 
                    const md_catalog_run_entry &run,
                    const std::vector<md_catalog_var_entry> &entries, int num_vars)
{
    
    add_timing_point(READ_PATTERN_1_START);

    int rc = RC_OK;

    for (int j=0; j<3; j++) {
        debug_log <<  "dims [" << to_string(j) << "] min is: " << to_string(proc_dims [j].min) << endl;
        debug_log <<  "dims [" << to_string(j) << "] max is: " << to_string(proc_dims [j].max) << endl;
    }    

    //each proc is responsible for a certain area of the sim space
    for(int i=0; i<num_vars; i++) {
       	md_catalog_var_entry var = entries.at(i);   

        debug_log << "read pattern 1 var: "<< i << endl;
        // debug_log << "beginning new var" << endl;
       
        extreme_debug_log << "var name is " << var.name << endl;
        extreme_debug_log << "var path is " << var.path << endl;
        extreme_debug_log << "var version is " << var.version << endl;

        std::vector<std::string> obj_names;
        std::vector<uint64_t> offsets_and_counts;

        if(generate_object_names) {
            add_timing_point(BOUNDING_BOX_TO_OBJ_NAMES_START);

	        rc = boundingBoxToObjNamesAndCounts (run, var, proc_dims, obj_names, offsets_and_counts);
	        if (rc != RC_OK) {
	            error_log << "error in boundingBoxToObjNamesAndCounts \n";
	            return rc;
	        }
	        add_timing_point(BOUNDING_BOX_TO_OBJ_NAMES_DONE);

	        print_obj_names(obj_names);
	    }


        add_timing_point(CATALOG_ALL_OBJECTS_WITH_VAR_DIMS_START);

        rc = call_md_catalog_all_objects_with_var_dims_stub (var, proc_dims);

        add_timing_point(CATALOG_ALL_OBJECTS_WITH_VAR_DIMS_DONE);

    }
    add_timing_point(READ_PATTERN_1_DONE); 

cleanup:
    return rc;
}


// 2. all of 1 var
int read_pattern_2 (const std::vector<md_dim_bounds> &proc_dims, 
                    const md_catalog_run_entry &run,
                    const md_catalog_var_entry &var
                    
	                )
{
    int rc = RC_OK;
    int index_of_var;
    
    add_timing_point(READ_PATTERN_2_START);

    std::vector<std::string> obj_names;
    std::vector<uint64_t> offsets_and_counts;

    debug_log << "read pattern 2 var: "<< var.var_id << endl;


    for (int j=0; j<3; j++) {
        debug_log <<  "dims [" << to_string(j) << "] min is: " << to_string(proc_dims [j].min) << endl;
        debug_log <<  "dims [" << to_string(j) << "] max is: " << to_string(proc_dims [j].max) << endl;
    }    

    if(generate_object_names) {
        add_timing_point(BOUNDING_BOX_TO_OBJ_NAMES_START);

        rc = boundingBoxToObjNamesAndCounts (run, var, proc_dims, obj_names, offsets_and_counts);
        if (rc != RC_OK) {
            error_log << "error in boundingBoxToObjNamesAndCounts \n";
            return rc;
        }
        add_timing_point(BOUNDING_BOX_TO_OBJ_NAMES_DONE);

        print_obj_names(obj_names);
    }

    add_timing_point(CATALOG_ALL_OBJECTS_WITH_VAR_DIMS_START);

    rc = call_md_catalog_all_objects_with_var_dims_stub (var, proc_dims);

    add_timing_point(CATALOG_ALL_OBJECTS_WITH_VAR_DIMS_DONE);

    add_timing_point(READ_PATTERN_2_DONE);

cleanup:

    return rc;
}

// 3. all of a few vars (3 for 3-d, for example)
int read_pattern_3 (const std::vector<md_dim_bounds> &proc_dims,
                    const md_catalog_run_entry &run,
                    const std::vector<md_catalog_var_entry> &vars 
                    
                    ) 
{
    int rc = RC_OK;

    add_timing_point(READ_PATTERN_3_START);

    for (int j=0; j<3; j++) {
        debug_log <<  "dims [" << to_string(j) << "] min is: " << to_string(proc_dims [j].min) << endl;
        debug_log <<  "dims [" << to_string(j) << "] max is: " << to_string(proc_dims [j].max) << endl;
    }    

    //one proc is in charge of a certain section of the sim space
    for (int i=0; i < vars.size(); i++) {
        md_catalog_var_entry var = vars.at(i);

        debug_log << "read pattern 3 var: "<< to_string(i) << endl;

        std::vector<std::string> obj_names;
        std::vector<uint64_t> offsets_and_counts;

	    if(generate_object_names) {
	        add_timing_point(BOUNDING_BOX_TO_OBJ_NAMES_START);

	        rc = boundingBoxToObjNamesAndCounts (run, var, proc_dims, obj_names, offsets_and_counts);
	        if (rc != RC_OK) {
	            error_log << "error in boundingBoxToObjNamesAndCounts \n";
	            return rc;
	        }
	        add_timing_point(BOUNDING_BOX_TO_OBJ_NAMES_DONE);

	        print_obj_names(obj_names);
	    }


        add_timing_point(CATALOG_ALL_OBJECTS_WITH_VAR_DIMS_START);

        rc = call_md_catalog_all_objects_with_var_dims_stub (var, proc_dims);

        add_timing_point(CATALOG_ALL_OBJECTS_WITH_VAR_DIMS_DONE);

    }

    add_timing_point(READ_PATTERN_3_DONE);


cleanup:
    return rc;
}


// 4. 1 plane in each dimension for 1 variable
int read_pattern_4 (int rank, int num_x_procs, int num_y_procs, int nx, int ny, int nz, 
                    const md_catalog_run_entry &run, const md_catalog_var_entry &var
                    
                    )
{
    
    add_timing_point(READ_PATTERN_4_START);

    int rc = RC_OK;

    int my_x_dim;
    int my_y_dim;

    int x_offset;
    int y_offset;

//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    //! do a plane in dim(3)
    my_x_dim = ny / num_x_procs;
    my_y_dim = nz / num_y_procs;

    x_offset = (rank % num_x_procs) * my_x_dim;
    y_offset = (rank/num_x_procs) * my_y_dim;

    extreme_debug_log << "my_x_dim: " << my_x_dim << endl;
    extreme_debug_log << "my_y_dim: " << my_y_dim << endl;
    extreme_debug_log << "x_offset: " << x_offset << endl;
    extreme_debug_log << "y_offset: " << y_offset << endl;

    uint32_t num_dims = 3;
    vector<md_dim_bounds> dims(num_dims);
    dims [0].min = nx / 2;
    dims [0].max = nx /2;
    dims [1].min = x_offset;
    dims [1].max = x_offset + my_x_dim - 1;
    dims [2].min = y_offset;
    dims [2].max = y_offset + my_y_dim - 1;
   
    debug_log << "read pattern 4 plane 1" << endl;
    for (int j=0; j<3; j++) {
        debug_log <<  "dims [" << to_string(j) << "] min is: " << to_string(dims [j].min) << endl;
        debug_log <<  "dims [" << to_string(j) << "] max is: " << to_string(dims [j].max) << endl;
    }  

    if(generate_object_names) {
    	std::vector<std::string> obj_names;
    	std::vector<uint64_t> offsets_and_counts;

        add_timing_point(BOUNDING_BOX_TO_OBJ_NAMES_START);

        rc = boundingBoxToObjNamesAndCounts (run, var, dims, obj_names, offsets_and_counts);
        if (rc != RC_OK) {
            error_log << "error in boundingBoxToObjNamesAndCounts \n";
            return rc;
        }
        add_timing_point(BOUNDING_BOX_TO_OBJ_NAMES_DONE);

        print_obj_names(obj_names);
    }


    add_timing_point(CATALOG_ALL_OBJECTS_WITH_VAR_DIMS_START);

    rc = call_md_catalog_all_objects_with_var_dims_stub (var, dims);

    add_timing_point(CATALOG_ALL_OBJECTS_WITH_VAR_DIMS_DONE);

 
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    //! do a plane in dim(2)
    my_x_dim = nx / num_x_procs;
    my_y_dim = nz / num_y_procs;
    x_offset = (rank % num_x_procs) * my_x_dim;
    y_offset = (rank/num_x_procs) * my_y_dim;

    extreme_debug_log << "my_x_dim: " << my_x_dim << endl;
    extreme_debug_log << "my_y_dim: " << my_y_dim << endl;
    extreme_debug_log << "x_offset: " << x_offset << endl;
    extreme_debug_log << "y_offset: " << y_offset << endl;

    dims [0].min = x_offset;
    dims [0].max = x_offset + my_x_dim - 1;
    dims [1].min = ny / 2;
    dims [1].max = ny / 2;
    dims [2].min = y_offset;
    dims [2].max = y_offset + my_y_dim - 1;

    debug_log << "read pattern 4 plane 2" << endl;
    for (int j=0; j<3; j++) {
        debug_log <<  "dims [" << to_string(j) << "] min is: " << to_string(dims [j].min) << endl;
        debug_log <<  "dims [" << to_string(j) << "] max is: " << to_string(dims [j].max) << endl;
    }  


    if(generate_object_names) {
    	std::vector<std::string> obj_names;
    	std::vector<uint64_t> offsets_and_counts;

        add_timing_point(BOUNDING_BOX_TO_OBJ_NAMES_START);

        rc = boundingBoxToObjNamesAndCounts (run, var, dims, obj_names, offsets_and_counts);
        if (rc != RC_OK) {
            error_log << "error in boundingBoxToObjNamesAndCounts \n";
            return rc;
        }
        add_timing_point(BOUNDING_BOX_TO_OBJ_NAMES_DONE);

        print_obj_names(obj_names);
    }

    add_timing_point(CATALOG_ALL_OBJECTS_WITH_VAR_DIMS_START);

    rc = call_md_catalog_all_objects_with_var_dims_stub (var, dims);

    add_timing_point(CATALOG_ALL_OBJECTS_WITH_VAR_DIMS_DONE);


//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    //! do a plane in dim(1)
    my_x_dim = nx / num_x_procs;
    my_y_dim = ny / num_y_procs;
    x_offset = (rank % num_x_procs) * my_x_dim;
    y_offset = (rank/num_x_procs) * my_y_dim;

    extreme_debug_log << "my_x_dim: " << to_string(my_x_dim) << endl;
    extreme_debug_log << "my_y_dim: " << to_string(my_y_dim) << endl;
    extreme_debug_log << "x_offset: " << to_string(x_offset) << endl;
    extreme_debug_log << "y_offset: " << to_string(y_offset) << endl;

    dims [0].min = x_offset;
    dims [0].max = x_offset + my_x_dim - 1;
    dims [1].min = y_offset;
    dims [1].max = y_offset + my_y_dim -1;
    dims [2].min = nz / 2;
    dims [2].max = nz / 2;

    debug_log << "read pattern 4 plane 3" << endl;
    for (int j=0; j<3; j++) {
        debug_log <<  "dims [" << to_string(j) << "] min is: " << to_string(dims [j].min) << endl;
        debug_log <<  "dims [" << to_string(j) << "] max is: " << to_string(dims [j].max) << endl;
    }  


    if(generate_object_names) {
    	std::vector<std::string> obj_names;
    	std::vector<uint64_t> offsets_and_counts;

        add_timing_point(BOUNDING_BOX_TO_OBJ_NAMES_START);

        rc = boundingBoxToObjNamesAndCounts (run, var, dims, obj_names, offsets_and_counts);
        if (rc != RC_OK) {
            error_log << "error in boundingBoxToObjNamesAndCounts \n";
            return rc;
        }
        add_timing_point(BOUNDING_BOX_TO_OBJ_NAMES_DONE);

        print_obj_names(obj_names);
    }


    add_timing_point(CATALOG_ALL_OBJECTS_WITH_VAR_DIMS_START);

    rc = call_md_catalog_all_objects_with_var_dims_stub (var, dims);

    add_timing_point(CATALOG_ALL_OBJECTS_WITH_VAR_DIMS_DONE);


    add_timing_point(READ_PATTERN_4_DONE); 
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
cleanup:
    debug_log << "initiating cleanup" << endl;

    return rc;
}

// 5. an arbitrary rectangular subset representing a cubic area of interest
int read_pattern_5 (int rank, int num_x_procs, int num_y_procs, int num_z_procs, 
                    int nx, int ny, int nz, const md_catalog_run_entry &run,
                    const md_catalog_var_entry &var
                    
                    )
{
    add_timing_point(READ_PATTERN_5_START);

    int rc = RC_OK;

    int my_x_dim;
    int my_y_dim;
    int my_z_dim;

    int x_offset;
    int y_offset;
    int z_offset;

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

    if(generate_object_names) {
    	std::vector<std::string> obj_names;
    	std::vector<uint64_t> offsets_and_counts;

        add_timing_point(BOUNDING_BOX_TO_OBJ_NAMES_START);

        rc = boundingBoxToObjNamesAndCounts (run, var, dims, obj_names, offsets_and_counts);
        if (rc != RC_OK) {
            error_log << "error in boundingBoxToObjNamesAndCounts \n";
            return rc;
        }
        add_timing_point(BOUNDING_BOX_TO_OBJ_NAMES_DONE);

        print_obj_names(obj_names);
    }



    add_timing_point(CATALOG_ALL_OBJECTS_WITH_VAR_DIMS_START);

    rc = call_md_catalog_all_objects_with_var_dims_stub (var, dims);

    add_timing_point(CATALOG_ALL_OBJECTS_WITH_VAR_DIMS_DONE);


    add_timing_point(READ_PATTERN_5_DONE);

cleanup:
    debug_log << "initiating cleanup" << endl;

    return rc;
}

// 6. an arbitrary area on an orthogonal plane (decomposition dimensions) aka partial plane
int read_pattern_6 (int rank, int num_x_procs, int num_y_procs, int nx, int ny, int nz, 
                    const md_catalog_run_entry &run, const md_catalog_var_entry &var
                    
                    )
{
    
    add_timing_point(READ_PATTERN_6_START);

    int rc = RC_OK;

    int my_x_dim;
    int my_y_dim;

    int x_offset;
    int y_offset;

   //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // read in dim(1)
    my_x_dim = ny / num_x_procs / 2;
    my_y_dim = nx / num_y_procs / 2;
    x_offset = (rank % num_x_procs) * my_x_dim;
    y_offset = (rank/num_x_procs) * my_y_dim;

    uint32_t num_dims = 3;
    vector<md_dim_bounds> dims(num_dims);
    dims [0].min = nx / 4 + y_offset;
    dims [0].max = nx / 4 + y_offset + my_y_dim - 1;
    dims [1].min = ny / 4 + x_offset;
    dims [1].max = ny / 4 + x_offset + my_x_dim -1;
    dims [2].min = nz / 4;
    dims [2].max = nz / 4;

    debug_log << "read pattern 6 plane 1" << endl;
    for (int j=0; j<3; j++) {
        debug_log <<  "dims [" << to_string(j) << "] min is: " << to_string(dims [j].min) << endl;
        debug_log <<  "dims [" << to_string(j) << "] max is: " << to_string(dims [j].max) << endl;
    }  


    if(generate_object_names) {
    	std::vector<std::string> obj_names;
    	std::vector<uint64_t> offsets_and_counts;

        add_timing_point(BOUNDING_BOX_TO_OBJ_NAMES_START);

        rc = boundingBoxToObjNamesAndCounts (run, var, dims, obj_names, offsets_and_counts);
        if (rc != RC_OK) {
            error_log << "error in boundingBoxToObjNamesAndCounts \n";
            return rc;
        }
        add_timing_point(BOUNDING_BOX_TO_OBJ_NAMES_DONE);

        print_obj_names(obj_names);
    }


    add_timing_point(CATALOG_ALL_OBJECTS_WITH_VAR_DIMS_START);

    rc = call_md_catalog_all_objects_with_var_dims_stub (var, dims);

    add_timing_point(CATALOG_ALL_OBJECTS_WITH_VAR_DIMS_DONE);

//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // read in dim(2)
    my_x_dim = nz / num_x_procs / 2;
    my_y_dim = nx / num_y_procs / 2;
    x_offset = (rank % num_x_procs) * my_x_dim;
    y_offset = (rank/num_x_procs) * my_y_dim;

    dims [0].min = nx / 4 + y_offset;
    dims [0].max = nx / 4 + y_offset + my_y_dim - 1;
    dims [1].min = ny / 4;
    dims [1].max = ny / 4;
    dims [2].min = nz / 4 + x_offset;
    dims [2].max = nz / 4 + x_offset + my_x_dim -1;
   
    debug_log << "read pattern 6 plane 2" << endl;
    for (int j=0; j<3; j++) {
        debug_log <<  "dims [" << to_string(j) << "] min is: " << to_string(dims [j].min) << endl;
        debug_log <<  "dims [" << to_string(j) << "] max is: " << to_string(dims [j].max) << endl;
    }  


    if(generate_object_names) {
    	std::vector<std::string> obj_names;
    	std::vector<uint64_t> offsets_and_counts;

        add_timing_point(BOUNDING_BOX_TO_OBJ_NAMES_START);

        rc = boundingBoxToObjNamesAndCounts (run, var, dims, obj_names, offsets_and_counts);
        if (rc != RC_OK) {
            error_log << "error in boundingBoxToObjNamesAndCounts \n";
            return rc;
        }
        add_timing_point(BOUNDING_BOX_TO_OBJ_NAMES_DONE);

        print_obj_names(obj_names);
    }

    add_timing_point(CATALOG_ALL_OBJECTS_WITH_VAR_DIMS_START);

    rc = call_md_catalog_all_objects_with_var_dims_stub (var, dims);

    add_timing_point(CATALOG_ALL_OBJECTS_WITH_VAR_DIMS_DONE);



//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // read in dim(3)
    my_x_dim = nz / num_x_procs / 2;
    my_y_dim = ny / num_y_procs / 2;
    x_offset = (rank % num_x_procs) * my_x_dim;
    y_offset = (rank/num_x_procs) * my_y_dim;

    dims [0].min = nx / 4;
    dims [0].max = nx / 4;
    dims [1].min = ny / 4 + y_offset;
    dims [1].max = ny / 4 + y_offset + my_y_dim - 1;
    dims [2].min = nz / 4 + x_offset;
    dims [2].max = nz / 4 + x_offset + my_x_dim -1;

    debug_log << "read pattern 6 plane 3" << endl;
    for (int j=0; j<3; j++) {
        debug_log <<  "dims [" << to_string(j) << "] min is: " << to_string(dims [j].min) << endl;
        debug_log <<  "dims [" << to_string(j) << "] max is: " << to_string(dims [j].max) << endl;
    }  

    if(generate_object_names) {
    	std::vector<std::string> obj_names;
    	std::vector<uint64_t> offsets_and_counts;

        add_timing_point(BOUNDING_BOX_TO_OBJ_NAMES_START);

        rc = boundingBoxToObjNamesAndCounts (run, var, dims, obj_names, offsets_and_counts);
        if (rc != RC_OK) {
            error_log << "error in boundingBoxToObjNamesAndCounts \n";
            return rc;
        }
        add_timing_point(BOUNDING_BOX_TO_OBJ_NAMES_DONE);

        print_obj_names(obj_names);
    }

    add_timing_point(CATALOG_ALL_OBJECTS_WITH_VAR_DIMS_START);

    rc = call_md_catalog_all_objects_with_var_dims_stub (var, dims);

    add_timing_point(CATALOG_ALL_OBJECTS_WITH_VAR_DIMS_DONE);

    add_timing_point(READ_PATTERN_6_DONE);
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

cleanup:
    debug_log << "initiating cleanup" << endl;
    return rc;
}


