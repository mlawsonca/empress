


#ifndef THREEDREADFORTESTINGLOCAL_HH
#define THREEDREADFORTESTINGLOCAL_HH

#include <my_metadata_client_local.hh> 


int read_pattern_1 (int rank, const std::vector<md_dim_bounds> &proc_dims, 
                    int num_client_procs,
                    uint64_t txn_id, const md_catalog_run_entry &run,
                    const std::vector<md_catalog_var_entry> &entries,
                    const std::vector<uint64_t> &type_ids, MPI_Comm read_comm
                    );

int read_pattern_2 (int rank, const std::vector<md_dim_bounds> &proc_dims, 
                    int num_client_procs,
                    uint64_t txn_id, const md_catalog_run_entry &run,
                    const md_catalog_var_entry &var, 
                    const std::vector<uint64_t> &type_ids, MPI_Comm read_comm
                    );

int read_pattern_3 (int rank, const std::vector<md_dim_bounds> &proc_dims, 
                    int num_client_procs,
                    uint64_t txn_id, const md_catalog_run_entry &run,
                    const std::vector<md_catalog_var_entry> &vars, 
                    const std::vector<uint64_t> &type_ids, MPI_Comm read_comm
                    );

int read_pattern_4 (int rank, int num_x_procs, int num_y_procs, 
                    int num_client_procs,
                    uint64_t txn_id, const md_catalog_run_entry &run,
                    const md_catalog_var_entry &var, 
                    const std::vector<uint64_t> &type_ids, MPI_Comm read_comm
                    );

int read_pattern_5 (int rank, int num_x_procs, int num_y_procs, int num_z_procs, 
	                int num_client_procs,
                    uint64_t txn_id, const md_catalog_run_entry &run,
                    const md_catalog_var_entry &var, 
                    const std::vector<uint64_t> &type_ids, MPI_Comm read_comm
                    );

int read_pattern_6 (int rank, int num_x_procs, int num_y_procs, 
                    int num_client_procs,
                    uint64_t txn_id, const md_catalog_run_entry &run,
                    const md_catalog_var_entry &var, 
                    const std::vector<uint64_t> &type_ids, MPI_Comm read_comm
                    ); 

#endif //THREEDREADFORTESTINGLOCAL_HH