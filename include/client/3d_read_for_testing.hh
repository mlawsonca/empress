


#ifndef THREEDREADFORTESTING_HH
#define THREEDREADFORTESTING_HH

#include <my_metadata_client.h> 


int read_pattern_1 (const std::vector<md_dim_bounds> &proc_dims, const md_catalog_run_entry &run,
                    const std::vector<md_catalog_var_entry> &entries, int num_vars, uint64_t txn_id);

int read_pattern_2 (int rank, const std::vector<md_dim_bounds> &proc_dims, 
                    int num_servers, int num_client_procs, const md_server &server, bool is_type,
                    uint64_t txn_id, const md_catalog_run_entry &run,
                    const md_catalog_var_entry &var,
                    const std::vector<uint64_t> &type_ids
                    );

int read_pattern_3 (int rank, const std::vector<md_dim_bounds> &proc_dims, int num_servers, 
					int num_client_procs,
                    const md_server &server,  bool is_type,
                    uint64_t txn_id, const md_catalog_run_entry &run,
                    const std::vector<md_catalog_var_entry> &vars,
                    const std::vector<uint64_t> &type_ids
                    );

int read_pattern_4 (int rank, int num_x_procs, int num_y_procs, int nx, int ny, int nz, 
                    uint64_t txn_id, const md_catalog_run_entry &run, const md_catalog_var_entry &var);

int read_pattern_5 (int rank, int num_x_procs, int num_y_procs, int num_z_procs, 
                    int nx, int ny, int nz, uint64_t txn_id, const md_catalog_run_entry &run,
                    const md_catalog_var_entry &var);

int read_pattern_6 (int rank, int num_x_procs, int num_y_procs, int nx, int ny, int nz, 
                    uint64_t txn_id, const md_catalog_run_entry &run, const md_catalog_var_entry &var);

#endif //THREEDREADFORTESTING_HH