


#ifndef EXTRA_TESTING_COLLECTIVE_HELPER_FUNCTIONS_HH
#define EXTRA_TESTING_COLLECTIVE_HELPER_FUNCTIONS_HH

#include <my_metadata_client.h>

int catalog_all_metadata_for_run (const md_server &server, int rank, uint32_t num_servers, uint32_t num_client_procs, uint64_t run_id, 
							const std::vector<md_dim_bounds> &proc_dims, uint64_t txn_id, 
							md_catalog_timestep_entry &timestep0,
							std::vector<md_catalog_var_entry> &timestep0_vars, std::vector<md_catalog_type_entry> &type_entries);

int catalog_types_in_timestep_functs ( const md_server &server, int rank, uint32_t num_servers, uint32_t num_client_procs,
					uint64_t run_id, uint64_t timestep_id, uint64_t var_id,
                    int num_dims, const std::vector<md_dim_bounds> &dims_to_search, uint64_t txn_id 
                    );

int catalog_timesteps_with_var_or_attr_functs ( const md_server &server, int rank, uint32_t num_servers, uint32_t num_client_procs,
					uint64_t run_id, uint64_t rare_type_id, uint64_t value_type_id, uint64_t var_id,
                    int num_dims, const std::vector<md_dim_bounds> &dims_to_search, uint64_t txn_id,
                    double min_range, double max_range, double above_max_val, double below_min_val
 					);

int catalog_run_attributes_with_type_or_val_functs ( const md_server &server, int rank, uint32_t num_servers, uint32_t num_client_procs,
							uint64_t run_id, uint64_t type_id, uint64_t txn_id,
                            double min_range, double max_range, double above_max_val, double below_min_val
                            );

int catalog_timestep_attributes_with_type_or_val_functs ( const md_server &server, int rank, uint32_t num_servers, uint32_t num_client_procs,
							uint64_t run_id, uint64_t timestep_id, uint64_t type_id, uint64_t txn_id,
                            double min_range, double max_range, double above_max_val, double below_min_val
							);

int catalog_all_var_attributes_with_dims ( const md_server &server, int rank, uint32_t num_servers, uint32_t num_client_procs,
							const md_catalog_run_entry &run, const std::vector<md_catalog_var_entry> &vars, 
							uint64_t run_id, uint64_t timestep_id, 
							uint64_t txn_id, int num_dims, const std::vector<md_dim_bounds> &dims_to_search,
							const std::vector<md_dim_bounds> &proc_dims
							);

int catalog_var_attributes_with_var_functs ( const md_server &server, int rank, uint32_t num_servers, uint32_t num_client_procs,
								const md_catalog_run_entry &run, const md_catalog_var_entry &var, 
								int num_dims, const std::vector<md_dim_bounds> &dims_to_search,
								const std::vector<md_dim_bounds> &proc_dims, uint64_t txn_id 
								);

int catalog_var_attributes_with_type_functs ( const md_server &server, int rank, uint32_t num_servers, uint32_t num_client_procs,
										const md_catalog_run_entry &run, const std::vector<md_catalog_var_entry> &vars,
										uint64_t timestep_id, const md_catalog_type_entry &type,
										const std::vector<md_dim_bounds> &proc_dims, uint64_t txn_id 
										);


int catalog_var_attributes_with_type_dims_functs ( const md_server &server, int rank, uint32_t num_servers, uint32_t num_client_procs,
												const md_catalog_run_entry &run, const std::vector<md_catalog_var_entry> &vars,
												uint64_t timestep_id, int num_dims, 
                                                const std::vector<md_dim_bounds> &dims_to_search, const md_catalog_type_entry &type,
                    							const std::vector<md_dim_bounds> &proc_dims, uint64_t txn_id 
 												);

int catalog_var_attributes_with_type_var ( const md_server &server, int rank, uint32_t num_servers, uint32_t num_client_procs,
										const md_catalog_run_entry &run, 
										const md_catalog_type_entry &type, const std::vector<md_catalog_var_entry> &vars,
										const md_catalog_var_entry &var,
                                        uint64_t txn_id, const std::vector<md_dim_bounds> &proc_dims
                                        );

int catalog_var_attributes_with_type_var_or_val_functs ( const md_server &server, int rank, uint32_t num_servers, uint32_t num_client_procs,
										const md_catalog_run_entry &run, const md_catalog_type_entry &rare_type,
										const md_catalog_type_entry &value_type, const std::vector<md_catalog_var_entry> &vars,
										const md_catalog_var_entry &var,
                                        uint64_t txn_id, int num_dims, const std::vector<md_dim_bounds> &dims_to_search,
                                      	double min_range, double max_range, double above_max_val, double below_min_val,
            							const std::vector<md_dim_bounds> &proc_dims
                                        );

int catalog_types_with_var_substr_in_timestep_functs ( const md_server &server, int rank, uint32_t num_servers, uint32_t num_client_procs,
					uint64_t run_id, uint64_t timestep_id, const std::string &var_substr,
                    int num_dims, const std::vector<md_dim_bounds> &dims_to_search, uint64_t txn_id 
                    );


int catalog_timesteps_with_var_substr_or_val_functs ( const md_server &server, int rank, uint32_t num_servers, uint32_t num_client_procs,
					uint64_t run_id, uint64_t type_id, 
					const std::string &var_name_substr, int num_dims, const std::vector<md_dim_bounds> &dims_to_search, uint64_t txn_id,
					 double min_range, double max_range, double above_max_val, double below_min_val
					 );

int catalog_var_attributes_with_var_name_substr_functs ( const md_server &server, int rank, uint32_t num_servers, uint32_t num_client_procs,
											const md_catalog_run_entry &run, const std::vector<md_catalog_var_entry> &vars,
											uint64_t run_id, uint64_t timestep_id,
											const std::string &var_name_substr, 
											int num_dims, const std::vector<md_dim_bounds> &dims_to_search, uint64_t txn_id,
            								const std::vector<md_dim_bounds> &proc_dims
											);


int catalog_var_attributes_with_type_var_name_substr_or_val_functs ( const md_server &server, int rank, uint32_t num_servers, uint32_t num_client_procs,
													const md_catalog_run_entry &run, const std::vector<md_catalog_var_entry> &vars,
													uint64_t run_id, uint64_t timestep_id,
													const std::string &var_name_substr, uint64_t rare_type_id,
                                                    uint64_t val_type_id, int num_dims, const std::vector<md_dim_bounds> &dims_to_search,
                                                    uint64_t txn_id,
                                                    double min_range, double max_range, double above_max_val, double below_min_val,
            										const std::vector<md_dim_bounds> &proc_dims
                                                    );

#endif //EXTRA_TESTING_COLLECTIVE_HELPER_FUNCTIONS_HH

