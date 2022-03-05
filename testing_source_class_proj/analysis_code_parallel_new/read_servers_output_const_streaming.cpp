



// #include "../../include/server/server_timing_constants.hh"
#include "../../include/server/server_timing_constants_new.hh"
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <queue>          // std::queue
#include <iomanip> //set precision
// #include <limits> //numeric_limits
#include "read_testing_output_const_streaming.hh"

using namespace std;

extern uint16_t NUM_SERVER_OPS;

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

int init(ifstream &file, uint16_t code, struct server_config_output &output, long double &start_time, long double &init_done, bool is_local=false);
static void ops(ifstream &file, uint16_t code, struct server_config_output &output, uint32_t &checkpt_count,
    vector<queue<double>> &pending_rdma_ops );
int shutdown(ifstream &file, uint16_t code, struct server_config_output &output, uint32_t checkpt_count, 
    long double &start_time, long double &shutdown_done, bool is_local=false);

static int evaluate_clock_times_server(ifstream &file, struct server_config_output &output);
int evaluate_storage_sizes(ifstream &file, struct server_config_output &output);
// static int evaluate_db_interaction_server(ifstream &file, struct server_config_output &output, uint16_t code);

static const uint16_t FIRST_ERR_CODE = ERR_ARGC;
static const uint16_t LAST_INIT_CODE = DIRMAN_SETUP_DONE_INIT_DONE;
static const uint16_t FIRST_OP_CODE = OP_ACTIVATE_RUN_START;
static const uint16_t LAST_TIMING_CODE = DB_CLOSED_SHUTDOWN_DONE; 
// static const uint16_t LAST_OP_CODE = OP_INSERT_VAR_ATTRIBUTE_BY_DIMS_BATCH_START + 49;
static const uint16_t LAST_OP_CODE = OP_COMMIT_TRANSACTION_START + 49;



static bool is_error(uint16_t code) {
    return code >= FIRST_ERR_CODE;
}

static bool is_op(uint16_t code) {
    return (FIRST_OP_CODE <= code && code < FIRST_ERR_CODE);
}

static bool is_db_load(uint16_t code) {
    return ( (DB_LOAD_START <= code && code <= DB_LOAD_DONE)); 
}

static bool is_create_writing_indices(uint16_t code) {
    return ( (DB_CREATE_WRITING_INDICES_START <= code && code <= DB_CREATE_WRITING_INDICES_DONE)); 
}
static bool is_create_reading_indices(uint16_t code) {
    return ( (DB_CREATE_READING_INDICES_START <= code && code <= DB_CREATE_READING_INDICES_DONE)); 
}
static bool is_create_indices(uint16_t code) {
    return (is_create_writing_indices(code) || is_create_reading_indices(code)); 
}
static bool is_db_compact(uint16_t code) {
    return ( DB_COMPACT_START == code || DB_COMPACT_DONE == code || DB_COMPACT_AND_OUTPUT_START == code || DB_CLOSED_SHUTDOWN_DONE == code); 
}

static bool is_db_compact_streaming(uint16_t code) {
    return ( DB_COMPACT_START == code || DB_COMPACT_DONE == code || DB_COMPACT_AND_OUTPUT_START == code || DB_CLOSED_SHUTDOWN_DONE == code); 
}

static bool is_init(uint16_t code) {
    return ( (PROGRAM_START <= code && code < LAST_INIT_CODE) || is_db_load(code) || is_create_indices(code) ); 
}

bool is_db_output(uint16_t code) {
    return ( (DB_CHECKPT_COPY_START <= code && code <= DB_CHECKPT_INCREMENTAL_OUTPUT_AND_RESET_DONE)); 
}


static bool is_db_compact_or_output(uint16_t code) {
    return ( (DB_COMPACT_AND_OUTPUT_START <= code && code <= DB_CHECKPT_INCREMENTAL_OUTPUT_AND_RESET_DONE)); 
}


bool is_shutdown(uint16_t code) {
    return ( (SHUTDOWN_START <= code && code < DB_CLOSED_SHUTDOWN_DONE) || is_create_indices(code)); 
}

// static bool post_shutdown_compact(server_config_output output)
// {
//  return (output.config.checkpt_type == DB_COPY || output.config.checkpt_type == DB_COPY_AND_DELETE || output.config.checkpt_type == DB_COPY_AND_RESET);
// }

// static bool post_shutdown_indices(server_config_output output, uint32_t checkpt_count)
// {
//  return (output.config.write_type == WRITE_DELAYED_INDEX && checkpt_count == 0);
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

