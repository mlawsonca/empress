
#include "../../include/client/md_client_timing_constants.hh"
#include "../../include/class_proj/client_timing_constants_read_class_proj.hh"
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <queue>
#include <iomanip> //set precision
// #include <limits> //numeric_limits
#include <map>
#include "read_testing_output_const_streaming.hh"
#include <numeric> //for accumulate function

using namespace std;

extern uint16_t NUM_CLIENT_OPS;

// enum RC
// {
//     RC_OK = 0,
//     RC_ERR = -1
// };
static bool extreme_debug_logging = false;
static bool debug_logging = false;
static bool error_logging = true;

static debugLog error_log = debugLog(error_logging);
static debugLog debug_log = debugLog(debug_logging);
static debugLog extreme_debug_log = debugLog(extreme_debug_logging);

static void init(std::vector<uint16_t>::const_iterator &my_codes, std::vector<long double>::const_iterator &my_time_pts, uint16_t code, struct read_client_config_output &output);
static void read_pattern(std::vector<uint16_t>::const_iterator &my_codes, std::vector<long double>::const_iterator &my_time_pts, int pattern, uint16_t code, 
        long double start_time, uint16_t end_code, struct read_client_config_output &output, vector<vector<long double>> &read_pattern_total_gather_times, 
        vector<vector<long double>> &read_pattern_total_op_times, vector<vector<long double>> &read_pattern_times, vector<queue<double>> &pending_rdma_ops);
static void ops(std::vector<uint16_t>::const_iterator &my_codes, std::vector<long double>::const_iterator &my_time_pts, 
    uint16_t code, long double start_time, struct read_client_config_output &output, vector<queue<double>> &pending_rdma_ops);
static void collective_ops(std::vector<uint16_t>::const_iterator &my_codes, std::vector<long double>::const_iterator &my_time_pts, 
    uint16_t code, long double start_time, struct read_client_config_output &output, long double &end_time, vector<queue<double>> &pending_rdma_ops);
static void collective_ops(std::vector<uint16_t>::const_iterator &my_codes, std::vector<long double>::const_iterator &my_time_pts, 
    uint16_t code, long double start_time, struct read_client_config_output &output, vector<queue<double>> &pending_rdma_ops);
static void shutdown(std::vector<uint16_t>::const_iterator &my_codes, std::vector<long double>::const_iterator &my_time_pts, int rank, uint16_t code, struct read_client_config_output &output);

// static int evaluate_all_clock_times_client(std::vector<uint16_t>::const_iterator &my_codes, std::vector<long double>::const_iterator &my_time_pts, struct read_client_config_output &output);
void add_last_first_timing(read_client_config_output &output);
void add_earliest_start(uint16_t code, long double start_time, read_client_config_output &output);
void add_latest_finish(uint16_t code, long double end_time, read_client_config_output &output);

static const uint16_t FIRST_ERR_CODE = ERR_ARGC;
static const uint16_t FIRST_OP_CODE = MD_ACTIVATE_RUN_START;
static const uint16_t FIRST_READ_PATTERN_CODE = READ_PATTERN_1_START;
// static const uint16_t LAST_TIMING_CODE = DIRMAN_SHUTDOWN_DONE;
// static const uint16_t LAST_TIMING_CODE = READING_DONE;

static const uint16_t FIRST_GATHER_CODE = GATHER_ATTR_ENTRIES_START;

// static const uint16_t FIRST_CODE_AFTER_OPS = BOUNDING_BOX_TO_OBJ_NAMES;
// static const uint16_t LAST_OP_CODE = MD_FULL_SHUTDOWN_DONE;
static const uint16_t LAST_OP_CODE = MD_COMMIT_TRANSACTION_START;

static bool is_error(uint16_t code) {
    return code >= FIRST_ERR_CODE;
}

static bool is_gather(uint16_t code) {
    return (FIRST_GATHER_CODE <= code && code < FIRST_OP_CODE);
}

static bool is_collective_read(uint16_t code) {
    return ( (code % 100 == 30) || (code % 100 == 31) || 
        (code % 100 == 40) || (code % 100 == 41) ||
        (code % 100 == 50) || (code % 100 == 51) );
}

static bool is_collective_start(uint16_t code) {
    return ( code > LAST_OP_CODE && code%10==0 && !is_collective_read(code) && !is_error(code) );
}

static bool is_collective_done(uint16_t code) {
    return ( code > LAST_OP_CODE && code%10==1 && !is_error(code) );
}

static bool is_op(uint16_t code) {
    return (FIRST_OP_CODE <= code && code <= LAST_OP_CODE);
}

static bool is_gather_or_op(uint16_t code) {
    return (is_op(code) || is_gather(code) );
}

