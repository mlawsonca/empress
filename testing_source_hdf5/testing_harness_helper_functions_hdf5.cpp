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


#include <stdlib.h> //needed for atoi/atol/malloc/free/rand
#include <stdint.h> //needed for uint
#include <vector>
#include <mpi.h>
#include <hdf5_hl.h>
#include <hdf5.h>

#include <my_metadata_client_hdf5.hh>
#include <testing_harness_helper_functions_hdf5.hh>
#include <client_timing_constants_read_hdf5.hh>


using namespace std;

extern bool read_data;

extern void add_timing_point(int catg);

// static bool extreme_debug_logging = false;
// static bool debug_logging = false;
// static bool error_logging = true;
// static bool testing_logging = true;

// static bool zero_rank_logging = true;
// static debugLog error_log = debugLog(error_logging, zero_rank_logging);
// static debugLog debug_log = debugLog(debug_logging, zero_rank_logging);
// static debugLog extreme_debug_log = debugLog(extreme_debug_logging, zero_rank_logging);
// static debugLog testing_log = debugLog(testing_logging, zero_rank_logging);

extern debugLog testing_log;
extern debugLog error_log;
extern debugLog extreme_debug_log;
extern debugLog debug_log;

extern bool testing_logging;


void find_data_range(int pattern, string timeste_file_name, string var_name, hid_t var_data_space, 
		uint32_t num_dims, const vector<double> &data_vect) {

	uint64_t ny = 0;
	uint64_t nz = 0;

	hsize_t var_dims[num_dims];
	herr_t status = H5Sget_simple_extent_dims(var_data_space, var_dims, NULL);

	if(num_dims >= 2) {
		ny = var_dims[1];
		if(num_dims >= 3) {
			nz = var_dims[2];
		}
	}

	double first_val = data_vect.at(0);
	double last_val = data_vect.at(data_vect.size()-1);
	// debug_log << "first val: " << first_val << " last val: " << last_val << endl;

    uint64_t number = ny;

    int y_digits = 0;
    while (number > 0) {
        number /= 10;
        y_digits++;
    }
    y_digits++;

    uint32_t z_digits = 0;
    uint64_t number2 = nz;

    while (number2 > 0) {
        number2 /= 10;
        z_digits++;
    }
    z_digits++;


    uint64_t x1 = first_val / pow(10.0, y_digits) - 1;
    uint64_t x2 = last_val / pow(10.0, y_digits) - 1;
    uint64_t y1 = first_val - (x1+1)* pow(10.0, y_digits) -1;
    uint64_t y2 = last_val - (x2+1)* pow(10.0, y_digits) -1;

    extreme_debug_log << "ny: " << ny << " first_val-1: " << first_val-1 << " (first_val-1) mod ny: " << (int)(first_val-1) % ny << endl;

    if(num_dims == 3) {
        uint64_t z1 = round((first_val - (int)first_val)*pow(10.0, z_digits-1) -1);
        uint64_t z2 = round((last_val - (int)last_val)*pow(10.0, z_digits-1) -1);
        printf("data range for %s/%s pattern %d: (%d, %d, %d),(%d, %d, %d)\n",timeste_file_name.c_str(),var_name.c_str(),pattern,x1,y1,z1,x2,y2,z2);  
    }
    else if(num_dims == 2) {
        printf("data range for %s/%s pattern %d: (%d, %d),(%d, %d)\n",timeste_file_name.c_str(),var_name.c_str(),pattern,x1,y1,x2,y2);  
    }
}


