#################################################################
####### PDSW18 evaluation code ##################################
#################################################################

analysis_code - used to analyze timing output and other information collected from the testing harness

3d_read_for_testing.cpp - evaluation based on the 6 Degrees papers (2D and 3D spatial queries)

extra_testing_collective.cpp - extra testing portion of the harness 

extra_testing_collective_helper_functions.cpp - helper code for the extra testing portion of the harness

hdf5_helper_functions_read.cpp - helper code for reading data associated with metadata using HDF5

hdf5_helper_functions_write.cpp - helper code for writing data associated with metadata using HDF5

make_txn_files.cpp - code for making the slurm job scripts for testing 

testing_harness_debug_helper_functions.cpp - helper code for debugging the testing harness

testing_harness_new_read.cpp - the reading portion of the testing harness

testing_harness_new_write.cpp - the writing portion of the testing harness