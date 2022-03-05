#include <my_metadata_client_local.hh>

#include <sstream>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <map>

using namespace std;

extern md_server_type my_server_type ;

static bool md_debug = true;
// static bool md_extreme_debug = false;


static void md_log(const std::string &s) {
  if(md_debug) std::cout << s<< std::endl;
}

static bool testing_logging = true;
static bool extreme_debug_logging = true;
static bool debug_logging = true;
static bool zero_rank_logging = false;
static bool error_logging  = true;

static debugLog testing_log = debugLog(testing_logging, zero_rank_logging);
static debugLog error_log = debugLog(error_logging, zero_rank_logging);
static debugLog extreme_debug_log = debugLog(extreme_debug_logging, false);
static debugLog debug_log = debugLog(debug_logging, false);

static bool output_timing = false;

extern std::map <string, vector<double>> data_outputs;

// int create_timestep(uint64_t run_id, uint64_t timestep_id, uint64_t txn_id,
//                     md_catalog_type_entry &new_type, md_catalog_var_entry &new_var);

// int catalog_all_var_attributes_with_type_var ( md_catalog_type_entry type, md_catalog_var_entry var,
//                                                     uint64_t type_id, uint64_t var_id );

// int catalog_all_var_attributes ( uint64_t run_id, uint64_t timestep_id, uint64_t txn_id, int num_dims, 
//                                                 vector<md_dim_bounds> vect_dims );

// int catalog_all_var_attributes_with_type ( uint64_t timestep_id, int num_dims, 
//                                                 vector<md_dim_bounds> vect_dims, md_catalog_type_entry type, uint64_t type_id );

static int catalog_all_var_attributes_with_type_id ( uint64_t timestep_id, uint64_t txn_id, uint64_t type_id );

// int catalog_all_var_attributes_with_var ( md_catalog_var_entry var, uint64_t var_id );


// int catalog_all_var_attributes_with_var_id ( uint64_t run_id, uint64_t timestep_id, uint64_t txn_id, uint64_t var_id );

// int test_activate_run ( md_catalog_run_entry prev_run, uint64_t type_id);

static int test_activate_run_attrs ( md_catalog_run_entry prev_run, uint64_t run_id, uint64_t type_id );

// int test_activate_and_delete_timestep ( md_catalog_timestep_entry prev_timestep, 
//                         md_catalog_type_entry type, md_catalog_var_entry var, uint64_t type_id);

static int test_activate_timestep_attrs ( md_catalog_timestep_entry timestep, uint64_t type_id );

//fix - should I be putting this in along with activate run? 
// int test_activate_and_delete_type ( md_catalog_type_entry prev_type, md_catalog_var_entry prev_attr);

// int test_activate_and_delete_var_and_var_attrs ( uint64_t type_id, 
//                                             md_catalog_type_entry prev_type, md_catalog_var_entry prev_var);

static int test_activate_var_attrs ( uint64_t type_id, uint64_t var_id, 
                                md_catalog_type_entry type, md_catalog_var_entry var, int num_dims, vector<md_dim_bounds> vect_dims);

// int delete_run (uint64_t run_id, uint64_t timestep_id, uint64_t txn_id );

void make_attr_data (string test_string, int test_int, string &serial_str);

// void //print_var_attr_data(uint32_t count, std::vector<md_catalog_var_attribute_entry> attr_entries);

//  void //print_timestep_attr_data(uint32_t count, std::vector<md_catalog_timestep_attribute_entry> attr_entries);

//   void //print_run_attr_data(uint32_t count, std::vector<md_catalog_run_attribute_entry> attr_entries);
// void make_single_val_data (string test_string, int test_int, string &serial_str);
// void make_range_data (auto min_int, auto max_int, string &serial_str);
// void make_range_data (static auto min_int, static auto max_int, string &serial_str);
// template <class T>
// void make_range_data (T min_int, T max_int, string &serial_str);


// static void make_range_data (auto min_int, auto max_int, string &serial_str);

int write_output(std::map <string, vector<double>> &data_outputs, md_catalog_run_entry run, 
                md_catalog_var_entry var);

int retrieveObjNamesAndDataForAttrCatalog(std::map <string, vector<double>> data_outputs, 
                                         md_catalog_run_entry run, uint64_t run_id, uint64_t timestep_id,
                                         uint64_t txn_id,
                                         vector<md_catalog_var_attribute_entry> attr_entries );

extern md_catalog_run_entry new_run;


// static void make_range_data (auto min_int, auto max_int, string &serial_str) {
//     stringstream ss;
//     boost::archive::text_oarchive oa(ss);
//     oa << min_int;
//     oa << max_int;
//     serial_str = ss.str();
// }

// static void make_single_val_data (auto val, string &serial_str) {
//     stringstream ss;
//     boost::archive::text_oarchive oa(ss);
//     oa << val;
//     serial_str = ss.str();
// }


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

string to_upper (string val) {
    string new_str = val;
    for (int i =0; i<val.size(); i++) {
        new_str[i] = toupper(new_str[i]);
    }
    return new_str;
}