int evaluate_all_servers(const string &file_path, struct server_config_output &output) {
    
    uint16_t code;
    int rc = RC_OK;
    ifstream file;
    int rank = -1;

    long double start_time;
    long double init_done;
    long double shutdown_start;
    long double shutdown_done;

    uint32_t checkpt_count = 0;

    vector<queue<double>> pending_rdma_ops(NUM_SERVER_OPS);

    extreme_debug_log << "got to the top of evaluate_all_servers \n";

    file.open(file_path);

    std::string line;   
    if(!file) {
        error_log << "error. failed to open file " << file_path << endl;
        return RC_ERR;
    }
    while (std::getline(file, line))
    {
        if( line.find("begin timing output") != std::string::npos) {
            break;
        }
    }

 //     std::string line; 
 //     int i = 0;          
 //    while (i < output.config.num_server_procs) {     
 //     std::getline(file, line);
    //     // if( line.find("<ERROR> [ibverbs_transport]") == std::string::npos) {          
    //     //     error_log << "line " << i << " for file path " << file_path << " does not contain <ERROR> [ibverbs_transport]. " <<
    //     //       "the line is: " << line << endl;
 //        //  }    
    //     i += 1;
    // }        

    while (file >> code) { 
        //extreme_debug_log << "code at top of while loop: " << code << endl;
        if(is_op(code)) {
            extreme_debug_log << "is op - code at top of while loop: " << to_string(code) << endl;
        }
        else { 
            debug_log << "is NOT op - code at top of while loop: " << to_string(code) << endl;
        }

        if(is_error(code)) {
            error_log << "error from in while loop of code: " << code << endl;
            // return 1;
        } 
        else if (code == DB_SIZES) {
            extreme_debug_log << "about to evaluate_storage_sizes"<< endl;
    
            rc = evaluate_storage_sizes(file, output);
            if(rc != RC_OK) {
                error_log << "server storage size error \n";
                goto cleanup;
            }
            extreme_debug_log << "done with evaluate_storage_sizes"<< endl;
        }
        else if (code == CLOCK_TIMES_NEW_PROC) {
            extreme_debug_log << "about to evaluate_clock_times_server"<< endl;

            rc = evaluate_clock_times_server(file, output);
            if(rc != RC_OK) {
                error_log << "server clock pts error \n";
                goto cleanup;
            }
            extreme_debug_log << "done with evaluate_clock_times_server"<< endl;

        }
        //proc is initing
        //all set up done at 4
        else if( (PROGRAM_START <= code && code <= LAST_INIT_CODE) ||  is_create_indices(code)) { 
        // else if( (PROGRAM_START <= code && code <= LAST_INIT_CODE) ) { 
        // else if(is_init(code)) {
            debug_log << "is_init. code: " << code << endl;
            debug_log << "initing" << endl;
            //new proc
            rank +=1;
            checkpt_count = 0;
            rc = init(file, code, output, start_time, init_done);
            if(rc != RC_OK) {
                goto cleanup;
            }
            debug_log << "done initing" << endl;
            
        }
        else if(LAST_INIT_CODE < code && code <= LAST_TIMING_CODE) {
            debug_log << "shutdown " << endl;
            shutdown(file, code, output, checkpt_count, shutdown_start, shutdown_done);

            output.run_times.push_back(shutdown_start - init_done);
            output.total_run_times.push_back(shutdown_done - start_time);

            debug_log << "done with shutdown" << endl;
    
            // if(rank == num_server_procs-1) {
            //  break;
            // }
            // file >> code;
            // error_log << "at end of shutdown again, code: " << code << endl;
            // return rc;
        }
        else if(is_op(code)) {
            extreme_debug_log << "about to ops"<< endl;

            if(code > LAST_OP_CODE) { //ignore, is a full shutdown code
                long double start_time;
                file >> start_time;
                continue;
            }
            ops(file, code, output, checkpt_count, pending_rdma_ops);         

            extreme_debug_log << "done with ops"<< endl;

        }
        // else if(is_db_load_or_output(code)) {
        //  rc = evaluate_db_interaction_server(file, output, code);

        // }
        else {
            error_log << "error. code " << code << " didn't fall into the category of error, or starting init, writing, or reading \n";
            return RC_ERR;
        }
    }
    debug_log << "done with analyze servers" << endl;

cleanup:
    for(int i = 0; i < pending_rdma_ops.size(); i++) {
        if(pending_rdma_ops[i].size() != 0) {
            error_log << "error. pending_rdma_ops[" << i << "].size(): " <<  pending_rdma_ops[i].size() << endl;
        }
    }


    file.close();
    return rc;
}


static void writing_index_init(ifstream &file, struct server_config_output &output, long double &start_time) {
    long double time_pt;
    uint16_t code;

    file >> code;
    file >> time_pt;

    if(code != DB_CREATE_WRITING_INDICES_DONE) { 
        error_log << "error. was expecting to see DB_CREATE_WRITING_INDICES_DONE (" << DB_CREATE_WRITING_INDICES_DONE << 
            ") after DB_CREATE_WRITING_INDICES_START (" << DB_CREATE_WRITING_INDICES_START << "). instead saw code: " << code << endl;
    }
    else {
        output.init_times.at(2).push_back(time_pt - start_time);
    }

}

static void reading_index_init(ifstream &file, struct server_config_output &output, long double &start_time) {
    long double time_pt;
    uint16_t code;

    file >> code;
    file >> time_pt;

    if(code != DB_CREATE_READING_INDICES_DONE) { 
        error_log << "error. was expecting to see DB_CREATE_READING_INDICES_DONE (" << DB_CREATE_READING_INDICES_DONE << 
            ") after DB_CREATE_READING_INDICES_START (" << DB_CREATE_READING_INDICES_START << "). instead saw code: " << code << endl;
    }
    else {
        output.init_times.at(3).push_back(time_pt - start_time);
    }

}

static void db_init(ifstream &file, struct server_config_output &output, long double &start_time, long double &end_time) {
    long double time_pt;
    uint16_t code;

    file >> code;
    file >> time_pt;

    if(code == DB_CREATE_WRITING_INDICES_START) {
        writing_index_init(file, output, time_pt);
        file >> code;
        file >> time_pt;
    }
    if(code == DB_CREATE_READING_INDICES_START) {
        reading_index_init(file, output, time_pt);
        file >> code;
        file >> time_pt;
    }
    if(code != DB_SETUP_DONE) { 
        error_log << "error. was expecting to see DB_SETUP_DONE (" << DB_SETUP_DONE << 
            ") after DB_SETUP_START (" << DB_SETUP_START << "). instead saw code: " << code << endl;
    }
    else {
        end_time = time_pt;
        // output.init_times.at(3).push_back(end_time - start_time);
        output.init_times.at(4).push_back(end_time - start_time);
    }

}



static void db_init(ifstream &file, struct server_config_output &output, long double &start_time) {
    long double time_pt;
    uint16_t code;

    file >> code;
    file >> time_pt;

    if(code == DB_CREATE_WRITING_INDICES_START) {
        writing_index_init(file, output, time_pt);
        file >> code;
        file >> time_pt;

    }
    if(code == DB_CREATE_READING_INDICES_START) {
        reading_index_init(file, output, time_pt);
        file >> code;
        file >> time_pt;
    }
    if(code != DB_SETUP_DONE) { 
        error_log << "error. was expecting to see DB_SETUP_DONE (" << DB_SETUP_DONE << 
            ") after DB_SETUP_START (" << DB_SETUP_START << "). instead saw code: " << code << endl;
    }
    else {
        output.init_times.at(4).push_back(time_pt - start_time);
    }

}

