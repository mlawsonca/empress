



#ifndef MYMETADATACLIENTLOCAL_H
#define MYMETADATACLIENTLOCAL_H

#include <my_metadata_args.h>

//for debugging testing purposes/////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void print_md_dim_bounds( const std::vector<md_dim_bounds> &dims);

void print_var_catalog (uint32_t num_vars, const std::vector<md_catalog_var_entry> &entries);
void print_type_catalog (uint32_t num_vars, const std::vector<md_catalog_type_entry> &entries);
// void print_attribute_list (uint32_t num_vars, const std::vector<md_catalog_var_attribute_entry> &attributes);
void print_run_catalog (uint32_t num_vars, const std::vector<md_catalog_run_entry> &runs);
void print_timestep_catalog (uint32_t num_vars, const std::vector<md_catalog_timestep_entry> &timesteps);
void print_var_attribute_list (uint32_t num_vars, const std::vector<md_catalog_var_attribute_entry> &attrs);
void print_timestep_attribute_list (uint32_t num_vars, const std::vector<md_catalog_timestep_attribute_entry> &attrs);
void print_run_attribute_list (uint32_t num_vars, const std::vector<md_catalog_run_attribute_entry> &attrs);

bool dims_overlap(const std::vector<md_dim_bounds> &attr_dims, const std::vector<md_dim_bounds> &proc_dims); 
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// NEW OPS BELOW: BASIC FUNCTIONALITY FOR RUNS/TIMESTEPS/THEIR ASSOCIATED ATTRS ////////////////////////////////////////////////////////////////////////////


// opens the lua libs
// int metadata_init_write (int proc_rank, md_write_type write_type);
// int metadata_init_write (int rank, uint64_t job_id, md_write_type write_type, md_db_checkpoint_type checkpt_type);
// int metadata_init_read(bool load_multiple_db, uint64_t job_id, int rank, uint32_t num_read_procs, md_write_type write_type);

int metadata_init_write (int rank, uint64_t job_id, md_server_type server_type, 
	md_db_index_type index_type, md_db_checkpoint_type checkpt_type);
int metadata_init_read(bool load_multiple_db, uint64_t job_id, int rank, uint32_t num_read_procs, 
	md_server_type server_type );
// int metadata_init (bool load_db, uint64_t job_id, int rank);
// int metadata_init_multi_db (uint64_t job_id, int rank_first, int rank_last);
// int metadata_init_multi_db (uint64_t job_id, int rank, uint32_t num_read_procs);

// send a message to the given server telling it to shutdown
// int metadata_finalize_server (const md_server &server);

// tells the client to close the lua state and db
// int metadata_finalize_client(bool output_db, uint64_t job_id, int rank);
// int metadata_finalize_client(int rank, uint64_t job_id, md_write_type write_type, md_db_checkpoint_type checkpt_type);
int metadata_finalize_client(int rank, uint64_t job_id);
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


// used by each of the activate functions
static int metadata_activate (uint64_t txn_id
                          , md_catalog_type catalog_type
                          );

// changes all run attributes associated with the given transaction id to "active"
int metadata_activate_run_attribute (uint64_t txn_id
                          );

// changes all runs associated with the given transaction id to "active"
int metadata_activate_run (uint64_t txn_id
                          );

// changes all timestep attributes associated with the given transaction id to "active"
int metadata_activate_timestep_attribute (uint64_t txn_id
                          );

// changes all timesteps associated with the given transaction id to "active"
int metadata_activate_timestep (uint64_t txn_id
                          );

// changes all custom metadata types associated with the given transaction id to "active"
int metadata_activate_type (uint64_t txn_id
                          );

// changes all var attributes associated with the given transaction id to "active"
int metadata_activate_var_attribute (uint64_t txn_id
                          );

// changes all vars associated with the given transaction id to "active"
int metadata_activate_var (uint64_t txn_id
                          );


int metadata_catalog_all_run_attributes (uint64_t txn_id
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_run_attribute_entry> &entries
                                        );

// for the given run (id), lists all attributes associated with the (entire) run that are active or match the given transaction id
// e.g., for run X, retrieve all of the attributes associated with the run
int metadata_catalog_all_run_attributes_in_run (uint64_t run_id             
                                        ,uint64_t txn_id
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_run_attribute_entry> &entries
                                        );

int metadata_catalog_all_run_attributes_with_type (uint64_t type_id             
                                        ,uint64_t txn_id
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_run_attribute_entry> &entries
                                        );

// for the given run (id), lists all attributes associated with the (entire) run of the given type (id) that are active or match the given transaction id
// e.g., for run X, retrieve all of the flag attributes associated with the run
int metadata_catalog_all_run_attributes_with_type_in_run (uint64_t run_id
                                        ,uint64_t type_id             
                                        ,uint64_t txn_id
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_run_attribute_entry> &entries
                                        );

int metadata_catalog_all_timestep_attributes (
                                         uint64_t run_id             
                                        ,uint64_t txn_id
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_timestep_attribute_entry> &entries
                                        );

// for the given timestep (id) of the given run (id), lists all attributes associated with the (entire)timestep that are active or match the given transaction id
// e.g., for the 100th timestep of run X, retrieve all of the attributes associated with the timestep
int metadata_catalog_all_timestep_attributes_in_timestep (uint64_t run_id             
                                        ,uint64_t timestep_id             
                                        ,uint64_t txn_id
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_timestep_attribute_entry> &entries
                                        );

int metadata_catalog_all_timestep_attributes_with_type (
                                         uint64_t run_id             
                                        ,uint64_t type_id             
                                        ,uint64_t txn_id
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_timestep_attribute_entry> &entries
                                        );

// for the given timestep (id) of the given run (id), lists all attributes associated with the timestep of the given type (id) 
// that are active or match the given transaction id
// e.g., for the 100th timestep of run X, retrieve all of the flag attributes associated with the timestep
int metadata_catalog_all_timestep_attributes_with_type_in_timestep (uint64_t run_id             
                                        ,uint64_t timestep_id
                                        ,uint64_t type_id             
                                        ,uint64_t txn_id
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_timestep_attribute_entry> &entries
                                        );

// for the given run (id), lists all timesteps where there are attributes of the given type (id) on the given var (id) in the given dimensions
// that are active or match the given transaction id
// e.g., for run X, list all timesteps where there is a flag (attribute) on temperature in the upper right quadrant
int metadata_catalog_all_timesteps_with_var_attributes_with_type_var_dims (uint64_t run_id             
                                        ,uint64_t type_id
                                        ,uint64_t var_id             
                                        ,uint64_t txn_id
                                        ,uint32_t num_dims
                                        ,const std::vector<md_dim_bounds> &dims
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_timestep_entry> &entries
                                        );

// for the given run (id), lists all timesteps where there are attributes of the given type (id) on the given var (id) 
// that are active or match the given transaction id
// e.g., for run X, list all timesteps where there is a flag (attribute) on temperature 
int metadata_catalog_all_timesteps_with_var_attributes_with_type_var (uint64_t run_id             
                                        ,uint64_t type_id
                                        ,uint64_t var_id             
                                        ,uint64_t txn_id
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_timestep_entry> &entries
                                        );

