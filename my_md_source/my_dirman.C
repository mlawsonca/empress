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
#include <iostream>
#include <chrono>
#include <fstream>
#include <sys/time.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include <mpi.h>
#include <unistd.h>

#include <OpFullShutdownMetaCommon.hh>

#include <opbox/services/dirman/DirectoryManager.hh>
#include <gutties/Gutties.hh>
#include <Globals.hh>
#include <opbox/OpBox.hh>

using namespace std;

std::string default_config_string = R"EOF(
# Select a transport to use for nnti 

nnti.logger.severity       error
nnti.transport.name        ibverbs
webhook.interfaces         ib0,lo

#config.additional_files.env_name.if_defined   CONFIG
lunasa.lazy_memory_manager   malloc
lunasa.eager_memory_manager  tcmalloc

#centralized dirman - sticks all the directory info on one node (called root). We use roles
# to designate which node is actually the root.
dirman.type           centralized
dirman.host_root true

node_role             dirman

# Turn these on if you want to see more debug messages
#bootstrap.debug           true
#webhook.debug             true 
#opbox.debug               true
#dirman.debug              true
#dirman.cache.others.debug true
#dirman.cache.mine.debug   true

)EOF";

int START = 0;
int MPI_INIT_DONE = 1;
int DIRMAN_SETUP_DONE = 2;
int REGISTER_OPS_DONE = 3;
int GENERATE_CONTACT_INFO_DONE = 4;
int FINALIZE = 5;

int CLOCK_TIMES_START = 98;
int CLOCK_TIMES_END = 99;

int ERR_GENERATE_CONTACT_INFO = 10000;

int RC_OK = 0;
int RC_ERR = -1;

bool debug_logging = false;
bool error_logging = false;

bool md_shutdown = false;

static int generate_contact_info (const string &dirmanHexID);
static void setup_dirman(const string &dir_path, const string &dir_info);

static void debug_log(const std::string &s) {
  if(debug_logging) std::cout << s<< std::endl;
}

static void error_log(const std::string &s) {
  if(error_logging) std::cout << s<< std::endl;
}


int main(int argc, char *argv[]) {
    char name[100];
    gethostname(name, sizeof(name));
    error_log(name);
    
    int num_time_pts = 6;
    vector<int> catg_of_time_pts;
    catg_of_time_pts.reserve(num_time_pts);
 
    struct timeval start, mpi_init_done, dirman_init_done, register_done, generate_done, finalize;
    vector<long double> clock_times;
    clock_times.reserve(num_time_pts);

    vector<chrono::high_resolution_clock::time_point> time_pts;
    time_pts.reserve(num_time_pts);

    gettimeofday(&start, NULL);
    int zero_time_sec = 3600 * (start.tv_sec / 3600);
    clock_times.push_back( (start.tv_sec - zero_time_sec) + start.tv_usec / 1000000.0);
    chrono::high_resolution_clock::time_point start_time = chrono::high_resolution_clock::now();
    time_pts.push_back(start_time);
    catg_of_time_pts.push_back(START); 


    int rc;
    debug_log("got here 1");
    MPI_Init(&argc, &argv);
    
    gettimeofday(&mpi_init_done, NULL);
    clock_times.push_back( (mpi_init_done.tv_sec - zero_time_sec) + mpi_init_done.tv_usec / 1000000.0);
    time_pts.push_back(chrono::high_resolution_clock::now());
    catg_of_time_pts.push_back(MPI_INIT_DONE);

    debug_log("got here 2");

    string dir_path="/metadata/testing";
    string dir_info="Dirman for sirius testing";
    setup_dirman(dir_path, dir_info);
    string myHexID = opbox::GetMyID().GetHex();
    debug_log("Current dirman nodeid is:   " + myHexID);

    gettimeofday(&dirman_init_done, NULL);
    clock_times.push_back((dirman_init_done.tv_sec - zero_time_sec) + dirman_init_done.tv_usec / 1000000.0);
    time_pts.push_back(chrono::high_resolution_clock::now());
    catg_of_time_pts.push_back(DIRMAN_SETUP_DONE);


    opbox::RegisterOp<OpFullShutdownMeta>();

    gettimeofday(&register_done, NULL);
    clock_times.push_back((register_done.tv_sec - zero_time_sec) + register_done.tv_usec / 1000000.0);
    time_pts.push_back(chrono::high_resolution_clock::now());
    catg_of_time_pts.push_back(REGISTER_OPS_DONE); 
    debug_log("got here 3");


    //write the URL to the contact info file so the clients and servers can find it
    rc = generate_contact_info (myHexID);
    if(rc != RC_OK) {
        time_pts.push_back(chrono::high_resolution_clock::now());
        catg_of_time_pts.push_back(ERR_GENERATE_CONTACT_INFO);
        goto cleanup;
    }
    debug_log("got here 4");

    gettimeofday(&generate_done, NULL);
    clock_times.push_back((generate_done.tv_sec - zero_time_sec) + generate_done.tv_usec / 1000000.0);
    time_pts.push_back(chrono::high_resolution_clock::now());
    catg_of_time_pts.push_back(GENERATE_CONTACT_INFO_DONE);

    debug_log("got here 5");

    while(!md_shutdown) {
         std::this_thread::sleep_for(std::chrono::milliseconds(10));
     }

cleanup:

    gettimeofday(&finalize, NULL);
    clock_times.push_back((finalize.tv_sec - zero_time_sec) + finalize.tv_usec / 1000000.0);
    time_pts.push_back(chrono::high_resolution_clock::now());
    catg_of_time_pts.push_back(FINALIZE);
  
    cout << CLOCK_TIMES_START << " ";
     for(int i=0; i<clock_times.size(); i++) {
        printf("%.6Lf ", clock_times[i]);
        // std::cout << total_time << " " << catg_of_time_pts.at(i) << " ";
    }
    for(int i=0; i<time_pts.size(); i++) {
        std::chrono::duration<double, std::nano> fp_ns = time_pts.at(i) - start_time;
        double total_time = fp_ns.count();
        printf("%d %10.0f ", catg_of_time_pts.at(i), total_time);

        // std::cout << total_time << " " << catg_of_time_pts.at(i) << " ";
    }
    debug_log("Finalizing");
    MPI_Finalize();
    gutties::bootstrap::Finish();
    return rc;
}

static int generate_contact_info (const string &dirmanHexID)
{
    char *contact_file=getenv("MD_DIRMAN_CONTACT_INFO"); //note: could make this more general
    if (contact_file==NULL) {
        error_log("Could not find env variable");
        return RC_ERR;
    }

    FILE *f=fopen(contact_file, "w");
    if (f==NULL) {
        error_log("Could not open file");
        return RC_ERR;
    }

    //write the URL to the contact file
    fprintf(f, "%s\n",dirmanHexID.c_str());
    fclose(f);
    return RC_OK;
}

//have the dirman create the directory for the servers to join
static void setup_dirman(const string &dir_path, const string &dir_info)
{
    bool ok;
    gutties::Configuration config(default_config_string);
    config.AppendFromReferences();
    gutties::bootstrap::Start(config, opbox::bootstrap);
    debug_log("Creating dirman"); 
    ok = dirman::HostNewDir(DirectoryInfo(dir_path, dir_info)); 
    assert(ok && "Root couldn't host a new directory?");
}

   