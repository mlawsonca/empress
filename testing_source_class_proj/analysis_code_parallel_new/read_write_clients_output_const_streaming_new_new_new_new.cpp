
#include "../../include/client/md_client_timing_constants.hh"
#include "../../include/class_proj/client_timing_constants_write_class_proj.hh"
#include "../../include/server/server_timing_constants_new.hh" //needed for local clients (w. embedded server timing)
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <vector>
#include <queue>
#include <iomanip> //set precision
// #include <limits> //numeric_limits
#include <map>
#include "read_testing_output_const_streaming.hh"

// #define SECOND_END_CODE_NULL 65535

using namespace std;

extern uint16_t NUM_CLIENT_OPS;
extern uint16_t NUM_THREADS;
extern void add_last_first_timing(read_client_config_output &output);

extern int init(std::vector<uint16_t>::const_iterator &my_codes, std::vector<long double>::const_iterator &my_time_pts, uint16_t code, struct server_config_output &output, long double &start_time, long double &init_done, bool is_local_server);
extern int evaluate_storage_sizes(ifstream &file, struct server_config_output &output);
extern bool is_db_output(uint16_t code);
extern int shutdown(std::vector<uint16_t>::const_iterator &my_codes, std::vector<long double>::const_iterator &my_time_pts, std::vector<uint16_t>::const_iterator my_end, uint16_t code, struct server_config_output &output, uint32_t checkpt_count,
    long double &start_time, long double &shutdown_done, bool is_local);
extern bool is_shutdown(uint16_t code);
// extern void add_earliest_start(uint16_t code, long double start_time, read_client_config_output &output);
// extern void add_latest_finish(uint16_t code, long double end_time, read_client_config_output &output);
extern void init_client_config_output(client_config_output &output);
extern bool is_local_run(uint32_t server_type);
extern bool is_local_run(client_config_output output);

static bool extreme_debug_logging = false;
static bool debug_logging = false;
static bool error_logging = true;

static debugLog error_log = debugLog(error_logging);
static debugLog debug_log = debugLog(debug_logging);
static debugLog extreme_debug_log = debugLog(extreme_debug_logging);

void read(std::vector<uint16_t>::const_iterator &my_codes, std::vector<long double>::const_iterator &my_time_pts, uint16_t init_code, double start_time, int rank, read_client_config_output &output);


static void ops(std::vector<uint16_t>::const_iterator &my_codes, std::vector<long double>::const_iterator &my_time_pts, uint16_t code, long double start_time, int rank,
    struct client_config_output &output, long double &end_time, long double &total_time, uint32_t &checkpt_count, vector<queue<double>> &pending_rdma_ops);

static void evaluate_pts_of_interest(const vector<pt_of_interest> &all_points_of_interest,
    vector<pt_of_interest> &all_last_first_points_of_interest, struct client_config_output &output, int global_rank);
static void evaluate_last_first_pts_of_interest(const vector<pt_of_interest> &all_last_first_points_of_interest, 
    struct client_config_output &output);
static vector<pt_of_interest> add_all_points_of_interest(uint32_t num_write_client_procs, uint32_t num_checkpts, uint32_t write_type, uint32_t server_type);
static vector<pt_of_interest> add_all_last_first_points_of_interest();

static int local_server_storage(ifstream &file, struct client_config_output &output);
// static void db_output(ifstream &file, struct client_config_output &output, uint16_t db_output_start_code, long double output_start_time, uint32_t checkpt_count );
static void db_output(std::vector<uint16_t>::const_iterator &my_codes, std::vector<long double>::const_iterator &my_time_pts,
                std::vector<uint64_t>::const_iterator &my_storage_sizes,
                struct client_config_output &output, uint16_t db_output_start_code, 
                long double output_start_time, uint32_t checkpt_count,
                long double &end_time );
// static void add_local_result_to_global_results(struct client_config_output my_output, struct client_config_output &output
//     );
static void add_local_result_to_global_results(struct client_config_output &my_output, struct client_config_output &output
    );
// static void analyze_results(client_config_output &output, int rank, const vector<vector<uint16_t>> &my_codes_vct, 
//     const vector<vector<long double>> &my_time_pts_vct, const vector<vector<uint64_t>> &my_storage_sizes_vct,
//     vector<pt_of_interest> &all_last_first_points_of_interest
//     );
static void analyze_results(client_config_output &output, int proc_chunk_size, const vector<uint16_t> &my_codes_vct, 
    const vector<long double> &my_time_pts_vct, const vector<uint64_t> &my_storage_sizes_vct,
    vector<pt_of_interest> &all_last_first_points_of_interest, vector<queue<double>> &pending_rdma_ops, int global_rank
    );
static void process_file(ifstream &file, const client_config_output &output, int proc_chunk_size,
    vector<uint16_t> &my_codes_vct, vector<long double> &my_time_pts_vct,
    vector<uint64_t> &my_storage_sizes_vct
    );

static void add_checkpoint_last_first(long double start_time, long double end_time, 
                                uint32_t checkpt_count, int rank, vector<pt_of_interest> &all_points_of_interest);

static const uint16_t FIRST_ERR_CODE = ERR_ARGC;
static const uint16_t FIRST_OP_CODE = MD_ACTIVATE_RUN_START;
// static const uint16_t LAST_TIMING_CODE = CREATE_RUN_ATTRS_DONE; 
static const uint16_t LAST_TIMING_CODE = INSERT_VAR_ATTRS_COLLECTIVE_DONE; 
// static const uint16_t LAST_OP_CODE = MD_FULL_SHUTDOWN_DONE;
static const uint16_t LAST_OP_CODE = MD_COMMIT_TRANSACTION_START;

// static const uint16_t FIRST_CODE_AFTER_OPS = BOUNDING_BOX_TO_OBJ_NAMES_START;

// static const uint16_t LAST_TIMING_CODE = WRITING_DONE_FOR_ALL_PROCS_START_CLEANUP;

// static const uint16_t FIRST_READ_CODE = 50;

static bool is_read(uint16_t code) {
    return (code == 50 || code == 60); //read init or read start
}

static bool is_error(uint16_t code) {
    return code >= FIRST_ERR_CODE;
}

static bool is_op(uint16_t code) {
    return (FIRST_OP_CODE <= code && code <= LAST_OP_CODE);
}

static bool is_transaction_op(uint16_t code) {
    return (MD_ABORT_TRANSACTION_START <= code && code <= MD_COMMIT_TRANSACTION_DONE);
}

static bool is_testing_harness(uint16_t code) {
    // return ( ( PROGRAM_START <= code && code <= LAST_TIMING_CODE) || is_obj_name(code) );
    return ( ( PROGRAM_START <= code && code <= LAST_TIMING_CODE) );
}


// int evaluate_all_write_clients(const string &file_path, struct client_config_output &output) {
int evaluate_all_write_clients(const string &file_path, struct client_config_output &output) {

    int rc = RC_OK;
    ifstream file;
    // int last_rank = 0;
    int id_reading = 0;

    uint32_t num_checkpts = output.config.num_checkpts;
    vector<pt_of_interest> all_last_first_points_of_interest = add_all_points_of_interest(1,num_checkpts,output.config.write_type,output.config.server_type);

    int proc_chunk_size = 50;

    std::string line;

    cout << "filename: " << file_path << endl;

    file.open(file_path);
    if(!file) {
        error_log << "error. failed to open file " << file_path << endl;
        return RC_ERR;
    }

    if (is_local_run(output)) {
        rc = local_server_storage(file, output);
        if(rc != RC_OK) {
            goto cleanup;
        }
    }

    //igore the outputted objector params
    //don't start reading until the timing information begins
    while (std::getline(file, line))
    {
        if( line.find("begin timing output") != std::string::npos) {
            break;
        }
    }


    // omp_set_num_threads(70);
    // omp_set_num_threads(48);
    // omp_set_num_threads(36);
    omp_set_num_threads(NUM_THREADS);
    // omp_set_num_threads(1);

    cout << "num threads: " << NUM_THREADS << endl;
    // omp_set_num_threads(30);
    // omp_set_num_threads(27);
    // omp_set_num_threads(27);
    // omp_set_num_threads(24);
    // omp_set_num_threads(22);
    // omp_set_num_threads(18);
    // omp_set_num_threads(10);
    // omp_set_num_threads(5);
    // omp_set_num_threads(1);
    //firstprivate(my_codes_vct,my_time_pts_vct)

    #pragma omp parallel
    {
        struct client_config_output my_output;
        my_output.config = output.config;
        init_client_config_output(my_output);

        #pragma omp for schedule(dynamic)

        for(int rank = 0; rank < output.config.num_write_client_procs; rank += proc_chunk_size) {
            vector<uint16_t> my_codes_vct;
            vector<long double> my_time_pts_vct;
            vector<uint64_t> my_storage_sizes_vct;
            vector<queue<double>> pending_rdma_ops(NUM_CLIENT_OPS);

            #pragma omp critical
            {   

                //extreme_debug_log << "rank: " << rank << " is reading now" << endl;
                process_file(file, my_output, proc_chunk_size, my_codes_vct, my_time_pts_vct, my_storage_sizes_vct);
            }
            //extreme_debug_log << "rank: " << rank <<  " my_codes_vct.size(): " << my_codes_vct.size() << endl;
            analyze_results(my_output, proc_chunk_size, my_codes_vct, my_time_pts_vct, my_storage_sizes_vct,
                        all_last_first_points_of_interest, pending_rdma_ops, rank);

            for(int i = 0; i < pending_rdma_ops.size(); i++) {
                if(pending_rdma_ops[i].size() != 0) {
                    error_log << "error. in read client, pending_rdma_ops[" << i << "].size(): " <<  pending_rdma_ops[i].size() << endl;
                }
            }
        }
        add_local_result_to_global_results(my_output, output);
            debug_log << "have finished" << endl;
    }

    evaluate_last_first_pts_of_interest(all_last_first_points_of_interest, output);
    debug_log << "output.read_outputs.size(): " << output.read_outputs.size() << endl;
    for(int i = 0; i < output.read_outputs.size(); i++) {
        add_last_first_timing( output.read_outputs[i]);
    }


    extreme_debug_log << "finishing" << endl;

cleanup:                
    file.close();

    return rc;
}


