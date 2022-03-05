
project(lib VERSION 0.1 LANGUAGES CXX)


# add_library(Database SHARED
#     database/sqlite3.c
# )
add_library(Database
    database/sqlite3.c
)

target_compile_definitions (Database PRIVATE SQLITE_ENABLE_RTREE=1)
TARGET_INCLUDE_DIRECTORIES(Database PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../include/database
)

if(MAKE_EMPRESS)

    set(CLIENT_OPS
        ops_rdma/common/OpCoreCommon.cpp  
        ops_rdma/client/OpCoreClient.cpp

        # ops_rdma/client_sqlite_transactions_only/OpAbortTransactionMetaClient.cpp
        # ops_rdma/client_sqlite_transactions_only/OpBeginTransactionMetaClient.cpp       
        # ops_rdma/client_sqlite_transactions_only/OpCommitTransactionMetaClient.cpp 

        ops_rdma/client/OpCheckpointDatabaseMetaClient.cpp

        ops_rdma/client/OpInsertTimestepAttributeBatchMetaClient.cpp
        ops_rdma/client/OpInsertRunAttributeBatchMetaClient.cpp
        ops_rdma/client/OpCreateVarBatchMetaClient.cpp
        ops_rdma/client/OpCreateTypeBatchMetaClient.cpp
        ops_rdma/client/OpInsertVarAttributeByDimsBatchMetaClient.cpp
        ops_rdma/client/OpDeleteAllVarsWithSubstrMetaClient.cpp
        ops_rdma/client/OpCatalogAllVarAttributesWithVarSubstrDimsMetaClient.cpp
        ops_rdma/client/OpCatalogAllVarAttributesWithTypeVarSubstrDimsMetaClient.cpp
        ops_rdma/client/OpCatalogAllVarAttributesWithTypeVarSubstrMetaClient.cpp
        ops_rdma/client/OpCatalogAllVarAttributesWithTypeVarSubstrRangeMetaClient.cpp
        ops_rdma/client/OpCatalogAllVarAttributesWithTypeVarSubstrDimsRangeMetaClient.cpp
        ops_rdma/client/OpCatalogAllTypesWithVarAttributesWithVarSubstrInTimestepMetaClient.cpp
        ops_rdma/client/OpCatalogAllTypesWithVarAttributesWithVarSubstrDimsInTimestepMetaClient.cpp
        ops_rdma/client/OpCatalogAllTimestepsWithVarSubstrMetaClient.cpp
        ops_rdma/client/OpCatalogAllTimestepsWithVarAttributesWithTypeVarSubstrRangeMetaClient.cpp
        ops_rdma/client/OpCatalogAllTimestepsWithVarAttributesWithTypeVarSubstrMetaClient.cpp
        ops_rdma/client/OpCatalogAllTimestepsWithVarAttributesWithTypeVarSubstrDimsRangeMetaClient.cpp
        ops_rdma/client/OpCatalogAllTimestepsWithVarAttributesWithTypeVarSubstrDimsMetaClient.cpp
        ops_rdma/client/OpCatalogAllVarAttributesWithVarSubstrMetaClient.cpp
        ops_rdma/client/OpProcessingMetaClient.cpp
        # ops_rdma/client/OpActivateMetaClient.cpp
        ops_rdma/client/OpActivateRunAttributeMetaClient.cpp
        ops_rdma/client/OpActivateRunMetaClient.cpp          
        ops_rdma/client/OpActivateTimestepAttributeMetaClient.cpp
        ops_rdma/client/OpActivateTimestepMetaClient.cpp
        ops_rdma/client/OpActivateTypeMetaClient.cpp   
        ops_rdma/client/OpActivateVarAttributeMetaClient.cpp          
        ops_rdma/client/OpActivateVarMetaClient.cpp
        ops_rdma/client/OpCatalogAllVarAttributesWithTypeVarDimsRangeMetaClient.cpp
        ops_rdma/client/OpCatalogAllVarAttributesWithTypeVarRangeMetaClient.cpp
        ops_rdma/client/OpCatalogAllTimestepAttributesWithTypeRangeMetaClient.cpp
        ops_rdma/client/OpCatalogAllTimestepsWithVarAttributesWithTypeVarDimsRangeMetaClient.cpp
        ops_rdma/client/OpCatalogAllTimestepsWithVarAttributesWithTypeVarRangeMetaClient.cpp
        ops_rdma/client/OpCatalogAllRunAttributesWithTypeRangeMetaClient.cpp
        ops_rdma/client/OpCatalogAllRunAttributesMetaClient.cpp
        ops_rdma/client/OpCatalogAllRunAttributesWithTypeMetaClient.cpp
        ops_rdma/client/OpCatalogAllTimestepAttributesMetaClient.cpp
        ops_rdma/client/OpCatalogAllTimestepAttributesWithTypeMetaClient.cpp
        ops_rdma/client/OpCatalogAllTimestepsWithVarAttributesWithTypeVarDimsMetaClient.cpp
        ops_rdma/client/OpCatalogAllTimestepsWithVarAttributesWithTypeVarMetaClient.cpp
        ops_rdma/client/OpCatalogAllTimestepsWithVarMetaClient.cpp
        ops_rdma/client/OpCatalogAllTypesWithVarAttributesInTimestepMetaClient.cpp
        ops_rdma/client/OpCatalogAllTypesWithVarAttributesWithVarDimsInTimestepMetaClient.cpp
        ops_rdma/client/OpCatalogAllTypesWithVarAttributesWithVarInTimestepMetaClient.cpp
        ops_rdma/client/OpCatalogAllVarAttributesMetaClient.cpp 
        ops_rdma/client/OpCatalogAllVarAttributesWithDimsMetaClient.cpp 
        ops_rdma/client/OpCatalogAllVarAttributesWithTypeByIdMetaClient.cpp 
        ops_rdma/client/OpCatalogAllVarAttributesWithTypeByNameVerMetaClient.cpp 
        ops_rdma/client/OpCatalogAllVarAttributesWithTypeDimsByIdMetaClient.cpp 
        ops_rdma/client/OpCatalogAllVarAttributesWithTypeDimsByNameVerMetaClient.cpp 
        ops_rdma/client/OpCatalogAllVarAttributesWithTypeVarByIdMetaClient.cpp 
        ops_rdma/client/OpCatalogAllVarAttributesWithTypeVarByNameVerMetaClient.cpp 
        ops_rdma/client/OpCatalogAllVarAttributesWithTypeVarDimsByIdMetaClient.cpp 
        ops_rdma/client/OpCatalogAllVarAttributesWithTypeVarDimsByNameVerMetaClient.cpp 
        ops_rdma/client/OpCatalogAllVarAttributesWithVarByIdMetaClient.cpp 
        ops_rdma/client/OpCatalogAllVarAttributesWithVarByNameVerMetaClient.cpp
        ops_rdma/client/OpCatalogAllVarAttributesWithVarDimsByIdMetaClient.cpp 
        ops_rdma/client/OpCatalogAllVarAttributesWithVarDimsByNameVerMetaClient.cpp    
        ops_rdma/client/OpCatalogRunMetaClient.cpp             
        ops_rdma/client/OpCatalogTimestepMetaClient.cpp
        ops_rdma/client/OpCatalogTypeMetaClient.cpp             
        ops_rdma/client/OpCatalogVarMetaClient.cpp             
        ops_rdma/client/OpCreateRunMetaClient.cpp           
        ops_rdma/client/OpCreateTimestepMetaClient.cpp
        ops_rdma/client/OpCreateTypeMetaClient.cpp              
        ops_rdma/client/OpCreateVarMetaClient.cpp       
        ops_rdma/client/OpDeleteRunByIdMetaClient.cpp  
        ops_rdma/client/OpDeleteTimestepByIdMetaClient.cpp
        ops_rdma/client/OpDeleteTypeByIdMetaClient.cpp   
        ops_rdma/client/OpDeleteTypeByNameVerMetaClient.cpp           
        ops_rdma/client/OpDeleteVarByIdMetaClient.cpp  
        ops_rdma/client/OpDeleteVarByNamePathVerMetaClient.cpp  
        ops_rdma/client/OpFullShutdownMetaClient.cpp       
        ops_rdma/client/OpInsertRunAttributeMetaClient.cpp
        ops_rdma/client/OpInsertTimestepAttributeMetaClient.cpp
        ops_rdma/client/OpInsertVarAttributeByDimsMetaClient.cpp
    )

    add_library(OpsServer
        ops_rdma/common/OpCoreCommon.cpp  
        ops_rdma/server/OpCoreServer.cpp  

        ops_rdma/server/sql_helper_functs.cpp

        # ops_rdma/server_sqlite_transactions_only_new/OpAbortTransactionMetaServer.cpp
        # ops_rdma/server_sqlite_transactions_only_new/OpBeginTransactionMetaServer.cpp       
        # ops_rdma/server_sqlite_transactions_only_new/OpCommitTransactionMetaServer.cpp

        ops_rdma/server/OpCheckpointDatabaseMetaServer.cpp
        ops_rdma/server/OpInsertTimestepAttributeBatchMetaServer.cpp
        ops_rdma/server/OpInsertRunAttributeBatchMetaServer.cpp
        ops_rdma/server/OpCreateVarBatchMetaServer.cpp
        ops_rdma/server/OpCreateTypeBatchMetaServer.cpp
        ops_rdma/server/OpInsertVarAttributeByDimsBatchMetaServer.cpp
        ops_rdma/server/OpDeleteAllVarsWithSubstrMetaServer.cpp
        ops_rdma/server/OpCatalogAllVarAttributesWithVarSubstrDimsMetaServer.cpp
        ops_rdma/server/OpCatalogAllVarAttributesWithTypeVarSubstrDimsMetaServer.cpp
        ops_rdma/server/OpCatalogAllVarAttributesWithTypeVarSubstrMetaServer.cpp
        ops_rdma/server/OpCatalogAllVarAttributesWithTypeVarSubstrRangeMetaServer.cpp
        ops_rdma/server/OpCatalogAllVarAttributesWithTypeVarSubstrDimsRangeMetaServer.cpp
        ops_rdma/server/OpCatalogAllTypesWithVarAttributesWithVarSubstrInTimestepMetaServer.cpp
        ops_rdma/server/OpCatalogAllTypesWithVarAttributesWithVarSubstrDimsInTimestepMetaServer.cpp
        ops_rdma/server/OpCatalogAllTimestepsWithVarSubstrMetaServer.cpp
        ops_rdma/server/OpCatalogAllTimestepsWithVarAttributesWithTypeVarSubstrRangeMetaServer.cpp
        ops_rdma/server/OpCatalogAllTimestepsWithVarAttributesWithTypeVarSubstrMetaServer.cpp
        ops_rdma/server/OpCatalogAllTimestepsWithVarAttributesWithTypeVarSubstrDimsRangeMetaServer.cpp
        ops_rdma/server/OpCatalogAllTimestepsWithVarAttributesWithTypeVarSubstrDimsMetaServer.cpp
        ops_rdma/server/OpCatalogAllVarAttributesWithVarSubstrMetaServer.cpp
        ops_rdma/server/OpProcessingMetaServer.cpp
        # ops_rdma/server/OpActivateMetaServer.cpp
        ops_rdma/server/OpActivateRunAttributeMetaServer.cpp
        ops_rdma/server/OpActivateRunMetaServer.cpp          
        ops_rdma/server/OpActivateTimestepAttributeMetaServer.cpp
        ops_rdma/server/OpActivateTimestepMetaServer.cpp
        ops_rdma/server/OpActivateTypeMetaServer.cpp   
        ops_rdma/server/OpActivateVarAttributeMetaServer.cpp          
        ops_rdma/server/OpActivateVarMetaServer.cpp
        ops_rdma/server/OpCatalogAllVarAttributesWithTypeVarDimsRangeMetaServer.cpp
        ops_rdma/server/OpCatalogAllVarAttributesWithTypeVarRangeMetaServer.cpp
        ops_rdma/server/OpCatalogAllTimestepAttributesWithTypeRangeMetaServer.cpp
        ops_rdma/server/OpCatalogAllTimestepsWithVarAttributesWithTypeVarDimsRangeMetaServer.cpp
        ops_rdma/server/OpCatalogAllTimestepsWithVarAttributesWithTypeVarRangeMetaServer.cpp
        ops_rdma/server/OpCatalogAllRunAttributesWithTypeRangeMetaServer.cpp
        ops_rdma/server/OpCatalogAllRunAttributesMetaServer.cpp
        ops_rdma/server/OpCatalogAllRunAttributesWithTypeMetaServer.cpp
        ops_rdma/server/OpCatalogAllTimestepAttributesMetaServer.cpp
        ops_rdma/server/OpCatalogAllTimestepAttributesWithTypeMetaServer.cpp
        ops_rdma/server/OpCatalogAllTimestepsWithVarAttributesWithTypeVarDimsMetaServer.cpp
        ops_rdma/server/OpCatalogAllTimestepsWithVarAttributesWithTypeVarMetaServer.cpp
        ops_rdma/server/OpCatalogAllTimestepsWithVarMetaServer.cpp
        ops_rdma/server/OpCatalogAllTypesWithVarAttributesInTimestepMetaServer.cpp
        ops_rdma/server/OpCatalogAllTypesWithVarAttributesWithVarDimsInTimestepMetaServer.cpp
        ops_rdma/server/OpCatalogAllTypesWithVarAttributesWithVarInTimestepMetaServer.cpp
        ops_rdma/server/OpCatalogAllVarAttributesMetaServer.cpp 
        ops_rdma/server/OpCatalogAllVarAttributesWithDimsMetaServer.cpp 
        ops_rdma/server/OpCatalogAllVarAttributesWithTypeByIdMetaServer.cpp 
        ops_rdma/server/OpCatalogAllVarAttributesWithTypeByNameVerMetaServer.cpp 
        ops_rdma/server/OpCatalogAllVarAttributesWithTypeDimsByIdMetaServer.cpp 
        ops_rdma/server/OpCatalogAllVarAttributesWithTypeDimsByNameVerMetaServer.cpp 
        ops_rdma/server/OpCatalogAllVarAttributesWithTypeVarByIdMetaServer.cpp 
        ops_rdma/server/OpCatalogAllVarAttributesWithTypeVarByNameVerMetaServer.cpp 
        ops_rdma/server/OpCatalogAllVarAttributesWithTypeVarDimsByIdMetaServer.cpp 
        ops_rdma/server/OpCatalogAllVarAttributesWithTypeVarDimsByNameVerMetaServer.cpp 
        ops_rdma/server/OpCatalogAllVarAttributesWithVarByIdMetaServer.cpp 
        ops_rdma/server/OpCatalogAllVarAttributesWithVarByNameVerMetaServer.cpp
        ops_rdma/server/OpCatalogAllVarAttributesWithVarDimsByIdMetaServer.cpp 
        ops_rdma/server/OpCatalogAllVarAttributesWithVarDimsByNameVerMetaServer.cpp    
        ops_rdma/server/OpCatalogRunMetaServer.cpp             
        ops_rdma/server/OpCatalogTimestepMetaServer.cpp
        ops_rdma/server/OpCatalogTypeMetaServer.cpp             
        ops_rdma/server/OpCatalogVarMetaServer.cpp             
        ops_rdma/server/OpCreateRunMetaServer.cpp           
        ops_rdma/server/OpCreateTimestepMetaServer.cpp
        ops_rdma/server/OpCreateTypeMetaServer.cpp              
        ops_rdma/server/OpCreateVarMetaServer.cpp    
        ops_rdma/server/OpDeleteRunByIdMetaServer.cpp  
        ops_rdma/server/OpDeleteTimestepByIdMetaServer.cpp
        ops_rdma/server/OpDeleteTypeByIdMetaServer.cpp   
        ops_rdma/server/OpDeleteTypeByNameVerMetaServer.cpp           
        ops_rdma/server/OpDeleteVarByIdMetaServer.cpp  
        ops_rdma/server/OpDeleteVarByNamePathVerMetaServer.cpp  
        ops_rdma/server/OpFullShutdownMetaServer.cpp       
        ops_rdma/server/OpInsertRunAttributeMetaServer.cpp
        ops_rdma/server/OpInsertTimestepAttributeMetaServer.cpp
        ops_rdma/server/OpInsertVarAttributeByDimsMetaServer.cpp 
    )

    TARGET_INCLUDE_DIRECTORIES(OpsServer PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/../include/ops_rdma
        # ${CMAKE_CURRENT_SOURCE_DIR}/../include/ops_rdma_sqlite_transactions_only
        # ${CMAKE_CURRENT_SOURCE_DIR}/../include/ops_rdma_server
        ${CMAKE_CURRENT_SOURCE_DIR}/../include/common
        ${CMAKE_CURRENT_SOURCE_DIR}/../include/database
        ${CMAKE_CURRENT_SOURCE_DIR}/../include/server
    )

    add_library(OpsClient
       ${CLIENT_OPS}             
    )

    TARGET_INCLUDE_DIRECTORIES(OpsClient PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/../include/ops_rdma
        # ${CMAKE_CURRENT_SOURCE_DIR}/../include/ops_rdma_sqlite_transactions_only
        # ${CMAKE_CURRENT_SOURCE_DIR}/../include/ops_rdma_client
        ${CMAKE_CURRENT_SOURCE_DIR}/../include/common
        ${CMAKE_CURRENT_SOURCE_DIR}/../include/client
    )

    add_library(FullShutdownOpForDirman
        # ops_rdma/common/OpFullShutdownMetaCommon.cpp 
        ops_rdma/common/OpCoreCommon.cpp
        ops_rdma/server/OpCoreServer.cpp  
        ops_rdma/server/OpFullShutdownMetaServer.cpp

    )
    TARGET_INCLUDE_DIRECTORIES(FullShutdownOpForDirman PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/../include/ops_rdma
        ${CMAKE_CURRENT_SOURCE_DIR}/../include/common
    )

   if(MAKE_CLASS_PROJ_SQLITE_TRANSACTIONS_ONLY)
        add_library(OpsServerSqliteTransactionOnly
            ops_rdma/common/OpCoreCommon.cpp  

            ops_rdma/server_sqlite_transactions_only_multiple_databases_write_ahead_log_denormalized/OpCoreServer.cpp  

            # #only needed since we are using a single metadata_server source code rather than
            # #a dedicated transaction one
            ops_rdma/server_sqlite_transactions_only_multiple_databases_write_ahead_log_denormalized/OpProcessingMetaServer.cpp
            # ops_rdma/server/OpActivateMetaServer.cpp
            ops_rdma/server_sqlite_transactions_only_multiple_databases_write_ahead_log_denormalized/OpActivateRunAttributeMetaServer.cpp
            ops_rdma/server_sqlite_transactions_only_multiple_databases_write_ahead_log_denormalized/OpActivateRunMetaServer.cpp          
            ops_rdma/server_sqlite_transactions_only_multiple_databases_write_ahead_log_denormalized/OpActivateTimestepAttributeMetaServer.cpp
            ops_rdma/server_sqlite_transactions_only_multiple_databases_write_ahead_log_denormalized/OpActivateTimestepMetaServer.cpp
            ops_rdma/server_sqlite_transactions_only_multiple_databases_write_ahead_log_denormalized/OpActivateTypeMetaServer.cpp   
            ops_rdma/server_sqlite_transactions_only_multiple_databases_write_ahead_log_denormalized/OpActivateVarAttributeMetaServer.cpp          
            ops_rdma/server_sqlite_transactions_only_multiple_databases_write_ahead_log_denormalized/OpActivateVarMetaServer.cpp

            ops_rdma/server_sqlite_transactions_only_multiple_databases_write_ahead_log_denormalized/sql_helper_functs.cpp

            # ops_rdma/server_sqlite_transactions_only_multiple_databases_write_ahead_log_denormalized/OpAbortTransactionMetaServer.cpp
            # ops_rdma/server_sqlite_transactions_only_multiple_databases_write_ahead_log_denormalized/OpBeginTransactionMetaServer.cpp       
            # ops_rdma/server_sqlite_transactions_only_multiple_databases_write_ahead_log_denormalized/OpCommitTransactionMetaServer.cpp

            ops_rdma/server_sqlite_transactions_only_multiple_databases_write_ahead_log_denormalized/OpCheckpointDatabaseMetaServer.cpp
            ops_rdma/server_sqlite_transactions_only_multiple_databases_write_ahead_log_denormalized/OpInsertTimestepAttributeBatchMetaServer.cpp
            ops_rdma/server_sqlite_transactions_only_multiple_databases_write_ahead_log_denormalized/OpInsertRunAttributeBatchMetaServer.cpp
            ops_rdma/server_sqlite_transactions_only_multiple_databases_write_ahead_log_denormalized/OpCreateVarBatchMetaServer.cpp
            ops_rdma/server_sqlite_transactions_only_multiple_databases_write_ahead_log_denormalized/OpCreateTypeBatchMetaServer.cpp
            ops_rdma/server_sqlite_transactions_only_multiple_databases_write_ahead_log_denormalized/OpInsertVarAttributeByDimsBatchMetaServer.cpp
            ops_rdma/server_sqlite_transactions_only_multiple_databases_write_ahead_log_denormalized/OpDeleteAllVarsWithSubstrMetaServer.cpp
            ops_rdma/server_sqlite_transactions_only_multiple_databases_write_ahead_log_denormalized/OpCatalogAllVarAttributesWithVarSubstrDimsMetaServer.cpp
            ops_rdma/server_sqlite_transactions_only_multiple_databases_write_ahead_log_denormalized/OpCatalogAllVarAttributesWithTypeVarSubstrDimsMetaServer.cpp
            ops_rdma/server_sqlite_transactions_only_multiple_databases_write_ahead_log_denormalized/OpCatalogAllVarAttributesWithTypeVarSubstrMetaServer.cpp
            ops_rdma/server_sqlite_transactions_only_multiple_databases_write_ahead_log_denormalized/OpCatalogAllVarAttributesWithTypeVarSubstrRangeMetaServer.cpp
            ops_rdma/server_sqlite_transactions_only_multiple_databases_write_ahead_log_denormalized/OpCatalogAllVarAttributesWithTypeVarSubstrDimsRangeMetaServer.cpp
            ops_rdma/server_sqlite_transactions_only_multiple_databases_write_ahead_log_denormalized/OpCatalogAllTypesWithVarAttributesWithVarSubstrInTimestepMetaServer.cpp
            ops_rdma/server_sqlite_transactions_only_multiple_databases_write_ahead_log_denormalized/OpCatalogAllTypesWithVarAttributesWithVarSubstrDimsInTimestepMetaServer.cpp
            ops_rdma/server_sqlite_transactions_only_multiple_databases_write_ahead_log_denormalized/OpCatalogAllTimestepsWithVarSubstrMetaServer.cpp
            ops_rdma/server_sqlite_transactions_only_multiple_databases_write_ahead_log_denormalized/OpCatalogAllTimestepsWithVarAttributesWithTypeVarSubstrRangeMetaServer.cpp
            ops_rdma/server_sqlite_transactions_only_multiple_databases_write_ahead_log_denormalized/OpCatalogAllTimestepsWithVarAttributesWithTypeVarSubstrMetaServer.cpp
            ops_rdma/server_sqlite_transactions_only_multiple_databases_write_ahead_log_denormalized/OpCatalogAllTimestepsWithVarAttributesWithTypeVarSubstrDimsRangeMetaServer.cpp
            ops_rdma/server_sqlite_transactions_only_multiple_databases_write_ahead_log_denormalized/OpCatalogAllTimestepsWithVarAttributesWithTypeVarSubstrDimsMetaServer.cpp
            ops_rdma/server_sqlite_transactions_only_multiple_databases_write_ahead_log_denormalized/OpCatalogAllVarAttributesWithVarSubstrMetaServer.cpp
            ops_rdma/server_sqlite_transactions_only_multiple_databases_write_ahead_log_denormalized/OpCatalogAllVarAttributesWithTypeVarDimsRangeMetaServer.cpp
            ops_rdma/server_sqlite_transactions_only_multiple_databases_write_ahead_log_denormalized/OpCatalogAllVarAttributesWithTypeVarRangeMetaServer.cpp
            ops_rdma/server_sqlite_transactions_only_multiple_databases_write_ahead_log_denormalized/OpCatalogAllTimestepAttributesWithTypeRangeMetaServer.cpp
            ops_rdma/server_sqlite_transactions_only_multiple_databases_write_ahead_log_denormalized/OpCatalogAllTimestepsWithVarAttributesWithTypeVarDimsRangeMetaServer.cpp
            ops_rdma/server_sqlite_transactions_only_multiple_databases_write_ahead_log_denormalized/OpCatalogAllTimestepsWithVarAttributesWithTypeVarRangeMetaServer.cpp
            ops_rdma/server_sqlite_transactions_only_multiple_databases_write_ahead_log_denormalized/OpCatalogAllRunAttributesWithTypeRangeMetaServer.cpp
            ops_rdma/server_sqlite_transactions_only_multiple_databases_write_ahead_log_denormalized/OpCatalogAllRunAttributesMetaServer.cpp
            ops_rdma/server_sqlite_transactions_only_multiple_databases_write_ahead_log_denormalized/OpCatalogAllRunAttributesWithTypeMetaServer.cpp
            ops_rdma/server_sqlite_transactions_only_multiple_databases_write_ahead_log_denormalized/OpCatalogAllTimestepAttributesMetaServer.cpp
            ops_rdma/server_sqlite_transactions_only_multiple_databases_write_ahead_log_denormalized/OpCatalogAllTimestepAttributesWithTypeMetaServer.cpp
            ops_rdma/server_sqlite_transactions_only_multiple_databases_write_ahead_log_denormalized/OpCatalogAllTimestepsWithVarAttributesWithTypeVarDimsMetaServer.cpp
            ops_rdma/server_sqlite_transactions_only_multiple_databases_write_ahead_log_denormalized/OpCatalogAllTimestepsWithVarAttributesWithTypeVarMetaServer.cpp
            ops_rdma/server_sqlite_transactions_only_multiple_databases_write_ahead_log_denormalized/OpCatalogAllTimestepsWithVarMetaServer.cpp
            ops_rdma/server_sqlite_transactions_only_multiple_databases_write_ahead_log_denormalized/OpCatalogAllTypesWithVarAttributesInTimestepMetaServer.cpp
            ops_rdma/server_sqlite_transactions_only_multiple_databases_write_ahead_log_denormalized/OpCatalogAllTypesWithVarAttributesWithVarDimsInTimestepMetaServer.cpp
            ops_rdma/server_sqlite_transactions_only_multiple_databases_write_ahead_log_denormalized/OpCatalogAllTypesWithVarAttributesWithVarInTimestepMetaServer.cpp
            ops_rdma/server_sqlite_transactions_only_multiple_databases_write_ahead_log_denormalized/OpCatalogAllVarAttributesMetaServer.cpp 
            ops_rdma/server_sqlite_transactions_only_multiple_databases_write_ahead_log_denormalized/OpCatalogAllVarAttributesWithDimsMetaServer.cpp 
            ops_rdma/server_sqlite_transactions_only_multiple_databases_write_ahead_log_denormalized/OpCatalogAllVarAttributesWithTypeByIdMetaServer.cpp 
            ops_rdma/server_sqlite_transactions_only_multiple_databases_write_ahead_log_denormalized/OpCatalogAllVarAttributesWithTypeByNameVerMetaServer.cpp 
            ops_rdma/server_sqlite_transactions_only_multiple_databases_write_ahead_log_denormalized/OpCatalogAllVarAttributesWithTypeDimsByIdMetaServer.cpp 
            ops_rdma/server_sqlite_transactions_only_multiple_databases_write_ahead_log_denormalized/OpCatalogAllVarAttributesWithTypeDimsByNameVerMetaServer.cpp 
            ops_rdma/server_sqlite_transactions_only_multiple_databases_write_ahead_log_denormalized/OpCatalogAllVarAttributesWithTypeVarByIdMetaServer.cpp 
            ops_rdma/server_sqlite_transactions_only_multiple_databases_write_ahead_log_denormalized/OpCatalogAllVarAttributesWithTypeVarByNameVerMetaServer.cpp 
            ops_rdma/server_sqlite_transactions_only_multiple_databases_write_ahead_log_denormalized/OpCatalogAllVarAttributesWithTypeVarDimsByIdMetaServer.cpp 
            ops_rdma/server_sqlite_transactions_only_multiple_databases_write_ahead_log_denormalized/OpCatalogAllVarAttributesWithTypeVarDimsByNameVerMetaServer.cpp 
            ops_rdma/server_sqlite_transactions_only_multiple_databases_write_ahead_log_denormalized/OpCatalogAllVarAttributesWithVarByIdMetaServer.cpp 
            ops_rdma/server_sqlite_transactions_only_multiple_databases_write_ahead_log_denormalized/OpCatalogAllVarAttributesWithVarByNameVerMetaServer.cpp
            ops_rdma/server_sqlite_transactions_only_multiple_databases_write_ahead_log_denormalized/OpCatalogAllVarAttributesWithVarDimsByIdMetaServer.cpp 
            ops_rdma/server_sqlite_transactions_only_multiple_databases_write_ahead_log_denormalized/OpCatalogAllVarAttributesWithVarDimsByNameVerMetaServer.cpp    
            ops_rdma/server_sqlite_transactions_only_multiple_databases_write_ahead_log_denormalized/OpCatalogRunMetaServer.cpp             
            ops_rdma/server_sqlite_transactions_only_multiple_databases_write_ahead_log_denormalized/OpCatalogTimestepMetaServer.cpp
            ops_rdma/server_sqlite_transactions_only_multiple_databases_write_ahead_log_denormalized/OpCatalogTypeMetaServer.cpp             
            ops_rdma/server_sqlite_transactions_only_multiple_databases_write_ahead_log_denormalized/OpCatalogVarMetaServer.cpp             
            ops_rdma/server_sqlite_transactions_only_multiple_databases_write_ahead_log_denormalized/OpCreateRunMetaServer.cpp           
            ops_rdma/server_sqlite_transactions_only_multiple_databases_write_ahead_log_denormalized/OpCreateTimestepMetaServer.cpp
            ops_rdma/server_sqlite_transactions_only_multiple_databases_write_ahead_log_denormalized/OpCreateTypeMetaServer.cpp              
            ops_rdma/server_sqlite_transactions_only_multiple_databases_write_ahead_log_denormalized/OpCreateVarMetaServer.cpp    
            ops_rdma/server_sqlite_transactions_only_multiple_databases_write_ahead_log_denormalized/OpDeleteRunByIdMetaServer.cpp  
            ops_rdma/server_sqlite_transactions_only_multiple_databases_write_ahead_log_denormalized/OpDeleteTimestepByIdMetaServer.cpp
            ops_rdma/server_sqlite_transactions_only_multiple_databases_write_ahead_log_denormalized/OpDeleteTypeByIdMetaServer.cpp   
            ops_rdma/server_sqlite_transactions_only_multiple_databases_write_ahead_log_denormalized/OpDeleteTypeByNameVerMetaServer.cpp           
            ops_rdma/server_sqlite_transactions_only_multiple_databases_write_ahead_log_denormalized/OpDeleteVarByIdMetaServer.cpp  
            ops_rdma/server_sqlite_transactions_only_multiple_databases_write_ahead_log_denormalized/OpDeleteVarByNamePathVerMetaServer.cpp  
            ops_rdma/server_sqlite_transactions_only_multiple_databases_write_ahead_log_denormalized/OpFullShutdownMetaServer.cpp       
            ops_rdma/server_sqlite_transactions_only_multiple_databases_write_ahead_log_denormalized/OpInsertRunAttributeMetaServer.cpp
            ops_rdma/server_sqlite_transactions_only_multiple_databases_write_ahead_log_denormalized/OpInsertTimestepAttributeMetaServer.cpp
            ops_rdma/server_sqlite_transactions_only_multiple_databases_write_ahead_log_denormalized/OpInsertVarAttributeByDimsMetaServer.cpp 
        )

        TARGET_INCLUDE_DIRECTORIES(OpsServerSqliteTransactionOnly PUBLIC
            ${CMAKE_CURRENT_SOURCE_DIR}/../include/ops_rdma
            # ${CMAKE_CURRENT_SOURCE_DIR}/../include/ops_rdma_sqlite_transactions_only
            ${CMAKE_CURRENT_SOURCE_DIR}/../include/common_sqlite_transactions_only_multiple_databases_read_and_write_new
            ${CMAKE_CURRENT_SOURCE_DIR}/../include/database
            ${CMAKE_CURRENT_SOURCE_DIR}/../include/server
        )


        add_library(OpsClientSqliteTransactionOnly
           ${CLIENT_OPS}
        )

        TARGET_INCLUDE_DIRECTORIES(OpsClientSqliteTransactionOnly PUBLIC
            ${CMAKE_CURRENT_SOURCE_DIR}/../include/ops_rdma
            # ${CMAKE_CURRENT_SOURCE_DIR}/../include/ops_rdma_sqlite_transactions_only
            ${CMAKE_CURRENT_SOURCE_DIR}/../include/common_sqlite_transactions_only_multiple_databases_read_and_write_new
            ${CMAKE_CURRENT_SOURCE_DIR}/../include/client
        )


        add_library(OpsServerSqliteTransactionOnlyDBStreams
            ops_rdma/common/OpCoreCommon.cpp  
            ops_rdma/server_sqlite_transactions_only_any_database_checkpts_denormalized/OpCoreServer.cpp  

            ops_rdma/server_sqlite_transactions_only_any_database_checkpts_denormalized/OpCommitTransactionMetaServer.cpp
            ops_rdma/server_sqlite_transactions_only_any_database_checkpts_denormalized/OpAbortTransactionMetaServer.cpp

            # #only needed since we are using a single metadata_server source code rather than
            # #a dedicated transaction one
            ops_rdma/server_sqlite_transactions_only_any_database_checkpts_denormalized/OpProcessingMetaServer.cpp
            # ops_rdma/server/OpActivateMetaServer.cpp
            ops_rdma/server_sqlite_transactions_only_any_database_checkpts_denormalized/OpActivateRunAttributeMetaServer.cpp
            ops_rdma/server_sqlite_transactions_only_any_database_checkpts_denormalized/OpActivateRunMetaServer.cpp          
            ops_rdma/server_sqlite_transactions_only_any_database_checkpts_denormalized/OpActivateTimestepAttributeMetaServer.cpp
            ops_rdma/server_sqlite_transactions_only_any_database_checkpts_denormalized/OpActivateTimestepMetaServer.cpp
            ops_rdma/server_sqlite_transactions_only_any_database_checkpts_denormalized/OpActivateTypeMetaServer.cpp   
            ops_rdma/server_sqlite_transactions_only_any_database_checkpts_denormalized/OpActivateVarAttributeMetaServer.cpp          
            ops_rdma/server_sqlite_transactions_only_any_database_checkpts_denormalized/OpActivateVarMetaServer.cpp

            ops_rdma/server_sqlite_transactions_only_any_database_checkpts_denormalized/sql_helper_functs.cpp

            ops_rdma/server_sqlite_transactions_only_any_database_checkpts_denormalized/OpCheckpointDatabaseMetaServer.cpp
            ops_rdma/server_sqlite_transactions_only_any_database_checkpts_denormalized/OpInsertTimestepAttributeBatchMetaServer.cpp
            ops_rdma/server_sqlite_transactions_only_any_database_checkpts_denormalized/OpInsertRunAttributeBatchMetaServer.cpp
            ops_rdma/server_sqlite_transactions_only_any_database_checkpts_denormalized/OpCreateVarBatchMetaServer.cpp
            ops_rdma/server_sqlite_transactions_only_any_database_checkpts_denormalized/OpCreateTypeBatchMetaServer.cpp
            ops_rdma/server_sqlite_transactions_only_any_database_checkpts_denormalized/OpInsertVarAttributeByDimsBatchMetaServer.cpp
            ops_rdma/server_sqlite_transactions_only_any_database_checkpts_denormalized/OpDeleteAllVarsWithSubstrMetaServer.cpp
            ops_rdma/server_sqlite_transactions_only_any_database_checkpts_denormalized/OpCatalogAllVarAttributesWithVarSubstrDimsMetaServer.cpp
            ops_rdma/server_sqlite_transactions_only_any_database_checkpts_denormalized/OpCatalogAllVarAttributesWithTypeVarSubstrDimsMetaServer.cpp
            ops_rdma/server_sqlite_transactions_only_any_database_checkpts_denormalized/OpCatalogAllVarAttributesWithTypeVarSubstrMetaServer.cpp
            ops_rdma/server_sqlite_transactions_only_any_database_checkpts_denormalized/OpCatalogAllVarAttributesWithTypeVarSubstrRangeMetaServer.cpp
            ops_rdma/server_sqlite_transactions_only_any_database_checkpts_denormalized/OpCatalogAllVarAttributesWithTypeVarSubstrDimsRangeMetaServer.cpp
            ops_rdma/server_sqlite_transactions_only_any_database_checkpts_denormalized/OpCatalogAllTypesWithVarAttributesWithVarSubstrInTimestepMetaServer.cpp
            ops_rdma/server_sqlite_transactions_only_any_database_checkpts_denormalized/OpCatalogAllTypesWithVarAttributesWithVarSubstrDimsInTimestepMetaServer.cpp
            ops_rdma/server_sqlite_transactions_only_any_database_checkpts_denormalized/OpCatalogAllTimestepsWithVarSubstrMetaServer.cpp
            ops_rdma/server_sqlite_transactions_only_any_database_checkpts_denormalized/OpCatalogAllTimestepsWithVarAttributesWithTypeVarSubstrRangeMetaServer.cpp
            ops_rdma/server_sqlite_transactions_only_any_database_checkpts_denormalized/OpCatalogAllTimestepsWithVarAttributesWithTypeVarSubstrMetaServer.cpp
            ops_rdma/server_sqlite_transactions_only_any_database_checkpts_denormalized/OpCatalogAllTimestepsWithVarAttributesWithTypeVarSubstrDimsRangeMetaServer.cpp
            ops_rdma/server_sqlite_transactions_only_any_database_checkpts_denormalized/OpCatalogAllTimestepsWithVarAttributesWithTypeVarSubstrDimsMetaServer.cpp
            ops_rdma/server_sqlite_transactions_only_any_database_checkpts_denormalized/OpCatalogAllVarAttributesWithVarSubstrMetaServer.cpp
            ops_rdma/server_sqlite_transactions_only_any_database_checkpts_denormalized/OpCatalogAllVarAttributesWithTypeVarDimsRangeMetaServer.cpp
            ops_rdma/server_sqlite_transactions_only_any_database_checkpts_denormalized/OpCatalogAllVarAttributesWithTypeVarRangeMetaServer.cpp
            ops_rdma/server_sqlite_transactions_only_any_database_checkpts_denormalized/OpCatalogAllTimestepAttributesWithTypeRangeMetaServer.cpp
            ops_rdma/server_sqlite_transactions_only_any_database_checkpts_denormalized/OpCatalogAllTimestepsWithVarAttributesWithTypeVarDimsRangeMetaServer.cpp
            ops_rdma/server_sqlite_transactions_only_any_database_checkpts_denormalized/OpCatalogAllTimestepsWithVarAttributesWithTypeVarRangeMetaServer.cpp
            ops_rdma/server_sqlite_transactions_only_any_database_checkpts_denormalized/OpCatalogAllRunAttributesWithTypeRangeMetaServer.cpp
            ops_rdma/server_sqlite_transactions_only_any_database_checkpts_denormalized/OpCatalogAllRunAttributesMetaServer.cpp
            ops_rdma/server_sqlite_transactions_only_any_database_checkpts_denormalized/OpCatalogAllRunAttributesWithTypeMetaServer.cpp
            ops_rdma/server_sqlite_transactions_only_any_database_checkpts_denormalized/OpCatalogAllTimestepAttributesMetaServer.cpp
            ops_rdma/server_sqlite_transactions_only_any_database_checkpts_denormalized/OpCatalogAllTimestepAttributesWithTypeMetaServer.cpp
            ops_rdma/server_sqlite_transactions_only_any_database_checkpts_denormalized/OpCatalogAllTimestepsWithVarAttributesWithTypeVarDimsMetaServer.cpp
            ops_rdma/server_sqlite_transactions_only_any_database_checkpts_denormalized/OpCatalogAllTimestepsWithVarAttributesWithTypeVarMetaServer.cpp
            ops_rdma/server_sqlite_transactions_only_any_database_checkpts_denormalized/OpCatalogAllTimestepsWithVarMetaServer.cpp
            ops_rdma/server_sqlite_transactions_only_any_database_checkpts_denormalized/OpCatalogAllTypesWithVarAttributesInTimestepMetaServer.cpp
            ops_rdma/server_sqlite_transactions_only_any_database_checkpts_denormalized/OpCatalogAllTypesWithVarAttributesWithVarDimsInTimestepMetaServer.cpp
            ops_rdma/server_sqlite_transactions_only_any_database_checkpts_denormalized/OpCatalogAllTypesWithVarAttributesWithVarInTimestepMetaServer.cpp
            ops_rdma/server_sqlite_transactions_only_any_database_checkpts_denormalized/OpCatalogAllVarAttributesMetaServer.cpp 
            ops_rdma/server_sqlite_transactions_only_any_database_checkpts_denormalized/OpCatalogAllVarAttributesWithDimsMetaServer.cpp 
            ops_rdma/server_sqlite_transactions_only_any_database_checkpts_denormalized/OpCatalogAllVarAttributesWithTypeByIdMetaServer.cpp 
            ops_rdma/server_sqlite_transactions_only_any_database_checkpts_denormalized/OpCatalogAllVarAttributesWithTypeByNameVerMetaServer.cpp 
            ops_rdma/server_sqlite_transactions_only_any_database_checkpts_denormalized/OpCatalogAllVarAttributesWithTypeDimsByIdMetaServer.cpp 
            ops_rdma/server_sqlite_transactions_only_any_database_checkpts_denormalized/OpCatalogAllVarAttributesWithTypeDimsByNameVerMetaServer.cpp 
            ops_rdma/server_sqlite_transactions_only_any_database_checkpts_denormalized/OpCatalogAllVarAttributesWithTypeVarByIdMetaServer.cpp 
            ops_rdma/server_sqlite_transactions_only_any_database_checkpts_denormalized/OpCatalogAllVarAttributesWithTypeVarByNameVerMetaServer.cpp 
            ops_rdma/server_sqlite_transactions_only_any_database_checkpts_denormalized/OpCatalogAllVarAttributesWithTypeVarDimsByIdMetaServer.cpp 
            ops_rdma/server_sqlite_transactions_only_any_database_checkpts_denormalized/OpCatalogAllVarAttributesWithTypeVarDimsByNameVerMetaServer.cpp 
            ops_rdma/server_sqlite_transactions_only_any_database_checkpts_denormalized/OpCatalogAllVarAttributesWithVarByIdMetaServer.cpp 
            ops_rdma/server_sqlite_transactions_only_any_database_checkpts_denormalized/OpCatalogAllVarAttributesWithVarByNameVerMetaServer.cpp
            ops_rdma/server_sqlite_transactions_only_any_database_checkpts_denormalized/OpCatalogAllVarAttributesWithVarDimsByIdMetaServer.cpp 
            ops_rdma/server_sqlite_transactions_only_any_database_checkpts_denormalized/OpCatalogAllVarAttributesWithVarDimsByNameVerMetaServer.cpp    
            ops_rdma/server_sqlite_transactions_only_any_database_checkpts_denormalized/OpCatalogRunMetaServer.cpp             
            ops_rdma/server_sqlite_transactions_only_any_database_checkpts_denormalized/OpCatalogTimestepMetaServer.cpp
            ops_rdma/server_sqlite_transactions_only_any_database_checkpts_denormalized/OpCatalogTypeMetaServer.cpp             
            ops_rdma/server_sqlite_transactions_only_any_database_checkpts_denormalized/OpCatalogVarMetaServer.cpp             
            ops_rdma/server_sqlite_transactions_only_any_database_checkpts_denormalized/OpCreateRunMetaServer.cpp           
            ops_rdma/server_sqlite_transactions_only_any_database_checkpts_denormalized/OpCreateTimestepMetaServer.cpp
            ops_rdma/server_sqlite_transactions_only_any_database_checkpts_denormalized/OpCreateTypeMetaServer.cpp              
            ops_rdma/server_sqlite_transactions_only_any_database_checkpts_denormalized/OpCreateVarMetaServer.cpp    
            ops_rdma/server_sqlite_transactions_only_any_database_checkpts_denormalized/OpDeleteRunByIdMetaServer.cpp  
            ops_rdma/server_sqlite_transactions_only_any_database_checkpts_denormalized/OpDeleteTimestepByIdMetaServer.cpp
            ops_rdma/server_sqlite_transactions_only_any_database_checkpts_denormalized/OpDeleteTypeByIdMetaServer.cpp   
            ops_rdma/server_sqlite_transactions_only_any_database_checkpts_denormalized/OpDeleteTypeByNameVerMetaServer.cpp           
            ops_rdma/server_sqlite_transactions_only_any_database_checkpts_denormalized/OpDeleteVarByIdMetaServer.cpp  
            ops_rdma/server_sqlite_transactions_only_any_database_checkpts_denormalized/OpDeleteVarByNamePathVerMetaServer.cpp  
            ops_rdma/server_sqlite_transactions_only_any_database_checkpts_denormalized/OpFullShutdownMetaServer.cpp       
            ops_rdma/server_sqlite_transactions_only_any_database_checkpts_denormalized/OpInsertRunAttributeMetaServer.cpp
            ops_rdma/server_sqlite_transactions_only_any_database_checkpts_denormalized/OpInsertTimestepAttributeMetaServer.cpp
            ops_rdma/server_sqlite_transactions_only_any_database_checkpts_denormalized/OpInsertVarAttributeByDimsMetaServer.cpp 
        )

        TARGET_INCLUDE_DIRECTORIES(OpsServerSqliteTransactionOnlyDBStreams PUBLIC
            ${CMAKE_CURRENT_SOURCE_DIR}/../include/ops_rdma
            ${CMAKE_CURRENT_SOURCE_DIR}/../include/common_sqlite_transactions_only_db_streams
            ${CMAKE_CURRENT_SOURCE_DIR}/../include/database
            ${CMAKE_CURRENT_SOURCE_DIR}/../include/server
        )

        add_library(OpsClientSqliteTransactionOnlyDBStreams
           ${CLIENT_OPS}
            ops_rdma/client_sqlite_transactions_only_any_database_checkpts_denormalized/OpCommitTransactionMetaClient.cpp
            ops_rdma/client_sqlite_transactions_only_any_database_checkpts_denormalized/OpAbortTransactionMetaClient.cpp
        )

        TARGET_INCLUDE_DIRECTORIES(OpsClientSqliteTransactionOnlyDBStreams PUBLIC
            ${CMAKE_CURRENT_SOURCE_DIR}/../include/ops_rdma
            # ${CMAKE_CURRENT_SOURCE_DIR}/../include/ops_rdma_sqlite_transactions_only
            ${CMAKE_CURRENT_SOURCE_DIR}/../include/common_sqlite_transactions_only_db_streams
            ${CMAKE_CURRENT_SOURCE_DIR}/../include/client
        )

    endif(MAKE_CLASS_PROJ_SQLITE_TRANSACTIONS_ONLY)

    if(MAKE_CLASS_PROJ_D2T_TRANSACTIONS_ONLY)
        add_library(OpsServerD2TTransactionOnly
            ops_rdma/common/OpCoreCommon.cpp  
            ops_rdma/server_d2t_transactions_only_denormalized/OpCoreServer.cpp  

            # #only needed since we are using a single metadata_server source code rather than
            # #a dedicated transaction one
            ops_rdma/server_d2t_transactions_only_denormalized/OpProcessingMetaServer.cpp
            # ops_rdma/server/OpActivateMetaServer.cpp
            ops_rdma/server_d2t_transactions_only_denormalized/OpActivateRunAttributeMetaServer.cpp
            ops_rdma/server_d2t_transactions_only_denormalized/OpActivateRunMetaServer.cpp          
            ops_rdma/server_d2t_transactions_only_denormalized/OpActivateTimestepAttributeMetaServer.cpp
            ops_rdma/server_d2t_transactions_only_denormalized/OpActivateTimestepMetaServer.cpp
            ops_rdma/server_d2t_transactions_only_denormalized/OpActivateTypeMetaServer.cpp   
            ops_rdma/server_d2t_transactions_only_denormalized/OpActivateVarAttributeMetaServer.cpp          
            ops_rdma/server_d2t_transactions_only_denormalized/OpActivateVarMetaServer.cpp

            ops_rdma/server_d2t_transactions_only_denormalized/sql_helper_functs.cpp

            ops_rdma/server_d2t_transactions_only_denormalized/OpCheckpointDatabaseMetaServer.cpp
            ops_rdma/server_d2t_transactions_only_denormalized/OpInsertTimestepAttributeBatchMetaServer.cpp
            ops_rdma/server_d2t_transactions_only_denormalized/OpInsertRunAttributeBatchMetaServer.cpp
            ops_rdma/server_d2t_transactions_only_denormalized/OpCreateVarBatchMetaServer.cpp
            ops_rdma/server_d2t_transactions_only_denormalized/OpCreateTypeBatchMetaServer.cpp
            ops_rdma/server_d2t_transactions_only_denormalized/OpInsertVarAttributeByDimsBatchMetaServer.cpp
            ops_rdma/server_d2t_transactions_only_denormalized/OpDeleteAllVarsWithSubstrMetaServer.cpp
            ops_rdma/server_d2t_transactions_only_denormalized/OpCatalogAllVarAttributesWithVarSubstrDimsMetaServer.cpp
            ops_rdma/server_d2t_transactions_only_denormalized/OpCatalogAllVarAttributesWithTypeVarSubstrDimsMetaServer.cpp
            ops_rdma/server_d2t_transactions_only_denormalized/OpCatalogAllVarAttributesWithTypeVarSubstrMetaServer.cpp
            ops_rdma/server_d2t_transactions_only_denormalized/OpCatalogAllVarAttributesWithTypeVarSubstrRangeMetaServer.cpp
            ops_rdma/server_d2t_transactions_only_denormalized/OpCatalogAllVarAttributesWithTypeVarSubstrDimsRangeMetaServer.cpp
            ops_rdma/server_d2t_transactions_only_denormalized/OpCatalogAllTypesWithVarAttributesWithVarSubstrInTimestepMetaServer.cpp
            ops_rdma/server_d2t_transactions_only_denormalized/OpCatalogAllTypesWithVarAttributesWithVarSubstrDimsInTimestepMetaServer.cpp
            ops_rdma/server_d2t_transactions_only_denormalized/OpCatalogAllTimestepsWithVarSubstrMetaServer.cpp
            ops_rdma/server_d2t_transactions_only_denormalized/OpCatalogAllTimestepsWithVarAttributesWithTypeVarSubstrRangeMetaServer.cpp
            ops_rdma/server_d2t_transactions_only_denormalized/OpCatalogAllTimestepsWithVarAttributesWithTypeVarSubstrMetaServer.cpp
            ops_rdma/server_d2t_transactions_only_denormalized/OpCatalogAllTimestepsWithVarAttributesWithTypeVarSubstrDimsRangeMetaServer.cpp
            ops_rdma/server_d2t_transactions_only_denormalized/OpCatalogAllTimestepsWithVarAttributesWithTypeVarSubstrDimsMetaServer.cpp
            ops_rdma/server_d2t_transactions_only_denormalized/OpCatalogAllVarAttributesWithVarSubstrMetaServer.cpp
            ops_rdma/server_d2t_transactions_only_denormalized/OpCatalogAllVarAttributesWithTypeVarDimsRangeMetaServer.cpp
            ops_rdma/server_d2t_transactions_only_denormalized/OpCatalogAllVarAttributesWithTypeVarRangeMetaServer.cpp
            ops_rdma/server_d2t_transactions_only_denormalized/OpCatalogAllTimestepAttributesWithTypeRangeMetaServer.cpp
            ops_rdma/server_d2t_transactions_only_denormalized/OpCatalogAllTimestepsWithVarAttributesWithTypeVarDimsRangeMetaServer.cpp
            ops_rdma/server_d2t_transactions_only_denormalized/OpCatalogAllTimestepsWithVarAttributesWithTypeVarRangeMetaServer.cpp
            ops_rdma/server_d2t_transactions_only_denormalized/OpCatalogAllRunAttributesWithTypeRangeMetaServer.cpp
            ops_rdma/server_d2t_transactions_only_denormalized/OpCatalogAllRunAttributesMetaServer.cpp
            ops_rdma/server_d2t_transactions_only_denormalized/OpCatalogAllRunAttributesWithTypeMetaServer.cpp
            ops_rdma/server_d2t_transactions_only_denormalized/OpCatalogAllTimestepAttributesMetaServer.cpp
            ops_rdma/server_d2t_transactions_only_denormalized/OpCatalogAllTimestepAttributesWithTypeMetaServer.cpp
            ops_rdma/server_d2t_transactions_only_denormalized/OpCatalogAllTimestepsWithVarAttributesWithTypeVarDimsMetaServer.cpp
            ops_rdma/server_d2t_transactions_only_denormalized/OpCatalogAllTimestepsWithVarAttributesWithTypeVarMetaServer.cpp
            ops_rdma/server_d2t_transactions_only_denormalized/OpCatalogAllTimestepsWithVarMetaServer.cpp
            ops_rdma/server_d2t_transactions_only_denormalized/OpCatalogAllTypesWithVarAttributesInTimestepMetaServer.cpp
            ops_rdma/server_d2t_transactions_only_denormalized/OpCatalogAllTypesWithVarAttributesWithVarDimsInTimestepMetaServer.cpp
            ops_rdma/server_d2t_transactions_only_denormalized/OpCatalogAllTypesWithVarAttributesWithVarInTimestepMetaServer.cpp
            ops_rdma/server_d2t_transactions_only_denormalized/OpCatalogAllVarAttributesMetaServer.cpp 
            ops_rdma/server_d2t_transactions_only_denormalized/OpCatalogAllVarAttributesWithDimsMetaServer.cpp 
            ops_rdma/server_d2t_transactions_only_denormalized/OpCatalogAllVarAttributesWithTypeByIdMetaServer.cpp 
            ops_rdma/server_d2t_transactions_only_denormalized/OpCatalogAllVarAttributesWithTypeByNameVerMetaServer.cpp 
            ops_rdma/server_d2t_transactions_only_denormalized/OpCatalogAllVarAttributesWithTypeDimsByIdMetaServer.cpp 
            ops_rdma/server_d2t_transactions_only_denormalized/OpCatalogAllVarAttributesWithTypeDimsByNameVerMetaServer.cpp 
            ops_rdma/server_d2t_transactions_only_denormalized/OpCatalogAllVarAttributesWithTypeVarByIdMetaServer.cpp 
            ops_rdma/server_d2t_transactions_only_denormalized/OpCatalogAllVarAttributesWithTypeVarByNameVerMetaServer.cpp 
            ops_rdma/server_d2t_transactions_only_denormalized/OpCatalogAllVarAttributesWithTypeVarDimsByIdMetaServer.cpp 
            ops_rdma/server_d2t_transactions_only_denormalized/OpCatalogAllVarAttributesWithTypeVarDimsByNameVerMetaServer.cpp 
            ops_rdma/server_d2t_transactions_only_denormalized/OpCatalogAllVarAttributesWithVarByIdMetaServer.cpp 
            ops_rdma/server_d2t_transactions_only_denormalized/OpCatalogAllVarAttributesWithVarByNameVerMetaServer.cpp
            ops_rdma/server_d2t_transactions_only_denormalized/OpCatalogAllVarAttributesWithVarDimsByIdMetaServer.cpp 
            ops_rdma/server_d2t_transactions_only_denormalized/OpCatalogAllVarAttributesWithVarDimsByNameVerMetaServer.cpp    
            ops_rdma/server_d2t_transactions_only_denormalized/OpCatalogRunMetaServer.cpp             
            ops_rdma/server_d2t_transactions_only_denormalized/OpCatalogTimestepMetaServer.cpp
            ops_rdma/server_d2t_transactions_only_denormalized/OpCatalogTypeMetaServer.cpp             
            ops_rdma/server_d2t_transactions_only_denormalized/OpCatalogVarMetaServer.cpp             
            ops_rdma/server_d2t_transactions_only_denormalized/OpCreateRunMetaServer.cpp           
            ops_rdma/server_d2t_transactions_only_denormalized/OpCreateTimestepMetaServer.cpp
            ops_rdma/server_d2t_transactions_only_denormalized/OpCreateTypeMetaServer.cpp              
            ops_rdma/server_d2t_transactions_only_denormalized/OpCreateVarMetaServer.cpp    
            ops_rdma/server_d2t_transactions_only_denormalized/OpDeleteRunByIdMetaServer.cpp  
            ops_rdma/server_d2t_transactions_only_denormalized/OpDeleteTimestepByIdMetaServer.cpp
            ops_rdma/server_d2t_transactions_only_denormalized/OpDeleteTypeByIdMetaServer.cpp   
            ops_rdma/server_d2t_transactions_only_denormalized/OpDeleteTypeByNameVerMetaServer.cpp           
            ops_rdma/server_d2t_transactions_only_denormalized/OpDeleteVarByIdMetaServer.cpp  
            ops_rdma/server_d2t_transactions_only_denormalized/OpDeleteVarByNamePathVerMetaServer.cpp  
            ops_rdma/server_d2t_transactions_only_denormalized/OpFullShutdownMetaServer.cpp       
            ops_rdma/server_d2t_transactions_only_denormalized/OpInsertRunAttributeMetaServer.cpp
            ops_rdma/server_d2t_transactions_only_denormalized/OpInsertTimestepAttributeMetaServer.cpp
            ops_rdma/server_d2t_transactions_only_denormalized/OpInsertVarAttributeByDimsMetaServer.cpp 
        )

        TARGET_INCLUDE_DIRECTORIES(OpsServerD2TTransactionOnly PUBLIC
            ${CMAKE_CURRENT_SOURCE_DIR}/../include/ops_rdma
            ${CMAKE_CURRENT_SOURCE_DIR}/../include/common_denormalized
            ${CMAKE_CURRENT_SOURCE_DIR}/../include/database
            ${CMAKE_CURRENT_SOURCE_DIR}/../include/server
        )


        add_library(OpsClientD2TTransactionOnly
           ${CLIENT_OPS}
        )

        TARGET_INCLUDE_DIRECTORIES(OpsClientD2TTransactionOnly PUBLIC
            ${CMAKE_CURRENT_SOURCE_DIR}/../include/ops_rdma
            # ${CMAKE_CURRENT_SOURCE_DIR}/../include/ops_rdma_sqlite_transactions_only
            ${CMAKE_CURRENT_SOURCE_DIR}/../include/common_denormalized
            ${CMAKE_CURRENT_SOURCE_DIR}/../include/client
        )


    endif(MAKE_CLASS_PROJ_D2T_TRANSACTIONS_ONLY)

    if(MAKE_CLASS_PROJ_RTREE)

        add_library(OpsServerRtree
            ops_rdma/common/OpCoreCommon.cpp  
            ops_rdma/server/OpCoreServer.cpp  

            ops_rdma/server_separate_rtree/sql_helper_functs.cpp
            ops_rdma/server_separate_rtree/OpCatalogAllTimestepsWithVarAttributesWithTypeVarDimsMetaServer.cpp
            ops_rdma/server_separate_rtree/OpCatalogAllTimestepsWithVarAttributesWithTypeVarDimsRangeMetaServer.cpp
            ops_rdma/server_separate_rtree/OpCatalogAllTimestepsWithVarAttributesWithTypeVarSubstrDimsMetaServer.cpp
            ops_rdma/server_separate_rtree/OpCatalogAllTimestepsWithVarAttributesWithTypeVarSubstrDimsRangeMetaServer.cpp
            ops_rdma/server_separate_rtree/OpCatalogAllTypesWithVarAttributesWithVarDimsInTimestepMetaServer.cpp
            ops_rdma/server_separate_rtree/OpCatalogAllTypesWithVarAttributesWithVarSubstrDimsInTimestepMetaServer.cpp
            ops_rdma/server_separate_rtree/OpCatalogAllVarAttributesWithDimsMetaServer.cpp 
            ops_rdma/server_separate_rtree/OpCatalogAllVarAttributesWithTypeDimsByIdMetaServer.cpp 
            ops_rdma/server_separate_rtree/OpCatalogAllVarAttributesWithTypeDimsByNameVerMetaServer.cpp 
            ops_rdma/server_separate_rtree/OpCatalogAllVarAttributesWithTypeVarDimsByIdMetaServer.cpp 
            ops_rdma/server_separate_rtree/OpCatalogAllVarAttributesWithTypeVarDimsByNameVerMetaServer.cpp 
            ops_rdma/server_separate_rtree/OpCatalogAllVarAttributesWithTypeVarDimsRangeMetaServer.cpp
            ops_rdma/server_separate_rtree/OpCatalogAllVarAttributesWithTypeVarSubstrDimsMetaServer.cpp
            ops_rdma/server_separate_rtree/OpCatalogAllVarAttributesWithTypeVarSubstrDimsRangeMetaServer.cpp
            ops_rdma/server_separate_rtree/OpCatalogAllVarAttributesWithVarDimsByIdMetaServer.cpp 
            ops_rdma/server_separate_rtree/OpCatalogAllVarAttributesWithVarDimsByNameVerMetaServer.cpp    
            ops_rdma/server_separate_rtree/OpCatalogAllVarAttributesWithVarSubstrDimsMetaServer.cpp
            ops_rdma/server_separate_rtree/OpInsertVarAttributeByDimsBatchMetaServer.cpp 
            ops_rdma/server_separate_rtree/OpInsertVarAttributeByDimsMetaServer.cpp

            ops_rdma/server_separate_rtree/OpCatalogAllVarAttributesWithTypeVarSubstrMetaServer.cpp
            ops_rdma/server_separate_rtree/OpCatalogAllVarAttributesWithTypeVarSubstrRangeMetaServer.cpp
            ops_rdma/server_separate_rtree/OpCatalogAllVarAttributesWithVarSubstrMetaServer.cpp
            ops_rdma/server_separate_rtree/OpCatalogAllVarAttributesWithTypeVarRangeMetaServer.cpp
            ops_rdma/server_separate_rtree/OpCatalogAllVarAttributesMetaServer.cpp 
            ops_rdma/server_separate_rtree/OpCatalogAllVarAttributesWithTypeByIdMetaServer.cpp 
            ops_rdma/server_separate_rtree/OpCatalogAllVarAttributesWithTypeByNameVerMetaServer.cpp 
            ops_rdma/server_separate_rtree/OpCatalogAllVarAttributesWithTypeVarByIdMetaServer.cpp 
            ops_rdma/server_separate_rtree/OpCatalogAllVarAttributesWithTypeVarByNameVerMetaServer.cpp 
            ops_rdma/server_separate_rtree/OpCatalogAllVarAttributesWithVarByIdMetaServer.cpp 
            ops_rdma/server_separate_rtree/OpCatalogAllVarAttributesWithVarByNameVerMetaServer.cpp

            ops_rdma/server/OpCheckpointDatabaseMetaServer.cpp
            ops_rdma/server/OpInsertTimestepAttributeBatchMetaServer.cpp
            ops_rdma/server/OpInsertRunAttributeBatchMetaServer.cpp
            ops_rdma/server/OpCreateVarBatchMetaServer.cpp
            ops_rdma/server/OpCreateTypeBatchMetaServer.cpp
            ops_rdma/server/OpDeleteAllVarsWithSubstrMetaServer.cpp
            ops_rdma/server/OpCatalogAllTypesWithVarAttributesWithVarSubstrInTimestepMetaServer.cpp
            ops_rdma/server/OpCatalogAllTimestepsWithVarSubstrMetaServer.cpp
            ops_rdma/server/OpCatalogAllTimestepsWithVarAttributesWithTypeVarSubstrRangeMetaServer.cpp
            ops_rdma/server/OpCatalogAllTimestepsWithVarAttributesWithTypeVarSubstrMetaServer.cpp
            ops_rdma/server/OpProcessingMetaServer.cpp
            # ops_rdma/server/OpActivateMetaServer.cpp
            ops_rdma/server/OpActivateRunAttributeMetaServer.cpp
            ops_rdma/server/OpActivateRunMetaServer.cpp          
            ops_rdma/server/OpActivateTimestepAttributeMetaServer.cpp
            ops_rdma/server/OpActivateTimestepMetaServer.cpp
            ops_rdma/server/OpActivateTypeMetaServer.cpp   
            ops_rdma/server/OpActivateVarAttributeMetaServer.cpp          
            ops_rdma/server/OpActivateVarMetaServer.cpp
            ops_rdma/server/OpCatalogAllTimestepAttributesWithTypeRangeMetaServer.cpp
            ops_rdma/server/OpCatalogAllTimestepsWithVarAttributesWithTypeVarRangeMetaServer.cpp
            ops_rdma/server/OpCatalogAllRunAttributesWithTypeRangeMetaServer.cpp
            ops_rdma/server/OpCatalogAllRunAttributesMetaServer.cpp
            ops_rdma/server/OpCatalogAllRunAttributesWithTypeMetaServer.cpp
            ops_rdma/server/OpCatalogAllTimestepAttributesMetaServer.cpp
            ops_rdma/server/OpCatalogAllTimestepAttributesWithTypeMetaServer.cpp
            ops_rdma/server/OpCatalogAllTimestepsWithVarAttributesWithTypeVarMetaServer.cpp
            ops_rdma/server/OpCatalogAllTimestepsWithVarMetaServer.cpp
            ops_rdma/server/OpCatalogAllTypesWithVarAttributesInTimestepMetaServer.cpp
            ops_rdma/server/OpCatalogAllTypesWithVarAttributesWithVarInTimestepMetaServer.cpp
            ops_rdma/server/OpCatalogRunMetaServer.cpp             
            ops_rdma/server/OpCatalogTimestepMetaServer.cpp
            ops_rdma/server/OpCatalogTypeMetaServer.cpp             
            ops_rdma/server/OpCatalogVarMetaServer.cpp             
            ops_rdma/server/OpCreateRunMetaServer.cpp           
            ops_rdma/server/OpCreateTimestepMetaServer.cpp
            ops_rdma/server/OpCreateTypeMetaServer.cpp              
            ops_rdma/server/OpCreateVarMetaServer.cpp    
            ops_rdma/server/OpDeleteRunByIdMetaServer.cpp  
            ops_rdma/server/OpDeleteTimestepByIdMetaServer.cpp
            ops_rdma/server/OpDeleteTypeByIdMetaServer.cpp   
            ops_rdma/server/OpDeleteTypeByNameVerMetaServer.cpp           
            ops_rdma/server/OpDeleteVarByIdMetaServer.cpp  
            ops_rdma/server/OpDeleteVarByNamePathVerMetaServer.cpp  
            ops_rdma/server/OpFullShutdownMetaServer.cpp       
            ops_rdma/server/OpInsertRunAttributeMetaServer.cpp
            ops_rdma/server/OpInsertTimestepAttributeMetaServer.cpp
        )

        TARGET_INCLUDE_DIRECTORIES(OpsServerRtree PUBLIC
            ${CMAKE_CURRENT_SOURCE_DIR}/../include/ops_rdma
            # ${CMAKE_CURRENT_SOURCE_DIR}/../include/ops_rdma_sqlite_transactions_only
            # ${CMAKE_CURRENT_SOURCE_DIR}/../include/ops_rdma_server
            ${CMAKE_CURRENT_SOURCE_DIR}/../include/common
            ${CMAKE_CURRENT_SOURCE_DIR}/../include/database
            ${CMAKE_CURRENT_SOURCE_DIR}/../include/server
        )
    endif(MAKE_CLASS_PROJ_RTREE)
  

