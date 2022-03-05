
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

#include <my_metadata_client_sqlite_transactions_only_multiple_databases_read_and_write_new.hh>
#include <my_metadata_client_lua_functs.h>

#include <md_client_timing_constants.hh>

#include <libops.hh>
#include "dirman/DirMan.hh"

#include <OpCoreClient.hh>


using namespace std;
using namespace net;

uint64_t my_job_id = 0;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//JUST FOR DEBUGGING/TESTING ////////////////////////////////////////////////////////////////////////////////////////////
// static bool extreme_debug_logging = false;
// static bool debug_logging = false;
static bool error_logging = true;
static debugLog error_log = debugLog(error_logging);
// static debugLog debug_log = debugLog(debug_logging);
// static debugLog extreme_debug_log = debugLog(extreme_debug_logging);

// extern std::vector<int> catg_of_time_pts;
// extern std::vector<std::chrono::high_resolution_clock::time_point> time_pts;
extern void add_timing_point(int catg);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

lua_State *L;
// md_query_type query_type = COMMITTED; //default to reading only committed metadata
md_query_type query_type = COMMITTED_AND_UNCOMMITTED; //default to reading only committed metadata
// uint32_t MAX_EAGER_MSG_SIZE = 8192;
// uint32_t MAX_EAGER_MSG_SIZE = 4096;
// uint32_t MAX_EAGER_MSG_SIZE = 2048;
// uint32_t MAX_EAGER_MSG_SIZE = 1024;
uint32_t MAX_EAGER_MSG_SIZE;

//used to keep track of name + version -> var id (so that varids will be consisten for a given name/version across timesteps)
// map <string, uint64_t> var_ids;

extern void print_var_attr_data(md_catalog_var_attribute_entry);
extern void print_timestep_attr_data(md_catalog_timestep_attribute_entry);
extern void print_run_attr_data(md_catalog_run_attribute_entry);

template <class T>
void deArchiveMsgFromServer(const string &incoming_msg,  std::vector<T> &entries,
                            uint32_t &count, int &return_value);


void deArchiveMsgFromServer(const std::string &serial_str,
                                             uint64_t &id,
                                             int &return_value);

template <class T>
string serializeMsgToServer(const vector<T> &args);
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
            " version: " << var.version << " data_size: " << var.data_size <<  
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
        type.version <<   endl;
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
    //         " type id: " << attr.type_id <<  
    //         << " data_type: " << (int)attr.data_type << endl;
    // }
        md_catalog_run_attribute_entry attr = entries [i];
        std::cout << "type id: " << attr.type_id << " ";
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
    //         " type id: " << attr.type_id << 
    //         " data_type: " << (int)attr.data_type << endl;
    // }
        md_catalog_timestep_attribute_entry attr = entries [i];
        std::cout << "timestep_id: " << attr.timestep_id << 
            " type id: " << attr.type_id << " ";
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
            " type_id: " << attr.type_id << " num_dims: " << attr.num_dims;
        // std::cout << "attribute id: " << attr.attribute_id << " timestep_id: " << attr.timestep_id << " var id: " << attr.var_id <<
        //     " type id: " << attr.type_id << " num_dims: " << attr.num_dims;
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
            " npx: " << run.npx << " npy: " << run.npy <<
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
        
        std::cout << "timestep_id: " << timestep.timestep_id << " run_id: " << timestep.run_id << " path: " << timestep.path << endl; 
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

// ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


//registers all available ops
//reminder: you get a performance boost if you register the ops before launching opbox/gutties
static int register_ops () {
    opbox::RegisterOp<OpCommitTransactionMeta>();

    opbox::RegisterOp<OpCheckpointDatabaseMeta>();
    opbox::RegisterOp<OpInsertTimestepAttributeBatchMeta>();
    opbox::RegisterOp<OpInsertRunAttributeBatchMeta>();
    opbox::RegisterOp<OpCreateVarBatchMeta>();
    opbox::RegisterOp<OpCreateTypeBatchMeta>();
    opbox::RegisterOp<OpInsertVarAttributeByDimsBatchMeta>();
    opbox::RegisterOp<OpDeleteAllVarsWithSubstrMeta>();
    opbox::RegisterOp<OpCatalogAllVarAttributesWithVarSubstrDimsMeta>();
    opbox::RegisterOp<OpCatalogAllVarAttributesWithTypeVarSubstrDimsMeta>();
    opbox::RegisterOp<OpCatalogAllVarAttributesWithTypeVarSubstrMeta>();
    opbox::RegisterOp<OpCatalogAllVarAttributesWithTypeVarSubstrRangeMeta>();
    opbox::RegisterOp<OpCatalogAllVarAttributesWithTypeVarSubstrDimsRangeMeta>();
    opbox::RegisterOp<OpCatalogAllTypesWithVarAttributesWithVarSubstrInTimestepMeta>();
    opbox::RegisterOp<OpCatalogAllTypesWithVarAttributesWithVarSubstrDimsInTimestepMeta>();
    opbox::RegisterOp<OpCatalogAllTimestepsWithVarSubstrMeta>();
    opbox::RegisterOp<OpCatalogAllTimestepsWithVarAttributesWithTypeVarSubstrRangeMeta>();
    opbox::RegisterOp<OpCatalogAllTimestepsWithVarAttributesWithTypeVarSubstrMeta>();
    opbox::RegisterOp<OpCatalogAllTimestepsWithVarAttributesWithTypeVarSubstrDimsRangeMeta>();
    opbox::RegisterOp<OpCatalogAllTimestepsWithVarAttributesWithTypeVarSubstrDimsMeta>();
    opbox::RegisterOp<OpCatalogAllVarAttributesWithVarSubstrMeta>();
    opbox::RegisterOp<OpProcessingMeta>();
    // opbox::RegisterOp<OpActivateMeta>();
    opbox::RegisterOp<OpActivateRunAttributeMeta>();
    opbox::RegisterOp<OpActivateTimestepAttributeMeta>();
    opbox::RegisterOp<OpActivateTimestepMeta>();
    opbox::RegisterOp<OpActivateVarMeta>();
    opbox::RegisterOp<OpActivateRunMeta>();
    opbox::RegisterOp<OpActivateTypeMeta>();  
    opbox::RegisterOp<OpActivateVarAttributeMeta>();
    opbox::RegisterOp<OpCatalogAllVarAttributesWithTypeVarDimsRangeMeta>();
    opbox::RegisterOp<OpCatalogAllVarAttributesWithTypeVarRangeMeta>();
    opbox::RegisterOp<OpCatalogAllTimestepAttributesWithTypeRangeMeta>();
    opbox::RegisterOp<OpCatalogAllTimestepsWithVarAttributesWithTypeVarDimsRangeMeta>();
    opbox::RegisterOp<OpCatalogAllTimestepsWithVarAttributesWithTypeVarRangeMeta>();
    opbox::RegisterOp<OpCatalogAllRunAttributesWithTypeRangeMeta>();
    opbox::RegisterOp<OpCatalogAllTimestepAttributesWithTypeMeta>();
    opbox::RegisterOp<OpCatalogAllTypesWithVarAttributesWithVarDimsInTimestepMeta>();
    opbox::RegisterOp<OpCatalogAllTypesWithVarAttributesWithVarInTimestepMeta>();
    opbox::RegisterOp<OpCatalogAllTypesWithVarAttributesInTimestepMeta>();
    opbox::RegisterOp<OpCatalogAllTimestepsWithVarMeta>();
    opbox::RegisterOp<OpCatalogAllTimestepsWithVarAttributesWithTypeVarDimsMeta>();
    opbox::RegisterOp<OpCatalogAllTimestepsWithVarAttributesWithTypeVarMeta>();
    opbox::RegisterOp<OpCatalogAllRunAttributesWithTypeMeta>();
    opbox::RegisterOp<OpCatalogAllTimestepAttributesMeta>();
    opbox::RegisterOp<OpCatalogAllRunAttributesMeta>();
    opbox::RegisterOp<OpDeleteTimestepByIdMeta>();
    opbox::RegisterOp<OpCatalogTimestepMeta>();
    opbox::RegisterOp<OpInsertTimestepAttributeMeta>();
    opbox::RegisterOp<OpInsertRunAttributeMeta>();
    opbox::RegisterOp<OpCreateTimestepMeta>();
    opbox::RegisterOp<OpDeleteVarByNamePathVerMeta>();
    opbox::RegisterOp<OpDeleteVarByIdMeta>();
    opbox::RegisterOp<OpDeleteTypeByNameVerMeta>();
    opbox::RegisterOp<OpDeleteTypeByIdMeta>();
    opbox::RegisterOp<OpDeleteRunByIdMeta>();
    opbox::RegisterOp<OpCatalogAllVarAttributesWithVarDimsByNameVerMeta>();
    opbox::RegisterOp<OpCatalogAllVarAttributesWithVarDimsByIdMeta>();
    opbox::RegisterOp<OpCatalogAllVarAttributesWithVarByNameVerMeta>();
    opbox::RegisterOp<OpCatalogAllVarAttributesWithVarByIdMeta>();
    opbox::RegisterOp<OpCatalogAllVarAttributesWithTypeVarDimsByNameVerMeta>();
    opbox::RegisterOp<OpCatalogAllVarAttributesWithTypeVarDimsByIdMeta>();
    opbox::RegisterOp<OpCatalogAllVarAttributesWithTypeVarByNameVerMeta>();
    opbox::RegisterOp<OpCatalogAllVarAttributesWithTypeVarByIdMeta>();
    opbox::RegisterOp<OpCatalogAllVarAttributesWithTypeDimsByNameVerMeta>();
    opbox::RegisterOp<OpCatalogAllVarAttributesWithTypeDimsByIdMeta>();
    opbox::RegisterOp<OpCatalogAllVarAttributesWithTypeByNameVerMeta>();
    opbox::RegisterOp<OpCatalogAllVarAttributesWithTypeByIdMeta>();
    opbox::RegisterOp<OpCatalogAllVarAttributesWithDimsMeta>();
    opbox::RegisterOp<OpCatalogAllVarAttributesMeta>(); 
    opbox::RegisterOp<OpCatalogRunMeta>();
    opbox::RegisterOp<OpCatalogTypeMeta>(); 
    opbox::RegisterOp<OpCatalogVarMeta>();
    opbox::RegisterOp<OpCreateRunMeta>();
    opbox::RegisterOp<OpCreateTypeMeta>();
    opbox::RegisterOp<OpCreateVarMeta>();
    opbox::RegisterOp<OpFullShutdownMeta>();  
    opbox::RegisterOp<OpInsertVarAttributeByDimsMeta>();


return RC_OK;
}


//registers all ops needed for writing
//reminder: you get a performance boost if you register the ops before launching opbox/gutties
static int register_ops_write () {
    // opbox::RegisterOp<OpActivateMeta>();
    opbox::RegisterOp<OpActivateRunAttributeMeta>();
    opbox::RegisterOp<OpActivateTimestepAttributeMeta>();
    opbox::RegisterOp<OpActivateTimestepMeta>();
    opbox::RegisterOp<OpActivateVarMeta>();
    opbox::RegisterOp<OpActivateRunMeta>();
    opbox::RegisterOp<OpActivateTypeMeta>();  
    opbox::RegisterOp<OpActivateVarAttributeMeta>();
    opbox::RegisterOp<OpCreateRunMeta>();
    opbox::RegisterOp<OpCreateTimestepMeta>();
    opbox::RegisterOp<OpCreateTypeMeta>();
    opbox::RegisterOp<OpCreateVarMeta>();
    opbox::RegisterOp<OpInsertRunAttributeMeta>();
    opbox::RegisterOp<OpInsertTimestepAttributeMeta>();
    // opbox::RegisterOp<OpFullShutdownMeta>();  
    opbox::RegisterOp<OpInsertVarAttributeByDimsMeta>();


return RC_OK;
}

// //registers all ops for reading
// //reminder: you get a performance boost if you register the ops before launching opbox/gutties
// static int register_ops_read () {
//  opbox::RegisterOp<OpCatalogAllVarAttributesWithVarSubstrDimsMeta>();
//  opbox::RegisterOp<OpCatalogAllVarAttributesWithTypeVarSubstrDimsMeta>();
//  opbox::RegisterOp<OpCatalogAllVarAttributesWithTypeVarSubstrMeta>();
//  opbox::RegisterOp<OpCatalogAllVarAttributesWithTypeVarSubstrRangeMeta>();
//  opbox::RegisterOp<OpCatalogAllVarAttributesWithTypeVarSubstrDimsRangeMeta>();
//  opbox::RegisterOp<OpCatalogAllTypesWithVarAttributesWithVarSubstrInTimestepMeta>();
//  opbox::RegisterOp<OpCatalogAllTypesWithVarAttributesWithVarSubstrDimsInTimestepMeta>();
//  opbox::RegisterOp<OpCatalogAllTimestepsWithVarSubstrMeta>();
//  opbox::RegisterOp<OpCatalogAllTimestepsWithVarAttributesWithTypeVarSubstrRangeMeta>();
//  opbox::RegisterOp<OpCatalogAllTimestepsWithVarAttributesWithTypeVarSubstrMeta>();
//  opbox::RegisterOp<OpCatalogAllTimestepsWithVarAttributesWithTypeVarSubstrDimsRangeMeta>();
//  opbox::RegisterOp<OpCatalogAllTimestepsWithVarAttributesWithTypeVarSubstrDimsMeta>();
//  opbox::RegisterOp<OpCatalogAllVarAttributesWithVarSubstrMeta>();
//  opbox::RegisterOp<OpCatalogAllVarAttributesWithTypeVarDimsRangeMeta>();
//  opbox::RegisterOp<OpCatalogAllVarAttributesWithTypeVarRangeMeta>();
//  opbox::RegisterOp<OpCatalogAllTimestepAttributesWithTypeRangeMeta>();
//  opbox::RegisterOp<OpCatalogAllTimestepsWithVarAttributesWithTypeVarDimsRangeMeta>();
//  opbox::RegisterOp<OpCatalogAllTimestepsWithVarAttributesWithTypeVarRangeMeta>();
//  opbox::RegisterOp<OpCatalogAllRunAttributesWithTypeRangeMeta>();
//  opbox::RegisterOp<OpCatalogAllTimestepAttributesWithTypeMeta>();
//  opbox::RegisterOp<OpCatalogAllTypesWithVarAttributesWithVarDimsInTimestepMeta>();
//  opbox::RegisterOp<OpCatalogAllTypesWithVarAttributesWithVarInTimestepMeta>();
//  opbox::RegisterOp<OpCatalogAllTypesWithVarAttributesInTimestepMeta>();
//  opbox::RegisterOp<OpCatalogAllTimestepsWithVarMeta>();
//  opbox::RegisterOp<OpCatalogAllTimestepsWithVarAttributesWithTypeVarDimsMeta>();
//  opbox::RegisterOp<OpCatalogAllTimestepsWithVarAttributesWithTypeVarMeta>();
//  opbox::RegisterOp<OpCatalogAllRunAttributesWithTypeMeta>();
//  opbox::RegisterOp<OpCatalogAllTimestepAttributesMeta>();
//  opbox::RegisterOp<OpCatalogAllRunAttributesMeta>();
//  opbox::RegisterOp<OpCatalogTimestepMeta>();
//  opbox::RegisterOp<OpCatalogAllVarAttributesWithVarDimsByNameVerMeta>();
//  opbox::RegisterOp<OpCatalogAllVarAttributesWithVarDimsByIdMeta>();
//  opbox::RegisterOp<OpCatalogAllVarAttributesWithVarByNameVerMeta>();
//  opbox::RegisterOp<OpCatalogAllVarAttributesWithVarByIdMeta>();
//  opbox::RegisterOp<OpCatalogAllVarAttributesWithTypeVarDimsByNameVerMeta>();
//  opbox::RegisterOp<OpCatalogAllVarAttributesWithTypeVarDimsByIdMeta>();
//  opbox::RegisterOp<OpCatalogAllVarAttributesWithTypeVarByNameVerMeta>();
//  opbox::RegisterOp<OpCatalogAllVarAttributesWithTypeVarByIdMeta>();
//  opbox::RegisterOp<OpCatalogAllVarAttributesWithTypeDimsByNameVerMeta>();
//  opbox::RegisterOp<OpCatalogAllVarAttributesWithTypeDimsByIdMeta>();
//  opbox::RegisterOp<OpCatalogAllVarAttributesWithTypeByNameVerMeta>();
//  opbox::RegisterOp<OpCatalogAllVarAttributesWithTypeByIdMeta>();
//  opbox::RegisterOp<OpCatalogAllVarAttributesWithDimsMeta>();
//  opbox::RegisterOp<OpCatalogAllVarAttributesMeta>(); 
//  opbox::RegisterOp<OpCatalogRunMeta>();
//  opbox::RegisterOp<OpCatalogTypeMeta>(); 
//  opbox::RegisterOp<OpCatalogVarMeta>();
//  opbox::RegisterOp<OpFullShutdownMeta>();  

// return RC_OK;
// }


// static int register_ops_delete () {
//  opbox::RegisterOp<OpDeleteAllVarsWithSubstrMeta>();
//  opbox::RegisterOp<OpProcessingMeta>();
//  opbox::RegisterOp<OpDeleteTimestepByIdMeta>();
//  opbox::RegisterOp<OpDeleteVarByNamePathVerMeta>();
//  opbox::RegisterOp<OpDeleteVarByIdMeta>();
//  opbox::RegisterOp<OpDeleteTypeByNameVerMeta>();
//  opbox::RegisterOp<OpDeleteTypeByIdMeta>();
//  opbox::RegisterOp<OpDeleteRunByIdMeta>();

// return RC_OK;
// }

int metadata_init (uint64_t job_id, md_query_type my_query_type) {
    my_job_id = job_id;
    metadata_init();
    metadata_set_query_type(my_query_type);
}

int metadata_init (uint64_t job_id) {
    my_job_id = job_id;
    return metadata_init();
}

int metadata_init () {

    int rc;

    L = luaL_newstate(); //fix - do I ever open a new state or deregister the rank to bounding box functions?
    luaL_openlibs(L);   

    //todo - if we decide user wont supply the entire objector funct, will need to load this
    // if ( (luaL_loadfile(L, "../lib_source/lua/metadata_functs.lua") || lua_pcall(L, 0, 0, 0)) && (luaL_loadfile(L, "lib_source/lua/metadata_functs.lua") || lua_pcall(L, 0, 0, 0)))
     // if ( (luaL_loadfile(L, "PATH_TO_EMPRESS/lib_source/lua/metadata_functs.lua") || lua_pcall(L, 0, 0, 0)) ) {
     //    error_log << "cannot load metadata_functs_new.lua: " << lua_tostring(L, -1) << "\n";
     //    return RC_ERR;   
     // }

    rc = register_ops();

    return rc;
} 

void metadata_set_query_type(md_query_type my_query_type)
{
    query_type = my_query_type;
}


void metadata_network_init() {
    opbox::net::Attrs nnti_attrs;
    opbox::net::GetAttrs(&nnti_attrs);
    MAX_EAGER_MSG_SIZE = nnti_attrs.max_eager_size;
}


// int metadata_init_write () {
//     int rc;

//     L = luaL_newstate(); //fix - do I ever open a new state or deregister the rank to bounding box functions?
//     luaL_openlibs(L);

//     //todo - if we decide user wont supply the entire objector funct, will need to load this
//     // if ( (luaL_loadfile(L, "../lib_source/lua/metadata_functs.lua") || lua_pcall(L, 0, 0, 0)) && (luaL_loadfile(L, "lib_source/lua/metadata_functs.lua") || lua_pcall(L, 0, 0, 0)))
//      // if ( (luaL_loadfile(L, "PATH_TO_EMPRESS/lib_source/lua/metadata_functs.lua") || lua_pcall(L, 0, 0, 0)) ) {
//      //    error_log << "cannot load metadata_functs_new.lua: " << lua_tostring(L, -1) << "\n";
//      //    return RC_ERR;   
//      // }

//     rc = register_ops_write();

//     return rc;
// } 

// int metadata_init_read () {
//     int rc;

//     L = luaL_newstate(); //fix - do I ever open a new state or deregister the rank to bounding box functions?
//     luaL_openlibs(L);

//     //todo - if we decide user wont supply the entire objector funct, will need to load this
//     // if ( (luaL_loadfile(L, "../lib_source/lua/metadata_functs.lua") || lua_pcall(L, 0, 0, 0)) && (luaL_loadfile(L, "lib_source/lua/metadata_functs.lua") || lua_pcall(L, 0, 0, 0)))
//      // if ( (luaL_loadfile(L, "PATH_TO_EMPRESS/lib_source/lua/metadata_functs.lua") || lua_pcall(L, 0, 0, 0)) ) {
//      //    error_log << "cannot load metadata_functs_new.lua: " << lua_tostring(L, -1) << "\n";
//      //    return RC_ERR;   
//      // }

//     rc = register_ops_read();

//     return rc;
// } 


// int metadata_init_read_and_delete () {
//     int rc;

//     L = luaL_newstate(); //fix - do I ever open a new state or deregister the rank to bounding box functions?
//     luaL_openlibs(L);

//     //todo - if we decide user wont supply the entire objector funct, will need to load this
//     // if ( (luaL_loadfile(L, "../lib_source/lua/metadata_functs.lua") || lua_pcall(L, 0, 0, 0)) && (luaL_loadfile(L, "lib_source/lua/metadata_functs.lua") || lua_pcall(L, 0, 0, 0)))
//      // if ( (luaL_loadfile(L, "PATH_TO_EMPRESS/lib_source/lua/metadata_functs.lua") || lua_pcall(L, 0, 0, 0)) ) {
//      //    error_log << "cannot load metadata_functs_new.lua: " << lua_tostring(L, -1) << "\n";
//      //    return RC_ERR;   
//      // }

//     rc = register_ops_read();
//     if (rc != RC_OK) {
//         error_log << "Error registering the read ops" << endl;
//         return rc;
//     }

//     rc = register_ops_delete();
//     if (rc != RC_OK) {
//         error_log << "Error registering the read ops" << endl;
//     }