static void analyze_results(client_config_output &output, int proc_chunk_size, const vector<uint16_t> &my_codes_vct, 
    const vector<long double> &my_time_pts_vct, const vector<uint64_t> &my_storage_sizes_vct,
    vector<pt_of_interest> &all_last_first_points_of_interest,
    vector<queue<double>> &pending_rdma_ops, int global_rank
    ) 
{
    int rc;

    long double init_done_time;
    long double start_time;
    long double op_time_since_last_testing_harness_pt;

    uint16_t code;
    long double time_pt;

    int timestep_num = -1;
    uint32_t checkpt_count = 0;

    int rank = -1;


    // debug_log << "starting with rank: " << rank << endl;

    extreme_debug_log << "my_codes_vct.size(): " << my_codes_vct.size() << endl;

    std::vector<uint16_t>::const_iterator my_codes = my_codes_vct.begin();
    std::vector<long double>::const_iterator my_time_pts = my_time_pts_vct.begin();
    std::vector<uint64_t>::const_iterator my_storage_sizes = my_storage_sizes_vct.begin();

    const std::vector<uint16_t>::const_iterator my_end = my_codes_vct.end();


    extreme_debug_log << "proc_chunk_size: " << proc_chunk_size << endl;
    vector<pt_of_interest> all_points_of_interest = add_all_points_of_interest(proc_chunk_size,output.config.num_checkpts,output.config.write_type,output.config.server_type);


    // while(my_codes != my_codes_vct[rank].end()) {
    while(my_codes != my_end) {
        code = *my_codes;
        time_pt = *my_time_pts;

        // extreme_debug_log << "code: " << code << " time_pt: " << time_pt << endl;

        extreme_debug_log << "for rank " << rank << " code at top of while loop: " << code << endl;
        extreme_debug_log << "indx: " << my_codes - my_codes_vct.begin() << endl;
        //extreme_debug_log << "checkpt_count: " << checkpt_count << endl;
        if( code ==  CREATE_NEW_TIMESTEP_START) {
            //extreme_debug_log << "code: " << code << " ==  CREATE_NEW_TIMESTEP_START" << endl;
            timestep_num += 1;
            if(timestep_num % output.config.num_timesteps_per_checkpt == 0 && timestep_num!= 0) {
                // extreme_debug_log << "timestep_num: " << timestep_num << " checkpt_count: " << checkpt_count << endl;
                checkpt_count += 1;
            }
        } 
        extreme_debug_log << "code at top of while loop: " << to_string(code) << endl;

        if(is_error(code)) {
            error_log << "error code found at top of while loop. code: " << code << endl;
            // return 1;
        } 
        else if(is_op(code)) {
            long double end_time;
            //extreme_debug_log << "op" << endl;
            extreme_debug_log  << "op" << endl;

            long double total_op_time;
            extreme_debug_log << "starting an op \n";
            // if(code == 1800) {
            //     extreme_debug_log << "am calling ops() for CHECKPOINT_DATABASE" << endl;
            // }
            ops(my_codes, my_time_pts, code, time_pt, rank, output, end_time, total_op_time, checkpt_count, pending_rdma_ops);
            extreme_debug_log << "just completed an op" << endl;
            extreme_debug_log << "code: " << code << " total_op_time: " << total_op_time << endl;
            op_time_since_last_testing_harness_pt += total_op_time;
            if(code == 1800) {
                add_checkpoint_last_first(time_pt, end_time, checkpt_count, rank, all_points_of_interest);
            }
            // debug_log << "am adding: " << total_op_time << " to op_time_since_last_testing_harness_pt" << endl;
            // debug_log << "now op_time_since_last_testing_harness_pt: " << op_time_since_last_testing_harness_pt << endl;
            // debug_log << "total_op_time: " << total_op_time << endl;
        }
        // else if(is_hdf5(code)) {
        //  long double start_pt = time_pt;
        //  uint16_t start_code = code;
        //  //file >> code;
        //  //file >> time_pt;
        //  if(code != start_code+1) {
        //      error_log << "error. expected code " << start_code + 1 << " to appear "
        //          << " after code " << start_code << " but instead " << code << " appeared" << endl;
        //  }
        //  else {
        //      hdf5_time_since_last_testing_harness_pt += (time_pt - start_pt);
        //  }
        // }
        //50 or 60
        else if(is_read(code)) {
            extreme_debug_log  << "about to read" << endl;
            extreme_debug_log << "read" << endl;

            debug_log << "read" << endl;
            debug_log << "about to read for code: " << code << ", time_pt: " << time_pt << ", rank: ";
            debug_log << rank << ", checkpt: " << checkpt_count << endl;

            extreme_debug_log  << "output.read_outputs.size(): " << output.read_outputs.size() << endl;
            read(my_codes, my_time_pts, code, time_pt, rank, output.read_outputs.at(checkpt_count));

            //rank 0 might to a few final per-run ops at the testing, these should be treated as part of the last checkpoint
            // if(checkpt_count < (output.config.num_timesteps / output.config.num_timesteps_per_checkpt - 1) ) {
            //  checkpt_count += 1;
            // }
        }
        //ends at 39
        else if (is_testing_harness(code)) {
            extreme_debug_log  << "testing harness" << endl;

            if(code == PROGRAM_START) {
                extreme_debug_log << "found new procs timing" << endl;
                start_time = time_pt;
                rank += 1;
                // cout << "global_rank: " << rank + global_rank << endl;
                timestep_num = -1;
                checkpt_count = 0;

            }
            // else if(code == SERVER_SETUP_DONE_INIT_DONE) {
            //     init_done_time = time_pt;
            // }

            for (pt_of_interest &pt : all_points_of_interest) {
                extreme_debug_log << "pt: " << pt.name << " open: " << pt.open << endl;
                if (pt.open) {
                    int adj_checkpt_count = checkpt_count;
                    if(pt.diff_last_first_checkpt) {
                        adj_checkpt_count = 0;
                    }
                    // if (pt.op_times[adj_checkpt_count][rank].size() == 0) {
                    //  // error_log << "checkpt end for pt: " << pt.name << ": " << adj_checkpt_count << endl;
                    //  // error_log << "error. pt: " << pt.name << " has 0 size for ops" << endl;
                    //  // error_log << "pt: " << pt.name << " has 0 size for ops for checkpt: " << adj_checkpt_count << endl;
                    // }
                    // if(code == pt.second_end_code && pt.second_end_code != 0 && rank == 0 ) {
                    
                    // if (pt.op_times[adj_checkpt_count][rank].size() == 0) {
                    //  // error_log << "checkpt end for pt: " << pt.name << ": " << adj_checkpt_count << endl;
                    //  // error_log << "error. pt: " << pt.name << " has 0 size for ops" << endl;
                    //  // error_log << "pt: " << pt.name << " has 0 size for ops for checkpt: " << adj_checkpt_count << endl;
                    // }
                    // else if(code == pt.second_end_code && rank == 0 ) {
                    //  // extreme_debug_log << "rank: " << rank << " found the end for a second end pt for " << pt.name << endl;
                    //  pt.open = false;
                    //  pt.end_times[adj_checkpt_count][rank].push_back(time_pt);
                    // }
                    if (code == pt.end_code) {

                        // extreme_debug_log << "pt.second_end_code: " << pt.second_end_code << endl;
                        // if(rank != 0 || pt.second_end_code == SECOND_END_CODE_NULL) {
                            pt.open = false;
                            extreme_debug_log  << "pt.end_times[adj_checkpt_count].size(): " << pt.end_times[adj_checkpt_count].size() << endl;
                            extreme_debug_log  << "pushing back for rank " << rank << endl;
                            pt.end_times[adj_checkpt_count][rank].push_back(time_pt);

                            if(pt.id == TOTAL_INIT_TIME) {
                                init_done_time = time_pt;
                            }
                        // }
                        // else {
                        //  extreme_debug_log << "rank: " << rank << " found a second end pt for " << pt.name << ". it's second end pt: " << pt.second_end_code << endl;
                        // }
                    }

                    // else {
                    //     //extreme_debug_log << "code: " << code << " pt " << pt.name << " is still open" << endl;
                    //     //extreme_debug_log << "pt.op_times.size(): " << pt.op_times.size() << endl;
                    //     //extreme_debug_log << "pt.op_times[" << adj_checkpt_count << "].size(): " << pt.op_times[adj_checkpt_count].size() << endl;                        
                    //     //extreme_debug_log << "rank: " << rank << " op time: " << pt.op_times[adj_checkpt_count][rank].back() << endl;
                    //     // if(op_time_since_last_testing_harness_pt > 0 && pt.op_times[adj_checkpt_count][rank].size() > 0) {
                    //     //    extreme_debug_log << "adding " << op_time_since_last_testing_harness_pt << " to " << pt.name << endl;
                    //     //    extreme_debug_log.op_times[adj_checkpt_count][rank].at(pt.op_times[adj_checkpt_count][rank].size()-1) += op_time_since_last_testing_harness_pt;   
                    //     // }

                    //     // pt.hdf5_times[checkpt_count][rank].at(pt.hdf5_times[checkpt_count][rank].size()-1) += hdf5_time_since_last_testing_harness_pt;
                    //     //extreme_debug_log << "op_time_since_last_testing_harness_pt: " << op_time_since_last_testing_harness_pt << endl;
                    //     //extreme_debug_log << "op time: " << pt.op_times[adj_checkpt_count][rank].back() << endl;
                    // }
                    if(op_time_since_last_testing_harness_pt > 0 && pt.op_times[adj_checkpt_count][rank].size() > 0) {
                        // debug_log << "adding " << op_time_since_last_testing_harness_pt << " to " << pt.name << endl;
                        pt.op_times[adj_checkpt_count][rank].at(pt.op_times[adj_checkpt_count][rank].size()-1) += op_time_since_last_testing_harness_pt;   
                    }
                }
                else if (code == pt.start_code) {
                    pt.open = true;
                    extreme_debug_log << "checkpt start for pt: " << pt.name << ": " << checkpt_count <<  endl;
                    extreme_debug_log  << "code: " << code << " is start point for " << pt.name << endl;
                    extreme_debug_log  << "pt.open: " << pt.open << endl;
                    extreme_debug_log  << "pt.op_times.size(): " << pt.op_times.size() << endl;
                    extreme_debug_log  << "pt.op_times[" << checkpt_count << "].size(): " << pt.op_times[checkpt_count].size() << endl;
                    extreme_debug_log << "rank: " << rank << " pt.op_times.size(): " << pt.op_times.size() << " pt.op_times[checkpt_count].size(): " << pt.op_times[checkpt_count].size() << endl;
                    extreme_debug_log << "pt.name: " << pt.name << " code: " << code << " time_pt: " << time_pt << " checkpt_count: " << checkpt_count << endl;
                    pt.op_times[checkpt_count][rank].push_back(0);
                    // pt.hdf5_times[checkpt_count][rank].push_back(0);
                    pt.start_times[checkpt_count][rank].push_back(time_pt);
                }
            }

            op_time_since_last_testing_harness_pt = 0;
            // hdf5_time_since_last_testing_harness_pt = 0;
        }
        //53
        else if( DB_SETUP_START == code && is_local_run(output) ) { 
            extreme_debug_log  << "db setup" << endl;

            long double server_start_time, server_init_done;
            debug_log << "about to init server" << endl;
            rc = init(my_codes, my_time_pts, code, output.server_output, time_pt, server_init_done, true);
            // if(rc != RC_OK) {
            //     goto cleanup;
            // }
            debug_log << "done initing server" << endl;
        }
        else if(is_db_output(code) && is_local_run(output)) {
            long double checkpt_end_time;
            extreme_debug_log  << "db output" << endl;
            extreme_debug_log << "before going, checkpt_count: " << checkpt_count << endl;

            db_output(my_codes, my_time_pts, my_storage_sizes, output, code, time_pt, checkpt_count, checkpt_end_time);
            add_checkpoint_last_first(time_pt, checkpt_end_time, checkpt_count, rank, all_points_of_interest);

            extreme_debug_log << "after returning, checkpt_count: " << checkpt_count << endl;
        }
        //56 - 58
        else if(is_shutdown(code) && is_local_run(output)) {
            long double shutdown_start_time = time_pt;
            long double shutdown_done_time;
            extreme_debug_log << "shutdown" << endl;
            debug_log << "shutdown " << endl;
            extreme_debug_log  << "shutdown" << endl;

            rc = shutdown(my_codes, my_time_pts, my_end, code, output.server_output, checkpt_count, shutdown_start_time, 
                        shutdown_done_time, true);

            debug_log << "shutdown_start_time: " << shutdown_start_time << " init_done_time: " << init_done_time << endl;
            output.server_output.run_times.push_back(shutdown_start_time - init_done_time);
            output.server_output.total_run_times.push_back(shutdown_done_time - start_time);
            debug_log << "done with shutdown" << endl;
            extreme_debug_log << "indx: " << my_codes - my_codes_vct.begin() << endl;
            if(my_codes == my_end) {
                extreme_debug_log << "break" << endl;
                break;
            }
        }
        else {
            error_log << "error in write client rank " << rank << ". code: " << code << " did not fit into one of the expected categories" << endl;
        }

        my_codes++;
        my_time_pts++;
    }//endif for rank

    extreme_debug_log << "found my end" << code << endl;
    extreme_debug_log << "indx: " << my_codes - my_codes_vct.begin() << endl;
    evaluate_pts_of_interest(all_points_of_interest, all_last_first_points_of_interest, output, global_rank);

}

