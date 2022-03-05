
cmake_minimum_required(VERSION 3.2)
project( empress
  LANGUAGES CXX C
  VERSION 0.1.0.0 )


set( CMAKE_CXX_STANDARD 17 )

set( CMAKE_CXX_STANDARD_REQUIRED YES )
set( CMAKE_CXX_EXTENSIONS OFF )

set ( FAODEL TRUE )
if( "${OPTIMIZED}" STREQUAL "" )
    set ( OPTIMIZED TRUE )
endif()

cmake_host_system_information(RESULT HOSTNAME QUERY HOSTNAME)


if(NOT CMAKE_CXX_FLAGS)
   set(CMAKE_CXX_FLAGS "-D_PTHREADS -pthread -DBOOST_LOG_DYN_LINK -std=c++17 -g -O3 -DNDEBUG -w -Wall -lz -ldl")
endif()

link_directories( "FILL_IN_WITH_APPROPRIATE_PATH/gperftools")


option( USE_MPI "build with MPI for parallelism." ON )

if(FAODEL)
    if( "${OLD_DATA_WAREHOUSE_SOURCE_DIR}" STREQUAL "" )
        set(OLD_DATA_WAREHOUSE_SOURCE_DIR $ENV{HOME}/data-warehouse-release)
    endif()
    if( "${DATA_WAREHOUSE_SOURCE_DIR}" STREQUAL "" )
        set(DATA_WAREHOUSE_SOURCE_DIR $ENV{HOME}/faodel)

    endif()
    if( "${DATA_WAREHOUSE_TPL_INSTALL_DIR}" STREQUAL "" )
        set(DATA_WAREHOUSE_TPL_INSTALL_DIR ${OLD_DATA_WAREHOUSE_SOURCE_DIR}/tpl/install)
    endif()
else(FAODEL)
    if( "${DATA_WAREHOUSE_SOURCE_DIR}" STREQUAL "" )
        set(DATA_WAREHOUSE_SOURCE_DIR $ENV{HOME}/data-warehouse-release)
    endif()
    if( "${DATA_WAREHOUSE_TPL_INSTALL_DIR}" STREQUAL "" )
      set(DATA_WAREHOUSE_TPL_INSTALL_DIR ${DATA_WAREHOUSE_SOURCE_DIR}/tpl/install )
    endif()
endif(FAODEL)

if( "${DATA_WAREHOUSE_INSTALL_DIR}" STREQUAL "" )
  set(DATA_WAREHOUSE_INSTALL_DIR ${DATA_WAREHOUSE_SOURCE_DIR}/install )
endif()

set(CMAKE_PREFIX_PATH ${DATA_WAREHOUSE_INSTALL_DIR})


if( "${BOOST_ROOT}" STREQUAL "" )
  set( BOOST_ROOT ${DATA_WAREHOUSE_TPL_INSTALL_DIR} )
endif()
message( STATUS "boostroot: ${BOOST_ROOT}")
if( "${GTEST_ROOT}" STREQUAL "" )
  set( GTEST_ROOT ${DATA_WAREHOUSE_TPL_INSTALL_DIR} )
endif()

if( "${NETWORK_LIBRARY}" STREQUAL "" )
  #set(NETWORK_LIBRARY "libfabric" CACHE STRING "Initial cache" FORCE)
  set(NETWORK_LIBRARY "nnti" CACHE STRING "Initial cache" FORCE)
endif()


set(OPBOX_PROJ_SOURCE_DIR ${DATA_WAREHOUSE_SOURCE_DIR}/src/opbox)

set( CMAKE_MODULE_PATH ${OPBOX_PROJ_SOURCE_DIR}/../../cmake/modules ${CMAKE_MODULE_PATH}  )

set(DATA_WAREHOUSE_INCLUDE_DIRS ${DATA_WAREHOUSE_INSTALL_DIR}/include/faodel )
message(STATUS "DATA_WAREHOUSE_INCLUDE_DIRS: ${DATA_WAREHOUSE_INCLUDE_DIRS}" )



find_package( Faodel CONFIG REQUIRED )

find_package(Lua "5.1" EXACT REQUIRED)


set ( MAKE_LOCAL FALSE )
# set ( MAKE_EMPRESS FALSE ) #needed to make the server and dirman
set ( MAKE_EMPRESS_HARNESS FALSE )
set ( MAKE_EMPRESS_WITH_HDF5 FALSE )
set ( MAKE_EMPRESS_TEST_CLIENT FALSE )
set ( MAKE_HDF5_HARNESS FALSE )
set ( MAKE_HDF5_TEST_CLIENT FALSE )
set ( MAKE_CLASS_PROJ_HARNESS FALSE )
set ( MAKE_LARGE_MD_VOL_HARNESS FALSE )
set ( MAKE_HDF5_CLASS_PROJ_HARNESS FALSE )
set ( MAKE_CLASS_PROJ_SQLITE_TRANSACTIONS_ONLY FALSE )
set ( MAKE_CLASS_PROJ_D2T_TRANSACTIONS_ONLY FALSE )
set ( MAKE_CLASS_PROJ_RTREE FALSE )
set ( MAKE_CLASS_PROJ_PREGENERATE FALSE )
set ( MAKE_ASYNC FALSE )


if( "${MAKE_LOCAL}" STREQUAL "" )
    set ( MAKE_LOCAL TRUE )
endif()

if( "${MAKE_EMPRESS}" STREQUAL "" )
    set ( MAKE_EMPRESS TRUE )
endif()

if( "${MAKE_EMPRESS_HARNESS}" STREQUAL "" )
    set ( MAKE_EMPRESS_HARNESS TRUE )
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
#   set ( MAKE_OBJECTOR_COMPARISON TRUE )
# endif()

if( "${MAKE_CLASS_PROJ_HARNESS}" STREQUAL "" )
    set ( MAKE_CLASS_PROJ_HARNESS TRUE )
endif()


if( "${MAKE_LARGE_MD_VOL_HARNESS}" STREQUAL "" )
    set ( MAKE_LARGE_MD_VOL_HARNESS TRUE )
endif()

if( "${MAKE_HDF5_CLASS_PROJ_HARNESS}" STREQUAL "" )
    set ( MAKE_HDF5_CLASS_PROJ_HARNESS TRUE )
