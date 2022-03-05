#include <my_metadata_client_sqlite_transactions_only_multiple_databases_read_and_write_new.hh>

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


// int retrieveObjNamesAndDataForAttrCatalog(std::map <string, vector<double>> data_outputs, md_catalog_run_entry run, uint64_t run_id, uint64_t timestep_id,
//                                          uint64_t txn_id,
//                                          vector<md_catalog_var_attribute_entry> attr_entries );



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

string to_upper (string val);


int catalog_all_types ( md_server server, md_catalog_var_entry var, uint64_t var_id,
                    int num_dims, vector<md_dim_bounds> vect_dims ) {

    int rc;
    uint32_t count;
    vector<md_catalog_type_entry> type_entries;


    rc = metadata_catalog_all_types_with_var_attributes (server, var.run_id, var.txn_id, count, type_entries);
    if (rc == RC_OK) {
        testing_log << "using metadata_catalog_all_types_with_var_attributes: types associated with run_id " << var.run_id <<
            " and txn_id: " << var.txn_id << "\n";
        if(testing_logging || (zero_rank_logging )) {
            print_type_catalog (count, type_entries);
        }
    }
    else {
        error_log << "Error metadata_catalog_all_types_with_var_attributes. Proceeding" << endl;
    }

    rc = metadata_catalog_all_types_with_var_attributes_in_timestep (server, var.run_id, var.timestep_id, var.txn_id, count, type_entries);
    if (rc == RC_OK) {
        testing_log << "using metadata_catalog_all_types_with_var_attributes_in_timestep: types associated with run_id " << var.run_id << ", timestep_id " << var.timestep_id << 
            " and txn_id: " << var.txn_id << "\n";
        if(testing_logging || (zero_rank_logging )) {
            print_type_catalog (count, type_entries);
        }
    }
    else {
        error_log << "Error metadata_catalog_all_types_with_var_attributes_in_timestep. Proceeding" << endl;
    }


    rc = metadata_catalog_all_types_with_var_attributes_with_var (server, var.run_id, var_id, var.txn_id, count, type_entries);
    if (rc == RC_OK) {
        testing_log << "using metadata_catalog_all_types_with_var_attributes_with_var: types associated with run_id " << var.run_id << 
           ", var_id: " << var_id << " and txn_id: " << var.txn_id << "\n";
        if(testing_logging || (zero_rank_logging )) {
            print_type_catalog (count, type_entries);
        }
    }
    else {
        error_log << "Error metadata_catalog_all_types_with_var_attributes_with_var. Proceeding" << endl;
    }

    rc = metadata_catalog_all_types_with_var_attributes_with_var_in_timestep (server, var.run_id, var.timestep_id, var_id, var.txn_id, count, type_entries);
    if (rc == RC_OK) {
        testing_log << "using metadata_catalog_all_types_with_var_attributes_with_var_in_timestep: types associated with run_id " << var.run_id << ", timestep_id " << var.timestep_id << 
           ", var_id: " << var_id << " and txn_id: " << var.txn_id << "\n";
        if(testing_logging || (zero_rank_logging )) {
            print_type_catalog (count, type_entries);
        }
    }
    else {
        error_log << "Error metadata_catalog_all_types_with_var_attributes_with_var_in_timestep. Proceeding" << endl;
    }


    rc = metadata_catalog_all_types_with_var_attributes_with_var_dims (server, var.run_id, var_id, var.txn_id, num_dims, vect_dims, count, type_entries);
    if (rc == RC_OK) {
       testing_log << "using metadata_catalog_all_types_with_var_attributes_with_var_dims: types associated with run_id " << var.run_id <<
            ", var_id: " << var_id << " and txn_id: " << var.txn_id << " overlapping with dims";
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
        error_log << "Error metadata_catalog_all_types_with_var_attributes_with_var_dims. Proceeding" << endl;
    }

    rc = metadata_catalog_all_types_with_var_attributes_with_var_dims_in_timestep (server, var.run_id, var.timestep_id, var_id, var.txn_id, num_dims, vect_dims, count, type_entries);
    if (rc == RC_OK) {
       testing_log << "using metadata_catalog_all_types_with_var_attributes_with_var_dims_in_timestep: types associated with run_id " << var.run_id << ", timestep_id " << var.timestep_id << 
            ", var_id: " << var_id << " and txn_id: " << var.txn_id << " overlapping with dims";
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
        error_log << "Error metadata_catalog_all_types_with_var_attributes_with_var_dims_in_timestep. Proceeding" << endl;
    }


    return rc;
}




