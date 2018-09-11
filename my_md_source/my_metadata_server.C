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
#include <libops.hh>

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
bool error_logging = true;

bool md_shutdown = false;


static bool output_timing = true;

//link to testing_debug.cpp in cmake to do debug tests
// static bool do_debug_testing = false;

std::vector<std::chrono::high_resolution_clock::time_point> time_pts;
std::vector<int> catg_of_time_pts;

static void add_timing_point(chrono::high_resolution_clock::time_point time_pt, int catg) {
    if (output_timing) {
        time_pts.push_back(time_pt);
        catg_of_time_pts.push_back(catg);
    }
}


void add_timing_point(int catg) {
    if (output_timing) {
        time_pts.push_back(chrono::high_resolution_clock::now());
        catg_of_time_pts.push_back(catg);
    }
}


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

int callback (void * NotUsed, int argc, char ** argv, char ** ColName)
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

static int sql_output_db(sqlite3 *pInMemory, uint64_t job_id, int rank);
static int sql_load_db(sqlite3 *pInMemory, uint64_t job_id, int rank);

//===========================================================================
static int metadata_database_init (bool load_db, uint64_t job_id, int rank)
{
    //setup the database
    int rc;
    char  *ErrMsg = NULL;
    
    rc = sqlite3_open (":memory:", &db);
    
    if (rc != SQLITE_OK)
    {   
        fprintf (stderr, "Can't open database: %s\n", sqlite3_errmsg (db));        
        sqlite3_close (db);
        goto cleanup;
    }

    if(load_db) {
        rc = sql_load_db(db, job_id, rank);
        while (rc == SQLITE_LOCKED || rc == SQLITE_BUSY) {
            if (rc == SQLITE_LOCKED) {
                error_log << "rank: " << rank << " db is locked. am waiting" << endl;
            }
            else {
                 error_log << "rank: " << rank << "db is busy. am waiting" << endl;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        if (rc != SQLITE_OK)
        {   
            fprintf (stderr, "Can't load database for job_id:%llu: %s\n",job_id, sqlite3_errmsg (db));        
            sqlite3_close (db);
            goto cleanup;
        }

    }
    else { //create new db
        rc = sqlite3_exec (db, "create table run_catalog (id integer primary key autoincrement not null, job_id int, name varchar (50) collate nocase, date varchar (50), active int, txn_id int, npx int, npy int, npz int, rank_to_dims_funct varchar (1000), objector_funct varchar (6000))", callback, 0, &ErrMsg);
        if (rc != SQLITE_OK)    {
            fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
            sqlite3_free (ErrMsg);
            sqlite3_close (db);
            goto cleanup;
        }

        rc = sqlite3_exec (db, "create table timestep_catalog (id integer not null, run_id int not null, path varchar (50) collate nocase, active int, txn_id int, primary key(id, run_id) )", callback, 0, &ErrMsg);
        if (rc != SQLITE_OK)    {
            fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
            sqlite3_free (ErrMsg);
            sqlite3_close (db);
            goto cleanup;
        }

        //todo - should just dataset be active instead of var?
        // rc = sqlite3_exec (db, "create table var_catalog (id integer primary key autoincrement not null, dataset_id int, name varchar (50), path varchar (50), version int, data_size int, num_dims int, d0_min int, d0_max int, d1_min int, d1_max int, d2_min int, d2_max int)", callback, 0, &ErrMsg);
        rc = sqlite3_exec (db, "create table var_catalog (id integer not null, run_id int not null, timestep_id int not null, name varchar (50) collate nocase, path varchar (50) collate nocase, version int, data_size int, active int, txn_id int, num_dims int, d0_min int, d0_max int, d1_min int, d1_max int, d2_min int, d2_max int, primary key(id, run_id, timestep_id) )", callback, 0, &ErrMsg);
        
        if (rc != SQLITE_OK)    {
            fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
            sqlite3_free (ErrMsg);
            sqlite3_close (db);
            goto cleanup;
        }

        // rc = sqlite3_exec (db, "create table type_catalog (id integer primary key autoincrement not null, dataset_id int, name varchar (50), version int, active int, txn_id int)", callback, 0, &ErrMsg);
        rc = sqlite3_exec (db, "create table type_catalog (id integer primary key autoincrement not null, run_id int not null, name varchar (50) collate nocase, version int, active int, txn_id int )", callback, 0, &ErrMsg);
        if (rc != SQLITE_OK)    {
            fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
            sqlite3_free (ErrMsg);
            sqlite3_close (db);
            goto cleanup;
        }

        rc = sqlite3_exec (db, "create table run_attribute_catalog (id integer primary key autoincrement not null, type_id int not null, active int, txn_id int, data_type int, data none )", callback, 0, &ErrMsg);
        if (rc != SQLITE_OK)    {
            fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
            sqlite3_free (ErrMsg);
            sqlite3_close (db);
            goto cleanup;
        }

        rc = sqlite3_exec (db, "create table timestep_attribute_catalog (id integer primary key autoincrement not null, timestep_id not null, type_id int not null, active int, txn_id int, data_type int, data none )", callback, 0, &ErrMsg);
        if (rc != SQLITE_OK)    {
            fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
            sqlite3_free (ErrMsg);
            sqlite3_close (db);
            goto cleanup;
        }

        rc = sqlite3_exec (db, "create table var_attribute_catalog (id integer primary key autoincrement not null, timestep_id not null, type_id int not null, var_id int not null, active int, txn_id int, num_dims int not null, d0_min int, d0_max int, d1_min int, d1_max int, d2_min int, d2_max int, data_type int, data none )", callback, 0, &ErrMsg);
        if (rc != SQLITE_OK)    {
            fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
            sqlite3_free (ErrMsg);
            sqlite3_close (db);
            goto cleanup;
        }
       
        

    // //INDICES/////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // //TXN-ACTIVE/////////////////////////////////////////////////////////////////////////////////////////////////////////////

    //     rc = sqlite3_exec (db, "create index rc_txn_id_active on run_catalog (txn_id, active)", callback, 0, &ErrMsg);
    //     if (rc != SQLITE_OK)    {
    //         fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
    //         sqlite3_free (ErrMsg);
    //         sqlite3_close (db);
    //         goto cleanup;
    //     }

    //     rc = sqlite3_exec (db, "create index tmc_txn_id_active on timestep_attribute_catalog (txn_id, active)", callback, 0, &ErrMsg);
    //     if (rc != SQLITE_OK)    {
    //         fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
    //         sqlite3_free (ErrMsg);
    //         sqlite3_close (db);
    //         goto cleanup;
    //     }

    //     rc = sqlite3_exec (db, "create index vc_txn_id_active on var_catalog (txn_id, active)", callback, 0, &ErrMsg);
    //     if (rc != SQLITE_OK)    {
    //         fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
    //         sqlite3_free (ErrMsg);
    //         sqlite3_close (db);
    //         goto cleanup;
    //     }


    //     rc = sqlite3_exec (db, "create index tc_txn_id_active on type_catalog (txn_id, active)", callback, 0, &ErrMsg);
    //     if (rc != SQLITE_OK)    {
    //         fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
    //         sqlite3_free (ErrMsg);
    //         sqlite3_close (db);
    //         goto cleanup;
    //     }

    //     rc = sqlite3_exec (db, "create index rac_txn_id_active on run_attribute_catalog (txn_id, active)", callback, 0, &ErrMsg);
    //     if (rc != SQLITE_OK)    {
    //         fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
    //         sqlite3_free (ErrMsg);
    //         sqlite3_close (db);
    //         goto cleanup;
    //     }

    //     rc = sqlite3_exec (db, "create index tac_txn_id_active on timestep_attribute_catalog (txn_id, active)", callback, 0, &ErrMsg);
    //     if (rc != SQLITE_OK)    {
    //         fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
    //         sqlite3_free (ErrMsg);
    //         sqlite3_close (db);
    //         goto cleanup;
    //     }

    //     rc = sqlite3_exec (db, "create index vac_txn_id_active on var_attribute_catalog (txn_id, active)", callback, 0, &ErrMsg);
    //     if (rc != SQLITE_OK)    {
    //         fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
    //         sqlite3_free (ErrMsg);
    //         sqlite3_close (db);
    //         goto cleanup;
    //     }

    // //ACTIVE/////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //     rc = sqlite3_exec (db, "create index rc_active on run_catalog (active)", callback, 0, &ErrMsg);
    //     if (rc != SQLITE_OK)    {
    //         fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
    //         sqlite3_free (ErrMsg);
    //         sqlite3_close (db);
    //         goto cleanup;
    //     }

    //     rc = sqlite3_exec (db, "create index tmc_active on timestep_catalog (active)", callback, 0, &ErrMsg);
    //     if (rc != SQLITE_OK)    {
    //         fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
    //         sqlite3_free (ErrMsg);
    //         sqlite3_close (db);
    //         goto cleanup;
    //     }

    //     rc = sqlite3_exec (db, "create index vc_active on var_catalog (active)", callback, 0, &ErrMsg);
    //     if (rc != SQLITE_OK)    {
    //         fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
    //         sqlite3_free (ErrMsg);
    //         sqlite3_close (db);
    //         goto cleanup;
    //     }

    //     rc = sqlite3_exec (db, "create index tc_active on type_catalog (active)", callback, 0, &ErrMsg);
    //     if (rc != SQLITE_OK)    {
    //         fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
    //         sqlite3_free (ErrMsg);
    //         sqlite3_close (db);
    //         goto cleanup;
    //     }

    //     rc = sqlite3_exec (db, "create index rac_active on run_attribute_catalog (active)", callback, 0, &ErrMsg);
    //     if (rc != SQLITE_OK)    {
    //         fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
    //         sqlite3_free (ErrMsg);
    //         sqlite3_close (db);
    //         goto cleanup;
    //     }

    //     rc = sqlite3_exec (db, "create index tac_active on timestep_attribute_catalog (active)", callback, 0, &ErrMsg);
    //     if (rc != SQLITE_OK)    {
    //         fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
    //         sqlite3_free (ErrMsg);
    //         sqlite3_close (db);
    //         goto cleanup;
    //     }

    //     rc = sqlite3_exec (db, "create index vac_active on var_attribute_catalog (active)", callback, 0, &ErrMsg);
    //     if (rc != SQLITE_OK)    {
    //         fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
    //         sqlite3_free (ErrMsg);
    //         sqlite3_close (db);
    //         goto cleanup;
    //     }

    //DIMS/////////////////////////////////////////////////////////////////////////////////////////////////////////////

        // rc = sqlite3_exec (db, "create index vac_dims on var_attribute_catalog (d0_min, d0_max, d1_min, d1_max, d2_min, d2_max)", callback, 0, &ErrMsg);
        // if (rc != SQLITE_OK)    {
        //     fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
        //     sqlite3_free (ErrMsg);
        //     sqlite3_close (db);
        //     goto cleanup;
        // }

    //MISC/////////////////////////////////////////////////////////////////////////////////////////////////////////////


        rc = sqlite3_exec (db, "create index tc_all_cols on type_catalog(run_id, name, version)", callback, 0, &ErrMsg);
        if (rc != SQLITE_OK)    {
            fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
            sqlite3_free (ErrMsg);
            sqlite3_close (db);
            goto cleanup;
        }

        rc = sqlite3_exec (db, "create index rac_all_cols on run_attribute_catalog(type_id, data_type)", callback, 0, &ErrMsg);
        if (rc != SQLITE_OK)    {
            fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
            sqlite3_free (ErrMsg);
            sqlite3_close (db);
            goto cleanup;
        }

        rc = sqlite3_exec (db, "create index tac_all_cols on timestep_attribute_catalog(type_id, timestep_id, data_type)", callback, 0, &ErrMsg);
        if (rc != SQLITE_OK)    {
            fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
            sqlite3_free (ErrMsg);
            sqlite3_close (db);
            goto cleanup;
        }

        rc = sqlite3_exec (db, "create index vac_all_cols on var_attribute_catalog (timestep_id, type_id, var_id, d0_min)", callback, 0, &ErrMsg);
        if (rc != SQLITE_OK)    {
            fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
            sqlite3_free (ErrMsg);
            sqlite3_close (db);
            goto cleanup;
        }

        rc = sqlite3_exec (db, "create index vc_run_timestep on var_catalog (timestep_id, run_id)", callback, 0, &ErrMsg);
        if (rc != SQLITE_OK)    {
            fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
            sqlite3_free (ErrMsg);
            sqlite3_close (db);
            goto cleanup;
        }

        // rc = sqlite3_exec (db, "create index vc_timestep on var_catalog (timestep_id)", callback, 0, &ErrMsg);
        // if (rc != SQLITE_OK)    {
        //     fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
        //     sqlite3_free (ErrMsg);
        //     sqlite3_close (db);
        //     goto cleanup;
        // }

        rc = sqlite3_exec (db, "create index tmc_run on timestep_catalog (run_id)", callback, 0, &ErrMsg);
        if (rc != SQLITE_OK)    {
            fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
            sqlite3_free (ErrMsg);
            sqlite3_close (db);
            goto cleanup;
        }

        rc = sqlite3_exec (db, "create index vac_type_id_d0 on var_attribute_catalog(type_id, d0_min)", callback, 0, &ErrMsg);
        if (rc != SQLITE_OK)    {
            fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
            sqlite3_free (ErrMsg);
            sqlite3_close (db);
            goto cleanup;
        }

        // rc = sqlite3_exec (db, "create index tac_type_id on timestep_attribute_catalog(type_id)", callback, 0, &ErrMsg);
        // if (rc != SQLITE_OK)    {
        //     fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
        //     sqlite3_free (ErrMsg);
        //     sqlite3_close (db);
        //     goto cleanup;
        // }

        rc = sqlite3_exec (db, "create index vac_varid_d0 on var_attribute_catalog(var_id, d0_min)", callback, 0, &ErrMsg);
        if (rc != SQLITE_OK)    {
            fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
            sqlite3_free (ErrMsg);
            sqlite3_close (db);
            goto cleanup;
        }
    } //end of if(!load_db)


cleanup:
    return rc;
}

//reminder: you get a performance boost if you register the ops before launching opbox/gutties
static int register_ops () {
    opbox::RegisterOp<OpInsertTimestepAttributeBatchMeta>();
    opbox::RegisterOp<OpInsertRunAttributeBatchMeta>();
    opbox::RegisterOp<OpCreateVarBatchMeta>();
    opbox::RegisterOp<OpCreateTypeBatchMeta>();
    opbox::RegisterOp<OpInsertVarAttributeByDimsBatchMeta>();
    opbox::RegisterOp<OpDeleteAllVarsWithSubstrMeta>();
    opbox::RegisterOp<OpCatalogAllVarAttributesWithVarSubstrDimsMeta>();
    opbox::RegisterOp<OpCatalogAllVarAttributesWithTypeVarSubstrDimsMeta>();
    opbox::RegisterOp<OpCatalogAllVarAttributesWithTypeVarSubstrMeta>();
    opbox::RegisterOp<OpCatalogAllVarAttributesWithTypeVarSubstrRangeMeta>();
    opbox::RegisterOp<OpCatalogAllVarAttributesWithTypeVarSubstrDimsRangeMeta>();
    opbox::RegisterOp<OpCatalogAllTypesWithVarAttributesWithVarSubstrInTimestepMeta>();
    opbox::RegisterOp<OpCatalogAllTypesWithVarAttributesWithVarSubstrDimsInTimestepMeta>();
    opbox::RegisterOp<OpCatalogAllTimestepsWithVarSubstrMeta>();
    opbox::RegisterOp<OpCatalogAllTimestepsWithVarAttributesWithTypeVarSubstrRangeMeta>();
    opbox::RegisterOp<OpCatalogAllTimestepsWithVarAttributesWithTypeVarSubstrMeta>();
    opbox::RegisterOp<OpCatalogAllTimestepsWithVarAttributesWithTypeVarSubstrDimsRangeMeta>();
    opbox::RegisterOp<OpCatalogAllTimestepsWithVarAttributesWithTypeVarSubstrDimsMeta>();
    opbox::RegisterOp<OpCatalogAllVarAttributesWithVarSubstrMeta>();
    opbox::RegisterOp<OpProcessingMeta>();
    opbox::RegisterOp<OpActivateMeta>();
    opbox::RegisterOp<OpCatalogAllVarAttributesWithTypeVarDimsRangeMeta>();
    opbox::RegisterOp<OpCatalogAllVarAttributesWithTypeVarRangeMeta>();
    opbox::RegisterOp<OpCatalogAllTimestepAttributesWithTypeRangeMeta>();
    opbox::RegisterOp<OpCatalogAllTimestepsWithVarAttributesWithTypeVarDimsRangeMeta>();
    opbox::RegisterOp<OpCatalogAllTimestepsWithVarAttributesWithTypeVarRangeMeta>();
    opbox::RegisterOp<OpCatalogAllRunAttributesWithTypeRangeMeta>();
    opbox::RegisterOp<OpCatalogAllTimestepAttributesWithTypeMeta>();
    opbox::RegisterOp<OpCatalogAllTypesWithVarAttributesWithVarDimsInTimestepMeta>();
    opbox::RegisterOp<OpCatalogAllTypesWithVarAttributesWithVarInTimestepMeta>();
    opbox::RegisterOp<OpCatalogAllTypesWithVarAttributesInTimestepMeta>();
    opbox::RegisterOp<OpCatalogAllTimestepsWithVarMeta>();
    opbox::RegisterOp<OpCatalogAllTimestepsWithVarAttributesWithTypeVarDimsMeta>();
    opbox::RegisterOp<OpCatalogAllTimestepsWithVarAttributesWithTypeVarMeta>();
    opbox::RegisterOp<OpCatalogAllRunAttributesWithTypeMeta>();
    opbox::RegisterOp<OpCatalogAllTimestepAttributesMeta>();
    opbox::RegisterOp<OpCatalogAllRunAttributesMeta>();
    opbox::RegisterOp<OpDeleteTimestepByIdMeta>();
    opbox::RegisterOp<OpCatalogTimestepMeta>();
    opbox::RegisterOp<OpInsertTimestepAttributeMeta>();
    opbox::RegisterOp<OpInsertRunAttributeMeta>();
    opbox::RegisterOp<OpCreateTimestepMeta>();
    opbox::RegisterOp<OpDeleteVarByNamePathVerMeta>();
    opbox::RegisterOp<OpDeleteVarByIdMeta>();
    opbox::RegisterOp<OpDeleteTypeByNameVerMeta>();
    opbox::RegisterOp<OpDeleteTypeByIdMeta>();
    opbox::RegisterOp<OpDeleteRunByIdMeta>();
    opbox::RegisterOp<OpCatalogAllVarAttributesWithVarDimsByNameVerMeta>();
    opbox::RegisterOp<OpCatalogAllVarAttributesWithVarDimsByIdMeta>();
    opbox::RegisterOp<OpCatalogAllVarAttributesWithVarByNameVerMeta>();
    opbox::RegisterOp<OpCatalogAllVarAttributesWithVarByIdMeta>();
    opbox::RegisterOp<OpCatalogAllVarAttributesWithTypeVarDimsByNameVerMeta>();
    opbox::RegisterOp<OpCatalogAllVarAttributesWithTypeVarDimsByIdMeta>();
    opbox::RegisterOp<OpCatalogAllVarAttributesWithTypeVarByNameVerMeta>();
    opbox::RegisterOp<OpCatalogAllVarAttributesWithTypeVarByIdMeta>();
    opbox::RegisterOp<OpCatalogAllVarAttributesWithTypeDimsByNameVerMeta>();
    opbox::RegisterOp<OpCatalogAllVarAttributesWithTypeDimsByIdMeta>();
    opbox::RegisterOp<OpCatalogAllVarAttributesWithTypeByNameVerMeta>();
    opbox::RegisterOp<OpCatalogAllVarAttributesWithTypeByIdMeta>();
    opbox::RegisterOp<OpCatalogAllVarAttributesWithDimsMeta>();
    opbox::RegisterOp<OpCatalogAllVarAttributesMeta>();
    opbox::RegisterOp<OpCatalogRunMeta>();
    opbox::RegisterOp<OpCatalogTypeMeta>(); 
    opbox::RegisterOp<OpCatalogVarMeta>();
    opbox::RegisterOp<OpCreateRunMeta>();
    opbox::RegisterOp<OpCreateTypeMeta>();
    opbox::RegisterOp<OpCreateVarMeta>();
    opbox::RegisterOp<OpFullShutdownMeta>();  
    opbox::RegisterOp<OpInsertVarAttributeByDimsMeta>();

    return RC_OK;
}



//argv[0] = name of program, argv[1]=directory manager path
int main (int argc, char **argv)
{
    char name[100];
    gethostname(name, sizeof(name));
    debug_log << name << endl;
    
    if (argc != 6)
    {
        cout << 0 << " " << (int)ERR_ARGC << endl;
        error_log << "Usage: " << argv[0] << " <md_dirman file path>, estm_num_time_pts, sql_load_db(0 or 1), sql_output_db(0 or 1), job_id \n";
        return RC_ERR;
    }

    int estm_num_time_pts = atoi(argv[2]); 
    bool load_db = atoi(argv[3]);
    bool output_db = atoi(argv[4]);
    uint64_t job_id = atoi(argv[5]);

    catg_of_time_pts.reserve(estm_num_time_pts);
    time_pts.reserve(estm_num_time_pts);

    int num_wall_time_pts = 6;
    int num_db_pts = 9;
    struct timeval start, mpi_init_done, register_done, db_init_done, dirman_init_done, finalize;
    vector<long double> clock_times;
    clock_times.reserve(num_wall_time_pts);

    gettimeofday(&start, NULL);
    int zero_time_sec = 86400 * (start.tv_sec / 86400);
    clock_times.push_back( (start.tv_sec - zero_time_sec) + start.tv_usec / 1000000.0);
    chrono::high_resolution_clock::time_point start_time = chrono::high_resolution_clock::now();
    add_timing_point(start_time, PROGRAM_START);

    int rc;
    int rank;
    int num_procs;
    string dir_path="/metadata/testing";
    uint64_t db_sizes[num_db_pts];

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

    gettimeofday(&mpi_init_done, NULL);
    clock_times.push_back( (mpi_init_done.tv_sec - zero_time_sec) + mpi_init_done.tv_usec / 1000000.0);
    add_timing_point(MPI_INIT_DONE);


    rc = register_ops();

    gettimeofday(&register_done, NULL);
    clock_times.push_back( (register_done.tv_sec - zero_time_sec) + register_done.tv_usec / 1000000.0);
    add_timing_point(REGISTER_OPS_DONE);

    //set up and return the database
    rc = metadata_database_init (load_db, job_id, rank);
    if (rc != RC_OK) {
        error_log << "Error. Could not init dbs. Error code " << rc << std::endl;
        time_pts.push_back(chrono::high_resolution_clock::now());
        catg_of_time_pts.push_back(ERR_DB_SETUP);
        goto cleanup;
    }
    gettimeofday(&db_init_done, NULL);
    clock_times.push_back( (db_init_done.tv_sec - zero_time_sec) + db_init_done.tv_usec / 1000000.0);
    add_timing_point(DB_SETUP_DONE);

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
    add_timing_point(DIRMAN_SETUP_DONE_INIT_DONE);


    debug_log << "starting ops" << endl;
    while(!md_shutdown) {
         std::this_thread::sleep_for(std::chrono::milliseconds(10));
     }

cleanup:
    debug_log << "Shutting down" << endl;
    gettimeofday(&finalize, NULL);
    clock_times.push_back( (finalize.tv_sec - zero_time_sec) + finalize.tv_usec / 1000000.0);
    add_timing_point(SHUTDOWN_START);

    db_sizes[1] = sqlite3_memory_used();

    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;


    rc = sqlite3_prepare_v2 (db, "select count (*) from run_catalog", -1, &stmt, &tail); assert (rc == SQLITE_OK);
    rc = sqlite3_step (stmt); assert (rc == SQLITE_OK || rc == SQLITE_ROW);
    db_sizes[2] = sqlite3_column_int (stmt, 0);
    rc = sqlite3_finalize (stmt); assert (rc == SQLITE_OK);

    rc = sqlite3_prepare_v2 (db, "select count (*) from timestep_catalog", -1, &stmt, &tail); assert (rc == SQLITE_OK);
    rc = sqlite3_step (stmt); assert (rc == SQLITE_OK || rc == SQLITE_ROW);
    db_sizes[3] = sqlite3_column_int (stmt, 0);
    rc = sqlite3_finalize (stmt); assert (rc == SQLITE_OK);

    rc = sqlite3_prepare_v2 (db, "select count (*) from var_catalog", -1, &stmt, &tail); assert (rc == SQLITE_OK);
    rc = sqlite3_step (stmt); assert (rc == SQLITE_OK || rc == SQLITE_ROW);
    db_sizes[4] = sqlite3_column_int (stmt, 0);
    rc = sqlite3_finalize (stmt); assert (rc == SQLITE_OK);

    rc = sqlite3_prepare_v2 (db, "select count (*) from type_catalog", -1, &stmt, &tail); assert (rc == SQLITE_OK);
    rc = sqlite3_step (stmt); assert (rc == SQLITE_OK || rc == SQLITE_ROW);
    db_sizes[5] = sqlite3_column_int (stmt, 0);
    rc = sqlite3_finalize (stmt); assert (rc == SQLITE_OK);

    rc = sqlite3_prepare_v2 (db, "select count (*) from run_attribute_catalog", -1, &stmt, &tail); assert (rc == SQLITE_OK);
    rc = sqlite3_step (stmt); assert (rc == SQLITE_OK || rc == SQLITE_ROW);
    db_sizes[6] = sqlite3_column_int (stmt, 0);
    rc = sqlite3_finalize (stmt); assert (rc == SQLITE_OK);

    rc = sqlite3_prepare_v2 (db, "select count (*) from timestep_attribute_catalog", -1, &stmt, &tail); assert (rc == SQLITE_OK);
    rc = sqlite3_step (stmt); assert (rc == SQLITE_OK || rc == SQLITE_ROW);
    db_sizes[7] = sqlite3_column_int (stmt, 0);
    rc = sqlite3_finalize (stmt); assert (rc == SQLITE_OK);

    rc = sqlite3_prepare_v2 (db, "select count (*) from var_attribute_catalog", -1, &stmt, &tail); assert (rc == SQLITE_OK);
    rc = sqlite3_step (stmt); assert (rc == SQLITE_OK || rc == SQLITE_ROW);
    db_sizes[8] = sqlite3_column_int (stmt, 0);
    rc = sqlite3_finalize (stmt); assert (rc == SQLITE_OK);   

    debug_log << "sizeof db: " << db_sizes[1] << endl;
    debug_log << "num runs: " << db_sizes[2] << " num timesteps: " << db_sizes[3] << " num vars: " << db_sizes[4] << 
        " num types: " << db_sizes[5] << " num run attributes: " << db_sizes[6] << " num timestep attributes: " << db_sizes[7] <<  
        " num var attributes: " << db_sizes[8] << endl;

    add_timing_point(DB_ANALYSIS_DONE);

    if(output_db) {
        rc = sql_output_db(db, job_id, rank);
        if (rc != SQLITE_OK)
        {   
            fprintf (stderr, "Can't output database for job_id:%llu: %s\n",job_id, sqlite3_errmsg (db));        
            sqlite3_close (db);
            goto cleanup;
        }
    }

    sqlite3_close (db);

    add_timing_point(DB_CLOSED_SHUTDOWN_DONE);

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
        all_db_sizes = (uint64_t *) malloc(num_procs * num_db_pts * sizeof(uint64_t));
        all_clock_times = (long double *) malloc(num_procs * num_wall_time_pts * sizeof(long double));
    }

    debug_log << "time_pts.size(): " << time_pts.size() << endl;
    MPI_Gather(&num_time_pts, 1, MPI_INT, each_proc_num_time_pts, 1, MPI_INT, 0, MPI_COMM_WORLD); 

    MPI_Gather(db_sizes, num_db_pts, MPI_UNSIGNED_LONG, all_db_sizes, num_db_pts, MPI_UNSIGNED_LONG, 0, MPI_COMM_WORLD); 

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
        //prevent it from buffering the printf statements
        setbuf(stdout, NULL);

        cout << (int)DB_SIZES << " ";
        for(int i=0; i<(num_db_pts*num_procs); i++) {
            std::cout << all_db_sizes[i] << " ";
            if (i%20 == 0 && i!=0) {
                std::cout << std::endl;
            }
        }
        for(int i = 0; i<(num_wall_time_pts * num_procs); i++) {
            if(i%num_wall_time_pts == 0) {
                std::cout << (int)CLOCK_TIMES_NEW_PROC << " ";
            }
            if(all_clock_times[i] != 0) {
                printf("%.6Lf ", all_clock_times[i]);
            }
            if (i%20 == 0 && i!=0) {
                std::cout << std::endl;
            } 
        }
        std::cout << (int)CLOCK_TIMES_DONE << " ";

        //prevent it buffering the 
        for(int i=0; i<sum; i++) {
            printf("%4d %10.0f ", all_catg_time_pts_buf[i], all_time_pts_buf[i]);
            // std::cout << all_time_pts_buf[i] << " " << all_catg_time_pts_buf[i] << " ";
            if (i%20 == 0 && i!=0) {
                std::cout << std::endl;
            }
        }
        std::cout << std::endl;

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

static int sql_output_db(sqlite3 *pInMemory, uint64_t job_id, int rank)
{
    int rc;
    sqlite3 *pFile;           /* Database connection  */
    sqlite3_backup *pBackup;  /* Backup object used to copy data */

    add_timing_point(DB_OUTPUT_START);

    string filename = to_string(job_id) + "_" + to_string(rank);
    /* Open the database file. Exit early if this fails
      ** for any reason. */
    rc = sqlite3_open(filename.c_str(), &pFile);
    if( rc==SQLITE_OK ){  
        /* Set up the backup procedure to copy from the the main database of connection pInMemory
        to the "main" database of the connection pFile.
        ** If something goes wrong, pBackup will be set to NULL and an error
        ** code and message left in connection pFile.
        */
        // pBackup = sqlite3_backup_init(pTo, "main", pFrom, "main");
        pBackup = sqlite3_backup_init(pFile, "main", pInMemory, "main");


        /* If the backup object is successfully created, call backup_step()
        ** to copy data from pInMemory to pFile . Then call backup_finish()
        ** to release resources associated with the pBackup object.  If an
        ** error occurred, then an error code and message will be left in
        ** connection pTo. If no error occurred, then the error code belonging
        ** to pFile is set to SQLITE_OK.
        */
        if( pBackup ){
          (void)sqlite3_backup_step(pBackup, -1);
          (void)sqlite3_backup_finish(pBackup);
        }
        // rc = sqlite3_errcode(pTo);
        rc = sqlite3_errcode(pFile);
    }

    /* Close the database connection opened on database file zFilename
    ** and return the result of this function. */
    (void)sqlite3_close(pFile);

    add_timing_point(DB_OUTPUT_DONE);

    return rc;

}

static int sql_load_db(sqlite3 *pInMemory, uint64_t job_id, int rank)
{
    int rc;
    sqlite3 *pFile;           /* Database connection  */
    sqlite3_backup *pBackup;  /* Backup object used to copy data */

    add_timing_point(DB_LOAD_START);

    string filename = to_string(job_id) + "_" + to_string(rank);
    /* Open the database file. Exit early if this fails
      ** for any reason. */
    rc = sqlite3_open(filename.c_str(), &pFile);
    if( rc==SQLITE_OK ){  
        /* Set up the backup procedure to copy from the the main database of connection pInMemory
        to the "main" database of the connection pFile.
        ** If something goes wrong, pBackup will be set to NULL and an error
        ** code and message left in connection pFile.
        */
        // pBackup = sqlite3_backup_init(pTo, "main", pFrom, "main");
        pBackup = sqlite3_backup_init(pInMemory, "main", pFile, "main");

        /* If the backup object is successfully created, call backup_step()
        ** to copy data from pInMemory to pFile . Then call backup_finish()
        ** to release resources associated with the pBackup object.  If an
        ** error occurred, then an error code and message will be left in
        ** connection pTo. If no error occurred, then the error code belonging
        ** to pFile is set to SQLITE_OK.
        */
        if( pBackup ){
          (void)sqlite3_backup_step(pBackup, -1);
          (void)sqlite3_backup_finish(pBackup);
        }
        // rc = sqlite3_errcode(pTo);
        rc = sqlite3_errcode(pInMemory);
    }

    /* Close the database connection opened on database file zFilename
    ** and return the result of this function. */
    (void)sqlite3_close(pFile);

    add_timing_point(DB_LOAD_DONE);

    return rc;

}