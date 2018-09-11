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
    RC_ERR = -1
};
#endif //RC_ENUM


#ifndef ATTR_DATA_TYPE_ENUM
#define ATTR_DATA_TYPE_ENUM
enum attr_data_type : unsigned short
{
    ATTR_DATA_TYPE_NULL = 0,
    ATTR_DATA_TYPE_INT = 1,
    ATTR_DATA_TYPE_REAL = 2,
    ATTR_DATA_TYPE_TEXT = 3,
    ATTR_DATA_TYPE_BLOB = 4,
};
#endif //ATTR_DATA_TYPE_ENUM

#ifndef DATA_RANGE_TYPE_ENUM
#define DATA_RANGE_TYPE_ENUM
enum data_range_type : unsigned short
{
    DATA_RANGE = 0,
    DATA_MAX = 1,
    DATA_MIN = 2
};
#endif //DATA_RANGE_TYPE_ENUM

#ifndef MD_CATALOG_TYPE_ENUM
#define MD_CATALOG_TYPE_ENUM
enum md_catalog_type : unsigned short
{
    RUN_CATALOG = 0,
    TIMESTEP_CATALOG = 1,
    VAR_CATALOG = 2,
    TYPE_CATALOG = 3,
    RUN_ATTR_CATALOG = 4,
    TIMESTEP_ATTR_CATALOG = 5,
    VAR_ATTR_CATALOG = 6
};
#endif //MD_CATALOG_TYPE_ENUM

#ifndef MYMETADATA_ARGS_H
#define MYMETADATA_ARGS_H

#include <vector>
#include <stdint.h>
#include <boost/serialization/vector.hpp>

//for debugging testing purposes/////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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



struct md_dim_bounds
{
    uint64_t min;
    uint64_t max;

    template <typename Archive>
    void serialize(Archive& ar, const unsigned int ver)
    {
    ar & min;
    ar & max;
    }
};


struct md_catalog_run_attribute_entry
{
    uint64_t attribute_id;
    uint64_t type_id;
    uint32_t active;
    uint64_t txn_id; 
    attr_data_type data_type;
    std::string data;

    template <typename Archive>
    void serialize(Archive& ar, const unsigned int ver)
    {
    ar & attribute_id;
    ar & type_id;
    ar & active;
    ar & txn_id;
    ar & data_type;
    ar & data;
    }
};



struct md_catalog_run_entry
{
    uint64_t run_id;
    uint64_t job_id;
    std::string name;
    // std::string path;
    std::string date;
    uint32_t active;
    uint64_t txn_id;
    uint32_t npx;
    uint32_t npy;
    uint32_t npz;
    std::string rank_to_dims_funct;
    std::string objector_funct;

    template <typename Archive>
    void serialize(Archive& ar, const unsigned int ver)
    {
    ar & run_id;
    ar & job_id;
    ar & name;
    // ar & path;
    ar & date;
    ar & active;
    ar & txn_id;
    ar & npx; 
    ar & npy;
    ar & npz;
    ar & rank_to_dims_funct;
    ar & objector_funct;
    }
};


struct md_catalog_timestep_attribute_entry
{
    uint64_t attribute_id;
    uint64_t timestep_id;
    uint64_t type_id;
    uint32_t active;
    uint64_t txn_id; 
    attr_data_type data_type;
    // std::xstring data;
    std::string data;

    template <typename Archive>
    void serialize(Archive& ar, const unsigned int ver)
    {
    ar & attribute_id;
    ar & timestep_id;
    ar & type_id;
    ar & active;
    ar & txn_id;
    ar & data_type;
    ar & data;
    }
};

struct md_catalog_timestep_entry
{
    uint64_t timestep_id;
    uint64_t run_id;
    std::string path;
    uint32_t active;
    uint64_t txn_id;


    template <typename Archive>
    void serialize(Archive& ar, const unsigned int ver)
    {
    ar & timestep_id;
    ar & run_id;
    ar & path;
    ar & active;
    ar & txn_id;
    }
};


struct md_catalog_type_entry
{
    uint64_t type_id;
    uint64_t run_id;
    std::string name;
    uint32_t version;
    uint32_t active;
    uint64_t txn_id;


    template <typename Archive>
    void serialize(Archive& ar, const unsigned int ver)
    {
    ar & type_id;
    ar & run_id;
    ar & name;
    ar & version;
    ar & active;
    ar & txn_id;
    }
};