int catalog_all_timesteps ( md_server server, md_catalog_var_entry var, uint64_t var_id, uint64_t type_id,
                    int num_dims, vector<md_dim_bounds> vect_dims ) {

    int rc;
    uint32_t count;
    vector<md_catalog_timestep_entry> timestep_entries;


    //Should be empty now 
    rc = metadata_catalog_timestep (server, var.run_id, var.txn_id, count, timestep_entries);
    if (rc == RC_OK) {
        testing_log << "timestep catalog for run_id " << var.run_id << " and txn_id " << var.txn_id << ": \n";
        if(testing_logging || (zero_rank_logging )) {
            print_timestep_catalog (count, timestep_entries);
        }
    }
    else {
        error_log << "Error metadata_catalog_timestep. Proceeding" << endl;
    }



    rc = metadata_catalog_all_timesteps_with_var (server, var.run_id, var_id, var.txn_id, count, timestep_entries);

    if (rc == RC_OK) {
        testing_log << "using metadata_catalog_all_timesteps_with_var funct: timesteps associated with run_id " << var.run_id << ", var_id " << var_id << 
            " and txn_id: " << var.txn_id << "\n";
        if(testing_logging || (zero_rank_logging )) {
            print_timestep_catalog (count, timestep_entries);
        }
    }
    else {
        error_log << "Error metadata_catalog_all_timesteps_with_var. Proceeding" << endl;
    }


    rc = metadata_catalog_all_timesteps_with_var_attributes_with_type_var (server, var.run_id, type_id, var_id, var.txn_id, count, timestep_entries);

    if (rc == RC_OK) {
        testing_log << "using metadata_catalog_all_timesteps_with_var_attributes_with_type_var funct: timesteps associated with run_id " << var.run_id << ", type_id " << type_id << 
        ", var_id " << var_id << " and txn_id: " << var.txn_id << "\n";
        if(testing_logging || (zero_rank_logging )) {
            print_timestep_catalog (count, timestep_entries);
        }
    }
    else {
        error_log << "Error metadata_catalog_all_timesteps_with_var_attributes_with_type_var. Proceeding" << endl;
    }


    // string range_data0;
    // uint64_t min_range = 5;
    // uint64_t max_range = 10;
    // make_range_data (min_range, max_range, range_data0);

    // rc = metadata_catalog_all_timesteps_with_var_attributes_with_type_var_range (server, var.run_id, type_id, var_id, var.txn_id, 
    //     ATTR_DATA_TYPE_BLOB, range_data0, count, timestep_entries);

    // if (rc == RC_OK) {
    //     testing_log << "using metadata_catalog_all_timesteps_with_var_attributes_with_type_var_range funct: timesteps associated with run_id " << var.run_id << ", type_id " << type_id << 
    //     ", var_id " << var_id << " txn_id: " << var.txn_id << " int attr data and range [" << min_range << "," << max_range << "]\n";
    //     if(testing_logging || (zero_rank_logging )) {
    //         print_timestep_catalog (count, timestep_entries);
    //     }
    // }
    // else {
    //     error_log << "Error metadata_catalog_all_timesteps_with_var_attributes_with_type_var_range. Proceeding" << endl;
    // }

    // uint64_t val = 12;
    // string range_data1;
    // // make_single_val_data (val, range_data1);

    // rc = metadata_catalog_all_timesteps_with_var_attributes_with_type_var_above_max (server, var.run_id, type_id, var_id, var.txn_id, 
    //     ATTR_DATA_TYPE_BLOB, range_data1, count, timestep_entries);

    // if (rc == RC_OK) {
    //     testing_log << "using metadata_catalog_all_timesteps_with_var_attributes_with_type_var_range funct: timesteps associated with run_id " << var.run_id << ", type_id " << type_id << 
    //     ", var_id " << var_id << " txn_id: " << var.txn_id << " int attr data greater than or eq to " << val << "]\n";
    //     if(testing_logging || (zero_rank_logging )) {
    //         print_timestep_catalog (count, timestep_entries);
    //     }
    // }
    // else {
    //     error_log << "Error metadata_catalog_all_timesteps_with_var_attributes_with_type_var_above_max. Proceeding" << endl;
    // }

    // uint64_t val2 = 11;
    // string range_data2;
    // // make_single_val_data (val2, range_data2);

    // rc = metadata_catalog_all_timesteps_with_var_attributes_with_type_var_below_min (server, var.run_id, type_id, var_id, var.txn_id, 
    //     ATTR_DATA_TYPE_BLOB, range_data2, count, timestep_entries);

    // if (rc == RC_OK) {
    //     testing_log << "using metadata_catalog_all_timesteps_with_var_attributes_with_type_var_range funct: timesteps associated with run_id " << var.run_id << ", type_id " << type_id << 
    //     ", var_id " << var_id << " txn_id: " << var.txn_id << " int attr data less than or eq to " << val2 << "]\n";
    //     if(testing_logging || (zero_rank_logging )) {
    //         print_timestep_catalog (count, timestep_entries);
    //     }
    // }
    // else {
    //     error_log << "Error metadata_catalog_all_timesteps_with_var_attributes_with_type_var_below_min. Proceeding" << endl;
    // }

    rc = metadata_catalog_all_timesteps_with_var_attributes_with_type_var_dims (server, var.run_id, type_id, var_id, var.txn_id, num_dims, vect_dims, count, timestep_entries);

    if (rc == RC_OK) {
        testing_log << "using metadata_catalog_all_timesteps_with_var_attributes_with_type_var_dims funct: timesteps associated with run_id " << var.run_id << ", type_id " << type_id << 
        ", var_id " << var_id << " and txn_id: " << var.txn_id << " overlapping with dims";
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
        error_log << "Error metadata_catalog_all_timesteps_with_var_attributes_with_type_var_dims. Proceeding" << endl;
    }

    // rc = metadata_catalog_all_timesteps_with_var_attributes_with_type_var_dims_range (server, var.run_id, type_id, var_id, var.txn_id, num_dims, vect_dims, 
    //     ATTR_DATA_TYPE_BLOB, range_data0, count, timestep_entries);

    // if (rc == RC_OK) {
    //     testing_log << "using metadata_catalog_all_timesteps_with_var_attributes_with_type_var_dims funct: timesteps associated with run_id " << var.run_id << ", type_id " << type_id << 
    //     ", var_id " << var_id << " and txn_id: " << var.txn_id << " int attr data and range [" << min_range << "," << max_range << "] overlapping with dims";
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
    //     error_log << "Error metadata_catalog_all_timesteps_with_var_attributes_with_type_var_dims_range. Proceeding" << endl;
    // }

    // rc = metadata_catalog_all_timesteps_with_var_attributes_with_type_var_dims_above_max (server, var.run_id, type_id, var_id, var.txn_id, num_dims, vect_dims, 
    //     ATTR_DATA_TYPE_BLOB, range_data1, count, timestep_entries);

    // if (rc == RC_OK) {
    //     testing_log << "using metadata_catalog_all_timesteps_with_var_attributes_with_type_var_dims funct: timesteps associated with run_id " << var.run_id << ", type_id " << type_id << 
    //     ", var_id " << var_id << " and txn_id: " << var.txn_id << " int attr data greater than or eq to " << val << " overlapping with dims";
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
    //     error_log << "Error metadata_catalog_all_timesteps_with_var_attributes_with_type_var_dims_above_max. Proceeding" << endl;
    // }

    // rc = metadata_catalog_all_timesteps_with_var_attributes_with_type_var_dims_below_min (server, var.run_id, type_id, var_id, var.txn_id, num_dims, vect_dims, 
    //     ATTR_DATA_TYPE_BLOB, range_data2, count, timestep_entries);

    // if (rc == RC_OK) {
    //     testing_log << "using metadata_catalog_all_timesteps_with_var_attributes_with_type_var_dims funct: timesteps associated with run_id " << var.run_id << ", type_id " << type_id << 
    //     ", var_id " << var_id << " and txn_id: " << var.txn_id << " int attr data less than or eq to " << val2 << " overlapping with dims";
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
    //     error_log << "Error metadata_catalog_all_timesteps_with_var_attributes_with_type_var_dims_below_min. Proceeding" << endl;
    // }

    return rc;
}


