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


#ifndef MY_METADATA_CLIENT_LUA_FUNCTS_H
#define MY_METADATA_CLIENT_LUA_FUNCTS_H

#include <my_metadata_args.h>

extern "C" {
  #include <lua.h>
  #include <lauxlib.h>
  #include <lualib.h>
}

int stringify_function(const std::string &path_to_function, const std::string &funct_name, std::string &code_string);

// void rankToBoundingBox(lua_State *L, uint32_t npx, uint32_t npy, uint32_t npz, md_dim_bounds var_dims, md_dim_bounds attr_dims, 
//                       int rank, md_dim_bounds &bounding_box);

int rankToBoundingBox(const md_catalog_run_entry &run, const md_catalog_var_entry &var, int rank, std::vector<md_dim_bounds> &bounding_box);

// int boundingBoxToObjNames(const md_catalog_run_entry &run, const std::string &objector_funct_name, const md_catalog_var_entry &var, const std::vector<md_dim_bounds> &bounding_box,
//                          std::vector<std::string> &obj_names);
int boundingBoxToObjNames(const md_catalog_run_entry &run, const md_catalog_var_entry &var, const std::vector<md_dim_bounds> &bounding_box,
                         std::vector<std::string> &obj_names);

int boundingBoxToObjNamesAndCounts(const md_catalog_run_entry &run, const md_catalog_var_entry &var, const std::vector<md_dim_bounds> &bounding_box,
                         std::vector<std::string> &obj_names, std::vector<uint64_t> &offsets_and_counts);

int register_objector_funct_write (const std::string &funct_name, const std::string &path_to_function, uint64_t job_id);


//note - just needed for dumping md to file for object name generation elsewhere
// function boundingBoxToObjectNamesAndCounts(get_counts, job_id, sim_name, timestep, ndx, ndy, ndz, ceph_obj_size, var_name, var_version, data_size, x1, y1, z1, x2, y2, z2, var_x1, var_x2, var_y1, var_y2, var_z1, var_z2)

// struct objector_params
// {
//     bool get_counts;
//     uint64_t job_id;
//     string sim_name;
//     uint64_t timestep_id;
//     uint64_t ndx;
//     uint64_t ndy;
//     uint64_t ndz;
//     uint64_t ceph_obj_size = 8000000;
//     string var_name;
//     uint32_t var_version;


//     template <typename Archive>
//     void serialize(Archive& ar, const unsigned int ver)
//     {
//     ar & min;
//     ar & max;
//     }
// };



#endif //MY_METADATA_CLIENT_LUA_FUNCTS_H