#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <math.h> 

#include <mpi.h>

#include <my_metadata_client.h>
#include <my_metadata_client_lua_functs.h>#include "dirman/DirMan.hh"
#include <sstream>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <chrono> //needed for high_resolution_clock
#include <thread> //needed for this_thread

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
md_server_type my_server_type ;

std::vector<int> catg_of_time_pts;
std::vector<std::chrono::high_resolution_clock::time_point> time_pts;

//A variable that will keep track of all mpi information

int run_test(uint64_t job_id, int rank, faodel::DirectoryInfo dir, md_server dirman);
// static int setup_dirman(const string &dirman_hostname, const string &dir_path, md_server &dirman, 
//                         faodel::DirectoryInfo &dir, int rank, uint32_t num_servers = 1, int dirman_port = 1990);
static int setup_dirman(const string &dirman_hexid, const string &dir_path, 
                        faodel::DirectoryInfo &dir, md_server &dirman);
static void setup_server(md_server &server, int rank, const faodel::DirectoryInfo &dir);

static bool md_debug = true;
// static bool md_extreme_debug = false;


static void md_log(const std::string &s) {
  if(md_debug) std::cout << s<< std::endl;
}

static bool testing_logging = true;
static bool extreme_debug_logging = false;
static bool zero_rank_logging = false;
static bool error_logging  = true;

static debugLog testing_log = debugLog(testing_logging, zero_rank_logging);
static debugLog error_log = debugLog(error_logging, zero_rank_logging);
static debugLog extreme_debug_log = debugLog(extreme_debug_logging, false);

static bool output_timing = false;


std::map <string, vector<double>> data_outputs;

int catalog_all_var_attributes_with_type_var ( md_server server, md_catalog_type_entry type, md_catalog_var_entry var,
                                                    uint64_t type_id, uint64_t var_id, int num_dims, vector<md_dim_bounds> vect_dims );

int catalog_all_var_attributes ( md_server server, uint64_t run_id, uint64_t timestep_id, uint64_t txn_id, int num_dims, 
                                                vector<md_dim_bounds> vect_dims );

int catalog_all_var_attributes_with_type ( md_server server, uint64_t timestep_id, int num_dims, 
                                                vector<md_dim_bounds> vect_dims, md_catalog_type_entry type, uint64_t type_id );

int catalog_all_var_attributes_with_var ( md_server server, md_catalog_var_entry var, uint64_t var_id, int num_dims, vector<md_dim_bounds> vect_dims );

// int test_activate_run ( md_server server, md_catalog_run_entry prev_run, md_catalog_type_entry prev_type);

int test_activate_run ( md_server server, md_catalog_run_entry prev_run, md_catalog_type_entry prev_type, 
                string rank_to_dims_funct_name, string rank_to_dims_funct_path, 
                string objector_funct_name, string objector_funct_path);

int test_activate_and_delete_timestep ( md_server server, md_catalog_timestep_entry prev_timestep, 
                        md_catalog_type_entry type, md_catalog_var_entry var, md_catalog_var_attribute_entry prev_attr, uint64_t timestep_id, uint64_t type_id,
                        int num_dims, vector<md_dim_bounds> vect_dims);

//fix - should I be putting this in along with activate run? 
int test_activate_and_delete_type ( md_server server, md_catalog_type_entry prev_type, md_catalog_var_attribute_entry prev_attr);

int test_activate_and_delete_var_and_var_attrs ( md_server server, uint64_t type_id, 
                                            md_catalog_type_entry prev_type, md_catalog_var_entry prev_var, int num_dims, vector<md_dim_bounds> vect_dims);

int delete_run (md_server server, uint64_t run_id, uint64_t timestep_id, uint64_t txn_id );

int create_timestep(md_server server, uint64_t run_id, uint64_t timestep_id, string timestep_path, uint64_t txn_id,
                    uint64_t type0_id, uint64_t type1_id, uint64_t &var0_id, uint64_t &var1_id, 
                    md_catalog_var_entry &new_var0, md_catalog_var_entry &new_var1, md_catalog_var_attribute_entry &new_attr);  

int create_run(md_server server, md_catalog_run_entry run, string rank_to_dims_funct_name, string rank_to_dims_funct_path, 
                string objector_funct_name, string objector_funct_path, uint64_t &run_id,
                uint64_t &type0_id, uint64_t &type1_id, md_catalog_type_entry &new_type0, md_catalog_type_entry &new_type1 );

int catalog_all_run_attributes ( md_server server, uint64_t run_id, uint64_t type_id, uint64_t txn_id);

int catalog_all_timestep_attributes ( md_server server, uint64_t run_id, uint64_t timestep_id, uint64_t type_id, uint64_t txn_id);

int catalog_all_timesteps ( md_server server, md_catalog_var_entry var, uint64_t var_id, uint64_t type_id,
                    int num_dims, vector<md_dim_bounds> vect_dims );

int catalog_all_types ( md_server server, md_catalog_var_entry var, uint64_t var_id,
                    int num_dims, vector<md_dim_bounds> vect_dims );

int create_run_attrs(md_server server, uint64_t run_id, uint64_t txn_id,
                    uint64_t type0_id, uint64_t type1_id );   

int create_timestep_attrs(md_server server, uint64_t run_id, uint64_t timestep_id, uint64_t timestep_id2, uint64_t txn_id,
                    uint64_t type0_id, uint64_t type1_id );   

int create_new_timesteps(md_server server, uint64_t run_id, uint64_t run_id2, uint64_t timestep_id, uint64_t timestep_id2, 
                    int64_t type_id, uint64_t var_id, uint64_t txn_id);

int create_new_types(md_server server, uint64_t run_id, uint64_t run_id2, uint64_t timestep_id, uint64_t type_id2, 
                    int64_t type_id, uint64_t var_id, uint64_t txn_id, md_catalog_type_entry prev_type);

int retrieveObjNamesAndDataForAttrCatalog(std::map <string, vector<double>> data_outputs, md_server server, md_catalog_run_entry run, 
                                         uint64_t run_id, uint64_t timestep_id,
                                         uint64_t txn_id,
                                         vector<md_catalog_var_attribute_entry> attr_entries );

int catalog_all_var_attributes_with_var_substr ( md_server server, md_catalog_var_entry var, int num_dims, vector<md_dim_bounds> vect_dims );


int catalog_all_var_attributes_with_type_var_substr ( md_server server, md_catalog_type_entry type, md_catalog_var_entry var,
                                                    uint64_t type_id, int num_dims, vector<md_dim_bounds> vect_dims );

int catalog_all_var_attributes_with_var_substr ( md_server server, md_catalog_var_entry var, int num_dims, vector<md_dim_bounds> vect_dims );

int catalog_all_types_substr ( md_server server, md_catalog_var_entry var, 
                    int num_dims, vector<md_dim_bounds> vect_dims );

int catalog_all_timesteps_substr ( md_server server, md_catalog_var_entry var, uint64_t type_id,
                    int num_dims, vector<md_dim_bounds> vect_dims );

int delete_var_substr (md_server server, uint64_t run_id, uint64_t timestep_id, uint64_t txn_id);

void add_timing_point(int catg) {
    if (output_timing) {
        time_pts.push_back(chrono::high_resolution_clock::now());
        catg_of_time_pts.push_back(catg);
    }
}