endif(MAKE_EMPRESS)


# if(MAKE_EMPRESS)

#     set(CLIENT_OPS
#         ops_rdma/common/OpCoreCommon.cpp  
#         ops_rdma/client/OpCoreClient.cpp

#         ops_rdma/client_sqlite_transactions_only/OpAbortTransactionMetaClient.cpp
#         ops_rdma/client_sqlite_transactions_only/OpBeginTransactionMetaClient.cpp       
#         ops_rdma/client_sqlite_transactions_only/OpCommitTransactionMetaClient.cpp 

#         ops_rdma/client/OpCheckpointDatabaseMetaClient.cpp

#         ops_rdma/client/OpInsertTimestepAttributeBatchMetaClient.cpp
#         ops_rdma/client/OpInsertRunAttributeBatchMetaClient.cpp
#         ops_rdma/client/OpCreateVarBatchMetaClient.cpp
#         ops_rdma/client/OpCreateTypeBatchMetaClient.cpp
#         ops_rdma/client/OpInsertVarAttributeByDimsBatchMetaClient.cpp
#         ops_rdma/client/OpDeleteAllVarsWithSubstrMetaClient.cpp
#         ops_rdma/client/OpCatalogAllVarAttributesWithVarSubstrDimsMetaClient.cpp
#         ops_rdma/client/OpCatalogAllVarAttributesWithTypeVarSubstrDimsMetaClient.cpp
#         ops_rdma/client/OpCatalogAllVarAttributesWithTypeVarSubstrMetaClient.cpp
#         ops_rdma/client/OpCatalogAllVarAttributesWithTypeVarSubstrRangeMetaClient.cpp
#         ops_rdma/client/OpCatalogAllVarAttributesWithTypeVarSubstrDimsRangeMetaClient.cpp
#         ops_rdma/client/OpCatalogAllTypesWithVarAttributesWithVarSubstrInTimestepMetaClient.cpp
#         ops_rdma/client/OpCatalogAllTypesWithVarAttributesWithVarSubstrDimsInTimestepMetaClient.cpp
#         ops_rdma/client/OpCatalogAllTimestepsWithVarSubstrMetaClient.cpp
#         ops_rdma/client/OpCatalogAllTimestepsWithVarAttributesWithTypeVarSubstrRangeMetaClient.cpp
#         ops_rdma/client/OpCatalogAllTimestepsWithVarAttributesWithTypeVarSubstrMetaClient.cpp
#         ops_rdma/client/OpCatalogAllTimestepsWithVarAttributesWithTypeVarSubstrDimsRangeMetaClient.cpp
#         ops_rdma/client/OpCatalogAllTimestepsWithVarAttributesWithTypeVarSubstrDimsMetaClient.cpp
#         ops_rdma/client/OpCatalogAllVarAttributesWithVarSubstrMetaClient.cpp
#         ops_rdma/client/OpProcessingMetaClient.cpp
#         # ops_rdma/client/OpActivateMetaClient.cpp
#         ops_rdma/client/OpActivateRunAttributeMetaClient.cpp
#         ops_rdma/client/OpActivateRunMetaClient.cpp          
#         ops_rdma/client/OpActivateTimestepAttributeMetaClient.cpp
#         ops_rdma/client/OpActivateTimestepMetaClient.cpp
#         ops_rdma/client/OpActivateTypeMetaClient.cpp   
#         ops_rdma/client/OpActivateVarAttributeMetaClient.cpp          
#         ops_rdma/client/OpActivateVarMetaClient.cpp
#         ops_rdma/client/OpCatalogAllVarAttributesWithTypeVarDimsRangeMetaClient.cpp
#         ops_rdma/client/OpCatalogAllVarAttributesWithTypeVarRangeMetaClient.cpp
#         ops_rdma/client/OpCatalogAllTimestepAttributesWithTypeRangeMetaClient.cpp
#         ops_rdma/client/OpCatalogAllTimestepsWithVarAttributesWithTypeVarDimsRangeMetaClient.cpp
#         ops_rdma/client/OpCatalogAllTimestepsWithVarAttributesWithTypeVarRangeMetaClient.cpp
#         ops_rdma/client/OpCatalogAllRunAttributesWithTypeRangeMetaClient.cpp
#         ops_rdma/client/OpCatalogAllRunAttributesMetaClient.cpp
#         ops_rdma/client/OpCatalogAllRunAttributesWithTypeMetaClient.cpp
#         ops_rdma/client/OpCatalogAllTimestepAttributesMetaClient.cpp
#         ops_rdma/client/OpCatalogAllTimestepAttributesWithTypeMetaClient.cpp
#         ops_rdma/client/OpCatalogAllTimestepsWithVarAttributesWithTypeVarDimsMetaClient.cpp
#         ops_rdma/client/OpCatalogAllTimestepsWithVarAttributesWithTypeVarMetaClient.cpp
#         ops_rdma/client/OpCatalogAllTimestepsWithVarMetaClient.cpp
#         ops_rdma/client/OpCatalogAllTypesWithVarAttributesInTimestepMetaClient.cpp
#         ops_rdma/client/OpCatalogAllTypesWithVarAttributesWithVarDimsInTimestepMetaClient.cpp
#         ops_rdma/client/OpCatalogAllTypesWithVarAttributesWithVarInTimestepMetaClient.cpp
#         ops_rdma/client/OpCatalogAllVarAttributesMetaClient.cpp 
#         ops_rdma/client/OpCatalogAllVarAttributesWithDimsMetaClient.cpp 
#         ops_rdma/client/OpCatalogAllVarAttributesWithTypeByIdMetaClient.cpp 
#         ops_rdma/client/OpCatalogAllVarAttributesWithTypeByNameVerMetaClient.cpp 
#         ops_rdma/client/OpCatalogAllVarAttributesWithTypeDimsByIdMetaClient.cpp 
#         ops_rdma/client/OpCatalogAllVarAttributesWithTypeDimsByNameVerMetaClient.cpp 
#         ops_rdma/client/OpCatalogAllVarAttributesWithTypeVarByIdMetaClient.cpp 
#         ops_rdma/client/OpCatalogAllVarAttributesWithTypeVarByNameVerMetaClient.cpp 
#         ops_rdma/client/OpCatalogAllVarAttributesWithTypeVarDimsByIdMetaClient.cpp 
#         ops_rdma/client/OpCatalogAllVarAttributesWithTypeVarDimsByNameVerMetaClient.cpp 
#         ops_rdma/client/OpCatalogAllVarAttributesWithVarByIdMetaClient.cpp 
#         ops_rdma/client/OpCatalogAllVarAttributesWithVarByNameVerMetaClient.cpp
#         ops_rdma/client/OpCatalogAllVarAttributesWithVarDimsByIdMetaClient.cpp 
#         ops_rdma/client/OpCatalogAllVarAttributesWithVarDimsByNameVerMetaClient.cpp    
#         ops_rdma/client/OpCatalogRunMetaClient.cpp             
#         ops_rdma/client/OpCatalogTimestepMetaClient.cpp
#         ops_rdma/client/OpCatalogTypeMetaClient.cpp             
#         ops_rdma/client/OpCatalogVarMetaClient.cpp             
#         ops_rdma/client/OpCreateRunMetaClient.cpp           
#         ops_rdma/client/OpCreateTimestepMetaClient.cpp
#         ops_rdma/client/OpCreateTypeMetaClient.cpp              
#         ops_rdma/client/OpCreateVarMetaClient.cpp       
#         ops_rdma/client/OpDeleteRunByIdMetaClient.cpp  
#         ops_rdma/client/OpDeleteTimestepByIdMetaClient.cpp
#         ops_rdma/client/OpDeleteTypeByIdMetaClient.cpp   
#         ops_rdma/client/OpDeleteTypeByNameVerMetaClient.cpp           
#         ops_rdma/client/OpDeleteVarByIdMetaClient.cpp  
#         ops_rdma/client/OpDeleteVarByNamePathVerMetaClient.cpp  
#         ops_rdma/client/OpFullShutdownMetaClient.cpp       
#         ops_rdma/client/OpInsertRunAttributeMetaClient.cpp
#         ops_rdma/client/OpInsertTimestepAttributeMetaClient.cpp
#         ops_rdma/client/OpInsertVarAttributeByDimsMetaClient.cpp
#     )

