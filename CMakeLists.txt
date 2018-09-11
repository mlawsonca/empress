#[[ 
 * Copyright 2018 National Technology & Engineering Solutions of
 * Sandia, LLC (NTESS). Under the terms of Contract DE-NA0003525 with NTESS,
 * the U.S. Government retains certain rights in this software.
 *
 * The MIT License (MIT)
 * 
 * Copyright (c) 2018 Sandia Corporation
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *]]

cmake_minimum_required(VERSION 3.2)
project( sirius
  LANGUAGES CXX C
  VERSION 0.1.0.0 )

#
# Require C++11 from whatever compiler is in use
#
#set( CMAKE_CXX_COMPILER /opt/openmpi-1.10-gnu/bin/mpiCC )

set( CMAKE_CXX_STANDARD 11 )
set( CMAKE_CXX_STANDARD_REQUIRED YES )
set( CMAKE_CXX_EXTENSIONS OFF )

cmake_host_system_information(RESULT HOSTNAME QUERY HOSTNAME)

if( "${HOSTNAME}" STREQUAL "s1018120" )
	message( STATUS "hostname: ${HOSTNAME}")
	set(BOOST_ROOT "/usr/local/Cellar/boost/1.65.1")
	#set(CMAKE_CXX_COMPILER /usr/local/Cellar/gcc@6/6.4.0_1 )
	#set(Opbox_CXX_COMPILER /usr/local/Cellar/gcc@6/6.4.0_1 )
	# set(Opbox_CXX_COMPILER /usr/local/Cellar/gcc@4.9/4.9.4 )
	# set(CMAKE_CXX_COMPILER /usr/local/Cellar/gcc@4.9/4.9.4 )
else ("${HOSTNAME}" STREQUAL "s1018120")
	set(Opbox_CXX_COMPILER /usr/bin/c++ )
endif()


if(NOT CMAKE_CXX_FLAGS)
   set(CMAKE_CXX_FLAGS "-D_PTHREADS  -pthread -DBOOST_LOG_DYN_LINK -L${PROJECT_SOURCE_DIR}/lib/database/sqlite3 -std=c++11 -g -w")

endif()

option( USE_MPI "build with MPI for parallelism." ON )

#option(BUILD_TESTING       "DataWarehouse testing gtest and ctest" ON)
#if(BUILD_TESTING)
    #enable_testing()
    #add_subdirectory(tests)
#endif()

if( "${DATA_WAREHOUSE_SOURCE_DIR}" STREQUAL "" )
	set(DATA_WAREHOUSE_SOURCE_DIR $ENV{HOME}/data-warehouse-release)
endif()
if( "${DATA_WAREHOUSE_INSTALL_DIR}" STREQUAL "" )
  set(DATA_WAREHOUSE_INSTALL_DIR ${DATA_WAREHOUSE_SOURCE_DIR}/install )
endif()
if( "${DATA_WAREHOUSE_TPL_INSTALL_DIR}" STREQUAL "" )
  set(DATA_WAREHOUSE_TPL_INSTALL_DIR ${DATA_WAREHOUSE_SOURCE_DIR}/tpl/install )
endif()


if( "${BOOST_ROOT}" STREQUAL "" )
  set( BOOST_ROOT ${DATA_WAREHOUSE_TPL_INSTALL_DIR} )
endif()
if( "${GTEST_ROOT}" STREQUAL "" )
  set( GTEST_ROOT ${DATA_WAREHOUSE_TPL_INSTALL_DIR} )
endif()

if( "${NETWORK_LIBRARY}" STREQUAL "" )
  #set(NETWORK_LIBRARY "libfabric" CACHE STRING "Initial cache" FORCE)
  set(NETWORK_LIBRARY "nnti" CACHE STRING "Initial cache" FORCE)
endif()


set(OPBOX_PROJ_SOURCE_DIR ${DATA_WAREHOUSE_SOURCE_DIR}/src/opbox)