void find_data_range(hid_t var_data_space, var_attribute_str attr,
	const vector<double> &data_vect) {

	uint64_t ny = 0;
	uint64_t nz = 0;

	hsize_t var_dims[attr.num_dims];
	herr_t status = H5Sget_simple_extent_dims(var_data_space, var_dims, NULL);

	if(attr.num_dims >= 2) {
		ny = var_dims[1];
		if(attr.num_dims >= 3) {
			nz = var_dims[2];
		}
	}

	double first_val = data_vect.at(0);
	double last_val = data_vect.at(data_vect.size()-1);
	// debug_log << "first val: " << first_val << " last val: " << last_val << endl;

    uint64_t number = ny;

    int y_digits = 0;
    while (number > 0) {
        number /= 10;
        y_digits++;
    }
    y_digits++;

    uint32_t z_digits = 0;
    uint64_t number2 = nz;

    while (number2 > 0) {
        number2 /= 10;
        z_digits++;
    }
    z_digits++;


    uint64_t x1 = first_val / pow(10.0, y_digits) - 1;
    uint64_t x2 = last_val / pow(10.0, y_digits) - 1;
    uint64_t y1 = first_val - (x1+1)* pow(10.0, y_digits) -1;
    uint64_t y2 = last_val - (x2+1)* pow(10.0, y_digits) -1;

    extreme_debug_log << "ny: " << ny << " first_val-1: " << first_val-1 << " (first_val-1) mod ny: " << (int)(first_val-1) % ny << endl;

    if(attr.num_dims == 3) {
        uint64_t z1 = round((first_val - (int)first_val)*pow(10.0, z_digits-1) -1);
        uint64_t z2 = round((last_val - (int)last_val)*pow(10.0, z_digits-1) -1);
        if (x1 == attr.d0_min && x2 == attr.d0_max &&
        	y1 == attr.d1_min && y2 == attr.d1_max &&
        	z1 == attr.d2_min && z2 == attr.d2_max ) {
        	testing_log << "attr dims matches data range:";
         	print_attr_dims(attr);
         	testing_log << endl;
        }
        else {
	        error_log << "error. attr dims does NOT match data range: ";
	        print_attr_dims(attr);
	        printf("data range: x: (%d, %d), y: (%d, %d), z:(%d, %d)\n",x1,x2,y1,y2,z1,z2);  
	    }
    }
    else if(attr.num_dims == 2) {
        if (x1 == attr.d0_min && x2 == attr.d0_max &&
        	y1 == attr.d1_min && y2 == attr.d1_max ) {
        	testing_log << "attr dims matches data range:";
         	print_attr_dims(attr);
         	testing_log << endl;
        }
        else {
	        error_log << "error. attr dims does NOT match data range: ";
	       	print_attr_dims(attr);
	        printf("data range: x: (%d, %d), y: (%d, %d)\n",x1,x2,y1,y2); 
	    } 
    }
}
void find_data_range(int ny, int nz, int num_dims, const vector<double> &data_vect) {

	double first_val = data_vect.at(0);
	double last_val = data_vect.at(data_vect.size()-1);
	debug_log << "first val: " << first_val << " last val: " << last_val << endl;

    int number = ny;

    int y_digits = 0;
    while (number > 0) {
        number /= 10;
        y_digits++;
    }
    y_digits++;

    int z_digits = 0;
    int number2 = nz;

    while (number2 > 0) {
        number2 /= 10;
        z_digits++;
    }
    z_digits++;


    uint64_t x1 = first_val / pow(10.0, y_digits) - 1;
    uint64_t x2 = last_val / pow(10.0, y_digits) - 1;
    uint64_t y1 = first_val - (x1+1)* pow(10.0, y_digits) -1;
    uint64_t y2 = last_val - (x2+1)* pow(10.0, y_digits) -1;

    extreme_debug_log << "ny: " << ny << " first_val-1: " << first_val-1 << " (first_val-1) mod ny: " << (int)(first_val-1) % ny << endl;

    if(num_dims == 3) {
        uint64_t z1 = round((first_val - (int)first_val)*pow(10.0, z_digits-1) -1);
        uint64_t z2 = round((last_val - (int)last_val)*pow(10.0, z_digits-1) -1);
        printf("data range for vctr is: (%d, %d, %d),(%d, %d, %d)\n",x1,y1,z1,x2,y2,z2);  
    }
    else if(num_dims == 2) {
        printf("data range for vctr is: (%d, %d),(%d, %d)\n",x1,y1,x2,y2);              
    }
}



