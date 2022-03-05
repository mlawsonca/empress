
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <functional>
#include <iostream>
#include <vector>
#include <string>
#include <numeric>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <map>
#include <math.h>
#include "read_testing_output_const_streaming.hh"
#include "../../include/client/md_client_timing_constants.hh"
// #include "stats_functions_const_streaming.hh"
#include <stdio.h>
#include <set>

#include <unistd.h> //needed for gethostname
#include <sys/time.h> // needed for gettimeofday


using namespace std;

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
// static debugLog extreme_debug_log = debugLog(extreme_debug_logging);

uint16_t NUM_CLOCK_PTS_CLIENT = 10;
uint16_t NUM_CLOCK_PTS_SERVER = 6; 
uint16_t NUM_CLOCK_PTS_DIRMAN = 6; 

uint16_t NUM_THREADS = 33;
// uint16_t NUM_THREADS = 1;

float DEC_PERCENT_CLIENT_PROCS_USED_TO_READ = .1; 
uint16_t NUM_SERVER_STORAGE_PTS = 9; 


uint16_t NUM_OPS = 79; //without the gaps
uint16_t NUM_CLIENT_OPS = 90; //really only 79 but we left some gaps
// uint16_t NUM_SERVER_OPS = NUM_CLIENT_OPS-1;
uint16_t NUM_SERVER_OPS = NUM_CLIENT_OPS; //doens't include timing for shutdown, but includes transaction ops 

uint16_t NUM_CLIENT_OP_TIMING_PIECES = 7; //includes rdma (used to be 5)
uint16_t NUM_SERVER_OP_TIMING_PIECES = 8; //includes rdma (used to be 6)

// uint16_t num_server_ops = 76;

template <class T1, class T2>
static void get_config_and_stats(ofstream &file, T1 output, T2 stats, bool print_times_per_proc_and_run, bool div_pow=false, bool suppress_endl=false);

int evaluate_all_servers(const string &file_path, struct server_config_output &output);
int evaluate_all_write_clients(const string &file_path, struct client_config_output &output);
int evaluate_all_read_clients(const string &file_path, struct client_config_output &output);
int evaluate_dirman(const string &file_path, struct dirman_config_output &output);

static int write_output_write_client(const string &results_path, bool first_run_clients, const struct client_config_output &output);
static int write_output_read_client(const string &results_path, bool first_run_clients, const struct read_client_config_output &output, int checkpt_count);
static int write_output_server(const string &results_path, bool first_run_servers, const struct server_config_output &output);
static int write_output_dirman(const string &results_path, bool first_run_dirman, const struct dirman_config_output &output);
static int write_op_numbering_guide(const string &results_path);
string conver_pt_id_to_name(uint16_t pt_id);
static void get_stats(char *buffer, vector<stats> output, bool div_pow );
static void get_stats(char *buffer, stats output, bool div_pow);
static void get_stats(stats_size output, char *buffer, bool div_pow);
static void get_stats(char *buffer, vector<double> data_pts, bool div_pow );
static void get_stats(char *buffer, vector<long double> data_pts, bool div_pow);
void init_client_config_output(client_config_output &output);

bool is_local_run(uint32_t server_type) {
    return (server_type == SERVER_LOCAL_IN_MEM || server_type == SERVER_LOCAL_ON_DISK);
}

bool is_local_run(client_config_output output) {
    return (is_local_run(output.config.server_type));
}

double get_time() {
    struct timeval tv;
    gettimeofday(&tv, 0);
    return (tv.tv_sec*1.0 + tv.tv_usec*1e-6);
}

