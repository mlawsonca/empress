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

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <mpi.h>
#include <ctime>
#include <stdint.h>
#include <ratio>
#include <chrono>
#include <math.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <fstream>

#include <3d_read_for_testing.hh>
#include <my_metadata_client.h>
#include <client_timing_constants.hh>

//needed for write
#include <OpCreateTypeMetaCommon.hh> 
#include <OpCreateVarMetaCommon.hh> 
#include <OpInsertAttributeMetaCommon.hh>
#include <OpInsertChunkMetaCommon.hh>

//needed for read 
#include <OpCatalogVarMetaCommon.hh>       
#include <OpCatalogTypeMetaCommon.hh>            
#include <OpGetAttributeMetaCommon.hh>
#include <OpGetChunkMetaCommon.hh>

#include <OpFullShutdownMetaCommon.hh>

//just for additional testing
#include <OpActivateTypeMetaCommon.hh>
#include <OpActivateVarMetaCommon.hh>
#include <OpCatalogTypeMetaCommon.hh>                   
#include <OpDeleteTypeMetaCommon.hh>           
#include <OpDeleteVarMetaCommon.hh>   
#include <OpGetAttributeListMetaCommon.hh>
#include <OpGetChunkListMetaCommon.hh>
#include <OpProcessingTypeMetaCommon.hh>
#include <OpProcessingVarMetaCommon.hh>

#include <opbox/services/dirman/DirectoryManager.hh>
#include <gutties/Gutties.hh>

//dw settings
std::string default_config_string = R"EOF(
# Tell the nnti transport to use infiniband

nnti.logger.severity       error
nnti.transport.name        ibverbs
webhook.interfaces         ib0,lo
 
#config.additional_files.env_name.if_defined   CONFIG
lunasa.lazy_memory_manager   malloc
lunasa.eager_memory_manager  tcmalloc

# Select the type of dirman to use. Currently we only have centralized, which
# just sticks all the directory info on one node (called root). 
dirman.type           centralized

# Turn these on if you want to see more debug messages
#bootstrap.debug           true
#webhook.debug             true
#opbox.debug               true
#dirman.debug              true
#dirman.cache.others.debug true
#dirman.cache.mine.debug   true

)EOF";

using namespace std;

static bool extreme_debug_logging = false;
static bool debug_logging = false;
static bool error_logging  =false;
static bool zero_rank_logging = false;
static errorLog error_log = errorLog(error_logging, zero_rank_logging);
static debugLog debug_log = debugLog(debug_logging, zero_rank_logging);
static extremeDebugLog extreme_debug_log = extremeDebugLog(extreme_debug_logging, zero_rank_logging);


//link to testing_debug.cpp in cmake to do debug tests
// static bool do_debug_testing = false;

std::vector<int> catg_of_time_pts;
std::vector<std::chrono::high_resolution_clock::time_point> time_pts;


static int setup_dirman(const string &dirman_file_path, const string &dir_path, metadata_server &dirman, vector<gutties::name_and_node_t> &server_nodes, int rank, uint32_t num_servers, uint32_t num_clients);
static void setup_server(metadata_server &server, int server_indx, int rank, uint32_t num_servers, const vector<gutties::name_and_node_t> &server_nodes);
static void setup_servers(std::vector<metadata_server> &servers, uint32_t num_servers, const vector<gutties::name_and_node_t> &server_nodes);
int extra_testing(MPI_Comm read_comm, int read_rank, std::vector<metadata_server> servers, uint32_t num_servers, metadata_server server, uint32_t num_vars, uint32_t num_types, uint64_t num_datasets);
// void write_testing(int rank, DirectoryInfo dir, metadata_server server, uint64_t num_datasets, uint32_t chunk_id);

