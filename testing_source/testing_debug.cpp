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
#include <opbox/services/dirman/DirectoryManager.hh>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <sstream>

using namespace std;

// bool debug = false;
static bool debug_logging = true;
static bool error_logging = true;

static void debug_log(const std::string &s) {
  if(debug_logging) std::cout << s<< std::endl;
}

static void error_log(const std::string &s) {
  if(error_logging) std::cout << s<< std::endl;
}

void write_testing(int rank, DirectoryInfo dir, metadata_server server, uint64_t num_datasets, uint32_t chunk_id) {
// //TESTING
    int rc;
    uint32_t items;
    std::vector<md_chunk_entry> chunks;
    uint32_t count2;
    uint32_t count;

    std::vector<md_catalog_type_entry> type_entries;
    std::vector<md_attribute_entry> attr_entries;

    MPI_Barrier(MPI_COMM_WORLD);

    //returns information about the variables associated with the given txn_id 
    if (rank < dir.children.size()) {
        vector<md_catalog_var_entry> entries;
        for(uint64_t txn_id=0; txn_id<num_datasets; txn_id++) {
            rc = metadata_catalog_var(server, txn_id, items, entries);
            if (rc != RC_OK) {
                error_log("Error cataloging the first set of entries. Proceeding");
            }       
            debug_log("num items is  " + to_string(items) + " num entries is " + to_string(entries.size()));

            std::cout << "catalog: \n";
            print_var_catalog (items, entries);

            for (int i=0; i<items; i++) {
                debug_log("i: " + i);
                md_catalog_var_entry var = entries.at(i);
                rc = metadata_get_chunk_list (server, txn_id, var.name, var.path, var.version, 
                                          count2, chunks);
                if (rc == RC_OK) {
                    std::cout << "chunk list for varid: " << var.var_id << " txn_id: " << txn_id << " name: " 
                      << var.name << " path: " << var.path << " version: " << var.version 
                      << " (count: " << count2 << ") \n";

                    print_chunk_list (count2, chunks);
                }
                else {
                    error_log("Error getting the " + var.name + " var chunk list. Proceeding");
                }
            }

            //returns information about the types associated with the given txn_id 
            rc = metadata_catalog_type (server, txn_id, count, type_entries);



            if (rc == RC_OK) {
                std::cout << "type catalog: \n";
                print_type_catalog (count, type_entries);
            }
            else {
                error_log("Error cataloging the " + to_string(rank) + "th set of type entries. Proceeding");
            }

            for (int j=0; j < count; j++) {
                md_catalog_type_entry type = type_entries.at(j);
                //note: can use this to get all chunks that have a certain type of metadata
                rc = metadata_get_attribute_list(server, txn_id, type.name, type.version, count2, attr_entries);

                if (rc == RC_OK) {
                    std::cout << "attribute list for type_id: " << type.type_id << " txn_id: " << txn_id << " name: " 
                          << type.name << " (count: " << count2 << ") \n";
                    print_attribute_list (count2, attr_entries);
                }
                else {
                    std::cout << "Error getting the given attribute list. Proceeding \n";
                } 
            }
           


            rc = metadata_get_attribute(server, txn_id, chunk_id, count, attr_entries);

            if (rc == RC_OK) {
                cout << "attr_entries.size: " << attr_entries.size() << endl;
                std::cout << "attributes associated with txn_id: " << txn_id << " and chunk_id: " << chunk_id <<
                " (count: " << count << ") \n";
                print_attribute_list (count, attr_entries);
                
                for (int k=0; k < count; k++) {
                    stringstream ss;
                    try {
                        ss << attr_entries.at(k).data;
                        boost::archive::text_iarchive ia(ss);
                
                        uint64_t type_id = attr_entries.at(k).type_id;

                    if(1 <= type_id && type_id <= 3) { //okay with this?
                        bool flag;
                        ia >> flag;
                        debug_log("The " + to_string(k) + "th attr is a flag with value " + to_string(flag));
                    }
                
                    if(4 <= type_id && type_id <= 5) {
                        double val;
                        ia >> val;
                        std::string s = (attr_entries.at(k).type_id == 4) ? "max" : "min";
                        debug_log("The " + to_string(k) + "th attr is a " + s + "flag with value " + to_string(val));
                    }
                    if(6<= type_id && type_id <= 8) {
                        std::vector<point> pts;
                        ia >> pts;
                        std::cout << "The " << k << "th attribute is a bounding box with pts: ";
                        debug_log("The " + to_string(k) + "th attr is a bounding box with pts: ");
                        for (int l=0; l<pts.size(); l++) {
                            point p = pts.at(l);
                             debug_log(" (x: " + to_string(p.x) + " y: " + to_string(p.y) + " z: " + to_string(p.z) + ") ");
                        }
                        std::cout << std::endl;
                    }
                    if(8 <= type_id && type_id <= 9) {
                        double vals[2];
                        ia >> vals;
                        debug_log("The " + to_string(k) + "th attr is a range of [" + to_string(vals[0]) + "," + to_string(vals[1]) + "]");
                   }

                    } catch (...) {
                        error_log("there was an exception. ss: " + ss.str());
                    }

                }
            }
            else {
                error_log("Error getting the matching attribute list. Proceeding");
            }
        }
    }
}