// for the given run (id), find all timesteps that have the given var (id) that are active or match the given transaction id
// note: var_id is constant across timesteps for the same var name and ver
// e.g., for run X, find all timesteps that the variable "temperature" version 0
int metadata_catalog_all_timesteps_with_var (uint64_t run_id             
                                        ,uint64_t var_id             
                                        ,uint64_t txn_id
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_timestep_entry> &entries
                                        );

int metadata_catalog_all_types_with_var_attributes (
                       uint64_t run_id
                      ,uint64_t txn_id
                      ,uint32_t &count
                      ,std::vector<md_catalog_type_entry> &entries
                     );

// for the timestep (id) of the given run (id), lists all custom metadata types that have an instance on a (any) variable
// that are active or match the given transaction id
// e.g., for the 100th timestep of run X, look at all var attributes and list their associated types (omitting duplicates)
int metadata_catalog_all_types_with_var_attributes_in_timestep (uint64_t run_id
                      ,uint64_t timestep_id
                      ,uint64_t txn_id
                      ,uint32_t &count
                      ,std::vector<md_catalog_type_entry> &entries
                     );

int metadata_catalog_all_types_with_var_attributes_with_var_dims (
                       uint64_t run_id
                      ,uint64_t var_id
                      ,uint64_t txn_id
                      ,uint32_t num_dims
                      ,const std::vector<md_dim_bounds> &dims
                      ,uint32_t &count
                      ,std::vector<md_catalog_type_entry> &entries
                     );

// for the given timestep (id) of the given run (id), lists all types that have an instance on the given var (id) in the given dimensions
// that are active or match the given transaction id
// e.g., for the 100th timestep of run X, list all types that have an instance temperature in the upper right quadrant
int metadata_catalog_all_types_with_var_attributes_with_var_dims_in_timestep (uint64_t run_id
                      ,uint64_t timestep_id
                      ,uint64_t var_id
                      ,uint64_t txn_id
                      ,uint32_t num_dims
                      ,const std::vector<md_dim_bounds> &dims
                      ,uint32_t &count
                      ,std::vector<md_catalog_type_entry> &entries
                     );


int metadata_catalog_all_types_with_var_attributes_with_var (
                       uint64_t run_id
                      ,uint64_t var_id
                      ,uint64_t txn_id
                      ,uint32_t &count
                      ,std::vector<md_catalog_type_entry> &entries
                     );

// for the given timestep (id) of the given run (id), lists all types that have an instance on the given var (id) 
// that are active or match the given transaction id
// e.g., for the 100th timestep of run X, list all types that have an instance temperature 
int metadata_catalog_all_types_with_var_attributes_with_var_in_timestep (uint64_t run_id
                      ,uint64_t timestep_id
                      ,uint64_t var_id
                      ,uint64_t txn_id
                      ,uint32_t &count
                      ,std::vector<md_catalog_type_entry> &entries
                     );

// for the given timestep (id) of the given run (id), lists all attributes on a variable (any variable)
// that are active or match the given transaction id
// e.g., for the 100th timestep of run X, list all custom metadata attributes
int metadata_catalog_all_var_attributes (uint64_t run_id             
                            ,uint64_t timestep_id             
                            ,uint64_t txn_id
                            ,uint32_t &count
                            ,std::vector<md_catalog_var_attribute_entry> &entries
                            );

// for the given timestep (id) of the given run (id), lists all attributes on a variable (any variable) in the given dimensions
// that are active or match the given transaction id
// e.g., for the 100th timestep of run X, list all custom metadata attributes in the upper right quadrant
int metadata_catalog_all_var_attributes_with_dims (uint64_t run_id             
                       ,uint64_t timestep_id
                       ,uint64_t txn_id
                       ,uint32_t num_dims
                       ,const std::vector<md_dim_bounds> &dims
                       ,uint32_t &count
                       ,std::vector<md_catalog_var_attribute_entry> &matching_attributes
                       );

// for the given timestep (id) of the given run (found using type_id), lists all attributes of the given type (id) on a variable (any variable)
// that are active or match the given transaction id
// e.g., for the 100th timestep of run X, list all flag attributes
int metadata_catalog_all_var_attributes_with_type_by_id (uint64_t timestep_id
                            ,uint64_t type_id                   
                            ,uint64_t txn_id
                            ,uint32_t &count
                            ,std::vector<md_catalog_var_attribute_entry> &entries
                            );


// for the given timestep (id) of the given run (id), lists all attributes of the given type (name and ver) on a variable (any variable)
// that are active or match the given transaction id
// e.g., for the 100th timestep of run X, list all flag attributes
int metadata_catalog_all_var_attributes_with_type_by_name_ver (uint64_t run_id             
                            ,uint64_t timestep_id
                            ,const std::string &type_name
                            ,uint32_t type_version                   
                            ,uint64_t txn_id
                            ,uint32_t &count
                            ,std::vector<md_catalog_var_attribute_entry> &entries
                            );

// for the given timestep (id) of the given run (found using type_id), lists all attributes of the given type (id) on a variable (any variable) 
// in the given dimension that are active or match the given transaction id
// e.g., for the 100th timestep of run X, list all flag attributes in the upper right quadrant
int metadata_catalog_all_var_attributes_with_type_dims_by_id (uint64_t timestep_id
                       ,uint64_t type_id
                       ,uint64_t txn_id
                       ,uint32_t num_dims
                       ,const std::vector<md_dim_bounds> &dims
                       ,uint32_t &count
                       ,std::vector<md_catalog_var_attribute_entry> &matching_attributes
                       );


// for the given timestep (id) of the given run (id), lists all attributes of the given type (name and ver) on a variable (any variable) 
// in the given dimension that are active or match the given transaction id
// e.g., for the 100th timestep of run X, list all flag attributes in the upper right quadrant
int metadata_catalog_all_var_attributes_with_type_dims_by_name_ver (uint64_t run_id             
                       ,uint64_t timestep_id
                       ,const std::string &type_name
                       ,uint32_t type_version
                       ,uint64_t txn_id
                       ,uint32_t num_dims
                       ,const std::vector<md_dim_bounds> &dims
                       ,uint32_t &count
                       ,std::vector<md_catalog_var_attribute_entry> &matching_attributes
                       );

// for the given timestep (id) of the given run (found using type_id), lists all attributes of the given type (id) and variable (id)
// that are active or match the given transaction id
// e.g., for the 100th timestep of run X, list all flag on temperature
int metadata_catalog_all_var_attributes_with_type_var_by_id (uint64_t timestep_id
                       ,uint64_t type_id
                       ,uint64_t var_id
                       ,uint64_t txn_id
                       ,uint32_t &count
                       ,std::vector<md_catalog_var_attribute_entry> &matching_attributes
                       );

