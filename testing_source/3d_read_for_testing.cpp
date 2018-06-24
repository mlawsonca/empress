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

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>

#include <3d_read_for_testing.hh>
#include <client_timing_constants.hh>
#include <OpCatalogVarMetaCommon.hh> 
#include <OpGetAttributeMetaCommon.hh>
#include <OpGetChunkMetaCommon.hh>


using namespace std;

extern std::vector<int> catg_of_time_pts;
extern std::vector<std::chrono::high_resolution_clock::time_point> time_pts;

static bool extreme_debug_logging = false;
static bool debug_logging = false;
static bool error_logging = false;
static errorLog error_log = errorLog(error_logging);
static debugLog debug_log = debugLog(debug_logging);
static extremeDebugLog extreme_debug_log = extremeDebugLog(extreme_debug_logging);

static bool do_debug_testing = false;


// 1. return chunks for all vars
int read_pattern_1 (int rank, int num_servers, std::vector<metadata_server> servers, 
                    bool is_type, int num_x_procs, int num_y_procs, 
                    int num_z_procs, std::vector<md_catalog_var_entry> entries, int num_vars,
                    int total_x_length, int total_y_length, int total_z_length, uint64_t txn_id)
{
    
    time_pts.push_back(chrono::high_resolution_clock::now());
    catg_of_time_pts.push_back(READ_PATTERN_1_START);

    int rc;

    md_catalog_var_entry var;

    uint32_t num_chunks;
    std::vector<md_chunk_entry> chunks;
    uint32_t num_attributes;
    std::vector<md_attribute_entry> attributes;

    int x_length_per_proc = total_x_length / num_x_procs; //fix - deal with edge case?
    int y_length_per_proc = total_y_length / num_y_procs;
    int z_length_per_proc = total_z_length / num_z_procs;

    int x_pos = rank % num_x_procs;
    int y_pos = (rank / num_x_procs) % num_y_procs;
    int z_pos = ((rank / (num_x_procs * num_y_procs)) % num_z_procs);

    int x_offset = x_pos * x_length_per_proc;
    int y_offset = y_pos * y_length_per_proc;
    int z_offset = z_pos * z_length_per_proc;

    extreme_debug_log << "about to assign var info \n";
    extreme_debug_log << "txn_id: " << to_string(txn_id) << endl;
    extreme_debug_log << "x_pos: " << x_pos << endl;
    extreme_debug_log << "y_pos: " << y_pos << endl;
    extreme_debug_log << "z_pos: " << z_pos << endl;
    extreme_debug_log << "x_offset : " << x_offset  << endl;
    extreme_debug_log << "y_offset : " << y_offset  << endl;
    extreme_debug_log << "z_offset : " << z_offset  << endl;
    extreme_debug_log << "x_length_per_proc : " << x_length_per_proc  << endl;
    extreme_debug_log << "y_length_per_proc : " << y_length_per_proc  << endl;
    extreme_debug_log << "z_length_per_proc : " << z_length_per_proc  << endl;

    time_pts.push_back(chrono::high_resolution_clock::now());
    catg_of_time_pts.push_back(READ_PATTERN_1_VAR_INIT_DONE_START_READING);

    //each proc is responsible for a certain area of the sim space
    //it must get all the chunk info for each var by asking each server
    for(int i=0; i<num_vars; i++) {
        time_pts.push_back(chrono::high_resolution_clock::now());
        catg_of_time_pts.push_back(READ_PATTERN_1_START_READING_VAR);
        debug_log << "beginning to fetch chunks" << endl;
        if( i == 0 ) {
            var = entries.at(0);
            var.dims [0].min = x_offset;
            var.dims [0].max = x_offset + x_length_per_proc - 1;
            var.dims [1].min = y_offset;
            var.dims [1].max = y_offset + y_length_per_proc - 1;
            var.dims [2].min = z_offset;
            var.dims [2].max = z_offset + z_length_per_proc - 1;

            extreme_debug_log << "var txn_id is " << var.txn_id << endl;
            extreme_debug_log << "var num_dims is " << var.num_dims << endl;     
            debug_log << "read pattern 1" << endl;
            for (int j=0; j<3; j++) {
                debug_log <<  "dims [" << to_string(j) << "] min is: " << to_string(var.dims [j].min) << endl;
                debug_log <<  "dims [" << to_string(j) << "] max is: " << to_string(var.dims [j].max) << endl;
            }  
        }
        else { //have already assigned this for the 0th var
            md_catalog_var_entry temp_var = entries.at(i);
            var.name = temp_var.name;
            var.path = temp_var.path;
            var.version = temp_var.version;    
        }

        extreme_debug_log << "var name is " << var.name << endl;
        extreme_debug_log << "var path is " << var.path << endl;
        extreme_debug_log << "var version is " << var.version << endl;
        extreme_debug_log << "num_servers is " << num_servers << endl;

        for (int j=0; j < num_servers; j++) { 
            time_pts.push_back(chrono::high_resolution_clock::now());
            catg_of_time_pts.push_back(READ_PATTERN_1_START_READING_FROM_NEW_SERVER);
            extreme_debug_log << "j: " << j << endl;
            rc = metadata_get_chunk (servers.at(j), var, num_chunks, chunks);
            extreme_debug_log << "rc: " << rc << endl;
            if (rc != RC_OK) {
                error_log << "Error retrieving chunk \n";
                time_pts.push_back(chrono::high_resolution_clock::now());
                catg_of_time_pts.push_back(ERR_GET_CHUNK);
                goto cleanup;
            }
            else{
                if(do_debug_testing) {
                    debug_log<< "chunk list for varid: " << var.var_id << " txn_id: " << var.txn_id << " name: " 
                          << var.name << " path: " << var.path << " version: " << var.version 
                          << " (count: " << num_chunks << ") \n";
                    print_chunk_list (num_chunks, chunks);                     
                }
            }    

            if( is_type ) {
                time_pts.push_back(chrono::high_resolution_clock::now());
                catg_of_time_pts.push_back(READ_PATTERN_1_START_GET_ATTRS);

                for (int i=0; i < num_chunks; i++) {
                    rc = metadata_get_attribute(servers.at(j), txn_id, chunks.at(i).chunk_id, num_attributes, attributes);
                    if (rc != RC_OK) {
                        error_log << "Error retrieving attribute \n";
                        time_pts.push_back(chrono::high_resolution_clock::now());
                        catg_of_time_pts.push_back(ERR_GET_ATTR);
                    }
                    else {
                        if(do_debug_testing) {
                            debug_log << "num attributes: " << num_attributes << endl;  
                            print_attribute_list(num_attributes, attributes);
                        }
                    }
                }
                time_pts.push_back(chrono::high_resolution_clock::now());
                catg_of_time_pts.push_back(READ_PATTERN_1_DONE_GET_ATTRS);
            }
            time_pts.push_back(chrono::high_resolution_clock::now());
            catg_of_time_pts.push_back(READ_PATTERN_1_DONE_READING_FROM_SERVER);
        }
        time_pts.push_back(chrono::high_resolution_clock::now());
        catg_of_time_pts.push_back(READ_PATTERN_1_DONE_READING_VAR); 
    }
    time_pts.push_back(chrono::high_resolution_clock::now());
    catg_of_time_pts.push_back(READ_PATTERN_1_DONE_READING); 

cleanup:
    return rc;
}


