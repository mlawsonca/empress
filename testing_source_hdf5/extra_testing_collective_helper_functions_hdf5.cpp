/* 
 * Copyright 2018 National Technology & Engineering Solutions of
 * Sandia, LLC (NTESS). Under the terms of Contract DE-NA0003525 with NTESS,
 * the U.S. Government retains certain rights in this software.
 *
 * The MIT License (MIT)
 * 
 * Copyright (c) 2018 Sandia Corporation
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */



#include <client_timing_constants_read_hdf5.hh>

#include <sstream>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

#include <mpi.h>

//includes the print attr functs
#include <testing_harness_helper_functions_hdf5.hh>

#include <unordered_set>

using namespace std;

extern debugLog testing_log;
extern debugLog extreme_debug_log;
extern void add_timing_point(int catg);
extern bool read_data;

template <class T>
void bcast_entries(vector<T> &entries, int rank, uint32_t num_client_procs);


void catalog_all_metadata_for_run (int rank, uint32_t num_client_procs, 
							string run_name, string job_id,
							const md_dim_bounds &proc_dims,
							vector<string> &all_timestep_file_names,
							string &timestep0_file_name,
							vector<md_var_entry> &timestep0_vars ) 
{
    vector<md_var_entry> var_entries;

    vector<non_var_attribute_str> run_attr_entries;
    vector<non_var_attribute_str> timestep_attr_entries;
    vector<var_attribute_str> var_attr_entries;

    hid_t run_file_id, run_attr_table_id;
 

    MPI_Barrier(MPI_COMM_WORLD);
  	add_timing_point(CATALOG_ALL_RUN_ATTRIBUTES_COLLECTIVE_START);
  	if(rank == 0) { 
  		open_run_and_attr_table_for_read (run_name, job_id, run_file_id, run_attr_table_id);

 		metadata_catalog_all_run_attributes (run_attr_table_id, run_attr_entries);

        testing_log << "rank: " << rank << " using metadata_catalog_all_run_attributes funct: runs attrs " << endl;
        print_run_attribute_list (run_attr_entries);

        close_run_and_attr_table (run_file_id, run_attr_table_id);
	}
	bcast_entries(run_attr_entries, rank, num_client_procs);
  	add_timing_point(CATALOG_ALL_RUN_ATTRIBUTES_COLLECTIVE_DONE); 
    // testing_log << "rank: " << rank << " using metadata_catalog_all_run_attributes funct: runs attrs " << endl;
    // print_run_attribute_list (run_attr_entries);

    MPI_Barrier(MPI_COMM_WORLD);
	add_timing_point(CATALOG_TIMESTEP_COLLECTIVE_START);
    if(rank == 0) {
    	open_run_for_read (run_name, job_id, run_file_id);

        metadata_catalog_timestep (run_file_id, all_timestep_file_names);
   
        testing_log << "rank: " << rank << " timestep catalog: \n";
        print_timestep_catalog (all_timestep_file_names);

		close_run (run_file_id);
    }
	bcast_entries(all_timestep_file_names, rank, num_client_procs);
  	add_timing_point(CATALOG_TIMESTEP_COLLECTIVE_DONE); 
    // testing_log << "rank: " << rank << " timestep catalog: \n";
    // print_timestep_catalog (all_timestep_file_names);

    for (string timestep_file_name : all_timestep_file_names) {
    // for(int i = 0; i < timestep_file_names.size(); i++) {
    	// string timestep_file_name  = timestep_file_names.at(i);

       	size_t found = timestep_file_name.find_last_of("/");
    	uint64_t timestep_id = stoull(timestep_file_name.substr(found+1));

    	md_timestep_entry timestep;

    	MPI_Barrier(MPI_COMM_WORLD);
        add_timing_point(CATALOG_ALL_TIMESTEP_ATTRIBUTES_COLLECTIVE_START);
        if(rank == 0) {
			open_timestep_file_and_timestep_attr_table_for_read(timestep_file_name, 
				timestep.file_id, timestep.timestep_attr_table_id);

    		metadata_catalog_all_timestep_attributes (timestep.timestep_attr_table_id, timestep_attr_entries);
        	testing_log << "rank: " << rank << " timestep attributes associated with timestep_id " << timestep_id << " \n";		        	
            print_timestep_attribute_list (timestep_attr_entries);

            close_timestep_file_and_attr_table(timestep.file_id, timestep.timestep_attr_table_id);
		}
		bcast_entries(timestep_attr_entries, rank, num_client_procs);
        add_timing_point(CATALOG_ALL_TIMESTEP_ATTRIBUTES_COLLECTIVE_DONE);
    	// testing_log << "rank: " << rank << " timestep attributes associated with timestep_id " << timestep_id << " \n";		        	
     //    print_timestep_attribute_list (timestep_attr_entries);

    	MPI_Barrier(MPI_COMM_WORLD);
        add_timing_point(CATALOG_VAR_COLLECTIVE_START);   
        if(rank == 0) {
        	open_file_for_read(timestep_file_name, timestep.file_id);

    		metadata_catalog_var (timestep.file_id, var_entries);
	        testing_log << "rank: " << rank << " var catalog for timestep_id " << timestep_id << endl;        	
            print_var_catalog (var_entries);

            close_timestep_file(timestep.file_id);
		}
		bcast_entries(var_entries, rank, num_client_procs);
        add_timing_point(CATALOG_VAR_COLLECTIVE_DONE);   
        // testing_log << "rank: " << rank << " var catalog for timestep_id " << timestep_id << endl;        	
        // print_var_catalog (var_entries);

        for (md_var_entry var : var_entries) {   	
        	MPI_Barrier(MPI_COMM_WORLD);
        	add_timing_point(CATALOG_ALL_VAR_ATTRIBUTES_WITH_VAR_COLLECTIVE_START);
	        if(rank == 0) {
	        	open_timestep_file_and_var_attr_table_for_read(timestep_file_name, 
									timestep.file_id, timestep.var_attr_table_id);

    			metadata_catalog_all_var_attributes_with_var (timestep.var_attr_table_id, var.var_name, var_attr_entries);
          		testing_log << "rank: " << rank << " all var attributes associated with timestep_id " << timestep_id << 
          			" and var_name: " << var.var_name << endl;
            	print_var_attribute_list (var_attr_entries);
            	
            	close_timestep_file_and_attr_table(timestep.file_id, timestep.var_attr_table_id);
			}
			bcast_entries(var_attr_entries, rank, num_client_procs);
      		// testing_log << "rank: " << rank << " all var attributes associated with timestep_id " << timestep_id << 
      		// 	" and var_name: " << var.var_name << endl;
        // 	print_var_attribute_list (var_attr_entries);

            //this produces too many objects to retrieve
	       //  if (output_objector_params) {
	       //  	count = 0;
		      //   for (var_attribute_str attr : var_attr_entries) {
		      //   	if (dims_overlap ( attr.dims, proc_dims) ) {
		      //   		all_objector_params.push_back( get_objector_params(run_entries.at(0), var, attr.dims) );
		      //   		count += 1;
		      //   	}
		      //   }
		     	// add_objector_point(BOUNDING_BOX_TO_OBJ_NAMES, count);
	       //  }
        	add_timing_point(CATALOG_ALL_VAR_ATTRIBUTES_WITH_VAR_COLLECTIVE_DONE);

        }

		extreme_debug_log << "timestep_file_name: " << timestep_file_name << " timestep: " << timestep_file_name.substr(found+1) << endl;
        if (timestep_id  == 0) { //fix
    		timestep0_file_name = timestep_file_name; //fix
    		timestep0_vars = var_entries; //fix
    	}

    }
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//TYPE FUNCTS //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void catalog_types_in_timestep_functs ( int rank, uint32_t num_client_procs, uint64_t timestep_id,
					string timestep_file_name, string var_name,
                    md_dim_bounds query_dims ) 
{

    vector<string> type_names;

    md_timestep_entry timestep;

    MPI_Barrier(MPI_COMM_WORLD);
    add_timing_point(CATALOG_ALL_TYPES_WITH_VAR_ATTRIBUTES_IN_TIMESTEP_COLLECTIVE_START);
    if(rank == 0) {
    	open_timestep_file_and_var_attr_table_for_read(timestep_file_name, timestep.file_id, timestep.var_attr_table_id);

     	metadata_catalog_all_types_with_var_attributes_in_timestep (timestep.var_attr_table_id, type_names);
		testing_log << "rank: " << rank << " all types associated with a var attribute timestep_id " << timestep_id <<
			 endl;
		print_type_catalog (type_names);

    	close_timestep_file_and_attr_table(timestep.file_id, timestep.var_attr_table_id);   	
	}
	bcast_entries(type_names, rank, num_client_procs);
    add_timing_point(CATALOG_ALL_TYPES_WITH_VAR_ATTRIBUTES_IN_TIMESTEP_COLLECTIVE_DONE);
	// testing_log << "rank: " << rank << " all types associated with a var attribute timestep_id " << timestep_id <<
	// 	 endl;
	// print_type_catalog (type_names);

    MPI_Barrier(MPI_COMM_WORLD);
    add_timing_point(CATALOG_ALL_TYPES_WITH_VAR_ATTRIBUTES_WITH_VAR_IN_TIMESTEP_COLLECTIVE_START);

    if(rank == 0) {
       	open_timestep_file_and_var_attr_table_for_read(timestep_file_name, timestep.file_id, timestep.var_attr_table_id);

    	metadata_catalog_all_types_with_var_attributes_with_var_in_timestep(timestep.var_attr_table_id, var_name, type_names);
		testing_log << "rank: " << rank << " all types associated with a var attribute timestep_id " << timestep_id <<
			 " var_name: " << var_name << endl;
		print_type_catalog (type_names);   	
    	close_timestep_file_and_attr_table(timestep.file_id, timestep.var_attr_table_id);   	
	}
	bcast_entries(type_names, rank, num_client_procs);
    add_timing_point(CATALOG_ALL_TYPES_WITH_VAR_ATTRIBUTES_WITH_VAR_IN_TIMESTEP_COLLECTIVE_DONE);
	// testing_log << "rank: " << rank << " all types associated with a var attribute timestep_id " << timestep_id <<
	// 	 " var_name: " << var_name << endl;
	// print_type_catalog (type_names);   	

	type_names.clear();

    MPI_Barrier(MPI_COMM_WORLD);
    add_timing_point(CATALOG_ALL_TYPES_WITH_VAR_ATTRIBUTES_WITH_VAR_DIMS_IN_TIMESTEP_COLLECTIVE_START);
	if(rank == 0) {
       	open_timestep_file_and_var_attr_table_for_read(timestep_file_name, timestep.file_id, timestep.var_attr_table_id);
    	metadata_catalog_all_types_with_var_attributes_with_var_dims_in_timestep (timestep.var_attr_table_id, var_name, query_dims, type_names);
		testing_log << "rank: " << rank << " all types associated with a var attribute timestep_id " << timestep_id <<
			 " var_name: " << var_name << " overlapping with dims";
		print_md_dim_bounds(query_dims);
		print_type_catalog (type_names);	
    	close_timestep_file_and_attr_table(timestep.file_id, timestep.var_attr_table_id);   	
	}
	bcast_entries(type_names, rank, num_client_procs);
    add_timing_point(CATALOG_ALL_TYPES_WITH_VAR_ATTRIBUTES_WITH_VAR_DIMS_IN_TIMESTEP_COLLECTIVE_DONE);
	// testing_log << "rank: " << rank << " all types associated with a var attribute timestep_id " << timestep_id <<
	// 	 " var_name: " << var_name << " overlapping with dims";
	// print_md_dim_bounds(query_dims);
	// print_type_catalog (type_names);	
 }



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//TIMESTEP FUNCTS //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void catalog_timesteps_with_var_or_attr_functs ( int rank, uint32_t num_client_procs,
					vector<string> all_timestep_file_names, 
					string rare_type_name, string var_name,
                    md_dim_bounds query_dims
 					) 
{

    vector<string> timestep_file_names;

    // //Should be empty now 
    // metadata_catalog_timestep (timestep_file_names);
    // if (rc != RC_OK) {
    // }
    // if(rank == 0) {
	    // testing_log << "timestep catalog: \n";
    //     print_timestep_catalog (timestep_file_names);
    // }

    MPI_Barrier(MPI_COMM_WORLD);
    add_timing_point(CATALOG_ALL_TIMESTEPS_WITH_VAR_COLLECTIVE_START);
	if(rank == 0) {
    	metadata_catalog_all_timesteps_with_var (all_timestep_file_names, var_name, timestep_file_names);
	    testing_log << "rank: " << rank << " using catalog_all_timesteps_with_var funct: timesteps associated var_name " << var_name << endl;       
		print_timestep_catalog (timestep_file_names);	
	}
	bcast_entries(timestep_file_names, rank, num_client_procs);
    add_timing_point(CATALOG_ALL_TIMESTEPS_WITH_VAR_COLLECTIVE_DONE);
 //    testing_log << "rank: " << rank << " using catalog_all_timesteps_with_var funct: timesteps associated var_name " << var_name << endl;       
	// print_timestep_catalog (timestep_file_names);	

	timestep_file_names.clear();


    MPI_Barrier(MPI_COMM_WORLD);
    add_timing_point(CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_COLLECTIVE_START);
	if(rank == 0) {
    	metadata_catalog_all_timesteps_with_var_attributes_with_type_var (all_timestep_file_names, rare_type_name, var_name, timestep_file_names);
		testing_log << "rank: " << rank << " all timesteps associated with a var attribute type_name " <<
			 rare_type_name << ", var_name " << var_name << endl;
		print_timestep_catalog (timestep_file_names);	
	}
	bcast_entries(timestep_file_names, rank, num_client_procs);
    add_timing_point(CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_COLLECTIVE_DONE);
	// testing_log << "rank: " << rank << " all timesteps associated with a var attribute type_name " <<
	// 	 rare_type_name << ", var_name " << var_name << endl;
	// print_timestep_catalog (timestep_file_names);

	timestep_file_names.clear();


    MPI_Barrier(MPI_COMM_WORLD);
    add_timing_point(CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_DIMS_COLLECTIVE_START);
	if(rank == 0) {
    	metadata_catalog_all_timesteps_with_var_attributes_with_type_var_dims (all_timestep_file_names, rare_type_name, var_name, query_dims, timestep_file_names);
		testing_log << "rank: " << rank << " all timesteps associated with a var attribute type_name " <<
			 rare_type_name << ", var_name " << var_name << " overlapping with dims";
		print_md_dim_bounds(query_dims);
		print_timestep_catalog (timestep_file_names);	
	}
	bcast_entries(timestep_file_names, rank, num_client_procs);
    add_timing_point(CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_DIMS_COLLECTIVE_DONE);
	// testing_log << "rank: " << rank << " all timesteps associated with a var attribute type_name " <<
	// 	 rare_type_name << ", var_name " << var_name << " overlapping with dims";
	// print_md_dim_bounds(query_dims);
	// print_timestep_catalog (timestep_file_names);	

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//RUN ATTR FUNCTS //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void catalog_run_attributes_with_type_or_val_functs ( int rank, uint32_t num_client_procs,
							string run_name, string job_id, string type_name
        					) 
{

    vector<non_var_attribute_str> attr_entries;

    hid_t run_file_id, run_attr_table_id;

    // metadata_catalog_all_run_attributes (attr_entries);

    //     if(rank == 0) {
	    //     testing_log << "using metadata_catalog_all_run_attributes funct: run attr_entries associated with" 
	    //         << " \n";    
    //         print_run_attribute_list (attr_entries);
    //     }
    // }

    MPI_Barrier(MPI_COMM_WORLD);
    add_timing_point(CATALOG_ALL_RUN_ATTRIBUTES_WITH_TYPE_COLLECTIVE_START);
	if(rank == 0) {
	  	open_run_and_attr_table_for_read (run_name, job_id, run_file_id, run_attr_table_id);

    	metadata_catalog_all_run_attributes_with_type (run_attr_table_id, type_name, attr_entries);
        testing_log << "rank: " << rank << " all attr_entries associated with" 
            " type_name " << type_name << endl;
    	print_run_attribute_list (attr_entries);

    	close_run_and_attr_table (run_file_id, run_attr_table_id);
	}
	bcast_entries(attr_entries, rank, num_client_procs);
    add_timing_point(CATALOG_ALL_RUN_ATTRIBUTES_WITH_TYPE_COLLECTIVE_DONE);
 //    testing_log << "rank: " << rank << " all attr_entries associated with" 
 //        " type_name " << type_name << endl;
	// print_run_attribute_list (attr_entries);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//TIMESTEP ATTR FUNCTS /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void catalog_timestep_attributes_with_type_or_val_functs ( int rank, uint32_t num_client_procs,
							uint64_t timestep_id, string timestep_file_name, string type_name
        							) 
{

    vector<non_var_attribute_str> attr_entries;
    md_timestep_entry timestep;
    // metadata_catalog_all_timestep_attributes (timestep_id, attr_entries);

    //     if(rank == 0) {
	    //     testing_log << "using metadata_catalog_all_timestep_attributes funct: timestep attr_entries associated with" 
	    //     		" timestep_id " << timestep_id << " \n";    
    //         print_timestep_attribute_list (attr_entries);
    //     }
    // }

    MPI_Barrier(MPI_COMM_WORLD);
	add_timing_point(CATALOG_ALL_TIMESTEP_ATTRIBUTES_WITH_TYPE_COLLECTIVE_START);	
	if(rank == 0) {
		open_timestep_file_and_timestep_attr_table_for_read(timestep_file_name, 
				timestep.file_id, timestep.timestep_attr_table_id);

    	metadata_catalog_all_timestep_attributes_with_type (timestep.timestep_attr_table_id, type_name, attr_entries);
  		testing_log << "rank: " << rank << " all timestep attributes associated with timestep_id " <<
  			timestep_id << " type_name " << type_name << endl;
    	print_timestep_attribute_list (attr_entries);

    	close_timestep_file_and_attr_table(timestep.file_id, timestep.timestep_attr_table_id);
	}
	bcast_entries(attr_entries, rank, num_client_procs);
	add_timing_point(CATALOG_ALL_TIMESTEP_ATTRIBUTES_WITH_TYPE_COLLECTIVE_DONE);	

	// testing_log << "rank: " << rank << " all timestep attributes associated with timestep_id " <<
	// 	timestep_id << " type_name " << type_name << endl;
	// print_timestep_attribute_list (attr_entries);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//VAR ATTR FUNCTS ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void catalog_all_var_attributes_with_dims ( int rank, uint32_t num_client_procs,
							uint64_t timestep_id,
							string timestep_file_name, 
							md_dim_bounds query_dims,
							const md_dim_bounds &proc_dims
							) 
{
    vector<var_attribute_str> attr_entries;

	md_timestep_entry timestep;

    MPI_Barrier(MPI_COMM_WORLD);
	add_timing_point(CATALOG_ALL_VAR_ATTRIBUTES_WITH_DIMS_COLLECTIVE_START);	
	if(rank == 0) {
		open_timestep_file_and_var_attr_table_for_read(timestep_file_name, 
									timestep.file_id, timestep.var_attr_table_id);

    	metadata_catalog_all_var_attributes_with_dims(timestep.var_attr_table_id, query_dims, attr_entries);
		testing_log << "rank: " << rank << " all var attributes associated with timestep_id " <<
			timestep_id << " overlapping with dims";
		print_md_dim_bounds(query_dims);
		print_var_attribute_list (attr_entries);

		close_timestep_file_and_attr_table(timestep.file_id, timestep.var_attr_table_id);
	}
	bcast_entries(attr_entries, rank, num_client_procs);

	// note - have commented this out since it results in too many objects
	// read_data_for_attrs(timestep_file_name, attr_entries, proc_dims);

	add_timing_point(CATALOG_ALL_VAR_ATTRIBUTES_WITH_DIMS_COLLECTIVE_DONE);	
	// testing_log << "rank: " << rank << " all var attributes associated with timestep_id " <<
	// 	timestep_id << " overlapping with dims";
	// print_md_dim_bounds(query_dims);
	// print_var_attribute_list (attr_entries);

}

