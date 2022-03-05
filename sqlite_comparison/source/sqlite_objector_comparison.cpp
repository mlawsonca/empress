
#include <stdio.h> //needed for printf
#include <stdlib.h> //needed for atoi/atol/malloc/free/rand
#include <assert.h> //needed for assert
#include <string.h> //needed for strcmp

// #include <ctime>
#include <stdint.h> //needed for uint
// #include <ratio>
#include <chrono> //needed for high_resolution_clock
#include <math.h> //needed for pow()
#include <sys/time.h> //needed for timeval
#include <sys/stat.h> //needed for stat
#include <fstream> //needed for ifstream
// #include <vector>
#include <float.h> //needed for DBL_MAX

#include <sqlite_objector_comparison.hh>

// #include <md_client_timing_constants.hh>
// #include <client_timing_constants_write.hh>

#include <3d_read_for_testing_objector_comparison.hh>

#include <sqlite3.h>

extern "C" {
  #include <lua.h>
  #include <lauxlib.h>
  #include <lualib.h>
}

sqlite3 * db = NULL;


using namespace std;

static bool extreme_debug_logging = false;
static bool debug_logging = false;
static bool error_logging = true;
static bool testing_logging = false;

static debugLog error_log = debugLog(error_logging);
static debugLog debug_log = debugLog(debug_logging);
static debugLog extreme_debug_log = debugLog(extreme_debug_logging);
static debugLog testing_log = debugLog(testing_logging);

static bool output_timing = true;
static bool output_queries = false;

std::vector<int> catg_of_time_pts;
vector<long double> time_pts;

void add_timing_point(uint16_t code) {
    if (output_timing) {
        struct timeval now;
        gettimeofday(&now, NULL);
        int zero_time_sec = 3600 * (now.tv_sec / 3600);
        // cout << "time_pts.push_back: " << ((now.tv_sec - zero_time_sec) + now.tv_usec / 1000000.0) << endl;
        time_pts.push_back( (now.tv_sec - zero_time_sec) + now.tv_usec / 1000000.0);
        catg_of_time_pts.push_back(code);
        // cout << "now.tv_sec: " << now.tv_sec << " zero_time_sec: " << zero_time_sec << 
        // 	" now.tv_usec: " << now.tv_usec << endl;
    }
}

static int sql_output_db(sqlite3 *pInMemory, uint64_t job_id, int rank);

lua_State *L;

