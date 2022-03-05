



#ifndef TESTING_HARNESS_WRITE_LOCAL_HH
#define TESTING_HARNESS_WRITE_LOCAL_HH

#include <my_metadata_args.h>

int create_and_activate_run(md_catalog_run_entry &run, const std::string &rank_to_dims_funct_name,
    const std::string &rank_to_dims_funct_path, const std::string &objector_funct_name, const std::string &objector_funct_path);

int create_and_activate_types(uint64_t run_id, uint32_t num_types, 
    uint32_t num_run_timestep_types);

int create_vars(int rank, uint32_t num_client_procs,
    md_catalog_var_entry &temp_var, uint32_t var_indx,
    uint64_t total_x_length, uint64_t total_y_length, uint64_t total_z_length,
    std::vector<md_catalog_var_entry> &all_vars_to_insert
    );

int write_data(int rank, const std::vector<double> &data_vctr, const md_catalog_run_entry &run, const std::string &objector_funct_name, 
    uint32_t timestep_num, int64_t timestep_file_id,
    const md_catalog_var_entry &var, int var_indx, const std::vector<md_dim_bounds> &proc_dims_vctr,
    uint32_t num_run_timestep_types,
    double &timestep_temp_min, double &timestep_temp_max);

int create_var_attrs(int rank, const std::vector<md_dim_bounds> proc_dims_vctr,
    uint32_t timestep_num, uint32_t var_indx, uint32_t num_types, 
    std::vector<md_catalog_var_attribute_entry> &all_attrs_to_insert,
    double &timestep_temp_min, double &timestep_temp_max
    );

int insert_vars_and_attrs_batch(int rank, const std::vector<md_catalog_var_entry> &all_vars_to_insert,
    const std::vector<md_catalog_var_attribute_entry> &all_attrs_to_insert );

int create_timestep_attrs(int rank, double timestep_temp_min, double timestep_temp_max,
    uint32_t timestep_num, uint32_t num_types, uint32_t num_run_timestep_types, uint32_t num_client_procs,
    double *all_timestep_temp_mins_for_all_procs, double *all_timestep_temp_maxes_for_all_procs);

int activate_timestep_components(uint32_t timestep_num);

int create_run_attrs(int rank, double *all_timestep_temp_mins_for_all_procs, 
    double *all_timestep_temp_maxes_for_all_procs, uint64_t run_id, uint32_t num_timesteps, uint32_t num_types
    );

void output_obj_names_and_timing(int rank, uint32_t num_client_procs, const md_catalog_run_entry &temp_run,
    const md_catalog_var_entry &temp_var, uint32_t num_timesteps, uint32_t num_vars
    );


//note - just for debugging
int debug_testing(int rank, std::vector<md_dim_bounds> dims);
void gather_and_print_output_params(const std::vector<objector_params> &object_names, int rank, uint32_t num_client_procs);
objector_params get_objector_params ( const md_catalog_run_entry &run, const md_catalog_var_entry &var, 
                            const std::vector<md_dim_bounds> &bounding_box);

void gather_and_print_object_names(const std::vector<std::vector<std::string>> &all_object_names, int rank, uint32_t num_client_procs, 
                            uint32_t num_timesteps, uint32_t num_vars);
void gatherv_int(std::vector<int> values, uint32_t num_client_procs, int rank, int *each_proc_num_values,
            int *displacement_for_each_proc, int **all_values);

static void get_obj_lengths(const md_catalog_var_entry &var, 
                    uint64_t &x_width, uint64_t &last_x_width, uint64_t ndx, uint64_t chunk_size) ;

void output_db_statistics(int rank, uint32_t num_procs);

#endif //TESTING_HARNESS_WRITE_LOCAL_HH