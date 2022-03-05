#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <math.h> 

#include <mpi.h>

#include <my_metadata_client.h>
// #include <my_metadata_client_lua_functs.h>

#include <Globals.hh>
#include "dirman/DirMan.hh"//#include <gutties/Gutties.hh>

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

std::vector<int> catg_of_time_pts;
std::vector<std::chrono::high_resolution_clock::time_point> time_pts;

//A variable that will keep track of all mpi information

int run_test(int rank, faodel::DirectoryInfo dir, int num_servers, md_server dirman);
static int setup_dirman(const string &dirman_hexid, const string &dir_path, faodel::DirectoryInfo &dir, md_server &dirman);
static void setup_server(md_server &server, int rank, const faodel::DirectoryInfo &dir);

void print_var_attr_data(uint32_t count, std::vector<md_catalog_var_attribute_entry> attr_entries);


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


int create_run(md_server server, md_catalog_run_entry run, uint64_t &run_id,
                uint64_t &type0_id, md_catalog_type_entry &new_type );

int create_timestep(md_server server, uint64_t run_id, uint64_t timestep_id, string timestep_path, uint64_t txn_id,
                    uint64_t type0_id, uint64_t &var0_id, 
                    md_catalog_var_entry &new_var);

void make_attr_data (int test_int, string &serial_str);
void make_attr_data (string test_string, int test_int, string &serial_str);

template <class T>
void gatherv_ser_and_combine(const vector<T> &values, uint32_t num_client_procs, int rank, MPI_Comm comm,
	vector<T> &all_vals
	);

template <class T>
void combine_ser( int num_client_procs, int *each_proc_ser_values_size, int *displacement_for_each_proc, 
        char *serialized_c_str_all_ser_values, vector<T> &all_vals);

int metadata_collective_insert_var_attribute_by_dims_batch (const md_server &server
                           // vector<uint64_t> &attribute_ids,
                           ,const vector<md_catalog_var_attribute_entry> &new_attributes,
                           MPI_Comm comm
                           );

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
	extreme_debug_log << "starting \n";

    int rc;
    bool ok;
    int rank;
    int num_servers;
    string dir_path="/metadata/testing";
    
    // faodel::DirectoryInfo dir(dir_path,"Sirius metadata servers");

    faodel::DirectoryInfo dir;
    md_server dirman;

    //make sure the user provided an argument, the progravarm is expecting to receive
    //the directory manager's full URL path
    if (argc < 2)
    {
        error_log << "Usage: " << argv[0] << "<directory manager hexid>\n";
        return RC_ERR;
    }

    extreme_debug_log << "argv[1]: " << argv[1] << endl;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    // if(rank != 0) {
    //     md_debug = false;
    // }
    testing_log.set_rank(rank);

    rc = setup_dirman(argv[1], dir_path, dir, dirman);
    if(rc != RC_OK) {
        goto cleanup;
    }
    num_servers = dir.members.size();

    // //have the processes preform a series of tests of the metadata client functions
    rc = run_test (rank, dir, num_servers, dirman);

    // G2.StopAll();

    MPI_Barrier(MPI_COMM_WORLD);

    //returns RC_OK if there were no errors, or RC_ERR if there were errors
    MPI_Finalize();
    gutties::bootstrap::Finish();

    md_log( "rc is " + to_string(rc) );

cleanup:
    return rc;
}

/*
 * Takes the full URL of the directory manager
 */