/*

*/
int main(int argc, char **argv) {
    int rc;


    // if (argc != 12) { 
    //     error_log << "Error. Program should be given 11 arguments. npx_write, npy_write, npz_write, npx_read, npy_read, npz_read, nx, ny, nz, number of timesteps, job_id" << endl;
    //     // cout << (int)ERR_ARGC << " " << 0 <<  endl;
    //     return RC_ERR;
    // }
    

    uint32_t num_x_procs = stoul(argv[1],nullptr,0);
    uint32_t num_y_procs = stoul(argv[2],nullptr,0);
    uint32_t num_z_procs = stoul(argv[3],nullptr,0);   
    // uint32_t num_read_x_procs = stoul(argv[4],nullptr,0);
    // uint32_t num_read_y_procs = stoul(argv[5],nullptr,0);
    // uint32_t num_read_z_procs = stoul(argv[6],nullptr,0);   
    uint64_t total_x_length = stoull(argv[4],nullptr,0);
    uint64_t total_y_length = stoull(argv[5],nullptr,0);
    uint64_t total_z_length = stoull(argv[6],nullptr,0);
    uint32_t num_timesteps = stoul(argv[7],nullptr,0);
    // uint64_t job_id = stoull(argv[11],nullptr,0);

	//1k clients
    // uint32_t total_x_length = 2500;
    // uint32_t total_y_length = 3200;
    // uint32_t total_z_length = 5000;
    // uint32_t num_x_procs = 10;
    // uint32_t num_y_procs = 10;
    // uint32_t num_z_procs = 10;

	//1k clients
    // uint32_t total_x_length = 5000;
    // uint32_t total_y_length = 6400;
    // uint32_t total_z_length = 12500;
   	// uint32_t num_x_procs = 20;
    // uint32_t num_y_procs = 20;
    // uint32_t num_z_procs = 25;

    // uint32_t num_read_x_procs = 4;
    // uint32_t num_read_y_procs = 5;
    // uint32_t num_read_z_procs = 5;
    uint32_t num_read_x_procs = 1;
    uint32_t num_read_y_procs = 1;
    uint32_t num_read_z_procs = 1;



    // uint32_t total_x_length = 1000;
    // uint32_t total_y_length = 2000;
    // uint32_t total_z_length = 2000;
    // uint32_t num_x_procs = 1;
    // uint32_t num_y_procs = 2;
    // uint32_t num_z_procs = 5;

    // uint32_t num_timesteps = 2;
    uint64_t job_id = 12345678;

    extreme_debug_log << "job_id: " << job_id << endl;

    uint32_t num_client_procs = num_x_procs * num_y_procs * num_z_procs;
    // uint32_t num_client_procs = 1;
    // uint32_t num_client_procs = 10000;

    int rank = 0;

    //all 10 char plus null character
    uint32_t num_vars = 10;
    char var_names[10][11] = {"temperat", "temperat", "pressure", "pressure", "velocity_x", "velocity_y", "velocity_z", "magn_field", "energy_var", "density_vr"};  

    uint32_t estm_num_time_pts = 200000;
    catg_of_time_pts.reserve(estm_num_time_pts);
    time_pts.reserve(estm_num_time_pts);


    add_timing_point(PROGRAM_START);


    vector<md_dim_bounds> var_dims(3);
    vector<md_dim_bounds> proc_dims(3);

    uint32_t version1 = 1;
    uint32_t version2 = 2;

    //given in %
    float type_freqs[] = {25, 5, .1, 100, 100, 20, 2.5, .5, 1, 10};
    // char type_types[10][3] = {"b", "b", "b", "d", "d", "2p","2p","2p","2d","2d"};
    char type_types[10][3] = {"b", "b", "b", "d", "d", "s","s","s","2d","2d"};
    //all 12 char plus null character
    char type_names[10][13] = {"blob_freq", "blob_ifreq", "blob_rare", "max_val_type", "min_val_type", "note_freq", "note_ifreq", "note_rare", "ranges_type1", "ranges_type2"}; 


    char run_timestep_type_names[2][9] = {"max_temp", "min_temp"}; 
    int num_run_timestep_types = 2;
    uint32_t num_types = 10;

    extreme_debug_log << "num_type: " << num_types << " num_run_timestep_types: " << num_run_timestep_types << endl;

    uint64_t x_length_per_proc = total_x_length / num_x_procs; //todo - deal with edge case?
    uint64_t y_length_per_proc = total_y_length / num_y_procs;
    uint64_t z_length_per_proc = total_z_length / num_z_procs;

    uint64_t chunk_size = x_length_per_proc * y_length_per_proc * z_length_per_proc * 8; //8 bytes values

    struct md_catalog_var_entry temp_var;

    uint64_t my_var_id;
    uint32_t num_dims;
    uint32_t var_num;
    uint32_t var_version;

    struct md_catalog_run_entry temp_run;


    uint32_t x_pos;
    uint32_t y_pos;
    uint32_t z_pos;
    uint32_t x_offset;
    uint32_t y_offset;
    uint32_t z_offset;

    vector<vector<md_catalog_var_entry>> all_var_entries(num_timesteps);


    add_timing_point(DB_INIT_START);

    //set up and return the database
    rc = metadata_database_init ();
    if (rc != RC_OK) {
        error_log << "Error. Could not init dbs. Error code " << rc << std::endl;
        add_timing_point(ERR_DB_SETUP);
        return RC_ERR;
    }
    add_timing_point(DB_INIT_DONE);

    uint64_t db_size0 = sqlite3_memory_used();

    add_timing_point(LUA_INIT_START);

    L = luaL_newstate(); //fix - do I ever open a new state or deregister the rank to bounding box functions?
    luaL_openlibs(L);   
    string objector_funct_path = "../source/metadata_functs.lua";

   if (luaL_loadfile(L, objector_funct_path.c_str()) || lua_pcall(L, 0, 0, 0)) {
        cout << "cannot load the given lua function path: " << lua_tostring(L, -1) << "\n";
        return RC_ERR;
    }

    add_timing_point(LUA_INIT_DONE);


    temp_run.job_id = job_id;
    temp_run.name = "XGC";
    temp_run.txn_id = 0;
    temp_run.npx = num_x_procs;
    temp_run.npy = num_y_procs;
    temp_run.npz = num_z_procs;



    //reminder - needs to be done for each CLIENT, not server
    uint64_t run_id = 1;

    temp_run.run_id = run_id; //todo - do we want to make them catalog to get this? or change so registering doesn't require a name

   	
   	uint64_t obj_x_width;
   	uint64_t last_obj_x_width;

    get_obj_lengths(obj_x_width, last_obj_x_width, x_length_per_proc, chunk_size);

    debug_log << "obj size: " << obj_x_width * y_length_per_proc * z_length_per_proc * 8 << endl;

    debug_log << "obj_x_width: " << obj_x_width << " last_obj_x_width: " << last_obj_x_width << endl;

    uint32_t num_objs;

    for(uint32_t timestep=0; timestep < num_timesteps; timestep++) {
        add_timing_point(WRITE_TIMESTEP_START);
        debug_log << "beginning writing for timestep " << timestep << endl;

        temp_var.run_id = run_id;
        temp_var.timestep_id = timestep;
        temp_var.data_size = 8; //double - 64 bit floating point
        temp_var.txn_id = timestep; 
     
        for(uint32_t var_indx=0; var_indx < num_vars; var_indx++) {
            add_timing_point(WRITE_VAR_START);
        debug_log << "beginning writing for var " << var_indx << endl;

            temp_var.var_id = var_indx;
            temp_var.name = var_names[var_indx];
            temp_var.path = "/"+ (string) temp_var.name;
            if(var_indx == 1 || var_indx == 3) { //these have been designated ver2
                temp_var.version = version2;
            }
            else {
                temp_var.version = version1;
            }

            temp_var.num_dims = 3;
            var_dims [0].min = 0;
            var_dims [0].max = total_x_length-1;
            var_dims [1].min = 0;
            var_dims [1].max = total_y_length-1;
            var_dims [2].min = 0;
            var_dims [2].max = total_z_length-1;
            // temp_var.dims = std::vector<md_dim_bounds>(var_dims, var_dims + temp_var.num_dims );
            temp_var.dims = var_dims;

            all_var_entries.at(timestep).push_back(temp_var);

	        add_timing_point(INSERT_OBJECTS_START);

            for (int rank = 0; rank < num_client_procs; rank ++) {
			    x_pos = rank % num_x_procs;
			    y_pos = (rank / num_x_procs) % num_y_procs; 
			    z_pos = ((rank / (num_x_procs * num_y_procs)) % num_z_procs);

			    x_offset = x_pos * x_length_per_proc;
			    y_offset = y_pos * y_length_per_proc;
			    z_offset = z_pos * z_length_per_proc;

			    num_dims = 3;
			    proc_dims [0].min = x_offset;
			    proc_dims [0].max = x_offset + x_length_per_proc-1;
			    proc_dims [1].min = y_offset;
			    proc_dims [1].max = y_offset + y_length_per_proc -1;
			    proc_dims [2].min = z_offset;
			    proc_dims [2].max = z_offset + z_length_per_proc -1;

	            std::vector<string> obj_names;

	            add_timing_point(BOUNDING_BOX_TO_OBJ_NAMES_START);

	            rc = boundingBoxToObjNames(temp_run, temp_var, proc_dims, obj_names);
	            if (rc != RC_OK) {
	                error_log << "Error doing the bounding box to obj names, returning \n";
	                return RC_ERR;
	            }
	            add_timing_point(BOUNDING_BOX_TO_OBJ_NAMES_DONE);

	           	md_insert_object_args obj_args;
	            // obj_args.run_id = temp_run.run_id;
	            obj_args.timestep_id = temp_var.timestep_id;
	            obj_args.var_id = temp_var.var_id;
	            // obj_args.txn_id = temp_var.txn_id;
	            // obj_args.num_dims = 3;
	            if(timestep == 0 && var_indx == 0) {
					num_objs = obj_names.size();
	            }

	            extreme_debug_log << "obj_names.size(): " << obj_names.size() << endl;
	            for (int i = 0; i < obj_names.size(); i++) {
	            	uint64_t x_width;

	            	if (i == (obj_names.size()-1 )) {
	            		x_width = last_obj_x_width;
	            	}
	            	else {
	            		x_width = obj_x_width;
	            	}

	            	vector<md_dim_bounds> obj_dims(3);
	            	obj_dims[0].min = proc_dims[0].min + i*x_width;
	        	    obj_dims[0].max = obj_dims[0].min + x_width - 1;
    		        obj_dims[1].min = proc_dims[1].min;
	        	    obj_dims[1].max = proc_dims[1].max;
    		        obj_dims[2].min = proc_dims[2].min;
	        	    obj_dims[2].max = proc_dims[2].max;
	            	obj_args.dims = obj_dims;
	            	obj_args.object_name = obj_names.at(i);

	            	uint64_t object_id;

	            	add_timing_point(INSERT_OBJECT_START);

	            	rc = md_insert_object_stub(obj_args, object_id);
	            	if(rc != RC_OK) {
	            		error_log << "error with md_insert_object_stub" << endl;
	            	}
	            	add_timing_point(INSERT_OBJECT_DONE);

	            }
	        	add_timing_point(INSERT_OBJECTS_DONE);

	        }
            add_timing_point(WRITE_VAR_DONE);

        } //end for(uint32_t var_indx=0; var_indx < num_vars; var_indx++)
        add_timing_point(WRITE_TIMESTEP_DONE);
    } // end for(uint32_t timestep=0; timestep < num_timesteps; timestep++)


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///reading section /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //fix - should switch to read procs 

    debug_log << "beginning writing \n";

    rc = read_3d_patterns ( temp_run, all_var_entries, num_read_x_procs, num_read_y_procs, num_read_z_procs, num_timesteps,
    				total_x_length, total_y_length, total_z_length
    				);
    if(rc != RC_OK) {
    	error_log << "error with read_3d_patterns \n";
    }


//-------------------------------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------------------------------        
cleanup:
        
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    uint64_t db_size1 = sqlite3_memory_used();


  	sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;

    rc = sqlite3_prepare_v2 (db, "select count (*) from oid_catalog", -1, &stmt, &tail); assert (rc == SQLITE_OK);
    rc = sqlite3_step (stmt); assert (rc == SQLITE_ROW || rc == SQLITE_DONE);
    uint64_t db_size2 = sqlite3_column_int (stmt, 0);
    rc = sqlite3_finalize (stmt); assert (rc == SQLITE_OK);

    cout << "begin timing output" << endl;
    cout << DB_SIZES << " " << db_size0 << " " << db_size1 << " " << db_size2 << endl;
    for(int i=0; i<time_pts.size(); i++) {
        printf("%d %Lf ", catg_of_time_pts[i], time_pts[i]);

        if (i%20 == 0 && i!=0) {
            std::cout << std::endl;
        }
    }
    std::cout << std::endl;

    cout << "num_objects: " << db_size2 << endl;


    debug_log << "got to cleanup7" << endl;

	rc = sql_output_db(db, job_id, rank);


    // sqlite3_close (db);

    return rc;

}