static void process_file(ifstream &file, const client_config_output &output, int proc_chunk_size,
    vector<uint16_t> &my_codes_vct, vector<long double> &my_time_pts_vct,
    vector<uint64_t> &my_storage_sizes_vct
    )
{

    uint16_t code;
    long double time_pt;

    // uint32_t checkpt_count = 0;
    // int timestep_num = -1;

    bool first_iter = true;

    int rank = 0;

    uint64_t len = file.tellg();

    while (file >> code) {
        file >> time_pt;

        extreme_debug_log << "rank: " << rank << " code: " << code << " time_pt: " << time_pt << endl;

        if(code == PROGRAM_START) {
            extreme_debug_log << "program start" << endl;
            if(first_iter) {
                first_iter = false;
            }
            else {
                if(rank+1 == proc_chunk_size) {
                    //roll it back so the next thread sees 'PROGRAM START' as its first code
                    file.seekg(len, std::ios_base::beg);
                    return;
                }
                rank += 1;
                extreme_debug_log << "Rank: " << rank << endl;
            }
        }
        // else if( code ==  CREATE_NEW_TIMESTEP_START) {
        //     extreme_debug_log << "new timestep" << endl;
        //     timestep_num += 1;
        //     if(timestep_num % output.config.num_timesteps_per_checkpt == 0 && timestep_num!= 0) {
        //         // extreme_debug_log << "timestep_num: " << timestep_num << " checkpt_count: " << checkpt_count << endl;
        //         checkpt_count += 1;
        //     }
        // } 
        else if(is_db_output(code) && is_local_run(output) && (code%2)==0) {
            uint16_t prev_code = code;
            long double prev_time_pt = time_pt;
            uint64_t db_size;
            file >> db_size;

            // std::ostringstream ss;

            // len =  file.tellg();

            // string str_next_val;
            // file >> str_next_val;

            // //the next value should be a code so if there's a decimal in it, it means the code accidentally
            // //got lumped with the database size
            // if(str_next_val.find('.') != std::string::npos) {
            //     ss << db_size;
            //     extreme_debug_log << "true! lumping problem" << endl;
            //     std::string str_db_size = ss.str();
            //     extreme_debug_log << "str_db_size: " << str_db_size << endl;
            //     int last_three_digits = stoi(str_db_size.substr(str_db_size.length() - 3));
            //     code = last_three_digits;
            //     db_size = std::stod(str_db_size.substr(0, str_db_size.length() - 3));
            //     extreme_debug_log << "str_next_val: " << str_next_val << endl;
            //     time_pt = stold(str_next_val);

            //     extreme_debug_log << "str_db_size: " << str_db_size << " last_three_digits: " << last_three_digits << " db_size: " << db_size << " time_pt: " << time_pt << endl;

            //     my_codes_vct.push_back(prev_code);
            //     my_time_pts_vct.push_back(prev_time_pt);
            // }
            // else {
            //     extreme_debug_log << "db size is correct. rewinding" << endl;
            //     file.seekg(len, std::ios_base::beg);
            // }
            my_storage_sizes_vct.push_back(db_size);
        }
        my_codes_vct.push_back(code);
        my_time_pts_vct.push_back(time_pt);

        len = file.tellg();

    };

}


