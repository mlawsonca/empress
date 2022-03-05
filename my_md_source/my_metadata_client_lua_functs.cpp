
#include <string>
#include <my_metadata_client.h> //note: this is just for the logging functions
#include <my_metadata_client_lua_functs.h>

extern lua_State *L;

using namespace std; 


static bool debug_logging = false;
static bool testing_logging = false;
static bool extreme_debug_logging = false;
static bool zero_rank_logging = false;
static bool error_logging  = true;

static debugLog testing_log = debugLog(testing_logging, zero_rank_logging);
static debugLog debug_log = debugLog(debug_logging, false);
static debugLog error_log = debugLog(error_logging, zero_rank_logging);
static debugLog extreme_debug_log = debugLog(extreme_debug_logging, false);


int stringify_function(const std::string &path_to_function, const std::string &funct_name, string &code_string) {
//stringify the function
    // extreme_debug_log << "path to funct: " << path_to_function.c_str() << endl;
    if (luaL_loadfile(L, path_to_function.c_str()) || lua_pcall(L, 0, 0, 0)) {
        error_log << "cannot load the given lua function path: " << lua_tostring(L, -1) << "\n";
        return RC_ERR;
    }

    luaL_loadstring(L, "return string.dump(...)"); 
    lua_getglobal(L, funct_name.c_str()); 
    if (!lua_isfunction(L, -1)) {
        error_log << "couldn't load " << funct_name << " function in stringify" << endl;
        return RC_ERR;
    }

    if(lua_pcall(L, 1, 1, 0)) {
        string errmsg = lua_tostring(L, -1);
        error_log << "errmsg: " << errmsg << endl;     
    }
    size_t length = lua_strlen(L, -1);
    code_string.assign(lua_tostring(L, -1), length);

    lua_pop(L, 1); //pop the code string off the stack

    // testing_log << "at the end of stringify_function, the lua stack size is: " << lua_gettop(L) << endl;
    return RC_OK;
}

int register_objector_funct_write (const std::string &funct_name, const std::string &path_to_function, uint64_t job_id) {
    // testing_log << "at the start of register_objector_funct_write, the lua stack size is: " << lua_gettop(L) << endl;


    string registry_name = to_string(job_id) + "o";

    //todo - do I really need to check this if I make sure to only register once?
    lua_getglobal(L, registry_name.c_str());
    if (!lua_isfunction(L, -1)) {
        extreme_debug_log << registry_name << " not found in registry for write" << endl;
        if (luaL_loadfile(L, path_to_function.c_str()) || lua_pcall(L, 0, 0, 0)) {
            error_log << "cannot load the given lua function path: " << lua_tostring(L, -1) << "\n";
            return RC_ERR;
        }
        lua_getglobal(L, funct_name.c_str()); 
        if (!lua_isfunction(L, -1)) {
            error_log << "error. couldn't load " << funct_name << " function in write" << endl;
            return RC_ERR;
        }
        lua_setglobal(L, registry_name.c_str()); 
        extreme_debug_log << "just registered in write " << registry_name << endl;
    } 
    else {
        extreme_debug_log << "already registered in write " << registry_name << endl;        
    }

    lua_pop(L, 1); //pop the return (function or nil) from getglobal off the stack
    // testing_log << "at the end of register_objector_funct_write, the lua stack size is: " << lua_gettop(L) << endl;
    return RC_OK;
}

static int register_and_load_objector_funct_read (const std::string &objector_funct, uint64_t job_id) {

    // testing_log << "at the start of register_and_load_objector_funct_read, the lua stack size is: " << lua_gettop(L) << endl;

    string registry_name = to_string(job_id) + "o";

    //todo - do I really need to check this if I make sure to only register once?
    lua_getglobal(L, registry_name.c_str());
    if (!lua_isfunction(L, -1)) {
        lua_pop(L, 1); //pop the returned nil off the stack

	    if(objector_funct.size() == 0) {
	        error_log << "register_and_load_objector_funct_read called before objector_funct has been assigned" << endl;
	        return RC_ERR;
	    }

        // testing_log << "before loading buffer in register_and_load_objector_funct_read, the lua stack size is: " << lua_gettop(L) << endl;
        if(luaL_loadbuffer(L, objector_funct.c_str(), objector_funct.size(), "funct")) {
            error_log << "errmsg: " << lua_tostring(L, -1) << endl;
            return RC_ERR;
        }
        // testing_log << "after loading buffer in register_and_load_objector_funct_read, the lua stack size is: " << lua_gettop(L) << endl;
        lua_setglobal(L, registry_name.c_str()); 
        // testing_log << "after set global in register_and_load_objector_funct_read, the lua stack size is: " << lua_gettop(L) << endl; 
        lua_getglobal(L, registry_name.c_str());  
        extreme_debug_log << "just registered in read " << registry_name << endl;
    }
    else {
        extreme_debug_log << "already registered in read " << registry_name << endl;
    }

    // testing_log << "at the end of register_and_load_objector_funct_read, the lua stack size is: " << lua_gettop(L) << endl;

    return RC_OK;
}

