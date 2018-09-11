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

project(lib VERSION 0.1 LANGUAGES CXX)

add_library(Database SHARED
    database/sqlite3.c
)

TARGET_INCLUDE_DIRECTORIES(Database PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../include/database
)


add_library(OpsCommon
	ops/common/OpInsertTimestepAttributeBatchMetaCommon.cpp
	ops/common/OpInsertRunAttributeBatchMetaCommon.cpp
	ops/common/OpCreateVarBatchMetaCommon.cpp
	ops/common/OpCreateTypeBatchMetaCommon.cpp
	ops/common/OpInsertVarAttributeByDimsBatchMetaCommon.cpp
	ops/common/OpDeleteAllVarsWithSubstrMetaCommon.cpp
	ops/common/OpCatalogAllVarAttributesWithVarSubstrDimsMetaCommon.cpp
	ops/common/OpCatalogAllVarAttributesWithTypeVarSubstrDimsMetaCommon.cpp
	ops/common/OpCatalogAllVarAttributesWithTypeVarSubstrMetaCommon.cpp
	ops/common/OpCatalogAllVarAttributesWithTypeVarSubstrRangeMetaCommon.cpp
	ops/common/OpCatalogAllVarAttributesWithTypeVarSubstrDimsRangeMetaCommon.cpp
	ops/common/OpCatalogAllTypesWithVarAttributesWithVarSubstrInTimestepMetaCommon.cpp
	ops/common/OpCatalogAllTypesWithVarAttributesWithVarSubstrDimsInTimestepMetaCommon.cpp
	ops/common/OpCatalogAllTimestepsWithVarSubstrMetaCommon.cpp
	ops/common/OpCatalogAllTimestepsWithVarAttributesWithTypeVarSubstrRangeMetaCommon.cpp
	ops/common/OpCatalogAllTimestepsWithVarAttributesWithTypeVarSubstrMetaCommon.cpp
	ops/common/OpCatalogAllTimestepsWithVarAttributesWithTypeVarSubstrDimsRangeMetaCommon.cpp
	ops/common/OpCatalogAllTimestepsWithVarAttributesWithTypeVarSubstrDimsMetaCommon.cpp
	ops/common/OpCatalogAllVarAttributesWithVarSubstrMetaCommon.cpp
	ops/common/OpProcessingMetaCommon.cpp
	ops/common/OpActivateMetaCommon.cpp
	ops/common/OpCatalogAllVarAttributesWithTypeVarDimsRangeMetaCommon.cpp
	ops/common/OpCatalogAllVarAttributesWithTypeVarRangeMetaCommon.cpp
	ops/common/OpCatalogAllTimestepAttributesWithTypeRangeMetaCommon.cpp
	ops/common/OpCatalogAllTimestepsWithVarAttributesWithTypeVarDimsRangeMetaCommon.cpp
	ops/common/OpCatalogAllTimestepsWithVarAttributesWithTypeVarRangeMetaCommon.cpp
	ops/common/OpCatalogAllRunAttributesWithTypeRangeMetaCommon.cpp
	ops/common/OpCatalogAllRunAttributesMetaCommon.cpp
	ops/common/OpCatalogAllRunAttributesWithTypeMetaCommon.cpp
	ops/common/OpCatalogAllTimestepAttributesMetaCommon.cpp
	ops/common/OpCatalogAllTimestepAttributesWithTypeMetaCommon.cpp
	ops/common/OpCatalogAllTimestepsWithVarAttributesWithTypeVarDimsMetaCommon.cpp
	ops/common/OpCatalogAllTimestepsWithVarAttributesWithTypeVarMetaCommon.cpp
	ops/common/OpCatalogAllTimestepsWithVarMetaCommon.cpp
	ops/common/OpCatalogAllTypesWithVarAttributesInTimestepMetaCommon.cpp
	ops/common/OpCatalogAllTypesWithVarAttributesWithVarDimsInTimestepMetaCommon.cpp
	ops/common/OpCatalogAllTypesWithVarAttributesWithVarInTimestepMetaCommon.cpp
	ops/common/OpCatalogAllVarAttributesMetaCommon.cpp 
	ops/common/OpCatalogAllVarAttributesWithDimsMetaCommon.cpp 
	ops/common/OpCatalogAllVarAttributesWithTypeByIdMetaCommon.cpp 
	ops/common/OpCatalogAllVarAttributesWithTypeByNameVerMetaCommon.cpp 
	ops/common/OpCatalogAllVarAttributesWithTypeDimsByIdMetaCommon.cpp 
	ops/common/OpCatalogAllVarAttributesWithTypeDimsByNameVerMetaCommon.cpp 
	ops/common/OpCatalogAllVarAttributesWithTypeVarByIdMetaCommon.cpp 
	ops/common/OpCatalogAllVarAttributesWithTypeVarByNameVerMetaCommon.cpp 
	ops/common/OpCatalogAllVarAttributesWithTypeVarDimsByIdMetaCommon.cpp 
	ops/common/OpCatalogAllVarAttributesWithTypeVarDimsByNameVerMetaCommon.cpp 
	ops/common/OpCatalogAllVarAttributesWithVarByIdMetaCommon.cpp 
	ops/common/OpCatalogAllVarAttributesWithVarByNameVerMetaCommon.cpp
	ops/common/OpCatalogAllVarAttributesWithVarDimsByIdMetaCommon.cpp 
	ops/common/OpCatalogAllVarAttributesWithVarDimsByNameVerMetaCommon.cpp    
	ops/common/OpCatalogRunMetaCommon.cpp             
	ops/common/OpCatalogTimestepMetaCommon.cpp
	ops/common/OpCatalogTypeMetaCommon.cpp             
	ops/common/OpCatalogVarMetaCommon.cpp             
	ops/common/OpCreateRunMetaCommon.cpp           
	ops/common/OpCreateTimestepMetaCommon.cpp
	ops/common/OpCreateTypeMetaCommon.cpp           	
	ops/common/OpCreateVarMetaCommon.cpp       
	ops/common/OpDeleteRunByIdMetaCommon.cpp  
	ops/common/OpDeleteTimestepByIdMetaCommon.cpp
	ops/common/OpDeleteTypeByIdMetaCommon.cpp   
	ops/common/OpDeleteTypeByNameVerMetaCommon.cpp           
	ops/common/OpDeleteVarByIdMetaCommon.cpp  
	ops/common/OpDeleteVarByNamePathVerMetaCommon.cpp  
	ops/common/OpFullShutdownMetaCommon.cpp       
	ops/common/OpInsertRunAttributeMetaCommon.cpp
	ops/common/OpInsertTimestepAttributeMetaCommon.cpp
	ops/common/OpInsertVarAttributeByDimsMetaCommon.cpp	

)