static void add_checkpoint_last_first(long double start_time, long double end_time, 
                                uint32_t checkpt_count, int rank, vector<pt_of_interest> &all_points_of_interest) {
    pt_of_interest *pt = &all_points_of_interest[all_points_of_interest.size()-1];
    if(pt->name != "CHECKPOINT_DB_TIME") {
        error_log << "error. last point of interest is not CHECKPOINT_DB_TIME" << endl;
        return;
    }
    pt->start_times[checkpt_count][rank].push_back(start_time);
    pt->op_times[checkpt_count][rank].push_back(end_time - start_time);
    pt->end_times[checkpt_count][rank].push_back(end_time);
}


static void add_earliest_start(uint16_t code, long double start_time, read_client_config_output &output) {

    debug_log << "add_earliest_start" << endl;

    extreme_debug_log << "am evaluating " << endl;
    // #pragma omp critical
    // {
        map<uint16_t,long double>::const_iterator i = output.earliest_starts.find(code);
        debug_log << "am adding earliest start for " << code << ": " << start_time << endl;

        if ( i == output.earliest_starts.end() || i->second > start_time) {            
            extreme_debug_log << "have decided to add it" << endl;
            output.earliest_starts[code] = start_time;
        }
    // }
}

static void add_latest_finish(uint16_t code, long double end_time, read_client_config_output &output) {
    debug_log << "add_latest_finish" << endl;
    
    extreme_debug_log << "am evaluating " << endl;

    // #pragma omp critical
    // {
        map<uint16_t,long double>::const_iterator i = output.latest_finishes.find(code);
        debug_log << "am adding latest finish for " << code << ": " << end_time << endl;

        if ( i == output.latest_finishes.end() || i->second < end_time) {
            extreme_debug_log << "have decided to add it" << endl;
            output.latest_finishes[code] = end_time;
        }
    // }
}


static void add_local_result_to_global_results(struct client_config_output &my_output, struct client_config_output &output
    )
{
    debug_log << "add_local_result_to_global_results" << endl;

    #pragma omp critical(l1)
    {
        combine_3D(output.per_proc_op_times, my_output.per_proc_op_times);
        combine_3D(output.per_proc_times, my_output.per_proc_times);
        combine_4D(output.op_times, my_output.op_times);
        
        for(int i = 0; i < output.read_outputs.size(); i++) {
            combine_1D(output.read_outputs[i].sum_all_read_patterns_times, my_output.read_outputs[i].sum_all_read_patterns_times);
            combine_2D(output.read_outputs[i].total_read_patterns_times, my_output.read_outputs[i].total_read_patterns_times);
            combine_1D(output.read_outputs[i].extra_testing_times, my_output.read_outputs[i].extra_testing_times);
            combine_1D(output.read_outputs[i].reading_times, my_output.read_outputs[i].reading_times);
            combine_2D(output.read_outputs[i].read_pattern_times, my_output.read_outputs[i].read_pattern_times);
            combine_2D(output.read_outputs[i].read_pattern_gather_times, my_output.read_outputs[i].read_pattern_gather_times);
            combine_2D(output.read_outputs[i].read_pattern_op_times, my_output.read_outputs[i].read_pattern_op_times);
            combine_2D(output.read_outputs[i].read_pattern_total_gather_times, my_output.read_outputs[i].read_pattern_total_gather_times);
            combine_2D(output.read_outputs[i].read_pattern_total_op_times, my_output.read_outputs[i].read_pattern_total_op_times);
            combine_3D(output.read_outputs[i].collective_op_times, my_output.read_outputs[i].collective_op_times);
            combine_3D(output.read_outputs[i].op_times, my_output.read_outputs[i].op_times);

            for (std::map<uint16_t,long double>::const_iterator start_it=my_output.read_outputs[i].earliest_starts.begin(); start_it!=my_output.read_outputs[i].earliest_starts.end(); ++start_it) {
                extreme_debug_log << "starting!" << endl;
                uint16_t code = start_it->first;
                long double time_pt = start_it->second;
                add_earliest_start(code, time_pt, output.read_outputs[i]);
            }
            for (std::map<uint16_t,long double>::const_iterator start_it=my_output.read_outputs[i].latest_finishes.begin(); start_it!=my_output.read_outputs[i].latest_finishes.end(); ++start_it) {
                uint16_t code = start_it->first;
                long double time_pt = start_it->second;
                add_latest_finish(code, time_pt, output.read_outputs[i]);
            }
        }
       
        if(is_local_run(output)) {
            combine_2D(output.server_output.init_times, my_output.server_output.init_times);
            combine_1D(output.server_output.run_times, my_output.server_output.run_times);
            combine_1D(output.server_output.shutdown_times, my_output.server_output.shutdown_times);
            combine_1D(output.server_output.total_run_times, my_output.server_output.total_run_times);
            combine_2D(output.server_output.db_checkpoint_times, my_output.server_output.db_checkpoint_times);
            combine_2D(output.server_output.db_checkpoint_sizes, my_output.server_output.db_checkpoint_sizes);
            combine_2D(output.server_output.db_compact_times, my_output.server_output.db_compact_times);
            combine_4D(output.server_output.op_times, my_output.server_output.op_times);
            combine_2D(output.server_output.all_clock_times, my_output.server_output.all_clock_times);
            combine_2D(output.server_output.clock_times_eval, my_output.server_output.clock_times_eval);
            combine_2D(output.server_output.all_storage_sizes, my_output.server_output.all_storage_sizes);
        }
    }
}


