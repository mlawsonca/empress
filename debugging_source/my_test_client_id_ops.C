#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include <mpi.h>#include <my_metadata_client.h>#include "dirman/DirMan.hh"
#include <sstream>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>


//Tells nnti how to initialize its network settings ???

//dw settings
std::string default_config_string = R"EOF(
# Tell the nnti transport to use infiniband

nnti.logger.severity       error
nnti.transport.name        ibverbs
whookie.interfaces         ib0,lo
 
#config.additional_files.env_name.if_defined   CONFIG
lunasa.lazy_memory_manager   malloc
lunasa.eager_memory_manager  tcmalloc

# Select the type of dirman to use. Currently we only have centralized, which
# just sticks all the directory info on one node (called root). 
dirman.type           centralized

# Turn these on if you want to see more debug messages
#bootstrap.debug           true
#whookie.debug             true
#opbox.debug               true
#dirman.debug              true
#dirman.cache.others.debug true
#dirman.cache.mine.debug   true

)EOF";

using namespace std;

bool testing;
md_server_type my_server_type;


std::vector<int> catg_of_time_pts;
std::vector<std::chrono::high_resolution_clock::time_point> time_pts;

//A variable that will keep track of all mpi information

int run_test(int rank, faodel::DirectoryInfo dir);
static int setup_dirman(const string &dirman_hexid, const string &dir_path, faodel::DirectoryInfo &dir);
static void setup_server(md_server &server, int rank, const faodel::DirectoryInfo &dir);

void print_attr_data(uint32_t count, std::vector<md_catalog_attribute_entry> attr_entries);


static bool md_debug = true;
// static bool md_extreme_debug = false;


static void md_log(const std::string &s) {
  if(md_debug) std::cout << s<< std::endl;
}

static bool testing_logging = false;
static bool extreme_debug_logging = false;
static bool zero_rank_logging = true;
static bool error_logging  = true;

static debugLog testing_log = debugLog(testing_logging, zero_rank_logging);
static debugLog error_log = debugLog(error_logging, zero_rank_logging);
static debugLog extreme_debug_log = debugLog(extreme_debug_logging, false);

static bool output_timing = false;


void add_timing_point(int catg) {
    if (output_timing) {
        time_pts.push_back(chrono::high_resolution_clock::now());
        catg_of_time_pts.push_back(catg);
    }
}


/*
 * Launches the number of mpi processes specified by the user, and has each process 
 * preform a series of tests of the metadata_client functions
 *
 * Inputs: 
 *   the user should provide the directory manager's full URL (argv[1])
 *   
 */
int main(int argc, char **argv)
{
    int rc;
    bool ok;
    int rank;
    int num_servers;
    string dir_path="/metadata/testing";
    
    // faodel::DirectoryInfo dir(dir_path,"Sirius metadata servers");

    faodel::DirectoryInfo dir;

    //make sure the user provided an argument, the program is expecting to receive
    //the directory manager's full URL path

    if(argc != 3) {
        error_log << "Usage: " << argv[0] << "my_server_type , <directory manager hexid> \n";
        return RC_ERR;
    }

    my_server_type = (md_server_type)stoul(argv[1]);


    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    if(rank != 0) {
        md_debug = false;
    }
    testing_log.set_rank(rank);

    rc = setup_dirman(argv[2], dir_path, dir);

    if(rc != RC_OK) {
        goto cleanup;
    }
    num_servers = dir.members.size();

    // //have the processes preform a series of tests of the metadata client functions
    rc = run_test (rank, dir);

    // G2.StopAll();

    MPI_Barrier(MPI_COMM_WORLD);

    //returns RC_OK if there were no errors, or RC_ERR if there were errors
    MPI_Finalize();
    faodel::bootstrap::Finish();

    md_log( "rc is " + to_string(rc) );

cleanup:
    return rc;
}

/*
 * Takes the full URL of the directory manager
 */
