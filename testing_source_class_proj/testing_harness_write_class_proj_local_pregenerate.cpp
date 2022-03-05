#include <stdio.h> //needed for printf
#include <stdlib.h> //needed for atoi/atol/malloc/free/rand
#include <assert.h> //needed for assert
#include <string.h> //needed for strcmp

// #include <ctime>
#include <stdint.h> //needed for uint
// #include <ratio>
#include <chrono> //needed for high_resolution_clock
#include <math.h> //needed for pow()
#include <sys/time.h> //needed for timeval
#include <sys/stat.h> //needed for stat
#include <fstream> //needed for ifstream
#include <vector>
#include <float.h> //needed for DBL_MAX
#include <numeric>

#include <mpi.h>

#include <my_metadata_client_local.hh>

#include <sstream>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

// #include <hdf5_helper_functions_write.hh>#include <testing_harness_debug_helper_functions.hh>
// #include <md_client_timing_constants.hh>
#include <client_timing_constants_write_class_proj.hh>
#include <server_timing_constants_new.hh>

#include <testing_harness_class_proj_local.hh>
#include <md_client_timing_constants.hh>



using namespace std;

// static bool extreme_debug_logging = false;
// static bool debug_logging = false;
// static bool testing_logging = false;
// static bool zero_rank_logging = false;
static bool error_logging = true;

debugLog error_log = debugLog(error_logging, false);
// debugLog debug_log = debugLog(debug_logging, zero_rank_logging);
// debugLog extreme_debug_log = debugLog(extreme_debug_logging, false);
// debugLog testing_log = debugLog(testing_logging, zero_rank_logging);

bool output_timing = true;
static bool insert_by_batch = true;
// static bool gathered_writes = true;
// static bool gathered_writes;

std::vector<int> catg_of_time_pts;
std::vector<long double> time_pts;
std::vector<uint64_t> db_checkpoint_sizes;

int zero_time_sec;

void add_db_output_timing_point(int catg, uint64_t size) {
    if (output_timing) {
        struct timeval now;
        gettimeofday(&now, NULL);
        time_pts.push_back( (now.tv_sec - zero_time_sec) + now.tv_usec / 1000000.0);
        catg_of_time_pts.push_back(catg);
        db_checkpoint_sizes.push_back(size);
    }
}

void add_timing_point(int catg) {
    if (output_timing) {
        struct timeval now;
        gettimeofday(&now, NULL);
        time_pts.push_back( (now.tv_sec - zero_time_sec) + now.tv_usec / 1000000.0);
        catg_of_time_pts.push_back(catg);
    }
}

static bool is_db_output_start(uint16_t code)
{
    return ( (DB_COMPACT_DONE < code) && (code < MD_ACTIVATE_RUN_START) && (code%2 == 0) );
}

double clear_cache(uint32_t ndx, uint32_t ndy, uint32_t ndz, uint32_t num_vars);