static int register_and_load_rank_to_bounding_box_funct (const md_catalog_run_entry &run) {
    string registry_name = to_string(run.job_id);

    //todo - do I really need to check this if I make sure to only register once?
    lua_getglobal(L, registry_name.c_str());
    if (!lua_isfunction(L, -1)) {
        lua_pop(L, 1); //pop the returned nil off the stack

	    if(run.rank_to_dims_funct.size() == 0) {
	        error_log << "register_and_load_rank_to_bounding_box_funct called before rank_to_dims_funct has been assigned" << endl;
	        return RC_ERR;
	    }

        if(luaL_loadbuffer(L, run.rank_to_dims_funct.c_str(), run.rank_to_dims_funct.size(), "funct")) {
            error_log << "errmsg: " << lua_tostring(L, -1) << endl;
            return RC_ERR;
        }
        lua_setglobal(L, registry_name.c_str());  
        lua_getglobal(L, registry_name.c_str());    
        extreme_debug_log << "just registered in read " << registry_name << endl;
        // lua_getglobal(L, registry_name.c_str());    
    }
    else {
        extreme_debug_log << "already registered in read " << registry_name << endl;
    }
    return RC_OK;
}


int rankToBoundingBox(const md_catalog_run_entry &run, const md_catalog_var_entry &var, 
                                  int rank, vector<md_dim_bounds> &bounding_box) {
    string errmsg;
    int rc;

    uint64_t ndx = (var.dims[0].max - var.dims[0].min + 1) / run.npx; //ndx = nx / npx
    uint64_t ndy = 0;
    uint64_t ndz = 0;

    if (2 <= var.num_dims) {
        ndy = (var.dims[1].max - var.dims[1].min + 1) / run.npy; //ndy = ny / npy
    }
    if (3 <= var.num_dims) {
        ndz = (var.dims[2].max - var.dims[2].min + 1) / run.npz; //ndz = nz / npz
    }
    rc = register_and_load_rank_to_bounding_box_funct(run);
    if (rc != RC_OK) {
        error_log << "error in register_and_load_rank_to_bounding_box_funct \n";
    }

    uint64_t npy = 1;
    uint64_t npz = 1;
    if (var.num_dims >= 2) {
        npy = run.npy;
        // extreme_debug_log << "nx: " << var.dims[0].max - var.dims[0].min + 1 << " ny: " << var.dims[1].max - var.dims[1].min + 1 << endl;
    }
    if (var.num_dims == 3) {
        npz = run.npz;
        // extreme_debug_log << "nx: " << var.dims[0].max - var.dims[0].min + 1 << " ny: " << var.dims[1].max - var.dims[1].min + 1 << " nz: " << var.dims[2].max - var.dims[2].min + 1 << endl;   
    }
    lua_pushinteger(L, run.npx);
    lua_pushinteger(L, npy); 
    lua_pushinteger(L, npz); 
    lua_pushinteger(L, ndx); 
    lua_pushinteger(L, ndy); 
    lua_pushinteger(L, ndz); 
    lua_pushinteger(L, rank);

    extreme_debug_log << "rank: " << rank << " npx: " << run.npx << " npy: " << npy << " npz: " << npz << endl;
    extreme_debug_log << "ndx: " << ndx << " ndy: " << ndy << " ndz: " << ndz << endl;

    if(lua_pcall(L, 7, 6, 0)) { 
        errmsg = lua_tostring(L, -1);
        error_log << "errmsg: " << errmsg << endl;
        return RC_ERR;
     
    }
    else {
        // bounding_box.reserve(var.num_dims);
        // bounding_box.reserve(3);

        for(int i = 0; i < 3; i++) {
        // for(int i = 0; i < var.num_dims; i++) {
            bounding_box[i].min = lua_tonumber(L, -1); //should return 0s for any dimension not used
            lua_pop(L, 1);
            bounding_box[i].max = lua_tonumber(L, -1);
            lua_pop(L, 1);

            // md_dim_bounds dims;
            // dims.min = lua_tonumber(L, -1);//should return 0s for any dimension not used
            // lua_pop(L, 1);
            // dims.max = lua_tonumber(L, -1);
            // lua_pop(L, 1);
            // bounding_box.push_back(dims); 
        }

        // for(int j = var.num_dims; j < 3; j++) { //pop off the 0s for any dimension we don't need
        //     lua_pop(L, 1);
        //     lua_pop(L, 1);
        //     bounding_box[j].min = 0; //should return 0s for any dimension not used
        //     bounding_box[j].max = 0; //should return 0s for any dimension not used
        // }
        if(extreme_debug_logging) {
            printf("Rank: %d, Bounding box: (%d, %d, %d),(%d, %d, %d)\n",rank,bounding_box[0].min,bounding_box[1].min,bounding_box[2].min,
                bounding_box[0].max,bounding_box[1].max,bounding_box[2].max);
        }

    } 

    // testing_log << "at the end of rankToBoundingBox, the lua stack size is: " << lua_gettop(L) << endl;
    return RC_OK;
}


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

    string registry_name = to_string(run.job_id) + "o";
    extreme_debug_log << "in callBoundingBoxToObjNamesAndCounts registry name: " << registry_name << endl;
    
    if (get_counts) {
        rc = register_and_load_objector_funct_read (run.objector_funct, run.job_id);
        if (rc != RC_OK) {
            error_log << "error with register_and_load_objector_funct_read \n";
            return rc;
        } 
    }
    else {
        lua_getglobal(L, registry_name.c_str());
        if (!lua_isfunction(L, -1)) {
            error_log << "error couldn't load objector function for " << registry_name << " by callBoundingBoxToObjNamesAndCounts in write \n";            
            return RC_ERR;
        }
    }


    
    extreme_debug_log << "get_counts: " << get_counts << " job_id: " << run.job_id << " run name: " << run.name << " timestep_id: " << var.timestep_id << endl;
    extreme_debug_log << "ndx: " << ndx << " ndy: " << ndy << " ndz: " << ndz << endl;
    extreme_debug_log << "ceph_obj_size: " << ceph_obj_size << " var name: " << var.name << " var version: " << var.version << " var.data_size: " << var.data_size << endl;
    extreme_debug_log << "bounding_box.size(): " << bounding_box.size() << endl;
    if(extreme_debug_logging) {
        printf("Bounding box: (%d, %d, %d),(%d, %d, %d)\n",bounding_box[0].min,bounding_box[1].min,bounding_box[2].min,bounding_box[0].max,bounding_box[1].max,bounding_box[2].max);
    }

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

    // cout << "got here" << endl;
    // cout << "bounding_box.size(): " << bounding_box.size() << endl;
    // cout << "got here 2" << endl;

    // int i = 0;
    // while(i < var.num_dims) {
    //     lua_pushinteger(L, bounding_box[i].min);
    //     i++; 
    // }
    // while(i < 3) {
    //     lua_pushinteger(L, 0);
    //     i++;   
    // }

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
    if(lua_pcall(L, 23, 1, 0)) {
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
            testing_log << "obj_name: " << obj_name << endl;
            // extreme_debug_log << "obj name: " << obj_name << " and we want to start at (" << x_start_indx << "," << y_start_indx << "," << z_start_indx << ") and to retrieve ";
            // extreme_debug_log << "counts: (" << x_count << "," << y_count << "," << z_count << ")" << endl;
        }
        lua_pop(L, 1); //pop the nil back off the stack
        testing_log << "End of matching object names \n";
    }

    // testing_log << "at the end of boundingBoxToObjNamesAndCounts, the lua stack size is: " << lua_gettop(L) << endl;

    return RC_OK;
}