int read_3d_patterns ( const md_catalog_run_entry run, const vector<vector<md_catalog_var_entry>> &all_var_entries, 
				uint32_t num_x_procs, uint32_t num_y_procs, uint32_t num_z_procs,
				uint32_t num_timesteps, uint64_t total_x_length, uint64_t total_y_length, uint64_t total_z_length 
				)
{
	int rc;

    add_timing_point(READING_START);

    uint32_t x_length_per_proc = total_x_length / num_x_procs; //todo - deal with edge case?
    uint32_t y_length_per_proc = total_y_length / num_y_procs;
    uint32_t z_length_per_proc = total_z_length / num_z_procs;

	vector<md_dim_bounds> proc_dims(3);
	uint32_t num_vars = 10;
	int txn_id = -1;

	const int num_timesteps_to_fetch = 6;

    uint64_t timestep_ids[num_timesteps_to_fetch] = {(num_timesteps-1) / 6, (num_timesteps-1) / 5, (num_timesteps-1) / 4,
                        (num_timesteps-1) / 3, (num_timesteps-1) / 2, num_timesteps-1};    
   
    ;
   
    std::vector<md_catalog_var_entry> vars_to_fetch_pattern2;
    std::vector<md_catalog_var_entry> vars_to_fetch_pattern3;
    std::vector<md_catalog_var_entry> vars_to_fetch_pattern4;
    std::vector<md_catalog_var_entry> vars_to_fetch_pattern5;
    std::vector<md_catalog_var_entry> vars_to_fetch_pattern6;

 
    int plane_x_procs;
    int plane_y_procs;

    bool var_found;
    uint64_t var_ids[3];

  
    // for (int j=0; j<3; j++) {
    //     debug_log <<  "proc_dims [" << to_string(j) << "] min is: " << to_string(proc_dims [j].min) << endl;
    //     debug_log <<  "proc_dims [" << to_string(j) << "] max is: " << to_string(proc_dims [j].max) << endl;
    // }        


    var_found = false;

    for (md_catalog_var_entry var : all_var_entries.at(timestep_ids[1])) {
        uint64_t var_id0 = 0; 
        var_ids[0] = var_id0;
        if (var.var_id == var_id0) {
            vars_to_fetch_pattern2.push_back(var);
            var_found = true;
            break;
        }
    }
    if (! var_found) {
        error_log << "error. var_id " << var_ids[0] << " not found for pattern 2. returning \n";
        return -1;
    }

    var_found = false;
    for (md_catalog_var_entry var : all_var_entries.at(timestep_ids[2])) {
        uint32_t num_vars = all_var_entries.at(timestep_ids[2]).size();
        uint64_t var_id1 = num_vars/6;
        uint64_t var_id2 = num_vars/2;
        uint64_t var_id3 = num_vars/4;
        var_ids[0] = var_id1;
        var_ids[1] = var_id2;
        var_ids[2] = var_id3;

        if (var.var_id == var_id1 || var.var_id == var_id2 || var.var_id == var_id3 ) {
            vars_to_fetch_pattern3.push_back(var);
        }
        if (vars_to_fetch_pattern3.size() == 3) {
            var_found = true;
            break;
        }
    }
    if (! var_found) {
        error_log << "error. var_ids " << var_ids[0] << ", " << var_ids[1] << ", " << var_ids[2] << " not found for pattern 3. returning \n";
        error_log << "cataloging all_var_entries.at(timestep_ids[2])" << endl;
        for (int i = 0; i< all_var_entries.at(timestep_ids[2]).size(); i++) {
            error_log << "all_var_entries.at(timestep_ids[2]).at(i).var_id: " << all_var_entries.at(timestep_ids[2]).at(i).var_id << endl;
        }
        return -1;
    }

    var_found = false;
    for (md_catalog_var_entry var : all_var_entries.at(timestep_ids[3])) {
        uint64_t var_id4 = all_var_entries.at(timestep_ids[3]).size()/3;
        var_ids[0] = var_id4;

        if (var.var_id == var_id4 ) {
            vars_to_fetch_pattern4.push_back(var);
            var_found = true;
            break;
        }
    }
    if (! var_found) {
        error_log << "error. var_id " << var_ids[0] << " not found for pattern 4. returning \n";
        return -1;
    }

    var_found = false;
    for (md_catalog_var_entry var : all_var_entries.at(timestep_ids[4])) {
        uint64_t var_id5 = all_var_entries.at(timestep_ids[4]).size() - 2;
        var_ids[0] = var_id5;

        if (var.var_id == var_id5 ) {
            vars_to_fetch_pattern5.push_back(var);
            var_found = true;
            break;
        }
    }
    if (! var_found) {
        error_log << "error. var_id " << var_ids[0] << " not found for pattern 5. returning \n";
        return -1;
    }

    var_found = false;
    for (md_catalog_var_entry var : all_var_entries.at(timestep_ids[5])) {
        uint64_t var_id6 = all_var_entries.at(timestep_ids[5]).size() - 1;
        var_ids[0] = var_id6;

        if (var.var_id == var_id6 ) {
            vars_to_fetch_pattern6.push_back(var);
            var_found = true;
            break;
        }
    }
    if (! var_found) {
        error_log << "error. var_id " << var_ids[0] << " not found for pattern 6. returning \n";
        return -1;
    }
    add_timing_point(FIND_VARS_DONE);

    uint32_t num_client_procs = num_x_procs * num_y_procs * num_z_procs;
    debug_log << "num_client_procs: " << num_client_procs << endl;

	int my_sqrt = floor(sqrt(num_client_procs));
	while(num_client_procs % my_sqrt != 0) {
	    my_sqrt -= 1;
	}

    for(int rank = 0; rank < num_client_procs; rank++) {

	    uint32_t x_pos = rank % num_x_procs;
	    uint32_t y_pos = (rank / num_x_procs) % num_y_procs; 
	    uint32_t z_pos = ((rank / (num_x_procs * num_y_procs)) % num_z_procs);

	    uint64_t x_offset = x_pos * x_length_per_proc;
	    uint64_t y_offset = y_pos * y_length_per_proc;
	    uint64_t z_offset = z_pos * z_length_per_proc;

	    proc_dims [0].min = x_offset;
	    proc_dims [0].max = x_offset + x_length_per_proc - 1;
	    proc_dims [1].min = y_offset;
	    proc_dims [1].max = y_offset + y_length_per_proc - 1;
	    proc_dims [2].min = z_offset;
	    proc_dims [2].max = z_offset + z_length_per_proc - 1;


	    add_timing_point(READ_PATTERN_1_START);
	    //pattern testing for vars
	    rc = read_pattern_1 ( proc_dims, run, all_var_entries.at(timestep_ids[0]), 
	            num_vars);
	    if(rc != RC_OK) {
	        add_timing_point(ERR_PATTERN_1);
	    }
	    debug_log << "finished pattern 1" << endl;
	    add_timing_point(READ_PATTERN_1_DONE);

	    if(vars_to_fetch_pattern2.size() != 1) {
	        error_log << "Error. For pattern 2, expecting vars_to_fetch.size() to equal 1 but instead it equals " << vars_to_fetch_pattern2.size() << endl;
	    }
	    else {
	        debug_log << "txn_id: " << txn_id << " varid to fetch: " << vars_to_fetch_pattern2.at(0).var_id << endl;

	        add_timing_point(READ_PATTERN_2_START);
	        rc = read_pattern_2 (proc_dims, run, vars_to_fetch_pattern2.at(0)); 
	        debug_log << "finished pattern 2" << endl;
	        if(rc != RC_OK) {
	            add_timing_point(ERR_PATTERN_2);
	        }
	        add_timing_point(READ_PATTERN_2_DONE);
	    }

	    if(vars_to_fetch_pattern3.size() != 3) {
	        error_log << "Error. For pattern 3, expecting vars_to_fetch.size() to equal 3 but instead it equals " << vars_to_fetch_pattern3.size() << endl;
	    }
	    else {
	        add_timing_point(READ_PATTERN_3_START);
	        rc = read_pattern_3 (proc_dims, run, vars_to_fetch_pattern3);
	        debug_log << "finished pattern 3" << endl;
	        if(rc != RC_OK) {
	            add_timing_point(ERR_PATTERN_3); 
	        }
	        add_timing_point(READ_PATTERN_3_DONE);   
	    }

	    //want to use all read procs in pattern 4 and 6 as "x and y" procs
	    extreme_debug_log << "num_client_procs: " << num_client_procs << endl;
	    plane_x_procs = my_sqrt;
	    plane_y_procs = num_client_procs / plane_x_procs;
	    extreme_debug_log << "num x procs: " << num_x_procs << " num y procs: "; 
	    extreme_debug_log << num_y_procs << " num z procs: " << num_z_procs << " num x procs plane: ";
	    extreme_debug_log << plane_x_procs << " num y procs plane: " << plane_y_procs << endl;

	    if(vars_to_fetch_pattern4.size() != 1) {
	        error_log << "Error. For pattern 4, expecting vars_to_fetch.size() to equal 1 but instead it equals " << vars_to_fetch_pattern4.size() << endl;
	    }
	    else {
	        add_timing_point(READ_PATTERN_4_START);
	        rc = read_pattern_4 (rank, plane_x_procs, plane_y_procs,
	                    total_x_length, total_y_length, total_z_length, 
	                    run, vars_to_fetch_pattern4.at(0));

	        debug_log << "finished pattern 4" << endl;
	        if(rc != RC_OK) {
	            add_timing_point(ERR_PATTERN_4);
	        }
	        add_timing_point(READ_PATTERN_4_DONE);
	    }

	    if(vars_to_fetch_pattern5.size() != 1) {
	        error_log << "Error. For pattern 5, expecting vars_to_fetch.size() to equal 1 but instead it equals " << vars_to_fetch_pattern5.size() << endl;
	    }
	    else {
	        add_timing_point(READ_PATTERN_5_START);
	        rc = read_pattern_5( rank, num_x_procs, num_y_procs, num_z_procs,
	                        total_x_length, total_y_length, total_z_length, 
	                        run, vars_to_fetch_pattern5.at(0));
	        debug_log << "finished pattern 5" << endl;
	        if(rc != RC_OK) {
	            add_timing_point(ERR_PATTERN_5);  
	        }
	        add_timing_point(READ_PATTERN_5_DONE);
	    }

	    if(vars_to_fetch_pattern6.size() != 1) {
	        error_log << "Error. For pattern 6, expecting vars_to_fetch.size() to equal 1 but instead it equals " << vars_to_fetch_pattern6.size() << endl;
	    }
	    else {
	        add_timing_point(READ_PATTERN_6_START);
	        rc = read_pattern_6 (rank, plane_x_procs, plane_y_procs,
	                        total_x_length, total_y_length, total_z_length, 
	                        run, vars_to_fetch_pattern6.at(0));
	        debug_log << "finished pattern 6" << endl;
	        if(rc != RC_OK) {
	            add_timing_point(ERR_PATTERN_6);
	        }
	        add_timing_point(READ_PATTERN_6_DONE);
	    }
	    add_timing_point(READING_PATTERNS_DONE);
	}

	return rc;
}