//     return rc;
// } 

int metadata_finalize_server (const md_server &server)
{
    add_timing_point(MD_FULL_SHUTDOWN_START);
    //debug_log << "about to finalize server" << endl;

    OpFullShutdownMeta *op = new OpFullShutdownMeta(server.peer_ptr);
    opbox::LaunchOp(op);  
    add_timing_point(MD_FULL_SHUTDOWN_DONE);
    //todo - anything else I need to do/free?

    return RC_OK;
}

void metadata_finalize_client() {
    //debug_log << "about to close lua state" << endl;

    lua_close(L); //todo - is this fine?
    //debug_log << "just closed lua state" << endl;
}





// mark all attribues from the dataset as active making them visible to other
// processes.
int metadata_activate_run_attribute (const md_server &server
                          ,uint64_t txn_id
                          // //,md_query_type query_type
                          )
{
    add_timing_point(MD_ACTIVATE_RUN_ATTRIBUTE_START);

    int return_value;
    // int res;

    md_activate_args args;

    args.job_id = my_job_id;
    // args.query_type = query_type;

    OpActivateRunAttributeMeta *op = new OpActivateRunAttributeMeta(server.peer_ptr, args);
    future<string> fut = op->GetFuture();
    opbox::LaunchOp(op);

    return_value=stoi(fut.get());  

    // deArchiveMsgFromServer(res, return_value);
    add_timing_point(MD_ACTIVATE_RUN_ATTRIBUTE_DONE);

    return return_value;
}

// mark all attribues from the dataset as active making them visible to other
// processes.
int metadata_activate_run (const md_server &server
                          ,uint64_t txn_id
                          // //,md_query_type query_type
                          )
{
    add_timing_point(MD_ACTIVATE_RUN_START);

    int return_value;
    // int res;

    md_activate_args args;

    args.job_id = my_job_id;
    // args.query_type = query_type;

    OpActivateRunMeta *op = new OpActivateRunMeta(server.peer_ptr, args);
    future<string> fut = op->GetFuture();
    opbox::LaunchOp(op);

    //extreme_debug_log << "metadata_activate_run: about to retrieve return value" << endl;

    return_value=stoi(fut.get());  
    //extreme_debug_log << "metadata_activate_run: return value: " << return_value << endl;


    // deArchiveMsgFromServer(res, return_value);
    add_timing_point(MD_ACTIVATE_RUN_DONE);

   //extreme_debug_log << "metadata_activate_run: finished" << endl;

    return return_value;
}

// mark all attribues from the dataset as active making them visible to other
// processes.
int metadata_activate_timestep_attribute (const md_server &server
                          ,uint64_t txn_id
                          // //,md_query_type query_type
                          )
{
    add_timing_point(MD_ACTIVATE_TIMESTEP_ATTRIBUTE_START);

    int return_value;
    // int res;

    md_activate_args args;

    args.job_id = my_job_id;
    // args.query_type = query_type;

    OpActivateTimestepAttributeMeta *op = new OpActivateTimestepAttributeMeta(server.peer_ptr, args);
    future<string> fut = op->GetFuture();
    opbox::LaunchOp(op);

    return_value=stoi(fut.get());  

    // deArchiveMsgFromServer(res, return_value);
    add_timing_point(MD_ACTIVATE_TIMESTEP_ATTRIBUTE_DONE);

    return return_value;
}


// mark all attribues from the dataset as active making them visible to other
// processes.
int metadata_activate_timestep (const md_server &server
                          ,uint64_t txn_id
                          // //,md_query_type query_type
                          )
{
    add_timing_point(MD_ACTIVATE_TIMESTEP_START);

    int return_value;
    // int res;

    md_activate_args args;

    args.job_id = my_job_id;
    // args.query_type = query_type;

    OpActivateTimestepMeta *op = new OpActivateTimestepMeta(server.peer_ptr, args);
    future<string> fut = op->GetFuture();
    opbox::LaunchOp(op);

    return_value=stoi(fut.get());  

    // deArchiveMsgFromServer(res, return_value);
    add_timing_point(MD_ACTIVATE_TIMESTEP_DONE);

    return return_value;
}


// mark a variable in the global_catalog as active making it visible to other
// processes.
int metadata_activate_type (const md_server &server
                          ,uint64_t txn_id
                          // //,md_query_type query_type
                          )
{
    add_timing_point(MD_ACTIVATE_TYPE_START);

    int return_value;
    // int res;

    md_activate_args args;

    args.job_id = my_job_id;
    // args.query_type = query_type;

    OpActivateTypeMeta *op = new OpActivateTypeMeta(server.peer_ptr, args);
    future<string> fut = op->GetFuture();
    opbox::LaunchOp(op);

    return_value=stoi(fut.get());   

    // deArchiveMsgFromServer(res, return_value);
    add_timing_point(MD_ACTIVATE_TYPE_DONE);   

    return return_value;
}

// mark a variable in the global_catalog as active making it visible to other
// processes.
int metadata_activate_var_attribute (const md_server &server
                          ,uint64_t txn_id
                          // //,md_query_type query_type
                          )
{
    add_timing_point(MD_ACTIVATE_VAR_ATTRIBUTE_START);

    int return_value;
    // int res;

    md_activate_args args;

    args.job_id = my_job_id;
    // args.query_type = query_type;

    OpActivateVarAttributeMeta *op = new OpActivateVarAttributeMeta(server.peer_ptr, args);
    future<string> fut = op->GetFuture();
    opbox::LaunchOp(op);

    return_value=stoi(fut.get());   

    // deArchiveMsgFromServer(res, return_value);
    add_timing_point(MD_ACTIVATE_VAR_ATTRIBUTE_DONE);   

    return return_value;
}


// mark a variable in the global_catalog as active making it visible to other
// processes.
int metadata_activate_var (const md_server &server
                          ,uint64_t txn_id
                          // //,md_query_type query_type
                          )
{
    add_timing_point(MD_ACTIVATE_VAR_START);

    int return_value;
    // int res;

    md_activate_args args;

    args.job_id = my_job_id;
    // args.query_type = query_type;

    OpActivateVarMeta *op = new OpActivateVarMeta(server.peer_ptr, args);
    future<string> fut = op->GetFuture();
    opbox::LaunchOp(op);

    return_value=stoi(fut.get());   

    // deArchiveMsgFromServer(res, return_value);
    add_timing_point(MD_ACTIVATE_VAR_DONE);   

    return return_value;
}




// // mark all attribues from the dataset as active making them visible to other
// // processes.
// int metadata_activate_run_attribute (const md_server &server
//                           ,uint64_t txn_id
//                           )
// {
//     int return_value;
    
//     return_value = metadata_activate ( server, txn_id, RUN_ATTR_CATALOG);

//     return return_value;
// } //end of funct

// // mark all attribues from the dataset as active making them visible to other
// // processes.
// int metadata_activate_run (const md_server &server
//                           ,uint64_t txn_id
//                           )
// {
//     int return_value;
    
//     return_value = metadata_activate ( server, txn_id, RUN_CATALOG);

//     return return_value;
// } //end of funct

// // mark all attribues from the dataset as active making them visible to other
// // processes.
// int metadata_activate_timestep_attribute (const md_server &server
//                           ,uint64_t txn_id
//                           )
// {
//     int return_value;
    
//     return_value = metadata_activate ( server, txn_id, TIMESTEP_ATTR_CATALOG);

//     return return_value;
// } //end of funct


// // mark all attribues from the dataset as active making them visible to other
// // processes.
// int metadata_activate_timestep (const md_server &server
//                           ,uint64_t txn_id
//                           )
// {
//     int return_value;
    
//     return_value = metadata_activate ( server, txn_id, TIMESTEP_CATALOG);

//     return return_value;
// } //end of funct


// // mark a variable in the global_catalog as active making it visible to other
// // processes.
// int metadata_activate_type (const md_server &server
//                           ,uint64_t txn_id
//                           )
// {
//     int return_value;
    
//     return_value = metadata_activate ( server, txn_id, TYPE_CATALOG);

//     return return_value;
// } //end of funct

// // mark a variable in the global_catalog as active making it visible to other
// // processes.
// int metadata_activate_var_attribute (const md_server &server
//                           ,uint64_t txn_id
//                           )
// {
//     int return_value;
    
//     return_value = metadata_activate ( server, txn_id, VAR_ATTR_CATALOG);

//     return return_value;
// } //end of funct


// // mark a variable in the global_catalog as active making it visible to other
// // processes.
// int metadata_activate_var (const md_server &server
//                           ,uint64_t txn_id
//                           )
// {
//     int return_value;
    
//     return_value = metadata_activate ( server, txn_id, VAR_CATALOG);

//     return return_value;
// } //end of funct


// static int metadata_activate (const md_server &server
//                           ,uint64_t txn_id
//                           , md_catalog_type catalog_type
//                           )
// {
//     add_timing_point(MD_ACTIVATE_START);

//     int return_value;
//     std::string res;

//     md_activate_args args;

//     args.job_id = my_job_id;
//     args.catalog_type = catalog_type;

//     OpActivateMeta *op = new OpActivateMeta(server.peer_ptr, args);
//     future<std::string> fut = op->GetFuture();
//     opbox::LaunchOp(op);

//     res=fut.get();  

//     deArchiveMsgFromServer(res, return_value);
//     add_timing_point(MD_ACTIVATE_DONE);

//     return return_value;
// } //end of funct

// retrieve the list all of attributes associated with a particular type
int metadata_catalog_all_run_attributes_in_run (const md_server &server
                                        ,uint64_t run_id             
                                        ,uint64_t txn_id
                                        //,md_query_type query_type
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_run_attribute_entry> &entries
                                        )
{
    add_timing_point(MD_CATALOG_ALL_RUN_ATTRIBUTES_START);
    int return_value;
    std::string res;
    // string res;    

    md_catalog_all_run_attributes_args args;

    args.job_id = my_job_id;
    args.query_type = query_type;
    args.run_id = run_id;
    //extreme_debug_log << "txn_id is " << to_string(txn_id ) << endl;  
   
    OpCatalogAllRunAttributesMeta *op = new OpCatalogAllRunAttributesMeta(server.peer_ptr, args);
    future<std::string> fut = op->GetFuture();
    // future<string> fut = op->GetFuture();    
    opbox::LaunchOp(op);

    res=fut.get();
    //extreme_debug_log << "in md_client, res: " << res << " size: " << res.size() << endl;

    deArchiveMsgFromServer(res, entries, count, return_value);
    add_timing_point(MD_CATALOG_ALL_RUN_ATTRIBUTES_DONE);  

    return return_value;
} //end of funct

// // retrieve the list all of attributes associated with a particular type
// int metadata_catalog_all_run_attributes (const md_server &server
//                                         ,uint64_t run_id             
//                                         ,uint64_t txn_id
//                                         //,md_query_type query_type
//                                         ,uint32_t &count
//                                         ,std::vector<md_catalog_run_attribute_entry> &entries
//                                         )
// {
//     add_timing_point(MD_CATALOG_ALL_RUN_ATTRIBUTES_START);
//     int return_value;
//     // std::string res;
//     string res;    

//     md_catalog_all_run_attributes_args args;

//     args.job_id = my_job_id;
//     args.query_type = query_type;
//     args.run_id = run_id;
//     //extreme_debug_log << "txn_id is " << to_string(txn_id ) << endl;  
   
//     OpCatalogAllRunAttributesMeta *op = new OpCatalogAllRunAttributesMeta(server.peer_ptr, args);
//     // future<std::string> fut = op->GetFuture();
//     future<string> fut = op->GetFuture();    
//     opbox::LaunchOp(op);

//     res=fut.get();

//     deArchiveMsgFromServer(res, entries, count, return_value);
//     add_timing_point(MD_CATALOG_ALL_RUN_ATTRIBUTES_DONE);  

//     return return_value;
// } //end of funct

// retrieve the list all of attributes associated with a particular type
int metadata_catalog_all_run_attributes_with_type_in_run (const md_server &server
                                        ,uint64_t run_id
                                        ,uint64_t type_id             
                                        ,uint64_t txn_id
                                        //,md_query_type query_type
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_run_attribute_entry> &entries
                                        )
{
    add_timing_point(MD_CATALOG_ALL_RUN_ATTRIBUTES_WITH_TYPE_START);
    int return_value;
    // std::string res;
    string res;    

    md_catalog_all_run_attributes_with_type_args args;

    args.run_id = run_id;
    args.type_id = type_id;
    args.job_id = my_job_id;
    args.query_type = query_type;
    //extreme_debug_log << "txn_id is " << to_string(txn_id ) << endl;  
   
    OpCatalogAllRunAttributesWithTypeMeta *op = new OpCatalogAllRunAttributesWithTypeMeta(server.peer_ptr, args);
    // future<std::string> fut = op->GetFuture();
    future<string> fut = op->GetFuture();    
    opbox::LaunchOp(op);

    res=fut.get();

    deArchiveMsgFromServer(res, entries, count, return_value);
    add_timing_point(MD_CATALOG_ALL_RUN_ATTRIBUTES_WITH_TYPE_DONE);  

    return return_value;
} //end of funct


// retrieve the list all of attributes associated with a particular type
int metadata_catalog_all_timestep_attributes_in_timestep (const md_server &server
                                        ,uint64_t run_id             
                                        ,uint64_t timestep_id             
                                        ,uint64_t txn_id
                                        //,md_query_type query_type
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_timestep_attribute_entry> &entries
                                        )
{
    add_timing_point(MD_CATALOG_ALL_TIMESTEP_ATTRIBUTES_START);
    int return_value;
    // std::string res;
    string res;    

    md_catalog_all_timestep_attributes_args args;

    args.job_id = my_job_id;
    args.query_type = query_type;
    args.run_id = run_id;
    args.timestep_id = timestep_id;
    //extreme_debug_log << "txn_id is " << to_string(txn_id ) << endl;  
   
    OpCatalogAllTimestepAttributesMeta *op = new OpCatalogAllTimestepAttributesMeta(server.peer_ptr, args);
    // future<std::string> fut = op->GetFuture();
    future<string> fut = op->GetFuture();    
    opbox::LaunchOp(op);

    res=fut.get();

    deArchiveMsgFromServer(res, entries, count, return_value);
    add_timing_point(MD_CATALOG_ALL_TIMESTEP_ATTRIBUTES_DONE);  

    return return_value;
} //end of funct

// retrieve the list all of attributes associated with a particular type
int metadata_catalog_all_timestep_attributes_with_type_in_timestep (const md_server &server
                                        ,uint64_t run_id             
                                        ,uint64_t timestep_id
                                        ,uint64_t type_id             
                                        ,uint64_t txn_id
                                        //,md_query_type query_type
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_timestep_attribute_entry> &entries
                                        )
{
    add_timing_point(MD_CATALOG_ALL_TIMESTEP_ATTRIBUTES_WITH_TYPE_START);
    int return_value;
    // std::string res;
    string res;    

    md_catalog_all_timestep_attributes_with_type_args args;

    args.run_id = run_id;
    args.timestep_id = timestep_id;
    args.type_id = type_id;
    args.job_id = my_job_id;
    args.query_type = query_type;
    //extreme_debug_log << "txn_id is " << to_string(txn_id ) << endl;  
   
    OpCatalogAllTimestepAttributesWithTypeMeta *op = new OpCatalogAllTimestepAttributesWithTypeMeta(server.peer_ptr, args);
    // future<std::string> fut = op->GetFuture();
    future<string> fut = op->GetFuture();    
    opbox::LaunchOp(op);

    res=fut.get();

    deArchiveMsgFromServer(res, entries, count, return_value);
    add_timing_point(MD_CATALOG_ALL_TIMESTEP_ATTRIBUTES_WITH_TYPE_DONE);  

    return return_value;
} //end of funct

// retrieve the list all of attributes associated with a particular type
int metadata_catalog_all_timesteps_with_var_attributes_with_type_var_dims (const md_server &server
                                        ,uint64_t run_id             
                                        ,uint64_t type_id
                                        ,uint64_t var_id             
                                        ,uint64_t txn_id
                                        //,md_query_type query_type
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
    args.job_id = my_job_id;
    args.query_type = query_type;
    args.num_dims = num_dims;
    args.dims = dims;
    //extreme_debug_log << "txn_id is " << to_string(txn_id ) << endl;  
   
    OpCatalogAllTimestepsWithVarAttributesWithTypeVarDimsMeta *op = new OpCatalogAllTimestepsWithVarAttributesWithTypeVarDimsMeta(server.peer_ptr, args);
    future<std::string> fut = op->GetFuture();
    opbox::LaunchOp(op);

    res=fut.get();

    deArchiveMsgFromServer(res, entries, count, return_value);
    add_timing_point(MD_CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_DIMS_DONE);  

    return return_value;
} //end of funct


// retrieve the list all of attributes associated with a particular type
int metadata_catalog_all_timesteps_with_var_attributes_with_type_var (const md_server &server
                                        ,uint64_t run_id             
                                        ,uint64_t type_id
                                        ,uint64_t var_id             
                                        ,uint64_t txn_id
                                        //,md_query_type query_type
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
    args.job_id = my_job_id;
    args.query_type = query_type;
    //extreme_debug_log << "txn_id is " << to_string(txn_id ) << endl;  
   
    OpCatalogAllTimestepsWithVarAttributesWithTypeVarMeta *op = new OpCatalogAllTimestepsWithVarAttributesWithTypeVarMeta(server.peer_ptr, args);
    future<std::string> fut = op->GetFuture();
    opbox::LaunchOp(op);

    res=fut.get();

    deArchiveMsgFromServer(res, entries, count, return_value);
    add_timing_point(MD_CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_DONE);  

    return return_value;
} //end of funct

int metadata_catalog_all_timesteps_with_var (const md_server &server
                                        ,uint64_t run_id             
                                        ,uint64_t var_id             
                                        ,uint64_t txn_id
                                        //,md_query_type query_type
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
    args.job_id = my_job_id;
    args.query_type = query_type;
    //extreme_debug_log << "txn_id is " << to_string(txn_id ) << endl;  
   
    OpCatalogAllTimestepsWithVarMeta *op = new OpCatalogAllTimestepsWithVarMeta(server.peer_ptr, args);
    future<std::string> fut = op->GetFuture();
    opbox::LaunchOp(op);

    res=fut.get();

    deArchiveMsgFromServer(res, entries, count, return_value);
    add_timing_point(MD_CATALOG_ALL_TIMESTEPS_WITH_VAR_DONE);  

    return return_value;
} //end of funct


// return a catalog of types in the metadata type table. 
int metadata_catalog_all_types_with_var_attributes_with_var_dims_in_timestep (const md_server &server
                      ,uint64_t run_id
                      ,uint64_t timestep_id
                      ,uint64_t var_id
                      ,uint64_t txn_id
                      //,md_query_type query_type
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
    args.job_id = my_job_id;
    args.query_type = query_type;
    args.num_dims = num_dims;
    args.dims = dims;

    OpCatalogAllTypesWithVarAttributesWithVarDimsInTimestepMeta *op = new OpCatalogAllTypesWithVarAttributesWithVarDimsInTimestepMeta(server.peer_ptr, args);
    future<std::string> fut = op->GetFuture();

    opbox::LaunchOp(op);

    res=fut.get();    

    deArchiveMsgFromServer(res, entries, count, return_value);
    add_timing_point(MD_CATALOG_ALL_TYPES_WITH_VAR_ATTRIBUTES_WITH_VAR_DIMS_IN_TIMESTEP_DONE);   

    return return_value;
} //end of funct


// return a catalog of types in the metadata type table. 
int metadata_catalog_all_types_with_var_attributes_with_var_in_timestep (const md_server &server
                      ,uint64_t run_id
                      ,uint64_t timestep_id
                      ,uint64_t var_id
                      ,uint64_t txn_id
                      //,md_query_type query_type
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
    args.job_id = my_job_id;
    args.query_type = query_type;

    OpCatalogAllTypesWithVarAttributesWithVarInTimestepMeta *op = new OpCatalogAllTypesWithVarAttributesWithVarInTimestepMeta(server.peer_ptr, args);
    future<std::string> fut = op->GetFuture();

    opbox::LaunchOp(op);

    res=fut.get();    

    deArchiveMsgFromServer(res, entries, count, return_value);
    add_timing_point(MD_CATALOG_ALL_TYPES_WITH_VAR_ATTRIBUTES_WITH_VAR_IN_TIMESTEP_DONE);   

    return return_value;
} //end of funct

// retrieve the list all of attributes associated with a particular type
int metadata_catalog_all_var_attributes (const md_server &server
                                        ,uint64_t run_id             
                                        ,uint64_t timestep_id             
                                        ,uint64_t txn_id
                                        //,md_query_type query_type
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_var_attribute_entry> &entries
                                        )
{
    add_timing_point(MD_CATALOG_ALL_VAR_ATTRIBUTES_START);
    int return_value;
    // std::string res;
    string res;    

    md_catalog_all_var_attributes_args args;

    args.job_id = my_job_id;
    args.query_type = query_type;
    args.run_id = run_id;
    args.timestep_id = timestep_id;
    //extreme_debug_log << "txn_id is " << to_string(txn_id ) << endl;  
   
    OpCatalogAllVarAttributesMeta *op = new OpCatalogAllVarAttributesMeta(server.peer_ptr, args);
    // future<std::string> fut = op->GetFuture();
    future<string> fut = op->GetFuture();    
    opbox::LaunchOp(op);

    res=fut.get();

    deArchiveMsgFromServer(res, entries, count, return_value);
    add_timing_point(MD_CATALOG_ALL_VAR_ATTRIBUTES_DONE);  

    return return_value;
} //end of funct