#set(CMAKE_PREFIX_PATH ${DATA_WAREHOUSE_TPL_INSTALL_DIR}/lib/cmake/libfabric)
#set(GTEST_DIR ${DATA_WAREHOUSE_SOURCE_DIR}/install/lib/cmake/opbox)

set(Opbox_DIR ${DATA_WAREHOUSE_INSTALL_DIR}/lib/cmake/opbox)
set(Graith_DIR ${DATA_WAREHOUSE_INSTALL_DIR}/lib/cmake/graith)
set(SBL_DIR ${DATA_WAREHOUSE_INSTALL_DIR}/lib/cmake/sbl)
set(Webhook_DIR ${DATA_WAREHOUSE_INSTALL_DIR}/lib/cmake/webhook)
set(Lunasa_DIR ${DATA_WAREHOUSE_INSTALL_DIR}/lib/cmake/lunasa)
set(NNTI_DIR ${DATA_WAREHOUSE_INSTALL_DIR}/lib/cmake/nnti)
set(Gutties_DIR ${DATA_WAREHOUSE_INSTALL_DIR}/lib/cmake/gutties)

set( CMAKE_MODULE_PATH ${OPBOX_PROJ_SOURCE_DIR}/../../cmake/modules ${CMAKE_MODULE_PATH}  )


set(Opbox_INCLUDE_DIRS ${DATA_WAREHOUSE_INSTALL_DIR}/include )
include( dw_cmake_base )
include( dw_threading )

#include( dw_tpl_targets )  # set flags for MPI

#find_package(IBVerbs)

dw_find_datawarehouse( COMPONENTS SBL Lunasa Webhook Gutties NNTI Opbox )

#find_package( Boost
  #COMPONENTS program_options
  #REQUIRED )

find_package(Lua "5.1" EXACT REQUIRED)


#set (HDF5_ROOT /gscratch/mlawso/hdf5-1.10.1/build)
#set(HDF5_DIR /gscratch/mlawso/hdf5-1.10.1/build)
#set (HDF5_ROOT /gscratch/mlawso)
#set(HDF5_DIR /gscratch/mlawso)

#message( STATUS "HDF5_ROOT   ${HDF5_ROOT}"   )
#message( STATUS "HDF5_DIR   ${HDF5_DIR}"   )

# set (MAKE_EMPRESS FALSE)
# set (MAKE_EMPRESS_TEST_CLIENT FALSE)

# set (MAKE_HDF5_HARNESS FALSE)
# set (MAKE_HDF5_TEST_CLIENT FALSE)

if( "${MAKE_LOCAL}" STREQUAL "" )
	set ( MAKE_LOCAL TRUE )
endif()

if( "${MAKE_EMPRESS}" STREQUAL "" )
	set ( MAKE_EMPRESS TRUE )
endif()

if( "${MAKE_EMPRESS_WITH_HDF5}" STREQUAL "" )
	set ( MAKE_EMPRESS_WITH_HDF5 TRUE )
endif()

if( "${MAKE_EMPRESS_TEST_CLIENT}" STREQUAL "" )
	set ( MAKE_EMPRESS_TEST_CLIENT TRUE )
endif()

if( "${MAKE_HDF5_HARNESS}" STREQUAL "" )
	set ( MAKE_HDF5_HARNESS TRUE )
endif()

if( "${MAKE_HDF5_TEST_CLIENT}" STREQUAL "" )
	set ( MAKE_HDF5_TEST_CLIENT TRUE )
endif()

# if( "${MAKE_OBJECTOR_COMPARISON}" STREQUAL "" )
# 	set ( MAKE_OBJECTOR_COMPARISON TRUE )
# endif()


if ( MAKE_HDF5_HARNESS OR MAKE_EMPRESS_WITH_HDF5 )
	set (HDF5_INCLUDE_DIRS 
		/gscratch/mlawso/hdf5-1.10.1/build/hdf5/include
		${PROJECT_SOURCE_DIR}/include/hdf5
	)
	set (HDF5_C_LIBRARIES 
		/gscratch/mlawso/hdf5-1.10.1/build/hdf5/lib/libhdf5.so
		/gscratch/mlawso/hdf5-1.10.1/build/hdf5/lib/libhdf5_hl.so
	)