endif()

if( "${MAKE_CLASS_PROJ_SQLITE_TRANSACTIONS_ONLY}" STREQUAL "" )
    set ( MAKE_CLASS_PROJ_SQLITE_TRANSACTIONS_ONLY TRUE )
endif()

if( "${MAKE_CLASS_PROJ_D2T_TRANSACTIONS_ONLY}" STREQUAL "" )
    set ( MAKE_CLASS_PROJ_D2T_TRANSACTIONS_ONLY TRUE )
endif()


if( "${MAKE_CLASS_PROJ_RTREE}" STREQUAL "" )
    set ( MAKE_CLASS_PROJ_RTREE TRUE )
endif()

if( "${MAKE_CLASS_PROJ_PREGENERATE}" STREQUAL "" )
    set ( MAKE_CLASS_PROJ_PREGENERATE TRUE )
endif()

if( "${MAKE_ASYNC}" STREQUAL "" )
    set ( MAKE_ASYNC TRUE )


# if ( MAKE_HDF5_HARNESS OR MAKE_EMPRESS_WITH_HDF5 )
    set (HDF5_INCLUDE_DIRS 
        FILL_IN_HDF5_INSTALL_DIR/include
        ${PROJECT_SOURCE_DIR}/include/hdf5
    )
    set (HDF5_C_LIBRARIES 
        FILL_IN_HDF5_INSTALL_DIR/include/lib/libhdf5.so
        FILL_IN_HDF5_INSTALL_DIR/include/lib/libhdf5_hl.so
    )

    set (HDF5_1_12_INCLUDE_DIRS 
        "$ENV{scratch}"/FILL_IN_HDF5_1_12_INSTALL_DIR/include
    )
    set (HDF5_1_12_C_LIBRARIES 
        "$ENV{scratch}"/FILL_IN_HDF5_1_12_INSTALL_DIR/lib/libhdf5.a
        "$ENV{scratch}"/FILL_IN_HDF5_1_12_INSTALL_DIR/lib/libhdf5_hl.a
    )
# endif()

# if ( MAKE_EMPRESS )
    set (EMPRESS_INCLUDE_DIRS 
        ${LUA_INCLUDE_DIR}
        ${PROJECT_SOURCE_DIR}/include/client    
        ${PROJECT_SOURCE_DIR}/include/ops_rdma
        ${PROJECT_SOURCE_DIR}/include/ops_rdma_sqlite_transactions_only
        ${DATA_WAREHOUSE_INCLUDE_DIRS}
        support
        ${DATA_WAREHOUSE_SOURCE_DIR}/examples/opbox/support #provides globals
        ${TPL_INCLUDE_DIRS}
        ${PROJECT_SOURCE_DIR}/include
    )

set (CLASS_PROJ_INCLUDE_DIRS
    ${PROJECT_SOURCE_DIR}/include/class_proj            
)

set (LOCAL_INCLUDE_DIRS
    ${PROJECT_SOURCE_DIR}/include/local         
)

include_directories(
  ${HDF5_1_12_INCLUDE_DIRS}
  ${EMPRESS_INCLUDE_DIRS}
  # ${OBJECTOR_COMPARISON_INCLUDE_DIRS}
  ${CLASS_PROJ_INCLUDE_DIRS}
  ${MPI_CXX_INCLUDE_PATH}
  ${Boost_INCLUDE_DIRS}
  ${LOCAL_INCLUDE_DIRS}
)


set(BOOST_MPI_LIBS
    Boost::program_options
    Boost::serialization
    MPI::MPI_CXX
)


message( STATUS "HDF5_C_LIBRARIES: ${HDF5_1_12_C_LIBRARIES}")
message( STATUS "BOOST_MPI_LIBS: ${BOOST_MPI_LIBS}")

# add_executable(lua_test 
#   debugging_source/lua_test.cpp
#   my_md_source/my_metadata_client_lua_functs.cpp 
# )
# set_include_directories(server PUBLIC ${PROJECT_SOURCE_DIR}/include/common)# 
# set_target_properties(lua_test PROPERTIES LINKER_LANGUAGE CXX )
# target_link_libraries(lua_test ${LUA_LIBRARIES})

add_subdirectory(lib_source)

