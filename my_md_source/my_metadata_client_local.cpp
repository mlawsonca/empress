
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
// #include <string.h>
#include <string>
#include <sys/stat.h>
#include <unistd.h>
#include <stdint.h>
#include <assert.h>
#include <mpi.h>
#include <boost/algorithm/string.hpp>   

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <sstream> 

#include <my_metadata_client_local.hh>
#include <my_metadata_client_lua_functs.h>
#include <md_client_timing_constants.hh>

// #include <md_client_timing_constants.hh>
// #include <md_client_timing_constants_local.hh>

#include <server_local.hh>
#include <sqlite3.h>

using namespace std;

sqlite3 * db = NULL;


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//JUST FOR DEBUGGING/TESTING ////////////////////////////////////////////////////////////////////////////////////////////
static bool extreme_debug_logging = false;
static bool debug_logging = false;
static bool error_logging = true;
static debugLog error_log = debugLog(error_logging);
static debugLog debug_log = debugLog(debug_logging);
static debugLog extreme_debug_log = debugLog(extreme_debug_logging);

// extern std::vector<int> catg_of_time_pts;
// extern std::vector<std::chrono::high_resolution_clock::time_point> time_pts;
//extern void add_timing_point(int catg);

extern void add_timing_point(int catg);
extern void print_var_attr_data(md_catalog_var_attribute_entry);
extern void print_timestep_attr_data(md_catalog_timestep_attribute_entry);
extern void print_run_attr_data(md_catalog_run_attribute_entry);
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

lua_State *L;

//used to keep track of name + version -> var id (so that varids will be consisten for a given name/version across timesteps)
// map <string, uint64_t> var_ids;

// int sql_output_db(uint64_t job_id, int rank);
int sql_load_db(uint64_t job_id, int rank);
int metadata_database_close();
// int metadata_database_init (bool load_db, bool load_multi, uint64_t job_id, int rank_first, int rank_last=0);
// int metadata_database_init (bool load_db, bool load_multiple_db, uint64_t job_id, int rank, uint32_t num_read_procs=0);
// int metadata_database_init (int rank, md_write_type write_type);
// int metadata_database_init (bool load_multiple_db, uint64_t job_id, int rank, uint32_t num_read_procs);
// int metadata_database_init (int proc_rank, uint64_t job_id, md_write_type write_type);
// int metadata_database_init (bool load_multiple_db, uint64_t job_id, int proc_rank, uint32_t num_read_procs, md_write_type write_type);

// int metadata_database_init (bool load_multiple_db, uint64_t job_id, int proc_rank, uint32_t num_read_procs, md_server_type server_type);
int metadata_database_init (bool load_multiple_db, uint64_t job_id, int proc_rank, uint32_t num_read_procs);
// int metadata_database_init (int proc_rank, uint64_t job_id, md_server_type server_type, md_db_index_type index_type);
// int metadata_database_init (int proc_rank, uint64_t job_id, md_server_type server_type);
int metadata_database_init (int proc_rank, uint64_t job_id);

template <class T>
void get_range_values(const std::string &data, data_range_type range_type, T &val1, T &val2)
{
	std::stringstream sso;
	sso << data;
	boost::archive::text_iarchive ia(sso);

	ia >> val1;
	if(range_type == DATA_RANGE) {
		ia >> val2;
	}
	else {
		val2 = val1;
	}
}
	
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//JUST FOR DEBUGGING/////////////////////////////////////////////////////////////////////////////////////////////////////

void print_md_dim_bounds( const vector<md_dim_bounds> &dims) {
    char v = 'x';
	for (int j = 0; j < dims.size(); j++) {
        	std::cout << " " << v <<": (" << dims [j].min << "/" << dims [j].max << ")";
            v++;
    }
    cout << " ";
	// cout << endl;
}





/*
 * Prints information about the given number of catalog var entries from the given list of entries
 *
 * Inputs: 
 *   num_vars - number of catalog var entries you want to print information for
 *   entries - the list of catalog var entries you want to print from
 */
void print_var_catalog (uint32_t num_vars, const std::vector<md_catalog_var_entry> &entries)
{
    if (num_vars == 0) {
        std::cout << "There are no matching vars \n";
    }
    else {
        std::cout << "matching vars (count: " << num_vars << "): " << endl;
    }
    if(num_vars != entries.size()) {
        cout << "error. expecting a different number of vars than received (expecting " << num_vars << " and received: " << entries.size() << ")\n";
    }

    for (int i = 0; i < entries.size(); i++)
    {
        md_catalog_var_entry var = entries [i];
        
        std::cout << "var_id: " << var.var_id << " run_id: " << var.run_id << " timestep_id: " << var.timestep_id << " name: " << var.name << " path: " << var.path << 
            " version: " << var.version << " data_size: " << var.data_size << " active: " << var.active << " txn_id: " << var.txn_id << 
            // " num_dims: " << var.num_dims << endl;
    	    " num_dims: " << var.num_dims;

    	print_md_dim_bounds( var.dims);
    	cout << endl;
        // std::cout << ("\n");
    }
    cout << endl;
}


/*
 * Prints information about the given number of type catalog entries from the given list of entries
 *
 * Inputs: 
 *   num_vars - number of type catalog entries you want to print information for
 *   entries - the list of type catalog entries you want to print from
 */
void print_type_catalog (uint32_t num_vars, const std::vector<md_catalog_type_entry> &entries)
{
    if (num_vars == 0) {
        std::cout << "There are no matching types \n";
    }
    else {
        std::cout << "matching types (count: " << num_vars << "): " << endl;
    }

   if(num_vars != entries.size()) {
        cout << "error. expecting a dif number of types than received. expecting: " << num_vars << " received: " << entries.size() << endl;
    }

    for (int i = 0; i < entries.size(); i++)
    {
        md_catalog_type_entry type = entries [i];
        std::cout << "type_id: " << type.type_id << " run_id: " << type.run_id << " name: " << type.name << " version: " <<
        type.version <<  " active: " << type.active << " txn_id: " << type.txn_id << endl;
    }
    cout << endl;
}


/*
 * Prints information about the given number of attribute entries from the given list of attribute entries
 *
 * Inputs: 
 *   num_vars - number of attribute entries you want to print information for
 *   entries - the list of attribute entries you want to print from
 */
void print_run_attribute_list (uint32_t num_vars, const std::vector<md_catalog_run_attribute_entry> &entries)
{
    if (num_vars == 0) {
        std::cout << "There are no matching attributes \n";
    }
    else {
        std::cout << "matching attributes (count: " << num_vars << "): " << endl;
    }

    if(num_vars != entries.size()) {
        cout << "error. expecting a dif number of run attributes than received. expecting: " << num_vars << " received: " << entries.size() << endl;
    }

    for (int i = 0; i < entries.size(); i++)
    {
    //     md_catalog_run_attribute_entry attr = entries [i];
    //     std::cout << "attribute id: " << attr.attribute_id << 
    //         " type id: " << attr.type_id << " active: " << attr.active << " txn_id: " << attr.txn_id 
    //         << " data_type: " << (int)attr.data_type << endl;
    // }
        md_catalog_run_attribute_entry attr = entries [i];
        std::cout << "type id: " << attr.type_id << " active: " << attr.active << " txn_id: " << attr.txn_id << " ";
            // << " data_type: " << (int)attr.data_type << endl;
		print_run_attr_data(attr);

    }
    cout << endl;

}

/*
 * Prints information about the given number of attribute entries from the given list of attribute entries
 *
 * Inputs: 
 *   num_vars - number of attribute entries you want to print information for
 *   entries - the list of attribute entries you want to print from
 */
void print_timestep_attribute_list (uint32_t num_vars, const std::vector<md_catalog_timestep_attribute_entry> &entries)
{
    if (num_vars == 0) {
        std::cout << "There are no matching attributes \n";
    }
    else {
        std::cout << "matching attributes (count: " << num_vars << "): " << endl;
    }

    if(num_vars != entries.size()) {
        cout << "error. expecting a dif number of timestep attributes than received. expecting: " << num_vars << " received: " << entries.size() << endl;
    }

    for (int i = 0; i < entries.size(); i++)
    {
    //     md_catalog_timestep_attribute_entry attr = entries [i];
    //     std::cout << "attribute id: " << attr.attribute_id << " timestep_id: " << attr.timestep_id << 
    //         " type id: " << attr.type_id << " active: " << attr.active << " txn_id: " << attr.txn_id <<
    //         " data_type: " << (int)attr.data_type << endl;
    // }
        md_catalog_timestep_attribute_entry attr = entries [i];
        std::cout << "timestep_id: " << attr.timestep_id << 
            " type id: " << attr.type_id << " active: " << attr.active << " txn_id: " << attr.txn_id << " ";
            // " data_type: " << (int)attr.data_type << endl;
		print_timestep_attr_data(attr);
    }
    cout << endl;

}

/*
 * Prints information about the given number of attribute entries from the given list of attribute entries
 *
 * Inputs: 
 *   num_vars - number of attribute entries you want to print information for
 *   entries - the list of attribute entries you want to print from
 */
void print_var_attribute_list (uint32_t num_vars, const std::vector<md_catalog_var_attribute_entry> &entries)
{
    if (num_vars == 0) {
        std::cout << "There are no matching attributes \n";
    }
    else {
        std::cout << "matching attributes (count: " << num_vars << "): " << endl;
    }

    if(num_vars != entries.size()) {
        cout << "error. expecting a dif number of var attributes than received. expecting: " << num_vars << " received: " << entries.size() << endl;
    }

    for (int i = 0; i < entries.size(); i++)
    {
        md_catalog_var_attribute_entry attr = entries [i];
        std::cout << "timestep_id: " << attr.timestep_id << " var_id: " << attr.var_id <<
            " type_id: " << attr.type_id << " active: " << attr.active << " txn_id: " << attr.txn_id << " num_dims: " << attr.num_dims;
        // std::cout << "attribute id: " << attr.attribute_id << " timestep_id: " << attr.timestep_id << " var id: " << attr.var_id <<
        //     " type id: " << attr.type_id << " active: " << attr.active << " txn_id: " << attr.txn_id << " num_dims: " << attr.num_dims;
        // for(int j=0; j< entries [i].num_dims; j++) {
        //     std::cout << " d" << j << "_min: " << entries [i].dims.at(j).min;
        //     std::cout << " d" << j << "_max: " << entries [i].dims.at(j).max;               
        // }
       	print_md_dim_bounds( attr.dims);
		print_var_attr_data(attr);

        // std::cout << " data_type: " << (int)attr.data_type << endl;
    }
    cout << endl;
}


/*
 * Prints information about the given number of run entries from the given list of entries
 *
 * Inputs: 
 *   num_vars - number of run entries you want to print information for
 *   entries - the list of run entries you want to print from
 */
void print_run_catalog (uint32_t num_vars, const std::vector<md_catalog_run_entry> &entries)
{
    if (num_vars == 0) {
        std::cout << "There are no matching entries \n";
    }
    else {
        std::cout << "matching entries (count: " << num_vars << "): " << endl;
    }

    if(num_vars != entries.size()) {
        cout << "error. expecting a dif number of runs than received. expecting: " << num_vars << " received: " << entries.size() << endl;
    }

    for (int i = 0; i < entries.size(); i++)
    {
        md_catalog_run_entry run = entries [i];
        
        std::cout << "run_id: " << run.run_id << " job_id: " << run.job_id << " name: " << run.name <<  " date: " << run.date <<
            " active: " << run.active << " txn_id: " << run.txn_id << " npx: " << run.npx << " npy: " << run.npy <<
            " npz: " << run.npz << endl; 
    }
    cout << endl;
}

/*
 * Prints information about the given number of timestep entries from the given list of entries
 *
 * Inputs: 
 *   num_vars - number of timestep entries you want to print information for
 *   entries - the list of timestep entries you want to print from
 */
void print_timestep_catalog (uint32_t num_vars, const std::vector<md_catalog_timestep_entry> &entries)
{
    if (num_vars == 0) {
        std::cout << "There are no matching entries \n";
    }
    else {
        std::cout << "matching entries (count: " << num_vars << "): " << endl;
    }

    if(num_vars != entries.size()) {
        cout << "error. expecting a dif number of timesteps than received. expecting: " << num_vars << " received: " << entries.size() << endl;
    }

    for (int i = 0; i < entries.size(); i++)
    {
        md_catalog_timestep_entry timestep = entries [i];
        
        std::cout << "timestep_id: " << timestep.timestep_id << " run_id: " << timestep.run_id << " path: " << timestep.path <<
            " active: " << timestep.active << " txn_id: " << timestep.txn_id << endl; 
    }
    cout << endl;
}

bool dims_overlap(const vector<md_dim_bounds> &attr_dims, const vector<md_dim_bounds> &proc_dims) {
	switch(attr_dims.size()) {
		case 3:
			return ( attr_dims[0].min <= proc_dims[0].max && attr_dims[0].max >= proc_dims[0].min &&
			 attr_dims[1].min <= proc_dims[1].max && attr_dims[1].max >= proc_dims[1].min &&
			 attr_dims[2].min <= proc_dims[2].max && attr_dims[2].max >= proc_dims[2].min );

		case 2:
			return ( attr_dims[0].min <= proc_dims[0].max && attr_dims[0].max >= proc_dims[0].min &&
			 attr_dims[1].min <= proc_dims[1].max && attr_dims[1].max >= proc_dims[1].min );		

		case 1:
			return ( attr_dims[0].min <= proc_dims[0].max && attr_dims[0].max >= proc_dims[0].min );

		default :
			return false;		
	}

}

void get_data_int( std::string data, data_range_type range_type, uint64_t &min, uint64_t &max) {
	std::stringstream sso;
	if(data.size() > 0) {
		sso << data;
		boost::archive::text_iarchive ia(sso);

		if(range_type == DATA_RANGE) {
			ia >> min;
	    	ia >> max;
		}
		else if(range_type == DATA_MAX || range_type == DATA_MIN) {
			ia >> min;
			max = min;
		}
	}
	else {
		error_log << "error. tried to get int attr data but it was empty" << endl;
	}
}