// 2. all of 1 var
int read_pattern_2 (MPI_Comm comm, int rank, int num_servers, std::vector<metadata_server> servers, 
                    bool is_type, int num_x_procs, int num_y_procs, 
                    int num_z_procs, std::vector<md_catalog_var_entry> entries, int num_vars,
                    int nx, int ny, int nz, uint64_t txn_id, uint64_t var_id)
{
    
    
    time_pts.push_back(chrono::high_resolution_clock::now());
    catg_of_time_pts.push_back(READ_PATTERN_2_START);

    int rc;
    int index_of_var;
    md_catalog_var_entry var;

    uint32_t num_chunks;
    std::vector<md_chunk_entry> chunks;
    uint32_t num_attributes;
    std::vector<md_attribute_entry> attributes;

    int x_length_per_proc = nx / num_x_procs; //fix - deal with edge case?
    int y_length_per_proc = ny / num_y_procs;
    int z_length_per_proc = nz / num_z_procs;

    int x_pos = rank % num_x_procs;
    int y_pos = (rank / num_x_procs) % num_y_procs;
    int z_pos = ((rank / (num_x_procs * num_y_procs)) % num_z_procs);
    int x_offset = x_pos * x_length_per_proc;
    int y_offset = y_pos * y_length_per_proc;
    int z_offset = z_pos * z_length_per_proc;

    time_pts.push_back(chrono::high_resolution_clock::now());
    catg_of_time_pts.push_back(READ_PATTERN_2_VAR_INIT_DONE);

    if (rank == 0) {
        int i=0;
        while (i < num_vars && entries.at(i).var_id != var_id) {
            i++;
        }
        if (i == num_vars) {
            time_pts.push_back(chrono::high_resolution_clock::now());
            catg_of_time_pts.push_back(ERR_VAR_ID_NOT_FOUND);
            error_log << "Error pattern 2. The given var id " << var_id << " was not found in the catalog for txnid: " << txn_id << ". Exiting \n";
            goto cleanup;
        }
        index_of_var = i;
    }

    MPI_Bcast(&index_of_var, 1, MPI_INT, 0, comm);

    time_pts.push_back(chrono::high_resolution_clock::now());
    catg_of_time_pts.push_back(READ_PATTERN_2_BCAST_DONE_START_READING_VAR);

    for (int j=0; j < num_servers; j++) {
        time_pts.push_back(chrono::high_resolution_clock::now());
        catg_of_time_pts.push_back(READ_PATTERN_2_START_READING_FROM_NEW_SERVER);

        if(j == 0) {
            var = entries.at(index_of_var);
            var.txn_id = txn_id;
            var.dims [0].min = x_offset;
            var.dims [0].max = x_offset + x_length_per_proc - 1;
            var.dims [1].min = y_offset;
            var.dims [1].max = y_offset + y_length_per_proc - 1;
            var.dims [2].min = z_offset;
            var.dims [2].max = z_offset + z_length_per_proc - 1;

            extreme_debug_log << "read pattern 2" << endl;
            for (int j=0; j<3; j++) {
                extreme_debug_log <<  "dims [" << to_string(j) << "] min is: " << to_string(var.dims [j].min) << endl;
                extreme_debug_log <<  "dims [" << to_string(j) << "] max is: " << to_string(var.dims [j].max) << endl;
            }  
        }
        rc = metadata_get_chunk (servers.at(j), var, num_chunks, chunks);
        if (rc != RC_OK) {
            time_pts.push_back(chrono::high_resolution_clock::now());
            catg_of_time_pts.push_back(ERR_GET_CHUNK);
            goto cleanup;

        }
        else{
            if(do_debug_testing) {
                debug_log << "chunk list for varid: " << var.var_id << " txn_id: " << var.txn_id << " name: " 
                      << var.name << " path: " << var.path << " version: " << var.version 
                      << " (count: " << num_chunks << ") \n";
                print_chunk_list (num_chunks, chunks); 
            }
        }

        if( is_type ) {
            time_pts.push_back(chrono::high_resolution_clock::now());
            catg_of_time_pts.push_back(READ_PATTERN_2_START_GET_ATTRS);

            for (int i=0; i < num_chunks; i++) {
                rc = metadata_get_attribute(servers.at(j), txn_id, chunks.at(i).chunk_id, num_attributes, attributes);
                if (rc != RC_OK) {

                    time_pts.push_back(chrono::high_resolution_clock::now());
                    catg_of_time_pts.push_back(ERR_GET_ATTR);                
                }
                else {
                    if(do_debug_testing) {
                        debug_log << "num attributes: " << num_attributes << endl;  
                        print_attribute_list(num_attributes, attributes); 
                    }        
                }
            }
            time_pts.push_back(chrono::high_resolution_clock::now());
            catg_of_time_pts.push_back(READ_PATTERN_2_DONE_GET_ATTRS);
        }
        time_pts.push_back(chrono::high_resolution_clock::now());
        catg_of_time_pts.push_back(READ_PATTERN_2_DONE_READING_FROM_SERVER);
    }
    time_pts.push_back(chrono::high_resolution_clock::now());
    catg_of_time_pts.push_back(READ_PATTERN_2_DONE_READING);    

cleanup:

    return rc;
}