void catalog_var_attributes_with_var_functs ( int rank, uint32_t num_client_procs,
								uint64_t timestep_id,
								const string &timestep_file_name,
								const string &var_name, 
								md_dim_bounds query_dims,
								const md_dim_bounds &proc_dims 								
								) 
{

    vector<var_attribute_str> attr_entries;

	md_timestep_entry timestep;

    MPI_Barrier(MPI_COMM_WORLD);
	add_timing_point(CATALOG_ALL_VAR_ATTRIBUTES_WITH_VAR_COLLECTIVE_START);	
	if(rank == 0) {
		open_timestep_file_and_var_attr_table_for_read(timestep_file_name, 
									timestep.file_id, timestep.var_attr_table_id);

    	metadata_catalog_all_var_attributes_with_var (timestep.var_attr_table_id, var_name, attr_entries);
  			testing_log << "rank: " << rank << " all var attributes associated with timestep_id " << timestep_id << 
				" and var_name: " << var_name << endl;
    	print_var_attribute_list (attr_entries);
		close_timestep_file_and_attr_table(timestep.file_id, timestep.var_attr_table_id);
	}
	bcast_entries(attr_entries, rank, num_client_procs);

	//note - have commented this out since it results in too many objects
    // read_data_for_attrs ();	
	add_timing_point(CATALOG_ALL_VAR_ATTRIBUTES_WITH_VAR_COLLECTIVE_DONE);	
	// testing_log << "rank: " << rank << " all var attributes associated with timestep_id " << timestep_id << 
	// 	" and var_name: " << var_name << endl;
	// print_var_attribute_list (attr_entries);

	attr_entries.clear();


    MPI_Barrier(MPI_COMM_WORLD);
	add_timing_point(CATALOG_ALL_VAR_ATTRIBUTES_WITH_VAR_DIMS_COLLECTIVE_START);	
	if(rank == 0) {
		open_timestep_file_and_var_attr_table_for_read(timestep_file_name, 
									timestep.file_id, timestep.var_attr_table_id);

    	metadata_catalog_all_var_attributes_with_var_dims (timestep.var_attr_table_id, var_name, query_dims, attr_entries);
		testing_log << "rank: " << rank << " all var attributes associated with timestep_id " << timestep_id << 
			" and var_name: " << var_name << " overlapping with dims";
		print_md_dim_bounds(query_dims);			
		print_var_attribute_list (attr_entries);
		close_timestep_file_and_attr_table(timestep.file_id, timestep.var_attr_table_id);
	}
	bcast_entries(attr_entries, rank, num_client_procs);

	//note - have commented this out since it results in too many objects
	//run_data();
	add_timing_point(CATALOG_ALL_VAR_ATTRIBUTES_WITH_VAR_DIMS_COLLECTIVE_DONE);	
	// testing_log << "rank: " << rank << " all var attributes associated with timestep_id " << timestep_id << 
	// 	" and var_name: " << var_name << " overlapping with dims";
	// print_md_dim_bounds(query_dims);			
	// print_var_attribute_list (attr_entries);
}