struct md_catalog_var_attribute_entry
{
    uint64_t attribute_id;
    uint64_t timestep_id;
    uint64_t type_id;
    uint64_t var_id;
    uint32_t active;
    uint64_t txn_id; 
    uint32_t num_dims;
    std::vector<md_dim_bounds> dims;    
    attr_data_type data_type;
    // std::xstring data;
    std::string data;

    template <typename Archive>
    void serialize(Archive& ar, const unsigned int ver)
    {
    ar & attribute_id;
    ar & timestep_id;
    ar & type_id;
    ar & var_id;
    ar & active;
    ar & txn_id;
    ar & num_dims;
    ar & dims;
    ar & data_type;
    ar & data;
    }
};


struct md_catalog_var_entry
{
    uint64_t var_id;
    uint64_t run_id;
    uint64_t timestep_id;
    std::string name;
    std::string path;
    uint32_t version;
    char data_size;
    uint32_t active;
    uint64_t txn_id;
    uint32_t num_dims;
    std::vector<md_dim_bounds> dims;

    template <typename Archive>
    void serialize(Archive& ar, const unsigned int ver)
    {
    ar & var_id;
    ar & run_id;
    ar & timestep_id;
    ar & name;
    ar & path;
    ar & version;
    ar & data_size;
    ar & active;
    ar & txn_id;
    ar & num_dims;
    ar & dims;
    }
};


struct objector_params
{   
    int rank;
    uint64_t run_id; //can be used to make sure the right run is being used
    uint64_t job_id;
    std::string run_name;
    uint64_t timestep_id;
    uint64_t ndx;
    uint64_t ndy;
    uint64_t ndz;
    uint64_t var_id; //can be used to make sure the right var is being used
    std::string var_name;
    uint32_t var_version;
    std::vector<md_dim_bounds> var_dims;
    std::vector<md_dim_bounds> bounding_box;
    uint16_t read_type;

    template <typename Archive>
    void serialize(Archive& ar, const unsigned int ver)
    {
    ar & rank;
    ar & run_id;
    ar & job_id;
    ar & run_name;
    ar & timestep_id;
    ar & ndx;
    ar & ndy;
    ar & ndz;
    ar & var_id;
    ar & var_name;
    ar & var_version;
    ar & var_dims;
    ar & bounding_box;
    ar & read_type;
    }
};

// struct objector_params
// {
//     md_catalog_run_entry run;
//     md_catalog_var_entry var;
//     std::vector<md_dim_bounds> bounding_box;

//     template <typename Archive>
//     void serialize(Archive& ar, const unsigned int ver)
//     {
//     ar & run;
//     ar & var;
//     ar & bounding_box;
//     }
// };

struct md_activate_args
{
    uint64_t txn_id; /* if an in-process txn, must provide */  
    md_catalog_type catalog_type;

    template <typename Archive>
    void serialize(Archive& ar, const unsigned int ver)
    {
      ar & txn_id;
      ar & catalog_type;
    }
};

struct md_catalog_all_run_attributes_args
{
    uint64_t run_id;
    uint64_t txn_id; 

    template <typename Archive>
    void serialize(Archive& ar, const unsigned int ver)
    {
      ar & run_id;
      ar & txn_id;
    }
};


struct md_catalog_all_run_attributes_with_type_args
{
    uint64_t run_id;
    uint64_t type_id;
    uint64_t txn_id; 

    template <typename Archive>
    void serialize(Archive& ar, const unsigned int ver)
    {
      ar & run_id;
      ar & type_id;
      ar & txn_id;
    }
};

struct md_catalog_all_run_attributes_with_type_range_args
{
    uint64_t run_id;
    uint64_t type_id;
    uint64_t txn_id;
    attr_data_type data_type;
    std::string data;
    data_range_type range_type;

    template <typename Archive>
    void serialize(Archive& ar, const unsigned int ver)
    {
      ar & run_id;
      ar & type_id;
      ar & txn_id;
      ar & data_type;
      ar & data;
      ar & range_type;
    }
};


struct md_catalog_all_timestep_attributes_args
{
    uint64_t run_id;
    uint64_t timestep_id;
    uint64_t txn_id; 

    template <typename Archive>
    void serialize(Archive& ar, const unsigned int ver)
    {
      ar & run_id;
      ar & timestep_id;
      ar & txn_id;
    }
};

struct md_catalog_all_timestep_attributes_with_type_args
{
    uint64_t run_id;
    uint64_t timestep_id;
    uint64_t type_id;
    uint64_t txn_id; 

