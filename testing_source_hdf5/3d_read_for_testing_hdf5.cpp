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
#include <client_timing_constants_read_hdf5.hh>
#include <my_metadata_client_hdf5.hh>
#include <testing_harness_helper_functions_hdf5.hh>

using namespace std;

extern void add_timing_point(int catg);

// static bool extreme_debug_logging = false;
// static bool debug_logging = true;
// static bool error_logging = true;
// static bool testing_logging = true;

// static debugLog error_log = debugLog(error_logging);
// static debugLog debug_log = debugLog(debug_logging);
// static debugLog extreme_debug_log = debugLog(extreme_debug_logging);
// static debugLog testing_log = debugLog(testing_logging);

extern debugLog testing_log;
extern debugLog error_log;
extern debugLog extreme_debug_log;
extern debugLog debug_log;

extern bool testing_logging;

// static void find_data_range(int pattern, string timestep_file_name, string var_name, uint64_t ny, uint64_t nz, 
// 		uint32_t num_dims, const vector<double> &data_vect);

static void read_hyperslab(int pattern, hid_t file_id, string timestep_file_name, string var_name, uint32_t num_dims, hsize_t *offset, hsize_t *stride, 
		hsize_t *count, hsize_t *block, uint64_t chunk_vol);

static void read_hyperslab(int pattern, string timestep_file_name, string var_name, 
		hid_t var_id, hid_t var_data_space,
		uint32_t num_dims, hsize_t *offset, hsize_t *stride, 
		hsize_t *count, hsize_t *block, uint64_t chunk_vol);

template <class T>
static void bcast_entries(vector<T> &entries, int rank, uint32_t num_client_procs);

// 1. return data for all vars
void read_pattern_1 (hsize_t *offset, hsize_t *stride, hsize_t *count, hsize_t *block, 
					string timestep_file_name, std::vector<string> var_names, uint64_t chunk_vol, uint32_t num_dims )
{
    
    add_timing_point(READ_PATTERN_1_START);

    for (int j=0; j<num_dims; j++) {
        extreme_debug_log <<  "proc_dims [" << to_string(j) << "] min is: " << to_string(offset [j]) << endl;
        extreme_debug_log <<  "proc_dims [" << to_string(j) << "] max is: " << to_string(offset [j]+ count[j] -1) << endl;
    }    

    hid_t file_id;

	open_timestep_file_collectively_for_read(timestep_file_name, file_id);

    cout << "count 1: " << count[0] * count[1] * count[2] << endl;

	// add_timing_point(FILE_OPEN_DONE); 

    //each proc is responsible for a certain area of the sim space
    //it must get all the info for each var by asking each server
    for(int i=0; i<var_names.size(); i++) {

        add_timing_point(READ_PATTERN_1_START_READING_VAR_DATA);
        // extreme_debug_log << "beginning new var" << endl;

        string var_name = var_names.at(i);
        // debug_log << "read pattern 1 var: "<< var_name << endl;

		read_hyperslab(1, file_id, timestep_file_name, var_name, num_dims, offset, stride, count, block, chunk_vol);

        add_timing_point(READ_PATTERN_1_DONE_READING_VAR_DATA); 
    }
  	herr_t status = H5Fclose(file_id); assert(status >= 0);

    add_timing_point(READ_PATTERN_1_DONE); 

}


// 2. all of 1 var
void read_pattern_2 (hsize_t *offset, hsize_t *stride, hsize_t *count, hsize_t *block, 
					string timestep_file_name, string var_name, uint64_t chunk_vol, uint32_t num_dims
					)
{
    
    herr_t status;
    hid_t file_id;

	add_timing_point(READ_PATTERN_2_START);

    open_timestep_file_collectively_for_read(timestep_file_name, file_id);

	// add_timing_point(FILE_OPEN_DONE); 

    extreme_debug_log << "read pattern 2 timestep_file_name: "<< timestep_file_name << endl;


    // add_timing_point(READ_PATTERN_2_VAR_INIT_DONE_START_READING);

    add_timing_point(READ_PATTERN_2_START_READING_VAR_DATA);
    cout << "count 2: " << count[0] * count[1] * count[2] << endl;

 	read_hyperslab(2, file_id, timestep_file_name, var_name, num_dims, offset, stride, 
 		count, block, chunk_vol);

    add_timing_point(READ_PATTERN_2_DONE_READING_VAR_DATA); 

  	status = H5Fclose(file_id); assert(status >= 0);

    add_timing_point(READ_PATTERN_2_DONE); 
}

