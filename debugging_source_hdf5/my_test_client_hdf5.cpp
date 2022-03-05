#include <stdio.h> //needed for printf
#include <stdlib.h> //needed for atoi/atol/malloc/free/rand
#include <string.h> //needed for ststatusmp

#include <stdint.h> //needed for uint
#include <vector>

#include <sstream>
#include <boost/serialization/vector.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

#include <helper_functions_test_client_hdf5.hh>
// #include <my_metadata_client_hdf5.hh>


using namespace std;

static bool testing_logging = true;
static bool extreme_debug_logging = false;
static bool zero_rank_logging = false;

debugLog testing_log = debugLog(testing_logging, zero_rank_logging);
debugLog extreme_debug_log = debugLog(extreme_debug_logging, false);

static bool output_timing = false;

void add_timing_point(int catg) {
}


int main(int argc, char **argv) {
    herr_t	status;

//     hsize_t     var_dims[num_dims] = {total_x_length, total_y_length, total_z_length}; /* dataset dimensions */
//     hsize_t     chunk_dims[num_dims] = {x_length_per_proc, y_length_per_proc, z_length_per_proc};           

//     /* chunk selection parameters */
//     hsize_t	offset[num_dims] = {x_offset, y_offset, z_offset};

    string run_name = "XGC";
    string job_id = "12345678";

    string type0_name = "typey1";
    string type1_name = "typey2";

    uint64_t timestep0_id = 0;
    uint64_t timestep1_id = 1;


    uint32_t var0_version = 0;
    uint32_t var1_version = 1;

    // string var0_name = "var" + to_string(var0_version) + '\0';
    // string var1_name = "var" + to_string(var1_version) + '\0';
    //assumption - pass around C++ strings but then use .str()
    string var0_name = "var" + to_string(var0_version);
    string var1_name = "var" + to_string(var1_version);

    hid_t run0_entry, run_attr_table0;

    md_timestep_entry timestep0;
    md_timestep_entry timestep1;
    md_timestep_entry timestep2;

    vector<var_attribute_str> attr_entries;

	metadata_create_run (run_name, job_id, run0_entry, run_attr_table0); 


    testing_log << "\nBeginning the testing setup \n";


    create_timestep(run_name, job_id, timestep0_id, run0_entry, type0_name, type1_name, var0_name, var1_name);

    create_timestep(run_name, job_id, timestep1_id, run0_entry, type0_name, type1_name, var0_name, var1_name);


//-------------------------------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------------------------------
//NEW ATTRIBUTE TESTING


    testing_log << "\nBeginning the new type and attribute testing \n" ;

	open_timestep_file_and_attr_tables_for_read(run_name, job_id, timestep0_id, timestep0);

	open_timestep_file_and_attr_tables_for_read(run_name, job_id, timestep1_id, timestep1);


    md_dim_bounds dims_3D;

    // dims_3D.d0_min = 15;
    // dims_3D.d0_max = 25;
    // dims_3D.d1_min = 15;
    // dims_3D.d1_max = 25;
    // dims_3D.d2_min = 15;
    // dims_3D.d2_max = 25;

    dims_3D.num_dims = 3;
    dims_3D.d0_min = 5;
    dims_3D.d0_max = 10;
    dims_3D.d1_min = 5;
    dims_3D.d1_max = 10;
    dims_3D.d2_min = 5;
    dims_3D.d2_max = 10;

    md_dim_bounds dims_2D;
    dims_2D.num_dims = 2;
    dims_2D.d0_min = 15;
    dims_2D.d0_max = 25;
    dims_2D.d1_min = 15;
    dims_2D.d1_max = 25;


    metadata_catalog_all_var_attributes_with_dims (timestep0.var_attr_table_id, dims_2D, attr_entries);

    testing_log << "2D to 3D testing: attributes associated with timestep_id: " << timestep0_id << " overlapping with dims ";
	print_md_dim_bounds(dims_2D);
    print_var_attribute_list (attr_entries);    



   catalog_all_var_attributes ( timestep0.var_attr_table_id, timestep0_id, dims_3D );


   catalog_all_var_attributes_with_type (timestep0.var_attr_table_id, timestep0_id, type0_name, dims_3D );

   catalog_all_var_attributes_with_type (timestep0.var_attr_table_id, timestep0_id, type1_name, dims_3D );


    catalog_all_var_attributes_with_var ( timestep0.var_attr_table_id, timestep0_id, var0_name, dims_3D );


    catalog_all_var_attributes_with_var ( timestep0.var_attr_table_id, timestep0_id, var1_name, dims_3D );

    catalog_all_var_attributes_with_type_var ( timestep0.var_attr_table_id, timestep0_id, type0_name, var0_name, dims_3D );

    catalog_all_var_attributes_with_type_var ( timestep0.var_attr_table_id, timestep0_id, type0_name, var1_name, dims_3D );

    catalog_all_var_attributes_with_type_var ( timestep0.var_attr_table_id, timestep0_id, type1_name, var0_name, dims_3D );

    catalog_all_var_attributes_with_type_var ( timestep0.var_attr_table_id, timestep0_id, type1_name, var1_name, dims_3D );


//-------------------------------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------------------------------
//TESTING VAR SUBSTR OPS

    testing_log << "\nBeginning var substring testing \n";

    string subtr = "va";
    uint32_t var_substr_version = 2;
	uint32_t num_dims_var_substr = 3;
    hsize_t dims_var_substr[num_dims_var_substr] = {100, 100, 100};
    string var_substr_name = subtr + to_string(var_substr_version);


    // vector<var_attribute_str> new_subtr_attrs;
    metadata_create_and_close_var(num_dims_var_substr, dims_var_substr, timestep0.file_id, var_substr_name);


    string substr_attr_data;
    string str("testNULnew4",11);
    make_attr_data(str, 13, substr_attr_data);

    var_attribute_str new_subtr_attr = var_attribute_str (type1_name, var_substr_name, dims_3D, substr_attr_data);
	metadata_insert_var_attribute (timestep0.var_attr_table_id, new_subtr_attr );

    catalog_all_var_attributes_with_var_substr ( timestep0.file_id, timestep0.var_attr_table_id, timestep0_id, dims_3D );

    catalog_all_var_attributes_with_type_var_substr ( timestep0.var_attr_table_id, timestep0_id, type1_name, dims_3D );

    string type_name_substr = "type_subtr";
    new_subtr_attr = var_attribute_str (type_name_substr, var_substr_name, dims_3D, substr_attr_data) ;
	metadata_insert_var_attribute (timestep0.var_attr_table_id, new_subtr_attr );



    uint64_t timestep2_id = 20;
    metadata_create_timestep (run_name, job_id, timestep2_id, run0_entry, timestep2);
    testing_log << "new timestep. timestep_id: " << timestep2_id << endl;

    var_attribute_str new_substr_attr = var_attribute_str (type1_name, var_substr_name, dims_3D, substr_attr_data);
    metadata_insert_var_attribute(timestep2.var_attr_table_id, new_substr_attr);

    metadata_create_and_close_var(num_dims_var_substr, dims_var_substr, timestep2.file_id, var_substr_name);

    catalog_all_var_attributes ( timestep2.var_attr_table_id, timestep2_id, dims_3D );


	catalog_all_types_substr ( timestep0.var_attr_table_id, timestep0_id, dims_3D ); 

    //reminder -have to keep track in some way what timesteps there are since each timestep is in its own file
    catalog_all_timesteps_substr ( run0_entry, type1_name, dims_3D );

	//reminder: can delete variables (by unlinking them, can't reclaim the space) but can't delete elements of a packet table (attrs)
    // delete_var_substr ( timestep0.file_id, timestep0.var_attr_table_id, timestep0_id );



//-------------------------------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------------------------------
//new testing - includes more robust run and timestep attr testing, finding timesteps that match criteria
//and finding certain types for a specific timestep


    testing_log << "Beginning new portion of testing \n \n";

    create_run_attrs( run_attr_table0, type0_name, type1_name ); 
	catalog_all_run_attributes ( run_attr_table0, type1_name );

    create_timestep_attrs(timestep0.timestep_attr_table_id, timestep1.timestep_attr_table_id, type0_name, type1_name);
 	catalog_all_timestep_attributes ( timestep0.timestep_attr_table_id, timestep0_id, type0_name);
 	catalog_all_timestep_attributes ( timestep0.timestep_attr_table_id, timestep0_id, type1_name);
 	catalog_all_timestep_attributes ( timestep1.timestep_attr_table_id, timestep1_id, type0_name);
 	catalog_all_timestep_attributes ( timestep1.timestep_attr_table_id, timestep1_id, type1_name);


    create_new_timesteps( run_name, job_id, timestep0_id, run0_entry, type0_name, type1_name, var1_name, var_substr_name, timestep2.var_attr_table_id);
	catalog_all_timesteps ( run0_entry, type1_name, var1_name, dims_3D );

	create_new_types(timestep0, timestep1, var0_name, var1_name);
    catalog_all_types (timestep0.var_attr_table_id, timestep0_id, var1_name, dims_3D);

    testing_log << "Done with  new portion of testing \n \n";


// //-------------------------------------------------------------------------------------------------------------------------------------------
// //-------------------------------------------------------------------------------------------------------------------------------------------
//     delete_run ( server, run0_id, timestep0_id, txn_id );


//     // metadata_delete_run_by_id (server, run1_id);



//-------------------------------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------------------------------
    
    testing_log << "done with testing\n";


	close_timestep(timestep2);
	close_timestep(timestep1);
	close_timestep(timestep0);

	H5PTclose(run_attr_table0);  assert(status >= 0);
   	H5Fclose(run0_entry); assert(status >= 0);


}