    template <typename Archive>
    void serialize(Archive& ar, const unsigned int ver)
    {
      ar & run_id;
      ar & timestep_id;
      ar & type_id;
      ar & txn_id;
    }
};

struct md_catalog_all_timestep_attributes_with_type_range_args
{
    uint64_t run_id;
    uint64_t timestep_id;
    uint64_t type_id;
    uint64_t txn_id; 
    attr_data_type data_type;
    std::string data;
    data_range_type range_type;

    template <typename Archive>
    void serialize(Archive& ar, const unsigned int ver)
    {
      ar & run_id;
      ar & timestep_id;
      ar & type_id;
      ar & txn_id;
      ar & data_type;
      ar & data;
      ar & range_type;
    }
};

struct md_catalog_all_timesteps_with_var_attributes_with_type_var_dims_args
{
    uint64_t run_id;
    uint64_t type_id;
    uint64_t var_id;
    uint64_t txn_id; 
    uint32_t num_dims;
    std::vector<md_dim_bounds> dims;  

    template <typename Archive>
    void serialize(Archive& ar, const unsigned int ver)
    {
      ar & run_id;
      ar & type_id;
      ar & var_id;
      ar & txn_id;
      ar & num_dims;
      ar & dims;
    }
};

struct md_catalog_all_timesteps_with_var_attributes_with_type_var_dims_range_args
{
    uint64_t run_id;
    uint64_t type_id;
    uint64_t var_id;
    uint64_t txn_id; 
    uint32_t num_dims;
    std::vector<md_dim_bounds> dims;
    attr_data_type data_type;
    std::string data;
    data_range_type range_type;

    template <typename Archive>
    void serialize(Archive& ar, const unsigned int ver)
    {
      ar & run_id;
      ar & type_id;
      ar & var_id;
      ar & txn_id;
      ar & num_dims;
      ar & dims;
      ar & data_type;
      ar & data;
      ar & range_type;
    }
};

struct md_catalog_all_timesteps_with_var_attributes_with_type_var_args
{
    uint64_t run_id;
    uint64_t type_id;
    uint64_t var_id;
    uint64_t txn_id; 

    template <typename Archive>
    void serialize(Archive& ar, const unsigned int ver)
    {
      ar & run_id;
      ar & type_id;
      ar & var_id;
      ar & txn_id;
    }
};

struct md_catalog_all_timesteps_with_var_attributes_with_type_var_range_args
{
    uint64_t run_id;
    uint64_t type_id;
    uint64_t var_id;
    uint64_t txn_id; 
    attr_data_type data_type;
    std::string data;
    data_range_type range_type;

    template <typename Archive>
    void serialize(Archive& ar, const unsigned int ver)
    {
      ar & run_id;
      ar & type_id;
      ar & var_id;
      ar & txn_id;
      ar & data_type;
      ar & data;
      ar & range_type;
    }
};


struct md_catalog_all_timesteps_with_var_args
{
    uint64_t run_id;
    uint64_t var_id;
    uint64_t txn_id; 

    template <typename Archive>
    void serialize(Archive& ar, const unsigned int ver)
    {
      ar & run_id;
      ar & var_id;
      ar & txn_id;
    }
};

struct md_catalog_all_types_with_var_attributes_in_timestep_args
{
    uint64_t run_id;
    uint64_t timestep_id;
    uint64_t txn_id; 

    template <typename Archive>
    void serialize(Archive& ar, const unsigned int ver)
    {
      ar & run_id;
      ar & timestep_id;
      ar & txn_id;
    }
};

struct md_catalog_all_types_with_var_attributes_with_var_dims_in_timestep_args
{
    uint64_t run_id;
    uint64_t timestep_id;
    uint64_t var_id;
    uint64_t txn_id; 
    uint32_t num_dims;
    std::vector<md_dim_bounds> dims;  

    template <typename Archive>
    void serialize(Archive& ar, const unsigned int ver)
    {
      ar & run_id;
      ar & timestep_id;
      ar & var_id;
      ar & txn_id;
      ar & num_dims;
      ar & dims;
    }
};

struct md_catalog_all_types_with_var_attributes_with_var_in_timestep_args
{
    uint64_t run_id;
    uint64_t timestep_id;
    uint64_t var_id;
    uint64_t txn_id; 

    template <typename Archive>
    void serialize(Archive& ar, const unsigned int ver)
    {
      ar & run_id;
      ar & timestep_id;
      ar & var_id;
      ar & txn_id;
    }
};

