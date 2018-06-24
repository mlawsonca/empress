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

#ifndef RC_ENUM
#define RC_ENUM
enum RC
{
    RC_OK = 0,
    RC_DIRMAN_ERR = 2,
    RC_ERR = -1
};
#endif

#ifndef MYMETADATA_ARGS_H
#define MYMETADATA_ARGS_H

#include <vector>
#include <stdint.h>
#include <boost/serialization/vector.hpp>



struct md_dim_bounds
{
  uint32_t min;
  uint32_t max;

  template <typename Archive>
  void serialize(Archive& ar, const unsigned int ver)
  {
  ar & min;
  ar & max;
  }
};

struct point 
{
  double x;
  double y;
  double z;

  template <typename Archive>
  void serialize(Archive& ar, const unsigned int ver)
  {
  ar & x;
  ar & y;
  ar & z;
  }
};

struct md_catalog_var_entry
{
  uint64_t var_id;
  std::string name;
  std::string path;
  uint32_t version;
  char type;
  uint32_t active;
  uint64_t txn_id;
  uint32_t num_dims;
  std::vector<md_dim_bounds> dims;

  template <typename Archive>
  void serialize(Archive& ar, const unsigned int ver)
  {
  ar & var_id;
  ar & name;
  ar & path;
  ar & version;
  ar & type;
  ar & active;
  ar & txn_id;
  ar & num_dims;
  ar & dims;
  }
};

struct md_catalog_type_entry
{
  uint64_t type_id;
  std::string name;
  uint32_t version;
  uint32_t active;
  uint64_t txn_id;


  template <typename Archive>
  void serialize(Archive& ar, const unsigned int ver)
  {
  ar & type_id;
  ar & name;
  ar & version;
  ar & active;
  ar & txn_id;
  }
};

struct md_attribute_entry
{
  uint64_t attribute_id;
  uint64_t chunk_id;
  uint64_t type_id;
  uint32_t type_version; //type_version
  std::string data;

  template <typename Archive>
  void serialize(Archive& ar, const unsigned int ver)
  {
  ar & attribute_id;
  ar & chunk_id;
  ar & type_id;
  ar & type_version;
  ar & data;

  }
};


struct md_chunk_entry
{
  uint64_t chunk_id;
  uint64_t length_of_chunk;
  std::string connection;
  uint32_t var_version; //var_version
  uint32_t num_dims;
  std::vector<md_dim_bounds> dims;

  template <typename Archive>
  void serialize(Archive& ar, const unsigned int ver)
  {
  ar & chunk_id;
  ar & length_of_chunk;
  ar & connection;
  ar & var_version;
  ar & num_dims;
  ar & dims;

  }
};

struct md_activate_type_args
{
    uint64_t txn_id; /* if an in-process txn, must provide */  

    template <typename Archive>
    void serialize(Archive& ar, const unsigned int ver)
    {
      ar & txn_id;
    }
};


struct md_activate_var_args
{
    uint64_t txn_id; /* if an in-process txn, must provide */ 

    template <typename Archive>
    void serialize(Archive& ar, const unsigned int ver)
    {
      ar & txn_id;
    }
};

struct md_catalog_args
{
    uint64_t txn_id; /* if an in-process txn, must provide */


    template <typename Archive>
    void serialize(Archive& ar, const unsigned int ver)
    {
      ar & txn_id;
    }
};

struct md_create_type_args
{
    uint64_t txn_id;
    /* uint64_t type_id is returned */
    std::string name;
    uint32_t version;
    
    template <typename Archive>
    void serialize(Archive& ar, const unsigned int ver)
    {
      ar & txn_id;
      ar & name;
      ar & version;

    }
};


struct md_create_var_args
{
    uint64_t txn_id;
    /* uint64_t var_id is returned */
    std::string name;
    std::string path;
    char type;
    uint32_t version;
    uint32_t num_dims;
    std::vector<md_dim_bounds> dims;

    
    template <typename Archive>
    void serialize(Archive& ar, const unsigned int ver)
    {
      ar & txn_id;
      ar & name;
      ar & path;
      ar & type;
      ar & version;
      ar & num_dims;
      ar & dims;
    }
};

struct md_delete_type_args
{
    uint64_t type_id;
    /* uint64_t type_id is returned */
    std::string name;
    uint32_t version;
    
    template <typename Archive>
    void serialize(Archive& ar, const unsigned int ver)
    {
      ar & type_id;
      ar & name;
      ar & version;

    }
};


struct md_delete_var_args
{
    uint64_t var_id;
    std::string name;
    std::string path;
    uint32_t version;

    template <typename Archive>
    void serialize(Archive& ar, const unsigned int ver)
    {
      ar & var_id;
      ar & name;
      ar & path;
      ar & version;
    }
};


struct md_get_attribute_args
{
    uint64_t txn_id; /* if an in-process txn, must provide */
    uint64_t chunk_id;

    /* struct (int32_t min, int32_t max} * box dims is bulk data */
    /* catalog struct is return bulk data of the relevant chunks */    

    template <typename Archive>
    void serialize(Archive& ar, const unsigned int ver)
    {
      ar & txn_id;
      ar & chunk_id;
    }
};

struct md_get_attribute_list_args
{
    uint64_t txn_id; /* if an in-process txn, must provide */
    std::string type_name;
    uint32_t type_version;


    template <typename Archive>
    void serialize(Archive& ar, const unsigned int ver)
    {
      ar & txn_id;
      ar & type_name;
      ar & type_version;
    }
};

struct md_get_chunk_args
{
    uint64_t txn_id; /* if an in-process txn, must provide */
    std::string name;
    std::string path;
    uint32_t var_version;
    uint32_t num_dims;
    std::vector<md_dim_bounds> dims;

    template <typename Archive>
    void serialize(Archive& ar, const unsigned int ver)
    {
      ar & txn_id;
      ar & name;
      ar & path;
      ar & var_version;
      ar & num_dims;
      ar & dims;
    }
};

struct md_get_chunk_list_args
{
    uint64_t txn_id; /* if an in-process txn, must provide */
    std::string name;
    std::string path;
    uint32_t var_version;
    uint32_t num_dims;
    /* catalog struct is return bulk data of the relevant chunks */

    template <typename Archive>
    void serialize(Archive& ar, const unsigned int ver)
    {
      ar & txn_id;
      ar & name;
      ar & path;
      ar & var_version;
      ar & num_dims;
    }
};

struct md_insert_attribute_args
{
    uint64_t chunk_id;
    uint64_t type_id;
    std::string data;

    template <typename Archive>
    void serialize(Archive& ar, const unsigned int ver)
    {
      ar & chunk_id;
      ar & type_id;
      ar & data;

    }
};


struct md_insert_chunk_args
{
    uint64_t var_id;
    uint32_t num_dims;
    std::string connection;
    uint64_t length_of_chunk;  /* use for retrieval of chunk from datastore */
    std::vector<md_dim_bounds> dims;


    template <typename Archive>
    void serialize(Archive& ar, const unsigned int ver)
    {
      ar & var_id;
      ar & num_dims;
      ar & connection;
      ar & length_of_chunk;
      ar & dims;

    }
};

struct md_processing_type_args
{
    uint64_t txn_id; /* if an in-process txn, must provide */    

    template <typename Archive>
    void serialize(Archive& ar, const unsigned int ver)
    {
      ar & txn_id;
    }
};

struct md_processing_var_args
{
    uint64_t txn_id; /* if an in-process txn, must provide */


    template <typename Archive>
    void serialize(Archive& ar, const unsigned int ver)
    {
      ar & txn_id;
    }
};


#endif
