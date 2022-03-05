


#ifndef HDF5_HELPER_FUNCTIONS_HH
#define HDF5_HELPER_FUNCTIONS_HH

// static void write_chunk_data(int64_t var_id, uint32_t num_dims, hsize_t *offset, 
// 						hsize_t *chunk_dims, uint32_t total_y_length, uint32_t total_z_length, 
// 						int rank, vector<double> &data_vct
// 						);

// void hdf5_write_chunk_data(int64_t timestep_file_id, const md_catalog_var_entry &var, const std::vector<md_dim_bounds> &chunk_dims,
// 						int rank, std::vector<double> &data_vct
// 						);
void hdf5_write_chunk_data(int64_t timestep_file_id, const md_catalog_var_entry &var, const std::vector<md_dim_bounds> &chunk_dims,
						int rank, const std::vector<double> &data_vct
						);
 
void hdf5_create_timestep_and_vars(std::string run_name, uint64_t job_id, uint64_t timestep_id, 
					uint32_t num_vars, unsigned long long nx, unsigned long long ny, unsigned long long nz,
					const std::vector<md_dim_bounds> &chunk_dims);

void hdf5_open_timestep_file_collectively_for_write(const std::string &run_name, uint64_t job_id, 
	uint64_t timestep_id, int64_t &file_id);

void hdf5_close_timestep_file(int64_t timestep_file_id);


void hdf5_open_var(int64_t step_file_id, std::string var_name, uint32_t var_version, int64_t &var_id);

void convert_dim_bounds_to_counts(const std::vector<md_dim_bounds> &dims, unsigned long long *counts);

#endif HDF5_HELPER_FUNCTIONS_HH