// void read_pattern_2_type (int rank, string timestep_file_name, const vector<string> &type_names,
// 					string var_name, md_dim_bounds proc_dims, uint32_t num_client_procs  )
// {
//    	add_timing_point(READ_PATTERN_2_TYPE_START);

// 	md_timestep_entry timestep;
// 	if(rank == 0) {
// 		open_timestep_file_and_var_attr_table_for_read(timestep_file_name, 
// 			timestep.file_id, timestep.var_attr_table_id);
// 	}
//     for (string type_name : type_names) {
//       	add_timing_point(READ_PATTERN_2_START_CATALOGING_VAR_ATTRS);

//     	vector<var_attribute_str> attr_entries;
//     	if(rank == 0) {
// 		    metadata_catalog_all_var_attributes_with_type_var (timestep.var_attr_table_id, type_name, var_name, attr_entries);
// 				testing_log << "rank: " << rank << " all var attributes associated with timestep_id 0"<< 
// 						" and type_name: " << type_name <<
// 						" and var_name: " << var_name << endl;
// 				print_var_attribute_list (attr_entries);
// 		}
// 		bcast_entries(attr_entries, rank, num_client_procs);
//         add_timing_point(READ_PATTERN_2_DONE_CATALOGING_VAR_ATTRS);

//     	add_timing_point(READ_PATTERN_2_START_READING_VAR_ATTR_DATA);
// 		read_data_for_attrs(timestep_file_name, attr_entries, proc_dims);
//     	add_timing_point(READ_PATTERN_2_DONE_READING_VAR_ATTR_DATA);


// 	} //end for (uint64_t type_id : type_ids)
// 	if(rank == 0) {
// 		close_timestep_file_and_attr_table(timestep.file_id, timestep.var_attr_table_id);
// 	}
//     add_timing_point(READ_PATTERN_2_TYPE_DONE); 
// }

void read_pattern_2_type (int rank, string timestep_file_name, const vector<string> &type_names,
					string var_name, md_dim_bounds proc_dims, uint32_t num_client_procs  )
{
   	add_timing_point(READ_PATTERN_2_TYPE_START);

	md_timestep_entry timestep;
    for (string type_name : type_names) {
      	add_timing_point(READ_PATTERN_2_START_CATALOGING_VAR_ATTRS);

    	vector<var_attribute_str> attr_entries;
    	if(rank == 0) {
    		open_timestep_file_and_var_attr_table_for_read(timestep_file_name, 
			timestep.file_id, timestep.var_attr_table_id);

		    metadata_catalog_all_var_attributes_with_type_var (timestep.var_attr_table_id, type_name, var_name, attr_entries);
				testing_log << "rank: " << rank << " all var attributes associated with timestep_id 0"<< 
						" and type_name: " << type_name <<
						" and var_name: " << var_name << endl;
				print_var_attribute_list (attr_entries);
			close_timestep_file_and_attr_table(timestep.file_id, timestep.var_attr_table_id);
		}
		bcast_entries(attr_entries, rank, num_client_procs);
        add_timing_point(READ_PATTERN_2_DONE_CATALOGING_VAR_ATTRS);

    	add_timing_point(READ_PATTERN_2_START_READING_VAR_ATTR_DATA);
		read_data_for_attrs(timestep_file_name, attr_entries, proc_dims);
    	add_timing_point(READ_PATTERN_2_DONE_READING_VAR_ATTR_DATA);


	} //end for (uint64_t type_id : type_ids)
    add_timing_point(READ_PATTERN_2_TYPE_DONE); 
}


