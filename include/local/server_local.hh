



#ifndef SERVERLOCAL_HH
#define SERVERLOCAL_HH

#include <my_metadata_args.h>


int metadata_finalize_server(int proc_rank, uint64_t job_id);
void metadata_init_server(md_server_type server_type, md_db_index_type index_type, md_db_checkpoint_type checkpt_type);

int db_checkpoint_copy(int proc_rank, uint64_t job_id);
int db_checkpoint_incremental_output(int proc_rank, uint64_t job_id);
int db_checkpoint_copy_and_delete(int proc_rank, uint64_t job_id);
int db_checkpoint_copy_and_reset(int proc_rank, uint64_t job_id);
int db_checkpoint_incremental_output_and_delete(int proc_rank, uint64_t job_id);
int db_checkpoint_incremental_output_and_reset(int proc_rank, uint64_t job_id);


// int metadata_finalize_server(int proc_rank, uint64_t job_id, md_write_type write_type, md_db_checkpoint_type checkpt_type);
// void metadata_init_server(md_server_type server_type, md_db_index_type index_type, md_db_checkpoint_type checkpt_type);

// int db_checkpoint_copy(int proc_rank, uint64_t job_id, md_write_type write_type);
// int db_checkpoint_incremental_output(int proc_rank, uint64_t job_id, md_write_type write_type);
// int db_checkpoint_copy_and_delete(int proc_rank, uint64_t job_id);
// int db_checkpoint_copy_and_reset(int proc_rank, uint64_t job_id, md_write_type write_type);
// // int db_checkpoint_copy_and_reset(int proc_rank, uint64_t job_id);
// int db_checkpoint_incremental_output_and_delete(int proc_rank, uint64_t job_id);
// int db_checkpoint_incremental_output_and_reset(int proc_rank, uint64_t job_id, md_write_type write_type);

template <class T>
void get_range_values(const std::string &data, data_range_type range_type, T &val1, T &val2);

// int md_activate_stub (const md_activate_args &args);
int md_activate_run_attribute_stub (const md_activate_args &args);
int md_activate_run_stub (const md_activate_args &args);
int md_activate_timestep_attribute_stub (const md_activate_args &args);
int md_activate_timestep_stub (const md_activate_args &args);
int md_activate_type_stub (const md_activate_args &args);
int md_activate_var_attribute_stub (const md_activate_args &args);
int md_activate_var_stub (const md_activate_args &args);



int md_catalog_all_run_attributes_stub (const md_catalog_all_run_attributes_args &args,
                           std::vector<md_catalog_run_attribute_entry> &attribute_list,
                           uint32_t &count);

int md_catalog_all_run_attributes_with_type_stub (const md_catalog_all_run_attributes_with_type_args &args,
                           std::vector<md_catalog_run_attribute_entry> &attribute_list,
                           uint32_t &count);

// int md_catalog_all_run_attributes_with_type_range_stub (const md_catalog_all_run_attributes_with_type_range_args &args,
//                            std::vector<md_catalog_run_attribute_entry> &attribute_list,
//                            uint32_t &count);

template <class T>
int md_catalog_all_run_attributes_with_type_range_stub (const md_catalog_all_run_attributes_with_type_range_args &args,
                           T min, T max,
                           std::vector<md_catalog_run_attribute_entry> &attribute_list,
                           uint32_t &count);

int md_catalog_all_timestep_attributes_stub (const md_catalog_all_timestep_attributes_args &args,
                           std::vector<md_catalog_timestep_attribute_entry> &attribute_list,
                           uint32_t &count);

int md_catalog_all_timestep_attributes_with_type_stub (const md_catalog_all_timestep_attributes_with_type_args &args,
                           std::vector<md_catalog_timestep_attribute_entry> &attribute_list,
                           uint32_t &count);

// int md_catalog_all_timestep_attributes_with_type_range_stub (const md_catalog_all_timestep_attributes_with_type_range_args &args,
//                            std::vector<md_catalog_timestep_attribute_entry> &attribute_list,
//                            uint32_t &count);
template <class T>
int md_catalog_all_timestep_attributes_with_type_range_stub (const md_catalog_all_timestep_attributes_with_type_range_args &args,
                           T min, T max,
                           std::vector<md_catalog_timestep_attribute_entry> &attribute_list,
                           uint32_t &count);

int md_catalog_all_timesteps_with_var_attributes_with_type_var_dims_stub (const md_catalog_all_timesteps_with_var_attributes_with_type_var_dims_args &args,
                           std::vector<md_catalog_timestep_entry> &entries,
                           uint32_t &count);

