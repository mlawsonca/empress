


#ifndef TESTING_HARNESS_DEBUG_HELPER_FUNCTIONS_HH
#define TESTING_HARNESS_DEBUG_HELPER_FUNCTIONS_HH

#include <my_metadata_args.h>
#include <map>
#include <vector>

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