int catalog_all_timestep_attributes ( md_server server, uint64_t run_id, uint64_t timestep_id, uint64_t type_id, uint64_t txn_id) {

    int rc;
    uint32_t count;
    vector<md_catalog_timestep_attribute_entry> attr_entries;

    rc = metadata_catalog_all_timestep_attributes (server, run_id, txn_id, count, attr_entries);
    if (rc == RC_OK) {
        testing_log << "using metadata_catalog_all_timestep_attributes funct: timestep attrs associated with run_id " << run_id << 
        " and txn_id: " << txn_id << "\n";
        if(testing_logging || (zero_rank_logging )) {
            print_timestep_attribute_list (count, attr_entries);
        }
        // print_timestep_attr_data(count, attr_entries);
    }
    else {
        error_log << "Error metadata_catalog_all_timestep_attributes. Proceeding" << endl;
    }

    rc = metadata_catalog_all_timestep_attributes_in_timestep (server, run_id, timestep_id, txn_id, count, attr_entries);
    if (rc == RC_OK) {
        testing_log << "using metadata_catalog_all_timestep_attributes_in_timestep funct: timestep attrs associated with run_id " << run_id << 
        " timestep_id " << timestep_id << " and txn_id: " << txn_id << "\n";
        if(testing_logging || (zero_rank_logging )) {
            print_timestep_attribute_list (count, attr_entries);
        }
        // print_timestep_attr_data(count, attr_entries);
    }
    else {
        error_log << "Error metadata_catalog_all_timestep_attributes_in_timestep. Proceeding" << endl;
    }

    rc = metadata_catalog_all_timestep_attributes_with_type (server, run_id, type_id, txn_id, count, attr_entries);
    if (rc == RC_OK) {
        testing_log << "using metadata_catalog_all_timestep_attributes_with_type funct: timestep attrs associated with run_id " << run_id << 
        " type_id " << type_id << " and txn_id: " << txn_id << "\n";
        if(testing_logging || (zero_rank_logging )) {
            print_timestep_attribute_list (count, attr_entries);
        }
        // print_timestep_attr_data(count, attr_entries);
    }
    else {
        error_log << "Error metadata_catalog_all_timestep_attributes_with_type. Proceeding" << endl;
    }

    rc = metadata_catalog_all_timestep_attributes_with_type_in_timestep (server, run_id, timestep_id, type_id, txn_id, count, attr_entries);

    if (rc == RC_OK) {
        testing_log << "using metadata_catalog_all_timestep_attributes_with_type_in_timestep funct: timestep attrs associated with run_id " << run_id << 
        " timestep_id " << timestep_id << " type_id " << type_id << " and txn_id: " << txn_id << "\n";
        if(testing_logging || (zero_rank_logging )) {
            print_timestep_attribute_list (count, attr_entries);
        }
        // print_timestep_attr_data(count, attr_entries);
    }
    else {
        error_log << "Error metadata_catalog_all_timestep_attributes_with_type_in_timestep. Proceeding" << endl;
    }


    // string range_data;
    // uint64_t double min_range = 5;
    // uint64_t max_range = 10;
    // make_range_data (min_range, max_range, range_data);

    // rc = metadata_catalog_all_timestep_attributes_with_type_range (server, run_id, type_id, txn_id, ATTR_DATA_TYPE_BLOB, range_data, count, attr_entries);
    // if (rc == RC_OK) {
    //     testing_log << "using metadata_catalog_all_timestep_attributes_with_type_range funct: timestep attrs associated with run_id " << run_id << 
    //     " type_id " << type_id << " and txn_id: " << txn_id << " int attr data and range [" << min_range << "," << max_range << "]\n";
    //     if(testing_logging || (zero_rank_logging )) {
    //         print_timestep_attribute_list (count, attr_entries);
    //     }
    //     print_timestep_attr_data(count, attr_entries);
    // }
    // else {
    //     error_log << "Error metadata_catalog_all_timestep_attributes_with_type_range. Proceeding" << endl;
    // }

    // rc = metadata_catalog_all_timestep_attributes_with_type_range_in_timestep (server, run_id, timestep_id, type_id, txn_id, ATTR_DATA_TYPE_BLOB, range_data, count, attr_entries);
    // if (rc == RC_OK) {
    //     testing_log << "using metadata_catalog_all_timestep_attributes_with_type_range_in_timestep funct: timestep attrs associated with run_id " << run_id << 
    //     " timestep_id " << timestep_id << " type_id " << type_id << " and txn_id: " << txn_id << " int attr data and range [" << min_range << "," << max_range << "]\n";
    //     if(testing_logging || (zero_rank_logging )) {
    //         print_timestep_attribute_list (count, attr_entries);
    //     }
    //     print_timestep_attr_data(count, attr_entries);
    // }
    // else {
    //     error_log << "Error metadata_catalog_all_timestep_attributes_with_type_range_in_timestep. Proceeding" << endl;
    // }

    // uint64_t val = 12;
    // string range_data1;
    // // make_single_val_data (val, range_data1);

    // rc = metadata_catalog_all_timestep_attributes_with_type_above_max (server, run_id, type_id, txn_id, ATTR_DATA_TYPE_BLOB, range_data1, count, attr_entries);
    // if (rc == RC_OK) {
    //     testing_log << "using metadata_catalog_all_timestep_attributes_with_type_above_max funct: timestep attrs associated with run_id " << run_id << 
    //     " type_id " << type_id << " and txn_id: " << txn_id << " int attr data greater than or eq to " << val << endl;
    //     if(testing_logging || (zero_rank_logging )) {
    //         print_timestep_attribute_list (count, attr_entries);
    //     }
    //     print_timestep_attr_data(count, attr_entries);
    // }
    // else {
    //     error_log << "Error metadata_catalog_all_timestep_attributes_with_type_above_max. Proceeding" << endl;
    // }

    // rc = metadata_catalog_all_timestep_attributes_with_type_above_max_in_timestep (server, run_id, timestep_id, type_id, txn_id, ATTR_DATA_TYPE_BLOB, range_data1, count, attr_entries);
    // if (rc == RC_OK) {
    //     testing_log << "using metadata_catalog_all_timestep_attributes_with_type_above_max_in_timestep funct: timestep attrs associated with run_id " << run_id << 
    //     " timestep_id " << timestep_id << " type_id " << type_id << " and txn_id: " << txn_id << " int attr data greater than or eq to " << val << endl;
    //     if(testing_logging || (zero_rank_logging )) {
    //         print_timestep_attribute_list (count, attr_entries);
    //     }
    //     print_timestep_attr_data(count, attr_entries);
    // }
    // else {
    //     error_log << "Error metadata_catalog_all_timestep_attributes_with_type_above_max_in_timestep. Proceeding" << endl;
    // }

    // uint64_t val2 = 11;
    // string range_data2;
    // // make_single_val_data (val2, range_data2);

    // rc = metadata_catalog_all_timestep_attributes_with_type_below_min (server, run_id, type_id, txn_id, ATTR_DATA_TYPE_BLOB, range_data2, count, attr_entries);
    // if (rc == RC_OK) {
    //     testing_log << "using metadata_catalog_all_timestep_attributes_with_type_below_min funct: timestep attrs associated with run_id " << run_id << 
    //     " type_id " << type_id << " and txn_id: " << txn_id << " int attr data less than or eq to " << val2 << endl;
    //     if(testing_logging || (zero_rank_logging )) {
    //         print_timestep_attribute_list (count, attr_entries);
    //     }
    //     print_timestep_attr_data(count, attr_entries);
    // }
    // else {
    //     error_log << "Error metadata_catalog_all_timestep_attributes_with_type_below_min. Proceeding" << endl;
    // }

    // rc = metadata_catalog_all_timestep_attributes_with_type_below_min_in_timestep (server, run_id, timestep_id, type_id, txn_id, ATTR_DATA_TYPE_BLOB, range_data2, count, attr_entries);
    // if (rc == RC_OK) {
    //     testing_log << "using metadata_catalog_all_timestep_attributes_with_type_below_min_in_timestep funct: timestep attrs associated with run_id " << run_id << 
    //     " timestep_id " << timestep_id << " type_id " << type_id << " and txn_id: " << txn_id << " int attr data less than or eq to " << val2 << endl;
    //     if(testing_logging || (zero_rank_logging )) {
    //         print_timestep_attribute_list (count, attr_entries);
    //     }
    //     print_timestep_attr_data(count, attr_entries);
    // }
    // else {
    //     error_log << "Error metadata_catalog_all_timestep_attributes_with_type_below_min_in_timestep. Proceeding" << endl;
    // }

    return rc;

}