void generate_data_for_proc(int ny, int nz, hsize_t *offset,
                        int rank, vector<double> &data_vct, 
                        uint64_t ndx, uint64_t ndy, uint64_t ndz, uint64_t chunk_vol) 
{
	// data_vct.reserve(chunk_vol);

    int number = ny;

    int y_digits = 0;
    while (number > 0) {
        number /= 10;
        y_digits++;
    }
    y_digits++;

    int z_digits = 0;
    int number2 = nz;

    while (number2 > 0) {
        number2 /= 10;
        z_digits++;
    }
    z_digits++;

    extreme_debug_log << "ndx: " << ndx << " ndy: " << ndy << " ndz: " << ndz << endl;

    // uint32_t offset = z + NZ(y + NY*x)
    // uint32_t offset = z1 + output.nz*(y1 + output.ny*x1);
    // extreme_debug_log << "starting offset: " << offset << " rank: " << rank << endl;

    uint64_t x1 = offset[0];
    uint64_t y1 = offset[1];
    uint64_t z1 = offset[2];

    extreme_debug_log << "x1: " << x1 << " y1: " << y1 << " z1: " << z1 << endl;


    extreme_debug_log << "chunk vol: " << chunk_vol << endl;

    for(int i=0; i<chunk_vol; i++) {

        uint64_t z = z1 + i % ndz;  
        uint64_t y = y1 + (i / ndz)%ndy;
        // uint32_t x = (i+offset) / (ndz * ndy);
        uint64_t x = x1 + i / (ndz * ndy);

        double x_portion_of_value = (x+1) * pow(10.0, y_digits);
        // extreme_debug_log << "y: " << y << endl;
        double y_portion_of_value = (y+1);
        double z_portion_of_value = (z+1)/pow(10.0, z_digits-1);
        // extreme_debug_log << "x_portion_of_value: " << x_portion_of_value << " y_portion_of_value: " << y_portion_of_value << " z_portion_of_value: " << z_portion_of_value << endl;

        double val = (x+1) * pow(10.0, y_digits) + (y+1) +(z+1)/pow(10.0, z_digits-1);
        if(rank == 1 && i == chunk_vol-1){
            extreme_debug_log << "val: " << val << " x_portion_of_value: " << x_portion_of_value << " y_portion_of_value: " << y_portion_of_value << " z_portion_of_value: " << z_portion_of_value << endl;

            // extreme_debug_log << "z_portion_of_value: " << z_portion_of_value << " val: " << val << endl;
        }
        data_vct[i] = val;
    }
}



int debug_testing(int rank, hsize_t *chunk_dims, int num_client_procs, string run_name, int num_timesteps, int num_vars,
				int num_dims, hsize_t *var_dims,
				string job_id, vector<string> var_names, uint32_t version1, uint32_t version2, bool write_data
				) 
{
    int rc = RC_OK;
    uint32_t count;
    herr_t	status;

    hsize_t chunk_vol = chunk_dims[0] * chunk_dims[1] * chunk_dims[2];
	std::vector<double> all_data_vct (chunk_vol * num_client_procs);

	debug_log << "num_client_procs: " << num_client_procs << endl;
	debug_log << "chunk_vol: " << chunk_vol << endl;

   	
   	for (int timestep_id = 0; timestep_id < num_timesteps; timestep_id++) {

   		string FILENAME = run_name + "/" + job_id + "/" + to_string(timestep_id);
   		extreme_debug_log << "FILENAME in read: " << FILENAME << endl;

   		// /*
	    // * Set up file access property list with parallel I/O access
	    // */
	    // //create a property list for file access
	    // hid_t property_list_id = H5Pcreate(H5P_FILE_ACCESS); assert(property_list_id >= 0);
	    // status = H5Pset_fapl_mpio(property_list_id, MPI_COMM_WORLD, MPI_INFO_NULL); assert(status >= 0);


	    // hid_t file_id = H5Fopen(FILENAME.c_str(), H5F_ACC_RDONLY, property_list_id); assert(file_id >= 0);

	   	// status = H5Pclose(property_list_id); assert(status >= 0);
   		// extreme_debug_log << "got here" << endl;
	    hid_t file_id = H5Fopen(FILENAME.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT); assert(file_id >= 0);
   		// extreme_debug_log << "got here 2" << endl;
	    hid_t packet_table_id = H5PTopen(file_id, "Var Attribute Table"); assert(packet_table_id >= 0);
   		// extreme_debug_log << "got here 3" << endl;


		vector<var_attribute_str> attrs;
    	metadata_catalog_all_var_attributes(packet_table_id, attrs);
    	print_var_attribute_list (attrs);


	   	// extreme_debug_log << "got here \n";

	   	num_vars = 10; //fix 
	    for (int var_indx = 0; var_indx < num_vars; var_indx++) {

	    	uint32_t var_version;
	    	if(var_indx == 1 || var_indx == 3) { //these have been designated ver2
	    		var_version = version2;
	    	}
	    	else {
	    		var_version = version1;
	    	}

	    	// extreme_debug_log << "var_indx: " << var_indx << " var_name: " << var_names.at(var_indx) << " var_version: " << var_version << endl;
	    	string VARNAME = var_names.at(var_indx) + to_string(var_version);


   			hid_t var_id = H5Dopen(file_id, VARNAME.c_str(),H5P_DEFAULT); assert(var_id >= 0);

			if(rank == 0 && write_data) {

			   	status = H5Dread (var_id, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL,
			                       H5P_DEFAULT, &all_data_vct[0]);  assert(status >= 0);

			    testing_log << "Data in File after Subset is Written:\n";

				find_data_range(var_dims[1], var_dims[2], num_dims, all_data_vct);

			    // for (int x = 0; x < var_dims[0]; x++){
			    //    for (int y = 0; y<var_dims[1]; y++) {
			    //    		for (int z = 0; z<var_dims[2]; z++) {
			    //    			uint64_t offset = z +  var_dims[2]*(y + var_dims[1]*x);
			    //        		printf ("x:%d y:%d z:%d val:%.6f ", x, y, z, all_data_vct[offset]);
			    //        	}
			    //    }
			    //    printf ("\n");
			    // }	
				// extreme_debug_log << "about to iterate" << endl;
			 //    status = H5Aiterate2(var_id, H5_INDEX_NAME, H5_ITER_INC, NULL, attr_info, NULL);
  		// 		extreme_debug_log << "done iterating \n";
			}

			status = H5Dclose(var_id); assert(status >= 0);

		}

		status = H5PTclose(packet_table_id); assert(status >= 0);
	    status = H5Fclose(file_id); assert(status >= 0);
	}

    //     } //for each timestep_id
    // } //for each run
    return rc;
}