struct md_catalog_all_var_attributes_args
{
    uint64_t run_id;
    uint64_t timestep_id;
    uint64_t txn_id; 

    template <typename Archive>
    void serialize(Archive& ar, const unsigned int ver)
    {
      ar & run_id;
      ar & timestep_id;
      ar & txn_id;
    }
};


struct md_catalog_all_var_attributes_with_dims_args
{
    uint64_t run_id;
    uint64_t timestep_id;
    uint64_t txn_id; /* if an in-process txn, must provide */
    uint32_t num_dims;
    std::vector<md_dim_bounds> dims;  

    template <typename Archive>
    void serialize(Archive& ar, const unsigned int ver)
    {
      ar & run_id;
      ar & txn_id;   
      ar & num_dims;
      ar & dims;
      ar & timestep_id;
    }
};

struct md_catalog_all_var_attributes_with_type_by_id_args
{
    uint64_t type_id; 
    uint64_t timestep_id;
    uint64_t txn_id; 

    template <typename Archive>
    void serialize(Archive& ar, const unsigned int ver)
    {
      ar & type_id;
      ar & timestep_id;
      ar & txn_id;
    }
};

struct md_catalog_all_var_attributes_with_type_by_name_ver_args
{
    uint64_t run_id;
    uint64_t timestep_id;
    std::string type_name;
    uint32_t type_version;
    uint64_t txn_id; 

    template <typename Archive>
    void serialize(Archive& ar, const unsigned int ver)
    {
      ar & run_id;
      ar & timestep_id;
      ar & type_name;
      ar & type_version;
      ar & txn_id;
    }
};

struct md_catalog_all_var_attributes_with_type_dims_by_id_args
{
    uint64_t txn_id; /* if an in-process txn, must provide */
    uint64_t type_id;
    uint32_t num_dims;
    std::vector<md_dim_bounds> dims; 
    uint64_t timestep_id;

    template <typename Archive>
    void serialize(Archive& ar, const unsigned int ver)
    {
      ar & txn_id;
      ar & type_id;   
      ar & num_dims;
      ar & dims;
      ar & timestep_id;
    }
};

struct md_catalog_all_var_attributes_with_type_dims_by_name_ver_args
{
    uint64_t run_id;
    uint64_t timestep_id;
    std::string type_name;
    uint32_t type_version;
    uint32_t num_dims;
    std::vector<md_dim_bounds> dims; 
    uint64_t txn_id; /* if an in-process txn, must provide */

    template <typename Archive>
    void serialize(Archive& ar, const unsigned int ver)
    {
      ar & run_id;
      ar & timestep_id;
      ar & type_name;
      ar & type_version;    
      ar & num_dims;
      ar & dims;
      ar & txn_id;
    }
};

struct md_catalog_all_var_attributes_with_type_var_by_id_args
{
    uint64_t type_id;
    uint64_t var_id;
    uint64_t timestep_id;
    uint64_t txn_id; 

    template <typename Archive>
    void serialize(Archive& ar, const unsigned int ver)
    {
      ar & type_id;
      ar & var_id;
      ar & timestep_id;
      ar & txn_id;
   }
};

struct md_catalog_all_var_attributes_with_type_var_by_name_ver_args
{
    uint64_t run_id;
    uint64_t timestep_id;
    std::string type_name;
    uint32_t type_version;
    std::string var_name;
    uint32_t var_version;
    uint64_t txn_id; 

    template <typename Archive>
    void serialize(Archive& ar, const unsigned int ver)
    {
      ar & run_id;
      ar & timestep_id;
      ar & type_name;
      ar & type_version;
      ar & var_name;
      ar & var_version;
      ar & txn_id;
    }
};


struct md_catalog_all_var_attributes_with_type_var_dims_by_id_args
{
    uint64_t timestep_id;
    uint64_t type_id;
    uint64_t var_id;
    uint32_t num_dims;
    std::vector<md_dim_bounds> dims; 
    uint64_t txn_id; /* if an in-process txn, must provide */

    template <typename Archive>
    void serialize(Archive& ar, const unsigned int ver)
    {
      ar & timestep_id;
      ar & type_id;
      ar & var_id;   
      ar & num_dims;
      ar & dims;
      ar & txn_id;
   }
};


