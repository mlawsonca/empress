#ifndef TESTING_CONFIGS_HH
#define TESTING_CONFIGS_HH

#include <vector>
#include "my_metadata_args.h"

struct testing_config {
    md_write_type write_type = WRITE_REG;
    md_db_checkpoint_type checkpt_type = DB_COPY;
    md_server_type server_type = SERVER_DEDICATED_IN_MEM;
    md_db_index_type index_type = WRITE_INDEX;
    bool do_read = false;
    std::vector<uint32_t> num_procs = {1000, 2000, 4000, 8000, 16000};
    std::vector<uint32_t> iterations = {5, 5, 5, 5, 5};

    testing_config() {

    }

    testing_config(md_db_checkpoint_type checkpt, bool rd=false) { 
        checkpt_type = checkpt;
        do_read = rd;
    }
    testing_config(md_server_type serv, bool rd=false) { 
        server_type = serv;
        do_read = rd;
    }
    testing_config(md_server_type serv, md_db_index_type indx, bool rd=false) { 
        server_type = serv;
        index_type = indx;
        do_read = rd;
    }
    testing_config(md_server_type serv, md_db_checkpoint_type checkpt, bool rd=false) { 
        server_type = serv;
        checkpt_type = checkpt;
        do_read = rd;
    }
    testing_config(md_server_type serv, md_db_checkpoint_type checkpt, md_db_index_type indx, bool rd=false) { 
        server_type = serv;
        checkpt_type = checkpt;
        index_type = indx;
        do_read = rd;
    }
    testing_config(md_db_index_type indx, bool rd=false) { 
        index_type = indx;
        do_read = rd;
    }
    testing_config(md_db_checkpoint_type checkpt, md_db_index_type indx, bool rd=false) { 
        checkpt_type = checkpt;
        index_type = indx;
        do_read = rd;
    }
    testing_config(md_write_type wrt, bool rd=false) { 
        write_type = wrt;
        do_read = rd;
    }
    testing_config(md_db_checkpoint_type checkpt, std::vector<uint32_t> procs, std::vector<uint32_t> iters, bool rd=false) { 
        checkpt_type = checkpt;
        do_read = rd;
        num_procs = procs;
        iterations = iters;
    }
    testing_config(md_server_type serv, std::vector<uint32_t> procs, std::vector<uint32_t> iters, bool rd=false) { 
        server_type = serv;
        do_read = rd;
        num_procs = procs;
        iterations = iters;
    }
    testing_config(md_server_type serv, md_db_index_type indx, std::vector<uint32_t> procs, std::vector<uint32_t> iters, bool rd=false) { 
        server_type = serv;
        index_type = indx;
        do_read = rd;
        num_procs = procs;
        iterations = iters;
    }
    testing_config(md_server_type serv, md_db_checkpoint_type checkpt, std::vector<uint32_t> procs, std::vector<uint32_t> iters, bool rd=false) { 
        server_type = serv;
        checkpt_type = checkpt;
        do_read = rd;
        num_procs = procs;
        iterations = iters;
    }
    testing_config(md_server_type serv, md_db_checkpoint_type checkpt, md_db_index_type indx, std::vector<uint32_t> procs, std::vector<uint32_t> iters, bool rd=false) { 
        server_type = serv;
        checkpt_type = checkpt;
        index_type = indx;
        num_procs = procs;
        iterations = iters;
        do_read = rd;
    }
    testing_config(md_db_index_type indx, std::vector<uint32_t> procs, std::vector<uint32_t> iters, bool rd=false) { 
        index_type = indx;
        do_read = rd;
        num_procs = procs;
        iterations = iters;
    }
    testing_config(md_db_checkpoint_type checkpt, md_db_index_type indx, std::vector<uint32_t> procs, std::vector<uint32_t> iters, bool rd=false) { 
        checkpt_type = checkpt;
        index_type = indx;
        do_read = rd;
        num_procs = procs;
        iterations = iters;
    }
    testing_config(md_write_type wrt, std::vector<uint32_t> procs, std::vector<uint32_t> iters, bool rd=false) { 
        write_type = wrt;
        do_read = rd;
        num_procs = procs;
        iterations = iters;
    }
};