static bool is_gather_start(uint16_t code) {
    return (is_gather(code) && (code % 2 == 0) );
}

// static bool is_gather_done(uint16_t code) {
//  return (code % 2 == 1);
// }

static bool is_read_pattern(uint16_t code) {
    return( FIRST_READ_PATTERN_CODE <= code && code < FIRST_OP_CODE );
}

static bool is_related_to_read_op(uint16_t code) {
    return ( (FIRST_OP_CODE < code && code < FIRST_ERR_CODE) );
}

static bool is_read_op(uint16_t code) {
    return ( (FIRST_OP_CODE < code && code <= LAST_OP_CODE) );
}

// static bool is_init(uint16_t code) {
//  return ( (PROGRAM_START <= code && code < SERVER_SETUP_DONE_INIT_DONE) );
// }

static bool is_init(uint16_t code) {
    return (  READING_INIT_START <= code && code <= FIND_VARS_DONE);

}

static bool is_reading(uint16_t code) {
    return ( (READING_START <= code && code < READING_DONE) || is_gather_or_op(code) 
               || is_read_pattern(code) || is_related_to_read_op(code) || is_init(code));
}
static bool is_local_run(uint32_t server_type) {
    return (server_type == SERVER_LOCAL_IN_MEM || server_type == SERVER_LOCAL_ON_DISK);
}

static bool is_local_run(read_client_config_output output) {
    return (is_local_run(output.config.server_type));
}
// static bool is_shutdown(uint16_t code) {
//  return ( (READING_DONE <= code && code < LAST_TIMING_CODE) || is_op(code));
// }



static long double adjust(long double start_time, long double end_time) {
    // if (end_time < start_time) {
    //  return (end_time + 3600 - start_time);
    //extreme_debug_log << "adjusting: start_time: " << start_time << " end_time: " << end_time << endl;
    // }
    // else {
        return end_time - start_time;
    // }
}

template <class T, class T2>
static void update_2D_stat(vector<T> &stats, vector<vector<T2>> &values )
{
    for(int i = 0; i < values.size(); i++) {
        update_stats_streaming(stats[i], values[i]);
    }
}