void catalog_var_attributes_with_type_functs ( int rank, uint32_t num_client_procs,
									uint64_t timestep_id, string timestep_file_name, const string &type_name,
									const md_dim_bounds &proc_dims 								
									) 
{


    vector<var_attribute_str> attr_entries;

    md_timestep_entry timestep;

    MPI_Barrier(MPI_COMM_WORLD);
	add_timing_point(CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_COLLECTIVE_START);	
	if(rank == 0) {
		open_timestep_file_and_var_attr_table_for_read(timestep_file_name, 
									timestep.file_id, timestep.var_attr_table_id);

    	metadata_catalog_all_var_attributes_with_type (timestep.var_attr_table_id, type_name, attr_entries);
			testing_log << "rank: " << rank << " all var attributes associated with and timestep_id " << timestep_id << 
				" and type_name: " << type_name << endl;
		print_var_attribute_list (attr_entries);
		close_timestep_file_and_attr_table(timestep.file_id, timestep.var_attr_table_id);
	}
	bcast_entries(attr_entries, rank, num_client_procs);
	
    read_data_for_attrs(timestep_file_name, attr_entries, proc_dims);

	add_timing_point(CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_COLLECTIVE_DONE);	
	// testing_log << "rank: " << rank << " all var attributes associated with and timestep_id " << timestep_id << 
	// 	" and type_name: " << type_name << endl;
	// print_var_attribute_list (attr_entries);
}

