/*
 * Copyright 2018 National Technology & Engineering Solutions of
 * Sandia, LLC (NTESS). Under the terms of Contract DE-NA0003525 with NTESS,
 * the U.S. Government retains certain rights in this software.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2018 Sandia Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

/* 
 * Copyright 2014 Sandia Corporation. Under the terms of Contract
 * DE-AC04-94AL85000, there is a non-exclusive license for use of this work by
 * or on behalf of the U.S. Government. Export of this program may require a
 * license from the United States Government.
 *
 * The MIT License (MIT)
 * 
 * Copyright (c) 2014 Sandia Corporation
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */


#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdint.h>
#include <assert.h>
#include <mpi.h>
#include <my_metadata_client.h>
#include <client_timing_constants.hh>
#include <OpActivateTypeMetaCommon.hh>
#include <OpActivateVarMetaCommon.hh>  
#include <OpCatalogVarMetaCommon.hh> 
#include <OpCatalogTypeMetaCommon.hh>                       
#include <OpCreateTypeMetaCommon.hh>          
#include <OpCreateVarMetaCommon.hh>          
#include <OpDeleteTypeMetaCommon.hh>           
#include <OpDeleteVarMetaCommon.hh>               
#include <OpFullShutdownMetaCommon.hh>     
#include <OpGetAttributeListMetaCommon.hh>     
#include <OpGetAttributeMetaCommon.hh>     
#include <OpGetChunkListMetaCommon.hh>
#include <OpGetChunkMetaCommon.hh>
#include <OpInsertAttributeMetaCommon.hh>
#include <OpInsertChunkMetaCommon.hh>
#include <OpProcessingTypeMetaCommon.hh>
#include <OpProcessingVarMetaCommon.hh>

#include <opbox/services/dirman/DirectoryManager.hh>
#include <Globals.hh>

using namespace std;
using namespace net;

static bool extreme_debug_logging = false;
static bool debug_logging = false;
static bool error_logging = false;
static errorLog error_log = errorLog(error_logging);
static debugLog debug_log = debugLog(debug_logging);
static extremeDebugLog extreme_debug_log = extremeDebugLog(extreme_debug_logging);

extern std::vector<int> catg_of_time_pts;
extern std::vector<std::chrono::high_resolution_clock::time_point> time_pts;


int metadata_init ()
{
    opbox::RegisterOp<OpActivateTypeMeta>();
    opbox::RegisterOp<OpActivateVarMeta>();    
    opbox::RegisterOp<OpCatalogVarMeta>();
    opbox::RegisterOp<OpCatalogTypeMeta>(); 
    opbox::RegisterOp<OpCreateTypeMeta>();
    opbox::RegisterOp<OpCreateVarMeta>();
    opbox::RegisterOp<OpDeleteTypeMeta>();
    opbox::RegisterOp<OpDeleteVarMeta>();
    opbox::RegisterOp<OpFullShutdownMeta>();
    opbox::RegisterOp<OpGetAttributeListMeta>();
    opbox::RegisterOp<OpGetAttributeMeta>();   
    opbox::RegisterOp<OpGetChunkListMeta>();
    opbox::RegisterOp<OpGetChunkMeta>();
    opbox::RegisterOp<OpInsertAttributeMeta>();
    opbox::RegisterOp<OpInsertChunkMeta>();
    opbox::RegisterOp<OpProcessingTypeMeta>();
    opbox::RegisterOp<OpProcessingVarMeta>();

    return RC_OK;
}

int metadata_finalize_server (const metadata_server &server)
{
    time_pts.push_back(chrono::high_resolution_clock::now());
    catg_of_time_pts.push_back(MD_FULL_SHUTDOWN_START);

    OpFullShutdownMeta *op = new OpFullShutdownMeta(server.peer_ptr);
    opbox::LaunchOp(op);  
    time_pts.push_back(chrono::high_resolution_clock::now());
    catg_of_time_pts.push_back(MD_FULL_SHUTDOWN_OP_DONE);
    //fix - anything else I need to do/free?

    return RC_OK;
}

