test code

#################################################################
####### EMPRESS debugging code ##################################
#################################################################

my_test_client.C
helper_functions_test_client.C
helper_functions_2_test_client.cpp
helper_functions_3_test_client.cpp


#################################################################
####### EMPRESS local mode debugging code #######################
#################################################################

my_test_client_local.cpp
helper_functions_test_client_local.C
helper_functions_2_test_client_local.cpp
helper_functions_3_test_client_local.cpp


#################################################################
####### EMPRESS transaction modes code  #########################
#################################################################

for EMPRESS with the "limited" transaction mode (doesn't use txn_ids or active indicators, all transaction management handled by SQLite, designed to have minimal overheads)

my_test_client_sqlite_transactions_only_db_streams.cpp
helper_functions_test_client_sqlite_transactions_only_db_streams.cpp
helper_functions_2_test_client_sqlite_transactions_only_db_streams.cpp

#################################################################

for EMPRESS with the transaction mode that uses a separate DB per client job for ongoing transactions

my_test_client_sqlite_transactions_only_new_new.cpp
helper_functions_test_client_sqlite_transactions_only_new_new.cpp
helper_functions_2_test_client_sqlite_transactions_only_new_new.cpp
helper_functions_3_test_client_sqlite_transactions_only_new.cpp

#################################################################
#######  misc ###################################################
#################################################################

my_gathered_write_test_client.cpp - tests message bundling

my_lua_test_client.C - tests the lua code functionality (used by the Objector)

my_test_client_id_ops.C - tests the id version of the ops (rather than the name and version ones)

#################################################################
####### async ###################################################
#################################################################

includes the debugging code for the asynchronous version of the EMPRESS library









