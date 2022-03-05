#ifndef TESTINGHARNESSCLASSPROJ_HH
#define TESTINGHARNESSCLASSPROJ_HH

#include <my_metadata_args.h>

static int setup_dirman(const std::string &dirman_hostname, const std::string &dir_path, md_server &dirman, std::vector<faodel::NameAndNode> &server_procs, int rank, uint32_t num_servers, uint32_t num_clients);
// static int setup_dirman(const std::string &dirman_hostname, const std::string &dir_path, md_server &dirman, std::vector<faodel::NameAndNode> &server_procs, int rank, uint32_t num_servers, uint32_t num_clients, int dirman_port=1990);
// static void setup_server(md_server &server, int &server_indx, int rank, uint32_t num_servers, const std::vector<faodel::NameAndNode> &server_procs);
static void setup_server(int rank, uint32_t num_servers, const std::vector<faodel::NameAndNode> &server_nodes, 
	md_server &server, int &server_indx);
// void write_testing(int rank, faodel::DirectoryInfo dir, md_server server, uint64_t num_timesteps, uint32_t chunk_id);

int create_and_activate_run(md_server server, md_catalog_run_entry &run, const std::string &rank_to_dims_funct_name,
    const std::string &rank_to_dims_funct_path, const std::string &objector_funct_name, const std::string &objector_funct_path);

int create_and_activate_types(md_server server, uint64_t run_id, uint32_t num_types, 
    uint32_t num_run_timestep_types, md_server_type server_type, uint64_t txn_id);

int create_vars(md_server server, int rank, uint32_t num_servers, uint32_t num_client_procs,
	uint32_t my_num_clients_per_server,
    uint64_t total_x_length, uint64_t total_y_length, uint64_t total_z_length, uint32_t num_vars,
    uint32_t timestep_num, md_server_type server_type, uint64_t txn_id
    );


int create_var_attrs(md_server server, int rank, const std::vector<md_dim_bounds> &proc_dims_vctr,
    uint32_t timestep_num, uint32_t num_vars, uint32_t num_types, md_write_type write_type, 
    double &timestep_temp_min, double &timestep_temp_max, md_server_type server_type, uint64_t txn_id
    );

int insert_vars_and_attrs_batch(md_server server, int rank, const std::vector<md_catalog_var_entry> &all_vars_to_insert,
    const std::vector<md_catalog_var_attribute_entry> &all_attrs_to_insert, uint32_t num_servers );

int create_timestep_attrs(md_server server, int rank, double timestep_temp_min, double timestep_temp_max,
    uint32_t timestep_num, uint32_t num_types, uint32_t num_run_timestep_types, uint32_t num_client_procs,
    double *all_timestep_temp_mins_for_all_procs, double *all_timestep_temp_maxes_for_all_procs,
    md_server_type server_type, uint64_t txn_id
    );

int activate_timestep_components(md_server server, uint32_t timestep_num);

int create_run_attrs(md_server server, int rank, double *all_timestep_temp_mins_for_all_procs, 
    double *all_timestep_temp_maxes_for_all_procs, uint64_t run_id, uint32_t num_timesteps, uint32_t num_types,
    md_server_type server_type, uint64_t txn_id
    );

void output_timing_stats(int rank, uint32_t num_client_procs );

template <class T>
static void make_single_val_data (T val, std::string &serial_str);
template <class T1, class T2>
static void make_double_val_data (T1 val_0, T2 val_1, std::string &serial_str);

//note - just for debugging
int debug_testing(md_server server, int rank, int num_servers, std::vector<md_dim_bounds> dims);

void gatherv_int(std::vector<int> values, uint32_t num_client_procs, int rank, int *each_proc_num_values,
            int *displacement_for_each_proc, int **all_values);

int metadata_collective_insert_var_attribute_by_dims_batch (const md_server &server
                           // std::vector<uint64_t> &attribute_ids,
                           ,const std::vector<md_catalog_var_attribute_entry> &new_attributes,
                           MPI_Comm comm
                           );


int do_reads(md_server server, int rank, uint32_t num_x_procs, uint32_t num_y_procs, uint32_t num_z_procs,
		uint32_t num_timesteps, uint32_t num_servers, uint32_t num_client_procs, uint64_t txn_id,
		MPI_Comm read_comm, MPI_Comm shared_server_comm,
		const md_catalog_run_entry &run, const std::vector<md_dim_bounds> &proc_dims,
		uint32_t plane_x_procs, uint32_t plane_y_procs, const std::vector<std::vector<md_catalog_var_entry>> &all_var_entries,
		const std::vector<md_catalog_var_entry> &vars_to_fetch_pattern2, 
		const std::vector<md_catalog_var_entry> &vars_to_fetch_pattern3,
		const std::vector<md_catalog_var_entry> &vars_to_fetch_pattern4, 
		const std::vector<md_catalog_var_entry> &vars_to_fetch_pattern5,
		const std::vector<md_catalog_var_entry> &vars_to_fetch_pattern6
	);

int read_init(md_server server, int rank, uint32_t num_servers, uint32_t num_client_procs,
		uint32_t num_x_procs, uint32_t num_y_procs, uint32_t num_z_procs,
		uint32_t num_timesteps_to_fetch, uint64_t txn_id, uint64_t *timestep_ids,
		MPI_Comm read_comm, MPI_Comm shared_server_comm, md_catalog_run_entry &run, 
		std::vector<md_dim_bounds> &proc_dims, int &plane_x_procs, int &plane_y_procs,
    	std::vector<std::vector<md_catalog_var_entry>> &all_var_entries,
		std::vector<md_catalog_var_entry> &vars_to_fetch_pattern2,
		std::vector<md_catalog_var_entry> &vars_to_fetch_pattern3, 
		std::vector<md_catalog_var_entry> &vars_to_fetch_pattern4,
		std::vector<md_catalog_var_entry> &vars_to_fetch_pattern5,
		std::vector<md_catalog_var_entry> &vars_to_fetch_pattern6
		);
#endif //TESTINGHARNESSCLASSPROJ_HH