void gatherv_int(vector<int> values, uint32_t num_client_procs, int rank, int *each_proc_num_values,
            int *displacement_for_each_proc, int **all_values)
{   
    int num_values = values.size();

    MPI_Gather(&num_values, 1, MPI_INT, each_proc_num_values, 1, MPI_INT, 0, MPI_COMM_WORLD);

    int sum = 0;
    if (rank == 0 ) {
        for(int i=0; i<num_client_procs; i++) {
            displacement_for_each_proc[i] = sum;
            sum += each_proc_num_values[i];
            // if(each_proc_num_values[i] != 0 && rank == 0) {
            //     extreme_debug_log << "rank " << i << " has a string of length " << each_proc_num_values[i] << endl;
            // }
        }
        *all_values = (int *) malloc(sum * sizeof(int));

        // extreme_debug_log << "sum: " << sum << endl;
    }

    MPI_Gatherv(&values[0], num_values, MPI_INT,
           *all_values, each_proc_num_values, displacement_for_each_proc,
           MPI_INT, 0, MPI_COMM_WORLD);

}


// void print_attr_data (string attr_data ) {
// 	string deserialized_test_string;
// 	int deserialized_test_int;

//     stringstream ss;
//     ss << attr_data;
//     boost::archive::text_iarchive ia(ss);
//     // extreme_debug_log << "when retrieving attr_data, string.size(): " << test_string.size() << endl;
//     ia >> deserialized_test_string;
//     ia >> deserialized_test_int;

//     testing_log << "data: blob: str: " << deserialized_test_string << " int: " << deserialized_test_int << endl; 
// }

void print_attr_data(var_attribute_str attr) {
	// extreme_debug_log << "serialized attr data: " << attr.data << endl;
    if( attr.type_name.find("blob") != string::npos ) {
        bool deserialized_bool;

        stringstream sso;
        sso << attr.data;
        boost::archive::text_iarchive ia(sso);
        ia >> deserialized_bool;

        testing_log << "data: bool: " << deserialized_bool << endl; 
    }
    else if( attr.type_name.find("val") != string::npos ) {
        double deserialized_test_real;

        stringstream sso;
        sso << attr.data;
        boost::archive::text_iarchive ia(sso);
        // extreme_debug_log << "about to deserialize str: " << sso.str() << endl;
        ia >> deserialized_test_real;

        testing_log << "data: double: " << deserialized_test_real << endl; 
    }           
    else if( attr.type_name.find("note") != string::npos ) {
            // string deserialized_test_string;
        testing_log << "data: text: " << attr.data << endl; 
    } 
    else if( attr.type_name.find("range") != string::npos ) {
        vector<int> deserialized_vals(2);

        stringstream sso;
        sso << attr.data;
        boost::archive::text_iarchive ia(sso);
        ia >> deserialized_vals;
        testing_log << "data: blob: val1: " << deserialized_vals.at(0) << " val2: " << deserialized_vals.at(1) << endl; 
    }
    else {
    	testing_log << "error. attr type: " << attr.type_name << " didn't fit into one of the expected categories for a var attr" << endl;
    }    
 }

