code used for ops (EMPRESS operations on the stored metadata, launched by calling EMPRESS client functions)

#################################################################
####### EMPRESS client code #####################################
#################################################################

client - standard EMPRESS library

client_sqlite_transactions_only_any_database_checkpts_denormalized - for EMPRESS with the "limited" transaction mode (doesn't use txn_ids or active indicators, all transaction management handled by SQLite, designed to have minimal overheads) 

#################################################################
####### EMPRESS server code #####################################
#################################################################

server - standard EMPRESS library

server_d2t_transactions_only_denormalized - for EMPRESS's d2t transaction managment mode with SQLite transaction overheads minimized

server_separate_rtree - for the version of EMPRESS that uses an R-tree

server_sqlite_transactions_only_any_database_checkpts_denormalized - for EMPRESS with the "limited" transaction mode (doesn't use txn_ids or active indicators, all transaction management handled by SQLite, designed to have minimal overheads) 

server_sqlite_transactions_only_multiple_databases_write_ahead_log_denormalized - for EMPRESS with the transaction mode that uses a separate DB per client job for ongoing transactions

#################################################################
####### code used by both client and server #####################
#################################################################

common