TARGET_INCLUDE_DIRECTORIES(OpsCommon PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/../include/ops
	${CMAKE_CURRENT_SOURCE_DIR}/../include/common
)


add_library(OpsServer
	ops/server/OpInsertTimestepAttributeBatchMetaServer.cpp
	ops/server/OpInsertRunAttributeBatchMetaServer.cpp
	ops/server/OpCreateVarBatchMetaServer.cpp
	ops/server/OpCreateTypeBatchMetaServer.cpp
	ops/server/OpInsertVarAttributeByDimsBatchMetaServer.cpp
	ops/server/OpDeleteAllVarsWithSubstrMetaServer.cpp
	ops/server/OpCatalogAllVarAttributesWithVarSubstrDimsMetaServer.cpp
	ops/server/OpCatalogAllVarAttributesWithTypeVarSubstrDimsMetaServer.cpp
	ops/server/OpCatalogAllVarAttributesWithTypeVarSubstrMetaServer.cpp
	ops/server/OpCatalogAllVarAttributesWithTypeVarSubstrRangeMetaServer.cpp
	ops/server/OpCatalogAllVarAttributesWithTypeVarSubstrDimsRangeMetaServer.cpp
	ops/server/OpCatalogAllTypesWithVarAttributesWithVarSubstrInTimestepMetaServer.cpp
	ops/server/OpCatalogAllTypesWithVarAttributesWithVarSubstrDimsInTimestepMetaServer.cpp
	ops/server/OpCatalogAllTimestepsWithVarSubstrMetaServer.cpp
	ops/server/OpCatalogAllTimestepsWithVarAttributesWithTypeVarSubstrRangeMetaServer.cpp
	ops/server/OpCatalogAllTimestepsWithVarAttributesWithTypeVarSubstrMetaServer.cpp
	ops/server/OpCatalogAllTimestepsWithVarAttributesWithTypeVarSubstrDimsRangeMetaServer.cpp
	ops/server/OpCatalogAllTimestepsWithVarAttributesWithTypeVarSubstrDimsMetaServer.cpp
	ops/server/OpCatalogAllVarAttributesWithVarSubstrMetaServer.cpp
	ops/server/OpProcessingMetaServer.cpp
	ops/server/OpActivateMetaServer.cpp
	ops/server/OpCatalogAllVarAttributesWithTypeVarDimsRangeMetaServer.cpp
	ops/server/OpCatalogAllVarAttributesWithTypeVarRangeMetaServer.cpp
	ops/server/OpCatalogAllTimestepAttributesWithTypeRangeMetaServer.cpp
	ops/server/OpCatalogAllTimestepsWithVarAttributesWithTypeVarDimsRangeMetaServer.cpp
	ops/server/OpCatalogAllTimestepsWithVarAttributesWithTypeVarRangeMetaServer.cpp
	ops/server/OpCatalogAllRunAttributesWithTypeRangeMetaServer.cpp
	ops/server/OpCatalogAllRunAttributesMetaServer.cpp
	ops/server/OpCatalogAllRunAttributesWithTypeMetaServer.cpp
	ops/server/OpCatalogAllTimestepAttributesMetaServer.cpp
	ops/server/OpCatalogAllTimestepAttributesWithTypeMetaServer.cpp
	ops/server/OpCatalogAllTimestepsWithVarAttributesWithTypeVarDimsMetaServer.cpp
	ops/server/OpCatalogAllTimestepsWithVarAttributesWithTypeVarMetaServer.cpp
	ops/server/OpCatalogAllTimestepsWithVarMetaServer.cpp
	ops/server/OpCatalogAllTypesWithVarAttributesInTimestepMetaServer.cpp
	ops/server/OpCatalogAllTypesWithVarAttributesWithVarDimsInTimestepMetaServer.cpp
	ops/server/OpCatalogAllTypesWithVarAttributesWithVarInTimestepMetaServer.cpp
	ops/server/OpCatalogAllVarAttributesMetaServer.cpp 
	ops/server/OpCatalogAllVarAttributesWithDimsMetaServer.cpp 
	ops/server/OpCatalogAllVarAttributesWithTypeByIdMetaServer.cpp 
	ops/server/OpCatalogAllVarAttributesWithTypeByNameVerMetaServer.cpp 
	ops/server/OpCatalogAllVarAttributesWithTypeDimsByIdMetaServer.cpp 
	ops/server/OpCatalogAllVarAttributesWithTypeDimsByNameVerMetaServer.cpp 
	ops/server/OpCatalogAllVarAttributesWithTypeVarByIdMetaServer.cpp 
	ops/server/OpCatalogAllVarAttributesWithTypeVarByNameVerMetaServer.cpp 
	ops/server/OpCatalogAllVarAttributesWithTypeVarDimsByIdMetaServer.cpp 
	ops/server/OpCatalogAllVarAttributesWithTypeVarDimsByNameVerMetaServer.cpp 
	ops/server/OpCatalogAllVarAttributesWithVarByIdMetaServer.cpp 
	ops/server/OpCatalogAllVarAttributesWithVarByNameVerMetaServer.cpp
	ops/server/OpCatalogAllVarAttributesWithVarDimsByIdMetaServer.cpp 
	ops/server/OpCatalogAllVarAttributesWithVarDimsByNameVerMetaServer.cpp    
	ops/server/OpCatalogRunMetaServer.cpp             
	ops/server/OpCatalogTimestepMetaServer.cpp
	ops/server/OpCatalogTypeMetaServer.cpp             
	ops/server/OpCatalogVarMetaServer.cpp             
	ops/server/OpCreateRunMetaServer.cpp           
	ops/server/OpCreateTimestepMetaServer.cpp
	ops/server/OpCreateTypeMetaServer.cpp           	
	ops/server/OpCreateVarMetaServer.cpp    
	ops/server/OpDeleteRunByIdMetaServer.cpp  
	ops/server/OpDeleteTimestepByIdMetaServer.cpp
	ops/server/OpDeleteTypeByIdMetaServer.cpp   
	ops/server/OpDeleteTypeByNameVerMetaServer.cpp           
	ops/server/OpDeleteVarByIdMetaServer.cpp  
	ops/server/OpDeleteVarByNamePathVerMetaServer.cpp  
	ops/server/OpFullShutdownMetaServer.cpp       
	ops/server/OpInsertRunAttributeMetaServer.cpp
	ops/server/OpInsertTimestepAttributeMetaServer.cpp
	ops/server/OpInsertVarAttributeByDimsMetaServer.cpp	
)