int catalog_all_var_attributes_with_type_var ( md_catalog_type_entry type, md_catalog_var_entry var,
                                                    uint64_t type_id, uint64_t var_id, int num_dims, vector<md_dim_bounds> vect_dims ) {

    int rc;
    uint32_t count;
    vector<md_catalog_var_attribute_entry> attr_entries;
    vector<md_catalog_var_entry> var_entries;
    var_entries.push_back(var);

    rc = metadata_catalog_all_var_attributes_with_type_var_by_id(var.timestep_id, type_id, var_id, var.txn_id, count, attr_entries);
    if (rc == RC_OK) {
        testing_log << "using metadata_catalog_all_var_attributes_with_type_var_by_id: var attrs associated with run_id " << var.run_id << ", timestep_id " << var.timestep_id << " and txn_id: " << var.txn_id << " and type " << type.name << " ver " << type.version <<
        " and var " << var.name << " ver " << var.version;
        testing_log << "\n";
        if(testing_logging || (zero_rank_logging )) {
            print_var_attribute_list (count, attr_entries);
        }

        //print_var_attr_data(count, attr_entries);
    }
    else {
        error_log << "Error metadata_catalog_all_var_attributes_with_type_var_by_id. Proceeding" << endl;
    }
    // rc = retrieveObjNamesAndDataForAttrCatalog(data_outputs, new_run, var.run_id, var.timestep_id, var.txn_id, attr_entries );
    // if (rc != RC_OK) {
    //     error_log << "error. could not perform retrieveObjNamesAndDataForAttrCatalog \n";
    // }

    rc = metadata_catalog_all_var_attributes_with_type_var_by_name_ver(var.run_id, var.timestep_id, to_upper(type.name), type.version, 
            to_upper(var.name), var.version, var.txn_id, count, attr_entries);
    if (rc == RC_OK) {
        testing_log << "using metadata_catalog_all_var_attributes_with_type_var_by_name_ver: var attrs associated with run_id " << var.run_id << ", timestep_id " << 
            var.timestep_id << " and txn_id: " << var.txn_id << " and type " << to_upper(type.name) << " ver " << type.version <<
            " and var " << to_upper(var.name) << " ver " << var.version;
        testing_log << "\n";
        if(testing_logging || (zero_rank_logging )) {
            print_var_attribute_list (count, attr_entries);
        }

        //print_var_attr_data(count, attr_entries);
    }
    else {
        error_log << "Error metadata_catalog_all_var_attributes_with_type_var_by_name_ver. Proceeding" << endl;
    }
    // rc = retrieveObjNamesAndDataForAttrCatalog(data_outputs, new_run, var.run_id, var.timestep_id, var.txn_id, attr_entries );
    // if (rc != RC_OK) {
    //     error_log << "error. could not perform retrieveObjNamesAndDataForAttrCatalog \n";
    // }
    // string range_data;
    // uint64_t min_range = 5;
    // uint64_t max_range = 10;
    // make_range_data (min_range, max_range, range_data);

    // rc = metadata_catalog_all_var_attributes_with_type_var_range(var.timestep_id, type_id, var_id, var.txn_id, 
    //     ATTR_DATA_TYPE_BLOB, range_data, count, attr_entries);

    // if (rc == RC_OK) {
    //     testing_log << "using var attrs with type var by id funct: attributes associated with run_id " << var.run_id << ", timestep_id " << 
    //     var.timestep_id << " and txn_id: " << var.txn_id << " and type " << type.name << " ver " << type.version <<
    //     " and var " << var.name << " ver " << var.version << " int attr data and range [" << min_range << "," << max_range << "]\n";
    //     if(testing_logging || (zero_rank_logging )) {
    //         print_var_attribute_list (count, attr_entries);
    //     }

    //     //print_var_attr_data(count, attr_entries);
    // }
    // else {
    //     error_log << "Error metadata_catalog_all_var_attributes_with_type_var_range. Proceeding" << endl;
    // }

    // string range_data_new;
    // uint64_t min_range_new = 5;
    // uint64_t max_range_new = 11;
    // make_range_data (min_range_new, max_range_new, range_data_new);

    // rc = metadata_catalog_all_var_attributes_with_type_var_range(var.timestep_id, type_id, var_id, var.txn_id, 
    //     ATTR_DATA_TYPE_BLOB, range_data_new, count, attr_entries);

    // if (rc == RC_OK) {
    //     testing_log << "using var attrs with type var by id funct: attributes associated with run_id " << var.run_id << ", timestep_id " << 
    //     var.timestep_id << " and txn_id: " << var.txn_id << " and type " << type.name << " ver " << type.version <<
    //     " and var " << var.name << " ver " << var.version << " int attr data and range [" << min_range_new << "," << max_range_new << "]\n";
    //     if(testing_logging || (zero_rank_logging )) {
    //         print_var_attribute_list (count, attr_entries);
    //     }

    //     //print_var_attr_data(count, attr_entries);
    // }
    // else {
    //     error_log << "Error metadata_catalog_all_var_attributes_with_type_var_range. Proceeding" << endl;
    // }

    // string range_data1;
    // uint64_t new_attr.data = 12;
    // make_single_val_data (val, range_data1);

    // rc = metadata_catalog_all_var_attributes_with_type_var_above_max(var.timestep_id, type_id, var_id, var.txn_id, 
    //     ATTR_DATA_TYPE_BLOB, range_data1, count, attr_entries);

    // if (rc == RC_OK) {
    //     testing_log << "using var attrs with type var by id funct: attributes associated with run_id " << var.run_id << ", timestep_id " << 
    //     var.timestep_id << " and txn_id: " << var.txn_id << " and type " << type.name << " ver " << type.version <<
    //     " and var " << var.name << " ver " << var.version << " int attr data with val geq " << val << "\n";
    //     if(testing_logging || (zero_rank_logging )) {
    //         print_var_attribute_list (count, attr_entries);
    //     }

    //     //print_var_attr_data(count, attr_entries);
    // }
    // else {
    //     error_log << "Error metadata_catalog_all_var_attributes_with_type_var_above_max. Proceeding" << endl;
    // }

    // uint64_t val_new = 9;
    // make_single_val_data (val_new, range_data_new);

    // rc = metadata_catalog_all_var_attributes_with_type_var_above_max(var.timestep_id, type_id, var_id, var.txn_id, 
    //     ATTR_DATA_TYPE_BLOB, range_data_new, count, attr_entries);
    // if (rc == RC_OK) {
    //     testing_log << "using var attrs with type var by id funct: attributes associated with run_id " << var.run_id << ", timestep_id " << 
    //     var.timestep_id << " and txn_id: " << var.txn_id << " and type " << type.name << " ver " << type.version <<
    //     " and var " << var.name << " ver " << var.version << " int attr data with val geq " << val_new << "\n";
    //     if(testing_logging || (zero_rank_logging )) {
    //         print_var_attribute_list (count, attr_entries);
    //     }

    //     //print_var_attr_data(count, attr_entries);
    // }
    // else {
    //     error_log << "Error metadata_catalog_all_var_attributes_with_type_var_above_max. Proceeding" << endl;
    // }

    // string range_data2;
    // uint64_t val2 = 11;
    // make_single_val_data (val2, range_data2);

    // rc = metadata_catalog_all_var_attributes_with_type_var_below_min(var.timestep_id, type_id, var_id, var.txn_id, 
    //     ATTR_DATA_TYPE_BLOB, range_data2, count, attr_entries);

    // if (rc == RC_OK) {
    //     testing_log << "using var attrs with type var by id funct: attributes associated with run_id " << var.run_id << ", timestep_id " << 
    //     var.timestep_id << " and txn_id: " << var.txn_id << " and type " << type.name << " ver " << type.version <<
    //     " and var " << var.name << " ver " << var.version << " int attr data with val leq " << val2 << "\n";
    //     if(testing_logging || (zero_rank_logging )) {
    //         print_var_attribute_list (count, attr_entries);
    //     }

    //     //print_var_attr_data(count, attr_entries);
    // }
    // else {
    //     error_log << "Error metadata_catalog_all_var_attributes_with_type_var_below_min. Proceeding" << endl;
    // }

    // rc = metadata_catalog_all_var_attributes_with_type_var_below_min(var.timestep_id, type_id, var_id, var.txn_id, 
    //     ATTR_DATA_TYPE_BLOB, range_data_new, count, attr_entries);

    // if (rc == RC_OK) {
    //     testing_log << "using var attrs with type var by id funct: attributes associated with run_id " << var.run_id << ", timestep_id " << 
    //     var.timestep_id << " and txn_id: " << var.txn_id << " and type " << type.name << " ver " << type.version <<
    //     " and var " << var.name << " ver " << var.version << " int attr data with val leq " << val_new << "\n";
    //     if(testing_logging || (zero_rank_logging )) {
    //         print_var_attribute_list (count, attr_entries);
    //     }

    //     //print_var_attr_data(count, attr_entries);
    // }
    // else {
    //     error_log << "Error metadata_catalog_all_var_attributes_with_type_var_below_min. Proceeding" << endl;
    // }


    rc = metadata_catalog_all_var_attributes_with_type_var_dims_by_id(var.timestep_id, type_id, var_id, var.txn_id, num_dims, vect_dims, count, attr_entries);
    if (rc == RC_OK) {
       testing_log << "using metadata_catalog_all_var_attributes_with_type_var_dims_by_id: var attributes associated with run_id " << var.run_id << ", timestep_id " << 
            var.timestep_id << " and txn_id: " << var.txn_id << " and type " << type.name << " ver " << type.version <<
            " and var " << var.name << " ver " << var.version << " overlapping with dims";
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
        error_log << "Error metadata_catalog_all_var_attributes_with_type_var_dims_by_id. Proceeding" << endl;
    }
    // rc = retrieveObjNamesAndDataForAttrCatalog(data_outputs, new_run, var.run_id, var.timestep_id, var.txn_id, attr_entries );
    // if (rc != RC_OK) {
    //     error_log << "error. could not perform retrieveObjNamesAndDataForAttrCatalog \n";
    // }

    rc = metadata_catalog_all_var_attributes_with_type_var_dims_by_name_ver(var.run_id, var.timestep_id, to_upper(type.name), type.version, 
            to_upper(var.name), var.version, var.txn_id, num_dims, vect_dims, count, attr_entries);
    if (rc == RC_OK) {
        testing_log << "using metadata_catalog_all_var_attributes_with_type_var_dims_by_name_ver: var attrs associated with run_id " << var.run_id << ", timestep_id " << 
            var.timestep_id << " and txn_id: " << var.txn_id << " and type " << to_upper(type.name) << " ver " << type.version <<
            " and var " << to_upper(var.name) << " ver " << var.version << " overlapping with dims";
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
        error_log << "Error metadata_catalog_all_var_attributes_with_type_var_dims_by_name_ver. Proceeding" << endl;
    }
    testing_log << "\n";
    // rc = retrieveObjNamesAndDataForAttrCatalog(data_outputs, new_run, var.run_id, var.timestep_id, var.txn_id, attr_entries );
    // if (rc != RC_OK) {
    //     error_log << "error. could not perform retrieveObjNamesAndDataForAttrCatalog \n";
    // }

    // rc = metadata_catalog_all_var_attributes_with_type_var_dims_range(var.timestep_id, type_id, var_id, var.txn_id, 
    //     num_dims, vect_dims, ATTR_DATA_TYPE_BLOB, range_data, count, attr_entries);

    // if (rc == RC_OK) {
    //     testing_log << "using var attrs with type var by id funct: attributes associated with run_id " << var.run_id << ", timestep_id " << 
    //     var.timestep_id << " and txn_id: " << var.txn_id << " and type " << type.name << " ver " << type.version <<
    //     " and var " << var.name << " ver " << var.version << " int attr data and range [" << min_range << "," << max_range << "]" <<
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
    //     error_log << "Error metadata_catalog_all_var_attributes_with_type_var_dims_range. Proceeding" << endl;
    // }

    // rc = metadata_catalog_all_var_attributes_with_type_var_dims_above_max(var.timestep_id, type_id, var_id, var.txn_id, 
    //     num_dims, vect_dims, ATTR_DATA_TYPE_BLOB, range_data1, count, attr_entries);

    // if (rc == RC_OK) {
    //     testing_log << "using var attrs with type var by id funct: attributes associated with run_id " << var.run_id << ", timestep_id " << 
    //     var.timestep_id << " and txn_id: " << var.txn_id << " and type " << type.name << " ver " << type.version <<
    //     " and var " << var.name << " ver " << var.version << " int attr data and val geq " << val <<
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
    //     error_log << "Error metadata_catalog_all_var_attributes_with_type_var_dims_above_max. Proceeding" << endl;
    // }

    // rc = metadata_catalog_all_var_attributes_with_type_var_dims_above_max(var.timestep_id, type_id, var_id, var.txn_id, 
    //     num_dims, vect_dims, ATTR_DATA_TYPE_BLOB, range_data_new, count, attr_entries);

    // if (rc == RC_OK) {
    //     testing_log << "using var attrs with type var by id funct: attributes associated with run_id " << var.run_id << ", timestep_id " << 
    //     var.timestep_id << " and txn_id: " << var.txn_id << " and type " << type.name << " ver " << type.version <<
    //     " and var " << var.name << " ver " << var.version << " int attr data and val geq " << val_new <<
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
    //     error_log << "Error metadata_catalog_all_var_attributes_with_type_var_dims_above_max. Proceeding" << endl;
    // }

    // rc = metadata_catalog_all_var_attributes_with_type_var_dims_below_min(var.timestep_id, type_id, var_id, var.txn_id, 
    //     num_dims, vect_dims, ATTR_DATA_TYPE_BLOB, range_data2, count, attr_entries);

    // if (rc == RC_OK) {
    //     testing_log << "using var attrs with type var by id funct: attributes associated with run_id " << var.run_id << ", timestep_id " << 
    //     var.timestep_id << " and txn_id: " << var.txn_id << " and type " << type.name << " ver " << type.version <<
    //     " and var " << var.name << " ver " << var.version << " int attr data and val leq " << val2 <<
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
    //     error_log << "Error metadata_catalog_all_var_attributes_with_type_var_dims_below_min. Proceeding" << endl;
    // }

    // rc = metadata_catalog_all_var_attributes_with_type_var_dims_below_min(var.timestep_id, type_id, var_id, var.txn_id, 
    //     num_dims, vect_dims, ATTR_DATA_TYPE_BLOB, range_data_new, count, attr_entries);

    // if (rc == RC_OK) {
    //     testing_log << "using var attrs with type var by id funct: attributes associated with run_id " << var.run_id << ", timestep_id " << 
    //     var.timestep_id << " and txn_id: " << var.txn_id << " and type " << type.name << " ver " << type.version <<
    //     " and var " << var.name << " ver " << var.version << " int attr data and val leq " << val_new <<
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
    //     error_log << "Error metadata_catalog_all_var_attributes_with_type_var_dims_below_min. Proceeding" << endl;
    // }


    return rc;
}



int catalog_all_var_attributes ( uint64_t run_id, uint64_t timestep_id, uint64_t txn_id, int num_dims, 
                                                vector<md_dim_bounds> vect_dims ) {
    
    int rc;
    uint32_t count;
    vector<md_catalog_var_attribute_entry> attr_entries;

    rc = metadata_catalog_all_var_attributes (run_id, timestep_id, txn_id, count, attr_entries);
    if (rc == RC_OK) {
        testing_log << "using metadata_catalog_all_var_attributes: var attrs associated with run_id " << run_id << " and timestep_id " << timestep_id << " and txn_id: " << txn_id;
        testing_log << "\n";
        if(testing_logging || (zero_rank_logging )) {
            print_var_attribute_list (count, attr_entries);
        }

        //print_var_attr_data(count, attr_entries);
    }
    else {
        error_log << "Error metadata_catalog_all_var_attributes. Proceeding" << endl;
    }

    rc = metadata_catalog_all_var_attributes_with_dims(run_id, timestep_id, txn_id, num_dims, vect_dims, count, attr_entries);

    if (rc == RC_OK) {
        testing_log << "using metadata_catalog_all_var_attributes_with_dims: var attrs associated with run_id " << run_id << ", timestep_id " << timestep_id << " and txn_id: " << txn_id << " overlapping with vect_dims";
        for(int j=0; j< num_dims; j++) {
            testing_log << " d" << j << "_min: " << vect_dims [j].min;
            testing_log << " d" << j << "_max: " << vect_dims [j].max;               
        }
        testing_log << " \n";
        if(testing_logging || (zero_rank_logging )) {
            print_var_attribute_list (count, attr_entries);
        }

        //print_var_attr_data(count, attr_entries);
    }
    else {
        error_log << "Error metadata_catalog_all_var_attributes_with_dims. Proceeding" << endl;
    }
    testing_log << "\n";
    return rc;
}