int run_test(int rank, faodel::DirectoryInfo dir, int num_servers, md_server dirman) {

    bool single_attr = false;
    bool gathered_write = true;

    int rc;
  
    struct md_catalog_run_entry new_run;
    struct md_catalog_type_entry new_type0;
    struct md_catalog_var_entry new_var0;

    uint64_t run0_id;
    uint64_t type0_id;
    uint64_t timestep0_id = 0;
    uint64_t var0_id;
    uint64_t attr0_id;



    //add create new runs and timesteps here
    uint64_t txn_id = 5;

    md_server server;

    setup_server(server, rank, dir);

    rc = metadata_init ();

    if (rc != RC_OK) {
        error_log << "Error. Was unable to initialize the client. Exitting \n";
        return RC_ERR;
    }

    testing_log << "\nBeginning the testing setup \n";


    // md_catalog_run_entry new_run;
    new_run.job_id = 1234567;
    new_run.name = "XGC";
    // new_run.path = "/XGC";
    new_run.txn_id = txn_id;
    new_run.npx = 10;
    new_run.npy = 10;
    new_run.npz = 10;
    // new_run.rank_to_dims_funct = "FIX ME"; 
    // new_run.objector_funct = "OBJECTOR FIX ME";

    if(rank < num_servers) {
	    rc = create_run( server, new_run, run0_id, type0_id, new_type0 );
	    if(rc != RC_OK) {
	    	error_log << "Error creating run. Exiting\n";
	    	return RC_ERR;
	    }

	    string timestep0_path = "/" + to_string(timestep0_id);
	   
	    md_catalog_timestep_entry new_timestep;
	    new_timestep.run_id = run0_id;
	    new_timestep.timestep_id = timestep0_id;
	    new_timestep.path = timestep0_path;
	    new_timestep.txn_id = txn_id;

	    rc = create_timestep(server, run0_id, timestep0_id, timestep0_path, txn_id, type0_id, var0_id, new_var0 );
	    if (rc != RC_OK) {
	        error_log << "Error. Was unable to create the first timestep. Exiting \n";
	        return RC_ERR;
	    }
	}


    md_catalog_var_attribute_entry new_attr;
    uint32_t num_dims = 3;
    md_dim_bounds *dims = (md_dim_bounds *) malloc(sizeof(md_dim_bounds) * 3);
    dims [0].min = 15;
    dims [0].max = 25;
    dims [1].min = 15;
    dims [1].max = 25;
    dims [2].min = 15;
    dims [2].max = 25;

    vector<md_dim_bounds> vect_dims = std::vector<md_dim_bounds>(dims, dims + num_dims);

    new_attr.timestep_id = timestep0_id;
    new_attr.type_id = 1;
    new_attr.var_id = 0;
    new_attr.txn_id = txn_id;
    new_attr.dims = vect_dims;
    new_attr.num_dims = num_dims;

    string str("act_\0newtest",9);
    make_attr_data(str, 500+rank, new_attr.data);
    // make_single_val_data (val, new_attr.data);
    new_attr.data_type = ATTR_DATA_TYPE_BLOB;

    md_catalog_var_attribute_entry new_attr2 = new_attr;
    dims [0].min = 10;
    dims [0].max = 11;
    dims [1].min = 12;
    dims [1].max = 13;
    dims [2].min = 14;
    dims [2].max = 15;
	vect_dims = std::vector<md_dim_bounds>(dims, dims + num_dims);
	new_attr2.dims = vect_dims;

    vector<md_catalog_var_attribute_entry> attrs;

    if(single_attr) {
    	attrs = {new_attr};
    }
    else {
    	attrs = {new_attr, new_attr2};
    }

    if(gathered_write) {
    	MPI_Comm comm = MPI_COMM_WORLD;
    	if(num_servers > 1) {
    		MPI_Comm_split(MPI_COMM_WORLD, rank%num_servers, rank, &comm);
    	}
		rc = metadata_collective_insert_var_attribute_by_dims_batch(server, attrs, comm);
		if (rc != RC_OK) {
	        error_log << "Error. Was unable to collectively insert the new var attributes. Proceeding" << endl;
	    }
	}
	else {
		rc = metadata_insert_var_attribute_by_dims_batch (server, attrs);
	    if (rc != RC_OK) {
	        error_log << "Error. Was unable to insert the new var attributes. Proceeding" << endl;
	    }
		// rc = metadata_insert_var_attribute_by_dims (server, attr0_id, new_attr);
	 //    if (rc != RC_OK) {
	 //        error_log << "Error. Was unable to insert the new var attribute. Proceeding" << endl;
	 //    }
	 //    testing_log << "New attr id: " << attr0_id << endl;

	}

    MPI_Barrier(MPI_COMM_WORLD);
    if(rank < num_servers) {
		uint32_t count;
		vector<md_catalog_var_attribute_entry> attr_entries;

	    rc = metadata_catalog_all_var_attributes (server, run0_id, timestep0_id, txn_id, count, attr_entries);
	    if (rc == RC_OK) {
	        testing_log << "using var attrs funct: attributes associated with run_id " << run0_id << " and timestep_id " << timestep0_id << " and txn_id: " << txn_id;
	        testing_log << "\n";
	        if(testing_logging || (zero_rank_logging )) {
	            print_var_attribute_list (count, attr_entries);
	        }

	        print_var_attr_data(count, attr_entries);
	    }
	    else {
	        error_log << "Error metadata_catalog_all_var_attributes. Proceeding" << endl;
	    }
	}

//-------------------------------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------------------------------
    
    testing_log << "done with testing\n";

    MPI_Barrier(MPI_COMM_WORLD);
    if( rank < num_servers) {
        metadata_finalize_server(server);    
        if(rank == 0) {
            metadata_finalize_server(dirman);
        }
     	testing_log << "just finalized server \n";

    }


    return rc;


}