# 	add_library(OpsServer
#         ops_rdma/common/OpCoreCommon.cpp  
#         ops_rdma/server/OpCoreServer.cpp  

#         ops_rdma/server/sql_helper_functs.cpp

#         ops_rdma/server_sqlite_transactions_only/OpAbortTransactionMetaServer.cpp
#         ops_rdma/server_sqlite_transactions_only/OpBeginTransactionMetaServer.cpp       
#         ops_rdma/server_sqlite_transactions_only/OpCommitTransactionMetaServer.cpp

#         ops_rdma/server/OpCheckpointDatabaseMetaServer.cpp
#         ops_rdma/server/OpInsertTimestepAttributeBatchMetaServer.cpp
#         ops_rdma/server/OpInsertRunAttributeBatchMetaServer.cpp
#         ops_rdma/server/OpCreateVarBatchMetaServer.cpp
#         ops_rdma/server/OpCreateTypeBatchMetaServer.cpp
#         ops_rdma/server/OpInsertVarAttributeByDimsBatchMetaServer.cpp
#         ops_rdma/server/OpDeleteAllVarsWithSubstrMetaServer.cpp
#         ops_rdma/server/OpCatalogAllVarAttributesWithVarSubstrDimsMetaServer.cpp
#         ops_rdma/server/OpCatalogAllVarAttributesWithTypeVarSubstrDimsMetaServer.cpp
#         ops_rdma/server/OpCatalogAllVarAttributesWithTypeVarSubstrMetaServer.cpp
#         ops_rdma/server/OpCatalogAllVarAttributesWithTypeVarSubstrRangeMetaServer.cpp
#         ops_rdma/server/OpCatalogAllVarAttributesWithTypeVarSubstrDimsRangeMetaServer.cpp
#         ops_rdma/server/OpCatalogAllTypesWithVarAttributesWithVarSubstrInTimestepMetaServer.cpp
#         ops_rdma/server/OpCatalogAllTypesWithVarAttributesWithVarSubstrDimsInTimestepMetaServer.cpp
#         ops_rdma/server/OpCatalogAllTimestepsWithVarSubstrMetaServer.cpp
#         ops_rdma/server/OpCatalogAllTimestepsWithVarAttributesWithTypeVarSubstrRangeMetaServer.cpp
#         ops_rdma/server/OpCatalogAllTimestepsWithVarAttributesWithTypeVarSubstrMetaServer.cpp
#         ops_rdma/server/OpCatalogAllTimestepsWithVarAttributesWithTypeVarSubstrDimsRangeMetaServer.cpp
#         ops_rdma/server/OpCatalogAllTimestepsWithVarAttributesWithTypeVarSubstrDimsMetaServer.cpp
#         ops_rdma/server/OpCatalogAllVarAttributesWithVarSubstrMetaServer.cpp
#         ops_rdma/server/OpProcessingMetaServer.cpp
#         # ops_rdma/server/OpActivateMetaServer.cpp
#         ops_rdma/server/OpActivateRunAttributeMetaServer.cpp
#         ops_rdma/server/OpActivateRunMetaServer.cpp          
#         ops_rdma/server/OpActivateTimestepAttributeMetaServer.cpp
#         ops_rdma/server/OpActivateTimestepMetaServer.cpp
#         ops_rdma/server/OpActivateTypeMetaServer.cpp   
#         ops_rdma/server/OpActivateVarAttributeMetaServer.cpp          
#         ops_rdma/server/OpActivateVarMetaServer.cpp
#         ops_rdma/server/OpCatalogAllVarAttributesWithTypeVarDimsRangeMetaServer.cpp
#         ops_rdma/server/OpCatalogAllVarAttributesWithTypeVarRangeMetaServer.cpp
#         ops_rdma/server/OpCatalogAllTimestepAttributesWithTypeRangeMetaServer.cpp
#         ops_rdma/server/OpCatalogAllTimestepsWithVarAttributesWithTypeVarDimsRangeMetaServer.cpp
#         ops_rdma/server/OpCatalogAllTimestepsWithVarAttributesWithTypeVarRangeMetaServer.cpp
#         ops_rdma/server/OpCatalogAllRunAttributesWithTypeRangeMetaServer.cpp
#         ops_rdma/server/OpCatalogAllRunAttributesMetaServer.cpp
#         ops_rdma/server/OpCatalogAllRunAttributesWithTypeMetaServer.cpp
#         ops_rdma/server/OpCatalogAllTimestepAttributesMetaServer.cpp
#         ops_rdma/server/OpCatalogAllTimestepAttributesWithTypeMetaServer.cpp
#         ops_rdma/server/OpCatalogAllTimestepsWithVarAttributesWithTypeVarDimsMetaServer.cpp
#         ops_rdma/server/OpCatalogAllTimestepsWithVarAttributesWithTypeVarMetaServer.cpp
#         ops_rdma/server/OpCatalogAllTimestepsWithVarMetaServer.cpp
#         ops_rdma/server/OpCatalogAllTypesWithVarAttributesInTimestepMetaServer.cpp
#         ops_rdma/server/OpCatalogAllTypesWithVarAttributesWithVarDimsInTimestepMetaServer.cpp
#         ops_rdma/server/OpCatalogAllTypesWithVarAttributesWithVarInTimestepMetaServer.cpp
#         ops_rdma/server/OpCatalogAllVarAttributesMetaServer.cpp 
#         ops_rdma/server/OpCatalogAllVarAttributesWithDimsMetaServer.cpp 
#         ops_rdma/server/OpCatalogAllVarAttributesWithTypeByIdMetaServer.cpp 
#         ops_rdma/server/OpCatalogAllVarAttributesWithTypeByNameVerMetaServer.cpp 
#         ops_rdma/server/OpCatalogAllVarAttributesWithTypeDimsByIdMetaServer.cpp 
#         ops_rdma/server/OpCatalogAllVarAttributesWithTypeDimsByNameVerMetaServer.cpp 
#         ops_rdma/server/OpCatalogAllVarAttributesWithTypeVarByIdMetaServer.cpp 
#         ops_rdma/server/OpCatalogAllVarAttributesWithTypeVarByNameVerMetaServer.cpp 
#         ops_rdma/server/OpCatalogAllVarAttributesWithTypeVarDimsByIdMetaServer.cpp 
#         ops_rdma/server/OpCatalogAllVarAttributesWithTypeVarDimsByNameVerMetaServer.cpp 
#         ops_rdma/server/OpCatalogAllVarAttributesWithVarByIdMetaServer.cpp 
#         ops_rdma/server/OpCatalogAllVarAttributesWithVarByNameVerMetaServer.cpp
#         ops_rdma/server/OpCatalogAllVarAttributesWithVarDimsByIdMetaServer.cpp 
#         ops_rdma/server/OpCatalogAllVarAttributesWithVarDimsByNameVerMetaServer.cpp    
#         ops_rdma/server/OpCatalogRunMetaServer.cpp             
#         ops_rdma/server/OpCatalogTimestepMetaServer.cpp
#         ops_rdma/server/OpCatalogTypeMetaServer.cpp             
#         ops_rdma/server/OpCatalogVarMetaServer.cpp             
#         ops_rdma/server/OpCreateRunMetaServer.cpp           
#         ops_rdma/server/OpCreateTimestepMetaServer.cpp
#         ops_rdma/server/OpCreateTypeMetaServer.cpp              
#         ops_rdma/server/OpCreateVarMetaServer.cpp    
#         ops_rdma/server/OpDeleteRunByIdMetaServer.cpp  
#         ops_rdma/server/OpDeleteTimestepByIdMetaServer.cpp
#         ops_rdma/server/OpDeleteTypeByIdMetaServer.cpp   
#         ops_rdma/server/OpDeleteTypeByNameVerMetaServer.cpp           
#         ops_rdma/server/OpDeleteVarByIdMetaServer.cpp  
#         ops_rdma/server/OpDeleteVarByNamePathVerMetaServer.cpp  
#         ops_rdma/server/OpFullShutdownMetaServer.cpp       
#         ops_rdma/server/OpInsertRunAttributeMetaServer.cpp
#         ops_rdma/server/OpInsertTimestepAttributeMetaServer.cpp
#         ops_rdma/server/OpInsertVarAttributeByDimsMetaServer.cpp 
# 	)

