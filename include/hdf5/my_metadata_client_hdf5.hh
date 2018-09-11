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


#ifndef MYMETADATACLIENTHDF5_HH
#define MYMETADATACLIENTHDF5_HH

#include <hdf5.h>
#include <hdf5_hl.h>
#include <assert.h> //needed for assert
#include <vector>
#include <md_timing_constants_hdf5.hh>

#ifndef RC_ENUM
#define RC_ENUM
enum RC
{
    RC_OK = 0,
    RC_ERR = -1
};
#endif //RC_ENUM

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
	uint32_t num_dims;
    uint64_t d0_min;
    uint64_t d0_max;
    uint64_t d1_min;
    uint64_t d1_max;
    uint64_t d2_min;
    uint64_t d2_max;

    md_dim_bounds() {

    }

    md_dim_bounds(uint32_t nm_dims, uint64_t d0_mn, uint64_t d0_mx,
    	uint64_t d1_mn, uint64_t d1_mx, uint64_t d2_mn, uint64_t d2_mx ) 
    {
    	num_dims = nm_dims;
    	d0_min = d0_mn;
    	d0_max = d0_mx;
    	d1_min = d1_mn;
    	d1_max = d1_mx;
    	d2_min = d2_mn;
    	d2_max = d2_mx;
    }
};

struct md_timestep_entry
{
	hid_t file_id;
	hid_t timestep_attr_table_id;
	hid_t var_attr_table_id;	    
};

struct md_var_entry
{
    uint64_t var_indx;
    std::string var_name;
    std::string var_path;
    uint32_t num_dims;
    hsize_t dims[3];
	size_t data_size;
	H5T_class_t datatype_class;
	bool chunked = false;
    hsize_t chunk_dims[3];
    // hid_t datatype;
    // hid_t property_list;
    // hid_t data space;

    template <typename Archive>
    void serialize(Archive& ar, const unsigned int ver)
    {
    ar & var_indx;
    ar & var_name;
    ar & var_path;
    ar & num_dims;
    ar & dims;
	ar & data_size;
	ar & datatype_class;
	ar & chunked;
    ar & chunk_dims;
    // hid_t datatype;
    // hid_t property_list;
    // hid_t data space;
    }
};

struct non_var_attribute_c_str
{
    // uint64_t var_attribute_str_id;
    // uint64_t type_id;
    // uint32_t active;
    // uint64_t txn_id;
    char *type_name;
    char *data;

};

struct non_var_attribute_str //reminder - 
{
    // uint64_t var_attribute_str_id;
    // uint64_t type_id;
    // uint32_t active;
    // uint64_t txn_id; 
    std::string type_name;  
    std::string data;

    non_var_attribute_str ()  
    {
	
    }

    non_var_attribute_str (non_var_attribute_c_str attr)  
    {
    	type_name = attr.type_name;
    	data = attr.data;  	
    }

    non_var_attribute_str (std::string t_name, std::string value)  
    {
    	type_name = t_name;
    	data = value;  	
    }


    template <typename Archive>
    void serialize(Archive& ar, const unsigned int ver)
    {
   	ar & type_name;  
    ar & data;
    // ar & var_attribute_str_id;
    // ar & type_id;
    // ar & active;
    // ar & txn_id; 
    }
};



struct var_attribute_c_str
{
    // uint64_t var_attribute_str_id;
    // uint64_t type_id;
    char *type_name;
    char *var_name;
    uint32_t num_dims;
    uint64_t d0_min;
    uint64_t d0_max;
    uint64_t d1_min;
    uint64_t d1_max;
    uint64_t d2_min;
    uint64_t d2_max;
    char *data;

};

struct var_attribute_str //reminder - 
{
    // uint64_t var_attribute_str_id;
    std::string type_name; 
    // uint64_t type_id;
    std::string var_name; 
    uint32_t num_dims;
    // uint32_t active;
    // uint64_t txn_id; 
    uint64_t d0_min;
    uint64_t d0_max;
    uint64_t d1_min;
    uint64_t d1_max;
    uint64_t d2_min;
    uint64_t d2_max;
    std::string data;

    // var_attribute_str () {

    // }

	var_attribute_str (var_attribute_c_str attr)  
    {
    	type_name = attr.type_name;
    	var_name = attr.var_name;
    	num_dims = attr.num_dims;
    	d0_min = attr.d0_min;
    	d0_max = attr.d0_max;
    	d1_min = attr.d1_min;
    	d1_max = attr.d1_max;
    	d2_min = attr.d2_min;
    	d2_max = attr.d2_max;  
    	data = attr.data;  	
    }