TARGET_INCLUDE_DIRECTORIES(OpsServer PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/../include/ops	
	${CMAKE_CURRENT_SOURCE_DIR}/../include/common
	${CMAKE_CURRENT_SOURCE_DIR}/../include/database
    ${CMAKE_CURRENT_SOURCE_DIR}/../include/server
)

add_library(OpsClient
	ops/client/OpInsertTimestepAttributeBatchMetaClient.cpp
	ops/client/OpInsertRunAttributeBatchMetaClient.cpp
	ops/client/OpCreateVarBatchMetaClient.cpp
	ops/client/OpCreateTypeBatchMetaClient.cpp
	ops/client/OpInsertVarAttributeByDimsBatchMetaClient.cpp
	ops/client/OpDeleteAllVarsWithSubstrMetaClient.cpp
	ops/client/OpCatalogAllVarAttributesWithVarSubstrDimsMetaClient.cpp
	ops/client/OpCatalogAllVarAttributesWithTypeVarSubstrDimsMetaClient.cpp
	ops/client/OpCatalogAllVarAttributesWithTypeVarSubstrMetaClient.cpp
	ops/client/OpCatalogAllVarAttributesWithTypeVarSubstrRangeMetaClient.cpp
	ops/client/OpCatalogAllVarAttributesWithTypeVarSubstrDimsRangeMetaClient.cpp
	ops/client/OpCatalogAllTypesWithVarAttributesWithVarSubstrInTimestepMetaClient.cpp
	ops/client/OpCatalogAllTypesWithVarAttributesWithVarSubstrDimsInTimestepMetaClient.cpp
	ops/client/OpCatalogAllTimestepsWithVarSubstrMetaClient.cpp
	ops/client/OpCatalogAllTimestepsWithVarAttributesWithTypeVarSubstrRangeMetaClient.cpp
	ops/client/OpCatalogAllTimestepsWithVarAttributesWithTypeVarSubstrMetaClient.cpp
	ops/client/OpCatalogAllTimestepsWithVarAttributesWithTypeVarSubstrDimsRangeMetaClient.cpp
	ops/client/OpCatalogAllTimestepsWithVarAttributesWithTypeVarSubstrDimsMetaClient.cpp
	ops/client/OpCatalogAllVarAttributesWithVarSubstrMetaClient.cpp
	ops/client/OpProcessingMetaClient.cpp
	ops/client/OpActivateMetaClient.cpp
	ops/client/OpCatalogAllVarAttributesWithTypeVarDimsRangeMetaClient.cpp
	ops/client/OpCatalogAllVarAttributesWithTypeVarRangeMetaClient.cpp
	ops/client/OpCatalogAllTimestepAttributesWithTypeRangeMetaClient.cpp
	ops/client/OpCatalogAllTimestepsWithVarAttributesWithTypeVarDimsRangeMetaClient.cpp
	ops/client/OpCatalogAllTimestepsWithVarAttributesWithTypeVarRangeMetaClient.cpp
	ops/client/OpCatalogAllRunAttributesWithTypeRangeMetaClient.cpp
	ops/client/OpCatalogAllRunAttributesMetaClient.cpp
	ops/client/OpCatalogAllRunAttributesWithTypeMetaClient.cpp
	ops/client/OpCatalogAllTimestepAttributesMetaClient.cpp
	ops/client/OpCatalogAllTimestepAttributesWithTypeMetaClient.cpp
	ops/client/OpCatalogAllTimestepsWithVarAttributesWithTypeVarDimsMetaClient.cpp
	ops/client/OpCatalogAllTimestepsWithVarAttributesWithTypeVarMetaClient.cpp
	ops/client/OpCatalogAllTimestepsWithVarMetaClient.cpp
	ops/client/OpCatalogAllTypesWithVarAttributesInTimestepMetaClient.cpp
	ops/client/OpCatalogAllTypesWithVarAttributesWithVarDimsInTimestepMetaClient.cpp
	ops/client/OpCatalogAllTypesWithVarAttributesWithVarInTimestepMetaClient.cpp
	ops/client/OpCatalogAllVarAttributesMetaClient.cpp 
	ops/client/OpCatalogAllVarAttributesWithDimsMetaClient.cpp 
	ops/client/OpCatalogAllVarAttributesWithTypeByIdMetaClient.cpp 
	ops/client/OpCatalogAllVarAttributesWithTypeByNameVerMetaClient.cpp 
	ops/client/OpCatalogAllVarAttributesWithTypeDimsByIdMetaClient.cpp 
	ops/client/OpCatalogAllVarAttributesWithTypeDimsByNameVerMetaClient.cpp 
	ops/client/OpCatalogAllVarAttributesWithTypeVarByIdMetaClient.cpp 
	ops/client/OpCatalogAllVarAttributesWithTypeVarByNameVerMetaClient.cpp 
	ops/client/OpCatalogAllVarAttributesWithTypeVarDimsByIdMetaClient.cpp 
	ops/client/OpCatalogAllVarAttributesWithTypeVarDimsByNameVerMetaClient.cpp 
	ops/client/OpCatalogAllVarAttributesWithVarByIdMetaClient.cpp 
	ops/client/OpCatalogAllVarAttributesWithVarByNameVerMetaClient.cpp
	ops/client/OpCatalogAllVarAttributesWithVarDimsByIdMetaClient.cpp 
	ops/client/OpCatalogAllVarAttributesWithVarDimsByNameVerMetaClient.cpp    
	ops/client/OpCatalogRunMetaClient.cpp             
	ops/client/OpCatalogTimestepMetaClient.cpp
	ops/client/OpCatalogTypeMetaClient.cpp             
	ops/client/OpCatalogVarMetaClient.cpp             
	ops/client/OpCreateRunMetaClient.cpp           
	ops/client/OpCreateTimestepMetaClient.cpp
	ops/client/OpCreateTypeMetaClient.cpp           	
	ops/client/OpCreateVarMetaClient.cpp       
	ops/client/OpDeleteRunByIdMetaClient.cpp  
	ops/client/OpDeleteTimestepByIdMetaClient.cpp
	ops/client/OpDeleteTypeByIdMetaClient.cpp   
	ops/client/OpDeleteTypeByNameVerMetaClient.cpp           
	ops/client/OpDeleteVarByIdMetaClient.cpp  
	ops/client/OpDeleteVarByNamePathVerMetaClient.cpp  
	ops/client/OpFullShutdownMetaClient.cpp       
	ops/client/OpInsertRunAttributeMetaClient.cpp
	ops/client/OpInsertTimestepAttributeMetaClient.cpp
	ops/client/OpInsertVarAttributeByDimsMetaClient.cpp	
	
)