void catalog_var_attributes_with_type_dims_functs ( int rank, uint32_t num_client_procs,
												uint64_t timestep_id, string timestep_file_name, 
                                                md_dim_bounds query_dims, const string &type_name,
                                                const md_dim_bounds &proc_dims 
                                                ) 
{
    vector<var_attribute_str> attr_entries;

    md_timestep_entry timestep;

	catalog_var_attributes_with_type_functs ( rank, num_client_procs, timestep_id, timestep_file_name, type_name, proc_dims);
	

    MPI_Barrier(MPI_COMM_WORLD);
	add_timing_point(CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_DIMS_COLLECTIVE_START);	
	if(rank == 0) {
		open_timestep_file_and_var_attr_table_for_read(timestep_file_name, 
									timestep.file_id, timestep.var_attr_table_id);

    	metadata_catalog_all_var_attributes_with_type_dims (timestep.var_attr_table_id, type_name, query_dims, attr_entries);
		testing_log << "rank: " << rank << " all var attributes associated with and timestep_id " << timestep_id << 
			" and type_name: " << type_name << " overlapping with dims";
		print_md_dim_bounds(query_dims);			
		print_var_attribute_list (attr_entries);
		close_timestep_file_and_attr_table(timestep.file_id, timestep.var_attr_table_id);
	}
	bcast_entries(attr_entries, rank, num_client_procs);

	read_data_for_attrs(timestep_file_name, attr_entries, proc_dims);

	add_timing_point(CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_DIMS_COLLECTIVE_DONE);	
	// testing_log << "rank: " << rank << " all var attributes associated with and timestep_id " << timestep_id << 
	// 	" and type_name: " << type_name << " overlapping with dims";
	// print_md_dim_bounds(query_dims);			
	// print_var_attribute_list (attr_entries);
}

