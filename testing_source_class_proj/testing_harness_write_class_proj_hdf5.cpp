

#include <stdlib.h> //needed for atoi/atol/malloc/free/rand
#include <stdint.h> //needed for uint
#include <vector>
#include <stdio.h> //needed for printf
#include <assert.h> //needed for assert
#include <string.h> //needed for strcmp
// #include <chrono> //needed for high_resolution_clock
#include <math.h> //needed for pow()
#include <sys/time.h> //needed for timeval
#include <float.h> //needed for DBL_MAX
#include <numeric> //needed for accumulate#include <mpi.h>
#include <hdf5_hl.h>
#include <hdf5.h>

// #include <testing_harness_debug_helper_functions.hh> 
#include <my_metadata_client_hdf5.hh>
// #include <client_timing_constants_write_hdf5.hh>
#include <client_timing_constants_write_class_proj.hh>
#include <testing_harness_write_class_proj_hdf5.hh>
#include <testing_harness_helper_functions_hdf5.hh>
// #include <testing_harness_hdf5.hh>

using namespace std;

// static bool extreme_debug_logging = false;
// static bool debug_logging = false;
// bool testing_logging = false;
// static bool zero_rank_logging = false;
static bool error_logging = true;

debugLog error_log = debugLog(error_logging, false);
// debugLog //debug_log = debugLog(debug_logging, zero_rank_logging);
// debugLog //extreme_debug_log = debugLog(extreme_debug_logging, zero_rank_logging);
// debugLog //testing_log = debugLog(testing_logging, zero_rank_logging);

static bool do_debug_testing = false;

static bool output_timing = true;
static bool insert_by_batch = true;
// static bool gathered_writes = true;
// static bool gathered_writes;

std::vector<int> catg_of_time_pts;
std::vector<long double> time_pts;
// std::vector<int> db_checkpt_types;

int zero_time_sec;

void add_timing_point(int catg) {
    if (output_timing) {
        struct timeval now;
        gettimeofday(&now, NULL);
        time_pts.push_back( (now.tv_sec - zero_time_sec) + now.tv_usec / 1000000.0);
        catg_of_time_pts.push_back(catg);
    }
}

double clear_cache(uint32_t ndx, uint32_t ndy, uint32_t ndz, uint32_t num_vars);