void print_attr_data(non_var_attribute_str attr) {
	// extreme_debug_log << "serialized attr data: " << attr.data << endl;
    if( attr.type_name.find("max") != string::npos || attr.type_name.find("min") != string::npos ) {
        double deserialized_test_real;

        stringstream sso;
        sso << attr.data;
        boost::archive::text_iarchive ia(sso);
        ia >> deserialized_test_real;

        testing_log << "data: double: " << deserialized_test_real << endl; 
    } 
    else {
    	error_log << "error. attr type: " << attr.type_name << " didn't fit into one of the expected categories for a non var attr" << endl;
    }         
}

void open_timestep_file_collectively_for_write(string FILENAME, hid_t &file_id)
{
	/*
    * Set up file access property list with parallel I/O access
    */
    //create a property list for file access
	hid_t property_list_id = H5Pcreate(H5P_FILE_ACCESS); assert(property_list_id >= 0);

    herr_t status = H5Pset_fapl_mpio(property_list_id, MPI_COMM_WORLD, MPI_INFO_NULL); assert(status >= 0);

    file_id = H5Fopen(FILENAME.c_str(), H5F_ACC_RDWR, property_list_id); assert(file_id >= 0);

    status = H5Pclose(property_list_id); assert(status >= 0);

}


void open_timestep_file_collectively_for_write(const string &run_name, string job_id, uint64_t timestep_id, hid_t &file_id)
{
	string FILENAME = run_name + "/" + job_id + "/" + to_string(timestep_id);
	extreme_debug_log << "FILENAME: " << FILENAME << endl;

    open_timestep_file_collectively_for_write(FILENAME, file_id);
}

void open_timestep_file_collectively_for_read(string FILENAME, hid_t &file_id)
{
	/*
    * Set up file access property list with parallel I/O access
    */
    //create a property list for file access
	hid_t property_list_id = H5Pcreate(H5P_FILE_ACCESS); assert(property_list_id >= 0);

    herr_t status = H5Pset_fapl_mpio(property_list_id, MPI_COMM_WORLD, MPI_INFO_NULL); assert(status >= 0);

    file_id = H5Fopen(FILENAME.c_str(), H5F_ACC_RDONLY, property_list_id); assert(file_id >= 0);

    status = H5Pclose(property_list_id); assert(status >= 0);

}


void open_timestep_file_collectively_for_read(const string &run_name, string job_id, uint64_t timestep_id, hid_t &file_id)
{
	string FILENAME = run_name + "/" + job_id + "/" + to_string(timestep_id);
	extreme_debug_log << "FILENAME: " << FILENAME << endl;

    open_timestep_file_collectively_for_read(FILENAME, file_id);
}


void read_data_for_attrs(string timestep_file_name, vector<var_attribute_str> attrs,
			 md_dim_bounds proc_dims)
{

    add_timing_point(READ_DATA_FOR_ATTRS_START);

   	hid_t timestep_file_id;
   	open_timestep_file_collectively_for_read(timestep_file_name, timestep_file_id);

    for (var_attribute_str attr : attrs) {
   		if (dims_overlap ( attr, proc_dims) ) {
      		get_overlapping_dims_for_attr ( attr, proc_dims);
      	   	if (read_data) {
   				read_attr(timestep_file_id, attr);
   			}
   			else if (testing_logging) {
   				testing_log << "attr data range: ";
   				print_attr_dims(attr);
   				// testing_log << endl;
   			}
   		}
   	}
        // testing_log << endl;
   		// cout << "rank about to close timestep file" << endl;
   	close_timestep_file(timestep_file_id);
   		// cout << "rank closed timestep file" << endl;

    add_timing_point(READ_DATA_FOR_ATTRS_DONE);
}

// void read_data_for_attrs(string timestep_file_name, vector<var_attribute_str> attrs,
// 			 md_dim_bounds proc_dims)
// {
//    	hid_t timestep_file_id;
//    	if (read_data && attrs.size() > 0) {
//    		// extreme_debug_log << "rank about to open timestep file" << endl;
//    		open_file_for_read(timestep_file_name.c_str(), timestep_file_id);
//    	}
//     for (var_attribute_str attr : attrs) {
//    		if (dims_overlap ( attr, proc_dims) ) {
//       		get_overlapping_dims_for_attr ( attr, proc_dims);
//       	   	if (read_data) {
//    				read_attr(timestep_file_id, attr);
//    			}
//    			else if (testing_logging) {
//    				testing_log << "attr data range: ";
//    				print_attr_dims(attr);
//    				// testing_log << endl;
//    			}
//    		}
//    	}
//     if (read_data && attrs.size() > 0) {
//         // testing_log << endl;
//    		// cout << "rank about to close timestep file" << endl;
//    		close_timestep_file(timestep_file_id);
//    		// cout << "rank closed timestep file" << endl;
//    	}

