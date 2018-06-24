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
	ops/common/OpActivateTypeMetaCommon.cpp         
	ops/common/OpActivateVarMetaCommon.cpp          
	ops/common/OpCatalogVarMetaCommon.cpp             
	ops/common/OpCatalogTypeMetaCommon.cpp             
	ops/common/OpCreateTypeMetaCommon.cpp           	
	ops/common/OpCreateVarMetaCommon.cpp           
	ops/common/OpDeleteTypeMetaCommon.cpp           
	ops/common/OpDeleteVarMetaCommon.cpp  
	ops/common/OpFullShutdownMetaCommon.cpp         
	ops/common/OpGetAttributeMetaCommon.cpp
	ops/common/OpGetAttributeListMetaCommon.cpp
	ops/common/OpGetChunkListMetaCommon.cpp
	ops/common/OpGetChunkMetaCommon.cpp
	ops/common/OpInsertAttributeMetaCommon.cpp	
	ops/common/OpInsertChunkMetaCommon.cpp	
	ops/common/OpProcessingTypeMetaCommon.cpp
	ops/common/OpProcessingVarMetaCommon.cpp

)

TARGET_INCLUDE_DIRECTORIES(OpsCommon PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/../include/ops
	${CMAKE_CURRENT_SOURCE_DIR}/../include/common
)


add_library(OpsServer
	ops/server/OpActivateTypeMetaServer.cpp
	ops/server/OpActivateVarMetaServer.cpp
	ops/server/OpCatalogVarMetaServer.cpp
	ops/server/OpCatalogTypeMetaServer.cpp
	ops/server/OpCreateTypeMetaServer.cpp           	
	ops/server/OpCreateVarMetaServer.cpp
	ops/server/OpDeleteTypeMetaServer.cpp
	ops/server/OpDeleteVarMetaServer.cpp
	ops/server/OpFullShutdownMetaServer.cpp
	ops/server/OpGetAttributeMetaServer.cpp
	ops/server/OpGetAttributeListMetaServer.cpp
	ops/server/OpGetChunkListMetaServer.cpp
	ops/server/OpGetChunkMetaServer.cpp
	ops/server/OpInsertAttributeMetaServer.cpp
	ops/server/OpInsertChunkMetaServer.cpp
	ops/server/OpProcessingTypeMetaServer.cpp
	ops/server/OpProcessingVarMetaServer.cpp
)

TARGET_INCLUDE_DIRECTORIES(OpsServer PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/../include/ops	
	${CMAKE_CURRENT_SOURCE_DIR}/../include/common
	${CMAKE_CURRENT_SOURCE_DIR}/../include/database
    ${CMAKE_CURRENT_SOURCE_DIR}/../include/server
)


add_library(OpsClient
    ops/client/OpActivateTypeMetaClient.cpp         
    ops/client/OpActivateVarMetaClient.cpp          
	ops/client/OpCatalogVarMetaClient.cpp  
	ops/client/OpCatalogTypeMetaClient.cpp             
	ops/client/OpCreateVarMetaClient.cpp           
	ops/client/OpCreateTypeMetaClient.cpp           	
	ops/client/OpDeleteTypeMetaClient.cpp           
	ops/client/OpDeleteVarMetaClient.cpp           
	ops/client/OpFullShutdownMetaClient.cpp  
	ops/client/OpGetAttributeMetaClient.cpp  
	ops/client/OpGetAttributeListMetaClient.cpp  
	ops/client/OpGetChunkListMetaClient.cpp       
	ops/client/OpGetChunkMetaClient.cpp  
	ops/client/OpInsertAttributeMetaClient.cpp        
	ops/client/OpInsertChunkMetaClient.cpp        
	ops/client/OpProcessingTypeMetaClient.cpp 
	ops/client/OpProcessingVarMetaClient.cpp 

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