// retrieve a list of attributes overlapping the given dimensions
int metadata_catalog_all_var_attributes_with_dims (const md_server &server
                                        ,uint64_t run_id             
                                        ,uint64_t timestep_id
                                        ,uint64_t txn_id
                                        //,md_query_type query_type
                                        ,uint32_t num_dims
                                        ,const std::vector<md_dim_bounds> &dims
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_var_attribute_entry> &matching_attributes
                                       )
    {
    add_timing_point(MD_CATALOG_ALL_VAR_ATTRIBUTES_WITH_DIMS_START);

    int return_value;
    // std::string res;
    string res;
    md_catalog_all_var_attributes_with_dims_args args;

    args.job_id = my_job_id;
    args.query_type = query_type;
    args.run_id = run_id;
    args.timestep_id = timestep_id;
    args.num_dims = num_dims;
    args.dims = dims;
    // for(int j=0; j< num_dims; j++) {
    //     //extreme_debug_log << "in client d" << j << "_min: " << dims [j].min << endl;
    //     //extreme_debug_log << "in client d" << j << "_max: " << dims [j].max << endl;               
    // }

    OpCatalogAllVarAttributesWithDimsMeta *op = new OpCatalogAllVarAttributesWithDimsMeta(server.peer_ptr, args);
    //extreme_debug_log << "just launched get attr by dims \n";
    // future<std::string> fut = op->GetFuture();
    future<string> fut = op->GetFuture();    
    opbox::LaunchOp(op);

    res=fut.get();    

    deArchiveMsgFromServer(res, matching_attributes, count, return_value);
    add_timing_point(MD_CATALOG_ALL_VAR_ATTRIBUTES_WITH_DIMS_DONE);   
    //extreme_debug_log << "just dearchived matching attributes by dims message from server \n";
    //extreme_debug_log << "number of matching_attributes is " << matching_attributes.size() << "\n";


    return return_value;
} //end of funct

// retrieve the list all of attributes associated with a particular type
int metadata_catalog_all_var_attributes_with_type_by_id (const md_server &server
                                        ,uint64_t timestep_id
                                        ,uint64_t type_id                   
                                        ,uint64_t txn_id
                                        //,md_query_type query_type
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_var_attribute_entry> &entries
                                        )
{
    add_timing_point(MD_CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_BY_ID_START);
    int return_value;
    // std::string res;
    string res;    

    md_catalog_all_var_attributes_with_type_by_id_args args;

    args.job_id = my_job_id;
    args.query_type = query_type;
    args.timestep_id = timestep_id;
    args.type_id = type_id;
    //extreme_debug_log << "type id is " << to_string(type_id ) << endl;  
   
    OpCatalogAllVarAttributesWithTypeByIdMeta *op = new OpCatalogAllVarAttributesWithTypeByIdMeta(server.peer_ptr, args);
    // future<std::string> fut = op->GetFuture();
    future<string> fut = op->GetFuture();    
    opbox::LaunchOp(op);

    res=fut.get();

    deArchiveMsgFromServer(res, entries, count, return_value);
    add_timing_point(MD_CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_BY_ID_DONE);  

    return return_value;
} //end of funct


// retrieve the list all of attributes associated with a particular type
int metadata_catalog_all_var_attributes_with_type_by_name_ver (const md_server &server
                                        ,uint64_t run_id             
                                        ,uint64_t timestep_id
                                        ,const std::string &type_name
                                        ,uint32_t type_version                   
                                        ,uint64_t txn_id
                                        //,md_query_type query_type
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_var_attribute_entry> &entries
                                        )
{
    add_timing_point(MD_CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_BY_NAME_VER_START);
    int return_value;
    // std::string res;
    string res;    

    md_catalog_all_var_attributes_with_type_by_name_ver_args args;

    args.job_id = my_job_id;
    args.query_type = query_type;
    args.run_id = run_id;
    args.timestep_id = timestep_id;
    args.type_name = type_name;
    args.type_version = type_version;
    // extreme_debug_log << " args.job_id is " <<  args.job_id << endl;            
    // extreme_debug_log << " args.run_id is " <<  args.run_id  << endl;        
    // extreme_debug_log << " args.timestep_id is " <<  args.timestep_id << endl;     
    // extreme_debug_log << " args.type_name is " <<  args.type_name  << endl;  
    // extreme_debug_log << " args.type_version is " << args.type_version << endl;  
   
    OpCatalogAllVarAttributesWithTypeByNameVerMeta *op = new OpCatalogAllVarAttributesWithTypeByNameVerMeta(server.peer_ptr, args);
    // future<std::string> fut = op->GetFuture();
    future<string> fut = op->GetFuture();    
    opbox::LaunchOp(op);

    res=fut.get();

    deArchiveMsgFromServer(res, entries, count, return_value);
    add_timing_point(MD_CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_BY_NAME_VER_DONE);  

    return return_value;
} //end of funct


// retrieve a list of attributes overlapping the given dimensions
int metadata_catalog_all_var_attributes_with_type_dims_by_id (const md_server &server
                                        ,uint64_t timestep_id
                                        ,uint64_t type_id
                                        ,uint64_t txn_id
                                        //,md_query_type query_type
                                        ,uint32_t num_dims
                                        ,const std::vector<md_dim_bounds> &dims
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_var_attribute_entry> &matching_attributes
                                        )
{
    add_timing_point(MD_CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_DIMS_BY_ID_START);

    int return_value;
    // std::string res;
    string res;
    md_catalog_all_var_attributes_with_type_dims_by_id_args args;

    args.job_id = my_job_id;
    args.query_type = query_type;
    args.timestep_id = timestep_id;
    args.type_id = type_id;
    args.num_dims = num_dims;
    args.dims = dims;

    OpCatalogAllVarAttributesWithTypeDimsByIdMeta *op = new OpCatalogAllVarAttributesWithTypeDimsByIdMeta(server.peer_ptr, args);
    //extreme_debug_log << "just launched get attr by dims \n";
    // future<std::string> fut = op->GetFuture();
    future<string> fut = op->GetFuture();    
    opbox::LaunchOp(op);

    res=fut.get();    

    deArchiveMsgFromServer(res, matching_attributes, count, return_value);
    add_timing_point(MD_CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_DIMS_BY_ID_DONE);   
    //extreme_debug_log << "just dearchived matching attributes by dims message from server \n";
    //extreme_debug_log << "number of matching_attributes is " << matching_attributes.size() << "\n";


    return return_value;
} //end of funct


// retrieve a list of attributes overlapping the given dimensions
int metadata_catalog_all_var_attributes_with_type_dims_by_name_ver (const md_server &server
                                        ,uint64_t run_id             
                                        ,uint64_t timestep_id
                                        ,const std::string &type_name
                                        ,uint32_t type_version
                                        ,uint64_t txn_id
                                        //,md_query_type query_type
                                        ,uint32_t num_dims
                                        ,const std::vector<md_dim_bounds> &dims
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_var_attribute_entry> &matching_attributes
                                       )
{
    add_timing_point(MD_CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_DIMS_BY_NAME_VER_START);

    int return_value;
    // std::string res;
    string res;
    md_catalog_all_var_attributes_with_type_dims_by_name_ver_args args;

    args.job_id = my_job_id;
    args.query_type = query_type;
    args.run_id = run_id;
    args.timestep_id = timestep_id;
    args.type_name = type_name;
    args.type_version = type_version;
    args.num_dims = num_dims;
    args.dims = dims;

    OpCatalogAllVarAttributesWithTypeDimsByNameVerMeta *op = new OpCatalogAllVarAttributesWithTypeDimsByNameVerMeta(server.peer_ptr, args);
    //extreme_debug_log << "just launched get attr by dims \n";
    // future<std::string> fut = op->GetFuture();
    future<string> fut = op->GetFuture();    
    opbox::LaunchOp(op);

    res=fut.get();    

    deArchiveMsgFromServer(res, matching_attributes, count, return_value);
    add_timing_point(MD_CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_DIMS_BY_NAME_VER_DONE);   
    //extreme_debug_log << "just dearchived matching attributes by dims message from server \n";
    //extreme_debug_log << "number of matching_attributes is " << matching_attributes.size() << "\n";


    return return_value;
} //end of funct

// retrieve a list of attributes overlapping the given dimensions
int metadata_catalog_all_var_attributes_with_type_var_by_id (const md_server &server
                                        ,uint64_t timestep_id
                                        ,uint64_t type_id
                                        ,uint64_t var_id
                                        ,uint64_t txn_id
                                        //,md_query_type query_type
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_var_attribute_entry> &matching_attributes
                                        )
{
    add_timing_point(MD_CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_BY_ID_START);

    int return_value;
    // std::string res;
    string res;
    md_catalog_all_var_attributes_with_type_var_by_id_args args;

    args.job_id = my_job_id;
    args.query_type = query_type;
    args.timestep_id = timestep_id;
    args.type_id = type_id;
    args.var_id = var_id;
    OpCatalogAllVarAttributesWithTypeVarByIdMeta *op = new OpCatalogAllVarAttributesWithTypeVarByIdMeta(server.peer_ptr, args);
    //extreme_debug_log << "just launched get attr by dims \n";
    // future<std::string> fut = op->GetFuture();
    future<string> fut = op->GetFuture();    
    opbox::LaunchOp(op);

    res=fut.get();    

    deArchiveMsgFromServer(res, matching_attributes, count, return_value);
    add_timing_point(MD_CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_BY_ID_DONE);   
    //extreme_debug_log << "just dearchived matching attributes by dims message from server \n";
    //extreme_debug_log << "number of matching_attributes is " << matching_attributes.size() << "\n";


    return return_value;
} //end of funct

// retrieve a list of attributes overlapping the given dimensions
int metadata_catalog_all_var_attributes_with_type_var_by_name_ver (const md_server &server
                                        ,uint64_t run_id             
                                        ,uint64_t timestep_id
                                        ,const std::string &type_name
                                        ,uint32_t type_version
                                        ,const std::string &var_name
                                        ,uint32_t var_version
                                        ,uint64_t txn_id
                                        //,md_query_type query_type
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_var_attribute_entry> &matching_attributes
                                        )
{
    add_timing_point(MD_CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_BY_NAME_VER_START);

    int return_value;
    // std::string res;
    string res;
    md_catalog_all_var_attributes_with_type_var_by_name_ver_args args;

    args.job_id = my_job_id;
    args.query_type = query_type;
    args.run_id = run_id;
    args.timestep_id = timestep_id;
    args.type_name = type_name;
    args.type_version = type_version;
    args.var_name = var_name;
    args.var_version = var_version;
    OpCatalogAllVarAttributesWithTypeVarByNameVerMeta *op = new OpCatalogAllVarAttributesWithTypeVarByNameVerMeta(server.peer_ptr, args);
    //extreme_debug_log << "just launched get attr by dims \n";
    // future<std::string> fut = op->GetFuture();
    future<string> fut = op->GetFuture();    
    opbox::LaunchOp(op);

    res=fut.get();    

    deArchiveMsgFromServer(res, matching_attributes, count, return_value);
    add_timing_point(MD_CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_BY_NAME_VER_DONE);   
    //extreme_debug_log << "just dearchived matching attributes by dims message from server \n";
    //extreme_debug_log << "number of matching_attributes is " << matching_attributes.size() << "\n";


    return return_value;
} //end of funct

// retrieve a list of attributes overlapping the given dimensions
int metadata_catalog_all_var_attributes_with_type_var_dims_by_id (const md_server &server
                                        ,uint64_t timestep_id
                                        ,uint64_t type_id
                                        ,uint64_t var_id
                                        ,uint64_t txn_id
                                        //,md_query_type query_type
                                        ,uint32_t num_dims
                                        ,const std::vector<md_dim_bounds> &dims
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_var_attribute_entry> &matching_attributes
                                        )
{
    add_timing_point(MD_CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_DIMS_BY_ID_START);

    int return_value;
    // std::string res;
    string res;
    md_catalog_all_var_attributes_with_type_var_dims_by_id_args args;

    args.job_id = my_job_id;
    args.query_type = query_type;
    args.timestep_id = timestep_id;
    args.type_id = type_id;
    args.var_id = var_id;
    args.num_dims = num_dims;
    args.dims = dims;
    OpCatalogAllVarAttributesWithTypeVarDimsByIdMeta *op = new OpCatalogAllVarAttributesWithTypeVarDimsByIdMeta(server.peer_ptr, args);
    //extreme_debug_log << "just launched get attr by dims \n";
    // future<std::string> fut = op->GetFuture();
    future<string> fut = op->GetFuture();    
    opbox::LaunchOp(op);

    res=fut.get();    

    deArchiveMsgFromServer(res, matching_attributes, count, return_value);
    add_timing_point(MD_CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_DIMS_BY_ID_DONE);   
    //extreme_debug_log << "just dearchived matching attributes by dims message from server \n";
    //extreme_debug_log << "number of matching_attributes is " << matching_attributes.size() << "\n";


    return return_value;
} //end of funct

// retrieve a list of attributes overlapping the given dimensions
int metadata_catalog_all_var_attributes_with_type_var_dims_by_name_ver (const md_server &server
                                        ,uint64_t run_id             
                                        ,uint64_t timestep_id
                                        ,const std::string &type_name
                                        ,uint32_t type_version
                                        ,const std::string &var_name
                                        ,uint32_t var_version
                                        ,uint64_t txn_id
                                        //,md_query_type query_type
                                        ,uint32_t num_dims
                                        ,const std::vector<md_dim_bounds> &dims
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_var_attribute_entry> &matching_attributes
                                        )
{
    add_timing_point(MD_CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_DIMS_BY_NAME_VER_START);

    int return_value;
    // std::string res;
    string res;
    md_catalog_all_var_attributes_with_type_var_dims_by_name_ver_args args;

    args.job_id = my_job_id;
    args.query_type = query_type;
    args.run_id = run_id;
    args.timestep_id = timestep_id;
    args.type_name = type_name;
    args.type_version = type_version;
    args.var_name = var_name;
    args.var_version = var_version;
    args.num_dims = num_dims;
    args.dims = dims;
    OpCatalogAllVarAttributesWithTypeVarDimsByNameVerMeta *op = new OpCatalogAllVarAttributesWithTypeVarDimsByNameVerMeta(server.peer_ptr, args);
    //extreme_debug_log << "just launched get attr by dims \n";
    // future<std::string> fut = op->GetFuture();
    future<string> fut = op->GetFuture();    
    opbox::LaunchOp(op);

    res=fut.get();    

    deArchiveMsgFromServer(res, matching_attributes, count, return_value);
    add_timing_point(MD_CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_DIMS_BY_NAME_VER_DONE);   
    //extreme_debug_log << "just dearchived matching attributes by dims message from server \n";
    //extreme_debug_log << "number of matching_attributes is " << matching_attributes.size() << "\n";


    return return_value;
} //end of funct

// retrieve a list of attributes overlapping the given dimensions
int metadata_catalog_all_var_attributes_with_var_by_id (const md_server &server
                                        ,uint64_t run_id             
                                        ,uint64_t timestep_id
                                        ,uint64_t var_id
                                        ,uint64_t txn_id
                                        //,md_query_type query_type
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_var_attribute_entry> &matching_attributes
                                        )
{
    add_timing_point(MD_CATALOG_ALL_VAR_ATTRIBUTES_WITH_VAR_BY_ID_START);

    int return_value;
    // std::string res;
    string res;
    md_catalog_all_var_attributes_with_var_by_id_args args;

    args.job_id = my_job_id;
    args.query_type = query_type;
    args.run_id = run_id;
    args.timestep_id = timestep_id;
    args.var_id = var_id;
    OpCatalogAllVarAttributesWithVarByIdMeta *op = new OpCatalogAllVarAttributesWithVarByIdMeta(server.peer_ptr, args);
    //extreme_debug_log << "just launched get attr by dims \n";
    // future<std::string> fut = op->GetFuture();
    future<string> fut = op->GetFuture();    
    opbox::LaunchOp(op);

    res=fut.get();    

    deArchiveMsgFromServer(res, matching_attributes, count, return_value);
    add_timing_point(MD_CATALOG_ALL_VAR_ATTRIBUTES_WITH_VAR_BY_ID_DONE);   
    //extreme_debug_log << "just dearchived matching attributes by dims message from server \n";
    //extreme_debug_log << "number of matching_attributes is " << matching_attributes.size() << "\n";


    return return_value;
} //end of funct

// retrieve a list of attributes overlapping the given dimensions
int metadata_catalog_all_var_attributes_with_var_by_name_ver (const md_server &server
                                        ,uint64_t run_id             
                                        ,uint64_t timestep_id
                                        ,const std::string &var_name
                                        ,uint32_t var_version
                                        ,uint64_t txn_id
                                        //,md_query_type query_type
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_var_attribute_entry> &matching_attributes
                                        )
{
    add_timing_point(MD_CATALOG_ALL_VAR_ATTRIBUTES_WITH_VAR_BY_NAME_VER_START);

    int return_value;
    // std::string res;
    string res;
    md_catalog_all_var_attributes_with_var_by_name_ver_args args;

    args.job_id = my_job_id;
    args.query_type = query_type;
    args.run_id = run_id;
    args.timestep_id = timestep_id;
    args.var_name = var_name;
    args.var_version = var_version;
    OpCatalogAllVarAttributesWithVarByNameVerMeta *op = new OpCatalogAllVarAttributesWithVarByNameVerMeta(server.peer_ptr, args);
    //extreme_debug_log << "just launched get attr by dims \n";
    // future<std::string> fut = op->GetFuture();
    future<string> fut = op->GetFuture();    
    opbox::LaunchOp(op);

    res=fut.get();    

    deArchiveMsgFromServer(res, matching_attributes, count, return_value);
    add_timing_point(MD_CATALOG_ALL_VAR_ATTRIBUTES_WITH_VAR_BY_NAME_VER_DONE);   
    //extreme_debug_log << "just dearchived matching attributes by dims message from server \n";
    //extreme_debug_log << "number of matching_attributes is " << matching_attributes.size() << "\n";


    return return_value;
} //end of funct

// retrieve a list of attributes overlapping the given dimensions
int metadata_catalog_all_var_attributes_with_var_dims_by_id (const md_server &server
                                        ,uint64_t run_id             
                                        ,uint64_t timestep_id
                                        ,uint64_t var_id
                                        ,uint64_t txn_id
                                        //,md_query_type query_type
                                        ,uint32_t num_dims
                                        ,const std::vector<md_dim_bounds> &dims
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_var_attribute_entry> &matching_attributes
                                        )
{
    add_timing_point(MD_CATALOG_ALL_VAR_ATTRIBUTES_WITH_VAR_DIMS_BY_ID_START);

    int return_value;
    // std::string res;
    string res;
    md_catalog_all_var_attributes_with_var_dims_by_id_args args;

    args.job_id = my_job_id;
    args.query_type = query_type;
    args.run_id = run_id;
    args.timestep_id = timestep_id;
    args.var_id = var_id;
    args.num_dims = num_dims;
    args.dims = dims;
    OpCatalogAllVarAttributesWithVarDimsByIdMeta *op = new OpCatalogAllVarAttributesWithVarDimsByIdMeta(server.peer_ptr, args);
    //extreme_debug_log << "just launched get attr by dims \n";
    // future<std::string> fut = op->GetFuture();
    future<string> fut = op->GetFuture();    
    opbox::LaunchOp(op);

    res=fut.get();    

    deArchiveMsgFromServer(res, matching_attributes, count, return_value);
    add_timing_point(MD_CATALOG_ALL_VAR_ATTRIBUTES_WITH_VAR_DIMS_BY_ID_DONE);   
    //extreme_debug_log << "just dearchived matching attributes by dims message from server \n";
    //extreme_debug_log << "number of matching_attributes is " << matching_attributes.size() << "\n";


    return return_value;
} //end of funct


// retrieve a list of attributes overlapping the given dimensions
int metadata_catalog_all_var_attributes_with_var_dims_by_name_ver (const md_server &server
                                        ,uint64_t run_id             
                                        ,uint64_t timestep_id
                                        ,const std::string &var_name
                                        ,uint32_t var_version                       
                                        ,uint64_t txn_id
                                        //,md_query_type query_type
                                        ,uint32_t num_dims
                                        ,const std::vector<md_dim_bounds> &dims
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_var_attribute_entry> &matching_attributes
                                        )
{
    add_timing_point(MD_CATALOG_ALL_VAR_ATTRIBUTES_WITH_VAR_DIMS_BY_NAME_VER_START);

    int return_value;
    // std::string res;
    string res;
    md_catalog_all_var_attributes_with_var_dims_by_name_ver_args args;

    args.job_id = my_job_id;
    args.query_type = query_type;
    args.run_id = run_id;
    args.timestep_id = timestep_id;
    args.var_name = var_name;
    args.var_version = var_version;
    args.num_dims = num_dims;
    args.dims = dims;
    OpCatalogAllVarAttributesWithVarDimsByNameVerMeta *op = new OpCatalogAllVarAttributesWithVarDimsByNameVerMeta(server.peer_ptr, args);
    //extreme_debug_log << "just launched get attr by dims \n";
    // future<std::string> fut = op->GetFuture();
    future<string> fut = op->GetFuture();    
    opbox::LaunchOp(op);

    res=fut.get();    

    deArchiveMsgFromServer(res, matching_attributes, count, return_value);
    add_timing_point(MD_CATALOG_ALL_VAR_ATTRIBUTES_WITH_VAR_DIMS_BY_NAME_VER_DONE);   
    //extreme_debug_log << "just dearchived matching attributes by dims message from server \n";
    //extreme_debug_log << "number of matching_attributes is " << matching_attributes.size() << "\n";


    return return_value;
} //end of funct

// return a catalog of items in the metadata service. For scalars, the num_dims
// will be 0
int metadata_catalog_run (const md_server &server
                      ,uint64_t txn_id
                      //,md_query_type query_type
                      ,uint32_t &count
                      ,std::vector<md_catalog_run_entry> &entries
                     )
{
    add_timing_point(MD_CATALOG_RUN_START);

    int return_value;
    // std::string res;
    // std::string res2;
    string res;

    md_catalog_run_args args;

    args.job_id = my_job_id;
    args.query_type = query_type;

    OpCatalogRunMeta *op = new OpCatalogRunMeta(server.peer_ptr, args);
    // future<std::string> fut = op->GetFuture();
    future<string> fut = op->GetFuture();
    opbox::LaunchOp(op);   

    res=fut.get();

    deArchiveMsgFromServer(res, entries, count, return_value);
    add_timing_point(MD_CATALOG_RUN_DONE);

    return return_value;
} //end of funct