int main(int argc, char **argv) {
    int rc = RC_OK;

    setbuf(stdout, NULL);

    char hostname[100];
    gethostname(hostname, sizeof(hostname));
    string str_hostname = hostname;

    bool skip_output = false;
    bool skip_dirman = false;
    bool skip_server = false;
    bool skip_clients = false;

    md_write_type write_type;
    md_server_type server_type;
    md_db_index_type index_type;
    md_db_checkpoint_type checkpt_type;
    int num_procs;

    bool all_write_types = false;
    bool all_server_types = false;
    bool all_index_types = false;
    bool all_checkpt_types = false;
    bool all_num_procs = false;

    vector<string> clusters;
    vector<string> file_systems;
    string str_num_procs;
    string run_type = "scratch";

    string results_path;
    vector<string> output_dirs;

    int argv_index = 1;

    if(argc <= argv_index) {
        error_log << "error. program should be given 2 arguments: cluster_name and number of procs to limit to" << endl;
        return RC_ERR;
    }

    string cluster_name = argv[argv_index];
    argv_index += 1;

    //note - should be adjusted to handle multiple clusters at once?
    if(cluster_name == "cluster_e" || cluster_name == "cluster_b") {
        clusters.push_back(cluster_name);
        file_systems.push_back("scratch2");
        if(cluster_name == "cluster_b") {
            NUM_THREADS = 15;
        }
    }
    else if(cluster_name == "cluster_a" || cluster_name == "cluster_d") {
        clusters.push_back(cluster_name);
        file_systems.push_back("scratch");
        if(cluster_name == "cluster_a") {
            NUM_THREADS = 15;
        }
    }
    else if(cluster_name == "remote") {
        run_type = "remote";
        // all_num_procs = true;
    }

    //debug_log << "hostname: " << hostname << endl;
    // if(!all_num_procs) {
        if(argc <= argv_index) {
            error_log << "error. program should be given at least 2 arguments: cluster_name and number of procs to limit to (or 'all')" << endl;
            return RC_ERR;
        }
        str_num_procs = argv[argv_index];
        argv_index += 1;

        // if(str_hostname.find("login") != std::string::npos ) {
        //     error_log << "error. trying to run on a login node. exiting" << endl;
        //     return RC_ERR;
        // }
        if(str_num_procs == "all") {
            all_num_procs = true;
        }
        else {
            num_procs = stoi(str_num_procs);
        }
    // }

   if(argc > argv_index) {
        string config_type = argv[argv_index];
        if(config_type.find("all") != std::string::npos) {
            all_write_types = true;
            all_server_types = true;
            all_index_types = true;
            all_checkpt_types = true;
            argv_index += 1;
        }
        else {
            if(argc < argv_index+4) {
                error_log << "error. arguments should be: cluster, num_procs, (config options: all or write type, server type, index type, checkpoint type)" << endl;
                return RC_ERR;
            }
            else {
                write_type = (md_write_type)atoi(argv[argv_index]);
                server_type = (md_server_type)atoi(argv[argv_index+1]);
                index_type = (md_db_index_type)atoi(argv[argv_index+2]);
                checkpt_type = (md_db_checkpoint_type)atoi(argv[argv_index+3]);
                argv_index += 4;
            }
        }


    }
    else {
        error_log << "error. arguments should be: cluster, num_procs, (config options: all or write type, server type, index type, checkpoint type)" << endl;
    }

    if(argc > argv_index) {
        results_path = argv[argv_index];
        //extreme_debug_log << "results path: " << results_path << endl;
        argv_index += 1;
    }

    uint32_t argv_index_orig = argv_index;
    while(argc > argv_index) {
        string output_dir = argv[argv_index];
        output_dirs.push_back(output_dir);
        argv_index++;
    }
    if(argv_index == argv_index_orig) {
        output_dirs.push_back("");
    }
    double start_time = get_time();

    if(run_type == "remote") {
        clusters.push_back("cluster_e");
        file_systems.push_back("scratch2");
    }

    //extreme_debug_log << "got here \n";

    for(int i=0; i<clusters.size(); i++) {
        std::map <string, client_config_output> client_outputs;
        std::map <string, server_config_output> server_outputs;
        std::map <string, dirman_config_output> dirman_outputs;
        //extreme_debug_log << "clusters.size(): " << clusters.size() << endl;
        //extreme_debug_log << "i: " << i << endl;
        string cluster_name = clusters.at(i);
        string pfs = file_systems.at(i);

        DIR *runtime_output;
        struct dirent *entry;

        cout << "outputting files to: " << results_path << endl;
        if (run_type == "remote") {

                if(results_path.empty() || results_path=="default" ) {
                    results_path = "PATH_TO_EMPRESS/testing_source_class_proj/analysis_code_parallel_new/analysis_cluster_c";
                }
        }
        else if (run_type == "scratch") {
            if(results_path.empty() || results_path=="default" ) {
                results_path = "PATH_TO_EMPRESS/testing_source_class_proj/analysis_code_parallel_new/analysis_" + cluster_name + "/" + str_num_procs;
            }
        }
        if(all_write_types) {
            results_path += "/" + cluster_name + "_all_";        }
        else {
            results_path += "/" + cluster_name + "_" + str_num_procs + "_" + to_string(write_type) + "_"
                            + to_string(server_type) + "_" + to_string(index_type) + "_" 
                            + to_string(checkpt_type) + "_";            
        }


        for(string output_dir : output_dirs) {

            if (run_type == "remote") {
                    if(output_dir.empty() || output_dir=="default") {
                        output_dir = "FILL_IN_WITH_DESIRED_VALUE";
                    }
            }
            else if (run_type == "scratch") {
                if(output_dir.empty() || output_dir=="default") {
                    int n = sprintf(&output_dir[0], "FILL_IN_WITH_DESIRED_VALUE"); 
                }
            }

            cout << "reading files from: " << output_dir << endl;

            if( (runtime_output = opendir(output_dir.c_str())) ) {
                while(  (entry = readdir(runtime_output)) ) {
                    string filename = (string)entry -> d_name;
                    size_t found = filename.find_last_of("_");
                    string filename_minus_iteration = filename.substr(0,found+1);


                    if( filename.find("testing_harness") != std::string::npos ) {
                        if(skip_clients) {
                            cout << "Have been instructed to skip clients, so am ignoring " << filename << endl;
                            continue;
                        }
                        debug_log << "filename: " << filename << endl;
                        debug_log << "filename minus iteration: " << filename_minus_iteration << endl;

                        if (client_outputs.find(filename_minus_iteration) == client_outputs.end() ) {
                            struct client_config_output new_output;
                            new_output.filenames.push_back(output_dir + "/" + filename);
                            client_outputs[filename_minus_iteration] = new_output;
                        }
                        else {
                            client_outputs.find(filename_minus_iteration)->second.filenames.push_back(output_dir + "/" + filename);
                        }
                        debug_log << "count of file params: " << (client_outputs.find(filename_minus_iteration)->second).filenames.size() << endl;

                    }       
                    else if( filename.find("my_metadata_server") != std::string::npos ) {
                        if(skip_server) {
                            continue;
                        }
                        debug_log << "filename: " << filename << endl;
                        debug_log << "filename minus iteration: " << filename_minus_iteration << endl;
                        if (server_outputs.find(filename_minus_iteration) == server_outputs.end() ) {
                            struct server_config_output new_output;
                            new_output.filenames.push_back(output_dir + "/" + filename);
                            server_outputs[filename_minus_iteration] = new_output;
                        }
                        else {
                            server_outputs.find(filename_minus_iteration)->second.filenames.push_back(output_dir + "/" + filename);
                        }
                        debug_log << "count of file params: " << (server_outputs.find(filename_minus_iteration)->second).filenames.size() << endl;                  
                    }
                    else if( filename.find("my_dirman") != std::string::npos ) {
                        if(skip_dirman) {
                            continue;
                        }
                        debug_log << "filename: " << filename << endl;
                        debug_log << "filename minus iteration: " << filename_minus_iteration << endl;
                        if (dirman_outputs.find(filename_minus_iteration) == dirman_outputs.end() ) {
                            struct dirman_config_output new_output;
                            new_output.filenames.push_back(output_dir + "/" + filename);
                            dirman_outputs[filename_minus_iteration] = new_output;
                        }
                        else {
                            dirman_outputs.find(filename_minus_iteration)->second.filenames.push_back(output_dir + "/" + filename);

                        }
                        debug_log << "count of file params: " << (dirman_outputs.find(filename_minus_iteration)->second).filenames.size() << endl;      

                    }
                    else {
                        debug_log << "found file that isn't client. name: " << entry -> d_name << endl;
                        continue;
                    }


                }
                debug_log << "done reading files \n";
                closedir(runtime_output);
                debug_log << "got here \n";
            }
            else {
                error_log << "Error. Could not open dir with path " << output_dir << endl;
            }
        }



        //note - currently doens't have options for server proc per node testing, client proc per node testing or txn testing. 
        //for these, please see single config granularity or output per proc versions

        bool write_server_first_output = true;
        bool write_client_first_output = true;
        bool read_client_first_output = true;

        bool do_read = false;

            vector<string> do_read_params =  {
                "_0_0_0_0_",
                "_0_1_0_0_",
                "_0_0_1_0_",
                "_0_0_1_2_",
                "_0_2_0_0_",
                "_0_2_1_0_",
                "_0_3_0_0_",
                "_2_0_0_0_",
                "_3_0_0_0_",
                // "_1_0_0_0_" //just for target log files
                "_0_0_2_0_",
                "_0_6_0_0_",
                "_0_7_0_0_",
                "_0_8_0_0_",
            };
        std::set<string> do_read_set (&do_read_params[0], &do_read_params[0]+do_read_params.size());


        for (std::map<string,client_config_output>::iterator it=client_outputs.begin(); it!=client_outputs.end(); ++it) {
            string config_params = it->first;
            client_config_output client_config_struct = it->second;

            bool local = false;

            string client_server_config = config_params.substr(config_params.length()-9); 
            if(!do_read_set.count(client_server_config)) {
            }
            else {
                do_read = true;
            }

            debug_log << "config_params: " << config_params << endl;

            string testing_harness_name;
            if(config_params.find("vol_") != std::string::npos) {
                testing_harness_name = "testing_harness_large_md_vol";
            }
            else if(config_params.find("hdf5_") != std::string::npos) {
                testing_harness_name = "testing_harness_class_proj_hdf5";
            }
            else if(config_params.find("local_") != std::string::npos) {
                testing_harness_name = "testing_harness_class_proj_local";
                local = true;
            } 
            else if(config_params.find("d2t") != std::string::npos) {
                testing_harness_name = "testing_harness_class_proj_d2t_transactions_only";
            }             
            else if(config_params.find("db_streams_") != std::string::npos) {
                testing_harness_name = "testing_harness_class_proj_sqlite_transactions_only_db_streams";
            } 
            else if(config_params.find("sqlite_transactions_only_") != std::string::npos) {
                testing_harness_name = "testing_harness_class_proj_sqlite_transactions_only";
            }
            else if( config_params.find("_class_proj_") != std::string::npos ) {
                testing_harness_name = "testing_harness_class_proj";
            }


            string file_name = testing_harness_name + "_%llu_%hu_%llu_%hu_%llu_%hu_%llu_%u_%d_%d_%d_%d";
            sscanf(config_params.c_str(), file_name.c_str(), 
                &client_config_struct.config.num_server_procs, &client_config_struct.config.num_server_nodes,
                &client_config_struct.config.num_write_client_procs, &client_config_struct.config.num_write_client_nodes, 
                &client_config_struct.config.num_read_client_procs, &client_config_struct.config.num_read_client_nodes,                     
                &client_config_struct.config.num_timesteps, &client_config_struct.config.num_timesteps_per_checkpt,
                &client_config_struct.config.write_type, &client_config_struct.config.server_type,
                &client_config_struct.config.index_type, &client_config_struct.config.checkpt_type
            );
          

            // extreme_debug_log << "finshed config_params parse" << endl;
            client_config_struct.config.do_read = do_read;



            bool do_config = (
                (all_write_types || (client_config_struct.config.write_type == write_type)) && 
                (all_server_types || (client_config_struct.config.server_type == server_type)) && 
                (all_index_types || (client_config_struct.config.index_type == index_type)) && 
                (all_checkpt_types || (client_config_struct.config.checkpt_type == checkpt_type)) && 
                (all_num_procs || (client_config_struct.config.num_write_client_procs == num_procs)) 

            );

            if(!do_config) {
                continue;
            }

            client_config_struct.config.num_checkpts = client_config_struct.config.num_timesteps / client_config_struct.config.num_timesteps_per_checkpt;

            //1000 -> 2400, 3000->8000

            debug_log << "about to init_client_config_output" << endl;
            init_client_config_output(client_config_struct);

            debug_log << "num_server_procs: " << client_config_struct.config.num_server_procs << endl;
            debug_log << "num_server_nodes: " << client_config_struct.config.num_server_nodes << endl;
            debug_log << "num_write_client_procs: " << client_config_struct.config.num_write_client_procs << endl;
            debug_log << "num_write_client_nodes: " << client_config_struct.config.num_write_client_nodes << endl;
            debug_log << "num_read_client_procs: " << client_config_struct.config.num_read_client_procs << endl;
            debug_log << "num_read_client_nodes: " << client_config_struct.config.num_read_client_nodes << endl;
            debug_log << "num_timesteps: " << client_config_struct.config.num_timesteps << endl;
            debug_log << "num_timesteps_per_checkpt: " << client_config_struct.config.num_timesteps_per_checkpt << endl;


            for(int i = 0; i<client_config_struct.filenames.size(); i++) {
                string filename = client_config_struct.filenames.at(i);
                // string file_path = (string)output_dir + "/" + filename;
                debug_log << "about to evaluate_all_write_clients" << endl;
                rc = evaluate_all_write_clients(filename, client_config_struct);
                if (rc != RC_OK) {
                    error_log << "Error in evaluate_all_write_clients \n";
                    return rc;
                }
                cout << "done with evaluate_all_write_clients" << endl;
            }
            if(!skip_output) {
                rc = write_output_write_client(results_path, write_client_first_output, client_config_struct);
                if (rc != RC_OK) {
                    error_log << "Error in write_output_write_client \n";
                    return rc;
                }
                write_client_first_output = false;

                if(do_read) {
                    for(int i = 0; i < client_config_struct.config.num_checkpts; i++) {
                        // cout << "checkpt: " << i << endl;
                            rc = write_output_read_client(results_path, read_client_first_output, client_config_struct.read_outputs.at(i), i);
                            if (rc != RC_OK) {
                                error_log << "Error in write_output_read_client \n";
                                return rc;
                            }
                            read_client_first_output = false;
                    }
                }

                if(local) {
                    cout << "about to write_output_server for local " << endl;
                    rc = write_output_server(results_path, write_server_first_output, client_config_struct.server_output);
                    if (rc != RC_OK) {
                        error_log << "Error in write_output_server \n";
                        return rc;
                    }
                    write_server_first_output = false;
                }
            }
        }

        for (std::map<string,server_config_output>::iterator it=server_outputs.begin(); it!=server_outputs.end(); ++it) {
            string config_params = it->first;
            server_config_output server_config_struct = it->second;

            string server_name;
            if(config_params.find("d2t") != std::string::npos) {
                server_name = "my_metadata_server_d2t_transactions_only";
            }             
            else if(config_params.find("db_streams_") != std::string::npos) {
                server_name = "my_metadata_server_sqlite_transactions_only_db_streams";
            } 
            else if(config_params.find("sqlite_transactions_only_") != std::string::npos) {
                server_name = "my_metadata_server_sqlite_transactions_only";
            }
            else if(config_params.find("rtree_") != std::string::npos) {
                server_name = "my_metadata_server_rtree";
            }
            else {
                server_name = "my_metadata_server";                
            }

            string file_name = server_name + "_%llu_%hu_%llu_%hu_%llu_%hu_%llu_%u_%d_%d_%d_%d";
            sscanf(config_params.c_str(), file_name.c_str(), 
                    &server_config_struct.config.num_server_procs, &server_config_struct.config.num_server_nodes,
                    &server_config_struct.config.num_write_client_procs, &server_config_struct.config.num_write_client_nodes, 
                    &server_config_struct.config.num_read_client_procs, &server_config_struct.config.num_read_client_nodes,                     
                    &server_config_struct.config.num_timesteps, &server_config_struct.config.num_timesteps_per_checkpt,
                    &server_config_struct.config.write_type, &server_config_struct.config.server_type,
                    &server_config_struct.config.index_type, &server_config_struct.config.checkpt_type
                    );


            bool do_config = (
                (all_write_types || (server_config_struct.config.write_type == write_type)) && 
                (all_server_types || (server_config_struct.config.server_type == server_type)) && 
                (all_index_types || (server_config_struct.config.index_type == index_type)) && 
                (all_checkpt_types || (server_config_struct.config.checkpt_type == checkpt_type)) && 
                (all_num_procs || (server_config_struct.config.num_write_client_procs == num_procs)) 

            );

            if(!do_config) {
                continue;
            }

  
            server_config_struct.config.num_checkpts = server_config_struct.config.num_timesteps / server_config_struct.config.num_timesteps_per_checkpt;

            server_config_struct.config.num_clock_pts = NUM_CLOCK_PTS_SERVER;
            server_config_struct.config.num_storage_pts = NUM_SERVER_STORAGE_PTS;
            server_config_struct.all_clock_times = vector<stats>(NUM_CLOCK_PTS_SERVER);
            server_config_struct.all_storage_sizes = vector<stats_size>(NUM_SERVER_STORAGE_PTS);
            server_config_struct.clock_times_eval = vector<stats>(3);
            server_config_struct.init_times = vector<stats>(7);

            debug_log << "num_checkpts: " << server_config_struct.config.num_checkpts << endl;
            debug_log << "num_timesteps: " << server_config_struct.config.num_timesteps << endl;
            debug_log << "num_timesteps_per_checkpt: " << server_config_struct.config.num_timesteps_per_checkpt << endl;

            for(int i = 0; i < server_config_struct.config.num_checkpts; i++) {
                std::vector<std::vector<stats>> op_times;
                for (short j = 0; j< NUM_SERVER_OPS; j++) {
                    op_times.push_back(std::vector<stats>(NUM_SERVER_OP_TIMING_PIECES));
                }
                server_config_struct.op_times.push_back(op_times);
            }
            server_config_struct.db_checkpoint_times = vector<stats>(server_config_struct.config.num_checkpts);
            server_config_struct.db_checkpoint_sizes = vector<stats_size>(server_config_struct.config.num_checkpts);

            debug_log << "num_server_procs: " << server_config_struct.config.num_server_procs << endl;
            debug_log << "num_server_nodes: " << server_config_struct.config.num_server_nodes << endl;
            debug_log << "num_write_client_procs: " << server_config_struct.config.num_write_client_procs << endl;
            debug_log << "num_write_client_nodes: " << server_config_struct.config.num_write_client_nodes << endl;
            debug_log << "num_read_client_procs: " << server_config_struct.config.num_read_client_procs << endl;
            debug_log << "num_read_client_nodes: " << server_config_struct.config.num_read_client_nodes << endl;
            debug_log << "num_timesteps: " << server_config_struct.config.num_timesteps << endl;
            debug_log << "num_timesteps_per_checkpt: " << server_config_struct.config.num_timesteps_per_checkpt << endl;
            debug_log << "server_config_struct.all_clock_times.size(): " << server_config_struct.all_clock_times.size() << endl;
            debug_log << "server_config_struct.all_storage_sizes.size(): " << server_config_struct.all_storage_sizes.size() << endl;
            debug_log << "server_config_struct.clock_times_eval.size(): " << server_config_struct.clock_times_eval.size() << endl;

            cout << "server config_params: " << config_params << " (count: " << server_config_struct.filenames.size() << ")" << endl;

            for(int i = 0; i<server_config_struct.filenames.size(); i++) {
                string filename = server_config_struct.filenames.at(i);
                // string file_path = (string)output_dir + "/" + filename;
                // debug_log << "server file_path: " << file_path << endl;
                rc = evaluate_all_servers(filename, server_config_struct);
                if (rc != RC_OK) {
                    error_log << "Error in evaluate_all_servers \n";
                    return rc;
                }
                cout << "done with evaluate_all_servers" << endl;
            }
            if(!skip_output) {
                debug_log << "about to write_output_server" << endl;
                rc = write_output_server(results_path, write_server_first_output, server_config_struct);
                if (rc != RC_OK) {
                    error_log << "Error in write_output_server \n";
                    return rc;
                }
                write_server_first_output = false;
            }
        }

        bool write_dirman_first_output = true;
        for (std::map<string,dirman_config_output>::iterator it=dirman_outputs.begin(); it!=dirman_outputs.end(); ++it) {
            string config_params = it->first;
            dirman_config_output dirman_config_struct = it->second;
            debug_log << "config_params: " << config_params << endl;

            sscanf(config_params.c_str(), "my_dirman_%llu_%hu_%llu_%hu_%llu_%hu_%llu_%u_%d_%d_%d_%d", 
                &dirman_config_struct.config.num_server_procs, &dirman_config_struct.config.num_server_nodes,
                &dirman_config_struct.config.num_write_client_procs, &dirman_config_struct.config.num_write_client_nodes, 
                &dirman_config_struct.config.num_read_client_procs, &dirman_config_struct.config.num_read_client_nodes,                     
                &dirman_config_struct.config.num_timesteps, &dirman_config_struct.config.num_timesteps_per_checkpt,
                &dirman_config_struct.config.write_type, &dirman_config_struct.config.server_type,
                &dirman_config_struct.config.index_type, &dirman_config_struct.config.checkpt_type              
            );

            bool do_config = (
                (all_write_types || (dirman_config_struct.config.write_type == write_type)) && 
                (all_server_types || (dirman_config_struct.config.server_type == server_type)) && 
                (all_index_types || (dirman_config_struct.config.index_type == index_type)) && 
                (all_checkpt_types || (dirman_config_struct.config.checkpt_type == checkpt_type)) && 
                (all_num_procs || (dirman_config_struct.config.num_write_client_procs == num_procs)) 

            );

            if(!do_config) {
                continue;
            }

            dirman_config_struct.config.num_clock_pts = NUM_CLOCK_PTS_DIRMAN;
            dirman_config_struct.all_clock_times = vector<vector<long double>>(NUM_CLOCK_PTS_DIRMAN);
            dirman_config_struct.clock_times_eval = vector<vector<long double>>(2);
            dirman_config_struct.all_time_pts = vector<vector<double>>(NUM_CLOCK_PTS_DIRMAN-1);

            debug_log << "num_server_procs: " << dirman_config_struct.config.num_server_procs << endl;
            debug_log << "num_server_nodes: " << dirman_config_struct.config.num_server_nodes << endl;
            debug_log << "num_write_client_procs: " << dirman_config_struct.config.num_write_client_procs << endl;
            debug_log << "num_write_client_nodes: " << dirman_config_struct.config.num_write_client_nodes << endl;
            debug_log << "num_read_client_procs: " << dirman_config_struct.config.num_read_client_procs << endl;
            debug_log << "num_read_client_nodes: " << dirman_config_struct.config.num_read_client_nodes << endl;
            debug_log << "num_timesteps: " << dirman_config_struct.config.num_timesteps << endl;
            debug_log << "num_timesteps_per_checkpt: " << dirman_config_struct.config.num_timesteps_per_checkpt << endl;
            debug_log << "dirman_config_struct.all_clock_times.size(): " << dirman_config_struct.all_clock_times.size() << endl;
            debug_log << "dirman_config_struct.clock_times_eval.size(): " << dirman_config_struct.clock_times_eval.size() << endl;
            debug_log << "dirman_config_struct.all_time_pts.size(): " << dirman_config_struct.all_time_pts.size() << endl;

            cout << "write dirman config_params: " << config_params << " (count: " << dirman_config_struct.filenames.size() << ")" << endl;

            for(int i = 0; i<dirman_config_struct.filenames.size(); i++) {
                string filename = dirman_config_struct.filenames.at(i);
                // string file_path = (string)output_dir + "/" + filename;
                // debug_log << "dirman file_path: " << file_path << endl;
                rc = evaluate_dirman(filename, dirman_config_struct);
                if (rc != RC_OK) {
                    error_log << "Error in evaluate_dirman \n";
                    return rc;
                }
                cout << "done with evaluate_dirman" << endl;
            }
        
            if(!skip_output) {
                rc = write_output_dirman(results_path, write_dirman_first_output, dirman_config_struct);
                if (rc != RC_OK) {
                    error_log << "Error in write_output_dirman \n";
                    return rc;
                }
                write_dirman_first_output = false;
            }
        }        
        
        if(!skip_output) {
            rc = write_op_numbering_guide(results_path);
            if (rc != RC_OK) {
                error_log << "Error in write_op_numbering_guide \n";
                return rc;
            }
        }
    }//end for cluster

    cout << endl << "Total analysis runtime: " << get_time()-start_time << endl;
}