// 3. all of a few vars (3 for 3-d, for example)
void read_pattern_3 (hsize_t *offset, hsize_t *stride, hsize_t *count, hsize_t *block, 
					string timestep_file_name, std::vector<string> var_names, uint64_t chunk_vol, uint32_t num_dims
					)
{
    
    herr_t status;  
    hid_t file_id;

    add_timing_point(READ_PATTERN_3_START);

    open_timestep_file_collectively_for_read(timestep_file_name, file_id);

    // extreme_debug_log << "read pattern 1 timestep_file_name: "<< timestep_file_name << endl;

	// add_timing_point(READ_PATTERN_3_FILE_OPEN_DONE); 

    //each proc is responsible for a certain area of the sim space
    //it must get all the info for each var by asking each server
    for(int i=0; i<var_names.size(); i++) {

        add_timing_point(READ_PATTERN_3_START_READING_VAR_DATA);
        // extreme_debug_log << "beginning new var" << endl;

        string var_name = var_names.at(i);
        
        read_hyperslab(3, file_id, timestep_file_name, var_name, num_dims, offset, stride, 
 			count, block, chunk_vol);

        add_timing_point(READ_PATTERN_3_DONE_READING_VAR_DATA); 
    }

  	status = H5Fclose(file_id); assert(status >= 0);
    add_timing_point(READ_PATTERN_3_DONE); 
}

// // 3. all of a few vars (3 for 3-d, for example)
// void read_pattern_3_type (int rank, string timestep_file_name, const vector<string> &type_names,
// 				const std::vector<string> &var_names, md_dim_bounds proc_dims, uint32_t num_client_procs )
// {
    
//     add_timing_point(READ_PATTERN_3_TYPE_START);

// 	md_timestep_entry timestep;
// 	if(rank == 0) {
// 		open_timestep_file_and_var_attr_table_for_read(timestep_file_name, 
// 			timestep.file_id, timestep.var_attr_table_id);
// 	}
//     for(int i=0; i<var_names.size(); i++) {
//     	string var_name = var_names.at(i);
// 	    for (string type_name : type_names) {
// 	      	add_timing_point(READ_PATTERN_3_START_CATALOGING_VAR_ATTRS);

// 	    	vector<var_attribute_str> attr_entries;
// 	    	if(rank == 0) {
// 			    metadata_catalog_all_var_attributes_with_type_var (timestep.var_attr_table_id, type_name, var_name, attr_entries);
// 				testing_log << "rank: " << rank << " all var attributes associated with timestep_id 0"<< 
// 						" and type_name: " << type_name <<
// 						" and var_name: " << var_name << endl;
// 				print_var_attribute_list (attr_entries);
// 			}
// 			bcast_entries(attr_entries, rank, num_client_procs);
// 	        add_timing_point(READ_PATTERN_3_DONE_CATALOGING_VAR_ATTRS);


//     		add_timing_point(READ_PATTERN_3_START_READING_VAR_ATTR_DATA);
// 			read_data_for_attrs(timestep_file_name, attr_entries, proc_dims);
//     		add_timing_point(READ_PATTERN_3_DONE_READING_VAR_ATTR_DATA);

// 		} //end for (uint64_t type_id : type_ids)
// 	}
// 	if(rank == 0) {
// 		close_timestep_file_and_attr_table(timestep.file_id, timestep.var_attr_table_id);
// 	}
//     add_timing_point(READ_PATTERN_3_TYPE_DONE);
// }