// 3. all of a few vars (3 for 3-d, for example)
int read_pattern_3 (MPI_Comm comm, int rank, int num_servers, std::vector<metadata_server> servers, 
                    bool is_type, int num_x_procs, int num_y_procs, int num_z_procs, 
                    std::vector<md_catalog_var_entry> entries, int num_vars, int total_x_length, 
                    int total_y_length, int total_z_length, uint64_t txn_id, 
                    vector<uint64_t> var_ids, int num_vars_to_fetch) 
{
    
    time_pts.push_back(chrono::high_resolution_clock::now());
    catg_of_time_pts.push_back(READ_PATTERN_3_START);

    int rc;

    md_catalog_var_entry var;
    int indices[num_vars_to_fetch];

    uint32_t num_chunks;
    std::vector<md_chunk_entry> chunks;
    uint32_t num_attributes;
    std::vector<md_attribute_entry> attributes;

    int x_length_per_proc = total_x_length / num_x_procs; //fix - deal with edge case?
    int y_length_per_proc = total_y_length / num_y_procs;
    int z_length_per_proc = total_z_length / num_z_procs;

    int x_pos = rank % num_x_procs;
    int y_pos = (rank / num_x_procs) % num_y_procs;
    int z_pos = ((rank / (num_x_procs * num_y_procs)) % num_z_procs);
    int x_offset = x_pos * x_length_per_proc;
    int y_offset = y_pos * y_length_per_proc;
    int z_offset = z_pos * z_length_per_proc;

    time_pts.push_back(chrono::high_resolution_clock::now());
    catg_of_time_pts.push_back(READ_PATTERN_3_VAR_INIT_DONE);

    if (rank == 0) {
        for(int i=0; i<num_vars_to_fetch; i++) {
            uint64_t var_id = var_ids.at(i);
            debug_log << "var_id: " << to_string(var_id) << endl;

            int j = 0;
            while (j < num_vars && entries.at(j).var_id != var_id) {
                j++;
            }
            if (j == num_vars) {
                time_pts.push_back(chrono::high_resolution_clock::now());
                catg_of_time_pts.push_back(ERR_VAR_ID_NOT_FOUND);
                error_log << "Error pattern 3. The given var id " << var_id << " was not found in the catalog for txnid: " << txn_id << ". Exiting \n";
                goto cleanup;
            }
            extreme_debug_log << "final j: " << j << endl;

            indices[i] = j;
        }
        debug_log << "bcaster thinks entries.size is " << entries.size() << endl;
    }

    MPI_Bcast(indices, num_vars_to_fetch, MPI_INT, 0, comm);

    time_pts.push_back(chrono::high_resolution_clock::now());
    catg_of_time_pts.push_back(READ_PATTERN_3_BCAST_DONE_START_READING);
    
    extreme_debug_log << "x_pos: " << x_pos << endl;
    extreme_debug_log << "y_pos: " << y_pos << endl;
    extreme_debug_log << "z_pos: " << z_pos << endl;
    extreme_debug_log << "x_offset : " << x_offset  << endl;
    extreme_debug_log << "y_offset : " << y_offset  << endl;
    extreme_debug_log << "z_offset : " << z_offset  << endl;
    extreme_debug_log << "x_length_per_proc : " << x_length_per_proc  << endl;
    extreme_debug_log << "y_length_per_proc : " << y_length_per_proc  << endl;
    extreme_debug_log << "z_length_per_proc : " << z_length_per_proc  << endl;
  
    //one proc is in charge of a certain section of the sim space
    //it has to get all chunk info for all vars in that space, this involves asking each server for the info
    for (int k=0; k < num_vars_to_fetch; k++) {
        time_pts.push_back(chrono::high_resolution_clock::now());
        catg_of_time_pts.push_back(READ_PATTERN_3_START_READING_VAR);

        if(k == 0) {
            if (0 < num_vars_to_fetch) {
                var = entries.at(indices[0]);
                var.dims [0].min = x_offset;
                var.dims [0].max = x_offset + x_length_per_proc - 1;
                var.dims [1].min = y_offset;
                var.dims [1].max = y_offset + y_length_per_proc - 1;
                var.dims [2].min = z_offset;
                var.dims [2].max = z_offset + z_length_per_proc - 1;
                
                debug_log << "read pattern 3 var: "<< to_string(k) << endl;
                for (int j=0; j<3; j++) {
                    debug_log <<  "dims [" << to_string(j) << "] min is: " << to_string(var.dims [j].min) << endl;
                    debug_log <<  "dims [" << to_string(j) << "] max is: " << to_string(var.dims [j].max) << endl;
                }      
            }
        }
        else { 
            extreme_debug_log << "k: " << k << endl;
            md_catalog_var_entry temp_var = entries.at(indices[k]);
            var.name = temp_var.name;
            var.path = temp_var.path;
            var.version = temp_var.version;     
            extreme_debug_log << "name: " << var.name << endl;  
            extreme_debug_log << "path: " << var.path << endl;  
            extreme_debug_log << "version: " << var.version << endl; 
            debug_log << "read pattern 3 var: "<< to_string(k) << endl;
            for (int j=0; j<3; j++) {
                extreme_debug_log <<  "dims [" << to_string(j) << "] min is: " << to_string(var.dims [j].min) << endl;
                extreme_debug_log <<  "dims [" << to_string(j) << "] max is: " << to_string(var.dims [j].max) << endl;
            }     
        }

        time_pts.push_back(chrono::high_resolution_clock::now());
        catg_of_time_pts.push_back(READ_PATTERN_3_VAR_TO_FIND_INIT_DONE);        

        for (int i=0; i < num_servers; i++) {
            time_pts.push_back(chrono::high_resolution_clock::now());
            catg_of_time_pts.push_back(READ_PATTERN_3_START_READING_FROM_NEW_SERVER);

            rc = metadata_get_chunk (servers.at(i), var, num_chunks, chunks);
            if (rc != RC_OK) {
                time_pts.push_back(chrono::high_resolution_clock::now());
                catg_of_time_pts.push_back(ERR_GET_CHUNK);
                error_log << "Error retrieving chunk \n";
                goto cleanup;
            }
            else{
                if(do_debug_testing) {
                    debug_log << "chunk list for varid: " << var.var_id << " txn_id: " << var.txn_id << " name: " 
                      << var.name << " path: " << var.path << " version: " << var.version 
                      << " (count: " << num_chunks << ") \n";
                    print_chunk_list (num_chunks, chunks);    
                }
            }
        
            if( is_type ) {
                time_pts.push_back(chrono::high_resolution_clock::now());
                catg_of_time_pts.push_back(READ_PATTERN_3_START_GET_ATTRS);

                for (int j=0; j < num_chunks; j++) {
                    rc = metadata_get_attribute(servers.at(i), txn_id, chunks.at(j).chunk_id, num_attributes, attributes);
                    if (rc != RC_OK) {
                        time_pts.push_back(chrono::high_resolution_clock::now());
                        catg_of_time_pts.push_back(ERR_GET_ATTR);
                    }
                    else {
                        if(do_debug_testing) {
                            debug_log << "num attributes: " << num_attributes << endl;  
                            print_attribute_list(num_attributes, attributes); 
                        }        
                    }
                }
                time_pts.push_back(chrono::high_resolution_clock::now());
                catg_of_time_pts.push_back(READ_PATTERN_3_DONE_GET_ATTRS); 
            }
            time_pts.push_back(chrono::high_resolution_clock::now());
            catg_of_time_pts.push_back(READ_PATTERN_3_DONE_READING_FROM_SERVER); 
        }
        time_pts.push_back(chrono::high_resolution_clock::now());
        catg_of_time_pts.push_back(READ_PATTERN_3_DONE_READING_VAR); 
    }
    time_pts.push_back(chrono::high_resolution_clock::now());
    catg_of_time_pts.push_back(READ_PATTERN_3_DONE_READING); 

cleanup:
    return rc;
}