    var_attribute_str (std::string t_name, std::string v_name, uint32_t nm_dims,
    		uint64_t d0_mn, uint64_t d0_mx, uint64_t d1_mn, uint64_t d1_mx,
    		uint64_t d2_mn, uint64_t d2_mx, std::string value
    		)  
    {
    	type_name = t_name;
    	var_name = v_name;
    	num_dims = nm_dims;
    	d0_min = d0_mn;
    	d0_max = d0_mx;
    	d1_min = d1_mn;
    	d1_max = d1_mx;
    	d2_min = d2_mn;
    	d2_max = d2_mx;  
    	data = value;  	
    }

    var_attribute_str (std::string t_name, std::string v_name, md_dim_bounds dims, std::string value)  
    {
    	type_name = t_name;
    	var_name = v_name;
    	num_dims = dims.num_dims;
    	if(dims.num_dims >= 1) {
	    	d0_min = dims.d0_min;
	    	d0_max = dims.d0_max;
	    	if(dims.num_dims >= 2) {
		    	d1_min = dims.d1_min;
		    	d1_max = dims.d1_max;
		    	if(dims.num_dims >= 3) {
			    	d2_min = dims.d2_min;
			    	d2_max = dims.d2_max;
			    }
			}
		}  
    	data = value;  	
    }

    var_attribute_str () {

    }

    template <typename Archive>
    void serialize(Archive& ar, const unsigned int ver)
    {
    // ar & attribute_str_id;
    // ar & type_id;
    ar & type_name;
    ar & var_name;
    // ar & var_id;
    ar & num_dims;
    // ar & active;
    // ar & txn_id;
    ar & d0_min;
    ar & d0_max;
    ar & d1_min;
    ar & d1_max;
    ar & d2_min;
    ar & d2_max;
    ar & data;
    }

};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool dims_overlap(var_attribute_str attr, md_dim_bounds query_dims);

std::string to_upper (std::string val);

void make_attr_data (int test_int, std::string &serial_str);
void make_attr_data (std::string test_string, int test_int, std::string &serial_str);
// void print_attr_data (std::string attr_data );
void print_attr_data(var_attribute_str attr);
void print_attr_data(non_var_attribute_str attr);
void print_var (md_var_entry var);
void print_var_catalog (const std::vector<md_var_entry> &entries);
void print_dataspace_dims(uint32_t num_dims, hsize_t *dims);

std::string print_datatype_class(H5T_class_t t_class);

// void packet_table_print (hid_t attr_table_id);
void print_type_catalog(std::vector<std::string> type_names);


void print_attr_dims(var_attribute_str attr);
void print_md_dim_bounds( md_dim_bounds dims);

void print_var_attribute (var_attribute_str attr);

void print_var_attribute_list (std::vector<var_attribute_str> attrs);

void print_timestep_attribute_list (std::vector<non_var_attribute_str> attrs);

void print_timestep_catalog(std::vector<std::string> timestep_file_names);

void print_run_attribute_list (std::vector<non_var_attribute_str> attrs);


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void metadata_catalog_var (hid_t file_id, std::vector<md_var_entry> &vars);

void metadata_catalog_all_types_with_var_attributes_in_timestep (hid_t var_attr_table_id
                      	  ,std::vector<std::string> &type_names
                     	  );

void metadata_catalog_all_types_with_var_attributes_with_var_in_timestep (hid_t var_attr_table_id
					  	  ,std::string var_name
                      	  ,std::vector<std::string> &type_names
                     	  );

void metadata_catalog_all_types_with_var_attributes_with_var_dims_in_timestep (hid_t var_attr_table_id
					  	  ,std::string var_name
					  	  ,md_dim_bounds query_dims
                      	  ,std::vector<std::string> &type_names
                     	  );

void metadata_insert_var_attribute_batch (hid_t var_attr_table_id, const std::vector<var_attribute_str> &attrs );
void metadata_insert_var_attribute (hid_t var_attr_table_id, var_attribute_str attr);

// void metadata_create_timestep (std::string run_name, std::string job_id, uint64_t timestep_id, 
// 						md_timestep_entry &timestep);

void metadata_create_timestep(std::string run_name, std::string job_id, uint64_t timestep_id 
						  ,hid_t run_file_id
						  ,md_timestep_entry &timestep
						  );


void metadata_create_and_close_var(uint32_t num_dims, hsize_t *var_dims,
						  hid_t file_id, const std::string &VARNAME);

void metadata_create_and_close_chunked_var(uint32_t num_dims, hsize_t *var_dims, hsize_t *chunk_dims,
						  hid_t file_id, const std::string var_name
						  );

void metadata_catalog_all_var_attributes_with_dims (hid_t var_attr_table_id
					  	  ,md_dim_bounds query_dims
                      	  ,std::vector<var_attribute_str> &attrs
					  	  );

void metadata_catalog_all_var_attributes (hid_t var_attr_table_id, std::vector<var_attribute_str> &attrs);


