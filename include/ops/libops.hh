/* 
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
 */


#ifndef LIBOPS_HH
#define LIBOPS_HH

#include <OpCatalogAllVarAttributesMetaCommon.hh>
#include <OpCatalogAllVarAttributesWithDimsMetaCommon.hh>
#include <OpCatalogAllVarAttributesWithTypeByIdMetaCommon.hh>
#include <OpCatalogAllVarAttributesWithTypeByNameVerMetaCommon.hh>
#include <OpCatalogAllVarAttributesWithTypeDimsByIdMetaCommon.hh>
#include <OpCatalogAllVarAttributesWithTypeDimsByNameVerMetaCommon.hh>
#include <OpCatalogAllVarAttributesWithTypeVarByIdMetaCommon.hh>
#include <OpCatalogAllVarAttributesWithTypeVarByNameVerMetaCommon.hh>
#include <OpCatalogAllVarAttributesWithTypeVarDimsByIdMetaCommon.hh>
#include <OpCatalogAllVarAttributesWithTypeVarDimsByNameVerMetaCommon.hh>
#include <OpCatalogAllVarAttributesWithVarByIdMetaCommon.hh>
#include <OpCatalogAllVarAttributesWithVarByNameVerMetaCommon.hh>
#include <OpCatalogAllVarAttributesWithVarDimsByIdMetaCommon.hh>
#include <OpCatalogAllVarAttributesWithVarDimsByNameVerMetaCommon.hh>
#include <OpCatalogRunMetaCommon.hh>                       
#include <OpCatalogTypeMetaCommon.hh>                       
#include <OpCatalogVarMetaCommon.hh> 
#include <OpCreateRunMetaCommon.hh>          
#include <OpCreateTypeMetaCommon.hh>          
#include <OpCreateVarMetaCommon.hh>                         
#include <OpDeleteRunByIdMetaCommon.hh>
#include <OpDeleteTypeByIdMetaCommon.hh>
#include <OpDeleteTypeByNameVerMetaCommon.hh>
#include <OpDeleteVarByIdMetaCommon.hh>
#include <OpDeleteVarByNamePathVerMetaCommon.hh>
#include <OpFullShutdownMetaCommon.hh>         
#include <OpInsertVarAttributeByDimsMetaCommon.hh>

#include <OpCreateTimestepMetaCommon.hh>
#include <OpInsertRunAttributeMetaCommon.hh>
#include <OpInsertTimestepAttributeMetaCommon.hh>
#include <OpCatalogTimestepMetaCommon.hh>
#include <OpDeleteTimestepByIdMetaCommon.hh>
#include <OpCatalogAllRunAttributesMetaCommon.hh>
#include <OpCatalogAllTimestepAttributesMetaCommon.hh>
#include <OpCatalogAllRunAttributesWithTypeMetaCommon.hh>
#include <OpCatalogAllTimestepsWithVarAttributesWithTypeVarMetaCommon.hh>
#include <OpCatalogAllTimestepsWithVarAttributesWithTypeVarDimsMetaCommon.hh>
#include <OpCatalogAllTimestepsWithVarMetaCommon.hh>
#include <OpCatalogAllTypesWithVarAttributesInTimestepMetaCommon.hh>
#include <OpCatalogAllTypesWithVarAttributesWithVarInTimestepMetaCommon.hh>
#include <OpCatalogAllTypesWithVarAttributesWithVarDimsInTimestepMetaCommon.hh>
#include <OpCatalogAllTimestepAttributesWithTypeMetaCommon.hh>
#include <OpCatalogAllRunAttributesWithTypeRangeMetaCommon.hh>
#include <OpCatalogAllTimestepsWithVarAttributesWithTypeVarRangeMetaCommon.hh>
#include <OpCatalogAllTimestepsWithVarAttributesWithTypeVarDimsRangeMetaCommon.hh>
#include <OpCatalogAllTimestepAttributesWithTypeRangeMetaCommon.hh>
#include <OpCatalogAllVarAttributesWithTypeVarRangeMetaCommon.hh>
#include <OpCatalogAllVarAttributesWithTypeVarDimsRangeMetaCommon.hh>
#include <OpActivateMetaCommon.hh>
#include <OpProcessingMetaCommon.hh>
#include <OpCatalogAllVarAttributesWithVarSubstrMetaCommon.hh>
#include <OpCatalogAllTimestepsWithVarAttributesWithTypeVarSubstrDimsMetaCommon.hh>
#include <OpCatalogAllTimestepsWithVarAttributesWithTypeVarSubstrDimsRangeMetaCommon.hh>
#include <OpCatalogAllTimestepsWithVarAttributesWithTypeVarSubstrMetaCommon.hh>
#include <OpCatalogAllTimestepsWithVarAttributesWithTypeVarSubstrRangeMetaCommon.hh>
#include <OpCatalogAllTimestepsWithVarSubstrMetaCommon.hh>
#include <OpCatalogAllTypesWithVarAttributesWithVarSubstrDimsInTimestepMetaCommon.hh>
#include <OpCatalogAllTypesWithVarAttributesWithVarSubstrInTimestepMetaCommon.hh>
#include <OpCatalogAllVarAttributesWithTypeVarSubstrDimsRangeMetaCommon.hh>
#include <OpCatalogAllVarAttributesWithTypeVarSubstrRangeMetaCommon.hh>
#include <OpCatalogAllVarAttributesWithTypeVarSubstrMetaCommon.hh>
#include <OpCatalogAllVarAttributesWithTypeVarSubstrDimsMetaCommon.hh>
#include <OpCatalogAllVarAttributesWithVarSubstrDimsMetaCommon.hh>
#include <OpDeleteAllVarsWithSubstrMetaCommon.hh>
#include <OpInsertVarAttributeByDimsBatchMetaCommon.hh>
#include <OpCreateTypeBatchMetaCommon.hh>
#include <OpCreateVarBatchMetaCommon.hh>
#include <OpInsertRunAttributeBatchMetaCommon.hh>
#include <OpInsertTimestepAttributeBatchMetaCommon.hh>
#endif // LIBOPS_HH