int catalog_all_run_attributes ( md_server server, uint64_t run_id, uint64_t type_id, uint64_t txn_id) {

    int rc;
    uint32_t count;
    vector<md_catalog_run_attribute_entry> attr_entries;

    rc = metadata_catalog_all_run_attributes (server, txn_id, count, attr_entries);
    if (rc == RC_OK) {
        testing_log << "using metadata_catalog_all_run_attributes funct: run attrs associated with txn_id: " << txn_id << "\n";
        if(testing_logging || (zero_rank_logging )) {
            print_run_attribute_list (count, attr_entries);
        }
        // print_run_attr_data(count, attr_entries);
    }
    else {
        error_log << "Error metadata_catalog_all_run_attributes. Proceeding" << endl;
    }

    rc = metadata_catalog_all_run_attributes_in_run (server, run_id, txn_id, count, attr_entries);
    if (rc == RC_OK) {
        testing_log << "using metadata_catalog_all_run_attributes funct: run attrs associated with run_id " << run_id << 
            " and txn_id: " << txn_id << "\n";
        if(testing_logging || (zero_rank_logging )) {
            print_run_attribute_list (count, attr_entries);
        }
        // print_run_attr_data(count, attr_entries);
    }
    else {
        error_log << "Error metadata_catalog_all_run_attributes. Proceeding" << endl;
    }

    rc = metadata_catalog_all_run_attributes_with_type (server, type_id, txn_id, count, attr_entries);
    if (rc == RC_OK) {
        testing_log << "using metadata_catalog_all_run_attributes_with_type funct: run attrs associated with" 
            " type_id " << type_id << " and txn_id: " << txn_id << "\n";
        if(testing_logging || (zero_rank_logging )) {
            print_run_attribute_list (count, attr_entries);
        }
        // print_run_attr_data(count, attr_entries);
    }
    else {
        error_log << "Error metadata_catalog_all_run_attributes_with_type. Proceeding" << endl;
    }

    rc = metadata_catalog_all_run_attributes_with_type_in_run (server, run_id, type_id, txn_id, count, attr_entries);

    if (rc == RC_OK) {
        testing_log << "using metadata_catalog_all_run_attributes_with_type_in_run funct: run attrs associated with run_id " << run_id << 
            " type_id " << type_id << " and txn_id: " << txn_id << "\n";
        if(testing_logging || (zero_rank_logging )) {
            print_run_attribute_list (count, attr_entries);
        }
        // print_run_attr_data(count, attr_entries);
    }
    else {
        error_log << "Error metadata_catalog_all_run_attributes_with_type_in_run. Proceeding" << endl;
    }
    // string range_data;
    // uint64_t double min_range = 5;
    // uint64_t max_range = 10;
    // make_range_data (min_range, max_range, range_data);

    // rc = metadata_catalog_all_run_attributes_with_type_range (server, type_id, txn_id, ATTR_DATA_TYPE_BLOB, range_data, count, attr_entries);
    // if (rc == RC_OK) {
    //     testing_log << "using metadata_catalog_all_run_attributes_with_type_range funct: run attrs associated with type_id " << type_id << " and txn_id: " << txn_id << " int attr data and range [" << min_range << "," << max_range << "]\n";
    //     if(testing_logging || (zero_rank_logging )) {
    //         print_run_attribute_list (count, attr_entries);
    //     }
    //     print_run_attr_data(count, attr_entries);
    // }
    // else {
    //     error_log << "Error metadata_catalog_all_run_attributes_with_type_range. Proceeding" << endl;
    // }

    // rc = metadata_catalog_all_run_attributes_with_type_range_in_run (server, run_id, type_id, txn_id, ATTR_DATA_TYPE_BLOB, range_data, count, attr_entries);

    // if (rc == RC_OK) {
    //     testing_log << "using metadata_catalog_all_run_attributes_with_type_range_in_run funct: run attrs associated with run_id " << run_id << 
    //         " type_id " << type_id << " and txn_id: " << txn_id << " int attr data and range [" << min_range << "," << max_range << "]\n";
    //     if(testing_logging || (zero_rank_logging )) {
    //         print_run_attribute_list (count, attr_entries);
    //     }
    //     print_run_attr_data(count, attr_entries);
    // }
    // else {
    //     error_log << "Error metadata_catalog_all_run_attributes_with_type_range_in_run. Proceeding" << endl;
    // }

    // uint64_t val = 12;
    // string range_data1;
    // // make_single_val_data (val, range_data1);

    // rc = metadata_catalog_all_run_attributes_with_type_above_max (server, type_id, txn_id, ATTR_DATA_TYPE_BLOB, range_data1, count, attr_entries);
    // if (rc == RC_OK) {
    //     testing_log << "using metadata_catalog_all_run_attributes_with_type_above_max funct: run attrs associated with type_id " << type_id << " and txn_id: " << txn_id << " int attr data and val geq " << val << "]\n";
    //     if(testing_logging || (zero_rank_logging )) {
    //         print_run_attribute_list (count, attr_entries);
    //     }
    //     print_run_attr_data(count, attr_entries);
    // }
    // else {
    //     error_log << "Error metadata_catalog_all_run_attributes_with_type_above_max. Proceeding" << endl;
    // }

    // rc = metadata_catalog_all_run_attributes_with_type_above_max_in_run (server, run_id, type_id, txn_id, ATTR_DATA_TYPE_BLOB, range_data1, count, attr_entries);
    // if (rc == RC_OK) {
    //     testing_log << "using metadata_catalog_all_run_attributes_with_type_above_max_in_run funct: run attrs associated with run_id " << run_id << 
    //         " type_id " << type_id << " and txn_id: " << txn_id << " int attr data and val geq " << val << "]\n";
    //     if(testing_logging || (zero_rank_logging )) {
    //         print_run_attribute_list (count, attr_entries);
    //     }
    //     print_run_attr_data(count, attr_entries);
    // }
    // else {
    //     error_log << "Error metadata_catalog_all_run_attributes_with_type_above_max_in_run. Proceeding" << endl;
    // }

    // uint64_t val2 = 12;
    // string range_data2;
    // // make_single_val_data (val2, range_data2);

    // rc = metadata_catalog_all_run_attributes_with_type_below_min (server, type_id, txn_id, ATTR_DATA_TYPE_BLOB, range_data2, count, attr_entries);
    // if (rc == RC_OK) {
    //     testing_log << "using metadata_catalog_all_run_attributes_with_type_below_min funct: run attrs associated with type_id " << type_id << " and txn_id: " << txn_id << " int attr data and val leq " << val2 << "]\n";
    //     if(testing_logging || (zero_rank_logging )) {
    //         print_run_attribute_list (count, attr_entries);
    //     }
    //     print_run_attr_data(count, attr_entries);
    // }
    // else {
    //     error_log << "Error metadata_catalog_all_run_attributes_with_type_below_min. Proceeding" << endl;
    // }

    // rc = metadata_catalog_all_run_attributes_with_type_below_min_in_run (server, run_id, type_id, txn_id, ATTR_DATA_TYPE_BLOB, range_data2, count, attr_entries);
    // if (rc == RC_OK) {
    //     testing_log << "using metadata_catalog_all_run_attributes_with_type_below_min_in_run funct: run attrs associated with run_id " << run_id << 
    //         " type_id " << type_id << " and txn_id: " << txn_id << " int attr data and val leq " << val2 << "]\n";
    //     if(testing_logging || (zero_rank_logging )) {
    //         print_run_attribute_list (count, attr_entries);
    //     }
    //     print_run_attr_data(count, attr_entries);
    // }
    // else {
    //     error_log << "Error metadata_catalog_all_run_attributes_with_type_below_min_in_run. Proceeding" << endl;
    // }

    return rc;
}




