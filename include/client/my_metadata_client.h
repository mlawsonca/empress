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

#ifndef MYMETADATA_CLIENT_H
#define MYMETADATA_CLIENT_H

#include <gutties/Gutties.hh>
#include <opbox/net/net.hh>
#include <my_metadata_args.h>


struct metadata_server
{
    gutties::name_and_node_t name_and_node;
    std::string URL;
    net::peer_ptr_t peer_ptr;

    template <typename Archive>
    void serialize(Archive& ar, const unsigned int ver)
    {
    ar & name_and_node;
    ar & URL;
    }

};

// void print_var_catalog (uint32_t num_vars, const std::vector<md_catalog_var_entry> &entries);
// void print_chunk_list (uint32_t num_vars, const std::vector<md_chunk_entry> &chunks);
// void print_type_catalog (uint32_t num_vars, const std::vector<md_catalog_type_entry> &entries);
// void print_attribute_list (uint32_t num_vars, const std::vector<md_attribute_entry> &attributes);


int metadata_init ();
int metadata_finalize_server (const metadata_server &server);

// create a variable in an inactive state. Chunks will be added later. Once the
// variable is complete (the transaction is ready to commit), it can then be
// activated to make it visible to other processes.
// For a scalar, num_dims should be 0 causing the other parameters
// to be ignored.

int metadata_activate_type (const metadata_server &server, uint64_t txn_id);

int metadata_activate_var (const metadata_server &server, uint64_t txn_id);

int metadata_catalog_var(const metadata_server &server, uint64_t txn_id, uint32_t &count, 
                    std::vector<md_catalog_var_entry> &entries);

int metadata_catalog_type (const metadata_server &server, uint64_t txn_id, uint32_t &count,
                           std::vector<md_catalog_type_entry> &entries);

int metadata_create_type (const metadata_server &server, uint64_t &type_id,
                          const struct md_catalog_type_entry &new_type);

int metadata_create_var (const metadata_server &server, uint64_t &var_id, 
                       const struct md_catalog_var_entry &new_var);

int metadata_delete_type (const metadata_server &server, uint64_t type_id, const std::string &name, 
                         uint32_t version);

int metadata_delete_var (const metadata_server &server, uint64_t var_id, const std::string &name, 
             const std::string &path, uint32_t version);

int metadata_get_attribute (const metadata_server &server, uint64_t txn_id, uint64_t chunk_id, uint32_t &count,
                        std::vector<md_attribute_entry> &matching_attributes);

int metadata_get_attribute_list (const metadata_server &server, uint64_t txn_id, 
                               const std::string &type_name, uint32_t type_version, 
                               uint32_t &count, std::vector<md_attribute_entry> &entries);

int metadata_get_chunk (const metadata_server &server, const struct md_catalog_var_entry &desired_box, 
                      uint32_t &count, std::vector<md_chunk_entry> &matching_chunks);

int metadata_get_chunk_list (const metadata_server &server, uint64_t txn_id, const std::string &name, 
                           const std::string &path, uint32_t version, uint32_t &count, 
               std::vector<md_chunk_entry> &entries);

int metadata_insert_attribute (const metadata_server &server, uint64_t &attribute_id,
                               const struct md_attribute_entry &new_attribute);

int metadata_insert_chunk (const metadata_server &server, uint64_t var_id, uint64_t &chunk_id,
                         const struct md_chunk_entry &new_chunk);

int metadata_processing_type (const metadata_server &server, uint64_t txn_id);

int metadata_processing_var (const metadata_server &server, uint64_t txn_id);

   
//for debug testing purposes
struct debugLog {
  private:
    bool on;
    bool zero_rank_logging;
    int my_rank;

  public:
  debugLog(bool turn_on, bool zero_rank_log=false, int rank=-1) {
    on = turn_on;
    zero_rank_logging = zero_rank_log;
    my_rank = rank;
  }
  void set_rank(int rank) {
    my_rank = rank;
  }
  void turn_on_zero_rank_logging() {
    zero_rank_logging = true;
  }
  void turn_off_zero_rank_logging() {
    zero_rank_logging = false;
  }
  void turn_on_logging() {
    on = true;
  }
  void turn_off_logging() {
    on = false;
  }
  template<typename T> debugLog& operator << (const T& x) {
   if(on || (zero_rank_logging && (my_rank == 0) )) {
      std::cout << x;
    }
    return *this;
  }
  debugLog& operator<<(std::ostream& (*manipulator)(std::ostream&)) {
   if(on || (zero_rank_logging && (my_rank == 0) )) {
      std::cout << manipulator;
    }
    return *this;
  }
};

struct extremeDebugLog {
  private:
    bool on;
    bool zero_rank_logging;
    int my_rank;

  public:
  extremeDebugLog(bool turn_on, bool zero_rank_log=false, int rank=-1) {
    on = turn_on;
    zero_rank_logging = zero_rank_log;
    my_rank = rank;
  }
  void set_rank(int rank) {
    my_rank = rank;
  }
  void turn_on_zero_rank_logging() {
    zero_rank_logging = true;
  }
  void turn_off_zero_rank_logging() {
    zero_rank_logging = false;
  }
  void turn_on_logging() {
    on = true;
  }
  void turn_off_logging() {
    on = false;
  }
  template<typename T> extremeDebugLog& operator << (const T& x) {
   if(on || (zero_rank_logging && (my_rank == 0) )) {
      std::cout << x;
    }
    return *this;
  }
  extremeDebugLog& operator<<(std::ostream& (*manipulator)(std::ostream&)) {
   if(on || (zero_rank_logging && (my_rank == 0) )) {
      std::cout << manipulator;
    }
    return *this;
  }
};


struct errorLog {
  private:
    bool on;
    bool zero_rank_logging;
    int my_rank;

  public:
  errorLog(bool turn_on, bool zero_rank_log=false, int rank=-1) {
    on = turn_on;
    zero_rank_logging = zero_rank_log;
    my_rank = rank;
  }
  void set_rank(int rank) {
    my_rank = rank;
  }
  void turn_on_zero_rank_logging() {
    zero_rank_logging = true;
  }
  void turn_off_zero_rank_logging() {
    zero_rank_logging = false;
  }
  void turn_on_logging() {
    on = true;
  }
  void turn_off_logging() {
    on = false;
  }
  template<typename T> errorLog& operator << (const T& x) {
   if(on || (zero_rank_logging && (my_rank == 0) )) {
      std::cout << x;
    }
    return *this;
  }
  errorLog& operator<<(std::ostream& (*manipulator)(std::ostream&)) {
   if(on || (zero_rank_logging && (my_rank == 0) )) {
      std::cout << manipulator;
    }
     return *this;
  }
};

void print_var_catalog (uint32_t num_vars, const std::vector<md_catalog_var_entry> &entries);
void print_chunk_list (uint32_t num_vars, const std::vector<md_chunk_entry> &chunks);
void print_type_catalog (uint32_t num_vars, const std::vector<md_catalog_type_entry> &entries);
void print_attribute_list (uint32_t num_vars, const std::vector<md_attribute_entry> &attributes);

#endif