// return a catalog of items in the metadata service. For scalars, the num_dims
// will be 0
int metadata_catalog_timestep (const md_server &server
                      ,uint64_t run_id            
                      ,uint64_t txn_id
                      //,md_query_type query_type
                      ,uint32_t &count
                      ,std::vector<md_catalog_timestep_entry> &entries
                     )
{
    add_timing_point(MD_CATALOG_TIMESTEP_START);

    int return_value;
    std::string res;

    md_catalog_timestep_args args;

    args.run_id = run_id;
    args.job_id = my_job_id;
    args.query_type = query_type;

    OpCatalogTimestepMeta *op = new OpCatalogTimestepMeta(server.peer_ptr, args);
    future<std::string> fut = op->GetFuture();
    opbox::LaunchOp(op);   

    res=fut.get();

    deArchiveMsgFromServer(res, entries, count, return_value);
    add_timing_point(MD_CATALOG_TIMESTEP_DONE);

    return return_value;
} //end of funct

// return a catalog of types in the metadata type table. 
int metadata_catalog_type (const md_server &server
                      ,uint64_t run_id
                      ,uint64_t txn_id
                      //,md_query_type query_type
                      ,uint32_t &count
                      ,std::vector<md_catalog_type_entry> &entries
                     )
{
    add_timing_point(MD_CATALOG_TYPE_START);
    int return_value;
    std::string res;

    md_catalog_type_args args;

    args.job_id = my_job_id;
    args.query_type = query_type;
    args.run_id = run_id;

    OpCatalogTypeMeta *op = new OpCatalogTypeMeta(server.peer_ptr, args);
    future<std::string> fut = op->GetFuture();

    opbox::LaunchOp(op);

    res=fut.get();    

    deArchiveMsgFromServer(res, entries, count, return_value);
    add_timing_point(MD_CATALOG_TYPE_DONE);   

    return return_value;
} //end of funct

// return a catalog of items in the metadata service. For scalars, the num_dims
// will be 0
int metadata_catalog_var (const md_server &server
                      ,uint64_t run_id
                      ,uint64_t timestep_id
                      ,uint64_t txn_id
                      //,md_query_type query_type
                      ,uint32_t &count
                      ,std::vector<md_catalog_var_entry> &entries
                     )
{
    add_timing_point(MD_CATALOG_VAR_START);

    int return_value;
    std::string res;

    md_catalog_var_args args;

    args.job_id = my_job_id;
    args.query_type = query_type;
    args.run_id = run_id;
    args.timestep_id = timestep_id;

    OpCatalogVarMeta *op = new OpCatalogVarMeta(server.peer_ptr, args);
    future<std::string> fut = op->GetFuture();
    opbox::LaunchOp(op);   

    res=fut.get();

    deArchiveMsgFromServer(res, entries, count, return_value);
    add_timing_point(MD_CATALOG_VAR_DONE);

    return return_value;
} //end of funct

// create a run  table in an inactive state. Vars will be added later. Once the
// run table is complete (the transaction is ready to commit), it can then be
// activated to make it visible to other processes.
int metadata_create_run  (const md_server &server
                        ,uint64_t &run_id   
                        ,uint64_t job_id
                        ,const std::string &name
                        // ,const std::string &path
                        ,uint64_t txn_id
                        // //,md_query_type query_type 
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
    //extreme_debug_log << "got to top of metadata_create_run  \n";
    int return_value;
    std::string res;

    md_create_run_args args;
    args.job_id = job_id;
    args.name = name;
    // args.path = path;
    // args.job_id = my_job_id;
    // args.query_type = query_type;
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

    // //extreme_debug_log << "name: " << name << " path: " << path << " txn_id: " << txn_id << " npx: " << npx << " npy: " << npy << " npz: " << npz << endl;
    // //extreme_debug_log << "rank_to_dims_funct.size(): " << rank_to_dims_funct.size() << " objector_funct.size(): " << objector_funct.size() << endl;
    // //extreme_debug_log << "rank_to_dims_funct: " << rank_to_dims_funct << " objector_funct: " << objector_funct << endl;

    //extreme_debug_log << "about to create new run  \n";
    
    OpCreateRunMeta *op = new OpCreateRunMeta(server.peer_ptr, args);    
    //extreme_debug_log << "about to get future" <<endl;
    future<std::string> fut = op->GetFuture();
    //extreme_debug_log << "about to launch op" <<endl;
    opbox::LaunchOp(op);
    //extreme_debug_log << "about to get final result" <<endl;

    //extreme_debug_log << "metadata_create_run: about to fut.get()" << endl;
    res = fut.get();
    //extreme_debug_log << "metadata_create_run: res: " << res << endl;

    //extreme_debug_log << "metadata_create_run: about to deArchiveMsgFromServer" << endl;
    deArchiveMsgFromServer(res, run_id, return_value);
    add_timing_point(MD_CREATE_RUN_DONE); 

    //extreme_debug_log << "metadata_create_run: finished" << endl;

    //extreme_debug_log << "finished metadata_create_run. run_id: " << run_id << " return_value: " << return_value << endl;

    // return_value = register_objector_funct_write (objector_funct_name, run_id);
    // if (return_value != RC_OK) {
    //     error_log << "error in registering the objector function in write for " << server.URL << endl;
    // }
    // else {
    //     //extreme_debug_log << "just registered for server " << server.URL << endl;        
    // }

    return return_value;
} //end of funct


int metadata_create_run  (const md_server &server
                        ,uint64_t &run_id   
                        ,const md_catalog_run_entry &run
                        )
{
    add_timing_point(MD_CREATE_RUN_START);
    //extreme_debug_log << "got to top of metadata_create_run  \n";
    int return_value;
    std::string res;

    md_create_run_args args;
    args.job_id = run.job_id;
    args.name = run.name;
    // args.path = path;
    // args.job_id = run.txn_id;
    args.npx = run.npx;
    args.npy = run.npy;
    args.npz = run.npz;

    OpCreateRunMeta *op = new OpCreateRunMeta(server.peer_ptr, args);    
    //extreme_debug_log << "about to get future" <<endl;
    future<std::string> fut = op->GetFuture();
    //extreme_debug_log << "about to launch op" <<endl;
    opbox::LaunchOp(op);
    //extreme_debug_log << "about to get final result" <<endl;

    res = fut.get();

    //extreme_debug_log << "about to deArchiveMsgFromServer in metadata_create_run" << endl;
    deArchiveMsgFromServer(res, run_id, return_value);
    add_timing_point(MD_CREATE_RUN_DONE); 

    //extreme_debug_log << "finished metadata_create_run. run_id: " << run_id << " return_value: " << return_value << endl;

    return return_value;
} //end of funct


// create a run  table in an inactive state. Vars will be added later. Once the
// run table is complete (the transaction is ready to commit), it can then be
// activated to make it visible to other processes.
// does not include the objector
int metadata_create_run  (const md_server &server
                        ,uint64_t &run_id   
                        ,uint64_t job_id
                        ,const std::string &name
                        // ,const std::string &path
                        ,uint64_t txn_id
                        // //,md_query_type query_type 
                        ,uint64_t npx
                        ,uint64_t npy
                        ,uint64_t npz
                        )
{
    add_timing_point(MD_CREATE_RUN_START);
    //extreme_debug_log << "got to top of metadata_create_run  \n";
    int return_value;
    std::string res;

    md_create_run_args args;
    args.job_id = job_id;
    args.name = name;
    // args.path = path;
    // args.job_id = my_job_id;
    // args.query_type = query_type;
    args.npx = npx;
    args.npy = npy;
    args.npz = npz;

    // //extreme_debug_log << "name: " << name << " path: " << path << " txn_id: " << txn_id << " npx: " << npx << " npy: " << npy << " npz: " << npz << endl;
    // //extreme_debug_log << "rank_to_dims_funct.size(): " << rank_to_dims_funct.size() << " objector_funct.size(): " << objector_funct.size() << endl;
    // //extreme_debug_log << "rank_to_dims_funct: " << rank_to_dims_funct << " objector_funct: " << objector_funct << endl;

    //extreme_debug_log << "about to create new run  \n";
    
    OpCreateRunMeta *op = new OpCreateRunMeta(server.peer_ptr, args);    
    //extreme_debug_log << "about to get future" <<endl;
    future<std::string> fut = op->GetFuture();
    //extreme_debug_log << "about to launch op" <<endl;
    opbox::LaunchOp(op);
    //extreme_debug_log << "about to get final result" <<endl;

    res = fut.get();

    deArchiveMsgFromServer(res, run_id, return_value);
    add_timing_point(MD_CREATE_RUN_DONE); 

    // return_value = register_objector_funct_write (objector_funct_name, run_id);
    // if (return_value != RC_OK) {
    //     error_log << "error in registering the objector function in write for " << server.URL << endl;
    // }
    // else {
    //     //extreme_debug_log << "just registered for server " << server.URL << endl;        
    // }

    return return_value;
} //end of funct

// create a timestep  table in an inactive state. Vars will be added later. Once the
// timestep table is complete (the transaction is ready to commit), it can then be
// activated to make it visible to other processes.
int metadata_create_timestep  (const md_server &server
                        ,uint64_t timestep_id                        
                        ,uint64_t run_id   
                        ,const std::string &path                     
                        ,uint64_t txn_id
                        // //,md_query_type query_type 
                        )
{
    add_timing_point(MD_CREATE_TIMESTEP_START);
    //extreme_debug_log << "got to top of metadata_create_timestep  \n";
    int return_value;
    std::string res;

    md_create_timestep_args args;
    args.timestep_id = timestep_id;
    args.run_id = run_id;
    args.path = path;
    // //extreme_debug_log << "in client, path: " << path << endl;
    args.job_id = my_job_id;
    // args.query_type = query_type;
    //extreme_debug_log << "about to create new timestep  \n";
    //extreme_debug_log << "timestep id: " << timestep_id << " run_id: " << run_id << "  \n";

    
    OpCreateTimestepMeta *op = new OpCreateTimestepMeta(server.peer_ptr, args);    
    //extreme_debug_log << "about to get future" <<endl;
    future<std::string> fut = op->GetFuture();
    //extreme_debug_log << "about to launch op" <<endl;
    opbox::LaunchOp(op);
    //extreme_debug_log << "about to get final result" <<endl;

    res = fut.get();

    deArchiveMsgFromServer(res, timestep_id, return_value);
    add_timing_point(MD_CREATE_TIMESTEP_DONE); 

    return return_value;
} //end of funct

int metadata_create_timestep  (const md_server &server
                        ,const md_catalog_timestep_entry &timestep)

{
    return metadata_create_timestep  (server, timestep.timestep_id, timestep.run_id,
        timestep.path, timestep.txn_id );                  
}


// // create a type in an inactive state. Attributes will be added later. Once the
// // type is complete (the transaction is ready to commit), it can then be
// // activated to make it visible to other processes.
// int metadata_create_type (const md_server &server
//                         ,uint64_t &type_id
//                         ,uint64_t run_id
//                         ,const std::string &name
//                         ,uint32_t version
//                         ,uint64_t txn_id
//                         )
// {

//note - could be modified to take indv parameters (see directly above) or to take a type_id from the user
int metadata_create_type (const md_server &server
                        ,uint64_t &type_id
                        ,const md_catalog_type_entry &new_type
                        )
{
    add_timing_point(MD_CREATE_TYPE_START);
    //extreme_debug_log << "got to top of metadata_create_type \n";
    int return_value;
    std::string res;

    md_create_type_args args;

    args.run_id = new_type.run_id;
    args.name = new_type.name;
    args.version = new_type.version;
    args.job_id = my_job_id;
    //extreme_debug_log << "about to create new type \n";
    
    OpCreateTypeMeta *op = new OpCreateTypeMeta(server.peer_ptr, args);
    //extreme_debug_log << "about to get future" <<endl;
    future<std::string> fut = op->GetFuture();
    //extreme_debug_log << "about to launch op" <<endl;
    opbox::LaunchOp(op);
    //extreme_debug_log << "about to get final result" <<endl;

    res = fut.get();

    deArchiveMsgFromServer(res, type_id, return_value); 
    add_timing_point(MD_CREATE_TYPE_DONE);   

    return return_value;
} //end of funct

// // create a variable in an inactive state. Chunks will be added later. Once the
// // variable is complete (the transaction is ready to commit), it can then be
// // activated to make it visible to other processes.
// // For a scalar, num_dims should be 0 causing the other parameters
// // to be ignored.
// int metadata_create_var (const md_server &server
//                         ,uint64_t &var_id
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
int metadata_create_var (const md_server &server
                        ,uint64_t &var_id
                        ,const md_catalog_var_entry &new_var
                        )
{
    add_timing_point(MD_CREATE_VAR_START);
    //extreme_debug_log << "got to top of metadata_create_var \n";
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
    // //extreme_debug_log << "temp: " << temp << endl;
    // args.var_id = stoull(temp, &size, 10); //todo
   //  string var_name_ver = new_var.name;
   //  var_name_ver += "_";
   //  var_name_ver += to_string(new_var.version);
   //  //extreme_debug_log << "var_name_ver: " << var_name_ver << endl;
   //  map <string, uint64_t>::iterator it = var_ids.find(var_name_ver);
   //  if ( it == var_ids.end() ) {
   //    // not found
   //      var_ids[var_name_ver] = var_ids.size();
   //      args.var_id = var_ids.size();

   //      //extreme_debug_log << "not found, and var_ids.size(): " << var_ids.size() << endl;
   //  } else {
   //    // found
   //      //extreme_debug_log << "found, var_id: " << it->second << endl;
   //      args.var_id = it->second;
   //  }

   // for (std::map<string,uint64_t>::iterator it=var_ids.begin(); it!=var_ids.end(); ++it) {
   //    string var_name = it->first;
   //    uint64_t temp_var_id = it->second;
   //    //extreme_debug_log << "var_name: " << var_name << " temp_var_id " << temp_var_id << endl;
   //  }
   //  //extreme_debug_log << "args.var_id: " << args.var_id << endl;

    //////////////////////////////////////////////////////////////////////////////////
    args.run_id = new_var.run_id;
    args.var_id = new_var.var_id;
    args.timestep_id = new_var.timestep_id;
    args.name = new_var.name;
    args.path = new_var.path;
    args.version = new_var.version;
    args.data_size = new_var.data_size;
    args.job_id = my_job_id;
    args.num_dims = new_var.num_dims;
    //extreme_debug_log << "run_id: " << new_var.run_id << " timestep_id: " << new_var.timestep_id << " var_id: " << args.var_id << " \n";

    args.dims = new_var.dims;
    //extreme_debug_log << "about to create new var \n";
    
    OpCreateVarMeta *op = new OpCreateVarMeta(server.peer_ptr, args);    
    //extreme_debug_log << "about to get future" <<endl;
    future<std::string> fut = op->GetFuture();
    //extreme_debug_log << "about to launch op" <<endl;
    opbox::LaunchOp(op);
    //extreme_debug_log << "about to get final result" <<endl;

    res = fut.get();

    uint64_t rowid; //todo
    var_id = args.var_id;
    deArchiveMsgFromServer(res, rowid, return_value); //todo
    add_timing_point(MD_CREATE_VAR_DONE); 

    return return_value;
} //end of funct



int metadata_delete_run_by_id (const md_server &server
                        ,uint64_t run_id
                        //,md_query_type query_type
                        )
{
    add_timing_point(MD_DELETE_RUN_BY_ID_START);

    int return_value;
    // int res;
    md_delete_run_by_id_args args;

    args.run_id = run_id;
    args.job_id = my_job_id;
    args.query_type = query_type;

    OpDeleteRunByIdMeta *op = new OpDeleteRunByIdMeta(server.peer_ptr, args);
    future<string> fut = op->GetFuture();
    opbox::LaunchOp(op);

    return_value=stoi(fut.get());

    // deArchiveMsgFromServer(res, return_value);
    add_timing_point(MD_DELETE_RUN_BY_ID_DONE);

    return return_value;
}

// // delete the specified run
// int metadata_delete_run_by_id (const md_server &server
//                         ,uint64_t run_id
//                         )
// {
//     add_timing_point(MD_DELETE_RUN_BY_ID_START);

//     int return_value;
//     // int res;
//     md_delete_run_by_id_args args;

//     args.run_id = run_id;
//     args.job_id = my_job_id; //fix

//     OpDeleteRunByIdMeta *op = new OpDeleteRunByIdMeta(server.peer_ptr, args);
//     future<string> fut = op->GetFuture();
//     opbox::LaunchOp(op);

//     return_value=stoi(fut.get());

//     // deArchiveMsgFromServer(res, return_value);
//     add_timing_point(MD_DELETE_RUN_BY_ID_DONE);

//     return return_value;
// } //end of funct


int metadata_delete_timestep_by_id (const md_server &server
                        ,uint64_t timestep_id
                        ,uint64_t run_id
                        //,md_query_type query_type
                        )
{
    add_timing_point(MD_DELETE_TIMESTEP_BY_ID_START);

    int return_value;
    // int res;
    md_delete_timestep_by_id_args args;

    args.timestep_id = timestep_id;
    args.run_id = run_id;
    args.job_id = my_job_id;
    args.query_type = query_type;

    OpDeleteTimestepByIdMeta *op = new OpDeleteTimestepByIdMeta(server.peer_ptr, args);
    future<string> fut = op->GetFuture();
    opbox::LaunchOp(op);

    return_value=stoi(fut.get());

    // deArchiveMsgFromServer(res, return_value);
    add_timing_point(MD_DELETE_TIMESTEP_BY_ID_DONE);

    return return_value;
} //end of funct

// // delete the specified timestep
// int metadata_delete_timestep_by_id (const md_server &server
//                         ,uint64_t timestep_id
//                         ,uint64_t run_id
//                         )
// {
//     add_timing_point(MD_DELETE_TIMESTEP_BY_ID_START);

//     int return_value;
//     // int res;
//     md_delete_timestep_by_id_args args;

//     args.timestep_id = timestep_id;
//     args.run_id = run_id;
//     args.job_id = my_job_id; //fix

//     OpDeleteTimestepByIdMeta *op = new OpDeleteTimestepByIdMeta(server.peer_ptr, args);
//     future<string> fut = op->GetFuture();
//     opbox::LaunchOp(op);

//     return_value=stoi(fut.get());

//     // deArchiveMsgFromServer(res, return_value);
//     add_timing_point(MD_DELETE_TIMESTEP_BY_ID_DONE);

//     return return_value;
// } //end of funct

// // delete the specified dataset
// int metadata_delete_dataset_by_name_path_timestep (const md_server &server
//                         ,const std::string &name
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

//     OpDeleteDatasetByNamePathTimestepMeta *op = new OpDeleteDatasetByNamePathTimestepMeta(server.peer_ptr, args);
//     future<std::string> fut = op->GetFuture();
//     opbox::LaunchOp(op);

//     res=fut.get();

//     deArchiveMsgFromServer(res, return_value);
//     add_timing_point(MD_DELETE_DATASET_BY_NAME_PATH_TIMESTEP_DONE);

//     return return_value;
// }

int metadata_delete_type_by_id (const md_server &server
                        // ,uint64_t dataset_id
                        ,uint64_t type_id
                        //,md_query_type query_type
                        )
{
    add_timing_point(MD_DELETE_TYPE_BY_ID_START);

    int return_value;
    // int res;
    md_delete_type_by_id_args args;

    // args.dataset_id = dataset_id;
    args.type_id = type_id;
    args.job_id = my_job_id;
    args.query_type = query_type;

    OpDeleteTypeByIdMeta *op = new OpDeleteTypeByIdMeta(server.peer_ptr, args);
    future<string> fut = op->GetFuture();
    opbox::LaunchOp(op);

    return_value=stoi(fut.get());

    // deArchiveMsgFromServer(res, return_value);
    add_timing_point(MD_DELETE_TYPE_BY_ID_DONE);

    return return_value;
} //end of funct

// // delete the given type and all of the attributes associated with it
// int metadata_delete_type_by_id (const md_server &server
//                         // ,uint64_t dataset_id
//                         ,uint64_t type_id
//                         )
// {
//     add_timing_point(MD_DELETE_TYPE_BY_ID_START);

//     int return_value;
//     // int res;
//     md_delete_type_by_id_args args;

//     // args.dataset_id = dataset_id;
//     args.type_id = type_id;
//     args.job_id = my_job_id; //fix

//     OpDeleteTypeByIdMeta *op = new OpDeleteTypeByIdMeta(server.peer_ptr, args);
//     future<string> fut = op->GetFuture();
//     opbox::LaunchOp(op);

//     return_value=stoi(fut.get());

//     // deArchiveMsgFromServer(res, return_value);
//     add_timing_point(MD_DELETE_TYPE_BY_ID_DONE);

//     return return_value;
// } //end of funct

// delete the given type and all of the attributes associated with it
int metadata_delete_type_by_name_ver (const md_server &server
                        ,uint64_t run_id
                        ,const std::string &name
                        ,uint32_t version
                        //,md_query_type query_type
                        )
{
    add_timing_point(MD_DELETE_TYPE_BY_NAME_VER_START);

    int return_value;
    // int res;
    md_delete_type_by_name_ver_args args;

    args.run_id = run_id;
    args.name = name;
    args.version = version;
    args.job_id = my_job_id;
    args.query_type = query_type;

    OpDeleteTypeByNameVerMeta *op = new OpDeleteTypeByNameVerMeta(server.peer_ptr, args);
    future<string> fut = op->GetFuture();
    opbox::LaunchOp(op);

    return_value=stoi(fut.get());

    // deArchiveMsgFromServer(res, return_value);
    add_timing_point(MD_DELETE_TYPE_BY_NAME_VER_DONE);

    return return_value;
} //end of funct

// // delete the given type and all of the attributes associated with it
// int metadata_delete_type_by_name_ver (const md_server &server
//                         ,uint64_t run_id
//                         ,const std::string &name
//                         ,uint32_t version
//                         )
// {
//     add_timing_point(MD_DELETE_TYPE_BY_NAME_VER_START);

//     int return_value;
//     // int res;
//     md_delete_type_by_name_ver_args args;

