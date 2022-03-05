



#ifndef THREEDREADFORTESTING_HH
#define THREEDREADFORTESTING_HH


int read_pattern_1 (const std::vector<md_dim_bounds> &proc_dims, const md_catalog_run_entry &run,
                    const std::vector<md_catalog_var_entry> &entries, int num_vars
                    , uint64_t txn_id
                    );

int read_pattern_2 (const std::vector<md_dim_bounds> &proc_dims, 
                    const md_catalog_run_entry &run,
                    const md_catalog_var_entry &var
                    , uint64_t txn_id
                    );

int read_pattern_3 (const std::vector<md_dim_bounds> &proc_dims, 
                    const md_catalog_run_entry &run,
                    const std::vector<md_catalog_var_entry> &vars
                    , uint64_t txn_id
                    );

int read_pattern_4 (int rank, int num_x_procs, int num_y_procs, int nx, int ny, int nz, 
                    const md_catalog_run_entry &run, const md_catalog_var_entry &var
                    , uint64_t txn_id
                    );

int read_pattern_5 (int rank, int num_x_procs, int num_y_procs, int num_z_procs, 
                    int nx, int ny, int nz, const md_catalog_run_entry &run,
                    const md_catalog_var_entry &var
                    , uint64_t txn_id
                    );

int read_pattern_6 (int rank, int num_x_procs, int num_y_procs, int nx, int ny, int nz, 
                    const md_catalog_run_entry &run, const md_catalog_var_entry &var
                    , uint64_t txn_id
                    );

#endif //THREEDREADFORTESTING_HH