static void evaluate_pts_of_interest(const vector<pt_of_interest> &all_points_of_interest,
    vector<pt_of_interest> &all_last_first_points_of_interest, struct client_config_output &output,
    int global_rank)
{
    debug_log << "about to begin eval" << endl;
    extreme_debug_log  << "evaluate_pts_of_interest" << endl;

    for(pt_of_interest pt : all_points_of_interest) {
        for(int checkpt_count = 0; checkpt_count < output.config.num_timesteps / output.config.num_timesteps_per_checkpt; checkpt_count++ ) {
            for(int rank = 0; rank < pt.start_times[checkpt_count].size(); rank++) {

                if(
                    checkpt_count >= pt.start_times.size() || 
                    checkpt_count >= pt.end_times.size() || 
                    checkpt_count >= pt.op_times.size() || 
                    rank >= pt.start_times[checkpt_count].size() ||
                    rank >= pt.end_times[checkpt_count].size() ||
                    rank >= pt.op_times[checkpt_count].size()
                )
                {
                    error_log << "error for rank: " << (rank+global_rank) << " pt: " << pt.name << " checkpt: " << checkpt_count << endl;
                    error_log << "checkpt_count: " << checkpt_count << ", pt.start_times.size(): " << pt.start_times.size() << "," 
                              <<  " pt.end_times.size(): " << pt.end_times.size() << ", pt.op_times.size(): " << pt.op_times.size() << endl;
                    error_log << "pt.start_times[" << checkpt_count << "].size(): " << pt.start_times[checkpt_count].size() << "," 
                              <<  " pt.end_times[" << checkpt_count << "].size(): " << pt.end_times[checkpt_count].size() << ","
                              << " pt.op_times[" << checkpt_count << "].size(): " << pt.op_times[checkpt_count].size() << endl;
                }

                vector<long double> start_times = pt.start_times[checkpt_count][rank];
                vector<long double> end_times = pt.end_times[checkpt_count][rank];
                vector<long double> op_times = pt.op_times[checkpt_count][rank];
                if(
                    ( 
                        (start_times.size() != end_times.size() && pt.name != "PER_RUN_CREATE_TIME" && (pt.name != "CREATE_VARS_TIME" || (rank+global_rank) < output.config.num_server_procs) && !pt.diff_last_first_checkpt)  ||
                        start_times.size() != op_times.size()
                    )
                )
                {
                    error_log << "error for rank: " << (rank+global_rank) << " pt: " << pt.name << " checkpt: " << checkpt_count << endl;
                    error_log << "checkpt_count: " << checkpt_count << ", start_times.size(): " << start_times.size()
                              << ", end_times.size(): " << end_times.size() << ", op_times.size(): " << op_times.size() << endl;
                }
                // vector<long double> hdf5_times = pt.hdf5_times[checkpt_count][rank];

                if(start_times.size() > 0 && end_times.size() > 0) {
                    // extreme_debug_log  << "pt.start_times.size(): " << pt.start_times.size() << endl;
                    // extreme_debug_log  << "pt.end_times[" << checkpt_count << "].size(): " << pt.end_times[checkpt_count].size() << endl;
                    // extreme_debug_log  << "pt.end_times[" << checkpt_count << "][" << rank << "].size(): " << pt.end_times[checkpt_count][rank].size() << endl;
                    // extreme_debug_log << "pt.end_times[checkpt_count][rank][0]: " << pt.end_times[checkpt_count][rank][0] << endl;
                    // extreme_debug_log  << "pt.op_times[checkpt_count].size(): " << pt.op_times[checkpt_count].size() << endl;
                    // extreme_debug_log  << "pt.op_times[checkpt_count][rank].size(): " << pt.op_times[checkpt_count][rank].size() << endl;
                    // extreme_debug_log << "pt.op_times[checkpt_count][rank][0]: " << pt.op_times[checkpt_count][rank][0] << endl;


                    //  continue;
                    // }
                    // else {
                    //  if(start_times.size() != end_times.size()) {
                    //      error_log << "error. for rank: " << rank << " for start code: " << pt.start_code << 
                    //          " size: " << start_times.size() << " for end code: " << pt.end_code <<
                    //          " size: " << end_times.size() << endl;
                    //      continue;
                    //  }       
                    // }

                    if(
                        output.per_proc_times.size() <= checkpt_count ||
                        output.per_proc_op_times.size() <= checkpt_count ||
                        output.per_proc_times[checkpt_count].size() <= pt.id ||
                        output.per_proc_op_times[checkpt_count].size() <= pt.id
                    ) 
                    {
                        error_log << "error for rank: " << (rank+global_rank) << " pt: " << pt.name << " checkpt: " << checkpt_count << endl;
                        error_log << "checkpt_count: " << checkpt_count << ", output.per_proc_times.size(): " << output.per_proc_times.size() << "," 
                                  <<  "  output.per_proc_op_times.size(): " <<  output.per_proc_op_times.size() << endl;
                        error_log << "output.per_proc_times[" << checkpt_count << "].size(): " << output.per_proc_times[checkpt_count].size() << "," 
                                  <<  " output.per_proc_op_times[" << checkpt_count << "].size(): " << output.per_proc_op_times[checkpt_count].size() << endl;
                    }


                    for(int i = 0; i < start_times.size(); i++) {
                        long double start_time = start_times.at(i);
                        long double end_time = end_times.at(i);
                        long double total_time = end_time - start_time;
                        long double op_time = op_times.at(i);

                        // long double hdf5_time = hdf5_times.at(i);

                        extreme_debug_log << "am adding to map total time: " << total_time << endl;
                        extreme_debug_log << "for rank: " << rank << " start time: " << start_time << " end time: " << end_time << endl; 
                        output.per_proc_times[checkpt_count][pt.id].push_back(total_time);
                        

                        extreme_debug_log << "am done adding to map total time: " << total_time << endl;

                        if(op_time > 0) {
                            extreme_debug_log << "pt.name: " << pt.name << " op_time: " << op_time << endl;
                            extreme_debug_log << "output.per_proc_op_times.size(): " << output.per_proc_op_times.size() << endl;
                            extreme_debug_log << "output.per_proc_op_times[checkpt_count].size(): " << output.per_proc_op_times[checkpt_count].size() << endl;
                            output.per_proc_op_times[checkpt_count][pt.id].push_back(op_time);
                        }
                        // if(hdf5_time > 0) {
                        //  output.per_proc_hdf5_times[checkpt_count][pt.id].push_back(hdf5_time);
                        // }
                        // else {
                        //  debug_log << "for pt: " << pt.name << " code: " << pt.start_code << " to code " << pt.end_code << " op_times sum == 0" << endl;
                        // }
                        extreme_debug_log << "output.per_proc_times.size: " << output.per_proc_times.size() << endl;
                        if(total_time < op_time) {
                            error_log << "error. for rank: " << rank << " for start code: " << pt.start_code << " and end code: " << pt.end_code <<
                                " total_time: " << total_time << " and op_time: " << op_time << endl;
                        }
                        // if(total_time < hdf5_times) {
                        //  error_log << "error. for rank: " << rank << " for start code: " << pt.start_code << " and end code: " << pt.end_code <<
                        //      " total_time: " << total_time << " and hdf5_time: " << hdf5_time << endl;
                        // }
                    }
                }
            }
        }
    }
    debug_log << "about to begin last_first timing" << endl;
    for(int pt_id = 0; pt_id < all_points_of_interest.size(); pt_id++) {
        pt_of_interest my_pt = all_points_of_interest[pt_id];
        // if (! my_pt.last_first) {
        //  continue;
        // }
        if(my_pt.last_first) {

            for(int checkpt_count = 0; checkpt_count < output.config.num_timesteps / output.config.num_timesteps_per_checkpt; checkpt_count++ ) {
                extreme_debug_log << "pt: " << my_pt.name << " start_times[checkpt_count].size(): " << my_pt.start_times[checkpt_count].size() << endl;
                extreme_debug_log << "pt: " << my_pt.name << " end_times[checkpt_count].size(): " << my_pt.end_times[checkpt_count].size() << endl;           
                if(my_pt.start_times[checkpt_count].size() == 0) {
                    error_log << "error. pt " << my_pt.name << " start_times for checkpt_count " << checkpt_count << " has size 0" << endl;
                    continue;
                }
                else if(my_pt.end_times[checkpt_count].size() == 0) {
                    error_log << "error. pt " << my_pt.name << " end_times for checkpt_count " << checkpt_count << " has size 0" << endl;
                    continue;
                }

                #pragma omp critical(pts)
                {
                    for(int rank = 0; rank < my_pt.start_times[checkpt_count].size(); rank++) {
                        if(my_pt.start_times[checkpt_count][rank].size() > 0 && my_pt.end_times[checkpt_count][rank].size() > 0) {
                            if( all_last_first_points_of_interest[pt_id].start_times[checkpt_count][0].size() == 0) {
                                all_last_first_points_of_interest[pt_id] = my_pt;
                            }
                            else {                        
                                // pt_of_interest all_pt = all_last_first_points_of_interest[pt_id];
                                uint32_t size = all_last_first_points_of_interest[pt_id].start_times[checkpt_count][0].size();
                                vector<long double> first_starts = all_last_first_points_of_interest[pt_id].start_times[checkpt_count][0];
                                vector<long double> last_finishes = all_last_first_points_of_interest[pt_id].end_times[checkpt_count][0];

                                extreme_debug_log << "my_pt.start_times[checkpt_count][rank].size(): " << my_pt.start_times[checkpt_count][rank].size() << endl;
                                extreme_debug_log << "my_pt.end_times[checkpt_count][rank].size(): " << my_pt.end_times[checkpt_count][rank].size() << endl;
                                extreme_debug_log << "first_starts.size(): " << first_starts.size() << endl;
                                extreme_debug_log << "last_finishes.size(): " << last_finishes.size() << endl;

                                for(int i = 0; i< my_pt.start_times[checkpt_count][rank].size(); i++) {
                                    extreme_debug_log << setprecision(12) << "my start time: " << my_pt.start_times[checkpt_count][rank].at(i) << " earliest start time: " << first_starts[i] << endl;
                                    extreme_debug_log << setprecision(12) << "my end time: " << my_pt.end_times[checkpt_count][rank].at(i) << " latest end time: " << last_finishes[i] << endl;
                                    long double start_time = my_pt.start_times[checkpt_count][rank].at(i);
                                    long double end_time = my_pt.end_times[checkpt_count][rank].at(i);

                                    if(start_time < first_starts.at(i)) {
                                        extreme_debug_log << setprecision(12) << "found a new first start for pt " << my_pt.name << " setting to " << start_time << endl;
                                        all_last_first_points_of_interest[pt_id].start_times[checkpt_count][0].at(i) = start_time;
                                    }
                                    if(end_time > last_finishes.at(i)) {
                                        extreme_debug_log << setprecision(12) << "found a new last finish for pt " << my_pt.name << " setting to " << end_time << endl;
                                        all_last_first_points_of_interest[pt_id].end_times[checkpt_count][0].at(i) = end_time;
                                    }               

                                }
                            }
                        }
                    }
                }
            }

        }
    }
    debug_log << "done with eval for pts of interest" << endl;
}

static void evaluate_last_first_pts_of_interest(const vector<pt_of_interest> &all_last_first_points_of_interest, 
    struct client_config_output &output)
{
    debug_log << "am beginning" << endl;
    debug_log << "about to begin last_first timing" << endl;
    for(int pt_id = 0; pt_id < all_last_first_points_of_interest.size(); pt_id++) {
        pt_of_interest my_pt = all_last_first_points_of_interest[pt_id];
        if (! my_pt.last_first) {
            continue;
        }

        for(int checkpt_count = 0; checkpt_count < output.config.num_timesteps / output.config.num_timesteps_per_checkpt; checkpt_count++ ) {
            // extreme_debug_log << "pt.name: " << my_pt.name << " pt.start_times.size(): " << my_pt.start_times.size()*output.config.num_write_client_procs << endl;

            // extreme_debug_log << "pt: " << pt.name << " start_times[checkpt_count].size(): " << pt.start_times[checkpt_count].size() << endl;
            // extreme_debug_log << "pt: " << pt.name << " end_times[checkpt_count].size(): " << pt.end_times[checkpt_count].size() << endl;         
            if(my_pt.start_times[checkpt_count].size() == 0) {
                error_log << "error. pt " << my_pt.name << " start_times for checkpt_count " << checkpt_count << " has size 0" << endl;
                continue;
            }
            else if(my_pt.end_times[checkpt_count].size() == 0) {
                error_log << "error. pt " << my_pt.name << " end_times for checkpt_count " << checkpt_count << " has size 0" << endl;
                continue;
            }

            uint32_t size = my_pt.start_times[checkpt_count][0].size();
            vector<long double> first_starts = my_pt.start_times[checkpt_count][0];
            vector<long double> last_finishes = my_pt.end_times[checkpt_count][0];

            if(last_finishes.size() != first_starts.size()) {
                error_log << "first_starts.size(): " << first_starts.size() << " last_finishes.size(): " << last_finishes.size() << endl;
                error_log << "pt.name: " << my_pt.name << " checkpt_count: " << checkpt_count << endl;
                continue;
            }

            for(int i = 0; i < first_starts.size(); i++) {
                long double total_time = last_finishes.at(i) - first_starts.at(i);
                // extreme_debug_log << "for last_first first_start: " << first_start << " last finish: " << last_finish << endl;
                output.last_first_times[checkpt_count][my_pt.id].push_back(total_time);
            }

        }

    }
    debug_log << "done with evaluate_last_first_pts_of_interest" << endl;

}


