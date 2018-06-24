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

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

#include <my_metadata_client.h>
#include <client_timing_constants.hh>
#include <OpActivateTypeMetaCommon.hh>
#include <OpActivateVarMetaCommon.hh>
#include <OpCatalogTypeMetaCommon.hh>                   
#include <OpDeleteTypeMetaCommon.hh>           
#include <OpDeleteVarMetaCommon.hh>   
#include <OpGetAttributeListMetaCommon.hh>
#include <OpGetChunkListMetaCommon.hh>
#include <OpProcessingTypeMetaCommon.hh>
#include <OpProcessingVarMetaCommon.hh>   

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

int extra_testing(MPI_Comm read_comm, int read_rank, std::vector<metadata_server> servers, uint32_t num_servers, metadata_server server, uint32_t num_vars, uint32_t num_types, uint64_t num_datasets) {
    int rc;
    std::vector<md_catalog_var_entry> var_entries;
    uint32_t num_var_entries;
    uint32_t num_chunks;
    std::vector<md_chunk_entry> chunks;
    md_catalog_var_entry var;

    md_catalog_type_entry type;
    std::vector<md_catalog_type_entry> type_entries;
    uint32_t num_type_entries;
    uint32_t num_attrs;
    std::vector<md_attribute_entry> attr_entries;

    if (read_rank < num_servers) {

        for(int txn_id=0; txn_id<num_datasets; txn_id++) {
            rc = metadata_activate_var(server, txn_id);
            if (rc != RC_OK) {
                error_log << "Error. Was unable to activate the var \n";
                time_pts.push_back(chrono::high_resolution_clock::now());
                catg_of_time_pts.push_back(ERR_ACTIVATE_VAR);        
            }
        }
        //since all datasets have been actiavted, all vars(across all datasets) are returned
        rc = metadata_catalog_var (server, 0, num_var_entries, var_entries);
        if (rc != RC_OK) {
            error_log << "Error cataloging the new set of var entries. Proceeding \n";
            time_pts.push_back(chrono::high_resolution_clock::now());
            catg_of_time_pts.push_back(ERR_CATALOG_VAR_EXTRA);    
        }
        else {
            if(do_debug_testing) {
                std::cout << "var catalog after activating: \n";
                print_var_catalog (num_var_entries, var_entries);
            }
        }  

        for(int txn_id=0; txn_id<num_datasets; txn_id++) {
            rc = metadata_processing_var(server, txn_id);
            if (rc != RC_OK) {
                error_log << "Error. Was unable to activate var \n";
                time_pts.push_back(chrono::high_resolution_clock::now());
                catg_of_time_pts.push_back(ERR_PROCESSING_VAR);    
            }
        }
    }
    MPI_Barrier(read_comm);


    if (read_rank < num_servers) {
        //since all datasets have been deactiavted, only the types for txnid = 0 are returned
        rc = metadata_catalog_var (server, 0, num_var_entries, var_entries);

        if (rc != RC_OK) {
             error_log << "Error cataloging the new set of var entries. Proceeding \n";
            time_pts.push_back(chrono::high_resolution_clock::now());
            catg_of_time_pts.push_back(ERR_CATALOG_VAR_EXTRA);
        }
        else {
            if(do_debug_testing) {
                debug_log << "var catalog after deactivating: \n";
                print_var_catalog (num_var_entries, var_entries);
            }
        } 

        var = var_entries.at(0);
        extreme_debug_log << "num servers: " << num_servers << endl;
        //get all of the chunks associated with the given variable and txn_id
        //each server has 1 num_procs_per_var / num_servers number of chunks
        rc = metadata_get_chunk_list (server, 0, var.name, var.path, var.version, 
                                        num_chunks, chunks);
        if (rc != RC_OK) {
            error_log << "Error getting the given chunk list. Proceeding \n";
            time_pts.push_back(chrono::high_resolution_clock::now());
            catg_of_time_pts.push_back(ERR_GET_CHUNK_LIST);
        }
        else {
            if(do_debug_testing) {
                debug_log << "chunk list for var named: " << var.name << " txn_id: " << 0 
                << " (count: " << num_chunks << ") \n";
                print_chunk_list (num_chunks, chunks);
            }
        }

        if (num_types > 0) {
            for(int txn_id=0; txn_id<num_datasets; txn_id++) {
                rc = metadata_activate_type(server, txn_id);
                if (rc != RC_OK) {
                    error_log << "Error. Was unable to activate type \n";
                    time_pts.push_back(chrono::high_resolution_clock::now());
                    catg_of_time_pts.push_back(ERR_ACTIVATE_TYPE);
                }
            }
            //since all datasets have been actiavted, all types (across all datasets) are returned
            rc = metadata_catalog_type (server, 1, num_type_entries, type_entries);

            if (rc != RC_OK) {
                error_log << "Error cataloging the new set of type entries. Proceeding \n";
                time_pts.push_back(chrono::high_resolution_clock::now());
                catg_of_time_pts.push_back(ERR_CATALOG_TYPE);
            }
            else {
                if(do_debug_testing) {
                    debug_log << "type catalog after activating: \n";
                    print_type_catalog (num_type_entries, type_entries);
                }
            } 

            for(int txn_id=0; txn_id<num_datasets; txn_id++) {
                rc = metadata_processing_type(server, txn_id);
                if (rc != RC_OK) {
                    error_log << "Error. Was unable to activate type \n";
                    time_pts.push_back(chrono::high_resolution_clock::now());
                    catg_of_time_pts.push_back(ERR_PROCESSING_TYPE);
                }
            }
            //since all datasets have been deactiavted, only the types for txnid = 1 are returned
            rc = metadata_catalog_type (server, 1, num_type_entries, type_entries);

            if (rc != RC_OK) {
                error_log << "Error cataloging the new set of type entries. Proceeding \n";
                time_pts.push_back(chrono::high_resolution_clock::now());
                catg_of_time_pts.push_back(ERR_CATALOG_TYPE);
            }
            else {
                if(do_debug_testing) {
                    debug_log << "type catalog after deactivating: \n";
                    print_type_catalog (num_type_entries, type_entries);
                }
            } 

            //can use this to get all chunks that have a certain type of metadata
            type = type_entries.at(0);

            for(int i=0; i<num_servers; i++)  {

                rc = metadata_get_attribute_list(servers.at(i), type.txn_id, type.name, type.version, num_attrs, attr_entries);

                if (rc != RC_OK) {
                    error_log << "Error getting the given attribute list. Proceeding \n";
                    time_pts.push_back(chrono::high_resolution_clock::now());
                    catg_of_time_pts.push_back(ERR_GET_ATTR_LIST);
                }
                else {
                    if(do_debug_testing) {
                        debug_log << "before deleting attribute list for txn_id: " << 1 << " name: " 
                              << type.name << " (count: " << num_attrs << ") \n";
                        print_attribute_list (num_attrs, attr_entries);
                    }
                } 
            }
        }

        if(do_debug_testing) {
            //try deleting all the vars for one dataset
            //should also delete all the chunks associated with this dataset
            //Delete the given var and all of its associated chunks
            
            for(int i=0; i<num_vars; i++) {
                md_catalog_var_entry var = var_entries.at(i);
                // debug_log << "var.id: " << var.var_id << " name: " << var.name << 
                // " path: " << var.path << " version: " << var.version << endl;
                rc = metadata_delete_var (server, var.var_id, var.name, var.path, var.version);
                if(rc != RC_OK) {
                    time_pts.push_back(chrono::high_resolution_clock::now());
                    catg_of_time_pts.push_back(ERR_DELETE_VAR);
                }
            }    

            //should be empty
            rc = metadata_catalog_var(server, 0, num_var_entries, var_entries);
            if (rc != RC_OK) {
                error_log << "Error cataloging the second set of entries. Proceeding \n";
                time_pts.push_back(chrono::high_resolution_clock::now());
                catg_of_time_pts.push_back(ERR_CATALOG_VAR_EXTRA);
            }

            debug_log << "catalog after deleting: \n";
            print_var_catalog (num_var_entries, var_entries);
        }

        if(num_types > 0) {
            if( num_chunks > 0) {
                 md_chunk_entry chunk = chunks.at(0);
                //can use this to get all chunks that have a certain type of metadata
                rc = metadata_get_attribute(server, 0, chunk.chunk_id, num_attrs, attr_entries);
                if (rc != RC_OK) {
                    error_log << "Error getting the given attribute list. Proceeding \n";
                    time_pts.push_back(chrono::high_resolution_clock::now());
                    catg_of_time_pts.push_back(ERR_GET_ATTR_EXTRA);
                }
                else {
                    if(do_debug_testing) {
                        debug_log << "attributes associated with txn_id: " << 0 << " and chunk_id: " << chunk.chunk_id <<
                        " (count: " << num_attrs << ") \n";                
                        print_attribute_list (num_attrs, attr_entries);
                    }
                }  
            }
            else {
                time_pts.push_back(chrono::high_resolution_clock::now());
                catg_of_time_pts.push_back(ERR_CHUNK_LIST_EMPTY);
            }
           
    
            if(do_debug_testing) {
                // delete all the types for dataset 1
                // should also delete all attrs associated with these types 
                for(int i=0; i<num_types; i++) {
                    md_catalog_type_entry temp_type = type_entries.at(i);
                    rc = metadata_delete_type (server, temp_type.type_id, temp_type.name, temp_type.version);
                    if(rc != RC_OK) {
                        time_pts.push_back(chrono::high_resolution_clock::now());
                        catg_of_time_pts.push_back(ERR_DELETE_TYPE);
                    }
                }
                
                rc = metadata_catalog_type (server, type.txn_id, num_type_entries, type_entries);
                if (rc != RC_OK) {
                    error_log << "Error cataloging the new set of type entries. Proceeding \n";
                    time_pts.push_back(chrono::high_resolution_clock::now());
                    catg_of_time_pts.push_back(ERR_CATALOG_TYPE); 
                }
                else {
                    debug_log << "catalog after deleting types: \n";
                    print_type_catalog (num_type_entries, type_entries);
                }

                 for(int i=0; i<num_servers; i++)  {

                    rc = metadata_get_attribute_list(server, type.txn_id, type.name, type.version, num_attrs, attr_entries);
                    if (rc != RC_OK) {
                        time_pts.push_back(chrono::high_resolution_clock::now());
                        catg_of_time_pts.push_back(ERR_GET_ATTR_LIST); 
                    }
                    else {
                         debug_log << "after deleting attribute list for txn_id: " << 1 << " name: " 
                          << type.name << " (count: " << num_attrs << ") \n";
                        print_attribute_list (num_attrs, attr_entries); 
                    }
                }
            } //end if(do_debug_testing)
        } //end if(num_types > 0)
    } //end if(read_rank < num_servers)
}