void make_attr_data (int test_int, string &serial_str) {
    stringstream ss;
    boost::archive::text_oarchive oa(ss);
    oa << test_int;
    serial_str = ss.str();
}


void make_attr_data (string test_string, int test_int, string &serial_str) {
    stringstream ss;
    boost::archive::text_oarchive oa(ss);
    // extreme_debug_log << "when making attr_data, string.size(): " << test_string.size() << endl;
    oa << test_string;
    oa << test_int;
    serial_str = ss.str();
    // extreme_debug_log << "serial_str: " << serial_str << endl;
}


void print_attr_data (var_attribute_str attr ) {
	string deserialized_test_string;
	int deserialized_test_int;

    stringstream ss;
    ss << attr.data;
    boost::archive::text_iarchive ia(ss);
    // extreme_debug_log << "when retrieving attr_data, string.size(): " << test_string.size() << endl;
    ia >> deserialized_test_string;
    ia >> deserialized_test_int;

    testing_log << "data: blob: str: " << deserialized_test_string << " int: " << deserialized_test_int << endl; 
}

void print_attr_data (non_var_attribute_str attr ) {
	string deserialized_test_string;
	int deserialized_test_int;

    stringstream ss;
    ss << attr.data;
    boost::archive::text_iarchive ia(ss);
    // extreme_debug_log << "when retrieving attr_data, string.size(): " << test_string.size() << endl;
    ia >> deserialized_test_string;
    ia >> deserialized_test_int;

    testing_log << "data: blob: str: " << deserialized_test_string << " int: " << deserialized_test_int << endl; 
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


// void print_attr_dims(var_attribute_str attr) {
// 	if(attr.num_dims >= 1) {
// 		testing_log << " d0_min: " << attr.d0_min;
// 		testing_log << " d0_max: " << attr.d0_max;
// 		if(attr.num_dims >= 2) {
// 			testing_log << " d1_min: " << attr.d1_min;
// 			testing_log << " d1_max: " << attr.d1_max;
// 			if(attr.num_dims >= 3) { 
// 				testing_log << " d2_min: " << attr.d2_min;
// 				testing_log << " d2_max: " << attr.d2_max;
// 			}
// 		}
// 	}
// 	testing_log << " ";
// }

// void print_md_dim_bounds( md_dim_bounds dims) {
// 	if(dims.num_dims >= 1) {
// 		testing_log << " d0_min: " << dims.d0_min;
// 		testing_log << " d0_max: " << dims.d0_max;
// 		if(dims.num_dims >= 2) {
// 			testing_log << " d1_min: " << dims.d1_min;
// 			testing_log << " d1_max: " << dims.d1_max;
// 			if(dims.num_dims >= 3) { 
// 				testing_log << " d2_min: " << dims.d2_min;
// 				testing_log << " d2_max: " << dims.d2_max;
// 			}
// 		}
// 	}	
// 	testing_log << endl;
// }
