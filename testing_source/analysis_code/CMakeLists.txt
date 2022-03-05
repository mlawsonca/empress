

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
  read_testing_output_one_output.cpp
  read_write_clients_output_new_new.cpp  
  # read_write_clients_output.cpp  
  read_read_clients_output.cpp  
  read_servers_output_one_output.cpp  
  read_dirmans_output_one_output.cpp  
)

set(HEADERS
  stats_functions.hh
  read_testing_output_one_output.hh
)


add_executable(read_output ${SOURCE_FILES} ${HEADERS})
set_target_properties(read_output PROPERTIES LINKER_LANGUAGE CXX )

  