//     args.run_id = run_id;
//     args.name = name;
//     args.version = version;
//     args.job_id = my_job_id; //fix

//     OpDeleteTypeByNameVerMeta *op = new OpDeleteTypeByNameVerMeta(server.peer_ptr, args);
//     future<string> fut = op->GetFuture();
//     opbox::LaunchOp(op);

//     return_value=stoi(fut.get());

//     // deArchiveMsgFromServer(res, return_value);
//     add_timing_point(MD_DELETE_TYPE_BY_NAME_VER_DONE);

//     return return_value;
// } //end of funct

// delete the specified var
int metadata_delete_var_by_id (const md_server &server
                        ,uint64_t run_id
                        ,uint64_t timestep_id
                        ,uint64_t var_id
                        //,md_query_type query_type
                        )
{
    add_timing_point(MD_DELETE_VAR_BY_ID_START);

    int return_value;
    // int res;
    md_delete_var_by_id_args args;

    args.run_id = run_id;
    args.timestep_id = timestep_id;
    args.var_id = var_id;
    args.job_id = my_job_id;
    args.query_type = query_type;

    OpDeleteVarByIdMeta *op = new OpDeleteVarByIdMeta(server.peer_ptr, args);
    future<string> fut = op->GetFuture();
    opbox::LaunchOp(op);

    return_value=stoi(fut.get());

    // deArchiveMsgFromServer(res, return_value);
    add_timing_point(MD_DELETE_VAR_BY_ID_DONE);

    return return_value;
} //end of funct

// // delete the specified var
// int metadata_delete_var_by_id (const md_server &server
//                         ,uint64_t run_id
//                         ,uint64_t timestep_id
//                         ,uint64_t var_id
//                         )
// {
//     add_timing_point(MD_DELETE_VAR_BY_ID_START);

//     int return_value;
//     // int res;
//     md_delete_var_by_id_args args;

//     args.run_id = run_id;
//     args.timestep_id = timestep_id;
//     args.var_id = var_id;
//     args.job_id = my_job_id; //fix

//     OpDeleteVarByIdMeta *op = new OpDeleteVarByIdMeta(server.peer_ptr, args);
//     future<string> fut = op->GetFuture();
//     opbox::LaunchOp(op);

//     return_value=stoi(fut.get());

//     // deArchiveMsgFromServer(res, return_value);
//     add_timing_point(MD_DELETE_VAR_BY_ID_DONE);

//     return return_value;
// } //end of funct

// delete the specified var
int metadata_delete_var_by_name_path_ver (const md_server &server
                        ,uint64_t run_id
                        ,uint64_t timestep_id
                        ,const std::string &name
                        ,const std::string &path
                        ,uint32_t version
                        //,md_query_type query_type
                        )
{
    add_timing_point(MD_DELETE_VAR_BY_NAME_PATH_VER_START);

    int return_value;
    // int res;
    md_delete_var_by_name_path_ver_args args;

    args.run_id = run_id;
    args.timestep_id = timestep_id;
    args.name = name;
    args.path = path;
    args.version = version;
    args.job_id = my_job_id;
    args.query_type = query_type;

    OpDeleteVarByNamePathVerMeta *op = new OpDeleteVarByNamePathVerMeta(server.peer_ptr, args);
    future<string> fut = op->GetFuture();
    opbox::LaunchOp(op);

    return_value=stoi(fut.get());

    // deArchiveMsgFromServer(res, return_value);
    add_timing_point(MD_DELETE_VAR_BY_NAME_PATH_VER_DONE);

    return return_value;
} //end of funct

// // delete the specified var
// int metadata_delete_var_by_name_path_ver (const md_server &server
//                         ,uint64_t run_id
//                         ,uint64_t timestep_id
//                         ,const std::string &name
//                         ,const std::string &path
//                         ,uint32_t version
//                         )
// {
//     add_timing_point(MD_DELETE_VAR_BY_NAME_PATH_VER_START);

//     int return_value;
//     // int res;
//     md_delete_var_by_name_path_ver_args args;

//     args.run_id = run_id;
//     args.timestep_id = timestep_id;
//     args.name = name;
//     args.path = path;
//     args.version = version;
//     args.job_id = my_job_id; //fix

//     OpDeleteVarByNamePathVerMeta *op = new OpDeleteVarByNamePathVerMeta(server.peer_ptr, args);
//     future<string> fut = op->GetFuture();
//     opbox::LaunchOp(op);

//     return_value=stoi(fut.get());

//     // deArchiveMsgFromServer(res, return_value);
//     add_timing_point(MD_DELETE_VAR_BY_NAME_PATH_VER_DONE);

//     return return_value;
// } //end of funct



// insert a attribute of a bounding box
int metadata_insert_run_attribute (const md_server &server
                           ,uint64_t &attribute_id
                           ,uint64_t run_id
                           ,uint64_t type_id
                           ,uint64_t txn_id
                           // //,md_query_type query_type
                           ,attr_data_type data_type
                           ,const std::string &data
                           )
{
    add_timing_point(MD_INSERT_RUN_ATTRIBUTE_START);
    int return_value;
    std::string res;

    // if(time_pts.size() != catg_of_time_pts.size()) {
    //     //debug_log << "error they are off at start of md insert attributes " << endl;
    // }
    md_insert_run_attribute_args args;
    args.run_id = run_id;
    args.type_id = type_id;
    args.job_id = my_job_id;
    // args.query_type = query_type;
    args.data_type = data_type;
    args.data = data;

    OpInsertRunAttributeMeta *op = new OpInsertRunAttributeMeta(server.peer_ptr, args);
    future<std::string> fut = op->GetFuture();
    opbox::LaunchOp(op);

    res=fut.get();

    deArchiveMsgFromServer(res, attribute_id, return_value);
    add_timing_point(MD_INSERT_RUN_ATTRIBUTE_DONE);   
    //debug_log << "according to the client the attribute_id is " << attribute_id << endl;
    // if(time_pts.size() != catg_of_time_pts.size()) {
    //     //debug_log << "error they are off at the end of md insert attributes " << endl;
    // }
    return return_value;
} //end of funct

int metadata_insert_run_attribute (const md_server &server
                           ,uint64_t &attribute_id
                           ,const md_catalog_run_attribute_entry &new_attribute)
{
    add_timing_point(MD_INSERT_RUN_ATTRIBUTE_START);
    int return_value;
    std::string res;

    // if(time_pts.size() != catg_of_time_pts.size()) {
    //     //debug_log << "error they are off at start of md insert attributes " << endl;
    // }
    md_insert_run_attribute_args args;
    args.run_id = new_attribute.run_id;
    args.type_id = new_attribute.type_id;
    args.job_id = my_job_id;
    args.data_type = new_attribute.data_type;
    args.data = new_attribute.data;

    OpInsertRunAttributeMeta *op = new OpInsertRunAttributeMeta(server.peer_ptr, args);
    future<std::string> fut = op->GetFuture();
    opbox::LaunchOp(op);

    res=fut.get();

    deArchiveMsgFromServer(res, attribute_id, return_value);
    add_timing_point(MD_INSERT_RUN_ATTRIBUTE_DONE);   
    //debug_log << "according to the client the attribute_id is " << attribute_id << endl;
    // if(time_pts.size() != catg_of_time_pts.size()) {
    //     //debug_log << "error they are off at the end of md insert attributes " << endl;
    // }
    return return_value;
} //end of funct


// insert a attribute of a bounding box
int metadata_insert_timestep_attribute (const md_server &server
                           ,uint64_t &attribute_id
                           ,uint64_t run_id
                           ,uint64_t timestep_id
                           ,uint64_t type_id
                           ,uint64_t txn_id
                           // //,md_query_type query_type
                           ,attr_data_type data_type
                           ,const std::string &data
                           )
{
    add_timing_point(MD_INSERT_TIMESTEP_ATTRIBUTE_START);
    int return_value;
    std::string res;

    // if(time_pts.size() != catg_of_time_pts.size()) {
    //     //debug_log << "error they are off at start of md insert attributes " << endl;
    // }
    md_insert_timestep_attribute_args args;
    args.run_id = run_id;
    args.timestep_id = timestep_id;
    args.type_id = type_id;
    args.job_id = my_job_id;
    // args.query_type = query_type;
    args.data_type = data_type;
    args.data = data;

    OpInsertTimestepAttributeMeta *op = new OpInsertTimestepAttributeMeta(server.peer_ptr, args);
    future<std::string> fut = op->GetFuture();
    opbox::LaunchOp(op);

    res=fut.get();

    deArchiveMsgFromServer(res, attribute_id, return_value);
    add_timing_point(MD_INSERT_TIMESTEP_ATTRIBUTE_DONE);   
    //debug_log << "according to the client the attribute_id is " << attribute_id << endl;
    // if(time_pts.size() != catg_of_time_pts.size()) {
    //     //debug_log << "error they are off at the end of md insert attributes " << endl;
    // }
    return return_value;
} //end of funct

int metadata_insert_timestep_attribute (const md_server &server,
                           uint64_t &attribute_id,
                           const md_catalog_timestep_attribute_entry &new_attribute)
{
    add_timing_point(MD_INSERT_TIMESTEP_ATTRIBUTE_START);
    int return_value;
    std::string res;

    // if(time_pts.size() != catg_of_time_pts.size()) {
    //     //debug_log << "error they are off at start of md insert attributes " << endl;
    // }
    md_insert_timestep_attribute_args args;
    args.run_id = new_attribute.run_id;
    args.timestep_id = new_attribute.timestep_id;
    args.type_id = new_attribute.type_id;
    args.job_id = my_job_id;
    args.data_type = new_attribute.data_type;
    args.data = new_attribute.data;

    OpInsertTimestepAttributeMeta *op = new OpInsertTimestepAttributeMeta(server.peer_ptr, args);
    future<std::string> fut = op->GetFuture();
    opbox::LaunchOp(op);

    res=fut.get();

    deArchiveMsgFromServer(res, attribute_id, return_value);
    add_timing_point(MD_INSERT_TIMESTEP_ATTRIBUTE_DONE);   
    //debug_log << "according to the client the attribute_id is " << attribute_id << endl;
    // if(time_pts.size() != catg_of_time_pts.size()) {
    //     //debug_log << "error they are off at the end of md insert attributes " << endl;
    // }
    return return_value;
} //end of funct


// insert a attribute of a bounding box
int metadata_insert_var_attribute_by_dims (const md_server &server
                           ,uint64_t &attribute_id
                           ,uint64_t run_id
                           ,uint64_t timestep_id
                           ,uint64_t type_id
                           ,uint64_t var_id
                           ,uint64_t txn_id
                           // //,md_query_type query_type
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
    //     //debug_log << "error they are off at start of md insert attributes " << endl;
    // }
    md_insert_var_attribute_by_dims_args args;
    args.run_id = run_id;
    args.timestep_id = timestep_id;
    args.type_id = type_id;
    args.var_id = var_id;
    args.job_id = my_job_id;
    // args.query_type = query_type;
    args.num_dims = num_dims;
    args.dims = dims;
    args.data_type = data_type;
    args.data = data;

    OpInsertVarAttributeByDimsMeta *op = new OpInsertVarAttributeByDimsMeta(server.peer_ptr, args);
    future<std::string> fut = op->GetFuture();
    opbox::LaunchOp(op);

    res=fut.get();

    deArchiveMsgFromServer(res, attribute_id, return_value);
    add_timing_point(MD_INSERT_VAR_ATTRIBUTE_BY_DIMS_DONE);   
    //debug_log << "according to the client the attribute_id is " << attribute_id << endl;
    // if(time_pts.size() != catg_of_time_pts.size()) {
    //     //debug_log << "error they are off at the end of md insert attributes " << endl;
    // }
    return return_value;
} //end of funct

int metadata_insert_var_attribute_by_dims (const md_server &server
                           ,uint64_t &attribute_id
                           ,const md_catalog_var_attribute_entry &new_attribute)
{
    add_timing_point(MD_INSERT_VAR_ATTRIBUTE_BY_DIMS_START);
    int return_value;
    std::string res;

    // if(time_pts.size() != catg_of_time_pts.size()) {
    //     //debug_log << "error they are off at start of md insert attributes " << endl;
    // }
    md_insert_var_attribute_by_dims_args args;
    args.run_id = new_attribute.run_id;
    args.timestep_id = new_attribute.timestep_id;
    args.type_id = new_attribute.type_id;
    args.var_id = new_attribute.var_id;
    args.job_id = my_job_id;
    args.num_dims = new_attribute.num_dims;
    args.dims = new_attribute.dims;
    args.data_type = new_attribute.data_type;
    args.data = new_attribute.data;

    OpInsertVarAttributeByDimsMeta *op = new OpInsertVarAttributeByDimsMeta(server.peer_ptr, args);
    future<std::string> fut = op->GetFuture();
    opbox::LaunchOp(op);

    res=fut.get();

    deArchiveMsgFromServer(res, attribute_id, return_value);
    add_timing_point(MD_INSERT_VAR_ATTRIBUTE_BY_DIMS_DONE);   
    //debug_log << "according to the client the attribute_id is " << attribute_id << endl;
    // if(time_pts.size() != catg_of_time_pts.size()) {
    //     //debug_log << "error they are off at the end of md insert attributes " << endl;
    // }
    return return_value;
} //end of funct

// mark a run_attributeiable in the global_catalog as processing making it invisible to
// other processes.
int metadata_processing_run_attribute (const md_server &server
                          ,uint64_t txn_id
                          // //,md_query_type query_type
                          )
{
    int return_value;
    
    return_value = metadata_processing ( server, txn_id, RUN_ATTR_CATALOG);

    return return_value;
} //end of funct


// mark a runiable in the global_catalog as processing making it invisible to
// other processes.
int metadata_processing_run (const md_server &server
                          ,uint64_t txn_id
                          // //,md_query_type query_type
                          )
{
    int return_value;
    
    return_value = metadata_processing ( server, txn_id, RUN_CATALOG);

    return return_value;
} //end of funct

// mark a timestep_attributeiable in the global_catalog as processing making it invisible to
// other processes.
int metadata_processing_timestep_attribute (const md_server &server
                          ,uint64_t txn_id
                          // //,md_query_type query_type
                          )
{
    int return_value;
    
    return_value = metadata_processing ( server, txn_id, TIMESTEP_ATTR_CATALOG);

    return return_value;
} //end of funct


// mark a timestep table in the global_catalog as processing making it invisible to
// other processes.
int metadata_processing_timestep (const md_server &server
                          ,uint64_t txn_id
                          // //,md_query_type query_type
                          )
{
    int return_value;
    
    return_value = metadata_processing ( server, txn_id, TIMESTEP_CATALOG);

    return return_value;
} //end of funct


// mark a variable in the global_catalog as processing making it invisible to
// other processes.
int metadata_processing_type (const md_server &server
                          ,uint64_t txn_id
                          // //,md_query_type query_type
                          )
{
    int return_value;
    
    return_value = metadata_processing ( server, txn_id, TYPE_CATALOG);

    return return_value;
} //end of funct

// mark all var_attributes from the given transaction as processing making it invisible to
// other processes.
int metadata_processing_var_attribute  (const md_server &server
                          ,uint64_t txn_id
                          // //,md_query_type query_type
                          )
{
    int return_value;
    
    return_value = metadata_processing ( server, txn_id, VAR_ATTR_CATALOG);

    return return_value;
} //end of funct

// mark all vars from the given transaction as processing making it invisible to
// other processes.
int metadata_processing_var  (const md_server &server
                          ,uint64_t txn_id
                          // //,md_query_type query_type
                          )
{
    int return_value;
    
    return_value = metadata_processing ( server, txn_id, VAR_CATALOG);

    return return_value;
} //end of funct



static int metadata_catalog_all_run_attributes_with_type_val (const md_server &server
                                        ,data_range_type type
                                        ,uint64_t run_id
                                        ,uint64_t type_id             
                                        ,uint64_t txn_id
                                        // ,md_query_type query_type
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
    args.job_id = my_job_id;
    args.query_type = query_type;
    args.data_type = data_type;
    args.data = data;
    // //extreme_debug_log << "run_id: " << run_id << " type_id: " << type_id << " txn_id: " << txn_id << " data_type: " << data_type <<
    //     " data: " << data << endl; 

    //extreme_debug_log << "txn_id is " << to_string(txn_id ) << endl;  
   
    OpCatalogAllRunAttributesWithTypeRangeMeta *op = new OpCatalogAllRunAttributesWithTypeRangeMeta(server.peer_ptr, args);
    future<std::string> fut = op->GetFuture();
    opbox::LaunchOp(op);

    res=fut.get();

    deArchiveMsgFromServer(res, entries, count, return_value);
    add_timing_point(MD_CATALOG_ALL_RUN_ATTRIBUTES_WITH_TYPE_RANGE_DONE);  

    return return_value;
} //end of funct

int metadata_catalog_all_run_attributes_with_type_range_in_run (const md_server &server
                                        ,uint64_t run_id
                                        ,uint64_t type_id             
                                        ,uint64_t txn_id
                                        //,md_query_type query_type
                                        ,attr_data_type data_type
                                        ,const std::string &data
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_run_attribute_entry> &entries
                                        )
{
    int return_value;

    return_value = metadata_catalog_all_run_attributes_with_type_val (server, DATA_RANGE, run_id, type_id, txn_id, data_type, data, count, entries);

    return return_value;
} //end of funct

int metadata_catalog_all_run_attributes_with_type_above_max_in_run (const md_server &server
                                        ,uint64_t run_id
                                        ,uint64_t type_id             
                                        ,uint64_t txn_id
                                        //,md_query_type query_type
                                        ,attr_data_type data_type
                                        ,const std::string &data
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_run_attribute_entry> &entries
                                        )
{
    int return_value;

    return_value = metadata_catalog_all_run_attributes_with_type_val (server, DATA_MAX, run_id, type_id, txn_id, data_type, data, count, entries);

    return return_value;
} //end of funct

int metadata_catalog_all_run_attributes_with_type_below_min_in_run (const md_server &server
                                        ,uint64_t run_id
                                        ,uint64_t type_id             
                                        ,uint64_t txn_id
                                        //,md_query_type query_type
                                        ,attr_data_type data_type
                                        ,const std::string &data
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_run_attribute_entry> &entries
                                        )
{
    int return_value;

    return_value = metadata_catalog_all_run_attributes_with_type_val (server, DATA_MIN, run_id, type_id, txn_id, data_type, data, count, entries);

    return return_value;
} //end of funct

static int metadata_catalog_all_timesteps_with_var_attributes_with_type_var_val (const md_server &server
                                        ,data_range_type type
                                        ,uint64_t run_id             
                                        ,uint64_t type_id
                                        ,uint64_t var_id             
                                        ,uint64_t txn_id
                                        //,md_query_type query_type
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
    args.job_id = my_job_id;
    args.query_type = query_type;
    args.data_type = data_type;
    args.data = data;
    //extreme_debug_log << "txn_id is " << to_string(txn_id ) << endl;  
   
    OpCatalogAllTimestepsWithVarAttributesWithTypeVarRangeMeta *op = new OpCatalogAllTimestepsWithVarAttributesWithTypeVarRangeMeta(server.peer_ptr, args);
    future<std::string> fut = op->GetFuture();
    opbox::LaunchOp(op);

    res=fut.get();

    deArchiveMsgFromServer(res, entries, count, return_value);
    add_timing_point(MD_CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_RANGE_DONE);  

    return return_value;
} //end of funct


int metadata_catalog_all_timesteps_with_var_attributes_with_type_var_range (const md_server &server
                                        ,uint64_t run_id             
                                        ,uint64_t type_id
                                        ,uint64_t var_id             
                                        ,uint64_t txn_id
                                        //,md_query_type query_type
                                        ,attr_data_type data_type
                                        ,const std::string &data                                        
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_timestep_entry> &entries
                                        )
{
    int return_value;

    return_value = metadata_catalog_all_timesteps_with_var_attributes_with_type_var_val (server, DATA_RANGE, run_id, type_id,
        var_id, txn_id, data_type, data, count, entries);

    return return_value;
} //end of funct

int metadata_catalog_all_timesteps_with_var_attributes_with_type_var_above_max (const md_server &server
                                        ,uint64_t run_id             
                                        ,uint64_t type_id
                                        ,uint64_t var_id             
                                        ,uint64_t txn_id
                                        //,md_query_type query_type
                                        ,attr_data_type data_type
                                        ,const std::string &data                                        
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_timestep_entry> &entries
                                        )
{
    int return_value;

    return_value = metadata_catalog_all_timesteps_with_var_attributes_with_type_var_val (server, DATA_MAX, run_id, type_id,
        var_id, txn_id, data_type, data, count, entries);

    return return_value;
} //end of funct

int metadata_catalog_all_timesteps_with_var_attributes_with_type_var_below_min (const md_server &server
                                        ,uint64_t run_id             
                                        ,uint64_t type_id
                                        ,uint64_t var_id             
                                        ,uint64_t txn_id
                                        //,md_query_type query_type
                                        ,attr_data_type data_type
                                        ,const std::string &data                                        
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_timestep_entry> &entries
                                        )
{
    int return_value;

    return_value = metadata_catalog_all_timesteps_with_var_attributes_with_type_var_val (server, DATA_MIN, run_id, type_id,
        var_id, txn_id, data_type, data, count, entries);

    return return_value;
} //end of funct

static int metadata_catalog_all_timesteps_with_var_attributes_with_type_var_dims_val (const md_server &server
                                        ,data_range_type type
                                        ,uint64_t run_id             
                                        ,uint64_t type_id
                                        ,uint64_t var_id             
                                        ,uint64_t txn_id
                                        //,md_query_type query_type
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
    args.job_id = my_job_id;
    args.query_type = query_type;
    args.num_dims = num_dims;
    args.dims = dims;
    args.data_type = data_type;
    args.data = data;
    //extreme_debug_log << "txn_id is " << to_string(txn_id ) << endl;  
   
    OpCatalogAllTimestepsWithVarAttributesWithTypeVarDimsRangeMeta *op = new OpCatalogAllTimestepsWithVarAttributesWithTypeVarDimsRangeMeta(server.peer_ptr, args);
    future<std::string> fut = op->GetFuture();
    opbox::LaunchOp(op);

    res=fut.get();

    deArchiveMsgFromServer(res, entries, count, return_value);
    add_timing_point(MD_CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_DIMS_RANGE_DONE);  

    return return_value;
} //end of funct