// mark a variable in the global_catalog as active making it visible to other
// processes.
int metadata_activate_type (const metadata_server &server
                          , uint64_t txn_id
                          )
{
    time_pts.push_back(chrono::high_resolution_clock::now());
    catg_of_time_pts.push_back(MD_ACTIVATE_TYPE_START);

    int return_value;
    std::string res;

    md_activate_type_args args;

    args.txn_id = txn_id;

    OpActivateTypeMeta *op = new OpActivateTypeMeta(server.peer_ptr, args);
    future<std::string> fut = op->GetFuture();
    opbox::LaunchOp(op);

    res=fut.get();   

    op->deArchiveMsgFromServer(res, return_value);
    time_pts.push_back(chrono::high_resolution_clock::now());
    catg_of_time_pts.push_back(MD_ACTIVATE_TYPE_DEARCHIVE_RESPONSE_OP_DONE);   

    return return_value;
}

// mark all vars from the dataset as active making them visible to other
// processes.
int metadata_activate_var (const metadata_server &server
                          , uint64_t txn_id
                          )
{
    time_pts.push_back(chrono::high_resolution_clock::now());
    catg_of_time_pts.push_back(MD_ACTIVATE_VAR_START);

    int return_value;
    std::string res;

    md_activate_var_args args;

    args.txn_id = txn_id;

    OpActivateVarMeta *op = new OpActivateVarMeta(server.peer_ptr, args);
    future<std::string> fut = op->GetFuture();
    opbox::LaunchOp(op);

    res=fut.get();  

    op->deArchiveMsgFromServer(res, return_value);
    time_pts.push_back(chrono::high_resolution_clock::now());
    catg_of_time_pts.push_back(MD_ACTIVATE_VAR_DEARCHIVE_RESPONSE_OP_DONE);

    return return_value;
}

// return a catalog of items in the metadata service. For scalars, the num_dims
// will be 0
int metadata_catalog_var (const metadata_server &server,
                      uint64_t txn_id,
                      uint32_t &count,
                      std::vector<md_catalog_var_entry> &entries
                     )
{
    time_pts.push_back(chrono::high_resolution_clock::now());
    catg_of_time_pts.push_back(MD_CATALOG_VAR_START);

    int return_value;
    std::string res;

    md_catalog_args args;

    args.txn_id = txn_id;

    OpCatalogVarMeta *op = new OpCatalogVarMeta(server.peer_ptr, args);
    future<std::string> fut = op->GetFuture();
    opbox::LaunchOp(op);   

    res=fut.get();

    op->deArchiveMsgFromServer(res, entries, count, return_value);
    time_pts.push_back(chrono::high_resolution_clock::now());
    catg_of_time_pts.push_back(MD_CATALOG_VAR_DEARCHIVE_RESPONSE_OP_DONE);

    return return_value;
}

// return a catalog of types in the metadata type table. 
int metadata_catalog_type (const metadata_server &server,
                      uint64_t txn_id,
                      uint32_t &count,
                      std::vector<md_catalog_type_entry> &entries
                     )
{
    time_pts.push_back(chrono::high_resolution_clock::now());
    catg_of_time_pts.push_back(MD_CATALOG_TYPE_START);
    int return_value;
    std::string res;

    md_catalog_args args;

    args.txn_id = txn_id;

    OpCatalogTypeMeta *op = new OpCatalogTypeMeta(server.peer_ptr, args);
    future<std::string> fut = op->GetFuture();

    opbox::LaunchOp(op);

    res=fut.get();    

    op->deArchiveMsgFromServer(res, entries, count, return_value);
    time_pts.push_back(chrono::high_resolution_clock::now());
    catg_of_time_pts.push_back(MD_CATALOG_TYPE_DEARCHIVE_RESPONSE_OP_DONE);   

    return return_value;
}

