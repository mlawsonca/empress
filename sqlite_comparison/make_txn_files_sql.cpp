#include <iostream>
#include <fstream>
#include <string>
#include <stdlib.h>
#include <math.h>       /* cbrt */
#include <vector>

using namespace std;

bool local = false;

struct debugLog {
  private:
    bool on;

  public:
  debugLog(bool turn_on) {
    on = turn_on;
  }
  void turn_on_logging() {
    on = true;
  }
  void turn_off_logging() {
    on = false;
  }
  template<typename T> debugLog& operator << (const T& x) {
   if(on) {
      cout << x;
    }
    return *this;
  }
  debugLog& operator<<(std::ostream& (*manipulator)(std::ostream&)) {
   if(on) {
      cout << manipulator;
    }
    return *this;
  }
};

bool debug_logging = false;
bool extreme_debug_logging = false;

bool write_hdf5 = true;

static debugLog debug_log = debugLog(debug_logging);
static debugLog extreme_debug_log = debugLog(extreme_debug_logging);


void test_fixed_procs_per_node();

void generate_txn_file(string cluster, uint32_t job_num, uint32_t iteration, uint32_t num_timesteps, 
						uint32_t num_write_procs,
						uint32_t num_write_x_procs, uint32_t num_write_y_procs, uint32_t num_write_z_procs,
						uint64_t nx, uint64_t ny, uint64_t nz
						);

template <class T>
void find_nearest_divisible (T nx, T ny, T nz, T &num_read_procs, T &num_read_x_procs, T &num_read_y_procs, T &num_read_z_procs);

template <class T>
void find_roots(T value, T &npx, T &npy, T &npz);

int main(int argc, char **argv) {



	test_fixed_procs_per_node();
							    // add return value/tests to make sure things went okay?

	// test_num_server_procs_per_node();

}

// ~~~!!!!!this is where I am leaving off  !!!!!~~~ - none of this has been modified for the new txn required format (or singularity execution commands)
// (or any other changes)


void generate_txn_file(string cluster, uint32_t job_num, uint32_t iteration, uint32_t num_timesteps, 
						uint32_t num_write_procs,
						uint32_t num_write_x_procs, uint32_t num_write_y_procs, uint32_t num_write_z_procs,
						uint64_t nx, uint64_t ny, uint64_t nz
						) 
{
		char cmd_line_args[256];

		//dirman contact info, estm num time pts, bool load_db, bool output_db, job_id 

		snprintf(cmd_line_args, sizeof(cmd_line_args), "%d %d %d %d %d %d %d", 
				num_write_x_procs, num_write_y_procs, num_write_z_procs, 
				nx, ny, nz, num_timesteps);


		bool cluster_a = !cluster.compare("cluster_a");
		bool cluster_b = !cluster.compare("cluster_b");
	 	bool cluster_d = !cluster.compare("cluster_d");
	 	bool cluster_c = !cluster.compare("cluster_c");
	 	bool cluster_e = !cluster.compare("cluster_e");

		//todo - this will change for titan
		string account = "FILL_IN_DESIRED_VALUE";

		string testing_file_name; 

		string sbatch_path;
		sbatch_path = "FILL_IN_DESIRED_VALUE";
		testing_file_name = "sqlite_objector_comparison";

		string job_name = "txn."+cluster + "_" + to_string(job_num); 

		string testing_params;
		testing_params = to_string(num_write_procs) +  "_" + to_string(num_timesteps) + "_" + to_string(iteration);

		string sbatch_file_name = testing_params + ".sl";
		string sbatch_file_path = sbatch_path + "/" + sbatch_file_name;
		debug_log << " sbatch_file_path: " << sbatch_file_path << endl;
		string results_file_name = testing_params + ".log";
		string sbatch_script_path= sbatch_path + "/"  + "sbatch_script";

		debug_log << " results file path: " << sbatch_file_path << endl;

	
		ofstream file;
		file.open(sbatch_file_path);
		file << "#!/bin/bash" << endl << endl;
		file << "#SBATCH --account=" << account << endl;
		// file << "#SBATCH --partition=ZZZ" << endl;

		file << "#SBATCH --partition=YYY" << endl;
		file << "#SBATCH --job-name=" << job_name << endl;
		file << "#SBATCH --nodes=" << 1 << endl;

		file << "#SBATCH --time=0:20:00" << endl;
		file << endl;
		// file << "echo starting txn testing" << endl;
		file << endl;
		file << "SOURCE_DIR=FILL_IN_DESIRED_VALUE" << endl;
		file << "OUTPUT_DIR=FILL_IN_DESIRED_VALUE" << endl;	


		file << "TESTING_FILE_NAME=" << testing_file_name << endl;
		file << "TESTING_LOG_FILE=${OUTPUT_DIR}/${TESTING_FILE_NAME}_"<<
				results_file_name << endl;

		file << "${SOURCE_DIR}/${TESTING_FILE_NAME} " << cmd_line_args << " &> ${TESTING_LOG_FILE} " << endl;
		file << endl;
	
		if(iteration > 0) { 
			file << "rm -f " << sbatch_path << "/${SLURM_JOBID}_*" << endl;
		}
		file.close();

		ofstream sbatch_script;
		if(job_num == 0) {
			sbatch_script.open(sbatch_script_path);	
			sbatch_script << "#! /bin/bash \n\n";
			sbatch_script << "jid0=$(sbatch " << sbatch_file_name << " | awk '{print $4}')\n"; 
		}
		else {
			sbatch_script.open(sbatch_script_path, std::ofstream::app);				
			sbatch_script << "jid"  << job_num << "=$(sbatch --dependency=afterany:$jid" << job_num-1 << " " << sbatch_file_name << " | awk '{print $4}')\n"; 
		}
		sbatch_script.close();	
}