int metadata_catalog_all_timesteps_with_var_attributes_with_type_var_dims_range (const md_server &server
                                        ,uint64_t run_id             
                                        ,uint64_t type_id
                                        ,uint64_t var_id             
                                        ,uint64_t txn_id
                                        //,md_query_type query_type
                                        ,uint32_t num_dims
                                        ,const std::vector<md_dim_bounds> &dims
                                        ,attr_data_type data_type
                                        ,const std::string &data
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_timestep_entry> &entries
                                        )
{
    int return_value;

    return_value = metadata_catalog_all_timesteps_with_var_attributes_with_type_var_dims_val (server, DATA_RANGE,
        run_id, type_id, var_id, txn_id, num_dims, dims, data_type, data, count, entries);

    return return_value;
} //end of funct

int metadata_catalog_all_timesteps_with_var_attributes_with_type_var_dims_above_max (const md_server &server
                                        ,uint64_t run_id             
                                        ,uint64_t type_id
                                        ,uint64_t var_id             
                                        ,uint64_t txn_id
                                        //,md_query_type query_type
                                        ,uint32_t num_dims
                                        ,const std::vector<md_dim_bounds> &dims
                                        ,attr_data_type data_type
                                        ,const std::string &data
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_timestep_entry> &entries
                                        )
{
    int return_value;

    return_value = metadata_catalog_all_timesteps_with_var_attributes_with_type_var_dims_val (server, DATA_MAX,
        run_id, type_id, var_id, txn_id, num_dims, dims, data_type, data, count, entries);

    return return_value;
} //end of funct

int metadata_catalog_all_timesteps_with_var_attributes_with_type_var_dims_below_min (const md_server &server
                                        ,uint64_t run_id             
                                        ,uint64_t type_id
                                        ,uint64_t var_id             
                                        ,uint64_t txn_id
                                        //,md_query_type query_type
                                        ,uint32_t num_dims
                                        ,const std::vector<md_dim_bounds> &dims
                                        ,attr_data_type data_type
                                        ,const std::string &data
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_timestep_entry> &entries
                                        )
{
    int return_value;

    return_value = metadata_catalog_all_timesteps_with_var_attributes_with_type_var_dims_val (server, DATA_MIN,
        run_id, type_id, var_id, txn_id, num_dims, dims, data_type, data, count, entries);

    return return_value;
} //end of funct

static int metadata_catalog_all_timestep_attributes_with_type_val (const md_server &server
                                        ,data_range_type type
                                        ,uint64_t run_id             
                                        ,uint64_t timestep_id
                                        ,uint64_t type_id             
                                        ,uint64_t txn_id
                                        //,md_query_type query_type
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
    args.job_id = my_job_id;
    args.query_type = query_type;
    args.data_type = data_type;
    args.data = data;
    //extreme_debug_log << "txn_id is " << to_string(txn_id ) << endl;  
   
    OpCatalogAllTimestepAttributesWithTypeRangeMeta *op = new OpCatalogAllTimestepAttributesWithTypeRangeMeta(server.peer_ptr, args);
    future<std::string> fut = op->GetFuture();
    opbox::LaunchOp(op);

    res=fut.get();

    deArchiveMsgFromServer(res, entries, count, return_value);
    add_timing_point(MD_CATALOG_ALL_TIMESTEP_ATTRIBUTES_WITH_TYPE_RANGE_DONE);  

    return return_value;
} //end of funct

int metadata_catalog_all_timestep_attributes_with_type_range_in_timestep (const md_server &server
                                        ,uint64_t run_id             
                                        ,uint64_t timestep_id
                                        ,uint64_t type_id             
                                        ,uint64_t txn_id
                                        //,md_query_type query_type
                                        ,attr_data_type data_type
                                        ,const std::string &data
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_timestep_attribute_entry> &entries
                                        )
{
    int return_value;

    return_value = metadata_catalog_all_timestep_attributes_with_type_val (server, DATA_RANGE, run_id,
        timestep_id, type_id, txn_id, data_type, data, count, entries);

    return return_value;
}

int metadata_catalog_all_timestep_attributes_with_type_above_max_in_timestep (const md_server &server
                                        ,uint64_t run_id             
                                        ,uint64_t timestep_id
                                        ,uint64_t type_id             
                                        ,uint64_t txn_id
                                        //,md_query_type query_type
                                        ,attr_data_type data_type
                                        ,const std::string &data
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_timestep_attribute_entry> &entries
                                        )
{
    int return_value;

    return_value = metadata_catalog_all_timestep_attributes_with_type_val (server, DATA_MAX, run_id,
        timestep_id, type_id, txn_id, data_type, data, count, entries);
    
    return return_value;
}

int metadata_catalog_all_timestep_attributes_with_type_below_min_in_timestep (const md_server &server
                                        ,uint64_t run_id             
                                        ,uint64_t timestep_id
                                        ,uint64_t type_id             
                                        ,uint64_t txn_id
                                        //,md_query_type query_type
                                        ,attr_data_type data_type
                                        ,const std::string &data
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_timestep_attribute_entry> &entries
                                        )
{
    int return_value;

    return_value = metadata_catalog_all_timestep_attributes_with_type_val (server, DATA_MIN, run_id,
        timestep_id, type_id, txn_id, data_type, data, count, entries);

    return return_value;
}

static int metadata_catalog_all_var_attributes_with_type_var_val (const md_server &server
                                        ,data_range_type type
                                        ,uint64_t timestep_id
                                        ,uint64_t type_id
                                        ,uint64_t var_id
                                        ,uint64_t txn_id
                                        //,md_query_type query_type
                                        ,attr_data_type data_type
                                        ,const std::string &data
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_var_attribute_entry> &matching_attributes
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
    args.job_id = my_job_id;
    args.query_type = query_type;
    args.data_type = data_type;
    args.data = data;

    OpCatalogAllVarAttributesWithTypeVarRangeMeta *op = new OpCatalogAllVarAttributesWithTypeVarRangeMeta(server.peer_ptr, args);
    //extreme_debug_log << "just launched get attr by dims \n";
    future<std::string> fut = op->GetFuture();
    opbox::LaunchOp(op);

    res=fut.get();    

    deArchiveMsgFromServer(res, matching_attributes, count, return_value);
    add_timing_point(MD_CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_RANGE_DONE);   
    //extreme_debug_log << "just dearchived matching attributes by dims message from server \n";
    //extreme_debug_log << "number of matching_attributes is " << matching_attributes.size() << "\n";


    return return_value;
} //end of funct


int metadata_catalog_all_var_attributes_with_type_var_range (const md_server &server
                                        ,uint64_t timestep_id
                                        ,uint64_t type_id
                                        ,uint64_t var_id
                                        ,uint64_t txn_id
                                        //,md_query_type query_type
                                        ,attr_data_type data_type
                                        ,const std::string &data
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_var_attribute_entry> &matching_attributes
                                        )
{
    int return_value;

    return_value = metadata_catalog_all_var_attributes_with_type_var_val (server, DATA_RANGE, timestep_id,   
        type_id, var_id, txn_id, data_type, data, count, matching_attributes);

    return return_value;
} //end of funct

int metadata_catalog_all_var_attributes_with_type_var_above_max (const md_server &server
                                        ,uint64_t timestep_id
                                        ,uint64_t type_id
                                        ,uint64_t var_id
                                        ,uint64_t txn_id
                                        //,md_query_type query_type
                                        ,attr_data_type data_type
                                        ,const std::string &data
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_var_attribute_entry> &matching_attributes
                                        )
{
    int return_value;

    return_value = metadata_catalog_all_var_attributes_with_type_var_val (server, DATA_MAX, timestep_id,   
        type_id, var_id, txn_id, data_type, data, count, matching_attributes);

    return return_value;
} //end of funct

int metadata_catalog_all_var_attributes_with_type_var_below_min (const md_server &server
                                        ,uint64_t timestep_id
                                        ,uint64_t type_id
                                        ,uint64_t var_id
                                        ,uint64_t txn_id
                                        //,md_query_type query_type
                                        ,attr_data_type data_type
                                        ,const std::string &data
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_var_attribute_entry> &matching_attributes
                                        )
{
    int return_value;

    return_value = metadata_catalog_all_var_attributes_with_type_var_val (server, DATA_MIN, timestep_id,   
        type_id, var_id, txn_id, data_type, data, count, matching_attributes);

    return return_value;
} //end of funct

static int metadata_catalog_all_var_attributes_with_type_var_dims_val (const md_server &server
                                        ,data_range_type type
                                        ,uint64_t timestep_id
                                        ,uint64_t type_id
                                        ,uint64_t var_id
                                        ,uint64_t txn_id
                                        //,md_query_type query_type
                                        ,uint32_t num_dims
                                        ,const std::vector<md_dim_bounds> &dims
                                        ,attr_data_type data_type
                                        ,const std::string &data
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_var_attribute_entry> &matching_attributes
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
    args.job_id = my_job_id;
    args.query_type = query_type;
    args.num_dims = num_dims;
    args.dims = dims;
    args.data_type = data_type;
    args.data = data;
    OpCatalogAllVarAttributesWithTypeVarDimsRangeMeta *op = new OpCatalogAllVarAttributesWithTypeVarDimsRangeMeta(server.peer_ptr, args);
    //extreme_debug_log << "just launched get attr by dims \n";
    future<std::string> fut = op->GetFuture();
    opbox::LaunchOp(op);

    res=fut.get();    

    deArchiveMsgFromServer(res, matching_attributes, count, return_value);
    add_timing_point(MD_CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_DIMS_RANGE_DONE);   
    //extreme_debug_log << "just dearchived matching attributes by dims message from server \n";
    //extreme_debug_log << "number of matching_attributes is " << matching_attributes.size() << "\n";


    return return_value;
} //end of funct

int metadata_catalog_all_var_attributes_with_type_var_dims_range (const md_server &server
                                        ,uint64_t timestep_id
                                        ,uint64_t type_id
                                        ,uint64_t var_id
                                        ,uint64_t txn_id
                                        //,md_query_type query_type
                                        ,uint32_t num_dims
                                        ,const std::vector<md_dim_bounds> &dims
                                        ,attr_data_type data_type
                                        ,const std::string &data
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_var_attribute_entry> &matching_attributes
                                        )
{
    int return_value;

    return_value = metadata_catalog_all_var_attributes_with_type_var_dims_val (server, DATA_RANGE, timestep_id, type_id,
        var_id, txn_id, num_dims, dims, data_type, data, count, matching_attributes);

    return return_value;

} //end of funct



int metadata_catalog_all_var_attributes_with_type_var_dims_above_max (const md_server &server
                                        ,uint64_t timestep_id
                                        ,uint64_t type_id
                                        ,uint64_t var_id
                                        ,uint64_t txn_id
                                        //,md_query_type query_type
                                        ,uint32_t num_dims
                                        ,const std::vector<md_dim_bounds> &dims
                                        ,attr_data_type data_type
                                        ,const std::string &data
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_var_attribute_entry> &matching_attributes
                                        )
{
    int return_value;

    return_value = metadata_catalog_all_var_attributes_with_type_var_dims_val (server, DATA_MAX, timestep_id, type_id,
        var_id, txn_id, num_dims, dims, data_type, data, count, matching_attributes);
    return return_value;
} //end of funct

int metadata_catalog_all_var_attributes_with_type_var_dims_below_min (const md_server &server
                                        ,uint64_t timestep_id
                                        ,uint64_t type_id
                                        ,uint64_t var_id
                                        ,uint64_t txn_id
                                        //,md_query_type query_type
                                        ,uint32_t num_dims
                                        ,const std::vector<md_dim_bounds> &dims
                                        ,attr_data_type data_type
                                        ,const std::string &data
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_var_attribute_entry> &matching_attributes
                                        )
{
    int return_value;

    return_value = metadata_catalog_all_var_attributes_with_type_var_dims_val (server, DATA_MIN, timestep_id, type_id,
        var_id, txn_id, num_dims, dims, data_type, data, count, matching_attributes);

    return return_value;
} //end of funct



static int metadata_processing (const md_server &server
                          ,uint64_t txn_id
                          //,md_query_type query_type
                          , md_catalog_type catalog_type                          
                          )
{
    add_timing_point(MD_PROCESSING_START);

    int return_value;
    // int res;
    md_processing_args args;

    args.job_id = my_job_id;
    // args.query_type = query_type;
    args.catalog_type = catalog_type;

    OpProcessingMeta *op = new OpProcessingMeta(server.peer_ptr, args);
    future<string> fut = op->GetFuture();
    opbox::LaunchOp(op);

    return_value=stoi(fut.get());    

    // deArchiveMsgFromServer(res, return_value);
    add_timing_point(MD_PROCESSING_DONE);

    return return_value;
} //end of funct

int metadata_catalog_all_types_with_var_attributes_in_timestep (const md_server &server
                      ,uint64_t run_id
                      ,uint64_t timestep_id
                      ,uint64_t txn_id
                      //,md_query_type query_type
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
    args.job_id = my_job_id;
    args.query_type = query_type;

    OpCatalogAllTypesWithVarAttributesInTimestepMeta *op = new OpCatalogAllTypesWithVarAttributesInTimestepMeta(server.peer_ptr, args);
    future<std::string> fut = op->GetFuture();

    opbox::LaunchOp(op);

    res=fut.get();    

    deArchiveMsgFromServer(res, entries, count, return_value);
    add_timing_point(MD_CATALOG_ALL_TYPES_WITH_VAR_ATTRIBUTES_IN_TIMESTEP_DONE);   

    return return_value;
} //end of funct

int metadata_catalog_all_var_attributes_with_var_substr (const md_server &server
                                        ,uint64_t run_id             
                                        ,uint64_t timestep_id
                                        ,const std::string &var_name_substr
                                        ,uint64_t txn_id
                                        //,md_query_type query_type
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_var_attribute_entry> &matching_attributes
                                        )
{
    add_timing_point(MD_CATALOG_ALL_VAR_ATTRIBUTES_WITH_VAR_SUBSTR_START);

    int return_value;
    // std::string res;
    string res;
    md_catalog_all_var_attributes_with_var_substr_args args;

    args.job_id = my_job_id;
    args.query_type = query_type;
    args.run_id = run_id;
    args.timestep_id = timestep_id;
    args.var_name_substr.reserve(2 + var_name_substr.size());
    args.var_name_substr = "%";
    //args.var_name_substr += boost::algorithm::to_lower_copy(var_name_substr);
    args.var_name_substr += var_name_substr;
    args.var_name_substr += "%";
    OpCatalogAllVarAttributesWithVarSubstrMeta *op = new OpCatalogAllVarAttributesWithVarSubstrMeta(server.peer_ptr, args);
    //extreme_debug_log << "just launched get attr by dims \n";
    // future<std::string> fut = op->GetFuture();
    future<string> fut = op->GetFuture();    
    opbox::LaunchOp(op);

    res=fut.get();    

    deArchiveMsgFromServer(res, matching_attributes, count, return_value);
    add_timing_point(MD_CATALOG_ALL_VAR_ATTRIBUTES_WITH_VAR_SUBSTR_DONE);   
    //extreme_debug_log << "just dearchived matching attributes by dims message from server \n";
    //extreme_debug_log << "number of matching_attributes is " << matching_attributes.size() << "\n";


    return return_value;
} //end of funct

int metadata_catalog_all_timesteps_with_var_attributes_with_type_var_substr_dims (const md_server &server
                                        ,uint64_t run_id             
                                        ,uint64_t type_id
                                        ,const std::string &var_name_substr             
                                        ,uint64_t txn_id
                                        //,md_query_type query_type
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
    args.job_id = my_job_id;
    args.query_type = query_type;
    args.num_dims = num_dims;
    args.dims = dims;
    //extreme_debug_log << "txn_id is " << to_string(txn_id ) << endl;  
   
    OpCatalogAllTimestepsWithVarAttributesWithTypeVarSubstrDimsMeta *op = new OpCatalogAllTimestepsWithVarAttributesWithTypeVarSubstrDimsMeta(server.peer_ptr, args);
    future<std::string> fut = op->GetFuture();
    opbox::LaunchOp(op);

    res=fut.get();

    deArchiveMsgFromServer(res, entries, count, return_value);
    add_timing_point(MD_CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_DIMS_DONE);  

    return return_value;
} //end of funct


int metadata_catalog_all_timesteps_with_var_attributes_with_type_var_substr (const md_server &server
                                        ,uint64_t run_id             
                                        ,uint64_t type_id
                                        ,const std::string &var_name_substr             
                                        ,uint64_t txn_id
                                        //,md_query_type query_type
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
    args.job_id = my_job_id;
    args.query_type = query_type;
    //extreme_debug_log << "txn_id is " << to_string(txn_id ) << endl;  
   
    OpCatalogAllTimestepsWithVarAttributesWithTypeVarSubstrMeta *op = new OpCatalogAllTimestepsWithVarAttributesWithTypeVarSubstrMeta(server.peer_ptr, args);
    future<std::string> fut = op->GetFuture();
    opbox::LaunchOp(op);

    res=fut.get();

    deArchiveMsgFromServer(res, entries, count, return_value);
    add_timing_point(MD_CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_DONE);  

    return return_value;
} //end of funct

int metadata_catalog_all_timesteps_with_var_substr (const md_server &server
                                        ,uint64_t run_id             
                                        ,const std::string &var_name_substr             
                                        ,uint64_t txn_id
                                        //,md_query_type query_type
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
    args.job_id = my_job_id;
    args.query_type = query_type;
    //extreme_debug_log << "txn_id is " << to_string(txn_id ) << endl;  
   
    OpCatalogAllTimestepsWithVarSubstrMeta *op = new OpCatalogAllTimestepsWithVarSubstrMeta(server.peer_ptr, args);
    future<std::string> fut = op->GetFuture();
    opbox::LaunchOp(op);

    res=fut.get();

    deArchiveMsgFromServer(res, entries, count, return_value);
    add_timing_point(MD_CATALOG_ALL_TIMESTEPS_WITH_VAR_SUBSTR_DONE);  

    return return_value;
} //end of funct

int metadata_catalog_all_types_with_var_attributes_with_var_substr_dims_in_timestep (const md_server &server
                      ,uint64_t run_id
                      ,uint64_t timestep_id
                      ,const std::string &var_name_substr
                      ,uint64_t txn_id
                      //,md_query_type query_type
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
    args.job_id = my_job_id;
    args.query_type = query_type;
    args.num_dims = num_dims;
    args.dims = dims;

    OpCatalogAllTypesWithVarAttributesWithVarSubstrDimsInTimestepMeta *op = new OpCatalogAllTypesWithVarAttributesWithVarSubstrDimsInTimestepMeta(server.peer_ptr, args);
    future<std::string> fut = op->GetFuture();

    opbox::LaunchOp(op);

    res=fut.get();    

    deArchiveMsgFromServer(res, entries, count, return_value);
    add_timing_point(MD_CATALOG_ALL_TYPES_WITH_VAR_ATTRIBUTES_WITH_VAR_SUBSTR_DIMS_IN_TIMESTEP_DONE);   

    return return_value;
} //end of funct

int metadata_catalog_all_types_with_var_attributes_with_var_substr_in_timestep (const md_server &server
                      ,uint64_t run_id
                      ,uint64_t timestep_id
                      ,const std::string &var_name_substr
                      ,uint64_t txn_id
                      //,md_query_type query_type
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
    args.job_id = my_job_id;
    args.query_type = query_type;

    OpCatalogAllTypesWithVarAttributesWithVarSubstrInTimestepMeta *op = new OpCatalogAllTypesWithVarAttributesWithVarSubstrInTimestepMeta(server.peer_ptr, args);
    future<std::string> fut = op->GetFuture();

    opbox::LaunchOp(op);

    res=fut.get();    

    deArchiveMsgFromServer(res, entries, count, return_value);
    add_timing_point(MD_CATALOG_ALL_TYPES_WITH_VAR_ATTRIBUTES_WITH_VAR_SUBSTR_IN_TIMESTEP_DONE);   

    return return_value;
} //end of funct


int metadata_catalog_all_var_attributes_with_type_var_substr (const md_server &server
                                        ,uint64_t run_id             
                                        ,uint64_t timestep_id
                                        ,uint64_t type_id
                                        ,const std::string &var_name_substr                                                                                
                                        ,uint64_t txn_id
                                        //,md_query_type query_type
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_var_attribute_entry> &matching_attributes
                                        )
{
    add_timing_point(MD_CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_START);

    int return_value;
    // std::string res;
    string res;
    md_catalog_all_var_attributes_with_type_var_substr_args args;

    args.job_id = my_job_id;
    args.query_type = query_type;
    args.run_id = run_id;
    args.timestep_id = timestep_id;
    args.type_id = type_id;
    args.var_name_substr.reserve(2 + var_name_substr.size());
    args.var_name_substr = "%";
    //args.var_name_substr += boost::algorithm::to_lower_copy(var_name_substr);
    args.var_name_substr += var_name_substr;
    args.var_name_substr += "%";
    OpCatalogAllVarAttributesWithTypeVarSubstrMeta *op = new OpCatalogAllVarAttributesWithTypeVarSubstrMeta(server.peer_ptr, args);
    //extreme_debug_log << "just launched get attr by dims \n";
    // future<std::string> fut = op->GetFuture();
    future<string> fut = op->GetFuture();    
    opbox::LaunchOp(op);

    res=fut.get();    

    deArchiveMsgFromServer(res, matching_attributes, count, return_value);
    add_timing_point(MD_CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_DONE);   
    //extreme_debug_log << "just dearchived matching attributes by dims message from server \n";
    //extreme_debug_log << "number of matching_attributes is " << matching_attributes.size() << "\n";


    return return_value;
} //end of funct