int create_run_attrs(md_server server, uint64_t run_id, uint64_t txn_id,
                    uint64_t type0_id, uint64_t type1_id )   
{
    int rc;

    uint64_t attribute0_id;
    uint64_t attribute1_id;
    uint64_t attribute2_id;
    uint64_t attribute3_id;
    uint64_t attribute4_id;
    uint64_t attribute5_id;

    vector<md_catalog_run_attribute_entry> all_run_attrs;
    bool insert_run_attrs_in_batch;


    md_catalog_run_attribute_entry new_attr;
    new_attr.run_id = run_id;
    new_attr.type_id = type0_id;
    new_attr.txn_id = txn_id;

    // new_attr.data = "9";
    // new_attr.data = val;
    // make_single_val_data (val, new_attr.data);

    string str("test\0new0",9);
    make_attr_data(str, 9, new_attr.data);
    new_attr.data_type = ATTR_DATA_TYPE_BLOB;

    if(insert_run_attrs_in_batch) {
        all_run_attrs.push_back(new_attr);
    }
    else {
        rc = metadata_insert_run_attribute (server, attribute0_id, new_attr);
        if (rc != RC_OK) {
            error_log << "Error. Was unable to insert the new run attribute. Proceeding" << endl;
        }
        md_log( "New run attribute id is " + to_string(attribute0_id) );
    }


    new_attr.type_id = type1_id;
    string str2("test\0new1",9);
    make_attr_data(str2, 10, new_attr.data);
    // new_attr.data = "10";
    // make_single_val_data (val, new_attr.data);

    if(insert_run_attrs_in_batch) {
        all_run_attrs.push_back(new_attr);
    }
    else {
        rc = metadata_insert_run_attribute (server, attribute1_id, new_attr);
        if (rc != RC_OK) {
            error_log << "Error. Was unable to insert the second new_attr. Proceeding" << endl;
        }
        md_log( "Second new_attr id is " + to_string(attribute1_id ) );
    }

    new_attr.type_id = type0_id;
    string str3("test\0new2",9);

    make_attr_data(str3, 11, new_attr.data);
    // new_attr.data = "11";
    // make_single_val_data (val, new_attr.data);

    if(insert_run_attrs_in_batch) {
        all_run_attrs.push_back(new_attr);
    }
    else {
        rc = metadata_insert_run_attribute (server, attribute2_id, new_attr);
        if (rc != RC_OK) {
            error_log << "Error. Was unable to insert the third new_attr. Proceeding" << endl;
        }
        md_log( "Third new_attr id is " + to_string( attribute2_id) );
    }

    new_attr.type_id = type1_id;
    string str4("test\0new3",9);
    make_attr_data(str4, 12, new_attr.data);
    // new_attr.data = "12";
    // make_single_val_data (val, new_attr.data);

    if(insert_run_attrs_in_batch) {
        all_run_attrs.push_back(new_attr);
    }
    else {
        rc = metadata_insert_run_attribute (server, attribute3_id, new_attr);
        if (rc != RC_OK) {
            error_log << "Error. Was unable to insert the Fourth new_attr. Proceeding" << endl;
        }
        md_log( "Fourth new_attr id is " + to_string(attribute3_id) );
    }

    string str5("test\0new4",9);
    make_attr_data(str5, 13, new_attr.data);
    // new_attr.data = "13";
    // make_single_val_data (val, new_attr.data);

    if(insert_run_attrs_in_batch) {
        all_run_attrs.push_back(new_attr);
    }
    else {
        rc = metadata_insert_run_attribute (server, attribute4_id, new_attr);
        if (rc != RC_OK) {
            error_log << "Error. Was unable to insert the fifth new_attr. Proceeding" << endl;
        }

        md_log( "Fifth new_attr id is " + to_string(attribute4_id) );
    }

    new_attr.txn_id = 100;
    string str6("test\0new5",9);
    make_attr_data(str6, 14, new_attr.data);
    // new_attr.data = "14";
    // make_single_val_data (val, new_attr.data);

    if(insert_run_attrs_in_batch) {
        all_run_attrs.push_back(new_attr);

        rc = metadata_insert_run_attribute_batch (server, all_run_attrs);
        if (rc != RC_OK) {
            error_log << "Error. Was unable to insert run attrs by batch. Proceeding" << endl;
        }
    }
    else {
        rc = metadata_insert_run_attribute (server, attribute5_id, new_attr);
        if (rc != RC_OK) {
            error_log << "Error. Was unable to insert the Sixth new_attr. Proceeding" << endl;
        }

        md_log( "Sixth new_attr id is " + to_string(attribute5_id) );
    }

    return rc;
}

