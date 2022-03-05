#ifndef READ_HELPER_FUNCTIONS_HH
#define READ_HELPER_FUNCTIONS_HH

#include <vector>
#include <client_timing_constants_read_class_proj.hh>
#include <sstream>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

extern void add_timing_point(int catg);

template <class T>
void gather_attr_entries(std::vector<T> attr_entries, uint32_t count, int rank, uint32_t num_servers, uint32_t num_client_procs,
        std::vector<T> &all_attr_entries, MPI_Comm comm) {

    add_timing_point(GATHER_ATTR_ENTRIES_START);

    //extreme_debug_log.set_rank(rank);

    int length_ser_c_str = 0;
    char *serialized_c_str;
    int each_proc_ser_attr_entries_size[num_client_procs];
    int displacement_for_each_proc[num_client_procs];

    char *serialized_c_str_all_attr_entries;        

    //extreme_debug_log << "rank " << rank << " attr_entries.size(): " <<attr_entries.size() << endl;
    //extreme_debug_log << "num_client_procs: " << num_client_procs << endl;
    if (rank < num_servers && attr_entries.size() > 0) {
        std::stringstream ss;
        boost::archive::text_oarchive oa(ss);
        // //extreme_debug_log << "rank " << rank << " about to try serializing at the beginning" << endl;
        oa << attr_entries;
        // //extreme_debug_log << "rank " << rank << " just finished serializing at the beginning" << endl;
        std::string serialized_str = ss.str();
        length_ser_c_str = serialized_str.size() + 1;
        serialized_c_str = (char *) malloc(length_ser_c_str);
        serialized_str.copy(serialized_c_str, serialized_str.size());
        serialized_c_str[serialized_str.size()]='\0';
        // //extreme_debug_log << "rank " << rank << " attrs ser string is of size " << length_ser_c_str << " serialized_str " << 
        //  serialized_str << " serialized_c_str: " << serialized_c_str << endl;
        //extreme_debug_log << "rank " << rank << " attrs ser string is of size " << length_ser_c_str << " serialized_str " << 
            // serialized_str << endl;
    }
        
    //extreme_debug_log << "rank " << rank << " about to allgather my length_ser_c_str: " << length_ser_c_str << endl;

    MPI_Allgather(&length_ser_c_str, 1, MPI_INT, each_proc_ser_attr_entries_size, 1, MPI_INT, comm);

    //extreme_debug_log << "rank " << rank << " done with allgather" << endl;

    int sum = 0;
    
    for(int i=0; i<num_client_procs; i++) {
        //extreme_debug_log << "i: " << i << " sum: " << sum << endl;
        displacement_for_each_proc[i] = sum;
        //extreme_debug_log << "each_proc_ser_attr_entries_size[i]: " << each_proc_ser_attr_entries_size[i] << endl;
        sum += each_proc_ser_attr_entries_size[i];
        // if(each_proc_ser_attr_entries_size[i] != 0 && rank == 0) {
            //extreme_debug_log << "rank " << i << " has a string of length " << each_proc_ser_attr_entries_size[i] << endl;
        // }
    }
    if (rank == 0 ) {
        //extreme_debug_log << "sum: " << sum << endl;
    }


    serialized_c_str_all_attr_entries = (char *) malloc(sum);
    // //extreme_debug_log << "num_bytes is "<< to_string(length_ser_c_str) + " and rank: " << to_string(rank) << endl;
    
    // //extreme_debug_log << "rank " << rank << " about to allgatherv" << endl;

    MPI_Allgatherv(serialized_c_str, length_ser_c_str, MPI_CHAR,
           serialized_c_str_all_attr_entries, each_proc_ser_attr_entries_size, displacement_for_each_proc,
           MPI_CHAR, comm);

    // //extreme_debug_log << "rank " << rank << " done with allgatherv" << endl;
    // //extreme_debug_log << "entire set of attr std::vectors: " << serialized_c_str_all_attr_entries << " and is of length: " << strlen(serialized_c_str_all_attr_entries) << endl;

    for(int i = 0; i < num_servers; i++) {
        int offset = displacement_for_each_proc[i];
        int count = each_proc_ser_attr_entries_size[i];
        if(count > 0) {
            if(i != rank) {
                //extreme_debug_log << "rank " << rank << " count: " << count << " offset: " << offset << endl;
                char serialzed_attr_entries_for_one_proc[count];

                memcpy ( serialzed_attr_entries_for_one_proc, serialized_c_str_all_attr_entries + offset, count);
                std::vector<T> rec_attr_entries;

                //extreme_debug_log << "rank " << rank << " serialized_c_str: " << (string)serialzed_attr_entries_for_one_proc << endl;
                //extreme_debug_log <<  " serialized_c_str length: " << strlen(serialzed_attr_entries_for_one_proc) << 
                    // " count: " << count << endl;
                std::stringstream ss1;
                ss1.write(serialzed_attr_entries_for_one_proc, count);
                //extreme_debug_log << "about to deserialize ss: " << ss1.str() << endl;
                boost::archive::text_iarchive ia(ss1);
                ia >> rec_attr_entries;
                //extreme_debug_log << "rank " << rank << " received attr_entries.size(): " << attr_entries.size() << endl;

                all_attr_entries.insert(all_attr_entries.end(), rec_attr_entries.begin(), rec_attr_entries.end());

            }
            else { //i == rank
                all_attr_entries.insert(all_attr_entries.end(), attr_entries.begin(), attr_entries.end());
            }
        }
    }

    if(length_ser_c_str > 0) {
        free(serialized_c_str);
    }
    free(serialized_c_str_all_attr_entries);

    add_timing_point(GATHER_ATTR_ENTRIES_DONE);
}

