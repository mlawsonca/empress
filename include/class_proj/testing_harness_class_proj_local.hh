#ifndef TESTINGHARNESSCLASSPROJLOCAL_HH
#define TESTINGHARNESSCLASSPROJLOCAL_HH

#include <my_metadata_args.h>

int create_and_activate_run(md_catalog_run_entry &run, const std::string &rank_to_dims_funct_name,
    const std::string &rank_to_dims_funct_path, const std::string &objector_funct_name, const std::string &objector_funct_path);

int create_and_activate_types(uint64_t run_id, uint32_t num_types, 
    uint32_t num_run_timestep_types);

int create_vars(int rank, uint32_t num_client_procs,
    uint64_t total_x_length, uint64_t total_y_length, uint64_t total_z_length, uint32_t num_vars,
    uint32_t timestep_num
    );


int create_var_attrs(int rank, const std::vector<md_dim_bounds> &proc_dims_vctr,
    uint32_t timestep_num, uint32_t num_vars, uint32_t num_types,
    double &timestep_temp_min, double &timestep_temp_max
    );

// int create_var_attrs(int rank, const std::vector<md_dim_bounds> &proc_dims_vctr,
//     uint32_t timestep_num, uint32_t num_vars, uint32_t num_types, md_write_type write_type, 
//     double &timestep_temp_min, double &timestep_temp_max
//     );

int insert_vars_and_attrs_batch(int rank, const std::vector<md_catalog_var_entry> &all_vars_to_insert,
    const std::vector<md_catalog_var_attribute_entry> &all_attrs_to_insert );

int create_timestep_attrs(int rank, double timestep_temp_min, double timestep_temp_max,
    uint32_t timestep_num, uint32_t num_types, uint32_t num_run_timestep_types, uint32_t num_client_procs,
    double *all_timestep_temp_mins_for_all_procs, double *all_timestep_temp_maxes_for_all_procs);

int activate_timestep_components(uint32_t timestep_num);

int create_run_attrs(int rank, double *all_timestep_temp_mins_for_all_procs, 
    double *all_timestep_temp_maxes_for_all_procs, uint64_t run_id, uint32_t num_timesteps, uint32_t num_types
    );

void output_timing_stats(int rank, uint32_t num_client_procs );

template <class T>
static void make_single_val_data (T val, std::string &serial_str);
template <class T1, class T2>
static void make_double_val_data (T1 val_0, T2 val_1, std::string &serial_str);

void gatherv_int(std::vector<int> values, uint32_t num_client_procs, int rank, int *each_proc_num_values,
            int *displacement_for_each_proc, int **all_values);


int do_reads(int rank, uint32_t num_x_procs, uint32_t num_y_procs, uint32_t num_z_procs,
		uint32_t num_timesteps, uint32_t num_client_procs, uint64_t txn_id,
		MPI_Comm read_comm, 
		const md_catalog_run_entry &run, const std::vector<md_dim_bounds> &proc_dims,
		uint32_t plane_x_procs, uint32_t plane_y_procs, const std::vector<std::vector<md_catalog_var_entry>> &all_var_entries,
		const std::vector<md_catalog_var_entry> &vars_to_fetch_pattern2, 
		const std::vector<md_catalog_var_entry> &vars_to_fetch_pattern3,
		const std::vector<md_catalog_var_entry> &vars_to_fetch_pattern4, 
		const std::vector<md_catalog_var_entry> &vars_to_fetch_pattern5,
		const std::vector<md_catalog_var_entry> &vars_to_fetch_pattern6
	);

int read_init(int rank, uint32_t num_client_procs,
		uint32_t num_x_procs, uint32_t num_y_procs, uint32_t num_z_procs,
		uint32_t num_timesteps_to_fetch, uint64_t txn_id, uint64_t *timestep_ids,
		MPI_Comm read_comm,  md_catalog_run_entry &run, 
		std::vector<md_dim_bounds> &proc_dims, int &plane_x_procs, int &plane_y_procs,
    	std::vector<std::vector<md_catalog_var_entry>> &all_var_entries,
		std::vector<md_catalog_var_entry> &vars_to_fetch_pattern2,
		std::vector<md_catalog_var_entry> &vars_to_fetch_pattern3, 
		std::vector<md_catalog_var_entry> &vars_to_fetch_pattern4,
		std::vector<md_catalog_var_entry> &vars_to_fetch_pattern5,
		std::vector<md_catalog_var_entry> &vars_to_fetch_pattern6
		);
#endif //TESTINGHARNESSCLASSPROJLOCAL_HH