static int callback (void * NotUsed, int argc, char ** argv, char ** ColName)
{
    int i;
    for (i = 0; i < argc; i++)
    {
        printf ("%s = %s\n", ColName [i], argv [i] ? argv [i] : "NULL");
    }
    printf ("\n");
    return 0;
}


//===========================================================================
static int metadata_database_init ()
{
    //setup the database
    int rc;
    char  *ErrMsg = NULL;
    
    rc = sqlite3_open (":memory:", &db);
    
    if (rc != SQLITE_OK)
    {   
        fprintf (stderr, "Can't open database: %s\n", sqlite3_errmsg (db));        
        sqlite3_close (db);
        goto cleanup;
    }

    rc = sqlite3_exec (db, "create table oid_catalog (id integer primary key autoincrement not null, timestep_id not null, var_id int not null, d0_min int, d0_max int, d1_min int, d1_max int, d2_min int, d2_max int, name varchar (50) )", callback, 0, &ErrMsg);
    if (rc != SQLITE_OK)    {
        fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
        sqlite3_free (ErrMsg);
        sqlite3_close (db);
        goto cleanup;
    }

    rc = sqlite3_exec (db, "create index oc_run_timestep_var_id_dims on oid_catalog (timestep_id, var_id, d0_min)", callback, 0, &ErrMsg);
    if (rc != SQLITE_OK)    {
        fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
        sqlite3_free (ErrMsg);
        sqlite3_close (db);
        goto cleanup;
    }


cleanup:
    return rc;
}