// 4. 1 plane in each dimension for 1 variable
int read_pattern_4 (MPI_Comm comm, int rank, int num_servers, std::vector<metadata_server> servers, 
                    bool is_type, int num_x_procs, int num_y_procs, 
                    std::vector<md_catalog_var_entry> entries, int num_vars,
                    int nx, int ny, int nz, uint64_t txn_id, uint64_t var_id)
{
    
    time_pts.push_back(chrono::high_resolution_clock::now());
    catg_of_time_pts.push_back(READ_PATTERN_4_START);

    int rc;
    
    int index_of_var;
    md_catalog_var_entry var;

    uint32_t num_chunks;
    std::vector<md_chunk_entry> chunks;
    uint32_t num_attributes;
    std::vector<md_attribute_entry> attributes;

    int my_x_dim;
    int my_y_dim;

    int x_offset;
    int y_offset;

     if (rank == 0) {
        int i=0;
        while (i < num_vars && entries.at(i).var_id != var_id) {
            i++;
        }
        if (i == num_vars) {
            time_pts.push_back(chrono::high_resolution_clock::now());
            catg_of_time_pts.push_back(ERR_VAR_ID_NOT_FOUND);
            error_log << "Error pattern 4. The given var id " << var_id << " was not found in the catalog for txnid: " << txn_id << ". Exiting \n";
            goto cleanup;
        }
        index_of_var = i;
    }

    MPI_Bcast(&index_of_var, 1, MPI_INT, 0, comm);

    time_pts.push_back(chrono::high_resolution_clock::now());
    catg_of_time_pts.push_back(READ_PATTERN_4_BCAST_DONE_START_READING);

    var = entries.at(index_of_var);

//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    //! do a plane in dim(3)
    my_x_dim = ny / num_x_procs;
    my_y_dim = nz / num_y_procs;

    x_offset = (rank % num_x_procs) * my_x_dim;
    y_offset = (rank/num_x_procs) * my_y_dim;

    extreme_debug_log << "my_x_dim: " << my_x_dim << endl;
    extreme_debug_log << "my_y_dim: " << my_y_dim << endl;
    extreme_debug_log << "x_offset: " << x_offset << endl;
    extreme_debug_log << "y_offset: " << y_offset << endl;

    var.txn_id = txn_id;
    var.dims [0].min = nx / 2;
    var.dims [0].max = nx /2;
    var.dims [1].min = x_offset;
    var.dims [1].max = x_offset + my_x_dim - 1;
    var.dims [2].min = y_offset;
    var.dims [2].max = y_offset + my_y_dim - 1;
    
    debug_log << "read pattern 4 plane 1" << endl;
    for (int j=0; j<3; j++) {
        debug_log <<  "dims [" << to_string(j) << "] min is: " << to_string(var.dims [j].min) << endl;
        debug_log <<  "dims [" << to_string(j) << "] max is: " << to_string(var.dims [j].max) << endl;
    }  

    time_pts.push_back(chrono::high_resolution_clock::now());
    catg_of_time_pts.push_back(READ_PATTERN_4_PLANE_TO_FIND_INIT_DONE_START_READING_PLANE); 

    for (int i=0; i < num_servers; i++) {
        time_pts.push_back(chrono::high_resolution_clock::now());
        catg_of_time_pts.push_back(READ_PATTERN_4_START_READING_FROM_NEW_SERVER); 

        rc = metadata_get_chunk (servers.at(i), var, num_chunks, chunks);
        if (rc != RC_OK) {
            time_pts.push_back(chrono::high_resolution_clock::now());
            catg_of_time_pts.push_back(ERR_GET_CHUNK);
            error_log << "Error retrieving chunk \n";        
            goto cleanup;
        }
        else{
            if(do_debug_testing) {
                debug_log << "chunk list for varid: " << var.var_id << " txn_id: " << var.txn_id << " name: " 
                      << var.name << " path: " << var.path << " version: " << var.version 
                      << " (count: " << num_chunks << ") \n";
                print_chunk_list (num_chunks, chunks);  
            }      
        }

        if( is_type ) {
            time_pts.push_back(chrono::high_resolution_clock::now());
            catg_of_time_pts.push_back(READ_PATTERN_4_START_GET_ATTRS);

            for (int j=0; j < num_chunks; j++) {
                rc = metadata_get_attribute(servers.at(i), txn_id, chunks.at(j).chunk_id, num_attributes, attributes);
                if (rc != RC_OK) {
                    error_log << "Error retrieving attribute \n";
                    time_pts.push_back(chrono::high_resolution_clock::now());
                    catg_of_time_pts.push_back(ERR_GET_ATTR);
                }
                else {
                    if(do_debug_testing) {
                        debug_log << "num attributes: " << num_attributes << endl;  
                        print_attribute_list(num_attributes, attributes); 
                    }        
                }
            }
            time_pts.push_back(chrono::high_resolution_clock::now());
            catg_of_time_pts.push_back(READ_PATTERN_4_DONE_GET_ATTRS);
        }
        time_pts.push_back(chrono::high_resolution_clock::now());
        catg_of_time_pts.push_back(READ_PATTERN_4_DONE_READING_FROM_SERVER);
    }
    time_pts.push_back(chrono::high_resolution_clock::now());
    catg_of_time_pts.push_back(READ_PATTERN_4_DONE_READING_PLANE);
 
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    //! do a plane in dim(2)
    my_x_dim = nx / num_x_procs;
    my_y_dim = nz / num_y_procs;
    x_offset = (rank % num_x_procs) * my_x_dim;
    y_offset = (rank/num_x_procs) * my_y_dim;

    extreme_debug_log << "my_x_dim: " << my_x_dim << endl;
    extreme_debug_log << "my_y_dim: " << my_y_dim << endl;
    extreme_debug_log << "x_offset: " << x_offset << endl;
    extreme_debug_log << "y_offset: " << y_offset << endl;

    var.dims [0].min = x_offset;
    var.dims [0].max = x_offset + my_x_dim - 1;
    var.dims [1].min = ny / 2;
    var.dims [1].max = ny / 2;
    var.dims [2].min = y_offset;
    var.dims [2].max = y_offset + my_y_dim - 1;

    debug_log << "read pattern 4 plane 2" << endl;
    for (int j=0; j<3; j++) {
        debug_log <<  "dims [" << to_string(j) << "] min is: " << to_string(var.dims [j].min) << endl;
        debug_log <<  "dims [" << to_string(j) << "] max is: " << to_string(var.dims [j].max) << endl;
    }  

    time_pts.push_back(chrono::high_resolution_clock::now());
    catg_of_time_pts.push_back(READ_PATTERN_4_PLANE_TO_FIND_INIT_DONE_START_READING_PLANE);

    for (int i=0; i < num_servers; i++) {
        time_pts.push_back(chrono::high_resolution_clock::now());
        catg_of_time_pts.push_back(READ_PATTERN_4_START_READING_FROM_NEW_SERVER);

        rc = metadata_get_chunk (servers.at(i), var, num_chunks, chunks);
        if (rc != RC_OK) {
            error_log << "Error retrieving chunk \n";
            time_pts.push_back(chrono::high_resolution_clock::now());
            catg_of_time_pts.push_back(ERR_GET_CHUNK);        }
        else{
            if(do_debug_testing) {
                debug_log << "chunk list for varid: " << var.var_id << " txn_id: " << var.txn_id << " name: " 
                      << var.name << " path: " << var.path << " version: " << var.version 
                      << " (count: " << num_chunks << ") \n";
                print_chunk_list (num_chunks, chunks);
            }        
        }

        if( is_type ) {
            time_pts.push_back(chrono::high_resolution_clock::now());
            catg_of_time_pts.push_back(READ_PATTERN_4_START_GET_ATTRS);

            for (int j=0; j < num_chunks; j++) {
                rc = metadata_get_attribute(servers.at(i), txn_id, chunks.at(j).chunk_id, num_attributes, attributes);
                if (rc != RC_OK) {
                    error_log << "Error retrieving attribute \n";
                  time_pts.push_back(chrono::high_resolution_clock::now());
                    catg_of_time_pts.push_back(ERR_GET_ATTR);              }
                else {
                    if(do_debug_testing) {
                        debug_log << "num attributes: " << num_attributes << endl;  
                        print_attribute_list(num_attributes, attributes); 
                    }        
                }
            }
            time_pts.push_back(chrono::high_resolution_clock::now());
            catg_of_time_pts.push_back(READ_PATTERN_4_DONE_GET_ATTRS);
        }

        time_pts.push_back(chrono::high_resolution_clock::now());
        catg_of_time_pts.push_back(READ_PATTERN_4_DONE_READING_FROM_SERVER);
    }
    time_pts.push_back(chrono::high_resolution_clock::now());
    catg_of_time_pts.push_back(READ_PATTERN_4_DONE_READING_PLANE);


//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    //! do a plane in dim(1)
    my_x_dim = nx / num_x_procs;
    my_y_dim = ny / num_y_procs;
    x_offset = (rank % num_x_procs) * my_x_dim;
    y_offset = (rank/num_x_procs) * my_y_dim;

    debug_log << "my_x_dim: " << to_string(my_x_dim) << endl;
    debug_log << "my_y_dim: " << to_string(my_y_dim) << endl;
    debug_log << "x_offset: " << to_string(x_offset) << endl;
    debug_log << "y_offset: " << to_string(y_offset) << endl;

    var.dims [0].min = x_offset;
    var.dims [0].max = x_offset + my_x_dim - 1;
    var.dims [1].min = y_offset;
    var.dims [1].max = y_offset + my_y_dim -1;
    var.dims [2].min = nz / 2;
    var.dims [2].max = nz / 2;

    debug_log << "read pattern 4 plane 3" << endl;
    for (int j=0; j<3; j++) {
        debug_log <<  "dims [" << to_string(j) << "] min is: " << to_string(var.dims [j].min) << endl;
        debug_log <<  "dims [" << to_string(j) << "] max is: " << to_string(var.dims [j].max) << endl;
    }  

    time_pts.push_back(chrono::high_resolution_clock::now());
    catg_of_time_pts.push_back(READ_PATTERN_4_PLANE_TO_FIND_INIT_DONE_START_READING_PLANE);

    for (int i=0; i < num_servers; i++) {
        time_pts.push_back(chrono::high_resolution_clock::now());
        catg_of_time_pts.push_back(READ_PATTERN_4_START_READING_FROM_NEW_SERVER);

        rc = metadata_get_chunk (servers.at(i), var, num_chunks, chunks);
        if (rc != RC_OK) {
            error_log << "Error retrieving chunk \n";
            time_pts.push_back(chrono::high_resolution_clock::now());
            catg_of_time_pts.push_back(ERR_GET_CHUNK);            
            goto cleanup;
        }
        else{
            if(do_debug_testing) {
                debug_log << "chunk list for varid: " << var.var_id << " txn_id: " << var.txn_id << " name: " 
                      << var.name << " path: " << var.path << " version: " << var.version 
                      << " (count: " << num_chunks << ") \n";
                print_chunk_list (num_chunks, chunks);  
            }     
        }

        if( is_type ) {
            time_pts.push_back(chrono::high_resolution_clock::now());
            catg_of_time_pts.push_back(READ_PATTERN_4_START_GET_ATTRS);
            for (int j=0; j < num_chunks; j++) {
                rc = metadata_get_attribute(servers.at(i), txn_id, chunks.at(j).chunk_id, num_attributes, attributes);
                if (rc != RC_OK) {
                    error_log << "Error retrieving attribute \n";
                  time_pts.push_back(chrono::high_resolution_clock::now());
                    catg_of_time_pts.push_back(ERR_GET_ATTR);              }
                else {
                    if(do_debug_testing) {
                        debug_log << "num attributes: " << num_attributes << endl;  
                        print_attribute_list(num_attributes, attributes);  
                    }       
                }
            }
            time_pts.push_back(chrono::high_resolution_clock::now());
            catg_of_time_pts.push_back(READ_PATTERN_4_DONE_GET_ATTRS); 
        }
        time_pts.push_back(chrono::high_resolution_clock::now());
        catg_of_time_pts.push_back(READ_PATTERN_4_DONE_READING_FROM_SERVER); 
    }
    time_pts.push_back(chrono::high_resolution_clock::now());
    catg_of_time_pts.push_back(READ_PATTERN_4_DONE_READING_PLANE); 

    time_pts.push_back(chrono::high_resolution_clock::now());
    catg_of_time_pts.push_back(READ_PATTERN_4_DONE_READING); 
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
cleanup:
    debug_log << "initiating cleanup" << endl;

    return rc;
}