endif()

if ( MAKE_EMPRESS )
	set (EMPRESS_INCLUDE_DIRS 
		${LUA_INCLUDE_DIR}
	  	${PROJECT_SOURCE_DIR}/include/client	
	  	${PROJECT_SOURCE_DIR}/include/common
	  	${PROJECT_SOURCE_DIR}/include/ops
	  	${Opbox_INCLUDE_DIRS}
		support
		${OPBOX_PROJ_SOURCE_DIR}/examples/support
		${TPL_INCLUDE_DIRS}
  	)
endif()

#just go to its dir to make
# if (MAKE_OBJECTOR_COMPARISON)
# 	set (OBJECTOR_COMPARISON_INCLUDE_DIRS
# 		${PROJECT_SOURCE_DIR}/include/objector_comparison
# 	)
# endif()


#find_package(HDF5 REQUIRED)

include_directories(
  ${HDF5_INCLUDE_DIRS}
  ${EMPRESS_INCLUDE_DIRS}
  # ${OBJECTOR_COMPARISON_INCLUDE_DIRS}

  ${MPI_CXX_INCLUDE_PATH}
  ${Boost_INCLUDE_DIRS}
)

# #-- Support Lib----------------------------------------------------------------
# add_library(mpi_support STATIC
#             ${OPBOX_PROJ_SOURCE_DIR}/examples/support/Globals.hh
#             ${OPBOX_PROJ_SOURCE_DIR}/examples/support/Globals.cpp
#             ${OPBOX_PROJ_SOURCE_DIR}/examples/support/SimpleDataStore.hh
#             ${OPBOX_PROJ_SOURCE_DIR}/examples/support/SimpleDataStore.cpp
# )
# set_target_properties(mpi_support PROPERTIES LINKER_LANGUAGE CXX )