/*
argv[1] = number of number of write processes in the x dimension
argv[2] = number of number of write processes in the y dimension
argv[3] = number of number of writ eprocesses in the z dimension
argv[4] = total length in the x dimension 
argv[5] = total length in the y dimension 
argv[6] = total length in the z dimension 
argv[7] = number of timesteps
argv[8] = estimate of the number of testing timing points we will have 
argv[9] = the job id (from the scheduler) 
argv[10] = number of number of write processes in the x dimension
argv[11] = number of number of write processes in the y dimension
argv[12] = number of number of writ eprocesses in the z dimension
argv[13] = number of timesteps per checkpt (of the DB to disk)
argv[14] = write type (in memory, on disk)
argv[15] = checkpt type (copy, incremental, copy and delete, copy and reset, incremental and delete, incremental and reset)
*/
int main(int argc, char **argv) {
    int rc;
    // char name[100];
    // gethostname(name, sizeof(name));
    // //extreme_debug_log << name << endl;

    if (argc != 15) { 
        error_log << "Error. Program should be given 14 arguments. npx, npy, npz, nx, ny, nz,"
                  << " number of timesteps, estm num time_pts, job_id, timesteps_per_checkpt, server_type, index_type, checkpt_type, do_read. " << argc-1 << " were given." << endl;
        cout << (int)ERR_ARGC << " " << 0 <<  endl;
        return RC_ERR;
    }
    uint32_t num_x_procs = stoul(argv[1],nullptr,0);
    uint32_t num_y_procs = stoul(argv[2],nullptr,0);
    uint32_t num_z_procs = stoul(argv[3],nullptr,0);
    uint64_t total_x_length = stoull(argv[4],nullptr,0);
    uint64_t total_y_length = stoull(argv[5],nullptr,0);
    uint64_t total_z_length = stoull(argv[6],nullptr,0);
    uint32_t num_timesteps = stoul(argv[7],nullptr,0);
    uint32_t estm_num_time_pts = stoul(argv[8],nullptr,0); 
    uint64_t job_id = stoull(argv[9],nullptr,0);
    uint32_t num_timesteps_per_checkpt = stoul(argv[10],nullptr,0);
    md_server_type server_type = (md_server_type)stoul(argv[11],nullptr,0);
    md_db_index_type index_type = (md_db_index_type)stoul(argv[12],nullptr,0);
    md_db_checkpoint_type checkpt_type = (md_db_checkpoint_type)stoul(argv[13],nullptr,0);
    bool do_read = stoul(argv[14],nullptr,0);

    catg_of_time_pts.reserve(estm_num_time_pts);
    time_pts.reserve(estm_num_time_pts);
    struct timeval now;
    gettimeofday(&now, NULL);
    zero_time_sec = 86400 * (now.tv_sec / 86400);

    add_timing_point(PROGRAM_START);

    MPI_Init(&argc, &argv);

    int rank;
    int num_client_procs;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size( MPI_COMM_WORLD, &num_client_procs);

    add_timing_point(MPI_INIT_DONE);

    //extreme_debug_log.set_rank(rank);
    //debug_log.set_rank(rank);
    error_log.set_rank(rank);

    rc = metadata_init_write(rank, job_id, server_type, index_type, checkpt_type);
    if (rc != RC_OK) {
        error_log << "Error initing the md_client. Exiting" << endl;
        return RC_ERR;
    }

    add_timing_point(METADATA_CLIENT_INIT_DONE);

    uint32_t num_vars = 10;

    int db_reset = false;

    md_catalog_run_entry run;
    vector<md_catalog_timestep_entry> timesteps;
    vector<md_catalog_type_entry> types;
    vector<vector<md_catalog_var_entry>> vars;
    vector<vector<md_catalog_var_attribute_entry>> all_var_attrs;
    vector<vector<md_catalog_timestep_attribute_entry>> all_timestep_attrs;
    vector<md_catalog_run_attribute_entry> run_attrs;


//-------------------------------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------------------------------    
    //all procs read since all procs have part of the md
    // int num_read_client_procs = num_read_x_procs * num_read_y_procs * num_read_z_procs;
    int read_color;

    // MPI_Comm read_comm;

    uint64_t txn_id = -1;

    vector<md_dim_bounds> read_proc_dims(num_dims);
    const int num_timesteps_to_fetch = 6;

    //makes sure that, by the time we're ready to read md and checkpt the db, the timesteps will have been written
    uint64_t timestep_ids[num_timesteps_to_fetch] = {0, 1%num_timesteps_per_checkpt, 2%num_timesteps_per_checkpt,
                        3%num_timesteps_per_checkpt, 4%num_timesteps_per_checkpt, 5%num_timesteps_per_checkpt};

    md_catalog_run_entry run;

    vector<vector<md_catalog_var_entry>> all_var_entries;   

    std::vector<md_catalog_var_entry> vars_to_fetch_pattern2;
    std::vector<md_catalog_var_entry> vars_to_fetch_pattern3;
    std::vector<md_catalog_var_entry> vars_to_fetch_pattern4;
    std::vector<md_catalog_var_entry> vars_to_fetch_pattern5;
    std::vector<md_catalog_var_entry> vars_to_fetch_pattern6;

    // std::vector<string> var_names_to_fetch(num_vars_to_fetch);
    // std::vector<uint32_t> var_vers_to_fetch(num_vars_to_fetch);
    int plane_x_procs;
    int plane_y_procs;
    // add_timing_point(INIT_READ_VARS_DONE);

    // read_color = MPI_UNDEFINED;
    // if(rank < num_read_client_procs) {
    //  read_color = 0;
    // }
    // //make a comm with just the "read" procs
    // MPI_Comm_split(MPI_COMM_WORLD, read_color, rank, &read_comm);

    add_timing_point(GENERATE_START);

    rc = generate_all_metadata(rank, num_client_procs, write_type, total_x_length, total_y_length, total_z_length,
        num_x_procs, num_y_procs, num_z_procs, num_timesteps, job_id,
        run, timesteps, types, vars, all_var_attrs, all_timestep_attrs, run_attrs
    );
    if(rc != RC_OK) {
        goto cleanup;
    }

    add_timing_point(GENERATE_DONE);


//-------------------------------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------------------------------    


    add_timing_point(INIT_VARS_DONE);

    MPI_Barrier(MPI_COMM_WORLD);

    //WRITE PROCESS
    add_timing_point(WRITING_START);


    rc = create_and_activate_run(run);
    if (rc != RC_OK) {
        goto cleanup;
    }

    //reminder - unless you adjust create_type to ask for a type id, if you want the types in the given order (so you know how)
    //type_id corresponds to name/version, you need to insert them in order
    rc = create_and_activate_types(run.run_id, types);
    if (rc != RC_OK) {
        goto cleanup;
    }

    // add_timing_point(CREATE_TIMESTEPS_START);
    add_timing_point(CREATE_TIMESTEPS_START);


    for(uint32_t timestep_num=0; timestep_num < num_timesteps; timestep_num++) {
        double sum = clear_cache(x_length_per_proc, y_length_per_proc, z_length_per_proc);
    
        //debug_log << "rank: " << rank << " about to barrier before start of timestep " << timestep_num << endl;
        MPI_Barrier(MPI_COMM_WORLD); //want to measure last-first for writing each timestep
        add_timing_point(CREATE_NEW_TIMESTEP_START);

        double timestep_temp_max;
        double timestep_temp_min;

        temp_timestep.timestep_id = timestep_num;      
        temp_timestep.path = temp_run.name + "/" + to_string(temp_timestep.timestep_id); //todo - do timestep need to use sprintf for this?
        temp_timestep.txn_id = timestep_num; 

        rc = metadata_create_timestep (temp_timestep.timestep_id, temp_timestep.run_id, temp_timestep.path, temp_timestep.txn_id);
        if (rc != RC_OK) {
            error_log << "Error. Was unable to create the " << temp_timestep.timestep_id << "th timestep. Exiting" << endl;
            add_timing_point(ERR_CREATE_TIMESTEP);
            goto cleanup;
        }
//-------------------------------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------------------------------

        // add_timing_point(CREATE_VARS_AND_ATTRS_FOR_NEW_TIMESTEP_START); 

        rc = create_vars(rank, num_client_procs,
                total_x_length, total_y_length, total_z_length, num_vars, timestep_num);
        if (rc != RC_OK) {
            goto cleanup;
        }
        // srand(rank*10000+1+timestep_num);

        rc = create_var_attrs(rank, proc_dims_vctr, timestep_num,
                num_vars, num_types, timestep_temp_min, timestep_temp_max); //if not okay, just proceed


        // add_timing_point(CREATE_VARS_AND_ATTRS_FOR_NEW_TIMESTEP_DONE); 

//-------------------------------------------------------------------------------------------------------------------------------------------    
//-------------------------------------------------------------------------------------------------------------------------------------------        
        rc = create_timestep_attrs(rank, timestep_temp_min, timestep_temp_max, timestep_num,
                num_types, num_run_timestep_types, num_client_procs, all_timestep_temp_mins_for_all_procs,
                all_timestep_temp_maxes_for_all_procs); //if not okay, just proceed


        //reminder - need to barrier so that all instances have been submitted before activating

        add_timing_point(CREATE_TIMESTEP_DONE); 

        // MPI_Barrier(MPI_COMM_WORLD);
        //debug_log << "rank: " << rank << " about to barrier before activate timestep " << timestep_num << endl;

       rc = activate_timestep_components(timestep_num);
        if (rc != RC_OK) {
            goto cleanup;
        }          
        add_timing_point(CREATE_AND_ACTIVATE_TIMESTEP_DONE); 


//-------------------------------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------------------------------    

        if(timestep_num == num_timesteps-1) {
            add_timing_point(CREATE_ALL_TIMESTEPS_DONE); 

            // if(rank == 0) {
               //  for (int i = 0; i<num_timesteps; i++) {
                        //debug_log << "timestep: " << i << " max: " << all_timestep_temp_maxes_for_all_procs[i] << endl;
                        //debug_log << "timestep: " << i << " min: " << all_timestep_temp_mins_for_all_procs[i] << endl;
            //     }  
            // }
            
            if (rank == 0 && num_run_timestep_types > 0) {
                rc = create_run_attrs(rank, all_timestep_temp_mins_for_all_procs, all_timestep_temp_maxes_for_all_procs,
                        run_id, num_timesteps, num_types);

                if(rc != RC_OK) {
                    goto cleanup;
                }
            }
            add_timing_point(WRITING_DONE);
        }

        if(timestep_num%10 == 0 && rank == 0) {
            cout << "timestep: " << timestep_num << endl;
        }
//-------------------------------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------------------------------    

        if( ((timestep_num+1) % num_timesteps_per_checkpt) == 0 ) {

            //initialize the reading vars before the first checkpt
            // if( timestep_num+1 == num_timesteps_per_checkpt && checkpt_type == DB_COPY && rank < num_read_client_procs ) {
            if( timestep_num+1 == num_timesteps_per_checkpt && do_read ) {
                rc = read_init(rank, num_client_procs, 
                    //num_read_x_procs, num_read_y_procs, num_read_z_procs,
                    num_x_procs, num_y_procs, num_z_procs, 
                    num_timesteps_to_fetch, txn_id, timestep_ids,
                    // read_comm, 
                    MPI_COMM_WORLD,
                    run, read_proc_dims, plane_x_procs, plane_y_procs,
                    all_var_entries, vars_to_fetch_pattern2, vars_to_fetch_pattern3, 
                    vars_to_fetch_pattern4, vars_to_fetch_pattern5, vars_to_fetch_pattern6
                );
                //debug_log << "rank: " << rank << " just finished read init " << endl;

            }
            //once the DB has exceeded RAM, the client no longer has access to the entire DB in mem so we stop doing reads
            // if(checkpt_type == DB_COPY && !db_reset && rank < num_read_client_procs ) {
            if(do_read && !db_reset ) {
                rc = do_reads(rank, 
                            num_x_procs, num_y_procs, num_z_procs,
                            //num_read_x_procs, num_read_y_procs, num_read_z_procs,
                            num_timesteps, num_client_procs, txn_id,
                            // read_comm, 
                            MPI_COMM_WORLD,
                            run, read_proc_dims, plane_x_procs, plane_y_procs, all_var_entries,
                            vars_to_fetch_pattern2, vars_to_fetch_pattern3, vars_to_fetch_pattern4, 
                            vars_to_fetch_pattern5, vars_to_fetch_pattern6
                        ); 
                    //debug_log << "rank: " << rank << " just finished do read " << endl;
            }

            // if(server_type != WRITE_ON_DISK) {
            if(server_type != SERVER_LOCAL_ON_DISK && server_type != SERVER_DEDICATED_ON_DISK) {
                if(rank == 0) {
                    cout << "timestep: " << timestep_num << endl;
                }
                //debug_log << "about to checkpoint the database" << endl;
                //debug_log << "time: " << time_pts.back() << endl;
                rc = metadata_checkpoint_database (rank, job_id);
                // rc = metadata_checkpoint_database (rank, job_id, checkpt_type, write_type);
                // rc = metadata_checkpoint_database (job_id, checkpt_type);
                if(rc == RC_DB_RESET) {
                    // if(!db_reset && rank < num_read_client_procs) {
                    if(!db_reset) {
                        int my_db_reset = true;
                        MPI_Allreduce(&my_db_reset, &db_reset, 1, MPI_INT, MPI_MAX, MPI_COMM_WORLD);
                    }
                    db_reset = true;
                    //debug_log << "db exceeded avail RAM so db is no longer whole" << endl;
                    rc = RC_OK;
                }
                else if(rc != RC_OK) {
                    error_log << "error doing a db checkpoint of type " << (int)checkpt_type << endl;
                }
                //debug_log << "done checkpointing the database" << endl;
            }
            //determine if any of the read procs no longer have a full DB. If so, stop performing reads
            // if(!db_reset && rank < num_read_client_procs) {
            if(!db_reset) {
                int my_db_reset = db_reset;
                MPI_Allreduce(&my_db_reset, &db_reset, 1, MPI_INT, MPI_MAX, MPI_COMM_WORLD);
            }
        }
    }



//-------------------------------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------------------------------        
cleanup:
    MPI_Barrier (MPI_COMM_WORLD);
    add_timing_point(WRITING_DONE_FOR_ALL_PROCS_START_CLEANUP);
    
    // metadata_finalize_client(rank, job_id, write_type, checkpt_type);
    metadata_finalize_client(rank, job_id);

    if(output_timing) {
        output_timing_stats(rank, num_client_procs);
    }

    MPI_Barrier (MPI_COMM_WORLD);
    MPI_Finalize();
    //debug_log << "got to cleanup7" << endl;

    return rc;

}