// 5. an arbitrary rectangular subset representing a cubic area of interest
int read_pattern_5 (MPI_Comm comm, int rank, int num_servers, std::vector<metadata_server> servers, 
                    bool is_type, int num_x_procs, int num_y_procs, 
                    int num_z_procs, std::vector<md_catalog_var_entry> entries, int num_vars,
                    int nx, int ny, int nz, uint64_t txn_id, uint64_t var_id)
{
    time_pts.push_back(chrono::high_resolution_clock::now());
    catg_of_time_pts.push_back(READ_PATTERN_5_START);

    int rc;

    int index_of_var;
    md_catalog_var_entry var;

    uint32_t num_chunks;
    std::vector<md_chunk_entry> chunks;
    uint32_t num_attributes;
    std::vector<md_attribute_entry> attributes;

    int my_x_dim;
    int my_y_dim;
    int my_z_dim;

    int x_offset;
    int y_offset;
    int z_offset;


       if (rank == 0) {
        int i=0;
        while (i < num_vars && entries.at(i).var_id != var_id) {
            i++;
        }
        if (i == num_vars) {
            error_log << "Error pattern 5. The given var id " << var_id << " was not found in the catalog for txnid: " << txn_id << ". Exiting \n";
            goto cleanup;
        }
        index_of_var = i;
    }

    MPI_Bcast(&index_of_var, 1, MPI_INT, 0, comm);

    time_pts.push_back(chrono::high_resolution_clock::now());
    catg_of_time_pts.push_back(READ_PATTERN_5_BCAST_DONE);

    var = entries.at(index_of_var);

    extreme_debug_log << "num_x_procs: " << num_x_procs << endl;
    extreme_debug_log << "num_y_procs: " << num_y_procs << endl;
    extreme_debug_log << "num_z_procs: " << num_z_procs << endl;
    extreme_debug_log << "nx: " << nx << endl;
    extreme_debug_log << "ny: " << ny << endl;
    extreme_debug_log << "nz: " << nz << endl;

    my_x_dim = nx / num_x_procs / 2;
    my_y_dim = ny / num_y_procs / 2;
    my_z_dim = nz / num_z_procs / 2;
    x_offset = (rank % num_x_procs) * my_x_dim;
    y_offset = ((rank/num_x_procs) % num_y_procs) * my_y_dim;
    z_offset = (rank/(num_x_procs*num_y_procs) % num_z_procs) * my_z_dim;

    extreme_debug_log << "my_x_dim: " << my_x_dim << endl;
    extreme_debug_log << "my_y_dim: " << my_y_dim << endl;
    extreme_debug_log << "my_z_dim: " << my_z_dim << endl;
    extreme_debug_log << "x_offset: " << x_offset << endl;
    extreme_debug_log << "y_offset: " << y_offset << endl;
    extreme_debug_log << "z_offset: " << z_offset << endl;

    var.txn_id = txn_id;
    var.dims [0].min = nx / 4 + x_offset;
    var.dims [0].max = nx / 4 + x_offset + my_x_dim - 1;
    var.dims [1].min = ny / 4 + y_offset;
    var.dims [1].max = ny / 4 + y_offset + my_y_dim -1;
    var.dims [2].min = nz / 4 + z_offset;
    var.dims [2].max = nz / 4 + z_offset + my_z_dim -1;

    debug_log << "read pattern 5" << endl;
    for (int j=0; j<3; j++) {
        debug_log <<  "dims [" << to_string(j) << "] min is: " << to_string(var.dims [j].min) << endl;
        debug_log <<  "dims [" << to_string(j) << "] max is: " << to_string(var.dims [j].max) << endl;
    }  

    time_pts.push_back(chrono::high_resolution_clock::now());
    catg_of_time_pts.push_back(READ_PATTERN_5_VAR_INIT_DONE_START_READING);


    for (int i=0; i < num_servers; i++) {
        time_pts.push_back(chrono::high_resolution_clock::now());
        catg_of_time_pts.push_back(READ_PATTERN_5_START_READING_FROM_NEW_SERVER);

        rc = metadata_get_chunk (servers.at(i), var, num_chunks, chunks);
        if (rc != RC_OK) {
            error_log << "Error retrieving chunk \n";
            time_pts.push_back(chrono::high_resolution_clock::now());
            catg_of_time_pts.push_back(ERR_GET_CHUNK);        }
        else{
            if(do_debug_testing) {
                debug_log << "chunk list for varid: " << var.var_id << " txn_id: " << var.txn_id << " name: " 
                      << var.name << " path: " << var.path << " version: " << var.version 
                      << " (count: " << num_chunks << ") \n";
                print_chunk_list (num_chunks, chunks);  
            }      
        }

        if( is_type ) {
            time_pts.push_back(chrono::high_resolution_clock::now());
            catg_of_time_pts.push_back(READ_PATTERN_5_START_GET_ATTRS);

            for (int j=0; j < num_chunks; j++) {
                rc = metadata_get_attribute(servers.at(i), txn_id, chunks.at(j).chunk_id, num_attributes, attributes);
                if (rc != RC_OK) {
                    error_log << "Error retrieving attribute \n";
                  time_pts.push_back(chrono::high_resolution_clock::now());
                    catg_of_time_pts.push_back(ERR_GET_ATTR);              }
                else {
                    if(do_debug_testing) {
                        debug_log << "num attributes: " << num_attributes << endl;  
                        print_attribute_list(num_attributes, attributes);  
                    }       
                }
            }
            time_pts.push_back(chrono::high_resolution_clock::now());
            catg_of_time_pts.push_back(READ_PATTERN_5_DONE_GET_ATTRS);
        }
        
        time_pts.push_back(chrono::high_resolution_clock::now());
        catg_of_time_pts.push_back(READ_PATTERN_5_DONE_READING_FROM_SERVER);
    }

    time_pts.push_back(chrono::high_resolution_clock::now());
    catg_of_time_pts.push_back(READ_PATTERN_5_DONE_READING);

cleanup:
    debug_log << "initiating cleanup" << endl;

    return rc;
}

