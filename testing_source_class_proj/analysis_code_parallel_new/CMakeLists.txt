

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

include(FindOpenMP)
if(OPENMP_FOUND)
    message("Found OpenMP, building parallel part.")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")

else(OPENMP_FOUND)
    message("ERROR: OpenMP not found.")
endif(OPENMP_FOUND)


# set(SOURCE_FILES
#   read_testing_output_one_output.cpp
#   read_write_clients_output_new_new.cpp  
#   # read_write_clients_output.cpp  
#   read_read_clients_output.cpp  
#   read_servers_output_one_output.cpp  
#   read_dirmans_output_one_output.cpp  
# )

# set(HEADERS
#   stats_functions.hh
#   read_testing_output_one_output.hh
# )

# add_executable(read_output ${SOURCE_FILES} ${HEADERS})
# set_target_properties(read_output PROPERTIES LINKER_LANGUAGE CXX )


set(SOURCE_FILES_NEW
  read_testing_output_streaming.cpp
  # read_testing_output_streaming_new.cpp
  read_write_clients_output_new_new.cpp  
  read_read_clients_output.cpp  
  read_servers_output_one_output.cpp  
  read_dirmans_output_one_output.cpp  
)

# set(HEADERS_NEW
#   stats_functions_streaming.hh
#   read_testing_output_streaming.hh
# )

set(SOURCE_FILES_NEW_NEW
  read_testing_output_const_streaming.cpp
  # read_testing_output_streaming_new.cpp
  read_write_clients_output_const_streaming_new_new_new_new.cpp  
  # read_write_clients_output_const_streaming_new.cpp  
  # read_write_clients_output_const_streaming.cpp  
  read_read_clients_output_const_streaming.cpp  
  read_servers_output_const_streaming.cpp  
  read_dirmans_output_one_output.cpp  
  stats_functions_const_streaming.cpp
)

set(HEADERS_NEW
  stats_functions_const_streaming.hh
  read_testing_output_const_streaming.hh
)

# add_executable(read_output ${SOURCE_FILES_NEW_NEW} ${HEADERS_NEW})
# set_target_properties(read_output PROPERTIES LINKER_LANGUAGE CXX )

add_executable(read_output_new ${SOURCE_FILES_NEW_NEW} ${HEADERS_NEW})
set_target_properties(read_output_new PROPERTIES LINKER_LANGUAGE CXX )
  
