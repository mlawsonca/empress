


#ifndef MYTESTCLIENTHDF5_HH
#define MYTESTCLIENTHDF5_HH

#include <hdf5.h>
#include <hdf5_hl.h>
#include <assert.h> //needed for assert
#include <vector>

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
};

struct timestep_entry
{
	hid_t file_id;
	hid_t timestep_attr_table_id;
	hid_t var_attr_table_id;	    
};

struct var_entry
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
};

struct non_var_attribute_str //reminder - 
{
    // uint64_t var_attribute_str_id;
    // uint64_t type_id;
    // uint32_t active;
    // uint64_t txn_id; 
    std::string type_name;  
    std::string data;

    non_var_attribute_str (std::string t_name, std::string value)  
    {
    	type_name = t_name;
    	data = value;  	
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
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string to_upper (std::string val);

void make_attr_data (int test_int, std::string &serial_str);
void make_attr_data (std::string test_string, int test_int, std::string &serial_str);
void print_attr_data (std::string attr_data );
void print_var (var_entry var);
void print_var_catalog (const std::vector<var_entry> &entries);
void print_dataspace_dims(uint32_t num_dims, hsize_t *dims);

std::string print_datatype_class(H5T_class_t t_class);

// void packet_table_print (hid_t attr_table_id);
void print_type_catalog(std::vector<std::string> type_names);


void print_attr_dims(var_attribute_c_str attr);
void print_md_dim_bounds( md_dim_bounds dims);

void print_var_attribute (var_attribute_c_str attr);

void print_var_attribute_list (std::vector<var_attribute_c_str> attrs);

void print_timestep_attribute_list (std::vector<non_var_attribute_c_str> attrs);

void print_timestep_catalog(std::vector<std::string> timestep_file_names);

void print_run_attribute_list (std::vector<non_var_attribute_c_str> attrs);


void create_timestep(std::string run_name, std::string job_id, uint64_t timestep_id, hid_t run_file_id,
					std::string TYPENAME_0, std::string TYPENAME_1,
					std::string VARNAME_0, std::string VARNAME_1);

// void open_timestep_file_and_attr_tables(std::string run_name, std::string job_id, uint64_t timestep_id, 
// 									hid_t &file_id, hid_t &timestep_attr_table_id, hid_t &var_attr_table_id
// 									);

void open_timestep_file_and_attr_tables(std::string run_name, std::string job_id, uint64_t timestep_id, 
									timestep_entry &timestep
									);

void open_timestep_file_and_var_attr_table(std::string run_name, std::string job_id, uint64_t timestep_id, 
									hid_t &file_id, hid_t &var_attr_table_id
									);

void catalog_all_var_attributes ( hid_t var_attr_table_id, uint64_t timestep_id, md_dim_bounds query_dims );

void catalog_all_var_attributes_with_type ( hid_t var_attr_table_id, uint64_t timestep_id, std::string type_name, md_dim_bounds query_dims );

void catalog_all_var_attributes_with_var ( hid_t var_attr_table_id, uint64_t timestep_id, std::string var_name, md_dim_bounds query_dims );

void catalog_all_var_attributes_with_type_var ( hid_t var_attr_table_id, uint64_t timestep_id, std::string type_name, std::string var_name,
		 								 md_dim_bounds query_dims );

void catalog_all_var_attributes_with_var_substr ( hid_t file_id, hid_t var_attr_table_id, uint64_t timestep_id, 
					md_dim_bounds query_dims );

void catalog_all_var_attributes_with_type_var_substr ( hid_t var_attr_table_id, uint64_t timestep_id, 
					std::string type_name, md_dim_bounds query_dims );

void catalog_all_types_substr ( hid_t var_attr_table_id, uint64_t timestep_id, md_dim_bounds query_dims );

// int delete_var_substr (hid_t file_id, hid_t var_attr_table_id, uint64_t timestep_id);

void create_timestep_attrs(hid_t timestep_attr_table0, hid_t timestep_attr_table1,
                    std::string type0_name, std::string type1_name );

void catalog_all_timestep_attributes ( hid_t timestep_attr_table_id, uint64_t timestep_id, std::string type_name);

void catalog_all_timesteps_substr ( hid_t run_file_id, std::string type_name, md_dim_bounds query_dims );

void create_new_timesteps(std::string run_name, std::string job_id, uint64_t timestep_id, hid_t run_file_id, std::string type0_name, 
					std::string type1_name, std::string var0_name, std::string var1_name, hid_t timestep20_var_attr_table_id);

void catalog_all_timesteps ( hid_t run_file_id, std::string type_name, std::string var_name, md_dim_bounds query_dims );

// void close_timestep(hid_t timestep_file_id, hid_t timestep_attr_table_id_id, hid_t var_attr_table_id);
void close_timestep(timestep_entry timestep);

void create_run_attrs(hid_t run_attr_table_id, std::string type0_name, std::string type1_name );

int catalog_all_run_attributes ( hid_t run_attr_table_id, std::string type_name );

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void metadata_catalog_var (hid_t file_id, std::vector<var_entry> &vars);

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

void metadata_insert_var_attribute_by_dims_batch (hid_t var_attr_table_id, const std::vector<var_attribute_str> &attrs );
void metadata_insert_var_attribute_by_dims (hid_t var_attr_table_id, var_attribute_str attr);

// void metadata_create_timestep (std::string run_name, std::string job_id, uint64_t timestep_id, 
// 						timestep_entry &timestep);

void metadata_create_timestep(std::string run_name, std::string job_id, uint64_t timestep_id 
						  ,hid_t run_file_id
						  ,timestep_entry &timestep
						  );


void metadata_create_var(uint32_t num_dims, hsize_t *var_dims,
						  hid_t file_id, const std::string &VARNAME);

void metadata_create_var_and_chunk(uint32_t num_dims, hsize_t *var_dims, hsize_t *chunk_dims,
						  hid_t file_id, const std::string var_name
						  );

void metadata_catalog_all_var_attributes_with_dims (hid_t var_attr_table_id
					  	  ,md_dim_bounds query_dims
                      	  ,std::vector<var_attribute_c_str> &attrs
					  	  );

void metadata_catalog_all_var_attributes (hid_t var_attr_table_id, std::vector<var_attribute_c_str> &attrs);


void metadata_catalog_all_var_attributes_with_type_by_name (hid_t var_attr_table_id
						  ,std::string type_name
	                      ,std::vector<var_attribute_c_str> &attrs
						  );

void metadata_catalog_all_var_attributes_with_type_dims_by_name (hid_t var_attr_table_id
						  ,std::string type_name
					  	  ,md_dim_bounds query_dims
	                      ,std::vector<var_attribute_c_str> &attrs
						  );

void metadata_catalog_all_var_attributes_with_var_by_name (hid_t var_attr_table_id
						  ,std::string var_name
	                      ,std::vector<var_attribute_c_str> &attrs
						  );

void metadata_catalog_all_var_attributes_with_var_dims_by_name (hid_t var_attr_table_id
						  ,std::string var_name
					  	  ,md_dim_bounds query_dims
	                      ,std::vector<var_attribute_c_str> &attrs
						  );

void metadata_catalog_all_var_attributes_with_type_var_by_name (hid_t var_attr_table_id
						  ,std::string type_name
						  ,std::string var_name
	                      ,std::vector<var_attribute_c_str> &attrs
						  );

void metadata_catalog_all_var_attributes_with_type_var_dims_by_name (hid_t var_attr_table_id
						  ,std::string type_name
						  ,std::string var_name
						  ,md_dim_bounds query_dims
	                      ,std::vector<var_attribute_c_str> &attrs
						  );

void metadata_catalog_all_var_attributes_with_var_substr (hid_t var_attr_table_id
						  ,std::string var_name_substr
	                      ,std::vector<var_attribute_c_str> &attrs
						  ); 


void metadata_catalog_all_var_attributes_with_var_substr_dims (hid_t var_attr_table_id
						  ,std::string var_name_substr
						  ,md_dim_bounds query_dims
	                      ,std::vector<var_attribute_c_str> &attrs
						  ); 

void metadata_catalog_all_var_attributes_with_type_var_substr (hid_t var_attr_table_id
						  ,std::string type_name
						  ,std::string var_name_substr
	                      ,std::vector<var_attribute_c_str> &attrs
						  ); 

void metadata_catalog_all_var_attributes_with_type_var_substr_dims (hid_t var_attr_table_id
						  ,std::string type_name
						  ,std::string var_name_substr
						  ,md_dim_bounds query_dims
	                      ,std::vector<var_attribute_c_str> &attrs
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
	                      ,std::vector<non_var_attribute_c_str> &attrs
						  );

void metadata_catalog_all_timestep_attributes_with_type (hid_t timestep_attr_table_id
						  ,std::string type_name
	                      ,std::vector<non_var_attribute_c_str> &attrs
						  );

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
	                      ,std::vector<non_var_attribute_c_str> &attrs
						  );

void metadata_catalog_all_run_attributes_with_type (hid_t timestep_attr_table_id
						  ,std::string type_name
	                      ,std::vector<non_var_attribute_c_str> &attrs
						  );

void open_var_attr_table(hid_t timestep_file_id, hid_t &var_attr_table_id);
void open_timestep_attr_table(hid_t timestep_file_id, hid_t &timestep_attr_table_id);
void open_var(hid_t timestep_file_id, std::string var_name, uint32_t var_version, hid_t &var_id);

#endif //MYTESTCLIENTHDF5_HH