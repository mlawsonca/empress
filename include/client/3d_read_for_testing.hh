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

#ifndef THREEDREADFORTESTING_HH
#define THREEDREADFORTESTING_HH

#include <my_metadata_client.h>          


// 1. all vars (use restart)
int read_pattern_1 (int rank, int num_servers, std::vector<metadata_server> servers, 
                    bool is_type, int num_x_procs, int num_y_procs, 
                    int num_z_procs, std::vector<md_catalog_var_entry> entries, int num_vars,
                    int total_x_length, int total_y_length, int total_z_length, uint64_t txn_id);

int read_pattern_2 (MPI_Comm comm, int rank, int num_servers, std::vector<metadata_server> servers, 
                    bool is_type, int num_x_procs, int num_y_procs, 
                    int num_z_procs, std::vector<md_catalog_var_entry> entries, int num_vars,
                    int nx, int ny, int nz, uint64_t txn_id, uint64_t var_id);
    
    
  

int read_pattern_3 (MPI_Comm comm, int rank, int num_servers, std::vector<metadata_server> servers, 
                    bool is_type, int num_x_procs, int num_y_procs, int num_z_procs, 
                    std::vector<md_catalog_var_entry> entries, int num_vars, int total_x_length, 
                    int total_y_length, int total_z_length, uint64_t txn_id, 
                    std::vector<uint64_t> var_ids, int num_vars_to_fetch);

// 4. 1 plane in each dimension for 1 variable

int read_pattern_4 (MPI_Comm comm, int rank, int num_servers, std::vector<metadata_server> servers, 
                    bool is_type, int num_x_procs, int num_y_procs, 
                    std::vector<md_catalog_var_entry> entries, int num_vars,
                    int nx, int ny, int nz, uint64_t txn_id, uint64_t var_id);


int read_pattern_5 (MPI_Comm comm, int rank, int num_servers, std::vector<metadata_server> servers, 
                    bool is_type, int num_x_procs, int num_y_procs, 
                    int num_z_procs, std::vector<md_catalog_var_entry> entries, int num_vars,
                    int nx, int ny, int nz, uint64_t txn_id, uint64_t var_id);

int read_pattern_6 (MPI_Comm comm, int rank, int num_servers, std::vector<metadata_server> servers, 
                    bool is_type, int num_x_procs, int num_y_procs, 
                    std::vector<md_catalog_var_entry> entries, int num_vars,
                    int nx, int ny, int nz, uint64_t txn_id, uint64_t var_id);

#endif //THREEDREADFORTESTING_HH