// 3. all of a few vars (3 for 3-d, for example)
void read_pattern_3_type (int rank, string timestep_file_name, const vector<string> &type_names,
				const std::vector<string> &var_names, md_dim_bounds proc_dims, uint32_t num_client_procs )
{
    
    add_timing_point(READ_PATTERN_3_TYPE_START);

	md_timestep_entry timestep;
    for(int i=0; i<var_names.size(); i++) {
    	string var_name = var_names.at(i);
	    for (string type_name : type_names) {
	      	add_timing_point(READ_PATTERN_3_START_CATALOGING_VAR_ATTRS);

	    	vector<var_attribute_str> attr_entries;
	    	if(rank == 0) {
	    		open_timestep_file_and_var_attr_table_for_read(timestep_file_name, 
					timestep.file_id, timestep.var_attr_table_id);

			    metadata_catalog_all_var_attributes_with_type_var (timestep.var_attr_table_id, type_name, var_name, attr_entries);
				testing_log << "rank: " << rank << " all var attributes associated with timestep_id 0"<< 
						" and type_name: " << type_name <<
						" and var_name: " << var_name << endl;
				print_var_attribute_list (attr_entries);
				close_timestep_file_and_attr_table(timestep.file_id, timestep.var_attr_table_id);
			}
			bcast_entries(attr_entries, rank, num_client_procs);
	        add_timing_point(READ_PATTERN_3_DONE_CATALOGING_VAR_ATTRS);


    		add_timing_point(READ_PATTERN_3_START_READING_VAR_ATTR_DATA);
			read_data_for_attrs(timestep_file_name, attr_entries, proc_dims);
    		add_timing_point(READ_PATTERN_3_DONE_READING_VAR_ATTR_DATA);

		} //end for (uint64_t type_id : type_ids)
	}
    add_timing_point(READ_PATTERN_3_TYPE_DONE);
}

// 4. 1 plane in each dimension for 1 variable
// note: this can only be used for 3 dimensional variables
void read_pattern_4 (int rank, uint32_t num_x_procs, uint32_t num_y_procs, hsize_t *stride, hsize_t *block,
					string timestep_file_name, string var_name, uint32_t num_dims )
{
    
    add_timing_point(READ_PATTERN_4_START);

    uint32_t my_x_dim, my_y_dim;
    uint64_t x_offset, y_offset;

	hsize_t plane_offset[3];
	hsize_t plane_count[3];

    herr_t status;
   	hid_t file_id, var_id, var_data_space;
	hsize_t var_dims[num_dims];

  	open_timestep_file_collectively_for_read(timestep_file_name, file_id);

	// add_timing_point(FILE_OPEN_DONE); 

    extreme_debug_log << "read pattern 4 timestep_file_name: "<< timestep_file_name << endl;

    extreme_debug_log << "read pattern 4 var: "<< var_name << endl;
    // extreme_debug_log << "beginning new var" << endl;


    extreme_debug_log << "num_x_procs: " << num_x_procs << " num_y_procs: " << num_y_procs << endl;

    var_id = H5Dopen(file_id, var_name.c_str(), H5P_DEFAULT);

    var_data_space = H5Dget_space(var_id);

	status = H5Sget_simple_extent_dims(var_data_space, var_dims, NULL);

    uint64_t plane_vol;
  
    for (int plane = 0; plane < 3; plane ++) {
	    plane_offset [plane] = var_dims[plane] / 2;
	    plane_count [plane] = 1;

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

	    plane_offset [x_dim] = x_offset;
	    plane_count [x_dim] = my_x_dim;

	    plane_offset [y_dim] = y_offset;
	    plane_count [y_dim] = my_y_dim;

	    plane_vol = plane_count[0] * plane_count[1] * plane_count[2];
   
	    debug_log << "read pattern 4 plane " << plane << endl;
	    for (int j=0; j<3; j++) {
	        debug_log <<  "dims [" << to_string(j) << "] min is: " << to_string(plane_offset[j]) << endl;
	        debug_log <<  "dims [" << to_string(j) << "] max is: " << to_string(plane_offset[j] + plane_count[j] -1) << endl;
	    }  

    	// add_timing_point(READ_PATTERN_4_PLANE_TO_FIND_INIT_DONE_START_READING_VAR_DATA); 

	    extreme_debug_log << "my_x_dim: " << my_x_dim << endl;
	    extreme_debug_log << "my_y_dim: " << my_y_dim << endl;
	    extreme_debug_log << "x_offset: " << x_offset << endl;
	    extreme_debug_log << "y_offset: " << y_offset << endl;

	    add_timing_point(READ_PATTERN_4_START_READING_VAR_DATA); 

	    read_hyperslab(4, timestep_file_name, var_name, var_id, var_data_space,
			num_dims, plane_offset, stride, plane_count, block, plane_vol);

	    add_timing_point(READ_PATTERN_4_DONE_READING_VAR_DATA); 
	}

 	// Close/release resources.
    status = H5Sclose(var_data_space); assert(status >= 0);
    status = H5Dclose(var_id); assert(status >= 0);
  	status = H5Fclose(file_id); assert(status >= 0);

    add_timing_point(READ_PATTERN_4_DONE); 
}