void catalog_var_attributes_with_type_var ( int rank, uint32_t num_client_procs,
										uint64_t timestep_id, string timestep_file_name,
										const string &type_name, const string &var_name,
                                        const md_dim_bounds &proc_dims
                                        ) 
{

    vector<var_attribute_str> attr_entries;

    md_timestep_entry timestep;

    MPI_Barrier(MPI_COMM_WORLD);
	add_timing_point(CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_COLLECTIVE_START);	
	if(rank == 0) {
		open_timestep_file_and_var_attr_table_for_read(timestep_file_name, 
									timestep.file_id, timestep.var_attr_table_id);

    	metadata_catalog_all_var_attributes_with_type_var (timestep.var_attr_table_id, type_name, var_name, attr_entries);
			testing_log << "rank: " << rank << " all var attributes associated with timestep_id " << timestep_id << 
				" and type_name: " << type_name <<
				" and var_name: " << var_name << endl;
		print_var_attribute_list (attr_entries);
		close_timestep_file_and_attr_table(timestep.file_id, timestep.var_attr_table_id);
	}
	bcast_entries(attr_entries, rank, num_client_procs);

	read_data_for_attrs(timestep_file_name, attr_entries, proc_dims);

	add_timing_point(CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_COLLECTIVE_DONE);	
	// testing_log << "rank: " << rank << " all var attributes associated with timestep_id " << timestep_id << 
	// 		" and type_name: " << type_name <<
	// 		" and var_name: " << var_name << endl;
	// print_var_attribute_list (attr_entries);
}