TARGET_INCLUDE_DIRECTORIES(OpsClient PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/../include/ops
	${CMAKE_CURRENT_SOURCE_DIR}/../include/common
    ${CMAKE_CURRENT_SOURCE_DIR}/../include/client
)

add_library(FullShutdownOpForDirman
	ops/server/OpFullShutdownMetaServer.cpp
	ops/common/OpFullShutdownMetaCommon.cpp         
)

TARGET_INCLUDE_DIRECTORIES(FullShutdownOpForDirman PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/../include/ops	
)



add_library(EmpressLocal
	local_db_functs/MdInsertTimestepAttributeBatch.cpp
	local_db_functs/MdInsertRunAttributeBatch.cpp
	local_db_functs/MdCreateVarBatch.cpp
	local_db_functs/MdCreateTypeBatch.cpp
	local_db_functs/MdInsertVarAttributeByDimsBatch.cpp
	local_db_functs/MdDeleteAllVarsWithSubstr.cpp
	local_db_functs/MdCatalogAllVarAttributesWithVarSubstrDims.cpp
	local_db_functs/MdCatalogAllVarAttributesWithTypeVarSubstrDims.cpp
	local_db_functs/MdCatalogAllVarAttributesWithTypeVarSubstr.cpp
	local_db_functs/MdCatalogAllVarAttributesWithTypeVarSubstrRange.cpp
	local_db_functs/MdCatalogAllVarAttributesWithTypeVarSubstrDimsRange.cpp
	local_db_functs/MdCatalogAllTypesWithVarAttributesWithVarSubstrInTimestep.cpp
	local_db_functs/MdCatalogAllTypesWithVarAttributesWithVarSubstrDimsInTimestep.cpp
	local_db_functs/MdCatalogAllTimestepsWithVarSubstr.cpp
	local_db_functs/MdCatalogAllTimestepsWithVarAttributesWithTypeVarSubstrRange.cpp
	local_db_functs/MdCatalogAllTimestepsWithVarAttributesWithTypeVarSubstr.cpp
	local_db_functs/MdCatalogAllTimestepsWithVarAttributesWithTypeVarSubstrDimsRange.cpp
	local_db_functs/MdCatalogAllTimestepsWithVarAttributesWithTypeVarSubstrDims.cpp
	local_db_functs/MdCatalogAllVarAttributesWithVarSubstr.cpp
	local_db_functs/MdProcessing.cpp
	local_db_functs/MdActivate.cpp
	local_db_functs/MdCatalogAllVarAttributesWithTypeVarDimsRange.cpp
	local_db_functs/MdCatalogAllVarAttributesWithTypeVarRange.cpp
	local_db_functs/MdCatalogAllTimestepAttributesWithTypeRange.cpp
	local_db_functs/MdCatalogAllTimestepsWithVarAttributesWithTypeVarDimsRange.cpp
	local_db_functs/MdCatalogAllTimestepsWithVarAttributesWithTypeVarRange.cpp
	local_db_functs/MdCatalogAllRunAttributesWithTypeRange.cpp
	local_db_functs/MdCatalogAllRunAttributes.cpp
	local_db_functs/MdCatalogAllRunAttributesWithType.cpp
	local_db_functs/MdCatalogAllTimestepAttributes.cpp
	local_db_functs/MdCatalogAllTimestepAttributesWithType.cpp
	local_db_functs/MdCatalogAllTimestepsWithVarAttributesWithTypeVarDims.cpp
	local_db_functs/MdCatalogAllTimestepsWithVarAttributesWithTypeVar.cpp
	local_db_functs/MdCatalogAllTimestepsWithVar.cpp
	local_db_functs/MdCatalogAllTypesWithVarAttributesInTimestep.cpp
	local_db_functs/MdCatalogAllTypesWithVarAttributesWithVarDimsInTimestep.cpp
	local_db_functs/MdCatalogAllTypesWithVarAttributesWithVarInTimestep.cpp
	local_db_functs/MdCatalogAllVarAttributes.cpp 
	local_db_functs/MdCatalogAllVarAttributesWithDims.cpp 
	local_db_functs/MdCatalogAllVarAttributesWithTypeById.cpp 
	local_db_functs/MdCatalogAllVarAttributesWithTypeByNameVer.cpp 
	local_db_functs/MdCatalogAllVarAttributesWithTypeDimsById.cpp 
	local_db_functs/MdCatalogAllVarAttributesWithTypeDimsByNameVer.cpp 
	local_db_functs/MdCatalogAllVarAttributesWithTypeVarById.cpp 
	local_db_functs/MdCatalogAllVarAttributesWithTypeVarByNameVer.cpp 
	local_db_functs/MdCatalogAllVarAttributesWithTypeVarDimsById.cpp 
	local_db_functs/MdCatalogAllVarAttributesWithTypeVarDimsByNameVer.cpp 
	local_db_functs/MdCatalogAllVarAttributesWithVarById.cpp 
	local_db_functs/MdCatalogAllVarAttributesWithVarByNameVer.cpp
	local_db_functs/MdCatalogAllVarAttributesWithVarDimsById.cpp 
	local_db_functs/MdCatalogAllVarAttributesWithVarDimsByNameVer.cpp    
	local_db_functs/MdCatalogRun.cpp             
	local_db_functs/MdCatalogTimestep.cpp
	local_db_functs/MdCatalogType.cpp             
	local_db_functs/MdCatalogVar.cpp             
	local_db_functs/MdCreateRun.cpp           
	local_db_functs/MdCreateTimestep.cpp
	local_db_functs/MdCreateType.cpp           	
	local_db_functs/MdCreateVar.cpp       
	local_db_functs/MdDeleteRunById.cpp  
	local_db_functs/MdDeleteTimestepById.cpp
	local_db_functs/MdDeleteTypeById.cpp   
	local_db_functs/MdDeleteTypeByNameVer.cpp           
	local_db_functs/MdDeleteVarById.cpp  
	local_db_functs/MdDeleteVarByNamePathVer.cpp  
	local_db_functs/MdInsertRunAttribute.cpp
	local_db_functs/MdInsertTimestepAttribute.cpp
	local_db_functs/MdInsertVarAttributeByDims.cpp	
	
)

TARGET_INCLUDE_DIRECTORIES(EmpressLocal PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/../include/database
	${CMAKE_CURRENT_SOURCE_DIR}/../include/local	
)

