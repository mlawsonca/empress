




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