// int md_catalog_all_timesteps_with_var_attributes_with_type_var_dims_range_stub (const md_catalog_all_timesteps_with_var_attributes_with_type_var_dims_range_args &args,
//                            std::vector<md_catalog_timestep_entry> &entries,
//                            uint32_t &count);
template <class T>
int md_catalog_all_timesteps_with_var_attributes_with_type_var_dims_range_stub (const md_catalog_all_timesteps_with_var_attributes_with_type_var_dims_range_args &args,
                           T min, T max,
                           std::vector<md_catalog_timestep_entry> &entries,
                           uint32_t &count);

int md_catalog_all_timesteps_with_var_attributes_with_type_var_stub (const md_catalog_all_timesteps_with_var_attributes_with_type_var_args &args,
                           std::vector<md_catalog_timestep_entry> &entries,
                           uint32_t &count);

// int md_catalog_all_timesteps_with_var_attributes_with_type_var_range_stub (const md_catalog_all_timesteps_with_var_attributes_with_type_var_range_args &args,
//                            std::vector<md_catalog_timestep_entry> &entries,
//                            uint32_t &count);
template <class T>
int md_catalog_all_timesteps_with_var_attributes_with_type_var_range_stub (const md_catalog_all_timesteps_with_var_attributes_with_type_var_range_args &args,
                           T min, T max,
                           std::vector<md_catalog_timestep_entry> &entries,
                           uint32_t &count);

int md_catalog_all_timesteps_with_var_attributes_with_type_var_substr_dims_stub (const md_catalog_all_timesteps_with_var_attributes_with_type_var_substr_dims_args &args,
                           std::vector<md_catalog_timestep_entry> &entries,
                           uint32_t &count);

// int md_catalog_all_timesteps_with_var_attributes_with_type_var_substr_dims_range_stub (const md_catalog_all_timesteps_with_var_attributes_with_type_var_substr_dims_range_args &args,
//                            std::vector<md_catalog_timestep_entry> &entries,
//                            uint32_t &count);

template <class T>
int md_catalog_all_timesteps_with_var_attributes_with_type_var_substr_dims_range_stub (const md_catalog_all_timesteps_with_var_attributes_with_type_var_substr_dims_range_args &args,
                           T min, T max,
                           std::vector<md_catalog_timestep_entry> &entries,
                           uint32_t &count);

int md_catalog_all_timesteps_with_var_attributes_with_type_var_substr_stub (const md_catalog_all_timesteps_with_var_attributes_with_type_var_substr_args &args,
                           std::vector<md_catalog_timestep_entry> &entries,
                           uint32_t &count);

// int md_catalog_all_timesteps_with_var_attributes_with_type_var_substr_range_stub (const md_catalog_all_timesteps_with_var_attributes_with_type_var_substr_range_args &args,
//                            std::vector<md_catalog_timestep_entry> &entries,
//                            uint32_t &count);
template <class T>
int md_catalog_all_timesteps_with_var_attributes_with_type_var_substr_range_stub (const md_catalog_all_timesteps_with_var_attributes_with_type_var_substr_range_args &args,
                           T min, T max,
                           std::vector<md_catalog_timestep_entry> &entries,
                           uint32_t &count);

int md_catalog_all_timesteps_with_var_stub (md_catalog_all_timesteps_with_var_args &args, std::vector<md_catalog_timestep_entry> &entries, uint32_t &count);

int md_catalog_all_timesteps_with_var_substr_stub (md_catalog_all_timesteps_with_var_substr_args &args, std::vector<md_catalog_timestep_entry> &entries, uint32_t &count);


int md_catalog_all_types_with_var_attributes_in_timestep_stub (md_catalog_all_types_with_var_attributes_in_timestep_args &args, 
                          std::vector<md_catalog_type_entry> &entries,
                          uint32_t &count);

int md_catalog_all_types_with_var_attributes_with_var_dims_in_timestep_stub (md_catalog_all_types_with_var_attributes_with_var_dims_in_timestep_args &args, 
                          std::vector<md_catalog_type_entry> &entries,
                          uint32_t &count);

int md_catalog_all_types_with_var_attributes_with_var_in_timestep_stub (md_catalog_all_types_with_var_attributes_with_var_in_timestep_args &args, 
                          std::vector<md_catalog_type_entry> &entries,
                          uint32_t &count);

int md_catalog_all_types_with_var_attributes_with_var_substr_dims_in_timestep_stub (md_catalog_all_types_with_var_attributes_with_var_substr_dims_in_timestep_args &args, 
                          std::vector<md_catalog_type_entry> &entries,
                          uint32_t &count);

int md_catalog_all_types_with_var_attributes_with_var_substr_in_timestep_stub (md_catalog_all_types_with_var_attributes_with_var_substr_in_timestep_args &args, 
                          std::vector<md_catalog_type_entry> &entries,
                          uint32_t &count);

int md_catalog_all_var_attributes_stub (const md_catalog_all_var_attributes_args &args,
						   std::vector<md_catalog_var_attribute_entry> &attribute_list,
						   uint32_t &count);