//Init - program_start .  server_setup_done_init_done
int init(ifstream &file, uint16_t code, struct server_config_output &output, long double &start_time, long double &init_done, bool is_db_init_only) {
    long double time_pt = start_time;

    int rc = RC_OK;

    // bool is_db_init_only = false; 
    if(code == PROGRAM_START) {
        file >> start_time; 
    }
    else if(is_db_init_only) {
     }
    else if(code == DB_CREATE_WRITING_INDICES_START || code == DB_SETUP_START) {
        is_db_init_only = true;     
        file >> start_time; 
    }
    else {
        error_log << "error, first init code wasn't program start. It was " << code << endl;
        return RC_ERR;
    }

    long double mpi_init_done_time;
    long double register_ops_done_time;
    long double db_setup_done_time;
    long double db_create_indices_start_time = 0;

    //note - as is the received code is Program_start
    while(is_init(code)) { 
        if(!is_db_init_only) {
            file >> code;
            file >> time_pt;
        }       
        extreme_debug_log << "init code: "  << to_string(code) << endl;
        switch(code) {
            case MPI_INIT_DONE:
                output.init_times.at(0).push_back(time_pt - start_time);
                mpi_init_done_time = time_pt;
                break;

            case REGISTER_OPS_DONE:
                output.init_times.at(1).push_back(time_pt - mpi_init_done_time);
                register_ops_done_time = time_pt;
                break;

            // case DB_CREATE_INDICES_START:
            //  index_init(file, output, time_pt);
            //  break;

            // case DB_CREATE_INDICES_DONE: {
            //  if(db_create_indices_start_time != 0) {
            //      output.init_times.at(2).push_back(time_pt - db_create_indices_start_time);
            //  }
            //  else {
            //      error_log << "error. saw DB_CREATE_INDICES_DONE but didn't see DB_CREATE_INDICES_START first" << endl;
            //  }
            //  break;
            // }
            case DB_CREATE_WRITING_INDICES_START:
                //extreme_debug_log << "is index_init" << endl;
                writing_index_init(file, output, time_pt);
                //extreme_debug_log << "just finished index_init" << endl;
                if(is_db_init_only && output.config.index_type==WRITE_DELAYED_INDEX) {
                    //extreme_debug_log << "is_db_init_only, so am returning" << endl;
                    return rc;
                }
                break;
    
            case DB_CREATE_READING_INDICES_START:
                //extreme_debug_log << "is index_init" << endl;
                writing_index_init(file, output, time_pt);
                //extreme_debug_log << "just finished index_init" << endl;
                if(is_db_init_only) {
                    //extreme_debug_log << "is_db_init_only, so am returning" << endl;
                    return rc;
                }
                break;
            case DB_SETUP_START:
                db_init(file, output, time_pt, db_setup_done_time);
                if(is_db_init_only) {
                    return rc;
                }
                break;

            // case DB_SETUP_DONE:
            //  output.init_times.at(3).push_back(time_pt - register_ops_done_time);
            //  db_setup_done_time = time_pt;
            //  break;
        }
    }
    if(code == DIRMAN_SETUP_DONE_INIT_DONE) {
        output.init_times.at(5).push_back(time_pt - db_setup_done_time);
        output.init_times.at(6).push_back(time_pt - start_time);
        init_done = time_pt;        
    }

    else if (is_error(code)) {
        error_log << "error code " << code << " received " << endl;
        return RC_ERR;
    }
    else {
        error_log << "error. code " << code << " received instead of init_done \n";
        return RC_ERR;
    }
    return rc;
}