// 5. an arbitrary rectangular subset representing a cubic area of interest
// note - this is currently just set up for 3d vars
void read_pattern_5 ( int rank, uint32_t num_x_procs, uint32_t num_y_procs, uint32_t num_z_procs, 
				hsize_t *stride, hsize_t *block, string timestep_file_name, string var_name, uint32_t num_dims )
{
    add_timing_point(READ_PATTERN_5_START);

    herr_t status;
    hid_t var_data_space, var_id, file_id;

 	hsize_t var_dims[num_dims];
   	hsize_t count[num_dims];
	hsize_t offset[num_dims]; 

	uint64_t chunk_vol;

    extreme_debug_log << "num_x_procs: " << num_x_procs << endl;
    extreme_debug_log << "num_y_procs: " << num_y_procs << endl;
    extreme_debug_log << "num_z_procs: " << num_z_procs << endl;

	open_timestep_file_collectively_for_read(timestep_file_name, file_id);
	// add_timing_point(FILE_OPEN_DONE); 

    extreme_debug_log << "read pattern 5 timestep_file_name: "<< timestep_file_name << endl;
    extreme_debug_log << "read pattern 5 var: "<< var_name << endl;

    var_id = H5Dopen(file_id, var_name.c_str(), H5P_DEFAULT);

    var_data_space = H5Dget_space(var_id);

	status = H5Sget_simple_extent_dims(var_data_space, var_dims, NULL);

    extreme_debug_log << "nx: " << var_dims[0] << endl;
    extreme_debug_log << "ny: " << var_dims[1] << endl;
    extreme_debug_log << "nz: " << var_dims[2] << endl;

    count[0] = var_dims[0] / num_x_procs / 2;
    count[1] = var_dims[1] / num_y_procs / 2;
    count[2] = var_dims[2] / num_z_procs / 2;

    offset[0] = var_dims[0]/4 + (rank % num_x_procs) *  count[0];
    offset[1] = var_dims[1]/4 +((rank/num_x_procs) % num_y_procs) *  count[1];
    offset[2] = var_dims[2]/4 +(rank/(num_x_procs*num_y_procs) % num_z_procs) * count[2];

    extreme_debug_log << "count_x: " << count[0] << endl;
    extreme_debug_log << " count_y: " <<  count[1] << endl;
    extreme_debug_log << " count_z: " <<  count[2] << endl;
    extreme_debug_log << "x_offset: " << offset[0] << endl;
    extreme_debug_log << "y_offset: " << offset[1] << endl;
    extreme_debug_log << "z_offset: " << offset[2] << endl;

    for (int j=0; j<3; j++) {
        extreme_debug_log <<  "proc_dims [" << to_string(j) << "] min is: " << to_string(offset [j]) << endl;
        extreme_debug_log <<  "proc_dims [" << to_string(j) << "] max is: " << to_string(offset [j]+ count[j] -1) << endl;
    }    

    chunk_vol = count[0] * count[1] * count[2];

	add_timing_point(READ_PATTERN_5_START_READING_VAR_DATA); 

    read_hyperslab(5, timestep_file_name, var_name, var_id, var_data_space,
			num_dims, offset, stride, count, block, chunk_vol);

    add_timing_point(READ_PATTERN_5_DONE_READING_VAR_DATA); 


	// Close/release resources
    status = H5Sclose(var_data_space); assert(status >= 0);
    status = H5Dclose(var_id); assert(status >= 0);
  	status = H5Fclose(file_id); assert(status >= 0);


    add_timing_point(READ_PATTERN_5_DONE); 
}

