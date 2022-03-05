#include <my_metadata_client_local.hh>
#include <map>

#include <sstream>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

using namespace std;

static bool md_debug = true;
// static bool md_extreme_debug = false;


static void md_log(const std::string &s) {
  if(md_debug) std::cout << s<< std::endl;
}

static bool testing_logging = true;
static bool extreme_debug_logging = false;
static bool zero_rank_logging = true;
static bool error_logging  = true;

static debugLog testing_log = debugLog(testing_logging, zero_rank_logging);
static debugLog error_log = debugLog(error_logging, zero_rank_logging);
static debugLog extreme_debug_log = debugLog(extreme_debug_logging, false);

static bool output_timing = false;


void make_attr_data (string test_string, int test_int, string &serial_str);

// void print_var_attr_data(uint32_t count, std::vector<md_catalog_var_attribute_entry> attr_entries);
// void print_timestep_attr_data(uint32_t count, std::vector<md_catalog_timestep_attribute_entry> attr_entries);
// void print_run_attr_data(uint32_t count, std::vector<md_catalog_run_attribute_entry> attr_entries);
// void // make_single_val_data (string test_string, int test_int, string &serial_str);
// void // make_single_val_data (int test_int, string &serial_str);
// void make_range_data (static auto min_int, static auto max_int, string &serial_str);

// template <class T>
// void make_range_data (T min_int, T max_int, string &serial_str);

// static void make_range_data (auto min_int, auto max_int, string &serial_str);


extern std::map <string, vector<double>> data_outputs;
extern md_catalog_run_entry new_run;

string to_upper (string val);


template <class T1, class T2>
static void make_range_data (T1 min_int, T2 max_int, string &serial_str) {
    stringstream ss;
    boost::archive::text_oarchive oa(ss);
    oa << min_int;
    oa << max_int;
    serial_str = ss.str();
}

template <class T2>
static void make_single_val_data (T2 val, string &serial_str) {
    stringstream ss;
    boost::archive::text_oarchive oa(ss);
    oa << val;
    serial_str = ss.str();
}