static void ops(ifstream &file, uint16_t code, 
    struct server_config_output &output, uint32_t &checkpt_count,
    vector<queue<double>> &pending_rdma_ops
    ) 
{
    long double start_time;
    int op_indx = (code - 1000) / 100;
    bool rdma = (code % 10 == 9); //rdma get finished

    //extreme_debug_log << "code: " << code << endl;
    if(rdma) {
        code = code - 9;
        //extreme_debug_log  << "adj code: " << code << endl;
    }
    //extreme_debug_log << "pending_rdma_ops[" << op_indx << "].size(): "  << pending_rdma_ops[op_indx].size() << endl;

    file >> start_time;
  
    long double time_pt;

    uint16_t deserialze_code = code + 1;
    uint16_t db_code = code + 2;
    uint16_t serialize_code = code + 3;
    uint16_t create_msg_code = code + 4;
    uint16_t end_code = code + 5;

    uint16_t rdma_get_start_code = code + 8;
    uint16_t rdma_get_done_code = code + 9;

    long double deserialize_done_time = 0;
    long double db_done_time = 0;
    long double serialize_done_time = 0;
    long double create_msg_done_time = 0;
    long double end_time = 0;

    long double rdma_init_time = 0;
    long double rdma_wait_time = 0;

    bool db_output = false;

    if(rdma) {
        rdma_init_time = pending_rdma_ops[op_indx].front();
        pending_rdma_ops[op_indx].pop();
        rdma_wait_time = start_time - pending_rdma_ops[op_indx].front();
        pending_rdma_ops[op_indx].pop();
        //extreme_debug_log << "after popping, pending_rdma_ops[" << op_indx << "].size(): "  << pending_rdma_ops[op_indx].size() << endl;
    }

    // while(code < end_code && file >> code) {
    while(code != end_code && file >> code) {
        file >> time_pt;
        // extreme_debug_log << "code: " << code << endl;
        if(code == deserialze_code) {
            deserialize_done_time = time_pt;
        }
        else if(code == db_code) {
            db_done_time = time_pt;
        }
        else if(code == serialize_code) {
            serialize_done_time = time_pt;
        }
        else if(code == create_msg_code) {
            create_msg_done_time = time_pt;
        }
        else if(code == rdma_get_start_code) {
            pending_rdma_ops[op_indx].push(time_pt-start_time); //will be used to calculate total op time
            pending_rdma_ops[op_indx].push(time_pt); //will be used to calculate get wait time
            //extreme_debug_log << "after pushing, pending_rdma_ops[" << op_indx << "].size(): "  << pending_rdma_ops[op_indx].size() << endl;
            return;
        }
        //fix - do something with the breakdown of these?
        else if(is_db_output(code)) {
            uint16_t db_output_start_code = code;

            //debug_log << "output_start_time: " << time_pt << ", start code: " << code << endl;
            long double output_start_time = time_pt;
            uint64_t db_size;
            file >> db_size;

            file >> code;
            file >> time_pt;

            bool missing_time_pt;
            while(code != db_output_start_code+1 && file >> code) {
                file >> time_pt;
                //fixes bug where one checkpointing method returned before adding the timing point (now fixed)
                if(code == OP_CHECKPOINT_DATABASE_MD_CHECKPOINT_DATABASE_STUB) {
                    missing_time_pt = true;
                    // error_log << "found missing time pt" << endl;
                    code = db_output_start_code+1;
                    break;
                }
            }
            if(!missing_time_pt && code != db_output_start_code+1) {
                error_log << "error. was expecting " << db_output_start_code+1 << " as my db output done code" << endl;

            }
            // //fix - do something with the breakdown of these?
            // if(code == DB_SETUP_START) {
            //  // db_init(file, output, time_pt);

            //  // file >> code;
            //  // file >> time_pt;
            // }
            // else if(code == DB_RESET) {
            //  // file >> code;
            //  // file >> time_pt;
            //  while(code != DB_SETUP_START) {
            //      file >> code;
            //      file >> time_pt;
            //  }

                // if(code == DB_SETUP_START) {
                
                    // db_init(file, output, time_pt);

                    // file >> code;
                    // file >> time_pt;
                    // if(code != DB_SETUP_DONE) {
                    //  error_log << "error. was expecting DB_SETUP_DONE after db_init" << endl;
                    // }
                    // else {
                    //  file >> code;
                    //  file >> time_pt;        
                    // }
                // }
                // else {
                //  error_log << "error. was expecting DB_SETUP_START after DB_RESET" << endl;
                // }
                //fix - do I want to breakdown the components of this?
            // }
            // if(code != db_output_start_code+1) {
            //  error_log << "error. was expecting " << db_output_start_code+1 << " as my db output done code" << endl;
            // }
            else {
                int diff = (code - DB_CHECKPT_COPY_DONE);
                if(diff > 0) {
                    diff -= 8; //there's a gap between DB_CHECKPT_COPY_START and the next checkpoint code
                }
                md_db_checkpoint_type checkpt_type = (md_db_checkpoint_type)(diff / 2);

                    // debug_log << "db_size: for checkpt_count: " << checkpt_count << " : " << db_size << endl;
                    // debug_log << "db_checkpoint_time: " << time_pt - output_start_time << endl;          
                // if(code == DB_OUTPUT_DONE) {
                if(checkpt_type != output.config.checkpt_type) {
                    error_log << "error. Expecting checkpt_type: " << output.config.checkpt_type << " but instead saw op for: " << checkpt_type  << endl;
                }
                // output.all_db_checkpoint_times.at(checkpt_type).at(checkpt_count).push_back(time_pt - output_start_time);
                // output.all_db_checkpoint_sizes.at(checkpt_type).at(checkpt_count).push_back(db_size);
                output.db_checkpoint_times.at(checkpt_count).push_back(time_pt - output_start_time);
                output.db_checkpoint_sizes.at(checkpt_count).push_back(db_size);
                debug_log << "db_size: " << db_size << endl;
                debug_log << "db_checkpoint_time: " << time_pt - output_start_time << endl;
                // checkpt_count += 1;
                db_output = true;

                //debug_log << "output_end_time: " << time_pt << ", end code: " << code << endl;

            }
        }
    }   
    if (code == end_code) {
        end_time = time_pt;
        extreme_debug_log << "output.op_times[" << checkpt_count << "].size(): " << output.op_times[checkpt_count].size() << endl;
        extreme_debug_log << "output.op_times[" << checkpt_count << "][" << op_indx << "].size(): " << output.op_times[checkpt_count][op_indx].size() << endl;
        if(!db_output) {
            output.op_times[checkpt_count][op_indx].at(0).push_back(rdma_init_time); //will be 0 if not rdma
            output.op_times[checkpt_count][op_indx].at(1).push_back(rdma_wait_time); //will be 0 if not rdma
            output.op_times[checkpt_count][op_indx].at(2).push_back(deserialize_done_time-start_time);
            output.op_times[checkpt_count][op_indx].at(3).push_back(db_done_time-deserialize_done_time);
            output.op_times[checkpt_count][op_indx].at(4).push_back(serialize_done_time-db_done_time);
            output.op_times[checkpt_count][op_indx].at(5).push_back(create_msg_done_time-serialize_done_time);
            // output.op_times[checkpt_count][op_indx].at(6).push_back(end_time-db_done_time);
            output.op_times[checkpt_count][op_indx].at(6).push_back(end_time-create_msg_done_time);
            output.op_times[checkpt_count][op_indx].at(7).push_back(rdma_init_time + rdma_wait_time + end_time-start_time);

            // long double temp = end_time-start_time + rdma_init_time + rdma_wait_time;
            // debug_log << "rdma_init_time: " << rdma_init_time << " rdma_wait_time: " << rdma_wait_time << " end_time: " << end_time << 
            //         " start_time: " << start_time << " pushed back: " << temp << endl;
        }
        else {
            // output.op_times[checkpt_count][op_indx].at(0).push_back(deserialize_done_time-start_time);
            output.op_times[checkpt_count][op_indx].at(3).push_back(db_done_time-start_time);
            // output.op_times[checkpt_count][op_indx].at(1).push_back(db_done_time-start_time);
            // output.op_times[checkpt_count][op_indx].at(2).push_back(serialize_done_time-db_done_time);
            // output.op_times[checkpt_count][op_indx].at(3).push_back(create_msg_done_time-serialize_done_time);
            // output.op_times[checkpt_count][op_indx].at(4).push_back(end_time-db_done_time);
            // output.op_times[checkpt_count][op_indx].at(5).push_back(end_time-start_time);
            output.op_times[checkpt_count][op_indx].at(7).push_back(end_time-start_time);
        }
    }
    else {
        error_log << "error. " << end_code << " end time code never appeared after start time code. Instead code " << code << " appeared \n";
    }       

    //since the db output is part of the op we have a boundary issue for the last checkpt if
    //we incrememnt as soon as we see DB_OUTPUT_DONE (since the op hasn't finished yet)
    if(db_output) {
        //rank 0 might to a few final per-run ops at the testing, these should be treated as part of the last checkpoint
        if(checkpt_count < (output.config.num_timesteps / output.config.num_timesteps_per_checkpt - 1) ) {
            checkpt_count += 1;
        }
    }
    
}