int catalog_all_var_attributes_with_type ( uint64_t timestep_id, int num_dims, 
                                                vector<md_dim_bounds> vect_dims, md_catalog_type_entry type, uint64_t type_id ) {

    int rc;
    uint32_t count;
    vector<md_catalog_var_attribute_entry> attr_entries;


    rc = metadata_catalog_all_var_attributes_with_type_by_id (timestep_id, type_id, type.txn_id, count, attr_entries);

    if (rc == RC_OK) {
        testing_log << "using metadata_catalog_all_var_attributes_with_type_by_id: var attrs associated with run_id " << type.run_id << ", timestep_id " << timestep_id << " and txn_id: " << type.txn_id << " timestep_id " << timestep_id << " and type_id " << type_id << endl;
        if(testing_logging || (zero_rank_logging )) {
            print_var_attribute_list (count, attr_entries);
        }

        //print_var_attr_data(count, attr_entries);
    }
    else {
        error_log << "Error metadata_catalog_all_var_attributes_with_type_by_id. Proceeding" << endl;
    }


    rc = metadata_catalog_all_var_attributes_with_type_by_name_ver (type.run_id, timestep_id, to_upper(type.name), type.version, type.txn_id, count, attr_entries);

    if (rc == RC_OK) {
        testing_log << "using metadata_catalog_all_var_attributes_with_type_by_name_ver: var attrs associated with run_id " << type.run_id << ", timestep_id " << 
            timestep_id << " and txn_id: " << type.txn_id << " and type " << to_upper(type.name) << " ver " << type.version;
        testing_log << "\n";
        if(testing_logging || (zero_rank_logging )) {
            print_var_attribute_list (count, attr_entries);
        }

        //print_var_attr_data(count, attr_entries);
    }
    else {
        error_log << "Error metadata_catalog_all_var_attributes_with_type_by_name_ver. Proceeding" << endl;
    }

    rc = metadata_catalog_all_var_attributes_with_type_dims_by_id (timestep_id, type_id, type.txn_id, num_dims, vect_dims, count, attr_entries);

    if (rc == RC_OK) {
        testing_log << "using metadata_catalog_all_var_attributes_with_type_dims_by_id: var attrs associated with run_id " << type.run_id << ", timestep_id " << 
            timestep_id << " and txn_id: " << type.txn_id << " timestep_id " << timestep_id << " and type_id " << type_id <<
            " overlapping with dims";
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
        error_log << "Error metadata_catalog_all_var_attributes_with_type_dims_by_id. Proceeding" << endl;
    }


    rc = metadata_catalog_all_var_attributes_with_type_dims_by_name_ver(type.run_id, timestep_id, to_upper(type.name), type.version, type.txn_id, num_dims, vect_dims, count, attr_entries);

    if (rc == RC_OK) {
        testing_log << "using metadata_catalog_all_var_attributes_with_type_dims_by_name_ver: var attrs associated with run_id " << type.run_id << ", timestep_id " << 
            timestep_id << " and txn_id: " << type.txn_id << " and type " << to_upper(type.name) << " ver " << type.version << 
            " overlapping with dims";
        for(int j=0; j< num_dims; j++) {
            testing_log << " d" << j << "_min: " << vect_dims [j].min;
            testing_log << " d" << j << "_max: " << vect_dims [j].max;               
        }
        testing_log << " \n";
        if(testing_logging || (zero_rank_logging )) {
            print_var_attribute_list (count, attr_entries);
        }

        //print_var_attr_data(count, attr_entries);
    }
    else {
        error_log << "Error metadata_catalog_all_var_attributes_with_type_dims_by_name_ver. Proceeding" << endl;
    }
    testing_log << "\n";
    return rc;
}



static int catalog_all_var_attributes_with_type_id ( uint64_t timestep_id, uint64_t txn_id, uint64_t type_id ) {

    int rc;
    uint32_t count;
    vector<md_catalog_var_attribute_entry> attr_entries;


    rc = metadata_catalog_all_var_attributes_with_type_by_id (timestep_id, type_id, txn_id, count, attr_entries);

    if (rc == RC_OK) {
        testing_log << "using metadata_catalog_all_var_attributes_with_type_by_id: var attrs associated with timestep_id " << timestep_id << " and txn_id: " << 
            txn_id << " timestep_id " << timestep_id << " and type_id " << type_id << endl;
        if(testing_logging || (zero_rank_logging )) {
            print_var_attribute_list (count, attr_entries);
        }

        //print_var_attr_data(count, attr_entries);
    }
    else {
        error_log << "Error metadata_catalog_all_var_attributes_with_type_by_id. Proceeding" << endl;
    }
    testing_log << "\n";

    return rc;
}

int catalog_all_var_attributes_with_var ( md_catalog_var_entry var, uint64_t var_id, int num_dims, vector<md_dim_bounds> vect_dims ) {

    int rc;
    uint32_t count;
    vector<md_catalog_var_attribute_entry> attr_entries;
    vector<md_catalog_var_entry> var_entries;
    var_entries.push_back(var);


    rc = metadata_catalog_all_var_attributes_with_var_by_id (var.run_id, var.timestep_id, var_id, var.txn_id, count, attr_entries);

    if (rc == RC_OK) {
        testing_log << "using metadata_catalog_all_var_attributes_with_var_by_id: var attrs associated with run_id " << var.run_id << ", timestep_id " << var.timestep_id << 
            " and txn_id: " << var.txn_id << " and var " << var.name << " ver " << var.version;
        testing_log << "\n";
        if(testing_logging || (zero_rank_logging )) {
            print_var_attribute_list (count, attr_entries);
        }

        //print_var_attr_data(count, attr_entries);
    }
    else {
        error_log << "Error metadata_catalog_all_var_attributes_with_var_by_id. Proceeding" << endl;
    }
    rc = retrieveObjNamesAndDataForAttrCatalog(data_outputs, new_run, var.run_id, var.timestep_id, var.txn_id, attr_entries );
    if (rc != RC_OK) {
        error_log << "error. could not perform retrieveObjNamesAndDataForAttrCatalog \n";
    }

    rc = metadata_catalog_all_var_attributes_with_var_by_name_ver (var.run_id, var.timestep_id, to_upper(var.name), var.version, var.txn_id, count, attr_entries);
    if (rc == RC_OK) {
        testing_log << "using metadata_catalog_all_var_attributes_with_var_by_name_ver: var attrs associated with run_id " << var.run_id << ", timestep_id " << 
            var.timestep_id << " and txn_id: " << var.txn_id << " and var " << to_upper(var.name) << " ver " << var.version;
        testing_log << "\n";
        if(testing_logging || (zero_rank_logging )) {
            print_var_attribute_list (count, attr_entries);
        }

        //print_var_attr_data(count, attr_entries);
    }
    else {
        error_log << "Error metadata_catalog_all_var_attributes_with_var_by_name_ver. Proceeding" << endl;
    }
    // rc = retrieveObjNamesAndDataForAttrCatalog(data_outputs, new_run, var.run_id, var.timestep_id, var.txn_id, attr_entries );
    // if (rc != RC_OK) {
    //     error_log << "error. could not perform retrieveObjNamesAndDataForAttrCatalog \n";
    // }

    rc = metadata_catalog_all_var_attributes_with_var_dims_by_id (var.run_id, var.timestep_id, var_id, var.txn_id, num_dims, vect_dims, count, attr_entries);
    if (rc == RC_OK) {
        testing_log << "using metadata_catalog_all_var_attributes_with_var_dims_by_id: var attrs associated with run_id " << var.run_id << ", timestep_id " << 
            var.timestep_id << " and txn_id: " << var.txn_id << " and var " << var.name << " ver " << var.version << 
            " overlapping with dims";
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
        error_log << "Error metadata_catalog_all_var_attributes_with_var_dims_by_id. Proceeding" << endl;
    }
    // rc = retrieveObjNamesAndDataForAttrCatalog(data_outputs, new_run, var.run_id, var.timestep_id, var.txn_id, attr_entries );
    // if (rc != RC_OK) {
    //     error_log << "error. could not perform retrieveObjNamesAndDataForAttrCatalog \n";
    // }

    rc = metadata_catalog_all_var_attributes_with_var_dims_by_name_ver(var.run_id, var.timestep_id, to_upper(var.name), var.version, var.txn_id, num_dims, vect_dims, count, attr_entries);
    if (rc == RC_OK) {
        testing_log << "using metadata_catalog_all_var_attributes_with_var_dims_by_name_ver: var attrs associated with run_id " << var.run_id << ", timestep_id " << 
            var.timestep_id << " and txn_id: " << var.txn_id << " and var " << to_upper(var.name) << " ver " << var.version << 
            " overlapping with dims";
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
        error_log << "Error metadata_catalog_all_var_attributes_with_var_dims_by_name_ver. Proceeding" << endl;
    }
    // rc = retrieveObjNamesAndDataForAttrCatalog(data_outputs, new_run, var.run_id, var.timestep_id, var.txn_id, attr_entries );
    // if (rc != RC_OK) {
    //     error_log << "error. could not perform retrieveObjNamesAndDataForAttrCatalog \n";
    // }

    return rc;
}

// int catalog_all_var_attributes_with_var_id ( uint64_t run_id, uint64_t timestep_id, uint64_t txn_id, uint64_t var_id ) {

//     int rc;
//     uint32_t count;
//     vector<md_catalog_var_attribute_entry> attr_entries;


//     rc = metadata_catalog_all_var_attributes_with_var_by_id (run_id, timestep_id, var_id, txn_id, count, attr_entries);

//     if (rc == RC_OK) {
//         testing_log << "attributes associated with run_id " << var.run_id << ", timestep_id " << var.timestep_id << " and txn_id: " << txn_id << " and var " << var_name << " ver " << var_version;
//         testing_log << "\n";
//         if(testing_logging || (zero_rank_logging )) {
//             print_var_attribute_list (count, attr_entries);
//         }