int metadata_catalog_all_var_attributes_with_type_var_substr_dims (const md_server &server
                                        ,uint64_t run_id             
                                        ,uint64_t timestep_id
                                        ,uint64_t type_id
                                        ,const std::string &var_name_substr                     
                                        ,uint64_t txn_id
                                        //,md_query_type query_type
                                        ,uint32_t num_dims
                                        ,const std::vector<md_dim_bounds> &dims
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_var_attribute_entry> &matching_attributes
                                        )
{
    add_timing_point(MD_CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_DIMS_START);

    int return_value;
    // std::string res;
    string res;
    md_catalog_all_var_attributes_with_type_var_substr_dims_args args;

    args.job_id = my_job_id;
    args.query_type = query_type;
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
    OpCatalogAllVarAttributesWithTypeVarSubstrDimsMeta *op = new OpCatalogAllVarAttributesWithTypeVarSubstrDimsMeta(server.peer_ptr, args);
    //extreme_debug_log << "just launched get attr by dims \n";
    // future<std::string> fut = op->GetFuture();
    future<string> fut = op->GetFuture();    
    opbox::LaunchOp(op);

    res=fut.get();    

    deArchiveMsgFromServer(res, matching_attributes, count, return_value);
    add_timing_point(MD_CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_DIMS_DONE);   
    //extreme_debug_log << "just dearchived matching attributes by dims message from server \n";
    //extreme_debug_log << "number of matching_attributes is " << matching_attributes.size() << "\n";


    return return_value;
} //end of funct

int metadata_catalog_all_var_attributes_with_var_substr_dims (const md_server &server
                                        ,uint64_t run_id             
                                        ,uint64_t timestep_id
                                        ,const std::string &var_name_substr                     
                                        ,uint64_t txn_id
                                        //,md_query_type query_type
                                        ,uint32_t num_dims
                                        ,const std::vector<md_dim_bounds> &dims
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_var_attribute_entry> &matching_attributes
                                        )
{
    add_timing_point(MD_CATALOG_ALL_VAR_ATTRIBUTES_WITH_VAR_SUBSTR_DIMS_START);

    int return_value;
    // std::string res;
    string res;
    md_catalog_all_var_attributes_with_var_substr_dims_args args;

    args.job_id = my_job_id;
    args.query_type = query_type;
    args.run_id = run_id;
    args.timestep_id = timestep_id;
    args.var_name_substr.reserve(2 + var_name_substr.size());
    args.var_name_substr = "%";
    //args.var_name_substr += boost::algorithm::to_lower_copy(var_name_substr);
    args.var_name_substr += var_name_substr;
    args.var_name_substr += "%";
    args.num_dims = num_dims;
    args.dims = dims;
    OpCatalogAllVarAttributesWithVarSubstrDimsMeta *op = new OpCatalogAllVarAttributesWithVarSubstrDimsMeta(server.peer_ptr, args);
    //extreme_debug_log << "just launched get attr by dims \n";
    // future<std::string> fut = op->GetFuture();
    future<string> fut = op->GetFuture();    
    opbox::LaunchOp(op);

    res=fut.get();    

    deArchiveMsgFromServer(res, matching_attributes, count, return_value);
    add_timing_point(MD_CATALOG_ALL_VAR_ATTRIBUTES_WITH_VAR_SUBSTR_DIMS_DONE);   
    //extreme_debug_log << "just dearchived matching attributes by dims message from server \n";
    //extreme_debug_log << "number of matching_attributes is " << matching_attributes.size() << "\n";


    return return_value;
} //end of funct


int metadata_delete_all_vars_with_substr (const md_server &server
                        ,uint64_t run_id
                        ,uint64_t timestep_id
                        ,const std::string &var_name_substr
                        //,md_query_type query_type
                        )
{
    add_timing_point(MD_DELETE_ALL_VARS_WITH_SUBSTR_START);

    int return_value;
    // int res;
    md_delete_all_vars_with_substr_args args;

    args.run_id = run_id;
    args.timestep_id = timestep_id;
    args.var_name_substr.reserve(2 + var_name_substr.size());
    args.var_name_substr = "%";
    //args.var_name_substr += boost::algorithm::to_lower_copy(var_name_substr);
    args.var_name_substr += var_name_substr;
    args.var_name_substr += "%";

    args.job_id = my_job_id;
    args.query_type = query_type;

    OpDeleteAllVarsWithSubstrMeta *op = new OpDeleteAllVarsWithSubstrMeta(server.peer_ptr, args);
    future<string> fut = op->GetFuture();
    opbox::LaunchOp(op);

    return_value=stoi(fut.get());

    // deArchiveMsgFromServer(res, return_value);
    add_timing_point(MD_DELETE_ALL_VARS_WITH_SUBSTR_DONE);

    return return_value;
} //end of funct

// int metadata_delete_all_vars_with_substr (const md_server &server
//                         ,uint64_t run_id
//                         ,uint64_t timestep_id
//                         ,const std::string &var_name_substr
//                         )
// {
//     add_timing_point(MD_DELETE_ALL_VARS_WITH_SUBSTR_START);

//     int return_value;
//     // int res;
//     md_delete_all_vars_with_substr_args args;

//     args.run_id = run_id;
//     args.timestep_id = timestep_id;
//     args.var_name_substr.reserve(2 + var_name_substr.size());
//     args.var_name_substr = "%";
//     //args.var_name_substr += boost::algorithm::to_lower_copy(var_name_substr);
//     args.var_name_substr += var_name_substr;
//     args.var_name_substr += "%";

//     args.job_id = my_job_id; //fix

//     OpDeleteAllVarsWithSubstrMeta *op = new OpDeleteAllVarsWithSubstrMeta(server.peer_ptr, args);
//     future<string> fut = op->GetFuture();
//     opbox::LaunchOp(op);

//     return_value=stoi(fut.get());

//     // deArchiveMsgFromServer(res, return_value);
//     add_timing_point(MD_DELETE_ALL_VARS_WITH_SUBSTR_DONE);

//     return return_value;
// } //end of funct



static int metadata_catalog_all_timesteps_with_var_attributes_with_type_var_substr_val (const md_server &server
                                        ,data_range_type type
                                        ,uint64_t run_id             
                                        ,uint64_t type_id
                                        ,const std::string &var_name_substr             
                                        ,uint64_t txn_id
                                        //,md_query_type query_type
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
    args.job_id = my_job_id;
    args.query_type = query_type;
    args.data_type = data_type;
    args.data = data;
    // //extreme_debug_log << "range_type: " << type << endl;
    // //extreme_debug_log << "run_id: " << run_id << endl;
    // //extreme_debug_log << "type_id: " << type_id << endl;
    // //extreme_debug_log << "var_name_substr: " << var_name_substr << endl;
    // //extreme_debug_log << "txn_id: " << txn_id << endl;
    // //extreme_debug_log << "data_type: " << data_type << endl;
    // //extreme_debug_log << "data: " << data << endl;

    //extreme_debug_log << "txn_id is " << to_string(txn_id ) << endl;  
   
    OpCatalogAllTimestepsWithVarAttributesWithTypeVarSubstrRangeMeta *op = new OpCatalogAllTimestepsWithVarAttributesWithTypeVarSubstrRangeMeta(server.peer_ptr, args);
    future<std::string> fut = op->GetFuture();
    opbox::LaunchOp(op);

    res=fut.get();

    deArchiveMsgFromServer(res, entries, count, return_value);
    add_timing_point(MD_CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_RANGE_DONE);  

    return return_value;
} //end of funct

int metadata_catalog_all_timesteps_with_var_attributes_with_type_var_substr_range (const md_server &server
                                        ,uint64_t run_id             
                                        ,uint64_t type_id
                                        ,const std::string &var_name_substr             
                                        ,uint64_t txn_id
                                        //,md_query_type query_type
                                        ,attr_data_type data_type
                                        ,const std::string &data                                        
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_timestep_entry> &entries
                                        )
{
    int return_value;

    return_value = metadata_catalog_all_timesteps_with_var_attributes_with_type_var_substr_val (server, DATA_RANGE, run_id, type_id,
        var_name_substr, txn_id, data_type, data, count, entries);

    return return_value;
} //end of funct

int metadata_catalog_all_timesteps_with_var_attributes_with_type_var_substr_above_max (const md_server &server
                                        ,uint64_t run_id             
                                        ,uint64_t type_id
                                        ,const std::string &var_name_substr             
                                        ,uint64_t txn_id
                                        //,md_query_type query_type
                                        ,attr_data_type data_type
                                        ,const std::string &data                                        
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_timestep_entry> &entries
                                        )
{
    int return_value;

    return_value = metadata_catalog_all_timesteps_with_var_attributes_with_type_var_substr_val (server, DATA_MAX, run_id, type_id,
        var_name_substr, txn_id, data_type, data, count, entries);

    return return_value;
} //end of funct

int metadata_catalog_all_timesteps_with_var_attributes_with_type_var_substr_below_min (const md_server &server
                                        ,uint64_t run_id             
                                        ,uint64_t type_id
                                        ,const std::string &var_name_substr             
                                        ,uint64_t txn_id
                                        //,md_query_type query_type
                                        ,attr_data_type data_type
                                        ,const std::string &data                                        
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_timestep_entry> &entries
                                        )
{
    int return_value;

    return_value = metadata_catalog_all_timesteps_with_var_attributes_with_type_var_substr_val (server, DATA_MIN, run_id, type_id,
        var_name_substr, txn_id, data_type, data, count, entries);

    return return_value;
} //end of funct


static int metadata_catalog_all_timesteps_with_var_attributes_with_type_var_substr_dims_val (const md_server &server
                                        ,data_range_type type
                                        ,uint64_t run_id             
                                        ,uint64_t type_id
                                        ,const std::string &var_name_substr             
                                        ,uint64_t txn_id
                                        //,md_query_type query_type
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
    args.job_id = my_job_id;
    args.query_type = query_type;
    args.num_dims = num_dims;
    args.dims = dims;
    args.data_type = data_type;
    args.data = data;
    //extreme_debug_log << "txn_id is " << to_string(txn_id ) << endl;  
   
    OpCatalogAllTimestepsWithVarAttributesWithTypeVarSubstrDimsRangeMeta *op = new OpCatalogAllTimestepsWithVarAttributesWithTypeVarSubstrDimsRangeMeta(server.peer_ptr, args);
    future<std::string> fut = op->GetFuture();
    opbox::LaunchOp(op);

    res=fut.get();

    deArchiveMsgFromServer(res, entries, count, return_value);
    add_timing_point(MD_CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_DIMS_RANGE_DONE);  

    return return_value;
} //end of funct

int metadata_catalog_all_timesteps_with_var_attributes_with_type_var_substr_dims_range (const md_server &server
                                        ,uint64_t run_id             
                                        ,uint64_t type_id
                                        ,const std::string &var_name_substr             
                                        ,uint64_t txn_id
                                        //,md_query_type query_type
                                        ,uint32_t num_dims
                                        ,const std::vector<md_dim_bounds> &dims
                                        ,attr_data_type data_type
                                        ,const std::string &data
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_timestep_entry> &entries
                                        )
{
    int return_value;

    return_value = metadata_catalog_all_timesteps_with_var_attributes_with_type_var_substr_dims_val (server, DATA_RANGE,
        run_id, type_id, var_name_substr, txn_id, num_dims, dims, data_type, data, count, entries);

    return return_value;
} //end of funct

int metadata_catalog_all_timesteps_with_var_attributes_with_type_var_substr_dims_above_max (const md_server &server
                                        ,uint64_t run_id             
                                        ,uint64_t type_id
                                        ,const std::string &var_name_substr             
                                        ,uint64_t txn_id
                                        //,md_query_type query_type
                                        ,uint32_t num_dims
                                        ,const std::vector<md_dim_bounds> &dims
                                        ,attr_data_type data_type
                                        ,const std::string &data
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_timestep_entry> &entries
                                        )
{
    int return_value;

    return_value = metadata_catalog_all_timesteps_with_var_attributes_with_type_var_substr_dims_val (server, DATA_MAX,
        run_id, type_id, var_name_substr, txn_id, num_dims, dims, data_type, data, count, entries);

    return return_value;
} //end of funct

int metadata_catalog_all_timesteps_with_var_attributes_with_type_var_substr_dims_below_min (const md_server &server
                                        ,uint64_t run_id             
                                        ,uint64_t type_id
                                        ,const std::string &var_name_substr             
                                        ,uint64_t txn_id
                                        //,md_query_type query_type
                                        ,uint32_t num_dims
                                        ,const std::vector<md_dim_bounds> &dims
                                        ,attr_data_type data_type
                                        ,const std::string &data
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_timestep_entry> &entries
                                        )
{
    int return_value;

    return_value = metadata_catalog_all_timesteps_with_var_attributes_with_type_var_substr_dims_val (server, DATA_MIN,
        run_id, type_id, var_name_substr, txn_id, num_dims, dims, data_type, data, count, entries);

    return return_value;
} //end of funct



static int metadata_catalog_all_var_attributes_with_type_var_substr_val (const md_server &server
                                        ,data_range_type type
                                        ,uint64_t timestep_id
                                        ,uint64_t type_id
                                        ,const std::string &var_name_substr 
                                        ,uint64_t txn_id
                                        //,md_query_type query_type
                                        ,attr_data_type data_type
                                        ,const std::string &data
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_var_attribute_entry> &matching_attributes
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
    args.job_id = my_job_id;
    args.query_type = query_type;
    args.data_type = data_type;
    args.data = data;

    OpCatalogAllVarAttributesWithTypeVarSubstrRangeMeta *op = new OpCatalogAllVarAttributesWithTypeVarSubstrRangeMeta(server.peer_ptr, args);
    //extreme_debug_log << "just launched get attr by dims \n";
    future<std::string> fut = op->GetFuture();
    opbox::LaunchOp(op);

    res=fut.get();    

    deArchiveMsgFromServer(res, matching_attributes, count, return_value);
    add_timing_point(MD_CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_RANGE_DONE);   
    //extreme_debug_log << "just dearchived matching attributes by dims message from server \n";
    //extreme_debug_log << "number of matching_attributes is " << matching_attributes.size() << "\n";


    return return_value;
} //end of funct

int metadata_catalog_all_var_attributes_with_type_var_substr_range (const md_server &server
                                        ,uint64_t timestep_id
                                        ,uint64_t type_id
                                        ,const std::string &var_name_substr 
                                        ,uint64_t txn_id
                                        //,md_query_type query_type
                                        ,attr_data_type data_type
                                        ,const std::string &data
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_var_attribute_entry> &matching_attributes
                                        )
{
    int return_value;

    return_value = metadata_catalog_all_var_attributes_with_type_var_substr_val (server, DATA_RANGE, timestep_id,   
        type_id, var_name_substr, txn_id, data_type, data, count, matching_attributes);

    return return_value;
} //end of funct

int metadata_catalog_all_var_attributes_with_type_var_substr_above_max (const md_server &server
                                        ,uint64_t timestep_id
                                        ,uint64_t type_id
                                        ,const std::string &var_name_substr 
                                        ,uint64_t txn_id
                                        //,md_query_type query_type
                                        ,attr_data_type data_type
                                        ,const std::string &data
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_var_attribute_entry> &matching_attributes
                                        )
{
    int return_value;

    return_value = metadata_catalog_all_var_attributes_with_type_var_substr_val (server, DATA_MAX, timestep_id,   
        type_id, var_name_substr, txn_id, data_type, data, count, matching_attributes);

    return return_value;
} //end of funct

int metadata_catalog_all_var_attributes_with_type_var_substr_below_min (const md_server &server
                                        ,uint64_t timestep_id
                                        ,uint64_t type_id
                                        ,const std::string &var_name_substr 
                                        ,uint64_t txn_id
                                        //,md_query_type query_type
                                        ,attr_data_type data_type
                                        ,const std::string &data
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_var_attribute_entry> &matching_attributes
                                        )
{
    int return_value;

    return_value = metadata_catalog_all_var_attributes_with_type_var_substr_val (server, DATA_MIN, timestep_id,   
        type_id, var_name_substr, txn_id, data_type, data, count, matching_attributes);

    return return_value;
} //end of funct

static int metadata_catalog_all_var_attributes_with_type_var_substr_dims_val (const md_server &server
                                        ,data_range_type type
                                        ,uint64_t timestep_id
                                        ,uint64_t type_id
                                        ,const std::string &var_name_substr 
                                        ,uint64_t txn_id
                                        //,md_query_type query_type
                                        ,uint32_t num_dims
                                        ,const std::vector<md_dim_bounds> &dims
                                        ,attr_data_type data_type
                                        ,const std::string &data
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_var_attribute_entry> &matching_attributes
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
    args.job_id = my_job_id;
    args.query_type = query_type;
    args.num_dims = num_dims;
    args.dims = dims;
    args.data_type = data_type;
    args.data = data;
    OpCatalogAllVarAttributesWithTypeVarSubstrDimsRangeMeta *op = new OpCatalogAllVarAttributesWithTypeVarSubstrDimsRangeMeta(server.peer_ptr, args);
    //extreme_debug_log << "just launched get attr by dims \n";
    future<std::string> fut = op->GetFuture();
    opbox::LaunchOp(op);

    res=fut.get();    

    deArchiveMsgFromServer(res, matching_attributes, count, return_value);
    add_timing_point(MD_CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_DIMS_RANGE_DONE);   
    //extreme_debug_log << "just dearchived matching attributes by dims message from server \n";
    //extreme_debug_log << "number of matching_attributes is " << matching_attributes.size() << "\n";


    return return_value;
} //end of funct

int metadata_catalog_all_var_attributes_with_type_var_substr_dims_range (const md_server &server
                                        ,uint64_t timestep_id
                                        ,uint64_t type_id
                                        ,const std::string &var_name_substr 
                                        ,uint64_t txn_id
                                        //,md_query_type query_type
                                        ,uint32_t num_dims
                                        ,const std::vector<md_dim_bounds> &dims
                                        ,attr_data_type data_type
                                        ,const std::string &data
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_var_attribute_entry> &matching_attributes
                                        )
{
    int return_value;

    return_value = metadata_catalog_all_var_attributes_with_type_var_substr_dims_val (server, DATA_RANGE, timestep_id, type_id,
        var_name_substr, txn_id, num_dims, dims, data_type, data, count, matching_attributes);

    return return_value;

} //end of funct



int metadata_catalog_all_var_attributes_with_type_var_substr_dims_above_max (const md_server &server
                                        ,uint64_t timestep_id
                                        ,uint64_t type_id
                                        ,const std::string &var_name_substr 
                                        ,uint64_t txn_id
                                        //,md_query_type query_type
                                        ,uint32_t num_dims
                                        ,const std::vector<md_dim_bounds> &dims
                                        ,attr_data_type data_type
                                        ,const std::string &data
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_var_attribute_entry> &matching_attributes
                                        )
{
    int return_value;

    return_value = metadata_catalog_all_var_attributes_with_type_var_substr_dims_val (server, DATA_MAX, timestep_id, type_id,
        var_name_substr, txn_id, num_dims, dims, data_type, data, count, matching_attributes);
    return return_value;
} //end of funct

int metadata_catalog_all_var_attributes_with_type_var_substr_dims_below_min (const md_server &server
                                        ,uint64_t timestep_id
                                        ,uint64_t type_id
                                        ,const std::string &var_name_substr 
                                        ,uint64_t txn_id
                                        //,md_query_type query_type
                                        ,uint32_t num_dims
                                        ,const std::vector<md_dim_bounds> &dims
                                        ,attr_data_type data_type
                                        ,const std::string &data
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_var_attribute_entry> &matching_attributes
                                        )
{
    int return_value;

    return_value = metadata_catalog_all_var_attributes_with_type_var_substr_dims_val (server, DATA_MIN, timestep_id, type_id,
        var_name_substr, txn_id, num_dims, dims, data_type, data, count, matching_attributes);

    return return_value;
} //end of funct






// int metadata_insert_var_attribute_by_dims_batch (const md_server &server
//                            ,uint64_t &attribute_id
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
//         //debug_log << "error they are off at start of md insert attributes " << endl;
//     }
//     md_insert_var_attribute_by_dims_args args;
//     args.timestep_id = timestep_id;
//     args.type_id = type_id;
//     args.var_id = var_id;
//     args.job_id = my_job_id;
//     args.num_dims = num_dims;
//     args.dims = dims;
//     args.data_type = data_type;
//     args.data = data;

//     OpInsertVarAttributeByDimsBatchMeta *op = new OpInsertVarAttributeByDimsBatchMeta(server.peer_ptr, args);
//     future<std::string> fut = op->GetFuture();
//     opbox::LaunchOp(op);

//     res=fut.get();

//     deArchiveMsgFromServer(res, return_value, attribute_id);
//     add_timing_point(MD_INSERT_VAR_ATTRIBUTE_BY_DIMS_BATCH_DONE);   
//     //debug_log << "according to the client the attribute_id is " << attribute_id << endl;
//     // if(time_pts.size() != catg_of_time_pts.size()) {
//     //     //debug_log << "error they are off at the end of md insert attributes " << endl;
//     // }
//     return return_value;
// } //end of funct

int metadata_insert_var_attribute_by_dims_batch (const md_server &server
                           // vector<uint64_t> &attribute_ids,
                           ,const vector<md_catalog_var_attribute_entry> &new_attributes
                           )
{
    int return_value = RC_OK;
    // if(new_attributes.size() > 0) {
        add_timing_point(MD_INSERT_VAR_ATTRIBUTE_BY_DIMS_BATCH_START);
        // int res;

        // if(time_pts.size() != catg_of_time_pts.size()) {
        //     //debug_log << "error they are off at start of md insert attributes " << endl;
        // }
        vector<md_insert_var_attribute_by_dims_args> all_args;
        all_args.reserve(new_attributes.size());
        for (int i = 0; i < new_attributes.size(); i++) {
            md_insert_var_attribute_by_dims_args args;
            md_catalog_var_attribute_entry new_attribute = new_attributes.at(i);
            args.run_id = new_attribute.run_id;
            args.timestep_id = new_attribute.timestep_id;
            args.type_id = new_attribute.type_id;
            args.var_id = new_attribute.var_id;
            args.job_id = my_job_id;
            args.num_dims = new_attribute.num_dims;
            args.dims = new_attribute.dims;
            args.data_type = new_attribute.data_type;
            args.data = new_attribute.data;

            all_args.push_back(args);
        }

        OpInsertVarAttributeByDimsBatchMeta *op = new OpInsertVarAttributeByDimsBatchMeta(server.peer_ptr, all_args);
        future<string> fut = op->GetFuture();
        opbox::LaunchOp(op);

        return_value=stoi(fut.get());

        // deArchiveMsgFromServer(res, return_value, attribute_ids);
         // deArchiveMsgFromServer(res, return_value);
        add_timing_point(MD_INSERT_VAR_ATTRIBUTE_BY_DIMS_BATCH_DONE);   
        // //debug_log << "according to the client the attribute_id is " << attribute_id << endl;
        // if(time_pts.size() != catg_of_time_pts.size()) {
        //     //debug_log << "error they are off at the end of md insert attributes " << endl;
        // }
    // }
    return return_value;
} //end of funct