double clear_cache(uint32_t ndx, uint32_t ndy, uint32_t ndz, uint32_t num_vars)
{
    const int num_elems = ndx*ndy*ndz*num_vars;
    double *A = (double *)malloc(num_elems*sizeof(double));

    int sign = 1;
    for(int i = 0; i < num_elems; i++) {
        A[i] = 2*i*sign;
        sign *= -1;
    }

    double sum = 0;
    for(int i = 0; i < num_elems; i++) {
        sum += A[i];
    }
    free(A);
    return sum;
}



int create_and_activate_run(md_catalog_run_entry &run, const string &rank_to_dims_funct_name,
    const string &rank_to_dims_funct_path, const string &objector_funct_name, const string &objector_funct_path)
{
    int rc = RC_OK;

    add_timing_point(CREATE_NEW_RUN_START);

    rc = metadata_create_run  ( run.run_id, run.job_id, run.name, run.txn_id, run.npx, run.npy, run.npz,
                        rank_to_dims_funct_name, rank_to_dims_funct_path, objector_funct_name, objector_funct_path);
    if (rc != RC_OK) {
        error_log << "Error. Was unable to create the first run. Exitting" << endl;
        add_timing_point(ERR_CREATE_RUN);
        return rc;
    }

    rc = metadata_activate_run  ( run.txn_id);
    if (rc != RC_OK) {
        error_log << "Error. Was unable to activate the run. Exitting" << endl;
        add_timing_point(ERR_ACTIVATE_RUN);
        return rc;
    }

    add_timing_point(CREATE_NEW_RUN_DONE);

    return rc;
}