void test_fixed_procs_per_node() 
{

	vector<uint32_t> num_write_procs = {1000, 2000, 4000, 8000, 16000};

	uint32_t num_timesteps = 3;
	uint32_t num_iterations = 10;
	// uint32_t num_iterations = 1;

	uint32_t num_vars = 10;
	uint32_t bytes_per_data_pt = 8; //doubles are 8 bytes

	uint32_t num_procs_opts = num_write_procs.size();
	// uint32_t num_types_opts = sizeof(num_types) / sizeof(uint32_t);

	uint32_t num_write_x_procs[num_procs_opts];
	uint32_t num_write_y_procs[num_procs_opts];
	uint32_t num_write_z_procs[num_procs_opts];


	for(uint32_t i=0; i<num_procs_opts; i++) {
		find_roots(num_write_procs[i], num_write_x_procs[i], num_write_y_procs[i], num_write_z_procs[i]);
	}

	//note: cluster_c and cluster_d actually have 3.5, but better to have the same data per chunk
	uint64_t RAM_per_proc = 4 * pow(10,9); //4 GB
	uint64_t bytes_per_proc_per_timestep = .1 * RAM_per_proc; //10% of RAM
	uint64_t bytes_per_chunk = bytes_per_proc_per_timestep / num_vars; 
	uint64_t data_pts_per_chunk = bytes_per_chunk / bytes_per_data_pt; 

	uint64_t ndx, ndy, ndz;
	//note - can just do this once for all 4 clusters since they all have the same RAM/proc (4GB)
	find_roots(data_pts_per_chunk, ndx, ndy, ndz);
	cout << "data_pts_per_chunk: " << data_pts_per_chunk << " ndx: " << ndx << " ndy: " << ndy << " ndz: " << 
			ndz << " confirming: ndx*ndy*ndz: " << ndx*ndy*ndz << endl;


	string cluster = "cluster_e";

	uint32_t job_num = 0;

	for (uint32_t j=0; j<num_procs_opts; j++) {
		// for (uint32_t l=0; l<num_types_opts; l++) { //no need to test 0 types any more
		for (int iteration = 0; iteration < num_iterations; iteration++) {
			uint32_t x_length_per_proc = ndx; 
			uint32_t y_length_per_proc = ndy;
			uint32_t z_length_per_proc = ndz; 
			uint32_t nx = x_length_per_proc * num_write_x_procs[j];
			uint32_t ny = y_length_per_proc * num_write_y_procs[j];
			uint32_t nz = z_length_per_proc * num_write_z_procs[j];


			uint32_t my_num_procs = num_write_procs[j];

			if (num_write_x_procs[j] * num_write_y_procs[j] * num_write_z_procs[j] != my_num_procs ) {
				cout << "error. requested write_x procs: " << num_write_x_procs[j] << " write_y procs: " << num_write_y_procs[j] <<
					" write_z procs: " << num_write_z_procs[j] << endl;
				cout << "this muliplies to " << (num_write_x_procs[j] * num_write_y_procs[j] * num_write_z_procs[j] ) <<
					" write procs instead of the requested " << my_num_procs << endl;
			}


			if (iteration == 0 ) {
				cout << "for " << my_num_procs << " write clients, npx for write: " << 
				num_write_x_procs[j] << " npy for write: " << num_write_y_procs[j] <<
					" and npz for write: " << num_write_z_procs[j] << endl;
			}


			generate_txn_file(cluster, job_num, iteration, num_timesteps, 
							my_num_procs, num_write_x_procs[j], num_write_y_procs[j], 
							num_write_z_procs[j], nx, ny, nz );
			job_num+=1;

		}

	}
}