//         //print_var_attr_data(count, attr_entries);
//     }
//     else {
//         error_log << "Error metadata_catalog_all_var_attributes_with_var_by_id. Proceeding" << endl;
//     }

// }

int test_activate_run ( md_catalog_run_entry prev_run, md_catalog_type_entry prev_type, 
                string rank_to_dims_funct_name, string rank_to_dims_funct_path, 
                string objector_funct_name, string objector_funct_path) {

    int rc;
    uint32_t count;
    vector<md_catalog_run_entry> run_entries;
    vector<md_catalog_run_attribute_entry> attr_entries;
    vector<md_catalog_type_entry> type_entries;

    uint64_t new_txn_id = prev_run.txn_id + 100;
    uint64_t new_run_id;

    uint64_t new_type_id;
    uint64_t job_id = 0;

    rc = metadata_create_run (new_run_id, job_id, prev_run.name, new_txn_id, prev_run.npx, prev_run.npy, prev_run.npz, 
                        rank_to_dims_funct_name, rank_to_dims_funct_path, objector_funct_name, objector_funct_path);
    if (rc != RC_OK) {
        error_log << "Error. Was unable to create the new run. Proceeding" << endl;
    }

    // uint64_t new_type.txn_id = prev_type.txn_id + 100;
    md_catalog_type_entry new_type = prev_type;
    new_type.run_id = new_run_id;
    new_type.txn_id = new_txn_id;
    new_type.version = 3;

    rc = metadata_create_type (new_type_id, new_type);
    if (rc != RC_OK) {
        error_log << "Error. Was unable to create the new type. Proceeding" << endl;
    }
    md_log( "New type id is " + to_string(new_type_id) );

/////////////active run testing//////////////////////////////////////////////////////////////////////////////
    // if(my_server_type  != SERVER_LOCAL_SQLITE_TRANSACTION_MANAGEMENT_WAL) {

        rc = metadata_activate_run(new_txn_id);
        if (rc == RC_OK) {
            testing_log << "just activated the new run  \n";
        }
        else {
            error_log << "Error. Was unable to activate the new run. Exiting \n";
            return RC_ERR;
        }

        //returns information about the runs associated with the given txn_id 
        rc = metadata_catalog_run (prev_run.txn_id, count, run_entries);

        if (rc == RC_OK) {
            md_log( "now the number of runs is " + to_string(count) );

            testing_log << "new run catalog: \n";
            if(testing_logging || (zero_rank_logging )) {
                print_run_catalog (count, run_entries);
            }
        }
        else {
            error_log << "Error cataloging the new set of run entries. Proceeding" << endl;
        }


        rc = metadata_processing_run(new_txn_id);

        if (rc == RC_OK) {
            testing_log << "just deactivated the new run \n";
        }
        else {
            error_log << "Error. Was unable to deactivate the new run. Exiting \n";
            return RC_ERR;
        }
    // }

        //returns information about the runs associated with the given txn_id 
    rc = metadata_catalog_run (prev_run.txn_id, count, run_entries);
    if (rc == RC_OK) {
        md_log( "now the number of runs is " + to_string(count) );

        testing_log << "new run catalog: \n";
        if(testing_logging || (zero_rank_logging )) {
            print_run_catalog (count, run_entries);
        }
    }
    else {
        error_log << "Error cataloging the new set of run entries. Proceeding" << endl;
    }

   //returns information about the types associated with the given txn_id 
    rc = metadata_catalog_type (new_run_id, new_txn_id, count, type_entries);
    if (rc == RC_OK) {
        md_log( "now the number of types is " + to_string(count) );

        testing_log << "new type catalog for run_id " << new_run_id << " and txn_id: " << new_txn_id << ": \n";
        if(testing_logging || (zero_rank_logging )) {
            print_type_catalog (count, type_entries);
        }
    }
    else {
        error_log << "Error cataloging the new set of type entries. Proceeding" << endl;
    }

/////////////end active run testing//////////////////////////////////////////////////////////////////////////////
/////////////start delete run testing//////////////////////////////////////////////////////////////////////////////

    // prev_run.txn_id = new_txn_id;
    rc = test_activate_run_attrs ( prev_run, new_run_id, new_type_id );
    if (rc == RC_OK) {
        testing_log << "just did activate run attr testing  \n";
    }
    else {
        error_log << "Error. Problem with activate run attr testing. Exiting \n";
        return RC_ERR;
    }

    rc = metadata_delete_run_by_id (new_run_id);
    if (rc == RC_OK) {
        testing_log << "just deleted the new run  \n";
    }
    else {
        error_log << "Error. Problem deleting the new run. Exiting \n";
        return RC_ERR;
    }

    //should be empty now
    rc = metadata_catalog_run (new_txn_id, count, run_entries);
    if (rc == RC_OK) {
        cout << "now the number of runs associated with txn_id " << new_txn_id << " is " << count << endl;
        testing_log << "new run catalog: \n";
        if(testing_logging || (zero_rank_logging )) {
            print_run_catalog (count, run_entries);
        }
    }
    else {
        error_log << "Error cataloging the new set of run entries. Proceeding" << endl;
    }

   //returns information about the types associated with the given txn_id 
    rc = metadata_catalog_type (new_run_id, new_txn_id, count, type_entries);

    if (rc == RC_OK) {
        md_log( "now the number of types is " + to_string(count) );

        testing_log << "new type catalog for run_id " << new_run_id << " and txn_id: " << new_txn_id << ": \n";
        if(testing_logging || (zero_rank_logging )) {
            print_type_catalog (count, type_entries);
        }
    }
    else {
        error_log << "Error cataloging the new set of type entries. Proceeding" << endl;
    }

    //should be empty now
    rc = metadata_catalog_all_run_attributes ( new_txn_id, count, attr_entries );
    if (rc == RC_OK) {
        testing_log << "using metadata_catalog_all_run_attributes: run attrs associated with txn_id: " << new_txn_id;
        testing_log << endl;
        if(testing_logging || (zero_rank_logging )) {
            print_run_attribute_list (count, attr_entries);
        }
    }
    else {
        error_log << "Error metadata_catalog_all_run_attributes. Proceeding" << endl;
    }

    rc = metadata_catalog_all_run_attributes_in_run ( new_run_id, new_txn_id, count, attr_entries );
    if (rc == RC_OK) {
        testing_log << "using metadata_catalog_all_run_attributes_in_run: run attrs associated with run_id " << new_run_id << " and txn_id: " << new_txn_id;
        testing_log << endl;
        if(testing_logging || (zero_rank_logging )) {
            print_run_attribute_list (count, attr_entries);
        }
    }
    else {
        error_log << "Error metadata_catalog_all_run_attributes_in_run. Proceeding" << endl;
    }

    testing_log << endl;
    return rc;
}

static int test_activate_run_attrs ( md_catalog_run_entry prev_run, uint64_t run_id, uint64_t type_id ) {

    int rc;
    uint32_t count;
    vector<md_catalog_run_attribute_entry> attr_entries;

    uint64_t new_attr_id;
    uint64_t new_txn_id = prev_run.txn_id + 100;

    md_catalog_run_attribute_entry new_attr;
    new_attr.run_id = run_id;
    new_attr.type_id = type_id;
    new_attr.txn_id = new_txn_id;

    // make_attr_data("act_test", 500, new_attr.data);
    string str("act_\0newtest",9);
    make_attr_data(str, 500, new_attr.data);
    // new_attr.data = "500";
    // new_attr.data = val;
    uint64_t val = 500;
    // make_single_val_data (val, new_attr.data);
    new_attr.data_type = ATTR_DATA_TYPE_BLOB;

    rc = metadata_insert_run_attribute (new_attr_id, new_attr);
    if (rc != RC_OK) {
        error_log << "Error. Was unable to insert the new run attribute. Proceeding" << endl;
    }
    md_log( "New run attribute id is " + to_string(new_attr_id) );
    
    // if(my_server_type  != SERVER_LOCAL_SQLITE_TRANSACTION_MANAGEMENT_WAL) {

        rc = metadata_activate_run_attribute(new_txn_id);
        if (rc == RC_OK) {
            testing_log << "just activated the new run attribute \n";
        }
        else {
            error_log << "Error. Was unable to activate the new run attribute. Exiting \n";
            return RC_ERR;
        }

        rc = metadata_catalog_all_run_attributes ( prev_run.txn_id, count, attr_entries );
        if (rc == RC_OK) {
            testing_log << "using metadata_catalog_all_run_attributes: run attrs associated with txn_id: " << prev_run.txn_id;
            testing_log << "\n";
            if(testing_logging || (zero_rank_logging )) {
                print_run_attribute_list (count, attr_entries);
            }
        }
        else {
            error_log << "Error metadata_catalog_all_run_attributes. Proceeding" << endl;
        }

        rc = metadata_catalog_all_run_attributes_in_run ( run_id, prev_run.txn_id, count, attr_entries );
        if (rc == RC_OK) {
            testing_log << "using metadata_catalog_all_run_attributes_in_run: run attrs associated with run_id " << run_id << " and txn_id: " << prev_run.txn_id;
            testing_log << "\n";
            if(testing_logging || (zero_rank_logging )) {
                print_run_attribute_list (count, attr_entries);
            }
        }
        else {
            error_log << "Error metadata_catalog_all_run_attributes_in_run. Proceeding" << endl;
        }

        rc = metadata_processing_run_attribute(new_txn_id);

        if (rc == RC_OK) {
            testing_log << "just deactivated the new run attribute \n";
        }
        else {
            error_log << "Error. Was unable to deactivate the new run attribute. Exiting \n";
            return RC_ERR;
        }
    // }

    rc = metadata_catalog_all_run_attributes ( prev_run.txn_id, count, attr_entries );
    if (rc == RC_OK) {
        testing_log << "using metadata_catalog_all_run_attributes: run attrs associated with txn_id: " << prev_run.txn_id;
        testing_log << "\n";
        if(testing_logging || (zero_rank_logging )) {
            print_run_attribute_list (count, attr_entries);
        }
    }
    else {
        error_log << "Error getting the matching run attribute list. Proceeding" << endl;
    }

    rc = metadata_catalog_all_run_attributes_in_run ( run_id, prev_run.txn_id, count, attr_entries );
    if (rc == RC_OK) {
        testing_log << "using metadata_catalog_all_run_attributes_in_run: run attrs associated with run_id " << run_id << " and txn_id: " << prev_run.txn_id;
        testing_log << "\n";
        if(testing_logging || (zero_rank_logging )) {
            print_run_attribute_list (count, attr_entries);
        }
    }
    else {
        error_log << "Error getting the matching run attribute list. Proceeding" << endl;
    }

    testing_log << "\n";
    return rc;
}