int create_and_activate_types(uint64_t run_id, uint32_t num_types, 
    uint32_t num_run_timestep_types) 
{
    int rc = RC_OK;
    md_catalog_type_entry temp_type;

    char type_names[10][13] = {"blob_freq", "blob_ifreq", "blob_rare", "max_val_type", "min_val_type", "note_freq", "note_ifreq", "note_rare", "ranges_type1", "ranges_type2"}; 
    char run_timestep_type_names[2][9] = {"max_temp", "min_temp"}; 

    add_timing_point(CREATE_TYPES_START);

    vector<md_catalog_type_entry> all_types_to_insert;
    all_types_to_insert.reserve(num_types + num_run_timestep_types);

    for(int type_indx=0; type_indx < (num_types + num_run_timestep_types); type_indx++) {
        // add_timing_point(CREATE_NEW_TYPE_START);

        uint64_t type_indx_adj = type_indx % num_types;
        bool is_run_type = (type_indx >= num_types);

        //extreme_debug_log << "type_indx: " << type_indx << endl;

        temp_type.run_id = run_id;
        if(is_run_type) {
            temp_type.name = run_timestep_type_names[type_indx_adj];
        }
        else {
            temp_type.name = type_names[type_indx_adj];
        }
        temp_type.version = 0; 
        temp_type.txn_id = run_id;
        // add_timing_point(TYPE_INIT_DONE);

        uint64_t temp_type_id;

        if(insert_by_batch) {
            all_types_to_insert.push_back(temp_type);
        }
        else {
            rc = metadata_create_type (temp_type_id, temp_type);
            if (rc != RC_OK) {
                add_timing_point(ERR_CREATE_TYPE);
                error_log << "Error. Was unable to insert the type of index " << type_indx << ". Proceeding" << endl;
                return rc;
            }
        }
    }

    if(insert_by_batch) {
        rc = metadata_create_type_batch (all_types_to_insert);
        if (rc != RC_OK) {
            add_timing_point(ERR_CREATE_TYPE_BATCH);
            error_log << "Error. Was unable to insert the types by batch. Proceeding" << endl;
            return rc;
        }
    }

    rc = metadata_activate_type  ( run_id);
    if (rc != RC_OK) {
        error_log << "Error. Was unable to activate the types. Exitting" << endl;
        add_timing_point(ERR_ACTIVATE_TYPE);
        return rc;
    }

    add_timing_point(CREATE_TYPES_DONE);

    return rc;
}

int create_vars(int rank, uint32_t num_client_procs, 
    uint64_t total_x_length, uint64_t total_y_length, uint64_t total_z_length, uint32_t num_vars,
    uint32_t timestep_num
    )
{
    add_timing_point(CREATE_VARS_START);

    int rc = RC_OK;

    char var_names[10][11] = {"temperat", "temperat", "pressure", "pressure", "velocity_x", "velocity_y", "velocity_z", "magn_field", "energy_var", "density_vr"};  
 
    uint32_t version1 = 1;
    uint32_t version2 = 2;

    vector<md_dim_bounds> var_dims(3);
    vector<md_catalog_var_entry> all_vars_to_insert;
    if(insert_by_batch) {
        all_vars_to_insert.reserve(num_vars);
    }

    for(uint32_t var_indx=0; var_indx < num_vars; var_indx++) {
        // add_timing_point(CREATE_VAR_AND_ATTRS_FOR_NEW_VAR_START);

        md_catalog_var_entry temp_var;

        temp_var.run_id = 1;
        temp_var.timestep_id = timestep_num;
        temp_var.data_size = 8; //double - 64 bit floating point
        temp_var.txn_id = timestep_num; 


        temp_var.name = var_names[var_indx];
        temp_var.path = "/"+ (string) temp_var.name;
        if(var_indx == 1 || var_indx == 3) { //these have been designated ver2
            temp_var.version = version2;
        }
        else {
            temp_var.version = version1;
        }
        temp_var.var_id = var_indx;
        temp_var.num_dims = 3;
        var_dims [0].min = 0;
        var_dims [0].max = total_x_length-1;
        var_dims [1].min = 0;
        var_dims [1].max = total_y_length-1;
        var_dims [2].min = 0;
        var_dims [2].max = total_z_length-1;
        // temp_var.dims = std::vector<md_dim_bounds>(var_dims, var_dims + temp_var.num_dims );
        temp_var.dims = var_dims;

        // add_timing_point(VAR_INIT_DONE);

        uint64_t temp_var_id;

        if(insert_by_batch) {
            //extreme_debug_log << "rank just pushed back var with id: " << temp_var.var_id << endl;
            all_vars_to_insert.push_back(temp_var);

        }
        else if (!insert_by_batch) {
      
            rc = metadata_create_var (temp_var_id, temp_var);
            if (rc != RC_OK) {
                error_log << "Error. Was unable to create the temp var. Exiting" << endl;
                error_log << "Failed var_id " << to_string(temp_var_id) << endl;
                add_timing_point(ERR_CREATE_VAR);
                return rc;
            }
        }

    } //var loop done

    if(insert_by_batch) { 
        rc = metadata_create_var_batch (all_vars_to_insert);
        if (rc != RC_OK) {
            error_log << "Error. Was unable to insert the batch vars. Proceeding" << endl;
            add_timing_point(ERR_CREATE_VAR_BATCH);
        }
    }

    add_timing_point(CREATE_VARS_DONE);

    return rc;
}

