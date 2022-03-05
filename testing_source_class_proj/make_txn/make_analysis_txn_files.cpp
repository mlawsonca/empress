#include <iostream>
#include <fstream>
#include <string>
#include <stdlib.h>
// #include <math.h>       /* cbrt */
#include <vector>
// #include <algorithm>    // std::min
// #include "../include/common/my_metadata_args.h"
// #include <map>
#include <unordered_map>
#include "testing_configs.hh"


using namespace std;

// bool local = false;

// bool debug_logging = false;
// bool extreme_debug_logging = false;

// // bool write_hdf5 = true;

// static debugLog debug_log = debugLog(debug_logging);
// static debugLog extreme_debug_log = debugLog(extreme_debug_logging);

void generate_slurm_file(string input_cluster, const testing_config &config, int num_procs, vector<string> output_folders,
            uint32_t job_num, int num_completed_iterations, bool short_time, string output_cluster
        );


int main(int argc, char **argv) {
    static bool hdf5 = false;
    int num_completed_iterations = 0;
    vector<string> clusters = {"cluster_d", "cluster_c", "cluster_e", "cluster_b", "cluster_a"};
    vector<string> input_clusters = {"cluster_d", "cluster_e"};

    // vector<string> output_folders;
    unordered_map<string,vector<string>> output_folders;

    if(hdf5) {
        output_folders = { 
            {"cluster_e", {"FILL_IN_WITH_DESIRED_VALUE"}},
            {"cluster_d", {"FILL_IN_WITH_DESIRED_VALUE"}}
        };
    }
    else {
        output_folders = { 
            {"cluster_e", {"FILL_IN_WITH_DESIRED_VALUE"}},
            {"cluster_d", {"FILL_IN_WITH_DESIRED_VALUE"}}
        };
    }

    // bool short_time = true;
    bool short_time = false;

    for(string output_cluster : clusters) {
        int job_num = 0;
        for(string input_cluster : input_clusters) {
            vector<testing_config> completed_configs = get_completed_testing_configs(input_cluster);

            for(testing_config config : completed_configs) {
                for (uint32_t num_procs : config.num_procs) {
                    if(num_procs < 8000) {
                        short_time = true;
                    }
                    generate_slurm_file(input_cluster, config, num_procs, output_folders[input_cluster], job_num, num_completed_iterations, short_time, output_cluster);
                    job_num += 1;
                }
            }
        }
    }
}