std::vector<testing_config> get_testing_configs() {
    bool do_read = true;
    std::vector<testing_config> configs = {
        testing_config(DB_COPY, do_read),
        testing_config(DB_INCR_OUTPUT),
        testing_config(DB_COPY_AND_DELETE),
        testing_config(DB_COPY_AND_RESET),
        testing_config(DB_INCR_OUTPUT_AND_DELETE),
        testing_config(DB_INCR_OUTPUT_AND_RESET),
        testing_config(SERVER_DEDICATED_ON_DISK, do_read),
        // testing_config(SERVER_DEDICATED_ON_DISK, WRITE_DELAYED_INDEX, do_read),
        testing_config(SERVER_LOCAL_IN_MEM, DB_COPY, do_read),
        testing_config(SERVER_LOCAL_IN_MEM, DB_INCR_OUTPUT),
        testing_config(SERVER_LOCAL_IN_MEM, DB_COPY_AND_DELETE),
        testing_config(SERVER_LOCAL_IN_MEM, DB_COPY_AND_RESET),
        testing_config(SERVER_LOCAL_IN_MEM, DB_INCR_OUTPUT_AND_DELETE),
        testing_config(SERVER_LOCAL_IN_MEM, DB_INCR_OUTPUT_AND_RESET),
        testing_config(SERVER_LOCAL_IN_MEM, WRITE_DELAYED_INDEX, do_read),
        testing_config(SERVER_LOCAL_IN_MEM, DB_COPY_AND_DELETE, WRITE_DELAYED_INDEX),
        testing_config(SERVER_LOCAL_ON_DISK, do_read),
        // testing_config(SERVER_LOCAL_ON_DISK, WRITE_DELAYED_INDEX, do_read),
        testing_config(WRITE_DELAYED_INDEX, do_read),
        testing_config(DB_COPY_AND_DELETE, WRITE_DELAYED_INDEX),
        testing_config(WRITE_GATHERED),
        testing_config(WRITE_LARGE_MD_VOL, do_read),
        testing_config(WRITE_HDF5, do_read)
    };
    return configs;

}


std::vector<testing_config> get_specific_testing_configs() {
    bool do_read = true;
    // // cluster_e 1000-4000
    
    std::vector<testing_config> configs = {
        testing_config(DB_COPY, std::vector<uint32_t>{8000, 16000}, std::vector<uint32_t>{5, 5}, do_read),
        testing_config(DB_INCR_OUTPUT, std::vector<uint32_t>{8000, 16000}, std::vector<uint32_t>{5, 5}),
        testing_config(DB_COPY_AND_DELETE, std::vector<uint32_t>{8000, 16000}, std::vector<uint32_t>{5, 5}),
        testing_config(DB_COPY_AND_RESET, std::vector<uint32_t>{8000, 16000}, std::vector<uint32_t>{5, 5}),
        testing_config(DB_INCR_OUTPUT_AND_DELETE, std::vector<uint32_t>{8000, 16000}, std::vector<uint32_t>{5, 5}),
        testing_config(DB_INCR_OUTPUT_AND_RESET, std::vector<uint32_t>{8000, 16000}, std::vector<uint32_t>{5, 5}),
        testing_config(SERVER_DEDICATED_ON_DISK, std::vector<uint32_t>{8000, 16000}, std::vector<uint32_t>{5, 5}, do_read),
        // testing_config(SERVER_DEDICATED_ON_DISK, WRITE_DELAYED_INDEX, std::vector<uint32_t>{8000, 16000}, std::vector<uint32_t>{5, 5}, do_read),
        testing_config(SERVER_LOCAL_IN_MEM, DB_COPY, std::vector<uint32_t>{8000, 16000}, std::vector<uint32_t>{5, 5}, do_read),
        testing_config(SERVER_LOCAL_IN_MEM, DB_INCR_OUTPUT, std::vector<uint32_t>{8000, 16000}, std::vector<uint32_t>{5, 5}),
        testing_config(SERVER_LOCAL_IN_MEM, DB_COPY_AND_DELETE, std::vector<uint32_t>{8000, 16000}, std::vector<uint32_t>{5, 5}),
        testing_config(SERVER_LOCAL_IN_MEM, DB_COPY_AND_RESET, std::vector<uint32_t>{8000, 16000}, std::vector<uint32_t>{5, 5}),
        testing_config(SERVER_LOCAL_IN_MEM, DB_INCR_OUTPUT_AND_DELETE, std::vector<uint32_t>{8000, 16000}, std::vector<uint32_t>{5, 5}),
        testing_config(SERVER_LOCAL_IN_MEM, DB_INCR_OUTPUT_AND_RESET, std::vector<uint32_t>{8000, 16000}, std::vector<uint32_t>{5, 5}),
        testing_config(SERVER_LOCAL_IN_MEM, WRITE_DELAYED_INDEX, std::vector<uint32_t>{8000, 16000}, std::vector<uint32_t>{5, 5}, do_read),
        testing_config(SERVER_LOCAL_IN_MEM, DB_COPY_AND_DELETE, WRITE_DELAYED_INDEX, std::vector<uint32_t>{8000, 16000}, std::vector<uint32_t>{5, 5}),
        testing_config(SERVER_LOCAL_ON_DISK, std::vector<uint32_t>{8000, 16000}, std::vector<uint32_t>{5, 5}, do_read),
        // testing_config(SERVER_LOCAL_ON_DISK, WRITE_DELAYED_INDEX, std::vector<uint32_t>{8000, 16000}, std::vector<uint32_t>{5, 5}, do_read),
        testing_config(WRITE_DELAYED_INDEX, std::vector<uint32_t>{8000, 16000}, std::vector<uint32_t>{5, 5}, do_read),
        testing_config(DB_COPY_AND_DELETE, WRITE_DELAYED_INDEX, std::vector<uint32_t>{8000, 16000}, std::vector<uint32_t>{5, 5}, do_read),
        testing_config(WRITE_GATHERED, std::vector<uint32_t>{8000, 16000}, std::vector<uint32_t>{5, 5}),
        testing_config(WRITE_LARGE_MD_VOL, std::vector<uint32_t>{8000, 16000}, std::vector<uint32_t>{5, 5}, do_read),
        testing_config(WRITE_HDF5, std::vector<uint32_t>{8000, 16000}, std::vector<uint32_t>{5, 5}, do_read)
    };

    return configs;

}