int test_activate_and_delete_timestep ( md_catalog_timestep_entry prev_timestep, 
                        md_catalog_type_entry type, md_catalog_var_entry prev_var, md_catalog_var_attribute_entry prev_attr, uint64_t old_timestep_id, uint64_t type_id,
                        int num_dims, vector<md_dim_bounds> vect_dims) {

    int rc;
    uint32_t count;
    vector<md_catalog_timestep_entry> timestep_entries;
    vector<md_catalog_var_entry> var_entries;


    md_catalog_timestep_entry new_timestep = prev_timestep;

    new_timestep.txn_id += 100;
    new_timestep.timestep_id = old_timestep_id + 100;

    rc = metadata_create_timestep (new_timestep.timestep_id, new_timestep.run_id, new_timestep.path, new_timestep.txn_id);
    if (rc != RC_OK) {
        error_log << "Error. Was unable to create the new timestep. Proceeding" << endl;
    }

    testing_log << "new timestep. timestep_id: " << new_timestep.timestep_id << " run_id: " << new_timestep.run_id << " txn_id: " << new_timestep.txn_id << endl;


/////////////active timestep testing//////////////////////////////////////////////////////////////////////////////
    // if(my_server_type  != SERVER_LOCAL_SQLITE_TRANSACTION_MANAGEMENT_WAL) {
   
        rc = metadata_activate_timestep(new_timestep.txn_id);
        if (rc == RC_OK) {
            testing_log << "just activated the new timestep  \n";
        }
        else {
            error_log << "Error. Was unable to activate the new timestep. Exiting \n";
            return RC_ERR;
        }

        testing_log << "old timestep run_id: " << prev_timestep.run_id << " txn_id: " << prev_timestep.txn_id << endl;

        //returns information about the timesteps associated with the given txn_id 
        rc = metadata_catalog_timestep (new_timestep.run_id, prev_timestep.txn_id, count, timestep_entries);
        if (rc == RC_OK) {
            md_log( "now the number of timesteps is " + to_string(count) );

            testing_log << "new timestep catalog associated with run_id: " << new_timestep.run_id << " and txn_id: " << prev_timestep.txn_id << ": \n";
            if(testing_logging || (zero_rank_logging )) {
                print_timestep_catalog (count, timestep_entries);
            }
        }
        else {
            error_log << "Error cataloging the new set of timestep entries. Proceeding" << endl;
        }

        rc = metadata_processing_timestep(new_timestep.txn_id);

        if (rc == RC_OK) {
            testing_log << "just deactivated the new timestep \n";
        }
        else {
            error_log << "Error. Was unable to deactivate the new timestep. Exiting \n";
            return RC_ERR;
        }
    // }

        //returns information about the timesteps associated with the given txn_id 
    rc = metadata_catalog_timestep (new_timestep.run_id, prev_timestep.txn_id, count, timestep_entries);

    if (rc == RC_OK) {
        md_log( "now the number of timesteps is " + to_string(count) );

        testing_log << "new timestep catalog associated with run_id: " << new_timestep.run_id << " and txn_id: " << prev_timestep.txn_id << ": \n";
        if(testing_logging || (zero_rank_logging )) {
            print_timestep_catalog (count, timestep_entries);
        }
    }
    else {
        error_log << "Error cataloging the new set of timestep entries. Proceeding" << endl;
    }
/////////////end active timestep testing//////////////////////////////////////////////////////////////////////////////
/////////////start delete timestep testing//////////////////////////////////////////////////////////////////////////////


    //add - need to add activate timestep attrs here, and to add other things for deletion (vars, timestep attrs, var attrs)

    rc = test_activate_timestep_attrs ( new_timestep, type_id );
    if (rc != RC_OK) {
        error_log << "Error with the test activate timestep attributes testing. Exiting \n";
        return rc;
    }

    uint64_t new_var_id;
    uint64_t new_attr_id;

    prev_var.timestep_id = new_timestep.timestep_id;
    prev_var.txn_id = new_timestep.txn_id;
    // prev_var.var_id = 2;
    rc = metadata_create_var (new_var_id, prev_var);
    if (rc != RC_OK) {
        error_log << "Error. Was unable to create the new var. Exiting \n";
        return RC_ERR;
    }

   //do for each var
    rc = write_output(data_outputs, new_run, prev_var); 
    if (rc != RC_OK) {
        error_log << "Error. Was unable to write output for the first var. Exiting \n";
        return RC_ERR;
    }


    //returns information about the vars associated with the given txn_id 
    rc = metadata_catalog_var (new_timestep.run_id, new_timestep.timestep_id, new_timestep.txn_id, count, var_entries);

    if (rc == RC_OK) {
        testing_log << "new var catalog: \n";
        if(testing_logging || (zero_rank_logging )) {
            print_var_catalog (count, var_entries);
        }
    }
    else {
        error_log << "Error cataloging the new set of var entries. Proceeding" << endl;
    }

    prev_attr.timestep_id = new_timestep.timestep_id;
    prev_attr.txn_id = new_timestep.txn_id;
    rc = metadata_insert_var_attribute_by_dims (new_attr_id, prev_attr);
    if (rc != RC_OK) {
        error_log << "Error. Was unable to insert the new attribute. Proceeding" << endl;
    }
    md_log( "New var attribute id is " + to_string(new_attr_id) );
    
    rc = catalog_all_var_attributes ( new_timestep.run_id, new_timestep.timestep_id, new_timestep.txn_id, num_dims, vect_dims );
    if (rc != RC_OK) {
        error_log << "Error cataloging all var attributes. Proceeding" << endl;
    }
 
    rc = metadata_delete_timestep_by_id ( new_timestep.timestep_id, new_timestep.run_id);
    if (rc != RC_OK) {
        error_log << "Error deleting timestep by id. Proceeding" << endl;
    }

    testing_log << "just deleted the timestep with id " << new_timestep.timestep_id << endl;

    //returns information about the vars associated with the given txn_id 
    rc = metadata_catalog_var (new_timestep.run_id, new_timestep.timestep_id, new_timestep.txn_id, count, var_entries);

    if (rc == RC_OK) {
        testing_log << "new var catalog: \n";
        if(testing_logging || (zero_rank_logging )) {
            print_var_catalog (count, var_entries);
        }
    }
    else {
        error_log << "Error cataloging the new set of var entries. Proceeding" << endl;
    }
    
    rc = catalog_all_var_attributes ( new_timestep.run_id, new_timestep.timestep_id, new_timestep.txn_id, num_dims, vect_dims );
    if (rc != RC_OK) {
        error_log << "Error cataloging all var attributes. Proceeding" << endl;
    }

    testing_log << "\n";
    return rc;
}

static int test_activate_timestep_attrs ( md_catalog_timestep_entry timestep, uint64_t type_id ) {

    int rc;
    uint32_t count;
    vector<md_catalog_timestep_attribute_entry> attr_entries;

    uint64_t new_attr_id;
    uint64_t new_txn_id = timestep.txn_id;

    md_catalog_timestep_attribute_entry new_attr;
    new_attr.timestep_id = timestep.timestep_id;
    new_attr.type_id = type_id;
    new_attr.txn_id = new_txn_id;

    string str("act_\0newtest",9);
    make_attr_data(str, 500, new_attr.data);
    // new_attr.data = "500";
    // new_attr.data = val;
    uint64_t val = 500;
    // make_single_val_data (val, new_attr.data);
    new_attr.data_type = ATTR_DATA_TYPE_BLOB;

    rc = metadata_insert_timestep_attribute (new_attr_id, new_attr);
    if (rc != RC_OK) {
        error_log << "Error. Was unable to insert the new timestep attribute. Proceeding" << endl;
    }
    md_log( "New timestep attribute id is " + to_string(new_attr_id) );
    
    // if(my_server_type  != SERVER_LOCAL_SQLITE_TRANSACTION_MANAGEMENT_WAL) {

        rc = metadata_activate_timestep_attribute(new_txn_id);
        if (rc == RC_OK) {
            testing_log << "just activated the new timestep attribute \n";
        }
        else {
            error_log << "Error. Was unable to activate the new attribute. Exiting \n";
            return RC_ERR;
        }

        rc = metadata_catalog_all_timestep_attributes(timestep.run_id, timestep.txn_id, count, attr_entries);
        if (rc == RC_OK) {
            testing_log << "using metadata_catalog_all_timestep_attributes: timestep attrs associated with run id: " << timestep.run_id << " and txn_id: " << 
                timestep.txn_id <<  "\n";
            if(testing_logging || (zero_rank_logging )) {
                print_timestep_attribute_list (count, attr_entries);
            }
        }
        else {
            error_log << "Error metadata_catalog_all_timestep_attributes. Proceeding" << endl;
        }

        rc = metadata_catalog_all_timestep_attributes_in_timestep(timestep.run_id, timestep.timestep_id, timestep.txn_id, count, attr_entries);
        if (rc == RC_OK) {
            testing_log << "using metadata_catalog_all_timestep_attributes_in_timestep: timestep attrs associated with run id: " << timestep.run_id << " and timestep: " << timestep.timestep_id << " and txn_id: " << 
                timestep.txn_id <<  "\n";
            if(testing_logging || (zero_rank_logging )) {
                print_timestep_attribute_list (count, attr_entries);
            }
        }
        else {
            error_log << "Error metadata_catalog_all_timestep_attributes_in_timestep. Proceeding" << endl;
        }

        rc = metadata_processing_timestep_attribute(new_txn_id);
        if (rc == RC_OK) {
            testing_log << "just deactivated the new timestep attribute \n";
        }
        else {
            error_log << "Error. Was unable to deactivate the new timestep attribute. Exiting \n";
            return RC_ERR;
        }
    // }
  

    rc = metadata_catalog_all_timestep_attributes(timestep.run_id, timestep.txn_id, count, attr_entries);
    if (rc == RC_OK) {
        testing_log << "using metadata_catalog_all_timestep_attributes: timestep attrs associated with run id: " << timestep.run_id << " and txn_id: " << 
            timestep.txn_id <<  "\n";
        if(testing_logging || (zero_rank_logging )) {
            print_timestep_attribute_list (count, attr_entries);
        }
    }
    else {
        error_log << "Error metadata_catalog_all_timestep_attributes. Proceeding" << endl;
    }

    rc = metadata_catalog_all_timestep_attributes_in_timestep(timestep.run_id, timestep.timestep_id, timestep.txn_id, count, attr_entries);
    if (rc == RC_OK) {
        testing_log << "using metadata_catalog_all_timestep_attributes_in_timestep: timestep attrs associated with run id: " << timestep.run_id << " and timestep: " << timestep.timestep_id << " and txn_id: " << 
            timestep.txn_id <<  "\n";
        if(testing_logging || (zero_rank_logging )) {
            print_timestep_attribute_list (count, attr_entries);
        }
    }
    else {
        error_log << "Error metadata_catalog_all_timestep_attributes_in_timestep. Proceeding" << endl;
    }

    testing_log << "\n";
    return rc;
}


