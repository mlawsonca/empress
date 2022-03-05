Headers

#################################################################
####### Common ##################################################
#################################################################

Contains the data structures used by the EMPRESS library for writing metadata, reading metadata, and other ops

common - used by the basic EMPRESS library

common_denormalized - slightly more denormalized version of storing the metadata in a database (run id included for timestep attributes and var attributes) 

common_sqlite_transactions_only_db_streams - for EMPRESS's "limited" transaction mode (doesn't use txn_ids or active indicators, all transaction management handled by SQLite, designed to have minimal overheads) 

common_sqlite_transactions_only_multiple_databases_read_and_write_new - for EMPRESS's transaction mode that uses a separate database per client job for in-progress transactions


#################################################################
####### Ops #####################################################
#################################################################

ops_rdma - the headers for EMPRESS's ops (the code that manages the metadata storage operations for each EMPRESS client function)


ops_rdma_sqlite_transactions_only - the headers for EMPRESS's ops if the limited functionality mode (which uses only SQLite for transaction managmeent) is used (the code that manages the metadata storage operations for each EMPRESS client function). The code is different because EMPRESS does not store transaction mangement info (txn_id, active)


#################################################################
####### Library #################################################
#################################################################

client - includes the headers for the metadata client for the core library and for the PDSW18 workshop paper: "Using a Robust Metadata Management System to Accelerate Scientific Discovery at Extreme Scales" testing harness

server - includes the headers for the metadata server and for the core library and for the PDSW18 workshop paper: "Using a Robust Metadata Management System to Accelerate Scientific Discovery at Extreme Scales" testing harness

local - includes the headers for local service mode of the core library and for the TOS'22 journal paper: "EMPRESS: Accelerating Scientific Discovery Through Descriptive Metadata Management" testing harness

database - includes the headers for SQLite / the metadata storage implementation. Is external - sqlite3.{hc} are generated from the public domain SQLite project.

#################################################################
####### Library variants ########################################
#################################################################

hdf5 - includes headers for the HDF5-based comparison system that provides EMPRESS's functionality (used for the PDSW18 workshop paper: "Using a Robust Metadata Management System to Accelerate Scientific Discovery at Extreme Scales" and TOS'22 journal paper: "EMPRESS: Accelerating Scientific Discovery Through Descriptive Metadata Management")

#################################################################
####### Evaluation ##############################################
#################################################################

class_proj - includes the headers for the testing harnesses/evaluation code used for the TOS'22 journal paper: "EMPRESS: Accelerating Scientific Discovery Through Descriptive Metadata Management"

objector_comparison - includes the headers for the objector evaluation used for the PDSW18 workshop paper: "Using a Robust Metadata Management System to Accelerate Scientific Discovery at Extreme Scales" paper