// for the given timestep (id) of the given run (id), lists all attributes of the given type (name ver) and variable (name ver)
// that are active or match the given transaction id
// e.g., for the 100th timestep of run X, list all flag on temperature
int metadata_catalog_all_var_attributes_with_type_var_by_name_ver (uint64_t run_id             
                       ,uint64_t timestep_id
                       ,const std::string &type_name
                       ,uint32_t type_version
                       ,const std::string &var_name
                       ,uint32_t var_version
                       ,uint64_t txn_id
                       ,uint32_t &count
                       ,std::vector<md_catalog_var_attribute_entry> &matching_attributes
                       );

// for the given timestep (id) of the given run (found using type_id), lists all attributes of the given type (id),  variable (id) and dimensions
// that are active or match the given transaction id
// e.g., for the 100th timestep of run X, list all flag on temperature in the upper right quadrant
int metadata_catalog_all_var_attributes_with_type_var_dims_by_id (uint64_t timestep_id
                       ,uint64_t type_id
                       ,uint64_t var_id
                       ,uint64_t txn_id
                       ,uint32_t num_dims
                       ,const std::vector<md_dim_bounds> &dims
                       ,uint32_t &count
                       ,std::vector<md_catalog_var_attribute_entry> &matching_attributes
                       );

// for the given timestep (id) of the given run (id), lists all attributes of the given type (name ver),  variable (name ver) and dimensions
// that are active or match the given transaction id
// e.g., for the 100th timestep of run X, list all flag on temperature in the upper right quadrant
int metadata_catalog_all_var_attributes_with_type_var_dims_by_name_ver (uint64_t run_id             
                       ,uint64_t timestep_id
                       ,const std::string &type_name
                       ,uint32_t type_version
                       ,const std::string &var_name
                       ,uint32_t var_version
                       ,uint64_t txn_id
                       ,uint32_t num_dims
                       ,const std::vector<md_dim_bounds> &dims
                       ,uint32_t &count
                       ,std::vector<md_catalog_var_attribute_entry> &matching_attributes
                       );


// for the given timestep (id) of the given run (id), lists all attributes associated with the given variable (id) 
// that are active or match the given transaction id
// e.g., for the 100th timestep of run X, list all attributes on all or part of the temperature variable
int metadata_catalog_all_var_attributes_with_var_by_id (uint64_t run_id             
                       ,uint64_t timestep_id
                       ,uint64_t var_id
                       ,uint64_t txn_id
                       ,uint32_t &count
                       ,std::vector<md_catalog_var_attribute_entry> &matching_attributes
                       );

// for the given timestep (id) of the given run (id), lists all attributes associated with the given variable (name ver) 
// that are active or match the given transaction id
// e.g., for the 100th timestep of run X, list all attributes on (all or part of) the temperature variable
int metadata_catalog_all_var_attributes_with_var_by_name_ver (uint64_t run_id             
                       ,uint64_t timestep_id
                       ,const std::string &var_name
                       ,uint32_t var_version
                       ,uint64_t txn_id
                       ,uint32_t &count
                       ,std::vector<md_catalog_var_attribute_entry> &matching_attributes
                       );

// for the given timestep (id) of the given run (id), lists all attributes associated with the given variable (id) in the given dimensions
// that are active or match the given transaction id
// e.g., for the 100th timestep of run X, list all attributes on the temperature variable in the upper right quadrant
int metadata_catalog_all_var_attributes_with_var_dims_by_id (uint64_t run_id             
                       ,uint64_t timestep_id
                       ,uint64_t var_id
                       ,uint64_t txn_id
                       ,uint32_t num_dims
                       ,const std::vector<md_dim_bounds> &dims
                       ,uint32_t &count
                       ,std::vector<md_catalog_var_attribute_entry> &matching_attributes
                       );

// for the given timestep (id) of the given run (id), lists all attributes associated with the given variable (name ver) in the given dimensions
// that are active or match the given transaction id
// e.g., for the 100th timestep of run X, list all attributes on the temperature variable in the upper right quadrant
int metadata_catalog_all_var_attributes_with_var_dims_by_name_ver (uint64_t run_id             
                       ,uint64_t timestep_id
                       ,const std::string &var_name
                       ,uint32_t var_version                       
                       ,uint64_t txn_id
                       ,uint32_t num_dims
                       ,const std::vector<md_dim_bounds> &dims
                       ,uint32_t &count
                       ,std::vector<md_catalog_var_attribute_entry> &matching_attributes
                       );

// lists all runs that are active or match the given transaction id
// e.g., retrieve info about all of the runs that are found in storage
int metadata_catalog_run (uint64_t txn_id
                      ,uint32_t &count
                      ,std::vector<md_catalog_run_entry> &entries
                     );


// for the given run (id), lists all timesteps that are active or match the given transaction id
// e.g., for run X, list all timesteps
int metadata_catalog_timestep (uint64_t run_id             
                      ,uint64_t txn_id
                      ,uint32_t &count
                      ,std::vector<md_catalog_timestep_entry> &entries
                     );

// for the given run (id), lists all custom metadata types that have been added to the database
// that are active or match the given transaction id
// e.g., for run X, list all custom metadata types
int metadata_catalog_type (uint64_t run_id
                      ,uint64_t txn_id                     
                      ,uint32_t &count
                      ,std::vector<md_catalog_type_entry> &entries
                     );


// for the given timestep (id) of the given run (id), lists all variables that have been added to the database
// that are active or match the given transaction id
// e.g., for the 100th timestep of run X, list all variables
int metadata_catalog_var (uint64_t run_id
                      ,uint64_t timestep_id
                      ,uint64_t txn_id
                      ,uint32_t &count
                      ,std::vector<md_catalog_var_entry> &entries
                     );

//checkpoint the in memory database to disk
// int metadata_checkpoint_database (uint64_t job_id
//   								 , md_db_checkpoint_type checkpt_type
//   								 );

// int metadata_checkpoint_database (int proc_rank
// 					  ,uint64_t job_id
//                       ,md_db_checkpoint_type checkpt_type
//                       ,md_write_type write_type
//                       );
int metadata_checkpoint_database (int proc_rank
					  ,uint64_t job_id
                      );

// create a run in an inactive state. Timesteps/Vars/types/run attributes/timestep attributes/var attributes will be added later. Once the
// run is complete (the transaction is ready to commit), it can then be
// activated to make it visible to other processes.
int metadata_create_run  (uint64_t &run_id     
                        ,uint64_t job_id                   
                        ,const std::string &name
                        // ,const std::string &path
                        ,uint64_t txn_id 
                        ,uint64_t npx
                        ,uint64_t npy
                        ,uint64_t npz
                        ,const std::string &rank_to_dims_funct_name
                        ,const std::string &rank_to_dims_funct_path
                        ,const std::string &objector_funct_name
                        ,const std::string &objector_funct_path
                        );

// create a timestep in an inactive state. Vars/timestep attributes/var attributes will be added later. Once the
// timestep is complete (the transaction is ready to commit), it can then be
// activated to make it visible to other processes.
int metadata_create_timestep  (uint64_t timestep_id                        
                        ,uint64_t run_id                        
                        ,const std::string &path
                        ,uint64_t txn_id 
                        );

