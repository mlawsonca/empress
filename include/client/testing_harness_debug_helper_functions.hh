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


#ifndef TESTING_HARNESS_DEBUG_HELPER_FUNCTIONS_HH
#define TESTING_HARNESS_DEBUG_HELPER_FUNCTIONS_HH

#include <my_metadata_args.h>
#include <map>

// int write_output(std::map <std::string, std::vector<double>> &data_outputs, int rank, const md_catalog_run_entry &run, const std::string &objector_funct_name,
//                 const md_catalog_var_entry &var, const std::vector<md_dim_bounds> &bounding_box);

int write_output(std::map <std::string, std::vector<double>> &data_outputs, int rank, const md_catalog_run_entry &run, const std::string &objector_funct_name,
                const md_catalog_var_entry &var, const std::vector<md_dim_bounds> &bounding_box, std::vector<double> &data_vctr);

int write_output(std::map <std::string, std::vector<double>> &data_outputs, int rank, const md_catalog_run_entry &run, const std::string &objector_funct_name,
                const md_catalog_var_entry &var, const std::vector<md_dim_bounds> &bounding_box, const std::vector<double> &data_vctr);

void make_real_attr_data (double test_real, std::string &serial_str);

void make_int_attr_data (int test_int, std::string &serial_str);

void make_string_attr_data (const std::string &test_str, std::string &serial_str);

void make_blob_attr_data (const std::string &test_string, int test_int, std::string &serial_str);

void make_attr_data (const std::string &test_string, int test_int, std::string &serial_str);

int retrieve_obj_names_and_data(std::map <std::string, std::vector<double>> data_outputs, const md_catalog_run_entry &run, 
                      const md_catalog_var_entry &var, const std::vector<md_dim_bounds> &bounding_box  
					  );

int retrieveObjNamesAndDataForAttrCatalog(const std::map <std::string, std::vector<double>> &data_outputs,
                                         const md_catalog_run_entry &run, uint64_t run_id, uint64_t timestep_id,
                                         uint64_t txn_id,
                                         const std::vector<md_catalog_var_attribute_entry> &attr_entries );

void print_var_attr_data(uint32_t count, const std::vector<md_catalog_var_attribute_entry> &attr_entries);

void print_timestep_attr_data(uint32_t count, const std::vector<md_catalog_timestep_attribute_entry> &attr_entries);

void print_run_attr_data(uint32_t count, const std::vector<md_catalog_run_attribute_entry> &attr_entries);

void generate_data_for_proc(const md_catalog_var_entry &var, 
                        int rank, const std::vector<md_dim_bounds> &bounding_box, std::vector<double> &data_vct, 
                        uint64_t ndx, uint64_t ndy, uint64_t ndz, uint64_t chunk_vol);

void generate_data_for_proc(uint64_t ny, uint64_t nz,
                        int rank, const std::vector<md_dim_bounds> &bounding_box, std::vector<double> &data_vct, 
                        uint64_t ndx, uint64_t ndy, uint64_t ndz, uint64_t chunk_vol);

void generate_data_for_proc2D(const md_catalog_var_entry &var, 
                        int rank, const std::vector<md_dim_bounds> &bounding_box, std::vector<double> &data_vct, 
                        uint64_t ndx, uint64_t ndy, uint64_t chunk_vol);

#endif //TESTING_HARNESS_DEBUG_HELPER_FUNCTIONS_HH