# 	TARGET_INCLUDE_DIRECTORIES(OpsServer PUBLIC
#         ${CMAKE_CURRENT_SOURCE_DIR}/../include/ops_rdma
#         ${CMAKE_CURRENT_SOURCE_DIR}/../include/ops_rdma_sqlite_transactions_only
# 		# ${CMAKE_CURRENT_SOURCE_DIR}/../include/ops_rdma_server
# 		${CMAKE_CURRENT_SOURCE_DIR}/../include/common
# 		${CMAKE_CURRENT_SOURCE_DIR}/../include/database
# 	    ${CMAKE_CURRENT_SOURCE_DIR}/../include/server
# 	)

# 	add_library(OpsClient
# 	   ${CLIENT_OPS}             
# 	)

# 	TARGET_INCLUDE_DIRECTORIES(OpsClient PUBLIC
#         ${CMAKE_CURRENT_SOURCE_DIR}/../include/ops_rdma
#         ${CMAKE_CURRENT_SOURCE_DIR}/../include/ops_rdma_sqlite_transactions_only
# 		# ${CMAKE_CURRENT_SOURCE_DIR}/../include/ops_rdma_client
# 		${CMAKE_CURRENT_SOURCE_DIR}/../include/common
# 	    ${CMAKE_CURRENT_SOURCE_DIR}/../include/client
# 	)