if(MAKE_EMPRESS)
    # add_subdirectory(lib_source)

    set(COMMON_LIBS
        Faodel::opbox #are these still needed?
        Faodel::lunasa
        Faodel::dirman
        # Faodel::gutties
        Boost::program_options
        Boost::serialization
        # ${TPL_LIBRARIES}
        MPI::MPI_CXX
    )

    set(CLIENT_LIBS
        ${LUA_LIBRARIES}
        # OpsCommon
        OpsClient 
    )

    set(SERVER_LIBS
        Database
        # OpsCommon
        OpsServer
    )

    set(DIRMAN_LIBS
        FullShutdownOpForDirman
    )

    set(LOCAL_LIBS
        ${LUA_LIBRARIES}
        Boost::serialization
        MPI::MPI_CXX
        Database
        # EmpressLocal
        OpsServer
        # OpsCommon
    )

    if(MAKE_CLASS_PROJ_SQLITE_TRANSACTIONS_ONLY)
        set(CLIENT_LIBS_SQLITE_TRANSACTION_ONLY
            ${LUA_LIBRARIES}
            # OpsCommon
            OpsClientSqliteTransactionOnly
        )

        set(SERVER_LIBS_SQLITE_TRANSACTION_ONLY
            Database
            # OpsCommon
            OpsServerSqliteTransactionOnly
        )

        set(LOCAL_LIBS_SQLITE_TRANSACTION_ONLY
            ${LUA_LIBRARIES}
            Boost::serialization
            MPI::MPI_CXX
            Database
            # EmpressLocal
            OpsServerSqliteTransactionOnly
            # OpsCommon
        )


        set(CLIENT_LIBS_SQLITE_TRANSACTION_ONLY_DB_STREAMS
            ${LUA_LIBRARIES}
            # OpsCommon
            OpsClientSqliteTransactionOnlyDBStreams
        )

        set(SERVER_LIBS_SQLITE_TRANSACTION_ONLY_DB_STREAMS
            Database
            # OpsCommon
            OpsServerSqliteTransactionOnlyDBStreams
        )


    endif(MAKE_CLASS_PROJ_SQLITE_TRANSACTIONS_ONLY)

    if(MAKE_CLASS_PROJ_D2T_TRANSACTIONS_ONLY)

        set(SERVER_LIBS_D2T_TRANSACTION_ONLY
            Database
            # OpsCommon
            OpsServerD2TTransactionOnly
        )

        set(CLIENT_LIBS_D2T_TRANSACTION_ONLY
            ${LUA_LIBRARIES}
            # OpsCommon
            OpsClientD2TTransactionOnly
        )


        set(LOCAL_LIBS_D2T_TRANSACTION_ONLY
            ${LUA_LIBRARIES}
            Boost::serialization
            MPI::MPI_CXX
            Database
            # EmpressLocal
            OpsServerD2TTransactionOnly
            # OpsCommon
        )
    endif(MAKE_CLASS_PROJ_D2T_TRANSACTIONS_ONLY)

    if(MAKE_CLASS_PROJ_RTREE)
        set(SERVER_LIBS_RTREE
            Database
            # OpsCommon
            OpsServerRtree
        )
    endif(MAKE_CLASS_PROJ_RTREE)


    add_executable(my_metadata_server my_md_source/my_metadata_server.C)
    target_include_directories(my_metadata_server PUBLIC ${PROJECT_SOURCE_DIR}/include/common)
    set_target_properties(my_metadata_server PROPERTIES LINKER_LANGUAGE CXX )
    target_link_libraries(my_metadata_server ${COMMON_LIBS} ${SERVER_LIBS} )

    add_executable(my_dirman my_md_source/my_dirman.C)
    target_include_directories(my_dirman PUBLIC ${PROJECT_SOURCE_DIR}/include/common)
    set_target_properties(my_dirman PROPERTIES LINKER_LANGUAGE CXX )
    target_link_libraries(my_dirman ${COMMON_LIBS} ${DIRMAN_LIBS})

    if(MAKE_EMPRESS_TEST_CLIENT)
        add_executable(my_test_client 
            my_md_source/my_metadata_client.C 
            debugging_source/my_test_client.C 
            debugging_source/helper_functions_test_client.C 
            debugging_source/helper_functions_2_test_client.cpp 
            debugging_source/helper_functions_3_test_client.cpp 
            my_md_source/my_metadata_client_lua_functs.cpp 
        )
        target_include_directories(my_test_client PUBLIC ${PROJECT_SOURCE_DIR}/include/common)
        set_target_properties(my_test_client PROPERTIES LINKER_LANGUAGE CXX )
        target_link_libraries(my_test_client ${COMMON_LIBS} ${CLIENT_LIBS} )

        # add_executable(my_gathered_write_test_client 
     #      my_md_source/my_metadata_client.C 
     #      debugging_source/my_gathered_write_test_client.cpp 
     #      my_md_source/my_metadata_client_lua_functs.cpp 
     #    )      target_include_directories(my_gathered_write_test_client PUBLIC ${PROJECT_SOURCE_DIR}/include/common)#    
     #    set_target_properties(my_gathered_write_test_client PROPERTIES LINKER_LANGUAGE CXX )
     #    target_link_libraries(my_gathered_write_test_client ${COMMON_LIBS} ${CLIENT_LIBS} )

        if(MAKE_ASYNC)
            add_executable(my_test_client_async 
                my_md_source/my_metadata_client_async.cpp
                # my_md_source/my_metadata_client.C #includes the metadata init, finalize & print functs
                debugging_source/async/my_test_client_async.cpp
                debugging_source/async/helper_functions_test_client_async.cpp
                debugging_source/async/helper_functions_2_test_client_async.cpp
                debugging_source/async/helper_functions_3_test_client_async.cpp
                my_md_source/my_metadata_client_lua_functs.cpp 
            )
            target_include_directories(my_test_client_async PUBLIC ${PROJECT_SOURCE_DIR}/include/common)
            set_target_properties(my_test_client_async PROPERTIES LINKER_LANGUAGE CXX )
            target_link_libraries(my_test_client_async ${COMMON_LIBS} ${CLIENT_LIBS} )

        endif(MAKE_ASYNC)

    endif(MAKE_EMPRESS_TEST_CLIENT)


    if(MAKE_CLASS_PROJ_SQLITE_TRANSACTIONS_ONLY)
        if(MAKE_EMPRESS_TEST_CLIENT)
            # add_executable(my_test_client_sqlite_transactions_only 
            #     # my_md_source/my_metadata_client_sqlite_transactions_only.cpp
            #     # my_md_source/my_metadata_client_sqlite_transactions_only_new.cpp
            #     my_md_source/my_metadata_client_sqlite_transactions_only_multiple_databases_read_and_write.cpp
            #     debugging_source/my_test_client_sqlite_transactions_only.cpp
            #     debugging_source/helper_functions_test_client_sqlite_transactions_only_new.cpp  
            #     debugging_source/helper_functions_2_test_client.cpp 
            #     debugging_source/helper_functions_3_test_client_sqlite_transactions_only_new.cpp 
            #     # debugging_source/my_test_client.C 
            #     # debugging_source/helper_functions_test_client.C 
            #     # debugging_source/helper_functions_2_test_client.cpp 
            #     # debugging_source/helper_functions_3_test_client.cpp
            #     my_md_source/my_metadata_client_lua_functs.cpp 
            # )
            # target_include_directories(my_test_client_sqlite_transactions_only PUBLIC ${PROJECT_SOURCE_DIR}/include/common_sqlite_transactions_only_multiple_databases_read_and_write)
            # set_target_properties(my_test_client_sqlite_transactions_only PROPERTIES LINKER_LANGUAGE CXX )
            # target_link_libraries(my_test_client_sqlite_transactions_only ${COMMON_LIBS} ${CLIENT_LIBS_SQLITE_TRANSACTION_ONLY} )
            add_executable(my_test_client_sqlite_transactions_only 
                # my_md_source/my_metadata_client_sqlite_transactions_only.cpp
                # my_md_source/my_metadata_client_sqlite_transactions_only_new.cpp
                my_md_source/my_metadata_client_sqlite_transactions_only_multiple_databases_read_and_write_new.cpp
                debugging_source/my_test_client_sqlite_transactions_only_new_new.cpp
                debugging_source/helper_functions_test_client_sqlite_transactions_only_new_new.cpp  
                debugging_source/helper_functions_2_test_client_sqlite_transactions_only_new_new.cpp
                debugging_source/helper_functions_3_test_client_sqlite_transactions_only_new.cpp 
                my_md_source/my_metadata_client_lua_functs.cpp 
            )
            target_include_directories(my_test_client_sqlite_transactions_only PUBLIC ${PROJECT_SOURCE_DIR}/include/common_sqlite_transactions_only_multiple_databases_read_and_write_new)
            set_target_properties(my_test_client_sqlite_transactions_only PROPERTIES LINKER_LANGUAGE CXX )
            target_link_libraries(my_test_client_sqlite_transactions_only ${COMMON_LIBS} ${CLIENT_LIBS_SQLITE_TRANSACTION_ONLY} )


            # add_executable(my_test_client_sqlite_transactions_only_db_streams
            #     # my_md_source/my_metadata_client_sqlite_transactions_only.cpp
            #     # my_md_source/my_metadata_client_sqlite_transactions_only_new.cpp
            #     my_md_source/my_metadata_client_sqlite_transactions_only_multiple_databases_by_job_id.cpp
            #     debugging_source/my_test_client_sqlite_transactions_only_new_new.cpp
            #     debugging_source/helper_functions_test_client_sqlite_transactions_only_new_new.cpp  
            #     debugging_source/helper_functions_2_test_client_sqlite_transactions_only_new_new.cpp
            #     debugging_source/helper_functions_3_test_client_sqlite_transactions_only_new.cpp 
            #     my_md_source/my_metadata_client_lua_functs.cpp 
            # )
            add_executable(my_test_client_sqlite_transactions_only_db_streams
                # my_md_source/my_metadata_client_sqlite_transactions_only.cpp
                # my_md_source/my_metadata_client_sqlite_transactions_only_new.cpp
                my_md_source/my_metadata_client_sqlite_transactions_only_multiple_databases_by_job_id_denormalized.cpp
                debugging_source/my_test_client_sqlite_transactions_only_db_streams.cpp
                debugging_source/helper_functions_test_client_sqlite_transactions_only_db_streams.cpp
                debugging_source/helper_functions_2_test_client_sqlite_transactions_only_db_streams.cpp
                debugging_source/helper_functions_3_test_client.cpp 
                my_md_source/my_metadata_client_lua_functs.cpp 
            )
            target_include_directories(my_test_client_sqlite_transactions_only_db_streams PUBLIC ${PROJECT_SOURCE_DIR}/include/common_sqlite_transactions_only_db_streams)
            set_target_properties(my_test_client_sqlite_transactions_only_db_streams PROPERTIES LINKER_LANGUAGE CXX )
            target_link_libraries(my_test_client_sqlite_transactions_only_db_streams ${COMMON_LIBS} ${CLIENT_LIBS_SQLITE_TRANSACTION_ONLY_DB_STREAMS} )
        endif(MAKE_EMPRESS_TEST_CLIENT)

        add_executable(testing_harness_class_proj_sqlite_transactions_only
            my_md_source/my_metadata_client_sqlite_transactions_only_multiple_databases_read_and_write_new.cpp
            # my_md_source/my_metadata_client_sqlite_transactions_only_new.cpp
            # my_md_source/my_metadata_client_sqlite_transactions_only.cpp
            my_md_source/my_metadata_client_lua_functs.cpp  
            testing_source_class_proj/testing_harness_write_class_proj_sqlite_transactions_only.cpp  
            testing_source_class_proj/testing_harness_debug_helper_functions.cpp
            testing_source_class_proj/testing_harness_read_class_proj_new.cpp 
            testing_source_class_proj/3d_read_for_testing_class_proj_sqlite_transactions_only.cpp 
            testing_source_class_proj/extra_testing_class_proj.cpp 
            # testing_source_class_proj/extra_testing_helper_functions_class_proj.cpp
        )
        target_include_directories(testing_harness_class_proj_sqlite_transactions_only PUBLIC ${PROJECT_SOURCE_DIR}/include/common_sqlite_transactions_only_multiple_databases_read_and_write_new)
        set_target_properties(testing_harness_class_proj_sqlite_transactions_only PROPERTIES LINKER_LANGUAGE CXX )
        target_link_libraries(testing_harness_class_proj_sqlite_transactions_only ${COMMON_LIBS} ${CLIENT_LIBS_SQLITE_TRANSACTION_ONLY} )

        # add_executable(my_metadata_server_sqlite_transactions_only my_md_source/my_metadata_server_sqlite_transactions_only.cpp)
        add_executable(my_metadata_server_sqlite_transactions_only my_md_source/my_metadata_server_sqlite_transactions_only_multiple_transactions_single_database_new.cpp)
        target_include_directories(my_metadata_server_sqlite_transactions_only PUBLIC ${PROJECT_SOURCE_DIR}/include/common_sqlite_transactions_only_multiple_databases_read_and_write_new)
        set_target_properties(my_metadata_server_sqlite_transactions_only PROPERTIES LINKER_LANGUAGE CXX )
        target_link_libraries(my_metadata_server_sqlite_transactions_only ${COMMON_LIBS} ${SERVER_LIBS_SQLITE_TRANSACTION_ONLY} )


        add_executable(testing_harness_class_proj_sqlite_transactions_only_db_streams
            my_md_source/my_metadata_client_sqlite_transactions_only_multiple_databases_by_job_id_denormalized.cpp
            # my_md_source/my_metadata_client_sqlite_transactions_only_new.cpp
            # my_md_source/my_metadata_client_sqlite_transactions_only.cpp
            my_md_source/my_metadata_client_lua_functs.cpp  
            testing_source_class_proj/testing_harness_write_class_proj_sqlite_transactions_only_db_streams.cpp  
            # testing_source_class_proj/testing_harness_write_class_proj.cpp  
            testing_source_class_proj/testing_harness_debug_helper_functions.cpp
            testing_source_class_proj/testing_harness_read_class_proj_new.cpp 
            testing_source_class_proj/3d_read_for_testing_class_proj.cpp 
            testing_source_class_proj/extra_testing_class_proj.cpp 
            # testing_source_class_proj/extra_testing_helper_functions_class_proj.cpp
        )
        target_include_directories(testing_harness_class_proj_sqlite_transactions_only_db_streams PUBLIC ${PROJECT_SOURCE_DIR}/include/common_sqlite_transactions_only_db_streams)
        set_target_properties(testing_harness_class_proj_sqlite_transactions_only_db_streams PROPERTIES LINKER_LANGUAGE CXX )
        target_link_libraries(testing_harness_class_proj_sqlite_transactions_only_db_streams ${COMMON_LIBS} ${CLIENT_LIBS_SQLITE_TRANSACTION_ONLY_DB_STREAMS} )

        # add_executable(my_metadata_server_sqlite_transactions_only_db_streams my_md_source/my_metadata_server_sqlite_transactions_only_multiple_database_connections.cpp)
        add_executable(my_metadata_server_sqlite_transactions_only_db_streams my_md_source/my_metadata_server_sqlite_transactions_only_single_database_connection.cpp)
        target_include_directories(my_metadata_server_sqlite_transactions_only_db_streams PUBLIC ${PROJECT_SOURCE_DIR}/include/common_sqlite_transactions_only_db_streams)
        set_target_properties(my_metadata_server_sqlite_transactions_only_db_streams PROPERTIES LINKER_LANGUAGE CXX )
        target_link_libraries(my_metadata_server_sqlite_transactions_only_db_streams ${COMMON_LIBS} ${SERVER_LIBS_SQLITE_TRANSACTION_ONLY_DB_STREAMS} )

        # if(MAKE_LOCAL)
        #     if(MAKE_EMPRESS_TEST_CLIENT)
        #         add_executable(my_test_client_local_sqlite_transactions_only 
        #             debugging_source/my_test_client_local.cpp 
        #             my_md_source/my_metadata_client_local_sqlite_transactions_only.cpp 
        #             my_md_source/my_metadata_server_local.cpp
        #             debugging_source/helper_functions_test_client_local.C 
        #             debugging_source/helper_functions_2_test_client_local.cpp 
        #             debugging_source/helper_functions_3_test_client_local.cpp 
        #             my_md_source/my_metadata_client_lua_functs.cpp 
        #         )
        #         target_include_directories(my_test_client_local_sqlite_transactions_only PUBLIC ${PROJECT_SOURCE_DIR}/include/common_sqlite_transactions_only_multiple_databases_read_and_write)
        #         set_target_properties(my_test_client_local_sqlite_transactions_only PROPERTIES LINKER_LANGUAGE CXX )
        #         target_link_libraries(my_test_client_local_sqlite_transactions_only ${LOCAL_LIBS_SQLITE_TRANSACTION_ONLY} ${COMMON_LIBS} )
        #     endif(MAKE_EMPRESS_TEST_CLIENT)

        #     if(MAKE_CLASS_PROJ_HARNESS)
        #         add_executable(testing_harness_class_proj_local_sqlite_transactions_only
        #             my_md_source/my_metadata_client_local_sqlite_transactions_only.cpp 
        #             my_md_source/my_metadata_server_local.cpp
        #             my_md_source/my_metadata_client_lua_functs.cpp  
        #             testing_source_class_proj/testing_harness_write_class_proj_local.cpp  
        #             testing_source_class_proj/testing_harness_debug_helper_functions.cpp
        #             testing_source_class_proj/testing_harness_read_class_proj_local.cpp 
        #             testing_source_class_proj/3d_read_for_testing_class_proj_local.cpp 
        #             testing_source_class_proj/extra_testing_class_proj_local.cpp 
        #             # testing_source_class_proj/extra_testing_helper_functions_class_proj.cpp
        #         )
        #         target_include_directories(testing_harness_class_proj_local_sqlite_transactions_only PUBLIC ${PROJECT_SOURCE_DIR}/include/common_sqlite_transactions_only_multiple_databases_read_and_write)
        #         set_target_properties(testing_harness_class_proj_local_sqlite_transactions_only PROPERTIES LINKER_LANGUAGE CXX )
        #         #note - we have to include the common libs, server ops, common ops for now since we have all of the 
        #         #sqlite server interaction code left in the server op files
        #         target_link_libraries(testing_harness_class_proj_local_sqlite_transactions_only ${LOCAL_LIBS_SQLITE_TRANSACTION_ONLY} ${COMMON_LIBS} )
        #     endif(MAKE_CLASS_PROJ_HARNESS)
        # endif(MAKE_LOCAL)


    endif(MAKE_CLASS_PROJ_SQLITE_TRANSACTIONS_ONLY)

    if(MAKE_CLASS_PROJ_D2T_TRANSACTIONS_ONLY)

        add_executable(testing_harness_class_proj_d2t_transactions_only
            my_md_source/my_metadata_client_denormalized.cpp 
            my_md_source/my_metadata_client_lua_functs.cpp  
            testing_source_class_proj/testing_harness_write_class_proj_sqlite_transactions_only.cpp  
            testing_source_class_proj/testing_harness_debug_helper_functions.cpp
            testing_source_class_proj/testing_harness_read_class_proj_new.cpp 
            testing_source_class_proj/3d_read_for_testing_class_proj.cpp 
            testing_source_class_proj/extra_testing_class_proj.cpp 
            # testing_source_class_proj/extra_testing_helper_functions_class_proj.cpp
        )
        target_include_directories(testing_harness_class_proj_d2t_transactions_only PUBLIC ${PROJECT_SOURCE_DIR}/include/common_denormalized)
        set_target_properties(testing_harness_class_proj_d2t_transactions_only PROPERTIES LINKER_LANGUAGE CXX )
        target_link_libraries(testing_harness_class_proj_d2t_transactions_only ${COMMON_LIBS} ${CLIENT_LIBS_D2T_TRANSACTION_ONLY} )

        add_executable(my_metadata_server_d2t_transactions_only my_md_source/my_metadata_server.C)
        target_include_directories(my_metadata_server_d2t_transactions_only PUBLIC ${PROJECT_SOURCE_DIR}/include/common_denormalized)
        set_target_properties(my_metadata_server_d2t_transactions_only PROPERTIES LINKER_LANGUAGE CXX )
        target_link_libraries(my_metadata_server_d2t_transactions_only ${COMMON_LIBS} ${SERVER_LIBS_D2T_TRANSACTION_ONLY} )

        if(MAKE_EMPRESS_TEST_CLIENT)
            add_executable(my_test_client_d2t_transactions_only 
                my_md_source/my_metadata_client_denormalized.cpp 
                debugging_source/my_test_client_sqlite_transactions_only_db_streams.cpp
                debugging_source/helper_functions_test_client_sqlite_transactions_only_db_streams.cpp
                debugging_source/helper_functions_2_test_client_sqlite_transactions_only_db_streams.cpp
                debugging_source/helper_functions_3_test_client.cpp 
                my_md_source/my_metadata_client_lua_functs.cpp 
            )
            target_include_directories(my_test_client_d2t_transactions_only PUBLIC ${PROJECT_SOURCE_DIR}/include/common_denormalized)
            set_target_properties(my_test_client_d2t_transactions_only PROPERTIES LINKER_LANGUAGE CXX )
            target_link_libraries(my_test_client_d2t_transactions_only ${COMMON_LIBS} ${CLIENT_LIBS_D2T_TRANSACTION_ONLY} )

        endif(MAKE_EMPRESS_TEST_CLIENT)

    endif(MAKE_CLASS_PROJ_D2T_TRANSACTIONS_ONLY)

    if(MAKE_CLASS_PROJ_RTREE)
        add_executable(my_metadata_server_rtree my_md_source/my_metadata_server.C)
        target_include_directories(my_metadata_server_rtree PUBLIC ${PROJECT_SOURCE_DIR}/include/common)
        set_target_properties(my_metadata_server_rtree PROPERTIES LINKER_LANGUAGE CXX )
        target_link_libraries(my_metadata_server_rtree ${COMMON_LIBS} ${SERVER_LIBS_RTREE} )

    endif(MAKE_CLASS_PROJ_RTREE)

    if(MAKE_EMPRESS_HARNESS )
        if(MAKE_EMPRESS_WITH_HDF5) 
            add_executable(testing_harness_new_write 
                my_md_source/my_metadata_client.C 
                my_md_source/my_metadata_client_lua_functs.cpp  
                testing_source/testing_harness_new_write.cpp  
                testing_source/testing_harness_debug_helper_functions.cpp
                testing_source/hdf5_helper_functions_write.cpp
            )
            target_include_directories(testing_harness_new_write PUBLIC ${PROJECT_SOURCE_DIR}/include/common)
            set_target_properties(testing_harness_new_write PROPERTIES LINKER_LANGUAGE CXX )
            target_link_libraries(testing_harness_new_write ${COMMON_LIBS} ${CLIENT_LIBS} ${HDF5_C_LIBRARIES})

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
            target_include_directories(testing_harness_new_read PUBLIC ${PROJECT_SOURCE_DIR}/include/common)
            set_target_properties(testing_harness_new_read PROPERTIES LINKER_LANGUAGE CXX )
            target_link_libraries(testing_harness_new_read ${COMMON_LIBS} ${CLIENT_LIBS} ${HDF5_C_LIBRARIES})
        else(MAKE_EMPRESS_WITH_HDF5)
            add_executable(testing_harness_new_write 
                my_md_source/my_metadata_client.C 
                my_md_source/my_metadata_client_lua_functs.cpp  
                testing_source/testing_harness_new_write.cpp  
                testing_source/testing_harness_debug_helper_functions.cpp
            )
            target_include_directories(testing_harness_new_write PUBLIC ${PROJECT_SOURCE_DIR}/include/common)
            set_target_properties(testing_harness_new_write PROPERTIES LINKER_LANGUAGE CXX )
            target_link_libraries(testing_harness_new_write ${COMMON_LIBS} ${CLIENT_LIBS} )

            add_executable(testing_harness_new_read 
                my_md_source/my_metadata_client.C 
                my_md_source/my_metadata_client_lua_functs.cpp 
                testing_source/testing_harness_new_read.cpp 
                testing_source/3d_read_for_testing.cpp 
                testing_source/testing_harness_debug_helper_functions.cpp 
                testing_source/extra_testing_collective.cpp 
                testing_source/extra_testing_collective_helper_functions.cpp
            )
            target_include_directories(testing_harness_new_read PUBLIC ${PROJECT_SOURCE_DIR}/include/common)
            set_target_properties(testing_harness_new_read PROPERTIES LINKER_LANGUAGE CXX )
            target_link_libraries(testing_harness_new_read ${COMMON_LIBS} ${CLIENT_LIBS} )
        endif(MAKE_EMPRESS_WITH_HDF5)
    endif(MAKE_EMPRESS_HARNESS)

    if(MAKE_CLASS_PROJ_HARNESS)
        add_executable(testing_harness_class_proj
            my_md_source/my_metadata_client.C 
            my_md_source/my_metadata_client_lua_functs.cpp  
            testing_source_class_proj/testing_harness_write_class_proj.cpp  
            testing_source_class_proj/testing_harness_debug_helper_functions.cpp
            testing_source_class_proj/testing_harness_read_class_proj_new.cpp 
            testing_source_class_proj/3d_read_for_testing_class_proj.cpp 
            testing_source_class_proj/extra_testing_class_proj.cpp 
            # testing_source_class_proj/extra_testing_helper_functions_class_proj.cpp
        )
        target_include_directories(testing_harness_class_proj PUBLIC ${PROJECT_SOURCE_DIR}/include/common)
        set_target_properties(testing_harness_class_proj PROPERTIES LINKER_LANGUAGE CXX )
        target_link_libraries(testing_harness_class_proj ${COMMON_LIBS} ${CLIENT_LIBS} )

        if(MAKE_CLASS_PROJ_PREGENERATE)

            add_executable(testing_harness_class_proj_pregenerate
                my_md_source/my_metadata_client.C 
                my_md_source/my_metadata_client_lua_functs.cpp  
                testing_source_class_proj/testing_harness_write_class_proj_pregenerate.cpp  
                testing_source_class_proj/testing_harness_debug_helper_functions.cpp
                testing_source_class_proj/testing_harness_read_class_proj_new.cpp 
                testing_source_class_proj/3d_read_for_testing_class_proj.cpp 
                testing_source_class_proj/extra_testing_class_proj.cpp 
                # testing_source_class_proj/extra_testing_helper_functions_class_proj.cpp
            )
            target_include_directories(testing_harness_class_proj_pregenerate PUBLIC ${PROJECT_SOURCE_DIR}/include/common)
            set_target_properties(testing_harness_class_proj_pregenerate PROPERTIES LINKER_LANGUAGE CXX )
            target_link_libraries(testing_harness_class_proj_pregenerate ${COMMON_LIBS} ${CLIENT_LIBS} )
        endif(MAKE_CLASS_PROJ_PREGENERATE)

        # add_executable(db_check
        #   testing_source_class_proj/db_check.cpp  
        # )
        # target_include_directories(db_check PUBLIC ${PROJECT_SOURCE_DIR}/include/common)# 
        # set_target_properties(db_check PROPERTIES LINKER_LANGUAGE CXX )
     #      target_link_libraries(db_check ${COMMON_LIBS} ${SERVER_LIBS})

        if(MAKE_HDF5_CLASS_PROJ_HARNESS)
            add_executable(testing_harness_class_proj_hdf5
                testing_source_hdf5/my_metadata_client_hdf5.cpp
                testing_source_class_proj/testing_harness_write_class_proj_hdf5.cpp  
                testing_source_class_proj/testing_harness_read_class_proj_hdf5.cpp 
                testing_source_class_proj/3d_read_for_testing_class_proj_hdf5.cpp 
                testing_source_class_proj/extra_testing_class_proj_hdf5.cpp 
                testing_source_class_proj/testing_harness_helper_functions_class_proj_hdf5.cpp 
                # testing_source_class_proj/extra_testing_helper_functions_class_proj.cpp
            )
            target_include_directories(testing_harness_class_proj_hdf5 PUBLIC ${PROJECT_SOURCE_DIR}/include/common)
            set_target_properties(testing_harness_class_proj_hdf5 PROPERTIES LINKER_LANGUAGE CXX )
            target_link_libraries(testing_harness_class_proj_hdf5 ${HDF5_C_LIBRARIES} ${BOOST_MPI_LIBS} )
        endif(MAKE_HDF5_CLASS_PROJ_HARNESS)
        # add_executable(testing_harness_read_class_proj
        #   my_md_source/my_metadata_client.C 
        #   my_md_source/my_metadata_client_lua_functs.cpp 
        #   testing_source_class_proj/testing_harness_read_class_proj_new.cpp 
        #   testing_source_class_proj/3d_read_for_testing_class_proj.cpp 
        #   testing_source_class_proj/testing_harness_debug_helper_functions.cpp 
        #   testing_source_class_proj/extra_testing_class_proj.cpp 
        #   # testing_source_class_proj/extra_testing_helper_functions_class_proj.cpp
        # )
        # target_include_directories(testing_harness_read_class_proj PUBLIC ${PROJECT_SOURCE_DIR}/include/common)# 
        # set_target_properties(testing_harness_read_class_proj PROPERTIES LINKER_LANGUAGE CXX )
        # target_link_libraries(testing_harness_read_class_proj ${COMMON_LIBS} ${CLIENT_LIBS}  )

        if(MAKE_LARGE_MD_VOL_HARNESS)
            add_executable(testing_harness_large_md_vol
                my_md_source/my_metadata_client.C 
                my_md_source/my_metadata_client_lua_functs.cpp  
                testing_source_class_proj/testing_harness_write_class_proj_large_md_volume.cpp  
                testing_source_class_proj/testing_harness_debug_helper_functions.cpp
                testing_source_class_proj/testing_harness_read_class_proj_large_md_volume.cpp 
                testing_source_class_proj/3d_read_for_testing_class_proj.cpp 
                testing_source_class_proj/extra_testing_class_proj.cpp 
                # testing_source_class_proj/extra_testing_class_proj_large_md_volume.cpp 
                # testing_source_class_proj/extra_testing_helper_functions_class_proj.cpp
            )
            target_include_directories(testing_harness_large_md_vol PUBLIC ${PROJECT_SOURCE_DIR}/include/common)
            set_target_properties(testing_harness_large_md_vol PROPERTIES LINKER_LANGUAGE CXX )
            target_link_libraries(testing_harness_large_md_vol ${COMMON_LIBS} ${CLIENT_LIBS}  )
        endif(MAKE_LARGE_MD_VOL_HARNESS)

    endif(MAKE_CLASS_PROJ_HARNESS)

    if(MAKE_LOCAL)
        if(MAKE_EMPRESS_HARNESS)
            if(MAKE_EMPRESS_WITH_HDF5) 
                add_executable(testing_harness_write_local 
                    my_md_source/my_metadata_client_local.cpp 
                    my_md_source/my_metadata_client_lua_functs.cpp  
                    my_md_source/my_metadata_server_local.cpp
                    testing_source_local/testing_harness_new_new_write_local.cpp  
                    testing_source/testing_harness_debug_helper_functions.cpp 
                    testing_source/hdf5_helper_functions_write.cpp
                )
                target_include_directories(testing_harness_write_local PUBLIC ${PROJECT_SOURCE_DIR}/include/common)
                set_target_properties(testing_harness_write_local PROPERTIES LINKER_LANGUAGE CXX )
                target_link_libraries(testing_harness_write_local ${LOCAL_LIBS} ${HDF5_C_LIBRARIES})

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
                target_include_directories(testing_harness_read_local PUBLIC ${PROJECT_SOURCE_DIR}/include/common)
                set_target_properties(testing_harness_read_local PROPERTIES LINKER_LANGUAGE CXX )
                target_link_libraries(testing_harness_read_local ${LOCAL_LIBS} ${HDF5_C_LIBRARIES})
            elseif(MAKE_EMPRESS_WITH_HDF5)
                add_executable(testing_harness_write_local 
                    my_md_source/my_metadata_client_local.cpp 
                    my_md_source/my_metadata_client_lua_functs.cpp  
                    my_md_source/my_metadata_server_local.cpp
                    testing_source_local/testing_harness_new_new_write_local.cpp  
                    testing_source/testing_harness_debug_helper_functions.cpp 
                )
                target_include_directories(testing_harness_write_local PUBLIC ${PROJECT_SOURCE_DIR}/include/common)
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
                target_include_directories(testing_harness_read_local PUBLIC ${PROJECT_SOURCE_DIR}/include/common)
                set_target_properties(testing_harness_read_local PROPERTIES LINKER_LANGUAGE CXX )
                target_link_libraries(testing_harness_read_local ${LOCAL_LIBS})

            endif(MAKE_EMPRESS_WITH_HDF5)
        endif(MAKE_EMPRESS_HARNESS)

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
            target_include_directories(my_test_client_local PUBLIC ${PROJECT_SOURCE_DIR}/include/common)
            set_target_properties(my_test_client_local PROPERTIES LINKER_LANGUAGE CXX )
            target_link_libraries(my_test_client_local ${LOCAL_LIBS} ${COMMON_LIBS} )
        endif(MAKE_EMPRESS_TEST_CLIENT)

        if(MAKE_CLASS_PROJ_HARNESS)
            add_executable(testing_harness_class_proj_local
                my_md_source/my_metadata_client_local.cpp 
                my_md_source/my_metadata_server_local.cpp
                my_md_source/my_metadata_client_lua_functs.cpp  
                testing_source_class_proj/testing_harness_write_class_proj_local.cpp  
                testing_source_class_proj/testing_harness_debug_helper_functions.cpp
                testing_source_class_proj/testing_harness_read_class_proj_local.cpp 
                testing_source_class_proj/3d_read_for_testing_class_proj_local.cpp 
                testing_source_class_proj/extra_testing_class_proj_local.cpp 
                # testing_source_class_proj/extra_testing_helper_functions_class_proj.cpp
            )
            target_include_directories(testing_harness_class_proj_local PUBLIC ${PROJECT_SOURCE_DIR}/include/common)
            set_target_properties(testing_harness_class_proj_local PROPERTIES LINKER_LANGUAGE CXX )
            #note - we have to include the common libs, server ops, common ops for now since we have all of the 
            #sqlite server interaction code left in the server op files
            target_link_libraries(testing_harness_class_proj_local ${LOCAL_LIBS} ${COMMON_LIBS} )
        endif(MAKE_CLASS_PROJ_HARNESS)

    endif(MAKE_LOCAL)