struct md_catalog_all_var_attributes_with_type_var_dims_by_name_ver_args
{
    uint64_t run_id;
    uint64_t timestep_id;
    std::string type_name;
    uint32_t type_version;
    std::string var_name;
    uint32_t var_version;
    uint32_t num_dims;
    std::vector<md_dim_bounds> dims; 
    uint64_t txn_id; /* if an in-process txn, must provide */


    template <typename Archive>
    void serialize(Archive& ar, const unsigned int ver)
    {
      ar & run_id;
      ar & timestep_id;
      ar & type_name;
      ar & type_version;
      ar & var_name;
      ar & var_version;     
      ar & num_dims;
      ar & dims;
      ar & txn_id;
    }
};

struct md_catalog_all_var_attributes_with_type_var_dims_range_args
{
    uint64_t timestep_id;
    uint64_t type_id;
    uint64_t var_id;
    uint64_t txn_id; /* if an in-process txn, must provide */
    uint32_t num_dims;
    std::vector<md_dim_bounds> dims;
    attr_data_type data_type;
    std::string data;
    data_range_type range_type;

    template <typename Archive>
    void serialize(Archive& ar, const unsigned int ver)
    {
      ar & timestep_id;
      ar & type_id;
      ar & var_id;
      ar & num_dims;
      ar & dims;   
      ar & txn_id;
      ar & data_type;
      ar & data;
      ar & range_type;
   }
};



struct md_catalog_all_var_attributes_with_type_var_range_args
{
    uint64_t timestep_id;
    uint64_t type_id;
    uint64_t var_id;
    uint64_t txn_id; /* if an in-process txn, must provide */
    attr_data_type data_type;
    std::string data;
    data_range_type range_type;

    template <typename Archive>
    void serialize(Archive& ar, const unsigned int ver)
    {
      ar & timestep_id;
      ar & type_id;
      ar & var_id;   
      ar & txn_id;
      ar & data_type;
      ar & data;
      ar & range_type;
   }
};

struct md_catalog_all_var_attributes_with_var_by_id_args
{
    uint64_t run_id;
    uint64_t timestep_id;
    uint64_t var_id;
    uint64_t txn_id; 

    template <typename Archive>
    void serialize(Archive& ar, const unsigned int ver)
    {
      ar & run_id;
      ar & timestep_id;
      ar & var_id;
      ar & txn_id;
    }
};

struct md_catalog_all_var_attributes_with_var_by_name_ver_args
{
    uint64_t run_id;
    uint64_t timestep_id;
    std::string var_name;
    uint32_t var_version;
    uint64_t txn_id; 

    template <typename Archive>
    void serialize(Archive& ar, const unsigned int ver)
    {
      ar & run_id;
      ar & timestep_id;
      ar & var_name;
      ar & var_version;
      ar & txn_id;
    }
};

struct md_catalog_all_var_attributes_with_var_dims_by_id_args
{
    uint64_t run_id;
    uint64_t timestep_id;
    uint64_t var_id;
    uint32_t num_dims;
    std::vector<md_dim_bounds> dims; 
    uint64_t txn_id; /* if an in-process txn, must provide */

    template <typename Archive>
    void serialize(Archive& ar, const unsigned int ver)
    {
      ar & run_id;
      ar & timestep_id;
      ar & var_id;     
      ar & num_dims;
      ar & dims;
      ar & txn_id;
    }
};

struct md_catalog_all_var_attributes_with_var_dims_by_name_ver_args
{
    uint64_t run_id;
    uint64_t timestep_id;
    std::string var_name;
    uint32_t var_version;
    uint32_t num_dims;
    std::vector<md_dim_bounds> dims; 
    uint64_t txn_id; /* if an in-process txn, must provide */


    template <typename Archive>
    void serialize(Archive& ar, const unsigned int ver)
    {
      ar & run_id;
      ar & timestep_id;
      ar & var_name;
      ar & var_version;     
      ar & num_dims;
      ar & dims;
      ar & txn_id;
    }
};

struct md_catalog_run_args
{
    uint64_t txn_id; /* if an in-process txn, must provide */


    template <typename Archive>
    void serialize(Archive& ar, const unsigned int ver)
    {
      ar & txn_id;
    }
};

struct md_catalog_timestep_args
{
    uint64_t txn_id; /* if an in-process txn, must provide */
    uint64_t run_id;


    template <typename Archive>
    void serialize(Archive& ar, const unsigned int ver)
    {
      ar & txn_id;
      ar & run_id;
    }
};

struct md_catalog_type_args
{
    uint64_t txn_id; /* if an in-process txn, must provide */
    uint64_t run_id;


