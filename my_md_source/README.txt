user-facing EMPRESS code

#################################################################
####### EMPRESS client code #####################################
#################################################################

my_metadata_client_async.cpp - asynchronous versions of EMPRESS functions

my_metadata_client.C - for standard EMPRESS

my_metadata_client_denormalized.cpp - for EMPRESS's d2t transaction managment mode with SQLite transaction overheads minimized

my_metadata_client_local.cpp - for EMPRESS's local service mode

my_metadata_client_local_sqlite_transactions_only.cpp - for EMPRESS's local service mode with the transaction mode that uses a separate DB per client job for ongoing transactions

my_metadata_client_lua_functs.cpp - code for managing the objector

my_metadata_client_sqlite_transactions_only_multiple_databases_by_job_id_denormalized.cpp - for EMPRESS with the "limited" transaction mode (doesn't use txn_ids or active indicators, all transaction management handled by SQLite, designed to have minimal overheads) 

my_metadata_client_sqlite_transactions_only_multiple_databases_read_and_write_new.cpp - for EMPRESS with the transaction mode that uses a separate DB per client job for ongoing transactions

#################################################################
####### EMPRESS server code #####################################
#################################################################

my_metadata_server.C - standard EMPRESS

my_metadata_server_local.cpp - for EMPRESS's local service mode

my_metadata_server_sqlite_transactions_only_multiple_transactions_single_database_new.cpp - for EMPRESS with the transaction mode that uses a separate DB per client job for ongoing transactions

my_metadata_server_sqlite_transactions_only_single_database_connection.cpp - for EMPRESS with the "limited" transaction mode (doesn't use txn_ids or active indicators, all transaction management handled by SQLite, designed to have minimal overheads) 

#################################################################
####### EMPRESS directory manager code ##########################
#################################################################

my_dirman.C - Provides discoverability of the EMPRESS service (allowing EMPRESS servers to be dynamically allocated)