void get_data_real( std::string data, data_range_type range_type, long double &min, long double &max) {
	std::stringstream sso;

	if(data.size() > 0) {
		sso << data;
		boost::archive::text_iarchive ia(sso);

		if(range_type == DATA_RANGE) {
			ia >> min;
	    	ia >> max;
		}
		else if(range_type == DATA_MAX || range_type == DATA_MIN) {
			ia >> min;
			max = min;
		}
	}
	else {
		error_log << "error. tried to get real attr data but it was empty" << endl;
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


// int metadata_init_write (uint64_t job_id, int rank) {

//  	int rc = metadata_init (false, job_id, rank);

//     return rc;
// } 

int metadata_init_read(bool load_multiple_db, uint64_t job_id, int rank, uint32_t num_read_procs, 
	md_server_type server_type ) {
    int rc;

    L = luaL_newstate(); //fix - do I ever open a new state or deregister the rank to bounding box functions?
    luaL_openlibs(L);   

    //set up and return the database
    // add_timing_point(DB_SETUP_START);

	// metadata_init_server(server_type, index_type, checkpt_type);
	metadata_init_server(server_type, (md_db_index_type)0, (md_db_checkpoint_type)0);

    // rc = metadata_database_init (load_multiple_db, job_id, rank, num_read_procs, write_type);
    rc = metadata_database_init (load_multiple_db, job_id, rank, num_read_procs);
    if (rc != RC_OK) {
        error_log << "Error. Could not init dbs. Error code " << rc << std::endl;
    }
}

int metadata_init_write (int rank, uint64_t job_id, md_server_type server_type, 
	md_db_index_type index_type, md_db_checkpoint_type checkpt_type) 
{
    int rc;

    L = luaL_newstate(); //fix - do I ever open a new state or deregister the rank to bounding box functions?
    luaL_openlibs(L);   

    //set up and return the database
    // add_timing_point(DB_SETUP_START);

	metadata_init_server(server_type, index_type, checkpt_type);

    // rc = metadata_database_init (rank, job_id, server_type, index_type);
    rc = metadata_database_init (rank, job_id);
    if (rc != RC_OK) {
        error_log << "Error. Could not init dbs. Error code " << rc << std::endl;
    }
    // add_timing_point(DB_SETUP_DONE);

    //todo - if we decide user wont supply the entire objector funct, will need to load this
    // if ( (luaL_loadfile(L, "../lib_source/lua/metadata_functs.lua") || lua_pcall(L, 0, 0, 0)) && (luaL_loadfile(L, "lib_source/lua/metadata_functs.lua") || lua_pcall(L, 0, 0, 0)))
     // if ( (luaL_loadfile(L, "PATH_TO_EMPRESS/lib_source/lua/metadata_functs.lua") || lua_pcall(L, 0, 0, 0)) ) {
     //    error_log << "cannot load metadata_functs_new.lua: " << lua_tostring(L, -1) << "\n";
     //    return RC_ERR;   
     // }

    return rc;
} 



// int metadata_init (bool load_db, uint64_t job_id, int rank) {
//     int rc;

//     L = luaL_newstate(); //fix - do I ever open a new state or deregister the rank to bounding box functions?
//     luaL_openlibs(L);   


//     //set up and return the database
//     // add_timing_point(DB_SETUP_START);

//     rc = metadata_database_init (load_db, false, job_id, rank);
//     if (rc != RC_OK) {
//         error_log << "Error. Could not init dbs. Error code " << rc << std::endl;
//         add_timing_point(ERR_DB_SETUP);
//     }
//     // add_timing_point(DB_SETUP_DONE);


//     //todo - if we decide user wont supply the entire objector funct, will need to load this
//     // if ( (luaL_loadfile(L, "../lib_source/lua/metadata_functs.lua") || lua_pcall(L, 0, 0, 0)) && (luaL_loadfile(L, "lib_source/lua/metadata_functs.lua") || lua_pcall(L, 0, 0, 0)))
//      // if ( (luaL_loadfile(L, "PATH_TO_EMPRESS/lib_source/lua/metadata_functs.lua") || lua_pcall(L, 0, 0, 0)) ) {
//      //    error_log << "cannot load metadata_functs_new.lua: " << lua_tostring(L, -1) << "\n";
//      //    return RC_ERR;   
//      // }

//     return rc;
// } 

// int metadata_init_multi_db (uint64_t job_id, int rank, uint32_t num_read_procs) {
//     int rc;

//     L = luaL_newstate(); //fix - do I ever open a new state or deregister the rank to bounding box functions?
//     luaL_openlibs(L);   


//     //set up and return the database
//     // add_timing_point(DB_SETUP_START);
//     rc = metadata_database_init (true, true, job_id, rank, num_read_procs);
//     if (rc != RC_OK) {
//         error_log << "Error. Could not init dbs. Error code " << rc << std::endl;
//         add_timing_point(ERR_DB_SETUP);
//     }
//     // add_timing_point(DB_SETUP_DONE);


//     //todo - if we decide user wont supply the entire objector funct, will need to load this
//     // if ( (luaL_loadfile(L, "../lib_source/lua/metadata_functs.lua") || lua_pcall(L, 0, 0, 0)) && (luaL_loadfile(L, "lib_source/lua/metadata_functs.lua") || lua_pcall(L, 0, 0, 0)))
//      // if ( (luaL_loadfile(L, "PATH_TO_EMPRESS/lib_source/lua/metadata_functs.lua") || lua_pcall(L, 0, 0, 0)) ) {
//      //    error_log << "cannot load metadata_functs_new.lua: " << lua_tostring(L, -1) << "\n";
//      //    return RC_ERR;   
//      // }

//     return rc;
// } 

// int metadata_init_multi_db (uint64_t job_id, int rank_first, int rank_last) {
//     int rc;

//     L = luaL_newstate(); //fix - do I ever open a new state or deregister the rank to bounding box functions?
//     luaL_openlibs(L);   


//     //set up and return the database
//     add_timing_point(DB_SETUP_START);
//     rc = metadata_database_init (true, true, job_id, rank_first, rank_last);
//     if (rc != RC_OK) {
//         error_log << "Error. Could not init dbs. Error code " << rc << std::endl;
//         add_timing_point(ERR_DB_SETUP);
//     }
//     add_timing_point(DB_SETUP_DONE);

//     return rc;
// } 


// int metadata_finalize_client(int rank, uint64_t job_id) {
// 	// return metadata_finalize_client(rank, job_id, (md_index_type)0, (md_db_checkpoint_type)0);
// 	return metadata_finalize_client(rank, job_id);
// }


// int metadata_finalize_client(bool output_db, uint64_t job_id, int rank) {
// int metadata_finalize_client(int rank, uint64_t job_id, md_index_type index_type, md_db_checkpoint_type checkpt_type) {
int metadata_finalize_client(int rank, uint64_t job_id) {
    int rc;
    debug_log << "about to close lua state" << endl;

    lua_close(L); //todo - is this fine?
    debug_log << "just closed lua state" << endl;


    // if(output_db) {
    //     rc = sql_output_db(job_id, rank);
    //     if (rc != RC_OK)
    //     {   
    //         fprintf (stderr, "Can't output database for job_id:%llu\n",job_id);        
    //         metadata_database_close();
    //     }
    // }

	// rc = metadata_finalize_server(rank, job_id, index_type, checkpt_type);
	rc = metadata_finalize_server(rank, job_id);
	if(rc != RC_OK) {
		error_log << "error in metadata_finalize_server for rank " << rank << endl;
	}

    return rc;
}

// mark all attribues from the dataset as active making them visible to other
// processes.
int metadata_activate_run_attribute (uint64_t txn_id
                          )
{
    add_timing_point(MD_ACTIVATE_RUN_ATTRIBUTE_START);

    int return_value;
    // int res;

    md_activate_args args;

    args.txn_id = txn_id;

    return_value = 	md_activate_run_attribute_stub(args);

    // deArchiveMsgFromServer(res, return_value);
    add_timing_point(MD_ACTIVATE_RUN_ATTRIBUTE_DONE);

    return return_value;
}

// mark all attribues from the dataset as active making them visible to other
// processes.
int metadata_activate_run (uint64_t txn_id
                          )
{
    add_timing_point(MD_ACTIVATE_RUN_START);

    int return_value;
    // int res;

    md_activate_args args;

    args.txn_id = txn_id;

    return_value = 	md_activate_run_stub(args);

    // deArchiveMsgFromServer(res, return_value);
    add_timing_point(MD_ACTIVATE_RUN_DONE);

    return return_value;
}

// mark all attribues from the dataset as active making them visible to other
// processes.
int metadata_activate_timestep_attribute (uint64_t txn_id
                          )
{
    add_timing_point(MD_ACTIVATE_TIMESTEP_ATTRIBUTE_START);

    int return_value;
    // int res;

    md_activate_args args;

    args.txn_id = txn_id;

    return_value = 	md_activate_timestep_attribute_stub(args);


    // deArchiveMsgFromServer(res, return_value);
    add_timing_point(MD_ACTIVATE_TIMESTEP_ATTRIBUTE_DONE);

    return return_value;
}


// mark all attribues from the dataset as active making them visible to other
// processes.
int metadata_activate_timestep (uint64_t txn_id
                          )
{
    add_timing_point(MD_ACTIVATE_TIMESTEP_START);

    int return_value;
    // int res;

    md_activate_args args;

    args.txn_id = txn_id;

    return_value = 	md_activate_timestep_stub(args);

    // deArchiveMsgFromServer(res, return_value);
    add_timing_point(MD_ACTIVATE_TIMESTEP_DONE);

    return return_value;
}


// mark a variable in the global_catalog as active making it visible to other
// processes.
int metadata_activate_type (uint64_t txn_id
                          )
{
    add_timing_point(MD_ACTIVATE_TYPE_START);

    int return_value;
    // int res;

    md_activate_args args;

    args.txn_id = txn_id;

    return_value = 	md_activate_type_stub(args);

    // deArchiveMsgFromServer(res, return_value);
    add_timing_point(MD_ACTIVATE_TYPE_DONE);   

    return return_value;
}

// mark a variable in the global_catalog as active making it visible to other
// processes.
int metadata_activate_var_attribute (uint64_t txn_id
                          )
{
    add_timing_point(MD_ACTIVATE_VAR_ATTRIBUTE_START);

    int return_value;
    // int res;

    md_activate_args args;

    args.txn_id = txn_id;

    return_value = 	md_activate_var_attribute_stub(args);

    // deArchiveMsgFromServer(res, return_value);
    add_timing_point(MD_ACTIVATE_VAR_ATTRIBUTE_DONE);   

    return return_value;
}


// mark a variable in the global_catalog as active making it visible to other
// processes.
int metadata_activate_var (uint64_t txn_id
                          )
{
    add_timing_point(MD_ACTIVATE_VAR_START);

    int return_value;
    // int res;

    md_activate_args args;

    args.txn_id = txn_id;

    return_value = 	md_activate_var_stub(args);

    // deArchiveMsgFromServer(res, return_value);
    add_timing_point(MD_ACTIVATE_VAR_DONE);   

    return return_value;
}


// static int metadata_activate (uint64_t txn_id
//                           , md_catalog_type catalog_type
//                           )
// {
//     add_timing_point(MD_ACTIVATE_START);

//     int return_value;
//     std::string res;

//     md_activate_args args;

//     args.txn_id = txn_id;
//     args.catalog_type = catalog_type;

//     return_value = md_activate_stub(args);
//     add_timing_point(MD_ACTIVATE_DONE);

//     return return_value;
// } //end of funct

// // mark all attribues from the dataset as active making them visible to other
// // processes.
// int metadata_activate_run_attribute (uint64_t txn_id
//                           )
// {
//     int return_value;
    
//     return_value = metadata_activate ( txn_id, RUN_ATTR_CATALOG);

//     return return_value;
// } //end of funct

// // mark all attribues from the dataset as active making them visible to other
// // processes.
// int metadata_activate_run (uint64_t txn_id
//                           )
// {
//     int return_value;
    
//     return_value = metadata_activate ( txn_id, RUN_CATALOG);

//     return return_value;
// } //end of funct

// // mark all attribues from the dataset as active making them visible to other
// // processes.
// int metadata_activate_timestep_attribute (uint64_t txn_id
//                           )
// {
//     int return_value;
    
//     return_value = metadata_activate ( txn_id, TIMESTEP_ATTR_CATALOG);

//     return return_value;
// } //end of funct


// // mark all attribues from the dataset as active making them visible to other
// // processes.
// int metadata_activate_timestep (uint64_t txn_id
//                           )
// {
//     int return_value;
    
//     return_value = metadata_activate ( txn_id, TIMESTEP_CATALOG);

//     return return_value;
// } //end of funct


// // mark a variable in the global_catalog as active making it visible to other
// // processes.
// int metadata_activate_type (uint64_t txn_id
//                           )
// {
//     int return_value;
    
//     return_value = metadata_activate ( txn_id, TYPE_CATALOG);

//     return return_value;
// } //end of funct

// // mark a variable in the global_catalog as active making it visible to other
// // processes.
// int metadata_activate_var_attribute (uint64_t txn_id
//                           )
// {
//     int return_value;
    
//     return_value = metadata_activate ( txn_id, VAR_ATTR_CATALOG);

//     return return_value;
// } //end of funct


// // mark a variable in the global_catalog as active making it visible to other
// // processes.
// int metadata_activate_var (uint64_t txn_id
//                           )
// {
//     int return_value;
    
//     return_value = metadata_activate ( txn_id, VAR_CATALOG);

//     return return_value;
// } //end of funct


int metadata_catalog_all_run_attributes (uint64_t txn_id
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_run_attribute_entry> &entries
                                        )
{
	int return_value = metadata_catalog_all_run_attributes_in_run (
							ALL_RUNS, txn_id, count, entries
							);
	return return_value;
}                                     

// retrieve the list all of attributes associated with a particular type
int metadata_catalog_all_run_attributes_in_run (uint64_t run_id             
                                        ,uint64_t txn_id
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_run_attribute_entry> &entries
                                        )
{
    add_timing_point(MD_CATALOG_ALL_RUN_ATTRIBUTES_START);
    int return_value;

    md_catalog_all_run_attributes_args args;

    args.txn_id = txn_id;
    args.run_id = run_id;
    extreme_debug_log << "txn_id is " << to_string(txn_id ) << endl;  
   
	entries.clear();
	return_value = md_catalog_all_run_attributes_stub (args, entries, count);

    add_timing_point(MD_CATALOG_ALL_RUN_ATTRIBUTES_DONE);  

    return return_value;
} //end of funct


int metadata_catalog_all_run_attributes_with_type (uint64_t type_id             
                                        ,uint64_t txn_id
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_run_attribute_entry> &entries
                                        )
{
	int return_value = metadata_catalog_all_run_attributes_with_type_in_run (
							ALL_RUNS, type_id, txn_id, count, entries
							);
	return return_value;
}

// retrieve the list all of attributes associated with a particular type
int metadata_catalog_all_run_attributes_with_type_in_run (uint64_t run_id
                                        ,uint64_t type_id             
                                        ,uint64_t txn_id
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_run_attribute_entry> &entries
                                        )
{
    add_timing_point(MD_CATALOG_ALL_RUN_ATTRIBUTES_WITH_TYPE_START);
    int return_value;

    md_catalog_all_run_attributes_with_type_args args;

    args.run_id = run_id;
    args.type_id = type_id;
    args.txn_id = txn_id;
    extreme_debug_log << "txn_id is " << to_string(txn_id ) << endl;  
   	entries.clear();
   	return_value = md_catalog_all_run_attributes_with_type_stub (args, entries, count);

    add_timing_point(MD_CATALOG_ALL_RUN_ATTRIBUTES_WITH_TYPE_DONE);  

    return return_value;
} //end of funct


int metadata_catalog_all_timestep_attributes (uint64_t run_id             
                                        ,uint64_t txn_id
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_timestep_attribute_entry> &entries
                                        )
{
	int return_value = metadata_catalog_all_timestep_attributes_in_timestep (run_id, ALL_TIMESTEPS, txn_id, count, entries
							);

	return return_value;
}

// retrieve the list all of attributes associated with a particular type
int metadata_catalog_all_timestep_attributes_in_timestep (uint64_t run_id             
                                        ,uint64_t timestep_id             
                                        ,uint64_t txn_id
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_timestep_attribute_entry> &entries
                                        )
{
    add_timing_point(MD_CATALOG_ALL_TIMESTEP_ATTRIBUTES_START);
    int return_value;

    md_catalog_all_timestep_attributes_args args;

    args.txn_id = txn_id;
    args.run_id = run_id;
    args.timestep_id = timestep_id;
    extreme_debug_log << "txn_id is " << to_string(txn_id ) << endl;  
   	entries.clear();
    return_value = md_catalog_all_timestep_attributes_stub(args, entries, count);

    add_timing_point(MD_CATALOG_ALL_TIMESTEP_ATTRIBUTES_DONE);  

    return return_value;
} //end of funct

int metadata_catalog_all_timestep_attributes_with_type (uint64_t run_id             
                                        ,uint64_t type_id             
                                        ,uint64_t txn_id
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_timestep_attribute_entry> &entries
                                        )
{
	int return_value = metadata_catalog_all_timestep_attributes_with_type_in_timestep (
							run_id, ALL_TIMESTEPS, type_id, txn_id, count, entries
							);

	return return_value;
}

// retrieve the list all of attributes associated with a particular type
int metadata_catalog_all_timestep_attributes_with_type_in_timestep (uint64_t run_id             
                                        ,uint64_t timestep_id
                                        ,uint64_t type_id             
                                        ,uint64_t txn_id
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_timestep_attribute_entry> &entries
                                        )
{
    add_timing_point(MD_CATALOG_ALL_TIMESTEP_ATTRIBUTES_WITH_TYPE_START);
    int return_value;

    md_catalog_all_timestep_attributes_with_type_args args;

    args.run_id = run_id;
    args.timestep_id = timestep_id;
    args.type_id = type_id;
    args.txn_id = txn_id;
    extreme_debug_log << "txn_id is " << to_string(txn_id ) << endl;  
   	entries.clear();
    return_value = md_catalog_all_timestep_attributes_with_type_stub(args, entries, count);

    add_timing_point(MD_CATALOG_ALL_TIMESTEP_ATTRIBUTES_WITH_TYPE_DONE);  

    return return_value;
} //end of funct

// retrieve the list all of attributes associated with a particular type
int metadata_catalog_all_timesteps_with_var_attributes_with_type_var_dims (uint64_t run_id             
                                        ,uint64_t type_id
                                        ,uint64_t var_id             
                                        ,uint64_t txn_id
                                        ,uint32_t num_dims
                                        ,const std::vector<md_dim_bounds> &dims
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_timestep_entry> &entries
                                        )
{
    add_timing_point(MD_CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_DIMS_START);
    int return_value;
    std::string res;
    
    md_catalog_all_timesteps_with_var_attributes_with_type_var_dims_args args;

    args.run_id = run_id;
    args.type_id = type_id;
    args.var_id = var_id;
    args.txn_id = txn_id;
    args.num_dims = num_dims;
    args.dims = dims;
    extreme_debug_log << "txn_id is " << to_string(txn_id ) << endl;  
   	entries.clear();
    return_value = md_catalog_all_timesteps_with_var_attributes_with_type_var_dims_stub(args, entries, count);
    add_timing_point(MD_CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_DIMS_DONE);  

    return return_value;
} //end of funct


// retrieve the list all of attributes associated with a particular type
int metadata_catalog_all_timesteps_with_var_attributes_with_type_var (uint64_t run_id             
                                        ,uint64_t type_id
                                        ,uint64_t var_id             
                                        ,uint64_t txn_id
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_timestep_entry> &entries
                                        )
{
    add_timing_point(MD_CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_START);
    int return_value;
    std::string res;
    

    md_catalog_all_timesteps_with_var_attributes_with_type_var_args args;

    args.run_id = run_id;
    args.type_id = type_id;
    args.var_id = var_id;
    args.txn_id = txn_id;
    extreme_debug_log << "txn_id is " << to_string(txn_id ) << endl;  
   	entries.clear();
    return_value = md_catalog_all_timesteps_with_var_attributes_with_type_var_stub(args, entries, count);
    add_timing_point(MD_CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_DONE);  

    return return_value;
} //end of funct

int metadata_catalog_all_timesteps_with_var (uint64_t run_id             
                                        ,uint64_t var_id             
                                        ,uint64_t txn_id
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_timestep_entry> &entries
                                        )
{
    add_timing_point(MD_CATALOG_ALL_TIMESTEPS_WITH_VAR_START);
    int return_value;
    std::string res;
    

    md_catalog_all_timesteps_with_var_args args;

    args.run_id = run_id;
    args.var_id = var_id;
    args.txn_id = txn_id;
    extreme_debug_log << "txn_id is " << to_string(txn_id ) << endl;  
   	entries.clear();
    return_value = md_catalog_all_timesteps_with_var_stub(args, entries, count);
    add_timing_point(MD_CATALOG_ALL_TIMESTEPS_WITH_VAR_DONE);  

    return return_value;
} //end of funct

int metadata_catalog_all_types_with_var_attributes_with_var_dims (uint64_t run_id
                      ,uint64_t var_id
                      ,uint64_t txn_id
                      ,uint32_t num_dims
                      ,const std::vector<md_dim_bounds> &dims
                      ,uint32_t &count
                      ,std::vector<md_catalog_type_entry> &entries
                     )
{
	int return_value = metadata_catalog_all_types_with_var_attributes_with_var_dims_in_timestep (
							run_id, ALL_TIMESTEPS, var_id, txn_id, num_dims, dims, count, entries
							);
	return return_value;
}


// return a catalog of types in the metadata type table. 
int metadata_catalog_all_types_with_var_attributes_with_var_dims_in_timestep (uint64_t run_id
                      ,uint64_t timestep_id
                      ,uint64_t var_id
                      ,uint64_t txn_id
                      ,uint32_t num_dims
                      ,const std::vector<md_dim_bounds> &dims
                      ,uint32_t &count
                      ,std::vector<md_catalog_type_entry> &entries
                     )
{
    add_timing_point(MD_CATALOG_ALL_TYPES_WITH_VAR_ATTRIBUTES_WITH_VAR_DIMS_IN_TIMESTEP_START);
    int return_value;
    std::string res;

    md_catalog_all_types_with_var_attributes_with_var_dims_in_timestep_args args;

    args.run_id = run_id;
    args.timestep_id = timestep_id;
    args.var_id = var_id;
    args.txn_id = txn_id;
    args.num_dims = num_dims;
    args.dims = dims;
	entries.clear();
    return_value = md_catalog_all_types_with_var_attributes_with_var_dims_in_timestep_stub(args, entries, count);

    add_timing_point(MD_CATALOG_ALL_TYPES_WITH_VAR_ATTRIBUTES_WITH_VAR_DIMS_IN_TIMESTEP_DONE);   

    return return_value;
} //end of funct

int metadata_catalog_all_types_with_var_attributes_with_var (uint64_t run_id
                      ,uint64_t var_id
                      ,uint64_t txn_id
                      ,uint32_t &count
                      ,std::vector<md_catalog_type_entry> &entries
                     )
{
	int return_value = metadata_catalog_all_types_with_var_attributes_with_var_in_timestep (
							run_id, ALL_TIMESTEPS, var_id, txn_id, count, entries
                     );

	return return_value;
}


// return a catalog of types in the metadata type table. 
int metadata_catalog_all_types_with_var_attributes_with_var_in_timestep (uint64_t run_id
                      ,uint64_t timestep_id
                      ,uint64_t var_id
                      ,uint64_t txn_id
                      ,uint32_t &count
                      ,std::vector<md_catalog_type_entry> &entries
                     )
{
    add_timing_point(MD_CATALOG_ALL_TYPES_WITH_VAR_ATTRIBUTES_WITH_VAR_IN_TIMESTEP_START);
    int return_value;
    std::string res;

    md_catalog_all_types_with_var_attributes_with_var_in_timestep_args args;

    args.run_id = run_id;
    args.timestep_id = timestep_id;
    args.var_id = var_id;
    args.txn_id = txn_id;
	entries.clear();
    return_value = md_catalog_all_types_with_var_attributes_with_var_in_timestep_stub(args, entries, count);

    add_timing_point(MD_CATALOG_ALL_TYPES_WITH_VAR_ATTRIBUTES_WITH_VAR_IN_TIMESTEP_DONE);   

    return return_value;
} //end of funct

// retrieve the list all of attributes associated with a particular type
int metadata_catalog_all_var_attributes (uint64_t run_id             
                                        ,uint64_t timestep_id             
                                        ,uint64_t txn_id
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_var_attribute_entry> &entries
                                        )
{
    add_timing_point(MD_CATALOG_ALL_VAR_ATTRIBUTES_START);
    int return_value;

    md_catalog_all_var_attributes_args args;

    args.txn_id = txn_id;
    args.run_id = run_id;
    args.timestep_id = timestep_id;
    extreme_debug_log << "txn_id is " << to_string(txn_id ) << endl;  
   	entries.clear();
    return_value = md_catalog_all_var_attributes_stub(args, entries, count);

    add_timing_point(MD_CATALOG_ALL_VAR_ATTRIBUTES_DONE);  

    return return_value;
} //end of funct

// retrieve a list of attributes overlapping the given dimensions
int metadata_catalog_all_var_attributes_with_dims (uint64_t run_id             
                                        ,uint64_t timestep_id
                                        ,uint64_t txn_id
                                        ,uint32_t num_dims
                                        ,const std::vector<md_dim_bounds> &dims
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_var_attribute_entry> &entries
                                       )
    {
    add_timing_point(MD_CATALOG_ALL_VAR_ATTRIBUTES_WITH_DIMS_START);

    int return_value;
    md_catalog_all_var_attributes_with_dims_args args;

    args.txn_id = txn_id;
    args.run_id = run_id;
    args.timestep_id = timestep_id;
    args.num_dims = num_dims;
    args.dims = dims;
    // for(int j=0; j< num_dims; j++) {
    //     extreme_debug_log << "in client d" << j << "_min: " << dims [j].min << endl;
    //     extreme_debug_log << "in client d" << j << "_max: " << dims [j].max << endl;               
    // }
	entries.clear();
    return_value = md_catalog_all_var_attributes_with_dims_stub(args, entries, count);
    extreme_debug_log << "just launched get attr _by dims \n";

    add_timing_point(MD_CATALOG_ALL_VAR_ATTRIBUTES_WITH_DIMS_DONE);   
    extreme_debug_log << "number of entries is " << entries.size() << "\n";


    return return_value;
} //end of funct

// retrieve the list all of attributes associated with a particular type
int metadata_catalog_all_var_attributes_with_type_by_id (uint64_t timestep_id
                                        ,uint64_t type_id                   
                                        ,uint64_t txn_id
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_var_attribute_entry> &entries
                                        )
{
    add_timing_point(MD_CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_BY_ID_START);
    int return_value;

    md_catalog_all_var_attributes_with_type_by_id_args args;

    args.txn_id = txn_id;
    args.timestep_id = timestep_id;
    args.type_id = type_id;
    extreme_debug_log << "type id is " << to_string(type_id ) << endl;  
   	entries.clear();
    return_value = md_catalog_all_var_attributes_with_type_by_id_stub(args, entries, count);

    add_timing_point(MD_CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_BY_ID_DONE);  

    return return_value;
} //end of funct


// retrieve the list all of attributes associated with a particular type
int metadata_catalog_all_var_attributes_with_type_by_name_ver (uint64_t run_id             
                                        ,uint64_t timestep_id
                                        ,const std::string &type_name
                                        ,uint32_t type_version                   
                                        ,uint64_t txn_id
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_var_attribute_entry> &entries
                                        )
{
    add_timing_point(MD_CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_BY_NAME_VER_START);
    int return_value;

    md_catalog_all_var_attributes_with_type_by_name_ver_args args;

    args.txn_id = txn_id;
    args.run_id = run_id;
    args.timestep_id = timestep_id;
    args.type_name = type_name;
    args.type_version = type_version;
    // cout << " args.txn_id is " <<  args.txn_id << endl;            
    // cout << " args.run_id is " <<  args.run_id  << endl;        
    // cout << " args.timestep_id is " <<  args.timestep_id << endl;     
    // cout << " args.type_name is " <<  args.type_name  << endl;  
    // cout << " args.type_version is " << args.type_version << endl;  
   	entries.clear();
    return_value = md_catalog_all_var_attributes_with_type_by_name_ver_stub(args, entries, count);

    add_timing_point(MD_CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_BY_NAME_VER_DONE);  

    return return_value;
} //end of funct


// retrieve a list of attributes overlapping the given dimensions
int metadata_catalog_all_var_attributes_with_type_dims_by_id (uint64_t timestep_id
                                        ,uint64_t type_id
                                        ,uint64_t txn_id
                                        ,uint32_t num_dims
                                        ,const std::vector<md_dim_bounds> &dims
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_var_attribute_entry> &entries
                                        )
{
    add_timing_point(MD_CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_DIMS_BY_ID_START);

    int return_value;
    md_catalog_all_var_attributes_with_type_dims_by_id_args args;

    args.txn_id = txn_id;
    args.timestep_id = timestep_id;
    args.type_id = type_id;
    args.num_dims = num_dims;
    args.dims = dims;
	entries.clear();
    return_value = md_catalog_all_var_attributes_with_type_dims_by_id_stub(args, entries, count);
    extreme_debug_log << "just launched get attr _by dims \n";

    add_timing_point(MD_CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_DIMS_BY_ID_DONE);   
    extreme_debug_log << "number of entries is " << entries.size() << "\n";


    return return_value;
} //end of funct


// retrieve a list of attributes overlapping the given dimensions
int metadata_catalog_all_var_attributes_with_type_dims_by_name_ver (uint64_t run_id             
                                        ,uint64_t timestep_id
                                        ,const std::string &type_name
                                        ,uint32_t type_version
                                        ,uint64_t txn_id
                                        ,uint32_t num_dims
                                        ,const std::vector<md_dim_bounds> &dims
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_var_attribute_entry> &entries
                                       )
{
    add_timing_point(MD_CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_DIMS_BY_NAME_VER_START);

    int return_value;
    md_catalog_all_var_attributes_with_type_dims_by_name_ver_args args;

    args.txn_id = txn_id;
    args.run_id = run_id;
    args.timestep_id = timestep_id;
    args.type_name = type_name;
    args.type_version = type_version;
    args.num_dims = num_dims;
    args.dims = dims;
	entries.clear();
    return_value = md_catalog_all_var_attributes_with_type_dims_by_name_ver_stub(args, entries, count);
    extreme_debug_log << "just launched get attr _by dims \n";

    add_timing_point(MD_CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_DIMS_BY_NAME_VER_DONE);   
    extreme_debug_log << "number of entries is " << entries.size() << "\n";


    return return_value;
} //end of funct

// retrieve a list of attributes overlapping the given dimensions
int metadata_catalog_all_var_attributes_with_type_var_by_id (uint64_t timestep_id
                                        ,uint64_t type_id
                                        ,uint64_t var_id
                                        ,uint64_t txn_id
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_var_attribute_entry> &entries
                                        )
{
    add_timing_point(MD_CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_BY_ID_START);

    int return_value;
    md_catalog_all_var_attributes_with_type_var_by_id_args args;

    args.txn_id = txn_id;
    args.timestep_id = timestep_id;
    args.type_id = type_id;
    entries.clear();
    args.var_id = var_id;
    return_value = md_catalog_all_var_attributes_with_type_var_by_id_stub(args, entries, count);
    extreme_debug_log << "just launched get attr _by dims \n";

    add_timing_point(MD_CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_BY_ID_DONE);   
    extreme_debug_log << "number of entries is " << entries.size() << "\n";


    return return_value;
} //end of funct

// retrieve a list of attributes overlapping the given dimensions
int metadata_catalog_all_var_attributes_with_type_var_by_name_ver (uint64_t run_id             
                                        ,uint64_t timestep_id
                                        ,const std::string &type_name
                                        ,uint32_t type_version
                                        ,const std::string &var_name
                                        ,uint32_t var_version
                                        ,uint64_t txn_id
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_var_attribute_entry> &entries
                                        )
{
    add_timing_point(MD_CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_BY_NAME_VER_START);

    int return_value;
    md_catalog_all_var_attributes_with_type_var_by_name_ver_args args;

    args.txn_id = txn_id;
    args.run_id = run_id;
    args.timestep_id = timestep_id;
    args.type_name = type_name;
    args.type_version = type_version;
    args.var_name = var_name;
    args.var_version = var_version;
	entries.clear();
    return_value = md_catalog_all_var_attributes_with_type_var_by_name_ver_stub(args, entries, count);
    extreme_debug_log << "just launched get attr _by dims \n";

    add_timing_point(MD_CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_BY_NAME_VER_DONE);   
    extreme_debug_log << "number of entries is " << entries.size() << "\n";


    return return_value;
} //end of funct

// retrieve a list of attributes overlapping the given dimensions
int metadata_catalog_all_var_attributes_with_type_var_dims_by_id (uint64_t timestep_id
                                        ,uint64_t type_id
                                        ,uint64_t var_id
                                        ,uint64_t txn_id
                                        ,uint32_t num_dims
                                        ,const std::vector<md_dim_bounds> &dims
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_var_attribute_entry> &entries
                                        )
                                        {
    add_timing_point(MD_CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_DIMS_BY_ID_START);

    int return_value;
    md_catalog_all_var_attributes_with_type_var_dims_by_id_args args;

    args.txn_id = txn_id;
    args.timestep_id = timestep_id;
    args.type_id = type_id;
    args.var_id = var_id;
    args.num_dims = num_dims;
    args.dims = dims;
	entries.clear();
    return_value = md_catalog_all_var_attributes_with_type_var_dims_by_id_stub(args, entries, count);
    extreme_debug_log << "just launched get attr _by dims \n";

    add_timing_point(MD_CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_DIMS_BY_ID_DONE);   
    extreme_debug_log << "number of entries is " << entries.size() << "\n";


    return return_value;
} //end of funct

// retrieve a list of attributes overlapping the given dimensions
int metadata_catalog_all_var_attributes_with_type_var_dims_by_name_ver (uint64_t run_id             
                                        ,uint64_t timestep_id
                                        ,const std::string &type_name
                                        ,uint32_t type_version
                                        ,const std::string &var_name
                                        ,uint32_t var_version
                                        ,uint64_t txn_id
                                        ,uint32_t num_dims
                                        ,const std::vector<md_dim_bounds> &dims
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_var_attribute_entry> &entries
                                        )
{
    add_timing_point(MD_CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_DIMS_BY_NAME_VER_START);

    int return_value;
    md_catalog_all_var_attributes_with_type_var_dims_by_name_ver_args args;

    args.txn_id = txn_id;
    args.run_id = run_id;
    args.timestep_id = timestep_id;
    args.type_name = type_name;
    args.type_version = type_version;
    args.var_name = var_name;
    args.var_version = var_version;
    args.num_dims = num_dims;
    args.dims = dims;
	entries.clear();
    return_value = md_catalog_all_var_attributes_with_type_var_dims_by_name_ver_stub(args, entries, count);
    extreme_debug_log << "just launched get attr _by dims \n";

    add_timing_point(MD_CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_DIMS_BY_NAME_VER_DONE);   
    extreme_debug_log << "number of entries is " << entries.size() << "\n";


    return return_value;
} //end of funct

// retrieve a list of attributes overlapping the given dimensions
int metadata_catalog_all_var_attributes_with_var_by_id (uint64_t run_id             
                                        ,uint64_t timestep_id
                                        ,uint64_t var_id
                                        ,uint64_t txn_id
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_var_attribute_entry> &entries
                                        )
{
    add_timing_point(MD_CATALOG_ALL_VAR_ATTRIBUTES_WITH_VAR_BY_ID_START);

    int return_value;
    md_catalog_all_var_attributes_with_var_by_id_args args;

    args.txn_id = txn_id;
    args.run_id = run_id;
    args.timestep_id = timestep_id;
    args.var_id = var_id;
	entries.clear();
    return_value = md_catalog_all_var_attributes_with_var_by_id_stub(args, entries, count);
    extreme_debug_log << "just launched get attr _by dims \n";

    add_timing_point(MD_CATALOG_ALL_VAR_ATTRIBUTES_WITH_VAR_BY_ID_DONE);   
    extreme_debug_log << "number of entries is " << entries.size() << "\n";


    return return_value;
} //end of funct

// retrieve a list of attributes overlapping the given dimensions
int metadata_catalog_all_var_attributes_with_var_by_name_ver (uint64_t run_id             
                                        ,uint64_t timestep_id
                                        ,const std::string &var_name
                                        ,uint32_t var_version
                                        ,uint64_t txn_id
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_var_attribute_entry> &entries
                                        )
{
    add_timing_point(MD_CATALOG_ALL_VAR_ATTRIBUTES_WITH_VAR_BY_NAME_VER_START);

    int return_value;
    md_catalog_all_var_attributes_with_var_by_name_ver_args args;

    args.txn_id = txn_id;
    args.run_id = run_id;
    args.timestep_id = timestep_id;
    args.var_name = var_name;
    args.var_version = var_version;
	entries.clear();
    return_value = md_catalog_all_var_attributes_with_var_by_name_ver_stub(args, entries, count);
    extreme_debug_log << "just launched get attr _by dims \n";

    add_timing_point(MD_CATALOG_ALL_VAR_ATTRIBUTES_WITH_VAR_BY_NAME_VER_DONE);   
    extreme_debug_log << "number of entries is " << entries.size() << "\n";


    return return_value;
} //end of funct

// retrieve a list of attributes overlapping the given dimensions
int metadata_catalog_all_var_attributes_with_var_dims_by_id (uint64_t run_id             
                                        ,uint64_t timestep_id
                                        ,uint64_t var_id
                                        ,uint64_t txn_id
                                        ,uint32_t num_dims
                                        ,const std::vector<md_dim_bounds> &dims
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_var_attribute_entry> &entries
                                        )
{
    add_timing_point(MD_CATALOG_ALL_VAR_ATTRIBUTES_WITH_VAR_DIMS_BY_ID_START);

    int return_value;
    md_catalog_all_var_attributes_with_var_dims_by_id_args args;

    args.txn_id = txn_id;
    args.run_id = run_id;
    args.timestep_id = timestep_id;
    args.var_id = var_id;
    args.num_dims = num_dims;
    args.dims = dims;
	entries.clear();
    return_value = md_catalog_all_var_attributes_with_var_dims_by_id_stub(args, entries, count);
    extreme_debug_log << "just launched get attr _by dims \n";

    add_timing_point(MD_CATALOG_ALL_VAR_ATTRIBUTES_WITH_VAR_DIMS_BY_ID_DONE);   
    extreme_debug_log << "number of entries is " << entries.size() << "\n";


    return return_value;
} //end of funct


// retrieve a list of attributes overlapping the given dimensions
int metadata_catalog_all_var_attributes_with_var_dims_by_name_ver (uint64_t run_id             
                                        ,uint64_t timestep_id
                                        ,const std::string &var_name
                                        ,uint32_t var_version                       
                                        ,uint64_t txn_id
                                        ,uint32_t num_dims
                                        ,const std::vector<md_dim_bounds> &dims
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_var_attribute_entry> &entries
                                        )
{
    add_timing_point(MD_CATALOG_ALL_VAR_ATTRIBUTES_WITH_VAR_DIMS_BY_NAME_VER_START);

    int return_value;
    md_catalog_all_var_attributes_with_var_dims_by_name_ver_args args;

    args.txn_id = txn_id;
    args.run_id = run_id;
    args.timestep_id = timestep_id;
    args.var_name = var_name;
    args.var_version = var_version;
    args.num_dims = num_dims;
    args.dims = dims;
	entries.clear();
    return_value = md_catalog_all_var_attributes_with_var_dims_by_name_ver_stub(args, entries, count);
    extreme_debug_log << "just launched get attr _by dims \n";

    add_timing_point(MD_CATALOG_ALL_VAR_ATTRIBUTES_WITH_VAR_DIMS_BY_NAME_VER_DONE);   
    extreme_debug_log << "number of entries is " << entries.size() << "\n";


    return return_value;
} //end of funct

// return a catalog of items in the metadata service. For scalars, the num_dims
// will be 0
int metadata_catalog_run (uint64_t txn_id
                      ,uint32_t &count
                      ,std::vector<md_catalog_run_entry> &entries
                     )
{
    add_timing_point(MD_CATALOG_RUN_START);

    int return_value;

    // std::string res2;
    md_catalog_run_args args;
    args.txn_id = txn_id;
	entries.clear();
    return_value = md_catalog_run_stub(args, entries, count);
   
    add_timing_point(MD_CATALOG_RUN_DONE);

    return return_value;
} //end of funct

// return a catalog of items in the metadata service. For scalars, the num_dims
// will be 0
int metadata_catalog_timestep (uint64_t run_id            
                      ,uint64_t txn_id
                      ,uint32_t &count
                      ,std::vector<md_catalog_timestep_entry> &entries
                     )
{
    add_timing_point(MD_CATALOG_TIMESTEP_START);

    int return_value;
    std::string res;

    md_catalog_timestep_args args;

    args.run_id = run_id;
    args.txn_id = txn_id;
	entries.clear();
    return_value = md_catalog_timestep_stub(args, entries, count);   
    add_timing_point(MD_CATALOG_TIMESTEP_DONE);

    return return_value;
} //end of funct

// return a catalog of types in the metadata type table. 
int metadata_catalog_type (uint64_t run_id
                      ,uint64_t txn_id
                      ,uint32_t &count
                      ,std::vector<md_catalog_type_entry> &entries
                     )
{
    add_timing_point(MD_CATALOG_TYPE_START);
    int return_value;
    std::string res;

    md_catalog_type_args args;

    args.txn_id = txn_id;
    args.run_id = run_id;
	entries.clear();
    return_value = md_catalog_type_stub(args, entries, count);

    add_timing_point(MD_CATALOG_TYPE_DONE);   

    return return_value;
} //end of funct

// return a catalog of items in the metadata service. For scalars, the num_dims
// will be 0
int metadata_catalog_var (uint64_t run_id
                      ,uint64_t timestep_id
                      ,uint64_t txn_id
                      ,uint32_t &count
                      ,std::vector<md_catalog_var_entry> &entries
                     )
{
    add_timing_point(MD_CATALOG_VAR_START);

    int return_value;
    std::string res;

    md_catalog_var_args args;

    args.txn_id = txn_id;
    args.run_id = run_id;
    args.timestep_id = timestep_id;
	entries.clear();
    return_value = md_catalog_var_stub(args, entries, count);   
    add_timing_point(MD_CATALOG_VAR_DONE);

    return return_value;
} //end of funct


// //checkpoint the database to disk
// int metadata_checkpoint_database (int proc_rank
// 					  ,uint64_t job_id
//                       ,md_db_checkpoint_type checkpt_type
//                       ,md_write_type write_type
//                       )
// {
//     add_timing_point(MD_CHECKPOINT_DATABASE_START);

//    	int return_value = checkpt_db(proc_rank, job_id);

    
//     // return_value = md_checkpoint_database_stub(job_id, checkpt_type);   
//     add_timing_point(MD_CHECKPOINT_DATABASE_DONE);

//     return return_value;
// } //end of funct

// create a run  table in an inactive state. var_s will be added later. Once the
// run table is complete (the transaction is ready to commit), it can then be
// activated to make it visible to other processes.
int metadata_create_run  (uint64_t &run_id   
                        ,uint64_t job_id
                        ,const std::string &name
                        // ,const std::string &path
                        ,uint64_t txn_id 
                        ,uint64_t npx
                        ,uint64_t npy
                        ,uint64_t npz
                        ,const std::string &rank_to_dims_funct_name
                        ,const std::string &rank_to_dims_funct_path
                        ,const std::string &objector_funct_name
                        ,const std::string &objector_funct_path
                        )
{
    add_timing_point(MD_CREATE_RUN_START);
    extreme_debug_log << "got to top of metadata_create_run  \n";
    int return_value;
    std::string res;

    md_create_run_args args;
    args.job_id = job_id;
    args.name = name;
    // args.path = path;
    args.txn_id = txn_id;
    args.npx = npx;
    args.npy = npy;
    args.npz = npz;


    return_value = stringify_function(rank_to_dims_funct_path, rank_to_dims_funct_name, args.rank_to_dims_funct);
    if (return_value != RC_OK) {
        error_log << "Error. Was unable to stringify the rank to bounding box function. Exitting \n";
        return return_value;
    }

    return_value = stringify_function(objector_funct_path, objector_funct_name, args.objector_funct);
    if (return_value != RC_OK) {
        error_log << "Error. Was unable to stringify the boundingBoxToObjectNamesAndCounts function. Exitting \n";
        return return_value;
    }

    // extreme_debug_log << "name: " << name << " path: " << path << " txn_id: " << txn_id << " npx: " << npx << " npy: " << npy << " npz: " << npz << endl;
    // extreme_debug_log << "rank_to_dims_funct.size(): " << rank_to_dims_funct.size() << " objector_funct.size(): " << objector_funct.size() << endl;
    // extreme_debug_log << "rank_to_dims_funct: " << rank_to_dims_funct << " objector_funct: " << objector_funct << endl;

    extreme_debug_log << "about to create new run  \n";
    
    return_value = md_create_run_stub(args, run_id);    
    extreme_debug_log << "about to get future" <<endl;    extreme_debug_log << "about to launch op" <<endl;

    extreme_debug_log << "about to get final result" <<endl;

    add_timing_point(MD_CREATE_RUN_DONE); 

    // return_value = register_objector_funct_write (objector_funct_name, run_id);
    // if (return_value != RC_OK) {
    //     error_log << "error in registering the objector function in write" << endl;
    // }

    return return_value;
} //end of funct

// create a timestep  table in an inactive state. var_s will be added later. Once the
// timestep table is complete (the transaction is ready to commit), it can then be
// activated to make it visible to other processes.
int metadata_create_timestep  (uint64_t timestep_id                        
                        ,uint64_t run_id   
                        ,const std::string &path                     
                        ,uint64_t txn_id 
                        )
{
    add_timing_point(MD_CREATE_TIMESTEP_START);
    extreme_debug_log << "got to top of metadata_create_timestep  \n";
    int return_value;
    std::string res;

    md_create_timestep_args args;
    args.timestep_id = timestep_id;
    args.run_id = run_id;
    args.path = path;
    // extreme_debug_log << "in client, path: " << path << endl;
    args.txn_id = txn_id;
    extreme_debug_log << "about to create new timestep  \n";
    extreme_debug_log << "timestep id: " << timestep_id << " run_id: " << run_id << "  \n";

    
    // return_value = md_create_timestep_stub(args, timestep_id);  
    return_value = md_create_timestep_stub(args);      
    extreme_debug_log << "about to get future" <<endl;    extreme_debug_log << "about to launch op" <<endl;

    extreme_debug_log << "about to get final result" <<endl;

    add_timing_point(MD_CREATE_TIMESTEP_DONE); 

    return return_value;
} //end of funct
    

// // create a type in an inactive state. attributes will be added later. Once the
// // type is complete (the transaction is ready to commit), it can then be
// // activated to make it visible to other processes.
// int metadata_create_type (uint64_t &type_id
//                         ,uint64_t run_id
//                         ,const std::string &name
//                         ,uint32_t version
//                         ,uint64_t txn_id
//                         )
// {

//note - could be modified to take indv parameters (see directly above) or to take a type_id from the user
int metadata_create_type (uint64_t &type_id
                        ,const md_catalog_type_entry &new_type
                        )
{
    add_timing_point(MD_CREATE_TYPE_START);
    extreme_debug_log << "got to top of metadata_create_type \n";
    int return_value;
    std::string res;

    md_create_type_args args;

    args.run_id = new_type.run_id;
    args.name = new_type.name;
    args.version = new_type.version;
    args.txn_id = new_type.txn_id;
    extreme_debug_log << "about to create new type \n";
    
    return_value = md_create_type_stub(args, type_id);
    extreme_debug_log << "about to get future" <<endl;    extreme_debug_log << "about to launch op" <<endl;

    extreme_debug_log << "about to get final result" <<endl;

    add_timing_point(MD_CREATE_TYPE_DONE);   

    return return_value;
} //end of funct

// create a variable in an inactive state. Chunks will be added later. Once the
// variable is complete (the transaction is ready to commit), it can then be
// activated to make it visible to other processes.
// For a scalar, num_dims should be 0 causing the other parameters
// to be ignored.
int metadata_create_var (uint64_t run_id
                        ,uint64_t timestep_id
                        ,uint64_t var_id
                        ,const std::string &name
                        ,const std::string &path
                        ,uint32_t version
                        ,char data_size
                        ,uint64_t txn_id
                        ,uint32_t num_dims
                        ,const std::vector<md_dim_bounds> &dims
                        )
{
	add_timing_point(MD_CREATE_VAR_START);
    extreme_debug_log << "got to top of metadata_create_var \n";
    int return_value;
    std::string res;

    md_create_var_args args;

    args.run_id = run_id;
    args.var_id = var_id;
    args.timestep_id = timestep_id;
    args.name = name;
    args.path = path;
    args.version = version;
    args.data_size = data_size;
    args.txn_id = txn_id;
    args.num_dims = num_dims;
    extreme_debug_log << "run_id: " << args.run_id << " timestep_id: " << args.timestep_id << " var_id: " << args.var_id << " \n";

    args.dims = dims;
    extreme_debug_log << "about to create new var \n";
    
    // return_value = md_create_var_stub(args, var_id); 
    return_value = md_create_var_stub(args);       
    extreme_debug_log << "about to get future" <<endl;    extreme_debug_log << "about to launch op" <<endl;

    extreme_debug_log << "about to get final result" <<endl;

    // uint64_t rowid; //todo
    // var_id = args.var_id;
    add_timing_point(MD_CREATE_VAR_DONE); 

    return return_value;
}

//note - could be modified to take indv parameters (see directly above) or to NOT take a var_id from the user
int metadata_create_var (uint64_t &var_id
                        ,const md_catalog_var_entry &new_var
                        )
{
    add_timing_point(MD_CREATE_VAR_START);
    extreme_debug_log << "got to top of metadata_create_var \n";
    int return_value;
    std::string res;

    md_create_var_args args;

    ///////todo/////////////////////////////////////////////////////////////////////
    // string temp;
    // size_t size;
    // for(int i=0; i< new_var.name.size(); i++) {
    //     temp += to_string(((int)new_var.name.at(i)));
    // }
    // temp += to_string((int)'_');
    // temp += to_string(new_var.version);
    // extreme_debug_log << "temp: " << temp << endl;
    // args.var_id = stoull(temp, &size, 10); //todo
   //  string var_name_ver = new_var.name;
   //  var_name_ver += "_";
   //  var_name_ver += to_string(new_var.version);
   //  extreme_debug_log << "var_name_ver: " << var_name_ver << endl;
   //  map <string, uint64_t>::iterator it = var_ids.find(var_name_ver);
   //  if ( it == var_ids.end() ) {
   //    // not found
   //      var_ids[var_name_ver] = var_ids.size();
   //      args.var_id = var_ids.size();

   //      extreme_debug_log << "not found, and var_ids.size(): " << var_ids.size() << endl;
   //  } else {
   //    // found
   //      extreme_debug_log << "found, var_id: " << it->second << endl;
   //      args.var_id = it->second;
   //  }

   // for (std::map<string,uint64_t>::iterator it=var_ids.begin(); it!=var_ids.end(); ++it) {
   //    string var_name = it->first;
   //    uint64_t temp_var_id = it->second;
   //    extreme_debug_log << "var_name: " << var_name << " temp_var_id " << temp_var_id << endl;
   //  }
   //  extreme_debug_log << "args.var_id: " << args.var_id << endl;

    //////////////////////////////////////////////////////////////////////////////////
    args.run_id = new_var.run_id;
    args.var_id = new_var.var_id;
    args.timestep_id = new_var.timestep_id;
    args.name = new_var.name;
    args.path = new_var.path;
    args.version = new_var.version;
    args.data_size = new_var.data_size;
    args.txn_id = new_var.txn_id;
    args.num_dims = new_var.num_dims;
    extreme_debug_log << "run_id: " << new_var.run_id << " timestep_id: " << new_var.timestep_id << " var_id: " << args.var_id << " \n";

    args.dims = new_var.dims;
    extreme_debug_log << "about to create new var \n";
    
    // return_value = md_create_var_stub(args, var_id);    
    return_value = md_create_var_stub(args);    
    extreme_debug_log << "about to get future" <<endl;    extreme_debug_log << "about to launch op" <<endl;

    extreme_debug_log << "about to get final result" <<endl;

    uint64_t rowid; //todo
    var_id = args.var_id;
    add_timing_point(MD_CREATE_VAR_DONE); 

    return return_value;
} //end of funct

// delete the specified run
int metadata_delete_run_by_id (uint64_t run_id
                        )
{
    add_timing_point(MD_DELETE_RUN_BY_ID_START);

    int return_value;
    std::string res;
    md_delete_run_by_id_args args;

    args.run_id = run_id;

    return_value = md_delete_run_by_id_stub(args);
    add_timing_point(MD_DELETE_RUN_BY_ID_DONE);

    return return_value;
} //end of funct

// delete the specified timestep
int metadata_delete_timestep_by_id (uint64_t timestep_id
                        ,uint64_t run_id
                        )
{
    add_timing_point(MD_DELETE_TIMESTEP_BY_ID_START);

    int return_value;
    std::string res;
    md_delete_timestep_by_id_args args;

    args.timestep_id = timestep_id;
    args.run_id = run_id;

    return_value = md_delete_timestep_by_id_stub(args);
    add_timing_point(MD_DELETE_TIMESTEP_BY_ID_DONE);

    return return_value;
} //end of funct

// // delete the specified dataset
// int metadata_delete_dataset_by_name_path_timestep (const std::string &name
//                         ,const std::string &path
//                         ,uint64_t timestep
//                         )
// {
//     add_timing_point(MD_DELETE_DATASET_BY_NAME_PATH_TIMESTEP_START);

//     int return_value;
//     std::string res;
//     md_delete_dataset_by_name_path_timestep_args args;

//     args.name = name;
//     args.path = path;
//     args.timestep = timestep;

//     delete_DatasetByNamePathtimestep_stub = new delete_DatasetByNamePathtimestepMeta(args, entries, count);// 
//     add_timing_point(MD_DELETE_DATASET_BY_NAME_PATH_TIMESTEP_DONE);

//     return return_value;
// }


// delete the given type and all of the attributes associated with it
int metadata_delete_type_by_id (uint64_t type_id
                        )
{
    add_timing_point(MD_DELETE_TYPE_BY_ID_START);

    int return_value;
    std::string res;
    md_delete_type_by_id_args args;

    // args.dataset_id = dataset_id;
    args.type_id = type_id;

    return_value = md_delete_type_by_id_stub(args);
    add_timing_point(MD_DELETE_TYPE_BY_ID_DONE);

    return return_value;
} //end of funct

// delete the given type and all of the attributes associated with it
int metadata_delete_type_by_name_ver (uint64_t run_id
                        ,const std::string &name
                        ,uint32_t version
                        )
{
    add_timing_point(MD_DELETE_TYPE_BY_NAME_VER_START);

    int return_value;
    std::string res;
    md_delete_type_by_name_ver_args args;

    args.run_id = run_id;
    args.name = name;
    args.version = version;

    return_value = md_delete_type_by_name_ver_stub(args);
    add_timing_point(MD_DELETE_TYPE_BY_NAME_VER_DONE);

    return return_value;
} //end of funct


// delete the specified var
int metadata_delete_var_by_id (uint64_t run_id
                        ,uint64_t timestep_id
                        ,uint64_t var_id
                        )
{
    add_timing_point(MD_DELETE_VAR_BY_ID_START);

    int return_value;
    std::string res;
    md_delete_var_by_id_args args;

    args.run_id = run_id;
    args.timestep_id = timestep_id;
    args.var_id = var_id;

    return_value = md_delete_var_by_id_stub(args);
    add_timing_point(MD_DELETE_VAR_BY_ID_DONE);

    return return_value;
} //end of funct


// delete the specified var
int metadata_delete_var_by_name_path_ver (uint64_t run_id
                        ,uint64_t timestep_id
                        ,const std::string &name
                        ,const std::string &path
                        ,uint32_t version
                        )
{
    add_timing_point(MD_DELETE_VAR_BY_NAME_PATH_VER_START);

    int return_value;
    std::string res;
    md_delete_var_by_name_path_ver_args args;

    args.run_id = run_id;
    args.timestep_id = timestep_id;
    args.name = name;
    args.path = path;
    args.version = version;

    return_value = md_delete_var_by_name_path_ver_stub(args);
    add_timing_point(MD_DELETE_VAR_BY_NAME_PATH_VER_DONE);

    return return_value;
} //end of funct



// insert a attribute of a bounding box
int metadata_insert_run_attribute (uint64_t &attribute_id
						   ,uint64_t run_id
                           ,uint64_t type_id
                           ,uint64_t txn_id
                           ,attr_data_type data_type
                           ,const std::string &data
                           )
{
    add_timing_point(MD_INSERT_RUN_ATTRIBUTE_START);
    int return_value;
    std::string res;

    // if(time_pts.size() != catg_of_time_pts.size()) {
    //     debug_log << "error they are off at start of md insert attributes " << endl;
    // }
    md_insert_run_attribute_args args;
    args.run_id = run_id;
    args.type_id = type_id;
    args.txn_id = txn_id;
    args.data_type = data_type;
    args.data = data;

    return_value = md_insert_run_attribute_stub(args, attribute_id);
    add_timing_point(MD_INSERT_RUN_ATTRIBUTE_DONE);   
    debug_log << "according to the client the attribute_id is " << attribute_id << endl;
    // if(time_pts.size() != catg_of_time_pts.size()) {
    //     debug_log << "error they are off at the end of md insert attributes " << endl;
    // }
    return return_value;
} //end of funct

int metadata_insert_run_attribute (uint64_t &attribute_id
                           ,const md_catalog_run_attribute_entry &new_attribute)
{
    add_timing_point(MD_INSERT_RUN_ATTRIBUTE_START);
    int return_value;
    std::string res;

    // if(time_pts.size() != catg_of_time_pts.size()) {
    //     debug_log << "error they are off at start of md insert attributes " << endl;
    // }
    md_insert_run_attribute_args args;
    args.run_id = new_attribute.run_id;
    args.type_id = new_attribute.type_id;
    args.txn_id = new_attribute.txn_id;
    args.data_type = new_attribute.data_type;
    args.data = new_attribute.data;

    return_value = md_insert_run_attribute_stub(args, attribute_id);
    add_timing_point(MD_INSERT_RUN_ATTRIBUTE_DONE);   
    debug_log << "according to the client the attribute_id is " << attribute_id << endl;
    // if(time_pts.size() != catg_of_time_pts.size()) {
    //     debug_log << "error they are off at the end of md insert attributes " << endl;
    // }
    return return_value;
} //end of funct


// insert a attribute of a bounding box
int metadata_insert_timestep_attribute (uint64_t &attribute_id
                           ,uint64_t timestep_id
                           ,uint64_t type_id
                           ,uint64_t txn_id
                           ,attr_data_type data_type
                           ,const std::string &data
                           )
{
    add_timing_point(MD_INSERT_TIMESTEP_ATTRIBUTE_START);
    int return_value;
    std::string res;

    // if(time_pts.size() != catg_of_time_pts.size()) {
    //     debug_log << "error they are off at start of md insert attributes " << endl;
    // }
    md_insert_timestep_attribute_args args;
    args.timestep_id = timestep_id;
    args.type_id = type_id;
    args.txn_id = txn_id;
    args.data_type = data_type;
    args.data = data;

    return_value = md_insert_timestep_attribute_stub(args, attribute_id);
    add_timing_point(MD_INSERT_TIMESTEP_ATTRIBUTE_DONE);   
    debug_log << "according to the client the attribute_id is " << attribute_id << endl;
    // if(time_pts.size() != catg_of_time_pts.size()) {
    //     debug_log << "error they are off at the end of md insert attributes " << endl;
    // }
    return return_value;
} //end of funct

int metadata_insert_timestep_attribute (uint64_t &attribute_id,
                           const md_catalog_timestep_attribute_entry &new_attribute)
{
    add_timing_point(MD_INSERT_TIMESTEP_ATTRIBUTE_START);
    int return_value;
    std::string res;

    // if(time_pts.size() != catg_of_time_pts.size()) {
    //     debug_log << "error they are off at start of md insert attributes " << endl;
    // }
    md_insert_timestep_attribute_args args;
    args.timestep_id = new_attribute.timestep_id;
    args.type_id = new_attribute.type_id;
    args.txn_id = new_attribute.txn_id;
    args.data_type = new_attribute.data_type;
    args.data = new_attribute.data;

    return_value = md_insert_timestep_attribute_stub(args, attribute_id);
    add_timing_point(MD_INSERT_TIMESTEP_ATTRIBUTE_DONE);   
    debug_log << "according to the client the attribute_id is " << attribute_id << endl;
    // if(time_pts.size() != catg_of_time_pts.size()) {
    //     debug_log << "error they are off at the end of md insert attributes " << endl;
    // }
    return return_value;
} //end of funct


// insert a attribute of a bounding box
int metadata_insert_var_attribute_by_dims (uint64_t &attribute_id
                           ,uint64_t timestep_id
                           ,uint64_t type_id
                           ,uint64_t var_id
                           ,uint64_t txn_id
                           ,uint32_t num_dims
                           ,const std::vector<md_dim_bounds> &dims
                           ,attr_data_type data_type
                           ,const std::string &data
                           )
{
    add_timing_point(MD_INSERT_VAR_ATTRIBUTE_BY_DIMS_START);
    int return_value;
    std::string res;

    // if(time_pts.size() != catg_of_time_pts.size()) {
    //     debug_log << "error they are off at start of md insert attributes " << endl;
    // }
    md_insert_var_attribute_by_dims_args args;
    args.timestep_id = timestep_id;
    args.type_id = type_id;
    args.var_id = var_id;
    args.txn_id = txn_id;
    args.num_dims = num_dims;
    args.dims = dims;
    args.data_type = data_type;
    args.data = data;

    return_value = md_insert_var_attribute_by_dims_stub(args, attribute_id);
    add_timing_point(MD_INSERT_VAR_ATTRIBUTE_BY_DIMS_DONE);   
    debug_log << "according to the client the attribute_id is " << attribute_id << endl;
    // if(time_pts.size() != catg_of_time_pts.size()) {
    //     debug_log << "error they are off at the end of md insert attributes " << endl;
    // }
    return return_value;
} //end of funct

int metadata_insert_var_attribute_by_dims (uint64_t &attribute_id
                           ,const md_catalog_var_attribute_entry &new_attribute)
{
    add_timing_point(MD_INSERT_VAR_ATTRIBUTE_BY_DIMS_START);
    int return_value;
    std::string res;

    // if(time_pts.size() != catg_of_time_pts.size()) {
    //     debug_log << "error they are off at start of md insert attributes " << endl;
    // }
    md_insert_var_attribute_by_dims_args args;
    args.timestep_id = new_attribute.timestep_id;
    args.type_id = new_attribute.type_id;
    args.var_id = new_attribute.var_id;
    args.txn_id = new_attribute.txn_id;
    args.num_dims = new_attribute.num_dims;
    args.dims = new_attribute.dims;
    args.data_type = new_attribute.data_type;
    args.data = new_attribute.data;

    return_value = md_insert_var_attribute_by_dims_stub(args, attribute_id);
    add_timing_point(MD_INSERT_VAR_ATTRIBUTE_BY_DIMS_DONE);   
    debug_log << "according to the client the attribute_id is " << attribute_id << endl;
    // if(time_pts.size() != catg_of_time_pts.size()) {
    //     debug_log << "error they are off at the end of md insert attributes " << endl;
    // }
    return return_value;
} //end of funct


static int metadata_processing (uint64_t txn_id
                          , md_catalog_type catalog_type                          
                          )
{
    add_timing_point(MD_PROCESSING_START);

    int return_value;
    std::string res;
    md_processing_args args;

    args.txn_id = txn_id;
    args.catalog_type = catalog_type;

    return_value = md_processing_stub(args);
    add_timing_point(MD_PROCESSING_DONE);

    return return_value;
} //end of funct

// mark a run_attributeiable in the global_catalog as processing making it invisible to
// other processes.
int metadata_processing_run_attribute (uint64_t txn_id
                          )
{
    int return_value;
    
    return_value = metadata_processing ( txn_id, RUN_ATTR_CATALOG);

    return return_value;
} //end of funct


// mark a runiable in the global_catalog as processing making it invisible to
// other processes.
int metadata_processing_run (uint64_t txn_id
                          )
{
    int return_value;
    
    return_value = metadata_processing ( txn_id, RUN_CATALOG);

    return return_value;
} //end of funct

// mark a timestep_attributeiable in the global_catalog as processing making it invisible to
// other processes.
int metadata_processing_timestep_attribute (uint64_t txn_id
                          )
{
    int return_value;
    
    return_value = metadata_processing ( txn_id, TIMESTEP_ATTR_CATALOG);

    return return_value;
} //end of funct


// mark a timestep table in the global_catalog as processing making it invisible to
// other processes.
int metadata_processing_timestep (uint64_t txn_id
                          )
{
    int return_value;
    
    return_value = metadata_processing ( txn_id, TIMESTEP_CATALOG);

    return return_value;
} //end of funct


// mark a variable in the global_catalog as processing making it invisible to
// other processes.
int metadata_processing_type (uint64_t txn_id
                          )
{
    int return_value;
    
    return_value = metadata_processing ( txn_id, TYPE_CATALOG);

    return return_value;
} //end of funct

// mark all var_attributes from the given transaction as processing making it invisible to
// other processes.
int metadata_processing_var_attribute  (uint64_t txn_id
                          )
{
    int return_value;
    
    return_value = metadata_processing ( txn_id, VAR_ATTR_CATALOG);

    return return_value;
} //end of funct

// mark all vars from the given transaction as processing making it invisible to
// other processes.
int metadata_processing_var  (uint64_t txn_id
                          )
{
    int return_value;
    
    return_value = metadata_processing ( txn_id, VAR_CATALOG);

    return return_value;
} //end of funct



static int metadata_catalog_all_run_attributes_with_type_val (data_range_type type
                                        ,uint64_t run_id
                                        ,uint64_t type_id             
                                        ,uint64_t txn_id
                                        ,attr_data_type data_type
                                        ,const std::string &data
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_run_attribute_entry> &entries
                                        )
{
    add_timing_point(MD_CATALOG_ALL_RUN_ATTRIBUTES_WITH_TYPE_RANGE_START);
    int return_value;
    std::string res;    

    md_catalog_all_run_attributes_with_type_range_args args;

    args.range_type = type;
    args.run_id = run_id;
    args.type_id = type_id;
    args.txn_id = txn_id;
    args.data_type = data_type;
    // args.data = data;
    // extreme_debug_log << "run_id: " << run_id << " type_id: " << type_id << " txn_id: " << txn_id << " data_type: " << data_type <<
    //     " data: " << data << endl; 

    extreme_debug_log << "txn_id is " << to_string(txn_id ) << endl;  
   	entries.clear();

   	//note - could just templatize this instead
	switch(data_type) {
        case ATTR_DATA_TYPE_INT : {
        	uint64_t min_int, max_int;

    	    get_range_values(data, type, min_int, max_int);

    		return_value = md_catalog_all_run_attributes_with_type_range_stub(args, 
    			min_int, max_int, entries, count);

            break;
        }
        case ATTR_DATA_TYPE_REAL : {
        	long double min_real, max_real;
    	    
    	    get_range_values(data, type, min_real, max_real);

    		return_value = md_catalog_all_run_attributes_with_type_range_stub(args, 
    			min_real, max_real, entries, count);

            break;
        }
    }   

    // return_value = md_catalog_all_run_attributes_with_type_range_stub(args, entries, count);
    add_timing_point(MD_CATALOG_ALL_RUN_ATTRIBUTES_WITH_TYPE_RANGE_DONE);  

    return return_value;
} //end of funct

int metadata_catalog_all_run_attributes_with_type_range (
										 uint64_t type_id             
                                        ,uint64_t txn_id
                                        ,attr_data_type data_type
                                        ,const std::string &data
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_run_attribute_entry> &entries
                                        )
{
    int return_value;

    return_value = metadata_catalog_all_run_attributes_with_type_val (DATA_RANGE, ALL_RUNS, type_id, txn_id, data_type, data, count, entries);

    return return_value;
} //end of funct

int metadata_catalog_all_run_attributes_with_type_above_max (
										 uint64_t type_id             
                                        ,uint64_t txn_id
                                        ,attr_data_type data_type
                                        ,const std::string &data
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_run_attribute_entry> &entries
                                        )
{
    int return_value;

    return_value = metadata_catalog_all_run_attributes_with_type_val (DATA_MAX, ALL_RUNS, type_id, txn_id, data_type, data, count, entries);

    return return_value;
} //end of funct

int metadata_catalog_all_run_attributes_with_type_below_min (
										 uint64_t type_id             
                                        ,uint64_t txn_id
                                        ,attr_data_type data_type
                                        ,const std::string &data
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_run_attribute_entry> &entries
                                        )
{
    int return_value;

    return_value = metadata_catalog_all_run_attributes_with_type_val (DATA_MIN, ALL_RUNS, type_id, txn_id, data_type, data, count, entries);

    return return_value;
} //end of funct


int metadata_catalog_all_run_attributes_with_type_range_in_run (uint64_t run_id
                                        ,uint64_t type_id             
                                        ,uint64_t txn_id
                                        ,attr_data_type data_type
                                        ,const std::string &data
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_run_attribute_entry> &entries
                                        )
{
    int return_value;

    return_value = metadata_catalog_all_run_attributes_with_type_val (DATA_RANGE, run_id, type_id, txn_id, data_type, data, count, entries);

    return return_value;
} //end of funct

int metadata_catalog_all_run_attributes_with_type_above_max_in_run (uint64_t run_id
                                        ,uint64_t type_id             
                                        ,uint64_t txn_id
                                        ,attr_data_type data_type
                                        ,const std::string &data
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_run_attribute_entry> &entries
                                        )
{
    int return_value;

    return_value = metadata_catalog_all_run_attributes_with_type_val (DATA_MAX, run_id, type_id, txn_id, data_type, data, count, entries);

    return return_value;
} //end of funct

int metadata_catalog_all_run_attributes_with_type_below_min_in_run (uint64_t run_id
                                        ,uint64_t type_id             
                                        ,uint64_t txn_id
                                        ,attr_data_type data_type
                                        ,const std::string &data
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_run_attribute_entry> &entries
                                        )
{
    int return_value;

    return_value = metadata_catalog_all_run_attributes_with_type_val (DATA_MIN, run_id, type_id, txn_id, data_type, data, count, entries);

    return return_value;
} //end of funct

static int metadata_catalog_all_timesteps_with_var_attributes_with_type_var_val (data_range_type type
                                        ,uint64_t run_id             
                                        ,uint64_t type_id
                                        ,uint64_t var_id             
                                        ,uint64_t txn_id
                                        ,attr_data_type data_type
                                        ,const std::string &data                                        
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_timestep_entry> &entries
                                        )
{
    add_timing_point(MD_CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_RANGE_START);
    int return_value;
    std::string res;
    

    md_catalog_all_timesteps_with_var_attributes_with_type_var_range_args args;

    args.range_type = type;
    args.run_id = run_id;
    args.type_id = type_id;
    args.var_id = var_id;
    args.txn_id = txn_id;
    args.data_type = data_type;
    args.data = data;
    extreme_debug_log << "txn_id is " << to_string(txn_id ) << endl;  
   	entries.clear();
	switch(data_type) {
        case ATTR_DATA_TYPE_INT : {
        	uint64_t min_int, max_int;

    	    get_range_values(data, type, min_int, max_int);

    		return_value = md_catalog_all_timesteps_with_var_attributes_with_type_var_range_stub(args, 
    			min_int, max_int, entries, count);

            break;
        }
        case ATTR_DATA_TYPE_REAL : {
        	long double min_real, max_real;
    	    
    	    get_range_values(data, type, min_real, max_real);

    		return_value = md_catalog_all_timesteps_with_var_attributes_with_type_var_range_stub(args, 
    			min_real, max_real, entries, count);

            break;
        }
    }   
    // return_value = md_catalog_all_timesteps_with_var_attributes_with_type_var_range_stub(args, entries, count);
    add_timing_point(MD_CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_RANGE_DONE);  

    return return_value;
} //end of funct


int metadata_catalog_all_timesteps_with_var_attributes_with_type_var_range (uint64_t run_id             
                                        ,uint64_t type_id
                                        ,uint64_t var_id             
                                        ,uint64_t txn_id
                                        ,attr_data_type data_type
                                        ,const std::string &data                                        
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_timestep_entry> &entries
                                        )
{
    int return_value;

    return_value = metadata_catalog_all_timesteps_with_var_attributes_with_type_var_val (DATA_RANGE, run_id, type_id,
        var_id, txn_id, data_type, data, count, entries);

    return return_value;
} //end of funct

int metadata_catalog_all_timesteps_with_var_attributes_with_type_var_above_max (uint64_t run_id             
                                        ,uint64_t type_id
                                        ,uint64_t var_id             
                                        ,uint64_t txn_id
                                        ,attr_data_type data_type
                                        ,const std::string &data                                        
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_timestep_entry> &entries
                                        )
{
    int return_value;

    return_value = metadata_catalog_all_timesteps_with_var_attributes_with_type_var_val (DATA_MAX, run_id, type_id,
        var_id, txn_id, data_type, data, count, entries);

    return return_value;
} //end of funct

int metadata_catalog_all_timesteps_with_var_attributes_with_type_var_below_min (uint64_t run_id             
                                        ,uint64_t type_id
                                        ,uint64_t var_id             
                                        ,uint64_t txn_id
                                        ,attr_data_type data_type
                                        ,const std::string &data                                        
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_timestep_entry> &entries
                                        )
{
    int return_value;

    return_value = metadata_catalog_all_timesteps_with_var_attributes_with_type_var_val (DATA_MIN, run_id, type_id,
        var_id, txn_id, data_type, data, count, entries);

    return return_value;
} //end of funct

static int metadata_catalog_all_timesteps_with_var_attributes_with_type_var_dims_val (data_range_type type
                                        ,uint64_t run_id             
                                        ,uint64_t type_id
                                        ,uint64_t var_id             
                                        ,uint64_t txn_id
                                        ,uint32_t num_dims
                                        ,const std::vector<md_dim_bounds> &dims
                                        ,attr_data_type data_type
                                        ,const std::string &data
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_timestep_entry> &entries
                                        )
{
    add_timing_point(MD_CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_DIMS_RANGE_START);
    int return_value;
    std::string res;
    
    md_catalog_all_timesteps_with_var_attributes_with_type_var_dims_range_args args;

    args.range_type = type;
    args.run_id = run_id;
    args.type_id = type_id;
    args.var_id = var_id;
    args.txn_id = txn_id;
    args.num_dims = num_dims;
    args.dims = dims;
    args.data_type = data_type;
    args.data = data;
    extreme_debug_log << "txn_id is " << to_string(txn_id ) << endl;  
   	entries.clear();
	switch(data_type) {
        case ATTR_DATA_TYPE_INT : {
        	uint64_t min_int, max_int;

    	    get_range_values(data, type, min_int, max_int);

    		return_value = md_catalog_all_timesteps_with_var_attributes_with_type_var_dims_range_stub(args, 
    			min_int, max_int, entries, count);

            break;
        }
        case ATTR_DATA_TYPE_REAL : {
        	long double min_real, max_real;
    	    
    	    get_range_values(data, type, min_real, max_real);

    		return_value = md_catalog_all_timesteps_with_var_attributes_with_type_var_dims_range_stub(args, 
    			min_real, max_real, entries, count);

            break;
        }
    }   
    // return_value = md_catalog_all_timesteps_with_var_attributes_with_type_var_dims_range_stub(args, entries, count);
    add_timing_point(MD_CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_DIMS_RANGE_DONE);  

    return return_value;
} //end of funct

int metadata_catalog_all_timesteps_with_var_attributes_with_type_var_dims_range (uint64_t run_id             
                                        ,uint64_t type_id
                                        ,uint64_t var_id             
                                        ,uint64_t txn_id
                                        ,uint32_t num_dims
                                        ,const std::vector<md_dim_bounds> &dims
                                        ,attr_data_type data_type
                                        ,const std::string &data
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_timestep_entry> &entries
                                        )
{
    int return_value;

    return_value = metadata_catalog_all_timesteps_with_var_attributes_with_type_var_dims_val (DATA_RANGE,
        run_id, type_id, var_id, txn_id, num_dims, dims, data_type, data, count, entries);

    return return_value;
} //end of funct

int metadata_catalog_all_timesteps_with_var_attributes_with_type_var_dims_above_max (uint64_t run_id             
                                        ,uint64_t type_id
                                        ,uint64_t var_id             
                                        ,uint64_t txn_id
                                        ,uint32_t num_dims
                                        ,const std::vector<md_dim_bounds> &dims
                                        ,attr_data_type data_type
                                        ,const std::string &data
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_timestep_entry> &entries
                                        )
{
    int return_value;

    return_value = metadata_catalog_all_timesteps_with_var_attributes_with_type_var_dims_val (DATA_MAX,
        run_id, type_id, var_id, txn_id, num_dims, dims, data_type, data, count, entries);

    return return_value;
} //end of funct

int metadata_catalog_all_timesteps_with_var_attributes_with_type_var_dims_below_min (uint64_t run_id             
                                        ,uint64_t type_id
                                        ,uint64_t var_id             
                                        ,uint64_t txn_id
                                        ,uint32_t num_dims
                                        ,const std::vector<md_dim_bounds> &dims
                                        ,attr_data_type data_type
                                        ,const std::string &data
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_timestep_entry> &entries
                                        )
{
    int return_value;

    return_value = metadata_catalog_all_timesteps_with_var_attributes_with_type_var_dims_val (DATA_MIN,
        run_id, type_id, var_id, txn_id, num_dims, dims, data_type, data, count, entries);

    return return_value;
} //end of funct

static int metadata_catalog_all_timestep_attributes_with_type_val (data_range_type type
                                        ,uint64_t run_id             
                                        ,uint64_t timestep_id
                                        ,uint64_t type_id             
                                        ,uint64_t txn_id
                                        ,attr_data_type data_type
                                        ,const std::string &data
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_timestep_attribute_entry> &entries
                                        )
{
    add_timing_point(MD_CATALOG_ALL_TIMESTEP_ATTRIBUTES_WITH_TYPE_RANGE_START);
    int return_value;
    std::string res;
    

    md_catalog_all_timestep_attributes_with_type_range_args args;

    args.range_type = type;
    args.run_id = run_id;
    args.timestep_id = timestep_id;
    args.type_id = type_id;
    args.txn_id = txn_id;
    args.data_type = data_type;
    args.data = data;
    extreme_debug_log << "txn_id is " << to_string(txn_id ) << endl;  
   	entries.clear();
	switch(data_type) {
        case ATTR_DATA_TYPE_INT : {
        	uint64_t min_int, max_int;

    	    get_range_values(data, type, min_int, max_int);

    		return_value = md_catalog_all_timestep_attributes_with_type_range_stub(args, 
    			min_int, max_int, entries, count);

            break;
        }
        case ATTR_DATA_TYPE_REAL : {
        	long double min_real, max_real;
    	    
    	    get_range_values(data, type, min_real, max_real);

    		return_value = md_catalog_all_timestep_attributes_with_type_range_stub(args, 
    			min_real, max_real, entries, count);

            break;
        }
    }   
    // return_value = md_catalog_all_timestep_attributes_with_type_range_stub(args, entries, count);
    add_timing_point(MD_CATALOG_ALL_TIMESTEP_ATTRIBUTES_WITH_TYPE_RANGE_DONE);  

    return return_value;
} //end of funct


int metadata_catalog_all_timestep_attributes_with_type_range (uint64_t run_id             
                                        ,uint64_t type_id             
                                        ,uint64_t txn_id
                                        ,attr_data_type data_type
                                        ,const std::string &data
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_timestep_attribute_entry> &entries
                                        )
{
    int return_value;

    return_value = metadata_catalog_all_timestep_attributes_with_type_val (DATA_RANGE, run_id,
        ALL_TIMESTEPS, type_id, txn_id, data_type, data, count, entries);

    return return_value;
}

int metadata_catalog_all_timestep_attributes_with_type_above_max (uint64_t run_id             
                                        ,uint64_t type_id             
                                        ,uint64_t txn_id
                                        ,attr_data_type data_type
                                        ,const std::string &data
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_timestep_attribute_entry> &entries
                                        )
{
    int return_value;

    return_value = metadata_catalog_all_timestep_attributes_with_type_val (DATA_MAX, run_id,
        ALL_TIMESTEPS, type_id, txn_id, data_type, data, count, entries);
    
    return return_value;
}

int metadata_catalog_all_timestep_attributes_with_type_below_min (uint64_t run_id             
                                        ,uint64_t type_id             
                                        ,uint64_t txn_id
                                        ,attr_data_type data_type
                                        ,const std::string &data
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_timestep_attribute_entry> &entries
                                        )
{
    int return_value;

    return_value = metadata_catalog_all_timestep_attributes_with_type_val (DATA_MIN, run_id,
        ALL_TIMESTEPS, type_id, txn_id, data_type, data, count, entries);

    return return_value;
}



int metadata_catalog_all_timestep_attributes_with_type_range_in_timestep (uint64_t run_id             
                                        ,uint64_t timestep_id
                                        ,uint64_t type_id             
                                        ,uint64_t txn_id
                                        ,attr_data_type data_type
                                        ,const std::string &data
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_timestep_attribute_entry> &entries
                                        )
{
    int return_value;

    return_value = metadata_catalog_all_timestep_attributes_with_type_val (DATA_RANGE, run_id,
        timestep_id, type_id, txn_id, data_type, data, count, entries);

    return return_value;
}

int metadata_catalog_all_timestep_attributes_with_type_above_max_in_timestep (uint64_t run_id             
                                        ,uint64_t timestep_id
                                        ,uint64_t type_id             
                                        ,uint64_t txn_id
                                        ,attr_data_type data_type
                                        ,const std::string &data
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_timestep_attribute_entry> &entries
                                        )
{
    int return_value;

    return_value = metadata_catalog_all_timestep_attributes_with_type_val (DATA_MAX, run_id,
        timestep_id, type_id, txn_id, data_type, data, count, entries);
    
    return return_value;
}

int metadata_catalog_all_timestep_attributes_with_type_below_min_in_timestep (uint64_t run_id             
                                        ,uint64_t timestep_id
                                        ,uint64_t type_id             
                                        ,uint64_t txn_id
                                        ,attr_data_type data_type
                                        ,const std::string &data
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_timestep_attribute_entry> &entries
                                        )
{
    int return_value;

    return_value = metadata_catalog_all_timestep_attributes_with_type_val (DATA_MIN, run_id,
        timestep_id, type_id, txn_id, data_type, data, count, entries);

    return return_value;
}

static int metadata_catalog_all_var_attributes_with_type_var_val (data_range_type type
                                        ,uint64_t timestep_id
                                        ,uint64_t type_id
                                        ,uint64_t var_id
                                        ,uint64_t txn_id
                                        ,attr_data_type data_type
                                        ,const std::string &data
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_var_attribute_entry> &entries
                                        )
{
    add_timing_point(MD_CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_RANGE_START);

    int return_value;
    std::string res;

    md_catalog_all_var_attributes_with_type_var_range_args args;

    args.range_type = type;
    args.timestep_id = timestep_id;
    args.type_id = type_id;
    args.var_id = var_id;
    args.txn_id = txn_id;
    args.data_type = data_type;
    args.data = data;
	entries.clear();
	switch(data_type) {
        case ATTR_DATA_TYPE_INT : {
        	uint64_t min_int, max_int;

    	    get_range_values(data, type, min_int, max_int);

    		return_value = md_catalog_all_var_attributes_with_type_var_range_stub(args, 
    			min_int, max_int, entries, count);

            break;
        }
        case ATTR_DATA_TYPE_REAL : {
        	long double min_real, max_real;
    	    
    	    get_range_values(data, type, min_real, max_real);

    		return_value = md_catalog_all_var_attributes_with_type_var_range_stub(args, 
    			min_real, max_real, entries, count);

            break;
        }
    }   
    // return_value = md_catalog_all_var_attributes_with_type_var_range_stub(args, entries, count);
    extreme_debug_log << "just launched get attr _by dims \n";
    add_timing_point(MD_CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_RANGE_DONE);   
    extreme_debug_log << "number of entries is " << entries.size() << "\n";


    return return_value;
} //end of funct


int metadata_catalog_all_var_attributes_with_type_var_range (uint64_t timestep_id
                                        ,uint64_t type_id
                                        ,uint64_t var_id
                                        ,uint64_t txn_id
                                        ,attr_data_type data_type
                                        ,const std::string &data
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_var_attribute_entry> &entries
                                        )
{
    int return_value;

    return_value = metadata_catalog_all_var_attributes_with_type_var_val (DATA_RANGE, timestep_id,   
        type_id, var_id, txn_id, data_type, data, count, entries);

    return return_value;
} //end of funct

int metadata_catalog_all_var_attributes_with_type_var_above_max (uint64_t timestep_id
                                        ,uint64_t type_id
                                        ,uint64_t var_id
                                        ,uint64_t txn_id
                                        ,attr_data_type data_type
                                        ,const std::string &data
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_var_attribute_entry> &entries
                                        )
{
    int return_value;

    return_value = metadata_catalog_all_var_attributes_with_type_var_val (DATA_MAX, timestep_id,   
        type_id, var_id, txn_id, data_type, data, count, entries);

    return return_value;
} //end of funct

int metadata_catalog_all_var_attributes_with_type_var_below_min (uint64_t timestep_id
                                        ,uint64_t type_id
                                        ,uint64_t var_id
                                        ,uint64_t txn_id
                                        ,attr_data_type data_type
                                        ,const std::string &data
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_var_attribute_entry> &entries
                                        )
{
    int return_value;

    return_value = metadata_catalog_all_var_attributes_with_type_var_val (DATA_MIN, timestep_id,   
        type_id, var_id, txn_id, data_type, data, count, entries);

    return return_value;
} //end of funct

static int metadata_catalog_all_var_attributes_with_type_var_dims_val (data_range_type type
                                        ,uint64_t timestep_id
                                        ,uint64_t type_id
                                        ,uint64_t var_id
                                        ,uint64_t txn_id
                                        ,uint32_t num_dims
                                        ,const std::vector<md_dim_bounds> &dims
                                        ,attr_data_type data_type
                                        ,const std::string &data
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_var_attribute_entry> &entries
                                        )
{
    add_timing_point(MD_CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_DIMS_RANGE_START);

    int return_value;
    std::string res;

    md_catalog_all_var_attributes_with_type_var_dims_range_args args;

    args.range_type = type;
    args.timestep_id = timestep_id;
    args.type_id = type_id;
    args.var_id = var_id;
    args.txn_id = txn_id;
    args.num_dims = num_dims;
    args.dims = dims;
    args.data_type = data_type;
    args.data = data;
	entries.clear();
	switch(data_type) {
        case ATTR_DATA_TYPE_INT : {
        	uint64_t min_int, max_int;

    	    get_range_values(data, type, min_int, max_int);

    		return_value = md_catalog_all_var_attributes_with_type_var_dims_range_stub(args, 
    			min_int, max_int, entries, count);

            break;
        }
        case ATTR_DATA_TYPE_REAL : {
        	long double min_real, max_real;
    	    
    	    get_range_values(data, type, min_real, max_real);

    		return_value = md_catalog_all_var_attributes_with_type_var_dims_range_stub(args, 
    			min_real, max_real, entries, count);

            break;
        }
    }   
    // return_value = md_catalog_all_var_attributes_with_type_var_dims_range_stub(args, entries, count);
    extreme_debug_log << "just launched get attr _by dims \n";
    add_timing_point(MD_CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_DIMS_RANGE_DONE);   
    extreme_debug_log << "number of entries is " << entries.size() << "\n";


    return return_value;
} //end of funct

int metadata_catalog_all_var_attributes_with_type_var_dims_range (uint64_t timestep_id
                                        ,uint64_t type_id
                                        ,uint64_t var_id
                                        ,uint64_t txn_id
                                        ,uint32_t num_dims
                                        ,const std::vector<md_dim_bounds> &dims
                                        ,attr_data_type data_type
                                        ,const std::string &data
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_var_attribute_entry> &entries
                                        )
{
    int return_value;

    return_value = metadata_catalog_all_var_attributes_with_type_var_dims_val (DATA_RANGE, timestep_id, type_id,
        var_id, txn_id, num_dims, dims, data_type, data, count, entries);

    return return_value;

} //end of funct



int metadata_catalog_all_var_attributes_with_type_var_dims_above_max (uint64_t timestep_id
                                        ,uint64_t type_id
                                        ,uint64_t var_id
                                        ,uint64_t txn_id
                                        ,uint32_t num_dims
                                        ,const std::vector<md_dim_bounds> &dims
                                        ,attr_data_type data_type
                                        ,const std::string &data
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_var_attribute_entry> &entries
                                        )
{
    int return_value;

    return_value = metadata_catalog_all_var_attributes_with_type_var_dims_val (DATA_MAX, timestep_id, type_id,
        var_id, txn_id, num_dims, dims, data_type, data, count, entries);
    return return_value;
} //end of funct

int metadata_catalog_all_var_attributes_with_type_var_dims_below_min (uint64_t timestep_id
                                        ,uint64_t type_id
                                        ,uint64_t var_id
                                        ,uint64_t txn_id
                                        ,uint32_t num_dims
                                        ,const std::vector<md_dim_bounds> &dims
                                        ,attr_data_type data_type
                                        ,const std::string &data
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_var_attribute_entry> &entries
                                        )
{
    int return_value;

    return_value = metadata_catalog_all_var_attributes_with_type_var_dims_val (DATA_MIN, timestep_id, type_id,
        var_id, txn_id, num_dims, dims, data_type, data, count, entries);

    return return_value;
} //end of funct


int metadata_catalog_all_types_with_var_attributes (uint64_t run_id
                      ,uint64_t txn_id
                      ,uint32_t &count
                      ,std::vector<md_catalog_type_entry> &entries
                     )
{
	int return_value = metadata_catalog_all_types_with_var_attributes_in_timestep (
							run_id, ALL_TIMESTEPS, txn_id, count, entries
							);

	return return_value;
}



int metadata_catalog_all_types_with_var_attributes_in_timestep (uint64_t run_id
                      ,uint64_t timestep_id
                      ,uint64_t txn_id
                      ,uint32_t &count
                      ,std::vector<md_catalog_type_entry> &entries
                     )
{
    add_timing_point(MD_CATALOG_ALL_TYPES_WITH_VAR_ATTRIBUTES_IN_TIMESTEP_START);
    int return_value;
    std::string res;

    md_catalog_all_types_with_var_attributes_in_timestep_args args;

    args.run_id = run_id;
    args.timestep_id = timestep_id;
    args.txn_id = txn_id;
	entries.clear();
    return_value = md_catalog_all_types_with_var_attributes_in_timestep_stub(args, entries, count);

    add_timing_point(MD_CATALOG_ALL_TYPES_WITH_VAR_ATTRIBUTES_IN_TIMESTEP_DONE);   

    return return_value;
} //end of funct

int metadata_catalog_all_var_attributes_with_var_substr (uint64_t run_id             
                                        ,uint64_t timestep_id
                                        ,const std::string &var_name_substr
                                        ,uint64_t txn_id
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_var_attribute_entry> &entries
                                        )
{
    add_timing_point(MD_CATALOG_ALL_VAR_ATTRIBUTES_WITH_VAR_SUBSTR_START);

    int return_value;
    md_catalog_all_var_attributes_with_var_substr_args args;

    args.txn_id = txn_id;
    args.run_id = run_id;
    args.timestep_id = timestep_id;
    args.var_name_substr.reserve(2 + var_name_substr.size());
    args.var_name_substr = "%";
    //args.var_name_substr += boost::algorithm::to_lower_copy(var_name_substr);
    args.var_name_substr += var_name_substr;
    args.var_name_substr += "%";
	entries.clear();
    return_value = md_catalog_all_var_attributes_with_var_substr_stub(args, entries, count);
    extreme_debug_log << "just launched get attr _by dims \n";

    add_timing_point(MD_CATALOG_ALL_VAR_ATTRIBUTES_WITH_VAR_SUBSTR_DONE);   
    extreme_debug_log << "number of entries is " << entries.size() << "\n";


    return return_value;
} //end of funct

int metadata_catalog_all_timesteps_with_var_attributes_with_type_var_substr_dims (uint64_t run_id             
                                        ,uint64_t type_id
                                        ,const std::string &var_name_substr             
                                        ,uint64_t txn_id
                                        ,uint32_t num_dims
                                        ,const std::vector<md_dim_bounds> &dims
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_timestep_entry> &entries
                                        )
{
    add_timing_point(MD_CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_DIMS_START);
    int return_value;
    std::string res;
    
    md_catalog_all_timesteps_with_var_attributes_with_type_var_substr_dims_args args;

    args.run_id = run_id;
    args.type_id = type_id;
    args.var_name_substr.reserve(2 + var_name_substr.size());
    args.var_name_substr = "%";
    //args.var_name_substr += boost::algorithm::to_lower_copy(var_name_substr);
    args.var_name_substr += var_name_substr;
    args.var_name_substr += "%";
    args.txn_id = txn_id;
    args.num_dims = num_dims;
    args.dims = dims;
    extreme_debug_log << "txn_id is " << to_string(txn_id ) << endl;  
   	entries.clear();
    return_value = md_catalog_all_timesteps_with_var_attributes_with_type_var_substr_dims_stub(args, entries, count);
    add_timing_point(MD_CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_DIMS_DONE);  

    return return_value;
} //end of funct


int metadata_catalog_all_timesteps_with_var_attributes_with_type_var_substr (uint64_t run_id             
                                        ,uint64_t type_id
                                        ,const std::string &var_name_substr             
                                        ,uint64_t txn_id
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_timestep_entry> &entries
                                        )
{
    add_timing_point(MD_CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_START);
    int return_value;
    std::string res;
    

    md_catalog_all_timesteps_with_var_attributes_with_type_var_substr_args args;

    args.run_id = run_id;
    args.type_id = type_id;
    args.var_name_substr.reserve(2 + var_name_substr.size());
    args.var_name_substr = "%";
    //args.var_name_substr += boost::algorithm::to_lower_copy(var_name_substr);
    args.var_name_substr += var_name_substr;
    args.var_name_substr += "%";
    args.txn_id = txn_id;
    extreme_debug_log << "txn_id is " << to_string(txn_id ) << endl;  
   	entries.clear();
    return_value = md_catalog_all_timesteps_with_var_attributes_with_type_var_substr_stub(args, entries, count);
    add_timing_point(MD_CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_DONE);  

    return return_value;
} //end of funct

int metadata_catalog_all_timesteps_with_var_substr (uint64_t run_id             
                                        ,const std::string &var_name_substr             
                                        ,uint64_t txn_id
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_timestep_entry> &entries
                                        )
{
    add_timing_point(MD_CATALOG_ALL_TIMESTEPS_WITH_VAR_SUBSTR_START);
    int return_value;
    std::string res;
    

    md_catalog_all_timesteps_with_var_substr_args args;

    args.run_id = run_id;
    args.var_name_substr.reserve(2 + var_name_substr.size());
    args.var_name_substr = "%";
    //args.var_name_substr += boost::algorithm::to_lower_copy(var_name_substr);
    args.var_name_substr += var_name_substr;
    args.var_name_substr += "%";
    args.txn_id = txn_id;
    extreme_debug_log << "txn_id is " << to_string(txn_id ) << endl;  
   	entries.clear();
    return_value = md_catalog_all_timesteps_with_var_substr_stub(args, entries, count);
    add_timing_point(MD_CATALOG_ALL_TIMESTEPS_WITH_VAR_SUBSTR_DONE);  

    return return_value;
} //end of funct

int metadata_catalog_all_types_with_var_attributes_with_var_substr_dims (uint64_t run_id
                      ,const std::string &var_name_substr
                      ,uint64_t txn_id
                      ,uint32_t num_dims
                      ,const std::vector<md_dim_bounds> &dims
                      ,uint32_t &count
                      ,std::vector<md_catalog_type_entry> &entries
                     )
{
	int return_value = metadata_catalog_all_types_with_var_attributes_with_var_substr_dims_in_timestep( 
	                      run_id, ALL_TIMESTEPS, var_name_substr, txn_id, num_dims, dims,
	                      count, entries
	                     );
	return return_value;
}

int metadata_catalog_all_types_with_var_attributes_with_var_substr_dims_in_timestep (uint64_t run_id
                      ,uint64_t timestep_id
                      ,const std::string &var_name_substr
                      ,uint64_t txn_id
                      ,uint32_t num_dims
                      ,const std::vector<md_dim_bounds> &dims
                      ,uint32_t &count
                      ,std::vector<md_catalog_type_entry> &entries
                     )
{
    add_timing_point(MD_CATALOG_ALL_TYPES_WITH_VAR_ATTRIBUTES_WITH_VAR_SUBSTR_DIMS_IN_TIMESTEP_START);
    int return_value;
    std::string res;

    md_catalog_all_types_with_var_attributes_with_var_substr_dims_in_timestep_args args;

    args.run_id = run_id;
    args.timestep_id = timestep_id;
    args.var_name_substr.reserve(2 + var_name_substr.size());
    args.var_name_substr = "%";
    //args.var_name_substr += boost::algorithm::to_lower_copy(var_name_substr);
    args.var_name_substr += var_name_substr;
    args.var_name_substr += "%";
    args.txn_id = txn_id;
    args.num_dims = num_dims;
    args.dims = dims;
	entries.clear();
    return_value = md_catalog_all_types_with_var_attributes_with_var_substr_dims_in_timestep_stub(args, entries, count);

    add_timing_point(MD_CATALOG_ALL_TYPES_WITH_VAR_ATTRIBUTES_WITH_VAR_SUBSTR_DIMS_IN_TIMESTEP_DONE);   

    return return_value;
} //end of funct

int metadata_catalog_all_types_with_var_attributes_with_var_substr (uint64_t run_id
                      ,const std::string &var_name_substr
                      ,uint64_t txn_id
                      ,uint32_t &count
                      ,std::vector<md_catalog_type_entry> &entries
                     )
{
    int return_value = metadata_catalog_all_types_with_var_attributes_with_var_substr_in_timestep (
                      run_id, ALL_TIMESTEPS, var_name_substr, txn_id, count, entries);

    return return_value;
} //end of funct

int metadata_catalog_all_types_with_var_attributes_with_var_substr_in_timestep (uint64_t run_id
                      ,uint64_t timestep_id
                      ,const std::string &var_name_substr
                      ,uint64_t txn_id
                      ,uint32_t &count
                      ,std::vector<md_catalog_type_entry> &entries
                     )
{
    add_timing_point(MD_CATALOG_ALL_TYPES_WITH_VAR_ATTRIBUTES_WITH_VAR_SUBSTR_IN_TIMESTEP_START);
    int return_value;
    std::string res;

    md_catalog_all_types_with_var_attributes_with_var_substr_in_timestep_args args;

    args.run_id = run_id;
    args.timestep_id = timestep_id;
    args.var_name_substr.reserve(2 + var_name_substr.size());
    args.var_name_substr = "%";
    //args.var_name_substr += boost::algorithm::to_lower_copy(var_name_substr);
    args.var_name_substr += var_name_substr;
    args.var_name_substr += "%";
    args.txn_id = txn_id;
	entries.clear();
    return_value = md_catalog_all_types_with_var_attributes_with_var_substr_in_timestep_stub(args, entries, count);

    add_timing_point(MD_CATALOG_ALL_TYPES_WITH_VAR_ATTRIBUTES_WITH_VAR_SUBSTR_IN_TIMESTEP_DONE);   

    return return_value;
} //end of funct


int metadata_catalog_all_var_attributes_with_type_var_substr (uint64_t run_id             
                                        ,uint64_t timestep_id
                                        ,uint64_t type_id
                                        ,const std::string &var_name_substr                                                                                
                                        ,uint64_t txn_id
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_var_attribute_entry> &entries
                                        )
{
    add_timing_point(MD_CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_START);

    int return_value;
    md_catalog_all_var_attributes_with_type_var_substr_args args;

    args.txn_id = txn_id;
    args.run_id = run_id;
    args.timestep_id = timestep_id;
    args.type_id = type_id;
    args.var_name_substr.reserve(2 + var_name_substr.size());
    args.var_name_substr = "%";
    //args.var_name_substr += boost::algorithm::to_lower_copy(var_name_substr);
    args.var_name_substr += var_name_substr;
    args.var_name_substr += "%";
	entries.clear();
    return_value = md_catalog_all_var_attributes_with_type_var_substr_stub(args, entries, count);
    extreme_debug_log << "just launched get attr _by dims \n";

    add_timing_point(MD_CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_DONE);   
    extreme_debug_log << "number of entries is " << entries.size() << "\n";


    return return_value;
} //end of funct

int metadata_catalog_all_var_attributes_with_type_var_substr_dims (uint64_t run_id             
                                        ,uint64_t timestep_id
                                        ,uint64_t type_id
                                        ,const std::string &var_name_substr                     
                                        ,uint64_t txn_id
                                        ,uint32_t num_dims
                                        ,const std::vector<md_dim_bounds> &dims
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_var_attribute_entry> &entries
                                        )
{
    add_timing_point(MD_CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_DIMS_START);

    int return_value;
    md_catalog_all_var_attributes_with_type_var_substr_dims_args args;

    args.txn_id = txn_id;
    args.run_id = run_id;
    args.timestep_id = timestep_id;
    args.type_id = type_id;
    args.var_name_substr.reserve(2 + var_name_substr.size());
    args.var_name_substr = "%";
    //args.var_name_substr += boost::algorithm::to_lower_copy(var_name_substr);
    args.var_name_substr += var_name_substr;
    args.var_name_substr += "%";
    args.num_dims = num_dims;
    args.dims = dims;
	entries.clear();
    return_value = md_catalog_all_var_attributes_with_type_var_substr_dims_stub(args, entries, count);
    extreme_debug_log << "just launched get attr _by dims \n";

    add_timing_point(MD_CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_DIMS_DONE);   
    extreme_debug_log << "number of entries is " << entries.size() << "\n";


    return return_value;
} //end of funct

int metadata_catalog_all_var_attributes_with_var_substr_dims (uint64_t run_id             
                                        ,uint64_t timestep_id
                                        ,const std::string &var_name_substr                     
                                        ,uint64_t txn_id
                                        ,uint32_t num_dims
                                        ,const std::vector<md_dim_bounds> &dims
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_var_attribute_entry> &entries
                                        )
{
    add_timing_point(MD_CATALOG_ALL_VAR_ATTRIBUTES_WITH_VAR_SUBSTR_DIMS_START);

    int return_value;
    md_catalog_all_var_attributes_with_var_substr_dims_args args;

    args.txn_id = txn_id;
    args.run_id = run_id;
    args.timestep_id = timestep_id;
    args.var_name_substr.reserve(2 + var_name_substr.size());
    args.var_name_substr = "%";
    //args.var_name_substr += boost::algorithm::to_lower_copy(var_name_substr);
    args.var_name_substr += var_name_substr;
    args.var_name_substr += "%";
    args.num_dims = num_dims;
    args.dims = dims;
	entries.clear();
    return_value = md_catalog_all_var_attributes_with_var_substr_dims_stub(args, entries, count);
    extreme_debug_log << "just launched get attr _by dims \n";

    add_timing_point(MD_CATALOG_ALL_VAR_ATTRIBUTES_WITH_VAR_SUBSTR_DIMS_DONE);   
    extreme_debug_log << "number of entries is " << entries.size() << "\n";


    return return_value;
} //end of funct

int metadata_delete_all_vars_with_substr (uint64_t run_id
                        ,uint64_t timestep_id
                        ,const std::string &var_name_substr
                        )
{
    add_timing_point(MD_DELETE_ALL_VARS_WITH_SUBSTR_START);

    int return_value;
    std::string res;
    md_delete_all_vars_with_substr_args args;

    args.run_id = run_id;
    args.timestep_id = timestep_id;
    args.var_name_substr.reserve(2 + var_name_substr.size());
    args.var_name_substr = "%";
    //args.var_name_substr += boost::algorithm::to_lower_copy(var_name_substr);
	args.var_name_substr += var_name_substr;
	args.var_name_substr += "%";

    return_value = md_delete_all_vars_with_substr_stub(args);
    add_timing_point(MD_DELETE_ALL_VARS_WITH_SUBSTR_DONE);

    return return_value;
} //end of funct



static int metadata_catalog_all_timesteps_with_var_attributes_with_type_var_substr_val (data_range_type type
                                        ,uint64_t run_id             
                                        ,uint64_t type_id
                                        ,const std::string &var_name_substr             
                                        ,uint64_t txn_id
                                        ,attr_data_type data_type
                                        ,const std::string &data                                        
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_timestep_entry> &entries
                                        )
{
    add_timing_point(MD_CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_RANGE_START);
    int return_value;
    std::string res;
    

    md_catalog_all_timesteps_with_var_attributes_with_type_var_substr_range_args args;

    args.range_type = type;
    args.run_id = run_id;
    args.type_id = type_id;
    args.var_name_substr.reserve(2 + var_name_substr.size());
    args.var_name_substr = "%";
    //args.var_name_substr += boost::algorithm::to_lower_copy(var_name_substr);
    args.var_name_substr += var_name_substr;
    args.var_name_substr += "%";
    args.txn_id = txn_id;
    args.data_type = data_type;
    args.data = data;
    // extreme_debug_log << "range_type: " << type << endl;
    // extreme_debug_log << "run_id: " << run_id << endl;
    // extreme_debug_log << "type_id: " << type_id << endl;
    // extreme_debug_log << "var_name_substr: " << var_name_substr << endl;
    // extreme_debug_log << "txn_id: " << txn_id << endl;
    // extreme_debug_log << "data_type: " << data_type << endl;
    // extreme_debug_log << "data: " << data << endl;

    extreme_debug_log << "txn_id is " << to_string(txn_id ) << endl;  
   	entries.clear();
	switch(data_type) {
        case ATTR_DATA_TYPE_INT : {
        	uint64_t min_int, max_int;

    	    get_range_values(data, type, min_int, max_int);

    		return_value = md_catalog_all_timesteps_with_var_attributes_with_type_var_substr_range_stub(args, 
    			min_int, max_int, entries, count);

            break;
        }
        case ATTR_DATA_TYPE_REAL : {
        	long double min_real, max_real;
    	    
    	    get_range_values(data, type, min_real, max_real);

    		return_value = md_catalog_all_timesteps_with_var_attributes_with_type_var_substr_range_stub(args, 
    			min_real, max_real, entries, count);

            break;
        }
    }   
    // return_value = md_catalog_all_timesteps_with_var_attributes_with_type_var_substr_range_stub(args, entries, count);
    add_timing_point(MD_CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_RANGE_DONE);  

    return return_value;
} //end of funct

int metadata_catalog_all_timesteps_with_var_attributes_with_type_var_substr_range (uint64_t run_id             
                                        ,uint64_t type_id
                                        ,const std::string &var_name_substr             
                                        ,uint64_t txn_id
                                        ,attr_data_type data_type
                                        ,const std::string &data                                        
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_timestep_entry> &entries
                                        )
{
    int return_value;

    return_value = metadata_catalog_all_timesteps_with_var_attributes_with_type_var_substr_val (DATA_RANGE, run_id, type_id,
        var_name_substr, txn_id, data_type, data, count, entries);

    return return_value;
} //end of funct

int metadata_catalog_all_timesteps_with_var_attributes_with_type_var_substr_above_max (uint64_t run_id             
                                        ,uint64_t type_id
                                        ,const std::string &var_name_substr             
                                        ,uint64_t txn_id
                                        ,attr_data_type data_type
                                        ,const std::string &data                                        
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_timestep_entry> &entries
                                        )
{
    int return_value;

    return_value = metadata_catalog_all_timesteps_with_var_attributes_with_type_var_substr_val (DATA_MAX, run_id, type_id,
        var_name_substr, txn_id, data_type, data, count, entries);

    return return_value;
} //end of funct

int metadata_catalog_all_timesteps_with_var_attributes_with_type_var_substr_below_min (uint64_t run_id             
                                        ,uint64_t type_id
                                        ,const std::string &var_name_substr             
                                        ,uint64_t txn_id
                                        ,attr_data_type data_type
                                        ,const std::string &data                                        
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_timestep_entry> &entries
                                        )
{
    int return_value;

    return_value = metadata_catalog_all_timesteps_with_var_attributes_with_type_var_substr_val (DATA_MIN, run_id, type_id,
        var_name_substr, txn_id, data_type, data, count, entries);

    return return_value;
} //end of funct


static int metadata_catalog_all_timesteps_with_var_attributes_with_type_var_substr_dims_val (data_range_type type
                                        ,uint64_t run_id             
                                        ,uint64_t type_id
                                        ,const std::string &var_name_substr             
                                        ,uint64_t txn_id
                                        ,uint32_t num_dims
                                        ,const std::vector<md_dim_bounds> &dims
                                        ,attr_data_type data_type
                                        ,const std::string &data
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_timestep_entry> &entries
                                        )
{
    add_timing_point(MD_CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_DIMS_RANGE_START);
    int return_value;
    std::string res;
    
    md_catalog_all_timesteps_with_var_attributes_with_type_var_substr_dims_range_args args;

    args.range_type = type;
    args.run_id = run_id;
    args.type_id = type_id;
    args.var_name_substr.reserve(2 + var_name_substr.size());
    args.var_name_substr = "%";
    //args.var_name_substr += boost::algorithm::to_lower_copy(var_name_substr);
    args.var_name_substr += var_name_substr;
    args.var_name_substr += "%";
    args.txn_id = txn_id;
    args.num_dims = num_dims;
    args.dims = dims;
    args.data_type = data_type;
    args.data = data;
    extreme_debug_log << "txn_id is " << to_string(txn_id ) << endl;  
   	entries.clear();
	switch(data_type) {
        case ATTR_DATA_TYPE_INT : {
        	uint64_t min_int, max_int;

    	    get_range_values(data, type, min_int, max_int);

    		return_value = md_catalog_all_timesteps_with_var_attributes_with_type_var_substr_dims_range_stub(args, 
    			min_int, max_int, entries, count);

            break;
        }
        case ATTR_DATA_TYPE_REAL : {
        	long double min_real, max_real;
    	    
    	    get_range_values(data, type, min_real, max_real);

    		return_value = md_catalog_all_timesteps_with_var_attributes_with_type_var_substr_dims_range_stub(args, 
    			min_real, max_real, entries, count);

            break;
        }
    }   
    // return_value = md_catalog_all_timesteps_with_var_attributes_with_type_var_substr_dims_range_stub(args, entries, count);
    add_timing_point(MD_CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_DIMS_RANGE_DONE);  

    return return_value;
} //end of funct

int metadata_catalog_all_timesteps_with_var_attributes_with_type_var_substr_dims_range (uint64_t run_id             
                                        ,uint64_t type_id
                                        ,const std::string &var_name_substr             
                                        ,uint64_t txn_id
                                        ,uint32_t num_dims
                                        ,const std::vector<md_dim_bounds> &dims
                                        ,attr_data_type data_type
                                        ,const std::string &data
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_timestep_entry> &entries
                                        )
{
    int return_value;

    return_value = metadata_catalog_all_timesteps_with_var_attributes_with_type_var_substr_dims_val (DATA_RANGE,
        run_id, type_id, var_name_substr, txn_id, num_dims, dims, data_type, data, count, entries);

    return return_value;
} //end of funct

int metadata_catalog_all_timesteps_with_var_attributes_with_type_var_substr_dims_above_max (uint64_t run_id             
                                        ,uint64_t type_id
                                        ,const std::string &var_name_substr             
                                        ,uint64_t txn_id
                                        ,uint32_t num_dims
                                        ,const std::vector<md_dim_bounds> &dims
                                        ,attr_data_type data_type
                                        ,const std::string &data
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_timestep_entry> &entries
                                        )
{
    int return_value;

    return_value = metadata_catalog_all_timesteps_with_var_attributes_with_type_var_substr_dims_val (DATA_MAX,
        run_id, type_id, var_name_substr, txn_id, num_dims, dims, data_type, data, count, entries);

    return return_value;
} //end of funct

int metadata_catalog_all_timesteps_with_var_attributes_with_type_var_substr_dims_below_min (uint64_t run_id             
                                        ,uint64_t type_id
                                        ,const std::string &var_name_substr             
                                        ,uint64_t txn_id
                                        ,uint32_t num_dims
                                        ,const std::vector<md_dim_bounds> &dims
                                        ,attr_data_type data_type
                                        ,const std::string &data
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_timestep_entry> &entries
                                        )
{
    int return_value;

    return_value = metadata_catalog_all_timesteps_with_var_attributes_with_type_var_substr_dims_val (DATA_MIN,
        run_id, type_id, var_name_substr, txn_id, num_dims, dims, data_type, data, count, entries);

    return return_value;
} //end of funct



static int metadata_catalog_all_var_attributes_with_type_var_substr_val (data_range_type type
                                        ,uint64_t timestep_id
                                        ,uint64_t type_id
                                        ,const std::string &var_name_substr 
                                        ,uint64_t txn_id
                                        ,attr_data_type data_type
                                        ,const std::string &data
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_var_attribute_entry> &entries
                                        )
{
    add_timing_point(MD_CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_RANGE_START);

    int return_value;
    std::string res;

    md_catalog_all_var_attributes_with_type_var_substr_range_args args;

    args.range_type = type;
    args.timestep_id = timestep_id;
    args.type_id = type_id;
    args.var_name_substr.reserve(2 + var_name_substr.size());
    args.var_name_substr = "%";
    //args.var_name_substr += boost::algorithm::to_lower_copy(var_name_substr);
    args.var_name_substr += var_name_substr;
    args.var_name_substr += "%";
    args.txn_id = txn_id;
    args.data_type = data_type;
    args.data = data;
	entries.clear();
	switch(data_type) {
        case ATTR_DATA_TYPE_INT : {
        	uint64_t min_int, max_int;

    	    get_range_values(data, type, min_int, max_int);

    		return_value = md_catalog_all_var_attributes_with_type_var_substr_range_stub(args, 
    			min_int, max_int, entries, count);

            break;
        }
        case ATTR_DATA_TYPE_REAL : {
        	long double min_real, max_real;
    	    
    	    get_range_values(data, type, min_real, max_real);

    		return_value = md_catalog_all_var_attributes_with_type_var_substr_range_stub(args, 
    			min_real, max_real, entries, count);

            break;
        }
    }   
    // return_value = md_catalog_all_var_attributes_with_type_var_substr_range_stub(args, entries, count);
    extreme_debug_log << "just launched get attr _by dims \n";
    add_timing_point(MD_CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_RANGE_DONE);   
    extreme_debug_log << "number of entries is " << entries.size() << "\n";


    return return_value;
} //end of funct

int metadata_catalog_all_var_attributes_with_type_var_substr_range (uint64_t timestep_id
                                        ,uint64_t type_id
                                        ,const std::string &var_name_substr 
                                        ,uint64_t txn_id
                                        ,attr_data_type data_type
                                        ,const std::string &data
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_var_attribute_entry> &entries
                                        )
{
    int return_value;

    return_value = metadata_catalog_all_var_attributes_with_type_var_substr_val (DATA_RANGE, timestep_id,   
        type_id, var_name_substr, txn_id, data_type, data, count, entries);

    return return_value;
} //end of funct

int metadata_catalog_all_var_attributes_with_type_var_substr_above_max (uint64_t timestep_id
                                        ,uint64_t type_id
                                        ,const std::string &var_name_substr 
                                        ,uint64_t txn_id
                                        ,attr_data_type data_type
                                        ,const std::string &data
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_var_attribute_entry> &entries
                                        )
{
    int return_value;

    return_value = metadata_catalog_all_var_attributes_with_type_var_substr_val (DATA_MAX, timestep_id,   
        type_id, var_name_substr, txn_id, data_type, data, count, entries);

    return return_value;
} //end of funct

int metadata_catalog_all_var_attributes_with_type_var_substr_below_min (uint64_t timestep_id
                                        ,uint64_t type_id
                                        ,const std::string &var_name_substr 
                                        ,uint64_t txn_id
                                        ,attr_data_type data_type
                                        ,const std::string &data
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_var_attribute_entry> &entries
                                        )
{
    int return_value;

    return_value = metadata_catalog_all_var_attributes_with_type_var_substr_val (DATA_MIN, timestep_id,   
        type_id, var_name_substr, txn_id, data_type, data, count, entries);

    return return_value;
} //end of funct

static int metadata_catalog_all_var_attributes_with_type_var_substr_dims_val (data_range_type type
                                        ,uint64_t timestep_id
                                        ,uint64_t type_id
                                        ,const std::string &var_name_substr 
                                        ,uint64_t txn_id
                                        ,uint32_t num_dims
                                        ,const std::vector<md_dim_bounds> &dims
                                        ,attr_data_type data_type
                                        ,const std::string &data
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_var_attribute_entry> &entries
                                        )
{
    add_timing_point(MD_CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_DIMS_RANGE_START);

    int return_value;
    std::string res;

    md_catalog_all_var_attributes_with_type_var_substr_dims_range_args args;

    args.range_type = type;
    args.timestep_id = timestep_id;
    args.type_id = type_id;
    args.var_name_substr.reserve(2 + var_name_substr.size());
    args.var_name_substr = "%";
    //args.var_name_substr += boost::algorithm::to_lower_copy(var_name_substr);
    args.var_name_substr += var_name_substr;
    args.var_name_substr += "%";
    args.txn_id = txn_id;
    args.num_dims = num_dims;
    args.dims = dims;
    args.data_type = data_type;
    args.data = data;
	entries.clear();
	switch(data_type) {
        case ATTR_DATA_TYPE_INT : {
        	uint64_t min_int, max_int;

    	    get_range_values(data, type, min_int, max_int);

    		return_value = md_catalog_all_var_attributes_with_type_var_substr_dims_range_stub(args, 
    			min_int, max_int, entries, count);

            break;
        }
        case ATTR_DATA_TYPE_REAL : {
        	long double min_real, max_real;
    	    
    	    get_range_values(data, type, min_real, max_real);

    		return_value = md_catalog_all_var_attributes_with_type_var_substr_dims_range_stub(args, 
    			min_real, max_real, entries, count);

            break;
        }
    }   
    // return_value = md_catalog_all_var_attributes_with_type_var_substr_dims_range_stub(args, entries, count);
    extreme_debug_log << "just launched get attr _by dims \n";
    add_timing_point(MD_CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_DIMS_RANGE_DONE);   
    extreme_debug_log << "number of entries is " << entries.size() << "\n";


    return return_value;
} //end of funct

int metadata_catalog_all_var_attributes_with_type_var_substr_dims_range (uint64_t timestep_id
                                        ,uint64_t type_id
                                        ,const std::string &var_name_substr 
                                        ,uint64_t txn_id
                                        ,uint32_t num_dims
                                        ,const std::vector<md_dim_bounds> &dims
                                        ,attr_data_type data_type
                                        ,const std::string &data
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_var_attribute_entry> &entries
                                        )
{
    int return_value;

    return_value = metadata_catalog_all_var_attributes_with_type_var_substr_dims_val (DATA_RANGE, timestep_id, type_id,
        var_name_substr, txn_id, num_dims, dims, data_type, data, count, entries);

    return return_value;

} //end of funct



int metadata_catalog_all_var_attributes_with_type_var_substr_dims_above_max (uint64_t timestep_id
                                        ,uint64_t type_id
                                        ,const std::string &var_name_substr 
                                        ,uint64_t txn_id
                                        ,uint32_t num_dims
                                        ,const std::vector<md_dim_bounds> &dims
                                        ,attr_data_type data_type
                                        ,const std::string &data
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_var_attribute_entry> &entries
                                        )
{
    int return_value;

    return_value = metadata_catalog_all_var_attributes_with_type_var_substr_dims_val (DATA_MAX, timestep_id, type_id,
        var_name_substr, txn_id, num_dims, dims, data_type, data, count, entries);
    return return_value;
} //end of funct

int metadata_catalog_all_var_attributes_with_type_var_substr_dims_below_min (uint64_t timestep_id
                                        ,uint64_t type_id
                                        ,const std::string &var_name_substr 
                                        ,uint64_t txn_id
                                        ,uint32_t num_dims
                                        ,const std::vector<md_dim_bounds> &dims
                                        ,attr_data_type data_type
                                        ,const std::string &data
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_var_attribute_entry> &entries
                                        )
{
    int return_value;

    return_value = metadata_catalog_all_var_attributes_with_type_var_substr_dims_val (DATA_MIN, timestep_id, type_id,
        var_name_substr, txn_id, num_dims, dims, data_type, data, count, entries);

    return return_value;
} //end of funct






// int metadata_insert_var_attribute_by_dims_batch (uint64_t &attribute_id
//                            ,uint64_t timestep_id
//                            ,uint64_t type_id
//                            ,uint64_t var_id
//                            ,uint64_t txn_id
//                            ,uint32_t num_dims
//                            ,const std::vector<md_dim_bounds> &dims
//                            ,attr_data_type data_type
//                            ,const std::string &data
//                            )
// {
//     add_timing_point(MD_INSERT_VAR_ATTRIBUTE_BY_DIMS_BATCH_START);
//     int return_value;
//     std::string res;

//     if(time_pts.size() != catg_of_time_pts.size()) {
//         debug_log << "error they are off at start of md insert attributes " << endl;
//     }
//     md_insert_var_attribute_by_dims_args args;
//     args.timestep_id = timestep_id;
//     args.type_id = type_id;
//     args.var_id = var_id;
//     args.txn_id = txn_id;
//     args.num_dims = num_dims;
//     args.dims = dims;
//     args.data_type = data_type;
//     args.data = data;

//     insert_var_attribute_by_dims_batch_stub = new insert_var_attribute_by_dims_batchMeta(args, entries, count);// 
//     add_timing_point(MD_INSERT_VAR_ATTRIBUTE_BY_DIMS_BATCH_DONE);   
//     debug_log << "according to the client the attribute_id is " << attribute_id << endl;
//     // if(time_pts.size() != catg_of_time_pts.size()) {
//     //     debug_log << "error they are off at the end of md insert attributes " << endl;
//     // }
//     return return_value;
// } //end of funct

int metadata_insert_var_attribute_by_dims_batch (const vector<md_catalog_var_attribute_entry> &new_attributes)
{
    add_timing_point(MD_INSERT_VAR_ATTRIBUTE_BY_DIMS_BATCH_START);
    int return_value;
    std::string res;

    // if(time_pts.size() != catg_of_time_pts.size()) {
    //     debug_log << "error they are off at start of md insert attributes " << endl;
    // }
    vector<md_insert_var_attribute_by_dims_args> all_args;
    for (int i = 0; i < new_attributes.size(); i++) {
    	md_insert_var_attribute_by_dims_args args;
    	md_catalog_var_attribute_entry new_attribute = new_attributes.at(i);

    	args.timestep_id = new_attribute.timestep_id;
    	args.type_id = new_attribute.type_id;
    	args.var_id = new_attribute.var_id;
    	args.txn_id = new_attribute.txn_id;
    	args.num_dims = new_attribute.num_dims;
    	args.dims = new_attribute.dims;
    	args.data_type = new_attribute.data_type;
    	args.data = new_attribute.data;

    	all_args.push_back(args);
    }

    // std::vector<uint64_t> ids;
    // ids.reserve(new_attributes.size());

    return_value = md_insert_var_attribute_by_dims_batch_stub(all_args);

    // return_value = md_insert_var_attribute_by_dims_batch_stub(all_args, ids);
    add_timing_point(MD_INSERT_VAR_ATTRIBUTE_BY_DIMS_BATCH_DONE);   
    // debug_log << "according to the client the attribute_id is " << attribute_id << endl;
    // if(time_pts.size() != catg_of_time_pts.size()) {
    //     debug_log << "error they are off at the end of md insert attributes " << endl;
    // }
    return return_value;
} //end of funct
// int metadata_create_type_batch (uint64_t &type_id
//                         ,uint64_t run_id
//                         ,const std::string &name
//                         ,uint32_t version
//                         ,uint64_t txn_id
//                         )
// {

//note - could be modified to take indv parameters (see directly above) or to take a type_id from the user
// int metadata_create_type_batch (uint64_t &type_id
//                         ,const vector<md_catalog_type_entry> &new_types
//                         )
// int metadata_create_type_batch (vector<uint64_t> &type_ids
//                         ,const vector<md_catalog_type_entry> &new_types
//                         )
// int metadata_create_type_batch (const vector<md_catalog_type_entry> &new_types
// 

int metadata_create_type_batch (const vector<md_catalog_type_entry> &new_types
                        )
{
	int rc;
	uint64_t first_type_id;
	rc = metadata_create_type_batch(first_type_id, new_types);
	return rc;
}



int metadata_create_type_batch (uint64_t &first_type_id,
						const vector<md_catalog_type_entry> &new_types
                        )
{
    add_timing_point(MD_CREATE_TYPE_BATCH_START);
    extreme_debug_log << "got to top of metadata_create_type_batch \n";
    int return_value;
    std::string res;

    // type_ids.reserve(new_types.size());

    vector<md_create_type_args> all_args;

    for (int i = 0; i < new_types.size(); i++) {
        md_create_type_args args;
        md_catalog_type_entry new_type = new_types.at(i);

        args.run_id = new_type.run_id;
        args.name = new_type.name;
        args.version = new_type.version;
        args.txn_id = new_type.txn_id;
        extreme_debug_log << "about to create new type \n";

        all_args.push_back(args);
    }
    

    // std::vector<uint64_t> ids;
    // ids.reserve(new_types.size());

    // return_value = md_create_type_batch_stub(all_args, ids);

    return_value = md_create_type_batch_stub(all_args, first_type_id);
    extreme_debug_log << "about to get future" <<endl;    extreme_debug_log << "about to launch op" <<endl;

    extreme_debug_log << "about to get final result" <<endl;

    add_timing_point(MD_CREATE_TYPE_BATCH_DONE);   

    return return_value;
} //end of funct

// int metadata_create_var_batch (uint64_t &var_id
//                         ,uint64_t run_id
//                         ,uint64_t timestep_id
//                         ,const std::string &name
//                         ,const std::string &path
//                         ,uint32_t version
//                         ,char data_size
//                         ,uint64_t txn_id
//                         ,uint32_t num_dims
//                         ,const std::vector<md_dim_bounds> &dims
//                         )

//note - could be modified to take indv parameters (see directly above) or to NOT take a var_id from the user
int metadata_create_var_batch (const vector<md_catalog_var_entry> &new_vars
                        )
{
    add_timing_point(MD_CREATE_VAR_BATCH_START);
    extreme_debug_log << "got to top of metadata_create_var_batch \n";
    int return_value;
    std::string res;


    ///////todo/////////////////////////////////////////////////////////////////////
    // string temp;
    // size_t size;
    // for(int i=0; i< new_var.name.size(); i++) {
    //     temp += to_string(((int)new_var.name.at(i)));
    // }
    // temp += to_string((int)'_');
    // temp += to_string(new_var.version);
    // extreme_debug_log << "temp: " << temp << endl;
    // args.var_id = stoull(temp, &size, 10); //todo
   //  string var_name_ver = new_var.name;
   //  var_name_ver += "_";
   //  var_name_ver += to_string(new_var.version);
   //  extreme_debug_log << "var_name_ver: " << var_name_ver << endl;
   //  map <string, uint64_t>::iterator it = var_ids.find(var_name_ver);
   //  if ( it == var_ids.end() ) {
   //    // not found
   //      var_ids[var_name_ver] = var_ids.size();
   //      args.var_id = var_ids.size();

   //      extreme_debug_log << "not found, and var_ids.size(): " << var_ids.size() << endl;
   //  } else {
   //    // found
   //      extreme_debug_log << "found, var_id: " << it->second << endl;
   //      args.var_id = it->second;
   //  }

   // for (std::map<string,uint64_t>::iterator it=var_ids.begin(); it!=var_ids.end(); ++it) {
   //    string var_name = it->first;
   //    uint64_t temp_var_id = it->second;
   //    extreme_debug_log << "var_name: " << var_name << " temp_var_id " << temp_var_id << endl;
   //  }
   //  extreme_debug_log << "args.var_id: " << args.var_id << endl;

    //////////////////////////////////////////////////////////////////////////////////

    vector<md_create_var_args> all_args;
    for (int i = 0; i < new_vars.size(); i++) {
        md_create_var_args args;
        md_catalog_var_entry new_var = new_vars.at(i);

        args.run_id = new_var.run_id;
        args.var_id = new_var.var_id;
        args.timestep_id = new_var.timestep_id;
        args.name = new_var.name;
        args.path = new_var.path;
        args.version = new_var.version;
        args.data_size = new_var.data_size;
        args.txn_id = new_var.txn_id;
        args.num_dims = new_var.num_dims;
        extreme_debug_log << "run_id: " << new_var.run_id << " timestep_id: " << new_var.timestep_id << " var_id: " << args.var_id << " \n";

        args.dims = new_var.dims;
        extreme_debug_log << "about to create new var \n";

        all_args.push_back(args);

    }
    

    // std::vector<uint64_t> ids;
    // ids.reserve(new_attributes.size());

    // return_value = md_create_var_batch_stub(all_args, ids);   

    return_value = md_create_var_batch_stub(all_args);

    extreme_debug_log << "about to get future" <<endl;    extreme_debug_log << "about to launch op" <<endl;

    extreme_debug_log << "about to get final result" <<endl;

    // uint64_t rowid; //todo
    // var_id = args.var_id;
    add_timing_point(MD_CREATE_VAR_BATCH_DONE); 

    return return_value;
} //end of funct

// int metadata_insert_run_attribute_batch (uint64_t &attribute_id
//                            ,uint64_t type_id
//                            ,uint64_t txn_id
//                            ,attr_data_type data_type
//                            ,const std::string &data
//                            )
// {
//     add_timing_point(MD_INSERT_RUN_ATTRIBUTE_BATCH_START);
//     int return_value;
//     std::string res;

//     // if(time_pts.size() != catg_of_time_pts.size()) {
//     //     debug_log << "error they are off at start of md insert attributes " << endl;
//     // }
//     md_insert_run_attribute_args args;
//     args.type_id = type_id;
//     args.txn_id = txn_id;
//     args.data_type = data_type;
//     args.data = data;

//     insert_run_attribute_batch_stub = new insert_run_attribute_batchMeta(args, entries, count);// 
//     add_timing_point(MD_INSERT_RUN_ATTRIBUTE_BATCH_DONE);   
//     debug_log << "according to the client the attribute_id is " << attribute_id << endl;
//     // if(time_pts.size() != catg_of_time_pts.size()) {
//     //     debug_log << "error they are off at the end of md insert attributes " << endl;
//     // }
//     return return_value;
// } //end of funct

int metadata_insert_run_attribute_batch (const vector<md_catalog_run_attribute_entry> &new_attributes
                           )
{
    add_timing_point(MD_INSERT_RUN_ATTRIBUTE_BATCH_START);
    int return_value;
    std::string res;

    // if(time_pts.size() != catg_of_time_pts.size()) {
    //     debug_log << "error they are off at start of md insert attributes " << endl;
    // }

    vector<md_insert_run_attribute_args> all_args;
    for (int i = 0; i < new_attributes.size(); i++) {
        md_insert_run_attribute_args args;
        md_catalog_run_attribute_entry new_attribute = new_attributes.at(i);
        args.run_id = new_attribute.run_id;
        args.type_id = new_attribute.type_id;
        args.txn_id = new_attribute.txn_id;
        args.data_type = new_attribute.data_type;
        args.data = new_attribute.data;

        all_args.push_back(args);
    }

    // std::vector<uint64_t> ids;
    // ids.reserve(new_attributes.size());

    // return_value = md_insert_run_attribute_batch_stub(all_args, ids);
    return_value = md_insert_run_attribute_batch_stub(all_args);

    add_timing_point(MD_INSERT_RUN_ATTRIBUTE_BATCH_DONE);   
    // debug_log << "according to the client the attribute_id is " << attribute_id << endl;
    // if(time_pts.size() != catg_of_time_pts.size()) {
    //     debug_log << "error they are off at the end of md insert attributes " << endl;
    // }
    return return_value;
} //end of funct

// int metadata_insert_timestep_attribute_batch (uint64_t &attribute_id
//                            ,uint64_t timestep_id
//                            ,uint64_t type_id
//                            ,uint64_t txn_id
//                            ,attr_data_type data_type
//                            ,const std::string &data
//                            )
// {
//     add_timing_point(MD_INSERT_TIMESTEP_ATTRIBUTE_BATCH_START);
//     int return_value;
//     std::string res;

//     // if(time_pts.size() != catg_of_time_pts.size()) {
//     //     debug_log << "error they are off at start of md insert attributes " << endl;
//     // }
//     md_insert_timestep_attribute_args args;
//     args.timestep_id = timestep_id;
//     args.type_id = type_id;
//     args.txn_id = txn_id;
//     args.data_type = data_type;
//     args.data = data;

//     insert_timestep_attribute_batch_stub = new insert_timestep_attribute_batchMeta(args, entries, count);// 
//     add_timing_point(MD_INSERT_TIMESTEP_ATTRIBUTE_BATCH_DONE);   
//     debug_log << "according to the client the attribute_id is " << attribute_id << endl;
//     // if(time_pts.size() != catg_of_time_pts.size()) {
//     //     debug_log << "error they are off at the end of md insert attributes " << endl;
//     // }
//     return return_value;
// } //end of funct

int metadata_insert_timestep_attribute_batch (const vector<md_catalog_timestep_attribute_entry> &new_attributes
                           )
{
    add_timing_point(MD_INSERT_TIMESTEP_ATTRIBUTE_BATCH_START);
    int return_value;
    std::string res;

    // if(time_pts.size() != catg_of_time_pts.size()) {
    //     debug_log << "error they are off at start of md insert attributes " << endl;
    // }

    vector<md_insert_timestep_attribute_args> all_args;
    for (int i = 0; i < new_attributes.size(); i++) {
        md_insert_timestep_attribute_args args;
        md_catalog_timestep_attribute_entry new_attribute = new_attributes.at(i);

        args.timestep_id = new_attribute.timestep_id;
        args.type_id = new_attribute.type_id;
        args.txn_id = new_attribute.txn_id;
        args.data_type = new_attribute.data_type;
        args.data = new_attribute.data;

        all_args.push_back(args);
    }

    // std::vector<uint64_t> ids;
    // ids.reserve(new_attributes.size());

    // return_value = md_insert_timestep_attribute_batch_stub(all_args, ids);
    return_value = md_insert_timestep_attribute_batch_stub(all_args);
    add_timing_point(MD_INSERT_TIMESTEP_ATTRIBUTE_BATCH_DONE);   
    // debug_log << "according to the client the attribute_id is " << attribute_id << endl;
    // if(time_pts.size() != catg_of_time_pts.size()) {
    //     debug_log << "error they are off at the end of md insert attributes " << endl;
    // }
    return return_value;
} //end of funct