// //to use to insert objects into the table
// int md_insert_object_stub (const md_insert_object_args &args,
//                           uint64_t &object_id)
// {
//     int rc;
//     int i = 0;
//     sqlite3_stmt * stmt_index = NULL;
//     const char * tail_index = NULL;

//     rc = sqlite3_prepare_v2 (db, "insert into oid_catalog (id, run_id, timestep_id, var_id, d0_min, d0_max, d1_min, d1_max, d2_min, d2_max, name) values (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)", -1, &stmt_index, &tail_index);
//     if (rc != SQLITE_OK)
//     {
//         fprintf (stderr, "Error at start of insert_var_object_stub: Line: %d SQL error: %s\n", __LINE__, sqlite3_errmsg (db));
//         sqlite3_close (db);
//         goto cleanup;
//     }

//     rc = sqlite3_bind_null (stmt_index, 1); assert (rc == SQLITE_OK);
//     rc = sqlite3_bind_int64 (stmt_index, 2, args.run_id); assert (rc == SQLITE_OK);   
//     rc = sqlite3_bind_int64 (stmt_index, 3, args.timestep_id); assert (rc == SQLITE_OK);   
//     rc = sqlite3_bind_int64 (stmt_index, 4, args.var_id); assert (rc == SQLITE_OK);
//     while(i < args.num_dims) {
//         rc = sqlite3_bind_int64 (stmt_index, 5 + i * 2, args.dims.at(i).min); assert (rc == SQLITE_OK);
//         rc = sqlite3_bind_int64 (stmt_index, 6 + i * 2, args.dims.at(i).max); assert (rc == SQLITE_OK);
//         i++;
//     }
//     rc = sqlite3_bind_text (stmt_index, 11, strdup(args.object_name.c_str()), -1, free); assert (rc == SQLITE_OK);

//     assert (rc == SQLITE_OK);
//     rc = sqlite3_step (stmt_index); assert (rc == SQLITE_ROW || rc == SQLITE_DONE);


//     object_id = (int) sqlite3_last_insert_rowid (db);

//     rc = sqlite3_finalize (stmt_index);


// cleanup:

//     return rc;
// }

//to use to insert objects into the table
int md_insert_object_stub (const md_insert_object_args &args,
                          uint64_t &object_id)
{
    int rc;
    int i = 0;
    sqlite3_stmt * stmt_index = NULL;
    const char * tail_index = NULL;

    rc = sqlite3_prepare_v2 (db, "insert into oid_catalog (id, timestep_id, var_id, d0_min, d0_max, d1_min, d1_max, d2_min, d2_max, name) values (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)", -1, &stmt_index, &tail_index);
    if (rc != SQLITE_OK)
    {
        fprintf (stderr, "Error at start of insert_var_object_stub: Line: %d SQL error: %s\n", __LINE__, sqlite3_errmsg (db));
        sqlite3_close (db);
        goto cleanup;
    }

    rc = sqlite3_bind_null (stmt_index, 1); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt_index, 2, args.timestep_id); assert (rc == SQLITE_OK);   
    rc = sqlite3_bind_int64 (stmt_index, 3, args.var_id); assert (rc == SQLITE_OK);
    while(i < 3) {
        rc = sqlite3_bind_int64 (stmt_index, 4 + i * 2, args.dims.at(i).min); assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int64 (stmt_index, 5 + i * 2, args.dims.at(i).max); assert (rc == SQLITE_OK);
        i++;
    }
    rc = sqlite3_bind_text (stmt_index, 10, strdup(args.object_name.c_str()), -1, free); assert (rc == SQLITE_OK);

    assert (rc == SQLITE_OK);
    rc = sqlite3_step (stmt_index); assert (rc == SQLITE_ROW || rc == SQLITE_DONE);


    object_id = (int) sqlite3_last_insert_rowid (db);

    rc = sqlite3_finalize (stmt_index);