# 	add_library(FullShutdownOpForDirman
# 		# ops_rdma/common/OpFullShutdownMetaCommon.cpp 
#         ops_rdma/common/OpCoreCommon.cpp
#         ops_rdma/server/OpCoreServer.cpp  
#         ops_rdma/server/OpFullShutdownMetaServer.cpp

# 	)
# 	TARGET_INCLUDE_DIRECTORIES(FullShutdownOpForDirman PUBLIC
# 		${CMAKE_CURRENT_SOURCE_DIR}/../include/ops_rdma
#         ${CMAKE_CURRENT_SOURCE_DIR}/../include/common
# 	)

#     if(MAKE_CLASS_PROJ_SQLITE_TRANSACTIONS_ONLY)
#         add_library(OpsServerSqliteTransactionOnly
#             ops_rdma/common/OpCoreCommon.cpp  
#             ops_rdma/server_sqlite_transactions_only/OpCoreServer.cpp  

#             # #only needed since we are using a single metadata_server source code rather than
#             # #a dedicated transaction one
#             # ops_rdma/server/OpProcessingMetaServer.cpp
#             # # ops_rdma/server/OpActivateMetaServer.cpp
#             # ops_rdma/server/OpActivateRunAttributeMetaServer.cpp
#             # ops_rdma/server/OpActivateRunMetaServer.cpp          
#             # ops_rdma/server/OpActivateTimestepAttributeMetaServer.cpp
#             # ops_rdma/server/OpActivateTimestepMetaServer.cpp
#             # ops_rdma/server/OpActivateTypeMetaServer.cpp   
#             # ops_rdma/server/OpActivateVarAttributeMetaServer.cpp          
#             # ops_rdma/server/OpActivateVarMetaServer.cpp