if(MAKE_EMPRESS)
	add_subdirectory(lib_source)

	set(COMMON_LIBS
	    opbox #are these still needed?
	    lunasa
	    gutties
	    Boost::program_options
	    Boost::serialization
	    # ${TPL_LIBRARIES}
	    MPI_CXX
	)

	set(CLIENT_LIBS
	    ${LUA_LIBRARIES}

	    OpsCommon
	    OpsClient 
	)

	set(SERVER_LIBS
	    Database
	    OpsCommon
	    OpsServer
	)

	set(DIRMAN_LIBS
	    FullShutdownOpForDirman
	)

	set(LOCAL_LIBS
	    ${LUA_LIBRARIES}
	    Boost::serialization
	    MPI_CXX
	    Database
	    EmpressLocal
	)

    add_executable(my_metadata_server my_md_source/my_metadata_server.C)
    set_target_properties(my_metadata_server PROPERTIES LINKER_LANGUAGE CXX )
    target_link_libraries(my_metadata_server ${COMMON_LIBS} ${SERVER_LIBS})

    add_executable(my_dirman my_md_source/my_dirman.C)
    set_target_properties(my_dirman PROPERTIES LINKER_LANGUAGE CXX )
    target_link_libraries(my_dirman ${COMMON_LIBS} ${DIRMAN_LIBS})

    if(MAKE_EMPRESS_TEST_CLIENT)
	    add_executable(my_test_client my_md_source/my_metadata_client.C debugging_source/my_test_client.C debugging_source/helper_functions_test_client.C debugging_source/helper_functions_2_test_client.cpp debugging_source/helper_functions_3_test_client.cpp my_md_source/my_metadata_client_lua_functs.cpp )
	    set_target_properties(my_test_client PROPERTIES LINKER_LANGUAGE CXX )
	    target_link_libraries(my_test_client ${COMMON_LIBS} ${CLIENT_LIBS})

	endif(MAKE_EMPRESS_TEST_CLIENT)

    if(MAKE_EMPRESS_WITH_HDF5) 
    	add_executable(testing_harness_new_write 
    		my_md_source/my_metadata_client.C 
    		my_md_source/my_metadata_client_lua_functs.cpp  
    		testing_source/testing_harness_new_write.cpp  
    		testing_source/testing_harness_debug_helper_functions.cpp
    		testing_source/hdf5_helper_functions_write.cpp
    	)
    	set_target_properties(testing_harness_new_write PROPERTIES LINKER_LANGUAGE CXX )
      	target_link_libraries(testing_harness_new_write ${COMMON_LIBS} ${CLIENT_LIBS} ${HDF5_LIBRARIES} ${HDF5_C_LIBRARIES})

        add_executable(testing_harness_new_read 
        	my_md_source/my_metadata_client.C 
        	my_md_source/my_metadata_client_lua_functs.cpp 
        	testing_source/testing_harness_new_read.cpp 
        	testing_source/3d_read_for_testing.cpp 
        	testing_source/testing_harness_debug_helper_functions.cpp 
        	testing_source/extra_testing_collective.cpp 
        	testing_source/extra_testing_collective_helper_functions.cpp
         	testing_source/hdf5_helper_functions_read.cpp
        )
    	set_target_properties(testing_harness_new_read PROPERTIES LINKER_LANGUAGE CXX )
    	target_link_libraries(testing_harness_new_read ${COMMON_LIBS} ${CLIENT_LIBS} ${HDF5_LIBRARIES} ${HDF5_C_LIBRARIES})
    else(MAKE_EMPRESS_WITH_HDF5)
  	    add_executable(testing_harness_new_write 
    		my_md_source/my_metadata_client.C 
    		my_md_source/my_metadata_client_lua_functs.cpp  
    		testing_source/testing_harness_new_write.cpp  
    		testing_source/testing_harness_debug_helper_functions.cpp
    	)
    	set_target_properties(testing_harness_new_write PROPERTIES LINKER_LANGUAGE CXX )
      	target_link_libraries(testing_harness_new_write ${COMMON_LIBS} ${CLIENT_LIBS})

        add_executable(testing_harness_new_read my_md_source/my_metadata_client.C my_md_source/my_metadata_client_lua_functs.cpp testing_source/testing_harness_new_read.cpp testing_source/3d_read_for_testing.cpp testing_source/testing_harness_debug_helper_functions.cpp testing_source/extra_testing_collective.cpp testing_source/extra_testing_collective_helper_functions.cpp)
    	set_target_properties(testing_harness_new_read PROPERTIES LINKER_LANGUAGE CXX )
    	target_link_libraries(testing_harness_new_read ${COMMON_LIBS} ${CLIENT_LIBS})
    endif(MAKE_EMPRESS_WITH_HDF5)


    if(MAKE_LOCAL)
	    if(MAKE_EMPRESS_WITH_HDF5) 
	    	add_executable(testing_harness_write_local 
	    		my_md_source/my_metadata_client_local.cpp 
	    		my_md_source/my_metadata_client_lua_functs.cpp  
	    		my_md_source/my_metadata_server_local.cpp
	    		testing_source_local/testing_harness_new_new_write_local.cpp  
	    		testing_source/testing_harness_debug_helper_functions.cpp 
	           	testing_source/hdf5_helper_functions_write.cpp
	  		)
		    set_target_properties(testing_harness_write_local PROPERTIES LINKER_LANGUAGE CXX )
		    target_link_libraries(testing_harness_write_local ${LOCAL_LIBS} ${HDF5_LIBRARIES} ${HDF5_C_LIBRARIES})

		    add_executable(testing_harness_read_local 
		    	my_md_source/my_metadata_client_local.cpp 
		    	my_md_source/my_metadata_client_lua_functs.cpp 
		    	my_md_source/my_metadata_server_local.cpp
		    	testing_source_local/testing_harness_read_local.cpp 
		    	testing_source_local/3d_read_for_testing_local_new.cpp 
		    	testing_source_local/extra_testing_collective_local.cpp 
		    	testing_source_local/extra_testing_collective_helper_functions_local_new.cpp
			    testing_source/testing_harness_debug_helper_functions.cpp 
	           	testing_source/hdf5_helper_functions_read.cpp
		    )
		    set_target_properties(testing_harness_read_local PROPERTIES LINKER_LANGUAGE CXX )
		    target_link_libraries(testing_harness_read_local ${LOCAL_LIBS} ${HDF5_LIBRARIES} ${HDF5_C_LIBRARIES})
		elseif(MAKE_EMPRESS_WITH_HDF5)
				add_executable(testing_harness_write_local 
	    		my_md_source/my_metadata_client_local.cpp 
	    		my_md_source/my_metadata_client_lua_functs.cpp  
	    		my_md_source/my_metadata_server_local.cpp
	    		testing_source_local/testing_harness_new_new_write_local.cpp  
	    		testing_source/testing_harness_debug_helper_functions.cpp 
	  		)
		    set_target_properties(testing_harness_write_local PROPERTIES LINKER_LANGUAGE CXX )
		    target_link_libraries(testing_harness_write_local ${LOCAL_LIBS})

		    add_executable(testing_harness_read_local 
		    	my_md_source/my_metadata_client_local.cpp 
		    	my_md_source/my_metadata_client_lua_functs.cpp 
		    	my_md_source/my_metadata_server_local.cpp
		    	testing_source_local/testing_harness_read_local.cpp 
		    	testing_source_local/3d_read_for_testing_local_new.cpp 
		    	testing_source_local/extra_testing_collective_local.cpp 
		    	testing_source_local/extra_testing_collective_helper_functions_local_new.cpp
			    testing_source/testing_harness_debug_helper_functions.cpp 
		    )
		    set_target_properties(testing_harness_read_local PROPERTIES LINKER_LANGUAGE CXX )
		    target_link_libraries(testing_harness_read_local ${LOCAL_LIBS})
		endif(MAKE_EMPRESS_WITH_HDF5)

    	if(MAKE_EMPRESS_TEST_CLIENT)
		    add_executable(my_test_client_local 
		    	debugging_source/my_test_client_local.cpp 
		    	my_md_source/my_metadata_client_local.cpp 
		    	my_md_source/my_metadata_server_local.cpp
		    	debugging_source/helper_functions_test_client_local.C 
		    	debugging_source/helper_functions_2_test_client_local.cpp 
		    	debugging_source/helper_functions_3_test_client_local.cpp 
		    	my_md_source/my_metadata_client_lua_functs.cpp 
		    )
		    set_target_properties(my_test_client_local PROPERTIES LINKER_LANGUAGE CXX )
		    target_link_libraries(my_test_client_local ${LOCAL_LIBS})
		endif(MAKE_EMPRESS_TEST_CLIENT)

	endif(MAKE_LOCAL)