// create a type in an inactive state. Attributes will be added later. Once the
// type is complete (the transaction is ready to commit), it can then be
// activated to make it visible to other processes.
int metadata_create_type (const metadata_server &server
                        ,uint64_t &type_id
                        ,const struct 
md_catalog_type_entry &new_type
                        )
{
    time_pts.push_back(chrono::high_resolution_clock::now());
    catg_of_time_pts.push_back(MD_CREATE_TYPE_START);
    extreme_debug_log << "got to top of metadata_create_type \n";
    int return_value;
    std::string res;

    md_create_type_args args;
    args.txn_id = new_type.txn_id;
    args.name = new_type.name;
    args.version = new_type.version;
    extreme_debug_log << "about to create new var \n";
    
    OpCreateTypeMeta *op = new OpCreateTypeMeta(server.peer_ptr, args);
    extreme_debug_log << "about to get future" <<endl;
    future<std::string> fut = op->GetFuture();
    extreme_debug_log << "about to launch op" <<endl;
    opbox::LaunchOp(op);
    extreme_debug_log << "about to get final result" <<endl;

    res = fut.get();

    op->deArchiveMsgFromServer(res, type_id, return_value);
    time_pts.push_back(chrono::high_resolution_clock::now());
    catg_of_time_pts.push_back(MD_CREATE_TYPE_DEARCHIVE_RESPONSE_OP_DONE);   

    return return_value;
}

// create a variable in an inactive state. Chunks will be added later. Once the
// variable is complete (the transaction is ready to commit), it can then be
// activated to make it visible to other processes.
// For a scalar, num_dims should be 0 causing the other parameters
// to be ignored.
int metadata_create_var (const metadata_server &server
                        ,uint64_t &var_id
                        ,const struct md_catalog_var_entry &new_var
                        )
{
    time_pts.push_back(chrono::high_resolution_clock::now());
    catg_of_time_pts.push_back(MD_CREATE_VAR_START);
    extreme_debug_log << "got to top of metadata_create_var \n";
    int return_value;
    std::string res;

    md_create_var_args args;
    args.txn_id = new_var.txn_id;
    args.name = new_var.name;
    args.path = new_var.path;
    args.type = new_var.type;
    args.version = new_var.version;
    args.num_dims = new_var.num_dims;
    args.dims = new_var.dims;
    extreme_debug_log << "about to create new var \n";
    
    OpCreateVarMeta *op = new OpCreateVarMeta(server.peer_ptr, args);    
    extreme_debug_log << "about to get future" <<endl;
    future<std::string> fut = op->GetFuture();
    extreme_debug_log << "about to launch op" <<endl;
    opbox::LaunchOp(op);
    extreme_debug_log << "about to get final result" <<endl;

    res = fut.get();

    op->deArchiveMsgFromServer(res, var_id, return_value);
    time_pts.push_back(chrono::high_resolution_clock::now());
    catg_of_time_pts.push_back(MD_CREATE_VAR_DEARCHIVE_RESPONSE_OP_DONE); 

    return return_value;
}

// delete the given type and all of the attributes associated with it
int metadata_delete_type (const metadata_server &server
                        ,uint64_t type_id
                        ,const std::string &name
                        ,uint32_t version
                        )
{
    time_pts.push_back(chrono::high_resolution_clock::now());
    catg_of_time_pts.push_back(MD_DELETE_TYPE_START);

    int return_value;
    std::string res;
    md_delete_type_args args;

    args.type_id = type_id;
    args.name = name;
    args.version = version;

    OpDeleteTypeMeta *op = new OpDeleteTypeMeta(server.peer_ptr, args);
    future<std::string> fut = op->GetFuture();
    opbox::LaunchOp(op);

    res=fut.get();

    op->deArchiveMsgFromServer(res, return_value);
    time_pts.push_back(chrono::high_resolution_clock::now());
    catg_of_time_pts.push_back(MD_DELETE_TYPE_DEARCHIVE_RESPONSE_OP_DONE);

    return return_value;
}

// delete all chunks (or just the scalar) associated with the specified var
int metadata_delete_var (const metadata_server &server
                        ,uint64_t var_id
                        ,const std::string &name
                        ,const std::string &path
                        ,uint32_t version
                        )
{
    time_pts.push_back(chrono::high_resolution_clock::now());
    catg_of_time_pts.push_back(MD_DELETE_VAR_START);

    int return_value;
    std::string res;
    md_delete_var_args args;

    args.var_id = var_id;
    args.name = name;
    args.path = path;
    args.version = version;

    OpDeleteVarMeta *op = new OpDeleteVarMeta(server.peer_ptr, args);
    future<std::string> fut = op->GetFuture();
    opbox::LaunchOp(op);

    res=fut.get();

    op->deArchiveMsgFromServer(res, return_value);
    time_pts.push_back(chrono::high_resolution_clock::now());
    catg_of_time_pts.push_back(MD_DELETE_VAR_DEARCHIVE_RESPONSE_OP_DONE);

    return return_value;
}