endif(MAKE_EMPRESS)


if(MAKE_HDF5_HARNESS)
    add_executable(testing_harness_write_hdf5 testing_source_hdf5/testing_harness_write_hdf5.cpp testing_source_hdf5/my_metadata_client_hdf5.cpp 
        testing_source_hdf5/testing_harness_helper_functions_hdf5.cpp )
    target_include_directories(testing_harness_write_hdf5 PUBLIC ${PROJECT_SOURCE_DIR}/include/common ${HDF5_INCLUDE_DIRS} )
    set_target_properties(testing_harness_write_hdf5 PROPERTIES LINKER_LANGUAGE CXX )
    target_link_libraries(testing_harness_write_hdf5 ${HDF5_C_LIBRARIES} ${BOOST_MPI_LIBS})

    add_executable(testing_harness_read_hdf5 testing_source_hdf5/testing_harness_read_hdf5.cpp testing_source_hdf5/3d_read_for_testing_hdf5.cpp  
        testing_source_hdf5/extra_testing_collective_hdf5.cpp testing_source_hdf5/extra_testing_collective_helper_functions_hdf5.cpp 
        testing_source_hdf5/my_metadata_client_hdf5.cpp testing_source_hdf5/testing_harness_helper_functions_hdf5.cpp )
    target_include_directories(testing_harness_read_hdf5 PUBLIC ${PROJECT_SOURCE_DIR}/include/common ${HDF5_INCLUDE_DIRS} )
    set_target_properties(testing_harness_read_hdf5 PROPERTIES LINKER_LANGUAGE CXX )
    target_link_libraries(testing_harness_read_hdf5 ${HDF5_C_LIBRARIES} ${BOOST_MPI_LIBS})

    if(MAKE_HDF5_TEST_CLIENT)

        add_executable(my_test_client_hdf5 debugging_source_hdf5/my_test_client_hdf5.cpp debugging_source_hdf5/helper_functions_test_client_hdf5.cpp 
            testing_source_hdf5/my_metadata_client_hdf5.cpp )
        target_include_directories(my_test_client_hdf5 PUBLIC ${PROJECT_SOURCE_DIR}/include/common ${HDF5_INCLUDE_DIRS} )
        set_target_properties(my_test_client_hdf5 PROPERTIES LINKER_LANGUAGE CXX )
        target_link_libraries(my_test_client_hdf5 ${HDF5_C_LIBRARIES} ${BOOST_MPI_LIBS})
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
# message(STATUS "Flags are ${CMAKE_CXX_FLAGS}")    
message(STATUS "Opbox_INCLUDE_DIRS     ${Opbox_INCLUDE_DIRS}")
# message(STATUS "Opbox_LIBRARIES        ${Opbox_LIBRARIES}")

message( STATUS "GTEST_INCLUDE_DIRS ${GTEST_INCLUDE_DIRS}" )
message( STATUS "GTEST_LIBRARIES    ${GTEST_LIBRARIES}"    )
# message( STATUS "GTEST_SOURCE_DIR   ${GTEST_SOURCE_DIR}"   )


message( STATUS "HDF5_INCLUDE_DIRS   ${HDF5_1_12_INCLUDE_DIRS}"   )
message( STATUS "HDF5_C_LIBRARIES   ${HDF5_1_12_C_LIBRARIES}"   )
message( STATUS "CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")
message(STATUS "MPI_CXX_FOUND = ${MPI_CXX_FOUND}")
message(STATUS "MPI_CXX = $MPI::MPI_CXX")
# message(STATUS "COMMON_LIBS        ${COMMON_LIBS}")