static vector<pt_of_interest> add_all_points_of_interest(uint32_t num_write_client_procs, uint32_t num_checkpts, uint32_t write_type, uint32_t server_type) {
    bool do_last_first = true;
    bool last_first_checkpt_diff = true;

    extreme_debug_log << "num_write_client_procs: " << num_write_client_procs << endl;

    pt_of_interest init_time = pt_of_interest(TOTAL_INIT_TIME, "TOTAL_INIT_TIME", PROGRAM_START, SERVER_SETUP_DONE_INIT_DONE, num_write_client_procs, num_checkpts, do_last_first);
    pt_of_interest per_run_create_time = pt_of_interest(PER_RUN_CREATE_TIME, "PER_RUN_CREATE_TIME", WRITING_START, CREATE_TYPES_DONE, num_write_client_procs, num_checkpts, do_last_first);      
    // pt_of_interest timestep_create_and_activate_time = pt_of_interest(TOTAL_TIMESTEP_CREATE_AND_ACTIVATE_TIME, "TOTAL_TIMESTEP_CREATE_AND_ACTIVATE_TIME", CREATE_NEW_TIMESTEP_START,CREATE_AND_ACTIVATE_TIMESTEP_DONE, num_write_client_procs, num_checkpts, do_last_first);
    // pt_of_interest timestep_activate_time = pt_of_interest(TOTAL_TIMESTEP_ACTIVATE_TIME, "TOTAL_TIMESTEP_ACTIVATE_TIME", CREATE_TIMESTEP_DONE,CREATE_AND_ACTIVATE_TIMESTEP_DONE, num_write_client_procs, num_checkpts, do_last_first);

    pt_of_interest create_collective_attrs_time = pt_of_interest(CREATE_COLLECTIVE_ATTRS_TIME, "CREATE_COLLECTIVE_ATTRS_TIME", INSERT_VAR_ATTRS_COLLECTIVE_START,INSERT_VAR_ATTRS_COLLECTIVE_DONE, num_write_client_procs, num_checkpts, do_last_first);
    pt_of_interest dirman_init_time = pt_of_interest(DIRMAN_INIT_TIME, "DIRMAN_INIT_TIME", INIT_VARS_DONE, DIRMAN_SETUP_DONE,num_write_client_procs, num_checkpts);
    pt_of_interest checkpt_db_time = pt_of_interest(CHECKPOINT_DB_TIME, "CHECKPOINT_DB_TIME", MD_CHECKPOINT_DATABASE_START, MD_CHECKPOINT_DATABASE_DONE,num_write_client_procs, num_checkpts,do_last_first);
    pt_of_interest activate_timestep_time = pt_of_interest(TOTAL_TIMESTEP_ACTIVATE_TIME, "TOTAL_TIMESTEP_ACTIVATE_TIME", ACTIVATE_TIMESTEP_COMPONENTS_START,ACTIVATE_TIMESTEP_COMPONENTS_DONE, num_write_client_procs, num_checkpts, do_last_first);

    if(write_type == WRITE_HDF5) {
        init_time = pt_of_interest(TOTAL_INIT_TIME, "TOTAL_INIT_TIME", PROGRAM_START, INIT_VARS_DONE, num_write_client_procs, num_checkpts, do_last_first);
        per_run_create_time = pt_of_interest(PER_RUN_CREATE_TIME, "PER_RUN_CREATE_TIME", WRITING_START, CREATE_NEW_RUN_DONE, num_write_client_procs, num_checkpts, do_last_first);   
        // create_collective_attrs_time = pt_of_interest(CREATE_COLLECTIVE_ATTRS_TIME, "CREATE_COLLECTIVE_ATTRS_TIME", GATHER_ATTRS_START, GATHER_ATTRS_DONE, CREATE_TIMESTEP_ATTRS_START, num_write_client_procs, num_checkpts, do_last_first, false);
        create_collective_attrs_time = pt_of_interest(CREATE_COLLECTIVE_ATTRS_TIME, "CREATE_COLLECTIVE_ATTRS_TIME", GATHER_ATTRS_START, CREATE_TIMESTEP_ATTRS_START, num_write_client_procs, num_checkpts, do_last_first, false);
    }
    if(is_local_run(server_type)) {
        init_time = pt_of_interest(TOTAL_INIT_TIME, "TOTAL_INIT_TIME", PROGRAM_START, INIT_VARS_DONE, num_write_client_procs, num_checkpts, do_last_first);
        activate_timestep_time = pt_of_interest(TOTAL_TIMESTEP_ACTIVATE_TIME, "TOTAL_TIMESTEP_ACTIVATE_TIME", CREATE_TIMESTEP_DONE,CREATE_AND_ACTIVATE_TIMESTEP_DONE, num_write_client_procs, num_checkpts, do_last_first);

    }

    vector<pt_of_interest> all_points_of_interest = {   

        pt_of_interest(TOTAL_RUN_TIME, "TOTAL_RUN_TIME", PROGRAM_START, WRITING_DONE, num_write_client_procs, num_checkpts, do_last_first, last_first_checkpt_diff), //problem
        pt_of_interest(MPI_INIT_TIME, "MPI_INIT_TIME", PROGRAM_START, MPI_INIT_DONE, num_write_client_procs, num_checkpts),
        init_time,
        // pt_of_interest(METADATA_CLIENT_INIT_TIME, "METADATA_CLIENT_INIT_TIME", MPI_INIT_DONE, METADATA_CLIENT_INIT_DONE, num_write_client_procs,num_write_client_procs, num_checkpts),
        // pt_of_interest(DIRMAN_INIT_TIME, "DIRMAN_INIT_TIME", INIT_VARS_DONE, DIRMAN_SETUP_DONE,num_write_client_procs, num_checkpts),
        pt_of_interest(SERVER_INIT_TIME, "SERVER_INIT_TIME", DIRMAN_SETUP_DONE, SERVER_SETUP_DONE_INIT_DONE,num_write_client_procs, num_checkpts),
        //no longer accurately captures time because read is interspersed with writing
        // pt_of_interest(TOTAL_WRITE_TIME, "TOTAL_WRITE_TIME", WRITING_START, WRITING_DONE, num_write_client_procs, num_checkpts, true), //problem       
        per_run_create_time,
        // pt_of_interest(OBJECTOR_LOAD_TIME, "OBJECTOR_LOAD_TIME", OBJECTOR_LOAD_START, OBJECTOR_LOAD_DONE,num_write_client_procs, num_checkpts),
        pt_of_interest(CREATE_RUN_TIME, "CREATE_RUN_TIME", CREATE_NEW_RUN_START, CREATE_NEW_RUN_DONE,num_write_client_procs, num_checkpts),
        pt_of_interest(CREATE_TYPES_TIME, "CREATE_TYPES_TIME", CREATE_TYPES_START, CREATE_TYPES_DONE,num_write_client_procs, num_checkpts),
        //no longer accurately captures time because read is interspersed with writing
        // pt_of_interest(TOTAL_TIMESTEPS_CREATE_TIME, "TOTAL_TIMESTEPS_CREATE_TIME",  CREATE_TIMESTEPS_START, CREATE_ALL_TIMESTEPS_DONE, num_write_client_procs, num_checkpts, true), //problem
        pt_of_interest(TOTAL_TIMESTEP_CREATE_TIME, "TOTAL_TIMESTEP_CREATE_TIME", CREATE_NEW_TIMESTEP_START,CREATE_TIMESTEP_DONE, num_write_client_procs, num_checkpts, do_last_first),
        activate_timestep_time,
        // pt_of_interest(CREATE_OBJS_AND_STORE_IN_MAP_TIME, "CREATE_OBJS_AND_STORE_IN_MAP_TIME", CREATE_OBJS_AND_STORE_IN_MAP_START, CREATE_OBJS_AND_STORE_IN_MAP_DONE,num_write_client_procs, num_checkpts),
        // pt_of_interest(WRITE_CHUNK_DATA_TIME, "WRITE_CHUNK_DATA_TIME", WRITE_CHUNK_DATA_START, WRITE_CHUNK_DATA_DONE,num_write_client_procs, num_checkpts),
        // pt_of_interest(CREATE_DATA_TIME, "CREATE_DATA_TIME", CREATE_DATA_START, CREATE_DATA_DONE,num_write_client_procs, num_checkpts),
        // pt_of_interest(FIND_CHUNK_MIN_AND_MAX_TIME, "FIND_CHUNK_MIN_AND_MAX_TIME", CHUNK_MAX_MIN_FIND_START, CHUNK_MAX_MIN_FIND_DONE,num_write_client_procs, num_checkpts),
        // pt_of_interest(OBJECTOR_TIME, "OBJECTOR_TIME", BOUNDING_BOX_TO_OBJ_NAMES_START, BOUNDING_BOX_TO_OBJ_NAMES_DONE,num_write_client_procs, num_checkpts),
        // pt_of_interest(CREATE_VAR_ATTRS_TIME, "CREATE_VAR_ATTRS_TIME", CREATE_VAR_ATTRS_FOR_NEW_VAR_START, CREATE_VAR_ATTRS_FOR_NEW_VAR_DONE,num_write_client_procs, num_checkpts),
        pt_of_interest(CREATE_VARS_TIME, "CREATE_VARS_TIME", CREATE_VARS_START, CREATE_VARS_DONE,num_write_client_procs, num_checkpts),       
        pt_of_interest(CREATE_VAR_ATTRS_TIME, "CREATE_VAR_ATTRS_TIME", CREATE_VAR_ATTRS_START, CREATE_VAR_ATTRS_DONE,num_write_client_procs, num_checkpts),
        pt_of_interest(CREATE_TIMESTEP_ATTRS_TIME, "CREATE_TIMESTEP_ATTRS_TIME", CREATE_TIMESTEP_ATTRS_START, CREATE_TIMESTEP_ATTRS_DONE,num_write_client_procs, num_checkpts),
        pt_of_interest(CREATE_RUN_ATTRS_TIME, "CREATE_RUN_ATTRS_TIME", CREATE_RUN_ATTRS_START, CREATE_RUN_ATTRS_DONE,num_write_client_procs, num_checkpts),
        //fix - do I want to do anything with shutdown?
        
        
    };

    if(write_type == WRITE_GATHERED) {
        pt_of_interest gather = pt_of_interest(GATHER_ATTRS_TIME, "GATHER_ATTRS_START", GATHER_ATTRS_START, GATHER_ATTRS_DONE,num_write_client_procs, num_checkpts, do_last_first);
        all_points_of_interest.push_back(gather);
        all_points_of_interest.push_back(create_collective_attrs_time);
    }
    if(!is_local_run(server_type) && write_type != WRITE_HDF5) {
        all_points_of_interest.push_back(dirman_init_time);
    }
    if(write_type != WRITE_HDF5) {
        all_points_of_interest.push_back(checkpt_db_time);
    //     all_points_of_interest.push_back(timestep_create_and_activate_time);
    }
    return all_points_of_interest;
}

