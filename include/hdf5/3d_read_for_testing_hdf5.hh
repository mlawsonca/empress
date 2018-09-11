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




#ifndef THREEDREADFORTESTINGHDF5_HH
#define THREEDREADFORTESTINGHDF5_HH
#include <string>

void read_pattern_1 (hsize_t *offset, hsize_t *stride, hsize_t *count, hsize_t *block, 
					std::string timestep_file_name, std::vector<std::string> var_names, uint64_t chunk_vol, uint32_t num_dims );

void read_pattern_2 (hsize_t *offset, hsize_t *stride, hsize_t *count, hsize_t *block, 
					std::string timestep_file_name, std::string var_name, uint64_t chunk_vol, uint32_t num_dims );

void read_pattern_2_type (int rank, std::string timestep_file_name, const std::vector<std::string> &type_names,
					std::string var_name, md_dim_bounds proc_dims, uint32_t num_client_procs  );

void read_pattern_3 (hsize_t *offset, hsize_t *stride, hsize_t *count, hsize_t *block, 
					std::string timestep_file_name, std::vector<std::string> var_names, uint64_t chunk_vol, 
					uint32_t num_dims );

void read_pattern_3_type (int rank, std::string timestep_file_name, const std::vector<std::string> &type_names,
				const std::vector<std::string> &var_names, md_dim_bounds proc_dims, uint32_t num_client_procs );

void read_pattern_4 (int rank, uint32_t num_x_procs, uint32_t num_y_procs, hsize_t *stride, hsize_t *block,
					std::string timestep_file_name, std::string var_name, uint32_t num_dims );

void read_pattern_5 ( int rank, uint32_t num_x_procs, uint32_t num_y_procs, uint32_t num_z_procs, hsize_t *stride, hsize_t *block,
				std::string timestep_file_name, std::string var_name, uint32_t num_dims );

void read_pattern_6 (int rank, uint32_t num_x_procs, uint32_t num_y_procs, hsize_t *stride, hsize_t *block,
					std::string timestep_file_name, std::string var_name, uint32_t num_dims );

#endif //THREEDREADFORTESTINGHDF5_HH