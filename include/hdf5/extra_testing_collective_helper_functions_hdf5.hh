



#ifndef EXTRATESTINGCOLLECTIVEHELPERFUNCTIONSHDF5_HH
#define EXTRATESTINGCOLLECTIVEHELPERFUNCTIONSHDF5_HH

#include <my_metadata_client_hdf5.hh>


void catalog_all_metadata_for_run (int rank, uint32_t num_client_procs, 
							std::string run_name, std::string job_id,
							const md_dim_bounds &proc_dims,
							std::vector<std::string> &all_timestep_file_names,
							std::string &timestep0_file_name,
							std::vector<md_var_entry> &timestep0_vars ); 

void catalog_types_in_timestep_functs ( int rank, uint32_t num_client_procs, uint64_t timestep_id,
					std::string timestep_file_name, std::string var_name,
                    md_dim_bounds query_dims ); 

void catalog_timesteps_with_var_or_attr_functs ( int rank, uint32_t num_client_procs,
					std::vector<std::string> all_timestep_file_names, 
					std::string rare_type_name, std::string var_name,
                    md_dim_bounds query_dims
 					); 

void catalog_run_attributes_with_type_or_val_functs ( int rank, uint32_t num_client_procs,
							std::string run_name, std::string job_id, std::string type_name
        					); 

void catalog_timestep_attributes_with_type_or_val_functs ( int rank, uint32_t num_client_procs,
							uint64_t timestep_id, std::string timestep_file_name, std::string type_name
        							); 

void catalog_all_var_attributes_with_dims ( int rank, uint32_t num_client_procs,
							uint64_t timestep_id,
							std::string timestep_file_name, 
							md_dim_bounds query_dims,
							const md_dim_bounds &proc_dims
							); 


void catalog_var_attributes_with_var_functs ( int rank, uint32_t num_client_procs,
								uint64_t timestep_id,
								const std::string &timestep_file_name,
								const std::string &var_name, 
								md_dim_bounds query_dims,
								const md_dim_bounds &proc_dims 								
								); 


void catalog_var_attributes_with_type_functs ( int rank, uint32_t num_client_procs,
									uint64_t timestep_id, std::string timestep_file_name, const std::string &type_name,
									const md_dim_bounds &proc_dims 								
									); 


void catalog_var_attributes_with_type_dims_functs ( int rank, uint32_t num_client_procs,
												uint64_t timestep_id, std::string timestep_file_name, 
                                                md_dim_bounds query_dims, const std::string &type_name,
                                                const md_dim_bounds &proc_dims 
                                                ); 

void catalog_var_attributes_with_type_var ( int rank, uint32_t num_client_procs,
										uint64_t timestep_id, std::string timestep_file_name,
										const std::string &type_name, const std::string &var_name,
                                        const md_dim_bounds &proc_dims
                                        ); 

void catalog_var_attributes_with_type_var_or_val_functs ( int rank, uint32_t num_client_procs,
										uint64_t timestep_id, 
										std::string timestep_file_name,
										const std::string &rare_type_name,
										const std::string &var_name,
                                        md_dim_bounds query_dims,
                                      	const md_dim_bounds &proc_dims
                                        ); 

void catalog_types_with_var_substr_in_timestep_functs ( int rank, uint32_t num_client_procs,
					uint64_t timestep_id, std::string timestep_file_name, const std::string &var_name_substr,
                    md_dim_bounds query_dims );



void catalog_timesteps_with_var_substr_or_val_functs ( int rank, uint32_t num_client_procs,
					uint64_t timestep_id, std::vector<std::string> all_timestep_file_names, 
					std::string type_name, 
					const std::string &var_name_substr, md_dim_bounds query_dims
					 ); 


void catalog_var_attributes_with_var_name_substr_functs ( int rank, uint32_t num_client_procs,
											uint64_t timestep_id, std::string timestep_file_name,
											const std::string &var_name_substr, 
											md_dim_bounds query_dims,
											const md_dim_bounds &proc_dims 
											); 


void catalog_var_attributes_with_type_var_name_substr_or_val_functs ( int rank, uint32_t num_client_procs,
													uint64_t timestep_id, std::string timestep_file_name,
													const std::string &var_name_substr, std::string rare_type_name,
                                                    md_dim_bounds query_dims,
                                                    const md_dim_bounds &proc_dims
                                                    );


#endif //EXTRATESTINGCOLLECTIVEHELPERFUNCTIONSHDF5_HH