cleanup:

    return rc;
}



//to use to query objects
int md_catalog_all_objects_with_var_dims_stub (const md_catalog_all_objects_with_var_dims_args &args,
						   std::vector<md_catalog_object_entry> &object_list,
						   uint32_t &count)
{
	int rc;
	sqlite3_stmt * stmt = NULL;
	const char * tail = NULL;
	const char * query = "select * from oid_catalog oc "
	"where oc.timestep_id = ? "
	"and oc.var_id = ? "
	"and (? <= oc.d0_max and ? >= oc.d0_min) "
	"and (? <= oc.d1_max and ? >= oc.d1_min) "
	"and (? <= oc.d2_max and ? >= oc.d2_min) ";

	if(output_queries) {
		printf("query: * from oid_catalog oc where oc.timestep_id = %llu "
		"and oc.var_id = %llu "
		"and (%llu <= oc.d0_max and %llu >= oc.d0_min) "
		"and (%llu <= oc.d1_max and %llu >= oc.d1_min) "
		"and (%llu <= oc.d2_max and %llu >= oc.d2_min) \n", args.timestep_id, args.var_id,
		args.dims[0].min, args.dims[0].max, args.dims[1].min, args.dims[1].max,
		args.dims[2].min, args.dims[2].max);
	}

	// extreme_debug_log << "starting" << endl;
	// extreme_debug_log << "run_id: " << args.run_id << " timestep_id: " << args.timestep_id << " txn_id: " << args.txn_id <<
	// 		" var_id: " << args.var_id << " d0min: " << args.dims.at(0).min << " d0max: " << args.dims.at(0).max << endl;
	// extreme_debug_log << "args.num_dims: " << args.num_dims << endl;
	rc = get_matching_object_count (args, count); assert (rc == RC_OK);

	if (count > 0) {
		object_list.reserve(count);
		// extreme_debug_log << "count: " << count << endl;

		rc = sqlite3_prepare_v2 (db, query, -1, &stmt, &tail); 
		if (rc != SQLITE_OK)
		{
			fprintf (stderr, "Error catalog_all_objects_with_var_dims: Line: %d SQL error (%d): %s\n", __LINE__, rc, sqlite3_errmsg (db));
			sqlite3_close (db);
			goto cleanup;
		}
		rc = sqlite3_bind_int64 (stmt, 1, args.timestep_id); assert (rc == SQLITE_OK);
		rc = sqlite3_bind_int64 (stmt, 2, args.var_id); assert (rc == SQLITE_OK);

		for (int j = 0; j < 3; j++)
		{
			// extreme_debug_log << "got here \n";
			rc = sqlite3_bind_int64 (stmt, 3 + (j * 2), args.dims.at(j).min);
			rc = sqlite3_bind_int64 (stmt, 4 + (j * 2), args.dims.at(j).max);
		}
		rc = sqlite3_step (stmt); assert (rc == SQLITE_ROW || rc == SQLITE_DONE);

		while (rc == SQLITE_ROW)
		{
			md_catalog_object_entry object;

			// extreme_debug_log << "got here 2\n";
			object.object_id = sqlite3_column_int64 (stmt, 0);
			object.timestep_id = sqlite3_column_int64 (stmt, 1);
			object.var_id = sqlite3_column_int64 (stmt, 2);
			object.dims.reserve(3);
			int j = 0;
			while (j < 3)
			{
				// extreme_debug_log << "got here 3\n";

				md_dim_bounds bounds;
				bounds.min =  sqlite3_column_double (stmt, 3 + (j * 2));
				bounds.max = sqlite3_column_double (stmt, 4 + (j * 2));
				object.dims.push_back(bounds);
				j++;
			}
			object.object_name = (char *)sqlite3_column_text (stmt, 9);
			// extreme_debug_log << "object_name: " << object.object_name << endl;
								
			rc = sqlite3_step (stmt);
			object_list.push_back(object);
		}

		rc = sqlite3_finalize (stmt);  
	}
		

cleanup:

	return rc;
}



static int get_matching_object_count (const md_catalog_all_objects_with_var_dims_args &args, uint32_t &count)
{
	int rc;
	sqlite3_stmt * stmt = NULL;
	const char * tail = NULL;
	const char * query = "select count (*) from oid_catalog oc "
	// "where (oc.txn_id = ? or oc.active = 1) "
	// "and oc.run_id = ? "
	"where oc.timestep_id = ? "
	"and oc.var_id = ? "
	"and (? <= oc.d0_max and ? >= oc.d0_min) "
	"and (? <= oc.d1_max and ? >= oc.d1_min) "
	"and (? <= oc.d2_max and ? >= oc.d2_min)";

	if(output_queries) {
		printf("query: select count (*) from oid_catalog oc where oc.timestep_id = %llu "
		"and oc.var_id = %llu "
		"and (%llu <= oc.d0_max and %llu >= oc.d0_min) "
		"and (%llu <= oc.d1_max and %llu >= oc.d1_min) "
		"and (%llu <= oc.d2_max and %llu >= oc.d2_min) \n", args.timestep_id, args.var_id,
		args.dims[0].min, args.dims[0].max, args.dims[1].min, args.dims[1].max,
		args.dims[2].min, args.dims[2].max);
	}

	rc = sqlite3_prepare_v2 (db, query, -1, &stmt, &tail); assert (rc == SQLITE_OK);
	if (rc != SQLITE_OK)
	{
		fprintf (stderr, "Error get all attrs by dim: Line: %d SQL error (%d): %s\n", __LINE__, rc, sqlite3_errmsg (db));
		sqlite3_close (db);
		return rc;
	}

	// rc = sqlite3_bind_int64 (stmt, 1, args.txn_id); assert (rc == SQLITE_OK);
	// rc = sqlite3_bind_int64 (stmt, 2, args.run_id); assert (rc == SQLITE_OK);
	rc = sqlite3_bind_int64 (stmt, 1, args.timestep_id); assert (rc == SQLITE_OK);
	rc = sqlite3_bind_int64 (stmt, 2, args.var_id); assert (rc == SQLITE_OK);

	for (int j = 0; j < 3; j++)
	{
		rc = sqlite3_bind_int64 (stmt, 3 + (j * 2), args.dims.at(j).min);
		rc = sqlite3_bind_int64 (stmt, 4 + (j * 2), args.dims.at(j).max);
	}
	rc = sqlite3_step (stmt); assert (rc == SQLITE_ROW || rc == SQLITE_DONE);

	count = sqlite3_column_int64 (stmt, 0);
	rc = sqlite3_finalize (stmt); assert (rc == SQLITE_OK);
 	
	return rc;
}


