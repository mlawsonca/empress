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


cmake_minimum_required(VERSION 3.2)
project( sirius
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

  
