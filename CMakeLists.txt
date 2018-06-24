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


set( CMAKE_CXX_STANDARD 11 )
set( CMAKE_CXX_STANDARD_REQUIRED YES )
set( CMAKE_CXX_EXTENSIONS OFF )

set(Opbox_CXX_COMPILER /usr/bin/c++ )

if(NOT CMAKE_CXX_FLAGS)
   set(CMAKE_CXX_FLAGS "-D_PTHREADS  -pthread -DBOOST_LOG_DYN_LINK -L${PROJECT_SOURCE_DIR}/lib/database/sqlite3 -std=c++11 -g")

endif()

option( USE_MPI "build with MPI for parallelism." ON )

if( "${DATA_WAREHOUSE_SOURCE_DIR}" STREQUAL "" )
    set(DATA_WAREHOUSE_SOURCE_DIR $ENV{HOME}/data-warehouse-release )
endif()

message( STATUS "DATA_WAREHOUSE_SOURCE_DIR: ${DATA_WAREHOUSE_SOURCE_DIR}")

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
  set(NETWORK_LIBRARY "nnti" CACHE STRING "Initial cache" FORCE)
endif()

message( STATUS "boostroot: ${BOOST_ROOT}")

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

dw_find_datawarehouse( COMPONENTS SBL Lunasa Webhook Gutties NNTI Opbox )


include_directories(
  ${TPL_INCLUDE_DIRS}
  ${Boost_INCLUDE_DIRS}
  ${PROJECT_SOURCE_DIR}/include/common
  ${PROJECT_SOURCE_DIR}/include/client
  ${PROJECT_SOURCE_DIR}/include/ops
  ${Opbox_INCLUDE_DIRS}
  support
  ${OPBOX_PROJ_SOURCE_DIR}/examples/support
)

#-- Support Lib----------------------------------------------------------------
add_library(mpi_support STATIC
            ${OPBOX_PROJ_SOURCE_DIR}/examples/support/Globals.hh
            ${OPBOX_PROJ_SOURCE_DIR}/examples/support/Globals.cpp
            ${OPBOX_PROJ_SOURCE_DIR}/examples/support/SimpleDataStore.hh
            ${OPBOX_PROJ_SOURCE_DIR}/examples/support/SimpleDataStore.cpp
)
set_target_properties(mpi_support PROPERTIES LINKER_LANGUAGE CXX )


add_subdirectory(lib_source)

set(COMMON_LIBS
    opbox
    lunasa
    gutties
    Boost::program_options
    Boost::serialization
    ${TPL_LIBRARIES}
    MPI_CXX
)

set(CLIENT_LIBS
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


    add_executable(my_metadata_server my_md_source/my_metadata_server.C)
    set_target_properties(my_metadata_server PROPERTIES LINKER_LANGUAGE CXX )
    target_link_libraries(my_metadata_server ${COMMON_LIBS} ${SERVER_LIBS})

    add_executable(my_dirman my_md_source/my_dirman.C)
    set_target_properties(my_dirman PROPERTIES LINKER_LANGUAGE CXX )
    target_link_libraries(my_dirman ${COMMON_LIBS} ${DIRMAN_LIBS})

    add_executable(testing_harness testing_source/testing_harness.cpp testing_source/3d_read_for_testing.cpp testing_source/extra_testing.cpp my_md_source/my_metadata_client.C )
    set_target_properties(testing_harness PROPERTIES LINKER_LANGUAGE CXX )
    target_link_libraries(testing_harness ${COMMON_LIBS} ${CLIENT_LIBS})


#-----------------------------------------
# Dump some info
message(STATUS " Gutties_LIBRARIES = ${Gutties_LIBRARIES}")
message(STATUS " Webhook_LIBRARIES = ${Webhook_LIBRARIES}")
message(STATUS " Lunasa_LIBRARIES = ${Lunasa_LIBRARIES}")
message(STATUS " SBL_LIBRARIES = ${SBL_LIBRARIES}")
message(STATUS " NNTI_LIBRARIES = ${NNTI_LIBRARIES}")
message(STATUS " LIBFABRIC_LIBRARIES = ${LIBFABRIC_LIBRARIES}")
message(STATUS " Final TPL: ${TPL_LIBRARIES}")
message(STATUS " Final inc: ${TPL_INCLUDE_DIRS}")
message( STATUS "PROJECT_SOURCE_DIR ${PROJECT_SOURCE_DIR}" )
message( STATUS "TPL_LIBRARIES      ${TPL_LIBRARIES}"      )
message( STATUS "TPL_INCLUDE_DIRS   ${TPL_INCLUDE_DIRS}"   )
message( STATUS "Network Library    ${NETWORK_LIBRARY}"    )
message( STATUS "EXAMPLES_LIBS   ${EXAMPLES_LIBS}"    )
message(STATUS "Flags are ${CMAKE_CXX_FLAGS}")    
message(STATUS "Opbox_INCLUDE_DIRS     ${Opbox_INCLUDE_DIRS}")
message(STATUS "Opbox_LIBRARIES        ${Opbox_LIBRARIES}")

message( STATUS "GTEST_INCLUDE_DIRS ${GTEST_INCLUDE_DIRS}" )
message( STATUS "GTEST_LIBRARIES    ${GTEST_LIBRARIES}"    )
message( STATUS "gtest_SOURCE_DIR   ${gtest_SOURCE_DIR}"   )