    template <typename Archive>
    void serialize(Archive& ar, const unsigned int ver)
    {
      ar & txn_id;
      ar & run_id;
    }
};

struct md_catalog_var_args
{
    uint64_t txn_id; /* if an in-process txn, must provide */
    uint64_t run_id;
    uint64_t timestep_id;


    template <typename Archive>
    void serialize(Archive& ar, const unsigned int ver)
    {
      ar & txn_id;
      ar & run_id;
      ar & timestep_id;
    }
};

struct md_create_run_args
{
    /* uint64_t run_id is returned */
    uint64_t job_id;
    std::string name;
    // std::string path;
    /* int active is initialized to 0 (inactive) */
    uint64_t txn_id;
    uint32_t npx;
    uint32_t npy;
    uint32_t npz;
    std::string rank_to_dims_funct;
    std::string objector_funct;

    template <typename Archive>
    void serialize(Archive& ar, const unsigned int ver)
    {
      ar & job_id;
      ar & name;
      // ar & path;
      ar & txn_id;
      ar & npx;
      ar & npy;
      ar & npz;
      ar & rank_to_dims_funct;
      ar & objector_funct;
   }
};

struct md_create_timestep_args
{
    uint64_t timestep_id; 
    uint64_t run_id;
    std::string path;
    /* int active is initialized to 0 (inactive) */
    uint64_t txn_id;
    
    template <typename Archive>
    void serialize(Archive& ar, const unsigned int ver)
    {
      ar & timestep_id;
      ar & run_id;
      ar & path;
      ar & txn_id;
    }
};

struct md_create_type_args
{
    // uint64_t type_id;
    uint64_t run_id;
    std::string name;
    uint32_t version;
    /* int active is initialized to 0 (inactive) */
    uint64_t txn_id;
    
    template <typename Archive>
    void serialize(Archive& ar, const unsigned int ver)
    {
      // ar & type_id;
      ar & run_id;
      ar & name;
      ar & version;
      ar & txn_id;
    }
};


struct md_create_var_args
{
    uint64_t var_id;
    uint64_t run_id;
    uint64_t timestep_id;
    std::string name;
    std::string path;
    uint32_t version;
    char data_size;
    uint32_t num_dims;
    /* int active is initialized to 0 (inactive) */
    uint64_t txn_id;
    std::vector<md_dim_bounds> dims;

    template <typename Archive>
    void serialize(Archive& ar, const unsigned int ver)
    {
      ar & var_id;
      ar & run_id;
      ar & timestep_id;
      ar & name;
      ar & path;
      ar & version;
      ar & data_size;
      ar & num_dims;
      ar & txn_id;
      ar & dims;
    }
};

struct md_delete_run_by_id_args
{
    uint64_t run_id;
    
    template <typename Archive>
    void serialize(Archive& ar, const unsigned int ver)
    {
      ar & run_id;
    }
};

struct md_delete_timestep_by_id_args
{
    uint64_t run_id;
    uint64_t timestep_id;
    
    template <typename Archive>
    void serialize(Archive& ar, const unsigned int ver)
    {
      ar & run_id;
      ar & timestep_id;
    }
};

struct md_delete_type_by_id_args
{
    uint64_t type_id;
    
    template <typename Archive>
    void serialize(Archive& ar, const unsigned int ver)
    {
      ar & type_id;
    }
};

struct md_delete_type_by_name_ver_args
{
    uint64_t run_id;
    std::string name;
    uint32_t version;
    
    template <typename Archive>
    void serialize(Archive& ar, const unsigned int ver)
    {
      ar & run_id;
      ar & name;
      ar & version;

    }
};


struct md_delete_var_by_id_args
{
    uint64_t var_id;
    uint64_t run_id;
    uint64_t timestep_id;

    template <typename Archive>
    void serialize(Archive& ar, const unsigned int ver)
    {
      ar & var_id;
      ar & run_id;
      ar & timestep_id;
    }
};

struct md_delete_var_by_name_path_ver_args
{
    uint64_t run_id;
    uint64_t timestep_id;
    std::string name;
    std::string path;
    uint32_t version;

    template <typename Archive>
    void serialize(Archive& ar, const unsigned int ver)
    {
      ar & run_id;
      ar & timestep_id;
      ar & name;
      ar & path;
      ar & version;
    }
};

struct md_insert_run_attribute_args
{
    uint64_t type_id;
    /* int active is initialized to 0 (inactive) */
    uint64_t txn_id;
    attr_data_type data_type;
    // std::xstring data;
    std::string data;

