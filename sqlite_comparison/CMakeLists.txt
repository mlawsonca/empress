
cmake_minimum_required(VERSION 3.2)
project( sqlite_comparison
  LANGUAGES CXX C
  VERSION 0.1.0.0 )


set( CMAKE_CXX_STANDARD 11 )
set( CMAKE_CXX_STANDARD_REQUIRED YES )
set( CMAKE_CXX_EXTENSIONS OFF )



if(NOT CMAKE_CXX_FLAGS)
   set(CMAKE_CXX_FLAGS "-D_PTHREADS  -pthread -L${PROJECT_SOURCE_DIR}/lib/database/sqlite3 -std=c++11 -g -w")

endif()

find_package(Lua "5.1" EXACT REQUIRED)


include_directories(
  ${PROJECT_SOURCE_DIR}/include
)

# add_library(Database SHARED
#     source/sqlite3.c
# )
add_library(Database SHARED
    ${PROJECT_SOURCE_DIR}/../lib_source/database/sqlite3.c
)
TARGET_INCLUDE_DIRECTORIES(Database PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../include/database
)


add_executable(sqlite_objector_comparison source/sqlite_objector_comparison.cpp source/3d_read_for_testing_objector_comparison.cpp)
set_target_properties(sqlite_objector_comparison PROPERTIES LINKER_LANGUAGE CXX )
target_link_libraries(sqlite_objector_comparison ${LUA_LIBRARIES} Database)

add_executable(analyze_timing_output source/analyze_timing_output.cpp )
set_target_properties(analyze_timing_output PROPERTIES LINKER_LANGUAGE CXX )
target_link_libraries(analyze_timing_output )

add_executable(db_oversize source/db_oversize.cpp )
set_target_properties(db_oversize PROPERTIES LINKER_LANGUAGE CXX )
target_link_libraries(db_oversize ${LUA_LIBRARIES} Database)



# add_executable(sqlite_objector_comparison1 sqlite_objector_comparison.cpp 3d_read_for_testing_objector_comparison.cpp)
# set_target_properties(sqlite_objector_comparison1 PROPERTIES LINKER_LANGUAGE CXX )
# target_link_libraries(sqlite_objector_comparison1 ${LUA_LIBRARIES} Database)

# add_executable(analyze_timing_output1 analyze_timing_output.cpp )
# set_target_properties(analyze_timing_output1 PROPERTIES LINKER_LANGUAGE CXX )
# target_link_libraries(analyze_timing_output1 )

# add_executable(sqlite_objector_comparison2 sqlite_objector_comparison.cpp 3d_read_for_testing_objector_comparison.cpp)
# set_target_properties(sqlite_objector_comparison2 PROPERTIES LINKER_LANGUAGE CXX )
# target_link_libraries(sqlite_objector_comparison2 ${LUA_LIBRARIES} Database)

# add_executable(analyze_timing_output2 analyze_timing_output.cpp )
# set_target_properties(analyze_timing_output2 PROPERTIES LINKER_LANGUAGE CXX )
# target_link_libraries(analyze_timing_output2 )

# add_executable(sqlite_objector_comparison3 sqlite_objector_comparison.cpp 3d_read_for_testing_objector_comparison.cpp)
# set_target_properties(sqlite_objector_comparison3 PROPERTIES LINKER_LANGUAGE CXX )
# target_link_libraries(sqlite_objector_comparison3 ${LUA_LIBRARIES} Database)

# add_executable(analyze_timing_output3 analyze_timing_output.cpp )
# set_target_properties(analyze_timing_output3 PROPERTIES LINKER_LANGUAGE CXX )
# target_link_libraries(analyze_timing_output3 )

message(STATUS "PROJECT_SOURCE_DIR        ${PROJECT_SOURCE_DIR}")
message(STATUS "LUA_LIBRARIES ${LUA_LIBRARIES}")