int shutdown(ifstream &file, uint16_t code, struct server_config_output &output, uint32_t checkpt_count,
    long double &start_time, long double &shutdown_done, bool is_local) {
    uint16_t num_server_procs = output.config.num_server_procs;

    if(code != SHUTDOWN_START) {
        error_log << "error. first shutdown code was " << code << " instead of SHUTDOWN_START" << endl;
        return RC_ERR;
    }
    long double time_pt;
    if(!is_local) {
       file >> start_time;
    }

    time_pt = start_time;

    //extreme_debug_log << "code: " << code << " start_time: " << start_time << endl;
    debug_log << "start_time: " << start_time << endl;

    long double db_compact_start_time = 0;
    long double db_compact_and_output_start_time = 0;
    long double db_create_reading_indices_start_time = 0;

    bool exit = false;

    uint16_t prev_code;
    long double prev_time_pt;

    //todo - do I want to do anything with this?
    // while( (is_shutdown(code) || (post_shutdown_compact(output) && is_db_compact(code) ) || post_shutdown_indices(output, checkpt_count)) && !file.eof())
    while( !exit && (is_shutdown(code) || is_db_compact(code) ) && !file.eof())
    {
        prev_code = code;
        prev_time_pt = time_pt;

        int len = file.tellg();
        file >> code;
        file >> time_pt;
        // if(code == 0 && is_local) {
        if(code == 0) {
            file.seekg(len, std::ios_base::beg);
            code = prev_code;
            time_pt = prev_time_pt;
            break;
        }

        switch(code) {
            case DB_COMPACT_AND_OUTPUT_START:
                db_compact_and_output_start_time = time_pt;
                break;

            case DB_COMPACT_AND_OUTPUT_DONE:
                if(db_compact_and_output_start_time != 0) {
                    output.db_compact_times.at(1).push_back(time_pt - db_compact_and_output_start_time);
                }
                else {
                    error_log << "error. saw DB_COMPACT_AND_OUTPUT_DONE but didn't see DB_COMPACT_AND_OUTPUT_START first" << endl;
                }
                exit = true;
                break;              

            case DB_COMPACT_START:
                db_compact_start_time = time_pt;
                break;

            case DB_COMPACT_DONE: {
                if(db_compact_start_time != 0) {
                    output.db_compact_times.at(0).push_back(time_pt - db_compact_start_time);
                }
                else {
                    error_log << "error. saw DB_COMPACT_DONE but didn't see DB_COMPACT_START first" << endl;
                }
                break;
            }
            case DB_CREATE_READING_INDICES_START: {
                db_create_reading_indices_start_time = time_pt;
                file >> code;
                file >> time_pt;
                if(code != DB_CREATE_READING_INDICES_DONE)  {
                    error_log << "error. Was expecting DB_CREATE_INDICES_DONE after DB_CREATE_INDICES_START. Instead saw " << code << endl;
                    return RC_ERR;
                }
                output.init_times.at(3).push_back(time_pt - db_create_reading_indices_start_time);

                if(output.config.index_type == WRITE_DELAYED_INDEX && db_compact_and_output_start_time == 0) {
                    exit = true;
                }
            }
            // case DB_CREATE_INDICES_START: 
            //  db_create_indices_start_time = time_pt;
            //  break;

            // case DB_CREATE_INDICES_DONE: {
            //  if(db_create_indices_start_time != 0) {
            //extreme_debug_log << "time_pt: " << time_pt << " db_create_indices_start_time: " << db_create_indices_start_time << " dif: " << time_pt - db_create_indices_start_time << endl;
            //      output.init_times.at(2).push_back(time_pt - db_create_indices_start_time);
            //  }
            //  else {
            //      error_log << "error. saw DB_CREATE_INDICES_DONE but didn't see DB_CREATE_INDICES_START first" << endl;
            //  }
            //  break;
            // }
        }

        extreme_debug_log << "code: " << code << endl;
    }
    extreme_debug_log << "code at end of shutdown: " << code << endl;
    if( output.config.index_type == WRITE_DELAYED_INDEX && db_compact_and_output_start_time == 0  ) {
        if(db_create_reading_indices_start_time == 0) {
            error_log << "error. write type: WRITE_DELAYED_INDEX but DB_CREATE_READING_INDICES_START did not appear during shutdown" << endl;
        }
    }
    else if(code != DB_CLOSED_SHUTDOWN_DONE && db_compact_start_time==0) {
        error_log << "error. last shutdown code was " << code << " instead of DB_CLOSED_SHUTDOWN_DONE" << endl;
        return RC_ERR;
    }
    else if(code != DB_COMPACT_AND_OUTPUT_DONE && db_compact_start_time!=0) {
        error_log << "error. last shutdown code was " << code << " instead of DB_COMPACT_AND_OUTPUT_DONE" << endl;
        return RC_ERR;
    }
    shutdown_done = time_pt;

    output.shutdown_times.push_back(shutdown_done - start_time);
    return RC_OK;
}