void init_client_config_output(client_config_output &output) {
    std::vector<std::vector<stats>> ops;
    for (int i = 0; i< NUM_CLIENT_OPS; i++) {
        ops.push_back(std::vector<stats>(NUM_CLIENT_OP_TIMING_PIECES));
    }     

    for (int i = 0; i < output.config.num_checkpts; i++) {

        output.op_times.push_back(ops);
    }
    output.per_proc_times.resize(output.config.num_checkpts);
    output.per_proc_op_times.resize(output.config.num_checkpts);
    output.last_first_times.resize(output.config.num_checkpts);

    //extreme_debug_log << "output.op_times.size(): " << output.op_times.size() << endl;
    //extreme_debug_log << "output.per_proc_times.size(): " << output.per_proc_times.size() << endl;
    //extreme_debug_log << "output.per_proc_op_times.size(): " << output.per_proc_op_times.size() << endl;
    //extreme_debug_log << "output.last_first_times.size(): " << output.last_first_times.size() << endl;

    for(int checkpt = 0; checkpt < output.config.num_checkpts; checkpt++) {
        uint32_t num_pts = (uint32_t)PTS_OF_INTEREST_ENUM::COUNT;
       
        output.per_proc_times[checkpt].resize(num_pts);
        output.per_proc_op_times[checkpt].resize(num_pts);
        output.last_first_times[checkpt].resize(num_pts);
    }

   if(output.config.do_read) {
        for (int i = 0; i < output.config.num_checkpts; i++) {

            read_client_config_output read_client;

            read_client.config = output.config;
            read_client.filenames = output.filenames;
            read_client.total_read_patterns_times = std::vector<stats>(6);
            read_client.read_pattern_gather_times = std::vector<stats>(18);
            read_client.read_pattern_op_times = std::vector<stats>(18);
            read_client.read_pattern_total_gather_times = std::vector<stats>(18);
            read_client.read_pattern_total_op_times = std::vector<stats>(18);
            read_client.read_pattern_times = std::vector<stats>(18);  
            for (int i = 0; i< NUM_CLIENT_OPS; i++) {
                read_client.op_times.push_back(std::vector<stats>(NUM_CLIENT_OP_TIMING_PIECES));
                read_client.collective_op_times.push_back(std::vector<stats>(4));
            }
        
            output.read_outputs.push_back(read_client);
        }
        // cout << "num_checkpts: " << output.config.num_checkpts << " output.read_outputs.size(): " << output.read_outputs.size() << endl;

    }

    if(is_local_run(output)) {
        //each client has an embedded server
        output.config.num_server_procs = output.config.num_write_client_procs;
        output.config.num_read_client_procs = output.config.num_write_client_procs;

        //single output file for client and server (since server is embedded)
        output.config.num_storage_pts = NUM_SERVER_STORAGE_PTS;
        output.server_output.all_storage_sizes = vector<stats_size>(NUM_SERVER_STORAGE_PTS);
        output.server_output.db_checkpoint_times = vector<stats>(output.config.num_checkpts);
        output.server_output.db_checkpoint_sizes = vector<stats_size>(output.config.num_checkpts);
        output.server_output.init_times = vector<stats>(7);

        output.server_output.config = output.config;
    }
}



// problem - some are per run
// template<class T1, class T2>
// uint32_t get_times_per_run( T1 output, const vector<T2> &vct ){
//  if (vct.size() == 0 || output.filenames.size() == 0) {
//      //extreme_debug_log << "vct.size(): " << vct.size() << endl;
//      return 0;
//  }
//  //extreme_debug_log << "vct.size(): " << vct.size() << endl;
//  //extreme_debug_log << "output.filenames.size(): " << output.filenames.size() << endl;
//  //extreme_debug_log << "output.config.num_timesteps: " << output.config.num_timesteps << endl;
//  if (vct.size() == output.filenames.size()) {
//      //extreme_debug_log << "are equal, returning" << endl;
//      return 1;
//  }
//  return ( vct.size() / (output.filenames.size() * output.config.num_timesteps) );
// }

template <class T1, class T2>
uint32_t get_times_per_run( T1 output, const vector<T2> &stats_vct ){
    if (stats_vct.size() == 0 || output.filenames.size() == 0) {
        return 0;
    }
    return ( stats_vct.back().size() / (output.filenames.size()) );
}

template <class T1, class T2>
uint32_t get_times_per_proc( T1 output, const vector<T2> &stats_vct ){
    if (stats_vct.size() == 0 || output.filenames.size() == 0) {
        return 0;
    }
    if (stats_vct.back().size() >= output.config.num_write_client_procs) {
        return (stats_vct.back().size() / (output.filenames.size() * output.config.num_write_client_procs));
    }
    return 0;
}


template <class T1, class T2>
uint32_t get_times_per_run( T1 output, const T2 &stats ){
    if (stats.size() == 0 || output.filenames.size() == 0) {
        return 0;
    }
    return ( stats.size() / (output.filenames.size()) );
}