/*
argv[1] = dirman hexid
argv[2] = number of subdivision of the x dimension (number of processes)
argv[3] = number of subdivisions of the y dimension (number of processes)
argv[4] = number of subdivisions of the z dimension (number of processes)
argv[5] = total length in the x dimension 
argv[6] = total length in the y dimension 
argv[7] = total length in the z dimension 
argv[8] = number of datasets to be stored in the database
argv[9] = number of types stored in each dataset
argv[10] = estimate of the number of testing time points we will have 
*/
int main(int argc, char **argv) {
    char name[100];
    gethostname(name, sizeof(name));
    extreme_debug_log << name << endl;

    if (argc != 12) { 
        error_log << "Error. Program should be given 10 arguments. Dirman hexid, npx, npy, npz, nx, ny, nz, number of datasets, number of types, estm num time_pts, num_servers" << endl;
        cout << ERR_ARGC << " " << 0 <<  endl;
        return RC_ERR;
    }

    float DEC_PERCENT_CLIENT_PROCS_USED_TO_READ = .1;

    uint32_t estm_num_time_pts = atoi(argv[10]); 
    catg_of_time_pts.reserve(estm_num_time_pts);
    time_pts.reserve(estm_num_time_pts);
    struct timeval start, mpi_init_done, register_ops_done, dirman_init_done, server_setup_done, write_var_and_type_end, write_end, pattern_read_end, read_end;
    int num_wall_time_pts = 9;
    vector<long double> clock_times;
    clock_times.reserve(num_wall_time_pts);

    gettimeofday(&start, NULL);
    int zero_time_sec = 3600 * (start.tv_sec / 3600);
    clock_times.push_back( (start.tv_sec - zero_time_sec) + start.tv_usec / 1000000.0);

    chrono::high_resolution_clock::time_point start_time = chrono::high_resolution_clock::now();
    time_pts.push_back(start_time);
    catg_of_time_pts.push_back(PROGRAM_START);

    MPI_Init(&argc, &argv);

    int rank;
    int num_client_procs;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size( MPI_COMM_WORLD, &num_client_procs);

    time_pts.push_back(chrono::high_resolution_clock::now());
    catg_of_time_pts.push_back(MPI_INIT_DONE);
    gettimeofday(&mpi_init_done, NULL);
    clock_times.push_back( (mpi_init_done.tv_sec - zero_time_sec) + mpi_init_done.tv_usec / 1000000.0);

    extreme_debug_log.set_rank(rank);
    debug_log.set_rank(rank);
    error_log.set_rank(rank);

    //needed for write
    opbox::RegisterOp<OpCreateTypeMeta>();
    opbox::RegisterOp<OpCreateVarMeta>();
    opbox::RegisterOp<OpInsertAttributeMeta>();
    opbox::RegisterOp<OpInsertChunkMeta>();

    //needed for read
    opbox::RegisterOp<OpCatalogVarMeta>();
    opbox::RegisterOp<OpFullShutdownMeta>();
    opbox::RegisterOp<OpGetAttributeMeta>();
    opbox::RegisterOp<OpGetChunkMeta>();

    //just for extra testing
    opbox::RegisterOp<OpActivateTypeMeta>();
    opbox::RegisterOp<OpActivateVarMeta>();   
    opbox::RegisterOp<OpCatalogTypeMeta>(); 
    opbox::RegisterOp<OpDeleteTypeMeta>();
    opbox::RegisterOp<OpDeleteVarMeta>();
    opbox::RegisterOp<OpGetAttributeListMeta>();
    opbox::RegisterOp<OpGetChunkListMeta>();
    opbox::RegisterOp<OpProcessingTypeMeta>();
    opbox::RegisterOp<OpProcessingVarMeta>();

    time_pts.push_back(chrono::high_resolution_clock::now());
    catg_of_time_pts.push_back(REGISTER_OPS_DONE);
    gettimeofday(&register_ops_done, NULL);
    clock_times.push_back( (register_ops_done.tv_sec - zero_time_sec) + register_ops_done.tv_usec / 1000000.0);

    if(rank == 0) {
        extreme_debug_log << "starting" << endl;
    }
    int rc;
    string dir_path="/metadata/testing";
    metadata_server dirman;

    metadata_server server;
    int server_indx;
    int my_num_clients_per_server;
    int remainder;

    md_dim_bounds dims[3];

    uint32_t num_vars = 10;
    vector<uint64_t> var_ids;

    //all 10 char plus null character
    char var_names[10][11] = {"temperat_1", "temperat_2", "pressure_1", "pressure_2", "velocity_1", "location_1", "magn_field", "energy_var", "density_vr", "conductivi"};  
    
    uint32_t version1 = 1;
    uint32_t version2 = 2;
    vector<uint64_t> type_ids;
    //given in %
    float type_freqs[] = {25, 5, .1, 100, 100, 20, 2.5, .5, 1, 10};
    char type_types[10][3] = {"b", "b", "b", "d", "d", "2p","2p","2p","2d","2d"};
    //all 12 char plus null character
    char type_names[10][13] = {"high_vel_flg", "high_tmp_flg", "high_prs_flg", "max_val_type", "min_val_type", "blob_bb_freq", "blb_bb_ifreq", "blob_bb_rare", "ranges_type1", "ranges_type2"}; 


    string dirman_hexid = argv[1];

    uint32_t num_x_procs_write = atoi(argv[2]);
    uint32_t num_y_procs_write = atoi(argv[3]);
    uint32_t num_z_procs_write = atoi(argv[4]);

    uint32_t total_x_length = atoi(argv[5]);
    uint32_t total_y_length = atoi(argv[6]);
    uint32_t total_z_length = atoi(argv[7]);

    uint64_t num_datasets = atoi(argv[8]);
    uint32_t num_types = atoi(argv[9]);
    uint32_t num_servers = atoi(argv[11]);

    uint32_t x_length_per_proc_write = total_x_length / num_x_procs_write; //fix - deal with edge case?
    uint32_t y_length_per_proc_write = total_y_length / num_y_procs_write;
    uint32_t z_length_per_proc_write = total_z_length / num_z_procs_write;

    struct md_catalog_var_entry temp_var;

    uint64_t chunk_id;

    struct md_chunk_entry new_chunk;
    uint64_t my_var_id;

    uint32_t var_num;

    uint32_t x_pos;
    uint32_t y_pos;
    uint32_t z_pos;
    uint32_t x_offset;
    uint32_t y_offset;
    uint32_t z_offset;

    vector<gutties::name_and_node_t> server_nodes;
    server_nodes.resize(num_servers);

    srand(rank);

    //just for testing-------------------------------------------
    double *all_time_pts_buf;
    int *all_catg_time_pts_buf;
    long double *all_clock_times;

    double *total_times; 
    //--------------------------------------------------------------

    time_pts.push_back(chrono::high_resolution_clock::now());
    catg_of_time_pts.push_back(INIT_VARS_DONE);

    rc = setup_dirman(argv[1], dir_path, dirman, server_nodes, rank, num_servers, num_client_procs);
    if (rc != RC_OK) {
        time_pts.push_back(chrono::high_resolution_clock::now());
        catg_of_time_pts.push_back(ERR_DIRMAN);
        error_log << "Error. Was unable to setup the dirman. Exiting" << endl;
        goto cleanup;
    }

    time_pts.push_back(chrono::high_resolution_clock::now());
    catg_of_time_pts.push_back(DIRMAN_SETUP_DONE);
    gettimeofday(&dirman_init_done, NULL);
    clock_times.push_back( (dirman_init_done.tv_sec - zero_time_sec) + dirman_init_done.tv_usec / 1000000.0);

    setup_server(server, server_indx, rank, num_servers, server_nodes);

    time_pts.push_back(chrono::high_resolution_clock::now());
    catg_of_time_pts.push_back(SERVER_SETUP_DONE_INIT_DONE);
    gettimeofday(&server_setup_done, NULL);
    clock_times.push_back( (server_setup_done.tv_sec - zero_time_sec) + server_setup_done.tv_usec / 1000000.0);
    //WRITE PROCESS
    //only want one copy of each var and type in each server's db
    time_pts.push_back(chrono::high_resolution_clock::now());
    catg_of_time_pts.push_back(WRITING_START);


    temp_var.type = 'l'; //fix - long, is this right?
    temp_var.num_dims = 3;
    dims [0].min = 0;
    dims [0].max = total_x_length-1;
    dims [1].min = 0;
    dims [1].max = total_y_length-1;
    dims [2].min = 0;
    dims [2].max = total_z_length-1;
    temp_var.dims = std::vector<md_dim_bounds>(dims, dims + temp_var.num_dims );

    my_num_clients_per_server = num_client_procs / num_servers; //each has this many at a min
    remainder = num_client_procs - (num_servers * my_num_clients_per_server);

    if(server_indx < remainder) {
        my_num_clients_per_server +=1;
    }

    var_ids.reserve ( num_datasets * num_vars / my_num_clients_per_server + 1); 
    type_ids.reserve ( num_datasets * num_types / my_num_clients_per_server + 1); 

    for(uint32_t i=0; (my_num_clients_per_server * i + (rank / num_servers) ) < (num_datasets * num_vars); i++) {
        time_pts.push_back(chrono::high_resolution_clock::now());
        catg_of_time_pts.push_back(CREATE_NEW_VAR_START);

        uint64_t varnum = (rank / num_servers) + (i*my_num_clients_per_server);      
        uint16_t index = varnum % num_vars;
        uint64_t dataset_num = varnum / num_vars;

        temp_var.txn_id = dataset_num;
        temp_var.name = var_names[index];
        temp_var.path = "/"+ (string) var_names[index];
        if(index == 1 || index == 3) { //these have been designated ver2
            temp_var.version = version2;
        }
        else {
            temp_var.version = version1;
        }
                
        time_pts.push_back(chrono::high_resolution_clock::now());
        catg_of_time_pts.push_back(VAR_INIT_DONE);

        rc = metadata_create_var (server, var_ids[i], temp_var);
        if (rc != RC_OK) {
            error_log << "Error. Was unable to create the temp var. Exiting" << endl;
            time_pts.push_back(chrono::high_resolution_clock::now());
            catg_of_time_pts.push_back(ERR_CREATE_VAR);
            goto cleanup;
        }

        debug_log << "Finished creating var. new var_id " << to_string(var_ids[i]) << " and dataset num " << dataset_num << endl;
    }

    if(num_types <= 0) {
        time_pts.push_back(chrono::high_resolution_clock::now());
        catg_of_time_pts.push_back(CREATE_VARS_DONE);
    }
    else {
        time_pts.push_back(chrono::high_resolution_clock::now());
        catg_of_time_pts.push_back(CREATE_VARS_DONE_START_CREATE_TYPES);    
    }

    for(int i=0; (my_num_clients_per_server * i + (rank / num_servers) ) < (num_datasets * num_types); i++) {
        time_pts.push_back(chrono::high_resolution_clock::now());
        catg_of_time_pts.push_back(CREATE_NEW_TYPE_START);

        uint64_t typenum = (rank / num_servers) + (i*my_num_clients_per_server);      
        uint16_t index = typenum % num_types;
        uint64_t dataset_num = typenum / num_types;

        struct md_catalog_type_entry temp_type;
        temp_type.name = type_names[index];
        temp_type.version = 0; 
        temp_type.txn_id = dataset_num;
        time_pts.push_back(chrono::high_resolution_clock::now());
        catg_of_time_pts.push_back(TYPE_INIT_DONE);

        rc = metadata_create_type (server, type_ids[i], temp_type);
        if (rc != RC_OK) {
            time_pts.push_back(chrono::high_resolution_clock::now());
            catg_of_time_pts.push_back(ERR_CREATE_TYPE);
            error_log << "Error. Was unable to insert the first chunk. Proceeding \n";
        }
        debug_log << "creating type with type_id " << to_string(type_ids[i]) << " and name " << temp_type.name << endl;
    }

    time_pts.push_back(chrono::high_resolution_clock::now());
    catg_of_time_pts.push_back(CREATE_TYPES_DONE);

    gettimeofday(&write_var_and_type_end, NULL);
    clock_times.push_back( (write_var_and_type_end.tv_sec - zero_time_sec) + write_var_and_type_end.tv_usec / 1000000.0);

   
//-------------------------------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------------------------------        
//INSERT CHUNKS AND ATTRIBUTE INSTANCES

    x_pos = rank % num_x_procs_write;
    y_pos = (rank / num_x_procs_write) % num_y_procs_write; 
    z_pos = ((rank / (num_x_procs_write * num_y_procs_write)) % num_z_procs_write);

    x_offset = x_pos * x_length_per_proc_write;
    y_offset = y_pos * y_length_per_proc_write;
    z_offset = z_pos * z_length_per_proc_write;

    extreme_debug_log << "zoffset: " << to_string(z_offset) << endl;
    extreme_debug_log << "x pos is " << to_string(x_pos) << " and x_offset is " << to_string(x_offset) << endl;
    extreme_debug_log << "y pos is " << to_string(y_pos) << " and y_offset is " << to_string(y_offset) << endl;
    extreme_debug_log << "z pos is " << to_string(z_pos) << " and z_offset is " << to_string(z_offset) << endl;
    extreme_debug_log << "num x procs " << to_string(num_x_procs_write) << " num y procs " << to_string(num_y_procs_write) << " num z procs " << to_string(num_z_procs_write) << endl;


    new_chunk.connection = server.URL; //note - this is a placeholder for the storage location of the chunk's data
    new_chunk.num_dims = 3;
    dims [0].min = x_offset;
    dims [0].max = x_offset + x_length_per_proc_write-1;
    dims [1].min = y_offset;
    dims [1].max = y_offset + y_length_per_proc_write -1;
    dims [2].min = z_offset;
    dims [2].max = z_offset + z_length_per_proc_write -1;
    new_chunk.length_of_chunk = (dims [0].max - dims [0].min + 1)  * (dims [1].max - dims [1].min + 1)  * (dims [2].max - dims [2].min + 1);
    new_chunk.dims = std::vector<md_dim_bounds>(dims, dims + new_chunk.num_dims );
    
    extreme_debug_log << "About to insert chunk" << endl;
    for (int j=0; j<3; j++) {
        extreme_debug_log << "dims [" << to_string(j) << "] min is: " << to_string(dims [j].min) << endl;
        extreme_debug_log << "dims [" << to_string(j) << "] max is: " << to_string(dims [j].max) << endl;
    }   

    time_pts.push_back(chrono::high_resolution_clock::now());
    catg_of_time_pts.push_back(CHUNK_INIT_DONE);

    for(uint64_t dataset_num=0; dataset_num< num_datasets; dataset_num++) {
        time_pts.push_back(chrono::high_resolution_clock::now());
        catg_of_time_pts.push_back(INSERT_FOR_NEW_DATASET_START);

        for(uint32_t var_indx = 0; var_indx<num_vars; var_indx++) {
            time_pts.push_back(chrono::high_resolution_clock::now());
            catg_of_time_pts.push_back(INSERT_FOR_NEW_VAR_START);

            var_num = var_indx + (10 * dataset_num);
            my_var_id = var_num+1; //note - if we change the varid systm this will change

            if ( (var_num%10) == 1 || (var_num%10) == 3) { 
                new_chunk.var_version = version2;
            }
            else {
               new_chunk.var_version = version1;
            }

            rc = metadata_insert_chunk (server, my_var_id, chunk_id, new_chunk);
            if(rc != RC_OK) {
                error_log << "Error. Was unable to insert the chunk for rank " << to_string(rank) << ". Exiting" << endl;
                time_pts.push_back(chrono::high_resolution_clock::now());
                catg_of_time_pts.push_back(ERR_INSERT_CHUNK);  
                goto cleanup;
            }
            extreme_debug_log << "txn_id: " << to_string(dataset_num) << " var_id: " << to_string(my_var_id) << " chunk_id: " << chunk_id << endl;

            //insert attribute instances
            if(num_types > 0) {
                time_pts.push_back(chrono::high_resolution_clock::now());
                catg_of_time_pts.push_back(INSERT_ATTRS_START);   
            }

            for(uint32_t type_indx=0; type_indx<num_types; type_indx++) {
                std::chrono::high_resolution_clock::time_point create_attr_start_time = chrono::high_resolution_clock::now();

                float freq = type_freqs[type_indx];
                uint32_t odds = 100 / freq;
                uint32_t val = rand() % odds; 
                extreme_debug_log << "rank: " << to_string(rank) << " and val: " << to_string(val) << " and odds: " << to_string(odds) << endl;
                if(val == 0) { //makes sure we have the desired frequency for each type
                    time_pts.push_back(create_attr_start_time);
                    catg_of_time_pts.push_back(CREATE_NEW_ATTR_START);
                    stringstream ss;
                    boost::archive::text_oarchive oa(ss);

                    uint64_t attr_id;
                    md_attribute_entry temp_attr;
                    temp_attr.type_id = (type_indx+1)+(dataset_num*10); //note - if we change the id system this will change
                    temp_attr.chunk_id = chunk_id;

                    if(strcmp(type_types[type_indx], "b") == 0) {
                        bool flag = rank % 2;
                        oa << flag;
                    }
                    else if(strcmp(type_types[type_indx], "d") == 0) {
                        double val = (double) (rand() % RAND_MAX); //a random double
                        oa << val;
                    }
                    else if(strcmp(type_types[type_indx], "2p") == 0) {
                        point points[2];
                        points[0].x = (double) (rand() % RAND_MAX); //a random double
                        points[0].y = (double) (rand() % RAND_MAX); //a random double
                        points[0].z = (double) (rand() % RAND_MAX); //a random double
                        points[1].x = points[0].x + x_length_per_proc_write / 2;
                        points[1].y = points[0].y + y_length_per_proc_write / 2;
                        points[1].z = points[0].z + z_length_per_proc_write / 2;

                        oa << points;
                    }            
                    else if(strcmp(type_types[type_indx], "2d") == 0) {
                        int vals[2];
                        vals[0] = (int) (rand() % RAND_MAX); 
                        vals[1] = vals[0] + 10000; 
                        oa << vals;
                    }
                    else {
                        error_log << "error. type didn't match list of possibilities" << endl;
                        time_pts.push_back(chrono::high_resolution_clock::now());
                        catg_of_time_pts.push_back(ERR_TYPE_COMPARE);
                    }
                    extreme_debug_log << "rank: " << to_string(rank) << " ss: " << ss.str() << endl;
                    extreme_debug_log <<  "sizeof str: " << to_string(ss.str().size()) << endl;
                    std::string serial_str = ss.str();
                    temp_attr.data = serial_str;

                    time_pts.push_back(chrono::high_resolution_clock::now());
                    catg_of_time_pts.push_back(ATTR_INIT_DONE);

                    rc = metadata_insert_attribute (server, attr_id, temp_attr);
                    if (rc != RC_OK) {
                        error_log << "Error. Was unable to insert the first attribute. Proceeding" << endl;
                        time_pts.push_back(chrono::high_resolution_clock::now());
                        catg_of_time_pts.push_back(ERR_INSERT_ATTR);
                    }

                }//val = 0 done
            } //type loop done
            if(num_types > 0) {
                time_pts.push_back(chrono::high_resolution_clock::now());
                catg_of_time_pts.push_back(INSERT_ATTRS_DONE);   
            } 
            time_pts.push_back(chrono::high_resolution_clock::now());
            catg_of_time_pts.push_back(INSERT_FOR_NEW_VAR_DONE);   
        } //var loop done 
        time_pts.push_back(chrono::high_resolution_clock::now());
        catg_of_time_pts.push_back(INSERT_FOR_NEW_DATASET_DONE);  
    } //dataset loop done
    time_pts.push_back(chrono::high_resolution_clock::now());
    catg_of_time_pts.push_back(WRITING_DONE);  

    gettimeofday(&write_end, NULL);
    clock_times.push_back( (write_end.tv_sec - zero_time_sec) + write_end.tv_usec / 1000000.0);

//done with initialization

    // if(do_debug_testing) {
    //     write_testing(rank, dir, server, num_datasets, chunk_id);
    // }
// //-------------------------------------------------------------------------------------------------------------------------------------------
// //-------------------------------------------------------------------------------------------------------------------------------------------        
// //READ

      
    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Comm read_comm;
    int color;
    if (rank < (num_client_procs * DEC_PERCENT_CLIENT_PROCS_USED_TO_READ) ) {
        color = 0;
    }
    else {
        color = MPI_UNDEFINED; //the read comm should not be used for procs that aren't assigned to read
    }

    MPI_Comm_split(MPI_COMM_WORLD, color, rank, &read_comm);

    if( color == 0 )  {
        time_pts.push_back(chrono::high_resolution_clock::now());
        catg_of_time_pts.push_back(READING_START);

        std::string serial_str;
        vector<metadata_server> servers;
        int total_num_read_procs = num_client_procs * DEC_PERCENT_CLIENT_PROCS_USED_TO_READ;
        int my_cbrt = cbrt(total_num_read_procs);
        while(total_num_read_procs % my_cbrt != 0) {
            my_cbrt -= 1;
        }
        int my_cbrt2 = cbrt(total_num_read_procs);
        while(total_num_read_procs % my_cbrt2 != 0) {
            my_cbrt2 += 1;
        }
        int num_x_procs_read = my_cbrt;
        int num_y_procs_read = my_cbrt2;
        if(num_x_procs_read < 1) {
            num_x_procs_read = 1;
        }
        if(num_x_procs_read < 1) {
            num_x_procs_read = 1;
        }
        int num_z_procs_read = total_num_read_procs / (my_cbrt * my_cbrt2);
        if(num_z_procs_read < num_y_procs_read) {
            int temp = num_y_procs_read;
            num_y_procs_read = num_z_procs_read;
            num_z_procs_read = temp;
        }
       
        const int num_datasets_to_fetch = 6;
        uint64_t txn_ids[num_datasets_to_fetch] = {(num_datasets-1) / 6, (num_datasets-1) / 5, (num_datasets-1) / 4,
                            (num_datasets-1) / 3, (num_datasets-1) / 2, num_datasets-1};
        char *serialized_c_str;
        int num_bytes_and_entries_and_sqrt[3];
        vector<vector<md_catalog_var_entry>> all_dataset_entries;
        all_dataset_entries.reserve(num_datasets_to_fetch);
        uint64_t var_id0 = 1; 
        uint64_t var_id1 = num_vars/6;
        uint64_t var_id2 = num_vars/2;
        uint64_t var_id3 = num_vars/4;

        std::vector<uint64_t> var_ids_to_fetch(3);
        int num_vars_to_fetch = 3;
        int plane_x_procs_read;
        int plane_y_procs_read;

        int read_rank;
        MPI_Comm_rank(read_comm, &read_rank);

        servers.reserve(num_servers);

        extreme_debug_log <<  "my read_rank is "<< to_string(read_rank) << endl;

        //NOTE: IF WE WERE USING SEPARATE PROCS, THEY WOULD ALL HAVE TO CONTACT THE DIRMAN HERE
        //NOTE: IF WE WERE USING SEPARATE PROCS, THEY WOULD ALL HAVE TO DO MPI_INIT/LAUNCH OPS

        time_pts.push_back(chrono::high_resolution_clock::now());
        catg_of_time_pts.push_back(READING_SETUP_DONE);
        setup_servers(servers, num_servers, server_nodes);
        time_pts.push_back(chrono::high_resolution_clock::now());
        catg_of_time_pts.push_back(SERVERS_SETUP_DONE);

        //CATALOG ENTRIES AND BCAST
       if (read_rank == 0) {
            std::vector<md_catalog_var_entry> entries;
            stringstream ss;
            boost::archive::text_oarchive oa(ss);
            uint32_t num_vars;
            serial_str;
            for(int i=0; i<num_datasets_to_fetch; i++) {
                //returns information about the variables associated with the given txn_id 
                rc = metadata_catalog_var(servers.at(0), txn_ids[i], num_vars, entries);
                if (rc != RC_OK) {
                    time_pts.push_back(chrono::high_resolution_clock::now());
                    catg_of_time_pts.push_back(ERR_CATALOG_VAR);  
                    goto cleanup;                 
                    error_log << "Error cataloging the second set of entries. Proceeding \n";
                }
                all_dataset_entries.push_back(entries);
            }

            oa << all_dataset_entries;
            debug_log << "total num read procs: " << to_string(total_num_read_procs) << endl;
            serial_str = ss.str();
            serialized_c_str = (char *) serial_str.c_str();
            int my_sqrt = floor(sqrt(total_num_read_procs));
            while(total_num_read_procs % my_sqrt != 0) {
                my_sqrt -= 1;
            }

            num_bytes_and_entries_and_sqrt[0] = serial_str.size() + 1;
            num_bytes_and_entries_and_sqrt[1] = num_vars;
            num_bytes_and_entries_and_sqrt[2] = my_sqrt;
            debug_log <<  "my_sqrt: " << to_string(my_sqrt) << endl;
        }
        MPI_Bcast(num_bytes_and_entries_and_sqrt, 3, MPI_INT, 0, read_comm);

        if(read_rank != 0) {
            debug_log << "num_bytes is "<< to_string(num_bytes_and_entries_and_sqrt[0]) + " and read_rank: " << to_string(read_rank) << endl;
            serialized_c_str = (char *) malloc(num_bytes_and_entries_and_sqrt[0]);  
            debug_log << "num of vars is " << to_string(num_bytes_and_entries_and_sqrt[1]) << endl;
        }

        MPI_Bcast(serialized_c_str, num_bytes_and_entries_and_sqrt[0], MPI_CHAR, 0, read_comm);

        if (read_rank != 0) {
            extreme_debug_log << "serialized_c_str: " << (string)serialized_c_str << endl;
            stringstream ss1;
            ss1 << serialized_c_str; 
            boost::archive::text_iarchive ia(ss1);
            ia >> all_dataset_entries;
            free(serialized_c_str);
        }

        time_pts.push_back(chrono::high_resolution_clock::now());
        catg_of_time_pts.push_back(BCAST_CATALOGS_DONE);

        //pattern testing for vars
        rc = read_pattern_1 (read_rank, num_servers, servers, false,
                        num_x_procs_read, num_y_procs_read, num_z_procs_read,
                        all_dataset_entries.at(0), num_bytes_and_entries_and_sqrt[1],
                        total_x_length, total_y_length, total_z_length, txn_ids[0]);
        if(rc != RC_OK) {
            time_pts.push_back(chrono::high_resolution_clock::now());
            catg_of_time_pts.push_back(ERR_PATTERN_1);  
        }
        debug_log << "finished pattern 1" << endl;

        debug_log << "txn_ids: " << txn_ids[1] << " varid to fetch: " << all_dataset_entries.at(1).at(2).var_id << endl;
        rc = read_pattern_2 (read_comm, read_rank, num_servers, servers, false,
                        num_x_procs_read, num_y_procs_read, num_z_procs_read,
                        all_dataset_entries.at(1), num_bytes_and_entries_and_sqrt[1],
                        total_x_length, total_y_length, total_z_length, txn_ids[1], all_dataset_entries.at(1).at(0).var_id);
        debug_log << "finished pattern 2" << endl;
        if(rc != RC_OK) {
            time_pts.push_back(chrono::high_resolution_clock::now());
            catg_of_time_pts.push_back(ERR_PATTERN_2);  
        }
        var_ids_to_fetch[0] = (all_dataset_entries.at(2).at(num_vars-1).var_id);
        var_ids_to_fetch[1] = (all_dataset_entries.at(2).at(num_vars/5).var_id);
        var_ids_to_fetch[2] = (all_dataset_entries.at(2).at(num_vars/3).var_id);

        rc = read_pattern_3 (read_comm, read_rank, num_servers, servers, false,
                        num_x_procs_read, num_y_procs_read, num_z_procs_read, all_dataset_entries.at(2), 
                        num_bytes_and_entries_and_sqrt[1], total_x_length, total_y_length, 
                        total_z_length, txn_ids[2], var_ids_to_fetch, num_vars_to_fetch);
        debug_log << "finished pattern 3" << endl;
        if(rc != RC_OK) {
            time_pts.push_back(chrono::high_resolution_clock::now());
            catg_of_time_pts.push_back(ERR_PATTERN_3);  
        }
        //want to use all read procs in pattern 4 and 6 as "x and y" procs
        extreme_debug_log << "total_num_read_procs: " << total_num_read_procs << endl;
        plane_x_procs_read = num_bytes_and_entries_and_sqrt[2];
        plane_y_procs_read = total_num_read_procs / plane_x_procs_read;
        extreme_debug_log << "num x procs: " << num_x_procs_read << " num y procs: "; 
        extreme_debug_log << num_y_procs_read << " num z procs: " << num_z_procs_read << " num x procs plane: ";
        extreme_debug_log << plane_x_procs_read << " num y procs plane: " << plane_y_procs_read << endl;
        rc = read_pattern_4 (read_comm, read_rank, num_servers, servers, false,
                        plane_x_procs_read, plane_y_procs_read,
                        all_dataset_entries.at(3), num_bytes_and_entries_and_sqrt[1],
                        total_x_length, total_y_length, total_z_length, txn_ids[3], all_dataset_entries.at(3).at(num_vars/2).var_id);
        debug_log << "finished pattern 4" << endl;
        if(rc != RC_OK) {
            time_pts.push_back(chrono::high_resolution_clock::now());
            catg_of_time_pts.push_back(ERR_PATTERN_4);  
        }
        rc = read_pattern_5 (read_comm, read_rank, num_servers, servers, false,
                        num_x_procs_read, num_y_procs_read, num_z_procs_read,
                        all_dataset_entries.at(4), num_bytes_and_entries_and_sqrt[1],
                        total_x_length, total_y_length, total_z_length, txn_ids[4], all_dataset_entries.at(4).at(num_vars/4).var_id);
        debug_log << "finished pattern 5" << endl;
        if(rc != RC_OK) {
            time_pts.push_back(chrono::high_resolution_clock::now());
            catg_of_time_pts.push_back(ERR_PATTERN_5);  
        }
        rc = read_pattern_6 (read_comm, read_rank, num_servers, servers, false,
                        plane_x_procs_read, plane_y_procs_read,
                        all_dataset_entries.at(5), num_bytes_and_entries_and_sqrt[1],
                        total_x_length, total_y_length, total_z_length, txn_ids[5], all_dataset_entries.at(5).at(num_vars/6).var_id);
        debug_log << "finished pattern 6" << endl;
        if(rc != RC_OK) {
            time_pts.push_back(chrono::high_resolution_clock::now());
            catg_of_time_pts.push_back(ERR_PATTERN_6);  
        }
        time_pts.push_back(chrono::high_resolution_clock::now());
        catg_of_time_pts.push_back(READING_PATTERNS_DONE); 
        if (num_types > 0) {
            //pattern testing for types
            rc = read_pattern_1 (read_rank, num_servers, servers, true,
                            num_x_procs_read, num_y_procs_read, num_z_procs_read,
                            all_dataset_entries.at(0), num_bytes_and_entries_and_sqrt[1],
                            total_x_length, total_y_length, total_z_length, txn_ids[0]);
            if(rc != RC_OK) {
                time_pts.push_back(chrono::high_resolution_clock::now());
                catg_of_time_pts.push_back(ERR_PATTERN_1);  
            }
            debug_log << "finished pattern 1t" << endl;

            rc = read_pattern_2 (read_comm, read_rank, num_servers, servers, true,
                            num_x_procs_read, num_y_procs_read, num_z_procs_read,
                            all_dataset_entries.at(1), num_bytes_and_entries_and_sqrt[1],
                            total_x_length, total_y_length, total_z_length, txn_ids[1], all_dataset_entries.at(1).at(0).var_id);
            debug_log << "finished pattern 2t" << endl;
            if(rc != RC_OK) {
                time_pts.push_back(chrono::high_resolution_clock::now());
                catg_of_time_pts.push_back(ERR_PATTERN_2);  
            }          
            rc = read_pattern_3 (read_comm, read_rank, num_servers, servers, true,
                            num_x_procs_read, num_y_procs_read, num_z_procs_read, all_dataset_entries.at(2), 
                            num_bytes_and_entries_and_sqrt[1], total_x_length, total_y_length, 
                            total_z_length, txn_ids[2], var_ids_to_fetch, num_vars_to_fetch);
            debug_log << "finished pattern 3t" << endl;
            if(rc != RC_OK) {
                time_pts.push_back(chrono::high_resolution_clock::now());
                catg_of_time_pts.push_back(ERR_PATTERN_3);  
            }
            //want to use all read procs in pattern 4 and 6 as "x and y" procs
            extreme_debug_log << "num z procs read / 2: " << (num_z_procs_read / 2) << endl;
            rc = read_pattern_4 (read_comm, read_rank, num_servers, servers, true,
                            plane_x_procs_read, plane_y_procs_read,
                            all_dataset_entries.at(3), num_bytes_and_entries_and_sqrt[1],
                            total_x_length, total_y_length, total_z_length, txn_ids[3], all_dataset_entries.at(3).at(num_vars/2).var_id);
            debug_log << "finished pattern 4t" << endl;
            if(rc != RC_OK) {
                time_pts.push_back(chrono::high_resolution_clock::now());
                catg_of_time_pts.push_back(ERR_PATTERN_4);  
            }

            read_pattern_5 (read_comm, read_rank, num_servers, servers, true,
                            num_x_procs_read, num_y_procs_read, num_z_procs_read,
                            all_dataset_entries.at(4), num_bytes_and_entries_and_sqrt[1],
                            total_x_length, total_y_length, total_z_length, txn_ids[4], all_dataset_entries.at(4).at(num_vars/4).var_id);
            debug_log << "finished pattern 5t" << endl;
            if(rc != RC_OK) {
                time_pts.push_back(chrono::high_resolution_clock::now());
                catg_of_time_pts.push_back(ERR_PATTERN_5);  
            }         

            rc = read_pattern_6 (read_comm, read_rank, num_servers, servers, true,
                            plane_x_procs_read, plane_y_procs_read,
                            all_dataset_entries.at(5), num_bytes_and_entries_and_sqrt[1],
                            total_x_length, total_y_length, total_z_length, txn_ids[5], all_dataset_entries.at(5).at(num_vars/6).var_id);
            debug_log << "finished pattern 6t" << endl;
            if(rc != RC_OK) {
                time_pts.push_back(chrono::high_resolution_clock::now());
                catg_of_time_pts.push_back(ERR_PATTERN_6);  
            }
            gettimeofday(&pattern_read_end, NULL);
            clock_times.push_back( (pattern_read_end.tv_sec - zero_time_sec) + pattern_read_end.tv_usec / 1000000.0 );
            time_pts.push_back(chrono::high_resolution_clock::now());
            catg_of_time_pts.push_back(READING_TYPE_PATTERNS_DONE); 

            extra_testing(read_comm, read_rank, servers, num_servers, server, num_vars, num_types, num_datasets);
            gettimeofday(&read_end, NULL);
            clock_times.push_back( (read_end.tv_sec - zero_time_sec) + read_end.tv_usec / 1000000.0 );
            time_pts.push_back(chrono::high_resolution_clock::now());
            catg_of_time_pts.push_back(EXTRA_TESTING_DONE); 
        }
        else {
            gettimeofday(&pattern_read_end, NULL);
            clock_times.push_back( (pattern_read_end.tv_sec - zero_time_sec) + pattern_read_end.tv_usec / 1000000.0);  
        }
        time_pts.push_back(chrono::high_resolution_clock::now());
        catg_of_time_pts.push_back(READING_DONE); 
    }


   
cleanup:
    MPI_Barrier (MPI_COMM_WORLD);
    time_pts.push_back(chrono::high_resolution_clock::now());
    catg_of_time_pts.push_back(READING_DONE_FOR_ALL_PROCS_START_CLEANUP);
    
    if(rc != RC_DIRMAN_ERR && rank < num_servers) {
        metadata_finalize_server(server);
        time_pts.push_back(chrono::high_resolution_clock::now());
        catg_of_time_pts.push_back(SERVER_SHUTDOWN_DONE); 
        debug_log << "just finished finalizing" << endl;
        if(rank == 0) {
            metadata_finalize_server(dirman);
            time_pts.push_back(chrono::high_resolution_clock::now());
            catg_of_time_pts.push_back(DIRMAN_SHUTDOWN_DONE); 
        }
    }

    int num_time_pts = time_pts.size();
    int *catg_of_time_pts_ary = &catg_of_time_pts[0];
    int displacement_for_each_proc[num_client_procs];
    total_times = (double *) malloc(sizeof(double) * num_time_pts);
    int *each_proc_num_time_pts;
    if(rank == 0) {
        debug_log << "num_client_procs: " << to_string(num_client_procs) << endl;
        each_proc_num_time_pts = (int *) malloc(num_client_procs * sizeof(int));
    }
   debug_log << "num_time pts: " << to_string(num_time_pts)  << "time_pts.size(): " << time_pts.size()<< endl;

    MPI_Gather(&num_time_pts, 1, MPI_INT, each_proc_num_time_pts, 1, MPI_INT, 0, MPI_COMM_WORLD); 

    int sum = 0;
    if(rank == 0) {
        for(int i=0; i<num_client_procs; i++) {
            displacement_for_each_proc[i] = sum;
            sum += each_proc_num_time_pts[i];
            extreme_debug_log << "each_proc_num_time_pts[i]: " << each_proc_num_time_pts[i] << endl;
            extreme_debug_log << "sum: " << sum << endl;
        }
        all_time_pts_buf = (double *) malloc(sum * sizeof(double));
        all_catg_time_pts_buf = (int *) malloc(sum * sizeof(int));
        all_clock_times = (long double *) malloc(num_client_procs * num_wall_time_pts * sizeof(long double));

        debug_log << "sum: " << to_string(sum) << endl;
        debug_log << "estm num_time_pts: " << to_string(estm_num_time_pts) << endl;
    }
    for(int i=0; i< time_pts.size(); i++) {
        std::chrono::duration<double, std::nano> fp_ns = time_pts.at(i) - start_time;
        total_times[i] = fp_ns.count();
    }


    MPI_Gatherv(total_times, num_time_pts, MPI_DOUBLE, all_time_pts_buf, each_proc_num_time_pts, displacement_for_each_proc, MPI_DOUBLE, 0, MPI_COMM_WORLD); 

    MPI_Gatherv(catg_of_time_pts_ary, num_time_pts, MPI_INT, all_catg_time_pts_buf, each_proc_num_time_pts, displacement_for_each_proc, MPI_INT, 0, MPI_COMM_WORLD); 
    
    MPI_Gather(&clock_times[0], num_wall_time_pts, MPI_LONG_DOUBLE, all_clock_times, num_wall_time_pts, MPI_LONG_DOUBLE, 0, MPI_COMM_WORLD); 

    if (rank == 0) {
        for(int i = 0; i<(num_wall_time_pts * num_client_procs); i++) {
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
            printf("%d %10.0f ", all_catg_time_pts_buf[i], all_time_pts_buf[i]);
            // std::cout << all_catg_time_pts_buf[i] << " " << all_time_pts_buf[i] << " ";
            if (i%20 == 0 && i!=0) {
                std::cout << std::endl;
            }
        }
        free(all_clock_times);
        free(all_time_pts_buf);
        free(all_catg_time_pts_buf);
        free(each_proc_num_time_pts);
    }
    
    free(total_times);

    MPI_Barrier (MPI_COMM_WORLD);
    if ( rc == RC_OK && read_comm != MPI_COMM_NULL ) {
        MPI_Comm_free(&read_comm);
    }
    MPI_Finalize();
    gutties::bootstrap::Finish();
    debug_log << "got to cleanup7" << endl;

    return rc;

}