int evaluate_storage_sizes(ifstream &file, struct server_config_output &output) {
    uint16_t num_server_storage_pts = output.config.num_storage_pts;
    uint16_t num_server_procs = output.config.num_server_procs;

    for(int i=0; i<num_server_procs; i++) {

        vector<uint64_t> storage_sizes;
        uint64_t storage_size;
        for(int j=0; j<num_server_storage_pts; j++) {
            file >> storage_size;
            extreme_debug_log << " storage pt " << j << ": " << storage_size << endl;
            output.all_storage_sizes.at(j).push_back(storage_size);

            // storage_sizes.push_back(storage_size);
        }
    }
    return RC_OK;
    // debug_log << "last size: " << storage_size << endl;
}

static int evaluate_clock_times_server(ifstream &file, struct server_config_output &output) {
    uint16_t num_clock_pts = output.config.num_clock_pts;
    uint16_t num_server_procs = output.config.num_server_procs;

    long double time_pt;
    uint16_t code = CLOCK_TIMES_NEW_PROC;
    long double earliest_start = 1000000;
    long double latest_init_done = 0;
    long double latest_shutdown_done = 0;
    long double earliest_init_done = 1000000;


    for(int i=0; i<num_server_procs; i++) {
        // debug_log << "i: " << i << endl;
        if(i != 0) {
            file >> code;
            if(code != CLOCK_TIMES_NEW_PROC) {
                error_log << "error. directly after server storage sizes was not the clock times. code: " << code << endl;
                return RC_ERR;
            }
        }
        for(int j=0; j<num_clock_pts; j++) {
            file >> time_pt;
            if(j==0) {
                if(time_pt < earliest_start) {
                    earliest_start = time_pt;
                }
            }   
            else if(j == (num_clock_pts-2) ) {
                if (time_pt > latest_init_done) {
                //the last time point is finalize
                    latest_init_done = time_pt;
                }
                if(time_pt < earliest_init_done) {
                    earliest_init_done = time_pt;
                }
            }
            else if(j == (num_clock_pts-1) && time_pt > latest_shutdown_done) {
                //the last time point is finalize
                latest_shutdown_done = time_pt;
            }
            extreme_debug_log << "time_pt: " << time_pt << endl;
            output.all_clock_times.at(j).push_back(time_pt);
        }
    }
    output.clock_times_eval.at(0).push_back(adjust(earliest_start,latest_init_done));
    output.clock_times_eval.at(1).push_back(adjust(earliest_init_done,latest_shutdown_done));   
    output.clock_times_eval.at(2).push_back(adjust(earliest_start,latest_shutdown_done));   

    file >> code;
    if(code != CLOCK_TIMES_DONE) {
        error_log << "error. last server clock code was " << code << " instead of CLOCK_TIMES_DONE" << endl;
        return RC_ERR;
    }
    debug_log << "last time_pt: " << time_pt << endl;
    return RC_OK;
}

// static int evaluate_db_interaction_server(ifstream &file, struct server_config_output &output, uint16_t code) {
//  long double start_pt, end_pt;
//  file >> start_pt;

//  if(code == DB_LOAD_START) {
//      file >> code;
//      if(code == DB_LOAD_DONE) {
//          file >> end_pt;
//          output.db_load_times.push_back(end_pt-start_pt);
//      }
//      else {
//          error_log << "error. was expecting DB_LOAD_DONE but instead saw " << code << endl;
//          return RC_ERR;
//      }

//  }
//  else if(code == DB_OUTPUT_START) {
//      file >> code;
//      if(code == DB_OUTPUT_DONE) {
//          file >> end_pt;
//          output.db_output_times.push_back(end_pt-start_pt);
//      }
//      else {
//          error_log << "error. was expecting DB_OUTPUT_DONE but instead saw " << code << endl;
//          return RC_ERR;
//      }

//  }
//  else {
//      error_log << "error. was expecting DB start pt but instead saw " << code << endl;
//      return RC_ERR;
//  }
//  return RC_OK;
// }


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void writing_index_init(std::vector<uint16_t>::const_iterator &my_codes, std::vector<long double>::const_iterator &my_time_pts, struct server_config_output &output, long double &start_time) {
    long double time_pt;
    uint16_t code;

    my_codes++;
    my_time_pts++;
    code = *my_codes;
    time_pt = *my_time_pts;

    if(code != DB_CREATE_WRITING_INDICES_DONE) { 
        error_log << "error. was expecting to see DB_CREATE_WRITING_INDICES_DONE (" << DB_CREATE_WRITING_INDICES_DONE << 
            ") after DB_CREATE_WRITING_INDICES_START (" << DB_CREATE_WRITING_INDICES_START << "). instead saw code: " << code << endl;
    }
    else {
        output.init_times.at(2).push_back(time_pt - start_time);
    }

}

static void reading_index_init(std::vector<uint16_t>::const_iterator &my_codes, std::vector<long double>::const_iterator &my_time_pts, 
    struct server_config_output &output, long double &start_time
    ) 
{
    long double time_pt;
    uint16_t code;

    my_codes++;
    my_time_pts++;
    code = *my_codes;
    time_pt = *my_time_pts;

    if(code != DB_CREATE_READING_INDICES_DONE) { 
        error_log << "error. was expecting to see DB_CREATE_READING_INDICES_DONE (" << DB_CREATE_READING_INDICES_DONE << 
            ") after DB_CREATE_READING_INDICES_START (" << DB_CREATE_READING_INDICES_START << "). instead saw code: " << code << endl;
    }
    else {
        output.init_times.at(3).push_back(time_pt - start_time);
    }

}