template <class T1, class T2>
uint32_t get_times_per_proc( T1 output, const T2 &stats ){
    if (stats.size() == 0 || output.filenames.size() == 0) {
        return 0;
    }
    if (stats.size() >= output.config.num_write_client_procs) {
        return (stats.size() / (output.filenames.size() * output.config.num_write_client_procs));
    }
    return 0;
}

template <class T2>
uint32_t get_times_per_run( dirman_config_output output, const vector<T2> &vct ){
    //extreme_debug_log << "vct.size(): " << vct.size() << endl;
    //extreme_debug_log << "output.filenames.size(): " << output.filenames.size() << endl;

    if (vct.size() == 0 || output.filenames.size() == 0) {
        return 0;
    }
    return ( vct.size() / (output.filenames.size()) );
}

template <class T2>
uint32_t get_times_per_proc( dirman_config_output output, const vector<T2> &vct ){
    //extreme_debug_log << "vct.size(): " << vct.size() << endl;
    //extreme_debug_log << "output.config.num_write_client_procs.size: " << output.config.num_write_client_procs << endl;

    if (vct.size() == 0 || output.filenames.size() == 0) {
        return 0;
    }
    if (vct.size() >= output.config.num_write_client_procs) {
        return (vct.size() / (output.filenames.size() * output.config.num_write_client_procs));
    }
    return 0;
}




string get_config_header()
{
    return("Write Clients per Server,num_server_nodes,num_write_client_procs,num_write_client_nodes,num_read_client_procs,num_read_client_nodes,num_timesteps,num_timesteps_per_checkpt,write_type,server_type,index_type,checkpt_type,");
}

string get_stats_header()
{
    return("num data pts,max,min,avg,variance,std deviation\n");
}

string get_stats_header_w_median()
{
    return("num data pts,max,min,avg,variance,std deviation,median\n");
}

string get_times_per_run_and_proc()
{
    return("pts per run,pts per proc,");
}


static int write_output_write_client(const string &results_path, bool first_run_clients, const struct client_config_output &output) 
{
    //note: these will in essence be key value tables with key: config params

    int rc = RC_OK;


    debug_log << "about to start writing client output \n";


    int num_ops = NUM_CLIENT_OPS;

    uint32_t num_reads = output.config.num_timesteps / output.config.num_timesteps_per_checkpt;


    string op_results_path = results_path + "write_client_ops.csv";
    ofstream op_fs;
    string op_breakdown_path = results_path + "write_client_op_breakdown.csv";
    ofstream op_breakdown_fs;
    string section_results_path = results_path + "write_client_sections.csv";
    ofstream section_fs;


    string last_first_results_path = results_path + "write_client_last_first_sections.csv";
    ofstream last_first_fs;

    debug_log << "output.filenames.at(0): " << output.filenames.at(0) << endl;

    if(first_run_clients) { 
        debug_log << "first run \n";

        op_fs.open(op_results_path, std::fstream::out);
        op_breakdown_fs.open(op_breakdown_path, std::fstream::out);
        section_fs.open(section_results_path, std::fstream::out);
        last_first_fs.open(last_first_results_path, std::fstream::out);

        op_fs << "checkpt_count,op,";
        op_fs << get_config_header();
        op_fs << get_times_per_run_and_proc();
        op_fs << get_stats_header();

        op_breakdown_fs << "checkpt_count,op,";
        op_breakdown_fs << get_config_header();
        op_breakdown_fs << get_times_per_run_and_proc();
        op_breakdown_fs << "num data pts,avg serialize time,avg send time,avg received return msg time,"
                        << "avg rdma init time,avg rdma wait time,avg deserialize time,avg total time" << endl;

        section_fs << "#categories: " << endl;
        section_fs << "#0: total time, 1:total op time" << endl << endl;

        section_fs << "name,checkpt_count,category,";
        section_fs << get_config_header();
        section_fs << get_times_per_run_and_proc();
        section_fs << get_stats_header();

        last_first_fs << "name,checkpt_count,";
        last_first_fs << get_config_header();
        last_first_fs << get_times_per_run_and_proc();
        last_first_fs << get_stats_header();
    }
    else {

        op_fs.open(op_results_path, std::fstream::app);
        op_breakdown_fs.open(op_breakdown_path, std::fstream::app);
        section_fs.open(section_results_path, std::fstream::app);
        last_first_fs.open(last_first_results_path, std::fstream::app);
    }

    if(!op_fs) {
        error_log << "error. failed to open ops results file: " << op_results_path << endl;
        rc = RC_ERR;
        goto cleanup;
    }
    else if(!op_breakdown_fs) {
        error_log << "error. failed to open ops breakdown file" << endl;
        rc = RC_ERR;
        goto cleanup;
    }
    if(!section_fs) {
        error_log << "error. failed to open section results file" << endl;
        rc = RC_ERR;
        goto cleanup;   
    }

    else if(!last_first_fs) {
        error_log << "error. failed to open last first results file" << endl;
        rc = RC_ERR;
        goto cleanup;   
    }
    char buffer[512];


    for(int checkpt_count = 0; checkpt_count < num_reads; checkpt_count++) {
        for(int op_index = 0; op_index < num_ops; op_index++) {
            vector<stats> op_pieces_times = output.op_times[checkpt_count][op_index];
            stats total_op_times = op_pieces_times.at(op_pieces_times.size()-1);
            debug_log << "write client - num time pts for op " << op_index << ": " << total_op_times.num_values << endl;
            debug_log << "write client - num categories for op breakdown " << op_index << ": " << op_pieces_times.size() << endl;

            if(total_op_times.num_values > 0) {


                sprintf(buffer, "%d,%d,", checkpt_count,op_index*100 + 1000);
                op_fs << buffer;
                get_config_and_stats(op_fs, output, total_op_times, true);

                //extreme_debug_log << "just wrote buffer" << endl;

                if(op_pieces_times.size() == NUM_CLIENT_OP_TIMING_PIECES) {
                  
                    sprintf(buffer, "%d,%d,", checkpt_count,op_index*100 + 1000);
                    op_breakdown_fs << buffer;
                    get_config_and_stats(op_breakdown_fs, output, op_pieces_times, true);
                }
                else {
                    error_log << "error. op " << op_index << " had " << op_pieces_times.size() << " time categories \n";
                }
            }
        } //end for op index
        op_fs << endl;
        op_breakdown_fs << endl;
    } //end for read count


    // extreme_debug_log << "output.per_proc_times.size(): " << output.per_proc_times.size() << endl;
    for(int checkpt_num = 0; checkpt_num < output.config.num_timesteps / output.config.num_timesteps_per_checkpt; checkpt_num++) {
        for (int i = 0; i < output.per_proc_times.at(checkpt_num).size(); i++) {
            stats timing_points = output.per_proc_times.at(checkpt_num).at(i);
            string pt_name = conver_pt_id_to_name(i);
            if(timing_points.size() > 0) {  
                cout << "timing_points.size(): " << timing_points.size() << endl;
                // //extreme_debug_log << "about to get max timing pt" << endl;
                // for(int i = 0; i < timing_points.size(); i++) {
                //     //extreme_debug_log << timing_points[i] << " ";
                // }
                // //extreme_debug_log << endl;

                sprintf(buffer, "%30s,%d,%d,", pt_name.c_str(),checkpt_num,0);
                section_fs << buffer;
                get_config_and_stats(section_fs, output, timing_points, true);


            }
            // int indx = 0;
            stats op_times = output.per_proc_op_times[checkpt_num][i];
            //extreme_debug_log << "output.per_proc_op_times[checkpt_num].size(): " << output.per_proc_op_times[checkpt_num].size() << endl;
            // extreme_debug_log << "op_times.num_values: " << op_times.num_values << endl;
            // for(indx = 0; indx < output.per_proc_op_times[checkpt_num].size(); indx++) {
            //     extreme_debug_log << "output.per_proc_op_times[" << checkpt_num << "][" << indx << "].size(): " << output.per_proc_op_times[checkpt_num][indx].size() << endl;
            // }
            if(op_times.num_values > 0) {   

                sprintf(buffer, "%30s,%d,%d,", pt_name.c_str(),checkpt_num, 1);
                section_fs << buffer;
                get_config_and_stats(section_fs, output, op_times, true);


            }
        }
    }


    // extreme_debug_log << "output.last_first_times.size(): " << output.last_first_times.size() << endl;
    for(int checkpt_num = 0; checkpt_num < output.config.num_timesteps / output.config.num_timesteps_per_checkpt; checkpt_num++) {
        for (int i = 0; i < output.last_first_times.at(checkpt_num).size(); i++) {
            string pt_name = conver_pt_id_to_name(i);
            stats timing_points = output.last_first_times.at(checkpt_num).at(i);
            if(timing_points.size() > 0) {
                sprintf(buffer, "%30s,%d,", pt_name.c_str(), checkpt_num );
                last_first_fs << buffer;
                get_config_and_stats(last_first_fs, output, timing_points, true);

            }   
        }
    }

    //means there is a line between timing info for different configs
    op_fs << endl;
    op_breakdown_fs << endl;
    section_fs << endl;
    last_first_fs << endl;

cleanup:
    if(op_fs.is_open()) {
        op_fs.close();
    }
    if(op_breakdown_fs.is_open()) {
        op_breakdown_fs.close();
    }
    if(section_fs.is_open()) {
        section_fs.close();
    }
    if(last_first_fs.is_open()) {
        last_first_fs.close();
    }

    return rc;
}