std::vector<testing_config> get_new_testing_configs( uint32_t num_procs, uint32_t num_iterations) {
    bool do_read = true;
    std::vector<testing_config> configs = {
        testing_config(SERVER_DEDICATED_SQLITE_TRANSACTION_MANAGEMENT_WAL, DB_COPY, std::vector<uint32_t>{num_procs}, std::vector<uint32_t>{num_iterations}, do_read),
        testing_config(SERVER_DEDICATED_SQLITE_TRANSACTION_MANAGEMENT_DB_STREAMS, DB_COPY, std::vector<uint32_t>{num_procs}, std::vector<uint32_t>{num_iterations}, do_read),
        testing_config(SERVER_DEDICATED_D2T_TRANSACTION_MANAGEMENT, DB_COPY, std::vector<uint32_t>{num_procs}, std::vector<uint32_t>{num_iterations}, do_read),
        testing_config(SERVER_DEDICATED_IN_MEM, DB_COPY, INDEX_RTREE, std::vector<uint32_t>{num_procs}, std::vector<uint32_t>{num_iterations}, do_read),
    };
    return configs;

}

std::vector<testing_config> get_new_testing_configs() {
    bool do_read = true;
    std::vector<testing_config> configs = {
        testing_config(SERVER_DEDICATED_SQLITE_TRANSACTION_MANAGEMENT_WAL, DB_COPY, do_read),
        testing_config(SERVER_DEDICATED_SQLITE_TRANSACTION_MANAGEMENT_DB_STREAMS, DB_COPY, do_read),
        testing_config(SERVER_DEDICATED_D2T_TRANSACTION_MANAGEMENT, DB_COPY, do_read),
        testing_config(SERVER_DEDICATED_IN_MEM, DB_COPY, INDEX_RTREE, do_read),
    };
    return configs;

}

std::vector<testing_config> get_specific_new_testing_configs() {
    bool do_read = true;
    std::vector<testing_config> configs = {
        testing_config(SERVER_DEDICATED_SQLITE_TRANSACTION_MANAGEMENT_WAL, DB_COPY, std::vector<uint32_t>{8000, 16000}, std::vector<uint32_t>{5, 5}, do_read),
        testing_config(SERVER_DEDICATED_SQLITE_TRANSACTION_MANAGEMENT_DB_STREAMS, DB_COPY, std::vector<uint32_t>{8000, 16000}, std::vector<uint32_t>{5, 5}, do_read),
        testing_config(SERVER_DEDICATED_D2T_TRANSACTION_MANAGEMENT, DB_COPY, std::vector<uint32_t>{8000, 16000}, std::vector<uint32_t>{5, 5}, do_read),
        testing_config(SERVER_DEDICATED_IN_MEM, DB_COPY, INDEX_RTREE, std::vector<uint32_t>{8000, 16000}, std::vector<uint32_t>{5, 5}, do_read),
    };
    return configs;

}