static void db_init(std::vector<uint16_t>::const_iterator &my_codes, std::vector<long double>::const_iterator &my_time_pts, 
    struct server_config_output &output, long double &start_time, long double &end_time
    ) 
{
    long double time_pt;
    uint16_t code;

    my_codes++;
    my_time_pts++;
    code = *my_codes;
    time_pt = *my_time_pts;


    if(code == DB_CREATE_WRITING_INDICES_START) {
        writing_index_init(my_codes, my_time_pts, output, time_pt);
        my_codes++;
        my_time_pts++;
        code = *my_codes;
        time_pt = *my_time_pts;

    }
    if(code == DB_CREATE_READING_INDICES_START) {
        reading_index_init(my_codes, my_time_pts, output, time_pt);
        my_codes++;
        my_time_pts++;
        code = *my_codes;
        time_pt = *my_time_pts;

    }
    if(code != DB_SETUP_DONE) { 
        error_log << "error. was expecting to see DB_SETUP_DONE (" << DB_SETUP_DONE << 
            ") after DB_SETUP_START (" << DB_SETUP_START << "). instead saw code: " << code << endl;
    }
    else {
        end_time = time_pt;
        // output.init_times.at(3).push_back(end_time - start_time);
        output.init_times.at(4).push_back(end_time - start_time);
    }

}



static void db_init(std::vector<uint16_t>::const_iterator &my_codes, std::vector<long double>::const_iterator &my_time_pts, 
    struct server_config_output &output, long double &start_time
    ) 
{
    long double time_pt;
    uint16_t code;

    my_codes++;
    my_time_pts++;
    code = *my_codes;
    time_pt = *my_time_pts;

    if(code == DB_CREATE_WRITING_INDICES_START) {
        writing_index_init(my_codes, my_time_pts, output, time_pt);
        my_codes++;
        my_time_pts++;
        code = *my_codes;
        time_pt = *my_time_pts;

    }
    if(code == DB_CREATE_READING_INDICES_START) {
        reading_index_init(my_codes, my_time_pts, output, time_pt);
        my_codes++;
        my_time_pts++;
        code = *my_codes;
        time_pt = *my_time_pts;
    }
    if(code != DB_SETUP_DONE) { 
        error_log << "error. was expecting to see DB_SETUP_DONE (" << DB_SETUP_DONE << 
            ") after DB_SETUP_START (" << DB_SETUP_START << "). instead saw code: " << code << endl;
    }
    else {
        output.init_times.at(4).push_back(time_pt - start_time);
    }

}

//Init - program_start .  server_setup_done_init_done
int init(std::vector<uint16_t>::const_iterator &my_codes, std::vector<long double>::const_iterator &my_time_pts, 
    uint16_t code, struct server_config_output &output, long double &start_time, long double &init_done, bool is_db_init_only) {
    long double time_pt = start_time;

    int rc = RC_OK;

    // bool is_db_init_only = false; 
    if(code == PROGRAM_START) {
        my_time_pts++;
        start_time = *my_time_pts;
    }
    else if(is_db_init_only) {
            // my_codes++;
            // my_time_pts++;
            // code = *my_codes;
            // time_pt = *my_time_pts;
     }
    else if(code == DB_CREATE_WRITING_INDICES_START || code == DB_SETUP_START) {
        is_db_init_only = true;     
        my_time_pts++;
        start_time = *my_time_pts;    
    }
    else {
        error_log << "error, first init code wasn't program start. It was " << code << endl;
        return RC_ERR;
    }

    long double mpi_init_done_time;
    long double register_ops_done_time;
    long double db_setup_done_time;
    long double db_create_indices_start_time = 0;

    //extreme_debug_log << "output.init_times: " << output.init_times << endl;
    //note - as is the received code is Program_start
    while(is_init(code)) { 
        extreme_debug_log << "code: " << code << endl;
        if(!is_db_init_only) {
            my_codes++;
            my_time_pts++;
            code = *my_codes;
            time_pt = *my_time_pts;
        }       
        switch(code) {
            case MPI_INIT_DONE:
                output.init_times.at(0).push_back(time_pt - start_time);
                mpi_init_done_time = time_pt;
                break;

            case REGISTER_OPS_DONE:
                output.init_times.at(1).push_back(time_pt - mpi_init_done_time);
                register_ops_done_time = time_pt;
                break;

            // case DB_CREATE_INDICES_START:
            //  index_init(file, output, time_pt);
            //  break;

            // case DB_CREATE_INDICES_DONE: {
            //  if(db_create_indices_start_time != 0) {
            //      output.init_times.at(2).push_back(time_pt - db_create_indices_start_time);
            //  }
            //  else {
            //      error_log << "error. saw DB_CREATE_INDICES_DONE but didn't see DB_CREATE_INDICES_START first" << endl;
            //  }
            //  break;
            // }
            case DB_CREATE_WRITING_INDICES_START:
                //extreme_debug_log << "is index_init" << endl;
                writing_index_init(my_codes, my_time_pts, output, time_pt);
                //extreme_debug_log << "just finished index_init" << endl;
                if(is_db_init_only && output.config.index_type==WRITE_DELAYED_INDEX) {
                    //extreme_debug_log << "is_db_init_only, so am returning" << endl;
                    return rc;
                }
                break;
    
            case DB_CREATE_READING_INDICES_START:
                //extreme_debug_log << "is index_init" << endl;
                writing_index_init(my_codes, my_time_pts, output, time_pt);
                //extreme_debug_log << "just finished index_init" << endl;
                if(is_db_init_only) {
                    //extreme_debug_log << "is_db_init_only, so am returning" << endl;
                    return rc;
                }
                break;
            case DB_SETUP_START:
                db_init(my_codes, my_time_pts, output, time_pt, db_setup_done_time);
                if(is_db_init_only) {
                    return rc;
                }
                break;

            // case DB_SETUP_DONE:
            //  output.init_times.at(3).push_back(time_pt - register_ops_done_time);
            //  db_setup_done_time = time_pt;
            //  break;
        }
    }
    if(code == DIRMAN_SETUP_DONE_INIT_DONE) {
        output.init_times.at(5).push_back(time_pt - db_setup_done_time);
        output.init_times.at(6).push_back(time_pt - start_time);
        init_done = time_pt;        
    }

    else if (is_error(code)) {
        error_log << "error code " << code << " received " << endl;
        return RC_ERR;
    }
    else {
        error_log << "error. code " << code << " received instead of init_done \n";
        return RC_ERR;
    }
    return rc;
}


