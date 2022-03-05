

#ifndef SQLITE_OBJECTOR_COMAPRISON_HH
#define SQLITE_OBJECTOR_COMAPRISON_HH

#include <iostream>
#include <vector>

struct debugLog {
  private:
    bool on;

  public:
  debugLog(bool turn_on) {
    on = turn_on;
  }

  template<typename T> debugLog& operator << (const T& x) {
   if(on ) {
      std::cout << x;
    }
    return *this;
  }
  debugLog& operator<<(std::ostream& (*manipulator)(std::ostream&)) {
   if(on ) {
      std::cout << manipulator;
    }
    return *this;
  }
};

#ifndef RC_ENUM
#define RC_ENUM
enum RC
{
    RC_OK = 0,
    RC_ERR = -1
};
#endif //RC_ENUM


struct md_dim_bounds
{
    uint64_t min;
    uint64_t max;

};

struct md_catalog_object_entry
{
    uint64_t object_id;
    // uint64_t run_id;
    uint64_t timestep_id;
    uint64_t var_id;
    // uint32_t active;
    // uint64_t txn_id; 
    uint32_t num_dims;
    std::vector<md_dim_bounds> dims;    
    std::string object_name;

};

struct md_catalog_all_objects_with_var_dims_args
{
    // uint64_t run_id;
    uint64_t timestep_id;
    uint64_t var_id;
    // uint64_t txn_id;  //if an in-process txn, must provide 
    // uint32_t num_dims;
    std::vector<md_dim_bounds> dims;  
};


struct md_insert_object_args
{
	// uint64_t run_id;
    uint64_t timestep_id;
    uint64_t var_id;
    /* int active is initialized to 0 (inactive) */
    // uint64_t txn_id;
    // uint32_t num_dims;
    std::vector<md_dim_bounds> dims;
    std::string object_name;

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

};

struct md_catalog_timestep_entry
{
    uint64_t timestep_id;
    uint64_t run_id;
    std::string path;
    uint32_t active;
    uint64_t txn_id;

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

};

static int metadata_database_init ();
static void get_obj_lengths(uint64_t &x_width, uint64_t &last_x_width, uint64_t ndx, uint64_t chunk_size);
int boundingBoxToObjNames(const md_catalog_run_entry &run, const md_catalog_var_entry &var, const std::vector<md_dim_bounds> &bounding_box,
                                     std::vector<std::string> &obj_names);
static int get_matching_object_count (const md_catalog_all_objects_with_var_dims_args &args, uint32_t &count);
int md_insert_object_stub (const md_insert_object_args &args, uint64_t &object_id);

int read_3d_patterns ( const md_catalog_run_entry run, const std::vector<std::vector<md_catalog_var_entry>> &all_var_entries, 
				uint32_t num_read_x_procs, uint32_t num_read_y_procs, uint32_t num_read_z_procs,
				uint32_t num_timesteps, uint64_t total_x_length, uint64_t total_y_length, uint64_t total_z_length 
				);

int boundingBoxToObjNamesAndCounts(const md_catalog_run_entry &run, const md_catalog_var_entry &var, 
							const std::vector<md_dim_bounds> &bounding_box,
                            std::vector<std::string> &obj_names, std::vector<uint64_t> &offsets_and_counts);

int md_catalog_all_objects_with_var_dims_stub (const md_catalog_all_objects_with_var_dims_args &args,
						   std::vector<md_catalog_object_entry> &object_list,
						   uint32_t &count);


enum SQLITE_OJBECTOR_COMPARISON_TIMING_CONSTANTS : unsigned short {

	ERR_DB_SETUP = 10000,
	ERR_PATTERN_1 = 10001,
	ERR_PATTERN_2 = 10002,
	ERR_PATTERN_3 = 10003,
	ERR_PATTERN_4 = 10004,
	ERR_PATTERN_5 = 10005,
	ERR_PATTERN_6 = 10006,

	PROGRAM_START = 0,
	DB_INIT_START = 1,
	DB_INIT_DONE = 2,
	LUA_INIT_START = 3,
	LUA_INIT_DONE = 4,

	WRITE_TIMESTEP_START = 10,
	WRITE_VAR_START = 11,
	BOUNDING_BOX_TO_OBJ_NAMES_START = 12,
	BOUNDING_BOX_TO_OBJ_NAMES_DONE = 13,
	INSERT_OBJECT_START = 14,
	INSERT_OBJECT_DONE = 15,
	INSERT_OBJECTS_START = 16,
	INSERT_OBJECTS_DONE = 17,
	WRITE_VAR_DONE = 18,
	WRITE_TIMESTEP_DONE = 19,

	READING_START = 20,
	FIND_VARS_DONE = 21,
	READING_PATTERNS_DONE = 22,

	CATALOG_ALL_OBJECTS_WITH_VAR_DIMS_START = 50,
	CATALOG_ALL_OBJECTS_WITH_VAR_DIMS_DONE = 51,

	READ_PATTERN_1_START = 100,
	READ_PATTERN_1_DONE = 101, 

	READ_PATTERN_2_START = 200,
	READ_PATTERN_2_DONE = 201, 

	READ_PATTERN_3_START = 300,
	READ_PATTERN_3_DONE = 301, 

	READ_PATTERN_4_START = 400,
	READ_PATTERN_4_DONE = 401, 

	READ_PATTERN_5_START = 500,
	READ_PATTERN_5_DONE = 501, 

	READ_PATTERN_6_START = 600,
	READ_PATTERN_6_DONE = 601,
	
	DB_SIZES = 1000
};


#endif //SQLITE_OBJECTOR_COMAPRISON_HH