int catalog_all_var_attributes_with_type_var_substr ( md_catalog_type_entry type, md_catalog_var_entry var,
                                                    uint64_t type_id, int num_dims, vector<md_dim_bounds> vect_dims ) {

    int rc;
    uint32_t count;
    vector<md_catalog_var_attribute_entry> attr_entries;
    vector<md_catalog_var_entry> var_entries;
    var_entries.push_back(var);

	// string full_var_name_substr = "var"; 
	// string partial_var_name_substr = "va"; 

	string full_var_name_substr = "VAR"; 
	string partial_var_name_substr = "VA"; 

    rc = metadata_catalog_all_var_attributes_with_type_var_substr (var.run_id, var.timestep_id, type_id, full_var_name_substr, var.txn_id, count, attr_entries);
    if (rc == RC_OK) {
        testing_log << "using var attrs with type var by name ver funct: attributes associated with run_id " << var.run_id << ", timestep_id " << 
        	var.timestep_id << " and txn_id: " << var.txn_id << " and type_id " << type_id <<
        	" and var_substr " << full_var_name_substr;
        testing_log << "\n";
        if(testing_logging || (zero_rank_logging )) {
            print_var_attribute_list (count, attr_entries);
        }

        //print_var_attr_data(count, attr_entries);
    }
    else {
        error_log << "Error metadata_catalog_all_var_attributes_with_type_var_substr. Proceeding" << endl;
    }

    rc = metadata_catalog_all_var_attributes_with_type_var_substr (var.run_id, var.timestep_id, type_id, partial_var_name_substr, var.txn_id, count, attr_entries);
    if (rc == RC_OK) {
        testing_log << "using var attrs with type var by name ver funct: attributes associated with run_id " << var.run_id << ", timestep_id " << 
        	var.timestep_id << " and txn_id: " << var.txn_id << " and type_id " << type_id <<
        	" and var_substr " << partial_var_name_substr;
        testing_log << "\n";
        if(testing_logging || (zero_rank_logging )) {
            print_var_attribute_list (count, attr_entries);
        }

        //print_var_attr_data(count, attr_entries);
    }
    else {
        error_log << "Error metadata_catalog_all_var_attributes_with_type_var_substr. Proceeding" << endl;
    }

    // string range_data;
    // uint64_t min_range = 5;
    // uint64_t max_range = 15;
    // make_range_data (min_range, max_range, range_data);

    // rc = metadata_catalog_all_var_attributes_with_type_var_substr_range (var.timestep_id, type_id, full_var_name_substr, var.txn_id, 
    //     ATTR_DATA_TYPE_BLOB, range_data, count, attr_entries);
    // if (rc == RC_OK) {
    //     testing_log << "using var attrs with type var by id funct: attributes associated with run_id " << var.run_id << ", timestep_id " << 
    //     var.timestep_id << " and txn_id: " << var.txn_id << " and type_id " << type_id <<
    //     " and var_substr " << full_var_name_substr << " int attr data and range [" << min_range << "," << max_range << "]\n";
    //     if(testing_logging || (zero_rank_logging )) {
    //         print_var_attribute_list (count, attr_entries);
    //     }

    //     //print_var_attr_data(count, attr_entries);
    // }
    // else {
    //     error_log << "Error metadata_catalog_all_var_attributes_with_type_var_substr_range. Proceeding" << endl;
    // }

    // rc = metadata_catalog_all_var_attributes_with_type_var_substr_range (var.timestep_id, type_id, partial_var_name_substr, var.txn_id, 
    //     ATTR_DATA_TYPE_BLOB, range_data, count, attr_entries);
    // if (rc == RC_OK) {
    //     testing_log << "using var attrs with type var by id funct: attributes associated with run_id " << var.run_id << ", timestep_id " << 
    //     var.timestep_id << " and txn_id: " << var.txn_id << " and type_id " << type_id <<
    //     " and var_substr " << partial_var_name_substr << " int attr data and range [" << min_range << "," << max_range << "]\n";
    //     if(testing_logging || (zero_rank_logging )) {
    //         print_var_attribute_list (count, attr_entries);
    //     }

    //     //print_var_attr_data(count, attr_entries);
    // }
    // else {
    //     error_log << "Error metadata_catalog_all_var_attributes_with_type_var_substr_range. Proceeding" << endl;
    // }

    // string range_data_new;
   	// uint64_t val_new = 9;
    // make_single_val_data (val_new, range_data_new);


    // rc = metadata_catalog_all_var_attributes_with_type_var_substr_above_max (var.timestep_id, type_id, full_var_name_substr, var.txn_id, 
    //     ATTR_DATA_TYPE_BLOB, range_data_new, count, attr_entries);

    // if (rc == RC_OK) {
    //     testing_log << "using metadata_catalog_all_var_attributes_with_type_var_substr_above_max funct: attributes associated with run_id " << var.run_id << ", timestep_id " << 
    //     var.timestep_id << " and txn_id: " << var.txn_id << " and type_id " << type_id <<
    //     " and var_substr " << full_var_name_substr << " int attr data with val geq " << val_new << "\n";
    //     if(testing_logging || (zero_rank_logging )) {
    //         print_var_attribute_list (count, attr_entries);
    //     }

    //     //print_var_attr_data(count, attr_entries);
    // }
    // else {
    //     error_log << "Error metadata_catalog_all_var_attributes_with_type_var_substr_above_max. Proceeding" << endl;
    // }

    // rc = metadata_catalog_all_var_attributes_with_type_var_substr_above_max (var.timestep_id, type_id, partial_var_name_substr, var.txn_id, 
    //     ATTR_DATA_TYPE_BLOB, range_data_new, count, attr_entries);

    // if (rc == RC_OK) {
    //     testing_log << "using metadata_catalog_all_var_attributes_with_type_var_substr_above_max funct: attributes associated with run_id " << var.run_id << ", timestep_id " << 
    //     var.timestep_id << " and txn_id: " << var.txn_id << " and type_id " << type_id <<
    //     " and var_substr " << partial_var_name_substr << " int attr data with val geq " << val_new << "\n";
    //     if(testing_logging || (zero_rank_logging )) {
    //         print_var_attribute_list (count, attr_entries);
    //     }

    //     //print_var_attr_data(count, attr_entries);
    // }
    // else {
    //     error_log << "Error metadata_catalog_all_var_attributes_with_type_var_substr_above_max. Proceeding" << endl;
    // }

 

    // string range_data2;
    // uint64_t val2 = 13;
    // make_single_val_data (val2, range_data2);

    // rc = metadata_catalog_all_var_attributes_with_type_var_substr_below_min (var.timestep_id, type_id, full_var_name_substr, var.txn_id, 
    //     ATTR_DATA_TYPE_BLOB, range_data2, count, attr_entries);

    // if (rc == RC_OK) {
    //     testing_log << "using metadata_catalog_all_var_attributes_with_type_var_substr_below_min funct: attributes associated with run_id " << var.run_id << ", timestep_id " << 
    //     var.timestep_id << " and txn_id: " << var.txn_id << " and type_id " << type_id <<
    //     " and var_substr " << full_var_name_substr << " int attr data with val leq " << val2 << "\n";
    //     if(testing_logging || (zero_rank_logging )) {
    //         print_var_attribute_list (count, attr_entries);
    //     }

    //     //print_var_attr_data(count, attr_entries);
    // }
    // else {
    //     error_log << "Error metadata_catalog_all_var_attributes_with_type_var_substr_below_min. Proceeding" << endl;
    // }

    // rc = metadata_catalog_all_var_attributes_with_type_var_substr_below_min (var.timestep_id, type_id, partial_var_name_substr, var.txn_id, 
    //     ATTR_DATA_TYPE_BLOB, range_data2, count, attr_entries);

    // if (rc == RC_OK) {
    //     testing_log << "using metadata_catalog_all_var_attributes_with_type_var_substr_below_min: attributes associated with run_id " << var.run_id << ", timestep_id " << 
    //     var.timestep_id << " and txn_id: " << var.txn_id << " and type_id " << type_id <<
    //     " and var_substr " << partial_var_name_substr << " int attr data with val leq " << val2 << "\n";
    //     if(testing_logging || (zero_rank_logging )) {
    //         print_var_attribute_list (count, attr_entries);
    //     }

    //     //print_var_attr_data(count, attr_entries);
    // }
    // else {
    //     error_log << "Error metadata_catalog_all_var_attributes_with_type_var_substr_below_min. Proceeding" << endl;
    // }

    rc = metadata_catalog_all_var_attributes_with_type_var_substr_dims (var.run_id, var.timestep_id, type_id, 
    	full_var_name_substr, var.txn_id, num_dims, vect_dims, count, attr_entries);
    if (rc == RC_OK) {
        testing_log << "using metadata_catalog_all_var_attributes_with_type_var_substr_dims funct: attributes associated with run_id " << var.run_id << ", timestep_id " << var.timestep_id << " and txn_id: " << var.txn_id << " and type_id " << type_id <<
        " and var_substr " << full_var_name_substr << " overlapping with dims";
        for(int j=0; j< num_dims; j++) {
            testing_log << " d" << j << "_min: " << vect_dims [j].min;
            testing_log << " d" << j << "_max: " << vect_dims [j].max;               
        }
        testing_log << "\n";
        if(testing_logging || (zero_rank_logging )) {
            print_var_attribute_list (count, attr_entries);
        }

        //print_var_attr_data(count, attr_entries);
    }
    else {
        error_log << "Error metadata_catalog_all_var_attributes_with_type_var_substr_dims. Proceeding" << endl;
    }
    testing_log << "\n";

    rc = metadata_catalog_all_var_attributes_with_type_var_substr_dims (var.run_id, var.timestep_id, type_id, 
    	partial_var_name_substr, var.txn_id, num_dims, vect_dims, count, attr_entries);
    if (rc == RC_OK) {
        testing_log << "using metadata_catalog_all_var_attributes_with_type_var_substr_dims funct: attributes associated with run_id " << var.run_id << ", timestep_id " << var.timestep_id << " and txn_id: " << var.txn_id << " and type_id " << type_id <<
        " and var_substr " << partial_var_name_substr << " overlapping with dims";
        for(int j=0; j< num_dims; j++) {
            testing_log << " d" << j << "_min: " << vect_dims [j].min;
            testing_log << " d" << j << "_max: " << vect_dims [j].max;               
        }
        testing_log << "\n";
        if(testing_logging || (zero_rank_logging )) {
            print_var_attribute_list (count, attr_entries);
        }

        //print_var_attr_data(count, attr_entries);
    }
    else {
        error_log << "Error metadata_catalog_all_var_attributes_with_type_var_substr_dims. Proceeding" << endl;
    }
    testing_log << "\n";


    // rc = metadata_catalog_all_var_attributes_with_type_var_substr_dims_range (var.timestep_id, type_id, full_var_name_substr, var.txn_id, 
    //     num_dims, vect_dims, ATTR_DATA_TYPE_BLOB, range_data, count, attr_entries);
    // if (rc == RC_OK) {
    //     testing_log << "using metadata_catalog_all_var_attributes_with_type_var_substr_dims_range funct: attributes associated with run_id " << var.run_id << ", timestep_id " << 
    //     var.timestep_id << " and txn_id: " << var.txn_id << " and type_id " << type_id <<
    //     " and var_substr " << full_var_name_substr << " int attr data and range [" << min_range << "," << max_range << "]" <<
    //     " overlapping with dims";
    //     for(int j=0; j< num_dims; j++) {
    //         testing_log << " d" << j << "_min: " << vect_dims [j].min;
    //         testing_log << " d" << j << "_max: " << vect_dims [j].max;               
    //     }
    //     testing_log << endl;
    //     if(testing_logging || (zero_rank_logging )) {
    //         print_var_attribute_list (count, attr_entries);
    //     }

    //     //print_var_attr_data(count, attr_entries);
    // }
    // else {
    //     error_log << "Error metadata_catalog_all_var_attributes_with_type_var_substr_dims_range. Proceeding" << endl;
    // }

    // rc = metadata_catalog_all_var_attributes_with_type_var_substr_dims_range (var.timestep_id, type_id, partial_var_name_substr, var.txn_id, 
    //     num_dims, vect_dims, ATTR_DATA_TYPE_BLOB, range_data, count, attr_entries);
    // if (rc == RC_OK) {
    //     testing_log << "using metadata_catalog_all_var_attributes_with_type_var_substr_dims_range funct: attributes associated with run_id " << var.run_id << ", timestep_id " << 
    //     var.timestep_id << " and txn_id: " << var.txn_id << " and type_id " << type_id <<
    //     " and var_substr " << partial_var_name_substr << " int attr data and range [" << min_range << "," << max_range << "]" <<
    //     " overlapping with dims";
    //     for(int j=0; j< num_dims; j++) {
    //         testing_log << " d" << j << "_min: " << vect_dims [j].min;
    //         testing_log << " d" << j << "_max: " << vect_dims [j].max;               
    //     }
    //     testing_log << endl;
    //     if(testing_logging || (zero_rank_logging )) {
    //         print_var_attribute_list (count, attr_entries);
    //     }

    //     //print_var_attr_data(count, attr_entries);
    // }
    // else {
    //     error_log << "Error metadata_catalog_all_var_attributes_with_type_var_substr_dims_range. Proceeding" << endl;
    // }

    // rc = metadata_catalog_all_var_attributes_with_type_var_substr_dims_above_max (var.timestep_id, type_id, full_var_name_substr, var.txn_id, 
    //     num_dims, vect_dims, ATTR_DATA_TYPE_BLOB, range_data_new, count, attr_entries);
    // if (rc == RC_OK) {
    //     testing_log << "using metadata_catalog_all_var_attributes_with_type_var_substr_dims_above_max funct: attributes associated with run_id " << var.run_id << ", timestep_id " << 
    //     var.timestep_id << " and txn_id: " << var.txn_id << " and type_id " << type_id <<
    //     " and var_substr " << full_var_name_substr << " int attr data and val geq " << val_new <<
    //     " overlapping with dims";
    //     for(int j=0; j< num_dims; j++) {
    //         testing_log << " d" << j << "_min: " << vect_dims [j].min;
    //         testing_log << " d" << j << "_max: " << vect_dims [j].max;               
    //     }
    //     testing_log << endl;
    //     if(testing_logging || (zero_rank_logging )) {
    //         print_var_attribute_list (count, attr_entries);
    //     }

    //     //print_var_attr_data(count, attr_entries);
    // }
    // else {
    //     error_log << "Error metadata_catalog_all_var_attributes_with_type_var_substr_dims_above_max. Proceeding" << endl;
    // }

    // rc = metadata_catalog_all_var_attributes_with_type_var_substr_dims_above_max (var.timestep_id, type_id, partial_var_name_substr, var.txn_id, 
    //     num_dims, vect_dims, ATTR_DATA_TYPE_BLOB, range_data_new, count, attr_entries);
    // if (rc == RC_OK) {
    //     testing_log << "using metadata_catalog_all_var_attributes_with_type_var_substr_dims_above_max funct: attributes associated with run_id " << var.run_id << ", timestep_id " << 
    //     var.timestep_id << " and txn_id: " << var.txn_id << " and type_id " << type_id <<
    //     " and var_substr " << partial_var_name_substr << " int attr data and val geq " << val_new <<
    //     " overlapping with dims";
    //     for(int j=0; j< num_dims; j++) {
    //         testing_log << " d" << j << "_min: " << vect_dims [j].min;
    //         testing_log << " d" << j << "_max: " << vect_dims [j].max;               
    //     }
    //     testing_log << endl;
    //     if(testing_logging || (zero_rank_logging )) {
    //         print_var_attribute_list (count, attr_entries);
    //     }

    //     //print_var_attr_data(count, attr_entries);
    // }
    // else {
    //     error_log << "Error metadata_catalog_all_var_attributes_with_type_var_substr_dims_above_max. Proceeding" << endl;
    // }

    

    // rc = metadata_catalog_all_var_attributes_with_type_var_substr_dims_below_min (var.timestep_id, type_id, full_var_name_substr, var.txn_id, 
    //     num_dims, vect_dims, ATTR_DATA_TYPE_BLOB, range_data2, count, attr_entries);
    // if (rc == RC_OK) {
    //     testing_log << "using metadata_catalog_all_var_attributes_with_type_var_substr_dims_below_min funct: attributes associated with run_id " << var.run_id << ", timestep_id " << 
    //     var.timestep_id << " and txn_id: " << var.txn_id << " and type_id " << type_id <<
    //     " and var_substr " << full_var_name_substr << " int attr data and val leq " << val2 <<
    //     " overlapping with dims";
    //     for(int j=0; j< num_dims; j++) {
    //         testing_log << " d" << j << "_min: " << vect_dims [j].min;
    //         testing_log << " d" << j << "_max: " << vect_dims [j].max;               
    //     }
    //     testing_log << endl;
    //     if(testing_logging || (zero_rank_logging )) {
    //         print_var_attribute_list (count, attr_entries);
    //     }

    //     //print_var_attr_data(count, attr_entries);
    // }
    // else {
    //     error_log << "Error metadata_catalog_all_var_attributes_with_type_var_substr_dims_below_min. Proceeding" << endl;
    // }

    // rc = metadata_catalog_all_var_attributes_with_type_var_substr_dims_below_min (var.timestep_id, type_id, partial_var_name_substr, var.txn_id, 
    //     num_dims, vect_dims, ATTR_DATA_TYPE_BLOB, range_data2, count, attr_entries);
    // if (rc == RC_OK) {
    //     testing_log << "using metadata_catalog_all_var_attributes_with_type_var_substr_dims_below_min funct: attributes associated with run_id " << var.run_id << ", timestep_id " << 
    //     var.timestep_id << " and txn_id: " << var.txn_id << " and type_id " << type_id <<
    //     " and var_substr " << partial_var_name_substr << " int attr data and val leq " << val2 <<
    //     " overlapping with dims";
    //     for(int j=0; j< num_dims; j++) {
    //         testing_log << " d" << j << "_min: " << vect_dims [j].min;
    //         testing_log << " d" << j << "_max: " << vect_dims [j].max;               
    //     }
    //     testing_log << endl;
    //     if(testing_logging || (zero_rank_logging )) {
    //         print_var_attribute_list (count, attr_entries);
    //     }

    //     //print_var_attr_data(count, attr_entries);
    // }
    // else {
    //     error_log << "Error metadata_catalog_all_var_attributes_with_type_var_substr_dims_below_min. Proceeding" << endl;
    // }


    return rc;
}

