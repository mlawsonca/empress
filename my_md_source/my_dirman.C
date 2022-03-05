
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

#include "dirman/DirMan.hh"
#include "opbox/OpBox.hh"
#include <netdb.h> //needed for gethostbyname


using namespace std;

std::string default_config_string = R"EOF(
# Select a transport to use for nnti 

nnti.logger.severity       error
nnti.transport.name        ibverbs
whookie.interfaces         ib0,lo

#config.additional_files.env_name.if_defined   CONFIG
lunasa.lazy_memory_manager   malloc
lunasa.eager_memory_manager  tcmalloc

#centralized dirman - sticks all the directory info on one node (called root). We use roles
# to designate which node is actually the root.

dirman.type           centralized
dirman.host_root true
master.whookie.port   1990

node_role             dirman

# Turn these on if you want to see more debug messages
#bootstrap.debug           true
#whookie.debug             true 
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
int CLOCK_TIMES_DONE = 99;

int ERR_GENERATE_CONTACT_INFO = 10000;

// uint32_t MAX_EAGER_MSG_SIZE = 2048;
uint32_t MAX_EAGER_MSG_SIZE;

// int RC_OK = 0;
// int RC_ERR = -1;

bool debug_logging = false;
bool error_logging = true;

// bool md_shutdown = false;
pthread_cond_t cond1 = PTHREAD_COND_INITIALIZER;
pthread_mutex_t lock1 = PTHREAD_MUTEX_INITIALIZER;

static bool output_timing = true;

std::vector<std::chrono::high_resolution_clock::time_point> time_pts;
std::vector<int> catg_of_time_pts;

static void add_timing_point(chrono::high_resolution_clock::time_point time_pt, int catg) {
    if (output_timing) {
        time_pts.push_back(time_pt);
        catg_of_time_pts.push_back(catg);
    }
}


void add_timing_point(int catg) {
    if (output_timing) {
        time_pts.push_back(chrono::high_resolution_clock::now());
        catg_of_time_pts.push_back(catg);
    }
}

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
    debug_log(name);
    
    int num_time_pts = 6;
    catg_of_time_pts.reserve(num_time_pts);
 
    struct timeval start, mpi_init_done, dirman_init_done, register_done, generate_done, finalize;
    vector<long double> clock_times;
    clock_times.reserve(num_time_pts);

    time_pts.reserve(num_time_pts);

    gettimeofday(&start, NULL);
    int zero_time_sec = 86400 * (start.tv_sec / 86400);
    clock_times.push_back( (start.tv_sec - zero_time_sec) + start.tv_usec / 1000000.0);
    chrono::high_resolution_clock::time_point start_time = chrono::high_resolution_clock::now();
    add_timing_point(start_time, START);

    int rc;
    debug_log("got here 1");
    MPI_Init(&argc, &argv);
    
    gettimeofday(&mpi_init_done, NULL);
    clock_times.push_back( (mpi_init_done.tv_sec - zero_time_sec) + mpi_init_done.tv_usec / 1000000.0);
    add_timing_point(MPI_INIT_DONE);

    debug_log("got here 2");

    string dir_path="/metadata/testing";
    string dir_info="Dirman for empress testing";
    setup_dirman(dir_path, dir_info);
    string myHexID = opbox::GetMyID().GetHex();
    debug_log("Current dirman nodeid is:   " + myHexID);

    gettimeofday(&dirman_init_done, NULL);
    clock_times.push_back((dirman_init_done.tv_sec - zero_time_sec) + dirman_init_done.tv_usec / 1000000.0);
    add_timing_point(DIRMAN_SETUP_DONE);


    opbox::RegisterOp<OpFullShutdownMeta>();
    
    opbox::net::Attrs nnti_attrs;
    opbox::net::GetAttrs(&nnti_attrs);
    MAX_EAGER_MSG_SIZE = nnti_attrs.max_eager_size;

    gettimeofday(&register_done, NULL);
    clock_times.push_back((register_done.tv_sec - zero_time_sec) + register_done.tv_usec / 1000000.0);
    add_timing_point(REGISTER_OPS_DONE); 
    debug_log("got here 3");


    //write the URL to the contact info file so the clients and servers can find it
    rc = generate_contact_info (myHexID);
    if(rc != RC_OK) {
    	add_timing_point(ERR_GENERATE_CONTACT_INFO);
        goto cleanup;
    }
    debug_log("got here 4");

    gettimeofday(&generate_done, NULL);
    clock_times.push_back((generate_done.tv_sec - zero_time_sec) + generate_done.tv_usec / 1000000.0);
    add_timing_point(GENERATE_CONTACT_INFO_DONE);

    debug_log("got here 5");

    // while(!md_shutdown) {
    //      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    //  }
    // cout << "am waiting for cond" << endl;
    pthread_cond_wait(&cond1, &lock1);
    // cout << "am continuing after cond" << endl;

cleanup:

    gettimeofday(&finalize, NULL);
    clock_times.push_back((finalize.tv_sec - zero_time_sec) + finalize.tv_usec / 1000000.0);
    add_timing_point(FINALIZE);
  	

  	if(output_timing) {
	    //prevent it from buffering the printf statements
	    setbuf(stdout, NULL);

	    cout << CLOCK_TIMES_START << " ";
	     for(int i=0; i<clock_times.size(); i++) {
	        printf("%.6Lf ", clock_times[i]);
	        // std::cout << total_time << " " << catg_of_time_pts.at(i) << " ";
	    }
	    // cout << CLOCK_TIMES_DONE << " ";
	    // cout << "time_pts.size(): " << time_pts.size() << endl;
	    for(int i=0; i<time_pts.size(); i++) {
	        std::chrono::duration<double, std::nano> fp_ns = time_pts.at(i) - start_time;
	        double total_time = fp_ns.count();
	        printf("%d %10.0f ", catg_of_time_pts.at(i), total_time);

	        // std::cout << total_time << " " << catg_of_time_pts.at(i) << " ";
	    }
	    std::cout << std::endl;
	}

    debug_log("Finalizing");
    faodel::bootstrap::Finish();
    MPI_Finalize();
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
    faodel::Configuration config(default_config_string);
    config.Append("whookie.port", to_string(1990));
    config.AppendFromReferences();
    faodel::bootstrap::Start(config, dirman::bootstrap);
    debug_log("Creating dirman"); 
    ok = dirman::HostNewDir(faodel::DirectoryInfo(dir_path, dir_info)); 
    assert(ok && "Root couldn't host a new directory?");
}