int create_timestep_attrs(md_server server, uint64_t run_id, uint64_t timestep_id, uint64_t timestep_id2, uint64_t txn_id,
                    uint64_t type0_id, uint64_t type1_id )   
{
    int rc;

    uint64_t attribute0_id;
    uint64_t attribute1_id;
    uint64_t attribute2_id;
    uint64_t attribute3_id;
    uint64_t attribute4_id;
    uint64_t attribute5_id;

    vector<md_catalog_timestep_attribute_entry> all_timestep_attrs;
    bool insert_timestep_attrs_in_batch = true;


    md_catalog_timestep_attribute_entry new_attr;
    new_attr.run_id = run_id;
    new_attr.timestep_id = timestep_id;
    new_attr.type_id = type0_id;
    new_attr.txn_id = txn_id;

    string str("test\0new0",9);
    cout << "before make_attr, str: " << str << " and size: " << str.size() << endl;
    make_attr_data(str, 9, new_attr.data);
    // new_attr.data = "9";
    // make_single_val_data (val, new_attr.data);
    // new_attr.data = val;
    new_attr.data_type = ATTR_DATA_TYPE_BLOB;

    if(insert_timestep_attrs_in_batch) {
        all_timestep_attrs.push_back(new_attr);
    }
    else {
        rc = metadata_insert_timestep_attribute (server, attribute0_id, new_attr);
        if (rc != RC_OK) {
            error_log << "Error. Was unable to insert the new timestep attribute. Proceeding" << endl;
        }
        md_log( "New timestep attribute id is " + to_string(attribute0_id) );
    }


    new_attr.type_id = type1_id;
    string str2("test\0new 1",9);
    make_attr_data(str2, 10, new_attr.data);
    // new_attr.data = "10";
    // make_single_val_data (val, new_attr.data);

    if(insert_timestep_attrs_in_batch) {
        all_timestep_attrs.push_back(new_attr);
    }
    else {
        rc = metadata_insert_timestep_attribute (server, attribute1_id, new_attr);
        if (rc != RC_OK) {
            error_log << "Error. Was unable to insert the second new_attr. Proceeding" << endl;
        }
        md_log( "Second new_attr id is " + to_string(attribute1_id ) );
    }


    new_attr.timestep_id = timestep_id2;
    new_attr.type_id = type0_id;

    string str3("test\0new2",9);
    make_attr_data(str3, 11, new_attr.data);
    // new_attr.data = "11";
    // make_single_val_data (val, new_attr.data);

    if(insert_timestep_attrs_in_batch) {
        all_timestep_attrs.push_back(new_attr);
    }
    else {
        rc = metadata_insert_timestep_attribute (server, attribute2_id, new_attr);
        if (rc != RC_OK) {
            error_log << "Error. Was unable to insert the third new_attr. Proceeding" << endl;
        }
        md_log( "Third new_attr id is " + to_string( attribute2_id) );
    }


    new_attr.type_id = type1_id;
    string str4("test\0new3",9);
    make_attr_data(str4, 12, new_attr.data);
    // new_attr.data = "12";
    // make_single_val_data (val, new_attr.data);

    if(insert_timestep_attrs_in_batch) {
        all_timestep_attrs.push_back(new_attr);
    }
    else {
        rc = metadata_insert_timestep_attribute (server, attribute3_id, new_attr);
        if (rc != RC_OK) {
            error_log << "Error. Was unable to insert the Fourth new_attr. Proceeding" << endl;
        }
        md_log( "Fourth new_attr id is " + to_string(attribute3_id) );
    }

    string str5("test\0new4",9);
    make_attr_data(str5, 13, new_attr.data);
    // new_attr.data = "13";
    // make_single_val_data (val, new_attr.data);

    if(insert_timestep_attrs_in_batch) {
        all_timestep_attrs.push_back(new_attr);
    }
    else {
        rc = metadata_insert_timestep_attribute (server, attribute4_id, new_attr);
        if (rc != RC_OK) {
            error_log << "Error. Was unable to insert the fifth new_attr. Proceeding" << endl;
        }

        md_log( "Fifth new_attr id is " + to_string(attribute4_id) );
    }


    new_attr.txn_id = 100;
    string str6("test\0new5",9);
    make_attr_data(str6, 14, new_attr.data);
    // new_attr.data = "14";
    // make_single_val_data (val, new_attr.data);

    if(insert_timestep_attrs_in_batch) {
        all_timestep_attrs.push_back(new_attr);

        rc = metadata_insert_timestep_attribute_batch (server, all_timestep_attrs);
        if (rc != RC_OK) {
            error_log << "Error. Was unable to insert the timestep attrs by batch. Proceeding" << endl;
        }
    }
    else {
        rc = metadata_insert_timestep_attribute (server, attribute5_id, new_attr);
        if (rc != RC_OK) {
            error_log << "Error. Was unable to insert the Sixth new_attr. Proceeding" << endl;
        }

        md_log( "Sixth new_attr id is " + to_string(attribute5_id) );
    }

    return rc;
}


