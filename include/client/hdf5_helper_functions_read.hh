


#ifndef HDF5_HELPER_FUNCTIONS_HH
#define HDF5_HELPER_FUNCTIONS_HH


 
void hdf5_close_timestep_file(int64_t timestep_file_id);

void hdf5_open_var(int64_t step_file_id, std::string var_name, uint32_t var_version, int64_t &var_id);

void hdf5_close_var(int64_t var_id);

void hdf5_open_timestep_file_collectively_for_read(const std::string &run_name, uint64_t job_id, uint64_t timestep_id, int64_t &file_id);

int64_t hdf5_get_dataspace(int64_t var_id);

void hdf5_close_dataspace(int64_t var_data_space);

void hdf5_read_hyperslab(int64_t file_id, const md_catalog_var_entry &var, const std::vector<md_dim_bounds> &chunk_dims );

void hdf5_read_hyperslab(const md_catalog_var_entry &var,
		int64_t var_id, int64_t var_data_space, const std::vector<md_dim_bounds> &chunk_dims);

void find_data_range(int64_t var_data_space, int num_dims, const std::vector<double> &data_vect);

void convert_dim_bounds_to_counts(const std::vector<md_dim_bounds> &dims, unsigned long long *counts);

void read_data_for_attrs(const md_catalog_run_entry &run, const md_catalog_var_entry &var, 
				const std::vector<md_catalog_var_attribute_entry> &attrs, const std::vector<md_dim_bounds> &proc_dims);

void read_data_for_attrs(const md_catalog_run_entry &run, const std::vector<md_catalog_var_entry> &vars, 
				const std::vector<md_catalog_var_attribute_entry> &attrs, const std::vector<md_dim_bounds> &proc_dims);

#endif HDF5_HELPER_FUNCTIONS_HH
