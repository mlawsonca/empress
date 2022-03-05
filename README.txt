#################################################################
####### test code ###############################################
#################################################################
the debugging_source folders have debugging code. 

debugging_source has the code needed to test the entire EMPRESS library, including the various transaction modes and service modes

debugging_source_hdf5 has the code needed to create the HDF5 comparison test system that provides the same functionality as EMPRESS (this system was used for testing for the PDSW18 workshop paper: "Using a Robust Metadata Management System to Accelerate Scientific Discovery at Extreme Scales"and the TOS'22 journal paper: "EMPRESS: Accelerating Scientific Discovery Through Descriptive Metadata Management")

#################################################################
###### headers ##################################################
#################################################################

include - contains all of the headers needed for the provided code (library, debugging code, testing harnesses)

#################################################################
###### library source ###########################################
#################################################################

lib_source - contains all of the code needed to create the ops (the code used by EMPRESS for performing operations on the metadata)

my_md_source - contains all of the rest of the library code. contains the various version of the EMPRESS client and server

#################################################################
###### evaluation code ##########################################
#################################################################

testing_source - includes the EMPRESS evaluation code used for the PDSW18 workshop paper: "Using a Robust Metadata Management System to Accelerate Scientific Discovery at Extreme Scales" (testing harness and analysis code)

testing_source_hdf5 - includes the evaluation code for the HDF5 comparison system used in the PDSW18 workshop paper: "Using a Robust Metadata Management System to Accelerate Scientific Discovery at Extreme Scales" and the TOS'22 journal paper: "EMPRESS: Accelerating Scientific Discovery Through Descriptive Metadata Management" (testing harness and analysis code)

testing_source_class_proj - includes the EMPRESS evaluation code used for the TOS'22 journal paper: "EMPRESS: Accelerating Scientific Discovery Through Descriptive Metadata Management" (testing harness and analysis code)

#################################################################
###### objector evaluation ######################################
#################################################################

sqlite_comparison - all of the code needed for evaluating the objector


#################################################################
######## acknowledgements #######################################
#################################################################

sqlite3.{hc} are generated from the public domain SQLite project.