// 6. an arbitrary area on an orthogonal plane (decomposition dimensions) aka partial plane
int read_pattern_6 (MPI_Comm comm, int rank, int num_servers, std::vector<metadata_server> servers, 
                    bool is_type, int num_x_procs, int num_y_procs, 
                    std::vector<md_catalog_var_entry> entries, int num_vars,
                    int nx, int ny, int nz, uint64_t txn_id, uint64_t var_id)
{
    
    time_pts.push_back(chrono::high_resolution_clock::now());
    catg_of_time_pts.push_back(READ_PATTERN_6_START);

    int rc;

    int index_of_var;
    md_catalog_var_entry var;

    uint32_t num_chunks;
    std::vector<md_chunk_entry> chunks;
    uint32_t num_attributes;
    std::vector<md_attribute_entry> attributes;

    int my_x_dim;
    int my_y_dim;

    int x_offset;
    int y_offset;

      if (rank == 0) {
        int i=0;
        while (i < num_vars && entries.at(i).var_id != var_id) {
            i++;
        }
        if (i == num_vars) {
            error_log << "Error pattern 6. The given var id " << var_id << " was not found in the catalog for txnid: " << txn_id << ". Exiting \n";
            goto cleanup;
        }
        index_of_var = i;
    }

    MPI_Bcast(&index_of_var, 1, MPI_INT, 0, comm);

    time_pts.push_back(chrono::high_resolution_clock::now());
    catg_of_time_pts.push_back(READ_PATTERN_6_BCAST_DONE_START_READING);

    var = entries.at(index_of_var);


   //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // read in dim(1)
    my_x_dim = ny / num_x_procs / 2;
    my_y_dim = nx / num_y_procs / 2;
    x_offset = (rank % num_x_procs) * my_x_dim;
    y_offset = (rank/num_x_procs) * my_y_dim;

    var.txn_id = txn_id;
    var.dims [0].min = nx / 4 + y_offset;
    var.dims [0].max = nx / 4 + y_offset + my_y_dim - 1;
    var.dims [1].min = ny / 4 + x_offset;
    var.dims [1].max = ny / 4 + x_offset + my_x_dim -1;
    var.dims [2].min = nz / 4;
    var.dims [2].max = nz / 4;

    debug_log << "read pattern 6 plane 1" << endl;
    for (int j=0; j<3; j++) {
        debug_log <<  "dims [" << to_string(j) << "] min is: " << to_string(var.dims [j].min) << endl;
        debug_log <<  "dims [" << to_string(j) << "] max is: " << to_string(var.dims [j].max) << endl;
    }  

    time_pts.push_back(chrono::high_resolution_clock::now());
    catg_of_time_pts.push_back(READ_PATTERN_6_PLANE_TO_FIND_INIT_DONE_START_READING_PLANE);

    for (int i=0; i < num_servers; i++) {
        time_pts.push_back(chrono::high_resolution_clock::now());
        catg_of_time_pts.push_back(READ_PATTERN_6_START_READING_FROM_NEW_SERVER);

        rc = metadata_get_chunk (servers.at(i), var, num_chunks, chunks);
        if (rc != RC_OK) {
            error_log << "Error retrieving chunk \n";
            time_pts.push_back(chrono::high_resolution_clock::now());
            catg_of_time_pts.push_back(ERR_GET_CHUNK);        
        }
        else{
            if(do_debug_testing) {
                debug_log << "chunk list for varid: " << var.var_id << " txn_id: " << var.txn_id << " name: " 
                      << var.name << " path: " << var.path << " version: " << var.version 
                      << " (count: " << num_chunks << ") \n";
                print_chunk_list (num_chunks, chunks);   
            }     
        }
        if( is_type ) {
            time_pts.push_back(chrono::high_resolution_clock::now());
            catg_of_time_pts.push_back(READ_PATTERN_6_START_GET_ATTRS);

            for (int j=0; j < num_chunks; j++) {
                rc = metadata_get_attribute(servers.at(i), txn_id, chunks.at(j).chunk_id, num_attributes, attributes);
                if (rc != RC_OK) {
                    error_log << "Error retrieving attribute \n";
                    time_pts.push_back(chrono::high_resolution_clock::now());
                    catg_of_time_pts.push_back(ERR_GET_ATTR);              
                }
                else {
                    if(do_debug_testing) {
                        debug_log << "num attributes: " << num_attributes << endl;  
                        print_attribute_list(num_attributes, attributes); 
                    }        
                }
            }
            time_pts.push_back(chrono::high_resolution_clock::now());
            catg_of_time_pts.push_back(READ_PATTERN_6_DONE_GET_ATTRS);
        }
        time_pts.push_back(chrono::high_resolution_clock::now());
        catg_of_time_pts.push_back(READ_PATTERN_6_DONE_READING_FROM_SERVER);
    }
    time_pts.push_back(chrono::high_resolution_clock::now());
    catg_of_time_pts.push_back(READ_PATTERN_6_DONE_READING_PLANE);

//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // read in dim(2)
    my_x_dim = nz / num_x_procs / 2;
    my_y_dim = nx / num_y_procs / 2;
    x_offset = (rank % num_x_procs) * my_x_dim;
    y_offset = (rank/num_x_procs) * my_y_dim;

    var.dims [0].min = nx / 4 + y_offset;
    var.dims [0].max = nx / 4 + y_offset + my_y_dim - 1;
    var.dims [1].min = ny / 4;
    var.dims [1].max = ny / 4;
    var.dims [2].min = nz / 4 + x_offset;
    var.dims [2].max = nz / 4 + x_offset + my_x_dim -1;
    
    debug_log << "read pattern 6 plane 2" << endl;
    for (int j=0; j<3; j++) {
        debug_log <<  "dims [" << to_string(j) << "] min is: " << to_string(var.dims [j].min) << endl;
        debug_log <<  "dims [" << to_string(j) << "] max is: " << to_string(var.dims [j].max) << endl;
    }  

    time_pts.push_back(chrono::high_resolution_clock::now());
    catg_of_time_pts.push_back(READ_PATTERN_6_PLANE_TO_FIND_INIT_DONE_START_READING_PLANE);

    for (int i=0; i < num_servers; i++) {
        time_pts.push_back(chrono::high_resolution_clock::now());
        catg_of_time_pts.push_back(READ_PATTERN_6_START_READING_FROM_NEW_SERVER);

        rc = metadata_get_chunk (servers.at(i), var, num_chunks, chunks);
        if (rc != RC_OK) {
            error_log << "Error retrieving chunk \n";
            time_pts.push_back(chrono::high_resolution_clock::now());
            catg_of_time_pts.push_back(ERR_GET_CHUNK);        
        }
        else{
            if(do_debug_testing) {
                debug_log << "chunk list for varid: " << var.var_id << " txn_id: " << var.txn_id << " name: " 
                      << var.name << " path: " << var.path << " version: " << var.version 
                      << " (count: " << num_chunks << ") \n";
                print_chunk_list (num_chunks, chunks);  
            }      
        }

        if( is_type ) {
            time_pts.push_back(chrono::high_resolution_clock::now());
            catg_of_time_pts.push_back(READ_PATTERN_6_START_GET_ATTRS);
            for (int j=0; j < num_chunks; j++) {
                rc = metadata_get_attribute(servers.at(i), txn_id, chunks.at(j).chunk_id, num_attributes, attributes);
                if (rc != RC_OK) {
                    error_log << "Error retrieving attribute \n";
                    time_pts.push_back(chrono::high_resolution_clock::now());
                    catg_of_time_pts.push_back(ERR_GET_ATTR);              
                }
                else {
                    if(do_debug_testing) {
                        debug_log << "num attributes: " << num_attributes << endl;  
                        print_attribute_list(num_attributes, attributes);
                    }         
                }
            }
            time_pts.push_back(chrono::high_resolution_clock::now());
            catg_of_time_pts.push_back(READ_PATTERN_6_DONE_GET_ATTRS);
        }
        time_pts.push_back(chrono::high_resolution_clock::now());
        catg_of_time_pts.push_back(READ_PATTERN_6_DONE_READING_FROM_SERVER);
    }
    time_pts.push_back(chrono::high_resolution_clock::now());
    catg_of_time_pts.push_back(READ_PATTERN_6_DONE_READING_PLANE);

//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // read in dim(3)
    my_x_dim = nz / num_x_procs / 2;
    my_y_dim = ny / num_y_procs / 2;
    x_offset = (rank % num_x_procs) * my_x_dim;
    y_offset = (rank/num_x_procs) * my_y_dim;

    var.dims [0].min = nx / 4;
    var.dims [0].max = nx / 4;
    var.dims [1].min = ny / 4 + y_offset;
    var.dims [1].max = ny / 4 + y_offset + my_y_dim - 1;
    var.dims [2].min = nz / 4 + x_offset;
    var.dims [2].max = nz / 4 + x_offset + my_x_dim -1;

    debug_log << "read pattern 6 plane 3" << endl;
    for (int j=0; j<3; j++) {
        debug_log <<  "dims [" << to_string(j) << "] min is: " << to_string(var.dims [j].min) << endl;
        debug_log <<  "dims [" << to_string(j) << "] max is: " << to_string(var.dims [j].max) << endl;
    }  

    time_pts.push_back(chrono::high_resolution_clock::now());
    catg_of_time_pts.push_back(READ_PATTERN_6_PLANE_TO_FIND_INIT_DONE_START_READING_PLANE);

    for (int i=0; i < num_servers; i++) {
        time_pts.push_back(chrono::high_resolution_clock::now());
        catg_of_time_pts.push_back(READ_PATTERN_6_START_READING_FROM_NEW_SERVER);

        rc = metadata_get_chunk (servers.at(i), var, num_chunks, chunks);
        if (rc != RC_OK) {
            error_log << "Error retrieving chunk \n";
            time_pts.push_back(chrono::high_resolution_clock::now());
            catg_of_time_pts.push_back(ERR_GET_CHUNK);        
        }
        else{
            if(do_debug_testing) {
                debug_log << "chunk list for varid: " << var.var_id << " txn_id: " << var.txn_id << " name: " 
                      << var.name << " path: " << var.path << " version: " << var.version 
                      << " (count: " << num_chunks << ") \n";
                print_chunk_list (num_chunks, chunks);        
            }
        }

        if( is_type ) {
            time_pts.push_back(chrono::high_resolution_clock::now());
            catg_of_time_pts.push_back(READ_PATTERN_6_START_GET_ATTRS);

            for (int j=0; j < num_chunks; j++) {
                rc = metadata_get_attribute(servers.at(i), txn_id, chunks.at(j).chunk_id, num_attributes, attributes);
                if (rc != RC_OK) {
                    error_log << "Error retrieving attribute \n";
                    time_pts.push_back(chrono::high_resolution_clock::now());
                    catg_of_time_pts.push_back(ERR_GET_ATTR);              
                }
                else {
                    if(do_debug_testing) {
                        debug_log << "num attributes: " << num_attributes << endl;  
                        print_attribute_list(num_attributes, attributes);
                    }         
                }
            }
            time_pts.push_back(chrono::high_resolution_clock::now());
            catg_of_time_pts.push_back(READ_PATTERN_6_DONE_GET_ATTRS);
        }
        time_pts.push_back(chrono::high_resolution_clock::now());
        catg_of_time_pts.push_back(READ_PATTERN_6_DONE_READING_FROM_SERVER);
    }

    time_pts.push_back(chrono::high_resolution_clock::now());
    catg_of_time_pts.push_back(READ_PATTERN_6_DONE_READING_PLANE);

    time_pts.push_back(chrono::high_resolution_clock::now());
    catg_of_time_pts.push_back(READ_PATTERN_6_DONE_READING);
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

cleanup:
    debug_log << "initiating cleanup" << endl;
    return rc;
}