// static vector<pt_of_interest> add_all_last_first_points_of_interest() {
//  vector<pt_of_interest> all_points_of_interest = {   
//      pt_of_interest("TOTAL_INIT_TIME", PROGRAM_START, SERVER_SETUP_DONE_INIT_DONE,num_write_client_procs, num_checkpts),
//      pt_of_interest("PER_RUN_CREATE_TIME", WRITING_START, CREATE_TYPES_DONE,num_write_client_procs, num_checkpts), //problem
//      pt_of_interest("TOTAL_TIMESTEPS_CREATE_TIME",  CREATE_TIMESTEPS_START, CREATE_ALL_TIMESTEPS_DONE,num_write_client_procs, num_checkpts), //problem
//      pt_of_interest("TOTAL_TIMESTEP_CREATE_TIME", CREATE_NEW_TIMESTEP_START,CREATE_AND_ACTIVATE_TIMESTEP_DONE,num_write_client_procs, num_checkpts), //problem
//      pt_of_interest("TOTAL_WRITE_TIME", WRITING_START, WRITING_DONE,num_write_client_procs, num_checkpts), //problem
//      pt_of_interest("TOTAL_RUN_TIME", PROGRAM_START, WRITING_DONE) //problem
//  };

//  return all_points_of_interest;
// }



static void ops(std::vector<uint16_t>::const_iterator &my_codes, std::vector<long double>::const_iterator &my_time_pts, uint16_t code, long double start_time, int rank,
    struct client_config_output &output, long double &end_time, long double &total_time, uint32_t &checkpt_count, vector<queue<double>> &pending_rdma_ops
    ) 
{
    debug_log <<  "ops- op: " << code << " start_time: " << setprecision(12) << start_time << endl;

    long double time_pt;
    int op_indx = (code - 1000) / 100;
    bool rdma = (code % 10 == 9); //rdma get finished

    if(rdma) {
        code = code - 9; //normalizes
    }

    long double serialize_done_time;
    long double send_done_time;
    long double rec_return_msg_done_time;
    // long double end_time;

    long double rdma_get_start_time;

    long double serialize_time = 0;
    long double send_time = 0;
    long double wait_for_reply_time = 0;
    long double rdma_init_time = 0;
    long double rdma_wait_time = 0;
    long double deserialize_time = 0;

    unsigned short serialze_code = code + 2;
    unsigned short send_code = code + 4;
    unsigned short rec_return_msg_code = code + 5;
    unsigned short end_code = code + 7;

    uint16_t rdma_get_start_code = code + 8;
    uint16_t rdma_get_done_code = code + 9;

    if(rdma) {
        serialize_time = pending_rdma_ops[op_indx].front();
        pending_rdma_ops[op_indx].pop();
        send_time = pending_rdma_ops[op_indx].front();
        pending_rdma_ops[op_indx].pop();
        wait_for_reply_time = pending_rdma_ops[op_indx].front();
        pending_rdma_ops[op_indx].pop();
        rdma_init_time = pending_rdma_ops[op_indx].front();
        pending_rdma_ops[op_indx].pop();
        rdma_get_start_time = pending_rdma_ops[op_indx].front();
        pending_rdma_ops[op_indx].pop();

        rdma_wait_time = start_time - rdma_get_start_time;
        //extreme_debug_log << "after popping, pending_rdma_ops[" << op_indx << "].size(): "  << pending_rdma_ops[op_indx].size() << endl;
    }

    extreme_debug_log << "code: " << code << " end_code: " << to_string(end_code) << " note this is in extreme debug logging" << endl;
    // extreme_debug_log << "am debug logging \n";
    while(code != end_code) {
        extreme_debug_log << "code:  " << code << endl;
        my_codes++;
        my_time_pts++;
        code = *my_codes;
        time_pt = *my_time_pts;
        //my_codes != my_vector.end
        if(code == serialze_code) {
            serialize_done_time = time_pt;
        }
        else if(code == send_code) {
            send_done_time = time_pt;
        }
        else if(code == rec_return_msg_code) {
            rec_return_msg_done_time = time_pt;
        }
        else if(code == rdma_get_start_code) {
            rdma_get_start_time = time_pt;

            serialize_time = serialize_done_time - start_time;
            send_time = send_done_time - serialize_done_time;
            wait_for_reply_time = rec_return_msg_done_time - send_done_time;
            rdma_init_time = rdma_get_start_time - rec_return_msg_done_time;

            //could just push this into the output.op_times but there could be an intervening non-rdma op of the same type
            pending_rdma_ops[op_indx].push(serialize_time);
            pending_rdma_ops[op_indx].push(send_time);
            pending_rdma_ops[op_indx].push(wait_for_reply_time);
            pending_rdma_ops[op_indx].push(rdma_init_time);
            pending_rdma_ops[op_indx].push(rdma_get_start_time);

            //extreme_debug_log << "after pushing, pending_rdma_ops[" << op_indx << "].size(): "  << pending_rdma_ops[op_indx].size() << endl;
            return;
        }
    }
    if (code == end_code) {
        end_time = time_pt;
        total_time = end_time - start_time;

        debug_log << "output.op_times[" << checkpt_count << "].size(): " << output.op_times[checkpt_count].size() << endl;
        debug_log << "output.op_times[" << checkpt_count << "][" << op_indx << "].size(): " << output.op_times[checkpt_count][op_indx].size() << endl;
        if(output.config.write_type == WRITE_HDF5 || is_local_run(output)) {
            output.op_times[checkpt_count][op_indx].at(6).push_back(end_time-start_time);           
            // output.op_times[checkpt_count][op_indx].at(4).push_back(end_time-start_time);           
        }
        else if(end_code != MD_FULL_SHUTDOWN_DONE && end_code != MD_CHECKPOINT_DATABASE_DONE &&
                !is_transaction_op(end_code)
               ) 
        {
            if(end_code != MD_INSERT_VAR_ATTRIBUTE_BY_DIMS_BATCH_DONE || output.config.write_type != WRITE_GATHERED || rank < output.config.num_server_procs) {
                if(!rdma) {
                    serialize_time = serialize_done_time - start_time;
                    send_time = send_done_time - serialize_done_time;
                    wait_for_reply_time = rec_return_msg_done_time - send_done_time;
                    deserialize_time = end_time - rec_return_msg_done_time;
                    total_time = end_time - start_time;
                }
                else {
                    deserialize_time = end_time - start_time;
                    total_time = serialize_time + send_time + wait_for_reply_time + rdma_init_time + rdma_wait_time + deserialize_time;
                }

                output.op_times[checkpt_count][op_indx].at(0).push_back(serialize_time);
                output.op_times[checkpt_count][op_indx].at(1).push_back(send_time);
                output.op_times[checkpt_count][op_indx].at(2).push_back(wait_for_reply_time);
                output.op_times[checkpt_count][op_indx].at(3).push_back(rdma_init_time); //will be 0 if not rdma
                output.op_times[checkpt_count][op_indx].at(4).push_back(rdma_wait_time); //will be 0 if not rdma
                output.op_times[checkpt_count][op_indx].at(5).push_back(deserialize_time);
                // output.op_times[checkpt_count][op_indx].at(3).push_back(adjust(rec_return_msg_done_time,end_time));
                // output.op_times[checkpt_count][op_indx].at(4).push_back(adjust(start_time,end_time)); 
                output.op_times[checkpt_count][op_indx].at(6).push_back(total_time);
                // output.op_times[checkpt_count][op_indx].at(0).push_back(serialize_done_time-start_time);
                // output.op_times[checkpt_count][op_indx].at(1).push_back(send_done_time-serialize_done_time);
                // output.op_times[checkpt_count][op_indx].at(2).push_back(rec_return_msg_done_time-send_done_time);
                // output.op_times[checkpt_count][op_indx].at(3).push_back(end_time-rec_return_msg_done_time);
                // output.op_times[checkpt_count][op_indx].at(4).push_back(end_time-start_time);
                // // op_piece_times->at(0).push_back(serialize_done_time-start_time);
                // // op_piece_times->at(1).push_back(send_done_time-serialize_done_time);
                // // op_piece_times->at(2).push_back(rec_return_msg_done_time-send_done_time);
                // // op_piece_times->at(3).push_back(end_time-rec_return_msg_done_time);
                // // op_piece_times->at(4).push_back(end_time-start_time);
            }
        }
        else if (end_code == MD_CHECKPOINT_DATABASE_DONE) {
            if(!rdma) {
                send_time = send_done_time - start_time;
                wait_for_reply_time = rec_return_msg_done_time - send_done_time;
                deserialize_time = end_time - rec_return_msg_done_time;
                total_time = end_time - start_time;
            }
            else {
                error_log << "assumed debug run since CHECKPOINT_DATABASE op is RDMA" << endl;

                deserialize_time = end_time - start_time;
                total_time = serialize_time + send_time + wait_for_reply_time + rdma_init_time + rdma_wait_time + deserialize_time;
            
            }
            // debug_log << "in ops, total_time: " << total_time << endl;

            // output.op_times[checkpt_count][op_indx].at(0).push_back(serialize_done_time-start_time);
            output.op_times[checkpt_count][op_indx].at(1).push_back(send_time);
            output.op_times[checkpt_count][op_indx].at(2).push_back(wait_for_reply_time);
            // output.op_times[checkpt_count][op_indx].at(3).push_back(end_time-rec_return_msg_done_time);
            // output.op_times[checkpt_count][op_indx].at(4).push_back(end_time-start_time);
            output.op_times[checkpt_count][op_indx].at(5).push_back(deserialize_time);
            output.op_times[checkpt_count][op_indx].at(6).push_back(total_time);   
            // checkpt_count += 1;
            //we wait to increment the read count until it does the db checkpoint
            // if(checkpt_count < (output.config.num_timesteps / output.config.num_timesteps_per_checkpt - 1) ) {
            //  checkpt_count += 1;
            // }
        }
        else { //is full shutdown or transaction op, which use empty messages
            extreme_debug_log << "output.op_times[" << op_indx << "].size(): " << output.op_times[checkpt_count][op_indx].size() << endl;
            output.op_times[checkpt_count][op_indx].at(6).push_back(end_time-start_time);           
            // output.op_times[checkpt_count][op_indx].at(4).push_back(end_time-start_time);           
            // op_piece_times->at(4).push_back(end_time-start_time);
        }
    }
    else {
        error_log << "error. " << to_string(end_code) << " end time code never appeared after start time code. Instead code " << to_string(code) << " appeared \n";
    }   
    debug_log << "end ops" << endl;

}