endif(MAKE_EMPRESS)

# if (MAKE_OBJECTOR_COMPARISON)
#     add_executable(sqlite_objector_comparison testing_source/sqlite_objector_comparison.cpp testing_source/3d_read_for_testing_objector_comparison.cpp)
#     set_target_properties(sqlite_objector_comparison PROPERTIES LINKER_LANGUAGE CXX )
#     target_link_libraries(sqlite_objector_comparison ${LUA_LIBRARIES} Database)
#  endif()   

if(MAKE_HDF5_HARNESS)
	set(HDF5_COMMON_LIBS
	    Boost::program_options
	    Boost::serialization
	    # ${TPL_LIBRARIES}
	    MPI_CXX
	)

    add_executable(testing_harness_write_hdf5 testing_source_hdf5/testing_harness_write_hdf5.cpp testing_source_hdf5/my_metadata_client_hdf5.cpp 
    	testing_source_hdf5/testing_harness_helper_functions_hdf5.cpp )
    set_target_properties(testing_harness_write_hdf5 PROPERTIES LINKER_LANGUAGE CXX )
    target_link_libraries(testing_harness_write_hdf5 ${HDF5_LIBRARIES} ${HDF5_C_LIBRARIES} ${HDF5_COMMON_LIBS})

    add_executable(testing_harness_read_hdf5 testing_source_hdf5/testing_harness_read_hdf5.cpp testing_source_hdf5/3d_read_for_testing_hdf5.cpp  
    	testing_source_hdf5/extra_testing_collective_hdf5.cpp testing_source_hdf5/extra_testing_collective_helper_functions_hdf5.cpp 
    	testing_source_hdf5/my_metadata_client_hdf5.cpp testing_source_hdf5/testing_harness_helper_functions_hdf5.cpp )
    set_target_properties(testing_harness_read_hdf5 PROPERTIES LINKER_LANGUAGE CXX )
    target_link_libraries(testing_harness_read_hdf5 ${HDF5_LIBRARIES} ${HDF5_C_LIBRARIES} ${HDF5_COMMON_LIBS})

    if(MAKE_HDF5_TEST_CLIENT)

	    add_executable(my_test_client_hdf5 debugging_source_hdf5/my_test_client_hdf5.cpp debugging_source_hdf5/helper_functions_test_client_hdf5.cpp 
	    	testing_source_hdf5/my_metadata_client_hdf5.cpp )
	    set_target_properties(my_test_client_hdf5 PROPERTIES LINKER_LANGUAGE CXX )
	    target_link_libraries(my_test_client_hdf5 ${HDF5_LIBRARIES} ${HDF5_C_LIBRARIES} ${HDF5_COMMON_LIBS})
	endif(MAKE_HDF5_TEST_CLIENT)