//fix - should I be putting this in along with activate run? 
int test_activate_and_delete_type ( md_catalog_type_entry prev_type, md_catalog_var_attribute_entry prev_attr) {

    int rc;
    uint32_t count;
    vector<md_catalog_type_entry> type_entries;

    vector<md_catalog_var_attribute_entry> attr_entries;

    uint64_t new_type_id;
    uint64_t new_attr_id;

    // uint64_t new_type.txn_id = prev_type.txn_id + 100;
    md_catalog_type_entry new_type = prev_type;
    new_type.txn_id = prev_type.txn_id + 100;
    new_type.version = 3;


    rc = metadata_create_type (new_type_id, new_type);
    if (rc != RC_OK) {
        error_log << "Error. Was unable to create the new type. Proceeding" << endl;
    }
    md_log( "New type id is " + to_string(new_type_id) );

/////////////active type testing//////////////////////////////////////////////////////////////////////////////
    // if(my_server_type  != SERVER_LOCAL_SQLITE_TRANSACTION_MANAGEMENT_WAL) {
   
        rc = metadata_activate_type(new_type.txn_id);
        if (rc == RC_OK) {
            testing_log << "just activated the new type  \n";
        }
        else {
            error_log << "Error. Was unable to activate the new type. Exiting \n";
            return RC_ERR;
        }

        //returns information about the types associated with the given txn_id 
        rc = metadata_catalog_type (new_type.run_id, prev_type.txn_id, count, type_entries);

        if (rc == RC_OK) {
            md_log( "now the number of types is " + to_string(count) );

            testing_log << "new type catalog for run_id " << new_type.run_id << " and txn_id: " << prev_type.txn_id << ": \n";
            if(testing_logging || (zero_rank_logging )) {
                print_type_catalog (count, type_entries);
            }
        }
        else {
            error_log << "Error cataloging the new set of type entries. Proceeding" << endl;
        }

        rc = metadata_processing_type(new_type.txn_id);

        if (rc == RC_OK) {
            testing_log << "just deactivated the new type \n";
        }
        else {
            error_log << "Error. Was unable to deactivate the new type. Exiting \n";
            return RC_ERR;
        }
    // }

        //returns information about the types associated with the given txn_id 
    rc = metadata_catalog_type (new_type.run_id, prev_type.txn_id, count, type_entries);

    if (rc == RC_OK) {
        md_log( "now the number of types is " + to_string(count) );

        testing_log << "new type catalog for run_id " << new_type.run_id << " and txn_id: " << prev_type.txn_id << ": \n";
        if(testing_logging || (zero_rank_logging )) {
            print_type_catalog (count, type_entries);
        }
    }
    else {
        error_log << "Error cataloging the new set of type entries. Proceeding" << endl;
    }
/////////////end active var testing//////////////////////////////////////////////////////////////////////////////
/////////////start delete var testing//////////////////////////////////////////////////////////////////////////////
    prev_attr.type_id = new_type_id;
    prev_attr.txn_id = new_type.txn_id;
    rc = metadata_insert_var_attribute_by_dims (new_attr_id, prev_attr);
    if (rc != RC_OK) {
        error_log << "Error. Was unable to insert the new attribute. Proceeding" << endl;
    }
    md_log( "New var attribute id is " + to_string(new_attr_id) );

    rc = catalog_all_var_attributes_with_type_id ( prev_attr.timestep_id, new_type.txn_id, new_type_id );
    if (rc != RC_OK) {
        error_log << "Error. Was unable to catalog all var attributes with_type_id " << new_type_id << ". Proceeding" << endl;
    }


    rc = metadata_delete_type_by_id (new_type_id);
    if (rc == RC_OK) {
        testing_log << "just deleted the new type by id  \n";
    }
    else {
        error_log << "Error. Problem deleting the new type. Exiting \n";
        return RC_ERR;
    }
    // rc = metadata_delete_type_by_name_ver (new_type.run_id, new_type.name, new_type.version);
    // if (rc == RC_OK) {
    //     testing_log << "just deleted the new type by name ver  \n";
    // }
    // else {
    //     error_log << "Error. Problem deleting the new type. Exiting \n";
    //     return RC_ERR;
    // }

    //should be empty now
    rc = metadata_catalog_type (new_type.run_id, prev_type.txn_id, count, type_entries);
    if (rc == RC_OK) {
        md_log( "now the number of types is " + to_string(count) );

        testing_log << "new type catalog for run_id " << new_type.run_id << ": \n";
        if(testing_logging || (zero_rank_logging )) {
            print_type_catalog (count, type_entries);
        }
    }
    else {
        error_log << "Error cataloging the new set of type entries. Proceeding" << endl;
    }

    //should be empty now
    rc = catalog_all_var_attributes_with_type_id ( prev_attr.timestep_id, new_type.txn_id, new_type_id );
    if (rc != RC_OK) {
        error_log << "Error. Was unable to catalog all var attributes with_type_id " << new_type_id << ". Proceeding" << endl;
    }
    testing_log << "\n";

    //should be empty now

    // rc = metadata_catalog_all_var_attributes ( prev_type.run_id, prev_attr.timestep_id, prev_type.txn_id, count, attr_entries);
    // if (rc == RC_OK) {
    //     testing_log << "var attributes associated with run_id " << prev_type.run_id << " and timestep_id " << prev_attr.timestep_id << " and txn_id: " << prev_type.txn_id;
    //     testing_log << "\n";
    //     if(testing_logging || (zero_rank_logging )) {
    //         print_var_attribute_list (count, attr_entries);
    //     }

    //     //print_var_attr_data(count, attr_entries);
    // }
    // else {
    //     error_log << "Error getting the matching attribute list. Proceeding" << endl;
    // }

    return rc;
}


int test_activate_and_delete_var_and_var_attrs ( uint64_t type_id, 
                                            md_catalog_type_entry prev_type, md_catalog_var_entry prev_var, int num_dims, vector<md_dim_bounds> vect_dims) {

    int rc;
    uint32_t count;
    vector<md_catalog_var_entry> var_entries;

    uint64_t new_var_id;
    md_catalog_var_entry new_var = prev_var;

    new_var.txn_id = prev_var.txn_id + 100;
    // new_var.name += "0";

    new_var.version = 2;
    new_var.var_id = 3;
    vector<md_dim_bounds> dims(3);
    // dims [0]. min = 1;
    // dims [0]. max = 50;
    // dims [1]. min = 1;
    // dims [1]. max = 50;
    // dims [2]. min = 1;
    // dims [2]. max = 50;
    dims [0]. min = 0;
    dims [0]. max = 49;
    dims [1]. min = 0;
    dims [1]. max = 49;
    dims [2]. min = 0;
    dims [2]. max = 49;
    
    new_var.dims = dims;
    extreme_debug_log << "about to create new var \n";

    rc = metadata_create_var (new_var_id, new_var);
    if (rc != RC_OK) {
        error_log << "Error. Was unable to create the new var. Exiting \n";
        return RC_ERR;
    }
   //do for each var

    extreme_debug_log << "about to write output \n";
    rc = write_output(data_outputs, new_run, new_var); 
    if (rc != RC_OK) {
        error_log << "Error. Was unable to write output for the first var. Exiting \n";
        return RC_ERR;
    }


/////////////active var testing//////////////////////////////////////////////////////////////////////////////
    // if(my_server_type  != SERVER_LOCAL_SQLITE_TRANSACTION_MANAGEMENT_WAL) {
   
        extreme_debug_log << "about to activate var \n";
        rc = metadata_activate_var(new_var.txn_id);
        if (rc == RC_OK) {
            testing_log << "just activated the new var  \n";
        }
        else {
            error_log << "Error. Was unable to activate the new var. Exiting \n";
            return RC_ERR;
        }


        // extreme_debug_log << "run_id: " << prev_var.run_id << " timestep_id: " << prev_var.timestep_id << " txn_id: " << prev_var.txn_id << endl;

        //returns information about the vars associated with the given txn_id 
        // extreme_debug_log << "about to catalog var \n";
        rc = metadata_catalog_var (prev_var.run_id, prev_var.timestep_id, prev_var.txn_id, count, var_entries);

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

        rc = metadata_processing_var(new_var.txn_id);

        if (rc == RC_OK) {
            testing_log << "just deactivated the new var \n";
        }
        else {
            error_log << "Error. Was unable to deactivate the new var. Exiting \n";
            return RC_ERR;
        }
    // }

        //returns information about the vars associated with the given txn_id 
    rc = metadata_catalog_var (prev_var.run_id, prev_var.timestep_id, prev_var.txn_id, count, var_entries);

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
/////////////end active var testing//////////////////////////////////////////////////////////////////////////////
/////////////start delete var testing//////////////////////////////////////////////////////////////////////////////

    rc = test_activate_var_attrs ( type_id, new_var_id, prev_type, new_var, num_dims, vect_dims);
    if (rc == RC_OK) {
        testing_log << "just did activate var attr testing  \n";
    }
    else {
        error_log << "Error. Problem with the activate var attr testing. Exiting \n";
        return RC_ERR;
    }

    rc = metadata_delete_var_by_id (new_var.run_id, new_var.timestep_id, new_var_id);
    if (rc == RC_OK) {
        testing_log << "just deleted the new var by id  \n";
    }
    else {
        error_log << "Error. Problem deleting the new var. Exiting \n";
        return RC_ERR;
    }
    // rc = metadata_delete_var_by_name_path_ver (new_var.run_id, new_var.timestep_id, new_var.name, new_var.path, new_var.version);
    // if (rc == RC_OK) {
    //     testing_log << "just deleted the new var by name path ver \n";
    // }
    // else {
    //     error_log << "Error. Problem deleting the new var. Exiting \n";
    //     return RC_ERR;
    // }


    rc = metadata_catalog_var (new_var.run_id, new_var.timestep_id, prev_var.txn_id, count, var_entries);
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

    rc = catalog_all_var_attributes ( new_var.run_id, new_var.timestep_id, prev_var.txn_id, num_dims, vect_dims );

    //should be empty now
    rc = metadata_catalog_var (new_var.run_id, new_var.timestep_id, new_var.txn_id, count, var_entries);
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

    //should be empty now
    rc = catalog_all_var_attributes ( new_var.run_id, new_var.timestep_id, new_var.txn_id, num_dims, vect_dims );

    testing_log << "\n";
    return rc;
}