void read(std::vector<uint16_t>::const_iterator &my_codes, std::vector<long double>::const_iterator &my_time_pts, uint16_t init_code, double read_start, int rank, struct read_client_config_output &output) {
    // long double read_start;
    // long double reading_patterns_start;
    // long double read_type_patterns_start;
    long double extra_testing_start;
    uint16_t num_server_procs = output.config.num_server_procs;

    // file >> read_start;

    long double time_pt;
    uint16_t code = init_code;
    uint16_t start_code;

    vector<vector<long double>> read_pattern_total_gather_times = vector<vector<long double>>(output.read_pattern_total_gather_times.size());
    vector<vector<long double>> read_pattern_total_op_times = vector<vector<long double>>(output.read_pattern_total_op_times.size());
    vector<vector<long double>> read_pattern_times = vector<vector<long double>>(output.read_pattern_times.size());

    vector<queue<double>> pending_rdma_ops(NUM_CLIENT_OPS);

    // read_init(my_codes, my_time_pts, init_code, rank, output);

    bool servers_shutdown = false;

    long double read_all_type_patterns_time = 0;
    long double extra_testing_time;
    // long double read_all_patterns_time = 0;
    bool first = true;
    debug_log << "initial code received: " << init_code << endl;
    if(init_code == READING_START) {
        add_earliest_start(init_code, read_start, output);
        start_code = init_code;
    }


    while(is_reading(code)) {
        // it != my_vect.end()
        my_codes++;
        my_time_pts++;
        code = *my_codes;
        time_pt = *my_time_pts;
 
        extreme_debug_log << "in read, code: " << code << endl;

        // md_log("time_pt: " + to_string(time_pt));
        //read patterns
        debug_log << "in reading, code: " << code << endl;
        if(READING_START <= code && code <= READING_DONE) {
            switch (code) {
                case READING_START: {
                    add_earliest_start(code, read_start, output);
                    start_code = code;
                    break;
                }

                case READING_PATTERNS_DONE: {
                        extreme_debug_log << "done reading type patterns" << endl;
                        
                        for(int pattern = 1; pattern <= 6; pattern++) {
                            double sum_read_pattern_times = 0;
                            for(int type = 0; type < 3; type++) {
                                int pattern_indx = (pattern-1)*3 + type;
                                debug_log << "output.read_pattern_times[pattern_indx].size(): " << output.read_pattern_times[pattern_indx].size() << endl;

                                sum_read_pattern_times += read_pattern_times[pattern_indx].back();
                                debug_log << "output.read_pattern_times[pattern_indx].back(): " << read_pattern_times[pattern_indx].back() << endl;

                            }
                            debug_log << "pushing back " << sum_read_pattern_times << " as my total read time for the pattern" << endl;
                            output.total_read_patterns_times.at(pattern-1).push_back(sum_read_pattern_times);
                            read_all_type_patterns_time += sum_read_pattern_times;
                            extreme_debug_log << "got here 2\n";
                        }
                        break;
                    }

                case EXTRA_TESTING_START:
                    extra_testing_start = time_pt;
                    break;  

                case EXTRA_TESTING_DONE:
                    extreme_debug_log << "done with extra testing" << endl; 
                    extra_testing_time = time_pt - extra_testing_start;
                    output.sum_all_read_patterns_times.push_back(read_all_type_patterns_time);
                    output.extra_testing_times.push_back(extra_testing_time);
                    debug_log << "extra_testing_time: " << extra_testing_time << endl;
                    debug_log << "read_all_type_patterns_time: " << read_all_type_patterns_time << endl;
                    output.reading_times.push_back(read_all_type_patterns_time + extra_testing_time);   

                    break;

                case READING_DONE:
                    break;
            }
        }
        else if(is_init(code)) {
            //todo
            // while(code != FIND_VARS_DONE) {
            // // it != my_vect.end()
            // my_codes++;
            // my_time_pts++;
            // code = *my_codes;
            // time_pt = *my_time_pts;
            //extreme_debug_log << "in init, code: " << code << endl;
            // }
        }
        else if(is_read_pattern(code)) {
            debug_log << "is read pattern" << code << endl;
            int pattern = code/100;

            uint16_t end_code = code + 3;
            read_pattern(my_codes, my_time_pts, pattern, code, time_pt, end_code, output, read_pattern_total_gather_times, 
                read_pattern_total_op_times, read_pattern_times, pending_rdma_ops);
        }
        //ops
        // else if(is_read_op(code) && !is_collective_read(code) && !is_collective_start(code)) {
        else if(is_read_op(code)) {
            debug_log << "is op but not collective" << endl;
            uint16_t start_code = code;
        
            // // it != my_vect.end()
            // my_codes++;
            // my_time_pts++;
            // code = *my_codes;
            // time_pt = *my_time_pts;

            if(is_op(code)) {
                ops(my_codes, my_time_pts, code, time_pt, output, pending_rdma_ops);
                extreme_debug_log << "just returned from ops \n";
                // // it != my_vect.end()
                // my_codes++;
                // my_time_pts++;
                // code = *my_codes;
                // time_pt = *my_time_pts;
            }
            // else {
            //  error_log << "error. expecting op code received " << code << " instead" << endl;
            //  return;
            // }
            // if(is_read_op(code) && code % 2 == 1) {
            //  add_latest_finish(start_code, time_pt, output);
            // }
            // else {
            //  error_log << "error. was expecting a read finish code but received " << code << " instead" << endl;
            // }
        }
        else if(is_collective_start(code)) {
            debug_log << "is collective start" << endl;
            uint16_t collective_start_code = code;
            add_earliest_start(code, time_pt, output);

            long double collective_done_time;
            long double collective_start_time = time_pt;

            // it != my_vect.end()
            my_codes++;
            my_time_pts++;
            code = *my_codes;
            time_pt = *my_time_pts;

            if(!is_op(code) && !is_collective_done(code)) {
                error_log << "error. was expecting an op code (" << collective_start_code - 10000 <<
                             "), but instead saw: " << code << endl;
            }
            //not all the clients do the op read
            else if(is_op(code)) {
                while(is_op(code)) {
                    ops(my_codes, my_time_pts, code, time_pt, output, pending_rdma_ops);
                    // it != my_vect.end()
                    my_codes++;
                    my_time_pts++;
                    code = *my_codes;
                    time_pt = *my_time_pts;
                }
            }
            if(!is_collective_done(code)) {
            //  add_latest_finish(collective_start_code, time_pt, output);
            // }
            // else {
                error_log << "error. collective done did not appear after collective start. instead code: " << code << endl;
            }
            else {
                // it != my_vect.end()
                my_codes++;
                my_time_pts++;
                code = *my_codes;
                time_pt = *my_time_pts;
            }           
            if(is_collective_read(code)) {
                debug_log << "is collective read" << endl;
                // uint16_t collective_start_code = code;
                // add_earliest_start(code, time_pt, output);

                collective_ops(my_codes, my_time_pts, code, time_pt, output, collective_done_time, pending_rdma_ops);
                add_latest_finish(collective_start_code, collective_done_time, output);
                int op_indx = (collective_start_code - 11000) / 100;
                output.collective_op_times[op_indx].at(1).push_back(adjust(collective_start_time,collective_done_time));
            }
            else {
                error_log << "error. was expecting a collective start reading code (" << code + 30 <<
                             "), but instead saw: " << code << endl;
            }

            // if(is_collective_read(code)) {
            //  extreme_debug_log << "code: " << code << " is collective op" << endl;
            //  collective_ops(my_codes, my_time_pts, code, time_pt, output);
            // }
            // else {
            //  error_log << "error. was expecting a collective start reading code (" << collective_start_code + 30 <<
            //               "), but instead saw: " << code << endl;
            // }
            // else {
            //  error_log << "error. was expecting a collective done code (" << collective_start_code <<
            //               "), but instead saw: " << code << endl;
            // }

        }
        // else if(is_collective_read(code) && output.config.write_type == WRITE_HDF5) {
        //  collective_ops(my_codes, my_time_pts, code, time_pt, output);
        // }
        else if(is_error(code)) {
            error_log << "there was an error code: " << code << " for time_pt: " << time_pt << endl;
            // return;
        }
        else {
            error_log << "error. received code " << code << " which did not match one of readings expected categories \n";
        }
    }
    add_latest_finish(start_code, time_pt, output);

    update_2D_stat(output.read_pattern_total_gather_times, read_pattern_total_gather_times);
    update_2D_stat(output.read_pattern_total_op_times, read_pattern_total_op_times);
    update_2D_stat(output.read_pattern_times, read_pattern_times);

    debug_log << "done reading file" << endl;
    debug_log << "time_pt: " << time_pt << endl;
    // add_last_first_timing(output);
    // debug_log << "done adding last first timing" << endl;

    for(int i = 0; i < pending_rdma_ops.size(); i++) {
        if(pending_rdma_ops[i].size() != 0) {
            error_log << "error. in read client, pending_rdma_ops[" << i << "].size(): " <<  pending_rdma_ops[i].size() << endl;
        }
    }

}