// 6. an arbitrary area on an orthogonal plane (decomposition dimensions) aka partial plane
// note: this can only be used for 3 dimensional variables
void read_pattern_6 (int rank, uint32_t num_x_procs, uint32_t num_y_procs, hsize_t *stride, hsize_t *block,
					string timestep_file_name, string var_name, uint32_t num_dims )
{
    
    add_timing_point(READ_PATTERN_6_START);

    herr_t status;

    uint32_t my_x_dim, my_y_dim;
    uint64_t x_offset, y_offset;

	hsize_t plane_offset[3];
	hsize_t plane_count[3];

   	hid_t var_data_space, var_id, file_id;

   	open_timestep_file_collectively_for_read(timestep_file_name, file_id);

	// add_timing_point(FILE_OPEN_DONE); 

    extreme_debug_log << "read pattern 6 timestep_file_name: "<< timestep_file_name << endl;

    extreme_debug_log << "read pattern 6 var: "<< var_name << endl;
    // extreme_debug_log << "beginning new var" << endl;
	//
	// extreme_debug_log << "offset[0]: " << offset [0] << " offset[1]: " << offset [1] << " offset[2]: " << offset [2] << endl;
	// extreme_debug_log << "count[0]: " << count [0] << " count[1]: " << count [1] << " count[2]: " << count [2] << endl;
	// extreme_debug_log << "var_dims[0]: " << var_dims [0] << " var_dims[1]: " << var_dims [1] << " var_dims[2]: " << var_dims [2] << endl;


    extreme_debug_log << "num_x_procs: " << num_x_procs << " num_y_procs: " << num_y_procs << endl;

   	var_id = H5Dopen(file_id, var_name.c_str(), H5P_DEFAULT);

    var_data_space = H5Dget_space(var_id);

	hsize_t var_dims[num_dims];
	status = H5Sget_simple_extent_dims(var_data_space, var_dims, NULL);

    uint64_t plane_vol;
  
    for (int plane = 0; plane < 3; plane ++) {
	    plane_offset [2-plane] = var_dims[2-plane] / 4;
	    plane_count [2-plane] = 1;

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

	    plane_offset [x_dim] = var_dims[x_dim]/4 + x_offset;
	    plane_count [x_dim] = my_x_dim;

	    plane_offset [y_dim] = var_dims[y_dim]/4 + y_offset;
	    plane_count [y_dim] = my_y_dim;

	    plane_vol = plane_count[0] * plane_count[1] * plane_count[2];
   
	    debug_log << "read pattern 6 plane " << plane << endl;
	    for (int j=0; j<3; j++) {
	        debug_log <<  "dims [" << to_string(j) << "] min is: " << to_string(plane_offset[j]) << endl;
	        debug_log <<  "dims [" << to_string(j) << "] max is: " << to_string(plane_offset[j] + plane_count[j] -1) << endl;
	    }  

    	// add_timing_point(READ_PATTERN_6_PLANE_TO_FIND_INIT_DONE_START_READING_VAR_DATA); 

   
    	// add_timing_point(READ_PATTERN_6_DONE_READING_VAR_DATA); 

	    extreme_debug_log << "my_x_dim: " << my_x_dim << endl;
	    extreme_debug_log << "my_y_dim: " << my_y_dim << endl;
	    extreme_debug_log << "x_offset: " << x_offset << endl;
	    extreme_debug_log << "y_offset: " << y_offset << endl;

        add_timing_point(READ_PATTERN_6_START_READING_VAR_DATA);

	    read_hyperslab(6, timestep_file_name, var_name, var_id, var_data_space,
			num_dims, plane_offset, stride, plane_count, block, plane_vol);

	    add_timing_point(READ_PATTERN_6_DONE_READING_VAR_DATA);

	}

 	/*
     * Close/release resources.
     */
	status = H5Sclose(var_data_space); assert(status >= 0);
	status = H5Dclose(var_id); assert(status >= 0);
  	status = H5Fclose(file_id); assert(status >= 0);

    add_timing_point(READ_PATTERN_6_DONE); 
}