static int test_activate_var_attrs ( uint64_t type_id, uint64_t var_id, 
                                md_catalog_type_entry type, md_catalog_var_entry var, int num_dims, vector<md_dim_bounds> vect_dims ) {

    int rc;

    uint64_t new_attr_id;

    uint64_t new_txn_id = var.txn_id;
    var.txn_id -= 100;

    // uint64_t new_txn_id = var.txn_id;
    // uint64_t old_txn_id = var.txn_id - 100;

    md_catalog_var_attribute_entry new_attr;
    new_attr.timestep_id = var.timestep_id; //FIX 
    new_attr.type_id = type_id;
    new_attr.var_id = var_id;
    new_attr.txn_id = new_txn_id;
    new_attr.dims = vect_dims;
    new_attr.num_dims = num_dims;

    string str("act_\0newtest",9);
    make_attr_data(str, 500, new_attr.data);
    // new_attr.data = "500";
    // new_attr.data = val;
    uint64_t val = 500;
    // make_single_val_data (val, new_attr.data);
    new_attr.data_type = ATTR_DATA_TYPE_BLOB;

    rc = metadata_insert_var_attribute_by_dims (new_attr_id, new_attr);
    if (rc != RC_OK) {
        error_log << "Error. Was unable to insert the new var attribute. Proceeding" << endl;
    }
    md_log( "New var attribute id is " + to_string(new_attr_id) );
    
    // if(my_server_type  != SERVER_LOCAL_SQLITE_TRANSACTION_MANAGEMENT_WAL) {

        rc = metadata_activate_var_attribute(new_txn_id);
        if (rc == RC_OK) {
            testing_log << "just activated the new var attribute \n";
        }
        else {
            error_log << "Error. Was unable to activate the new attribute. Exiting \n";
            return RC_ERR;
        }


        rc = catalog_all_var_attributes ( var.run_id, var.timestep_id, var.txn_id, num_dims, vect_dims );

        rc = catalog_all_var_attributes_with_type (var.timestep_id, num_dims, vect_dims, type, type_id );

        rc = catalog_all_var_attributes_with_var ( var, var_id, num_dims, vect_dims );

        rc = catalog_all_var_attributes_with_type_var ( type, var, type_id, var_id, num_dims, vect_dims );


        rc = metadata_processing_var_attribute(new_txn_id);

        if (rc == RC_OK) {
            testing_log << "just deactivated the new var attribute \n";
        }
        else {
            error_log << "Error. Was unable to deactivate the new var attribute. Exiting \n";
            return RC_ERR;
        }
    // }
      
   
    rc = catalog_all_var_attributes ( var.run_id, var.timestep_id, var.txn_id, num_dims, vect_dims );

    rc = catalog_all_var_attributes_with_type (var.timestep_id, num_dims, vect_dims, type, type_id );


    rc = catalog_all_var_attributes_with_var ( var, var_id, num_dims, vect_dims );

    rc = catalog_all_var_attributes_with_type_var ( type, var, type_id, var_id, num_dims, vect_dims );
    
    testing_log << "\n";
    return rc;
}


int delete_run (uint64_t run_id, uint64_t timestep_id, uint64_t txn_id ) {

    int rc;

    uint32_t count;
    vector<md_catalog_run_entry> run_entries;
    vector<md_catalog_timestep_entry> timestep_entries;
    vector<md_catalog_var_entry> var_entries;
    vector<md_catalog_type_entry> type_entries;
    vector<md_catalog_run_attribute_entry> run_attr_entries;
    vector<md_catalog_timestep_attribute_entry> timestep_attr_entries;
    vector<md_catalog_var_attribute_entry> var_attr_entries;

    // testing_log << "deleting run: " << run_id << " name: " << run0_name <<  endl;
    testing_log << "deleting run: " << run_id << endl;

    // rc = metadata_delete_run_by_name_path_timestep (run0_name, run0_timestep);
    rc = metadata_delete_run_by_id (run_id);

    //Should be empty now 
    rc = metadata_catalog_run (txn_id, count, run_entries);
    if (rc != RC_OK) {
        error_log << "Error cataloging the post deletion set of run entries. Proceeding" << endl;
    }
    testing_log << "run catalog for txn_id " << txn_id << ": \n";
    if(testing_logging || (zero_rank_logging )) {
        print_run_catalog (count, run_entries);
    }

    //Should be empty now 
    rc = metadata_catalog_timestep (run_id, txn_id, count, timestep_entries);
    if (rc != RC_OK) {
        error_log << "Error cataloging the post deletion set of timestep entries. Proceeding" << endl;
    }
    testing_log << "timestep catalog for run_id " << run_id << " and txn_id " << txn_id << ": \n";
    if(testing_logging || (zero_rank_logging )) {
        print_timestep_catalog (count, timestep_entries);
    }

    //Should be empty now 
    rc = metadata_catalog_var(run_id, timestep_id, txn_id, count, var_entries);
    if (rc != RC_OK) {
        error_log << "Error cataloging the post deletion set of var_entries. Proceeding" << endl;
    }
    testing_log << "var catalog for run_id " << run_id << " and timestep_id " << timestep_id << " and txn_id " << txn_id << ": \n";
    if(testing_logging || (zero_rank_logging )) {
        print_var_catalog (count, var_entries);
    }

    //Should be empty now 
    rc = metadata_catalog_type (run_id, txn_id, count, type_entries);
    if (rc == RC_OK) {

        testing_log << "new type catalog for run_id " << run_id << " and txn_id " << txn_id << ": \n";
        if(testing_logging || (zero_rank_logging )) {
            print_type_catalog (count, type_entries);
        }
    }
    else {
        error_log << "Error cataloging the new set of type entries. Proceeding" << endl;
    }

    //Should be empty now 
    rc = metadata_catalog_all_run_attributes ( txn_id, count, run_attr_entries );
    if (rc == RC_OK) {
        testing_log << "run attributes associated with txn_id: " << txn_id;
        testing_log << "\n";
        if(testing_logging || (zero_rank_logging )) {
            print_run_attribute_list (count, run_attr_entries);
        }
    }
    else {
        error_log << "Error metadata_catalog_all_run_attributes. Proceeding" << endl;
    }
    rc = metadata_catalog_all_run_attributes_in_run ( run_id, txn_id, count, run_attr_entries );
    if (rc == RC_OK) {
        testing_log << "run attributes associated with run_id " << run_id << " and txn_id: " << txn_id;
        testing_log << "\n";
        if(testing_logging || (zero_rank_logging )) {
            print_run_attribute_list (count, run_attr_entries);
        }
    }
    else {
        error_log << "Error metadata_catalog_all_run_attributes_in_run. Proceeding" << endl;
    }

    //Should be empty now 
    rc = metadata_catalog_all_timestep_attributes ( run_id, txn_id, count, timestep_attr_entries );
    if (rc == RC_OK) {
        testing_log << "timestep attributes associated with run_id " << run_id << " and txn_id: " << txn_id;
        testing_log << "\n";
        if(testing_logging || (zero_rank_logging )) {
            print_timestep_attribute_list (count, timestep_attr_entries);
        }
    }
    else {
        error_log << "Error metadata_catalog_all_timestep_attributes. Proceeding" << endl;
    }
    rc = metadata_catalog_all_timestep_attributes_in_timestep ( run_id, timestep_id, txn_id, count, timestep_attr_entries );
    if (rc == RC_OK) {
        testing_log << "timestep attributes associated with run_id " << run_id << " and timestep_id " << timestep_id << " and txn_id: " << txn_id;
        testing_log << "\n";
        if(testing_logging || (zero_rank_logging )) {
            print_timestep_attribute_list (count, timestep_attr_entries);
        }
    }
    else {
        error_log << "Error metadata_catalog_all_timestep_attributes_in_timestep. Proceeding" << endl;
    }

    //Should be empty now 
    rc = metadata_catalog_all_var_attributes (run_id, timestep_id, txn_id, count, var_attr_entries);
    if (rc == RC_OK) {
        testing_log << "var attributes associated with run_id " << run_id << " and timestep_id " << timestep_id << " and txn_id: " << txn_id;
        testing_log << "\n";
        if(testing_logging || (zero_rank_logging )) {
            print_var_attribute_list (count, var_attr_entries);
        }
    }
    else {
        error_log << "Error metadata_catalog_all_var_attributes. Proceeding" << endl;
    }
    testing_log << "\n";
    return rc;
}

int create_run(md_catalog_run_entry run, string rank_to_dims_funct_name, string rank_to_dims_funct_path, 
                string objector_funct_name, string objector_funct_path, uint64_t &run_id,
                uint64_t &type0_id, uint64_t &type1_id, md_catalog_type_entry &new_type, md_catalog_type_entry &new_type2 ) 
{    
    int rc;
    bool insert_types_in_batch = true;

    vector<md_catalog_type_entry> all_types;
    // vector<uint64_t> all_type_ids;

    rc = metadata_create_run (run_id, run.job_id, run.name, run.txn_id, run.npx, run.npy, run.npz,  
                rank_to_dims_funct_name, rank_to_dims_funct_path, objector_funct_name, objector_funct_path);
    if (rc != RC_OK) {
        error_log << "Error. Was unable to create the first run. Exiting \n";
        return RC_ERR;
    }
    md_log( "First run id is " + to_string(run_id ) );


    uint32_t type0_version = 1;
    uint32_t type1_version = 2;
    string type0_name = "typey1";
    string type1_name = "typey2";

    new_type.run_id = run_id;
    new_type.name = type0_name;
    new_type.version = type0_version;
    new_type.txn_id = run.txn_id;


    if(insert_types_in_batch) {
    	all_types.push_back(new_type);
    	// type0_id = 1;
   	}
   	else {
	    rc = metadata_create_type (type0_id, new_type);
	    if (rc != RC_OK) {
	        error_log << "Error. Was unable to insert the first type. Proceeding" << endl;
	    }
	    md_log( "First type id is " + to_string(type0_id ) );
	}

    new_type2.run_id = run_id;
    new_type2.name = type1_name;
    new_type2.version = type1_version;
    new_type2.txn_id = run.txn_id;

    if(insert_types_in_batch) {
    	all_types.push_back(new_type2);
    	// type1_id = 2;

    	// rc = metadata_create_type_batch (all_types);
    	// rc = metadata_create_type_batch (all_type_ids, all_types);
    	rc = metadata_create_type_batch (type0_id, all_types);
	    if (rc != RC_OK) {
	        error_log << "Error. Was unable to insert the types by batch. Proceeding" << endl;
	    }
	    // type0_id = all_type_ids.at(0);
	    type1_id = type0_id + 1;
	    debug_log << "type0_id: " << type0_id << endl;
	    debug_log << "type1_id: " << type1_id << endl;

    }
    else {
	    rc = metadata_create_type (type1_id, new_type2);
	    if (rc != RC_OK) {
	        error_log << "Error. Was unable to insert the second type. Proceeding" << endl;
	    }
	    md_log( "Second type id is " + to_string(type1_id ) );
	}
    return rc;
}

