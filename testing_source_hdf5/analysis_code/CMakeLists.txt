

cmake_minimum_required(VERSION 3.2)
project( empress
  LANGUAGES CXX C
  VERSION 0.1.0.0 )

#
# Require C++11 from whatever compiler is in use
#


#set( CMAKE_CXX_COMPILER /opt/openmpi-1.10-gnu/bin/mpiCC )

set( CMAKE_CXX_STANDARD 11 )
set( CMAKE_CXX_STANDARD_REQUIRED YES )
set( CMAKE_CXX_EXTENSIONS OFF )

if(NOT CMAKE_CXX_FLAGS)
   set(CMAKE_CXX_FLAGS "-std=c++11 -g")

endif()


set(SOURCE_FILES
  eval_testing_output_hdf5.cpp
  eval_write_clients_output_hdf5.cpp  
  eval_read_clients_output_hdf5.cpp  
)

set(HEADERS
  stats_functions.hh
  eval_testing_output_hdf5.hh
)


add_executable(read_output ${SOURCE_FILES} ${HEADERS})
set_target_properties(read_output PROPERTIES LINKER_LANGUAGE CXX )

  