    template <typename Archive>
    void serialize(Archive& ar, const unsigned int ver)
    {
      ar & type_id;
      ar & txn_id;
      ar & data_type;
      ar & data;
    }
};

struct md_insert_timestep_attribute_args
{
    uint64_t timestep_id;
    uint64_t type_id;
    /* int active is initialized to 0 (inactive) */
    uint64_t txn_id;
    attr_data_type data_type;
    // std::xstring data;
    std::string data;

    template <typename Archive>
    void serialize(Archive& ar, const unsigned int ver)
    {
      ar & timestep_id;
      ar & type_id;
      ar & txn_id;
      ar & data_type;
      ar & data;
    }
};

struct md_insert_var_attribute_by_dims_args
{
    uint64_t timestep_id;
    uint64_t type_id;
    uint64_t var_id;
    /* int active is initialized to 0 (inactive) */
    uint64_t txn_id;
    uint32_t num_dims;
    std::vector<md_dim_bounds> dims;
    attr_data_type data_type;
    // std::xstring data;
    std::string data;

    template <typename Archive>
    void serialize(Archive& ar, const unsigned int ver)
    {
      ar & timestep_id;
      ar & type_id;
      ar & var_id;
      ar & txn_id;
      ar & num_dims;
      ar & dims;
      ar & data_type;
      ar & data;
    }
};

struct md_processing_args
{
    uint64_t txn_id; /* if an in-process txn, must provide */   
    md_catalog_type catalog_type; 

    template <typename Archive>
    void serialize(Archive& ar, const unsigned int ver)
    {
      ar & txn_id;
      ar & catalog_type;
    }
};



struct md_catalog_all_var_attributes_with_var_substr_args
{
    uint64_t run_id;
    uint64_t timestep_id;
    std::string var_name_substr;
    uint64_t txn_id; 

    template <typename Archive>
    void serialize(Archive& ar, const unsigned int ver)
    {
      ar & run_id;
      ar & timestep_id;
      ar & var_name_substr;
      ar & txn_id;
    }
};

struct md_catalog_all_timesteps_with_var_attributes_with_type_var_substr_dims_args
{
    uint64_t run_id;
    uint64_t type_id;
    std::string var_name_substr;
    uint64_t txn_id; 
    uint32_t num_dims;
    std::vector<md_dim_bounds> dims;  

    template <typename Archive>
    void serialize(Archive& ar, const unsigned int ver)
    {
      ar & run_id;
      ar & type_id;
      ar & var_name_substr;
      ar & txn_id;
      ar & num_dims;
      ar & dims;
    }
};

struct md_catalog_all_timesteps_with_var_attributes_with_type_var_substr_dims_range_args
{
    uint64_t run_id;
    uint64_t type_id;
    std::string var_name_substr;
    uint64_t txn_id; 
    uint32_t num_dims;
    std::vector<md_dim_bounds> dims;
    attr_data_type data_type;
    std::string data;
    data_range_type range_type;

    template <typename Archive>
    void serialize(Archive& ar, const unsigned int ver)
    {
      ar & run_id;
      ar & type_id;
      ar & var_name_substr;
      ar & txn_id;
      ar & num_dims;
      ar & dims;
      ar & data_type;
      ar & data;
      ar & range_type;
    }
};

struct md_catalog_all_timesteps_with_var_attributes_with_type_var_substr_args
{
    uint64_t run_id;
    uint64_t type_id;
    std::string var_name_substr;
    uint64_t txn_id; 

    template <typename Archive>
    void serialize(Archive& ar, const unsigned int ver)
    {
      ar & run_id;
      ar & type_id;
      ar & var_name_substr;
      ar & txn_id;
    }
};

struct md_catalog_all_timesteps_with_var_attributes_with_type_var_substr_range_args
{
    uint64_t run_id;
    uint64_t type_id;
    std::string var_name_substr;
    uint64_t txn_id; 
    attr_data_type data_type;
    std::string data;
    data_range_type range_type;

    template <typename Archive>
    void serialize(Archive& ar, const unsigned int ver)
    {
      ar & run_id;
      ar & type_id;
      ar & var_name_substr;
      ar & txn_id;
      ar & data_type;
      ar & data;
      ar & range_type;
    }
};

struct md_catalog_all_timesteps_with_var_substr_args
{
    uint64_t run_id;
    std::string var_name_substr;
    uint64_t txn_id; 