static int write_output_read_client(const string &results_path, bool first_run_clients, 
    const struct read_client_config_output &output, int checkpt_count) {
    //note: these will in essence be key value tables with key: config params

    int rc = RC_OK;

    vector<vector<stats>> all_read_pattern_times = {output.read_pattern_op_times, output.read_pattern_total_op_times, 
                output.read_pattern_gather_times, output.read_pattern_total_gather_times,
                output.read_pattern_times
            };

    vector<stats> all_timing_points{ output.sum_all_read_patterns_times,
                                        output.total_read_patterns_times.at(0), output.total_read_patterns_times.at(1),
                                        output.total_read_patterns_times.at(2),output.total_read_patterns_times.at(3),
                                        output.total_read_patterns_times.at(4),output.total_read_patterns_times.at(5),
                                        output.extra_testing_times,output.reading_times};

    debug_log << "about to start read client output \n";

    int num_read_patterns = 18; //6 patterns each with 3 types
    int num_ops = NUM_CLIENT_OPS;
    int num_sections = all_timing_points.size();

    string pattern_results_path = results_path + "read_client_patterns.csv";
    ofstream pattern_fs;
    string op_results_path = results_path + "read_client_ops.csv";
    ofstream op_fs;
    string op_breakdown_path = results_path + "read_client_op_breakdown.csv";
    ofstream op_breakdown_fs;
    string collective_op_results_path = results_path + "read_client_collective_ops.csv";
    ofstream collective_op_fs;
    string section_results_path = results_path + "read_client_sections.csv";
    ofstream section_fs;
    string clock_eval_results_path = results_path + "read_client_clocks_eval.csv";
    ofstream clock_eval_fs;


    vector<double> freqs = {.25, .05, .001};

    if(first_run_clients) { 
        debug_log << "first run \n";
        pattern_fs.open(pattern_results_path, std::fstream::out);
        op_fs.open(op_results_path, std::fstream::out);
        op_breakdown_fs.open(op_breakdown_path, std::fstream::out);
        section_fs.open(section_results_path, std::fstream::out);

        clock_eval_fs.open(clock_eval_results_path, std::fstream::out);
        collective_op_fs.open(collective_op_results_path, std::fstream::out);


        pattern_fs << "#category: " << endl;
        pattern_fs << "#0:op time, 1:sum op time, 2:avg gather time, 3:sum gather time, 4:total time" << endl;  
        pattern_fs << endl;
        pattern_fs << "checkpt_count,pattern,Selectivity,category,";
        pattern_fs << get_config_header();
        pattern_fs << get_stats_header();

        op_fs << "checkpt_count,op,";
        op_fs << get_config_header();
        op_fs << get_stats_header();

        collective_op_fs << "#timing categories: " << endl;
        collective_op_fs << "#0:gather time, 1:total collective op time" << endl;
        collective_op_fs << endl;
        collective_op_fs << "checkpt_count,op,category,";
        collective_op_fs << get_config_header();
        collective_op_fs << get_stats_header();

        op_breakdown_fs << "checkpt_count,op,";
        op_breakdown_fs << get_config_header();

        op_breakdown_fs << "num data pts,avg serialize time,avg send time,avg received return msg time,"
                        << "avg rdma init time,avg rdma wait time,avg deserialize time,avg total time" << endl;

        section_fs << "#section categories: " << endl;
        section_fs << "#0:total for 6 read patterns, "
                   << "1:total for read pattern 1, 2:total for read pattern 2, "
                   << "3:total for read pattern 3, 4:total for read pattern 4, "
                   << "5:total for read pattern 5, 6:total for read pattern 6, "
                   << "7:extra testing time, "
                   << "8:total read time" << endl;
        section_fs << endl;
        section_fs << "checkpt_count,section,";
        section_fs << get_config_header();
        section_fs << get_stats_header();

                //start, mpi init done, register done, db_init done, dirman_init_done, finalize

        clock_eval_fs << "#Numbering guide:" << endl;
        clock_eval_fs << "#0: init time" << endl;
        clock_eval_fs << "#50: read init time" << endl;
        clock_eval_fs << "#100: pattern 1" << endl;
        clock_eval_fs << "#200: pattern 2" << endl;
        clock_eval_fs << "#210: pattern 2 with types" << endl;
        clock_eval_fs << "#300: pattern 3" << endl;
        clock_eval_fs << "#310: pattern 3 with types" << endl;
        clock_eval_fs << "#400: pattern 4" << endl;
        clock_eval_fs << "#500: pattern 5" << endl;
        clock_eval_fs << "#600: pattern 6" << endl;
        clock_eval_fs << endl;
        clock_eval_fs << "#For numbers XX00, see the op numbering guide (op_numbering_guide.csv)" << endl;
        clock_eval_fs << endl;
        clock_eval_fs << "#XX10 - ABOVE_MAX op instead of range" << endl;
        clock_eval_fs << "#XX20 - BELOW_MIN op instead of range" << endl;
        clock_eval_fs << "#XX30 - Gather" << endl;
        clock_eval_fs << "#XX40 - Gather for ABOVE_MAX op instead of range" << endl;
        clock_eval_fs << "#XX50 - Gather for BELOW_MIN op instead of range" << endl;
        clock_eval_fs << endl;



        clock_eval_fs << "checkpt_count,category,";
        clock_eval_fs << get_config_header();
        clock_eval_fs << get_stats_header();

    }
    else {
        pattern_fs.open(pattern_results_path, std::fstream::app);
        op_fs.open(op_results_path, std::fstream::app);
        op_breakdown_fs.open(op_breakdown_path, std::fstream::app);
        collective_op_fs.open(collective_op_results_path, std::fstream::app);       
        section_fs.open(section_results_path, std::fstream::app);
        clock_eval_fs.open(clock_eval_results_path, std::fstream::app);
    }

    
    if(!pattern_fs) {
        error_log << "error. failed to open pattern results file" << endl;
        rc = RC_ERR;
        goto cleanup;   
    }
    else if(!op_fs) {
        error_log << "error. failed to open ops results file: " << op_results_path << endl;
        rc = RC_ERR;
        goto cleanup;
    }
    else if(!collective_op_fs) {
        error_log << "error. failed to open collective op results file" << endl;
        rc = RC_ERR;
        goto cleanup;
    }
    else if(!op_breakdown_fs) {
        error_log << "error. failed to open ops breakdown file" << endl;
        rc = RC_ERR;
        goto cleanup;
    }
    else if(!section_fs) {
        error_log << "error. failed to open section results file" << endl;
        rc = RC_ERR;
        goto cleanup;   
    }
    else if(!clock_eval_fs) {
        error_log << "error. failed to open clock results file" << endl;
        rc = RC_ERR;
        goto cleanup;   
    }
    char buffer[1024];


    for(int pattern_indx=0; pattern_indx<num_read_patterns; pattern_indx++) {
        for(int category = 0; category < all_read_pattern_times.size(); category++) {
            stats read_pattern_times = all_read_pattern_times.at(category).at(pattern_indx);

            debug_log << "pattern size for pattern " << pattern_indx + 1 << ": " << read_pattern_times.size() << endl;

            if(read_pattern_times.size() > 0) {


                int pattern = (pattern_indx / 3) + 1;
                float freq = freqs[pattern_indx%3];

                sprintf(buffer, "%d,%d,%f,%d,", checkpt_count,pattern,freq,category);
                pattern_fs << buffer;
                get_config_and_stats(pattern_fs, output, read_pattern_times, false);


            }
        }
    }


    for(int op_index=0; op_index < num_ops; op_index++) {
        vector<stats> op_pieces_times = output.op_times[op_index];
        stats total_op_times = op_pieces_times.at(op_pieces_times.size()-1);
        debug_log << "read client - num time pts for op " << op_index << ": " << total_op_times.size() << endl;
        debug_log << "read client - num categories for op breakdown " << op_index << ": " << op_pieces_times.size() << endl;

        if(total_op_times.size() > 0) {
            sprintf(buffer, "%d,%d,", checkpt_count,op_index*100 + 1000);
            op_fs << buffer;
            get_config_and_stats(op_fs, output, total_op_times, false);

            if(op_pieces_times.size() == NUM_CLIENT_OP_TIMING_PIECES) {
                sprintf(buffer, "%d,%d,", checkpt_count,op_index*100 + 1000);
                op_breakdown_fs << buffer;
                get_config_and_stats(op_breakdown_fs, output, op_pieces_times, false);


            }
            else {
                error_log << "error. op " << op_index << " had " << op_pieces_times.size() << " time categories \n";
            }
        }
    }

    for(int op_index=0; op_index < num_ops; op_index++) {
        vector<stats> op_pieces_times = output.collective_op_times[op_index];
        debug_log << "read client - num time points for collective op " << op_index << ": " << op_pieces_times.at(0).size() << endl;
        debug_log << "read client - num cateogries for collective op breakdown " << op_index << ": " << op_pieces_times.size() << endl;

        for (int category = 0; category < op_pieces_times.size(); category++) {
            stats category_times = op_pieces_times.at(category);

            if(category_times.size() > 0) {
                sprintf(buffer, "%d,%d,%d,", checkpt_count,op_index*100 + 1000, category);
                collective_op_fs << buffer;
                get_config_and_stats(collective_op_fs, output, category_times, false);
            }
        }
    }

    for(int section_index=0; section_index<num_sections; section_index++) {
        stats timing_points = all_timing_points.at(section_index);
        debug_log << "size for section " << section_index << ": " << timing_points.size() << endl;


        if(timing_points.size() > 0) {  
            sprintf(buffer, "%d,%d,", checkpt_count, section_index);
            section_fs << buffer;
            get_config_and_stats(section_fs, output, timing_points, false);

        }
    }


    //struct timeval start, mpi_init_done, register_ops_done, db_init done, dirman_init_done /
    debug_log << "output.clock_times_eval.size(): " << output.clock_times_eval.size() << endl;
    for(int i=0; i<output.clock_times_eval.size(); i++) {
        stats clock_times_eval = output.clock_times_eval.at(i);
        debug_log << "clock times eval for category " << i << ": " << clock_times_eval.size() << endl;
        if(clock_times_eval.size() > 0) {
            sprintf(buffer, "%d,%d,", checkpt_count, output.clock_times_eval_catgs.at(i));
            clock_eval_fs << buffer;
            get_config_and_stats(clock_eval_fs, output, clock_times_eval, false);
        }
    }
    debug_log << "done with write_output_read_client" << endl;

    //means there is a line between timing info for different configs
    pattern_fs << endl;
    op_fs << endl;
    op_breakdown_fs << endl;
    section_fs << endl;
    clock_eval_fs << endl;
    collective_op_fs << endl;

cleanup:
    if(pattern_fs.is_open()) {
        pattern_fs.close();
    }
    if(op_fs.is_open()) {
        op_fs.close();
    }
    if(op_breakdown_fs.is_open()) {
        op_breakdown_fs.close();
    }
    if(section_fs.is_open()) {
        section_fs.close();
    }
    if(clock_eval_fs.is_open()) {
        clock_eval_fs.close();
    }
    if(collective_op_fs.is_open()) {
        collective_op_fs.close();
    }
    return rc;
}