int catalog_all_var_attributes_with_var_substr ( md_catalog_var_entry var, int num_dims, vector<md_dim_bounds> vect_dims ) {

    int rc;
    uint32_t count;
    vector<md_catalog_var_attribute_entry> attr_entries;
    vector<md_catalog_var_entry> var_entries;
    var_entries.push_back(var);


	// string full_var_name_substr = "var"; 
	// string partial_var_name_substr = "va"; 
	string full_var_name_substr = "VAR"; 
	string partial_var_name_substr = "VA"; 

    rc = metadata_catalog_var (var.run_id, var.timestep_id, var.txn_id, count, var_entries);
    if (rc == RC_OK) {
        md_log( "now the number of vars is " + to_string(count) );

        testing_log << "new var catalog: \n";
        if(testing_logging || (zero_rank_logging )) {
            print_var_catalog (count, var_entries);
        }
    }
    else {
        error_log << "Error cataloging the new set of var entries. Proceeding" << endl;
    }


    rc = metadata_catalog_all_var_attributes (var.run_id, var.timestep_id, var.txn_id, count, attr_entries);
    if (rc == RC_OK) {
        testing_log << "var attributes associated with run_id " << var.run_id << " and timestep_id " << var.timestep_id << " and txn_id: " << var.txn_id;
        testing_log << "\n";
        if(testing_logging || (zero_rank_logging )) {
            print_var_attribute_list (count, attr_entries);
        }

        //print_var_attr_data(count, attr_entries);
    }
    else {
        error_log << "Error metadata_catalog_all_var_attributes. Proceeding" << endl;
    }
    testing_log << "\n";


    rc = metadata_catalog_all_var_attributes_with_var_substr (var.run_id, var.timestep_id, partial_var_name_substr, var.txn_id, count, attr_entries);

    if (rc == RC_OK) {
        testing_log << "using var attrs by var substr funct: attributes associated with run_id " << var.run_id << ", timestep_id " << var.timestep_id << 
        	" and txn_id: " << var.txn_id << " and var_name_substr " << partial_var_name_substr << "\n";
        if(testing_logging || (zero_rank_logging )) {
            print_var_attribute_list (count, attr_entries);
        }

        //print_var_attr_data(count, attr_entries);
    }
    else {
        error_log << "Error metadata_catalog_all_var_attributes. Proceeding" << endl;
    }

    rc = metadata_catalog_all_var_attributes_with_var_substr (var.run_id, var.timestep_id, full_var_name_substr, var.txn_id, count, attr_entries);

    if (rc == RC_OK) {
        testing_log << "using var attrs by var substr funct: attributes associated with run_id " << var.run_id << ", timestep_id " << var.timestep_id << 
        	" and txn_id: " << var.txn_id << " and var_name_substr " << full_var_name_substr << "\n";
        if(testing_logging || (zero_rank_logging )) {
            print_var_attribute_list (count, attr_entries);
        }

        //print_var_attr_data(count, attr_entries);
    }
    else {
        error_log << "Error metadata_catalog_all_var_attributes_with_var_substr. Proceeding" << endl;
    }

    rc = metadata_catalog_all_var_attributes_with_var_substr_dims (var.run_id, var.timestep_id, partial_var_name_substr, var.txn_id, num_dims, vect_dims, count, attr_entries);
    if (rc == RC_OK) {
        testing_log << "using var attrs by var dims name ver funct: attributes associated with run_id " << var.run_id << ", timestep_id " << var.timestep_id << 
        	" and txn_id: " << var.txn_id << " and var_name_substr " << partial_var_name_substr << " overlapping with dims";
        for(int j=0; j< num_dims; j++) {
            testing_log << " d" << j << "_min: " << vect_dims [j].min;
            testing_log << " d" << j << "_max: " << vect_dims [j].max;               
        }
        testing_log << "\n";
        if(testing_logging || (zero_rank_logging )) {
            print_var_attribute_list (count, attr_entries);
        }

        //print_var_attr_data(count, attr_entries);
    }
    else {
        error_log << "Error metadata_catalog_all_var_attributes_with_var_substr_dims. Proceeding" << endl;
    }


    rc = metadata_catalog_all_var_attributes_with_var_substr_dims (var.run_id, var.timestep_id, full_var_name_substr, var.txn_id, num_dims, vect_dims, count, attr_entries);
    if (rc == RC_OK) {
        testing_log << "using var attrs by var dims name ver funct: attributes associated with run_id " << var.run_id << ", timestep_id " << var.timestep_id << 
        	" and txn_id: " << var.txn_id << " and var_name_substr " << full_var_name_substr << " overlapping with dims";
        for(int j=0; j< num_dims; j++) {
            testing_log << " d" << j << "_min: " << vect_dims [j].min;
            testing_log << " d" << j << "_max: " << vect_dims [j].max;               
        }
        testing_log << "\n";
        if(testing_logging || (zero_rank_logging )) {
            print_var_attribute_list (count, attr_entries);
        }

        //print_var_attr_data(count, attr_entries);
    }
    else {
        error_log << "Error metadata_catalog_all_var_attributes_with_var_substr_dims. Proceeding" << endl;
    }

    return rc;
}