int md_catalog_all_var_attributes_with_dims_stub (const md_catalog_all_var_attributes_with_dims_args &args,
						   std::vector<md_catalog_var_attribute_entry> &attribute_list,
						   uint32_t &count);

int md_catalog_all_var_attributes_with_type_by_id_stub (const md_catalog_all_var_attributes_with_type_by_id_args &args,
                           std::vector<md_catalog_var_attribute_entry> &attribute_list,
                           uint32_t &count);

int md_catalog_all_var_attributes_with_type_by_name_ver_stub (const md_catalog_all_var_attributes_with_type_by_name_ver_args &args,
                           std::vector<md_catalog_var_attribute_entry> &attribute_list,
                           uint32_t &count);

int md_catalog_all_var_attributes_with_type_dims_by_id_stub (const md_catalog_all_var_attributes_with_type_dims_by_id_args &args,
						   std::vector<md_catalog_var_attribute_entry> &attribute_list,
						   uint32_t &count);

int md_catalog_all_var_attributes_with_type_dims_by_name_ver_stub (const md_catalog_all_var_attributes_with_type_dims_by_name_ver_args &args,
                           std::vector<md_catalog_var_attribute_entry> &attribute_list,
                           uint32_t &count);

int md_catalog_all_var_attributes_with_type_var_by_id_stub (const md_catalog_all_var_attributes_with_type_var_by_id_args &args,
                           std::vector<md_catalog_var_attribute_entry> &attribute_list,
                           uint32_t &count);

int md_catalog_all_var_attributes_with_type_var_by_name_ver_stub (const md_catalog_all_var_attributes_with_type_var_by_name_ver_args &args,
                           std::vector<md_catalog_var_attribute_entry> &attribute_list,
                           uint32_t &count);

int md_catalog_all_var_attributes_with_type_var_dims_by_id_stub (const md_catalog_all_var_attributes_with_type_var_dims_by_id_args &args,
                           std::vector<md_catalog_var_attribute_entry> &attribute_list,
                           uint32_t &count);

int md_catalog_all_var_attributes_with_type_var_dims_by_name_ver_stub (const md_catalog_all_var_attributes_with_type_var_dims_by_name_ver_args &args,
                           std::vector<md_catalog_var_attribute_entry> &attribute_list,
                           uint32_t &count);

// int md_catalog_all_var_attributes_with_type_var_dims_range_stub (const md_catalog_all_var_attributes_with_type_var_dims_range_args &args,
//                            std::vector<md_catalog_var_attribute_entry> &attribute_list,
//                            uint32_t &count);
template <class T>
int md_catalog_all_var_attributes_with_type_var_dims_range_stub (const md_catalog_all_var_attributes_with_type_var_dims_range_args &args,
                           T min, T max,
                           std::vector<md_catalog_var_attribute_entry> &attribute_list,
                           uint32_t &count);

// int md_catalog_all_var_attributes_with_type_var_range_stub (const md_catalog_all_var_attributes_with_type_var_range_args &args,
//                            std::vector<md_catalog_var_attribute_entry> &attribute_list,
//                            uint32_t &count);

template <class T>
int md_catalog_all_var_attributes_with_type_var_range_stub (const md_catalog_all_var_attributes_with_type_var_range_args &args,
                           T min, T max,
                           std::vector<md_catalog_var_attribute_entry> &attribute_list,
                           uint32_t &count);

int md_catalog_all_var_attributes_with_type_var_substr_dims_stub (const md_catalog_all_var_attributes_with_type_var_substr_dims_args &args,
                           std::vector<md_catalog_var_attribute_entry> &attribute_list,
                           uint32_t &count);

// int md_catalog_all_var_attributes_with_type_var_substr_dims_range_stub (const md_catalog_all_var_attributes_with_type_var_substr_dims_range_args &args,
//                            std::vector<md_catalog_var_attribute_entry> &attribute_list,
//                            uint32_t &count);
template <class T>
int md_catalog_all_var_attributes_with_type_var_substr_dims_range_stub (const md_catalog_all_var_attributes_with_type_var_substr_dims_range_args &args,
						   T min, T max,
                           std::vector<md_catalog_var_attribute_entry> &attribute_list,
                           uint32_t &count);

int md_catalog_all_var_attributes_with_type_var_substr_stub (const md_catalog_all_var_attributes_with_type_var_substr_args &args,
                           std::vector<md_catalog_var_attribute_entry> &attribute_list,
                           uint32_t &count);


// int md_catalog_all_var_attributes_with_type_var_substr_range_stub (const md_catalog_all_var_attributes_with_type_var_substr_range_args &args,
//                            std::vector<md_catalog_var_attribute_entry> &attribute_list,
//                            uint32_t &count);
template <class T>
int md_catalog_all_var_attributes_with_type_var_substr_range_stub (const md_catalog_all_var_attributes_with_type_var_substr_range_args &args,
                           T min, T max,
                           std::vector<md_catalog_var_attribute_entry> &attribute_list,
                           uint32_t &count);