#             ops_rdma/server_sqlite_transactions_only/sql_helper_functs.cpp

#             ops_rdma/server_sqlite_transactions_only/OpAbortTransactionMetaServer.cpp
#             ops_rdma/server_sqlite_transactions_only/OpBeginTransactionMetaServer.cpp       
#             ops_rdma/server_sqlite_transactions_only/OpCommitTransactionMetaServer.cpp

#             ops_rdma/server_sqlite_transactions_only/OpCheckpointDatabaseMetaServer.cpp
#             ops_rdma/server_sqlite_transactions_only/OpInsertTimestepAttributeBatchMetaServer.cpp
#             ops_rdma/server_sqlite_transactions_only/OpInsertRunAttributeBatchMetaServer.cpp
#             ops_rdma/server_sqlite_transactions_only/OpCreateVarBatchMetaServer.cpp
#             ops_rdma/server_sqlite_transactions_only/OpCreateTypeBatchMetaServer.cpp
#             ops_rdma/server_sqlite_transactions_only/OpInsertVarAttributeByDimsBatchMetaServer.cpp
#             ops_rdma/server_sqlite_transactions_only/OpDeleteAllVarsWithSubstrMetaServer.cpp
#             ops_rdma/server_sqlite_transactions_only/OpCatalogAllVarAttributesWithVarSubstrDimsMetaServer.cpp
#             ops_rdma/server_sqlite_transactions_only/OpCatalogAllVarAttributesWithTypeVarSubstrDimsMetaServer.cpp
#             ops_rdma/server_sqlite_transactions_only/OpCatalogAllVarAttributesWithTypeVarSubstrMetaServer.cpp
#             ops_rdma/server_sqlite_transactions_only/OpCatalogAllVarAttributesWithTypeVarSubstrRangeMetaServer.cpp
#             ops_rdma/server_sqlite_transactions_only/OpCatalogAllVarAttributesWithTypeVarSubstrDimsRangeMetaServer.cpp
#             ops_rdma/server_sqlite_transactions_only/OpCatalogAllTypesWithVarAttributesWithVarSubstrInTimestepMetaServer.cpp
#             ops_rdma/server_sqlite_transactions_only/OpCatalogAllTypesWithVarAttributesWithVarSubstrDimsInTimestepMetaServer.cpp
#             ops_rdma/server_sqlite_transactions_only/OpCatalogAllTimestepsWithVarSubstrMetaServer.cpp
#             ops_rdma/server_sqlite_transactions_only/OpCatalogAllTimestepsWithVarAttributesWithTypeVarSubstrRangeMetaServer.cpp
#             ops_rdma/server_sqlite_transactions_only/OpCatalogAllTimestepsWithVarAttributesWithTypeVarSubstrMetaServer.cpp
#             ops_rdma/server_sqlite_transactions_only/OpCatalogAllTimestepsWithVarAttributesWithTypeVarSubstrDimsRangeMetaServer.cpp
#             ops_rdma/server_sqlite_transactions_only/OpCatalogAllTimestepsWithVarAttributesWithTypeVarSubstrDimsMetaServer.cpp
#             ops_rdma/server_sqlite_transactions_only/OpCatalogAllVarAttributesWithVarSubstrMetaServer.cpp
#             ops_rdma/server_sqlite_transactions_only/OpCatalogAllVarAttributesWithTypeVarDimsRangeMetaServer.cpp
#             ops_rdma/server_sqlite_transactions_only/OpCatalogAllVarAttributesWithTypeVarRangeMetaServer.cpp
#             ops_rdma/server_sqlite_transactions_only/OpCatalogAllTimestepAttributesWithTypeRangeMetaServer.cpp
#             ops_rdma/server_sqlite_transactions_only/OpCatalogAllTimestepsWithVarAttributesWithTypeVarDimsRangeMetaServer.cpp
#             ops_rdma/server_sqlite_transactions_only/OpCatalogAllTimestepsWithVarAttributesWithTypeVarRangeMetaServer.cpp
#             ops_rdma/server_sqlite_transactions_only/OpCatalogAllRunAttributesWithTypeRangeMetaServer.cpp
#             ops_rdma/server_sqlite_transactions_only/OpCatalogAllRunAttributesMetaServer.cpp
#             ops_rdma/server_sqlite_transactions_only/OpCatalogAllRunAttributesWithTypeMetaServer.cpp
#             ops_rdma/server_sqlite_transactions_only/OpCatalogAllTimestepAttributesMetaServer.cpp
#             ops_rdma/server_sqlite_transactions_only/OpCatalogAllTimestepAttributesWithTypeMetaServer.cpp
#             ops_rdma/server_sqlite_transactions_only/OpCatalogAllTimestepsWithVarAttributesWithTypeVarDimsMetaServer.cpp
#             ops_rdma/server_sqlite_transactions_only/OpCatalogAllTimestepsWithVarAttributesWithTypeVarMetaServer.cpp
#             ops_rdma/server_sqlite_transactions_only/OpCatalogAllTimestepsWithVarMetaServer.cpp
#             ops_rdma/server_sqlite_transactions_only/OpCatalogAllTypesWithVarAttributesInTimestepMetaServer.cpp
#             ops_rdma/server_sqlite_transactions_only/OpCatalogAllTypesWithVarAttributesWithVarDimsInTimestepMetaServer.cpp
#             ops_rdma/server_sqlite_transactions_only/OpCatalogAllTypesWithVarAttributesWithVarInTimestepMetaServer.cpp
#             ops_rdma/server_sqlite_transactions_only/OpCatalogAllVarAttributesMetaServer.cpp 
#             ops_rdma/server_sqlite_transactions_only/OpCatalogAllVarAttributesWithDimsMetaServer.cpp 
#             ops_rdma/server_sqlite_transactions_only/OpCatalogAllVarAttributesWithTypeByIdMetaServer.cpp 
#             ops_rdma/server_sqlite_transactions_only/OpCatalogAllVarAttributesWithTypeByNameVerMetaServer.cpp 
#             ops_rdma/server_sqlite_transactions_only/OpCatalogAllVarAttributesWithTypeDimsByIdMetaServer.cpp 
#             ops_rdma/server_sqlite_transactions_only/OpCatalogAllVarAttributesWithTypeDimsByNameVerMetaServer.cpp 
#             ops_rdma/server_sqlite_transactions_only/OpCatalogAllVarAttributesWithTypeVarByIdMetaServer.cpp 
#             ops_rdma/server_sqlite_transactions_only/OpCatalogAllVarAttributesWithTypeVarByNameVerMetaServer.cpp 
#             ops_rdma/server_sqlite_transactions_only/OpCatalogAllVarAttributesWithTypeVarDimsByIdMetaServer.cpp 
#             ops_rdma/server_sqlite_transactions_only/OpCatalogAllVarAttributesWithTypeVarDimsByNameVerMetaServer.cpp 
#             ops_rdma/server_sqlite_transactions_only/OpCatalogAllVarAttributesWithVarByIdMetaServer.cpp 
#             ops_rdma/server_sqlite_transactions_only/OpCatalogAllVarAttributesWithVarByNameVerMetaServer.cpp
#             ops_rdma/server_sqlite_transactions_only/OpCatalogAllVarAttributesWithVarDimsByIdMetaServer.cpp 
#             ops_rdma/server_sqlite_transactions_only/OpCatalogAllVarAttributesWithVarDimsByNameVerMetaServer.cpp    
#             ops_rdma/server_sqlite_transactions_only/OpCatalogRunMetaServer.cpp             
#             ops_rdma/server_sqlite_transactions_only/OpCatalogTimestepMetaServer.cpp
#             ops_rdma/server_sqlite_transactions_only/OpCatalogTypeMetaServer.cpp             
#             ops_rdma/server_sqlite_transactions_only/OpCatalogVarMetaServer.cpp             
#             ops_rdma/server_sqlite_transactions_only/OpCreateRunMetaServer.cpp           
#             ops_rdma/server_sqlite_transactions_only/OpCreateTimestepMetaServer.cpp
#             ops_rdma/server_sqlite_transactions_only/OpCreateTypeMetaServer.cpp              
#             ops_rdma/server_sqlite_transactions_only/OpCreateVarMetaServer.cpp    
#             ops_rdma/server_sqlite_transactions_only/OpDeleteRunByIdMetaServer.cpp  
#             ops_rdma/server_sqlite_transactions_only/OpDeleteTimestepByIdMetaServer.cpp
#             ops_rdma/server_sqlite_transactions_only/OpDeleteTypeByIdMetaServer.cpp   
#             ops_rdma/server_sqlite_transactions_only/OpDeleteTypeByNameVerMetaServer.cpp           
#             ops_rdma/server_sqlite_transactions_only/OpDeleteVarByIdMetaServer.cpp  
#             ops_rdma/server_sqlite_transactions_only/OpDeleteVarByNamePathVerMetaServer.cpp  
#             ops_rdma/server_sqlite_transactions_only/OpFullShutdownMetaServer.cpp       
#             ops_rdma/server_sqlite_transactions_only/OpInsertRunAttributeMetaServer.cpp
#             ops_rdma/server_sqlite_transactions_only/OpInsertTimestepAttributeMetaServer.cpp
#             ops_rdma/server_sqlite_transactions_only/OpInsertVarAttributeByDimsMetaServer.cpp 
#         )