int run_test(int rank, faodel::DirectoryInfo dir) {
    int rc;
    uint64_t txn_id = 5;
    int32_t mins [3];
    int32_t maxes [3];
    std::string connection;
    uint64_t length;
    struct md_catalog_var_entry new_var;
    struct md_catalog_type_entry new_type;
    uint64_t var0_id;
    uint64_t var1_id;
    uint32_t var0_version = 0;
    uint32_t var1_version = 1;
    uint32_t type0_version = 0;
    uint32_t type1_version = 1;

    uint64_t type0_id;
    uint64_t type1_id;

    uint64_t attribute0_id;
    uint64_t attribute1_id;
    uint64_t attribute2_id;
    uint64_t attribute3_id;
    uint64_t attribute4_id;


    std::string var0_name = "var0";
    std::string var0_path = "/var0";
    std::string var1_name = "var1";
    std::string var1_path = "/var1";

    std::string dataset0_name = "dataset0";
    std::string dataset0_path = "/dataset0";
    uint64_t dataset0_timestep = 123456789;
    std::string dataset0_rank_to_dims_funct = "FIX ME"; // FIX!

    char type0 = 8;
    char type1 = 16;
    string type0_name = "typey0";
    string type1_name = "typey1";

    md_server server;

    uint64_t npx0 = 10;
    uint64_t npy0 = 10;
    uint64_t npz0 = 10;
    uint64_t npx1 = 100;
    uint64_t npy1 = 100;
    uint64_t npz1 = 100;


    setup_server(server, rank, dir);


    rc = metadata_init ();

    if (rc != RC_OK) {
        error_log << "Error. Was unable to initialize the client. Exitting \n";
        return RC_ERR;
    }

    testing_log << "\nBeginning the testing setup \n";

    uint64_t dataset0_id;

    rc = metadata_create_dataset (server, dataset0_id, txn_id, dataset0_name, dataset0_path, dataset0_timestep,
                        npx0, npy0, npz0, dataset0_rank_to_dims_funct);

    if (rc != RC_OK) {
        error_log << "Error. Was unable to create the first dataset. Exiting \n";
        return RC_ERR;
    }

    md_log( "First dataset id is " + to_string(dataset0_id ) );

    new_var.txn_id = txn_id;
    new_var.dataset_id = dataset0_id;
    new_var.name = var0_name;
    new_var.path = var0_path;
    new_var.data_size = type0;
    new_var.version = var0_version;
    new_var.num_dims = 2;
    new_var.txn_id = txn_id;
    md_dim_bounds *dims = (md_dim_bounds *) malloc(sizeof(md_dim_bounds) * 3);

    dims [0].min = 0;
    dims [0].max = 9;
    dims [1].min = 0;
    dims [1].max = 9;
    new_var.dims = std::vector<md_dim_bounds>(dims, dims + new_var.num_dims );



    rc = metadata_create_var (server, var0_id, new_var);

    if (rc != RC_OK) {
        error_log << "Error. Was unable to create the first var. Exiting \n";
        return RC_ERR;
    }
    md_log( "First var id is " + to_string(var0_id ) );

    //creates a variable, assigning it a location in the simulation space
    new_var.name = var1_name;
    new_var.path = var1_path;
    new_var.version = var1_version;
    new_var.data_size = type1;
    new_var.num_dims = 3;
    dims [0]. min = 0;
    dims [0]. max = 99;
    dims [1]. min = 0;
    dims [1]. max = 99;
    dims [2]. min = 0;
    dims [2]. max = 99;
    new_var.dims = std::vector<md_dim_bounds>(dims, dims + new_var.num_dims );
    extreme_debug_log << "about to create var \n";
    rc = metadata_create_var (server, var1_id, new_var);

    if (rc != RC_OK) {
        error_log << "Error. Was unable to create the second var. Exiting \n";
        return RC_ERR;
    }
    md_log( "Second var id is " + to_string(var1_id ) );


//-------------------------------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------------------------------
//TYPE AND ATTRIBUTE SETUP

    new_type.dataset_id = dataset0_id;
    new_type.name = type0_name;
    new_type.version = type0_version;
    new_type.txn_id = txn_id;

    rc = metadata_create_type (server, type0_id, new_type);
    if (rc != RC_OK) {
        error_log << "Error. Was unable to insert the first type. Proceeding" << endl;
    }
    md_log( "First type id is " + to_string(type0_id ) );


    new_type.name = type1_name;
    new_type.version = type1_version;
    new_type.txn_id = txn_id;

    rc = metadata_create_type (server, type1_id, new_type);
    if (rc != RC_OK) {
        error_log << "Error. Was unable to insert the second type. Proceeding" << endl;
    }
    md_log( "Second type id is " + to_string(type1_id ) );

    md_catalog_attribute_entry attr;
    attr.txn_id = txn_id;
    attr.dataset_id = dataset0_id;
    attr.type_id = type0_id;
    attr.var_id = var0_id;
    attr.num_dims = 2;

    dims [0].min = 1;
    dims [0].max = 5;
    dims [1].min = 2;
    dims [1].max = 5;
    // dims [2].min = 3;
    // dims [2].max = 10;
    attr.dims = std::vector<md_dim_bounds>(dims, dims + attr.num_dims );

    stringstream ss;
    string test_string = "test0";
    int test_int = 9;
    boost::archive::text_oarchive oa(ss);
    oa << test_string;
    oa << test_int;
    std::string serial_str = ss.str();

    attr.data = serial_str;

    rc = metadata_insert_attribute_by_dims(server, attribute0_id, attr);
    if (rc != RC_OK) {
        error_log << "Error. Was unable to insert the first attr. Proceeding" << endl;
    }
    md_log( "First attr id is " + to_string(attribute0_id ) );


    attr.type_id = type1_id;

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
    attr.dims = std::vector<md_dim_bounds>(dims, dims + attr.num_dims );

    stringstream ss1;
    test_string = "test1";
    test_int = 10;
    boost::archive::text_oarchive oa1(ss1);
    oa1 << test_string;
    oa1 << test_int;
    serial_str = ss1.str();

    attr.data = serial_str;

    rc = metadata_insert_attribute_by_dims(server, attribute1_id, attr);
    if (rc != RC_OK) {
        error_log << "Error. Was unable to insert the second attr. Proceeding" << endl;
    }
    md_log( "Second attr id is " + to_string(attribute1_id ) );

    attr.type_id = type0_id;
    attr.var_id = var1_id;
    attr.num_dims = 3;

    dims [0].min = 0;
    dims [0].max = 9;
    dims [1].min = 0;
    dims [1].max = 9;
    dims [2].min = 0;
    dims [2].max = 9;
    attr.dims = std::vector<md_dim_bounds>(dims, dims + attr.num_dims );

    stringstream ss2;
    test_string = "test2";
    test_int= 11;
    boost::archive::text_oarchive oa2(ss2);
    oa2 << test_string;
    oa2 << test_int;
    serial_str = ss2.str();

    attr.data = serial_str;

    rc = metadata_insert_attribute_by_dims(server, attribute2_id, attr);
    if (rc != RC_OK) {
        error_log << "Error. Was unable to insert the third attr. Proceeding" << endl;
    }


    md_log( "Third attr id is " + to_string( attribute2_id) );


    attr.type_id = type1_id;

    dims [0].min = 10;
    dims [0].max = 19;
    dims [1].min = 10;
    dims [1].max = 19;
    dims [2].min = 10;
    dims [2].max = 19;
    attr.dims = std::vector<md_dim_bounds>(dims, dims + attr.num_dims );

    stringstream ss3;
    test_string = "test3";
    test_int= 12;
    boost::archive::text_oarchive oa3(ss3);
    oa3 << test_string;
    oa3 << test_int;
    serial_str = ss3.str();

    attr.data = serial_str;

    rc = metadata_insert_attribute_by_dims(server, attribute3_id, attr);
    if (rc != RC_OK) {
        error_log << "Error. Was unable to insert the Fourth attr. Proceeding" << endl;
    }


    md_log( "Fourth attr id is " + to_string(attribute3_id) );
    dims [0].min = 20;
    dims [0].max = 29;
    dims [1].min = 20;
    dims [1].max = 29;
    dims [2].min = 20;
    dims [2].max = 29;
    attr.dims = std::vector<md_dim_bounds>(dims, dims + attr.num_dims );

    stringstream ss4;
    test_string = "test4";
    test_int= 13;
    boost::archive::text_oarchive oa4(ss4);
    oa4 << test_string;
    oa4 << test_int;
    serial_str = ss4.str();

    attr.data = serial_str;

    rc = metadata_insert_attribute_by_dims(server, attribute4_id, attr);
    if (rc != RC_OK) {
        error_log << "Error. Was unable to insert the fifth attr. Proceeding" << endl;
    }

    md_log( "Fifth attr id is " + to_string(attribute4_id) );


//-------------------------------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------------------------------
//TYPE AND ATTRIBUTE TESTING

    uint32_t count;
    std::vector<md_catalog_type_entry> type_entries;
    std::vector<md_catalog_attribute_entry> attr_entries;
    string deserialized_test_string;
    int deserialized_test_int;

    testing_log << "\nBeginning the type and attribute testing \n" ;

    //returns information about the types associated with the given txn_id 
    rc = metadata_catalog_type (server, txn_id, dataset0_id, count, type_entries);

    if (rc == RC_OK) {
        testing_log << "type catalog: \n";
        if(testing_logging || (zero_rank_logging && rank == 0)) {
            print_type_catalog (count, type_entries);
        }
    }
    else {
        error_log << "Error cataloging the first set of type entries. Proceeding" << endl;
    }



//-------------------------------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------------------------------
//NEW ATTRIBUTE TESTING


    testing_log << "\nBeginning the new type and attribute testing \n" ;

    // uint64_t attribute_id_new;
    // md_catalog_attribute_entry attr_new;
    // attr_new.type_id = type0_id;

    // attr_new.num_dims = 3;
    // dims [0].min = 5;
    // dims [0].max = 10;
    // dims [1].min = 5;
    // dims [1].max = 10;
    // dims [2].min = 5;
    // dims [2].max = 10;
    // attr_new.dims = std::vector<md_dim_bounds>(dims, dims + attr_new.num_dims );

    // stringstream ss_new;
    // string test_string_new = "new_attr";
    // int test_int_new = 14;
    // boost::archive::text_oarchive oa_new(ss_new);
    // oa_new << test_string_new;
    // oa_new << test_int_new;
    // std::string serial_str_new = ss_new.str();
    // attr_new.data = serial_str_new;

    // rc = metadata_insert_attribute_by_dims (server, attribute_id_new, attr_new);
    // if (rc != RC_OK) {
    //     error_log << "Error. Was unable to insert the first attribute. Proceeding" << endl;
    // }


    // rc = metadata_catalog_all_attributes_with_dims(server, txn_id, dataset0_id, attr_new.num_dims, attr_new.dims, count, attr_entries);

    // if (rc == RC_OK) {
    //     testing_log << "attributes associated with txn_id: " << txn_id << " and chunk_id: " << chunk_id1 <<
    //     " (count: " << count << ") \n";
    //     if(testing_logging || (zero_rank_logging && rank == 0)) {
    //         print_attribute_list (count, attr_entries);
    //     }

    //     print_attr_data(count, attr_entries);
    // }
    // else {
    //     error_log << "Error getting the matching attribute list. Proceeding" << endl;
    // }


    uint32_t new_num_dims = 3;
    dims [0].min = 15;
    dims [0].max = 25;
    dims [1].min = 15;
    dims [1].max = 25;
    dims [2].min = 15;
    dims [2].max = 25;

    vector<md_dim_bounds> vect_dims = std::vector<md_dim_bounds>(dims, dims + new_num_dims);


    rc = metadata_catalog_all_attributes_with_dims(server, txn_id, dataset0_id, new_num_dims, vect_dims, count, attr_entries);

    if (rc == RC_OK) {
        testing_log << "attributes associated with txn_id: " << txn_id << " overlapping with dims";
        for(int j=0; j< new_num_dims; j++) {
            testing_log << " d" << j << "_min: " << dims [j].min;
            testing_log << " d" << j << "_max: " << dims [j].max;               
        }
        testing_log << "\n";
        if(testing_logging || (zero_rank_logging && rank == 0)) {
            print_attribute_list (count, attr_entries);
        }

        print_attr_data(count, attr_entries);
    }
    else {
        error_log << "Error getting the matching attribute list. Proceeding" << endl;
    }

    dims [0].min = 5;
    dims [0].max = 9;
    dims [1].min = 5;
    dims [1].max = 9;
    dims [2].min = 5;
    dims [2].max = 9;

    vect_dims = std::vector<md_dim_bounds>(dims, dims + new_num_dims);
    
    rc = metadata_catalog_all_attributes_with_dims(server, txn_id, dataset0_id, new_num_dims, vect_dims, count, attr_entries);

    if (rc == RC_OK) {
        testing_log << "attributes associated with txn_id: " << txn_id << " overlapping with dims";
        for(int j=0; j< new_num_dims; j++) {
            testing_log << " d" << j << "_min: " << dims [j].min;
            testing_log << " d" << j << "_max: " << dims [j].max;               
        }
        testing_log << "\n";
        if(testing_logging || (zero_rank_logging && rank == 0)) {
            print_attribute_list (count, attr_entries);
        }

        print_attr_data(count, attr_entries);
    }
    else {
        error_log << "Error getting the matching attribute list. Proceeding" << endl;
    }

    dims [0].min = 5;
    dims [0].max = 10;
    dims [1].min = 5;
    dims [1].max = 10;
    dims [2].min = 5;
    dims [2].max = 10;

    vect_dims = std::vector<md_dim_bounds>(dims, dims + new_num_dims);

    rc = metadata_catalog_all_attributes_with_dims(server, txn_id, dataset0_id, new_num_dims, vect_dims, count, attr_entries);

    if (rc == RC_OK) {
        testing_log << "attributes associated with txn_id: " << txn_id << " overlapping with dims";
        for(int j=0; j< new_num_dims; j++) {
            testing_log << " d" << j << "_min: " << dims [j].min;
            testing_log << " d" << j << "_max: " << dims [j].max;               
        }
        testing_log << " \n";
        if(testing_logging || (zero_rank_logging && rank == 0)) {
            print_attribute_list (count, attr_entries);
        }

        print_attr_data(count, attr_entries);
    }
    else {
        error_log << "Error getting the matching attribute list. Proceeding" << endl;
    }



   rc = metadata_catalog_all_attributes_with_type_dims_by_id(server, txn_id, dataset0_id, type0_id, new_num_dims, vect_dims, count, attr_entries);

    if (rc == RC_OK) {
        testing_log << "attributes associated with txn_id: " << txn_id << " and type " << type0_name << " ver " << type0_version << 
            " overlapping with dims";
        for(int j=0; j< new_num_dims; j++) {
            testing_log << " d" << j << "_min: " << dims [j].min;
            testing_log << " d" << j << "_max: " << dims [j].max;               
        }
        testing_log << " \n";
        if(testing_logging || (zero_rank_logging && rank == 0)) {
            print_attribute_list (count, attr_entries);
        }

        print_attr_data(count, attr_entries);
    }
    else {
        error_log << "Error getting the matching attribute list. Proceeding" << endl;
    }



   rc = metadata_catalog_all_attributes_with_type_dims_by_id(server, txn_id, dataset0_id, type1_id, new_num_dims, vect_dims, count, attr_entries);

    if (rc == RC_OK) {
        testing_log << "attributes associated with txn_id: " << txn_id << " and type " << type1_name << " ver " << type1_version << 
            " overlapping with dims";
        for(int j=0; j< new_num_dims; j++) {
            testing_log << " d" << j << "_min: " << dims [j].min;
            testing_log << " d" << j << "_max: " << dims [j].max;               
        }
        testing_log << "\n";
        if(testing_logging || (zero_rank_logging && rank == 0)) {
            print_attribute_list (count, attr_entries);
        }

        print_attr_data(count, attr_entries);
    }
    else {
        error_log << "Error getting the matching attribute list. Proceeding" << endl;
    }

   rc = metadata_catalog_all_attributes_with_var_dims_by_id(server, txn_id, dataset0_id, var0_id, new_num_dims, vect_dims, count, attr_entries);

    if (rc == RC_OK) {
        testing_log << "attributes associated with txn_id: " << txn_id << " and var " << var0_name << " ver " << var0_version << 
            " overlapping with dims";
        for(int j=0; j< new_num_dims; j++) {
            testing_log << " d" << j << "_min: " << dims [j].min;
            testing_log << " d" << j << "_max: " << dims [j].max;               
        }
        testing_log << "\n";
        if(testing_logging || (zero_rank_logging && rank == 0)) {
            print_attribute_list (count, attr_entries);
        }

        print_attr_data(count, attr_entries);
    }
    else {
        error_log << "Error getting the matching attribute list. Proceeding" << endl;
    }



   rc = metadata_catalog_all_attributes_with_var_dims_by_id(server, txn_id, dataset0_id, var1_id, new_num_dims, vect_dims, count, attr_entries);

    if (rc == RC_OK) {
        testing_log << "attributes associated with txn_id: " << txn_id << " and var " << var1_name << " ver " << var1_version << 
            " overlapping with dims";
        for(int j=0; j< new_num_dims; j++) {
            testing_log << " d" << j << "_min: " << dims [j].min;
            testing_log << " d" << j << "_max: " << dims [j].max;               
        }
        testing_log << "\n";
        if(testing_logging || (zero_rank_logging && rank == 0)) {
            print_attribute_list (count, attr_entries);
        }

        print_attr_data(count, attr_entries);
    }
    else {
        error_log << "Error getting the matching attribute list. Proceeding" << endl;
    }



   rc = metadata_catalog_all_attributes_with_type_var_dims_by_id(server, txn_id, dataset0_id, type0_id, var0_id, new_num_dims, vect_dims, count, attr_entries);

    if (rc == RC_OK) {
        testing_log << "attributes associated with txn_id: " << txn_id << " and type " << type0_name << " ver " << type0_version <<
        " and var " << var0_name << " ver " << var0_version << " overlapping with dims";
        for(int j=0; j< new_num_dims; j++) {
            testing_log << " d" << j << "_min: " << dims [j].min;
            testing_log << " d" << j << "_max: " << dims [j].max;               
        }
        testing_log << "\n";
        if(testing_logging || (zero_rank_logging && rank == 0)) {
            print_attribute_list (count, attr_entries);
        }

        print_attr_data(count, attr_entries);
    }
    else {
        error_log << "Error getting the matching attribute list. Proceeding" << endl;
    }



   rc = metadata_catalog_all_attributes_with_type_var_dims_by_id(server, txn_id, dataset0_id, type0_id, var1_id, new_num_dims, vect_dims, count, attr_entries);

    if (rc == RC_OK) {
        testing_log << "attributes associated with txn_id: " << txn_id << " and type " << type0_name << " ver " << type0_version <<
        " and var " << var1_name << " ver " << var1_version << " overlapping with dims";
        for(int j=0; j< new_num_dims; j++) {
            testing_log << " d" << j << "_min: " << dims [j].min;
            testing_log << " d" << j << "_max: " << dims [j].max;               
        }
        testing_log << "\n";
        if(testing_logging || (zero_rank_logging && rank == 0)) {
            print_attribute_list (count, attr_entries);
        }

        print_attr_data(count, attr_entries);
    }
    else {
        error_log << "Error getting the matching attribute list. Proceeding" << endl;
    }

   rc = metadata_catalog_all_attributes_with_type_var_dims_by_id(server, txn_id, dataset0_id, type1_id, var0_id, new_num_dims, vect_dims, count, attr_entries);

    if (rc == RC_OK) {
        testing_log << "attributes associated with txn_id: " << txn_id << " and type " << type1_name << " ver " << type1_version <<
        " and var " << var0_name << " ver " << var0_version << " overlapping with dims";
        for(int j=0; j< new_num_dims; j++) {
            testing_log << " d" << j << "_min: " << dims [j].min;
            testing_log << " d" << j << "_max: " << dims [j].max;               
        }
        testing_log << "\n";
        if(testing_logging || (zero_rank_logging && rank == 0)) {
            print_attribute_list (count, attr_entries);
        }

        print_attr_data(count, attr_entries);
    }
    else {
        error_log << "Error getting the matching attribute list. Proceeding" << endl;
    }


   rc = metadata_catalog_all_attributes_with_type_var_dims_by_id(server, txn_id, dataset0_id, type1_id, var1_id, new_num_dims, vect_dims, count, attr_entries);

    if (rc == RC_OK) {
        testing_log << "attributes associated with txn_id: " << txn_id << " and type " << type1_name << " ver " << type1_version <<
        " and var " << var1_name << " ver " << var1_version << " overlapping with dims";
        for(int j=0; j< new_num_dims; j++) {
            testing_log << " d" << j << "_min: " << dims [j].min;
            testing_log << " d" << j << "_max: " << dims [j].max;               
        }
        testing_log << "\n";
        if(testing_logging || (zero_rank_logging && rank == 0)) {
            print_attribute_list (count, attr_entries);
        }

        print_attr_data(count, attr_entries);
    }
    else {
        error_log << "Error getting the matching attribute list. Proceeding" << endl;
    }


    rc = metadata_catalog_all_attributes (server, txn_id, dataset0_id, count, attr_entries);
    if (rc == RC_OK) {
        testing_log << "attributes associated with txn_id: " << txn_id;
        testing_log << "\n";
        if(testing_logging || (zero_rank_logging && rank == 0)) {
            print_attribute_list (count, attr_entries);
        }

        print_attr_data(count, attr_entries);
    }
    else {
        error_log << "Error getting the matching attribute list. Proceeding" << endl;
    }


    rc = metadata_catalog_all_attributes_with_type_by_id (server, txn_id, dataset0_id, type0_id, count, attr_entries);

    if (rc == RC_OK) {
        testing_log << "attributes associated with txn_id: " << txn_id << " and type " << type0_name << " ver " << type0_version;
        testing_log << "\n";
        if(testing_logging || (zero_rank_logging && rank == 0)) {
            print_attribute_list (count, attr_entries);
        }

        print_attr_data(count, attr_entries);
    }
    else {
        error_log << "Error getting the matching attribute list. Proceeding" << endl;
    }


    rc = metadata_catalog_all_attributes_with_type_by_id (server, txn_id, dataset0_id, type1_id, count, attr_entries);

    if (rc == RC_OK) {
        testing_log << "attributes associated with txn_id: " << txn_id << " and type " << type1_name << " ver " << type1_version;
        testing_log << "\n";
        if(testing_logging || (zero_rank_logging && rank == 0)) {
            print_attribute_list (count, attr_entries);
        }

        print_attr_data(count, attr_entries);
    }
    else {
        error_log << "Error getting the matching attribute list. Proceeding" << endl;
    }


    rc = metadata_catalog_all_attributes_with_var_by_id (server, txn_id, dataset0_id, var0_id, count, attr_entries);

    if (rc == RC_OK) {
        testing_log << "attributes associated with txn_id: " << txn_id << " and var " << var0_name << " ver " << var0_version;
        testing_log << "\n";
        if(testing_logging || (zero_rank_logging && rank == 0)) {
            print_attribute_list (count, attr_entries);
        }

        print_attr_data(count, attr_entries);
    }
    else {
        error_log << "Error getting the matching attribute list. Proceeding" << endl;
    }


    rc = metadata_catalog_all_attributes_with_var_by_id (server, txn_id, dataset0_id, var1_id, count, attr_entries);

    if (rc == RC_OK) {
        testing_log << "attributes associated with txn_id: " << txn_id << " and var " << var1_name << " ver " << var1_version;
        testing_log << "\n";
        if(testing_logging || (zero_rank_logging && rank == 0)) {
            print_attribute_list (count, attr_entries);
        }

        print_attr_data(count, attr_entries);
    }
    else {
        error_log << "Error getting the matching attribute list. Proceeding" << endl;
    }


   rc = metadata_catalog_all_attributes_with_type_var_by_id(server, txn_id, dataset0_id, type0_id, var0_id, count, attr_entries);

    if (rc == RC_OK) {
        testing_log << "attributes associated with txn_id: " << txn_id << " and type " << type0_name << " ver " << type0_version <<
        " and var " << var0_name << " ver " << var0_version;
        testing_log << "\n";
        if(testing_logging || (zero_rank_logging && rank == 0)) {
            print_attribute_list (count, attr_entries);
        }

        print_attr_data(count, attr_entries);
    }
    else {
        error_log << "Error getting the matching attribute list. Proceeding" << endl;
    }



   rc = metadata_catalog_all_attributes_with_type_var_by_id(server, txn_id, dataset0_id, type0_id, var1_id, count, attr_entries);

    if (rc == RC_OK) {
        testing_log << "attributes associated with txn_id: " << txn_id << " and type " << type0_name << " ver " << type0_version <<
        " and var " << var1_name << " ver " << var1_version;
        testing_log << "\n";
        if(testing_logging || (zero_rank_logging && rank == 0)) {
            print_attribute_list (count, attr_entries);
        }

        print_attr_data(count, attr_entries);
    }
    else {
        error_log << "Error getting the matching attribute list. Proceeding" << endl;
    }

   rc = metadata_catalog_all_attributes_with_type_var_by_id(server, txn_id, dataset0_id, type1_id, var0_id, count, attr_entries);

    if (rc == RC_OK) {
        testing_log << "attributes associated with txn_id: " << txn_id << " and type " << type1_name << " ver " << type1_version <<
        " and var " << var0_name << " ver " << var0_version;
        testing_log << "\n";
        if(testing_logging || (zero_rank_logging && rank == 0)) {
            print_attribute_list (count, attr_entries);
        }

        print_attr_data(count, attr_entries);
    }
    else {
        error_log << "Error getting the matching attribute list. Proceeding" << endl;
    }


   rc = metadata_catalog_all_attributes_with_type_var_by_id(server, txn_id, dataset0_id, type1_id, var1_id, count, attr_entries);

    if (rc == RC_OK) {
        testing_log << "attributes associated with txn_id: " << txn_id << " and type " << type1_name << " ver " << type1_version <<
        " and var " << var1_name << " ver " << var1_version;
        testing_log << "\n";
        if(testing_logging || (zero_rank_logging && rank == 0)) {
            print_attribute_list (count, attr_entries);
        }

        print_attr_data(count, attr_entries);
    }
    else {
        error_log << "Error getting the matching attribute list. Proceeding" << endl;
    }

//-------------------------------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------------------------------
//TESTING ACTIVATE AND PROCESSING FOR TYPE
    testing_log << "\nBeginning the activate and process portion of the type and attribute testing \n";

    uint64_t new_txn_id = 107;

    uint64_t dataset_id_new; 

    std::string dataset_name_new = "dataset_new";
    std::string dataset_path_new = "/dataset_new";
    uint64_t dataset_timestep_new = 975312468;
    std::string dataset_rank_to_dims_funct_new = "FIX ME NEW!"; // FIX!

    rc = metadata_create_dataset (server, dataset_id_new, new_txn_id, dataset_name_new, dataset_path_new, 
        dataset_timestep_new, npx1, npy1, npz1, dataset_rank_to_dims_funct_new);
    if (rc != RC_OK) {
        error_log << "Error. Was unable to create the new dataset. Exiting \n";
        return RC_ERR;
    }
    else {
        md_log( "Second dataset id is " + to_string(dataset_id_new ) );
    }

    //note: am testing with a type of the new_txn_id with and without the same dataset id 
    //the same dataset id is used to confirm that the new txn id is properly activated in catalog type functions (which require a dataset id)
    //a different dataset id is used to confirm that all types associated with a dataset are deleted when the dataset is deleted
    uint64_t new_type_id;
    uint64_t new_type_id1;
    uint64_t new_attr_id;
    uint64_t new_attr_id2;

    new_type.txn_id = new_txn_id;

    new_type.version = 2;
    rc = metadata_create_type (server, new_type_id, new_type);
    if (rc != RC_OK) {
        error_log << "Error. Was unable to create the new type. Proceeding" << endl;
    }

    // new_type.version = 3;
    new_type.dataset_id = dataset_id_new;
    rc = metadata_create_type (server, new_type_id1, new_type);
    if (rc != RC_OK) {
        error_log << "Error. Was unable to create the new type. Proceeding" << endl;
    }

    uint64_t new_var_id;
    uint64_t new_var_id1;

    //note: am testing with a var of the new_txn_id with and without the same dataset id 
    //the same dataset id is used to confirm that the new txn id is properly activated in catalog var functions (which require a dataset id)
    //a different dataset id is used to confirm that all vars associated with a dataset are deleted when the dataset is deleted
    new_var.dataset_id = dataset0_id;
    new_var.name = var1_name;
    new_var.path = var1_path;
    new_var.version = 2;
    new_var.data_size = type1;
    new_var.num_dims = 3;
    new_var.txn_id = new_txn_id;
    dims [0]. min = 1;
    dims [0]. max = 50;
    dims [1]. min = 1;
    dims [1]. max = 50;
    dims [2]. min = 1;
    dims [2]. max = 50;
    new_var.dims = std::vector<md_dim_bounds>(dims, dims + new_var.num_dims );
    extreme_debug_log << "about to create new var \n";
    rc = metadata_create_var (server, new_var_id, new_var);
    if (rc != RC_OK) {
        error_log << "Error. Was unable to create the new var. Exiting \n";
        return RC_ERR;
    }

    new_var.dataset_id = dataset_id_new;
    rc = metadata_create_var (server, new_var_id1, new_var);
    if (rc != RC_OK) {
        error_log << "Error. Was unable to create the new var. Exiting \n";
        return RC_ERR;
    }



    md_catalog_attribute_entry new_attr;
    new_attr.txn_id = new_txn_id;
    new_attr.dataset_id = dataset0_id;
    new_attr.type_id = new_type_id;
    new_attr.var_id = var1_id;
    new_attr.dims = std::vector<md_dim_bounds>(dims, dims + new_num_dims);
    new_attr.num_dims = new_num_dims;

    stringstream ss_new;
    string new_test_string = "act_test";
    int new_test_int= 500;
    boost::archive::text_oarchive oa_new(ss_new);
    oa_new << new_test_string;
    oa_new << new_test_int;
    std::string new_serial_str = ss_new.str();

    new_attr.data = new_serial_str;

    rc = metadata_insert_attribute_by_dims (server, new_attr_id, new_attr);
    if (rc != RC_OK) {
        error_log << "Error. Was unable to insert the new attribute. Proceeding" << endl;
    }

    md_log( "New attribute id is " + to_string(new_attr_id) );

    new_attr.var_id = new_var_id1;
    new_attr.type_id = new_type_id1;
    new_attr.dataset_id = dataset_id_new;

    rc = metadata_insert_attribute_by_dims (server, new_attr_id2, new_attr);
    if (rc != RC_OK) {
        error_log << "Error. Was unable to insert the new attribute. Proceeding" << endl;
    }

    md_log( "New new attribute id is " + to_string(new_attr_id2) );


    rc = metadata_activate_type(server, new_txn_id);
    if (rc == RC_OK) {
        testing_log << "just activated the new type  \n";
    }
    else {
        error_log << "Error. Was unable to activate the new type. Exiting \n";
        return RC_ERR;
    }

    //returns information about the types associated with the given txn_id 
    rc = metadata_catalog_type (server, txn_id, dataset0_id, count, type_entries);

    if (rc == RC_OK) {
        md_log( "now the number of types is " + to_string(count) );

        testing_log << "new type catalog: \n";
        if(testing_logging || (zero_rank_logging && rank == 0)) {
            print_type_catalog (count, type_entries);
        }
    }
    else {
        error_log << "Error cataloging the new set of type entries. Proceeding" << endl;
    }

    rc = metadata_processing_type(server, new_txn_id);

    if (rc == RC_OK) {
        testing_log << "just deactivated the new type \n";
    }
    else {
        error_log << "Error. Was unable to deactivate the new type. Exiting \n";
        return RC_ERR;
    }

        //returns information about the types associated with the given txn_id 
    rc = metadata_catalog_type (server, txn_id, dataset0_id, count, type_entries);

    if (rc == RC_OK) {
        md_log( "now the number of types is " + to_string(count) );

        testing_log << "new type catalog: \n";
        if(testing_logging || (zero_rank_logging && rank == 0)) {
            print_type_catalog (count, type_entries);
        }
    }
    else {
        error_log << "Error cataloging the new set of type entries. Proceeding" << endl;
    }


    rc = metadata_activate_attribute(server, new_txn_id);
    if (rc == RC_OK) {
        testing_log << "just activated the new attribute \n";
    }
    else {
        error_log << "Error. Was unable to activate the new attribute. Exiting \n";
        return RC_ERR;
    }

    rc = metadata_catalog_all_attributes (server, txn_id, dataset0_id, count, attr_entries);

    if (rc == RC_OK) {
        testing_log << "attributes with txn_id: " << txn_id << " (count: " << count << ") \n";
        if(testing_logging || (zero_rank_logging && rank == 0)) {
            print_attribute_list (count, attr_entries);
        }

        print_attr_data(count, attr_entries);

    }
    else {
        error_log << "Error getting the given attribute list. Proceeding" << endl;
    }    

    rc = metadata_catalog_all_attributes_with_type_by_id (server, txn_id, dataset0_id, new_type_id, count, attr_entries);

    if (rc == RC_OK) {
        testing_log << "attributes with txn_id: " << txn_id << "and type with name: " 
              << new_type.name << " ver " << new_type.version << " (count: " << count << ") \n";
        if(testing_logging || (zero_rank_logging && rank == 0)) {
            print_attribute_list (count, attr_entries);
        }

        print_attr_data(count, attr_entries);

    }
    else {
        error_log << "Error getting the given attribute list. Proceeding" << endl;
    }    

    rc = metadata_catalog_all_attributes_with_type_var_by_id (server, txn_id, dataset0_id, new_type_id, var1_id, count, attr_entries);

    if (rc == RC_OK) {
        testing_log << "attributes associated with txn_id: " << txn_id << " and type " << new_type.name << " ver " << new_type.version <<
        " and var " << var1_name << " ver " << var1_version;
        if(testing_logging || (zero_rank_logging && rank == 0)) {
            print_attribute_list (count, attr_entries);
        }

        print_attr_data(count, attr_entries);

    }
    else {
        error_log << "Error getting the given attribute list. Proceeding" << endl;
    }    


    rc = metadata_catalog_all_attributes_with_dims(server, txn_id, dataset0_id, new_num_dims, vect_dims, count, attr_entries);

    if (rc == RC_OK) {
        testing_log << "attributes associated with txn_id: " << txn_id << " overlapping with dims";
        for(int j=0; j< new_num_dims; j++) {
            testing_log << " d" << j << "_min: " << dims [j].min;
            testing_log << " d" << j << "_max: " << dims [j].max;               
        }
        testing_log << "\n";
        if(testing_logging || (zero_rank_logging && rank == 0)) {
            print_attribute_list (count, attr_entries);
        }

        print_attr_data(count, attr_entries);
    }
    else {
        error_log << "Error getting the matching attribute list. Proceeding" << endl;
    }



   rc = metadata_catalog_all_attributes_with_type_dims_by_id(server, txn_id, dataset0_id, type0_id, new_num_dims, vect_dims, count, attr_entries);

    if (rc == RC_OK) {
        testing_log << "attributes associated with txn_id: " << txn_id << " and type " << type0_name << " ver " << type0_version << 
            " overlapping with dims";
        for(int j=0; j< new_num_dims; j++) {
            testing_log << " d" << j << "_min: " << dims [j].min;
            testing_log << " d" << j << "_max: " << dims [j].max;               
        }
        testing_log << "\n";
        if(testing_logging || (zero_rank_logging && rank == 0)) {
            print_attribute_list (count, attr_entries);
        }

        print_attr_data(count, attr_entries);
    }
    else {
        error_log << "Error getting the matching attribute list. Proceeding" << endl;
    }



   rc = metadata_catalog_all_attributes_with_var_dims_by_id(server, txn_id, dataset0_id, var0_id, new_num_dims, vect_dims, count, attr_entries);

    if (rc == RC_OK) {
        testing_log << "attributes associated with txn_id: " << txn_id << " and var " << var0_name << " ver " << var0_version << 
            " overlapping with dims";
        for(int j=0; j< new_num_dims; j++) {
            testing_log << " d" << j << "_min: " << dims [j].min;
            testing_log << " d" << j << "_max: " << dims [j].max;               
        }
        testing_log << "\n";
        if(testing_logging || (zero_rank_logging && rank == 0)) {
            print_attribute_list (count, attr_entries);
        }

        print_attr_data(count, attr_entries);
    }
    else {
        error_log << "Error getting the matching attribute list. Proceeding" << endl;
    }


   rc = metadata_catalog_all_attributes_with_type_var_dims_by_id(server, txn_id, dataset0_id, type0_id, var0_id, new_num_dims, vect_dims, count, attr_entries);

    if (rc == RC_OK) {
        testing_log << "attributes associated with txn_id: " << txn_id << " and type " << type0_name << " ver " << type0_version <<
        " and var " << var0_name << " ver " << var0_version << " overlapping with dims";
        for(int j=0; j< new_num_dims; j++) {
            testing_log << " d" << j << "_min: " << dims [j].min;
            testing_log << " d" << j << "_max: " << dims [j].max;               
        }
        testing_log << "\n";
        if(testing_logging || (zero_rank_logging && rank == 0)) {
            print_attribute_list (count, attr_entries);
        }

        print_attr_data(count, attr_entries);
    }
    else {
        error_log << "Error getting the matching attribute list. Proceeding" << endl;
    }


    rc = metadata_processing_attribute(server, new_txn_id);

    if (rc == RC_OK) {
        testing_log << "just deactivated the new attribute \n";
    }
    else {
        error_log << "Error. Was unable to deactivate the new attribute. Exiting \n";
        return RC_ERR;
    }
  
    rc = metadata_catalog_all_attributes (server, txn_id, dataset0_id, count, attr_entries);

    if (rc == RC_OK) {
        testing_log << "attributes with txn_id: " << txn_id << " (count: " << count << ") \n";
        if(testing_logging || (zero_rank_logging && rank == 0)) {
            print_attribute_list (count, attr_entries);
        }

        print_attr_data(count, attr_entries);

    }
    else {
        error_log << "Error getting the given attribute list. Proceeding" << endl;
    }    

    rc = metadata_catalog_all_attributes_with_type_by_id (server, txn_id, dataset0_id, new_type_id, count, attr_entries);

    if (rc == RC_OK) {
        testing_log << "attributes with  txn_id: " << txn_id << " name: " 
              << new_type.name << " ver " << new_type.version << " (count: " << count << ") \n";
        if(testing_logging || (zero_rank_logging && rank == 0)) {
            print_attribute_list (count, attr_entries);
        }

        print_attr_data(count, attr_entries);

    }
    else {
        error_log << "Error getting the given attribute list. Proceeding" << endl;
    }    

    rc = metadata_catalog_all_attributes_with_type_var_by_id (server, txn_id, dataset0_id, new_type_id, var1_id, count, attr_entries);

    if (rc == RC_OK) {
        testing_log << "attributes associated with txn_id: " << txn_id << " and type " << new_type.name << " ver " << new_type.version <<
        " and var " << var1_name << " ver " << var1_version << ": \n";
        if(testing_logging || (zero_rank_logging && rank == 0)) {
            print_attribute_list (count, attr_entries);
        }

        print_attr_data(count, attr_entries);

    }
    else {
        error_log << "Error getting the given attribute list. Proceeding" << endl;
    }    


    rc = metadata_catalog_all_attributes_with_dims(server, txn_id, dataset0_id, new_num_dims, vect_dims, count, attr_entries);

    if (rc == RC_OK) {
        testing_log << "attributes associated with txn_id: " << txn_id << " overlapping with dims";
        for(int j=0; j< new_num_dims; j++) {
            testing_log << " d" << j << "_min: " << dims [j].min;
            testing_log << " d" << j << "_max: " << dims [j].max;               
        }
        testing_log << "\n";
        if(testing_logging || (zero_rank_logging && rank == 0)) {
            print_attribute_list (count, attr_entries);
        }

        print_attr_data(count, attr_entries);
    }
    else {
        error_log << "Error getting the matching attribute list. Proceeding" << endl;
    }



   rc = metadata_catalog_all_attributes_with_type_dims_by_id(server, txn_id, dataset0_id, type0_id, new_num_dims, vect_dims, count, attr_entries);

    if (rc == RC_OK) {
        testing_log << "attributes associated with txn_id: " << txn_id << " and type " << type0_name << " ver " << type0_version << 
            " overlapping with dims";
        for(int j=0; j< new_num_dims; j++) {
            testing_log << " d" << j << "_min: " << dims [j].min;
            testing_log << " d" << j << "_max: " << dims [j].max;               
        }
        testing_log << "\n";
        if(testing_logging || (zero_rank_logging && rank == 0)) {
            print_attribute_list (count, attr_entries);
        }

        print_attr_data(count, attr_entries);
    }
    else {
        error_log << "Error getting the matching attribute list. Proceeding" << endl;
    }



   rc = metadata_catalog_all_attributes_with_var_dims_by_id(server, txn_id, dataset0_id, var0_id, new_num_dims, vect_dims, count, attr_entries);

    if (rc == RC_OK) {
        testing_log << "attributes associated with txn_id: " << txn_id << " and var " << var0_name << " ver " << var0_version << 
            " overlapping with dims";
        for(int j=0; j< new_num_dims; j++) {
            testing_log << " d" << j << "_min: " << dims [j].min;
            testing_log << " d" << j << "_max: " << dims [j].max;               
        }
        testing_log << "\n";
        if(testing_logging || (zero_rank_logging && rank == 0)) {
            print_attribute_list (count, attr_entries);
        }

        print_attr_data(count, attr_entries);
    }
    else {
        error_log << "Error getting the matching attribute list. Proceeding" << endl;
    }


   rc = metadata_catalog_all_attributes_with_type_var_dims_by_id(server, txn_id, dataset0_id, type0_id, var0_id, new_num_dims, vect_dims, count, attr_entries);

    if (rc == RC_OK) {
        testing_log << "attributes associated with txn_id: " << txn_id << " and type " << type0_name << " ver " << type0_version <<
        " and var " << var0_name << " ver " << var0_version << " overlapping with dims";
        for(int j=0; j< new_num_dims; j++) {
            testing_log << " d" << j << "_min: " << dims [j].min;
            testing_log << " d" << j << "_max: " << dims [j].max;               
        }
        testing_log << "\n";
        if(testing_logging || (zero_rank_logging && rank == 0)) {
            print_attribute_list (count, attr_entries);
        }

        print_attr_data(count, attr_entries);
    }
    else {
        error_log << "Error getting the matching attribute list. Proceeding" << endl;
    }


    testing_log << "Done with activate and process portion of the type and attribute testing \n \n";

//-------------------------------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------------------------------


    vector<md_catalog_dataset_entry> dataset_entries;

    rc = metadata_catalog_all_attributes (server, txn_id, dataset0_id, count, attr_entries);

    if (rc == RC_OK) {
        testing_log << "attributes with txn_id: " << txn_id << " (count: " << count << ") \n";
        if(testing_logging || (zero_rank_logging && rank == 0)) {
            print_attribute_list (count, attr_entries);
        }

        print_attr_data(count, attr_entries);

    }
    else {
        error_log << "Error getting the given attribute list. Proceeding" << endl;
    }    


    testing_log << "about to try deleting type id " << type0_id << " and dataset_id: " << dataset0_id << " with name " << type0_name <<  
            " and version " <<  type0_version << endl;

    rc = metadata_delete_type_by_id (server, dataset0_id, type0_id);

    if (rc == RC_OK) {
        //returns information about the types associated with the given txn_id 
        rc = metadata_catalog_type (server, txn_id, dataset0_id, count, type_entries);

        if (rc == RC_OK) {
            md_log( "now the number of types is " + to_string(count) );

            testing_log << "new type catalog: \n";
            if(testing_logging || (zero_rank_logging && rank == 0)) {
                print_type_catalog (count, type_entries);
            }
        }
        else {
            error_log << "Error cataloging the new set of type entries. Proceeding" << endl;
        }
       

        rc = metadata_catalog_all_attributes (server, txn_id, dataset0_id, count, attr_entries);

        if (rc == RC_OK) {
            testing_log << "attributes with txn_id: " << txn_id << " (count: " << count << ") \n";
            if(testing_logging || (zero_rank_logging && rank == 0)) {
                print_attribute_list (count, attr_entries);
            }

            print_attr_data(count, attr_entries);

        }
        else {
            error_log << "Error getting the given attribute list. Proceeding" << endl;
        }    

    }
    else {
        error_log << "Error deleting the first type. Proceeding" << endl;
    }
    

    testing_log << "Done with the type and attribute portion of the testing \n \n";

//-------------------------------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------------------------------
//VARIABLE TESTING  

    //tests metadata_catalog
    std::vector<md_catalog_var_entry> var_entries;

    testing_log << "Beginning the variable testing \n";


    //returns information about the variables associated with the given txn_id 
    rc = metadata_catalog_var(server, txn_id, dataset0_id, count, var_entries);
    if (rc != RC_OK) {
        error_log << "Error cataloging the first set of var_entries. Proceeding" << endl;
    }

    md_log( "num items is  " + to_string(count) + " num var_entries is " + to_string(var_entries.size()) );

    testing_log << "catalog: \n";
    if(testing_logging || (zero_rank_logging && rank == 0)) {
        print_var_catalog (count, var_entries);
    }


    // // search for chunks that are associated with the given var and have dimensions
    // //within the given range
    // new_var.var_id = var1_id;
    // new_var.num_dims=3;
    // new_var.txn_id = txn_id;
    // dims [0].min = 8;
    // dims [0].max = 25;
    // dims [1].min = 7;
    // dims [1].max = 26;
    // dims [2].min = 6;
    // dims [2].max = 27;
    // new_var.dims = std::vector<md_dim_bounds>(dims, dims + new_var.num_dims );

    md_log( "Got to delete var " );

    //Delete the given var 
    testing_log << "deleting var_id: " << var0_id << " name: " << var0_name << " path: " 
              << var0_path << " version: " << var0_version << endl;
    rc = metadata_delete_var_by_id (server, dataset0_id, var0_id);

    //returns information about the variables associated with the given txn_id 
    //should only return info about var1_id now that var1_id is deleted
    rc = metadata_catalog_var(server, txn_id, dataset0_id, count, var_entries);
    if (rc != RC_OK) {
        error_log << "Error cataloging the second set of var_entries. Proceeding" << endl;
    }

    testing_log << "catalog: \n";
    if(testing_logging || (zero_rank_logging && rank == 0)) {
        print_var_catalog (count, var_entries);
    }

    rc = metadata_catalog_all_attributes (server, txn_id, dataset0_id, count, attr_entries);

    if (rc == RC_OK) {
        testing_log << "attributes with txn_id: " << txn_id << " (count: " << count << ") \n";
        if(testing_logging || (zero_rank_logging && rank == 0)) {
            print_attribute_list (count, attr_entries);
        }

        print_attr_data(count, attr_entries);

    }
    else {
        error_log << "Error getting the given attribute list. Proceeding" << endl;
    }    
//-------------------------------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------------------------------
//TESTING ACTIVATE AND PROCESSING FOR VAR
    //creates a variable, assigning it a location in the simulation space


    testing_log << "\nBeginning the activate and process portion of the var testing \n";



    rc = metadata_activate_dataset(server, new_txn_id);
    if (rc == RC_OK) {
        testing_log << "just activated the new dataset  \n";
    }
    else {
        error_log << "Error. Was unable to activated the dataset Exiting \n";
        return RC_ERR;
    }

    //since the new txn_id is activated, both the old and the new dataset should show
    rc = metadata_catalog_dataset (server, txn_id, count, dataset_entries);
    if (rc != RC_OK) {
        error_log << "Error cataloging the activated set of dataset_entries. Proceeding" << endl;
    }
    testing_log << "catalog: \n";
    if(testing_logging || (zero_rank_logging && rank == 0)) {
        print_dataset_catalog (count, dataset_entries);
    }

    //since the new txn_id is activated, both the old (x2) and the new vars should show
    rc = metadata_catalog_var(server, txn_id, dataset0_id, count, var_entries);
     if (rc != RC_OK) {
        error_log << "Error cataloging the activated set of var_entries. Proceeding" << endl;
    }

    testing_log << "catalog: \n";
    if(testing_logging || (zero_rank_logging && rank == 0)) {
        print_var_catalog (count, var_entries);
    }   

    rc = metadata_processing_dataset(server, new_txn_id);
    if (rc == RC_OK) {
        testing_log << "just deactivated the new dataset \n";
    }
    else {
        error_log << "Error. Was unable to deactivate the new dataset. Exiting \n";
        return RC_ERR;
    }

    rc = metadata_catalog_dataset (server, txn_id, count, dataset_entries);
    if (rc != RC_OK) {
        error_log << "Error cataloging the deactivated set of dataset entries. Proceeding" << endl;
    }
    testing_log << "catalog: \n";
    if(testing_logging || (zero_rank_logging && rank == 0)) {
        print_dataset_catalog (count, dataset_entries);
    }


    rc = metadata_catalog_var(server, txn_id, dataset0_id, count, var_entries);
     if (rc != RC_OK) {
        error_log << "Error cataloging the deactivated set of var_entries. Proceeding" << endl;
    }

    testing_log << "catalog: \n";
    if(testing_logging || (zero_rank_logging && rank == 0)) {
        print_var_catalog (count, var_entries);
    }   
    
    testing_log << "Done with activate and process portion of the var testing \n \n";

//-------------------------------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------------------------------



    // //Delete the given var and all of its associated chunks
    // testing_log << "deleting var_id: " << var1_id << " name: " << var1_name << " path: " 
    //           << var1_path << " version: " << var1_version << endl;

    // rc = metadata_delete_var_by_id (server, var0_id);
    // if (rc != RC_OK) {
    //     error_log << "Error deleting the first var. Proceeding" << endl;
    // }

    testing_log << "deleting dataset: " << dataset0_id << " name: " << dataset0_name << " path: " 
              << dataset0_path << " timestep: " << dataset0_timestep << endl;

    rc = metadata_delete_dataset_by_id (server, dataset0_id);

    //Should be empty now 
    rc = metadata_catalog_dataset (server, txn_id, count, dataset_entries);
    if (rc != RC_OK) {
        error_log << "Error cataloging the post deletion set of dataset entries. Proceeding" << endl;
    }
    testing_log << "catalog for txn_id " << txn_id << ": \n";
    if(testing_logging || (zero_rank_logging && rank == 0)) {
        print_dataset_catalog (count, dataset_entries);
    }

    //Should be empty now 
    rc = metadata_catalog_var(server, txn_id, dataset0_id, count, var_entries);
    if (rc != RC_OK) {
        error_log << "Error cataloging the post deletion set of var_entries. Proceeding" << endl;
    }
    testing_log << "catalog for txn_id " << txn_id << ": \n";
    if(testing_logging || (zero_rank_logging && rank == 0)) {
        print_var_catalog (count, var_entries);
    }

    //Should be empty now 
    rc = metadata_catalog_type (server, txn_id, dataset0_id, count, type_entries);
    if (rc == RC_OK) {
        md_log( "now the number of types for txn_id " + to_string(txn_id) + " is: " + to_string(count) );

        testing_log << "new type catalog: \n";
        if(testing_logging || (zero_rank_logging && rank == 0)) {
            print_type_catalog (count, type_entries);
        }
    }
    else {
        error_log << "Error cataloging the new set of type entries. Proceeding" << endl;
    }

    //Should be empty now 
    rc = metadata_catalog_all_attributes (server, txn_id, dataset0_id, count, attr_entries);
    if (rc == RC_OK) {
        testing_log << "attributes associated with txn_id: " << txn_id;
        testing_log << "\n";
        if(testing_logging || (zero_rank_logging && rank == 0)) {
            print_attribute_list (count, attr_entries);
        }

        print_attr_data(count, attr_entries);
    }
    else {
        error_log << "Error getting the matching attribute list. Proceeding" << endl;
    }


////////////new_dataset
    rc = metadata_catalog_dataset (server, new_txn_id, count, dataset_entries);
    if (rc != RC_OK) {
        error_log << "Error cataloging the new set of dataset entries. Proceeding" << endl;
    }
    testing_log << "dataset catalog for txn_id " << new_txn_id << ": \n";
    if(testing_logging || (zero_rank_logging && rank == 0)) {
        print_dataset_catalog (count, dataset_entries);
    }

    rc = metadata_catalog_var(server, new_txn_id, dataset_id_new, count, var_entries);
    if (rc != RC_OK) {
        error_log << "Error cataloging the new set of var_entries. Proceeding" << endl;
    }
    testing_log << "var catalog for txn_id " << new_txn_id << ": \n";
    if(testing_logging || (zero_rank_logging && rank == 0)) {
        print_var_catalog (count, var_entries);
    }

    rc = metadata_catalog_type (server, new_txn_id, dataset_id_new, count, type_entries);
    if (rc == RC_OK) {
        testing_log << "type catalog for txn_id " << new_txn_id << ": \n";
        if(testing_logging || (zero_rank_logging && rank == 0)) {
            print_type_catalog (count, type_entries);
        }
    }
    else {
        error_log << "Error cataloging the new set of type entries. Proceeding" << endl;
    }

    rc = metadata_catalog_all_attributes (server, new_txn_id, dataset_id_new, count, attr_entries);
    if (rc == RC_OK) {
        testing_log << "all attributes associated with txn_id: " << new_txn_id << " and dataset_id: " << dataset_id_new << endl;
        if(testing_logging || (zero_rank_logging && rank == 0)) {
            print_attribute_list (count, attr_entries);
        }

        print_attr_data(count, attr_entries);
    }
    else {
        error_log << "Error getting the matching attribute list. Proceeding" << endl;
    }


    testing_log << "deleting dataset: " << dataset_id_new << " name: " << dataset_name_new << " path: " 
              << dataset_path_new << " timestep: " << dataset_timestep_new << endl;


    rc = metadata_delete_dataset_by_id (server, dataset_id_new);



    //Should be empty now 
    rc = metadata_catalog_dataset (server, new_txn_id, count, dataset_entries);
    if (rc != RC_OK) {
        error_log << "Error cataloging the post deletion set of dataset entries. Proceeding" << endl;
    }
    testing_log << "catalog: \n";
    if(testing_logging || (zero_rank_logging && rank == 0)) {
        print_dataset_catalog (count, dataset_entries);
    }

    //Should be empty now 
    rc = metadata_catalog_var(server, new_txn_id, dataset0_id, count, var_entries);
    if (rc != RC_OK) {
        error_log << "Error cataloging the third set of var_entries. Proceeding" << endl;
    }
    testing_log << "catalog: \n";
    if(testing_logging || (zero_rank_logging && rank == 0)) {
        print_var_catalog (count, var_entries);
    }

    //Should be empty now 
    rc = metadata_catalog_type (server, new_txn_id, dataset0_id, count, type_entries);
    if (rc == RC_OK) {
        md_log( "now the number of types is " + to_string(count) );

        testing_log << "new type catalog: \n";
        if(testing_logging || (zero_rank_logging && rank == 0)) {
            print_type_catalog (count, type_entries);
        }
    }
    else {
        error_log << "Error cataloging the new set of type entries. Proceeding" << endl;
    }

    //Should be empty now 
    rc = metadata_catalog_all_attributes (server, new_txn_id, dataset0_id, count, attr_entries);
    if (rc == RC_OK) {
        testing_log << "attributes associated with txn_id: " << txn_id;
        testing_log << "\n";
        if(testing_logging || (zero_rank_logging && rank == 0)) {
            print_attribute_list (count, attr_entries);
        }

        print_attr_data(count, attr_entries);
    }
    else {
        error_log << "Error getting the matching attribute list. Proceeding" << endl;
    }

//-------------------------------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------------------------------
    
    testing_log << "done with testing\n";

    if( rank < dir.members.size()) {
        metadata_finalize_server(server);    
        // if(rank == 0) {
        //     metadata_finalize_server(dirman);
        // }
    }

    testing_log << "just finalized server \n";

    return rc;


}


