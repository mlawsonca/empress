#ifndef TESTINGHARNESSWRITECLASSPROJHDF5_HH
#define TESTINGHARNESSWRITECLASSPROJHDF5_HH

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <sstream>

int do_reads(int rank, std::string run_name, std::string job_id,
		uint32_t num_client_procs,
		uint32_t num_timesteps,
		MPI_Comm read_comm,
		// const md_dim_bounds &proc_dims,
		const std::vector<std::string> &all_var_names,
		const std::vector<std::string> &type_names,
		const std::vector<std::string> &run_timestep_type_names,
		const std::vector<std::string> &file_names_to_fetch,
		const std::vector<std::string> &var_names_to_fetch,	
		hsize_t *var_dims
	);

void read_init(
		const std::string &run_name,
		const std::string &job_id,
		const std::vector<uint64_t> &timestep_ids_to_fetch,
		const std::vector<std::string> &var_names,
		std::vector<std::string> &all_var_names,		
		std::vector<std::string> &file_names_to_fetch,
		std::vector<std::string> &var_names_to_fetch
		);


void create_var_attrs(int rank, md_dim_bounds proc_dims, int timestep_num, int var_indx, std::string VARNAME, uint32_t num_types, 
		std::vector<var_attribute_str> &attrs,
		double &timestep_temp_max, double &timestep_temp_min);

void create_timestep_attrs(hid_t timestep_file_id, int rank, int timestep_num, uint32_t num_client_procs, 
	double timestep_temp_max, double timestep_temp_min, char *max_type_name, char *min_type_name,
	double *all_timestep_temp_maxes_for_all_procs, double *all_timestep_temp_mins_for_all_procs);

void create_run_attrs(std::string run_name, std::string job_id, uint32_t num_timesteps, double *all_timestep_temp_maxes_for_all_procs, 
	double *all_timestep_temp_mins_for_all_procs, char *max_type_name, char *min_type_name);

void do_cleanup(int rank, uint32_t num_client_procs);

int debug_testing(int rank, hsize_t *chunk_dims, int num_client_procs, std::string run_name, int num_timesteps, int num_vars,
				int num_dims, hsize_t *var_dims,
				std::string job_id, std::vector<std::string> var_names, uint32_t version1, uint32_t version2, bool write_data
				);


#endif //TESTINGHARNESSWRITECLASSPROJHDF5_HH