static int write_output_server(const string &results_path, bool first_run_servers, 
                    const struct server_config_output &output
                    ) 
{
    //note: these will in essence be key value tables with key: config params

    int rc = RC_OK;

    debug_log << "about to start writing server output \n";

    int num_ops = NUM_SERVER_OPS;
    debug_log << "num ops: " << num_ops << endl;

    bool local = (output.config.server_type == SERVER_LOCAL_IN_MEM || output.config.server_type == SERVER_LOCAL_ON_DISK);

    vector<stats> all_timing_points{output.init_times.at(0),output.init_times.at(1),output.init_times.at(2),
                                        output.init_times.at(3),output.init_times.at(4),output.init_times.at(5),
                                        output.init_times.at(6),
                                        output.run_times,output.shutdown_times,output.total_run_times};

    int num_sections = all_timing_points.size();

    string op_results_path = results_path + "server_ops.csv";
    ofstream op_fs;
    string op_breakdown_path = results_path + "server_op_breakdown.csv";
    ofstream op_breakdown_fs;
    string section_results_path = results_path + "server_sections.csv";
    ofstream section_fs;
    string storage_results_path = results_path + "server_storage.csv";
    ofstream storage_fs;
    string clock_eval_results_path = results_path + "server_clocks_eval.csv";
    ofstream clock_eval_fs;

    string db_checkpt_results_path = results_path + "server_db_checkpt.csv";
    ofstream db_checkpt_fs;

    string db_compact_results_path = results_path + "server_db_compact.csv";
    ofstream db_compact_fs;

    bool divide_by_nanoseconds = true;
    if(local) {
        divide_by_nanoseconds = false;
    }


     vector<string> storage_pts_names = {"DB Initial", "DB Final", "Num Runs", "Num Timesteps",
                        "Num Vars", "Num Types", "Num Run Attrs", "Num Timestep Attrs", "Num Var Attrs"};

    if(first_run_servers) {
        op_fs.open(op_results_path, std::fstream::out);
        op_breakdown_fs.open(op_breakdown_path, std::fstream::out);
        section_fs.open(section_results_path, std::fstream::out);
        storage_fs.open(storage_results_path, std::fstream::out);
        clock_eval_fs.open(clock_eval_results_path, std::fstream::out); 
        db_checkpt_fs.open(db_checkpt_results_path, std::fstream::out);

        db_compact_fs.open(db_compact_results_path, std::fstream::out);

        op_fs << "checkpt_count,op,";
        op_fs << get_config_header();
        op_fs << get_stats_header();

        op_breakdown_fs << "checkpt_count,op,";
        op_breakdown_fs << get_config_header();
        // op_breakdown_fs << "num data pts,avg deserialize time,avg db time,avg serialize time,"
        //       << "avg create msg,avg ser-create-send msg,avg total time" << endl;
        op_breakdown_fs << "num data pts,avg rdma init time,avg rdma wait time,avg deserialize time,"
              // << "avg db time,avg serialize time,avg create msg,avg ser-create-send msg,"
              << "avg db time,avg serialize time,avg create msg,avg send msg,"
              << "avg total time" << endl;

        section_fs << "#section categories: " << endl;
        section_fs << "#0:mpi init time, 1:register ops time, 2:db create write indices time, 3:db create read indices time, "
                   << "4:db setup time, 5:dirman setup time, "
                   << "6:total init time, 7:run time, 8:shutdown time, 9:total run time" << endl;
        section_fs << endl;
        section_fs << "section,";
        section_fs << get_config_header();
        section_fs << get_stats_header();
            storage_fs << "Storage Type,";
            storage_fs << get_config_header();
            storage_fs << "num data pts,max,min,avg,variance,std deviation,avg total per iteration" << endl;
        clock_eval_fs << "#categories: " << endl;
        clock_eval_fs << "#0:full init time, 1:full run (minus init)time, 2:full run time" << endl;
        clock_eval_fs << endl;
        clock_eval_fs << "category,";
        clock_eval_fs << get_config_header();
        clock_eval_fs << get_stats_header();

        db_checkpt_fs << "#categories: " << endl;
        db_checkpt_fs << "#0:db checkpt times, 1:db checkpt sizes" << endl;
        db_checkpt_fs << endl;
        db_checkpt_fs << "#checkpt type: " << endl;
        db_checkpt_fs << "#0:database copy, 1:incremental output, 2:database copy and delete, 3:database copy and reset, 4:database output incermental and delete, 5:database output incermental and reset" << endl;
        db_checkpt_fs << endl;
        db_checkpt_fs << "checkpt_count,category,";
        db_checkpt_fs << get_config_header();
        db_checkpt_fs << get_stats_header();

        db_compact_fs << "#categories: " << endl;
        db_compact_fs << "#0:compact time, 1:compact, create indices (if necessary), and close" << endl;
        db_compact_fs << endl;
        db_compact_fs << "category,";
        db_compact_fs << get_config_header();
        db_compact_fs << get_stats_header();
    }
    else {
        op_fs.open(op_results_path, std::fstream::app);
        op_breakdown_fs.open(op_breakdown_path, std::fstream::app);
        section_fs.open(section_results_path, std::fstream::app);
            storage_fs.open(storage_results_path, std::fstream::app);
        clock_eval_fs.open(clock_eval_results_path, std::fstream::app); 
        db_checkpt_fs.open(db_checkpt_results_path, std::fstream::app);
        db_compact_fs.open(db_compact_results_path, std::fstream::app);
    }
    //start, mpi init done, register done, db_init done, dirman_init_done, finalize

    

    if(!op_fs) {
        error_log << "error. failed to open ops results file: " << op_results_path << endl;
        rc = RC_ERR;
        goto cleanup;
    }
    else if(!op_breakdown_fs) {
        error_log << "error. failed to open ops breakdown file" << endl;
        rc = RC_ERR;
        goto cleanup;
    }
    else if(!section_fs) {
        error_log << "error. failed to open section results file" << endl;
        rc = RC_ERR;
        goto cleanup;
    }
    else if(!storage_fs) {
        error_log << "error. failed to open storage results file" << endl;
        rc = RC_ERR;
        goto cleanup;
    }
    else if(!clock_eval_fs) {
        error_log << "error. failed to open clock results file" << endl;
        rc = RC_ERR;
        goto cleanup;
    }
    else if(!db_checkpt_fs) {
        error_log << "error. failed to db checkpt results file" << endl;
        rc = RC_ERR;
        goto cleanup;
    }
    else if(!db_compact_fs) {
        error_log << "error. failed to db checkpt incr results file" << endl;
        rc = RC_ERR;
        goto cleanup;
    }
    char buffer[512];

    //extreme_debug_log << "got here" << endl;

    //for local, no separate 'server op' time (its all client), and no clock times
    if(!local) {

        for(int checkpt_count = 0; checkpt_count < output.config.num_checkpts; checkpt_count++) {
            for(int op_index=0; op_index < num_ops; op_index++) {
                vector<stats> op_pieces_times = output.op_times[checkpt_count][op_index];

                bool divide_by_nanoseconds = false;

                if(output.config.server_type != SERVER_LOCAL_IN_MEM && output.config.server_type != SERVER_LOCAL_ON_DISK) {
                    divide_by_nanoseconds = true;
                }
                stats total_op_times = op_pieces_times.at(op_pieces_times.size()-1);
                debug_log << "write server - num time pts for op " << op_index << ": " << total_op_times.size() << endl;
                debug_log << "write server - num categories for op breakdown " << op_index << ": " << op_pieces_times.size() << endl;

                if(total_op_times.size() > 0) {
                    sprintf(buffer, "%d,%d,", checkpt_count,op_index*100 + 1000);
                    op_fs << buffer;
                    get_config_and_stats(op_fs, output, total_op_times, false, divide_by_nanoseconds);

                    if(op_pieces_times.size() == NUM_SERVER_OP_TIMING_PIECES) {
                        sprintf(buffer, "%d,%d,", checkpt_count,op_index*100 + 1000);
                        op_breakdown_fs << buffer;
                        get_config_and_stats(op_breakdown_fs, output, op_pieces_times, false, divide_by_nanoseconds);
                    }
                    else {
                        error_log << "error. op " << op_index << " had " << op_pieces_times.size() << " time categories \n";
                    }
                }
            }
            op_fs << endl;
            op_breakdown_fs << endl;
        }

        //struct timeval start, mpi_init_done, register_ops_done, db_init done, dirman_init_done /
        for(int i=0; i<output.clock_times_eval.size(); i++) {
            stats clock_times_eval = output.clock_times_eval.at(i);
            debug_log << "clock times eval for category " << i << ": " << clock_times_eval.size() << endl;
            if(clock_times_eval.size() > 0) {
                sprintf(buffer, "%d,", i);
                clock_eval_fs << buffer;
                get_config_and_stats(clock_eval_fs, output, clock_times_eval, false);
            }
        }
    }
    //extreme_debug_log << "got here2" << endl;

    for(int section_index=0; section_index<num_sections; section_index++) {
        stats timing_points = all_timing_points.at(section_index);

        if(timing_points.size() > 0) {
            sprintf(buffer, "%d,", section_index);
            section_fs << buffer;
            get_config_and_stats(section_fs, output, timing_points, false, divide_by_nanoseconds);
        }
    }
    //extreme_debug_log << "got here3" << endl;

    for(int i=0; i<output.all_storage_sizes.size(); i++) {
        stats_size storage_sizes = output.all_storage_sizes.at(i);
        debug_log << "storage sizes for category " << i << ": " << storage_sizes.size() << endl;
        if(storage_sizes.size() > 0) {

            uint64_t total = output.config.num_server_procs * storage_sizes.avg;
        
            sprintf(buffer, "%s,", storage_pts_names.at(i).c_str());
            storage_fs << buffer;
            get_config_and_stats(storage_fs, output, storage_sizes, false, false, true);
            sprintf(buffer, ",%llu", total);

            storage_fs << buffer << endl;
        }
    }
    //extreme_debug_log << "got here4" << endl;

    debug_log << "about to eval checkpt times" << endl;

    for(int i=0; i<  output.db_checkpoint_times.size(); i++) {
        stats db_checkpoint_times =  output.db_checkpoint_times.at(i);
        stats_size db_checkpoint_sizes =  output.db_checkpoint_sizes.at(i);

        // debug_log << "about to get median. db_checkpoint_times.size(): " << db_checkpoint_times.size() << endl;
        // double median = get_median(db_checkpoint_times);
        // debug_log << "median: " << median << endl;
        // for(int i = 0; i < db_checkpoint_times.size(); i++) {
        //     if(db_checkpoint_times[i] > 10*median) {
        //         extreme_debug_log << "found a db checkpt outlier. median: " << median << " checkpt time: " << db_checkpoint_times[i] << ". am removing it" << endl;
        //         db_checkpoint_times.erase(db_checkpoint_times.begin() + i);
        //     }
        // }

        cout << "db_checkpoint_times for category " << i << ": " << db_checkpoint_times.size() << endl;
        if(db_checkpoint_times.size() > 0) {
            // cout << "about to get db_checkpoint_times" << endl;
            sprintf(buffer, "%d,%d,", i, 0);
            db_checkpt_fs << buffer;
            get_config_and_stats(db_checkpt_fs, output, db_checkpoint_times, false, divide_by_nanoseconds);
            
        }
        if(db_checkpoint_sizes.size() > 0) {
            sprintf(buffer, "%d,%d,", i, 1 );
            db_checkpt_fs << buffer;
            get_config_and_stats(db_checkpt_fs, output, db_checkpoint_sizes, false);
        }
        db_checkpt_fs << endl;
    }
    //extreme_debug_log << "got here6" << endl;

    cout << "db_compact_times: " << output.db_compact_times.at(0).size() << endl;
    if(output.db_compact_times.at(0).size() > 0) {
        sprintf(buffer, "%d,", 0 );
        db_compact_fs << buffer;
        get_config_and_stats(db_compact_fs, output, output.db_compact_times.at(0), false, divide_by_nanoseconds);
        sprintf(buffer, "%d,", 1 );
        db_compact_fs << buffer;
        get_config_and_stats(db_compact_fs, output, output.db_compact_times.at(1), false, divide_by_nanoseconds);

        db_compact_fs << endl;
    }
    //extreme_debug_log << "got here7" << endl;

    //means there is a line between timing info for different configs
    op_fs << endl;
    op_breakdown_fs << endl;
    section_fs << endl;
        storage_fs << endl; 
    clock_eval_fs << endl;

cleanup:
    if(op_fs.is_open()) {
        op_fs.close();
    }
    if(op_breakdown_fs.is_open()) {
        op_breakdown_fs.close();
    }
    if(section_fs.is_open()) {
        section_fs.close();
    }
    if(storage_fs.is_open()) {
        storage_fs.close();
    }
    if(clock_eval_fs.is_open()) {
        clock_eval_fs.close();
    }
    if(db_checkpt_fs.is_open()) {
        db_checkpt_fs.close();
    }
    if(db_compact_fs.is_open()) {
        db_compact_fs.close();
    }
    return rc;
}