int create_timestep(uint64_t run_id, uint64_t timestep_id, string timestep_path, uint64_t txn_id,
                    uint64_t type0_id, uint64_t type1_id,  uint64_t &var0_id, uint64_t &var1_id, 
                    md_catalog_var_entry &new_var, md_catalog_var_entry &new_var2, md_catalog_var_attribute_entry &new_attr )   
{
    int rc;

    bool insert_attrs_in_batch = true;
    bool insert_vars_in_batch = true;

    uint32_t var0_version = 0;
    uint32_t var1_version = 1;

    uint64_t attribute0_id;
    uint64_t attribute1_id;
    uint64_t attribute2_id;
    uint64_t attribute3_id;
    uint64_t attribute4_id;

    std::string var0_name = "var0";
    std::string var0_path = "/var0";
    std::string var1_name = "var1";
    std::string var1_path = "/var1";
    // std::string var0_name = "VAR0";
    // std::string var0_path = "/VAR0";
    // std::string var1_name = "VAR1";
    // std::string var1_path = "/VAR1";

    char datasize0 = 4;
    char datasize1 = 8;
    
    var0_id = 0;
    var1_id = 1;

	vector<md_catalog_var_entry> all_vars_to_insert;


    rc = metadata_create_timestep (timestep_id, run_id, timestep_path, txn_id);
    if (rc != RC_OK) {
        error_log << "Error. Was unable to create the first timestep. Exiting \n";
        return RC_ERR;
    }
    md_log( "First timestep id is " + to_string(timestep_id ) );

    new_var.txn_id = txn_id;
    new_var.run_id = run_id;
    new_var.timestep_id = timestep_id;
    new_var.name = var0_name;
    new_var.path = var0_path;
    new_var.version = var0_version;
    new_var.data_size = datasize0;
    new_var.num_dims = 2;
    new_var.txn_id = txn_id;
    new_var.var_id = var0_id;
    md_dim_bounds dims[3];

    dims [0].min = 0;
    dims [0].max = 9;
    dims [1].min = 0;
    dims [1].max = 9;
    new_var.dims = std::vector<md_dim_bounds>(dims, dims + new_var.num_dims );

    if(insert_vars_in_batch) {
    	all_vars_to_insert.push_back(new_var);
    }
    else {
	    rc = metadata_create_var (var0_id, new_var);

	    if (rc != RC_OK) {
	        error_log << "Error. Was unable to create the first var. Exiting \n";
	        return RC_ERR;
	    }
	    md_log( "First var id is " + to_string(var0_id ) );
    }

   //do for each var
    rc = write_output(data_outputs, new_run, new_var); 
    if (rc != RC_OK) {
        error_log << "Error. Was unable to write output for the first var. Exiting \n";
        return RC_ERR;
    }

    //creates a variable, assigning it a location in the simulation space
    new_var2.txn_id = txn_id;
    new_var2.run_id = run_id;
    new_var2.timestep_id = timestep_id;
    new_var2.txn_id = txn_id;
    new_var2.var_id = var1_id;
    new_var2.name = var1_name;
    new_var2.path = var1_path;
    new_var2.version = var1_version;
    new_var2.data_size = datasize1;
    new_var2.num_dims = 3;
    dims [0]. min = 0;
    dims [0]. max = 99;
    dims [1]. min = 0;
    dims [1]. max = 99;
    dims [2]. min = 0;
    dims [2]. max = 99;
    new_var2.dims = std::vector<md_dim_bounds>(dims, dims + new_var2.num_dims );
    extreme_debug_log << "about to create var \n";


    if(insert_vars_in_batch) {
    	all_vars_to_insert.push_back(new_var2);


	    rc = metadata_create_var_batch (all_vars_to_insert);
	    if (rc != RC_OK) {
	        error_log << "Error. Was unable to create batch vars. Exiting \n";
	        return RC_ERR;
	    }
    }
    else {
	    rc = metadata_create_var (var1_id, new_var2);
	    if (rc != RC_OK) {
	        error_log << "Error. Was unable to create the second var. Exiting \n";
	        return RC_ERR;
	    }
	    md_log( "Second var id is " + to_string(var1_id ) );
	}

   //do for each var
    rc = write_output(data_outputs, new_run, new_var2); 
    if (rc != RC_OK) {
        error_log << "Error. Was unable to write output for the second var. Exiting \n";
        return RC_ERR;
    }

//-------------------------------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------------------------------
//TYPE AND ATTRIBUTE SETUP

	vector<md_catalog_var_attribute_entry> all_attrs_to_insert;

    new_attr.timestep_id = timestep_id;
    new_attr.type_id = type0_id;
    new_attr.var_id = var0_id;
    new_attr.txn_id = txn_id;
    new_attr.num_dims = 2;

    dims [0].min = 1;
    dims [0].max = 5;
    dims [1].min = 2;
    dims [1].max = 5;
    // dims [2].min = 3;
    // dims [2].max = 10;
    new_attr.dims = std::vector<md_dim_bounds>(dims, dims + new_attr.num_dims );

    string str("test\0new0",9);
    // cout << "before make attr, str.size(): " << str.size() << endl;
    make_attr_data(str, 9, new_attr.data);
    // new_attr.data = "9";

    uint64_t val = 9;
    // make_single_val_data (val, new_attr.data);
    new_attr.data_type = ATTR_DATA_TYPE_BLOB;


    if(insert_attrs_in_batch) {
    	all_attrs_to_insert.push_back(new_attr);
    }
    else {
	    rc = metadata_insert_var_attribute_by_dims(attribute0_id, new_attr);
	    if (rc != RC_OK) {
	        error_log << "Error. Was unable to insert the first new_attr. Proceeding" << endl;
	    }
	    md_log( "First new_attr id is " + to_string(attribute0_id ) );
	}

    new_attr.type_id = type1_id;

    // dims [0].min = 11; 
    // dims [0].max = 20;
    // dims [1].min = 4;
    // dims [1].max = 10;

    dims [0].min = 3; 
    dims [0].max = 9;
    dims [1].min = 4;
    dims [1].max = 9;
    // dims [2].min = 5;
    // dims [2].max = 10;
    new_attr.dims = std::vector<md_dim_bounds>(dims, dims + new_attr.num_dims );

    string str2("test\0new1",9);
    make_attr_data(str2, 10, new_attr.data);
    // new_attr.data = "10";
    val = 10;
    // make_single_val_data (val, new_attr.data);

    if(insert_attrs_in_batch) {
    	all_attrs_to_insert.push_back(new_attr);
    }
    else {
	    rc = metadata_insert_var_attribute_by_dims(attribute1_id, new_attr);
	    if (rc != RC_OK) {
	        error_log << "Error. Was unable to insert the second new_attr. Proceeding" << endl;
	    }
	    md_log( "Second new_attr id is " + to_string(attribute1_id ) );
	}

    new_attr.type_id = type0_id;
    new_attr.var_id = var1_id;
    new_attr.num_dims = 3;

    dims [0].min = 0;
    dims [0].max = 9;
    dims [1].min = 0;
    dims [1].max = 9;
    dims [2].min = 0;
    dims [2].max = 9;
    new_attr.dims = std::vector<md_dim_bounds>(dims, dims + new_attr.num_dims );

    string str3("test\0new2",9);
    make_attr_data(str3, 11, new_attr.data);
    // new_attr.data = "11";
    val = 11;
    // make_single_val_data (val, new_attr.data);

    if (insert_attrs_in_batch) {
    	all_attrs_to_insert.push_back(new_attr);
    }
    else {
	    rc = metadata_insert_var_attribute_by_dims(attribute2_id, new_attr);
	    if (rc != RC_OK) {
	        error_log << "Error. Was unable to insert the third new_attr. Proceeding" << endl;
	    }
	    md_log( "Third new_attr id is " + to_string( attribute2_id) );
	}


    new_attr.type_id = type1_id;

    dims [0].min = 10;
    dims [0].max = 19;
    dims [1].min = 10;
    dims [1].max = 19;
    dims [2].min = 10;
    dims [2].max = 19;
    new_attr.dims = std::vector<md_dim_bounds>(dims, dims + new_attr.num_dims );

    string str4("test\0new3",9);
    make_attr_data(str4, 12, new_attr.data);
    // new_attr.data = "12";
    val = 12;
    // make_single_val_data (val, new_attr.data);

    if(insert_attrs_in_batch) {
    	all_attrs_to_insert.push_back(new_attr);
    }
    else {
	    rc = metadata_insert_var_attribute_by_dims(attribute3_id, new_attr);
	    if (rc != RC_OK) {
	        error_log << "Error. Was unable to insert the Fourth new_attr. Proceeding" << endl;
	    }
	    md_log( "Fourth new_attr id is " + to_string(attribute3_id) );
	}

    dims [0].min = 20;
    dims [0].max = 29;
    dims [1].min = 20;
    dims [1].max = 29;
    dims [2].min = 20;
    dims [2].max = 29;
    new_attr.dims = std::vector<md_dim_bounds>(dims, dims + new_attr.num_dims );

    string str5("test\0new4",9);
    make_attr_data(str5, 13, new_attr.data);
    // new_attr.data = "13";
    val = 13;
    // make_single_val_data (val, new_attr.data);

    if(insert_attrs_in_batch) {
    	all_attrs_to_insert.push_back(new_attr);

       	cout << "about to insert all attrs of size: " << all_attrs_to_insert.size() << " for run: " <<
       		run_id << " and timestep: " << timestep_id << endl;
        rc = metadata_insert_var_attribute_by_dims_batch (all_attrs_to_insert);
        if (rc != RC_OK) {
            error_log << "Error. Was unable to insert the batch attributes. Proceeding" << endl;
        }
    }
    else {
	    rc = metadata_insert_var_attribute_by_dims(attribute4_id, new_attr);
	    if (rc != RC_OK) {
	        error_log << "Error. Was unable to insert the fifth new_attr. Proceeding" << endl;
	    }

	    md_log( "Fifth new_attr id is " + to_string(attribute4_id) );
	}


    return rc;
}