static void get_obj_lengths(uint64_t &x_width, uint64_t &last_x_width, uint64_t ndx, uint64_t chunk_size) 
{

    uint64_t ceph_obj_size = 8000000; //todo 

    debug_log << "chunk size: " << chunk_size << endl;
    uint32_t num_objs_per_chunk = round(chunk_size / ceph_obj_size);
    debug_log << "num_objs_per_chunk: " << num_objs_per_chunk << endl;
    if(num_objs_per_chunk <= 0) {
        num_objs_per_chunk = 1;
    }
    extreme_debug_log << "num_objs_per_chunk: " << num_objs_per_chunk << endl;
    x_width = round(ndx / num_objs_per_chunk);
    if(x_width <= 0) {
        x_width = 1;
    }
    extreme_debug_log << "x_width: " << x_width << endl;
    debug_log << "ndx: " << ndx << endl;
    debug_log << "x_width * (num_objs_per_chunk-1): " << ndx - x_width * (num_objs_per_chunk-1) << endl;
    if (ndx < x_width * (num_objs_per_chunk-1)) {
       	num_objs_per_chunk = ndx / x_width;
    	if (ndx % x_width == 0) {
    		last_x_width = x_width;
    	}
    	else {
    		last_x_width = x_width + (ndx % x_width);
    	}
    	// cout << "last_x_width: " << ndx - x_width * (num_objs_per_chunk-1) << endl;
     //    num_objs_per_chunk = num_objs_per_chunk + floor( (ndx - x_width * (num_objs_per_chunk-1)-1) / x_width);
        debug_log << "new num_objs_per_chunk: " << num_objs_per_chunk << endl;    
     //    last_x_width = ndx - x_width * (num_objs_per_chunk-1);
      	debug_log << "new last_x_width: " << last_x_width << endl;
      

    }
    else {
     	last_x_width = ndx - x_width * (num_objs_per_chunk-1);
    }
    // if(last_x_width <= 0) {
    // 	cout << "last_x_width: " << last_x_width << endl;
    //     num_objs_per_chunk = num_objs_per_chunk + floor( (last_x_width-1) / x_width);
    //     cout << "new num_objs_per_chunk: " << num_objs_per_chunk << endl;    
    //     last_x_width = ndx - x_width * (num_objs_per_chunk-1);
    //   	cout << "new last_x_width: " << last_x_width << endl;
      
    // }
}





/////////////


static int callBoundingBoxToObjNamesAndCounts(const md_catalog_run_entry &run, const md_catalog_var_entry &var, const vector<md_dim_bounds> &bounding_box,
                                     bool get_counts) {
    string errmsg;
    int rc;

    uint64_t ndx = (var.dims[0].max - var.dims[0].min + 1) / run.npx; //ndx = nx / npx
    uint64_t ndy = 0;
    uint64_t ndz = 0;

    // extreme_debug_log << "ndx: " << ndx << endl;
    // extreme_debug_log << "var.num_dims: " << var.num_dims << " var.dims.size(): " << var.dims.size() << endl;

    if (2 <= var.num_dims) {
        ndy = (var.dims[1].max - var.dims[1].min + 1) / run.npy; //ndy = ny / npy
        // extreme_debug_log << "ndy: " << ndy << endl;
    }
    if (3 <= var.num_dims) {
        ndz = (var.dims[2].max - var.dims[2].min + 1) / run.npz; //ndz = nz / npz
        // extreme_debug_log << "ndz: " << ndz << endl;
    }
    uint64_t ceph_obj_size = 8000000; //todo 


    // printf("Bounding box: (%d, %d, %d),(%d, %d, %d)\n",bounding_box[0].min,bounding_box[1].min,bounding_box[2].min,bounding_box[0].max,bounding_box[1].max,bounding_box[2].max);

    lua_getglobal(L, "boundingBoxToObjectNamesAndCounts");


    extreme_debug_log << "var.name.c_str(): " << var.name.c_str() << endl;

    lua_pushboolean(L, get_counts);
    lua_pushinteger(L, run.job_id);    
    lua_pushstring(L, run.name.c_str());
    lua_pushinteger(L, var.timestep_id);
    lua_pushinteger(L, ndx); 
    lua_pushinteger(L, ndy); 
    lua_pushinteger(L, ndz); 
    lua_pushinteger(L, ceph_obj_size);

    lua_pushstring(L, var.name.c_str());
    lua_pushinteger(L, var.version);
    lua_pushinteger(L, var.data_size);
    lua_pushinteger(L, bounding_box[0].min); 
    lua_pushinteger(L, bounding_box[1].min);
    lua_pushinteger(L, bounding_box[2].min);
    lua_pushinteger(L, bounding_box[0].max);
    lua_pushinteger(L, bounding_box[1].max);
    lua_pushinteger(L, bounding_box[2].max);

    // for(int i =0; i<var.num_dims; i++) {
    //     lua_pushinteger(L, var.dims[i].min);  
    //     lua_pushinteger(L, var.dims[i].max);    
    // }

    // if(lua_pcall(L, 15 + 2*var.num_dims, 1, 0)) {
    //     error_log << "errmsg: " << errmsg << endl; 
    //     return RC_ERR;    
    // }

    for(int i =0; i<var.num_dims; i++) {
        lua_pushinteger(L, var.dims[i].min);  
        lua_pushinteger(L, var.dims[i].max);    
    }
    for(int i = var.num_dims; i<3; i++) {
        lua_pushinteger(L, 0);  
        lua_pushinteger(L, 0);    
    }
    // if(lua_pcall(L, 15 + 2*var.num_dims, 1, 0)) {
    //     error_log << "errmsg: " << errmsg << endl; 
    //     return RC_ERR;    
    // }
    if(lua_pcall(L, 23, 2, 0)) {
        errmsg = lua_tostring(L, -1);
        error_log << "errmsg: " << errmsg << endl; 
        return RC_ERR;    
    }

    // testing_log << "at the end of callBoundingBoxToObjNamesAndCounts, the lua stack size is: " << lua_gettop(L) << endl;

    return RC_OK;
}