// }

void get_overlapping_dims_for_attr ( var_attribute_str &attr, 
                            const md_dim_bounds &proc_dims
                            ) 
{
	attr.d0_min = max( attr.d0_min, proc_dims.d0_min );
    attr.d0_max = min( attr.d0_max, proc_dims.d0_max );         
	attr.d1_min = max( attr.d1_min, proc_dims.d1_min );
    attr.d1_max = min( attr.d1_max, proc_dims.d1_max );  
	attr.d2_min = max( attr.d2_min, proc_dims.d2_min );
    attr.d2_max = min( attr.d2_max, proc_dims.d2_max );
}


void read_attr(hid_t timestep_file_id, var_attribute_str attr)
{
	add_timing_point(READ_DATA_FOR_ATTR_START);

	herr_t status;

	extreme_debug_log << "reading attr" << endl;

	uint64_t attr_vol = 0;
	hid_t var_data_space, var_id;
	// hid_t attr_data_space, property_list_id;
	hid_t attr_data_space;

	hsize_t stride[attr.num_dims];
	hsize_t block[attr.num_dims];

	// hsize_t stride[attr.num_dims];
	// hsize_t block[attr.num_dims];
	hsize_t offset[attr.num_dims];
	hsize_t count[attr.num_dims];

	get_hyperslab_dims_and_vol(attr, count, offset, stride, block, attr_vol);

	vector<double> data_vct(attr_vol);


    var_id = H5Dopen(timestep_file_id, attr.var_name.c_str(), H5P_DEFAULT); assert(var_id >= 0);

    //get the storage layout of the var
    var_data_space = H5Dget_space(var_id); assert(var_data_space >= 0);

	//select which hyperslab in storage to read
 	status = H5Sselect_hyperslab(var_data_space, H5S_SELECT_SET, offset, stride, count, block); assert(status >= 0);

 	//maintain the stored layout out the data reading it back in
	attr_data_space  = H5Screate_simple(attr.num_dims, count, NULL); assert(attr_data_space >= 0);

    // // Create property list for collective dataset read
    // property_list_id = H5Pcreate(H5P_DATASET_XFER); assert(property_list_id >= 0);
    // status = H5Pset_dxpl_mpio(property_list_id, H5FD_MPIO_COLLECTIVE); assert(status >= 0);

	status = H5Dread (var_id, H5T_NATIVE_DOUBLE, attr_data_space, var_data_space,
               H5P_DEFAULT, &data_vct[0]); assert(status >= 0);

    // Close/release resources
    status = H5Sclose(attr_data_space); assert(status >= 0);
 	// status = H5Pclose(property_list_id); assert(status >= 0);

 	if(testing_logging) {
   		find_data_range(var_data_space, attr, data_vct);
   	}

    status = H5Dclose(var_id); assert(status >= 0);
    status = H5Sclose(var_data_space); assert(status >= 0); 	

	add_timing_point(READ_DATA_FOR_ATTR_DONE);
}


void get_hyperslab_dims_and_vol(var_attribute_str attr, hsize_t *count, hsize_t *offset, 
	hsize_t *stride, hsize_t *block, uint64_t &attr_vol)
{
	if(attr.num_dims >= 1) {
		stride[0] = 1;
		block[0] = 1;
		offset[0] = attr.d0_min;
		count[0] = attr.d0_max - attr.d0_min + 1;
		attr_vol = count[0];
		if(attr.num_dims >= 2) {
			stride[1] = 1;
			block[1] = 1;
			offset[1] = attr.d1_min;
			count[1] = attr.d1_max - attr.d1_min + 1;
			attr_vol *= count[1];		
			if(attr.num_dims >= 2) {
				stride[2] = 1;
				block[2] = 1;
				offset[2] = attr.d2_min;
				count[2] = attr.d2_max - attr.d2_min + 1;	
				attr_vol *= count[2];		
			}	
		}
	}
}