// retrieve a list of attributes associated with a specific chunk id
int metadata_get_attribute (const metadata_server &server
                       ,uint64_t txn_id
                       ,uint64_t chunk_id
                       ,uint32_t &count
                       ,std::vector<md_attribute_entry> &matching_attributes
                       )
{
    time_pts.push_back(chrono::high_resolution_clock::now());
    catg_of_time_pts.push_back(MD_GET_ATTR_START);

    int return_value;
    std::string res;

    md_get_attribute_args args;

    args.txn_id = txn_id;
    args.chunk_id = chunk_id;

    OpGetAttributeMeta *op = new OpGetAttributeMeta(server.peer_ptr, args);
    extreme_debug_log << "just launched get chunk meta \n";
    future<std::string> fut = op->GetFuture();
    opbox::LaunchOp(op);

    res=fut.get();    

    op->deArchiveMsgFromServer(res, matching_attributes, count, return_value);
    time_pts.push_back(chrono::high_resolution_clock::now());
    catg_of_time_pts.push_back(MD_GET_ATTR_DEARCHIVE_RESPONSE_OP_DONE);   
    extreme_debug_log << "just dearchived matching attributes message from server \n";
    extreme_debug_log << "numebr of matching_attributes is " << matching_attributes.size() << "\n";


    return return_value;
}


// retrieve the list all of attributes associated with a particular type
int metadata_get_attribute_list (const metadata_server &server
                            ,uint64_t txn_id
                            ,const std::string &type_name
                            ,uint32_t type_version
                            ,uint32_t &count
                            ,std::vector<md_attribute_entry> &entries
                            )
{
    time_pts.push_back(chrono::high_resolution_clock::now());
    catg_of_time_pts.push_back(MD_GET_ATTR_LIST_START);
    int return_value;
    std::string res;
    

    md_get_attribute_list_args args;

    args.txn_id = txn_id;
    args.type_name = type_name;
    args.type_version = type_version;
    extreme_debug_log << "type version is " << to_string(type_version ) << endl;  
    extreme_debug_log << " args.type_version is " <<  to_string(args.type_version ) << endl;  
   
    OpGetAttributeListMeta *op = new OpGetAttributeListMeta(server.peer_ptr, args);
    future<std::string> fut = op->GetFuture();
    opbox::LaunchOp(op);

    res=fut.get();

    op->deArchiveMsgFromServer(res, entries, count, return_value);
    time_pts.push_back(chrono::high_resolution_clock::now());
    catg_of_time_pts.push_back(MD_GET_ATTR_LIST_DEARCHIVE_RESPONSE_OP_DONE);  

    return return_value;
}


// retrieve a list of chunks of a var that have data within a specified range.
int metadata_get_chunk (const metadata_server &server
                       ,const struct md_catalog_var_entry &desired_box
                       ,uint32_t &count
                       ,std::vector<md_chunk_entry> &matching_chunks
                       )
{
    time_pts.push_back(chrono::high_resolution_clock::now());
    catg_of_time_pts.push_back(MD_GET_CHUNK_START);

    int return_value;
    std::string res;

    md_get_chunk_args args;

    args.txn_id = desired_box.txn_id;
    args.name = desired_box.name;
    args.path = desired_box.path;
    args.var_version = desired_box.version;
    args.num_dims = desired_box.num_dims;
    args.dims = desired_box.dims;

    OpGetChunkMeta *op = new OpGetChunkMeta(server.peer_ptr, args);
    extreme_debug_log << "just launched get chunk meta \n";
    future<std::string> fut = op->GetFuture();
    opbox::LaunchOp(op);

    res=fut.get();    

    op->deArchiveMsgFromServer(res, matching_chunks, count, return_value);
    time_pts.push_back(chrono::high_resolution_clock::now());
    catg_of_time_pts.push_back(MD_GET_CHUNK_DEARCHIVE_RESPONSE_OP_DONE);  
    extreme_debug_log << "just dearchived matching chunks message from server \n";
    extreme_debug_log << "numebr of  matching chunks is " << matching_chunks.size() << "\n";


    return return_value;
}