static int setup_dirman(const string &dirman_hexid, const string &dir_path, 
                        faodel::DirectoryInfo &dir, md_server &dirman)
{
    int rc;
    bool ok;

 //Add the directory manager's URL to the config string so the clients know
    //Where to access the directory ???
    md_log( "dirman root url is" + dirman_hexid );
    gutties::Configuration config(default_config_string);
    config.Append("dirman.root_node", dirman_hexid);

    config.AppendFromReferences();

    //Start the mpi/client processes, and launch the directory manager ???
    // G2.StartAll(argc, argv, config);
    gutties::bootstrap::Start(config, opbox::bootstrap);

    gutties::nodeid_t dirman_nodeid(dirman_hexid);
    md_log("Dirman node ID " + dirman_nodeid.GetHex()+" " +
       dirman_nodeid.GetIP() + " port " + dirman_nodeid.GetPort());
    dirman.name_and_node.node = dirman_nodeid;
    extreme_debug_log << "about to connect peer to node" << endl;
    rc = net::Connect(&dirman.peer_ptr, dirman.name_and_node.node);
    assert(rc==0 && "could not connect");

    //-------------------------------------------

     //Have all nodes join the directory
    ok = dirman::Getfaodel::DirectoryInfo(gutties::ResourceURL(dir_path), &dir);
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

    net::Connect(&server.peer_ptr, server.name_and_node.node);

    md_log("dir.members.size = " + to_string(dir.members.size()));
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


void print_var_attr_data(uint32_t count, std::vector<md_catalog_var_attribute_entry> attr_entries) {

    if(count != attr_entries.size()) {
        error_log << "Error trying to print var attr data. Count: " << count << " while attr_entries.size(): " << attr_entries.size() << endl;
    }

    printf("attr_entries.size(): %d\n", attr_entries.size());
    for (int i=0; i<attr_entries.size(); i++) {


        switch(attr_entries.at(i).data_type) {
            case ATTR_DATA_TYPE_INT : {
                uint64_t deserialized_test_int;

                stringstream sso;
                sso << attr_entries.at(i).data;
                boost::archive::text_iarchive ia(sso);
                ia >> deserialized_test_int;

                testing_log << "data: int: " << deserialized_test_int << endl; 
                break;
            }
            case ATTR_DATA_TYPE_REAL : {
                long double deserialized_test_real;

                stringstream sso;
                sso << attr_entries.at(i).data;
                boost::archive::text_iarchive ia(sso);
                ia >> deserialized_test_real;

                testing_log << "data: real: " << deserialized_test_real << endl; 
                break;
            }           
            case ATTR_DATA_TYPE_TEXT : {
                    // string deserialized_test_string;
                testing_log << "data: text: " << attr_entries.at(i).data << endl; 
                break;
            } 
            case ATTR_DATA_TYPE_BLOB : {
                string deserialized_test_string;
                int deserialized_test_int;

                stringstream sso;
                sso << attr_entries.at(i).data;
                boost::archive::text_iarchive ia(sso);
                // cout << "sso: " << sso.str() << endl;
                ia >> deserialized_test_string;
                ia >> deserialized_test_int;

                testing_log << "data: blob: str: " << deserialized_test_string << " int: " << deserialized_test_int << endl; 
                // testing_log << "str length: " << deserialized_test_string.size() << endl; 

                break;
            } 
        }
        // extreme_debug_log << "serialized var data: " << attr_entries.at(i).data << endl;

        // testing_log << "data: string: " << deserialized_test_string << " int: " << deserialized_test_int << endl; 
    }
 }


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


// int create_run(md_server server, md_catalog_run_entry run, uint64_t &run_id,
//                 uint64_t &type0_id, md_catalog_type_entry &new_type ) 
// {    
//     int rc;

//     string rank_to_dims_funct_name = "rank_to_bounding_box";
//     string rank_to_dims_funct_path = "PATH_TO_EMPRESS/lib_source/lua/lua_function.lua";
//     string objector_funct_name = "boundingBoxToObjectNamesAndCounts";
//     string objector_funct_path = "PATH_TO_EMPRESS/lib_source/lua/metadata_functs.lua";


//     rc = metadata_create_run (server, run_id, run.job_id, run.name, run.txn_id, run.npx, run.npy, run.npz,  
//                 rank_to_dims_funct_name, rank_to_dims_funct_path, objector_funct_name, objector_funct_path);
//     if (rc != RC_OK) {
//         error_log << "Error. Was unable to create the first run. Exiting \n";
//         return RC_ERR;
//     }
//     md_log( "First run id is " + to_string(run_id ) );


//     uint32_t type0_version = 1;
//     string type0_name = "typey1";

//     new_type.run_id = run_id;
//     new_type.name = type0_name;
//     new_type.version = type0_version;
//     new_type.txn_id = run.txn_id;

//     rc = metadata_create_type (server, type0_id, new_type);
//     if (rc != RC_OK) {
//         error_log << "Error. Was unable to insert the first type. Proceeding" << endl;
//     }
//     md_log( "First type id is " + to_string(type0_id ) );

// }



int create_run(md_server server, md_catalog_run_entry run, uint64_t &run_id,
                uint64_t &type0_id, md_catalog_type_entry &new_type ) 
{    
    int rc;

    rc = metadata_create_run (server, run_id, run.job_id, run.name, run.txn_id, run.npx, run.npy, run.npz);
    if (rc != RC_OK) {
        error_log << "Error. Was unable to create the first run. Exiting \n";
        return RC_ERR;
    }
    md_log( "First run id is " + to_string(run_id ) );


    uint32_t type0_version = 1;
    string type0_name = "typey1";

    new_type.run_id = run_id;
    new_type.name = type0_name;
    new_type.version = type0_version;
    new_type.txn_id = run.txn_id;

    rc = metadata_create_type (server, type0_id, new_type);
    if (rc != RC_OK) {
        error_log << "Error. Was unable to insert the first type. Proceeding" << endl;
    }
    md_log( "First type id is " + to_string(type0_id ) );

}


int create_timestep(md_server server, uint64_t run_id, uint64_t timestep_id, string timestep_path, uint64_t txn_id,
                    uint64_t type0_id, uint64_t &var0_id, 
                    md_catalog_var_entry &new_var)   
{
    int rc;

    uint32_t var0_version = 0;

    std::string var0_name = "var0";
    std::string var0_path = "/var0";

    char datasize0 = 4;
    char datasize1 = 8;
    
    var0_id = 0;


    rc = metadata_create_timestep (server, timestep_id, run_id, timestep_path, txn_id);
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


}




template <class T>
void gatherv_ser_and_combine(const vector<T> &values, uint32_t num_client_procs, int rank, MPI_Comm comm,
	vector<T> &all_vals
	)
{
    
    extreme_debug_log << "starting gatherv_ser_and_combine" << endl;   

  	int each_proc_ser_values_size[num_client_procs];
    int displacement_for_each_proc[num_client_procs];
	char *serialized_c_str_all_ser_values;

    int length_ser_c_str = 0;
    char *serialized_c_str;

    stringstream ss;
    boost::archive::text_oarchive oa(ss);
    oa << values;
    string serialized_str = ss.str();
    length_ser_c_str = serialized_str.size() + 1;
    serialized_c_str = (char *) malloc(length_ser_c_str);
    serialized_str.copy(serialized_c_str, serialized_str.size());
    serialized_c_str[serialized_str.size()]='\0';
    // extreme_debug_log << "rank " << rank << " object_names ser string is of size " << length_ser_c_str << " serialized_str " << 
    //     serialized_str << endl;
    extreme_debug_log << "rank " << rank << " ser string is of size " << length_ser_c_str << " str: " << serialized_c_str << endl; 

    // extreme_debug_log << "rank " << rank << " about to allgather" << endl;

    MPI_Gather(&length_ser_c_str, 1, MPI_INT, each_proc_ser_values_size, 1, MPI_INT, 0, comm);

    int sum = 0;
    // int max_value = 0;
    if (rank == 0 ) {
        for(int i=0; i<num_client_procs; i++) {
            displacement_for_each_proc[i] = sum;
            sum += each_proc_ser_values_size[i];
            if(each_proc_ser_values_size[i] != 0 && rank == 0) {
                extreme_debug_log << "rank " << i << " has a string of length " << each_proc_ser_values_size[i] << endl;
            }
            // if(displacement_for_each_proc[i] > max_value) {
            //     max_value = displacement_for_each_proc[i];
            // }
        }
        extreme_debug_log << "sum: " << sum << endl;

        serialized_c_str_all_ser_values = (char *) malloc(sum);

    }

    extreme_debug_log << "num_bytes is "<< to_string(length_ser_c_str) + " and rank: " << to_string(rank) << endl;
    
    extreme_debug_log << "rank " << rank << " about to gatherv" << endl;

    MPI_Gatherv(serialized_c_str, length_ser_c_str, MPI_CHAR,
           serialized_c_str_all_ser_values, each_proc_ser_values_size, displacement_for_each_proc,
           MPI_CHAR, 0, comm);

    extreme_debug_log << "just finished with gather v" << endl;

    free(serialized_c_str);

    if(rank == 0) {
    	all_vals.reserve(3*num_client_procs);

    	combine_ser( num_client_procs, each_proc_ser_values_size, displacement_for_each_proc, 
        serialized_c_str_all_ser_values, values, all_vals);
    }
}

//asssumes the rank is 0
template <class T>
void combine_ser( int num_client_procs, int *each_proc_ser_values_size, int *displacement_for_each_proc, 
        char *serialized_c_str_all_ser_values, const vector<T> &my_vals, vector<T> &all_vals)
{

// extreme_debug_log << "rank " << rank << " done with allgatherv" << endl;
    // extreme_debug_log << "entire set of attr vectors: " << serialized_c_str_all_ser_values << " and is of length: " << strlen(serialized_c_str_all_ser_values) << endl;

    for(int i = 0; i < num_client_procs; i++) {
        int offset = displacement_for_each_proc[i];
        int count = each_proc_ser_values_size[i];
        if(count > 0) {
            vector<T> rec_values;

            //0 rank does not need to deserialize its own attrs
            if(i != 0) {

                extreme_debug_log << "rank " << i << " count: " << count << " offset: " << offset << endl;
                char serialzed_vals_for_one_proc[count];

                memcpy ( serialzed_vals_for_one_proc, serialized_c_str_all_ser_values + offset, count);

                extreme_debug_log << "rank " << i << " serialized_c_str: " << (string)serialzed_vals_for_one_proc << endl;
                extreme_debug_log <<    " serialized_c_str length: " << strlen(serialzed_vals_for_one_proc) << 
                    " count: " << count << endl;
                stringstream ss1;
                ss1.write(serialzed_vals_for_one_proc, count);
                extreme_debug_log << "about to deserialize ss: " << ss1.str() << endl;
                boost::archive::text_iarchive ia(ss1);
                ia >> rec_values;
            }
            else { //i == rank
                rec_values = my_vals;
            }
            all_vals.insert( all_vals.end(), rec_values.begin(), rec_values.end() );
        }
    }
    free(serialized_c_str_all_ser_values);
}


int metadata_collective_insert_var_attribute_by_dims_batch (const md_server &server
                           // vector<uint64_t> &attribute_ids,
                           ,const vector<md_catalog_var_attribute_entry> &new_attributes,
                           MPI_Comm comm
                           )
{
	int rank, num_client_procs, return_value;
	vector<md_catalog_var_attribute_entry> all_attrs;

	MPI_Comm_rank(comm, &rank);
    MPI_Comm_size(comm, &num_client_procs);

    gatherv_ser_and_combine(new_attributes, num_client_procs, rank, comm, all_attrs); 
 
    
    return_value = metadata_insert_var_attribute_by_dims_batch (server
                           // vector<uint64_t> &attribute_ids,
                           ,all_attrs
                           );

    return return_value;
}
