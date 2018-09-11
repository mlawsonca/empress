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



#ifndef HELPERFUNCTIONSTESTCLIENTHDF5_HH
#define HELPERFUNCTIONSTESTCLIENTHDF5_HH

#include <my_metadata_client_hdf5.hh>
#include <hdf5.h>
#include <hdf5_hl.h>
#include <vector>


void create_timestep(std::string run_name, std::string job_id, uint64_t timestep_id, hid_t run_file_id,
					std::string TYPENAME_0, std::string TYPENAME_1,
					std::string VARNAME_0, std::string VARNAME_1);

void catalog_all_var_attributes ( hid_t var_attr_table_id, uint64_t timestep_id, md_dim_bounds query_dims );

void catalog_all_var_attributes_with_type ( hid_t var_attr_table_id, uint64_t timestep_id, std::string type_name, md_dim_bounds query_dims );

void catalog_all_var_attributes_with_var ( hid_t var_attr_table_id, uint64_t timestep_id, std::string var_name, md_dim_bounds query_dims );

void catalog_all_var_attributes_with_type_var ( hid_t var_attr_table_id, uint64_t timestep_id, std::string type_name, std::string var_name,
		 								 md_dim_bounds query_dims );

void catalog_all_var_attributes_with_var_substr ( hid_t file_id, hid_t var_attr_table_id, uint64_t timestep_id, 
					md_dim_bounds query_dims );

void catalog_all_var_attributes_with_type_var_substr ( hid_t var_attr_table_id, uint64_t timestep_id, 
					std::string type_name, md_dim_bounds query_dims );

void catalog_all_types_substr ( hid_t var_attr_table_id, uint64_t timestep_id, md_dim_bounds query_dims );

// int delete_var_substr (hid_t file_id, hid_t var_attr_table_id, uint64_t timestep_id);

void create_timestep_attrs(hid_t timestep_attr_table0, hid_t timestep_attr_table1,
                    std::string type0_name, std::string type1_name );

void catalog_all_timestep_attributes ( hid_t timestep_attr_table_id, uint64_t timestep_id, std::string type_name);

void catalog_all_timesteps_substr ( hid_t run_file_id, std::string type_name, md_dim_bounds query_dims );

void create_new_timesteps(std::string run_name, std::string job_id, uint64_t timestep_id, hid_t run_file_id, std::string type0_name, 
					std::string type1_name, std::string var0_name, std::string var1_name, hid_t timestep20_var_attr_table_id);

void catalog_all_timesteps ( hid_t run_file_id, std::string type_name, std::string var_name, md_dim_bounds query_dims );


void create_run_attrs(hid_t run_attr_table_id, std::string type0_name, std::string type1_name );

void catalog_all_run_attributes ( hid_t run_attr_table_id, std::string type_name );

void catalog_all_types ( hid_t var_attr_table_id, uint64_t timestep_id, std::string var_name, md_dim_bounds query_dims );

void create_new_types(md_timestep_entry timestep0, md_timestep_entry timestep1, std::string var0_name, std::string var1_name);

#endif //HELPERFUNCTIONSTESTCLIENTHDF5_HH