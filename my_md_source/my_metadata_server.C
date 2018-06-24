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

/* 
 * Copyright 2014 Sandia Corporation. Under the terms of Contract
 * DE-AC04-94AL85000, there is a non-exclusive license for use of this work by
 * or on behalf of the U.S. Government. Export of this program may require a
 * license from the United States Government.
 *
 * The MIT License (MIT)
 * 
 * Copyright (c) 2014 Sandia Corporation
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

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include <mpi.h>
#include <unistd.h>
#include <iostream>
#include <chrono>
#include <fstream>
#include <exception>
#include <sys/stat.h>
#include <sys/time.h>

#include <my_metadata_args.h>
#include <server_timing_constants.hh>
#include <OpActivateTypeMetaCommon.hh>
#include <OpActivateVarMetaCommon.hh>
#include <OpCatalogVarMetaCommon.hh>     
#include <OpCatalogTypeMetaCommon.hh>                   
#include <OpCreateTypeMetaCommon.hh>          
#include <OpCreateVarMetaCommon.hh>          
#include <OpDeleteTypeMetaCommon.hh>           
#include <OpDeleteVarMetaCommon.hh>               
#include <OpFullShutdownMetaCommon.hh>     
#include <OpGetAttributeListMetaCommon.hh>     
#include <OpGetAttributeMetaCommon.hh>      
#include <OpGetChunkListMetaCommon.hh>
#include <OpGetChunkMetaCommon.hh>
#include <OpInsertAttributeMetaCommon.hh>
#include <OpInsertChunkMetaCommon.hh>
#include <OpProcessingTypeMetaCommon.hh>
#include <OpProcessingVarMetaCommon.hh>


#include <opbox/services/dirman/DirectoryManager.hh>
#include <gutties/Gutties.hh>
#include "opbox/OpBox.hh"
#include <Globals.hh>
#include <sqlite3.h>

using namespace std;

std::string default_config_string = R"EOF(
# Select a transport to use for nnti (laptop tries ib if not forced to mpi)

nnti.logger.severity       error
nnti.transport.name        ibverbs
webhook.interfaces         ib0,lo

#config.additional_files.env_name.if_defined   CONFIG
lunasa.lazy_memory_manager   malloc
lunasa.eager_memory_manager  tcmalloc

# Select the type of dirman to use. Currently we only have centralized, which
# just sticks all the directory info on one node (called root). We use roles
# to designate which node is actually the root.
dirman.type           centralized

# Turn these on if you want to see more debug messages
#bootstrap.debug           true
#webhook.debug             true 
#opbox.debug               true
#dirman.debug              true
#dirman.cache.others.debug true
#dirman.cache.mine.debug   true

)EOF";

sqlite3 * db = NULL;

bool debug_logging = false;
bool error_logging = false;

bool md_shutdown = false;

vector<int> catg_of_time_pts;
vector<chrono::high_resolution_clock::time_point> time_pts;

struct debug_log {
  template<typename T> debug_log& operator << (const T& x) {
   if(debug_logging) {
      std::cout << x;
    }
    return *this;
  }
  debug_log& operator<<(std::ostream& (*manipulator)(std::ostream&)) {
    if(debug_logging) {
      std::cout << manipulator;
    }
    return *this;
  }
} debug_log;

// struct extreme_debug_log {
//   template<typename T> extreme_debug_log& operator << (const T& x) {
//    if(md_extreme_debug_logging) {
//       std::cout << x;
//     }
//     return *this;
//   }
//   extreme_debug_log& operator<<(std::ostream& (*manipulator)(std::ostream&)) {
//     if(md_extreme_debug_logging) {
//       std::cout << manipulator;
//     }
//     return *this;
//   }
// } extreme_debug_log;


struct error_log {
  template<typename T> error_log& operator << (const T& x) {
    if(error_logging) {
      std::cout << x;
    }
    return *this;
  }
  error_log& operator<<(std::ostream& (*manipulator)(std::ostream&)) {
    if(error_logging) {
      std::cout << manipulator;
    }
     return *this;
  }
} error_log;

static int callback (void * NotUsed, int argc, char ** argv, char ** ColName)
{
    int i;
    for (i = 0; i < argc; i++)
    {
        printf ("%s = %s\n", ColName [i], argv [i] ? argv [i] : "NULL");
    }
    printf ("\n");
    return 0;
}


static int metadata_database_init ();
static int setup_dirman(const string &dirman_file_path, const string &app_name, int rank);


//===========================================================================
static int metadata_database_init ()
{
    //setup the database
    int rc;
    char  *ErrMsg = NULL;
    
    rc = sqlite3_open (":memory:", &db);
    
    if (rc != SQLITE_OK)
    {   
        fprintf (stderr, "Can't open database: %s\n", sqlite3_errmsg (db));        sqlite3_close (db);
        goto cleanup;
    }
    rc = sqlite3_exec (db, "create table var_catalog (id integer primary key autoincrement not null, name varchar (50), path varchar (50), version int, active int, txn_id int, num_dims int, d0_min int, d0_max int, d1_min int, d1_max int, d2_min int, d2_max int)", callback, 0, &ErrMsg);
    if (rc != SQLITE_OK)    {
        fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
        sqlite3_free (ErrMsg);
        sqlite3_close (db);
        goto cleanup;
    }

    rc = sqlite3_exec (db, "create table chunk_data (chunk_id integer primary key autoincrement not null, var_id int not null, connection varchar(128), length int, d0_min int not null, d0_max int not null, d1_min int not null, d1_max int not null, d2_min int not null, d2_max int not null)", callback, 0, &ErrMsg);
    if (rc != SQLITE_OK)    {
        fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);        
        sqlite3_free (ErrMsg);
        sqlite3_close (db);
        goto cleanup;
    }

    rc = sqlite3_exec (db, "create table type_catalog (type_id integer primary key autoincrement not null, name varchar (50), version int, active int, txn_id int)", callback, 0, &ErrMsg);
    if (rc != SQLITE_OK)    {
        fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
        sqlite3_free (ErrMsg);
        sqlite3_close (db);
        goto cleanup;
    }

    rc = sqlite3_exec (db, "create table attribute_data (attribute_id integer primary key autoincrement not null, chunk_id int not null, type_id int not null, data varchar(256))", callback, 0, &ErrMsg);
    if (rc != SQLITE_OK)    {
        fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);        
        sqlite3_free (ErrMsg);
        sqlite3_close (db);
        goto cleanup;
    }

    //should be used by catalog
    rc = sqlite3_exec (db, "create index gc_active on var_catalog (active)", callback, 0, &ErrMsg);
    if (rc != SQLITE_OK)    {
        fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);        
        sqlite3_free (ErrMsg);
        sqlite3_close (db);
        goto cleanup;
    }

    //should be used by catalog
    rc = sqlite3_exec (db, "create index tc_active on type_catalog (active)", callback, 0, &ErrMsg);
    if (rc != SQLITE_OK)    {
        fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);        
        sqlite3_free (ErrMsg);
        sqlite3_close (db);
        goto cleanup;
    }

    //might need once I get delete working properly
    // //should be used by delete
    // rc = sqlite3_exec (db, "create index gc_id_name_path_ver on var_catalog (id, name, path, version)", callback, 0, &ErrMsg);
    // if (rc != SQLITE_OK)    {
    //     fprintf (stderr, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
    //     sqlite3_free (ErrMsg);
    //     sqlite3_close (db);
    //     goto cleanup;
    // }

    //should be used by get list, get chunk
    //note - as is, should never use the active part
    rc = sqlite3_exec (db, "create index gc_txnid_id_name_path_ver_active on var_catalog (txn_id, id, name, path, version, active)", callback, 0, &ErrMsg);
    if (rc != SQLITE_OK)    {
        fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);        
        sqlite3_free (ErrMsg);
        sqlite3_close (db);
        goto cleanup;
    }

    //should be used by get list
    rc = sqlite3_exec (db, "create index tc_txnid_typeid_name_ver_active on type_catalog (txn_id, type_id, name, version, active)", callback, 0, &ErrMsg);
    if (rc != SQLITE_OK)    {
        fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);        
        sqlite3_free (ErrMsg);
        sqlite3_close (db);
        goto cleanup;
    }

    //should be used by get chunk
    rc = sqlite3_exec (db, "create index vd_varid_dims on chunk_data (var_id, d0_max, d0_min, d1_max, d1_min, d2_max, d2_min)", callback, 0, &ErrMsg);
    if (rc != SQLITE_OK)    {
        fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);        
        sqlite3_free (ErrMsg);
        sqlite3_close (db);
        goto cleanup;
    }
    
    //should be used by get attribute
    rc = sqlite3_exec (db, "create index ad_typeid_chunkid on attribute_data (type_id, chunk_id)", callback, 0, &ErrMsg);
    if (rc != SQLITE_OK)    {
        fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);        
        sqlite3_free (ErrMsg);
        sqlite3_close (db);
        goto cleanup;
    }

//fix - should any of these ids be unique?

cleanup:
    return rc;
}



//argv[0] = name of program, argv[1]=directory manager path
int main (int argc, char **argv)
{
    char name[100];
    gethostname(name, sizeof(name));
    debug_log << name << endl;
    
    if (argc != 3)
    {
        cout << 0 << " " << ERR_ARGC << endl;
        error_log << "Usage: " << argv[0] << " <md_dirman file path>, estm_num_time_pts \n";
        return RC_ERR;
    }

    int estm_num_time_pts = atoi(argv[2]); 
    catg_of_time_pts.reserve(estm_num_time_pts);
    time_pts.reserve(estm_num_time_pts);

    int num_wall_time_pts = 6;
    struct timeval start, mpi_init_done, register_done, db_init_done, dirman_init_done, finalize;
    vector<long double> clock_times;
    clock_times.reserve(num_wall_time_pts);

    gettimeofday(&start, NULL);
    int zero_time_sec = 3600 * (start.tv_sec / 3600);
    clock_times.push_back( (start.tv_sec - zero_time_sec) + start.tv_usec / 1000000.0);
    chrono::high_resolution_clock::time_point start_time = chrono::high_resolution_clock::now();
    time_pts.push_back(start_time);
    catg_of_time_pts.push_back(PROGRAM_START);

    int rc;
    int rank;
    int num_procs;
    string dir_path="/metadata/testing";
    uint64_t db_sizes[8];

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

    gettimeofday(&mpi_init_done, NULL);
    clock_times.push_back( (mpi_init_done.tv_sec - zero_time_sec) + mpi_init_done.tv_usec / 1000000.0);
    time_pts.push_back(chrono::high_resolution_clock::now());
    catg_of_time_pts.push_back(MPI_INIT_DONE);

    opbox::RegisterOp<OpActivateTypeMeta>();
    opbox::RegisterOp<OpActivateVarMeta>();   
    opbox::RegisterOp<OpCatalogVarMeta>();
    opbox::RegisterOp<OpCatalogTypeMeta>(); 
    opbox::RegisterOp<OpCreateTypeMeta>(); 
    opbox::RegisterOp<OpCreateVarMeta>();
    opbox::RegisterOp<OpDeleteTypeMeta>();
    opbox::RegisterOp<OpDeleteVarMeta>();
    opbox::RegisterOp<OpFullShutdownMeta>();
    opbox::RegisterOp<OpGetAttributeListMeta>();
    opbox::RegisterOp<OpGetAttributeMeta>();   
    opbox::RegisterOp<OpGetChunkListMeta>();
    opbox::RegisterOp<OpGetChunkMeta>();
    opbox::RegisterOp<OpInsertAttributeMeta>();
    opbox::RegisterOp<OpInsertChunkMeta>();
    opbox::RegisterOp<OpProcessingTypeMeta>();
    opbox::RegisterOp<OpProcessingVarMeta>();

    gettimeofday(&register_done, NULL);
    clock_times.push_back( (register_done.tv_sec - zero_time_sec) + register_done.tv_usec / 1000000.0);
    time_pts.push_back(chrono::high_resolution_clock::now());
    catg_of_time_pts.push_back(REGISTER_OPS_DONE);

    //set up and return the database
    rc = metadata_database_init ();
    if (rc != RC_OK) {
        error_log << "Error. Could not init dbs. Error code " << rc << std::endl;
        time_pts.push_back(chrono::high_resolution_clock::now());
        catg_of_time_pts.push_back(ERR_DB_SETUP);
        goto cleanup;
    }
    gettimeofday(&db_init_done, NULL);
    clock_times.push_back( (db_init_done.tv_sec - zero_time_sec) + db_init_done.tv_usec / 1000000.0);
    time_pts.push_back(chrono::high_resolution_clock::now());
    catg_of_time_pts.push_back(DB_SETUP_DONE);

    db_sizes[0] = sqlite3_memory_used();
    debug_log << "mem after setup and index creation: " << db_sizes[0] << endl;

    rc = setup_dirman(argv[1], dir_path, rank);
    if (rc != RC_OK) {
        time_pts.push_back(chrono::high_resolution_clock::now());
        catg_of_time_pts.push_back(ERR_DIRMAN);
        error_log << "Error. Could not init dirman. Error code " << rc << std::endl;
        goto cleanup;
    }
    gettimeofday(&dirman_init_done, NULL);
    clock_times.push_back( (dirman_init_done.tv_sec - zero_time_sec) + dirman_init_done.tv_usec / 1000000.0);
    time_pts.push_back(chrono::high_resolution_clock::now());
    catg_of_time_pts.push_back(DIRMAN_SETUP_DONE_INIT_DONE);


    while(!md_shutdown) {
         std::this_thread::sleep_for(std::chrono::milliseconds(10));
     }

cleanup:
    debug_log << "Shutting down" << endl;
    gettimeofday(&finalize, NULL);
    clock_times.push_back( (finalize.tv_sec - zero_time_sec) + finalize.tv_usec / 1000000.0);
    time_pts.push_back(chrono::high_resolution_clock::now());
    catg_of_time_pts.push_back(SHUTDOWN_START);

    db_sizes[1] = sqlite3_memory_used();

    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;

    rc = sqlite3_prepare_v2 (db, "select count (*) from var_catalog", -1, &stmt, &tail); assert (rc == SQLITE_OK);
    rc = sqlite3_step (stmt); assert (rc == SQLITE_OK || rc == SQLITE_ROW);
    db_sizes[2] = sqlite3_column_int (stmt, 0);
    rc = sqlite3_finalize (stmt); assert (rc == SQLITE_OK);

    rc = sqlite3_prepare_v2 (db, "select count (*) from chunk_data", -1, &stmt, &tail); assert (rc == SQLITE_OK);
    rc = sqlite3_step (stmt); assert (rc == SQLITE_OK || rc == SQLITE_ROW);
    db_sizes[3] = sqlite3_column_int (stmt, 0);
    rc = sqlite3_finalize (stmt); assert (rc == SQLITE_OK);

    rc = sqlite3_prepare_v2 (db, "select count (*) from type_catalog", -1, &stmt, &tail); assert (rc == SQLITE_OK);
    rc = sqlite3_step (stmt); assert (rc == SQLITE_OK || rc == SQLITE_ROW);
    db_sizes[4] = sqlite3_column_int (stmt, 0);
    rc = sqlite3_finalize (stmt); assert (rc == SQLITE_OK);

    rc = sqlite3_prepare_v2 (db, "select count (*) from attribute_data", -1, &stmt, &tail); assert (rc == SQLITE_OK);
    rc = sqlite3_step (stmt); assert (rc == SQLITE_OK || rc == SQLITE_ROW);
    db_sizes[5] = sqlite3_column_int (stmt, 0);
    rc = sqlite3_finalize (stmt); assert (rc == SQLITE_OK);

    debug_log << "sizeof db: " << db_sizes[1] << endl;
    debug_log << "num vars: " << db_sizes[2] << " num types: " << db_sizes[4] << " num chunks: " << db_sizes[3] << " num_attrs: " << db_sizes[5] << endl;
    uint64_t estm_size_needed = 42*db_sizes[2] + 34*db_sizes[3] + 17*db_sizes[4] + (int) (16.615*db_sizes[5]);
    db_sizes[6] = estm_size_needed;
    debug_log << "estm_size_needed: " << estm_size_needed << " dif: " << db_sizes[1] - estm_size_needed << endl;
    uint64_t estm_size_needed_with_serialization = 42*db_sizes[2] + 34*db_sizes[3] + 17*db_sizes[4] + (int) (67.076*db_sizes[5]);
    debug_log << "estm_size_needed_with_serialization: " << estm_size_needed_with_serialization << " dif: " << db_sizes[1] - estm_size_needed_with_serialization << endl;
    db_sizes[7] = estm_size_needed_with_serialization;

    time_pts.push_back(chrono::high_resolution_clock::now());
    catg_of_time_pts.push_back(DB_ANALYSIS_DONE);

    sqlite3_close (db);

    time_pts.push_back(chrono::high_resolution_clock::now());
    catg_of_time_pts.push_back(DB_CLOSED_SHUTDOWN_DONE);

    int num_time_pts = time_pts.size();
    int *catg_of_time_pts_ary = &catg_of_time_pts[0];
    double *total_times = (double *) malloc(sizeof(double) * num_time_pts);
    
    //needed by rank 0
    int displacement_for_each_proc[num_procs];
    int *each_proc_num_time_pts;
    int *all_catg_time_pts_buf;
    double *all_time_pts_buf;
    uint64_t *all_db_sizes;
    long double *all_clock_times;

    if(rank == 0) {
        each_proc_num_time_pts = (int *) malloc(num_procs * sizeof(int));
        all_db_sizes = (uint64_t *) malloc(num_procs * 8 * sizeof(uint64_t));
        all_clock_times = (long double *) malloc(num_procs * num_wall_time_pts * sizeof(long double));
    }

    debug_log << "time_pts.size(): " << time_pts.size() << endl;
    MPI_Gather(&num_time_pts, 1, MPI_INT, each_proc_num_time_pts, 1, MPI_INT, 0, MPI_COMM_WORLD); 

    MPI_Gather(db_sizes, 8, MPI_UNSIGNED_LONG, all_db_sizes, 8, MPI_UNSIGNED_LONG, 0, MPI_COMM_WORLD); 

    MPI_Gather(&clock_times[0], num_wall_time_pts, MPI_LONG_DOUBLE, all_clock_times, num_wall_time_pts, MPI_LONG_DOUBLE, 0, MPI_COMM_WORLD); 

    int sum = 0;
    if(rank == 0) {
        for(int i=0; i<num_procs; i++) {
            displacement_for_each_proc[i] = sum;
            sum += each_proc_num_time_pts[i];
        }
        all_time_pts_buf = (double *) malloc(sum * sizeof(double));
        all_catg_time_pts_buf = (int *) malloc(sum * sizeof(int));
        debug_log << "sum: " + to_string(sum) << endl;
        debug_log << "estm num_time_pts: " << to_string(estm_num_time_pts) << endl;
    }
    for(int i=0; i< time_pts.size(); i++) {
        std::chrono::duration<double, std::nano> fp_ns = time_pts.at(i) - start_time;
        total_times[i] = fp_ns.count();
    }

    MPI_Gatherv(total_times, num_time_pts, MPI_DOUBLE, all_time_pts_buf, each_proc_num_time_pts, displacement_for_each_proc, MPI_DOUBLE, 0, MPI_COMM_WORLD); 

    MPI_Gatherv(catg_of_time_pts_ary, num_time_pts, MPI_INT, all_catg_time_pts_buf, each_proc_num_time_pts, displacement_for_each_proc, MPI_INT, 0, MPI_COMM_WORLD); 
    
    if (rank == 0) {
        cout << DB_SIZES << " ";
        for(int i=0; i<(8*num_procs); i++) {
            std::cout << all_db_sizes[i] << " ";
            if (i%20 == 0 && i!=0) {
                std::cout << std::endl;
            }
        }
        for(int i = 0; i<(num_wall_time_pts * num_procs); i++) {
            if(i%num_wall_time_pts == 0) {
                std::cout << CLOCK_TIMES_NEW_PROC << " ";
            }
            if(all_clock_times[i] != 0) {
                printf("%.6Lf ", all_clock_times[i]);
            }
            if (i%20 == 0 && i!=0) {
                std::cout << std::endl;
            } 
        }
        std::cout << CLOCK_TIMES_DONE << " ";
        for(int i=0; i<sum; i++) {
            printf("%4d %10.0f ", all_catg_time_pts_buf[i], all_time_pts_buf[i]);
            // std::cout << all_time_pts_buf[i] << " " << all_catg_time_pts_buf[i] << " ";
            if (i%20 == 0 && i!=0) {
                std::cout << std::endl;
            }
        }


        free(all_time_pts_buf);
        free(all_catg_time_pts_buf);
        free(each_proc_num_time_pts);
        free(all_db_sizes);
    }

    free(total_times);

    MPI_Barrier(MPI_COMM_WORLD);
    debug_log << "about to MPI_Finalize \n";
    MPI_Finalize();
    debug_log << "just mpi finalized \n";
    gutties::bootstrap::Finish();
    debug_log << "returning \n";
    return rc;
}


static int setup_dirman(const string &dirman_file_path, const string &dir_path, int rank)
{
   struct stat buffer;
    bool dirman_initted = false;
    std::ifstream file;
    char dirman_hexid_c_str[14];
    string dirman_hexid;

    bool ok;
    int rc;

    if(rank == 0) { 
        while (!dirman_initted) {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            dirman_initted = (stat (dirman_file_path.c_str(), &buffer) == 0);
            debug_log << "dirman not initted yet \n";
        }
        file.open(dirman_file_path);
        if(!file) {
            return RC_ERR;
        }
        while( file.peek() == std::ifstream::traits_type::eof() ) {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            debug_log << "dirman file empty \n";
        }
        file >> dirman_hexid;
        // dirman_hexid_c_str = (char *)dirman_hexid.c_str();
        dirman_hexid.copy(dirman_hexid_c_str,13);
        dirman_hexid_c_str[13]='\0';
        debug_log << "just got the hexid: " << dirman_hexid << endl;
    }
    MPI_Bcast(dirman_hexid_c_str, 14, MPI_CHAR, 0, MPI_COMM_WORLD);

    debug_log << "just got the hexid from bcast: " << dirman_hexid_c_str << endl;

    //Add the directory manager's URL to the config string so the clients know
    //Where to access the directory 
    gutties::Configuration config(default_config_string);
    config.Append("dirman.root_node", dirman_hexid_c_str);
    config.Append("webhook.port", to_string(1991+rank)); //note if you up number of servers you'll want to up this
    config.AppendFromReferences();

    debug_log << "just configged" << endl; 
    gutties::bootstrap::Start(config, opbox::bootstrap);
    //-------------------------------------------
    //TODO: This connect is temporarily necessary
    gutties::nodeid_t dirman_nodeid((string)dirman_hexid_c_str);

    debug_log << "Dirman node ID " << dirman_nodeid.GetHex() << " " << dirman_nodeid.GetIP() << " port " << dirman_nodeid.GetPort() << endl;
    net::peer_ptr_t peer;
    rc = net::Connect(&peer, dirman_nodeid);
    assert(rc==0 && "could not connect");
    //-------------------------------------------
    
    DirectoryInfo dir;
    gutties::nodeid_t myid = opbox::GetMyID();
    debug_log << "AppServer ID   " << myid.GetHex() << endl;
    //Have all nodes join the directory
    ok = dirman::JoinDirWithoutName(gutties::ResourceURL(dir_path), &dir);
    assert(ok && "Could not join the directory?");

    debug_log << "App Server Info: '" << dir.info << "ReferenceNode: " << dir.GetReferenceNode().GetHex() << " NumberChildren: " << to_string(dir.children.size()) << endl;

    return rc;
}