// create a timestep in an inactive state. Run attribtues/timestep attributes/var attributes will be added later. Once the
// type is complete (the transaction is ready to commit), it can then be
// activated to make it visible to other processes.
int metadata_create_type (uint64_t &type_id
                        ,uint64_t run_id
                        ,const std::string &name
                        ,uint32_t version
                        ,uint64_t txn_id
                        );
int metadata_create_type (uint64_t &type_id
                        ,const md_catalog_type_entry &new_type
                        ); 

// create a var in an inactive state. var attributes will be added later. Once the
// var is complete (the transaction is ready to commit), it can then be
// activated to make it visible to other processes.
int metadata_create_var (uint64_t run_id
                        ,uint64_t timestep_id
                        ,uint64_t var_id
                        ,const std::string &name
                        ,const std::string &path
                        ,uint32_t version
                        ,char data_size
                        ,uint64_t txn_id
                        ,uint32_t num_dims
                        ,const std::vector<md_dim_bounds> &dims
                        );
int metadata_create_var (uint64_t &var_id
                        ,const md_catalog_var_entry &new_var
                        );

// delete the specified run (id) and all of the types/timesteps/vars/run attributes/timestep attributes/var attributes associated with it
int metadata_delete_run_by_id (uint64_t run_id
                        );

// delete the specified timestep (id) and all of the vars/timestep attributes/var attributes associated with it
int metadata_delete_timestep_by_id (uint64_t timestep_id
                        ,uint64_t run_id
                        );

// delete the specified type (id) and all of the run attributes/timestep attributes/var attributes associated with it
int metadata_delete_type_by_id (uint64_t type_id
                        );

// delete the specified type (name ver) and all of the run attributes/timestep attributes/var attributes associated with it
int metadata_delete_type_by_name_ver (uint64_t run_id
                        ,const std::string &name
                        ,uint32_t version
                        );

// delete the specified var (id) and all of the var attributes associated with it
int metadata_delete_var_by_id (uint64_t run_id
                        ,uint64_t timestep_id
                        ,uint64_t var_id
                        );

// delete the specified var (name ver) and all of the var attributes associated with it
int metadata_delete_var_by_name_path_ver (uint64_t run_id
                        ,uint64_t timestep_id
                        ,const std::string &name
                        ,const std::string &path
                        ,uint32_t version
                        );

// create a run attribute (attribtue on the entire run) in an inactive state. Once the transaction is ready to commit), it can then be
// activated to make it visible to other processes.
int metadata_insert_run_attribute (uint64_t &attribute_id
						   ,uint64_t run_id
                           ,uint64_t type_id
                           ,uint64_t txn_id
                           ,attr_data_type data_type
                           ,const std::string &data
                           );
int metadata_insert_run_attribute (uint64_t &attribute_id,
                           const md_catalog_run_attribute_entry &new_attribute);

// create a timestep attribute (attribtue on the entire timestep) in an inactive state. Once the transaction is ready to commit), it can then be
// activated to make it visible to other processes.
int metadata_insert_timestep_attribute (uint64_t &attribute_id
                           ,uint64_t timestep_id
                           ,uint64_t type_id
                           ,uint64_t txn_id
                           ,attr_data_type data_type
                           ,const std::string &data
                           );
int metadata_insert_timestep_attribute (uint64_t &attribute_id,
                           const md_catalog_timestep_attribute_entry &new_attribute);

// create a var attribute in an inactive state. Once the transaction is ready to commit), it can then be
// activated to make it visible to other processes.
int metadata_insert_var_attribute_by_dims (uint64_t &attribute_id
                           ,uint64_t timestep_id
                           ,uint64_t type_id
                           ,uint64_t var_id
                           ,uint64_t txn_id
                           ,uint32_t num_dims
                           ,const std::vector<md_dim_bounds> &dims
                           ,attr_data_type data_type
                           ,const std::string &data
                           );
int metadata_insert_var_attribute_by_dims (uint64_t &attribute_id,
                           const md_catalog_var_attribute_entry &new_attribute);

// used by each of the processing functions
static int metadata_processing (uint64_t txn_id
                          , md_catalog_type catalog_type                          
                          );

// changes all run attributes associated with the given transaction id to "inactive"
int metadata_processing_run_attribute (uint64_t txn_id
                          );

// changes all runs associated with the given transaction id to "inactive"
int metadata_processing_run (uint64_t txn_id
                          );

// changes all timestep attributes associated with the given transaction id to "inactive"
int metadata_processing_timestep_attribute (uint64_t txn_id
                          );

// changes all timesteps associated with the given transaction id to "inactive"
int metadata_processing_timestep (uint64_t txn_id
                          );

// changes all types associated with the given transaction id to "inactive"
int metadata_processing_type (uint64_t txn_id
                          );

// changes all var attributes associated with the given transaction id to "inactive"
int metadata_processing_var_attribute  (uint64_t txn_id
                          );

// changes all vars associated with the given transaction id to "inactive"
int metadata_processing_var  (uint64_t txn_id
                          );


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// NEW OPS: RANGE/MIN/MAX QUERIES //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   
int metadata_catalog_all_run_attributes_with_type_range (
										 uint64_t type_id             
                                        ,uint64_t txn_id
                                        ,attr_data_type data_type
                                        ,const std::string &data
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_run_attribute_entry> &entries
                                        );

int metadata_catalog_all_run_attributes_with_type_above_max (
										 uint64_t type_id             
                                        ,uint64_t txn_id
                                        ,attr_data_type data_type
                                        ,const std::string &data
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_run_attribute_entry> &entries
                                        );

int metadata_catalog_all_run_attributes_with_type_below_min (
										 uint64_t type_id             
                                        ,uint64_t txn_id
                                        ,attr_data_type data_type
                                        ,const std::string &data
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_run_attribute_entry> &entries
                                        );


// for the given run (id), lists all attributes associated with the (entire) run of the given type (id) that have data that is numeric and in 
// the given (serialized) range, and which are active or match the given transaction id
// e.g., for run X, retrieve all of the 'high turbulence' attributes between values 1000 and 5000 associated with the run
int metadata_catalog_all_run_attributes_with_type_range_in_run (uint64_t run_id
                                        ,uint64_t type_id             
                                        ,uint64_t txn_id
                                        ,attr_data_type data_type
                                        ,const std::string &data
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_run_attribute_entry> &entries
                                        );

// for the given run (id), lists all attributes associated with the (entire) run of the given type (id) that have data that is numeric and above
// the given (serialized) value, and which are active or match the given transaction id
// e.g., for run X, retrieve all of the 'high turbulence' attributes above 5000 associated with the run
int metadata_catalog_all_run_attributes_with_type_above_max_in_run (uint64_t run_id
                                        ,uint64_t type_id             
                                        ,uint64_t txn_id
                                        ,attr_data_type data_type
                                        ,const std::string &data
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_run_attribute_entry> &entries
                                        );