#         TARGET_INCLUDE_DIRECTORIES(OpsServerSqliteTransactionOnly PUBLIC
#             ${CMAKE_CURRENT_SOURCE_DIR}/../include/ops_rdma
#             ${CMAKE_CURRENT_SOURCE_DIR}/../include/ops_rdma_sqlite_transactions_only
#             ${CMAKE_CURRENT_SOURCE_DIR}/../include/common_sqlite_transactions_only
#             ${CMAKE_CURRENT_SOURCE_DIR}/../include/database
#             ${CMAKE_CURRENT_SOURCE_DIR}/../include/server
#         )

#         add_library(OpsClientSqliteTransactionOnly
#            ${CLIENT_OPS}
#         )

#         TARGET_INCLUDE_DIRECTORIES(OpsClientSqliteTransactionOnly PUBLIC
#             ${CMAKE_CURRENT_SOURCE_DIR}/../include/ops_rdma
#             ${CMAKE_CURRENT_SOURCE_DIR}/../include/ops_rdma_sqlite_transactions_only
#             ${CMAKE_CURRENT_SOURCE_DIR}/../include/common_sqlite_transactions_only
#             ${CMAKE_CURRENT_SOURCE_DIR}/../include/client
#         )

#     endif(MAKE_CLASS_PROJ_SQLITE_TRANSACTIONS_ONLY)