int md_catalog_all_var_attributes_with_var_by_id_stub (const md_catalog_all_var_attributes_with_var_by_id_args &args,
                           std::vector<md_catalog_var_attribute_entry> &attribute_list,
                           uint32_t &count);

int md_catalog_all_var_attributes_with_var_by_name_ver_stub (const md_catalog_all_var_attributes_with_var_by_name_ver_args &args,
                           std::vector<md_catalog_var_attribute_entry> &attribute_list,
                           uint32_t &count);

int md_catalog_all_var_attributes_with_var_dims_by_id_stub (const md_catalog_all_var_attributes_with_var_dims_by_id_args &args,
                           std::vector<md_catalog_var_attribute_entry> &attribute_list,
                           uint32_t &count);

int md_catalog_all_var_attributes_with_var_dims_by_name_ver_stub (const md_catalog_all_var_attributes_with_var_dims_by_name_ver_args &args,
                           std::vector<md_catalog_var_attribute_entry> &attribute_list,
                           uint32_t &count);

int md_catalog_all_var_attributes_with_var_substr_dims_stub (const md_catalog_all_var_attributes_with_var_substr_dims_args &args,
                           std::vector<md_catalog_var_attribute_entry> &attribute_list,
                           uint32_t &count);


int md_catalog_all_var_attributes_with_var_substr_stub (const md_catalog_all_var_attributes_with_var_substr_args &args,
                           std::vector<md_catalog_var_attribute_entry> &attribute_list,
                           uint32_t &count);

int md_catalog_run_stub (md_catalog_run_args &args, std::vector<md_catalog_run_entry> &entries, uint32_t &count);

int md_catalog_timestep_stub (md_catalog_timestep_args &args, std::vector<md_catalog_timestep_entry> &entries, uint32_t &count);


int md_catalog_type_stub (md_catalog_type_args &args, 
                          std::vector<md_catalog_type_entry> &entries,
                          uint32_t &count);

int md_catalog_var_stub (md_catalog_var_args &args, std::vector<md_catalog_var_entry> &entries, uint32_t &count);

int md_checkpoint_database_stub (uint64_t job_id, md_db_checkpoint_type checkpt_type);

int md_create_run_stub (const md_create_run_args &args, uint64_t &run_id);

// int md_create_timestep_stub (const md_create_timestep_args &args, uint64_t &timestep_id);
int md_create_timestep_stub (const md_create_timestep_args &args);


// int md_create_type_batch_stub (const std::vector<md_create_type_args> &all_args
// 							,std::vector<uint64_t> &type_ids
// 							);

// int md_create_type_batch_stub (const std::vector<md_create_type_args> &all_args);
int md_create_type_batch_stub (const std::vector<md_create_type_args> &all_args, uint64_t &first_type_id);

int md_create_type_stub (const md_create_type_args &args,
                        uint64_t &type_id);

int md_create_var_batch_stub (const std::vector<md_create_var_args> &args);

// int md_create_var_stub (const md_create_var_args &args,
//                         uint64_t &row_id);
int md_create_var_stub (const md_create_var_args &args);


int md_delete_all_vars_with_substr_stub (const md_delete_all_vars_with_substr_args &args);

int md_delete_run_by_id_stub (const md_delete_run_by_id_args &args);


int md_delete_timestep_by_id_stub (const md_delete_timestep_by_id_args &args);


int md_delete_type_by_id_stub (const md_delete_type_by_id_args &args);

int md_delete_type_by_name_ver_stub (const md_delete_type_by_name_ver_args &args);

int md_delete_var_by_id_stub (const md_delete_var_by_id_args &args);

int md_delete_var_by_name_path_ver_stub (const md_delete_var_by_name_path_ver_args &args);


int md_insert_run_attribute_batch_stub (const std::vector<md_insert_run_attribute_args> &args);

int md_insert_run_attribute_stub (const md_insert_run_attribute_args &args, uint64_t &attribute_id);

int md_insert_timestep_attribute_batch_stub (const std::vector<md_insert_timestep_attribute_args> &args);

int md_insert_timestep_attribute_stub (const md_insert_timestep_attribute_args &args, uint64_t &attribute_id);

int md_insert_var_attribute_by_dims_batch_stub (const std::vector<md_insert_var_attribute_by_dims_args> &all_args);

int md_insert_var_attribute_by_dims_stub (const md_insert_var_attribute_by_dims_args &args, uint64_t &attribute_id);

int md_processing_stub (const md_processing_args &args);

void output_db_statistics(int rank, uint32_t num_procs);

#endif // SERVERLOCAL_HH