// retrieve the list of chunks that comprise a var.
int metadata_get_chunk_list (const metadata_server &server
                            ,uint64_t txn_id
                            ,const std::string &name
                            ,const std::string &path
                            ,uint32_t version
                            ,uint32_t &count
                            ,std::vector<md_chunk_entry> &entries
                            )
{
    time_pts.push_back(chrono::high_resolution_clock::now());
    catg_of_time_pts.push_back(MD_GET_CHUNK_LIST_START);
    int return_value;
    std::string res;

    md_get_chunk_list_args args;

    args.txn_id = txn_id;
    args.name = name;
    args.path = path;
    args.var_version = version;

    OpGetChunkListMeta *op = new OpGetChunkListMeta(server.peer_ptr, args);
    future<std::string> fut = op->GetFuture();
    opbox::LaunchOp(op);

    res=fut.get();    

    op->deArchiveMsgFromServer(res, entries, count, return_value);
    time_pts.push_back(chrono::high_resolution_clock::now());
    catg_of_time_pts.push_back(MD_GET_CHUNK_LIST_DEARCHIVE_RESPONSE_OP_DONE);   

    return return_value;
}

// insert a attribute of a chunk
int metadata_insert_attribute (const metadata_server &server,
                           uint64_t &attribute_id,
                           const struct md_attribute_entry &new_attribute)
{
    time_pts.push_back(chrono::high_resolution_clock::now());
    catg_of_time_pts.push_back(MD_INSERT_ATTR_START);
    int return_value;
    std::string res;

    if(time_pts.size() != catg_of_time_pts.size()) {
        debug_log << "error they are off at start of md insert attributes " << endl;
    }
    md_insert_attribute_args args;
    args.chunk_id = new_attribute.chunk_id;
    args.type_id = new_attribute.type_id;
    args.data = new_attribute.data;

    OpInsertAttributeMeta *op = new OpInsertAttributeMeta(server.peer_ptr, args);
    future<std::string> fut = op->GetFuture();
    opbox::LaunchOp(op);

    res=fut.get();

    op->deArchiveMsgFromServer(res, return_value, attribute_id);
    time_pts.push_back(chrono::high_resolution_clock::now());
    catg_of_time_pts.push_back(MD_INSERT_ATTR_DEARCHIVE_RESPONSE_OP_DONE);   
    debug_log << "according to the client the attribute_id is " << attribute_id << endl;
    if(time_pts.size() != catg_of_time_pts.size()) {
        debug_log << "error they are off at the end of md insert attributes " << endl;
    }
    return return_value;
}

// insert a chunk of an array
int metadata_insert_chunk (const metadata_server &server,
                           uint64_t var_id, 
                           uint64_t &chunk_id,
                           const struct md_chunk_entry &new_chunk)
{
    time_pts.push_back(chrono::high_resolution_clock::now());
    catg_of_time_pts.push_back(MD_INSERT_CHUNK_START);
    int return_value;
    std::string res;

    md_insert_chunk_args args;
    args.var_id = var_id;
    args.num_dims = new_chunk.num_dims;
    args.connection = new_chunk.connection;
    args.length_of_chunk = new_chunk.length_of_chunk;
    args.dims = new_chunk.dims;
    extreme_debug_log << "new chunk connection is " << args.connection << std::endl;

    OpInsertChunkMeta *op = new OpInsertChunkMeta(server.peer_ptr, args);
    future<std::string> fut = op->GetFuture();
    opbox::LaunchOp(op);

    res=fut.get();

    op->deArchiveMsgFromServer(res, return_value, chunk_id);
    time_pts.push_back(chrono::high_resolution_clock::now());
    catg_of_time_pts.push_back(MD_INSERT_CHUNK_DEARCHIVE_RESPONSE_OP_DONE);     
    extreme_debug_log << "according to the client the chunk id is " << chunk_id << endl;

    return return_value;
}