// for the given run (id), lists all attributes associated with the (entire) run of the given type (id) that have data that is numeric and below
// the given (serialized) value, and which are active or match the given transaction id
// e.g., for run X, retrieve all of the 'low temperature' attributes below -100 associated with the run
int metadata_catalog_all_run_attributes_with_type_below_min_in_run (uint64_t run_id
                                        ,uint64_t type_id             
                                        ,uint64_t txn_id
                                        ,attr_data_type data_type
                                        ,const std::string &data
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_run_attribute_entry> &entries
                                        );


// for the given run (id), lists all timesteps where there are attributes of the given type (id) on the given var (id) 
// that have data that is numeric and in the given (serialized) range, and which are active or match the given transaction id
// e.g., for run X, list all timesteps where there is a max on temperature between 100 and 1000
int metadata_catalog_all_timesteps_with_var_attributes_with_type_var_range (uint64_t run_id             
                                        ,uint64_t type_id
                                        ,uint64_t var_id             
                                        ,uint64_t txn_id
                                        ,attr_data_type data_type
                                        ,const std::string &data
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_timestep_entry> &entries
                                        );

// for the given run (id), lists all timesteps where there are attributes of the given type (id) on the given var (id) 
// that have data that is numeric and above the given (serialized) value, and which are active or match the given transaction id
// e.g., for run X, list all timesteps where there is a max on temperature above 1000
int metadata_catalog_all_timesteps_with_var_attributes_with_type_var_above_max (uint64_t run_id             
                                        ,uint64_t type_id
                                        ,uint64_t var_id             
                                        ,uint64_t txn_id
                                        ,attr_data_type data_type
                                        ,const std::string &data                                        
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_timestep_entry> &entries
                                        );

// for the given run (id), lists all timesteps where there are attributes of the given type (id) on the given var (id) 
// that have data that is numeric and below the given (serialized) value, and which are active or match the given transaction id
// e.g., for run X, list all timesteps where there is a max on temperature below 1000
int metadata_catalog_all_timesteps_with_var_attributes_with_type_var_below_min (uint64_t run_id             
                                        ,uint64_t type_id
                                        ,uint64_t var_id             
                                        ,uint64_t txn_id
                                        ,attr_data_type data_type
                                        ,const std::string &data                                        
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_timestep_entry> &entries
                                        );


// for the given run (id), lists all timesteps where there are attributes of the given type (id) on the given var (id) in the given dimensions
// that have data that is numeric and in the given (serialized) range, and which are active or match the given transaction id
// e.g., for run X, list all timesteps where there is a max on temperature between 100 and 1000 in the upper right quadrant
int metadata_catalog_all_timesteps_with_var_attributes_with_type_var_dims_range (uint64_t run_id             
                                        ,uint64_t type_id
                                        ,uint64_t var_id             
                                        ,uint64_t txn_id
                                        ,uint32_t num_dims
                                        ,const std::vector<md_dim_bounds> &dims
                                        ,attr_data_type data_type
                                        ,const std::string &data
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_timestep_entry> &entries
                                        );

// for the given run (id), lists all timesteps where there are attributes of the given type (id) on the given var (id) in the given dimensions
// that have data that is numeric and above the given (serialized) value, and which are active or match the given transaction id
// e.g., for run X, list all timesteps where there is a max on temperature above 1000 in the upper right quadrant
int metadata_catalog_all_timesteps_with_var_attributes_with_type_var_dims_above_max (uint64_t run_id             
                                        ,uint64_t type_id
                                        ,uint64_t var_id             
                                        ,uint64_t txn_id
                                        ,uint32_t num_dims
                                        ,const std::vector<md_dim_bounds> &dims
                                        ,attr_data_type data_type
                                        ,const std::string &data
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_timestep_entry> &entries
                                        );



// for the given run (id), lists all timesteps where there are attributes of the given type (id) on the given var (id) in the given dimensions
// that have data that is numeric and below the given (serialized) value, and which are active or match the given transaction id
// e.g., for run X, list all timesteps where there is a max on temperature below 1000 in the upper right quadrant
int metadata_catalog_all_timesteps_with_var_attributes_with_type_var_dims_below_min (uint64_t run_id             
                                        ,uint64_t type_id
                                        ,uint64_t var_id             
                                        ,uint64_t txn_id
                                        ,uint32_t num_dims
                                        ,const std::vector<md_dim_bounds> &dims
                                        ,attr_data_type data_type
                                        ,const std::string &data
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_timestep_entry> &entries
                                        );


int metadata_catalog_all_timestep_attributes_with_type_range (uint64_t run_id             
                                        ,uint64_t type_id             
                                        ,uint64_t txn_id
                                        ,attr_data_type data_type
                                        ,const std::string &data
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_timestep_attribute_entry> &entries
                                        );


int metadata_catalog_all_timestep_attributes_with_type_above_max (uint64_t run_id             
                                        ,uint64_t type_id             
                                        ,uint64_t txn_id
                                        ,attr_data_type data_type
                                        ,const std::string &data
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_timestep_attribute_entry> &entries
                                        );


int metadata_catalog_all_timestep_attributes_with_type_below_min (uint64_t run_id             
                                        ,uint64_t type_id             
                                        ,uint64_t txn_id
                                        ,attr_data_type data_type
                                        ,const std::string &data
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_timestep_attribute_entry> &entries
                                        );

// for the given timestep (id) of the given run (id), lists all attributes associated with the timestep of the given type (id) 
// that have data that is numeric and in the given (serialized) range, and which are active or match the given transaction id
// e.g., for the 100th timestep of run X, retrieve all of the 'high turbulence' attributes between values 1000 and 5000
int metadata_catalog_all_timestep_attributes_with_type_range_in_timestep (uint64_t run_id             
                                        ,uint64_t timestep_id
                                        ,uint64_t type_id             
                                        ,uint64_t txn_id
                                        ,attr_data_type data_type
                                        ,const std::string &data
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_timestep_attribute_entry> &entries
                                        );

// for the given timestep (id) of the given run (id), lists all attributes associated with the timestep of the given type (id) 
// that have data that is numeric and above the given (serialized) value, and which are active or match the given transaction id
// e.g., for the 100th timestep of run X, retrieve all of the 'high turbulence' attributes between above value 5000
int metadata_catalog_all_timestep_attributes_with_type_above_max_in_timestep (uint64_t run_id             
                                        ,uint64_t timestep_id
                                        ,uint64_t type_id             
                                        ,uint64_t txn_id
                                        ,attr_data_type data_type
                                        ,const std::string &data
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_timestep_attribute_entry> &entries
                                        );

// for the given timestep (id) of the given run (id), lists all attributes associated with the timestep of the given type (id) 
// that have data that is numeric and below the given (serialized) value, and which are active or match the given transaction id
// e.g., for the 100th timestep of run X, retrieve all of the 'low pressure zone' attributes below value 1000
int metadata_catalog_all_timestep_attributes_with_type_below_min_in_timestep (uint64_t run_id             
                                        ,uint64_t timestep_id
                                        ,uint64_t type_id             
                                        ,uint64_t txn_id
                                        ,attr_data_type data_type
                                        ,const std::string &data
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_timestep_attribute_entry> &entries
                                        );