static int setup_dirman(const string &dirman_hexid, const string &dir_path, 
                        faodel::DirectoryInfo &dir)
{
    int rc;
    bool ok;

 //Add the directory manager's URL to the config string so the clients know
    //Where to access the directory ???
    md_log( "dirman root url is" + dirman_hexid );
    faodel::Configuration config(default_config_string);
    config.Append("dirman.root_node", dirman_hexid);

    config.AppendFromReferences();

    //Start the mpi/client processes, and launch the directory manager ???
    // G2.StartAll(argc, argv, config);
    faodel::bootstrap::Start(config, opbox::bootstrap);

    //-------------------------------------------
    //TODO: This connect is temporarily necessary
    faodel::nodeid_t dirman_nodeid(dirman_hexid);
    md_log("Dirman node ID " + dirman_nodeid.GetHex()+" " +
       dirman_nodeid.GetIP() + " port " + dirman_nodeid.GetPort());
    opbox::net::peer_ptr_t peer;
    dirman.name_and_node.node = dirman_nodeid;
    rc = opbox::net::Connect(&dirman.peer_ptr, dirman.name_and_node.node);
    assert(rc==0 && "could not connect");
    //-------------------------------------------

     //Have all nodes join the directory
    ok = dirman::Getfaodel::DirectoryInfo(faodel::ResourceURL(dir_path), &dir);
    assert(ok && "Could not get info abuot the directory?");

    if( dir.members.size()==0 ) {
        error_log << "Error. There are not any servers initialized \n";
        return RC_ERR;
    }

    md_log("done initing dirman");
    return rc;
}


 static void setup_server(md_server &server, int rank, const faodel::DirectoryInfo &dir) {
    int server_id;
    extreme_debug_log << "rank is " << rank << endl;
    server_id = rank % dir.members.size(); 
    md_log("server_id is " + to_string(server_id));
    server.name_and_node = dir.members.at(server_id);
    server.URL = server.name_and_node.node.GetHex();

    opbox::net::Connect(&server.peer_ptr, server.name_and_node.node);

    md_log("dir.members.size = " + to_string(dir.members.size()));
}


void print_attr_data(uint32_t count, std::vector<md_catalog_attribute_entry> attr_entries) {

    for (int i=0; i<count; i++) {
        string deserialized_test_string;
        int deserialized_test_int;
        
        stringstream sso;
        sso << attr_entries.at(i).data;
        boost::archive::text_iarchive ia(sso);
        ia >> deserialized_test_string;
        ia >> deserialized_test_int;

        testing_log << "data: string: " << deserialized_test_string << " int: " << deserialized_test_int << endl; 
    }
 }