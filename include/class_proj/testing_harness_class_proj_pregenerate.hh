#ifndef TESTINGHARNESSCLASSPROJPREGENERATE_HH
#define TESTINGHARNESSCLASSPROJPREGENERATE_HH

#include <my_metadata_args.h>

static int setup_dirman(const std::string &dirman_hostname, const std::string &dir_path, md_server &dirman, std::vector<faodel::NameAndNode> &server_procs, int rank, uint32_t num_servers, uint32_t num_clients);
// static int setup_dirman(const std::string &dirman_hostname, const std::string &dir_path, md_server &dirman, std::vector<faodel::NameAndNode> &server_procs, int rank, uint32_t num_servers, uint32_t num_clients, int dirman_port=1990);
// static void setup_server(md_server &server, int &server_indx, int rank, uint32_t num_servers, const std::vector<faodel::NameAndNode> &server_procs);
static void setup_server(int rank, uint32_t num_servers, const std::vector<faodel::NameAndNode> &server_nodes, 
    md_server &server, int &server_indx);
// void write_testing(int rank, faodel::DirectoryInfo dir, md_server server, uint64_t num_timesteps, uint32_t chunk_id);


int generate_all_metadata(int rank, int num_client_procs, md_write_type write_type, uint64_t total_x_length, uint64_t total_y_length, uint64_t total_z_length,
    uint32_t num_x_procs, uint32_t num_y_procs, uint32_t num_z_procs, uint32_t num_timesteps, uint64_t job_id,
    md_catalog_run_entry &run, 
    std::vector<md_catalog_timestep_entry> &timesteps, 
    std::vector<md_catalog_type_entry> &types, 
    std::vector<std::vector<md_catalog_var_entry>> &vars,
    std::vector<std::vector<md_catalog_var_attribute_entry>> &all_var_attrs, 
    std::vector<std::vector<md_catalog_timestep_attribute_entry>> &timestep_attrs,
    std::vector<md_catalog_run_attribute_entry> &run_attrs
    );


int generate_run(uint64_t job_id, uint32_t num_x_procs, uint32_t num_y_procs, uint32_t num_z_procs, 
     md_catalog_run_entry &run);

void generate_timesteps(uint64_t run_id, uint32_t num_timesteps, std::vector<md_catalog_timestep_entry> &timesteps);

void generate_types(const std::vector<std::string> &type_names, const std::vector<std::string> &run_timestep_type_names,
    uint64_t run_id, uint32_t num_types, uint32_t num_run_timestep_types, 
    std::vector<md_catalog_type_entry> &types);

void generate_vars(const std::vector<std::string> &var_names, uint64_t total_x_length, uint64_t total_y_length, uint64_t total_z_length,
    uint32_t num_timesteps, std::vector<std::vector<md_catalog_var_entry>> &vars
    );

void generate_var_attrs(const std::vector<double> &type_freqs, const std::vector<std::string> &type_types,
    int rank, uint32_t orig_num_vars, uint32_t orig_num_var_types, 
    const std::vector<md_dim_bounds> &proc_dims_vctr, uint32_t num_timesteps, uint32_t num_vars, uint32_t num_types,
    double *all_timestep_temp_mins, double *all_timestep_temp_maxes, std::vector<std::vector<md_catalog_var_attribute_entry>> &all_attrs
    );

void generate_timestep_attrs(int rank, double *all_timestep_temp_mins, double *all_timestep_temp_maxes,
    uint32_t num_timesteps,
    uint32_t num_types, uint32_t num_run_timestep_types, const std::vector<std::string> &run_timestep_type_data_types,
    uint32_t num_client_procs,
    double *all_timestep_temp_mins_for_all_procs, double *all_timestep_temp_maxes_for_all_procs,
    std::vector<std::vector<md_catalog_timestep_attribute_entry>> &timestep_attrs);

void generate_run_attrs(int rank, double *all_timestep_temp_mins_for_all_procs, 
    double *all_timestep_temp_maxes_for_all_procs, uint64_t run_id, uint32_t num_timesteps, uint32_t num_types,
    uint32_t num_run_timestep_types, const std::vector<std::string> &run_timestep_type_data_types,
    std::vector<md_catalog_run_attribute_entry> &run_attrs
    );

void generate_attr_data(int rank, bool orig, int type_indx, std::string type_data_type, 
    std::string &attr_data, attr_data_type &attr_data_type);

int create_and_activate_run(md_server server, md_catalog_run_entry &run);

int create_and_activate_types(md_server server, uint64_t run_id, const std::vector<md_catalog_type_entry> &all_types_to_insert);

int create_vars(md_server server, int rank, uint32_t num_servers, uint32_t num_client_procs, 
    uint32_t my_num_clients_per_server,
    uint32_t timestep_num, const std::vector<md_catalog_var_entry> &all_vars_to_insert
    );

int create_var_attrs(md_server server, md_write_type write_type, const std::vector<md_catalog_var_attribute_entry> &all_attrs_to_insert
    );


int create_timestep_attrs(md_server server, const std::vector<md_catalog_timestep_attribute_entry> &all_timestep_attrs_to_insert);

int activate_timestep_components(md_server server, uint32_t timestep_num);

int create_run_attrs(md_server server, const std::vector<md_catalog_run_attribute_entry> &all_run_attributes_to_insert
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
#endif //TESTINGHARNESSCLASSPROJPREGENERATE_HH