int create_var_attrs(int rank, const vector<md_dim_bounds> &proc_dims_vctr,
    uint32_t timestep_num, uint32_t num_vars, uint32_t num_types,
    double &timestep_temp_min, double &timestep_temp_max
    )
{
    int rc = RC_OK;

    add_timing_point(CREATE_VAR_ATTRS_START); 

    vector<md_catalog_var_attribute_entry> all_attrs_to_insert;
    if(insert_by_batch) {
        all_attrs_to_insert.reserve(3*num_vars);
    }

    //given in %
    float type_freqs[] = {25, 5, .1, 100, 100, 20, 2.5, .5, 1, 10};
    // char type_types[10][3] = {"b", "b", "b", "d", "d", "2p","2p","2p","2d","2d"};
    char type_types[10][3] = {"b", "b", "b", "d", "d", "s","s","s","2d","2d"};


    for(uint32_t var_indx=0; var_indx < num_vars; var_indx++) {

        // add_timing_point(CREATE_VAR_ATTRS_FOR_NEW_VAR_START); 

        for(uint32_t type_indx=0; type_indx<num_types; type_indx++) {
            // std::chrono::high_resolution_clock::time_point create_attr_start_time = chrono::high_resolution_clock::now();

            float freq = type_freqs[type_indx];
            uint32_t odds = 100 / freq;
            uint32_t val = rand() % odds; 
            ////extreme_debug_log << "rank: " << to_string(rank) << " and val: " << to_string(val) << " and odds: " << to_string(odds) << endl;
            if(val == 0) { //makes sure we have the desired frequency for each type
                // add_timing_point(CREATE_NEW_VAR_ATTR_START);

                uint64_t attr_id;
                md_catalog_var_attribute_entry temp_attr;
                temp_attr.timestep_id = timestep_num;
                temp_attr.type_id = type_indx+1; //note - if we change the id system this will change
                temp_attr.var_id = var_indx; //todo - are we okay with this?
                temp_attr.txn_id = timestep_num;
                temp_attr.num_dims = 3;
                // temp_attr.dims = std::vector<md_dim_bounds>(proc_dims_vctr, proc_dims_vctr + num_dims ); 
                temp_attr.dims = proc_dims_vctr; //todo - you wont want this to be the entire chunk for all attrs

                if(strcmp(type_types[type_indx], "b") == 0) {
                    bool flag = rank % 2;
                    // make_single_val_data(flag, temp_attr.data);
                    temp_attr.data = to_string(flag);
                    temp_attr.data_type = ATTR_DATA_TYPE_INT;                            
                }
                else if(strcmp(type_types[type_indx], "d") == 0) {
                    // int sign =  pow(-1, rand() % 2);
                    int sign = pow(-1,(type_indx+1)%2); //max will be positive, min will be neg
                    double val = (double)rand() / RAND_MAX;
                    // val = sign * (DBL_MIN + val * (DBL_MAX - DBL_MIN) );
                    //note - can use DBL MAX/MIN but the numbers end up being E305 to E308
                    // val = sign * (FLT_MIN + val * (FLT_MAX - FLT_MIN) );
                    //note - can use DBL MAX/MIN but the numbers end up being E35 to E38
                    val = sign * val * ( pow(10,10) ) ; //produces a number in the range [0,10^10]
                    // make_single_val_data(val, temp_attr.data);
                    temp_attr.data = to_string(val);
                    temp_attr.data_type = ATTR_DATA_TYPE_REAL;  
                    //if write data the chunk min and max for var 0 will be based on the actual data values (this is done above)
                    if(var_indx == 0 )  { 
                        if ( type_indx == 3 )  {
                            timestep_temp_max = val;
                            //extreme_debug_log << "for rank: " << rank << " timestep: " << timestep_num << " just added " << val << " as the max value \n";
                        }
                        else if (type_indx == 4 ) {
                            timestep_temp_min = val;
                            //extreme_debug_log << "for rank: " << rank << " timestep: " << timestep_num << " just added " << val << " as the min value \n";                                    
                        }
                    }                       
                }
                else if(strcmp(type_types[type_indx], "s") == 0) { 
                    temp_attr.data = "attr data for var" + to_string(var_indx) + "timestep"+to_string(timestep_num);
                    temp_attr.data_type = ATTR_DATA_TYPE_TEXT;

                }               
                else if(strcmp(type_types[type_indx], "2d") == 0) {
                    vector<int> vals(2);
                    vals[0] = (int) (rand() % RAND_MAX); 
                    vals[1] = vals[0] + 10000; 
                    make_single_val_data(vals, temp_attr.data);
                    temp_attr.data_type = ATTR_DATA_TYPE_BLOB;

                }
                else {
                    error_log << "error. type didn't match list of possibilities" << endl;
                    add_timing_point(ERR_TYPE_COMPARE);
                }
                // add_timing_point(VAR_ATTR_INIT_DONE);

                if(insert_by_batch) {
                    all_attrs_to_insert.push_back(temp_attr);
                    // if(all_attrs_to_insert.size() < 5) {
                    //     //extreme_debug_log << "for rank: " << rank << " the " << all_attrs_to_insert.size() << "th attr has val: " << temp_attr.data << endl;
                    // }
                }
                else {
                    // rc = metadata_insert_var_attribute_by_dims (attr_id, temp_attr);
                    rc = metadata_insert_var_attribute_by_dims (attr_id, temp_attr);
                    if (rc != RC_OK) {
                        error_log << "Error. Was unable to insert the attribute for type indx" << type_indx << "var indx " << var_indx <<". Proceeding" << endl;
                        add_timing_point(ERR_INSERT_VAR_ATTR);
                    }
                    //extreme_debug_log << "rank: " << rank << "attr.data: " << temp_attr.data << endl;
                }

               // add_timing_point(CREATE_NEW_VAR_ATTR_DONE);
            }//val = 0 done
        } //type loop done
        // add_timing_point(CREATE_VAR_ATTRS_FOR_NEW_VAR_DONE);
    }//var loop done

    if(insert_by_batch) {
        //debug_log << "about to insert all attrs of size: " << all_attrs_to_insert.size() << endl;
        rc = metadata_insert_var_attribute_by_dims_batch (all_attrs_to_insert);
        if (rc != RC_OK) {
            error_log << "Error. Was unable to insert the batch attributes. Proceeding" << endl;
            add_timing_point(ERR_INSERT_VAR_ATTR_BATCH);
        }
    }
    add_timing_point(CREATE_VAR_ATTRS_DONE); 

    return rc;
}