static int write_output_dirman(const string &results_path, bool first_run_dirman, 
                            const struct dirman_config_output &output) {
    //note: these will in essence be key value tables with key: rank with the addition of num clients, num servers, and num client and server nodes in addition

    int rc = RC_OK;

    debug_log << "about to start writing dirmans output \n";


    string time_pts_results_path = results_path + "dirman_time_pts.csv";
    ofstream time_pts_fs;
    string clock_eval_results_path = results_path + "dirman_clocks_eval.csv";
    ofstream clock_eval_fs;

    if(first_run_dirman) {

        time_pts_fs.open(time_pts_results_path, std::fstream::out);
        time_pts_fs << "#categories: " << endl;
        time_pts_fs << "#0:mpi init done, 1:dirman setup done, 2:register op done, 3:generate contact info done, 4:run time" << endl;
        time_pts_fs << endl;
        time_pts_fs << "code,";
        time_pts_fs << get_config_header();
        time_pts_fs << get_stats_header_w_median();

        //struct timeval start, mpi_init_done, dirman_init_done, register_done, generate_done, finalize;
        clock_eval_fs.open(clock_eval_results_path, std::fstream::out); 
        clock_eval_fs << "#categories: " << endl;
        clock_eval_fs << "#0:full init time, 1:full runtime" << endl;
        clock_eval_fs << endl;
        clock_eval_fs << "category,";
        clock_eval_fs << get_config_header();
        clock_eval_fs << get_stats_header_w_median();
    }
    else {
        time_pts_fs.open(time_pts_results_path, std::fstream::app);
        clock_eval_fs.open(clock_eval_results_path, std::fstream::app);
    }

    if(!time_pts_fs) {
        error_log << "error. failed to open time pts results file" << endl;
        rc = RC_ERR;
        goto cleanup;
    }
    else if(!clock_eval_fs) {
        error_log << "error. failed to open clock eval results file" << endl;
        rc = RC_ERR;
        goto cleanup;   
    }
    char buffer[512];


    //extreme_debug_log << "output.all_time_pts.size(): " << output.all_time_pts.size() << endl;
    for(int i=0; i<output.all_time_pts.size(); i++) {
        vector<double> time_pts = output.all_time_pts.at(i);
        //extreme_debug_log << "num time pts for category " << i << ": " << time_pts.size() << endl;
        for(int i = 0; i < time_pts.size(); i++) {
            time_pts[i] = (time_pts[i] / pow(10, 9));
        }
        if(time_pts.size() > 0) {
            sprintf(buffer, "%d,", i);
            time_pts_fs << buffer;
            get_config_and_stats(time_pts_fs, output, time_pts, false);
        }
    }




    //struct timeval start, mpi_init_done, register_ops_done, db_init done, dirman_init_done /
    for(int i=0; i<output.clock_times_eval.size(); i++) {
        vector<long double> clock_times_eval = output.clock_times_eval.at(i);
        for(int i = 0; i < clock_times_eval.size(); i++) {
            clock_times_eval[i] = (clock_times_eval[i] / pow(10, 9));
        }   
        debug_log << "clock times eval for category " << i << ": " << clock_times_eval.size() << endl;
        if(clock_times_eval.size() > 0) {
            sprintf(buffer, "%d,", i);
            clock_eval_fs << buffer;
            get_config_and_stats(clock_eval_fs, output, clock_times_eval, false);
            //extreme_debug_log << "clock_times[0]: " << clock_times_eval.at(0) << endl;
        }
    }


    //means there is a line between timing info for different configs
    time_pts_fs << endl;
    clock_eval_fs << endl;

cleanup:
    if(time_pts_fs.is_open()) {
        time_pts_fs.close();
    }
    if(clock_eval_fs.is_open()) {
        clock_eval_fs.close();
    }
    return rc;
}


static int write_op_numbering_guide(const string &results_path) {
    int rc = RC_OK;

    debug_log << "about to start writing op numbering guide \n";


    std::vector<string> op_names = {
        "Activate the Run",
        "Activate the Timestep",
        "Activate the Variable",

        "Catalog All Timesteps with Variable",
        "Catalog All Timesteps with Variable Whose Name Contains Substring",
        "Catalog All Application Runs",
        "Catalog All Timesteps for Run",
        "Catalog All Variables for Timestep",

        "Checkpoint the Database to Disk",

        "Create Application Run",
        "Create Timestep for Run",
        "Create Variable for Timestep in Run",
        "Create Variables for Timestep in Run in Batch",

        "Delete All Variables in Timestep Whose Name Contains Substring",
        "Delete All Metadata For Run (ID)",
        "Delete All Metadata For Timestep (ID)",
        "Delete All Metadata For Variable (ID)",
        "Delete All Metadata For Variable (Name, Path, Version)",

        "Deactivate/Processing",

        "Activate the Run Attribute",
        "Activate the Timestep Attribute",
        "Activate the Type",
        "Activate the Variable Attribute",

        "Catalog All Attributes on Run",
        "Catalog All Attributes on Run With Type",
        "Catalog All Attributes on Run With Type and Value",
        "Catalog All Attributes on Timestep",
        "Catalog All Attributes on Timestep With Type",
        "Catalog All Attributes on Timestep With Type and Value",

        "Catalog All Timesteps with Variable Attributes With Type and Variable",
        "Catalog All Timesteps with Variable Attributes With Type and Variable in Dimensions",
        "Catalog All Timesteps with Variable Attributes With Type, Variable, in Dimensions and with Value",
        "Catalog All Timesteps with Variable Attributes With Type, Variable, and Value",
        "Catalog All Timesteps with Variable Attributes With Type, and Variable Whose Name Contains Substring",
        "Catalog All Timesteps with Variable Attributes With Type, and Variable Whose Name Contains Substring in Dimensions",
        "Catalog All Timesteps with Variable Attributes With Type, and Variable Whose Name Contains Substring in Dimensions and with Value",
        "Catalog All Timesteps with Variable Attributes With Type, and Variable Whose Name Contains Substring with Value",

        "Catalog All Types with An Instance on Any Variable in Timestep",
        "Catalog All Types with An Instance on Variable in Dimensions in Timestep",
        "Catalog All Types with An Instance on Variable in Timestep",
        "Catalog All Types with An Instance on a Variable Whose Name Contains Substring in Dimensions in Timestep",
        "Catalog All Types with An Instance on a Variable Whose Name Contains Substring in Timestep",

        "Catalog All Variable Attributes in Timestep",
        "Catalog All Variable Attributes in Dimensions in Timestep",
        "Catalog All Variable Attributes of Type (ID) in Timestep",
        "Catalog All Variable Attributes of Type (Name and Version) in Timestep",
        "Catalog All Variable Attributes of Type (ID) in Dimensions",
        "Catalog All Variable Attributes of Type (Name and Version) in Dimensions in Timestep",
        "Catalog All Variable Attributes of Type (ID) on Variable (ID) in Timestep",
        "Catalog All Variable Attributes of Type (Name and Version) on Variable (Name and Version) in Timestep",
        "Catalog All Variable Attributes of Type (ID) on Variable (ID) in Dimensions in Timestep",
        "Catalog All Variable Attributes of Type (Name and Version) on Variable (Name and Version) in Dimensions in Timestep",
        "Catalog All Variable Attributes of Type (ID) on Variable (ID) in Dimensions with Value in Timestep",
        "Catalog All Variable Attributes of Type (ID) on Variable (ID) with Value in Timestep",
        "Catalog All Variable Attributes of Type (ID) on Variable Whose Name Contains Substring in Timestep",
        "Catalog All Variable Attributes of Type (ID) on Variable Whose Name Contains Substring in Dimensions in Timestep",
        "Catalog All Variable Attributes of Type (ID) on Variable Whose Name Contains Substring in Dimensions with Value in Timestep",
        "Catalog All Variable Attributes of Type (ID) on Variable Whose Name Contains Substring with Value in Timestep",
        "Catalog All Variable Attributes on Variable (ID) in Timestep",
        "Catalog All Variable Attributes on Variable (Name and Version) in Timestep",
        "Catalog All Variable Attributes on Variable (ID) in Dimensions in Timestep",
        "Catalog All Variable Attributes on Variable (Name and Version) in Dimensions in Timestep",
        "Catalog All Variable Attributes on Variable Whose Name Contains Substring in Timestep",
        "Catalog All Variable Attributes on Variable Whose Name Contains Substring in Dimensions in Timestep",

        "Catalog All Types in Run",
        "Create Type",
        "Create Types in Batch",

        "Delete All Metadata For Type (ID)",
        "Delete All Metadata For Type (Name and Version)",

        "Create Attribute on Run",
        "Create Attributes on in Batch Run",
        "Create Attribute on Timestep",
        "Create Attributes on Timestep in Batch",
        "Create Attribute on Variable",
        "Create Attributes on Variable in Batch",

        "Shutdown",

        "Abort Transaction",
        "Begin Transaction",
        "Commit Transaction"
    };

    string file_path = results_path + "op_numbering_guide.csv";
    ofstream file_path_fs;


    file_path_fs.open(file_path, std::fstream::out);
    file_path_fs << "op number, op name" << endl;

    if(!file_path_fs) {
        error_log << "error. failed to op numbering guide file" << endl;
        rc = RC_ERR;
        goto cleanup;
    }

    char buffer[512];



    if(op_names.size() != NUM_OPS) {
        error_log << "Error. Was expecting " << NUM_CLIENT_OPS << " but received " << op_names.size() << " instead" << endl;
    }

    for(int i=0; i<op_names.size(); i++) {
        int op_num = i*100 + 1000;
        //adjusts for the fact that we left a gap between the shutdown op and the new transaction ops
        if(op_names[i] == "Abort Transaction") {
            op_num = MD_ABORT_TRANSACTION_START;
        }
        else if(op_names[i] == "Begin Transaction") {
            op_num = MD_BEGIN_TRANSACTION_START;
        }
        else if(op_names[i] == "Commit Transaction") {
            op_num = MD_COMMIT_TRANSACTION_START;
        }
        sprintf(buffer, "%d,%s", 
                op_num, op_names.at(i).c_str());
        file_path_fs << buffer << endl;     
    }


cleanup:
    if(file_path_fs.is_open()) {
        file_path_fs.close();
    }

    return rc;
}



template <class T1>
static void get_config_params(char *buffer, T1 output)
{
    sprintf(buffer, "%d,%d,%llu,%d,%llu,%d,%llu,%u,%u,%u,%u,%u,", 
        output.config.num_write_client_procs / output.config.num_server_procs,
        output.config.num_server_nodes, 
        output.config.num_write_client_procs, output.config.num_write_client_nodes, 
        output.config.num_read_client_procs, output.config.num_read_client_nodes, 
        output.config.num_timesteps, output.config.num_timesteps_per_checkpt, output.config.write_type,
        output.config.server_type, output.config.index_type, output.config.checkpt_type
        );

}