void metadata_catalog_all_var_attributes_with_type (hid_t var_attr_table_id
						  ,std::string type_name
	                      ,std::vector<var_attribute_str> &attrs
						  );

void metadata_catalog_all_var_attributes_with_type_dims (hid_t var_attr_table_id
						  ,std::string type_name
					  	  ,md_dim_bounds query_dims
	                      ,std::vector<var_attribute_str> &attrs
						  );

void metadata_catalog_all_var_attributes_with_var (hid_t var_attr_table_id
						  ,std::string var_name
	                      ,std::vector<var_attribute_str> &attrs
						  );

void metadata_catalog_all_var_attributes_with_var_dims (hid_t var_attr_table_id
						  ,std::string var_name
					  	  ,md_dim_bounds query_dims
	                      ,std::vector<var_attribute_str> &attrs
						  );

void metadata_catalog_all_var_attributes_with_type_var (hid_t var_attr_table_id
						  ,std::string type_name
						  ,std::string var_name
	                      ,std::vector<var_attribute_str> &attrs
						  );

void metadata_catalog_all_var_attributes_with_type_var_dims (hid_t var_attr_table_id
						  ,std::string type_name
						  ,std::string var_name
						  ,md_dim_bounds query_dims
	                      ,std::vector<var_attribute_str> &attrs
						  );

void metadata_catalog_all_var_attributes_with_var_substr (hid_t var_attr_table_id
						  ,std::string var_name_substr
	                      ,std::vector<var_attribute_str> &attrs
						  ); 


void metadata_catalog_all_var_attributes_with_var_substr_dims (hid_t var_attr_table_id
						  ,std::string var_name_substr
						  ,md_dim_bounds query_dims
	                      ,std::vector<var_attribute_str> &attrs
						  ); 

void metadata_catalog_all_var_attributes_with_type_var_substr (hid_t var_attr_table_id
						  ,std::string type_name
						  ,std::string var_name_substr
	                      ,std::vector<var_attribute_str> &attrs
						  ); 

void metadata_catalog_all_var_attributes_with_type_var_substr_dims (hid_t var_attr_table_id
						  ,std::string type_name
						  ,std::string var_name_substr
						  ,md_dim_bounds query_dims
	                      ,std::vector<var_attribute_str> &attrs
						  ); 

void metadata_catalog_all_types_with_var_attributes_with_var_substr_in_timestep (hid_t var_attr_table_id
					  	  ,std::string var_name_substr
                      	  ,std::vector<std::string> &type_names
                     	  );

void metadata_catalog_all_types_with_var_attributes_with_var_substr_dims_in_timestep (hid_t var_attr_table_id
					  	  ,std::string var_name_substr
					  	  ,md_dim_bounds query_dims
                      	  ,std::vector<std::string> &type_names
                     	  );

void metadata_insert_timestep_attribute_batch(hid_t timestep_attr_table_id, const std::vector<non_var_attribute_str> &attrs );

void metadata_insert_timestep_attribute(hid_t timestep_attr_table_id, non_var_attribute_str attr); 

void metadata_catalog_all_timestep_attributes (hid_t timestep_attr_table_id
	                      ,std::vector<non_var_attribute_str> &attrs
						  ,bool is_timestep=true);

void metadata_catalog_all_timestep_attributes_with_type (hid_t timestep_attr_table_id
						  ,std::string type_name
	                      ,std::vector<non_var_attribute_str> &attrs
						  ,bool is_timestep=true);

void metadata_create_run (std::string run_name, std::string job_id, hid_t &file_id, hid_t &run_attr_table_id); 

void metadata_catalog_timestep (hid_t run_file_id, std::vector<std::string> &timestep_file_names);


// void metadata_catalog_all_timesteps_with_var_substr(hid_t run_file_id
// 						  ,std::string full_var_name_substr
// 						  ,std::vector<std::string> &matching_timestep_entries
// 						  );

void metadata_catalog_all_timesteps_with_var_substr(std::vector<std::string> all_timestep_file_names
						  ,std::string var_name_substr
						  ,std::vector<std::string> &matching_timesteps
						  );

void metadata_catalog_all_timesteps_with_var_attributes_with_type_var_substr(std::vector<std::string> all_timestep_file_names
						  ,std::string type_name
						  ,std::string var_name_substr
						  ,std::vector<std::string> &matching_timesteps
						  );

void metadata_catalog_all_timesteps_with_var_attributes_with_type_var_substr_dims(std::vector<std::string> all_timestep_file_names
						  ,std::string type_name
						  ,std::string var_name_substr
						  ,md_dim_bounds dims
						  ,std::vector<std::string> &matching_timesteps
						  );


void metadata_catalog_all_timesteps_with_var (std::vector<std::string> all_timestep_file_names
						  ,std::string var_name
						  ,std::vector<std::string> &matching_timesteps
						  );