int create_timestep_attrs(int rank, double timestep_temp_min, double timestep_temp_max,
    uint32_t timestep_num,
    uint32_t num_types, uint32_t num_run_timestep_types, uint32_t num_client_procs,
    double *all_timestep_temp_mins_for_all_procs, double *all_timestep_temp_maxes_for_all_procs)
{
    int rc = RC_OK;
    add_timing_point(CREATE_TIMESTEP_ATTRS_START);

    if(rank == 0 && num_run_timestep_types > 0) {

        double all_temp_maxes[num_client_procs];
        double all_temp_mins[num_client_procs];

        MPI_Gather(&timestep_temp_max, 1, MPI_DOUBLE, all_temp_maxes, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD); 
        MPI_Gather(&timestep_temp_min, 1, MPI_DOUBLE, all_temp_mins, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);

        double timestep_var_max = -1 * RAND_MAX;
        double timestep_var_min = RAND_MAX;

        add_timing_point(GATHER_DONE_PROC_MAX_MIN_FIND_START);
        for (int proc_rank = 0; proc_rank<num_client_procs; proc_rank++) {

            if( all_temp_maxes[proc_rank] > timestep_var_max) {
                timestep_var_max = all_temp_maxes[proc_rank];
            }
            if( all_temp_mins[proc_rank] < timestep_var_min) {
                timestep_var_min = all_temp_mins[proc_rank];
            }                    
        }
        add_timing_point(PROC_MAX_MIN_FIND_DONE);

        all_timestep_temp_maxes_for_all_procs[timestep_num] = timestep_var_max;
        all_timestep_temp_mins_for_all_procs[timestep_num] = timestep_var_min;

        //debug_log << "timestep " << timestep_num << " temperature max: " << timestep_var_max << endl;
        //debug_log << "timestep " << timestep_num << " temperature min: " << timestep_var_min << endl;

        uint64_t temp_max_id;
        md_catalog_timestep_attribute_entry temp_max;
        temp_max.timestep_id = timestep_num;
        temp_max.type_id = num_types+1;
        temp_max.txn_id = timestep_num;
        temp_max.data = to_string(timestep_var_max);
        // make_single_val_data(timestep_var_max, temp_max.data);
        temp_max.data_type = ATTR_DATA_TYPE_REAL;

        uint64_t temp_min_id;
        md_catalog_timestep_attribute_entry temp_min;
        temp_min.timestep_id = timestep_num;
        temp_min.type_id = num_types+2;
        temp_min.txn_id = timestep_num;
        temp_min.data = to_string(timestep_var_min);
        // make_single_val_data(timestep_var_min, temp_min.data);
        temp_min.data_type = ATTR_DATA_TYPE_REAL;

        if(insert_by_batch) {
            vector<md_catalog_timestep_attribute_entry> all_timestep_attrs_to_insert = {temp_max, temp_min};

            rc = metadata_insert_timestep_attribute_batch (all_timestep_attrs_to_insert);
            if (rc != RC_OK) {
                error_log << "Error. Was unable to insert timestep attributes by batch. Proceeding" << endl;
                add_timing_point(ERR_INSERT_TIMESTEP_ATTR_BATCH);
            }
            //debug_log << "Rank " << rank << " just inserted all timestep attrs for timestep " << timestep_num << endl;
        }

        else {
            rc = metadata_insert_timestep_attribute (temp_max_id, temp_max);
            if (rc != RC_OK) {
                error_log << "Error. Was unable to insert the new timestep attribute. Proceeding" << endl;
                add_timing_point(ERR_INSERT_TIMESTEP_ATTR);
            }
            //debug_log << "New timestep attribute id is " << to_string(temp_max_id) << endl;

            rc = metadata_insert_timestep_attribute (temp_min_id, temp_min);
            if (rc != RC_OK) {
                error_log << "Error. Was unable to insert the new timestep attribute. Proceeding" << endl;
                add_timing_point(ERR_INSERT_TIMESTEP_ATTR);
            }
            //debug_log << "New timestep attribute id is " << to_string(temp_min_id) << endl;
        }
    }
    else if (num_run_timestep_types > 0) {
        MPI_Gather(&timestep_temp_max, 1, MPI_DOUBLE, NULL, NULL, MPI_DOUBLE, 0, MPI_COMM_WORLD); 
        MPI_Gather(&timestep_temp_min, 1, MPI_DOUBLE, NULL, NULL, MPI_DOUBLE, 0, MPI_COMM_WORLD); 
    }

    add_timing_point(CREATE_TIMESTEP_ATTRS_DONE);

    return rc;
}

int activate_timestep_components(uint32_t timestep_num)
{
    int rc; 

    rc = metadata_activate_timestep  ( timestep_num);
    if (rc != RC_OK) {
        error_log << "Error. Was unable to activate the " << timestep_num << "th timestep. Exitting" << endl;
        add_timing_point(ERR_ACTIVATE_TIMESTEP);
        return rc;
    }

    rc = metadata_activate_var ( timestep_num);
    if (rc != RC_OK) {
        error_log << "Error. Was unable to activate the vars for the " << timestep_num << "th timestep. Exitting" << endl;
        add_timing_point(ERR_ACTIVATE_VAR);
        return rc;
    }

    rc = metadata_activate_var_attribute (timestep_num);
    if (rc != RC_OK) {
        error_log << "Error. Was unable to activate the types. Exitting" << endl;
        add_timing_point(ERR_ACTIVATE_VAR_ATTR);
        return rc;
    }

    rc = metadata_activate_timestep_attribute  ( timestep_num);
    if (rc != RC_OK) {
        error_log << "Error. Was unable to activate the timestep attrs. Exitting" << endl;
        add_timing_point(ERR_ACTIVATE_TIMESTEP_ATTR);
        return rc;
    }

    return rc;
}

int create_run_attrs(int rank, double *all_timestep_temp_mins_for_all_procs, 
    double *all_timestep_temp_maxes_for_all_procs, uint64_t run_id, uint32_t num_timesteps, uint32_t num_types
    )
{
    int rc = RC_OK;

    add_timing_point(CREATE_RUN_ATTRS_START);

    double run_temp_max = -1 * RAND_MAX;
    double run_temp_min = RAND_MAX;

    add_timing_point(TIMESTEP_MAX_MIN_FIND_START);

    for(int timestep = 0; timestep < num_timesteps; timestep++) {
        if( all_timestep_temp_maxes_for_all_procs[timestep] > run_temp_max) {
            run_temp_max = all_timestep_temp_maxes_for_all_procs[timestep];
        }
        if( all_timestep_temp_mins_for_all_procs[timestep] < run_temp_min) {
            run_temp_min = all_timestep_temp_mins_for_all_procs[timestep];
        }    
    }
    add_timing_point(TIMESTEP_MAX_MIN_FIND_DONE);

    uint64_t temp_max_id;
    md_catalog_run_attribute_entry temp_max;
    temp_max.run_id = run_id;
    temp_max.type_id = num_types+1;
    temp_max.txn_id = run_id;
    temp_max.data = to_string(run_temp_max);
    // make_single_val_data(run_temp_max, temp_max.data);
    //debug_log << "run temperature max: " << run_temp_max << endl;
    temp_max.data_type = ATTR_DATA_TYPE_REAL;

    uint64_t temp_min_id;
    md_catalog_run_attribute_entry temp_min;
    temp_min.run_id = run_id;
    temp_min.type_id = num_types+2;
    temp_min.txn_id = run_id;
    temp_min.data = to_string(run_temp_min);
    // make_single_val_data(run_temp_min, temp_min.data);
    //debug_log << "run temperature min: " << run_temp_min << endl;
    temp_min.data_type = ATTR_DATA_TYPE_REAL;

    if(insert_by_batch) {
        vector<md_catalog_run_attribute_entry> all_run_attributes_to_insert = {temp_max, temp_min};

        rc = metadata_insert_run_attribute_batch (all_run_attributes_to_insert);
        if (rc != RC_OK) {
            error_log << "Error. Was unable to insert the new run attributes by batch. Proceeding" << endl;
            add_timing_point(ERR_INSERT_RUN_ATTR_BATCH);
        }
    }
    else {
        rc = metadata_insert_run_attribute (temp_max_id, temp_max);
        if (rc != RC_OK) {
            error_log << "Error. Was unable to insert the new run attribute. Proceeding" << endl;
            add_timing_point(ERR_INSERT_RUN_ATTR);
        }
        //debug_log << "New run attribute id is " << to_string(temp_max_id) << endl;

        rc = metadata_insert_run_attribute (temp_min_id, temp_min);
        if (rc != RC_OK) {
            error_log << "Error. Was unable to insert the new run attribute. Proceeding" << endl;
            add_timing_point(ERR_INSERT_RUN_ATTR);
        }
        //debug_log << "New run attribute id is " << to_string(temp_min_id) << endl;
    }
    rc = metadata_activate_run_attribute  ( run_id);
    if (rc != RC_OK) {
        error_log << "Error. Was unable to activate the run attrs. Exitting" << endl;
        add_timing_point(ERR_ACTIVATE_RUN_ATTR);
        return rc;
    }

    add_timing_point(CREATE_RUN_ATTRS_DONE);

    return rc;
}