static int local_server_storage(ifstream &file, struct client_config_output &output) {
    int rc;
    string line;
    uint16_t code;
    // while (true)
    // { 
    //     uint64_t len = file.tellg();
    //     std::getline(file, line);

    //     if( line.find("timestep:") == std::string::npos) {
    //         debug_log << "line: " << line << endl;
    //         file.seekg(len, std::ios_base::beg);
    //         break;
    //     }
    // }
    while (true)
    { 
        uint64_t len = file.tellg();
        std::getline(file, line);

        if( line.find("97") == 0) { //found the start of the storage sizes
            debug_log << "line: " << line << endl;
            file.seekg(len, std::ios_base::beg);
            break;
        }
    }
    file >> code;
    if(code != DB_SIZES) {
        error_log << "error. local client didn't start w the database sizes. code: " << code << endl;
        return RC_ERR;
    }
    extreme_debug_log << "about to evaluate_storage_sizes"<< endl;

    rc = evaluate_storage_sizes(file, output.server_output);
    if(rc != RC_OK) {
        error_log << "server storage size error \n";
        return RC_ERR;
    }
    extreme_debug_log << "done with evaluate_storage_sizes"<< endl;

    return RC_OK;
}

static void db_output(std::vector<uint16_t>::const_iterator &my_codes, std::vector<long double>::const_iterator &my_time_pts,
                std::vector<uint64_t>::const_iterator &my_storage_sizes,
                struct client_config_output &output, uint16_t db_output_start_code, 
                long double output_start_time, uint32_t checkpt_count,
                long double &end_time ) 
{

    uint16_t code = db_output_start_code;
    long double time_pt = output_start_time;

    uint64_t db_size = *my_storage_sizes;

    //extreme_debug_log << "db size: " << db_size << " db output start code: " << code << " time_pt: " << time_pt << endl;
    extreme_debug_log << "checkpt count: " << checkpt_count << " output.server_output.db_checkpoint_times.size(): " << output.server_output.db_checkpoint_times.size() << endl;

    while(code != db_output_start_code+1) {
        extreme_debug_log << "code: " << code << endl;
        extreme_debug_log << "code: " << code << " time_pt: " << time_pt << endl;
        my_codes++;
        my_time_pts++;
        code = *my_codes;
        time_pt = *my_time_pts;
    }
    extreme_debug_log << "code: " << code << " time_pt: " << time_pt << endl;
    end_time = time_pt;

    if(code != db_output_start_code+1) {
        error_log << "error. was expecting " << db_output_start_code+1 << " as my db output done code. instead saw " << code << endl;
    }
    else {
        int diff = (code - DB_CHECKPT_COPY_DONE);
        if(diff > 0) {
            diff -= 8; //there's a gap between DB_CHECKPT_COPY_START and the next checkpoint code
        }
        extreme_debug_log << "diff: " << diff << endl;

        md_db_checkpoint_type checkpt_type = (md_db_checkpoint_type)(diff / 2);

        extreme_debug_log << "checkpt_type: " << checkpt_type << endl;


            debug_log << "db_size: for checkpt_count: " << checkpt_count << " : " << db_size << endl;
            debug_log << "db_checkpoint_time: " << time_pt - output_start_time << endl;          
        // if(code == DB_OUTPUT_DONE) {
        if(checkpt_type != output.config.checkpt_type) {
            error_log << "error. Expecting checkpt_type: " << output.config.checkpt_type << " but instead saw op for: " << checkpt_type  << endl;
        }
        // output.all_db_checkpoint_times.at(checkpt_type).at(checkpt_count).push_back(time_pt - output_start_time);
        // output.all_db_checkpoint_sizes.at(checkpt_type).at(checkpt_count).push_back(db_size);
        extreme_debug_log << "output.server_output.db_checkpoint_times.size(): " << output.server_output.db_checkpoint_times.size() << endl;
        output.server_output.db_checkpoint_times.at(checkpt_count).push_back(time_pt - output_start_time);
        output.server_output.db_checkpoint_sizes.at(checkpt_count).push_back(db_size);
        //extreme_debug_log << "pushing back: " << (time_pt - output_start_time) << endl;

        //extreme_debug_log << "db size: " << db_size << " db output end code: " << code << " time_pt: " << time_pt << endl;
        my_storage_sizes++;
        debug_log << "db_size: " << db_size << endl;
        //extreme_debug_log << "db_checkpoint_time: " << time_pt - output_start_time << endl;
        // checkpt_count += 1;
        // db_output = true;
    }
}