endif(MAKE_HDF5_HARNESS)
#-----------------------------------------
# Dump some info
message( STATUS "DATA_WAREHOUSE_SOURCE_DIR: ${DATA_WAREHOUSE_SOURCE_DIR}")
message( STATUS "boostroot: ${BOOST_ROOT}")
# message(STATUS " Gutties_LIBRARIES = ${Gutties_LIBRARIES}")
# message(STATUS " Webhook_LIBRARIES = ${Webhook_LIBRARIES}")
# message(STATUS " Lunasa_LIBRARIES = ${Lunasa_LIBRARIES}")
# message(STATUS " SBL_LIBRARIES = ${SBL_LIBRARIES}")
# message(STATUS " NNTI_LIBRARIES = ${NNTI_LIBRARIES}")
# message(STATUS " LIBFABRIC_LIBRARIES = ${LIBFABRIC_LIBRARIES}")
# message(STATUS " Final TPL: ${TPL_LIBRARIES}")
# message(STATUS " Final inc: ${TPL_INCLUDE_DIRS}")
message( STATUS "PROJECT_SOURCE_DIR ${PROJECT_SOURCE_DIR}" )
# message( STATUS "TPL_LIBRARIES      ${TPL_LIBRARIES}"      )
# message( STATUS "TPL_INCLUDE_DIRS   ${TPL_INCLUDE_DIRS}"   )
message( STATUS "Network Library    ${NETWORK_LIBRARY}"    )
message( STATUS "EXAMPLES_LIBS   ${EXAMPLES_LIBS}"    )
message(STATUS "Flags are ${CMAKE_CXX_FLAGS}")    
message(STATUS "Opbox_INCLUDE_DIRS     ${Opbox_INCLUDE_DIRS}")
# message(STATUS "Opbox_LIBRARIES        ${Opbox_LIBRARIES}")

message( STATUS "GTEST_INCLUDE_DIRS ${GTEST_INCLUDE_DIRS}" )
message( STATUS "GTEST_LIBRARIES    ${GTEST_LIBRARIES}"    )
# message( STATUS "GTEST_SOURCE_DIR   ${GTEST_SOURCE_DIR}"   )


message( STATUS "HDF5_INCLUDE_DIRS   ${HDF5_INCLUDE_DIRS}"   )
message( STATUS "HDF5_C_LIBRARIES   ${HDF5_C_LIBRARIES}"   )

# message(STATUS "COMMON_LIBS        ${COMMON_LIBS}")