// for the given timestep (id) of the given run (found using type_id), lists all attributes of the given type (id) and variable (id)
// that have data that is numeric and in the given (serialized) range, and which are active or match the given transaction id
// e.g., for the 100th timestep of run X, list all maximums on temperature between the values 1000 and 5000
int metadata_catalog_all_var_attributes_with_type_var_range (uint64_t timestep_id
                                        ,uint64_t type_id
                                        ,uint64_t var_id
                                        ,uint64_t txn_id
                                        ,attr_data_type data_type
                                        ,const std::string &data
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_var_attribute_entry> &matching_attributes
                                        );

// for the given timestep (id) of the given run (found using type_id), lists all attributes of the given type (id) and variable (id)
// that have data that is numeric and above the given (serialized) value, and which are active or match the given transaction id
// e.g., for the 100th timestep of run X, list all maximums on temperature above 5000
int metadata_catalog_all_var_attributes_with_type_var_above_max (uint64_t timestep_id
                                        ,uint64_t type_id
                                        ,uint64_t var_id
                                        ,uint64_t txn_id
                                        ,attr_data_type data_type
                                        ,const std::string &data
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_var_attribute_entry> &matching_attributes
                                        );

// for the given timestep (id) of the given run (found using type_id), lists all attributes of the given type (id) and variable (id)
// that have data that is numeric and below the given (serialized) value, and which are active or match the given transaction id
// e.g., for the 100th timestep of run X, list all minimums on temperature below 1000
int metadata_catalog_all_var_attributes_with_type_var_below_min (uint64_t timestep_id
                                        ,uint64_t type_id
                                        ,uint64_t var_id
                                        ,uint64_t txn_id
                                        ,attr_data_type data_type
                                        ,const std::string &data
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_var_attribute_entry> &matching_attributes
                                        );

// for the given timestep (id) of the given run (found using type_id), lists all attributes of the given type (id) and variable (id) and dimensions
// that have data that is numeric and in the given (serialized) range, and which are active or match the given transaction id
// e.g., for the 100th timestep of run X, list all maximums on temperature between the values 1000 and 5000 in the upper right quadrant
int metadata_catalog_all_var_attributes_with_type_var_dims_range (uint64_t timestep_id
                                        ,uint64_t type_id
                                        ,uint64_t var_id
                                        ,uint64_t txn_id
                                        ,uint32_t num_dims
                                        ,const std::vector<md_dim_bounds> &dims
                                        ,attr_data_type data_type
                                        ,const std::string &data
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_var_attribute_entry> &matching_attributes
                                        );

// for the given timestep (id) of the given run (found using type_id), lists all attributes of the given type (id) and variable (id) and dimensions
// that have data that is numeric and above the given (serialized) value, and which are active or match the given transaction id
// e.g., for the 100th timestep of run X, list all maximums on temperature above 5000 in the upper right quadrant
int metadata_catalog_all_var_attributes_with_type_var_dims_above_max (uint64_t timestep_id
                                        ,uint64_t type_id
                                        ,uint64_t var_id
                                        ,uint64_t txn_id
                                        ,uint32_t num_dims
                                        ,const std::vector<md_dim_bounds> &dims
                                        ,attr_data_type data_type
                                        ,const std::string &max
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_var_attribute_entry> &matching_attributes
                                        );

// for the given timestep (id) of the given run (found using type_id), lists all attributes of the given type (id) and variable (id) and dimensions
// that have data that is numeric and below the given (serialized) value, and which are active or match the given transaction id
// e.g., for the 100th timestep of run X, list all minimums on temperature below 1000 in the upper right quadrant
int metadata_catalog_all_var_attributes_with_type_var_dims_below_min (uint64_t timestep_id
                                        ,uint64_t type_id
                                        ,uint64_t var_id
                                        ,uint64_t txn_id
                                        ,uint32_t num_dims
                                        ,const std::vector<md_dim_bounds> &dims
                                        ,attr_data_type data_type
                                        ,const std::string &max
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_var_attribute_entry> &matching_attributes
                                        );



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// NEW OPS: VAR SUBSTR QUERIES /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// for the given timestep (id) of the given run (id), lists all attributes associated with a variable whose name contains the given substring
// that are active or match the given transaction id
// e.g., for the 100th timestep of run X, all attributes associated with a variable with "temperature" in its name
int metadata_catalog_all_var_attributes_with_var_substr (uint64_t run_id             
                                        ,uint64_t timestep_id
                                        ,const std::string &var_name_substr
                                        ,uint64_t txn_id
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_var_attribute_entry> &matching_attributes
                                        );

// for the given run (id), lists all timesteps where there are attributes of the given type (id) associated with a variable whose name contains 
// the given substring in the given dimensions, which are active or match the given transaction id
// e.g., for run X, list all timesteps where there is a flag (attribute) on a variable with "temperature" in its name in the upper right quadrant
int metadata_catalog_all_timesteps_with_var_attributes_with_type_var_substr_dims (uint64_t run_id             
                                        ,uint64_t type_id
                                        ,const std::string &var_name_substr             
                                        ,uint64_t txn_id
                                        ,uint32_t num_dims
                                        ,const std::vector<md_dim_bounds> &dims
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_timestep_entry> &entries
                                        );

// for the given run (id), lists all timesteps where there are attributes of the given type (id) associated with a variable whose name contains 
// the given substring that are active or match the given transaction id
// e.g., for run X, list all timesteps where there is a flag (attribute) on a variable with "temperature" in its name 
int metadata_catalog_all_timesteps_with_var_attributes_with_type_var_substr (uint64_t run_id             
                                        ,uint64_t type_id
                                        ,const std::string &var_name_substr             
                                        ,uint64_t txn_id
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_timestep_entry> &entries
                                        );

// for the given run (id), find all timesteps that have a variable whose name contains the given substring that are active or match the given transaction id
// e.g., for run X, find all timesteps with a variable with "temperature" in its name 
int metadata_catalog_all_timesteps_with_var_substr (uint64_t run_id             
                                        ,const std::string &var_name_substr             
                                        ,uint64_t txn_id
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_timestep_entry> &entries
                                        );


int metadata_catalog_all_types_with_var_attributes_with_var_substr_dims (
                       uint64_t run_id
                      ,const std::string &var_name_substr
                      ,uint64_t txn_id
                      ,uint32_t num_dims
                      ,const std::vector<md_dim_bounds> &dims
                      ,uint32_t &count
                      ,std::vector<md_catalog_type_entry> &entries
                     );


// for the given timestep (id) of the given run (id), lists all types that have an instance associated with a variable whose name contains 
// the given substring in the given dimensions, which are active or match the given transaction id
// e.g., for the 100th timestep of run X, list all types that have an instance on a variable with "temperature" in its name in the upper right quadrant
int metadata_catalog_all_types_with_var_attributes_with_var_substr_dims_in_timestep (uint64_t run_id
                      ,uint64_t timestep_id
                      ,const std::string &var_name_substr
                      ,uint64_t txn_id
                      ,uint32_t num_dims
                      ,const std::vector<md_dim_bounds> &dims
                      ,uint32_t &count
                      ,std::vector<md_catalog_type_entry> &entries
                     );