static void read_pattern(std::vector<uint16_t>::const_iterator &my_codes, std::vector<long double>::const_iterator &my_time_pts, 
        int pattern, uint16_t code, long double start_time, uint16_t end_code, struct read_client_config_output &output, 
        vector<vector<long double>> &read_pattern_total_gather_times, vector<vector<long double>> &read_pattern_total_op_times, 
        vector<vector<long double>> &read_pattern_times, vector<queue<double>> &pending_rdma_ops
    )
{

    long double time_pt;
    uint16_t start_code = code;
    uint16_t type_start_code = code + 1;
    uint16_t type_done_code = code + 2;

    long double op_start_time;
    bool uses_op = false;

    int pattern_indx = (pattern - 1)*3;
    int plane_count = 0;

    // vector<long double> all_gather_times;
    // vector<long double> all_op_times;
    stats all_gather_times;
    stats all_op_times;


    bool firsts[] = {true, true, true};

    add_earliest_start(code, start_time, output);

    extreme_debug_log << "starting new pattern " << pattern_indx << endl;
    while(code != end_code && !is_error(code)) {
        extreme_debug_log << "code: " << code << endl;
        // it != my_vect.end()
        my_codes++;
        my_time_pts++;
        code = *my_codes;
        time_pt = *my_time_pts;
        extreme_debug_log << "code: " << code << endl;
        extreme_debug_log << "waiting for code: " << end_code << endl;

        if(is_op(code)) {
            op_start_time = time_pt;
            uses_op = true;
            if (is_collective_read(code)) {
                collective_ops(my_codes, my_time_pts, code, time_pt, output, pending_rdma_ops);
            }
            else {
                ops(my_codes, my_time_pts, code, time_pt, output, pending_rdma_ops);
            }
            extreme_debug_log << "just returned from ops in pattern " << pattern_indx << " \n";
        }

        else if (is_gather_start(code)) { 
            //starts gathering once the md query completes
            if (uses_op) {
                output.read_pattern_op_times.at(pattern_indx).push_back(adjust(op_start_time,time_pt));
                all_op_times.push_back(adjust(op_start_time,time_pt));
            }
            long double gather_start_time = time_pt;
            uint16_t gather_start_code = code;

            // it != my_vect.end()
            my_codes++;
            my_time_pts++;
            code = *my_codes;
            time_pt = *my_time_pts;
            if(code == (gather_start_code + 1) ) {
                output.read_pattern_gather_times.at(pattern_indx).push_back(adjust(gather_start_time,time_pt));
                all_gather_times.push_back(adjust(gather_start_time,time_pt));
            }
            else {
                error_log << "error. saw gather start code: " << gather_start_code << 
                        " but did not see gather end code. instead saw: " << code << endl;
                return;
            }
        }
        else if(code == type_done_code) {
            debug_log << "end code found. pattern: " << pattern << " pattern_indx: " << pattern_indx << endl;

            if(firsts[pattern_indx % 3]) { //first time adding stats for this pattern
                if(all_gather_times.size() > 0) {
                    //extreme_debug_log << "all_gather_times.size(): " << all_gather_times.size() << endl;
                    //extreme_debug_log << "read_pattern_total_gather_times.at(pattern_indx).size(): " << read_pattern_total_gather_times.at(pattern_indx).size() << endl;
                    //extreme_debug_log << "read_pattern_times.at(pattern_indx).size(): " << read_pattern_times.at(pattern_indx).size() << endl;

                    read_pattern_total_gather_times.at(pattern_indx).push_back(all_gather_times.sum());
                    debug_log << "read_pattern_total_gather_times.at(" << pattern_indx << ").back(): " << read_pattern_total_gather_times.at(pattern_indx).back() << endl;
                    debug_log << "all_gather accumulate: " << (all_gather_times.sum()) << endl;
                    all_gather_times.clear();
                }
                if(all_op_times.size() > 0) {
                    read_pattern_total_op_times.at(pattern_indx).push_back(all_op_times.sum());
                    debug_log << "read_pattern_total_op_times.at(" << pattern_indx << ").back(): " << read_pattern_total_op_times.at(pattern_indx).back() << endl;
                    debug_log << "all_gather accumulate: " << (all_op_times.sum()) << endl;
                    all_op_times.clear();
                }
                read_pattern_times.at(pattern_indx).push_back(adjust(start_time,time_pt));
                // extreme_debug_log << "all_gather_times.size() end: " << all_gather_times.size() << endl; 
                //extreme_debug_log << "read_pattern_total_gather_times.at(pattern_indx).size(): " << read_pattern_total_gather_times.at(pattern_indx).size() << endl;
                debug_log << "read_pattern_times.at(" << pattern_indx << ").back(): " << read_pattern_times.at(pattern_indx).back() << endl;
                firsts[pattern_indx%3] = false;
            }
            else {
                //extreme_debug_log << "read_pattern_total_gather_times.at(pattern_indx).size() start: " << read_pattern_total_gather_times.at(pattern_indx).size() << endl;
                if(all_gather_times.size() > 0) {
                    read_pattern_total_gather_times.at(pattern_indx).back() += (all_gather_times.sum());
                    debug_log << "read_pattern_total_gather_times.at(" << pattern_indx << ").back(): " << read_pattern_total_gather_times.at(pattern_indx).back() << endl;
                    debug_log << "all_gather accumulate: " << (all_gather_times.sum()) << endl;
                    all_gather_times.clear();
                }
                if(all_op_times.size() > 0) {
                    read_pattern_total_op_times.at(pattern_indx).back() += (all_op_times.sum());
                    debug_log << "read_pattern_total_op_times.at(" << pattern_indx << ").back(): " << read_pattern_total_op_times.at(pattern_indx).back() << endl;
                    debug_log << "all_op accumulate: " << (all_op_times.sum()) << endl;
                    all_op_times.clear();
                }
                read_pattern_times.at(pattern_indx).back() += (adjust(start_time,time_pt));
                //extreme_debug_log << "read_pattern_total_gather_times.at(pattern_indx).size() end: " << read_pattern_total_gather_times.at(pattern_indx).size() << endl;
                debug_log << "read_pattern_times.at(" << pattern_indx << ").back(): " << read_pattern_times.at(pattern_indx).back() << endl;
            }

            pattern_indx += 1;
            if(pattern == 1) {
                pattern_indx = pattern_indx % 3; //all 3 are done for 10 variables in turn
            }
            else if(pattern == 3) {
                pattern_indx = (pattern_indx % 3) + 3*(pattern-1);
            }
            else if(pattern == 4 || pattern == 6) {
                plane_count += 1;
                if(plane_count == 3) {
                    plane_count = 0;
                }
                else { //haven't moved on to the next type until all 3 planes have been read
                    pattern_indx -= 1;
                }
            }
            debug_log << "pattern: " << pattern << "pattern_indx: " << pattern_indx << " plane_count: " << plane_count << endl;
        }
        else if(code == end_code) {
            // if(pattern == 1) {
            //extreme_debug_log << "end code found. pattern: " << pattern << " pattern_indx: " << pattern_indx << endl;
            // if(read_pattern_times.at(pattern_indx).size() == 0) {
            //  if(all_gather_times.size() > 0) {
            //extreme_debug_log << "all_gather_times.size(): " << all_gather_times.size() << endl;
            //extreme_debug_log << "read_pattern_total_gather_times.size(): " << read_pattern_total_gather_times.size() << endl;
            //extreme_debug_log << "read_pattern_times.size(): " << read_pattern_times.size() << endl;

            //      read_pattern_total_gather_times.at(pattern_indx).push_back(std::accumulate(all_gather_times.begin(), all_gather_times.end(), 0.0));
            //      all_gather_times.clear();
            //  }
            //  read_pattern_times.at(pattern_indx).push_back(adjust(start_time,time_pt));
            //  extreme_debug_log << "all_gather_times.size() end: " << all_gather_times.size() << endl; 
            //  extreme_debug_log << "read_pattern_total_gather_times.at(pattern_indx).size(): " << read_pattern_total_gather_times.at(pattern_indx).size() << endl;
            // }
            // else {
            //  extreme_debug_log << "read_pattern_total_gather_times.at(pattern_indx).size() start: " << read_pattern_total_gather_times.at(pattern_indx).size() << endl;
            //  if(all_gather_times.size() > 0) {
            //      read_pattern_total_gather_times.at(pattern_indx).back() += (std::accumulate(all_gather_times.begin(), all_gather_times.end(), 0.0));
            //      all_gather_times.clear();
            //  }
            //  read_pattern_times.at(pattern_indx).back() += (adjust(start_time,time_pt));
            //  extreme_debug_log << "read_pattern_total_gather_times.at(pattern_indx).size() end: " << read_pattern_total_gather_times.at(pattern_indx).size() << endl;
            // }
        }
        else if(code > end_code) {
            error_log << "error in read pattern " << pattern << ". code: " << code << " and is not of expected type" << endl;
        }
        else if(code == type_start_code) {
            start_time = time_pt;
        }
    }
    if(code != end_code) {
        error_log << "error. pattern " << pattern << " end time code never appeared after start time code. Instead code " << code << " appeared \n";
    }   
    extreme_debug_log << "got to end of read pattern" << endl;

    add_latest_finish(start_code, time_pt, output);

    // extreme_debug_log << "read_pattern_total_gather_times.at(pattern_indx).size() end1: " << read_pattern_total_gather_times.at(pattern_indx).size() << endl;

    // if(all_gather_times.size() > 1) {
    //  read_pattern_total_gather_times.at(pattern_indx).push_back(std::accumulate(all_gather_times.begin(), all_gather_times.end(), 0.0));
    // }
    // extreme_debug_log << "read_pattern_total_gather_times.at(pattern_indx).size() end2: " << read_pattern_total_gather_times.at(pattern_indx).size() << endl;

}