void output_timing_stats(int rank, uint32_t num_client_procs )
{
    long double *all_time_pts_buf;

    int each_proc_num_time_pts[num_client_procs];
    int displacement_for_each_proc[num_client_procs];
    int *all_catg_time_pts_buf;

    uint64_t *db_checkpoint_sizes_ary;
    uint64_t *all_db_checkpoint_sizes_buf;
    if(db_checkpoint_sizes.size() > 0) {
        //debug_log << "rank: " << rank << " my actual db size: " << db_checkpoint_sizes[0] << endl;
        db_checkpoint_sizes_ary = &db_checkpoint_sizes[0];
    }

    gatherv_int(catg_of_time_pts, num_client_procs, rank, each_proc_num_time_pts,
        displacement_for_each_proc, &all_catg_time_pts_buf);

    int sum = 0;
    if(rank == 0) {
        sum = accumulate(each_proc_num_time_pts, each_proc_num_time_pts+num_client_procs, sum);
        //extreme_debug_log << "sum: " << sum << endl;
        all_time_pts_buf = (long double *) malloc(sum * sizeof(long double));
        all_db_checkpoint_sizes_buf = (uint64_t *) malloc(num_client_procs * db_checkpoint_sizes.size() * sizeof(uint64_t));

    }
 
    int num_time_pts = time_pts.size();
    //debug_log << "num_time pts: " << to_string(num_time_pts)  << "time_pts.size(): " << time_pts.size()<< endl;

    MPI_Gatherv(&time_pts[0], num_time_pts, MPI_LONG_DOUBLE, all_time_pts_buf, each_proc_num_time_pts, displacement_for_each_proc, MPI_LONG_DOUBLE, 0, MPI_COMM_WORLD); 
    
    MPI_Gather(db_checkpoint_sizes_ary, db_checkpoint_sizes.size(), MPI_UINT64_T, all_db_checkpoint_sizes_buf, db_checkpoint_sizes.size(), MPI_UINT64_T, 0, MPI_COMM_WORLD); 

    // int db_checkpt_indx = 0;
    if (rank == 0) {
        //prevent it from buffering the printf statements
        setbuf(stdout, NULL);
        std::cout << "begin timing output" << endl;

        int checkpt_indx = 0;
        for(int i=0; i<sum; i++) {
            // if(all_catg_time_pts_buf[i] == MD_CHECKPOINT_DATABASE_START) {
         //        printf("%d %Lf %d ", all_catg_time_pts_buf[i], all_time_pts_buf[i], all_db_checkpt_types[db_checkpt_indx]);
         //        db_checkpt_indx += 1;
            // }
            // else {
            if( is_db_output_start(all_catg_time_pts_buf[i])) {
                printf("%d %Lf %llu ", all_catg_time_pts_buf[i], all_time_pts_buf[i], all_db_checkpoint_sizes_buf[checkpt_indx]);
                //only increment for the incremental since one always follows the original output
                checkpt_indx += 1;
            }
            else {
                printf("%d %Lf ", all_catg_time_pts_buf[i], all_time_pts_buf[i]);
            }
            // }
            // std::cout << all_catg_time_pts_buf[i] << " " << all_time_pts_buf[i] << " ";
            if (i%20 == 0 && i!=0) {
                std::cout << std::endl;
            }
        }
        std::cout << std::endl;

        // // free(all_clock_times);
        free(all_time_pts_buf);
        free(all_catg_time_pts_buf);
        free(all_db_checkpoint_sizes_buf);

    }
}


template <class T>
static void make_single_val_data (T val, string &serial_str) {
    //extreme_debug_log << "about to make single val data \n";
    stringstream ss;
    boost::archive::text_oarchive oa(ss);
    oa << val;
    serial_str = ss.str();
}

template <class T1, class T2>
static void make_double_val_data (T1 val_0, T2 val_1, string &serial_str) {
    //extreme_debug_log << "about to make double val data \n";
    stringstream ss;
    boost::archive::text_oarchive oa(ss);
    oa << val_0;
    oa << val_1;
    serial_str = ss.str();
}



void gatherv_int(vector<int> values, uint32_t num_client_procs, int rank, int *each_proc_num_values,
            int *displacement_for_each_proc, int **all_values)
{   
    int num_values = values.size();
    MPI_Gather(&num_values, 1, MPI_INT, each_proc_num_values, 1, MPI_INT, 0, MPI_COMM_WORLD);

    int sum = 0;
    if (rank == 0 ) {
        for(int i=0; i<num_client_procs; i++) {
            displacement_for_each_proc[i] = sum;
            sum += each_proc_num_values[i];
            if(each_proc_num_values[i] != 0 && rank == 0) {
                //extreme_debug_log << "rank " << i << " has a string of length " << each_proc_num_values[i] << endl;
            }
        }
        *all_values = (int *) malloc(sum * sizeof(int));

        //extreme_debug_log << "sum: " << sum << endl;
    }

    // //extreme_debug_log << "num_bytes is "<< to_string(length_ser_c_str) + " and rank: " << to_string(rank) << endl;
    
    // //extreme_debug_log << "rank " << rank << " about to allgatherv" << endl;

    MPI_Gatherv(&values[0], num_values, MPI_INT,
           *all_values, each_proc_num_values, displacement_for_each_proc,
           MPI_INT, 0, MPI_COMM_WORLD);

}