// int boundingBoxToObjNames(const md_catalog_run_entry &run, const string &objector_funct_name, const md_catalog_var_entry &var, const vector<md_dim_bounds> &bounding_box,
//                                      vector<string> &obj_names) {
int boundingBoxToObjNames(const md_catalog_run_entry &run, const md_catalog_var_entry &var, const vector<md_dim_bounds> &bounding_box,
                                     vector<string> &obj_names) {
    int rc;

    // note - if you stop registering functs right after creating them, you will need to add this back in
    // rc = register_objector_funct_write (objector_funct_name, run.job_id);
    // if (rc != RC_OK) {
    //     error_log << "error with register_objector_funct_write \n";
    //     return rc;
    // } 

    bool get_counts = false;
    rc = callBoundingBoxToObjNamesAndCounts(run, var, bounding_box, get_counts);
      if (rc != RC_OK) {
        error_log << "error with callBoundingBoxToObjNames \n";
        return rc;
    } 

    uint32_t obj_count = 0;

    if (lua_isnumber(L, -1)) {
    	obj_count = (uint32_t)lua_tonumber(L, -1);
        lua_pop(L, 1);
        extreme_debug_log << "obj_count: " << obj_count << endl;
    }
    else {
    	error_log << "error. was expecting a number on top \n";
    	return RC_ERR;
    }
    obj_names.reserve(obj_count);

    if (lua_istable(L, -1)) {
        lua_pushnil(L); // first key
        debug_log << "Matching object names: \n";
        while (lua_next(L, -2) != 0) {  
            string obj_name = (string)lua_tostring(L, -1);
            debug_log << obj_name << endl;
            obj_names.push_back(obj_name);
            lua_pop(L, 1);
        }
        lua_pop(L, 1); //pop the nil back off the stack
        debug_log << "End of matching object names \n";
    }


    // testing_log << "at the end of boundingBoxToObjNames, the lua stack size is: " << lua_gettop(L) << endl;

    return RC_OK;
}


int boundingBoxToObjNamesAndCounts(const md_catalog_run_entry &run, const md_catalog_var_entry &var, const vector<md_dim_bounds> &bounding_box,
                                     vector<string> &obj_names, vector<uint64_t> &offsets_and_counts) {

    int rc;


    bool get_counts = true;
    rc = callBoundingBoxToObjNamesAndCounts(run, var, bounding_box, get_counts);
    if (rc != RC_OK) {
        error_log << "error with callBoundingBoxToObjNamesAndCounts \n";
        return rc;
    }

    extreme_debug_log << "var.num_dims: " << var.num_dims << endl;

    uint32_t obj_count = 0;

    if (lua_isnumber(L, -1)) {
    	obj_count = (uint32_t)lua_tonumber(L, -1);
        lua_pop(L, 1);
        extreme_debug_log << "obj_count: " << obj_count << endl;
    }
    else {
    	error_log << "error. was expecting a number on top \n";
    	return RC_ERR;
    }
    obj_names.reserve(obj_count);
    offsets_and_counts.reserve(obj_count*6);

    if (lua_istable(L, -1)) {
        lua_pushnil(L); // first key
        testing_log << "Matching object names: \n";
        while (lua_next(L, -2) != 0) {  
            for(int i=0; i< (2*var.num_dims); i++) { 
              uint32_t temp = lua_tonumber(L, -1);
              extreme_debug_log << "temp_new: " << temp << endl;
              offsets_and_counts.push_back(temp);
              lua_pop(L, 1);
              lua_next(L, -2);
            }

            string obj_name = (string)lua_tostring(L, -1);
            obj_names.push_back(obj_name);
            lua_pop(L, 1);
            // testing_log << "obj_name: " << obj_name << endl;
            // extreme_debug_log << "obj name: " << obj_name << " and we want to start at (" << x_start_indx << "," << y_start_indx << "," << z_start_indx << ") and to retrieve ";
            // extreme_debug_log << "counts: (" << x_count << "," << y_count << "," << z_count << ")" << endl;
        }
        lua_pop(L, 1); //pop the nil back off the stack
        testing_log << "End of matching object names \n";
    }

    // testing_log << "at the end of boundingBoxToObjNamesAndCounts, the lua stack size is: " << lua_gettop(L) << endl;

    return RC_OK;
}

static int sql_output_db(sqlite3 *pInMemory, uint64_t job_id, int rank)
{
    int rc;
    sqlite3 *pFile;           /* Database connection  */
    sqlite3_backup *pBackup;  /* Backup object used to copy data */

    string filename = to_string(job_id) + "_" + to_string(rank);
    /* Open the database file. Exit early if this fails
      ** for any reason. */
    rc = sqlite3_open(filename.c_str(), &pFile);
    if( rc==SQLITE_OK ){  
        /* Set up the backup procedure to copy from the the main database of connection pInMemory
        to the "main" database of the connection pFile.
        ** If something goes wrong, pBackup will be set to NULL and an error
        ** code and message left in connection pFile.
        */
        // pBackup = sqlite3_backup_init(pTo, "main", pFrom, "main");
        pBackup = sqlite3_backup_init(pFile, "main", pInMemory, "main");


        /* If the backup object is successfully created, call backup_step()
        ** to copy data from pInMemory to pFile . Then call backup_finish()
        ** to release resources associated with the pBackup object.  If an
        ** error occurred, then an error code and message will be left in
        ** connection pTo. If no error occurred, then the error code belonging
        ** to pFile is set to SQLITE_OK.
        */
        if( pBackup ){
          (void)sqlite3_backup_step(pBackup, -1);
          (void)sqlite3_backup_finish(pBackup);
        }
        // rc = sqlite3_errcode(pTo);
        rc = sqlite3_errcode(pFile);
    }

    /* Close the database connection opened on database file zFilename
    ** and return the result of this function. */
    (void)sqlite3_close(pFile);
    return rc;

}