int catalog_all_types_substr ( md_catalog_var_entry var, 
                    int num_dims, vector<md_dim_bounds> vect_dims ) {

    int rc;
    uint32_t count;
    vector<md_catalog_type_entry> type_entries;

	// string full_var_name_substr = "var"; 
	// string partial_var_name_substr = "va"; 
	string full_var_name_substr = "VAR"; 
	string partial_var_name_substr = "VA"; 

    rc = metadata_catalog_all_types_with_var_attributes_in_timestep (var.run_id, var.timestep_id, var.txn_id, count, type_entries);

    if (rc == RC_OK) {
        testing_log << "using catalog all types in timestep funct: types associated with run_id " << var.run_id << ", timestep_id " << var.timestep_id << 
            " and txn_id: " << var.txn_id << "\n";
        if(testing_logging || (zero_rank_logging )) {
            print_type_catalog (count, type_entries);
        }
    }
    else {
        error_log << "Error getting the matching type catalog. Proceeding" << endl;
    }

    rc = metadata_catalog_all_types_with_var_attributes_with_var_substr_in_timestep (var.run_id, var.timestep_id, full_var_name_substr, var.txn_id, count, type_entries);

    if (rc == RC_OK) {
        testing_log << "using catalog all types with instances on var str in timestep funct: types associated with run_id " << var.run_id << 
        	", timestep_id " << var.timestep_id << " and var_name_substr " << full_var_name_substr << " and txn_id: " << var.txn_id << "\n";
        if(testing_logging || (zero_rank_logging )) {
            print_type_catalog (count, type_entries);
        }
    }
    else {
        error_log << "Error getting the matching type catalog. Proceeding" << endl;
    }

    rc = metadata_catalog_all_types_with_var_attributes_with_var_substr_in_timestep (var.run_id, var.timestep_id, partial_var_name_substr, var.txn_id, count, type_entries);

    if (rc == RC_OK) {
        testing_log << "using catalog all types with instances on var str in timestep funct: types associated with run_id " << var.run_id << 
        	", timestep_id " << var.timestep_id << " and var_name_substr " << partial_var_name_substr << " and txn_id: " << var.txn_id << "\n";
        if(testing_logging || (zero_rank_logging )) {
            print_type_catalog (count, type_entries);
        }
    }
    else {
        error_log << "Error getting the matching type catalog. Proceeding" << endl;
    }


    rc = metadata_catalog_all_types_with_var_attributes_with_var_substr_dims_in_timestep (var.run_id, var.timestep_id, full_var_name_substr, var.txn_id, num_dims, vect_dims, count, type_entries);
    if (rc == RC_OK) {
       testing_log << "using catalog all types with instances on var str dims in timestep funct: types associated with run_id " << 
       		var.run_id << ", timestep_id " << var.timestep_id << 
            " and var_name_substr " << full_var_name_substr << " and txn_id: " << var.txn_id << " overlapping with dims";
        for(int j=0; j< num_dims; j++) {
            testing_log << " d" << j << "_min: " << vect_dims [j].min;
            testing_log << " d" << j << "_max: " << vect_dims [j].max;               
        }
        testing_log << "\n";
        if(testing_logging || (zero_rank_logging )) {
            print_type_catalog (count, type_entries);
        }
    }
    else {
        error_log << "Error getting the matching type catalog. Proceeding" << endl;
    }

    rc = metadata_catalog_all_types_with_var_attributes_with_var_substr_dims_in_timestep (var.run_id, var.timestep_id, partial_var_name_substr, var.txn_id, num_dims, vect_dims, count, type_entries);
    if (rc == RC_OK) {
       testing_log << "using catalog all types with instances on var str dims in timestep funct: types associated with run_id " << 
       		var.run_id << ", timestep_id " << var.timestep_id << 
            " and var_name_substr " << partial_var_name_substr << " and txn_id: " << var.txn_id << " overlapping with dims";
        for(int j=0; j< num_dims; j++) {
            testing_log << " d" << j << "_min: " << vect_dims [j].min;
            testing_log << " d" << j << "_max: " << vect_dims [j].max;               
        }
        testing_log << "\n";
        if(testing_logging || (zero_rank_logging )) {
            print_type_catalog (count, type_entries);
        }
    }
    else {
        error_log << "Error getting the matching type catalog. Proceeding" << endl;
    }


    return rc;
}