// void find_roots(uint32_t value, uint32_t &npx, uint32_t &npy, uint32_t &npz) {
template <class T>
void find_roots(T value, T &npx, T &npy, T &npz) {
	T incr = cbrt(value);
	T decr = cbrt(value);
	while(true) {
		debug_log << "value: " << value << " incr: " << incr << " decr: " << decr << endl;
		if (value % decr == 0) {
			npx = decr;
			break;
		}
		else if(value % incr == 0) {
			npx = incr;
			break;
		}
		incr += 1;
		decr -= 1;
	}

	value = value / npx;
	incr = sqrt( value );
	decr = sqrt( value );
	while( true ) {
		debug_log << "value: " << value << " npx: " << npx << " incr: " << incr << " decr: " << decr << endl;

		if(value % incr == 0) {
			npy = incr;
			break;
		}
		else if (value % decr == 0) {
			npy = decr;
			break;
		}
		incr += 1;
		decr -= 1;
	}	
	npz = value / npy;
	debug_log << "value: " << value << " npx: " << npx << " npy: " << npy << endl;

	if(npy < npx) {
		T temp = npy;
		npy = npx;
		npx = temp;
	}
	if(npz < npy) {
		T temp = npy;
		npy = npz;
		npz = temp;
	}
	debug_log << "value: " << value*npx << " npx: " << npx << " npy: " << npy << " npz: " << npz << endl;

}

// void find_next_roots(uint32_t value, uint32_t &npx, uint32_t &npy, uint32_t &npz) {

template <class T>
void find_next_roots(T value, T &npx, T &npy, T &npz) {
	T incr = npx+1;
	T decr = npx-1;
	while(true) {
		extreme_debug_log << "value: " << value << " incr: " << incr << " decr: " << decr << endl;
		if (value % decr == 0) {
			npx = decr;
			break;
		}
		else if(value % incr == 0) {
			npx = incr;
			break;
		}
		incr += 1;
		decr -= 1;
	}

	value = value / npx;
	incr = sqrt( value );
	decr = sqrt( value );
	while( true ) {
		extreme_debug_log << "value: " << value << " npx: " << npx << " incr: " << incr << " decr: " << decr << endl;

		if(value % incr == 0) {
			npy = incr;
			break;
		}
		else if (value % decr == 0) {
			npy = decr;
			break;
		}
		incr += 1;
		decr -= 1;
	}
	npz = value / npy;

	if(npy < npx) {
		T temp = npy;
		npy = npx;
		npx = temp;
	}
	if(npz < npy) {
		T temp = npy;
		npy = npz;
		npz = temp;
	}
	extreme_debug_log << "value: " << value*npx << " npx: " << npx << " npy: " << npy << " npz: " << npz << endl;

}


// void find_nearest_divisible (uint32_t nx, uint32_t ny, uint32_t nz, uint32_t &num_procs, uint32_t &npx, uint32_t &npy, uint32_t &npz) {
template <class T>
void find_nearest_divisible (T nx, T ny, T nz, T &num_procs, T &npx, T &npy, T &npz) {
	T incr = num_procs + 1;
	T decr = num_procs - 1;

	extreme_debug_log << "num read_procs: " << num_procs << " npx: " << npx << 
		" npy: " << npy << " npz: " << npz << endl;

	extreme_debug_log << "nx: " << nx << " ny: " << ny << " nz: " << nz << endl;
	

	find_next_roots(num_procs, npx, npy, npz);

	debug_log << "less balanced roots: npx: " << npx << " npy: " << npy << " npz: " << npz << endl;

	if ( (nx % npx == 0) && (ny % npy == 0) && (nz % npz != 0) ) {
		debug_log << "for the less balanced roots, npx,npy,npz do not match. continuing " << endl;
	}
	else if (npx * 3 < npz ) {
		debug_log << "for the less balanced roots, npx,npy,npz match but npx and npz are more than 3x different. continuing " << endl;
	}
	else { 
		debug_log << "for the less balanced roots, npx,npy,npz match and npx and npz are sufficiently close. returning " << endl;
		return;
	}

	while ( true ) {

		T incr_cbrt = cbrt(incr);
		find_roots(incr, npx, npy, npz);

		extreme_debug_log << "incr: " << incr << " incr npx: " << npx << " incr npy: " << npy <<
			" incr npz: " << npz << endl;

		if( (nx % npx == 0) && (ny % npy == 0) && (nz % npz == 0) ) {
			num_procs = incr;
			extreme_debug_log << "incr matches!" << endl;

			break;
		}

		T decr_cbrt = cbrt(decr);
		while(decr % decr_cbrt != 0) {
			decr_cbrt -= 1;
		}
		T decr_rt2 = sqrt( decr / decr_cbrt );
		while( (decr / decr_cbrt) % decr_rt2 != 0) {
			decr_rt2 += 1;
		}
		npx = decr_cbrt;
		npy = decr_rt2;
		npz = decr / (decr_cbrt * decr_rt2);
		if(npz < npy) {
			T temp = npy;
			npy = npz;
			npz = temp;
		}
		extreme_debug_log << "decr: " << decr << " decr npx: " << npx << " decr npy: " << npy <<
			" decr npz: " << npz << endl;

		if( (nx % npx != 0) || (ny % npy != 0) || (nz % npz != 0) ) {
			incr += 1;
			decr -= 1;
		}
		else {
			extreme_debug_log << "decr matches!" << endl;
			num_procs = decr;
			break;
		}
	}

}