// mark a variable in the global_catalog as processing making it invisible to
// other processes.
int metadata_processing_type (const metadata_server &server
                          ,uint64_t txn_id
                          )
{
    time_pts.push_back(chrono::high_resolution_clock::now());
    catg_of_time_pts.push_back(MD_PROCESSING_TYPE_START);

    int return_value;
    std::string res;
    md_processing_type_args args;

    args.txn_id = txn_id;

    OpProcessingTypeMeta *op = new OpProcessingTypeMeta(server.peer_ptr, args);
    future<std::string> fut = op->GetFuture();
    opbox::LaunchOp(op);

    res=fut.get();

    op->deArchiveMsgFromServer(res, return_value);
    time_pts.push_back(chrono::high_resolution_clock::now());
    catg_of_time_pts.push_back(MD_PROCESSING_TYPE_DEARCHIVE_RESPONSE_OP_DONE);

    return return_value;
}

// mark a variable in the global_catalog as processing making it invisible to
// other processes.
int metadata_processing_var (const metadata_server &server
                          ,uint64_t txn_id
                          )
{
    time_pts.push_back(chrono::high_resolution_clock::now());
    catg_of_time_pts.push_back(MD_PROCESSING_VAR_START);

    int return_value;
    std::string res;
    md_processing_var_args args;

    args.txn_id = txn_id;


    OpProcessingVarMeta *op = new OpProcessingVarMeta(server.peer_ptr, args);
    future<std::string> fut = op->GetFuture();
    opbox::LaunchOp(op);

    res=fut.get();    

    op->deArchiveMsgFromServer(res, return_value);
    time_pts.push_back(chrono::high_resolution_clock::now());
    catg_of_time_pts.push_back(MD_PROCESSING_VAR_DEARCHIVE_RESPONSE_OP_DONE);

    return return_value;
}



/*
 * Prints information about the given number of catalog entries from the given list of entries
 *
 * Inputs: 
 *   num_vars - number of catalog entries you want to print information for
 *   entries - the list of catalog entries you want to print from
 */
void print_var_catalog (uint32_t num_vars, const std::vector<md_catalog_var_entry> &entries)
{
    for (int i = 0; i < num_vars; i++)
    {
        
        std::cout << "var_id: " << entries [i].var_id << " name: " << entries [i].name << " path: " << entries [i].path << " version: " << entries [i].version << " num_dims: " << entries [i].num_dims << endl;
        char v = 'x';
        for (int j = 0; j < entries [i].num_dims; j++)
        {
            std::cout << " " << v <<": (" << entries [i].dims [j].min << "/" << entries [i].dims [j].max << ") ";
            v++;
        }
        std::cout << ("\n");
    }
}

/*
 * Prints information about the given number of chunks from the given list of chunks
 *
 * Inputs: 
 *   num_vars - number of chunks you want to print information for
 *   entries - the list of chunks you want to print from
 */
void print_chunk_list (uint32_t num_vars, const std::vector<md_chunk_entry> &chunks)
{
    for (int i = 0; i < num_vars; i++)
    {
        std::cout << "chunk id: " << chunks [i].chunk_id << " connection: " << chunks [i].connection << " length: " << chunks [i].length_of_chunk << " num_dims: " << chunks [i].num_dims;
        char v = 'x';
        for (int j = 0; j < chunks [i].num_dims; j++)
        {
            std::cout << " " << v << ": (" << chunks [i].dims [j].min << "/" << chunks [i].dims [j].max << ") ";
            v++;
        }
        std::cout << "\n";
    }
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
    for (int i = 0; i < num_vars; i++)
    {
        
        std::cout << "type_id: " << entries [i].type_id << " name: " << entries [i].name << " version: " <<
        entries [i].version <<  " active: " << entries [i].active << " txn_id: " << entries [i].txn_id << endl;
    }

    if (num_vars == 0) {
        std::cout << "The catalog is empty \n";
    }
}


/*
 * Prints information about the given number of attributes from the given list of attributes
 *
 * Inputs: 
 *   num_vars - number of attributes you want to print information for
 *   entries - the list of attributes you want to print from
 */
void print_attribute_list (uint32_t num_vars, const std::vector<md_attribute_entry> &attributes)
{
    for (int i = 0; i < num_vars; i++)
    {
        std::cout << "attribute id: " << attributes [i].attribute_id << " chunk id: " << attributes [i].chunk_id << 
        " type id: " << attributes [i].type_id << endl;

    }

    if (num_vars == 0) {
        std::cout << "There are no matching attributes \n";
    }
}