md_catalog_run_entry new_run;

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
    extreme_debug_log << "starting \n";

    int rc;
    bool ok;
    int rank;
    int num_servers;
    string dir_path="/metadata/testing";
    md_server dirman;
    
    // faodel::DirectoryInfo dir(dir_path,"Sirius metadata servers");

    faodel::DirectoryInfo dir;

    //make sure the user provided an argument, the progravarm is expecting to receive
    //the directory manager's full URL path
    if (argc != 4)
    {
        error_log << "Usage: " << argv[0] << "job_id, my_server_type, <directory manager hexid> \n";
        return RC_ERR;
    }


    uint64_t job_id = stoull(argv[1]);
    my_server_type  = (md_server_type)stoul(argv[2]);

    extreme_debug_log << "argv[1]: " << argv[1] << endl;
    extreme_debug_log << "argv[2]: " << argv[2] << endl;
    extreme_debug_log << "argv[3]: " << argv[3] << endl;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    if(rank != 0) {
        md_debug = false;
    }
    testing_log.set_rank(rank);

    // rc = setup_dirman(argv[1], dir_path, dirman, dir, rank);
    rc = setup_dirman(argv[3], dir_path, dir, dirman);

    if(rc != RC_OK) {
        goto cleanup;
    }
    num_servers = dir.members.size();

    // //have the processes preform a series of tests of the metadata client functions
    rc = run_test (job_id, rank, dir, dirman);

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
int run_test(uint64_t job_id, int rank, faodel::DirectoryInfo dir, md_server dirman) {
    int rc;
  
    struct md_catalog_var_attribute_entry new_attr;
    struct md_catalog_var_attribute_entry new_attr2;
    struct md_catalog_var_attribute_entry new_attr3;
    struct md_catalog_var_attribute_entry new_attr4;


    struct md_catalog_type_entry new_type0;
    struct md_catalog_type_entry new_type1;
    struct md_catalog_type_entry new_type2;
    struct md_catalog_type_entry new_type3;

    struct md_catalog_var_entry new_var0;
    struct md_catalog_var_entry new_var1;
    struct md_catalog_var_entry new_var2;
    struct md_catalog_var_entry new_var3;

    uint64_t type0_id;
    uint64_t type1_id;
    uint64_t type2_id;
    uint64_t type3_id;
    uint64_t var0_id;
    uint64_t var1_id;

    // uint64_t type_id_new;
    // uint64_t type_id_new2;
    // uint64_t var_id_new;
    // uint64_t var_id_new2;

    //add create new runs and timesteps here
    uint64_t txn_id = 5;


    md_server server;

    uint64_t run0_id;
    uint64_t run1_id; 

    uint64_t timestep0_id = 0;
    uint64_t timestep1_id = 1;


    setup_server(server, rank, dir);

    // rc = metadata_init (txn_id);

    rc = metadata_init (job_id);

    if (rc != RC_OK) {
        error_log << "Error. Was unable to initialize the client. Exitting \n";
        return RC_ERR;
    }

    testing_log << "\nBeginning the testing setup \n";


    // md_catalog_run_entry new_run;
    // new_run.job_id = 1234567;
    new_run.job_id = job_id;
    new_run.name = "XGC";
    // new_run.path = "/XGC";
    new_run.txn_id = txn_id;
    new_run.npx = 10;
    new_run.npy = 10;
    new_run.npz = 10;
    // new_run.rank_to_dims_funct = "FIX ME"; 
    // new_run.objector_funct = "OBJECTOR FIX ME";


    string rank_to_dims_funct_name = "rank_to_bounding_box";
    string rank_to_dims_funct_path = "PATH_TO_EMPRESS/lib_source/lua/lua_function.lua";
    string objector_funct_name = "boundingBoxToObjectNamesAndCounts";
    string objector_funct_path = "PATH_TO_EMPRESS/lib_source/lua/metadata_functs.lua";


    // rc = stringify_function("PATH_TO_EMPRESS/lib_source/lua/lua_function.lua", "rank_to_bounding_box",  new_run.rank_to_dims_funct);
    // if (rc != RC_OK) {
    //     error_log << "Error. Was unable to stringify the rank to bounding box function. Exitting \n";
    //     return RC_ERR;
    // }

    // rc = stringify_function("PATH_TO_EMPRESS/lib_source/lua/metadata_functs.lua", "boundingBoxToObjectNamesAndCounts",  new_run.objector_funct);
    // if (rc != RC_OK) {
    //     error_log << "Error. Was unable to stringify the boundingBoxToObjectNamesAndCounts function. Exitting \n";
    //     return RC_ERR;
    // }


    rc = create_run( server, new_run, rank_to_dims_funct_name, rank_to_dims_funct_path, objector_funct_name, objector_funct_path,
          run0_id, type0_id, type1_id, new_type0, new_type1 );

    rc = stringify_function(rank_to_dims_funct_path, rank_to_dims_funct_name, new_run.rank_to_dims_funct);
    if (rc != RC_OK) {
        error_log << "Error. Was unable to stringify the rank to bounding box function. Exitting \n";
    }


    string timestep0_path = "/" + to_string(timestep0_id);
    string timestep1_path = "/" + to_string(timestep1_id);
   
    md_catalog_timestep_entry new_timestep;
    new_timestep.run_id = run0_id;
    new_timestep.timestep_id = timestep0_id;
    new_timestep.path = timestep0_path;
    new_timestep.txn_id = txn_id;


    rc = register_objector_funct_write (objector_funct_name, objector_funct_path, new_run.job_id);
    if (rc != RC_OK) {
        error_log << "error in registering the objector function in write for run_id: " << run0_id << endl;
    }


    rc = create_timestep(server, run0_id, timestep0_id, timestep0_path, txn_id, type0_id, type1_id, var0_id, var1_id, new_var0, new_var1, new_attr );
    if (rc != RC_OK) {
        error_log << "Error. Was unable to create the first timestep. Exiting \n";
        return RC_ERR;
    }
    rc = metadata_checkpoint_database (server, new_run.job_id, DB_COPY);

    rc = create_timestep(server, run0_id, timestep1_id, timestep1_path, txn_id, type0_id, type1_id, var0_id, var1_id, new_var2, new_var3, new_attr2);
    if (rc != RC_OK) {
        error_log << "Error. Was unable to create the second timestep. Exiting \n";
        return RC_ERR;
    }

    rc = metadata_checkpoint_database (server, new_run.job_id, DB_COPY);


    md_catalog_run_entry new_run2;
    new_run2 = new_run;
    new_run2.name = "XGC";
    // new_run2.path = "/XGC";
    new_run2.txn_id = txn_id;
    new_run2.npx = 100;
    new_run2.npy = 100;
    new_run2.npz = 100;
    // new_run.rank_to_dims_funct = "FIX ME NEW!"; // FIX!
    // new_run.objector_funct = "OBJECTOR FIX ME NEW!"; // FIX!

    // rc = stringify_function("PATH_TO_EMPRESS/lib_source/lua/lua_function.lua", "rank_to_bounding_box",  new_run2.rank_to_dims_funct);
    // if (rc != RC_OK) {
    //     error_log << "Error. Was unable to stringify the rank to bounding box function. Exitting \n";
    //     return RC_ERR;
    // }

    // rc = stringify_function("PATH_TO_EMPRESS/lib_source/lua/metadata_functs.lua", "boundingBoxToObjectNamesAndCounts",  new_run2.objector_funct);
    // if (rc != RC_OK) {
    //     error_log << "Error. Was unable to stringify the boundingBoxToObjectNamesAndCounts function. Exitting \n";
    //     return RC_ERR;
    // }

    if(my_server_type != SERVER_DEDICATED_SQLITE_TRANSACTION_MANAGEMENT_DB_STREAMS) {
        new_run2.job_id = 7654321;
    }
    else {
        new_run2.job_id = job_id;        
    }

    rc = create_run( server, new_run2, rank_to_dims_funct_name, rank_to_dims_funct_path, objector_funct_name,
            objector_funct_path, run1_id, type2_id, type3_id, new_type2, new_type3 );

    rc = register_objector_funct_write (objector_funct_name, objector_funct_path, new_run2.job_id);
    if (rc != RC_OK) {
        error_log << "error in registering the objector function in write for run_id: " << run1_id << endl;
    }


    rc = create_timestep(server, run1_id, timestep0_id, timestep0_path, txn_id, type2_id, type3_id, var0_id, var1_id, new_var2, new_var3, new_attr3 );
    if (rc != RC_OK) {
        error_log << "Error. Was unable to create the first timestep. Exiting \n";
        return RC_ERR;
    }

    rc = create_timestep(server, run1_id, timestep1_id, timestep1_path, txn_id, type2_id, type3_id, var0_id, var1_id, new_var2, new_var3, new_attr4);
    if (rc != RC_OK) {
        error_log << "Error. Was unable to create the second timestep. Exiting \n";
        return RC_ERR;
    }


//-------------------------------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------------------------------
//TYPE AND ATTRIBUTE TESTING

    uint32_t count;
    std::vector<md_catalog_type_entry> type_entries;
    std::vector<md_catalog_var_attribute_entry> attr_entries;
    string deserialized_test_string;
    int deserialized_test_int;

    testing_log << "\nBeginning the type and attribute testing \n" ;

    //returns information about the types associated with the given txn_id 
    rc = metadata_catalog_type (server, run0_id, txn_id, count, type_entries);

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


    uint32_t new_num_dims = 3;
    md_dim_bounds *dims = (md_dim_bounds *) malloc(sizeof(md_dim_bounds) * 3);
    dims [0].min = 15;
    dims [0].max = 25;
    dims [1].min = 15;
    dims [1].max = 25;
    dims [2].min = 15;
    dims [2].max = 25;

    vector<md_dim_bounds> vect_dims = std::vector<md_dim_bounds>(dims, dims + new_num_dims);



    // vector<md_catalog_var_entry> var_entries;
    // rc = metadata_catalog_var (server, run0_id, timestep0_id, txn_id, count, var_entries);

    // if (rc == RC_OK) {
    //     testing_log << "new var catalog: \n";
    //     if(testing_logging || (zero_rank_logging )) {
    //         print_var_catalog (count, var_entries);
    //     }
    // }
    // else {
    //     error_log << "Error cataloging the new set of var entries. Proceeding" << endl;
    // }

    uint32_t temp_new_num_dims = 2;
    md_dim_bounds *temp_dims = (md_dim_bounds *) malloc(sizeof(md_dim_bounds) * temp_new_num_dims);
    temp_dims [0].min = 15;
    temp_dims [0].max = 25;
    temp_dims [1].min = 15;
    temp_dims [1].max = 25;

    vector<md_dim_bounds> temp_vect_dims = std::vector<md_dim_bounds>(temp_dims, temp_dims + temp_new_num_dims);
    free(temp_dims);

    rc = metadata_catalog_all_var_attributes_with_dims (server, run0_id, timestep0_id, txn_id, temp_new_num_dims, temp_vect_dims, count, attr_entries);

    if (rc == RC_OK) {
        testing_log << "using metadata_catalog_all_var_attributes_with_dims: attributes associated with txn_id: " << txn_id << " run_id: " << run0_id << " and timestep_id: " << timestep0_id << " overlapping with dims";
        for(int j=0; j< temp_new_num_dims; j++) {
            testing_log << " d" << j << "_min: " << temp_vect_dims [j].min;
            testing_log << " d" << j << "_max: " << temp_vect_dims [j].max;               
        }
        testing_log << "\n";
        if(testing_logging || (zero_rank_logging && rank == 0)) {
            print_var_attribute_list (count, attr_entries);
        }
        // print_var_attr_data(count, attr_entries);
    }
    else {
        error_log << "Error metadata_catalog_all_var_attributes_with_dims. Proceeding" << endl;
    }
    // rc = retrieveObjNamesAndDataForAttrCatalog(data_outputs, server, new_run, run0_id, new_var1.timestep_id, new_var1.txn_id, attr_entries );
    // if (rc != RC_OK) {
    //     error_log << "error. could not perform retrieveObjNamesAndDataForAttrCatalog \n";
    // }

    // dims [0].min = 5;
    // dims [0].max = 9;
    // dims [1].min = 5;
    // dims [1].max = 9;
    // dims [2].min = 5;
    // dims [2].max = 9;

    // vect_dims = std::vector<md_dim_bounds>(dims, dims + new_num_dims);
    
    // rc = metadata_catalog_all_var_attributes_with_dims(server, run0_id, timestep0_id, txn_id, new_num_dims, vect_dims, count, attr_entries);

    // if (rc == RC_OK) {
    //     testing_log << "attributes associated with txn_id: " << txn_id << " run_id: " << run0_id << " and timestep_id: " << timestep0_id << " overlapping with dims";
    //     for(int j=0; j< new_num_dims; j++) {
    //         testing_log << " d" << j << "_min: " << vect_dims [j].min;
    //         testing_log << " d" << j << "_max: " << vect_dims [j].max;               
    //     }
    //     testing_log << "\n";
    //     if(testing_logging || (zero_rank_logging && rank == 0)) {
    //         print_var_attribute_list (count, attr_entries);
    //     }

    //     print_var_attr_data(count, attr_entries);
    // }
    // else {
    //     error_log << "Error getting the matching attribute list. Proceeding" << endl;
    // }
    // // rc = retrieveObjNamesAndDataForAttrCatalog(data_outputs, server, new_run, run0_id, new_var1.timestep_id, new_var1.txn_id, attr_entries );
    // // if (rc != RC_OK) {
    // //     error_log << "error. could not perform retrieveObjNamesAndDataForAttrCatalog \n";
    // // }

    dims [0].min = 5;
    dims [0].max = 10;
    dims [1].min = 5;
    dims [1].max = 10;
    dims [2].min = 5;
    dims [2].max = 10;

    vect_dims = std::vector<md_dim_bounds>(dims, dims + new_num_dims);
    free(dims);

   rc = catalog_all_var_attributes ( server, run0_id, timestep0_id, txn_id, new_num_dims, vect_dims );
    if (rc != RC_OK) {
        error_log << "Error cataloging all var attributes associated with run_id: " << run0_id << " and timestep_id: " << timestep0_id << " and txn_id: " << txn_id << ". Exiting \n";
        return rc;
    }

   rc = catalog_all_var_attributes_with_type (server, timestep0_id, new_num_dims, vect_dims, 
                                        new_type0, type0_id);
    if (rc != RC_OK) {
        error_log << "Error cataloging all var attributes associated with run_id: " << new_type0.run_id << " and timestep_id: " << timestep0_id <<
             " and txn_id: " << txn_id << " and type_id: " << type0_id << ". Exiting \n";
        return rc;
    }

   rc = catalog_all_var_attributes_with_type (server, timestep0_id, new_num_dims, vect_dims, 
                                        new_type1, type1_id);
    if (rc != RC_OK) {
        error_log << "Error cataloging all var attributes associated with run_id: " << new_type1.run_id << " and timestep_id: " << timestep0_id <<
             " and txn_id: " << txn_id << " and type_id: " << type1_id << ". Exiting \n";
        return rc;
    }

    rc = catalog_all_var_attributes_with_var ( server, new_var0, var0_id, new_num_dims, vect_dims );
    if (rc != RC_OK) {
        error_log << "Error cataloging all var attributes associated with run_id: " << new_var0.run_id << " and timestep_id: " << new_var0.timestep_id <<
             " and txn_id: " << new_var0.txn_id << " and var_id: " << var0_id << ". Exiting \n";
        return rc;
    }


    rc = catalog_all_var_attributes_with_var ( server, new_var1, var1_id, new_num_dims, vect_dims );
    if (rc != RC_OK) {
        error_log << "Error cataloging all var attributes associated with run_id: " << new_var1.run_id << " and timestep_id: " << new_var1.timestep_id <<
             " and txn_id: " << new_var1.txn_id << " and var_id: " << var1_id << ". Exiting \n";
       return rc;
    }

    rc = catalog_all_var_attributes_with_type_var ( server, new_type0, new_var0, type0_id, var0_id, new_num_dims, vect_dims );
    if (rc != RC_OK) {
        error_log << "Error cataloging all var attributes associated with run_id: " << new_var0.run_id << " and timestep_id: " << new_var0.timestep_id <<
             " and txn_id: " << new_var0.txn_id << " and type_id: " << type0_id << " and var_id: " << var0_id << ". Exiting \n";
        return rc;
    }

    rc = catalog_all_var_attributes_with_type_var ( server, new_type0, new_var1, type0_id, var1_id, new_num_dims, vect_dims );
    if (rc != RC_OK) {
        error_log << "Error cataloging all var attributes associated with run_id: " << new_var1.run_id << " and timestep_id: " << new_var1.timestep_id <<
             " and txn_id: " << new_var1.txn_id << " and type_id: " << type0_id << " and var_id: " << var1_id << ". Exiting \n";
        return rc;
    }


    rc = catalog_all_var_attributes_with_type_var ( server, new_type1, new_var0, type1_id, var0_id, new_num_dims, vect_dims );
    if (rc != RC_OK) {
        error_log << "Error cataloging all var attributes associated with run_id: " << new_var0.run_id << " and timestep_id: " << new_var0.timestep_id <<
             " and txn_id: " << new_var0.txn_id << " and type_id: " << type1_id << " and var_id: " << var0_id << ". Exiting \n";
        return rc;
    }


    rc = catalog_all_var_attributes_with_type_var ( server, new_type1, new_var1, type1_id, var1_id, new_num_dims, vect_dims );
    if (rc != RC_OK) {
        error_log << "Error cataloging all var attributes associated with run_id: " << new_var1.run_id << " and timestep_id: " << new_var1.timestep_id <<
             " and txn_id: " << new_var1.txn_id << " and type_id: " << type1_id << " and var_id: " << var1_id << ". Exiting \n";
        return rc;
    }

//-------------------------------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------------------------------
//TESTING VAR SUBSTR OPS

    testing_log << "\nBeginning var substring testing \n";

    uint64_t new_substr_var_id;
    md_catalog_var_entry new_substr_var = new_var1;
    new_substr_var.name = "va";
    new_substr_var.var_id = 2;

    rc = metadata_create_var (server, new_substr_var_id, new_substr_var);
    if (rc != RC_OK) {
        error_log << "Error. Was unable to create the new substr var. Exiting \n";
        return RC_ERR;
    }

    uint64_t attribute_id_new;
    md_catalog_var_attribute_entry new_subst_attr = new_attr;
    new_subst_attr.dims = vect_dims;
    new_subst_attr.var_id = new_substr_var_id;
    testing_log << "new_subst_attr.timestep_id: " << new_subst_attr.timestep_id << " new_subst_attr.txn_id: " << new_subst_attr.txn_id << 
        " new_subst_attr.type_id: " << new_subst_attr.type_id << endl;
    rc = metadata_insert_var_attribute_by_dims(server, attribute_id_new, new_subst_attr);
    if (rc != RC_OK) {
        error_log << "Error. Was unable to insert the new substr attr. Proceeding" << endl;
    }

    rc = catalog_all_var_attributes_with_var_substr ( server, new_var1, new_num_dims, vect_dims );
    if (rc != RC_OK) {
        error_log << "Error with catalog_all_var_attributes_with_var_substr. Exiting \n";
        return rc;
    }


    rc = catalog_all_var_attributes_with_type_var_substr ( server, new_type1, new_var1, type1_id, new_num_dims, vect_dims );
    if (rc != RC_OK) {
        error_log << "Error with catalog_all_var_attributes_with_type_var_substr. Exiting \n";
        return rc;
    }

    uint64_t timestep2_id = 20;
    string timestep2_path = "/" + to_string(timestep2_id);

    rc = metadata_create_timestep (server, timestep2_id, new_timestep.run_id, timestep2_path, new_timestep.txn_id);
    if (rc != RC_OK) {
        error_log << "Error. Was unable to create the new timestep. Proceeding" << endl;
    }

    testing_log << "new timestep. timestep_id: " << timestep2_id << " run_id: " << new_timestep.run_id << " txn_id: " << new_timestep.txn_id << endl;


    uint64_t new_type_id_subst;
    md_catalog_type_entry new_type_substr = new_type1;

    rc = metadata_create_type (server, new_type_id_subst, new_type_substr);
    if (rc != RC_OK) {
        error_log << "Error. Was unable to insert the new substr type. Proceeding" << endl;
    }
    md_log( "New type id is " + to_string(new_type_id_subst ) );

    new_subst_attr.timestep_id = timestep0_id;
    new_subst_attr.type_id = new_type_id_subst;
    rc = metadata_insert_var_attribute_by_dims(server, attribute_id_new, new_subst_attr);
    if (rc != RC_OK) {
        error_log << "Error. Was unable to insert the new substr attr. Proceeding" << endl;
    }

    new_subst_attr.type_id = new_attr.type_id;
    new_subst_attr.timestep_id = timestep2_id;
    rc = metadata_insert_var_attribute_by_dims(server, attribute_id_new, new_subst_attr);
    if (rc != RC_OK) {
        error_log << "Error. Was unable to insert the new substr attr. Proceeding" << endl;
    }


    new_substr_var.timestep_id = timestep2_id;

    rc = metadata_create_var (server, new_substr_var_id, new_substr_var);
    if (rc != RC_OK) {
        error_log << "Error. Was unable to create the new substr var. Exiting \n";
        return RC_ERR;
    }
   rc = catalog_all_var_attributes ( server, new_timestep.run_id, new_subst_attr.timestep_id, txn_id, new_num_dims, vect_dims );
    if (rc != RC_OK) {
        error_log << "Error cataloging all var attributes associated with run_id: " << new_timestep.run_id << " and timestep_id: " << new_subst_attr.timestep_id << " and txn_id: " << txn_id << ". Exiting \n";
        return rc;
    }


    rc = catalog_all_types_substr ( server, new_var1, new_num_dims, vect_dims );
    if (rc != RC_OK) {
        error_log << "Error with catalog_all_types_substr. Exiting \n";
        return rc;
    }

    rc = catalog_all_timesteps_substr ( server, new_var1, new_attr.type_id, new_num_dims, vect_dims );
    if (rc != RC_OK) {
        error_log << "Error with catalog_all_timesteps_substr. Exiting \n";
        return rc;
    }

    rc = delete_var_substr ( server, new_var1.run_id, new_var1.timestep_id, new_var1.txn_id );
    if (rc != RC_OK) {
        error_log << "Error with delete_var_substr. Exiting \n";
        return rc;
    }



// //-------------------------------------------------------------------------------------------------------------------------------------------
// //-------------------------------------------------------------------------------------------------------------------------------------------
// //TESTING ACTIVATE AND PROCESSING FOR TYPE
//     testing_log << "\nBeginning the activate and process portion of the type and attribute testing \n";


    rc = test_activate_and_delete_type ( server, new_type1, new_attr);
    if (rc != RC_OK) {
        // error_log << "Error with the activate and delete type testing. Exiting \n";
        error_log << "Error with the activate and delete type testing. Continuing \n";
        // return rc;
    }
  

//     testing_log << "Done with activate and process portion of the type and attribute testing \n \n";

// //-------------------------------------------------------------------------------------------------------------------------------------------
// //-------------------------------------------------------------------------------------------------------------------------------------------
// //TESTING ACTIVATE AND PROCESSING FOR VAR
//     //creates a variable, assigning it a location in the simulation space


    testing_log << "\nBeginning the activate and process portion of the var, run and timestep testing \n";


    rc = test_activate_and_delete_var_and_var_attrs ( server, type1_id, new_type1, new_var1, new_num_dims, vect_dims);
    if (rc != RC_OK) {
        // error_log << "Error with the activate and delete var and var attributes testing. Exiting \n";
        error_log << "Error with the activate and delete var and var attributes testing. Continuing \n";
        // return rc;
    }


    // rc = test_activate_run ( server, new_run , new_type1);
    // if (rc != RC_OK) {
    //     error_log << "Error with the activate run testing. Exiting \n";
    //     return rc;
    // }
    rc = test_activate_run ( server, new_run , new_type1, rank_to_dims_funct_name, rank_to_dims_funct_path,
            objector_funct_name, objector_funct_path);
    if (rc != RC_OK) {
        error_log << "Error with the activate run testing. Exiting \n";
        return rc;
    }

    rc = test_activate_and_delete_timestep ( server, new_timestep, new_type1, new_var1, new_attr, timestep0_id, type1_id, new_num_dims, vect_dims);
    if (rc != RC_OK) {
        // error_log << "Error with the activate timestep testing. Exiting \n";
        error_log << "Error with the activate timestep testing. Continuing \n";
        // return rc;
    }

    testing_log << "Done with activate and process portion of the var, run and timestep testing \n \n";



// //-------------------------------------------------------------------------------------------------------------------------------------------
// //-------------------------------------------------------------------------------------------------------------------------------------------
// //new testing - includes more robust run and timestep attr testing, finding timesteps that match criteria
// //and finding certain types for a specific timestep


    testing_log << "Beginning new portion of testing \n \n";

    rc = create_run_attrs( server, run0_id, txn_id, type0_id, type1_id );
    rc = catalog_all_run_attributes ( server, run0_id, type1_id, txn_id);

    rc = create_timestep_attrs( server, run0_id, timestep0_id, timestep1_id, txn_id, type0_id, type1_id );
    rc = catalog_all_timestep_attributes ( server, run0_id, timestep0_id, type0_id, txn_id);
    rc = catalog_all_timestep_attributes ( server, run0_id, timestep0_id, type1_id, txn_id);
    rc = catalog_all_timestep_attributes ( server, run0_id, timestep1_id, type0_id, txn_id);
    rc = catalog_all_timestep_attributes ( server, run0_id, timestep1_id, type1_id, txn_id);


    rc = create_new_timesteps( server, run0_id, run1_id, timestep0_id, timestep1_id, type1_id, var1_id, txn_id);
    rc = catalog_all_timesteps ( server, new_var1, var1_id, type1_id, new_num_dims, vect_dims );


    rc = create_new_types ( server, run0_id, run1_id, timestep0_id, timestep1_id, type1_id, var1_id, txn_id, new_type1);
    rc = catalog_all_types ( server, new_var1, var1_id, new_num_dims, vect_dims );

    testing_log << "Done with  new portion of testing \n \n";


//-------------------------------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------------------------------
    rc = delete_run ( server, run0_id, timestep0_id, txn_id );


    // rc = metadata_delete_run_by_id (server, run1_id);



//-------------------------------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------------------------------
    
    testing_log << "done with testing\n";

    if( rank < dir.members.size()) {
        metadata_finalize_server(server);    
        if(rank == 0) {
            metadata_finalize_server(dirman);
        }
    }

    testing_log << "just finalized server \n";

    return rc;


}