int shutdown(std::vector<uint16_t>::const_iterator &my_codes, std::vector<long double>::const_iterator &my_time_pts,
    std::vector<uint16_t>::const_iterator my_end,
    uint16_t code, struct server_config_output &output, uint32_t checkpt_count,
    long double &start_time, long double &shutdown_done, bool is_local) {
    uint16_t num_server_procs = output.config.num_server_procs;

    if(code != SHUTDOWN_START) {
        error_log << "error. first shutdown code was " << code << " instead of SHUTDOWN_START" << endl;
        return RC_ERR;
    }
    //extreme_debug_log << "code: " << code << " start_time: " << start_time << endl;

    long double time_pt;
    if(!is_local) {
        my_time_pts++;
        start_time = *my_time_pts;
        //extreme_debug_log << "isn't local" << endl;
    }
    debug_log << "start_time: " << start_time << endl;

    time_pt = start_time;

    long double db_compact_start_time = 0;
    long double db_compact_and_output_start_time = 0;
    long double db_create_reading_indices_start_time = 0;

    bool exit = false;

    //todo - do I want to do anything with this?
    // while( (is_shutdown(code) || (post_shutdown_compact(output) && is_db_compact(code) ) || post_shutdown_indices(output, checkpt_count)) && !file.eof())
    while( !exit && (is_shutdown(code) || is_db_compact(code) ) && (my_codes != my_end) )
    {

        my_codes++;
        my_time_pts++;
        if(my_codes == my_end) {
            //extreme_debug_log << "breaking" << endl;
            break;
        }
        else if(*my_codes == PROGRAM_START) {
            my_codes--;
            my_time_pts--;
            extreme_debug_log << "break" << endl;
            break;
        }
        code = *my_codes;
        time_pt = *my_time_pts;
        extreme_debug_log << "code: " << code << ", time_pt: " << time_pt << endl;

        //extreme_debug_log << "code: " << code << " time_pt: " << time_pt << endl;
        switch(code) {
            case DB_COMPACT_AND_OUTPUT_START:
                db_compact_and_output_start_time = time_pt;
                break;

            case DB_COMPACT_AND_OUTPUT_DONE:
                if(db_compact_and_output_start_time != 0) {
                    output.db_compact_times.at(1).push_back(time_pt - db_compact_and_output_start_time);
                }
                else {
                    error_log << "error. saw DB_COMPACT_AND_OUTPUT_DONE but didn't see DB_COMPACT_AND_OUTPUT_START first" << endl;
                }
                exit = true;
                break;              

            case DB_COMPACT_START:
                db_compact_start_time = time_pt;
                break;

            case DB_COMPACT_DONE: {
                if(db_compact_start_time != 0) {
                    output.db_compact_times.at(0).push_back(time_pt - db_compact_start_time);
                }
                else {
                    error_log << "error. saw DB_COMPACT_DONE but didn't see DB_COMPACT_START first" << endl;
                }
                break;
            }
            case DB_CREATE_READING_INDICES_START: {
                db_create_reading_indices_start_time = time_pt;
                my_codes++;
                my_time_pts++;
                code = *my_codes;
                time_pt = *my_time_pts;
                if(code != DB_CREATE_READING_INDICES_DONE)  {
                    error_log << "error. Was expecting DB_CREATE_INDICES_DONE after DB_CREATE_INDICES_START. Instead saw " << code << endl;
                    return RC_ERR;
                }
                output.init_times.at(3).push_back(time_pt - db_create_reading_indices_start_time);

                if(output.config.index_type == WRITE_DELAYED_INDEX && db_compact_and_output_start_time == 0) {
                    exit = true;
                }
            }
            // case DB_CREATE_INDICES_START: 
            //  db_create_indices_start_time = time_pt;
            //  break;

            // case DB_CREATE_INDICES_DONE: {
            //  if(db_create_indices_start_time != 0) {
            //extreme_debug_log << "time_pt: " << time_pt << " db_create_indices_start_time: " << db_create_indices_start_time << " dif: " << time_pt - db_create_indices_start_time << endl;
            //      output.init_times.at(2).push_back(time_pt - db_create_indices_start_time);
            //  }
            //  else {
            //      error_log << "error. saw DB_CREATE_INDICES_DONE but didn't see DB_CREATE_INDICES_START first" << endl;
            //  }
            //  break;
            // }
        }

        extreme_debug_log << "code: " << code << endl;
    }

    //extreme_debug_log << "code at end of shutdown: " << code << endl;
    if( output.config.index_type == WRITE_DELAYED_INDEX && db_compact_and_output_start_time == 0  ) {
        if(db_create_reading_indices_start_time == 0) {
            error_log << "error. write type: WRITE_DELAYED_INDEX but DB_CREATE_READING_INDICES_START did not appear during shutdown" << endl;
        }
    }
    else if(code != DB_CLOSED_SHUTDOWN_DONE && db_compact_start_time==0) {
        error_log << "error. last shutdown code was " << code << " instead of DB_CLOSED_SHUTDOWN_DONE" << endl;
        return RC_ERR;
    }
    else if(code != DB_COMPACT_AND_OUTPUT_DONE && db_compact_start_time!=0) {
        error_log << "error. last shutdown code was " << code << " instead of DB_COMPACT_AND_OUTPUT_DONE" << endl;
        return RC_ERR;
    }
    shutdown_done = time_pt;

    output.shutdown_times.push_back(shutdown_done - start_time);
    return RC_OK;
}