// int metadata_create_type_batch (const md_server &server
//                         ,uint64_t &type_id
//                         ,uint64_t run_id
//                         ,const std::string &name
//                         ,uint32_t version
//                         ,uint64_t txn_id
//                         )
// {

//note - could be modified to take indv parameters (see directly above) or to take a type_id from the user
// int metadata_create_type_batch (const md_server &server
//                         ,uint64_t &type_id
//                         ,const vector<md_catalog_type_entry> &new_types
//                         )
int metadata_create_type_batch (const md_server &server
                        // ,vector<uint64_t> &type_ids
                        ,uint64_t &type_id
                        ,const vector<md_catalog_type_entry> &new_types
                        )
{
    add_timing_point(MD_CREATE_TYPE_BATCH_START);
    //extreme_debug_log << "got to top of metadata_create_type_batch \n";
    int return_value;
    string res;

    // type_ids.reserve(new_types.size());

    vector<md_create_type_args> all_args;

    for (int i = 0; i < new_types.size(); i++) {
        md_create_type_args args;
        md_catalog_type_entry new_type = new_types.at(i);

        args.run_id = new_type.run_id;
        args.name = new_type.name;
        args.version = new_type.version;
        args.job_id = my_job_id;
        //extreme_debug_log << "about to create new type \n";

        all_args.push_back(args);
    }
    
    OpCreateTypeBatchMeta *op = new OpCreateTypeBatchMeta(server.peer_ptr, all_args);
    //extreme_debug_log << "about to get future" <<endl;
    future<string> fut = op->GetFuture();
    //extreme_debug_log << "about to launch op" <<endl;
    opbox::LaunchOp(op);
    //extreme_debug_log << "about to get final result" <<endl;

    res = fut.get();

    deArchiveMsgFromServer(res, type_id, return_value); 
    // deArchiveMsgFromServer(res, return_value); 
    // deArchiveMsgFromServer(res, type_ids, return_value); 
    add_timing_point(MD_CREATE_TYPE_BATCH_DONE);   

    return return_value;
} //end of funct

// int metadata_create_var_batch (const md_server &server
//                         ,uint64_t &var_id
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
int metadata_create_var_batch (const md_server &server
                        // ,uint64_t &var_id
                        ,const vector<md_catalog_var_entry> &new_vars
                        )
{
    add_timing_point(MD_CREATE_VAR_BATCH_START);
    //extreme_debug_log << "got to top of metadata_create_var_batch \n";
    int return_value;
    // int res;


    ///////todo/////////////////////////////////////////////////////////////////////
    // string temp;
    // size_t size;
    // for(int i=0; i< new_var.name.size(); i++) {
    //     temp += to_string(((int)new_var.name.at(i)));
    // }
    // temp += to_string((int)'_');
    // temp += to_string(new_var.version);
    // //extreme_debug_log << "temp: " << temp << endl;
    // args.var_id = stoull(temp, &size, 10); //todo
   //  string var_name_ver = new_var.name;
   //  var_name_ver += "_";
   //  var_name_ver += to_string(new_var.version);
   //  //extreme_debug_log << "var_name_ver: " << var_name_ver << endl;
   //  map <string, uint64_t>::iterator it = var_ids.find(var_name_ver);
   //  if ( it == var_ids.end() ) {
   //    // not found
   //      var_ids[var_name_ver] = var_ids.size();
   //      args.var_id = var_ids.size();

   //      //extreme_debug_log << "not found, and var_ids.size(): " << var_ids.size() << endl;
   //  } else {
   //    // found
   //      //extreme_debug_log << "found, var_id: " << it->second << endl;
   //      args.var_id = it->second;
   //  }

   // for (std::map<string,uint64_t>::iterator it=var_ids.begin(); it!=var_ids.end(); ++it) {
   //    string var_name = it->first;
   //    uint64_t temp_var_id = it->second;
   //    //extreme_debug_log << "var_name: " << var_name << " temp_var_id " << temp_var_id << endl;
   //  }
   //  //extreme_debug_log << "args.var_id: " << args.var_id << endl;

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
        args.job_id = my_job_id;
        args.num_dims = new_var.num_dims;
        //extreme_debug_log << "run_id: " << new_var.run_id << " timestep_id: " << new_var.timestep_id << " var_id: " << args.var_id << " \n";

        args.dims = new_var.dims;
        //extreme_debug_log << "about to create new var \n";

        all_args.push_back(args);

    }
    
    OpCreateVarBatchMeta *op = new OpCreateVarBatchMeta(server.peer_ptr, all_args);    
    //extreme_debug_log << "about to get future" <<endl;
    future<string> fut = op->GetFuture();
    //extreme_debug_log << "about to launch op" <<endl;
    opbox::LaunchOp(op);
    //extreme_debug_log << "about to get final result" <<endl;

    return_value = stoi(fut.get());

    // uint64_t rowid; //todo
    // var_id = args.var_id;
    // deArchiveMsgFromServer(res, rowid, return_value); //todo
    // deArchiveMsgFromServer(res, return_value); //todo
    add_timing_point(MD_CREATE_VAR_BATCH_DONE); 

    return return_value;
} //end of funct

// int metadata_insert_run_attribute_batch (const md_server &server
//                            ,uint64_t &attribute_id
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
//     //     //debug_log << "error they are off at start of md insert attributes " << endl;
//     // }
//     md_insert_run_attribute_args args;
//     args.type_id = type_id;
//     args.job_id = my_job_id;
//     args.data_type = data_type;
//     args.data = data;

//     OpInsertRunAttributeBatchMeta *op = new OpInsertRunAttributeBatchMeta(server.peer_ptr, args);
//     future<std::string> fut = op->GetFuture();
//     opbox::LaunchOp(op);

//     res=fut.get();

//     deArchiveMsgFromServer(res, return_value, attribute_id);
//     add_timing_point(MD_INSERT_RUN_ATTRIBUTE_BATCH_DONE);   
//     //debug_log << "according to the client the attribute_id is " << attribute_id << endl;
//     // if(time_pts.size() != catg_of_time_pts.size()) {
//     //     //debug_log << "error they are off at the end of md insert attributes " << endl;
//     // }
//     return return_value;
// } //end of funct

int metadata_insert_run_attribute_batch (const md_server &server
                           // ,uint64_t &attribute_id
                           ,const vector<md_catalog_run_attribute_entry> &new_attributes
                           )
{
    add_timing_point(MD_INSERT_RUN_ATTRIBUTE_BATCH_START);
    int return_value;
    // int res;

    // if(time_pts.size() != catg_of_time_pts.size()) {
    //     //debug_log << "error they are off at start of md insert attributes " << endl;
    // }

    vector<md_insert_run_attribute_args> all_args;
    for (int i = 0; i < new_attributes.size(); i++) {
        md_insert_run_attribute_args args;
        md_catalog_run_attribute_entry new_attribute = new_attributes.at(i);
        args.run_id = new_attribute.run_id;
        args.type_id = new_attribute.type_id;
        args.job_id = my_job_id;
        args.data_type = new_attribute.data_type;
        args.data = new_attribute.data;

        all_args.push_back(args);
    }

    OpInsertRunAttributeBatchMeta *op = new OpInsertRunAttributeBatchMeta(server.peer_ptr, all_args);
    future<string> fut = op->GetFuture();
    opbox::LaunchOp(op);

    return_value=stoi(fut.get());

    // deArchiveMsgFromServer(res, return_value, attribute_id);
    // deArchiveMsgFromServer(res, return_value);
    add_timing_point(MD_INSERT_RUN_ATTRIBUTE_BATCH_DONE);   
    // //debug_log << "according to the client the attribute_id is " << attribute_id << endl;
    // if(time_pts.size() != catg_of_time_pts.size()) {
    //     //debug_log << "error they are off at the end of md insert attributes " << endl;
    // }
    return return_value;
} //end of funct

// int metadata_insert_timestep_attribute_batch (const md_server &server
//                            ,uint64_t &attribute_id
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
//     //     //debug_log << "error they are off at start of md insert attributes " << endl;
//     // }
//     md_insert_timestep_attribute_args args;
//     args.timestep_id = timestep_id;
//     args.type_id = type_id;
//     args.job_id = my_job_id;
//     args.data_type = data_type;
//     args.data = data;

//     OpInsertTimestepAttributeBatchMeta *op = new OpInsertTimestepAttributeBatchMeta(server.peer_ptr, args);
//     future<std::string> fut = op->GetFuture();
//     opbox::LaunchOp(op);

//     res=fut.get();

//     deArchiveMsgFromServer(res, return_value, attribute_id);
//     add_timing_point(MD_INSERT_TIMESTEP_ATTRIBUTE_BATCH_DONE);   
//     //debug_log << "according to the client the attribute_id is " << attribute_id << endl;
//     // if(time_pts.size() != catg_of_time_pts.size()) {
//     //     //debug_log << "error they are off at the end of md insert attributes " << endl;
//     // }
//     return return_value;
// } //end of funct

int metadata_insert_timestep_attribute_batch (const md_server &server
                           // ,uint64_t &attribute_id
                           ,const vector<md_catalog_timestep_attribute_entry> &new_attributes
                           )
{
    add_timing_point(MD_INSERT_TIMESTEP_ATTRIBUTE_BATCH_START);
    int return_value;
    // int res;

    // if(time_pts.size() != catg_of_time_pts.size()) {
    //     //debug_log << "error they are off at start of md insert attributes " << endl;
    // }

    vector<md_insert_timestep_attribute_args> all_args;
    for (int i = 0; i < new_attributes.size(); i++) {
        md_insert_timestep_attribute_args args;
        md_catalog_timestep_attribute_entry new_attribute = new_attributes.at(i);
        args.run_id = new_attribute.run_id;
        args.timestep_id = new_attribute.timestep_id;
        args.type_id = new_attribute.type_id;
        args.job_id = my_job_id;
        args.data_type = new_attribute.data_type;
        args.data = new_attribute.data;

        all_args.push_back(args);
    }

    OpInsertTimestepAttributeBatchMeta *op = new OpInsertTimestepAttributeBatchMeta(server.peer_ptr, all_args);
    future<string> fut = op->GetFuture();
    opbox::LaunchOp(op);

    return_value=stoi(fut.get());

    // deArchiveMsgFromServer(res, return_value, attribute_id);
    // deArchiveMsgFromServer(res, return_value);
    add_timing_point(MD_INSERT_TIMESTEP_ATTRIBUTE_BATCH_DONE);   
    // //debug_log << "according to the client the attribute_id is " << attribute_id << endl;
    // if(time_pts.size() != catg_of_time_pts.size()) {
    //     //debug_log << "error they are off at the end of md insert attributes " << endl;
    // }
    return return_value;
} //end of funct


int metadata_checkpoint_database (const md_server &server
                                 , uint64_t job_id
                                 , md_db_checkpoint_type checkpt_type
                                 )
{
    // add_db_checkpt_timing_point(MD_CHECKPOINT_DATABASE_START, checkpt_type);
    add_timing_point(MD_CHECKPOINT_DATABASE_START);

    int return_value;

    md_checkpoint_database_args args;
    args.job_id = job_id;
    args.checkpt_type = checkpt_type;

    OpCheckpointDatabaseMeta *op = new OpCheckpointDatabaseMeta(server.peer_ptr, args);
    future<string> fut = op->GetFuture();
    opbox::LaunchOp(op);

    return_value=stoi(fut.get());  

    add_timing_point(MD_CHECKPOINT_DATABASE_DONE);

    return return_value;
} //end of funct




int metadata_catalog_all_run_attributes (const md_server &server
                                        ,uint64_t txn_id
                                        //,md_query_type query_type
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_run_attribute_entry> &entries
                                        )
{
    int return_value = metadata_catalog_all_run_attributes_in_run (server,
                            ALL_RUNS, txn_id, count, entries
                            );
    return return_value;
}                                      


int metadata_catalog_all_run_attributes_with_type (const md_server &server
                                        ,uint64_t type_id             
                                        ,uint64_t txn_id
                                        //,md_query_type query_type
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_run_attribute_entry> &entries
                                        )
{
    int return_value = metadata_catalog_all_run_attributes_with_type_in_run (server,
                            ALL_RUNS, type_id, txn_id, count, entries
                            );
    return return_value;
}

int metadata_catalog_all_run_attributes_with_type_range (const md_server &server
                                        ,uint64_t type_id             
                                        ,uint64_t txn_id
                                        //,md_query_type query_type
                                        ,attr_data_type data_type
                                        ,const std::string &data
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_run_attribute_entry> &entries
                                        )
{
    int return_value;

    return_value = metadata_catalog_all_run_attributes_with_type_val (server, DATA_RANGE, ALL_RUNS, type_id, txn_id, data_type, data, count, entries);

    return return_value;
} //end of funct

int metadata_catalog_all_run_attributes_with_type_above_max (const md_server &server
                                        ,uint64_t type_id             
                                        ,uint64_t txn_id
                                        //,md_query_type query_type
                                        ,attr_data_type data_type
                                        ,const std::string &data
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_run_attribute_entry> &entries
                                        )
{
    int return_value;

    return_value = metadata_catalog_all_run_attributes_with_type_val (server, DATA_MAX, ALL_RUNS, type_id, txn_id, data_type, data, count, entries);

    return return_value;
} //end of funct

int metadata_catalog_all_run_attributes_with_type_below_min (const md_server &server
                                        ,uint64_t type_id             
                                        ,uint64_t txn_id
                                        //,md_query_type query_type
                                        ,attr_data_type data_type
                                        ,const std::string &data
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_run_attribute_entry> &entries
                                        )
{
    int return_value;

    return_value = metadata_catalog_all_run_attributes_with_type_val (server, DATA_MIN, ALL_RUNS, type_id, txn_id, data_type, data, count, entries);

    return return_value;
} //end of funct



int metadata_catalog_all_timestep_attributes (const md_server &server
                                        ,uint64_t run_id             
                                        ,uint64_t txn_id
                                        //,md_query_type query_type
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_timestep_attribute_entry> &entries
                                        )
{
    int return_value = metadata_catalog_all_timestep_attributes_in_timestep (server,
                            run_id, ALL_TIMESTEPS, txn_id, count, entries
                            );

    return return_value;
}



int metadata_catalog_all_timestep_attributes_with_type (const md_server &server
                                        ,uint64_t run_id             
                                        ,uint64_t type_id             
                                        ,uint64_t txn_id
                                        //,md_query_type query_type
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_timestep_attribute_entry> &entries
                                        )
{
    int return_value = metadata_catalog_all_timestep_attributes_with_type_in_timestep (server,
                            run_id, ALL_TIMESTEPS, type_id, txn_id, count, entries
                            );

    return return_value;
}

int metadata_catalog_all_timestep_attributes_with_type_range (const md_server &server
                                        ,uint64_t run_id             
                                        ,uint64_t type_id             
                                        ,uint64_t txn_id
                                        //,md_query_type query_type
                                        ,attr_data_type data_type
                                        ,const std::string &data
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_timestep_attribute_entry> &entries
                                        )
{
    int return_value;

    return_value = metadata_catalog_all_timestep_attributes_with_type_val (server, DATA_RANGE, run_id,
        ALL_TIMESTEPS, type_id, txn_id, data_type, data, count, entries);

    return return_value;
}

int metadata_catalog_all_timestep_attributes_with_type_above_max (const md_server &server
                                        ,uint64_t run_id             
                                        ,uint64_t type_id             
                                        ,uint64_t txn_id
                                        //,md_query_type query_type
                                        ,attr_data_type data_type
                                        ,const std::string &data
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_timestep_attribute_entry> &entries
                                        )
{
    int return_value;

    return_value = metadata_catalog_all_timestep_attributes_with_type_val (server, DATA_MAX, run_id,
        ALL_TIMESTEPS, type_id, txn_id, data_type, data, count, entries);
    
    return return_value;
}

int metadata_catalog_all_timestep_attributes_with_type_below_min (const md_server &server
                                        ,uint64_t run_id             
                                        ,uint64_t type_id             
                                        ,uint64_t txn_id
                                        //,md_query_type query_type
                                        ,attr_data_type data_type
                                        ,const std::string &data
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_timestep_attribute_entry> &entries
                                        )
{
    int return_value;

    return_value = metadata_catalog_all_timestep_attributes_with_type_val (server, DATA_MIN, run_id,
        ALL_TIMESTEPS, type_id, txn_id, data_type, data, count, entries);

    return return_value;
}


int metadata_catalog_all_types_with_var_attributes (const md_server &server
                      ,uint64_t run_id
                      ,uint64_t txn_id
                      //,md_query_type query_type
                      ,uint32_t &count
                      ,std::vector<md_catalog_type_entry> &entries
                     )
{
    int return_value = metadata_catalog_all_types_with_var_attributes_in_timestep (server,
                            run_id, ALL_TIMESTEPS, txn_id, count, entries
                            );

    return return_value;
}



int metadata_catalog_all_types_with_var_attributes_with_var_dims (const md_server &server
                      ,uint64_t run_id
                      ,uint64_t var_id
                      ,uint64_t txn_id
                      //,md_query_type query_type
                      ,uint32_t num_dims
                      ,const std::vector<md_dim_bounds> &dims
                      ,uint32_t &count
                      ,std::vector<md_catalog_type_entry> &entries
                     )
{
    int return_value = metadata_catalog_all_types_with_var_attributes_with_var_dims_in_timestep (server,
                            run_id, ALL_TIMESTEPS, var_id, txn_id, num_dims, dims, count, entries
                            );
    return return_value;
}



int metadata_catalog_all_types_with_var_attributes_with_var (const md_server &server
                      ,uint64_t run_id
                      ,uint64_t var_id
                      ,uint64_t txn_id
                      //,md_query_type query_type
                      ,uint32_t &count
                      ,std::vector<md_catalog_type_entry> &entries
                     )
{
    int return_value = metadata_catalog_all_types_with_var_attributes_with_var_in_timestep (server,
                            run_id, ALL_TIMESTEPS, var_id, txn_id, count, entries
                     );

    return return_value;
}



int metadata_catalog_all_types_with_var_attributes_with_var_substr_dims (const md_server &server
                      ,uint64_t run_id
                      ,const std::string &var_name_substr
                      ,uint64_t txn_id
                      //,md_query_type query_type
                      ,uint32_t num_dims
                      ,const std::vector<md_dim_bounds> &dims
                      ,uint32_t &count
                      ,std::vector<md_catalog_type_entry> &entries
                     )
{
    int return_value = metadata_catalog_all_types_with_var_attributes_with_var_substr_dims_in_timestep( server, 
                          run_id, ALL_TIMESTEPS, var_name_substr, txn_id, num_dims, dims,
                          count, entries
                         );
    return return_value;
}



int metadata_catalog_all_types_with_var_attributes_with_var_substr (const md_server &server
                      ,uint64_t run_id
                      ,const std::string &var_name_substr
                      ,uint64_t txn_id
                      //,md_query_type query_type
                      ,uint32_t &count
                      ,std::vector<md_catalog_type_entry> &entries
                     )
{
    int return_value = metadata_catalog_all_types_with_var_attributes_with_var_substr_in_timestep (server,
                      run_id, ALL_TIMESTEPS, var_name_substr, txn_id, count, entries);

    return return_value;
} //end of funct

int metadata_commit_transaction (const md_server &server
                                 , uint64_t job_id)
{
    add_timing_point(MD_COMMIT_TRANSACTION_START);

    int return_value;

    OpCommitTransactionMeta *op = new OpCommitTransactionMeta(server.peer_ptr, job_id);
    future<string> fut = op->GetFuture();
    opbox::LaunchOp(op);

    return_value=stoi(fut.get());  

    add_timing_point(MD_COMMIT_TRANSACTION_DONE);

    return return_value;
} //end of funct


int metadata_abort_transaction (const md_server &server
                                 , uint64_t job_id)
{
    add_timing_point(MD_ABORT_TRANSACTION_START);

    int return_value;

    OpAbortTransactionMeta *op = new OpAbortTransactionMeta(server.peer_ptr, job_id);
    future<string> fut = op->GetFuture();
    opbox::LaunchOp(op);

    return_value=stoi(fut.get());  

    add_timing_point(MD_ABORT_TRANSACTION_DONE);

    return return_value;
} //end of funct





template <class T>
void deArchiveMsgFromServer(const string &incoming_msg,  
                                           std::vector<T> &entries,
                                           uint32_t &count,
                                           int &return_value) {

  std::stringstream ss;
  ss << incoming_msg;
  boost::archive::text_iarchive ia(ss);
  ia >> count;
  if( count > 0) {
    ia >> entries;
  }
  else {
    entries.clear();
  }
  ia >> return_value;
 

}

void deArchiveMsgFromServer(const std::string &serial_str,
                                             uint64_t &id,
                                             int &return_value) {

  std::stringstream ss;
  ss << serial_str;
  boost::archive::text_iarchive ia(ss);
  ia >> id;
  ia >> return_value;

}

template <class T>
string serializeMsgToServer(const vector<T> &args) {

  std::stringstream ss;
  boost::archive::text_oarchive oa(ss);
  oa << args;
  return ss.str();
}




