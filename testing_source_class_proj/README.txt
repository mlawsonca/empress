code for the 2021 journal paper evaluation

#################################################################
####### analysis code ###########################################
#################################################################

analysis_code_parallel_new - the analysis code (uses OpenMP to analyze the output in parallel). can be done in serial by setting the number of threads to 1. statistics are calculated in a streaming fashion

#################################################################
####### job scripts #############################################
#################################################################

make_txn - code for making the slurm job scripts for testing 

#################################################################
####### testing harness for EMPRESS #############################
#################################################################

testing_harness_write_class_proj.cpp - write portion of testing
testing_harness_read_class_proj_new.cpp - read portion of testing
3d_read_for_testing_class_proj.cpp - portion of read testing based on the 6 Degrees papers (2D and 3D spatial queries)
extra_testing_class_proj.cpp - additional portion of read testing

#################################################################
####### testing harness for hdf5 comparison system ##############
#################################################################

testing_harness_write_class_proj_hdf5.cpp - write portion of testing
testing_harness_read_class_proj_hdf5.cpp - read portion of testing
3d_read_for_testing_class_proj_hdf5.cpp - portion of read testing based on the 6 Degrees papers (2D and 3D spatial queries)
extra_testing_class_proj_hdf5.cpp - additional portion of read testing
testing_harness_helper_functions_class_proj_hdf5.cpp - additional code to help with debugging the testing harness

#################################################################
####### testing harness for EMPRESS's local service mode ########
#################################################################

testing_harness_write_class_proj_local.cpp - write portion of testing
testing_harness_read_class_proj_local.cpp - read portion of testing
3d_read_for_testing_class_proj_local.cpp - portion of read testing based on the 6 Degrees papers (2D and 3D spatial queries)
extra_testing_class_proj_local.cpp - additional portion of read testing

#################################################################
####### testing harness for EMPRESS with large md objects #######
#################################################################

testing_harness_write_class_proj_large_md_volume.cpp - write portion of testing
testing_harness_read_class_proj_large_md_volume.cpp - read portion of testing

#################################################################
####### testing harness for EMPRESS's transaction modes #########
#################################################################


for EMPRESS with the transaction mode that uses a separate DB per client job for ongoing transactions

testing_harness_write_class_proj_sqlite_transactions_only.cpp
3d_read_for_testing_class_proj_sqlite_transactions_only.cpp

#################################################################


for EMPRESS with the "limited" transaction mode (doesn't use txn_ids or active indicators, all transaction management handled by SQLite, designed to have minimal overheads) 

testing_harness_write_class_proj_sqlite_transactions_only_db_streams.cpp

#################################################################
####### misc ####################################################
#################################################################

testing_harness_debug_helper_functions.cpp - additional code to help with debugging the testing harness

testing_harness_write_class_proj_local_pregenerate.cpp - version of write portion of local testing harness where metadata objects are created prior to starting the timed portion of the testing harness. not used in the journal paper / run during the evaluation

testing_harness_write_class_proj_pregenerate.cpp - version of write portion of testing harness where metadata objects are created prior to starting the timed portion of the testing harness. not used in the journal paper / run during the evaluation

#################################################################
####### debug #########################################
#################################################################

db_check.cpp - used to test the data that is persisted in the DB

db_delete_debug.cpp - tests deleting the db to confirm it works