// static int setup_dirman(const string &dirman_hostname, const string &dir_path, md_server &dirman, 
//                         faodel::DirectoryInfo &dir, int rank, uint32_t num_servers, int dirman_port) {
//     int rc = RC_ERR;
//     bool ok;

//     opbox::net::peer_ptr_t peer;
//     faodel::nodeid_t dirman_nodeid(dirman_hostname, to_string(dirman_port));
//     dirman.name_and_node.node = dirman_nodeid;

//     faodel::Configuration config(default_config_string);
//     config.Append("dirman.type", "centralized");
//     config.Append("dirman.root_node", dirman_nodeid.GetHex());
//     config.Append("whookie.port", to_string(3000+rank)); //note if you up number of servers you'll want to up this
//     config.AppendFromReferences();
//     //debug_log << "just configged" << endl; 
//     faodel::bootstrap::Start(config, dirman::bootstrap);

//     do {
//         std::this_thread::sleep_for(std::chrono::milliseconds(10));
//         rc = net::Connect(&dirman.peer_ptr, dirman_nodeid); 
//         cout << "could not connect. trying again" << endl;
//     } while (rc != RC_OK);
    

//     ok = dirman::GetDirectoryInfo(faodel::ResourceURL(dir_path), &dir); 
//     // if(!ok) return RC_ERR;
//     assert(ok && "Could not get info about the directory?");