int create_new_timesteps(md_server server, uint64_t run_id, uint64_t run_id2, uint64_t timestep_id, uint64_t timestep_id2, 
                    int64_t type_id, uint64_t var_id, uint64_t txn_id) 
{

    int rc;
    uint64_t attribute0_id;

    string new_timestep_path = "/" + to_string(timestep_id+10);
    rc = metadata_create_timestep (server, timestep_id+10, run_id, new_timestep_path, txn_id);
    if (rc != RC_OK) {
        error_log << "Error. Was unable to create the first new timestep. Exiting \n";
        return RC_ERR;
    }
    md_log( "First new timestep id is " + to_string(timestep_id+10 ) );


    md_catalog_var_attribute_entry new_attr;
    new_attr.run_id = run_id;
    new_attr.timestep_id = timestep_id+10;
    new_attr.type_id = type_id;
    new_attr.var_id = var_id;
    new_attr.txn_id = txn_id;
    new_attr.num_dims = 3;

    vector<md_dim_bounds> dims(3);
    dims [0].min = 1;
    dims [0].max = 5;
    dims [1].min = 2;
    dims [1].max = 5;
    dims [2].min = 3;
    dims [2].max = 5;

    new_attr.dims = dims;

    string str("test\0new0",9);
    make_attr_data(str, 9, new_attr.data);
    // new_attr.data = "9";
    // new_attr.data = val;
    // make_single_val_data (val, new_attr.data);
    new_attr.data_type = ATTR_DATA_TYPE_BLOB;

    rc = metadata_insert_var_attribute_by_dims(server, attribute0_id, new_attr);
    if (rc != RC_OK) {
        error_log << "Error. Was unable to insert the first new_attr. Proceeding" << endl;
    }
    md_log( "First new_attr id is " + to_string(attribute0_id ) );


    //have already created timestep 20 in my_test_client
    // new_timestep_path = "/" + to_string(timestep_id+20);
    // rc = metadata_create_timestep (server, timestep_id+20, run_id, new_timestep_path, txn_id);
    // if (rc != RC_OK) {
    //     error_log << "Error. Was unable to create the first new timestep. Exiting \n";
    //     return RC_ERR;
    // }
    // md_log( "First new timestep id is " + to_string(timestep_id+20 ) );


    new_attr.timestep_id = timestep_id+20;
    new_attr.type_id = type_id+1;
    rc = metadata_insert_var_attribute_by_dims(server, attribute0_id, new_attr);
    if (rc != RC_OK) {
        error_log << "Error. Was unable to insert the first new_attr. Proceeding" << endl;
    }
    md_log( "new_attr id is " + to_string(attribute0_id ) );


    new_timestep_path = "/" + to_string(timestep_id+30);
    rc = metadata_create_timestep (server, timestep_id+30, run_id, new_timestep_path, txn_id);
    if (rc != RC_OK) {
        error_log << "Error. Was unable to create the first new timestep. Exiting \n";
        return RC_ERR;
    }
    md_log( "First new timestep id is " + to_string(timestep_id+30 ) );

    new_attr.timestep_id = timestep_id+30;
    new_attr.type_id = type_id;
    new_attr.var_id = var_id+1;
    rc = metadata_insert_var_attribute_by_dims(server, attribute0_id, new_attr);
    if (rc != RC_OK) {
        error_log << "Error. Was unable to insert the first new_attr. Proceeding" << endl;
    }
    md_log( "new_attr id is " + to_string(attribute0_id ) );


    new_timestep_path = "/" + to_string(timestep_id+40);
    rc = metadata_create_timestep (server, timestep_id+40, run_id, new_timestep_path, txn_id);
    if (rc != RC_OK) {
        error_log << "Error. Was unable to create the first new timestep. Exiting \n";
        return RC_ERR;
    }
    md_log( "First new timestep id is " + to_string(timestep_id+40 ) );


    new_attr.var_id = var_id;
    new_attr.timestep_id = timestep_id+40;
    dims [0].min = 1;
    dims [0].max = 4;
    dims [1].min = 2;
    dims [1].max = 3;
    dims [2].min = 3;
    dims [2].max = 3;
    new_attr.dims = dims;

    rc = metadata_insert_var_attribute_by_dims(server, attribute0_id, new_attr);
    if (rc != RC_OK) {
        error_log << "Error. Was unable to insert the first new_attr. Proceeding" << endl;
    }
    md_log( "First new_attr id is " + to_string(attribute0_id ) );


    new_timestep_path = "/" + to_string(timestep_id+10);
    rc = metadata_create_timestep (server, timestep_id+10, run_id2, new_timestep_path, txn_id);
    if (rc != RC_OK) {
        error_log << "Error. Was unable to create the second new timestep. Exiting \n";
        return RC_ERR;
    }
    md_log( "Second new timestep id is " + to_string(timestep_id+10 ) );


    new_timestep_path = "/" + to_string(timestep_id2+100);
    rc = metadata_create_timestep (server, timestep_id2+100, run_id, new_timestep_path, txn_id+100);
    if (rc != RC_OK) {
        error_log << "Error. Was unable to create the third new timestep. Exiting \n";
        return RC_ERR;
    }
    md_log( "Third new timestep id is " + to_string(timestep_id2+100 ) );

    return rc;
}