int catalog_all_timesteps_substr ( md_catalog_var_entry var, uint64_t type_id,
                    int num_dims, vector<md_dim_bounds> vect_dims ) {

    int rc;
    uint32_t count;
    vector<md_catalog_timestep_entry> timestep_entries;


	// string full_var_name_substr = "var"; 
	// string partial_var_name_substr = "va"; 
	string full_var_name_substr = "VAR"; 
	string partial_var_name_substr = "VA"; 

    rc = metadata_catalog_timestep (var.run_id, var.txn_id, count, timestep_entries);
    if (rc != RC_OK) {
        error_log << "Error set of timestep entries. Proceeding" << endl;
    }
    testing_log << "timestep catalog for run_id " << var.run_id << " and txn_id " << var.txn_id << ": \n";
    if(testing_logging || (zero_rank_logging )) {
        print_timestep_catalog (count, timestep_entries);
    }

    rc = metadata_catalog_all_timesteps_with_var_substr (var.run_id, full_var_name_substr, var.txn_id, count, timestep_entries);

    if (rc == RC_OK) {
        testing_log << "using catalog_all_timesteps_with_var_substr funct: timesteps associated with run_id " << var.run_id << 
        ", var_name_substr " << full_var_name_substr << " and txn_id: " << var.txn_id << "\n";
        if(testing_logging || (zero_rank_logging )) {
            print_timestep_catalog (count, timestep_entries);
        }
    }
    else {
        error_log << "Error getting the matching timestep catalog. Proceeding" << endl;
    }

    rc = metadata_catalog_all_timesteps_with_var_substr (var.run_id, partial_var_name_substr, var.txn_id, count, timestep_entries);

    if (rc == RC_OK) {
        testing_log << "using catalog_all_timesteps_with_var_substr funct: timesteps associated with run_id " << var.run_id << 
        ", var_name_substr " << partial_var_name_substr << " and txn_id: " << var.txn_id << "\n";
        if(testing_logging || (zero_rank_logging )) {
            print_timestep_catalog (count, timestep_entries);
        }
    }
    else {
        error_log << "Error getting the matching timestep catalog. Proceeding" << endl;
    }

    rc = metadata_catalog_all_timesteps_with_var_attributes_with_type_var_substr (var.run_id, type_id, full_var_name_substr, var.txn_id, count, timestep_entries);

    if (rc == RC_OK) {
        testing_log << "using metadata_catalog_all_timesteps_with_var_attributes_with_type_var_substr funct: timesteps associated with run_id " << 
        var.run_id << ", type_id " << type_id << 
        ", var_name_substr " << full_var_name_substr << " and txn_id: " << var.txn_id << "\n";
        if(testing_logging || (zero_rank_logging )) {
            print_timestep_catalog (count, timestep_entries);
        }
    }
    else {
        error_log << "Error getting the matching timestep catalog. Proceeding" << endl;
    }


    rc = metadata_catalog_all_timesteps_with_var_attributes_with_type_var_substr (var.run_id, type_id, partial_var_name_substr, var.txn_id, count, timestep_entries);

    if (rc == RC_OK) {
        testing_log << "using metadata_catalog_all_timesteps_with_var_attributes_with_type_var_substr funct: timesteps associated with run_id " << 
        var.run_id << ", type_id " << type_id << 
        ", var_name_substr " << partial_var_name_substr << " and txn_id: " << var.txn_id << "\n";
        if(testing_logging || (zero_rank_logging )) {
            print_timestep_catalog (count, timestep_entries);
        }
    }
    else {
        error_log << "Error getting the matching timestep catalog. Proceeding" << endl;
    }

    // string range_data0;
    // uint64_t min_range = 5;
    // uint64_t max_range = 15;
    // make_range_data (min_range, max_range, range_data0);


    // rc = metadata_catalog_all_timesteps_with_var_attributes_with_type_var_substr_range (var.run_id, type_id, full_var_name_substr, var.txn_id, 
    //     ATTR_DATA_TYPE_BLOB, range_data0, count, timestep_entries);
    // if (rc == RC_OK) {
    //     testing_log << "using metadata_catalog_all_timesteps_with_var_attributes_with_type_var_substr_range funct: timesteps associated with run_id " << var.run_id << ", type_id " << type_id << 
    //     ", var_name_substr " << full_var_name_substr << " txn_id: " << var.txn_id << " int attr data and range [" << min_range << "," << max_range << "]\n";
    //     if(testing_logging || (zero_rank_logging )) {
    //         print_timestep_catalog (count, timestep_entries);
    //     }
    // }
    // else {
    //     error_log << "Error getting the matching timestep catalog. Proceeding" << endl;
    // }

    // rc = metadata_catalog_all_timesteps_with_var_attributes_with_type_var_substr_range (var.run_id, type_id, partial_var_name_substr, var.txn_id, 
    //     ATTR_DATA_TYPE_BLOB, range_data0, count, timestep_entries);
    // if (rc == RC_OK) {
    //     testing_log << "using metadata_catalog_all_timesteps_with_var_attributes_with_type_var_substr_range funct: timesteps associated with run_id " << var.run_id << ", type_id " << type_id << 
    //     ", var_name_substr " << partial_var_name_substr << " txn_id: " << var.txn_id << " int attr data and range [" << min_range << "," << max_range << "]\n";
    //     if(testing_logging || (zero_rank_logging )) {
    //         print_timestep_catalog (count, timestep_entries);
    //     }
    // }
    // else {
    //     error_log << "Error getting the matching timestep catalog. Proceeding" << endl;
    // }


    // uint64_t val = 12;
    // string range_data1;
    // make_single_val_data (val, range_data1);

    // rc = metadata_catalog_all_timesteps_with_var_attributes_with_type_var_substr_above_max (var.run_id, type_id, full_var_name_substr, var.txn_id, 
    //     ATTR_DATA_TYPE_BLOB, range_data1, count, timestep_entries);

    // if (rc == RC_OK) {
    //     testing_log << "using metadata_catalog_all_timesteps_with_var_attributes_with_type_var_substr_above_max funct: timesteps associated with run_id " << var.run_id << ", type_id " << type_id << 
    //     ", var_name_substr " << full_var_name_substr << " txn_id: " << var.txn_id << " int attr data greater than or eq to " << val << "]\n";
    //     if(testing_logging || (zero_rank_logging )) {
    //         print_timestep_catalog (count, timestep_entries);
    //     }
    // }
    // else {
    //     error_log << "Error getting the matching timestep catalog. Proceeding" << endl;
    // }

    // rc = metadata_catalog_all_timesteps_with_var_attributes_with_type_var_substr_above_max (var.run_id, type_id, partial_var_name_substr, var.txn_id, 
    //     ATTR_DATA_TYPE_BLOB, range_data1, count, timestep_entries);

    // if (rc == RC_OK) {
    //     testing_log << "using metadata_catalog_all_timesteps_with_var_attributes_with_type_var_substr_above_max funct: timesteps associated with run_id " << var.run_id << ", type_id " << type_id << 
    //     ", var_name_substr " << partial_var_name_substr << " txn_id: " << var.txn_id << " int attr data greater than or eq to " << val << "]\n";
    //     if(testing_logging || (zero_rank_logging )) {
    //         print_timestep_catalog (count, timestep_entries);
    //     }
    // }
    // else {
    //     error_log << "Error getting the matching timestep catalog. Proceeding" << endl;
    // }


    // uint64_t val2 = 13;
    // string range_data2;
    // make_single_val_data (val2, range_data2);

    // rc = metadata_catalog_all_timesteps_with_var_attributes_with_type_var_substr_below_min (var.run_id, type_id, full_var_name_substr, var.txn_id, 
    //     ATTR_DATA_TYPE_BLOB, range_data2, count, timestep_entries);

    // if (rc == RC_OK) {
    //     testing_log << "using metadata_catalog_all_timesteps_with_var_attributes_with_type_var_substr_below_min funct: timesteps associated with run_id " << var.run_id << ", type_id " << type_id << 
    //     ", var_name_substr " << full_var_name_substr << " txn_id: " << var.txn_id << " int attr data less than or eq to " << val2 << "]\n";
    //     if(testing_logging || (zero_rank_logging )) {
    //         print_timestep_catalog (count, timestep_entries);
    //     }
    // }
    // else {
    //     error_log << "Error getting the matching timestep catalog. Proceeding" << endl;
    // }

    // rc = metadata_catalog_all_timesteps_with_var_attributes_with_type_var_substr_below_min (var.run_id, type_id, partial_var_name_substr, var.txn_id, 
    //     ATTR_DATA_TYPE_BLOB, range_data2, count, timestep_entries);

    // if (rc == RC_OK) {
    //     testing_log << "using metadata_catalog_all_timesteps_with_var_attributes_with_type_var_substr_below_min funct: timesteps associated with run_id " << var.run_id << ", type_id " << type_id << 
    //     ", var_name_substr " << partial_var_name_substr << " txn_id: " << var.txn_id << " int attr data less than or eq to " << val2 << "]\n";
    //     if(testing_logging || (zero_rank_logging )) {
    //         print_timestep_catalog (count, timestep_entries);
    //     }
    // }
    // else {
    //     error_log << "Error getting the matching timestep catalog. Proceeding" << endl;
    // }

    rc = metadata_catalog_all_timesteps_with_var_attributes_with_type_var_substr_dims (var.run_id, type_id, full_var_name_substr, var.txn_id, num_dims, vect_dims, count, timestep_entries);
    if (rc == RC_OK) {
        testing_log << "using metadata_catalog_all_timesteps_with_var_attributes_with_type_var_substr_dims funct: timesteps associated with run_id " << var.run_id << ", type_id " << type_id << 
        ", var_name_substr " << full_var_name_substr << " and txn_id: " << var.txn_id << " overlapping with dims";
        for(int j=0; j< num_dims; j++) {
            testing_log << " d" << j << "_min: " << vect_dims [j].min;
            testing_log << " d" << j << "_max: " << vect_dims [j].max;               
        }
        testing_log << "\n";
        if(testing_logging || (zero_rank_logging )) {
            print_timestep_catalog (count, timestep_entries);
        }
    }
    else {
        error_log << "Error getting the matching timestep catalog. Proceeding" << endl;
    }


    rc = metadata_catalog_all_timesteps_with_var_attributes_with_type_var_substr_dims (var.run_id, type_id, partial_var_name_substr, var.txn_id, num_dims, vect_dims, count, timestep_entries);
    if (rc == RC_OK) {
        testing_log << "using metadata_catalog_all_timesteps_with_var_attributes_with_type_var_substr_dims funct: timesteps associated with run_id " << var.run_id << ", type_id " << type_id << 
        ", var_name_substr " << partial_var_name_substr << " and txn_id: " << var.txn_id << " overlapping with dims";
        for(int j=0; j< num_dims; j++) {
            testing_log << " d" << j << "_min: " << vect_dims [j].min;
            testing_log << " d" << j << "_max: " << vect_dims [j].max;               
        }
        testing_log << "\n";
        if(testing_logging || (zero_rank_logging )) {
            print_timestep_catalog (count, timestep_entries);
        }
    }
    else {
        error_log << "Error getting the matching timestep catalog. Proceeding" << endl;
    }

    // rc = metadata_catalog_all_timesteps_with_var_attributes_with_type_var_substr_dims_range (var.run_id, type_id, full_var_name_substr, var.txn_id, num_dims, vect_dims, 
    //     ATTR_DATA_TYPE_BLOB, range_data0, count, timestep_entries);

    // if (rc == RC_OK) {
    //     testing_log << "using metadata_catalog_all_timesteps_with_var_attributes_with_type_var_substr_dims funct: timesteps associated with run_id " << var.run_id << ", type_id " << type_id << 
    //     ", var_name_substr " << full_var_name_substr << " and txn_id: " << var.txn_id << " int attr data and range [" << min_range << "," << max_range << "] overlapping with dims";
    //     for(int j=0; j< num_dims; j++) {
    //         testing_log << " d" << j << "_min: " << vect_dims [j].min;
    //         testing_log << " d" << j << "_max: " << vect_dims [j].max;               
    //     }
    //     testing_log << "\n";
    //     if(testing_logging || (zero_rank_logging )) {
    //         print_timestep_catalog (count, timestep_entries);
    //     }
    // }
    // else {
    //     error_log << "Error getting the matching timestep catalog. Proceeding" << endl;
    // }

    // rc = metadata_catalog_all_timesteps_with_var_attributes_with_type_var_substr_dims_range (var.run_id, type_id, partial_var_name_substr, var.txn_id, num_dims, vect_dims, 
    //     ATTR_DATA_TYPE_BLOB, range_data0, count, timestep_entries);

    // if (rc == RC_OK) {
    //     testing_log << "using metadata_catalog_all_timesteps_with_var_attributes_with_type_var_substr_dims funct: timesteps associated with run_id " << var.run_id << ", type_id " << type_id << 
    //     ", var_name_substr " << partial_var_name_substr << " and txn_id: " << var.txn_id << " int attr data and range [" << min_range << "," << max_range << "] overlapping with dims";
    //     for(int j=0; j< num_dims; j++) {
    //         testing_log << " d" << j << "_min: " << vect_dims [j].min;
    //         testing_log << " d" << j << "_max: " << vect_dims [j].max;               
    //     }
    //     testing_log << "\n";
    //     if(testing_logging || (zero_rank_logging )) {
    //         print_timestep_catalog (count, timestep_entries);
    //     }
    // }
    // else {
    //     error_log << "Error getting the matching timestep catalog. Proceeding" << endl;
    // }

    // rc = metadata_catalog_all_timesteps_with_var_attributes_with_type_var_substr_dims_above_max (var.run_id, type_id, full_var_name_substr, var.txn_id, num_dims, vect_dims, 
    //     ATTR_DATA_TYPE_BLOB, range_data1, count, timestep_entries);

    // if (rc == RC_OK) {
    //     testing_log << "using metadata_catalog_all_timesteps_with_var_attributes_with_type_var_substr_dims funct: timesteps associated with run_id " << var.run_id << ", type_id " << type_id << 
    //     ", var_name_substr " << full_var_name_substr << " and txn_id: " << var.txn_id << " int attr data greater than or eq to " << val << " overlapping with dims";
    //     for(int j=0; j< num_dims; j++) {
    //         testing_log << " d" << j << "_min: " << vect_dims [j].min;
    //         testing_log << " d" << j << "_max: " << vect_dims [j].max;               
    //     }
    //     testing_log << "\n";
    //     if(testing_logging || (zero_rank_logging )) {
    //         print_timestep_catalog (count, timestep_entries);
    //     }
    // }
    // else {
    //     error_log << "Error getting the matching timestep catalog. Proceeding" << endl;
    // }

    // rc = metadata_catalog_all_timesteps_with_var_attributes_with_type_var_substr_dims_above_max (var.run_id, type_id, partial_var_name_substr, var.txn_id, num_dims, vect_dims, 
    //     ATTR_DATA_TYPE_BLOB, range_data1, count, timestep_entries);

    // if (rc == RC_OK) {
    //     testing_log << "using metadata_catalog_all_timesteps_with_var_attributes_with_type_var_substr_dims funct: timesteps associated with run_id " << var.run_id << ", type_id " << type_id << 
    //     ", var_name_substr " << partial_var_name_substr << " and txn_id: " << var.txn_id << " int attr data greater than or eq to " << val << " overlapping with dims";
    //     for(int j=0; j< num_dims; j++) {
    //         testing_log << " d" << j << "_min: " << vect_dims [j].min;
    //         testing_log << " d" << j << "_max: " << vect_dims [j].max;               
    //     }
    //     testing_log << "\n";
    //     if(testing_logging || (zero_rank_logging )) {
    //         print_timestep_catalog (count, timestep_entries);
    //     }
    // }
    // else {
    //     error_log << "Error getting the matching timestep catalog. Proceeding" << endl;
    // }

    // rc = metadata_catalog_all_timesteps_with_var_attributes_with_type_var_substr_dims_below_min (var.run_id, type_id, full_var_name_substr, var.txn_id, num_dims, vect_dims, 
    //     ATTR_DATA_TYPE_BLOB, range_data2, count, timestep_entries);

    // if (rc == RC_OK) {
    //     testing_log << "using metadata_catalog_all_timesteps_with_var_attributes_with_type_var_substr_dims funct: timesteps associated with run_id " << var.run_id << ", type_id " << type_id << 
    //     ", var_name_substr " << full_var_name_substr << " and txn_id: " << var.txn_id << " int attr data less than or eq to " << val2 << " overlapping with dims";
    //     for(int j=0; j< num_dims; j++) {
    //         testing_log << " d" << j << "_min: " << vect_dims [j].min;
    //         testing_log << " d" << j << "_max: " << vect_dims [j].max;               
    //     }
    //     testing_log << "\n";
    //     if(testing_logging || (zero_rank_logging )) {
    //         print_timestep_catalog (count, timestep_entries);
    //     }
    // }
    // else {
    //     error_log << "Error getting the matching timestep catalog. Proceeding" << endl;
    // }

    // rc = metadata_catalog_all_timesteps_with_var_attributes_with_type_var_substr_dims_below_min (var.run_id, type_id, partial_var_name_substr, var.txn_id, num_dims, vect_dims, 
    //     ATTR_DATA_TYPE_BLOB, range_data2, count, timestep_entries);

    // if (rc == RC_OK) {
    //     testing_log << "using metadata_catalog_all_timesteps_with_var_attributes_with_type_var_substr_dims funct: timesteps associated with run_id " << var.run_id << ", type_id " << type_id << 
    //     ", var_name_substr " << partial_var_name_substr << " and txn_id: " << var.txn_id << " int attr data less than or eq to " << val2 << " overlapping with dims";
    //     for(int j=0; j< num_dims; j++) {
    //         testing_log << " d" << j << "_min: " << vect_dims [j].min;
    //         testing_log << " d" << j << "_max: " << vect_dims [j].max;               
    //     }
    //     testing_log << "\n";
    //     if(testing_logging || (zero_rank_logging )) {
    //         print_timestep_catalog (count, timestep_entries);
    //     }
    // }
    // else {
    //     error_log << "Error getting the matching timestep catalog. Proceeding" << endl;
    // }

    return rc;
}

