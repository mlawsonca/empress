


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