//asssumes the rank is 0
template <class T>
void combine_ser( int num_client_procs, int *each_proc_ser_values_size, int *displacement_for_each_proc, 
        char *serialized_c_str_all_ser_values, const vector<T> &my_vals, vector<T> &all_vals)
{

// //extreme_debug_log << "rank " << rank << " done with allgatherv" << endl;
    // //extreme_debug_log << "entire set of attr vectors: " << serialized_c_str_all_ser_values << " and is of length: " << strlen(serialized_c_str_all_ser_values) << endl;

    for(int i = 0; i < num_client_procs; i++) {
        int offset = displacement_for_each_proc[i];
        int count = each_proc_ser_values_size[i];
        if(count > 0) {
            vector<T> rec_values;

            //0 rank does not need to deserialize its own attrs
            if(i != 0) {

                //extreme_debug_log << "rank " << i << " count: " << count << " offset: " << offset << endl;
                char serialzed_vals_for_one_proc[count];

                memcpy ( serialzed_vals_for_one_proc, serialized_c_str_all_ser_values + offset, count);

                //extreme_debug_log << "rank " << i << " serialized_c_str: " << (string)serialzed_vals_for_one_proc << endl;
                //extreme_debug_log <<    " serialized_c_str length: " << strlen(serialzed_vals_for_one_proc) << endl;
                //extreme_debug_log << " count: " << count << endl;
                stringstream ss1;
                ss1.write(serialzed_vals_for_one_proc, count);
                //extreme_debug_log << "about to deserialize ss: " << ss1.str() << endl;
                boost::archive::text_iarchive ia(ss1);
                ia >> rec_values;
            }
            else { //i == rank
                rec_values = my_vals;
            }
            all_vals.insert( all_vals.end(), rec_values.begin(), rec_values.end() );
        }
    }
    free(serialized_c_str_all_ser_values);
}


template <class T>
void gatherv_ser_and_combine(const vector<T> &values, uint32_t num_client_procs, int rank, MPI_Comm comm,
    vector<T> &all_vals
    )
{
    
    //extreme_debug_log << "starting gatherv_ser_and_combine" << endl;   

    int each_proc_ser_values_size[num_client_procs];
    int displacement_for_each_proc[num_client_procs];
    char *serialized_c_str_all_ser_values;

    int length_ser_c_str = 0;
    char *serialized_c_str;

    stringstream ss;
    boost::archive::text_oarchive oa(ss);
    oa << values;
    string serialized_str = ss.str();
    length_ser_c_str = serialized_str.size() + 1;
    serialized_c_str = (char *) malloc(length_ser_c_str);
    serialized_str.copy(serialized_c_str, serialized_str.size());
    serialized_c_str[serialized_str.size()]='\0';
    // //extreme_debug_log << "rank " << rank << " object_names ser string is of size " << length_ser_c_str << " serialized_str " << 
    //     serialized_str << endl;
    //extreme_debug_log << "rank " << rank << " ser string is of size " << length_ser_c_str << " str: " << serialized_c_str << endl; 

    // //extreme_debug_log << "rank " << rank << " about to allgather" << endl;

    //debug_log << "rank: " << rank << " about to gather attrs " << endl;
    MPI_Gather(&length_ser_c_str, 1, MPI_INT, each_proc_ser_values_size, 1, MPI_INT, 0, comm);

    int sum = 0;
    // int max_value = 0;
    if (rank == 0 ) {
        for(int i=0; i<num_client_procs; i++) {
            displacement_for_each_proc[i] = sum;
            sum += each_proc_ser_values_size[i];
            if(each_proc_ser_values_size[i] != 0 && rank == 0) {
                //extreme_debug_log << "rank " << i << " has a string of length " << each_proc_ser_values_size[i] << endl;
            }
            // if(displacement_for_each_proc[i] > max_value) {
            //     max_value = displacement_for_each_proc[i];
            // }
        }
        //extreme_debug_log << "sum: " << sum << endl;

        serialized_c_str_all_ser_values = (char *) malloc(sum);

    }

    //extreme_debug_log << "num_bytes is "<< to_string(length_ser_c_str) + " and rank: " << to_string(rank) << endl;
    
    //extreme_debug_log << "rank " << rank << " about to gatherv" << endl;

    MPI_Gatherv(serialized_c_str, length_ser_c_str, MPI_CHAR,
           serialized_c_str_all_ser_values, each_proc_ser_values_size, displacement_for_each_proc,
           MPI_CHAR, 0, comm);

    //extreme_debug_log << "just finished with gather v" << endl;

    free(serialized_c_str);

    if(rank == 0) {
        all_vals.reserve(3*num_client_procs);

        combine_ser( num_client_procs, each_proc_ser_values_size, displacement_for_each_proc, 
        serialized_c_str_all_ser_values, values, all_vals);
    }
}


template <class T>
void print_attr_data(T attr)
 {

    switch(attr.data_type) {
        case ATTR_DATA_TYPE_INT : {
            long long deserialized_test_int = stoll(attr.data);
            // uint64_t deserialized_test_int;

            // stringstream sso;
            // sso << attr.data;
            // boost::archive::text_iarchive ia(sso);
            // ia >> deserialized_test_int;

            cout << "data: int: " << deserialized_test_int << endl; 
            break;
        }
        case ATTR_DATA_TYPE_REAL : {
            // long double deserialized_test_real;
            long double deserialized_test_real = stold(attr.data);

            // stringstream sso;
            // sso << attr.data;
            // boost::archive::text_iarchive ia(sso);
            // ia >> deserialized_test_real;

            cout << "data: real: " << deserialized_test_real << endl; 
            break;
        }           
        case ATTR_DATA_TYPE_TEXT : {
                // string deserialized_test_string;
            cout << "data: text: " << attr.data << endl; 
            break;
        } 
        case ATTR_DATA_TYPE_BLOB : {
            vector<int> deserialized_vals(2);

            stringstream sso;
            sso << attr.data;
            boost::archive::text_iarchive ia(sso);
            ia >> deserialized_vals;
            cout << "data: blob: val1: " << deserialized_vals.at(0) << " val2: " << deserialized_vals.at(1) << endl; 
            break;
        }    
    }
        // //extreme_debug_log << "serialized var data: " << attr.data << endl;

        // testing_log << "data: string: " << deserialized_test_string << " int: " << deserialized_test_int << endl; 
 }

 void print_var_attr_data(md_catalog_var_attribute_entry attr) {
    print_attr_data(attr);
}
void print_timestep_attr_data(md_catalog_timestep_attribute_entry attr) {
    print_attr_data(attr);
}
void print_run_attr_data(md_catalog_run_attribute_entry attr) {
    print_attr_data(attr);
}