int delete_var_substr (uint64_t run_id, uint64_t timestep_id, uint64_t txn_id) {

	int rc;

	uint32_t count;
	vector<md_catalog_var_entry> var_entries;
	vector<md_catalog_var_attribute_entry> attr_entries;

    rc = metadata_catalog_var (run_id, timestep_id, txn_id, count, var_entries);
    if (rc != RC_OK) {
        error_log << "Error cataloging the new set of var entries. Proceeding" << endl;
        return RC_ERR;
    }
    if (rc == RC_OK) {
        testing_log << "prior to deleting, catalog: \n";
        if(testing_logging || (zero_rank_logging )) {
            print_var_catalog (count, var_entries);
        }
    }
    else {
        error_log << "Error cataloging the new set of var entries. Proceeding" << endl;
    }

    rc = metadata_catalog_all_var_attributes (run_id, timestep_id, txn_id, count, attr_entries);
    if (rc == RC_OK) {
        testing_log << "using var attrs funct: attributes associated with run_id " << run_id << " and timestep_id " << timestep_id << " and txn_id: " << txn_id;
        testing_log << "\n";
        if(testing_logging || (zero_rank_logging )) {
            print_var_attribute_list (count, attr_entries);
        }

        //print_var_attr_data(count, attr_entries);
    }
    else {
        error_log << "Error metadata_catalog_all_var_attributes. Proceeding" << endl;
    }

    // string var_name_substr = "var";
    string var_name_substr = "VAR";

    rc = metadata_delete_all_vars_with_substr ( run_id, timestep_id, var_name_substr);

    testing_log << "just deleted all vars for run " << run_id << " and timestep " << timestep_id << " matching substring " << var_name_substr << " \n";

    rc = metadata_catalog_var (run_id, timestep_id, txn_id, count, var_entries);
    if (rc != RC_OK) {
        error_log << "Error cataloging the new set of var entries. Proceeding" << endl;
        return RC_ERR;
    }
    if (rc == RC_OK) {
        testing_log << "new var catalog: \n";
        if(testing_logging || (zero_rank_logging )) {
            print_var_catalog (count, var_entries);
        }
    }
    else {
        error_log << "Error cataloging the new set of var entries. Proceeding" << endl;
    }

    rc = metadata_catalog_all_var_attributes (run_id, timestep_id, txn_id, count, attr_entries);
    if (rc == RC_OK) {
        testing_log << "using var attrs funct: attributes associated with run_id " << run_id << " and timestep_id " << timestep_id << " and txn_id: " << txn_id;
        testing_log << "\n";
        if(testing_logging || (zero_rank_logging )) {
            print_var_attribute_list (count, attr_entries);
        }

        //print_var_attr_data(count, attr_entries);
    }
    else {
        error_log << "Error metadata_catalog_all_var_attributes. Proceeding" << endl;
    }


   // var_name_substr = "va";
    var_name_substr = "VA";

    rc = metadata_delete_all_vars_with_substr ( run_id, timestep_id, var_name_substr);

    testing_log << "just deleted all vars for run " << run_id << " and timestep " << timestep_id << " matching substring " << var_name_substr << " \n";


    rc = metadata_catalog_var (run_id, timestep_id, txn_id, count, var_entries);
    if (rc != RC_OK) {
        error_log << "Error cataloging the new set of var entries. Proceeding" << endl;
        return RC_ERR;
    }
    if (rc == RC_OK) {
        testing_log << "new var catalog: \n";
        if(testing_logging || (zero_rank_logging )) {
            print_var_catalog (count, var_entries);
        }
    }
    else {
        error_log << "Error cataloging the new set of var entries. Proceeding" << endl;
    }

    rc = metadata_catalog_all_var_attributes (run_id, timestep_id, txn_id, count, attr_entries);
    if (rc == RC_OK) {
        testing_log << "using var attrs funct: attributes associated with run_id " << run_id << " and timestep_id " << timestep_id << " and txn_id: " << txn_id;
        testing_log << "\n";
        if(testing_logging || (zero_rank_logging )) {
            print_var_attribute_list (count, attr_entries);
        }

        //print_var_attr_data(count, attr_entries);
    }
    else {
        error_log << "Error metadata_catalog_all_var_attributes. Proceeding" << endl;
    }

    return rc;
}