//     // do {
//     //     std::this_thread::sleep_for(std::chrono::milliseconds(10));
//     //     ok = dirman::GetDirectoryInfo(faodel::ResourceURL(dir_path), &dir); 
//     //     // if (!ok) return RC_ERR;
//     //     assert(ok && "Could not get info about the directory?");
//     //     cout << "dir.members.size(): " << dir.members.size() << endl;
//     //     cout << "num_servers: " << num_servers << endl;
//     // } while (dir.members.size() < num_servers);
       
//     return RC_OK;
//  } 


static int setup_dirman(const string &dirman_hexid, const string &dir_path, 
                        faodel::DirectoryInfo &dir, md_server &dirman)
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
    faodel::bootstrap::Start(config, dirman::bootstrap);

    //-------------------------------------------
    //TODO: This connect is temporarily necessary
    faodel::nodeid_t dirman_nodeid(dirman_hexid);
    //extreme_debug_log << "Dirman node ID " << dirman_nodeid.GetHex() << " " << dirman_nodeid.GetIP() << " port " << dirman_nodeid.GetPort() << endl;
    dirman.name_and_node.node = dirman_nodeid;
    //extreme_debug_log << "about to connect peer to node" << endl;
    rc = opbox::net::Connect(&dirman.peer_ptr, dirman.name_and_node.node);
    assert(rc==RC_OK && "could not connect");
    //-------------------------------------------

     //Have all nodes join the directory
    ok = dirman::GetDirectoryInfo(faodel::ResourceURL(dir_path), &dir);
    assert(ok && "Could not get info abuot the directory?");

    if( dir.members.size()==0 ) {
        error_log << "Error. There are not any servers initialized \n";
        return RC_ERR;
    }
    dirman.name_and_node.node = dirman_nodeid;

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

    md_log("dir.children.size = " + to_string(dir.members.size()));
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
    // cout << "when making attr_data, string.size(): " << test_string.size() << endl;
    oa << test_string;
    oa << test_int;
    serial_str = ss.str();
    // cout << "serial_str: " << serial_str << endl;
}