static void read_hyperslab(int pattern, hid_t file_id, string timestep_file_name, string var_name, uint32_t num_dims, 
		hsize_t *offset, hsize_t *stride, hsize_t *count, hsize_t *block, uint64_t chunk_vol)
{
	hid_t var_data_space, var_id;
	herr_t status;

    var_id = H5Dopen(file_id, var_name.c_str(), H5P_DEFAULT); assert(var_id >= 0);

    //get the storage layout of the var
    var_data_space = H5Dget_space(var_id); assert(var_data_space >= 0);

	read_hyperslab(pattern, timestep_file_name, var_name, var_id, var_data_space,
		num_dims, offset, stride, count, block, chunk_vol);

	// Close/release resources
    status = H5Dclose(var_id); assert(status >= 0);
    status = H5Sclose(var_data_space); assert(status >= 0); 	
}

static void read_hyperslab(int pattern, string timestep_file_name, string var_name, 
		hid_t var_id, hid_t var_data_space,
		uint32_t num_dims, hsize_t *offset, hsize_t *stride, 
		hsize_t *count, hsize_t *block, uint64_t chunk_vol)
{

	hid_t chunk_data_space, property_list_id;

	vector<double> data_vct(chunk_vol);

	herr_t status;
	
	//select which hyperslab in storage to read
 	status = H5Sselect_hyperslab(var_data_space, H5S_SELECT_SET, offset, stride, count, block); assert(status >= 0);

 	//maintain the stored layout out the data reading it back in
	chunk_data_space  = H5Screate_simple(num_dims, count, NULL); assert(chunk_data_space >= 0);

    // Create property list for collective dataset read
    property_list_id = H5Pcreate(H5P_DATASET_XFER); assert(property_list_id >= 0);
    status = H5Pset_dxpl_mpio(property_list_id, H5FD_MPIO_COLLECTIVE); assert(status >= 0);

	status = H5Dread (var_id, H5T_NATIVE_DOUBLE, chunk_data_space, var_data_space,
               property_list_id, &data_vct[0]); assert(status >= 0);

    // Close/release resources
    status = H5Sclose(chunk_data_space); assert(status >= 0);
 	status = H5Pclose(property_list_id); assert(status >= 0);

 	if(testing_logging) {
   		find_data_range(pattern, timestep_file_name, var_name, var_data_space, num_dims, data_vct);
   	}
}

template <class T>
static void bcast_entries(vector<T> &entries, int rank, uint32_t num_client_procs) {
	add_timing_point(BCAST_ENTRIES_COLLECTIVE_START);

	extreme_debug_log.set_rank(rank);

	int length_ser_c_str = 0;
	char *serialized_c_str;

	extreme_debug_log << "rank " << rank << " entries.size(): " <<entries.size() << endl;

	if (rank == 0 && entries.size() > 0) {
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

	MPI_Bcast(&length_ser_c_str, 1, MPI_INT, 0, MPI_COMM_WORLD);

	if(length_ser_c_str > 0) {
		if(rank != 0) {
			serialized_c_str = (char *) malloc(length_ser_c_str);
		}
	 	MPI_Bcast(serialized_c_str, length_ser_c_str, MPI_CHAR, 0, MPI_COMM_WORLD);
		// if(rank == 1) {
		// 	cout << "length_ser_c_str: " << length_ser_c_str << endl;
		// 	cout << "serialized_c_str: " << serialized_c_str << endl;
		// }

		if(rank != 0) {
	        stringstream ss1;
	        ss1.write(serialized_c_str, length_ser_c_str);
	        // if (rank == 1) {
	        // 	extreme_debug_log << "about to deserialize ss: " << ss1.str() << endl;
	        // }
	        boost::archive::text_iarchive ia(ss1);
	        ia >> entries;
	       //  if (rank == 1) {
	      	// 	extreme_debug_log << "rank " << rank << " received entries.size(): " << entries.size() << endl;
	      	// }
		}
	    free(serialized_c_str);
	}

	add_timing_point(BCAST_ENTRIES_COLLECTIVE_DONE);
}

