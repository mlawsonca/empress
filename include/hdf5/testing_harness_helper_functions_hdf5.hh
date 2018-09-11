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



#ifndef TESTINGHARNESSHELPERFUNCTIONSHDF5_HH
#define TESTINGHARNESSHELPERFUNCTIONSHDF5_HH

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <sstream>

#include <my_metadata_client_hdf5.hh>

void gatherv_int(std::vector<int> values, uint32_t num_client_procs, int rank, int *each_proc_num_values,
            int *displacement_for_each_proc, int **all_values);

template <class T>
void ser_gatherv_and_deser(std::vector<T> attrs, uint32_t num_client_procs, int rank, std::vector<T> &all_attrs)
{
    int length_ser_c_str = 0;
    char *serialized_c_str;
    char *serialized_c_str_all_ser_values;

    int *each_proc_ser_values_size;
    int *displacement_for_each_proc;

    std::stringstream ss;
    boost::archive::text_oarchive oa(ss);
    oa << attrs;
    std::string serialized_str = ss.str();
    length_ser_c_str = serialized_str.size() + 1;
    // //extreme_debug_log << "length_ser_c_str:" << length_ser_c_str << endl;
    serialized_c_str = (char *) malloc(length_ser_c_str);
    serialized_str.copy(serialized_c_str, serialized_str.size());
    serialized_c_str[serialized_str.size()]='\0';


    // cout << "sending attrs.dims: " << attrs.at(0).d0_min << " " << attrs.at(0).d0_max << endl;

    // //extreme_debug_log << "rank " << rank << " object_names ser std::string is of size " << length_ser_c_str << " serialized_str " << 
    //     serialized_str << endl;
    //extreme_debug_log << "rank " << rank << " object_names ser std::string is of size " << length_ser_c_str << endl; 
    // //extreme_debug_log << "serialized_c_str: " << serialized_c_str << endl;  
    // //extreme_debug_log << "rank " << rank << " about to allgather" << endl;
    if(rank == 0) {
    	each_proc_ser_values_size = (int *)malloc( num_client_procs * sizeof(int));
    }

    MPI_Gather(&length_ser_c_str, 1, MPI_INT, each_proc_ser_values_size, 1, MPI_INT, 0, MPI_COMM_WORLD);

    //extreme_debug_log << "rank " << rank << " about with allgather" << endl;

    int sum = 0;
    
    if (rank == 0 ) {
    	displacement_for_each_proc = (int *)malloc( num_client_procs * sizeof(int));

        for(int i=0; i<num_client_procs; i++) {
            displacement_for_each_proc[i] = sum;
            sum += each_proc_ser_values_size[i];
            if(each_proc_ser_values_size[i] != 0 && rank == 0) {
                //extreme_debug_log << "rank " << i << " has a std::string of length " << each_proc_ser_values_size[i] << endl;
            }
        }
        //extreme_debug_log << "sum: " << sum << endl;

       serialized_c_str_all_ser_values = (char *) malloc(sum);
    }

    //extreme_debug_log << "num_bytes is "<< to_string(length_ser_c_str) + " and rank: " << to_string(rank) << endl;
    
    // //extreme_debug_log << "rank " << rank << " about to allgatherv" << endl;

    MPI_Gatherv(serialized_c_str, length_ser_c_str, MPI_CHAR,
           serialized_c_str_all_ser_values, each_proc_ser_values_size, displacement_for_each_proc,
           MPI_CHAR, 0, MPI_COMM_WORLD);

    free(serialized_c_str);


   if(rank == 0) {
       all_attrs.reserve(sum);

        for(int proc_rank = 0; proc_rank < num_client_procs; proc_rank++) {
            int offset = displacement_for_each_proc[proc_rank];
            int count = each_proc_ser_values_size[proc_rank];

            if(count > 0) {
            	std::vector<T> rec_attrs;
                if(proc_rank != 0) {
                    //extreme_debug_log << "rank " << proc_rank << " count: " << count << " offset: " << offset << endl;
                    char serialzed_attrs_for_one_proc[count];

                    //extreme_debug_log << "about to memcopy1" << endl;
                    memcpy ( serialzed_attrs_for_one_proc, serialized_c_str_all_ser_values + offset, count);

                    //extreme_debug_log << "rank " << rank << " serialized_c_str length: " << strlen(serialzed_attrs_for_one_proc) << 
                        // " count: " << count << endl;

                    std::stringstream ss1;
                    ss1.write(serialzed_attrs_for_one_proc, count);
                    // //extreme_debug_log << "about to deserialize ss: " << ss1.str() << endl;
                    boost::archive::text_iarchive ia(ss1);
                    ia >> rec_attrs;
                }
                else { //i == rank
                    rec_attrs = attrs;
                }

                all_attrs.insert(all_attrs.end(), rec_attrs.begin(), rec_attrs.end());
                // cout << "rec_attrs.dims: " << rec_attrs.at(0).d0_min << " " << rec_attrs.at(0).d0_max << endl;
            }
        }
        free(serialized_c_str_all_ser_values);
        free(each_proc_ser_values_size);
        free(displacement_for_each_proc);
    } 
}



//fix - just for debugging
int debug_testing(int rank, hsize_t *chunk_dims, int num_client_procs, std::string run_name, int num_timesteps, int num_vars,
				int num_dims, hsize_t *var_dims,
				std::string job_id, std::vector<std::string> var_names, uint32_t version1, uint32_t version2,
				bool write_data);

void generate_data_for_proc(int ny, int nz, hsize_t *offset,
                        int rank, std::vector<double> &data_vct, 
                        uint64_t ndx, uint64_t ndy, uint64_t ndz, uint64_t chunk_vol);

void find_data_range(int ny, int nz, int num_dims, const std::vector<double> &data_vect);



template <class T>
void make_single_val_data (T val, std::string &serial_str) {
    std::stringstream ss;
    boost::archive::text_oarchive oa(ss);
    oa << val;
    serial_str = ss.str();
}

// void print_attr_data (std::string attr_data );
void print_attr_data(var_attribute_str attr);
void print_attr_data(non_var_attribute_str attr);

void open_timestep_file_collectively_for_write(const std::string &run_name, std::string job_id, uint64_t timestep_id, hid_t &file_id);
void open_timestep_file_collectively_for_write(std::string FILENAME, hid_t &file_id);

void open_timestep_file_collectively_for_read(std::string FILENAME, hid_t &file_id);
void open_timestep_file_collectively_for_read(const std::string &run_name, std::string job_id, uint64_t timestep_id, hid_t &file_id);


void read_data_for_attrs(std::string timestep_file_name, std::vector<var_attribute_str> attrs,
			 md_dim_bounds proc_dims);

void get_overlapping_dims_for_attr ( var_attribute_str &attr, const md_dim_bounds &proc_dims);

void read_attr(hid_t timestep_file_id, var_attribute_str attr);

void get_hyperslab_dims_and_vol(var_attribute_str attr, hsize_t *count, hsize_t *offset, 
	hsize_t *stride, hsize_t *block, uint64_t &attr_vol);

void find_data_range(hid_t var_data_space, var_attribute_str attr, const std::vector<double> &data_vect);
void find_data_range(int pattern, std::string timestep_file_name, std::string var_name, hid_t var_data_space, 
		uint32_t num_dims, const std::vector<double> &data_vect);



#endif //TESTINGHARNESSHELPERFUNCTIONSHDF5_HH