# endif(MAKE_EMPRESS)



# if(MAKE_LOCAL)
# 	add_library(EmpressLocal
# 		local_db_functs/MdInsertTimestepAttributeBatch.cpp
# 		local_db_functs/MdInsertRunAttributeBatch.cpp
# 		local_db_functs/MdCreateVarBatch.cpp
# 		local_db_functs/MdCreateTypeBatch.cpp
# 		local_db_functs/MdInsertVarAttributeByDimsBatch.cpp
# 		local_db_functs/MdDeleteAllVarsWithSubstr.cpp
# 		local_db_functs/MdCatalogAllVarAttributesWithVarSubstrDims.cpp
# 		local_db_functs/MdCatalogAllVarAttributesWithTypeVarSubstrDims.cpp
# 		local_db_functs/MdCatalogAllVarAttributesWithTypeVarSubstr.cpp
# 		local_db_functs/MdCatalogAllVarAttributesWithTypeVarSubstrRange.cpp
# 		local_db_functs/MdCatalogAllVarAttributesWithTypeVarSubstrDimsRange.cpp
# 		local_db_functs/MdCatalogAllTypesWithVarAttributesWithVarSubstrInTimestep.cpp
# 		local_db_functs/MdCatalogAllTypesWithVarAttributesWithVarSubstrDimsInTimestep.cpp
# 		local_db_functs/MdCatalogAllTimestepsWithVarSubstr.cpp
# 		local_db_functs/MdCatalogAllTimestepsWithVarAttributesWithTypeVarSubstrRange.cpp
# 		local_db_functs/MdCatalogAllTimestepsWithVarAttributesWithTypeVarSubstr.cpp
# 		local_db_functs/MdCatalogAllTimestepsWithVarAttributesWithTypeVarSubstrDimsRange.cpp
# 		local_db_functs/MdCatalogAllTimestepsWithVarAttributesWithTypeVarSubstrDims.cpp
# 		local_db_functs/MdCatalogAllVarAttributesWithVarSubstr.cpp
# 		local_db_functs/MdProcessing.cpp
# 		local_db_functs/MdActivate.cpp
# 		local_db_functs/MdCatalogAllVarAttributesWithTypeVarDimsRange.cpp
# 		local_db_functs/MdCatalogAllVarAttributesWithTypeVarRange.cpp
# 		local_db_functs/MdCatalogAllTimestepAttributesWithTypeRange.cpp
# 		local_db_functs/MdCatalogAllTimestepsWithVarAttributesWithTypeVarDimsRange.cpp
# 		local_db_functs/MdCatalogAllTimestepsWithVarAttributesWithTypeVarRange.cpp
# 		local_db_functs/MdCatalogAllRunAttributesWithTypeRange.cpp
# 		local_db_functs/MdCatalogAllRunAttributes.cpp
# 		local_db_functs/MdCatalogAllRunAttributesWithType.cpp
# 		local_db_functs/MdCatalogAllTimestepAttributes.cpp
# 		local_db_functs/MdCatalogAllTimestepAttributesWithType.cpp
# 		local_db_functs/MdCatalogAllTimestepsWithVarAttributesWithTypeVarDims.cpp
# 		local_db_functs/MdCatalogAllTimestepsWithVarAttributesWithTypeVar.cpp
# 		local_db_functs/MdCatalogAllTimestepsWithVar.cpp
# 		local_db_functs/MdCatalogAllTypesWithVarAttributesInTimestep.cpp
# 		local_db_functs/MdCatalogAllTypesWithVarAttributesWithVarDimsInTimestep.cpp
# 		local_db_functs/MdCatalogAllTypesWithVarAttributesWithVarInTimestep.cpp
# 		local_db_functs/MdCatalogAllVarAttributes.cpp 
# 		local_db_functs/MdCatalogAllVarAttributesWithDims.cpp 
# 		local_db_functs/MdCatalogAllVarAttributesWithTypeById.cpp 
# 		local_db_functs/MdCatalogAllVarAttributesWithTypeByNameVer.cpp 
# 		local_db_functs/MdCatalogAllVarAttributesWithTypeDimsById.cpp 
# 		local_db_functs/MdCatalogAllVarAttributesWithTypeDimsByNameVer.cpp 
# 		local_db_functs/MdCatalogAllVarAttributesWithTypeVarById.cpp 
# 		local_db_functs/MdCatalogAllVarAttributesWithTypeVarByNameVer.cpp 
# 		local_db_functs/MdCatalogAllVarAttributesWithTypeVarDimsById.cpp 
# 		local_db_functs/MdCatalogAllVarAttributesWithTypeVarDimsByNameVer.cpp 
# 		local_db_functs/MdCatalogAllVarAttributesWithVarById.cpp 
# 		local_db_functs/MdCatalogAllVarAttributesWithVarByNameVer.cpp
# 		local_db_functs/MdCatalogAllVarAttributesWithVarDimsById.cpp 
# 		local_db_functs/MdCatalogAllVarAttributesWithVarDimsByNameVer.cpp    
# 		local_db_functs/MdCatalogRun.cpp             
# 		local_db_functs/MdCatalogTimestep.cpp
# 		local_db_functs/MdCatalogType.cpp             
# 		local_db_functs/MdCatalogVar.cpp             
# 		local_db_functs/MdCreateRun.cpp           
# 		local_db_functs/MdCreateTimestep.cpp
# 		local_db_functs/MdCreateType.cpp           	
# 		local_db_functs/MdCreateVar.cpp       
# 		local_db_functs/MdDeleteRunById.cpp  
# 		local_db_functs/MdDeleteTimestepById.cpp
# 		local_db_functs/MdDeleteTypeById.cpp   
# 		local_db_functs/MdDeleteTypeByNameVer.cpp           
# 		local_db_functs/MdDeleteVarById.cpp  
# 		local_db_functs/MdDeleteVarByNamePathVer.cpp  
# 		local_db_functs/MdInsertRunAttribute.cpp
# 		local_db_functs/MdInsertTimestepAttribute.cpp
# 		local_db_functs/MdInsertVarAttributeByDims.cpp	
		
# 	)

# 	TARGET_INCLUDE_DIRECTORIES(EmpressLocal PUBLIC
# 		${CMAKE_CURRENT_SOURCE_DIR}/../include/database
# 		${CMAKE_CURRENT_SOURCE_DIR}/../include/local	
# 	)
# endif(MAKE_LOCAL)