// static void get_stats(char *buffer, vector<vector<double>> data_pts, bool div_pow )
// {
//     if(data_pts.size() == 6) {
//         sprintf(buffer, "%lu,%lf,%lf,%lf,%lf,%lf,%lf", data_pts.back().size(),
//             get_mean(data_pts.at(0)), get_mean(data_pts.at(1)), get_mean(data_pts.at(2)),
//             get_mean(data_pts.at(3)), get_mean(data_pts.at(4)), get_mean(data_pts.at(5))
//             ); 
//     }
//     else if(data_pts.size() == 5) {
//         sprintf(buffer, "%lu,%lf,%lf,%lf,%lf,%lf", data_pts.back().size(),
//             get_mean(data_pts.at(0)), get_mean(data_pts.at(1)), get_mean(data_pts.at(2)),
//             get_mean(data_pts.at(3)), get_mean(data_pts.at(4))
//             ); 
//     }
// }

static void get_stats(char *buffer, vector<stats> output, bool div_pow )
{
    if(output.size() == 8) {
        if(div_pow) {
            sprintf(buffer, "%lu,%Lf,%Lf,%Lf,%Lf,%Lf,%Lf,%Lf,%Lf", output.back().size(),
                output.at(0).avg/pow(10, 9), output.at(1).avg/pow(10, 9), output.at(2).avg/pow(10, 9),
                output.at(3).avg/pow(10, 9), output.at(4).avg/pow(10, 9), output.at(5).avg/pow(10, 9),
                output.at(6).avg/pow(10, 9), output.at(7).avg/pow(10, 9)
                );
        }
        else {
            sprintf(buffer, "%lu,%Lf,%Lf,%Lf,%Lf,%Lf,%Lf,%Lf,%Lf", output.back().size(),
                output.at(0).avg, output.at(1).avg, output.at(2).avg,
                output.at(3).avg, output.at(4).avg, output.at(5).avg, output.at(6).avg,
                output.at(7).avg
                ); 
        }
    }
    else if(output.size() == 7) {
        if(div_pow) {
            sprintf(buffer, "%lu,%Lf,%Lf,%Lf,%Lf,%Lf,%Lf,%Lf", output.back().size(),
                output.at(0).avg/pow(10, 9), output.at(1).avg/pow(10, 9), output.at(2).avg/pow(10, 9),
                output.at(3).avg/pow(10, 9), output.at(4).avg/pow(10, 9), output.at(5).avg/pow(10, 9),
                output.at(6).avg/pow(10, 9)
                );
        }
        else {
            sprintf(buffer, "%lu,%Lf,%Lf,%Lf,%Lf,%Lf,%Lf,%Lf", output.back().size(),
                output.at(0).avg, output.at(1).avg, output.at(2).avg,
                output.at(3).avg, output.at(4).avg, output.at(5).avg, output.at(6).avg
                ); 
        }
    }
    else {
        error_log << "error. get_stats was given stats of size " << output.size() << endl;
    }
    // if(output.size() == 6) {
    //     if(div_pow) {
    //         sprintf(buffer, "%lu,%Lf,%Lf,%Lf,%Lf,%Lf,%Lf", output.back().size(),
    //             output.at(0).avg/pow(10, 9), output.at(1).avg/pow(10, 9), output.at(2).avg/pow(10, 9),
    //             output.at(3).avg/pow(10, 9), output.at(4).avg/pow(10, 9), output.at(5).avg/pow(10, 9)
    //             );
    //     }
    //     else {
    //         sprintf(buffer, "%lu,%Lf,%Lf,%Lf,%Lf,%Lf,%Lf", output.back().size(),
    //             output.at(0).avg, output.at(1).avg, output.at(2).avg,
    //             output.at(3).avg, output.at(4).avg, output.at(5).avg
    //             ); 
    //     }
    // }
    // else if(output.size() == 5) {
    //     if(div_pow) {
    //         sprintf(buffer, "%lu,%Lf,%Lf,%Lf,%Lf,%Lf", output.back().size(),
    //             output.at(0).avg/pow(10, 9), output.at(1).avg/pow(10, 9), output.at(2).avg/pow(10, 9),
    //             output.at(3).avg/pow(10, 9), output.at(4).avg/pow(10, 9)
    //             ); 
    //     }
    //     else {
    //         sprintf(buffer, "%lu,%Lf,%Lf,%Lf,%Lf,%Lf", output.back().size(),
    //             output.at(0).avg, output.at(1).avg, output.at(2).avg,
    //             output.at(3).avg, output.at(4).avg
    //             ); 
    //     }
    // }
}

// //double
// static void get_stats(stats output, char *buffer, bool div_pow )
// {
//     double max = output.max;
//     double min = output.min;
//     double avg = output.avg;
//     double variance = output.variance;
//     double std_dev = get_std_dev(variance);

//     if(div_pow) {
//         sprintf(buffer, "%lu,%lf,%lf,%lf,%lf,%lf", output.size(), 
//             max/pow(10, 9), min/pow(10, 9), avg/pow(10, 9), variance/pow(10, 18), std_dev/pow(10, 9)); 
//     }
//     else {
//         sprintf(buffer, "%lu,%lf,%lf,%lf,%lf,%lf", output.size(), max, min, avg, variance, std_dev); 
//     }
// }

static void get_stats(char *buffer, stats output, bool div_pow)
{

    long double max = output.max;
    long double min = output.min;
    long double avg = output.avg;
    long double variance = output.variance;
    long double std_dev = get_std_dev(variance);

    if(div_pow) {
        sprintf(buffer, "%lu,%Lf,%Lf,%Lf,%Lf,%Lf", output.size(), 
            max/pow(10, 9), min/pow(10, 9), avg/pow(10, 9), variance/pow(10, 18), std_dev/pow(10, 9)); 
    }
    else {
        sprintf(buffer, "%lu,%Lf,%Lf,%Lf,%Lf,%Lf", output.size(), max, min, avg, variance, std_dev); 
    } 
}

static void get_stats(char *buffer, stats_size output, bool div_pow)
{
    uint64_t max = output.max;
    uint64_t min = output.min;
    uint64_t avg = output.avg;
    uint64_t variance = output.variance;
    uint64_t std_dev = get_std_dev(variance);

    if(div_pow) {
        sprintf(buffer, "%lu,%llu,%llu,%llu,%llu,%llu", output.size(), 
            max/pow(10, 9), min/pow(10, 9), avg/pow(10, 9), variance/pow(10, 18), std_dev/pow(10, 9)); 
    }
    else {
        sprintf(buffer, "%lu,%llu,%llu,%llu,%llu,%llu", output.size(), max, min, avg, variance, std_dev); 
    }
}


template <class T1, class T2>
static void get_config_and_stats(ofstream &file, T1 output, T2 my_stats, bool print_times_per_proc_and_run, bool div_pow, bool suppress_endl)
{
    char config_buffer[512], stats_buffer[512];
    get_config_params(config_buffer, output);
    file << config_buffer;
    if(print_times_per_proc_and_run) {
        file << get_times_per_run(output, my_stats) << "," << get_times_per_proc(output, my_stats) << ",";
    }
    get_stats(stats_buffer, my_stats, div_pow);
    file << stats_buffer;
    if(!suppress_endl) {
        file << endl;
    }
}

string conver_pt_id_to_name(uint16_t pt_id) {
    switch(pt_id) {
        case TOTAL_INIT_TIME: return "TOTAL_INIT_TIME";
        case PER_RUN_CREATE_TIME: return "PER_RUN_CREATE_TIME";
        case TOTAL_TIMESTEP_CREATE_TIME: return "TOTAL_TIMESTEP_CREATE_TIME";
        case TOTAL_TIMESTEP_ACTIVATE_TIME: return "TOTAL_TIMESTEP_ACTIVATE_TIME";
        case CREATE_COLLECTIVE_ATTRS_TIME: return "CREATE_COLLECTIVE_ATTRS_TIME";
        case DIRMAN_INIT_TIME: return "DIRMAN_INIT_TIME";
        case TOTAL_RUN_TIME: return "TOTAL_RUN_TIME";
        case MPI_INIT_TIME: return "MPI_INIT_TIME";
        case SERVER_INIT_TIME: return "SERVER_INIT_TIME";
        case CREATE_RUN_TIME: return "CREATE_RUN_TIME";
        case CREATE_TYPES_TIME: return "CREATE_TYPES_TIME";
        case CREATE_VARS_TIME: return "CREATE_VARS_TIME";
        case CREATE_VAR_ATTRS_TIME: return "CREATE_VAR_ATTRS_TIME";
        case CREATE_TIMESTEP_ATTRS_TIME: return "CREATE_TIMESTEP_ATTRS_TIME";
        case CREATE_RUN_ATTRS_TIME: return "CREATE_RUN_ATTRS_TIME";
        case GATHER_ATTRS_TIME: return "GATHER_ATTRS_TIME";
        case CHECKPOINT_DB_TIME: return "CHECKPOINT_DB_TIME";
        default:
            error_log << "pt id: " << pt_id << ", did not match an expected category" << endl;
            return "";
    }

}

//////////////////////////////////////////////////////////////////////////////
//just for dirman

static void get_stats(char *buffer, vector<double> data_pts, bool div_pow )
{
    double max = get_max(data_pts);
    double min = get_min(data_pts);
    double avg = get_mean(data_pts);
    double variance = get_variance(data_pts);
    double std_dev = get_std_dev(data_pts);
    double median = get_median(data_pts);
    if(div_pow) {
        sprintf(buffer, "%lu,%lf,%lf,%lf,%lf,%lf,%lf", data_pts.size(), 
            max/pow(10, 9), min/pow(10, 9), avg/pow(10, 9), variance/pow(10, 18), std_dev/pow(10, 9), median/pow(10, 9)); 
    }
    else {
        sprintf(buffer, "%lu,%lf,%lf,%lf,%lf,%lf,%lf", data_pts.size(), max, min, avg, variance, std_dev, median); 
    }
}

static void get_stats(char *buffer, vector<long double> data_pts, bool div_pow)
{

    long double max = get_max(data_pts);
    long double min = get_min(data_pts);
    long double avg = get_mean(data_pts);
    long double variance = get_variance(data_pts);
    long double std_dev = get_std_dev(data_pts);
    long double median = get_median(data_pts);
    // sprintf(buffer, "%lu,%Lf,%Lf,%Lf,%Lf,%Lf,%Lf", data_pts.size(), max, min, avg, variance, std_dev, median);
    if(div_pow) {
        sprintf(buffer, "%lu,%Lf,%Lf,%Lf,%Lf,%Lf,%Lf", data_pts.size(), 
            max/pow(10, 9), min/pow(10, 9), avg/pow(10, 9), variance/pow(10, 18), std_dev/pow(10, 9), median/pow(10, 9)); 
    }
    else {
        sprintf(buffer, "%lu,%Lf,%Lf,%Lf,%Lf,%Lf,%Lf", data_pts.size(), max, min, avg, variance, std_dev, median); 
    } 
}

// template <class T1, class T2>
// static void get_config_and_stats(ofstream &file, T1 output, vector<T2> data_pts, bool print_times_per_proc_and_run, bool div_pow, bool suppress_endl)
// {
//     char config_buffer[512], stats_buffer[512];
//     get_config_params(config_buffer, output);
//     file << config_buffer;
//     if(print_times_per_proc_and_run) {
//         file << get_times_per_run(output, data_pts) << "," << get_times_per_proc(output, data_pts) << ",";
//     }
//     get_stats(stats_buffer, data_pts, div_pow);
//     file << stats_buffer;
//     if(!suppress_endl) {
//         file << endl;
//     }
// }