    template <typename Archive>
    void serialize(Archive& ar, const unsigned int ver)
    {
      ar & run_id;
      ar & var_name_substr;
      ar & txn_id;
    }
};

struct md_catalog_all_types_with_var_attributes_with_var_substr_dims_in_timestep_args
{
    uint64_t run_id;
    uint64_t timestep_id;
    std::string var_name_substr;
    uint64_t txn_id; 
    uint32_t num_dims;
    std::vector<md_dim_bounds> dims;  

    template <typename Archive>
    void serialize(Archive& ar, const unsigned int ver)
    {
      ar & run_id;
      ar & timestep_id;
      ar & var_name_substr;
      ar & txn_id;
      ar & num_dims;
      ar & dims;
    }
};

struct md_catalog_all_types_with_var_attributes_with_var_substr_in_timestep_args
{
    uint64_t run_id;
    uint64_t timestep_id;
    std::string var_name_substr;
    uint64_t txn_id; 

    template <typename Archive>
    void serialize(Archive& ar, const unsigned int ver)
    {
      ar & run_id;
      ar & timestep_id;
      ar & var_name_substr;
      ar & txn_id;
    }
};

struct md_catalog_all_var_attributes_with_type_var_substr_dims_range_args
{
    uint64_t timestep_id;
    uint64_t type_id;
    std::string var_name_substr;
    uint64_t txn_id; /* if an in-process txn, must provide */
    uint32_t num_dims;
    std::vector<md_dim_bounds> dims;
    attr_data_type data_type;
    std::string data;
    data_range_type range_type;

    template <typename Archive>
    void serialize(Archive& ar, const unsigned int ver)
    {
      ar & timestep_id;
      ar & type_id;
      ar & var_name_substr;
      ar & num_dims;
      ar & dims;   
      ar & txn_id;
      ar & data_type;
      ar & data;
      ar & range_type;
   }
};

struct md_catalog_all_var_attributes_with_type_var_substr_range_args
{
    uint64_t timestep_id;
    uint64_t type_id;
    std::string var_name_substr;
    uint64_t txn_id; /* if an in-process txn, must provide */
    attr_data_type data_type;
    std::string data;
    data_range_type range_type;

    template <typename Archive>
    void serialize(Archive& ar, const unsigned int ver)
    {
      ar & timestep_id;
      ar & type_id;
      ar & var_name_substr;   
      ar & txn_id;
      ar & data_type;
      ar & data;
      ar & range_type;
   }
};

struct md_catalog_all_var_attributes_with_type_var_substr_args
{
    uint64_t run_id;
    uint64_t timestep_id;
    uint64_t type_id;
    std::string var_name_substr;
    uint64_t txn_id; 

    template <typename Archive>
    void serialize(Archive& ar, const unsigned int ver)
    {
      ar & run_id;
      ar & timestep_id;
      ar & type_id;
      ar & var_name_substr;
      ar & txn_id;
    }
};

struct md_catalog_all_var_attributes_with_type_var_substr_dims_args
{
    uint64_t run_id;
    uint64_t timestep_id;
    uint64_t type_id;
    std::string var_name_substr;
    uint32_t num_dims;
    std::vector<md_dim_bounds> dims; 
    uint64_t txn_id; /* if an in-process txn, must provide */


    template <typename Archive>
    void serialize(Archive& ar, const unsigned int ver)
    {
      ar & run_id;
      ar & timestep_id;
      ar & type_id;
      ar & var_name_substr;     
      ar & num_dims;
      ar & dims;
      ar & txn_id;
    }
};

struct md_catalog_all_var_attributes_with_var_substr_dims_args
{
    uint64_t run_id;
    uint64_t timestep_id;
    std::string var_name_substr;
    uint32_t num_dims;
    std::vector<md_dim_bounds> dims; 
    uint64_t txn_id; /* if an in-process txn, must provide */


    template <typename Archive>
    void serialize(Archive& ar, const unsigned int ver)
    {
      ar & run_id;
      ar & timestep_id;
      ar & var_name_substr;     
      ar & num_dims;
      ar & dims;
      ar & txn_id;
    }
};

struct md_delete_all_vars_with_substr_args
{
    uint64_t run_id;
    uint64_t timestep_id;
    std::string var_name_substr;

    template <typename Archive>
    void serialize(Archive& ar, const unsigned int ver)
    {
      ar & run_id;
      ar & timestep_id;
      ar & var_name_substr;
    }
};


#endif //MYMETADATA_ARGS_H