void catalog_var_attributes_with_type_var_or_val_functs ( int rank, uint32_t num_client_procs,
										uint64_t timestep_id, 
										string timestep_file_name,
										const string &rare_type_name,
										const string &var_name,
                                        md_dim_bounds query_dims,
                                      	const md_dim_bounds &proc_dims
                                        ) 
{

    vector<var_attribute_str> attr_entries;

    md_timestep_entry timestep;

	catalog_var_attributes_with_type_var ( rank, num_client_procs, timestep_id, timestep_file_name, rare_type_name, var_name, proc_dims ) ;
  

    MPI_Barrier(MPI_COMM_WORLD);
	add_timing_point(CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_DIMS_COLLECTIVE_START);	
	if(rank == 0) {
		open_timestep_file_and_var_attr_table_for_read(timestep_file_name, 
									timestep.file_id, timestep.var_attr_table_id);

    	metadata_catalog_all_var_attributes_with_type_var_dims (timestep.var_attr_table_id, rare_type_name, var_name, query_dims, attr_entries);
			testing_log << "rank: " << rank << " all var attributes associated with timestep_id " << timestep_id << 
			" and type_name: " << rare_type_name << " and var_name: " << var_name << " overlapping with dims:"; 
		print_md_dim_bounds(query_dims);
		print_var_attribute_list (attr_entries);
		close_timestep_file_and_attr_table(timestep.file_id, timestep.var_attr_table_id);
	}
	bcast_entries(attr_entries, rank, num_client_procs);

	read_data_for_attrs(timestep_file_name, attr_entries, proc_dims);

	add_timing_point(CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_DIMS_COLLECTIVE_DONE);	
	// testing_log << "rank: " << rank << " all var attributes associated with timestep_id " << timestep_id << 
	// 	" and type_name: " << rare_type_name << " and var_name: " << var_name << " overlapping with dims:"; 
	// print_md_dim_bounds(query_dims);
	// print_var_attribute_list (attr_entries);
}





////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//SUBSTR FUNCTS ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void catalog_types_with_var_substr_in_timestep_functs ( int rank, uint32_t num_client_procs,
					uint64_t timestep_id, string timestep_file_name, const string &var_name_substr,
                    md_dim_bounds query_dims ) 
{

    vector<string> type_names;

    md_timestep_entry timestep;


    MPI_Barrier(MPI_COMM_WORLD);
	add_timing_point(CATALOG_ALL_TYPES_WITH_VAR_ATTRIBUTES_WITH_VAR_SUBSTR_IN_TIMESTEP_COLLECTIVE_START);	
	if(rank == 0) {
		open_timestep_file_and_var_attr_table_for_read(timestep_file_name, 
									timestep.file_id, timestep.var_attr_table_id);

    	metadata_catalog_all_types_with_var_attributes_with_var_substr_in_timestep (timestep.var_attr_table_id, var_name_substr, type_names);
		testing_log << "rank: " << rank << " all types associated with a var attribute timestep_id " << timestep_id <<
			 " and var_name_substr " << var_name_substr << endl;
		print_type_catalog (type_names);
		close_timestep_file_and_attr_table(timestep.file_id, timestep.var_attr_table_id);
	}
	bcast_entries(type_names, rank, num_client_procs);

	add_timing_point(CATALOG_ALL_TYPES_WITH_VAR_ATTRIBUTES_WITH_VAR_SUBSTR_IN_TIMESTEP_COLLECTIVE_DONE);	
	// testing_log << "rank: " << rank << " all types associated with a var attribute timestep_id " << timestep_id <<
	// 	 " and var_name_substr " << var_name_substr << endl;
	// print_type_catalog (type_names);

	type_names.clear();


    MPI_Barrier(MPI_COMM_WORLD);
	add_timing_point(CATALOG_ALL_TYPES_WITH_VAR_ATTRIBUTES_WITH_VAR_SUBSTR_DIMS_IN_TIMESTEP_COLLECTIVE_START);	
	if(rank == 0) {
 		open_timestep_file_and_var_attr_table_for_read(timestep_file_name, 
									timestep.file_id, timestep.var_attr_table_id);

		metadata_catalog_all_types_with_var_attributes_with_var_substr_dims_in_timestep (timestep.var_attr_table_id, var_name_substr, query_dims, type_names);
		testing_log << "rank: " << rank << " all types associated with a var attribute timestep_id " << timestep_id <<
			 " and var_name_substr " << var_name_substr << " overlapping with dims";
		print_md_dim_bounds(query_dims);
		print_type_catalog (type_names);
		close_timestep_file_and_attr_table(timestep.file_id, timestep.var_attr_table_id);
	}
	bcast_entries(type_names, rank, num_client_procs);

	add_timing_point(CATALOG_ALL_TYPES_WITH_VAR_ATTRIBUTES_WITH_VAR_SUBSTR_DIMS_IN_TIMESTEP_COLLECTIVE_DONE);	
	// testing_log << "rank: " << rank << " all types associated with a var attribute timestep_id " << timestep_id <<
	// 	 " and var_name_substr " << var_name_substr << " overlapping with dims";
	// print_md_dim_bounds(query_dims);
	// print_type_catalog (type_names);
}