// void make_range_data (auto min_int, auto max_int, string &serial_str) {
// void make_range_data (auto min_int, auto max_int, string &serial_str) {
//     stringstream ss;
//     boost::archive::text_oarchive oa(ss);
//     oa << min_int;
//     oa << max_int;
//     serial_str = ss.str();
// }

// template <class T>
// void make_range_data (T min_int, T max_int, string &serial_str) {
//     stringstream ss;
//     boost::archive::text_oarchive oa(ss);
//     oa << min_int;
//     oa << max_int;
//     serial_str = ss.str();
// }

// static void make_range_data (auto min_int, auto max_int, string &serial_str) {
//     stringstream ss;
//     boost::archive::text_oarchive oa(ss);
//     oa << min_int;
//     oa << max_int;
//     serial_str = ss.str();
// }

// void make_range_data (uint64_t min_int, uint64_t max_int, string &serial_str) {
//     stringstream ss;
//     boost::archive::text_oarchive oa(ss);
//     oa << min_int;
//     oa << max_int;
//     serial_str = ss.str();
// }

template <class T>
void print_attr_data(T attr)
{

    switch(attr.data_type) {
        case ATTR_DATA_TYPE_INT : {
            uint64_t deserialized_test_int;

            stringstream sso;
            sso << attr.data;
            boost::archive::text_iarchive ia(sso);
            ia >> deserialized_test_int;

            testing_log << "data: int: " << deserialized_test_int << endl; 
            break;
        }
        case ATTR_DATA_TYPE_REAL : {
            long double deserialized_test_real;

            stringstream sso;
            sso << attr.data;
            boost::archive::text_iarchive ia(sso);
            ia >> deserialized_test_real;

            testing_log << "data: real: " << deserialized_test_real << endl; 
            break;
        }           
        case ATTR_DATA_TYPE_TEXT : {
                // string deserialized_test_string;
            testing_log << "data: text: " << attr.data << endl; 
            break;
        } 
        case ATTR_DATA_TYPE_BLOB : {
            string deserialized_test_string;
            int deserialized_test_int;

            stringstream sso;
            sso << attr.data;
            boost::archive::text_iarchive ia(sso);
            // cout << "sso: " << sso.str() << endl;
            ia >> deserialized_test_string;
            ia >> deserialized_test_int;

            testing_log << "data: blob: str: " << deserialized_test_string << " int: " << deserialized_test_int << endl; 
            // testing_log << "str length: " << deserialized_test_string.size() << endl; 

            break;
        }         // extreme_debug_log << "serialized var data: " << attr.data << endl;

        // testing_log << "data: string: " << deserialized_test_string << " int: " << deserialized_test_int << endl; 
    }
 }

void print_var_attr_data(md_catalog_var_attribute_entry attr) {
    print_attr_data(attr);
}
void print_timestep_attr_data(md_catalog_timestep_attribute_entry attr) {
    print_attr_data(attr);
}
void print_run_attr_data(md_catalog_run_attribute_entry attr) {
    print_attr_data(attr);
}

 
 ////////////these are just for testing////////////////////////////////////////////////////////////////////////////////////////////////


static void generate_data_for_proc(md_catalog_var_entry var, 
                        int rank, const vector<md_dim_bounds> &bounding_box, vector<double> &data_vct, 
                        uint64_t ndx, uint64_t ndy, uint64_t ndz, uint64_t chunk_vol);

static void generate_data_for_proc2D(md_catalog_var_entry var, 
                        int rank, const vector<md_dim_bounds> &bounding_box, vector<double> &data_vct, 
                        uint64_t ndx, uint64_t ndy, uint64_t chunk_vol);

static void get_obj_lengths(md_catalog_var_entry var, 
                    uint64_t &x_width, uint64_t &last_x_width, uint64_t ndx, uint64_t chunk_size);

int write_output(std::map <string, vector<double>> &data_outputs, md_catalog_run_entry run, 
                md_catalog_var_entry var);

static int retrieve_obj_names_and_data(std::map <string, vector<double>> data_outputs, md_catalog_run_entry run, 
                      md_catalog_var_entry var, vector<md_dim_bounds> bounding_box);

// void retrieve_obj_names_and_data2D(std::map <string, vector<double>> data_outputs, md_catalog_dataset_entry dataset, 
//                       md_catalog_var_entry var, vector<md_dim_bounds> bounding_box);

static void getFirstAndLastIndex(md_catalog_run_entry run, md_catalog_var_entry var, vector<uint64_t> offsets_and_counts, 
                        uint64_t &first_index, uint64_t &last_index);

static void getFirstAndLastIndex2D(md_catalog_run_entry run, md_catalog_var_entry var, vector<uint64_t> offsets_and_counts, 
                        uint64_t &first_index, uint64_t &last_index);


