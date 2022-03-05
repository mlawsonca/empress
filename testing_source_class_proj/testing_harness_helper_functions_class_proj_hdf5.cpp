#include <stdlib.h> //needed for atoi/atol/malloc/free/rand
#include <stdint.h> //needed for uint
#include <vector>
#include <mpi.h>
#include <hdf5_hl.h>
#include <hdf5.h>

#include <my_metadata_client_hdf5.hh>
#include <testing_harness_helper_functions_hdf5.hh>
// #include <client_timing_constants_read_hdf5.hh>
#include <client_timing_constants_read_class_proj.hh>


using namespace std;

extern bool read_data;

extern void add_timing_point(int catg);

// static bool extreme_debug_logging = false;
// static bool debug_logging = false;
static bool error_logging = true;
// static bool testing_logging = false;

// static bool zero_rank_logging = false;
static debugLog error_log = debugLog(error_logging, false);
// static debugLog debug_log = debugLog(debug_logging, zero_rank_logging);
// static debugLog extreme_debug_log = debugLog(extreme_debug_logging, zero_rank_logging);
// static debugLog testing_log = debugLog(testing_logging, zero_rank_logging);

// extern debugLog //testing_log;
// extern debugLog error_log;
// extern debugLog //extreme_debug_log;
// extern debugLog //debug_log;

// extern bool testing_logging;



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
            //     //extreme_debug_log << "rank " << i << " has a string of length " << each_proc_num_values[i] << endl;
            // }
        }
        *all_values = (int *) malloc(sum * sizeof(int));

        // //extreme_debug_log << "sum: " << sum << endl;
    }

    MPI_Gatherv(&values[0], num_values, MPI_INT,
           *all_values, each_proc_num_values, displacement_for_each_proc,
           MPI_INT, 0, MPI_COMM_WORLD);

}


void print_attr_data(var_attribute_str attr) {
	// //extreme_debug_log << "serialized attr data: " << attr.data << endl;
    if( attr.type_name.find("blob") != string::npos ) {
        bool deserialized_bool = stoi(attr.data);

        // stringstream sso;
        // sso << attr.data;
        // boost::archive::text_iarchive ia(sso);
        // ia >> deserialized_bool;

        //testing_log << "data: bool: " << deserialized_bool << endl; 
    }
    else if( attr.type_name.find("val") != string::npos ) {
        double deserialized_test_real = std::stod (attr.data);

        // stringstream sso;
        // sso << attr.data;
        // boost::archive::text_iarchive ia(sso);
        // // //extreme_debug_log << "about to deserialize str: " << sso.str() << endl;
        // ia >> deserialized_test_real;

        //testing_log << "data: double: " << deserialized_test_real << endl; 
    }           
    else if( attr.type_name.find("note") != string::npos ) {
            // string deserialized_test_string;
        //testing_log << "data: text: " << attr.data << endl; 
    } 
    else if( attr.type_name.find("range") != string::npos ) {
        vector<int> deserialized_vals(2);

        stringstream sso;
        sso << attr.data;
        boost::archive::text_iarchive ia(sso);
        ia >> deserialized_vals;
        //testing_log << "data: blob: val1: " << deserialized_vals.at(0) << " val2: " << deserialized_vals.at(1) << endl; 
    }
    else {
    	//testing_log << "error. attr type: " << attr.type_name << " didn't fit into one of the expected categories for a var attr" << endl;
    }    
 }

void print_attr_data(non_var_attribute_str attr) {
	// //extreme_debug_log << "serialized attr data: " << attr.data << endl;
    if( attr.type_name.find("max") != string::npos || attr.type_name.find("min") != string::npos ) {
        double deserialized_test_real = std::stod (attr.data);

        // stringstream sso;
        // sso << attr.data;
        // boost::archive::text_iarchive ia(sso);
        // ia >> deserialized_test_real;

        //testing_log << "data: double: " << deserialized_test_real << endl; 
    } 
    else {
    	error_log << "error. attr type: " << attr.type_name << " didn't fit into one of the expected categories for a non var attr" << endl;
    }         
}


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