void catalog_timesteps_with_var_substr_or_val_functs ( int rank, uint32_t num_client_procs,
					uint64_t timestep_id, vector<string> all_timestep_file_names, 
					string type_name, 
					const string &var_name_substr, md_dim_bounds query_dims
					 ) 
{

    vector<string> timestep_file_names;

    MPI_Barrier(MPI_COMM_WORLD);
	add_timing_point(CATALOG_ALL_TIMESTEPS_WITH_VAR_SUBSTR_COLLECTIVE_START);	
	if(rank == 0) {
    	metadata_catalog_all_timesteps_with_var_substr (all_timestep_file_names, var_name_substr, timestep_file_names);
		testing_log << "rank: " << rank << " all types associated with a var attribute timestep_id " << timestep_id <<
			 " and var_name_substr " << var_name_substr << endl;
		print_timestep_catalog (timestep_file_names);
	}
	bcast_entries(timestep_file_names, rank, num_client_procs);

	add_timing_point(CATALOG_ALL_TIMESTEPS_WITH_VAR_SUBSTR_COLLECTIVE_DONE);	
	// testing_log << "rank: " << rank << " all types associated with a var attribute timestep_id " << timestep_id <<
	// 	 " and var_name_substr " << var_name_substr << endl;
	// print_timestep_catalog (timestep_file_names);

	timestep_file_names.clear();


    MPI_Barrier(MPI_COMM_WORLD);
	add_timing_point(CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_COLLECTIVE_START);	
	if(rank == 0) {
    	metadata_catalog_all_timesteps_with_var_attributes_with_type_var_substr (all_timestep_file_names, 
    		type_name, var_name_substr, timestep_file_names);

		testing_log << "rank: " << rank << " all timesteps associated with a var attribute type_name " <<
			 type_name << ", var_name_substr " << var_name_substr << endl;
		print_timestep_catalog (timestep_file_names);
	}
	bcast_entries(timestep_file_names, rank, num_client_procs);
	add_timing_point(CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_COLLECTIVE_DONE);	
	// testing_log << "rank: " << rank << " all timesteps associated with a var attribute type_name " <<
	// 	 type_name << ", var_name_substr " << var_name_substr << endl;
	// print_timestep_catalog (timestep_file_names);

	timestep_file_names.clear();



    MPI_Barrier(MPI_COMM_WORLD);
	add_timing_point(CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_DIMS_COLLECTIVE_START);	
	if(rank == 0) {
    	metadata_catalog_all_timesteps_with_var_attributes_with_type_var_substr_dims (all_timestep_file_names, type_name, var_name_substr, query_dims, timestep_file_names);
		testing_log << "rank: " << rank << " all timesteps associated with a var attribute type_name " <<
			 type_name << ", var_name_substr " << var_name_substr << 
			" overlapping with dims";
		print_md_dim_bounds(query_dims);
		print_timestep_catalog (timestep_file_names);
	}
	bcast_entries(timestep_file_names, rank, num_client_procs);
	add_timing_point(CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_DIMS_COLLECTIVE_DONE);	
	// testing_log << "rank: " << rank << " all timesteps associated with a var attribute type_name " <<
	// 	 type_name << ", var_name_substr " << var_name_substr << 
	// 	" overlapping with dims";
	// print_md_dim_bounds(query_dims);
	// print_timestep_catalog (timestep_file_names);

	timestep_file_names.clear();	

}

void catalog_var_attributes_with_var_name_substr_functs ( int rank, uint32_t num_client_procs,
											uint64_t timestep_id, string timestep_file_name,
											const string &var_name_substr, 
											md_dim_bounds query_dims,
											const md_dim_bounds &proc_dims 
											) 
{

    vector<var_attribute_str> attr_entries;

    md_timestep_entry timestep;

    MPI_Barrier(MPI_COMM_WORLD);
	add_timing_point(CATALOG_ALL_VAR_ATTRIBUTES_WITH_VAR_SUBSTR_COLLECTIVE_START);	
	if(rank == 0) {
 		open_timestep_file_and_var_attr_table_for_read(timestep_file_name, 
									timestep.file_id, timestep.var_attr_table_id);

    	metadata_catalog_all_var_attributes_with_var_substr (timestep.var_attr_table_id, var_name_substr, attr_entries);
		testing_log << "rank: " << rank << " all var attributes associated with timestep_id " << timestep_id << 
				" and var_name_substr " << var_name_substr << endl;
		print_var_attribute_list (attr_entries);
 		close_timestep_file_and_attr_table(timestep.file_id, timestep.var_attr_table_id);
	}
	bcast_entries(attr_entries, rank, num_client_procs);

	// note - have commented this out since it results in too many objects
	// read_data_for_attrs(timestep_file_name, attr_entries, proc_dims);		
	add_timing_point(CATALOG_ALL_VAR_ATTRIBUTES_WITH_VAR_SUBSTR_COLLECTIVE_DONE);	
	// testing_log << "rank: " << rank << " all var attributes associated with timestep_id " << timestep_id << 
	// 		" and var_name_substr " << var_name_substr << endl;
	// print_var_attribute_list (attr_entries);

	attr_entries.clear();



    MPI_Barrier(MPI_COMM_WORLD);
	add_timing_point(CATALOG_ALL_VAR_ATTRIBUTES_WITH_VAR_SUBSTR_DIMS_COLLECTIVE_START);	
	if(rank == 0) {
 		open_timestep_file_and_var_attr_table_for_read(timestep_file_name, 
									timestep.file_id, timestep.var_attr_table_id);

    	metadata_catalog_all_var_attributes_with_var_substr_dims (timestep.var_attr_table_id, var_name_substr, query_dims, attr_entries);
		testing_log << "rank: " << rank << " all var attributes associated with timestep_id " << timestep_id << 
			" and var_name_substr " << var_name_substr << " overlapping with dims";
		print_md_dim_bounds(query_dims);
		print_var_attribute_list (attr_entries);
 		close_timestep_file_and_attr_table(timestep.file_id, timestep.var_attr_table_id);
	}
	bcast_entries(attr_entries, rank, num_client_procs);

	// note - have commented this out since it results in too many objects
  	//read_data_for_attrs(timestep_file_name, attr_entries, proc_dims);
	add_timing_point(CATALOG_ALL_VAR_ATTRIBUTES_WITH_VAR_SUBSTR_DIMS_COLLECTIVE_DONE);	
	// testing_log << "rank: " << rank << " all var attributes associated with timestep_id " << timestep_id << 
	// 	" and var_name_substr " << var_name_substr << " overlapping with dims";
	// print_md_dim_bounds(query_dims);
	// print_var_attribute_list (attr_entries);
}