void metadata_catalog_all_timesteps_with_var_attributes_with_type_var(std::vector<std::string> all_timestep_file_names
						  ,std::string type_name
						  ,std::string var_name
						  ,std::vector<std::string> &matching_timesteps
						  );

void metadata_catalog_all_timesteps_with_var_attributes_with_type_var_dims(std::vector<std::string> all_timestep_file_names
						  ,std::string type_name
						  ,std::string var_name
						  ,md_dim_bounds query_dims
						  ,std::vector<std::string> &matching_timesteps
						  );

void metadata_insert_run_attribute_batch(hid_t run_attr_table_id, const std::vector<non_var_attribute_str> &attrs );

void metadata_insert_run_attribute(hid_t run_attr_table_id, non_var_attribute_str attr);


void metadata_catalog_all_run_attributes (hid_t timestep_attr_table_id
	                      ,std::vector<non_var_attribute_str> &attrs
						  );

void metadata_catalog_all_run_attributes_with_type (hid_t timestep_attr_table_id
						  ,std::string type_name
	                      ,std::vector<non_var_attribute_str> &attrs
						  );



//////////////////////////

void open_timestep_file_and_attr_tables_for_read(std::string full_timestep_file_name, 
									md_timestep_entry &timestep
									);
void open_timestep_file_and_attr_tables_for_write(std::string full_timestep_file_name, 
									md_timestep_entry &timestep
									);
void open_timestep_file_and_attr_tables_for_read(std::string run_name, std::string job_id, uint64_t timestep_id, 
									md_timestep_entry &timestep
									);
void open_timestep_file_and_attr_tables_for_write(std::string run_name, std::string job_id, uint64_t timestep_id, 
									md_timestep_entry &timestep
									);
void open_timestep_file_and_var_attr_table_for_read(std::string run_name, std::string job_id, uint64_t timestep_id, 
									hid_t &file_id, hid_t &var_attr_table_id
									);
void open_timestep_file_and_var_attr_table_for_read(std::string full_timestep_file_name, 
									hid_t &timestep_file_id, hid_t &var_attr_table_id
									);
void open_timestep_file_and_var_attr_table_for_write(std::string run_name, std::string job_id, uint64_t timestep_id, 
									hid_t &file_id, hid_t &var_attr_table_id
									);
void open_timestep_file_and_var_attr_table_for_write(std::string full_timestep_file_name, 
									hid_t &timestep_file_id, hid_t &var_attr_table_id
									);
void open_timestep_file_and_timestep_attr_table_for_write(std::string full_timestep_file_name, 
									hid_t &timestep_file_id, hid_t &timestep_attr_table_id
									);
void open_timestep_file_and_timestep_attr_table_for_write(std::string run_name, std::string job_id, uint64_t timestep_id, 
									hid_t &timestep_file_id, hid_t &timestep_attr_table_id);
void open_timestep_file_and_timestep_attr_table_for_read(std::string full_timestep_file_name, 
									hid_t &timestep_file_id, hid_t &timestep_attr_table_id
									);
void open_timestep_file_and_timestep_attr_table_for_read(std::string run_name, std::string job_id, uint64_t timestep_id, 
									hid_t &timestep_file_id, hid_t &timestep_attr_table_id);

void open_file_for_read(std::string full_file_name, hid_t &file_id);
void open_file_for_write(std::string full_file_name, hid_t &file_id);
// void open_timestep_file(std::string run_name, std::string job_id, uint64_t timestep_id, hid_t &timestep_file_id);
void open_var_attr_table(hid_t timestep_file_id, hid_t &var_attr_table_id);
void open_timestep_attr_table(hid_t timestep_file_id, hid_t &timestep_attr_table_id);
void open_var(hid_t timestep_file_id, std::string var_name, uint32_t var_version, hid_t &var_id);
void close_timestep(md_timestep_entry timestep);
void close_timestep_file(hid_t file_id);
void open_run_for_write (std::string run_name, std::string job_id, hid_t &run_file_id);
void open_run_for_read (std::string run_name, std::string job_id, hid_t &run_file_id);
void open_run_and_attr_table_for_write (std::string run_name, std::string job_id, hid_t &run_file_id, hid_t &run_attr_table_id);
void open_run_and_attr_table_for_read (std::string run_name, std::string job_id, hid_t &run_file_id, hid_t &run_attr_table_id);
void open_run_attr_table (hid_t run_file_id, hid_t &run_attr_table_id);
void close_run_and_attr_table (hid_t run_file_id, hid_t run_attr_table_id);
void close_run (hid_t run_file_id);


void close_timestep_file_and_attr_table(hid_t file_id, hid_t attr_table_id);

void close_timestep_file_and_timestep_attr_table(hid_t file_id, hid_t timestep_attr_table_id);

#endif //MYMETADATACLIENTHDF5_HH