int create_new_types(md_server server, uint64_t run_id, uint64_t run_id2, uint64_t timestep_id, uint64_t type_id2, 
                    int64_t type_id, uint64_t var_id, uint64_t txn_id, md_catalog_type_entry prev_type) 
{

    int rc;
    uint64_t attribute0_id;
    uint64_t new_type_id;


    md_catalog_type_entry new_type = prev_type;
    new_type.version = 10;

    rc = metadata_create_type (server, new_type_id, new_type);
    if (rc != RC_OK) {
        error_log << "Error. Was unable to create the first new type. Exiting \n";
        return RC_ERR;
    }
    md_log( "First new type id is " + to_string(new_type_id ) );


    md_catalog_var_attribute_entry new_attr;
    new_attr.run_id = run_id;
    new_attr.type_id = new_type_id;
    new_attr.timestep_id = timestep_id;
    new_attr.var_id = var_id;
    new_attr.txn_id = txn_id;
    new_attr.num_dims = 3;

    vector<md_dim_bounds> dims(3);
    dims [0].min = 1;
    dims [0].max = 5;
    dims [1].min = 2;
    dims [1].max = 5;
    dims [2].min = 3;
    dims [2].max = 5;

    new_attr.dims = dims;

    string str("test\0new0",9);
    make_attr_data(str, 9, new_attr.data);
    // new_attr.data = "9";
    // new_attr.data = val;
    // make_single_val_data (val, new_attr.data);
    new_attr.data_type = ATTR_DATA_TYPE_BLOB;

    rc = metadata_insert_var_attribute_by_dims(server, attribute0_id, new_attr);
    if (rc != RC_OK) {
        error_log << "Error. Was unable to insert the first new_attr. Proceeding" << endl;
    }
    md_log( "First new_attr id is " + to_string(attribute0_id ) );



    new_type.version = 20;

  rc = metadata_create_type (server, new_type_id, new_type);
    if (rc != RC_OK) {
        error_log << "Error. Was unable to create the first new type. Exiting \n";
        return RC_ERR;
    }
    md_log( " new type id is " + to_string(new_type_id ) );


    new_attr.type_id = new_type_id;
    new_attr.timestep_id = timestep_id+1;
    rc = metadata_insert_var_attribute_by_dims(server, attribute0_id, new_attr);
    if (rc != RC_OK) {
        error_log << "Error. Was unable to insert the first new_attr. Proceeding" << endl;
    }
    md_log( "new_attr id is " + to_string(attribute0_id ) );


    new_type.version = 30;

    rc = metadata_create_type (server, new_type_id, new_type);
    if (rc != RC_OK) {
        error_log << "Error. Was unable to create the first new type. Exiting \n";
        return RC_ERR;
    }
    md_log( "First new type id is " + to_string(new_type_id ) );

    new_attr.type_id = new_type_id;
    new_attr.timestep_id = timestep_id;
    new_attr.var_id = var_id+1;
    rc = metadata_insert_var_attribute_by_dims(server, attribute0_id, new_attr);
    if (rc != RC_OK) {
        error_log << "Error. Was unable to insert the first new_attr. Proceeding" << endl;
    }
    md_log( "new_attr id is " + to_string(attribute0_id ) );


    new_type.version = 40;

    rc = metadata_create_type (server, new_type_id, new_type);
    if (rc != RC_OK) {
        error_log << "Error. Was unable to create the first new type. Exiting \n";
        return RC_ERR;
    }
    md_log( "First new type id is " + to_string(new_type_id ) );


    new_attr.var_id = var_id;
    new_attr.type_id = new_type_id;
    dims [0].min = 1;
    dims [0].max = 4;
    dims [1].min = 2;
    dims [1].max = 3;
    dims [2].min = 3;
    dims [2].max = 3;
    new_attr.dims = dims;

    rc = metadata_insert_var_attribute_by_dims(server, attribute0_id, new_attr);
    if (rc != RC_OK) {
        error_log << "Error. Was unable to insert the first new_attr. Proceeding" << endl;
    }
    md_log( "First new_attr id is " + to_string(attribute0_id ) );


    new_type.version = 10;
    new_type.run_id = run_id2;
    rc = metadata_create_type (server, new_type_id, new_type);
    if (rc != RC_OK) {
        error_log << "Error. Was unable to create the second new type. Exiting \n";
        return RC_ERR;
    }
    md_log( "Second new type id is " + to_string(new_type_id ) );


    new_type.version = 100;
    new_type.txn_id = txn_id + 100;
    rc = metadata_create_type (server, new_type_id, new_type);
    if (rc != RC_OK) {
        error_log << "Error. Was unable to create the third new type. Exiting \n";
        return RC_ERR;
    }
    md_log( "Third new type id is " + to_string(new_type_id) );

    return rc;
}