void catalog_var_attributes_with_type_var_name_substr_or_val_functs ( int rank, uint32_t num_client_procs,
													uint64_t timestep_id, string timestep_file_name,
													const string &var_name_substr, string rare_type_name,
                                                    md_dim_bounds query_dims,
                                                    const md_dim_bounds &proc_dims
                                                    ) 
{

    vector<var_attribute_str> attr_entries;

    md_timestep_entry timestep;

    MPI_Barrier(MPI_COMM_WORLD);
	add_timing_point(CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_COLLECTIVE_START);	
	if(rank == 0) {
 		open_timestep_file_and_var_attr_table_for_read(timestep_file_name, 
									timestep.file_id, timestep.var_attr_table_id);

    	metadata_catalog_all_var_attributes_with_type_var_substr (timestep.var_attr_table_id, rare_type_name, var_name_substr, attr_entries);
		testing_log << "rank: " << rank << " all var attributes associated with timestep_id " << timestep_id << 
			" type: " << rare_type_name << 
			" and var_name_substr " << var_name_substr << endl;
		print_var_attribute_list (attr_entries);
 		close_timestep_file_and_attr_table(timestep.file_id, timestep.var_attr_table_id);
	}
	bcast_entries(attr_entries, rank, num_client_procs);

	read_data_for_attrs(timestep_file_name, attr_entries, proc_dims);

	add_timing_point(CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_COLLECTIVE_DONE);	
	// testing_log << "rank: " << rank << " all var attributes associated with timestep_id " << timestep_id << 
	// 	" type: " << rare_type_name << 
	// 	" and var_name_substr " << var_name_substr << endl;
	// print_var_attribute_list (attr_entries);

	attr_entries.clear();


    MPI_Barrier(MPI_COMM_WORLD);
	add_timing_point(CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_DIMS_COLLECTIVE_START);	
	if(rank == 0) {
 		open_timestep_file_and_var_attr_table_for_read(timestep_file_name, 
									timestep.file_id, timestep.var_attr_table_id);

    	metadata_catalog_all_var_attributes_with_type_var_substr_dims (timestep.var_attr_table_id, rare_type_name, 
    	var_name_substr, query_dims, attr_entries);		
		testing_log << "rank: " << rank << " all var attributes associated with timestep_id " << timestep_id << 
			" type: " << rare_type_name << 
			" and var_name_substr " << var_name_substr << " overlapping with dims";
		print_md_dim_bounds(query_dims);
		print_var_attribute_list (attr_entries);
 		close_timestep_file_and_attr_table(timestep.file_id, timestep.var_attr_table_id);
	}
	bcast_entries(attr_entries, rank, num_client_procs);

	read_data_for_attrs(timestep_file_name, attr_entries, proc_dims);
	add_timing_point(CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_DIMS_COLLECTIVE_DONE);	

	// attr_entries.clear();
	// testing_log << "rank: " << rank << " all var attributes associated with timestep_id " << timestep_id << 
	// 	" type: " << rare_type_name << 
	// 	" and var_name_substr " << var_name_substr << " overlapping with dims";
	// print_md_dim_bounds(query_dims);
	// print_var_attribute_list (attr_entries);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//helper functions//////////////////////////////////////////////////////////////////////////////////////////////////////////////////


template <class T>
static void bcast_entries(vector<T> &entries, int rank, uint32_t num_client_procs) {

	add_timing_point(BCAST_ENTRIES_COLLECTIVE_START);

	extreme_debug_log.set_rank(rank);

	int length_ser_c_str = 0;
	char *serialized_c_str;

	extreme_debug_log << "rank " << rank << " entries.size(): " <<entries.size() << endl;

	if (rank == 0 && entries.size() > 0) {
	    stringstream ss;
		boost::archive::text_oarchive oa(ss);
		// extreme_debug_log << "rank " << rank << " about to try serializing at the beginning" << endl;
	    oa << entries;
	  	// extreme_debug_log << "rank " << rank << " just finished serializing at the beginning" << endl;
	    string serialized_str = ss.str();
	    length_ser_c_str = serialized_str.size() + 1;
	    serialized_c_str = (char *) malloc(length_ser_c_str);
	    serialized_str.copy(serialized_c_str, serialized_str.size());
	    serialized_c_str[serialized_str.size()]='\0';
		// extreme_debug_log << "rank " << rank << " attrs ser string is of size " << length_ser_c_str << " serialized_str " << 
		// 	serialized_str << " serialized_c_str: " << serialized_c_str << endl;
		extreme_debug_log << "rank " << rank << " attrs ser string is of size " << length_ser_c_str << " serialized_str " << 
			serialized_str << endl;
	}
		
	// extreme_debug_log << "rank " << rank << " about to allgather" << endl;

	MPI_Bcast(&length_ser_c_str, 1, MPI_INT, 0, MPI_COMM_WORLD);

	if(length_ser_c_str > 0) {
		if(rank != 0) {
			serialized_c_str = (char *) malloc(length_ser_c_str);
		}
	 	MPI_Bcast(serialized_c_str, length_ser_c_str, MPI_CHAR, 0, MPI_COMM_WORLD);
		// if(rank == 1) {
		// 	cout << "length_ser_c_str: " << length_ser_c_str << endl;
		// 	cout << "serialized_c_str: " << serialized_c_str << endl;
		// }

		if(rank != 0) {
	        stringstream ss1;
	        ss1.write(serialized_c_str, length_ser_c_str);
	        // if (rank == 1) {
	        // 	extreme_debug_log << "about to deserialize ss: " << ss1.str() << endl;
	        // }
	        boost::archive::text_iarchive ia(ss1);
	        ia >> entries;
	       //  if (rank == 1) {
	      	// 	extreme_debug_log << "rank " << rank << " received entries.size(): " << entries.size() << endl;
	      	// }
		}
	    free(serialized_c_str);
	}

	add_timing_point(BCAST_ENTRIES_COLLECTIVE_DONE);
}