int metadata_catalog_all_types_with_var_attributes_with_var_substr (
                       uint64_t run_id
                      ,const std::string &var_name_substr
                      ,uint64_t txn_id
                      ,uint32_t &count
                      ,std::vector<md_catalog_type_entry> &entries
                     );

// for the given timestep (id) of the given run (id), lists all types that have an instance associated with a variable whose name contains
// the given substring that are active or match the given transaction id
// e.g., for the 100th timestep of run X, list all types that have an instance on a variable with "temperature" in its name
int metadata_catalog_all_types_with_var_attributes_with_var_substr_in_timestep (uint64_t run_id
                      ,uint64_t timestep_id
                      ,const std::string &var_name_substr
                      ,uint64_t txn_id
                      ,uint32_t &count
                      ,std::vector<md_catalog_type_entry> &entries
                     );

// for the given timestep (id) of the given run (id), lists all attributes of the given type (id) associated with a variable whose name contains
// the given substring that are active or match the given transaction id
// e.g., for the 100th timestep of run X, list all flags on a variable with "temperature" in its name
int metadata_catalog_all_var_attributes_with_type_var_substr (uint64_t run_id             
                                        ,uint64_t timestep_id
                                        ,uint64_t type_id
                                        ,const std::string &var_name_substr
                                        ,uint64_t txn_id
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_var_attribute_entry> &matching_attributes
                                        );


// for the given timestep (id) of the given run (id), lists all attributes of the given type (id), associated with a variable whose name contains
// the given substring that are active or match the given transaction id
// e.g., for the 100th timestep of run X, list all flag on a variable with "temperature" in its name in the upper right quadrant
int metadata_catalog_all_var_attributes_with_type_var_substr_dims (uint64_t run_id             
                                        ,uint64_t timestep_id
                                        ,uint64_t type_id
                                        ,const std::string &var_name_substr
                                        ,uint64_t txn_id
                                        ,uint32_t num_dims
                                        ,const std::vector<md_dim_bounds> &dims
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_var_attribute_entry> &matching_attributes
                                        );

// for the given timestep (id) of the given run (id), lists all attributes associated with a variable whose name contains
// the given substring in the given dimensions, which are active or match the given transaction id
// e.g., for the 100th timestep of run X, list all attributes on a variable with "temperature" in its name in the upper right quadrant
int metadata_catalog_all_var_attributes_with_var_substr_dims (uint64_t run_id             
                                        ,uint64_t timestep_id
                                        ,const std::string &var_name_substr                       
                                        ,uint64_t txn_id
                                        ,uint32_t num_dims
                                        ,const std::vector<md_dim_bounds> &dims
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_var_attribute_entry> &matching_attributes
                                        );

// for the given timestep (id) of the given run (id), delete all vars whose name contains the given substring, and delete all of the var attributes associated with them
// e.g., for run X, delete all variable with "temperature" in its name
int metadata_delete_all_vars_with_substr (uint64_t run_id
                        ,uint64_t timestep_id
                        ,const std::string &var_name_substr
                        );


// for the given run (id), lists all timesteps where there are attributes of the given type (id) on a variable whose name contains 
// the given substring that have data that is numeric and in the given (serialized) range, and which are active or match the given transaction id
// e.g., for run X, list all timesteps where there is a max on a variable with "temperature" in its name between 100 and 1000
int metadata_catalog_all_timesteps_with_var_attributes_with_type_var_substr_range (uint64_t run_id             
                                        ,uint64_t type_id
                                        ,const std::string &var_name_substr             
                                        ,uint64_t txn_id
                                        ,attr_data_type data_type
                                        ,const std::string &data                                        
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_timestep_entry> &entries
                                        );

// for the given run (id), lists all timesteps where there are attributes of the given type (id) on a variable whose name contains
// the given substring that have data that is numeric and above the given (serialized) value, and which are active or match the given transaction id
// e.g., for run X, list all timesteps where there is a max on a variable with "temperature" in its name above 1000
int metadata_catalog_all_timesteps_with_var_attributes_with_type_var_substr_above_max (uint64_t run_id             
                                        ,uint64_t type_id
                                        ,const std::string &var_name_substr             
                                        ,uint64_t txn_id
                                        ,attr_data_type data_type
                                        ,const std::string &data                                        
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_timestep_entry> &entries
                                        );

// for the given run (id), lists all timesteps where there are attributes of the given type (id) on a variable whose name contains
// the given substring that have data that is numeric and below the given (serialized) value, 
// and which are active or match the given transaction id
// e.g., for run X, list all timesteps where there is a max on a variable with "temperature" in its name below 1000
int metadata_catalog_all_timesteps_with_var_attributes_with_type_var_substr_below_min (uint64_t run_id             
                                        ,uint64_t type_id
                                        ,const std::string &var_name_substr             
                                        ,uint64_t txn_id
                                        ,attr_data_type data_type
                                        ,const std::string &data                                        
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_timestep_entry> &entries
                                        );

// for the given run (id), lists all timesteps where there are attributes of the given type (id) on a variable whose name contains
// the given substring in the given dimensions that have data that is numeric and in the given (serialized) range, 
// and which are active or match the given transaction id
// e.g., for run X, list all timesteps where there is a max on a variable with "temperature" in its name between 100 and 1000 in the upper right quadrant
int metadata_catalog_all_timesteps_with_var_attributes_with_type_var_substr_dims_range (uint64_t run_id             
                                        ,uint64_t type_id
                                        ,const std::string &var_name_substr             
                                        ,uint64_t txn_id
                                        ,uint32_t num_dims
                                        ,const std::vector<md_dim_bounds> &dims
                                        ,attr_data_type data_type
                                        ,const std::string &data
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_timestep_entry> &entries
                                        );

// for the given run (id), lists all timesteps where there are attributes of the given type (id) on a variable whose name contains
// the given substring in the given dimensions that have data that is numeric and above the given (serialized) value, 
// and which are active or match the given transaction id
// e.g., for run X, list all timesteps where there is a max on a variable with "temperature" in its name above 1000 in the upper right quadrant
int metadata_catalog_all_timesteps_with_var_attributes_with_type_var_substr_dims_above_max (uint64_t run_id             
                                        ,uint64_t type_id
                                        ,const std::string &var_name_substr             
                                        ,uint64_t txn_id
                                        ,uint32_t num_dims
                                        ,const std::vector<md_dim_bounds> &dims
                                        ,attr_data_type data_type
                                        ,const std::string &data
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_timestep_entry> &entries
                                        );

// for the given run (id), lists all timesteps where there are attributes of the given type (id) on a variable whose name contains
// the given substring in the given dimensions that have data that is numeric and below the given (serialized) value, 
// and which are active or match the given transaction id
// e.g., for run X, list all timesteps where there is a max on a variable with "temperature" in its name below 1000 in the upper right quadrant
int metadata_catalog_all_timesteps_with_var_attributes_with_type_var_substr_dims_below_min (uint64_t run_id             
                                        ,uint64_t type_id
                                        ,const std::string &var_name_substr             
                                        ,uint64_t txn_id
                                        ,uint32_t num_dims
                                        ,const std::vector<md_dim_bounds> &dims
                                        ,attr_data_type data_type
                                        ,const std::string &data
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_timestep_entry> &entries
                                        );