template <class T>
void gather_attr_entries(std::vector<T> attr_entries, uint32_t count, int rank, uint32_t num_client_procs,
     std::vector<T> &all_attr_entries, MPI_Comm comm) {

 add_timing_point(GATHER_ATTR_ENTRIES_START);

 //extreme_debug_log.set_rank(rank);

 int length_ser_c_str = 0;
 char *serialized_c_str;
 int each_proc_ser_attr_entries_size[num_client_procs];
 int displacement_for_each_proc[num_client_procs];

 char *serialized_c_str_all_attr_entries;        

 //extreme_debug_log << "rank " << rank << " attr_entries.size(): " <<attr_entries.size() << endl;
 //extreme_debug_log << "num_client_procs: " << num_client_procs << endl;
 if (attr_entries.size() > 0) {
     std::stringstream ss;
     boost::archive::text_oarchive oa(ss);
     // //extreme_debug_log << "rank " << rank << " about to try serializing at the beginning" << endl;
     oa << attr_entries;
     // //extreme_debug_log << "rank " << rank << " just finished serializing at the beginning" << endl;
     std::string serialized_str = ss.str();
     length_ser_c_str = serialized_str.size() + 1;
     serialized_c_str = (char *) malloc(length_ser_c_str);
     serialized_str.copy(serialized_c_str, serialized_str.size());
     serialized_c_str[serialized_str.size()]='\0';
     // //extreme_debug_log << "rank " << rank << " attrs ser string is of size " << length_ser_c_str << " serialized_str " << 
     //  serialized_str << " serialized_c_str: " << serialized_c_str << endl;
     //extreme_debug_log << "rank " << rank << " attrs ser string is of size " << length_ser_c_str << " serialized_str " << 
         // serialized_str << endl;
 }
        
 //extreme_debug_log << "rank " << rank << " about to allgather my length_ser_c_str: " << length_ser_c_str << endl;

 MPI_Allgather(&length_ser_c_str, 1, MPI_INT, each_proc_ser_attr_entries_size, 1, MPI_INT, comm);

 //extreme_debug_log << "rank " << rank << " done with allgather" << endl;

    int sum = 0;
    
    for(int i=0; i<num_client_procs; i++) {
     //extreme_debug_log << "i: " << i << " sum: " << sum << endl;
        displacement_for_each_proc[i] = sum;
        //extreme_debug_log << "each_proc_ser_attr_entries_size[i]: " << each_proc_ser_attr_entries_size[i] << endl;
        sum += each_proc_ser_attr_entries_size[i];
        // if(each_proc_ser_attr_entries_size[i] != 0 && rank == 0) {
         //extreme_debug_log << "rank " << i << " has a string of length " << each_proc_ser_attr_entries_size[i] << endl;
        // }
    }

    serialized_c_str_all_attr_entries = (char *) malloc(sum);
    // //extreme_debug_log << "num_bytes is "<< to_string(length_ser_c_str) + " and rank: " << to_string(rank) << endl;
    
     // //extreme_debug_log << "rank " << rank << " about to allgatherv" << endl;

     MPI_Allgatherv(serialized_c_str, length_ser_c_str, MPI_CHAR,
           serialized_c_str_all_attr_entries, each_proc_ser_attr_entries_size, displacement_for_each_proc,
           MPI_CHAR, comm);

 // //extreme_debug_log << "rank " << rank << " done with allgatherv" << endl;
 // //extreme_debug_log << "entire set of attr std::vectors: " << serialized_c_str_all_attr_entries << " and is of length: " << strlen(serialized_c_str_all_attr_entries) << endl;

 for(int i = 0; i < num_client_procs; i++) {
     int offset = displacement_for_each_proc[i];
     int count = each_proc_ser_attr_entries_size[i];
     if(count > 0) {
         if(i != rank) {
             //extreme_debug_log << "rank " << rank << " count: " << count << " offset: " << offset << endl;
             char serialzed_attr_entries_for_one_proc[count];

             memcpy ( serialzed_attr_entries_for_one_proc, serialized_c_str_all_attr_entries + offset, count);
             std::vector<T> rec_attr_entries;

             //extreme_debug_log << "rank " << rank << " serialized_c_str: " << (string)serialzed_attr_entries_for_one_proc << endl;
             //extreme_debug_log <<  " serialized_c_str length: " << strlen(serialzed_attr_entries_for_one_proc) << 
                 // " count: " << count << endl;
             std::stringstream ss1;
             ss1.write(serialzed_attr_entries_for_one_proc, count);
             //extreme_debug_log << "about to deserialize ss: " << ss1.str() << endl;
             boost::archive::text_iarchive ia(ss1);
             ia >> rec_attr_entries;
             //extreme_debug_log << "rank " << rank << " received attr_entries.size(): " << attr_entries.size() << endl;

             all_attr_entries.insert(all_attr_entries.end(), rec_attr_entries.begin(), rec_attr_entries.end());

         }
         else { //i == rank
             all_attr_entries.insert(all_attr_entries.end(), attr_entries.begin(), attr_entries.end());
         }
        }
 }

 if(length_ser_c_str > 0) {
     free(serialized_c_str);
 }
    free(serialized_c_str_all_attr_entries);

 add_timing_point(GATHER_ATTR_ENTRIES_DONE);
}

#endif //READ_HELPER_FUNCTIONS_HH