static void generate_data_for_proc(md_catalog_var_entry var, 
                        int rank, const vector<md_dim_bounds> &bounding_box, vector<double> &data_vct, 
                        uint64_t ndx, uint64_t ndy, uint64_t ndz, uint64_t chunk_vol) 
{

    // data_vct.reserve(chunk_vol);
    uint64_t ny = var.dims.at(1).max - var.dims.at(1).min + 1;
    uint64_t nz = var.dims.at(2).max - var.dims.at(2).min + 1;

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

    extreme_debug_log << "z1: " << bounding_box.at(2).min << " y1: " << bounding_box.at(1).min << " x1: " << bounding_box.at(0).min << endl;

    uint64_t x1 = bounding_box.at(0).min;
    uint64_t y1 = bounding_box.at(1).min;
    uint64_t z1 = bounding_box.at(2).min;

    // extreme_debug_log << "chunk vol: " << chunk_vol << endl;

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


static void generate_data_for_proc2D(md_catalog_var_entry var, 
                        int rank, const vector<md_dim_bounds> &bounding_box, vector<double> &data_vct, 
                        uint64_t ndx, uint64_t ndy, uint64_t chunk_vol) 
{
    uint64_t ny = var.dims.at(1).max - var.dims.at(1).min + 1;

    int number = ny;

    int y_digits = 0;
    while (number > 0) {
        number /= 10;
        y_digits++;
    }
    y_digits++;

    extreme_debug_log << "ndx: " << ndx << " ndy: " << ndy <<  endl;

    extreme_debug_log << "y1: " << bounding_box.at(1).min << " x1: " << bounding_box.at(0).min << endl;

    uint64_t x1 = bounding_box.at(0).min;
    uint64_t y1 = bounding_box.at(1).min;

    for(int i=0; i<chunk_vol; i++) {

        uint64_t y = y1 + i%ndy;
        uint64_t x = x1 + i / (ndy);

        double x_portion_of_value = (x+1) * pow(10.0, y_digits);
        double y_portion_of_value = (y+1);

        double val = x_portion_of_value + y_portion_of_value;
        if(rank == 1 && i == chunk_vol-1){
            extreme_debug_log << "val: " << val << " x_portion_of_value: " << x_portion_of_value << " y_portion_of_value: " << y_portion_of_value << endl;
        }
        data_vct[i] = val;
    }

    // extreme_debug_log << "data_vct.size(): " << 
}


static void get_obj_lengths(md_catalog_var_entry var, 
                    uint64_t &x_width, uint64_t &last_x_width, uint64_t ndx, uint64_t chunk_size) 
{

    uint64_t ceph_obj_size = 8000000; //fix 

    extreme_debug_log << "chunk size: " << chunk_size << endl;
    uint32_t num_objs_per_chunk = round(chunk_size / ceph_obj_size);
    if(num_objs_per_chunk <= 0) {
        num_objs_per_chunk = 1;
    }
    extreme_debug_log << "num_objs_per_chunk: " << num_objs_per_chunk << endl;
    x_width = round(ndx / num_objs_per_chunk);
    extreme_debug_log << "x_width: " << x_width << endl;
    last_x_width = ndx - x_width * (num_objs_per_chunk-1);
    if(last_x_width <= 0) {
        num_objs_per_chunk = num_objs_per_chunk + floor( (last_x_width-1) / x_width);
        last_x_width = ndx - x_width * (num_objs_per_chunk-1);
    }
}


int write_output(std::map <string, vector<double>> &data_outputs, md_catalog_run_entry run, 
                md_catalog_var_entry var) 
{
    int rc;

    uint64_t ndx = (var.dims[0].max - var.dims[0].min + 1) / run.npx; //ndx = nx / npx
    uint64_t ndy = 1; 
    uint64_t ndz = 1; 
    uint64_t chunk_vol;
    uint64_t chunk_size;


    // uint64_t ndx = (var.dims[0].max - var.dims[0].min) / run.npx; //ndx = nx / npx
    // uint64_t ndy = (var.dims[1].max - var.dims[1].min) / run.npy; //ndx = nx / npx
    // uint64_t ndz = (var.dims[2].max - var.dims[2].min) / run.npz; //ndx = nx / npx

    int num_ranks;
    if(var.num_dims == 3) {
        num_ranks = run.npx*run.npy*run.npz;
        ndy = (var.dims[1].max - var.dims[1].min + 1) / run.npy; //ndx = nx / npx
        ndz = (var.dims[2].max - var.dims[2].min + 1) / run.npz; //ndx = nx / npx
        // chunk_vol = ndx * ndy * ndz;
       extreme_debug_log << "nx: " << (var.dims[0].max - var.dims[0].min) << " ny: " << (var.dims[1].max - var.dims[1].min) << " nz: " << (var.dims[2].max - var.dims[2].min) << endl;

    }
    else if(var.num_dims == 2) {
        num_ranks = run.npx*run.npy;
        ndy = (var.dims[1].max - var.dims[1].min + 1) / run.npy; //ndx = nx / npx
        // ndz = 0; //fix - be careful with this
        // chunk_vol = ndx * ndy;
         extreme_debug_log << "nx: " << (var.dims[0].max - var.dims[0].min) << " ny: " << (var.dims[1].max - var.dims[1].min) << endl;

    }
    else if(var.num_dims == 1) {
        num_ranks = run.npx;
        // ndy = 0; //fix - be careful with this
        // ndz = 0; //fix - be careful with this
        // chunk_vol = ndx;
    }
    chunk_vol = ndx * ndy * ndz;
    chunk_size = chunk_vol * var.data_size;
    extreme_debug_log << "num_ranks: " << num_ranks << endl;
    extreme_debug_log << "ndx: " << ndx << " ndy: " << ndy << " ndz: " << ndz << endl;

    for(int rank = 0; rank < (num_ranks); rank++) {
        std::vector<string> obj_names;
  
        vector<md_dim_bounds> bounding_box(3);
        uint64_t x_width;
        uint64_t last_x_width;

        extreme_debug_log << "about to get bounding box \n";
        // question: should I just pass ndx/ndy/ndz to these functs?
        rc = rankToBoundingBox(run, var, rank, bounding_box);
        if (rc != RC_OK) {
            error_log << "Error doing the rank to bounding box function, returning \n";
            return RC_ERR;
        }
        if(var.num_dims == 3) {
            if (extreme_debug_logging) {
                printf("Bounding box 2: (%llu, %llu, %llu),(%llu, %llu, %llu)\n",bounding_box[0].min,bounding_box[1].min,bounding_box[2].min,bounding_box[0].max,bounding_box[1].max,bounding_box[2].max);
            }
        }
        else if(var.num_dims == 2) {
        //     bounding_box[2].min = 0;
        //     bounding_box[2].max = 0;

        //     // bounding_box.erase( bounding_box.end() );
        //     extreme_debug_log << "bounding_box.size(): " << bounding_box.size() << endl;
            if (extreme_debug_logging) {
                printf("Bounding box: (%llu, %llu),(%llu, %llu)\n",bounding_box[0].min,bounding_box[1].min,bounding_box[0].max,bounding_box[1]);
                // printf("Bounding box 2: (%llu, %llu, %llu),(%llu, %llu, %llu)\n",bounding_box[0].min,bounding_box[1].min,bounding_box[2].min,bounding_box[0].max,bounding_box[1].max,bounding_box[2].max);
            }
        }


        // rc = boundingBoxToObjNames(run, "boundingBoxToObjectNamesAndCounts", var, bounding_box, obj_names);
        rc = boundingBoxToObjNames(run, var, bounding_box, obj_names);
        if (rc != RC_OK) {
            error_log << "Error doing the bounding box to obj names, returning \n";
            return RC_ERR;
        }

        extreme_debug_log << "about to try generating data \n";
        vector<double> data_vct (chunk_vol); 

        if(var.num_dims == 3) {
            generate_data_for_proc(var, rank, bounding_box, data_vct, ndx, ndy, ndz, chunk_vol);
        }
        else if (var.num_dims == 2) {
            generate_data_for_proc2D(var, rank, bounding_box, data_vct, ndx, ndy, chunk_vol);            
        }
        extreme_debug_log << "data_vct.size(): " << data_vct.size() << endl;

        get_obj_lengths(var, x_width, last_x_width, ndx, chunk_size);
        extreme_debug_log << "x_width: " << x_width << " last_x_width: " << last_x_width << endl;

        extreme_debug_log << "obj names.size(): " << obj_names.size() << endl;

        extreme_debug_log << "data_vct.size(): " << data_vct.size() << endl;
        for(int i=0; i<obj_names.size(); i++) {
            if(i != obj_names.size()-1) {

                // vector<double>::const_iterator first = data_vct.begin() + x_width * ndy * ndz * i; // fix - if you switch ndy/ndz to be 0 when npy/npz=0, then need to change this
                // vector<double>::const_iterator last = data_vct.begin() + x_width * ndy * ndz * (i+1);
                // vector<double> temp(first, last);
                // extreme_debug_log << "temp.size(): " << temp.size() << endl;

                // if(i == 0) {
                //     extreme_debug_log << "obj name: " << obj_names[i] << " val 0: " << temp[0] << endl;
                // }
    //             extreme_debug_log << "about to create vector " << obj_names[i] << endl;

                // // std::vector<double> temp(data_vct.begin()+x_width*ndy*ndz*i, data_vct.begin()+x_width*ndy*ndz*(i+1));
    // //             extreme_debug_log << "about to assign vector " << obj_names[i] << endl;

    // //             data_outputs[obj_names[i]] = temp;

    //                          std::vector<double> temp(data_vct.begin()+x_width*ndy*ndz*i, data_vct.begin()+x_width*ndy*ndz*(i+1));
    //             extreme_debug_log << "about to assign vector " << obj_names[i] << endl;
                extreme_debug_log << "storing data for " << obj_names[i] << " with data from: (" << x_width*ndy*ndz*i << "," << x_width*ndy*ndz*(i+1) << ")" << endl;
                extreme_debug_log << "data_vct.length(): " << data_vct.size() << endl;
                data_outputs[obj_names[i]].assign(data_vct.begin()+x_width*ndy*ndz*i, data_vct.begin()+x_width*ndy*ndz*(i+1));
                // extreme_debug_log << "storing data for " << obj_names[i] << endl;
                // if(rank == 0) {
                //  for(int j =0; j<10; j++) {
                //      testing_log << "data array[x_width*i+j]: " << data_vct[x_width*output.ndy*output.ndz*i+j] << endl;
                //      testing_log << "temp[j]: " << temp[j] << endl;
                //  }          
                // }
            // free(temp);
            }
            else {
                // vector<double>::const_iterator first = data_vct.end() - last_x_width * ndy * ndz;
                // vector<double>::const_iterator last = data_vct.end();
                // vector<double> temp(first, last);
                extreme_debug_log << "storing data for " << obj_names[i] << " with data from: (" << data_vct.size()-last_x_width*ndy*ndz << "," << data_vct.size() << ")" << endl;
                data_outputs[obj_names[i]].assign(data_vct.end()-last_x_width*ndy*ndz, data_vct.end());

                // std::vector<double> temp(data_vct.end()-last_x_width*ndy*ndz, data_vct.end());

             //    data_outputs[obj_names[i]] = temp;
             //    extreme_debug_log << "temp.size(): " << temp.size() << endl;

             //    extreme_debug_log << "storing data for " << obj_names[i] << endl;

              // free(temp);
              // memcpy(data_outputs[obj_names[i]], &data_vct[x_width*i], last_x_width*sizeof(double));
            }
        //DONT FORGET - NEED TO FREE THESE ARRAYS
        }
        extreme_debug_log << "rank: " << rank << endl;
    }
    return rc;
}


static int retrieve_obj_names_and_data(std::map <string, vector<double>> data_outputs, md_catalog_run_entry run, 
                      md_catalog_var_entry var, vector<md_dim_bounds> bounding_box) 
{
    int rc;

    string errmsg;
    uint64_t ny = 0;
    uint64_t nz = 0;

    if (var.num_dims > 1) {
        ny = (var.dims.at(1).max - var.dims.at(1).min + 1);
    }
    if (var.num_dims > 2) {
        nz = (var.dims.at(2).max - var.dims.at(2).min + 1);
    }

    // extreme_debug_log << "got here 2\n";

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

    vector<string> obj_names;
    vector<uint64_t> offsets_and_counts;
        // extreme_debug_log << "got here 4 \n";



    int j = bounding_box.size();
    while(j < 3) {
        md_dim_bounds dims;
        dims.min = 0;
        dims.max = 0;
        bounding_box.push_back(dims);
        j++;
    }
    // cout << "bounding box size: " << bounding_box.size() << endl;
    // printf("Bounding box 3: (%llu, %llu, %llu),(%llu, %llu, %llu)\n",bounding_box[0].min,bounding_box[1].min,bounding_box[2].min,bounding_box[0].max,bounding_box[1].max,bounding_box[2].max);

    // extreme_debug_log << "got here 3\n";

    rc = boundingBoxToObjNamesAndCounts(run, var, bounding_box, obj_names, offsets_and_counts);
    if (rc != RC_OK) {
        error_log << "error with boundingBoxToObjNamesAndCounts \n";
        return rc;
    }

    // extreme_debug_log << "got here 5 \n";

    // printf("Bounding box: (%llu, %llu, %llu),(%llu, %llu, %llu)\n",bounding_box[0].min,bounding_box[1].min,bounding_box[2].min,bounding_box[0].max,bounding_box[1].max,bounding_box[2].max);

    // extreme_debug_log << "obj_names.size(): " << obj_names.size() << endl;
    // extreme_debug_log << "offsets_and_counts.size(): " << offsets_and_counts.size() << endl;
    for(int i=0; i<obj_names.size(); i++) {
        vector<uint64_t>::const_iterator first = offsets_and_counts.begin() + (2*var.num_dims)*i; 
        vector<uint64_t>::const_iterator last = offsets_and_counts.begin() + (2*var.num_dims)*(i+1);
        vector<uint64_t> my_offsets_and_counts(first, last);

        if(extreme_debug_logging) {
            printf("Offsets and counts: x: (%llu, %llu), y: (%llu, %llu), z: (%llu, %llu)\n",my_offsets_and_counts[0],my_offsets_and_counts[1],my_offsets_and_counts[2],my_offsets_and_counts[3],my_offsets_and_counts[4],my_offsets_and_counts[5]);
        }

        vector<double> data_vct;
        // cout << "obj name to find: " << obj_names[i] << endl;
        if(data_outputs.find(obj_names[i]) != data_outputs.end()) {
            data_vct = data_outputs.find(obj_names[i])->second;
            extreme_debug_log << "found the data vector \n";
        }
        else {
            error_log << "error. could not find given object name \n";
            error_log << "obj_name: " << obj_names[i] << endl;

            error_log << "map: \n";
            for (std::map<string,vector<double>>::iterator it=data_outputs.begin(); it!=data_outputs.end(); ++it) {
              string obj_name = it->first;
              vector<double> temp_ary = it->second;
              error_log << "obj_name: " << obj_name << endl;
            }
            return RC_ERR;
        }

        //note: this is just for debug purposes/////////////////////////////////////////////////////////////////////////////////////////////
        uint64_t first_index;
        uint64_t last_index;

        if(var.num_dims == 3) {
            getFirstAndLastIndex(run, var, my_offsets_and_counts, first_index, last_index);
        }
        else if(var.num_dims == 2) {
            getFirstAndLastIndex2D(run, var, my_offsets_and_counts, first_index, last_index);
        }
        extreme_debug_log << "first_index: " << first_index << endl;
        extreme_debug_log << "last_index: " << last_index << endl;
        extreme_debug_log << "data_vct.size(): " << data_vct.size() << endl;

        double first_val = data_vct.at(first_index);
        double last_val = data_vct.at(last_index);
        extreme_debug_log << "first val: " << first_val << " last val: " << last_val << endl;
        extreme_debug_log << "obj name:" << obj_names[i].c_str() << endl;

        uint64_t x1 = first_val / pow(10.0, y_digits) - 1;
        uint64_t x2 = last_val / pow(10.0, y_digits) - 1;
        uint64_t y1 = first_val - (x1+1)* pow(10.0, y_digits) -1;
        uint64_t y2 = last_val - (x2+1)* pow(10.0, y_digits) -1;
        // uint64_t y1 = (int)(first_val-1) % ny;
        // uint64_t y2 = (int)(last_val-1) % ny;
        extreme_debug_log << "ny: " << ny << " first_val-1: " << first_val-1 << " (first_val-1) mod ny: " << (int)(first_val-1) % ny << endl;

        if(var.num_dims == 3) {
            uint64_t z1 = round((first_val - (int)first_val)*pow(10.0, z_digits-1) -1);
            uint64_t z2 = round((last_val - (int)last_val)*pow(10.0, z_digits-1) -1);
            printf("data range for obj %llu is: (%llu, %llu, %llu),(%llu, %llu, %llu)\n",i,x1,y1,z1,x2,y2,z2);  
        }
        else if(var.num_dims == 2) {
            printf("data range for obj %llu is: (%llu, %llu),(%llu, %llu)\n",i,x1,y1,x2,y2);              
        }
        
    }

    return RC_OK;
}



static void getFirstAndLastIndex(md_catalog_run_entry run, md_catalog_var_entry var, vector<uint64_t> offsets_and_counts, 
                        uint64_t &first_index, uint64_t &last_index)
{

    uint64_t ndx = (var.dims[0].max - var.dims[0].min + 1) / run.npx; //ndx = nx / npx
    uint64_t ndy = (var.dims.at(1).max - var.dims.at(1).min + 1) / run.npy;
    uint64_t ndz = (var.dims.at(2).max - var.dims.at(2).min + 1) / run.npz;
    extreme_debug_log << "ndx: " << ndx << " ndy: " << ndy << " ndz: " << ndz << endl;

    uint64_t start_x = offsets_and_counts.at(0);
    uint64_t start_y = offsets_and_counts.at(2);
    uint64_t start_z = offsets_and_counts.at(4);
    uint64_t count_x = offsets_and_counts.at(1);
    uint64_t count_y = offsets_and_counts.at(3);
    uint64_t count_z = offsets_and_counts.at(5);

    if(extreme_debug_logging) {
        printf("Offsets and counts: x: (%llu, %llu), y: (%llu, %llu), z: (%llu, %llu)\n",offsets_and_counts[0],offsets_and_counts[1],offsets_and_counts[2],offsets_and_counts[3],offsets_and_counts[4],offsets_and_counts[5]);
    }


    uint64_t end_x = start_x + count_x - 1;
    uint64_t end_y = start_y + count_y - 1;
    uint64_t end_z = start_z + count_z - 1;

    first_index = start_z + ndz*(start_y + ndy*start_x);
    last_index = end_z + ndz*(end_y + ndy*end_x);
    extreme_debug_log << "last_index: " << last_index << endl;
    extreme_debug_log << "end_z: " << end_z << " end_y: " << end_y << " end_x: " << end_x << endl;

}


static void getFirstAndLastIndex2D(md_catalog_run_entry run, md_catalog_var_entry var, vector<uint64_t> offsets_and_counts, 
                        uint64_t &first_index, uint64_t &last_index)
{
    extreme_debug_log << "var.dims.size(): " << var.dims.size() << endl;

    uint64_t ndx = (var.dims[0].max - var.dims[0].min + 1) / run.npx; //ndx = nx / npx
    uint64_t ndy = (var.dims.at(1).max - var.dims.at(1).min + 1) / run.npy;
    extreme_debug_log << "ndx: " << ndx << " ndy: " << ndy << endl;

    // extreme_debug_log << "got here \n";

    uint64_t start_x = offsets_and_counts.at(0);
    uint64_t start_y = offsets_and_counts.at(2);
    uint64_t count_x = offsets_and_counts.at(1);
    uint64_t count_y = offsets_and_counts.at(3);
    if(extreme_debug_logging) {
        printf("Offsets and counts: x: (%llu, %llu), y: (%llu, %llu)\n",offsets_and_counts[0],offsets_and_counts[1],offsets_and_counts[2],offsets_and_counts[3]);
    }

    uint64_t end_x = start_x + count_x - 1;
    uint64_t end_y = start_y + count_y - 1;

    first_index = start_y + ndy*start_x;
    last_index = end_y + ndy*end_x;
    extreme_debug_log << "end_y: " << end_y << " end_x: " << end_x << endl;
}

int retrieveObjNamesAndDataForAttrCatalog(std::map <string, vector<double>> data_outputs, md_server server, md_catalog_run_entry run, 
                                         uint64_t run_id, uint64_t timestep_id,
                                         uint64_t txn_id,
                                         vector<md_catalog_var_attribute_entry> attr_entries )
{
    int rc;

    vector<md_catalog_var_entry> var_entries;
    uint32_t count;

    extreme_debug_log << "got here 1\n";

    rc = metadata_catalog_var (server, run_id, timestep_id, txn_id, count, var_entries);
    if (rc != RC_OK) {
        error_log << "Error cataloging the new set of var entries. Proceeding" << endl;
        return RC_ERR;
    }
    extreme_debug_log << "got here 2\n";

    for(int i = 0; i < attr_entries.size(); i++) {
        md_catalog_var_entry matching_var;

        md_catalog_var_attribute_entry attr = attr_entries.at(i);

        int index_var=0;
        while (index_var < var_entries.size() && var_entries.at(index_var).var_id != attr.var_id) {
            index_var++;
        }
        if (var_entries.size() <= index_var) {
            error_log << "error. couldn't find the var with id " << attr.var_id << endl;
            print_var_catalog (var_entries.size(), var_entries);
            return RC_ERR;
        }
        // extreme_debug_log << "got here 3 \n";

        matching_var = var_entries.at(index_var);

        // cout << "attr.dims.size(): " << attr.dims.size() << endl;

        rc = retrieve_obj_names_and_data(data_outputs, run, matching_var, attr.dims);
        if (rc != RC_OK) {
            error_log << "error in retrieveObjNamesAndDataForAttrCatalog \n";
            return rc;
        }
    }
    return RC_OK;
}



// void generate_data_for_proc(uint32_t npx, uint32_t npy, uint32_t npz, vector<md_dim_bounds> var_dims, int rank, vector<md_dim_bounds> dims, double data_vct) {
//   uint64_t ndx = (var_dims[0].max - var_dims[0].min) / dataset.npx; //ndx = nx / npx
//   uint64_t ndy = (var_dims[1].max - var_dims[1].min) / dataset.npy; //ndx = nx / npx
//   uint64_t ndz = (var_dims[2].max - var_dims[2].min) / dataset.npz; //ndx = nx / npx

//   uint32_t chunk_vol = ndx * ndy * ndz;
//   // uint32_t chunk_size = chunk_vol * data_size;
//   // extreme_debug_log << "chunk_size: " << chunk_size << endl;
//   data_vct.reserve(chunk_vol);

//   int number = var_dims[1].max - var_dims[1].min; //ny

//   int y_digits = 0;
//   while (number > 0) {
//       number /= 10;
//       y_digits++;
//   }
//   y_digits++;

//   int z_digits = 0;
//   int number2 = var_dims[2].max - var_dims[2].min; //nz

//   while (number2 > 0) {
//       number2 /= 10;
//       z_digits++;
//   }
//   z_digits++;

//   extreme_debug_log << "ndx: " << ndx << " ndy: " << ndy << " ndz: " << ndz << endl;

//   // uint32_t offset = z + NZ(y + NY*x)
//   // uint32_t offset = z1 + output.nz*(y1 + output.ny*x1);
//   // extreme_debug_log << "starting offset: " << offset << " rank: " << rank << endl;
//   // extreme_debug_log << "z1: " << z1 << " y1: " << y1 << " x1: " << x1 << endl;

//   // extreme_debug_log << "chunk vol: " << chunk_vol << endl;

//   for(int i=0; i<chunk_vol; i++) {

//     uint32_t z = z1 + i % ndz;  
//     uint32_t y = y1 + (i / ndz)%ndy;
//     // uint32_t x = (i+offset) / (output.ndz * output.ndy);
//     uint32_t x = x1 + i / (ndz * ndy);

//     double x_portion_of_value = (x+1) * pow(10.0, y_digits);
//     // extreme_debug_log << "y: " << y << endl;
//     double y_portion_of_value = (y+1);
//     double z_portion_of_value = (z+1)/pow(10.0, z_digits-1);
//     // extreme_debug_log << "x_portion_of_value: " << x_portion_of_value << " y_portion_of_value: " << y_portion_of_value << " z_portion_of_value: " << z_portion_of_value << endl;

//     double val = (x+1) * pow(10.0, y_digits) + (y+1) +(z+1)/pow(10.0, z_digits-1);
//     if(rank == 1 && i == chunk_vol-1){
//       extreme_debug_log << "val: " << val << " x_portion_of_value: " << x_portion_of_value << " y_portion_of_value: " << y_portion_of_value << " z_portion_of_value: " << z_portion_of_value << endl;

//       // extreme_debug_log << "z_portion_of_value: " << z_portion_of_value << " val: " << val << endl;
//     }
//     data_vct[i] = val;
//   }
// }

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