//READ_PATTERN_1_START
//READ_PATTERN_1_DONE


static void ops(std::vector<uint16_t>::const_iterator &my_codes, std::vector<long double>::const_iterator &my_time_pts, 
    uint16_t code, long double start_time, struct read_client_config_output &output, vector<queue<double>> &pending_rdma_ops
    ) 
{
    long double time_pt;
    int op_indx = (code - 1000) / 100;
    bool rdma = (code % 10 == 9); //rdma get finished

    if(rdma) {
        code = code - 9; //normalizes
    }
    else {
        add_earliest_start(code, start_time, output); //note: may produce weird results for rdma
    }
    
    if(code % 100 != 0) {
        error_log << "error. was expecting a code divisible by 100 but received " << code << " instead" << endl;
        return;
    }

    long double serialize_done_time;
    long double send_done_time;
    long double rec_return_msg_done_time;
    long double end_time;

    long double rdma_get_start_time;

    long double serialize_time = 0;
    long double send_time = 0;
    long double wait_for_reply_time = 0;
    long double rdma_init_time = 0;
    long double rdma_wait_time = 0;
    long double deserialize_time = 0;
    long double total_time = 0;

    unsigned short start_code = code;
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

    extreme_debug_log << "end_code: " << to_string(end_code) << " note this is in extreme debug logging" << endl;
    // extreme_debug_log << "am debug logging \n";
    while(code != end_code && !is_error(code)) {
        extreme_debug_log << "code: " << code << endl;
        // it != my_vect.end()
        my_codes++;
        my_time_pts++;
        code = *my_codes;
        time_pt = *my_time_pts;

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

            serialize_time = adjust(start_time,serialize_done_time);
            send_time = adjust(serialize_done_time,send_done_time);
            wait_for_reply_time = adjust(send_done_time,rec_return_msg_done_time);
            rdma_init_time = adjust(rec_return_msg_done_time,rdma_get_start_time);

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
    extreme_debug_log << "code in ops: " << code << endl;
    if (code == end_code) {
        end_time = time_pt;
        extreme_debug_log << "is end code: " << code << endl;

        if(output.config.write_type == WRITE_HDF5 || is_local_run(output)) {            
            // output.op_times[op_indx].at(4).push_back(adjust(start_time,end_time));  
            output.op_times[op_indx].at(6).push_back(adjust(start_time,end_time));                  
        }
        else if(end_code != MD_FULL_SHUTDOWN_DONE && end_code != MD_CHECKPOINT_DATABASE_DONE) {
            if(!rdma) {
                serialize_time = adjust(start_time,serialize_done_time);
                send_time = adjust(serialize_done_time,send_done_time);
                wait_for_reply_time = adjust(send_done_time,rec_return_msg_done_time);
                deserialize_time = adjust(rec_return_msg_done_time,end_time);
                total_time = adjust(start_time,end_time);
            }
            else {
                deserialize_time = adjust(start_time,end_time);
                total_time = serialize_time + send_time + wait_for_reply_time + rdma_init_time + rdma_wait_time + deserialize_time;
            }

            output.op_times[op_indx].at(0).push_back(serialize_time);
            output.op_times[op_indx].at(1).push_back(send_time);
            output.op_times[op_indx].at(2).push_back(wait_for_reply_time);
            output.op_times[op_indx].at(3).push_back(rdma_init_time); //will be 0 if not rdma
            output.op_times[op_indx].at(4).push_back(rdma_wait_time); //will be 0 if not rdma
            output.op_times[op_indx].at(5).push_back(deserialize_time);
            // output.op_times[op_indx].at(3).push_back(adjust(rec_return_msg_done_time,end_time));
            // output.op_times[op_indx].at(4).push_back(adjust(start_time,end_time)); 

            output.op_times[op_indx].at(6).push_back(total_time);

            extreme_debug_log << "just finished pushing back " << endl;
        }
        else {
            output.op_times[op_indx].at(6).push_back(adjust(start_time,end_time));          
            // op_piece_times->at(4).push_back(end_time-start_time);
        }
    }
    else {
        error_log << "error. " << " end time code (" <<  to_string(end_code) << ") never appeared after start time code. Instead code " << to_string(code) << " appeared \n";
    }           
    extreme_debug_log << "got to bottom of ops" << endl;
    extreme_debug_log << "about to add latest finish for " << start_code << endl;
    add_latest_finish(start_code, end_time, output);
}

static void collective_ops(std::vector<uint16_t>::const_iterator &my_codes, std::vector<long double>::const_iterator &my_time_pts, 
    uint16_t code, long double start_time, struct read_client_config_output &output, vector<queue<double>> &pending_rdma_ops)  {
    long double end_time;
    collective_ops(my_codes, my_time_pts, code, start_time, output, end_time, pending_rdma_ops);

}


static void collective_ops(std::vector<uint16_t>::const_iterator &my_codes, std::vector<long double>::const_iterator &my_time_pts, 
    uint16_t code, long double start_time, struct read_client_config_output &output, long double &end_time, vector<queue<double>> &pending_rdma_ops) 
{
    long double time_pt;

    long double gather_start_time;
    long double gather_end_time;
    bool gathering = false;

    uint16_t start_code = code;
    unsigned short collective_done_code = code + 1;

    unsigned short gather_end_code;
    bool uses_gather = false;

    // unsigned short op_start_cde = code - 20;

    add_earliest_start(start_code, start_time, output);

    extreme_debug_log << "end_code: " << to_string(collective_done_code) << " note this is in extreme debug logging" << endl;
    // extreme_debug_log << "am debug logging \n";
    while(code != collective_done_code && !is_error(code)) {
        extreme_debug_log << "code: " << code << endl;
        // it != my_vect.end()
        my_codes++;
        my_time_pts++;
        code = *my_codes;
        time_pt = *my_time_pts;
        if( is_op(code) && !is_collective_read(code) ) {
            ops(my_codes, my_time_pts, code, time_pt, output, pending_rdma_ops);
        }
        else if ( is_gather(code) ) {
            uses_gather = true;
            if (gathering) {
                gather_end_time = time_pt;
                gathering = false;
            }
            else { //not gathering
                gather_start_time = time_pt;
                gather_end_code = code + 1;
                gathering = true;
            }

        }
        else if (code != collective_done_code){
            error_log << "error in collective ops. expecting op code or gather code and instead saw " << code << endl;
        }
    }
    if (gathering) {
        error_log << "error. saw gather start code but not end code " << endl;
    }

    extreme_debug_log << "code in ops: " << code << endl;
    int op_indx = (code - 11000) / 100;
    if (code == collective_done_code) {
        end_time = time_pt;
        extreme_debug_log << "is end code: " << code << endl;

        extreme_debug_log << "op_indx: " << op_indx << " output.collective_op_times.size(): " <<
                output.collective_op_times.size() <<
                " output.collective_op_times[op_indx].size(): " <<
                output.collective_op_times[op_indx].size() << endl;

        if(uses_gather) {
            //extreme_debug_log << "op: " << op_indx + 1000 << " end_time-start_time: " << end_time -start_time << endl;
            output.collective_op_times[op_indx].at(0).push_back(adjust(gather_start_time,gather_end_time));
            extreme_debug_log << "got here 1\n";
        }
        else {
            error_log << "error. didn't see a gather in collective op " << code << endl;
        }
        // if(output.config.write_type == WRITE_HDF5) {
        //  output.collective_op_times[op_indx].at(1).push_back(adjust(start_time,end_time));
        // }
        // extreme_debug_log << "just finished pushing back " << endl;

        add_latest_finish(start_code, time_pt, output);
    }
    else {
        error_log << "error. end time code (" << to_string(collective_done_code) << ") never appeared after gather start time code. Instead code " << to_string(code) << " appeared \n";
    }           
    extreme_debug_log << "got to bottom of ops" << endl;

}

void add_earliest_start(uint16_t code, long double start_time, read_client_config_output &output) {
    map<uint16_t,long double>::const_iterator i = output.earliest_starts.find(code);
    debug_log << "am adding earliest start for " << code << ": " << start_time << endl;

    if ( i == output.earliest_starts.end() || i->second > start_time) {
        output.earliest_starts[code] = start_time;
    }
}

void add_latest_finish(uint16_t code, long double end_time, read_client_config_output &output) {
    map<uint16_t,long double>::const_iterator i = output.latest_finishes.find(code);
    debug_log << "am adding latest finish for " << code << ": " << end_time << endl;

    if ( i == output.latest_finishes.end() || i->second < end_time) {
        output.latest_finishes[code] = end_time;
    }
}


void add_last_first_timing(read_client_config_output &output) {
    int indx = 0;
    bool first_file = false;
    if(output.clock_times_eval_catgs.size() == 0) {
        first_file = true;
    }
    for (std::map<uint16_t,long double>::const_iterator start_it=output.earliest_starts.begin(); start_it!=output.earliest_starts.end(); ++start_it) {
        debug_log << start_it->first << " => " << start_it->second << '\n'; 

        while(!first_file && start_it->first != output.clock_times_eval_catgs.at(indx) && start_it->first -10000 != output.clock_times_eval_catgs.at(indx)) {
            debug_log << "was expecting catg: " << output.clock_times_eval_catgs.at(indx) <<
                        " but instead saw: " << start_it->first << " so am incrementing indx" << endl;
            indx += 1;
        }

        map<uint16_t,long double>::const_iterator end_it = output.latest_finishes.find(start_it->first);
        if ( end_it != output.latest_finishes.end() ) {
            debug_log << "indx: " << indx << endl;
            extreme_debug_log << "for category: " << start_it->first << " earliest start: " << start_it->second << " latest finish: " << end_it->second  << endl;
            if(output.clock_times_eval.size() <= indx) {
                stats new_stat;
                output.clock_times_eval.push_back(new_stat);
            }
            extreme_debug_log << "output.clock_times_eval.size(): " << output.clock_times_eval.size() << " indx: " << indx << endl;
            output.clock_times_eval.at(indx).push_back(end_it->second - start_it->second);
            extreme_debug_log << "just pushed back: " << end_it->second - start_it->second << " for category " << start_it->first << endl;
            if (first_file) {
                if ( start_it->first > 10000) {
                    output.clock_times_eval_catgs.push_back(start_it->first - 10000);
                }
                else {
                    output.clock_times_eval_catgs.push_back(start_it->first);                               
                }
            }
            else {
                extreme_debug_log << "output.clock_times_eval_catgs.size(): " << output.clock_times_eval_catgs.size() << " indx: " << indx << endl;
                if (start_it->first != output.clock_times_eval_catgs.at(indx) && start_it->first -10000 != output.clock_times_eval_catgs.at(indx))  {
                    error_log << "error in add_last_first_timing" << endl;
                    error_log << "error. was expecting catg: " << output.clock_times_eval_catgs.at(indx) <<
                        " but instead saw: " << start_it->first << endl;
                }
                extreme_debug_log << "completed" << endl;
            }
            indx += 1;
        }
        else {
            error_log << "error. found code " << start_it->first << " as an earliest start but not a latest finish" << endl;
        }
    }
    output.earliest_starts.clear();
    output.latest_finishes.clear();
}