std::vector<testing_config> get_completed_testing_configs(std::string cluster) {
    bool do_read = true;
    std::vector<testing_config> configs;
    if(cluster == "cluster_e") {
        configs = {
            //analysis finished
            testing_config(DB_COPY, std::vector<uint32_t>{1000,2000,4000,8000}, std::vector<uint32_t> {}),
            testing_config(DB_INCR_OUTPUT, std::vector<uint32_t>{8000}, std::vector<uint32_t> {}),
            testing_config(DB_COPY_AND_DELETE, std::vector<uint32_t>{8000}, std::vector<uint32_t> {}),
            testing_config(DB_INCR_OUTPUT_AND_DELETE, std::vector<uint32_t>{8000}, std::vector<uint32_t> {}),
            testing_config(DB_INCR_OUTPUT_AND_RESET, std::vector<uint32_t>{8000}, std::vector<uint32_t> {}),
            testing_config(SERVER_DEDICATED_ON_DISK, std::vector<uint32_t>{8000}, std::vector<uint32_t> {}),
            testing_config(WRITE_DELAYED_INDEX, std::vector<uint32_t>{8000}, std::vector<uint32_t> {}),
            testing_config(DB_COPY_AND_DELETE, WRITE_DELAYED_INDEX, std::vector<uint32_t>{8000}, std::vector<uint32_t> {}),
            testing_config(SERVER_DEDICATED_D2T_TRANSACTION_MANAGEMENT, DB_COPY, std::vector<uint32_t>{8000}, std::vector<uint32_t> {}),
            testing_config(SERVER_LOCAL_IN_MEM, DB_COPY, std::vector<uint32_t>{8000}, std::vector<uint32_t> {}),
            testing_config(SERVER_LOCAL_IN_MEM, DB_INCR_OUTPUT, std::vector<uint32_t>{8000}, std::vector<uint32_t> {}),
            testing_config(SERVER_LOCAL_IN_MEM, DB_COPY_AND_DELETE, std::vector<uint32_t>{8000}, std::vector<uint32_t> {}),
            testing_config(SERVER_LOCAL_IN_MEM, DB_COPY_AND_RESET, std::vector<uint32_t>{8000}, std::vector<uint32_t> {}),
            testing_config(SERVER_LOCAL_IN_MEM, DB_INCR_OUTPUT_AND_DELETE, std::vector<uint32_t>{8000}, std::vector<uint32_t> {}),
            testing_config(SERVER_LOCAL_IN_MEM, DB_INCR_OUTPUT_AND_RESET, std::vector<uint32_t>{8000}, std::vector<uint32_t> {}),
            testing_config(SERVER_LOCAL_IN_MEM, WRITE_DELAYED_INDEX, std::vector<uint32_t>{8000}, std::vector<uint32_t> {}),
            testing_config(SERVER_LOCAL_IN_MEM, DB_COPY_AND_DELETE, WRITE_DELAYED_INDEX, std::vector<uint32_t>{8000}, std::vector<uint32_t> {}),
            testing_config(SERVER_LOCAL_ON_DISK, std::vector<uint32_t>{8000}, std::vector<uint32_t> {}),
            testing_config(DB_COPY_AND_RESET, std::vector<uint32_t>{8000}, std::vector<uint32_t> {}),
            testing_config(WRITE_GATHERED, std::vector<uint32_t>{8000}, std::vector<uint32_t> {}),
            testing_config(SERVER_DEDICATED_SQLITE_TRANSACTION_MANAGEMENT_WAL, DB_COPY, std::vector<uint32_t>{8000}, std::vector<uint32_t> {}),
            testing_config(SERVER_DEDICATED_IN_MEM, DB_COPY, INDEX_RTREE, std::vector<uint32_t>{8000}, std::vector<uint32_t> {}),
            testing_config(SERVER_DEDICATED_SQLITE_TRANSACTION_MANAGEMENT_DB_STREAMS, DB_COPY, std::vector<uint32_t>{8000}, std::vector<uint32_t> {}),

            //not doing
            // testing_config(WRITE_LARGE_MD_VOL, std::vector<uint32_t>{8000}, std::vector<uint32_t> {}), 
            // testing_config(WRITE_HDF5, std::vector<uint32_t>{1000, 2000, 4000, 8000}, std::vector<uint32_t> {}),

        };
    }
    else if (cluster == "cluster_d") {
        configs = {
            //analysis finished
            testing_config(DB_COPY, std::vector<uint32_t>{1000,2000,4000,8000}, std::vector<uint32_t> {}),
            testing_config(DB_INCR_OUTPUT, std::vector<uint32_t>{8000}, std::vector<uint32_t> {}),
            testing_config(DB_COPY_AND_DELETE, std::vector<uint32_t>{8000}, std::vector<uint32_t> {}),
            testing_config(DB_COPY_AND_RESET, std::vector<uint32_t>{8000}, std::vector<uint32_t> {}),
            testing_config(DB_INCR_OUTPUT_AND_DELETE, std::vector<uint32_t>{8000}, std::vector<uint32_t> {}),
            testing_config(DB_INCR_OUTPUT_AND_RESET, std::vector<uint32_t>{8000}, std::vector<uint32_t> {}),
            testing_config(SERVER_DEDICATED_ON_DISK, std::vector<uint32_t>{8000}, std::vector<uint32_t> {}),
            testing_config(WRITE_DELAYED_INDEX, std::vector<uint32_t>{8000}, std::vector<uint32_t> {}),
            testing_config(DB_COPY_AND_DELETE, WRITE_DELAYED_INDEX, std::vector<uint32_t>{8000}, std::vector<uint32_t> {}),
            testing_config(SERVER_DEDICATED_D2T_TRANSACTION_MANAGEMENT, DB_COPY, std::vector<uint32_t>{8000}, std::vector<uint32_t> {}),
            testing_config(SERVER_LOCAL_IN_MEM, DB_COPY, std::vector<uint32_t>{8000}, std::vector<uint32_t> {}),
            testing_config(SERVER_LOCAL_IN_MEM, DB_INCR_OUTPUT, std::vector<uint32_t>{8000}, std::vector<uint32_t> {}),
            testing_config(SERVER_LOCAL_IN_MEM, DB_COPY_AND_DELETE, std::vector<uint32_t>{8000}, std::vector<uint32_t> {}),
            testing_config(SERVER_LOCAL_IN_MEM, DB_INCR_OUTPUT_AND_DELETE, std::vector<uint32_t>{8000}, std::vector<uint32_t> {}),
            testing_config(SERVER_LOCAL_IN_MEM, DB_INCR_OUTPUT_AND_RESET, std::vector<uint32_t>{8000}, std::vector<uint32_t> {}),
            testing_config(SERVER_LOCAL_IN_MEM, WRITE_DELAYED_INDEX, std::vector<uint32_t>{8000}, std::vector<uint32_t> {}),
            testing_config(SERVER_LOCAL_IN_MEM, DB_COPY_AND_DELETE, WRITE_DELAYED_INDEX, std::vector<uint32_t>{8000}, std::vector<uint32_t> {}),
            testing_config(SERVER_LOCAL_ON_DISK, std::vector<uint32_t>{8000}, std::vector<uint32_t> {}),            

            testing_config(SERVER_LOCAL_IN_MEM, DB_COPY_AND_RESET, std::vector<uint32_t>{8000}, std::vector<uint32_t> {}),
            testing_config(WRITE_GATHERED, std::vector<uint32_t>{8000}, std::vector<uint32_t> {}),
            testing_config(SERVER_DEDICATED_SQLITE_TRANSACTION_MANAGEMENT_WAL, DB_COPY, std::vector<uint32_t>{8000}, std::vector<uint32_t> {}),
            testing_config(SERVER_DEDICATED_IN_MEM, DB_COPY, INDEX_RTREE, std::vector<uint32_t>{8000}, std::vector<uint32_t> {}),
            testing_config(SERVER_DEDICATED_SQLITE_TRANSACTION_MANAGEMENT_DB_STREAMS, DB_COPY, std::vector<uint32_t>{8000}, std::vector<uint32_t> {}),

            //not doing 
            // testing_config(WRITE_LARGE_MD_VOL, std::vector<uint32_t>{8000}, std::vector<uint32_t> {}),
            // testing_config(WRITE_HDF5, std::vector<uint32_t>{1000, 2000, 4000, 8000}, std::vector<uint32_t> {}),

   

        };
    }

 
    return configs;

}



#endif //TESTING_CONFIGS_HH