void generate_slurm_file(string input_cluster, const testing_config &config, int num_procs, vector<string> output_folders,
            uint32_t job_num, int num_completed_iterations, bool short_time, string output_cluster
        )
{
        bool cluster_b = !input_cluster.compare("cluster_b");
        bool cluster_a = !input_cluster.compare("cluster_a");
        bool cluster_d = !input_cluster.compare("cluster_d");
        bool cluster_c = !input_cluster.compare("cluster_c");
        bool cluster_e = !input_cluster.compare("cluster_e");

        string cluster_abreviation = "";
        if(!input_cluster.compare("cluster_b")) {
            cluster_abreviation = "b";
        }
        if(!input_cluster.compare("cluster_a")) {
            cluster_abreviation = "a";
        }
        else if(!input_cluster.compare("cluster_d")) {
            cluster_abreviation = "d";
        }
        else if(!input_cluster.compare("cluster_c")) {
            cluster_abreviation = "c";
        }
        else if(!input_cluster.compare("cluster_e")) {
            cluster_abreviation = "e";
        }

        string input_pfs_name = "FILL_IN_WITH_DESIRED_VALUE";

        cluster_b = !output_cluster.compare("cluster_b");
        cluster_a = !output_cluster.compare("cluster_a");
        cluster_d = !output_cluster.compare("cluster_d");
        cluster_c = !output_cluster.compare("cluster_c");
        cluster_e = !output_cluster.compare("cluster_e");

        string output_pfs_name = "FILL_IN_WITH_DESIRED_VALUE"


        string testing_file_name, read_testing_file_name;


        string sbatch_path;
        sbatch_path = "/FILL_IN_WITH_DESIRED_VALUE";

       

        string config_params_spaces = to_string(num_procs) + " " +
                        to_string(config.write_type) + " " + to_string(config.server_type) + " " + 
                        to_string(config.index_type) + " " + to_string(config.checkpt_type);

        string config_params_commas = to_string(num_procs) + "," +
                        to_string(config.write_type) + "," + to_string(config.server_type) + "," + 
                        to_string(config.index_type) + "," + to_string(config.checkpt_type);

        string config_params_underscores = to_string(num_procs) + "_" + to_string(config.write_type) + "_" + 
                        to_string(config.server_type) + "_" + to_string(config.index_type) + "_" + 
                        to_string(config.checkpt_type);

        string job_name = cluster_abreviation + "," + config_params_commas;
        string params = input_cluster + "_" + config_params_underscores;

        string sbatch_file_name = params + ".sl";
        string sbatch_file_path = sbatch_path + "/" + sbatch_file_name;
        cout << "am writing to " << sbatch_file_path << endl;

        string results_file_name = "analysis_" + params + ".log";
        string sbatch_script_path= sbatch_path + "/"  + "sbatch_script";

        ofstream file;
        file.open(sbatch_file_path);
        file << "#!/bin/bash" << endl << endl;
        if(cluster_c) {
            file << "#SBATCH --account=YYY" << endl;
        }
        else {
            file << "#SBATCH --account=XXX" << endl;
        }
        file << "#SBATCH --partition=ZZZ" << endl;
        file << "#SBATCH --job-name=" << job_name << endl;
        file << "#SBATCH --nodes=1" << endl;
        if(short_time) {
            file << "#SBATCH --time=8:00:00" << endl;
        }
        else {
            file << "#SBATCH --time=16:00:00" << endl;            
        }
        file << endl;
        file << endl;


        file << "SOURCE_DIR=/FILL_IN_WITH_DESIRED_VALUE" << endl;
        file << "OUTPUT_DIR=/FILL_IN_WITH_DESIRED_VALUE" << endl;   

        file << "TESTING_FILE_NAME=read_output" + to_string(job_num+num_completed_iterations) << endl;     
        file << endl;
        file << "export OMP_PROC_BIND=true" << endl;
        file << endl;


        file << "ALLNODES=`scontrol show hostname $SLURM_HOSTNAME`" << endl;
        file << "echo \"$ALLNODES\" " << endl;
        file << "echo \"srun ${SOURCE_DIR}/${TESTING_FILE_NAME} " << input_cluster << " " << config_params_spaces
             << " ${OUTPUT_DIR} ";
            for(int i = 0; i < output_folders.size(); i++) {
                file << output_folders[i] << " ";
            }
        file <<  "\" " << endl;

        file <<"srun ${SOURCE_DIR}/${TESTING_FILE_NAME} " << input_cluster << " " << config_params_spaces
             << " ${OUTPUT_DIR} ";

        for(int i = 0; i < output_folders.size(); i++) {
            file << output_folders[i] << " ";
        }
        file << " &> ${OUTPUT_DIR}/" << results_file_name << endl;
        file.close();

        ofstream sbatch_script;
        if(job_num == 0) {
            sbatch_script.open(sbatch_script_path); 
            sbatch_script << "#! /bin/bash \n\n";
            sbatch_script << "jid0=$(sbatch " << sbatch_file_name << " | awk '{print $4}')\n"; 
        }
        else {
            sbatch_script.open(sbatch_script_path, std::ofstream::app);             
            sbatch_script << "jid"  << job_num << "=$(sbatch " <<  sbatch_file_name << " | awk '{print $4}')\n"; 
        }
        sbatch_script.close();  

}



