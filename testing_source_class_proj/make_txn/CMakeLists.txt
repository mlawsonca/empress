cmake_minimum_required(VERSION 3.2)
project( make_txn
  LANGUAGES CXX C
  VERSION 0.1.0.0 )


set( CMAKE_CXX_STANDARD 17 )
set( CMAKE_CXX_STANDARD_REQUIRED YES )
set( CMAKE_CXX_EXTENSIONS OFF )



if(NOT CMAKE_CXX_FLAGS)
   set(CMAKE_CXX_FLAGS "-std=c++17 -g -w")

endif()

find_package(Lua "5.1" EXACT REQUIRED)


include_directories(
  ${PROJECT_SOURCE_DIR}/../../include/common
)


add_executable(make_txn_files make_txn_files.cpp make_txn_helper_functions.cpp )
set_target_properties(make_txn_files PROPERTIES LINKER_LANGUAGE CXX )

add_executable(make_analysis_txn_files make_analysis_txn_files.cpp make_txn_helper_functions.cpp )
set_target_properties(make_analysis_txn_files PROPERTIES LINKER_LANGUAGE CXX )