// for the given timestep (id) of the given run (found using type_id), lists all attributes of the given type (id) on a variable whose name contains
// the given substring
// that have data that is numeric and in the given (serialized) range, and which are active or match the given transaction id
// e.g., for the 100th timestep of run X, list all maximums on a variable with "temperature" in its name between the values 1000 and 5000
int metadata_catalog_all_var_attributes_with_type_var_substr_range (uint64_t timestep_id
                                        ,uint64_t type_id
                                        ,const std::string &var_name_substr 
                                        ,uint64_t txn_id
                                        ,attr_data_type data_type
                                        ,const std::string &data
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_var_attribute_entry> &matching_attributes
                                        );

// for the given timestep (id) of the given run (found using type_id), lists all attributes of the given type (id) on a variable whose name contains
// the given substring
// that have data that is numeric and above the given (serialized) value, and which are active or match the given transaction id
// e.g., for the 100th timestep of run X, list all maximums on a variable with "temperature" in its name above 5000
int metadata_catalog_all_var_attributes_with_type_var_substr_above_max (uint64_t timestep_id
                                        ,uint64_t type_id
                                        ,const std::string &var_name_substr 
                                        ,uint64_t txn_id
                                        ,attr_data_type data_type
                                        ,const std::string &data
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_var_attribute_entry> &matching_attributes
                                        );

// for the given timestep (id) of the given run (found using type_id), lists all attributes of the given type (id) on a variable whose name contains
// the given substring
// that have data that is numeric and below the given (serialized) value, and which are active or match the given transaction id
// e.g., for the 100th timestep of run X, list all minimums on a variable with "temperature" in its name below 1000
int metadata_catalog_all_var_attributes_with_type_var_substr_below_min (uint64_t timestep_id
                                        ,uint64_t type_id
                                        ,const std::string &var_name_substr 
                                        ,uint64_t txn_id
                                        ,attr_data_type data_type
                                        ,const std::string &data
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_var_attribute_entry> &matching_attributes
                                        );

// for the given timestep (id) of the given run (found using type_id), lists all attributes of the given type (id) on a variable whose name contains
// the given substring and which are in the given dimensions
// that have data that is numeric and in the given (serialized) range, and which are active or match the given transaction id
// e.g., for the 100th timestep of run X, list all maximums on a variable with "temperature" in its name between the values 1000 and 5000 in the upper right quadrant
int metadata_catalog_all_var_attributes_with_type_var_substr_dims_range (uint64_t timestep_id
                                        ,uint64_t type_id
                                        ,const std::string &var_name_substr 
                                        ,uint64_t txn_id
                                        ,uint32_t num_dims
                                        ,const std::vector<md_dim_bounds> &dims
                                        ,attr_data_type data_type
                                        ,const std::string &data
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_var_attribute_entry> &matching_attributes
                                        );

// for the given timestep (id) of the given run (found using type_id), lists all attributes of the given type (id) on a variable whose name contains
// the given substring and which are in the given dimensions
// that have data that is numeric and above the given (serialized) value, and which are active or match the given transaction id
// e.g., for the 100th timestep of run X, list all maximums on a variable with "temperature" in its name above 5000 in the upper right quadrant
int metadata_catalog_all_var_attributes_with_type_var_substr_dims_above_max (uint64_t timestep_id
                                        ,uint64_t type_id
                                        ,const std::string &var_name_substr 
                                        ,uint64_t txn_id
                                        ,uint32_t num_dims
                                        ,const std::vector<md_dim_bounds> &dims
                                        ,attr_data_type data_type
                                        ,const std::string &data
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_var_attribute_entry> &matching_attributes
                                        );

// for the given timestep (id) of the given run (found using type_id), lists all attributes of the given type (id) on a variable whose name contains
// the given substring and which are in the given dimensions
// that have data that is numeric and below the given (serialized) value, and which are active or match the given transaction id
// e.g., for the 100th timestep of run X, list all minimums on a variable with "temperature" in its name below 1000 in the upper right quadrant
int metadata_catalog_all_var_attributes_with_type_var_substr_dims_below_min (uint64_t timestep_id
                                        ,uint64_t type_id
                                        ,const std::string &var_name_substr 
                                        ,uint64_t txn_id
                                        ,uint32_t num_dims
                                        ,const std::vector<md_dim_bounds> &dims
                                        ,attr_data_type data_type
                                        ,const std::string &data
                                        ,uint32_t &count
                                        ,std::vector<md_catalog_var_attribute_entry> &matching_attributes
                                        );



// int metadata_insert_var_attribute_by_dims_batch (uint64_t &attribute_id
//                            ,uint64_t timestep_id
//                            ,uint64_t type_id
//                            ,uint64_t var_id
//                            ,uint64_t txn_id
//                            ,uint32_t num_dims
//                            ,const std::vector<md_dim_bounds> &dims
//                            ,attr_data_type data_type
//                            ,const std::string &data
//                            )

int metadata_insert_var_attribute_by_dims_batch (const std::vector<md_catalog_var_attribute_entry> &new_attributes
                           );

// int metadata_create_type_batch (uint64_t &type_id
//                         ,uint64_t run_id
//                         ,const std::string &name
//                         ,uint32_t version
//                         ,uint64_t txn_id
//                         )

int metadata_create_type_batch (uint64_t &type_id
                        ,const std::vector<md_catalog_type_entry> &new_types
                        );

int metadata_create_type_batch (const std::vector<md_catalog_type_entry> &new_types
                        );
// int metadata_create_var_batch (uint64_t &var_id
//                         ,uint64_t run_id
//                         ,uint64_t timestep_id
//                         ,const std::string &name
//                         ,const std::string &path
//                         ,uint32_t version
//                         ,char data_size
//                         ,uint64_t txn_id
//                         ,uint32_t num_dims
//                         ,const std::vector<md_dim_bounds> &dims
//                         )
int metadata_create_var_batch (const std::vector<md_catalog_var_entry> &new_vars
                        );

// int metadata_insert_run_attribute_batch (uint64_t &attribute_id
//                            ,uint64_t type_id
//                            ,uint64_t txn_id
//                            ,attr_data_type data_type
//                            ,const std::string &data
//                            )

int metadata_insert_run_attribute_batch (const std::vector<md_catalog_run_attribute_entry> &new_attributes
                           );

// int metadata_insert_timestep_attribute_batch (uint64_t &attribute_id
//                            ,uint64_t timestep_id
//                            ,uint64_t type_id
//                            ,uint64_t txn_id
//                            ,attr_data_type data_type
//                            ,const std::string &data
//                            )

int metadata_insert_timestep_attribute_batch (const std::vector<md_catalog_timestep_attribute_entry> &new_attributes
                           );

#endif
