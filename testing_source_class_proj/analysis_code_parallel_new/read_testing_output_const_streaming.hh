



#ifndef READTESTINGOUTPUTONEOUTPUT_HH
#define READTESTINGOUTPUTONEOUTPUT_HH

// #include <stdint.h>
#include <map>
#include "../../include/common/my_metadata_args.h"
#include <omp.h>
#include "stats_functions_const_streaming.hh"


enum PTS_OF_INTEREST_ENUM : unsigned short {
    TOTAL_RUN_TIME = 0,
    MPI_INIT_TIME = 1,
    DIRMAN_INIT_TIME = 2,
    SERVER_INIT_TIME = 3,
    TOTAL_INIT_TIME = 4,
    PER_RUN_CREATE_TIME = 5,
    CREATE_RUN_TIME = 6,
    CREATE_TYPES_TIME = 7,
    CREATE_VARS_TIME = 8,
    CREATE_VAR_ATTRS_TIME = 9,
    CREATE_COLLECTIVE_ATTRS_TIME = 10,
    GATHER_ATTRS_TIME = 11,
    CREATE_TIMESTEP_ATTRS_TIME = 12,
    TOTAL_TIMESTEP_CREATE_TIME = 13,
    TOTAL_TIMESTEP_ACTIVATE_TIME = 14,
    CREATE_RUN_ATTRS_TIME = 15,
    CHECKPOINT_DB_TIME = 16,

    COUNT = 17,
};

struct testing_config {
    uint16_t num_clock_pts;
    uint64_t num_server_procs;
    uint16_t num_server_nodes;
    uint64_t num_write_client_procs;
    uint16_t num_write_client_nodes;
    uint64_t num_read_client_procs;
    uint16_t num_read_client_nodes;
    uint64_t num_timesteps;
    uint32_t num_timesteps_per_checkpt;
    uint32_t num_checkpts;
    uint32_t write_type;
    uint32_t checkpt_type;
    uint32_t index_type;
    uint32_t server_type;

    // uint16_t num_types;
    uint16_t num_storage_pts; // just used by server (dedicated or local)
    bool do_read;
}; 

struct pt_of_interest {
    bool open;
    bool last_first;
    uint16_t start_code;
    uint16_t end_code;
    uint16_t id;
    // uint16_t second_end_code = 65535;

    bool diff_last_first_checkpt;
    std::string name;
    std::vector<std::vector<std::vector<long double>>> start_times;
    std::vector<std::vector<std::vector<long double>>> end_times;
    std::vector<std::vector<std::vector<long double>>> op_times;


    pt_of_interest ( uint16_t my_id, std::string point_name, uint16_t start, uint16_t end, uint32_t num_clients,
             uint32_t num_checkpts, bool last_frst = false, bool dif_last_first_checkpt = false) {
        id = my_id;
        open = false;
        start_code = start;
        end_code = end;
        name = point_name;
        last_first = last_frst;
        diff_last_first_checkpt = dif_last_first_checkpt;

        for (int i = 0; i < num_checkpts; i++) 
        {
            start_times.push_back(std::vector<std::vector<long double>>(num_clients));
            end_times.push_back(std::vector<std::vector<long double>>(num_clients));
            op_times.push_back(std::vector<std::vector<long double>>(num_clients));

        }
    }
};


struct server_config_output {

    testing_config config; 

    std::vector<std::string> filenames;

    //sections 
    std::vector<stats> init_times;
    stats run_times;
    stats shutdown_times;
    stats total_run_times;
    stats db_load_times;

    std::vector<stats> db_checkpoint_times;
    std::vector<stats_size> db_checkpoint_sizes;

    std::vector<stats> db_compact_times = std::vector<stats>(2);

    //vector of pts for each op for each of the 6 categories (op breakdown)
    std::vector<std::vector<std::vector<stats>>> op_times;

    //time of day
    std::vector<stats> all_clock_times;
    std::vector<stats> clock_times_eval;

    //db sizes and estimates
    std::vector<stats_size> all_storage_sizes;

    // server_stats stats;
};


struct dirman_config_output {
    testing_config config; 

    std::vector<std::string> filenames;

    std::vector<std::vector<double>> all_time_pts;

    std::vector<std::vector<long double>> clock_times_eval;
    std::vector<std::vector<long double>> all_clock_times;

};


struct read_client_config_output {
    testing_config config; 
    std::vector<std::string> filenames;

    stats sum_all_read_patterns_times;
    std::vector<stats> total_read_patterns_times;
    stats extra_testing_times;
    stats reading_times;

    //read patterns
    std::vector<stats> read_pattern_times;
    std::vector<stats> read_pattern_gather_times;
    std::vector<stats> read_pattern_op_times;
    std::vector<stats> read_pattern_total_gather_times;
    std::vector<stats> read_pattern_total_op_times;

    std::vector<std::vector<stats>> collective_op_times;
    std::vector<std::vector<stats>> op_times;

    std::map <uint16_t, long double> earliest_starts;
    std::map <uint16_t, long double> latest_finishes;

    std::vector<stats> clock_times_eval;
    std::vector<uint16_t> clock_times_eval_catgs;

};


struct client_config_output {
    testing_config config; 

    std::vector<std::string> filenames;

    std::vector<std::vector<stats>> per_proc_op_times;
    std::vector<std::vector<stats>> per_proc_times;
    std::vector<std::vector<stats>> last_first_times;
    std::vector<std::vector<std::vector<stats>>> op_times;


    std::vector<read_client_config_output> read_outputs;

    server_config_output server_output;

};



#endif //READTESTINGOUTPUTONEOUTPUT_HH