static int setup_dirman(const string &dirman_file_path, const string &dir_path, metadata_server &dirman, 
                        vector<gutties::name_and_node_t> &server_nodes, int rank, uint32_t num_servers, uint32_t num_clients) {

    bool ok;
    int rc;
    // gutties::name_and_node_t *server_ary;

    if(rank == 0) { 
        struct stat buffer;
        bool dirman_initted = false;
        std::ifstream file;
        string dirman_hexid;
        char dirman_hexid_c_str[14];
        DirectoryInfo dir;

        while (!dirman_initted) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            dirman_initted = (stat (dirman_file_path.c_str(), &buffer) == 0);
            debug_log << "dirman not initted yet \n";
        }
        file.open(dirman_file_path);
        if(!file) {
            return RC_ERR;
        }
        while( file.peek() == std::ifstream::traits_type::eof() ) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            debug_log << "dirman file empty \n";
        }
        file >> dirman_hexid;
        // dirman_hexid_c_str = (char *)dirman_hexid.c_str();
        debug_log << "just got the hexid: " << dirman_hexid << endl;

          //Add the directory manager's URL to the config string so the clients know
        //Where to access the directory 
        gutties::Configuration config(default_config_string);
        config.Append("dirman.root_node", dirman_hexid_c_str);
        config.Append("webhook.port", to_string(3000+rank)); //note if you up number of servers you'll want to up this
        config.AppendFromReferences();

        debug_log << "just configged" << endl; 
        gutties::bootstrap::Start(config, opbox::bootstrap);

        //-------------------------------------------
        //TODO: This connect is temporarily necessary
        gutties::nodeid_t dirman_nodeid(dirman_hexid_c_str);
        extreme_debug_log << "Dirman node ID " << dirman_nodeid.GetHex() << " " << dirman_nodeid.GetIP() << " port " << dirman_nodeid.GetPort() << endl;
        net::peer_ptr_t peer;
        dirman.name_and_node.node = dirman_nodeid;
        extreme_debug_log << "about to connect peer to node" << endl;
        rc = net::Connect(&dirman.peer_ptr, dirman.name_and_node.node);
        assert(rc==RC_OK && "could not connect");
        extreme_debug_log << "just connected" << endl;
        //-------------------------------------------
        extreme_debug_log << "app name is " << dir_path << endl;
        ok = dirman::GetRemoteDirectoryInfo(gutties::ResourceURL(dir_path), &dir);
        assert(ok && "Could not get info about the directory?");
        extreme_debug_log << "just got directory info" << endl;

        while( dir.children.size() < num_servers) {
            debug_log << "dir.children.size() < num_servers. dir.children.size(): "  << dir.children.size() << " num_servers: " << num_servers << endl;

            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            ok = dirman::GetRemoteDirectoryInfo(gutties::ResourceURL(dir_path), &dir);
            extreme_debug_log << "and now dir.children.size() < num_servers. dir.children.size(): "  << dir.children.size() << " num_servers: " << num_servers << endl;

            assert(ok && "Could not get info about the directory?");
            extreme_debug_log << "just got directory info" << endl;
            // error_log <<"Error. There are not any servers initialized \n";
            // return RC_DIRMAN_ERR;
        }
        debug_log << "dir init is done" << endl;
        // uint16_t num_full_inserts = num_client_procs / num_server_procs;
        // uint16_t remainder = num_client_procs % num_server_procs;
        // for(int i=0; i<num_full_inserts; i++) {
        //     servers_for_scatter.insert( servers_for_scatter.end(), dir.children.begin(), dir.children.end() );
        // }
        // servers_for_scatter.insert( servers_for_scatter.end(), dir.children.begin(), dir.children.begin()+remainder );

        // for(int i=0; i< dir.children.size(); i++) {
        //     cout << "server " << i << " in dir has hexid " << dir.children.at(i).node.GetHex() << endl;
        // }

        // for(int i=0; i< servers_for_scatter.size(); i++) {
        //     cout << "server " << i << " in vector has hexid " << servers_for_scatter.at(i).node.GetHex() << endl;
        // }
        server_nodes = dir.children;
    }

    MPI_Bcast(&server_nodes[0], num_servers*sizeof(gutties::name_and_node_t), MPI_BYTE, 0, MPI_COMM_WORLD);
    for (int i=0; i<num_servers; i++) {
        cout << "server " << i << " in ary has hexid " << server_nodes[i].node.GetHex() << endl;
    }

    // server_nodes = temp(server_ary, server_ary +(int)num_servers);
    extreme_debug_log << "done initing dirman" << endl;
    return RC_OK;
 }
 

 static void setup_server(metadata_server &server, int server_indx, int rank, uint32_t num_servers, const vector<gutties::name_and_node_t> &server_nodes) {
    server_indx = rank % num_servers; 
    server.name_and_node = server_nodes[server_indx];
    server.URL = server.name_and_node.node.GetHex();
    net::Connect(&server.peer_ptr, server.name_and_node.node);

    extreme_debug_log << "dir.children.size = " << to_string(num_servers) << endl;
}



static void setup_servers(std::vector<metadata_server> &servers, uint32_t num_servers, const vector<gutties::name_and_node_t> &server_nodes) {
    debug_log << "num_servers: " << to_string(num_servers) << endl;

    for(int i=0; i < num_servers; i++) {
        metadata_server server;
        server.name_and_node = server_nodes[i];
        net::Connect(&server.peer_ptr, server.name_and_node.node);
        server.URL = server.name_and_node.node.GetHex();
        extreme_debug_log << "server.URL: " << server.URL << endl;
        servers.push_back(server);
    }

    debug_log << "servers.size: " << to_string(servers.size()) << endl;
    
}

     




 




