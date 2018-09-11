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


#ifndef CLIENTTIMINGCONSTANTSWRITEHDF5_HH
#define CLIENTTIMINGCONSTANTSWRITEHDF5_HH

//unsigned short -> max value: 65535
enum client_timing_constants_write_hdf5 : unsigned short {
    ERR_ARGC = 10000,
    ERR_TYPE_COMPARE = 10001,

    PROGRAM_START = 0,
    MPI_INIT_DONE = 1,
    INIT_VARS_DONE = 2,
    WRITING_START = 3,
    CREATE_NEW_RUN_START = 4,
    CREATE_NEW_RUN_DONE = 5,

    CREATE_TIMESTEPS_START = 6,
    CREATE_NEW_TIMESTEP_START = 7,
    CREATE_TIMESTEP_MD_TABLES_START = 8,
    CREATE_TIMESTEP_MD_TABLES_DONE = 9,
    OPEN_TIMESTEP_START = 10,
    CREATE_VARS_AND_ATTRS_FOR_NEW_TIMESTEP_START = 11,
    CREATE_VAR_AND_ATTRS_FOR_NEW_VAR_START = 12,
    CREATE_AND_WRITE_CHUNK_DATA_START = 13,
    CREATE_AND_WRITE_CHUNK_DATA_DONE = 14,

    CHUNK_MAX_MIN_FIND_START = 15,
    CHUNK_MAX_MIN_FIND_DONE = 16,
    CREATE_VARS_AND_ATTRS_FOR_NEW_TIMESTEP_DONE = 17,
    GATHER_ATTRS_START = 18,
    GATHER_ATTRS_DONE = 19,
    INSERT_VAR_ATTRS_START = 20,
    INSERT_VAR_ATTRS_DONE = 21,

    CREATE_TIMESTEP_DONE = 22,
    CREATE_ALL_TIMESTEPS_DONE = 23,
    CREATE_DATA_START = 24,
    CREATE_DATA_DONE = 25,
    CREATE_VAR_ATTRS_FOR_NEW_VAR_START = 26,
    CREATE_NEW_VAR_ATTR_START = 27,
    VAR_ATTR_INIT_DONE = 28,

    CREATE_NEW_VAR_ATTR_DONE = 29,
    CREATE_VAR_ATTRS_FOR_NEW_VAR_DONE = 30,
    CREATE_TIMESTEP_ATTRS_START = 31,
    GATHER_DONE_PROC_MAX_MIN_FIND_START = 32,
    PROC_MAX_MIN_FIND_DONE = 33,
    CREATE_TIMESTEP_ATTRS_DONE = 34,

    CREATE_RUN_ATTRS_START = 35,
    RUN_MAX_MIN_FIND_START = 36,
    RUN_MAX_MIN_FIND_DONE = 37,
    CREATE_RUN_ATTRS_DONE = 38,
    WRITING_DONE = 39,
    WRITING_DONE_FOR_ALL_PROCS_START_CLEANUP = 40,




};

#endif 