/*
argv[1] = dirman hexid
argv[2] = number of subdivision of the x dimension (number of processes)
argv[3] = number of subdivisions of the y dimension (number of processes)
argv[4] = number of subdivisions of the z dimension (number of processes)
argv[5] = total length in the x dimension 
argv[6] = total length in the y dimension 
argv[7] = total length in the z dimension 
argv[8] = number of datasets to be stored in the database
argv[9] = number of types stored in each dataset
argv[10] = estimate of the number of testing time points we will have 
argv[11] = the number of EMPRESS (metadata) servers 
argv[12] = the job id (from the scheduler) 
*/
int main(int argc, char **argv) {
    int rc = RC_OK;
    // char name[100];
    // gethostname(name, sizeof(name));
    // //extreme_debug_log << name << endl;

    if (argc != 14) { 
        error_log << "Error. Program should be given 10 arguments. npx, npy, npz, nx, ny, nz,"
        		  << " number of timesteps, estm num time_pts, num_servers, job_id, read_npx, read_npy, read_npz, " 
        		  << "timesteps_per_checkpt" << endl;
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
    // uint32_t num_types = stoul(argv[9],nullptr,0);
    uint32_t estm_num_time_pts = stoul(argv[8],nullptr,0); 
    string job_id = argv[9];
    uint32_t num_read_x_procs = stoul(argv[10],nullptr,0);
    uint32_t num_read_y_procs = stoul(argv[11],nullptr,0);
    uint32_t num_read_z_procs = stoul(argv[12],nullptr,0);
    uint32_t num_timesteps_per_checkpt = stoul(argv[13],nullptr,0);

    //all 10 char plus null character
    uint32_t num_vars = 10;

    vector<string> var_names = {"temperat", "temperat", "pressure", "pressure", "velocity_x", "velocity_y", "velocity_z", "magn_field", "energy_var", "density_vr"};  
    char run_timestep_type_names[2][10] = {"max_temp0", "min_temp0"}; 
    vector<string> type_names = {"blob_freq0", "blob_ifreq0", "blob_rare0", "max_val_type0", "min_val_type0", 
    "note_freq0", "note_ifreq0", "note_rare0", "ranges_type10", "ranges_type20"}; 

    vector<string> run_timestep_type_names_vct = {"max_temp0", "min_temp0"};

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

    if(rank == 0) {
        //extreme_debug_log << "starting" << endl;
    }

    string run_name = "XGC";

    uint32_t version1 = 1;
    uint32_t version2 = 2;

    md_dim_bounds proc_dims;

    int num_run_timestep_types = 2;
    uint32_t num_types = 10;

    //extreme_debug_log << "num_type: " << num_types << " num_run_timestep_types: " << num_run_timestep_types << endl;

    uint32_t x_length_per_proc = total_x_length / num_x_procs; //todo - deal with edge case?
    uint32_t y_length_per_proc = total_y_length / num_y_procs;
    uint32_t z_length_per_proc = total_z_length / num_z_procs;

    uint64_t chunk_vol = x_length_per_proc * y_length_per_proc * z_length_per_proc;

    uint32_t num_dims;
    uint32_t var_num;
    uint32_t var_version;

    uint32_t x_pos, y_pos, z_pos;
    uint32_t x_offset, y_offset, z_offset;

    double all_timestep_temp_maxes_for_all_procs[num_timesteps];
    double all_timestep_temp_mins_for_all_procs[num_timesteps];

    // srand(rank+1);
    //extreme_debug_log << "rank: " << rank << " first rgn: " << rand() << endl;

    //just for testing-------------------------------------------

    // long double *all_clock_times;

    // double *total_times; 
    //--------------------------------------------------------------


    x_pos = rank % num_x_procs;
    y_pos = (rank / num_x_procs) % num_y_procs; 
    z_pos = ((rank / (num_x_procs * num_y_procs)) % num_z_procs);

    x_offset = x_pos * x_length_per_proc;
    y_offset = y_pos * y_length_per_proc;
    z_offset = z_pos * z_length_per_proc;

    //extreme_debug_log << "zoffset: " << to_string(z_offset) << endl;
    //extreme_debug_log << "x pos is " << to_string(x_pos) << " and x_offset is " << to_string(x_offset) << endl;
    //extreme_debug_log << "y pos is " << to_string(y_pos) << " and y_offset is " << to_string(y_offset) << endl;
    //extreme_debug_log << "z pos is " << to_string(z_pos) << " and z_offset is " << to_string(z_offset) << endl;
    //extreme_debug_log << "num x procs " << to_string(num_x_procs) << " num y procs " << to_string(num_y_procs) << " num z procs " << to_string(num_z_procs) << endl;


    num_dims = 3;
    proc_dims.num_dims = num_dims;
    proc_dims.d0_min = x_offset;
    proc_dims.d0_max = x_offset + x_length_per_proc-1;
    proc_dims.d1_min = y_offset;
    proc_dims.d1_max = y_offset + y_length_per_proc -1;
    proc_dims.d2_min = z_offset;
    proc_dims.d2_max = z_offset + z_length_per_proc -1;

    /*
     * HDF5 APIs definitions
     */ 	

    hid_t run_attr_table_id;
    hid_t       run_file_id, var_id;         
    hid_t       var_data_space, chunk_data_space;     
    herr_t	status;

    hsize_t     chunk_dims[num_dims] = {x_length_per_proc, y_length_per_proc, z_length_per_proc};           

    /* chunk selection parameters */
    hsize_t	offset[num_dims] = {x_offset, y_offset, z_offset};

	hsize_t var_dims[num_dims] = {total_x_length, total_y_length, total_z_length};;
    /*
     */ 	
//-------------------------------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------------------------------    

	int num_read_client_procs = num_read_x_procs * num_read_y_procs * num_read_z_procs;
	int read_color;

    MPI_Comm read_comm;

    // md_dim_bounds read_proc_dims;

    const int num_timesteps_to_fetch = 6;

    //makes sure that, by the time we're ready to read md and checkpt the db, the timesteps will have been written
    vector<uint64_t> timestep_ids = {0, 1%num_timesteps_per_checkpt, 2%num_timesteps_per_checkpt,
                        3%num_timesteps_per_checkpt, 4%num_timesteps_per_checkpt, 5%num_timesteps_per_checkpt};

	std::vector<string> file_names_to_fetch;
	std::vector<string> var_names_to_fetch;
	std::vector<string> all_var_names;

    uint64_t timestep0_id = 0;
    string timestep0_file_name = run_name + "/" + job_id + "/" + to_string(timestep0_id);

    // std::vector<string> var_names_to_fetch(num_vars_to_fetch);
    // std::vector<uint32_t> var_vers_to_fetch(num_vars_to_fetch);
    // int plane_x_procs;
    // int plane_y_procs;
    // add_timing_point(INIT_READ_VARS_DONE);

    read_color = MPI_UNDEFINED;
    if(rank < num_read_client_procs) {
    	read_color = 0;
    }
    //make a comm with just the "read" procs
    MPI_Comm_split(MPI_COMM_WORLD, read_color, rank, &read_comm);

//-------------------------------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------------------------------    


    add_timing_point(INIT_VARS_DONE);

    MPI_Barrier(MPI_COMM_WORLD);

    //WRITE PROCESS
    //only want one copy of each var and type in each server's db
    add_timing_point(WRITING_START);

    if (rank == 0) {

        add_timing_point(CREATE_NEW_RUN_START);

    	metadata_create_run (run_name, job_id, run_file_id, run_attr_table_id);
    	//can't close the file since each new timestep needs to link to the run attr table
    	// //close so as to prevent memory leaks since we wont be adding run attributes until later
    	status = H5Fclose(run_file_id); assert(status >= 0);
    	status = H5PTclose(run_attr_table_id); assert(status >= 0);

        add_timing_point(CREATE_NEW_RUN_DONE);
    }


     add_timing_point(CREATE_TIMESTEPS_START);

    for(int timestep_id = 0; timestep_id < num_timesteps; timestep_id++) {

    	double sum = clear_cache(x_length_per_proc, y_length_per_proc, z_length_per_proc, num_vars);

    	//extreme_debug_log << "rank: " << rank << " about to barrier for timestep: " << timestep_id << endl;
    	MPI_Barrier(MPI_COMM_WORLD); //want to measure last-first for writing each timestep_id
        add_timing_point(CREATE_NEW_TIMESTEP_START);

        double timestep_temp_max;
        double timestep_temp_min;

        md_timestep_entry timestep_entry;

        vector<var_attribute_str> attrs;

        //create all of the md/data layout for the file before opening to avoid the collectives
        if (rank == 0) {
        	//create the file and associated metadata tables individually
       		metadata_create_timestep(run_name, job_id, timestep_id, timestep_entry);

			add_timing_point(CREATE_VARS_START);
	    	for (int var_indx = 0; var_indx < num_vars; var_indx++) {
	    		uint32_t var_version;
		    	if(var_indx == 1 || var_indx == 3) { //these have been designated ver2
		    		var_version = version2;
		    	}
		    	else {
		    		var_version = version1;
		    	}

	    		string VARNAME = var_names.at(var_indx) + to_string(var_version);
	    		//extreme_debug_log << "VARNAME: " << VARNAME << endl;

				metadata_create_and_close_chunked_var(num_dims, var_dims, chunk_dims,
					timestep_entry.file_id, VARNAME);
       		}

       		close_timestep(timestep_entry);
			add_timing_point(CREATE_VARS_DONE);
            // add_timing_point(CREATE_TIMESTEP_MD_TABLES_DONE);
       	}

       	// //debug_log << "rank: " << rank << " about to open file " << endl;
        // add_timing_point(CREATE_VARS_AND_ATTRS_FOR_NEW_TIMESTEP_START); 

		srand(rank*10000+1+timestep_id);

		add_timing_point(CREATE_VAR_ATTRS_START); 
	    for (int var_indx = 0; var_indx < num_vars; var_indx++) {
	    	// add_timing_point(CREATE_VAR_AND_ATTRS_FOR_NEW_VAR_START);

    		uint32_t var_version;
	    	if(var_indx == 1 || var_indx == 3) { //these have been designated ver2
	    		var_version = version2;
	    	}
	    	else {
	    		var_version = version1;
	    	}
	    	string VARNAME = var_names[var_indx] + to_string(var_version);
	    	
	    	create_var_attrs(rank, proc_dims, timestep_id, var_indx, VARNAME, num_types,
	    			attrs, timestep_temp_max, timestep_temp_min);
  			
        } //var loop done 
        // add_timing_point(CLOSE_TIMESTEP_DONE); 
        //close since the attr writing can't be done with a collectively opened file
        // add_timing_point(CREATE_VARS_AND_ATTRS_FOR_NEW_TIMESTEP_DONE); 
    	add_timing_point(CREATE_VAR_ATTRS_DONE);

       vector<var_attribute_str> all_attrs;

       add_timing_point(GATHER_ATTRS_START); 

       //extreme_debug_log << "rank " << rank << " about to gatherv for timestep " << timestep_id << endl;
       ser_gatherv_and_deser(attrs, num_client_procs, rank, all_attrs);

       add_timing_point(GATHER_ATTRS_DONE); 

       if(rank == 0) {
       	//first need to open the table
       		//extreme_debug_log << "rank 0 about to insert var attrs" << endl;

       		metadata_insert_var_attribute_batch(run_name, job_id, timestep_id, all_attrs, timestep_entry );
		}

		create_timestep_attrs(timestep_entry.file_id, rank, timestep_id, num_client_procs, 
			timestep_temp_max, timestep_temp_min, run_timestep_type_names[0], run_timestep_type_names[1], 
			all_timestep_temp_maxes_for_all_procs, all_timestep_temp_mins_for_all_procs);	    
		    
		if(rank == 0) {
			//extreme_debug_log << "writing done for timestep " << timestep_id << endl;
			H5Fclose(timestep_entry.file_id);
			//extreme_debug_log << "timestep file closed" << timestep_id << endl;
		}
		add_timing_point(CREATE_TIMESTEP_DONE); 

		if(timestep_id%10 == 0 && rank == 0) {
			cout << "timestep: " << timestep_id << endl;
		}

        if( ((timestep_id+1) % num_timesteps_per_checkpt) == 0) {
        	//extreme_debug_log << "time for a checkpt - rank: " << rank << endl;
        	//initialize the reading vars before the first checkpt
        	if( timestep_id+1 == num_timesteps_per_checkpt && rank < num_read_client_procs) {
				read_init(run_name, job_id, timestep_ids, var_names, all_var_names, file_names_to_fetch, var_names_to_fetch);

				//extreme_debug_log << "rank: " << rank << " just finished read init " << endl;
			}
			//once the DB has exceeded RAM, the server no longer has access to the entire DB in mem so we stop doing reads
        	if(rank < num_read_client_procs) {

				do_reads(rank, run_name, job_id,
					num_client_procs, timestep_id, read_comm, all_var_names,
					type_names, run_timestep_type_names_vct, 
					file_names_to_fetch, var_names_to_fetch,
					var_dims
				);
				//extreme_debug_log << "rank: " << rank << " just finished do read " << endl;
			}
			if(rank == 0) {
				cout << "timestep: " << timestep_id << endl;
			}
		}

	}
    add_timing_point(CREATE_ALL_TIMESTEPS_DONE); 

    if (rank == 0) {

		create_run_attrs(run_name, job_id, num_timesteps, all_timestep_temp_maxes_for_all_procs, 
			all_timestep_temp_mins_for_all_procs, run_timestep_type_names[0], run_timestep_type_names[1]);
	}

//-------------------------------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------------------------------        
r:
	do_cleanup(rank, num_client_procs);

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


void create_var_attrs(int rank, md_dim_bounds proc_dims, int timestep_num, int var_indx, string VARNAME, uint32_t num_types, 
		vector<var_attribute_str> &attrs,
		double &timestep_temp_max, double &timestep_temp_min)
{
	float type_freqs[] = {25, 5, .1, 100, 100, 20, 2.5, .5, 1, 10};
    // char type_types[10][3] = {"b", "b", "b", "d", "d", "2p","2p","2p","2d","2d"};
    char type_types[10][3] = {"b", "b", "b", "d", "d", "s","s","s","2d","2d"};
    //all 12 char plus null character
    char type_names[10][13] = {"blob_freq", "blob_ifreq", "blob_rare", "max_val_type", "min_val_type", "note_freq", "note_ifreq", "note_rare", "ranges_type1", "ranges_type2"}; 
   
	uint32_t type_version = 0;

	// add_timing_point(CREATE_VAR_ATTRS_FOR_NEW_VAR_START); 

    for(uint32_t type_indx=0; type_indx<num_types; type_indx++) {
        float freq = type_freqs[type_indx];
        uint32_t odds = 100 / freq;
        uint32_t val = rand() % odds; 
        // //extreme_debug_log << "rank: " << to_string(rank) << " and val: " << to_string(val) << " and odds: " << to_string(odds) << endl;
        if(val == 0) { //makes sure we have the desired frequency for each type
            // add_timing_point(CREATE_NEW_VAR_ATTR_START);

			string attr_data;

            if(strcmp(type_types[type_indx], "b") == 0) {
                bool flag = rank % 2;
                // make_single_val_data(flag, attr_data);
                attr_data = to_string(flag);
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
                // make_single_val_data(val, attr_data);
                attr_data = to_string(val);
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
                attr_data = "attr data for var" + to_string(var_indx) + "timestep"+to_string(timestep_num);

            }               
            else if(strcmp(type_types[type_indx], "2d") == 0) {
                vector<int> vals(2);
                vals[0] = (int) (rand() % RAND_MAX); 
                vals[1] = vals[0] + 10000; 
                make_single_val_data(vals, attr_data);

            }
            else {
                error_log << "error. type didn't match list of possibilities" << endl;
                add_timing_point(ERR_TYPE_COMPARE);
            }
            // add_timing_point(VAR_ATTR_INIT_DONE);
				
			string type_name = type_names[type_indx] + to_string(type_version);

			attrs.push_back ( var_attribute_str (type_name.c_str(), VARNAME.c_str(), proc_dims, attr_data) );

	        // if(attrs.size() < 5) {
	        //     //extreme_debug_log << "for rank: " << rank << " the " << attrs.size() << "th attr has val: " << attr_data << endl;
	        // }

           // add_timing_point(CREATE_NEW_VAR_ATTR_DONE);
        }//val = 0 done
    } //type loop done
    // add_timing_point(CREATE_VAR_ATTRS_FOR_NEW_VAR_DONE);
}

void create_timestep_attrs(hid_t timestep_file_id, int rank, int timestep_num, uint32_t num_client_procs, 
	double timestep_temp_max, double timestep_temp_min, char *max_type_name, char *min_type_name,
	double *all_timestep_temp_maxes_for_all_procs, double *all_timestep_temp_mins_for_all_procs)
{
	 add_timing_point(CREATE_TIMESTEP_ATTRS_START);

    if(rank == 0) {

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

        //testing_log << "timestep " << timestep_num << " temperature max: " << timestep_var_max << endl;
        //testing_log << "timestep " << timestep_num << " temperature min: " << timestep_var_min << endl;

        vector<non_var_attribute_str> attrs;

        string temp_max_data, temp_min_data;

        temp_max_data = to_string(timestep_var_max);
        temp_min_data = to_string(timestep_var_min);
        // make_single_val_data(timestep_var_max, temp_max_data);
        // make_single_val_data(timestep_var_min, temp_min_data);

        attrs.push_back ( non_var_attribute_str (max_type_name, temp_max_data) );
        attrs.push_back ( non_var_attribute_str (min_type_name, temp_min_data) );

        hid_t timestep_attr_table_id;
       	open_timestep_attr_table (timestep_file_id, timestep_attr_table_id);

    	metadata_insert_timestep_attribute_batch(timestep_attr_table_id, attrs );

    	H5PTclose(timestep_attr_table_id);

    }
    else {
        MPI_Gather(&timestep_temp_max, 1, MPI_DOUBLE, NULL, NULL, MPI_DOUBLE, 0, MPI_COMM_WORLD); 
        MPI_Gather(&timestep_temp_min, 1, MPI_DOUBLE, NULL, NULL, MPI_DOUBLE, 0, MPI_COMM_WORLD); 
    }

    add_timing_point(CREATE_TIMESTEP_ATTRS_DONE);
}

void create_run_attrs(string run_name, string job_id, uint32_t num_timesteps, double *all_timestep_temp_maxes_for_all_procs, 
	double *all_timestep_temp_mins_for_all_procs, char *max_type_name, char *min_type_name
	)	
{
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


    vector<non_var_attribute_str> attrs;

    string temp_max_data;
    string temp_min_data;

	temp_max_data = to_string(run_temp_max);
	temp_min_data = to_string(run_temp_min);
    // make_single_val_data(run_temp_max, temp_max_data);
    // make_single_val_data(run_temp_min, temp_min_data);

    attrs.push_back ( non_var_attribute_str (max_type_name, temp_max_data) );
    attrs.push_back ( non_var_attribute_str (min_type_name, temp_min_data) );

    hid_t run_file_id;
    hid_t run_attr_table_id;
    open_run_and_attr_table_for_write (run_name, job_id, run_file_id, run_attr_table_id);

	metadata_insert_run_attribute_batch(run_attr_table_id, attrs );

	H5PTclose(run_attr_table_id);
	H5Fclose(run_file_id);

    add_timing_point(CREATE_RUN_ATTRS_DONE);
    
}

void do_cleanup(int rank, uint32_t num_client_procs)
{
	long double *all_time_pts_buf;
    int *all_catg_time_pts_buf;

    add_timing_point(WRITING_DONE);

    MPI_Barrier (MPI_COMM_WORLD);
    //testing_log << "about to start cleanup" << endl;
    add_timing_point(WRITING_DONE_FOR_ALL_PROCS_START_CLEANUP);
        
    if(output_timing) {
        int each_proc_num_time_pts[num_client_procs];
        int displacement_for_each_proc[num_client_procs];
        int *all_catg_time_pts_buf;

        gatherv_int(catg_of_time_pts, num_client_procs, rank, each_proc_num_time_pts,
            displacement_for_each_proc, &all_catg_time_pts_buf);

        int sum = 0;
        if(rank == 0) {
            sum = accumulate(each_proc_num_time_pts, each_proc_num_time_pts+num_client_procs, sum);
            //extreme_debug_log << "sum: " << sum << endl;
            all_time_pts_buf = (long double *) malloc(sum * sizeof(long double));
        }
     
        int num_time_pts = time_pts.size();
        //debug_log << "num_time pts: " << to_string(num_time_pts)  << "time_pts.size(): " << time_pts.size()<< endl;

        MPI_Gatherv(&time_pts[0], num_time_pts, MPI_LONG_DOUBLE, all_time_pts_buf, each_proc_num_time_pts, displacement_for_each_proc, MPI_LONG_DOUBLE, 0, MPI_COMM_WORLD); 
        
        if (rank == 0) {
            //prevent it from buffering the printf statements
            setbuf(stdout, NULL);
            std::cout << "begin timing output" << endl;

            for(int i=0; i<sum; i++) {
                    // cout << "all_time_pts_buf[i]: " << all_time_pts_buf[i] << endl;
                printf("%d %Lf ", all_catg_time_pts_buf[i], all_time_pts_buf[i]);
                // std::cout << all_catg_time_pts_buf[i] << " " << all_time_pts_buf[i] << " ";
                if (i%20 == 0 && i!=0) {
                    std::cout << std::endl;
                }
            }
            std::cout << std::endl;

            free(all_time_pts_buf);
            free(all_catg_time_pts_buf);

        }
    }

    MPI_Barrier (MPI_COMM_WORLD);
    MPI_Finalize();
    //debug_log << "got to cleanup7" << endl;
    //debug_log << "finished \n";
}


