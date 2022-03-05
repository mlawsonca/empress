#include <my_metadata_client_hdf5.hh>

#include <set>

using namespace std;

// static bool testing_logging = false;
// static bool extreme_debug_logging = false;
// static bool debug_logging = false;
// static bool zero_rank_logging = false;
static bool error_logging  = true;

// static debugLog testing_log = debugLog(testing_logging, zero_rank_logging);
static debugLog error_log = debugLog(error_logging, false);
// static debugLog extreme_debug_log = debugLog(extreme_debug_logging, false);
// static debugLog debug_log = debugLog(debug_logging, false);

extern void add_timing_point(int catg);

// extern debugLog testing_log;
// extern debugLog error_log;
// extern debugLog extreme_debug_log;
// extern debugLog debug_log;


string to_upper (string val) {
    string new_str = val;
    for (int i =0; i<val.size(); i++) {
        new_str[i] = toupper(new_str[i]);
    }
    // new_str[val.size()] = 'NUL';
    return new_str;
}



void print_dataspace_dims(uint32_t num_dims, hsize_t *dims) {
    char v = 'x';
    for (int j = 0; j < num_dims; j++)
    {
        //testing_log << " " << v <<": (0/" << dims [j]-1 << ") ";
        v++;
    }
    //testing_log << ("\n");
}


void print_attr_dims(var_attribute_str attr) {
	if(attr.num_dims >= 1) {
		//testing_log << " x: (" << attr.d0_min << "/" << attr.d0_max << ") ";
		if(attr.num_dims >= 2) {
			//testing_log << "y: (" << attr.d1_min << "/" << attr.d1_max << ") ";
			if(attr.num_dims >= 3) { 
				//testing_log << "z: (" << attr.d2_min << "/" << attr.d2_max << ") ";
			}
		}
	}
	// //testing_log << " ";
}

void print_md_dim_bounds( md_dim_bounds dims) {
	if(dims.num_dims >= 1) {
		//testing_log << " x: (" << dims.d0_min << "/" << dims.d0_max << ") ";
		if(dims.num_dims >= 2) {
			//testing_log << "y: (" << dims.d1_min << "/" << dims.d1_max << ") ";
			if(dims.num_dims >= 3) { 
				//testing_log << "z: (" << dims.d2_min << "/" << dims.d2_max << ") ";
			}
		}
	}
	//testing_log << endl;
}


void print_type_catalog(vector<string> type_names)
{
    if (type_names.size() == 0) {
        //testing_log << "There are no matching types \n";
    }
    else {
        //testing_log << "matching types (count: " << type_names.size() << "): " << endl;
    }	
    for (string type_name : type_names) {
		//testing_log << "type_name: " << type_name << endl;
	}
	//testing_log << endl;
}

void print_timestep_catalog(vector<string> timestep_file_names)
{
    if (timestep_file_names.size() == 0) {
        //testing_log << "There are no matching timesteps \n";
    }
    else {
        //testing_log << "matching timesteps (count: " << timestep_file_names.size() << "): " << endl;
    }	
    for (string timestep_file_name : timestep_file_names) {
    	std::size_t found = timestep_file_name.find_last_of("/");
		//testing_log << "timestep_file_name: " << timestep_file_name << " timestep: " << timestep_file_name.substr(found+1) << endl;
	}
	//testing_log << endl;
}

void print_var_attribute(var_attribute_str attr) {
	cout << "type_name: " << attr.type_name << " var_name: " << attr.var_name << 
	" num_dims: " << attr.num_dims ;

	print_attr_dims(attr);
	print_attr_data(attr);
}

void free_var_attr(var_attribute_c_str attr) {
    free(attr.type_name);
    free(attr.var_name);
    free(attr.data);
}

void free_non_var_attr(non_var_attribute_c_str attr) {
    free(attr.type_name);
    free(attr.data);
}

void print_var_attribute_list (vector<var_attribute_str> attrs)
{
	if (attrs.size() == 0) {
        cout << "There are no matching attributes \n";
    }
    else {
	    cout << "matching attributes (count: " << attrs.size() << "): " << endl;

	    for(int i = 0; i < attrs.size(); i++) {
	    	var_attribute_str attr = attrs.at(i);
	    	print_var_attribute(attr);
			// free_var_attr(attr);
		}
	}
	cout << endl;	
}


void print_run_attribute_list (vector<non_var_attribute_str> attrs) {
	print_timestep_attribute_list(attrs);
}

void print_timestep_attribute_list (vector<non_var_attribute_str> attrs) {
	if (attrs.size() == 0) {
        cout << "There are no matching attributes \n";
    }
    else {
	    cout << "matching attributes (count: " << attrs.size() << "): " << endl;
	    for(int i = 0; i < attrs.size(); i++) {
	    	non_var_attribute_str attr = attrs.at(i);
			cout << "type_name: " << attr.type_name << " ";
			print_attr_data(attr);
			// free_non_var_attr(attr);
		}
	}
	cout << endl;
}

//3D queries shouldn't return 2D attributes!
//will allow 2D queries to return 3D attributes
bool dims_overlap(var_attribute_str attr, md_dim_bounds query_dims) {
	if (attr.num_dims < query_dims.num_dims) {
		return false;
	}
	switch(query_dims.num_dims) {
		case 3:
			return ( attr.d0_min <= query_dims.d0_max && attr.d0_max >= query_dims.d0_min &&
			 attr.d1_min <= query_dims.d1_max && attr.d1_max >= query_dims.d1_min &&
			 attr.d2_min <= query_dims.d2_max && attr.d2_max >= query_dims.d2_min );

		case 2:
			return ( attr.d0_min <= query_dims.d0_max && attr.d0_max >= query_dims.d0_min &&
			 attr.d1_min <= query_dims.d1_max && attr.d1_max >= query_dims.d1_min );		

		case 1:
			return ( attr.d0_min <= query_dims.d0_max && attr.d0_max >= query_dims.d0_min );

		default :
			return false;		
	}

}

bool dims_overlap(var_attribute_c_str attr, md_dim_bounds query_dims) {
	if (attr.num_dims < query_dims.num_dims) {
		return false;
	}
	switch(query_dims.num_dims) {
		case 3:
			return ( attr.d0_min <= query_dims.d0_max && attr.d0_max >= query_dims.d0_min &&
			 attr.d1_min <= query_dims.d1_max && attr.d1_max >= query_dims.d1_min &&
			 attr.d2_min <= query_dims.d2_max && attr.d2_max >= query_dims.d2_min );

		case 2:
			return ( attr.d0_min <= query_dims.d0_max && attr.d0_max >= query_dims.d0_min &&
			 attr.d1_min <= query_dims.d1_max && attr.d1_max >= query_dims.d1_min );		

		case 1:
			return ( attr.d0_min <= query_dims.d0_max && attr.d0_max >= query_dims.d0_min );

		default :
			return false;		
	}

}



static void create_non_var_attr_table(hid_t file_id, string table_name, hid_t &attr_table_id)
{	
	herr_t status;

    hid_t strtype = H5Tcopy (H5T_C_S1);
    status = H5Tset_size (strtype, H5T_VARIABLE);

    hid_t compound_data_type = H5Tcreate (H5T_COMPOUND, sizeof (non_var_attribute_c_str)); assert(compound_data_type >= 0);
    status = H5Tinsert (compound_data_type, "type_name", HOFFSET (non_var_attribute_c_str, type_name), strtype); assert(status >= 0);     
    status = H5Tinsert (compound_data_type, "data", HOFFSET (non_var_attribute_c_str, data), strtype); assert(status >= 0);   

	attr_table_id = H5PTcreate(file_id, table_name.c_str(), compound_data_type, (hsize_t)100, H5P_DEFAULT); assert(attr_table_id >= 0);

    status = H5Tclose (compound_data_type); assert(status >= 0);
    status = H5Tclose(strtype); assert(status >= 0);

}

static void create_timestep_attr_table(hid_t file_id, hid_t &timestep_attr_table_id) 
{
	create_non_var_attr_table(file_id, "Timestep Attribute Table", timestep_attr_table_id);
}

static void create_run_attr_table(hid_t file_id, hid_t &run_attr_table_id) 
{
	create_non_var_attr_table(file_id, "Run Attribute Table", run_attr_table_id);
}


static void create_var_attr_table(hid_t file_id, hid_t &var_attr_table_id)
{	
	herr_t status;

    hid_t strtype = H5Tcopy (H5T_C_S1);
    status = H5Tset_size (strtype, H5T_VARIABLE);

    hid_t compound_data_type = H5Tcreate (H5T_COMPOUND, sizeof (var_attribute_c_str)); assert(compound_data_type >= 0);
    status = H5Tinsert (compound_data_type, "type_name", HOFFSET (var_attribute_c_str, type_name), strtype); assert(status >= 0);   
    status = H5Tinsert (compound_data_type, "var_name", HOFFSET (var_attribute_c_str, var_name), strtype); assert(status >= 0);   
    status = H5Tinsert (compound_data_type, "num_dims", HOFFSET (var_attribute_c_str, num_dims), H5T_NATIVE_UINT32); assert(status >= 0);
    status = H5Tinsert (compound_data_type, "d0_min", HOFFSET (var_attribute_c_str, d0_min), H5T_NATIVE_UINT64); assert(status >= 0);
    status = H5Tinsert (compound_data_type, "d0_max", HOFFSET (var_attribute_c_str, d0_max), H5T_NATIVE_UINT64); assert(status >= 0);
    status = H5Tinsert (compound_data_type, "d1_min", HOFFSET (var_attribute_c_str, d1_min), H5T_NATIVE_UINT64); assert(status >= 0);
    status = H5Tinsert (compound_data_type, "d1_max", HOFFSET (var_attribute_c_str, d1_max), H5T_NATIVE_UINT64); assert(status >= 0);
    status = H5Tinsert (compound_data_type, "d2_min", HOFFSET (var_attribute_c_str, d2_min), H5T_NATIVE_UINT64); assert(status >= 0);
    status = H5Tinsert (compound_data_type, "d2_max", HOFFSET (var_attribute_c_str, d2_max), H5T_NATIVE_UINT64); assert(status >= 0);   
    status = H5Tinsert (compound_data_type, "data", HOFFSET (var_attribute_c_str, data), strtype); assert(status >= 0);   

	var_attr_table_id = H5PTcreate(file_id, "Var Attribute Table", compound_data_type, (hsize_t)100, H5P_DEFAULT); assert(var_attr_table_id >= 0);

    status = H5Tclose (compound_data_type); assert(status >= 0);
    status = H5Tclose(strtype); assert(status >= 0);

}

void metadata_create_timestep(string run_name, string job_id, uint64_t timestep_id,
					md_timestep_entry &timestep)
{
    add_timing_point(MD_CREATE_TIMESTEP_START);

	 hid_t run_file_id;

    open_run_for_write (run_name, job_id, run_file_id); 
	// char buffer[1024];
	// int len = sprintf (buffer, "%s/%s/%llu", run_id, job_id, timestep_id);
	// int len = sprintf (buffer, "%llu_link", timestep_id);

	string FILENAME = run_name + "/" + job_id + "/" + to_string(timestep_id);
	// cout << "just created FILENAME: " << FILENAME << endl;

    timestep.file_id = H5Fcreate(FILENAME.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT); assert(timestep.file_id >= 0);

	// add_timing_point(CREATE_TIMESTEP_FILE_DONE);

	create_timestep_attr_table(timestep.file_id, timestep.timestep_attr_table_id);

	// add_timing_point(CREATE_TIMESTEP_ATTR_TABLE_DONE);

	create_var_attr_table(timestep.file_id, timestep.var_attr_table_id);

	// add_timing_point(CREATE_VAR_ATTR_TABLE_DONE);

	char buffer[64];
	int len = sprintf (buffer, "%llu_link", timestep_id);

	//reminder - have to keep track of the "run file" to be able to make this link, to later be able to iterate over the timesteps (to catalog them)
	herr_t status = H5Lcreate_external( FILENAME.c_str(), "/", run_file_id, buffer, H5P_DEFAULT, H5P_DEFAULT);

    H5Fclose (run_file_id); 

    add_timing_point(MD_CREATE_TIMESTEP_DONE);
}


void metadata_insert_var_attribute_batch(string run_name, string job_id, uint64_t timestep_id,
	 const vector<var_attribute_str> &attrs, md_timestep_entry &timestep_entry ) 
{
	add_timing_point(MD_INSERT_VAR_ATTRIBUTE_BY_DIMS_BATCH_START);

	open_timestep_file_and_var_attr_table_for_write(run_name, job_id, timestep_id, 
				timestep_entry.file_id, timestep_entry.var_attr_table_id );

	herr_t status = H5PTappend(timestep_entry.var_attr_table_id, attrs.size(), &attrs[0]); assert(status >= 0);
	status = H5PTclose(timestep_entry.var_attr_table_id); assert(status >= 0);

	add_timing_point(MD_INSERT_VAR_ATTRIBUTE_BY_DIMS_BATCH_DONE);
}

void metadata_insert_var_attribute(hid_t var_attr_table_id, var_attribute_str attr) 
{
	add_timing_point(MD_INSERT_VAR_ATTRIBUTE_BY_DIMS_START);

	herr_t status = H5PTappend(var_attr_table_id, 1, &attr); assert(status >= 0);

	add_timing_point(MD_INSERT_VAR_ATTRIBUTE_BY_DIMS_DONE);
}


void metadata_insert_run_attribute_batch(hid_t run_attr_table_id, const vector<non_var_attribute_str> &attrs ) 
{
	add_timing_point(MD_INSERT_RUN_ATTRIBUTE_BATCH_START);

	herr_t status = H5PTappend(run_attr_table_id, attrs.size(), &attrs[0]); assert(status >= 0);

	add_timing_point(MD_INSERT_RUN_ATTRIBUTE_BATCH_DONE);
}

void metadata_insert_run_attribute(hid_t run_attr_table_id, non_var_attribute_str attr) 
{
	add_timing_point(MD_INSERT_RUN_ATTRIBUTE_START);

	herr_t status = H5PTappend(run_attr_table_id, 1, &attr); assert(status >= 0);

	add_timing_point(MD_INSERT_RUN_ATTRIBUTE_DONE);
}

void metadata_insert_timestep_attribute_batch(hid_t timestep_attr_table_id, const vector<non_var_attribute_str> &attrs ) 
{
	add_timing_point(MD_INSERT_TIMESTEP_ATTRIBUTE_BATCH_START);

	herr_t status = H5PTappend(timestep_attr_table_id, attrs.size(), &attrs[0]); assert(status >= 0);
	add_timing_point(MD_INSERT_TIMESTEP_ATTRIBUTE_BATCH_DONE);
}

void metadata_insert_timestep_attribute(hid_t timestep_attr_table_id, non_var_attribute_str attr) 
{
	add_timing_point(MD_INSERT_TIMESTEP_ATTRIBUTE_START);

	herr_t status = H5PTappend(timestep_attr_table_id, 1, &attr); assert(status >= 0);

	add_timing_point(MD_INSERT_TIMESTEP_ATTRIBUTE_DONE);
}

void metadata_catalog_var (hid_t file_id, vector<md_var_entry> &vars ) 
{
	add_timing_point(MD_CATALOG_VAR_START);

	if(vars.size() > 0) {
		vars.clear();
	}
    hid_t group_id = H5Gopen(file_id, "/", H5P_DEFAULT);

    int MAX_NAME = 1024; 

	int i;
	ssize_t len;
	// hsize_t num_objs;
	herr_t status;

	char var_name[MAX_NAME];
	char var_path[MAX_NAME];
	hid_t var_id;

	int object_type;

	H5G_info_t group_info;

    //Get all the members of the groups, one at a time.
	status = H5Gget_info( group_id, &group_info ); assert(status >= 0); assert(status >= 0);


	int var_indx = 0;
	for (i = 0; i < group_info.nlinks; i++) {

		len = H5Lget_name_by_idx( group_id, ".", H5_INDEX_NAME, H5_ITER_INC, i, var_name, MAX_NAME, H5P_DEFAULT ); 

		if( to_upper(var_name).find("TABLE") != string::npos ) {
			// //extreme_debug_log << "found table: " << var_name << endl;
			continue; //is an attribute table
		}	

		H5O_info_t object_info;
		status = H5Oget_info_by_idx( group_id, ".", H5_INDEX_NAME, H5_ITER_INC, i, &object_info, H5P_DEFAULT );

		switch(object_info.type) {
			case H5O_TYPE_DATASET:
				md_var_entry var;

				var.var_name = var_name;

				var_id = H5Dopen(group_id,var_name, H5P_DEFAULT);
				H5Iget_name(var_id, var_path, MAX_NAME  );
				var.var_path = var_path;
				hid_t dataspace = H5Dget_space(var_id); /* the dimensions of the dataset (not shown) */

				//reminder - this index can change if variables are added later
				var.var_indx = var_indx;

				var.num_dims = H5Sget_simple_extent_ndims (dataspace);
				hsize_t max[3];
				status = H5Sget_simple_extent_dims (dataspace, var.dims, max); assert(status >= 0);

				hid_t datatype = H5Dget_type(var_id);
				var.data_size = H5Tget_size (datatype);
				var.datatype_class = H5Tget_class(datatype);

				hid_t property_list = H5Dget_create_plist(var_id); 
				if(H5D_CHUNKED == H5Pget_layout(property_list)){ 
					int rank_chunk = H5Pget_chunk(property_list, 3, var.chunk_dims);
					var.chunked = true;
				}

				// print_var(var);
				var_indx ++;


				H5Pclose(property_list);
				H5Tclose(datatype);
				H5Sclose(dataspace);
				H5Dclose(var_id);

				vars.push_back(var);
				break;

		}
	}

	H5Gclose(group_id);

	add_timing_point(MD_CATALOG_VAR_DONE);
}

void print_var (md_var_entry var)
{
    cout << "var_indx: " << var.var_indx << " name: " << var.var_name << " path: " << var.var_path << 
        " data_size: " << var.data_size << " datatype: " << print_datatype_class(var.datatype_class) << " num_dims: " << var.num_dims << endl;
    cout << "dims: ";
    print_dataspace_dims(var.num_dims, var.dims);
    if (var.chunked) {
    	cout << "chunk dims: ";
        print_dataspace_dims(var.num_dims, var.chunk_dims);
    }
    // //testing_log << endl;
}


void print_var_catalog (const std::vector<md_var_entry> &entries)
{
    if (entries.size() == 0) {
        cout << "There are no matching vars \n";
    }
    else {
        cout << "matching vars (count: " << entries.size() << "): " << endl;
    }

    for (int i = 0; i < entries.size(); i++)
    {
        md_var_entry var = entries [i]; 
        print_var(var);
    }
    cout << endl;
}


string print_datatype_class(H5T_class_t t_class) {

	string data_type_class_name;

	if(t_class < 0){ 
		return "Invalid datatype";
	} else {
		if(t_class == H5T_INTEGER) {
		     return "H5T_INTEGER";
		} else if(t_class == H5T_FLOAT) {
			return "H5T_FLOAT";
		} else if(t_class == H5T_STRING) {
			return "H5T_STRING";
		} else if(t_class == H5T_BITFIELD) {
			return "H5T_BITFIELD";
		} else if(t_class == H5T_OPAQUE) {
			return "H5T_OPAQUE";
		} else if(t_class == H5T_COMPOUND) {
			return "H5T_COMPOUND";
		} else if(t_class == H5T_ARRAY) {
			return "H5T_ARRAY";
		} else if(t_class == H5T_ENUM) {
			return "H5T_ENUM";
		} else  {
			return "Other";
		}
	}
}


void metadata_catalog_all_types_with_var_attributes_in_timestep (hid_t var_attr_table_id
                      	  ,std::vector<string> &type_names
                     	  )
{	
	add_timing_point(MD_CATALOG_ALL_TYPES_WITH_VAR_ATTRIBUTES_IN_TIMESTEP_START);

	if(type_names.size() >= 0) {
		type_names.clear();
	}

	herr_t status = H5PTcreate_index(var_attr_table_id); assert(status >= 0);

	var_attribute_c_str rec_attr;

	std::set<string> set_type_names;

	while( (H5PTget_next(var_attr_table_id, (size_t)1, &rec_attr)) >= 0 ) {
		set_type_names.insert(rec_attr.type_name);
		free_var_attr(rec_attr);
	}

	type_names.resize(set_type_names.size());
	std::copy(set_type_names.begin(), set_type_names.end(), type_names.begin());

	add_timing_point(MD_CATALOG_ALL_TYPES_WITH_VAR_ATTRIBUTES_IN_TIMESTEP_DONE);
}

void metadata_catalog_all_types_with_var_attributes_with_var_in_timestep (hid_t var_attr_table_id
					  	  ,string var_name
                      	  ,std::vector<string> &type_names
                     	  )
{
	add_timing_point(MD_CATALOG_ALL_TYPES_WITH_VAR_ATTRIBUTES_WITH_VAR_IN_TIMESTEP_START);

	if(type_names.size() >= 0) {
		type_names.clear();
	}

	string upper_var_name = to_upper(var_name);

	herr_t status = H5PTcreate_index(var_attr_table_id); assert(status >= 0);

	var_attribute_c_str rec_attr;

	std::set<string> set_type_names;

	while( (H5PTget_next(var_attr_table_id, (size_t)1, &rec_attr)) >= 0 ) {
		if( to_upper(rec_attr.var_name) == upper_var_name ) {
			set_type_names.insert(rec_attr.type_name);
		}
		free_var_attr(rec_attr);
	}

	type_names.resize(set_type_names.size());
	std::copy(set_type_names.begin(), set_type_names.end(), type_names.begin()); 

	add_timing_point(MD_CATALOG_ALL_TYPES_WITH_VAR_ATTRIBUTES_WITH_VAR_IN_TIMESTEP_DONE);
}



void metadata_catalog_all_types_with_var_attributes_with_var_dims_in_timestep (hid_t var_attr_table_id
					  	  ,string var_name
					  	  ,md_dim_bounds query_dims
                      	  ,std::vector<string> &type_names
                     	  )
{
	add_timing_point(MD_CATALOG_ALL_TYPES_WITH_VAR_ATTRIBUTES_WITH_VAR_DIMS_IN_TIMESTEP_START);

	if(type_names.size() >= 0) {
		type_names.clear();
	}

	string upper_var_name = to_upper(var_name);

	herr_t status = H5PTcreate_index(var_attr_table_id); assert(status >= 0);

	var_attribute_c_str rec_attr;

	std::set<string> set_type_names;

	while( (H5PTget_next(var_attr_table_id, (size_t)1, &rec_attr)) >= 0 ) {
		if( to_upper(rec_attr.var_name) == upper_var_name && dims_overlap(rec_attr, query_dims) ) {
			set_type_names.insert(rec_attr.type_name);
		}
		free_var_attr(rec_attr);
	}

	type_names.resize(set_type_names.size());
	std::copy(set_type_names.begin(), set_type_names.end(), type_names.begin()); 

	add_timing_point(MD_CATALOG_ALL_TYPES_WITH_VAR_ATTRIBUTES_WITH_VAR_DIMS_IN_TIMESTEP_DONE);

}


void metadata_create_and_close_var(uint32_t num_dims, hsize_t *var_dims,
					hid_t file_id, const string &VARNAME) 
{

	add_timing_point(MD_CREATE_VAR_START);

	herr_t status;

	/*
     * Create the var data space 
     */
    hid_t var_data_space = H5Screate_simple(num_dims, var_dims, NULL); assert(var_data_space >= 0);


    //create a property list for dataset creation
    hid_t property_list_id = H5Pcreate(H5P_DATASET_CREATE); assert(property_list_id >= 0);


    hid_t var_id = H5Dcreate(file_id, VARNAME.c_str(), H5T_NATIVE_DOUBLE, var_data_space,
			H5P_DEFAULT, property_list_id, H5P_DEFAULT);  assert(var_id >= 0);


    status = H5Dclose(var_id); assert(status >= 0);
    status = H5Pclose(property_list_id); assert(status >= 0);
    status = H5Sclose(var_data_space); assert(status >= 0);

	add_timing_point(MD_CREATE_VAR_DONE);

}

void metadata_catalog_all_var_attributes_with_dims(hid_t var_attr_table_id
					  	  ,md_dim_bounds query_dims
	                      ,vector<var_attribute_str> &attrs
					  	  )
{
	add_timing_point(MD_CATALOG_ALL_VAR_ATTRIBUTES_WITH_DIMS_START);

	if(attrs.size() >= 0) {
		attrs.clear();
	}

	herr_t status = H5PTcreate_index(var_attr_table_id); assert(status >= 0);

	var_attribute_c_str rec_attr;

	while( (H5PTget_next(var_attr_table_id, (size_t)1, &rec_attr)) >= 0 ) {
		if( dims_overlap(rec_attr, query_dims) ) {
			var_attribute_str rec_str_attr = rec_attr;
			attrs.push_back(rec_str_attr);
		}
		free_var_attr(rec_attr);
	}

	add_timing_point(MD_CATALOG_ALL_VAR_ATTRIBUTES_WITH_DIMS_DONE);
}

void metadata_catalog_all_var_attributes (hid_t var_attr_table_id
	                      ,vector<var_attribute_str> &attrs
						  ) 
{
	add_timing_point(MD_CATALOG_ALL_VAR_ATTRIBUTES_START);

	if(attrs.size() >= 0) {
		attrs.clear();
	}

	herr_t status = H5PTcreate_index(var_attr_table_id); assert(status >= 0);

	var_attribute_c_str rec_attr;

	while( H5PTget_next(var_attr_table_id, (size_t)1, &rec_attr) >= 0) {
		var_attribute_str rec_str_attr = rec_attr;
		attrs.push_back(rec_str_attr);
		free_var_attr(rec_attr);
	}

	add_timing_point(MD_CATALOG_ALL_VAR_ATTRIBUTES_DONE);
}

void metadata_catalog_all_var_attributes_with_type (hid_t var_attr_table_id
						  ,string type_name
	                      ,vector<var_attribute_str> &attrs
						  ) 
{
	add_timing_point(MD_CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_BY_ID_START);

	if(attrs.size() >= 0) {
		attrs.clear();
	}

	string upper_type_name = to_upper(type_name);

	herr_t status = H5PTcreate_index(var_attr_table_id); assert(status >= 0);

	var_attribute_c_str rec_attr;

	while( H5PTget_next(var_attr_table_id, (size_t)1, &rec_attr) >= 0) {
		if( to_upper(rec_attr.type_name) == upper_type_name) {
			var_attribute_str rec_str_attr = rec_attr;
			attrs.push_back(rec_str_attr);
		}
		free_var_attr(rec_attr);
	}

	add_timing_point(MD_CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_BY_ID_DONE);
}

void metadata_catalog_all_var_attributes_with_type_dims (hid_t var_attr_table_id
						  ,string type_name
						  ,md_dim_bounds query_dims
	                      ,vector<var_attribute_str> &attrs
						  ) 
{
	add_timing_point(MD_CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_DIMS_BY_ID_START);

	if(attrs.size() >= 0) {
		attrs.clear();
	}

	string upper_type_name = to_upper(type_name);


	herr_t status = H5PTcreate_index(var_attr_table_id); assert(status >= 0);

	var_attribute_c_str rec_attr;

	while( H5PTget_next(var_attr_table_id, (size_t)1, &rec_attr) >= 0) {
		if( to_upper(rec_attr.type_name) == upper_type_name && dims_overlap(rec_attr, query_dims)) {
			var_attribute_str rec_str_attr = rec_attr;
			attrs.push_back(rec_str_attr);
		}
		free_var_attr(rec_attr);
	}
	add_timing_point(MD_CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_DIMS_BY_ID_DONE);
}

void metadata_catalog_all_var_attributes_with_var (hid_t var_attr_table_id
						  ,string var_name
	                      ,vector<var_attribute_str> &attrs
						  ) 
{
	add_timing_point(MD_CATALOG_ALL_VAR_ATTRIBUTES_WITH_VAR_BY_ID_START);

	if(attrs.size() >= 0) {
		attrs.clear();
	}

	string upper_var_name = to_upper(var_name);

	herr_t status = H5PTcreate_index(var_attr_table_id); assert(status >= 0);

	var_attribute_c_str rec_attr;

	// //extreme_debug_log << "upper_var_name: " << upper_var_name << endl;
	while( H5PTget_next(var_attr_table_id, (size_t)1, &rec_attr) >= 0) {
		//case insensitive comparison
		// //extreme_debug_log << "upper attr var name: " << to_upper(rec_attr.var_name) << endl;
		if(to_upper(rec_attr.var_name) == upper_var_name) {
			// //extreme_debug_log << "match" << endl;
			var_attribute_str rec_str_attr = rec_attr;
			attrs.push_back(rec_str_attr);
		}
		free_var_attr(rec_attr);
		// else {
		// 	//extreme_debug_log << to_upper(rec_attr.var_name) << " != " << upper_var_name << endl;
		// }
	}
	add_timing_point(MD_CATALOG_ALL_VAR_ATTRIBUTES_WITH_VAR_BY_ID_DONE);
}

void metadata_catalog_all_var_attributes_with_var_dims (hid_t var_attr_table_id
						  ,string var_name
						  ,md_dim_bounds query_dims
	                      ,vector<var_attribute_str> &attrs
						  ) 
{
	add_timing_point(MD_CATALOG_ALL_VAR_ATTRIBUTES_WITH_VAR_DIMS_BY_ID_START);

	if(attrs.size() >= 0) {
		attrs.clear();
	}

	string upper_var_name = to_upper(var_name);
	// //extreme_debug_log << "upper_var_name: " << upper_var_name << endl;

	herr_t status = H5PTcreate_index(var_attr_table_id); assert(status >= 0);

	var_attribute_c_str rec_attr;

	while( H5PTget_next(var_attr_table_id, (size_t)1, &rec_attr) >= 0) {
		//case insensitive comparison
		// //extreme_debug_log << "upper attr var name: " << to_upper(rec_attr.var_name) << endl;
		if( to_upper(rec_attr.var_name) == upper_var_name && dims_overlap(rec_attr, query_dims)) {
			var_attribute_str rec_str_attr = rec_attr;
			attrs.push_back(rec_str_attr);
		}
		free_var_attr(rec_attr);
	}
	add_timing_point(MD_CATALOG_ALL_VAR_ATTRIBUTES_WITH_VAR_DIMS_BY_ID_DONE);
}

void metadata_catalog_all_var_attributes_with_type_var (hid_t var_attr_table_id
						  ,string type_name
						  ,string var_name
	                      ,vector<var_attribute_str> &attrs
						  ) 
{
	add_timing_point(MD_CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_BY_ID_START);

	if(attrs.size() >= 0) {
		attrs.clear();
	}

	string upper_var_name = to_upper(var_name);
	string upper_type_name = to_upper(type_name);

	herr_t status = H5PTcreate_index(var_attr_table_id); assert(status >= 0);

	var_attribute_c_str rec_attr;

	while( H5PTget_next(var_attr_table_id, (size_t)1, &rec_attr) >= 0) {
		if(to_upper(rec_attr.type_name) == upper_type_name && to_upper(rec_attr.var_name) == upper_var_name) {
			var_attribute_str rec_str_attr = rec_attr;
			attrs.push_back(rec_str_attr);
		}
		free_var_attr(rec_attr);
	}
	add_timing_point(MD_CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_BY_ID_DONE);
}

void metadata_catalog_all_var_attributes_with_type_var_dims (hid_t var_attr_table_id
						  ,string type_name
						  ,string var_name
						  ,md_dim_bounds query_dims
	                      ,vector<var_attribute_str> &attrs
						  ) 
{
	add_timing_point(MD_CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_DIMS_BY_ID_START);

	if(attrs.size() >= 0) {
		attrs.clear();
	}

	string upper_var_name = to_upper(var_name);
	string upper_type_name = to_upper(type_name);

	herr_t status = H5PTcreate_index(var_attr_table_id); assert(status >= 0);

	var_attribute_c_str rec_attr;

	while( H5PTget_next(var_attr_table_id, (size_t)1, &rec_attr) >= 0) {
		if(to_upper(rec_attr.type_name) == upper_type_name && to_upper(rec_attr.var_name) == upper_var_name &&
			dims_overlap(rec_attr, query_dims) ) {
			
			var_attribute_str rec_str_attr = rec_attr;
			attrs.push_back(rec_str_attr);
		}
		free_var_attr(rec_attr);
	}
	add_timing_point(MD_CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_DIMS_BY_ID_DONE);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void metadata_catalog_all_var_attributes_with_var_substr (hid_t var_attr_table_id
						  ,string var_name_substr
	                      ,vector<var_attribute_str> &attrs
						  ) 
{
	add_timing_point(MD_CATALOG_ALL_VAR_ATTRIBUTES_WITH_VAR_SUBSTR_START);

	if(attrs.size() >= 0) {
		attrs.clear();
	}

	string upper_var_name_substr = to_upper(var_name_substr);

	herr_t status = H5PTcreate_index(var_attr_table_id); assert(status >= 0);

	var_attribute_c_str rec_attr;

	while( H5PTget_next(var_attr_table_id, (size_t)1, &rec_attr) >= 0) {
		//case insensitive comparison
		if( to_upper(rec_attr.var_name).find(upper_var_name_substr) != string::npos ) {
			var_attribute_str rec_str_attr = rec_attr;
			attrs.push_back(rec_str_attr);
		}
		free_var_attr(rec_attr);
	}
	add_timing_point(MD_CATALOG_ALL_VAR_ATTRIBUTES_WITH_VAR_SUBSTR_DONE);
}

void metadata_catalog_all_var_attributes_with_var_substr_dims (hid_t var_attr_table_id
						  ,string var_name_substr
						  ,md_dim_bounds query_dims
	                      ,vector<var_attribute_str> &attrs
						  ) 
{
	add_timing_point(MD_CATALOG_ALL_VAR_ATTRIBUTES_WITH_VAR_SUBSTR_DIMS_START);

	if(attrs.size() >= 0) {
		attrs.clear();
	}

	string upper_var_name_substr = to_upper(var_name_substr);

	herr_t status = H5PTcreate_index(var_attr_table_id); assert(status >= 0);

	var_attribute_c_str rec_attr;

	while( H5PTget_next(var_attr_table_id, (size_t)1, &rec_attr) >= 0) {
		//case insensitive comparison
		if( dims_overlap(rec_attr, query_dims) && to_upper(rec_attr.var_name).find(upper_var_name_substr) != string::npos ) {
			var_attribute_str rec_str_attr = rec_attr;
			attrs.push_back(rec_str_attr);
		}
		free_var_attr(rec_attr);
	}
	add_timing_point(MD_CATALOG_ALL_VAR_ATTRIBUTES_WITH_VAR_SUBSTR_DIMS_DONE);
}

void metadata_catalog_all_var_attributes_with_type_var_substr (hid_t var_attr_table_id
						  ,string type_name
						  ,string var_name_substr
	                      ,vector<var_attribute_str> &attrs
						  ) 
{
	add_timing_point(MD_CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_START);

	if(attrs.size() >= 0) {
		attrs.clear();
	}

	string upper_type_name = to_upper(type_name);
	string upper_var_name_substr = to_upper(var_name_substr);

	herr_t status = H5PTcreate_index(var_attr_table_id); assert(status >= 0);

	var_attribute_c_str rec_attr;

	while( H5PTget_next(var_attr_table_id, (size_t)1, &rec_attr) >= 0) {
		//case insensitive comparison
		if( to_upper(rec_attr.type_name) == upper_type_name && to_upper(rec_attr.var_name).find(upper_var_name_substr) != string::npos ) {
			var_attribute_str rec_str_attr = rec_attr;
			attrs.push_back(rec_str_attr);
		}
		free_var_attr(rec_attr);
	}
	add_timing_point(MD_CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_DONE);
}


void metadata_catalog_all_var_attributes_with_type_var_substr_dims (hid_t var_attr_table_id
						  ,string type_name
						  ,string var_name_substr
						  ,md_dim_bounds query_dims
	                      ,vector<var_attribute_str> &attrs
						  ) 
{
	add_timing_point(MD_CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_DIMS_START);

	if(attrs.size() >= 0) {
		attrs.clear();
	}

	string upper_type_name = to_upper(type_name);
	string upper_var_name_substr = to_upper(var_name_substr);

	herr_t status = H5PTcreate_index(var_attr_table_id); assert(status >= 0);

	var_attribute_c_str rec_attr;

	while( H5PTget_next(var_attr_table_id, (size_t)1, &rec_attr) >= 0) {
		//case insensitive comparison
		if( to_upper(rec_attr.type_name) == upper_type_name && dims_overlap(rec_attr, query_dims) &&
			to_upper(rec_attr.var_name).find(upper_var_name_substr) != string::npos ) {

			var_attribute_str rec_str_attr = rec_attr;
			attrs.push_back(rec_str_attr);
		}
		free_var_attr(rec_attr);
	}
	add_timing_point(MD_CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_DIMS_DONE);
}


void metadata_catalog_all_types_with_var_attributes_with_var_substr_in_timestep (hid_t var_attr_table_id
					  	  ,string var_name_substr
                      	  ,std::vector<string> &type_names
                     	  )
{
	add_timing_point(MD_CATALOG_ALL_TYPES_WITH_VAR_ATTRIBUTES_WITH_VAR_SUBSTR_IN_TIMESTEP_START);

	if(type_names.size() >= 0) {
		type_names.clear();
	}

	string upper_var_name_substr = to_upper(var_name_substr);

	herr_t status = H5PTcreate_index(var_attr_table_id); assert(status >= 0);

	var_attribute_c_str rec_attr;

	std::set<string> set_type_names;

	while( (H5PTget_next(var_attr_table_id, (size_t)1, &rec_attr)) >= 0 ) {
		if( to_upper(rec_attr.var_name).find(upper_var_name_substr) != string::npos )  {
			set_type_names.insert(rec_attr.type_name);
		}
		free_var_attr(rec_attr);
	}

	type_names.resize(set_type_names.size());
	std::copy(set_type_names.begin(), set_type_names.end(), type_names.begin()); 

	add_timing_point(MD_CATALOG_ALL_TYPES_WITH_VAR_ATTRIBUTES_WITH_VAR_SUBSTR_IN_TIMESTEP_DONE);
}

void metadata_catalog_all_types_with_var_attributes_with_var_substr_dims_in_timestep (hid_t var_attr_table_id
					  	  ,string var_name_substr
					  	  ,md_dim_bounds query_dims
                      	  ,std::vector<string> &type_names
                     	  )
{
	add_timing_point(MD_CATALOG_ALL_TYPES_WITH_VAR_ATTRIBUTES_WITH_VAR_SUBSTR_DIMS_IN_TIMESTEP_START);

	if(type_names.size() >= 0) {
		type_names.clear();
	}

	string upper_var_name_substr = to_upper(var_name_substr);

	herr_t status = H5PTcreate_index(var_attr_table_id); assert(status >= 0);

	var_attribute_c_str rec_attr;

	std::set<string> set_type_names;

	while( (H5PTget_next(var_attr_table_id, (size_t)1, &rec_attr)) >= 0 ) {
		if( dims_overlap(rec_attr, query_dims) && to_upper(rec_attr.var_name).find(upper_var_name_substr) != string::npos )  {
			set_type_names.insert(rec_attr.type_name);
		}
		free_var_attr(rec_attr);
	}

	type_names.resize(set_type_names.size());
	std::copy(set_type_names.begin(), set_type_names.end(), type_names.begin()); 

	add_timing_point(MD_CATALOG_ALL_TYPES_WITH_VAR_ATTRIBUTES_WITH_VAR_SUBSTR_DIMS_IN_TIMESTEP_DONE);
}


void metadata_catalog_all_timestep_attributes (hid_t timestep_attr_table_id
	                      ,vector<non_var_attribute_str> &attrs
						  ,bool is_timestep //=true declared
						  ) 
{
	if(is_timestep) {
		add_timing_point(MD_CATALOG_ALL_TIMESTEP_ATTRIBUTES_START);
	}

	if(attrs.size() >= 0) {
		attrs.clear();
	}

	herr_t status = H5PTcreate_index(timestep_attr_table_id); assert(status >= 0);

	non_var_attribute_c_str rec_attr;

	while( H5PTget_next(timestep_attr_table_id, (size_t)1, &rec_attr) >= 0) {
		non_var_attribute_str rec_str_attr = rec_attr;
		attrs.push_back(rec_str_attr);
		free_non_var_attr(rec_attr);
	}
	if(is_timestep) {
		add_timing_point(MD_CATALOG_ALL_TIMESTEP_ATTRIBUTES_DONE);
	}
}

void metadata_catalog_all_timestep_attributes_with_type (const vector<string> &timestep_file_names
						  ,string type_name
	                      ,vector<non_var_attribute_str> &attrs
						  ,bool is_timestep //=true declared
						  ) 
{
	if(is_timestep) {
		add_timing_point(MD_CATALOG_ALL_TIMESTEP_ATTRIBUTES_WITH_TYPE_START);
	}
	string upper_type_name = to_upper(type_name);

	if(attrs.size() >= 0) {
		attrs.clear();
	}

	for(string timestep_file_name : timestep_file_names) {
		md_timestep_entry timestep;

		open_timestep_file_and_timestep_attr_table_for_read(timestep_file_name, 
				timestep.file_id, timestep.timestep_attr_table_id);

		herr_t status = H5PTcreate_index(timestep.timestep_attr_table_id); assert(status >= 0);

		non_var_attribute_c_str rec_attr;

		while( H5PTget_next(timestep.timestep_attr_table_id, (size_t)1, &rec_attr) >= 0) {
			if(to_upper(rec_attr.type_name) == upper_type_name ) {
				non_var_attribute_str rec_str_attr = rec_attr;
				attrs.push_back(rec_str_attr);
			}
			free_non_var_attr(rec_attr);
		}
		close_timestep_file_and_attr_table(timestep.file_id, timestep.timestep_attr_table_id);
	} 
	if(is_timestep) {
		add_timing_point(MD_CATALOG_ALL_TIMESTEP_ATTRIBUTES_WITH_TYPE_DONE);
	}
}

void metadata_catalog_all_timestep_attributes_with_type_in_timestep (hid_t timestep_attr_table_id
						  ,string type_name
	                      ,vector<non_var_attribute_str> &attrs
						  ,bool is_timestep //=true declared
						  ) 
{
	if(is_timestep) {
		add_timing_point(MD_CATALOG_ALL_TIMESTEP_ATTRIBUTES_WITH_TYPE_START);
	}

	if(attrs.size() >= 0) {
		attrs.clear();
	}

	string upper_type_name = to_upper(type_name);

	herr_t status = H5PTcreate_index(timestep_attr_table_id); assert(status >= 0);

	non_var_attribute_c_str rec_attr;

	while( H5PTget_next(timestep_attr_table_id, (size_t)1, &rec_attr) >= 0) {
		if(to_upper(rec_attr.type_name) == upper_type_name ) {
			non_var_attribute_str rec_str_attr = rec_attr;
			attrs.push_back(rec_str_attr);
		}
		free_non_var_attr(rec_attr);
	}
	if(is_timestep) {
		add_timing_point(MD_CATALOG_ALL_TIMESTEP_ATTRIBUTES_WITH_TYPE_DONE);
	}
}

void metadata_create_run (string run_name, string job_id, hid_t &file_id, hid_t &run_attr_table_id) 
{    
	add_timing_point(MD_CREATE_RUN_START);

   	string FILENAME = run_name + "/" + job_id + "/run";


	//extreme_debug_log << "FILENAME: " << FILENAME << endl;

    file_id = H5Fcreate(FILENAME.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT); assert(file_id >= 0);

	// add_timing_point(CREATE_RUN_FILE_DONE);

	create_run_attr_table(file_id, run_attr_table_id);
  
 	add_timing_point(MD_CREATE_RUN_DONE);

}

// void metadata_catalog_timestep (hid_t run_file_id, vector<string> &timestep_names ) {
void metadata_catalog_timestep (hid_t run_file_id, vector<string> &timestep_file_names) 
{

	add_timing_point(MD_CATALOG_TIMESTEP_START);

	if(timestep_file_names.size() >= 0) {
		timestep_file_names.clear();
	}

    hid_t group_id = H5Gopen(run_file_id, "/", H5P_DEFAULT);

    int MAX_NAME = 1024; 

	hsize_t i;
	ssize_t size;
	herr_t status;


	H5G_info_t group_info;

    //Get all the members of the groups, one at a time.
	status = H5Gget_info( group_id, &group_info ); assert(status >= 0); assert(status >= 0);

	// cout << "num objs: " << group_info.nlinks << endl;

     //For each object in the group, get the name and what type of object it is
	for (i = 0; i < group_info.nlinks; i++) {

		// size_t link_size;
		H5L_info_t link_info;
		// status = H5Lget_info( group_id, link_name, &link_info, H5P_DEFAULT ); assert(status >= 0);
		status = H5Lget_info_by_idx( group_id, ".", H5_INDEX_NAME, H5_ITER_INC, i, &link_info, H5P_DEFAULT ); 

		if(link_info.type == H5L_TYPE_EXTERNAL) {
			// char linked_filename[MAX_NAME];
			// char linked_filepath[MAX_NAME];
			const char *linked_filename;
			const char *linked_filepath;
			char link_buf[link_info.u.val_size];

			status = H5Lget_val_by_idx( group_id, ".", H5_INDEX_NAME, H5_ITER_INC, i, link_buf, link_info.u.val_size, H5P_DEFAULT ); assert(status >= 0);

			status = H5Lunpack_elink_val( link_buf, link_info.u.val_size, 0, &linked_filename, &linked_filepath ); assert(status >= 0);
			// //debug_log << "linked filename: " << linked_filename << " linked file path: " << linked_filepath << endl;

			timestep_file_names.push_back(linked_filename);
		}

		// free(link_name);
	}

	H5Gclose(group_id);
	add_timing_point(MD_CATALOG_TIMESTEP_DONE);
}

// void metadata_catalog_all_timesteps_with_var_substr(hid_t run_file_id
// 						  ,string var_name_substr
// 						  ,vector<string> &matching_timestep_entries
// 						  )
// {
// 	vector<string> all_timestep_file_names;
// 	metadata_catalog_timestep (run_file_id, all_timestep_file_names);

// 	metadata_catalog_all_timesteps_with_var_substr(all_timestep_file_names, var_name_substr, matching_timestep_entries);

// }


void metadata_catalog_all_timesteps_with_var_substr(vector<string> all_timestep_file_names
						  ,string var_name_substr
						  ,vector<string> &matching_timesteps
						  )
{
	add_timing_point(MD_CATALOG_ALL_TIMESTEPS_WITH_VAR_SUBSTR_START);

	if(matching_timesteps.size() >= 0) {
		matching_timesteps.clear();
	}

	herr_t status;
	int MAX_NAME = 1024;
	ssize_t name_len;

	string upper_var_name_substr = to_upper(var_name_substr);

	for (string timestep_filename : all_timestep_file_names) {
		// cout << "filename: " << timestep_filename << endl;
		hid_t file_id = H5Fopen(timestep_filename.c_str(), H5F_ACC_RDWR, H5P_DEFAULT); assert(file_id >= 0);

    	hid_t group_id = H5Gopen(file_id, "/", H5P_DEFAULT);

		H5G_info_t group_info;

	    //Get all the members of the groups, one at a time.
		status = H5Gget_info( group_id, &group_info ); assert(status >= 0); assert(status >= 0);

		for (hsize_t i = 0; i < group_info.nlinks; i++) {
			char var_name[MAX_NAME];

	    	H5O_info_t object_info;
			herr_t status = H5Oget_info_by_idx( group_id, ".", H5_INDEX_NAME, H5_ITER_INC, i, &object_info, H5P_DEFAULT );
			if( object_info.type == H5O_TYPE_DATASET) {
				ssize_t name_len = H5Lget_name_by_idx( group_id, ".", H5_INDEX_NAME, H5_ITER_INC, i, var_name, MAX_NAME, H5P_DEFAULT ); 

				string upper_var_name = to_upper(var_name);
				if (upper_var_name.find("TABLE") == string::npos && upper_var_name.find(upper_var_name_substr) != string::npos) {
					matching_timesteps.push_back(timestep_filename);
					// cout << "found match" << endl;
					break;
				}
			}
		}
		H5Gclose(group_id);
		H5Fclose(file_id);
	}
	add_timing_point(MD_CATALOG_ALL_TIMESTEPS_WITH_VAR_SUBSTR_DONE);
}


// void metadata_catalog_all_timesteps_with_var_attributes_with_type_var_substr(hid_t run_file_id
// 						  ,string type_name
// 						  ,string var_name_substr
// 						  ,vector<string> &matching_timestep_entries
// 						  )
// {
// 	vector<string> all_timestep_file_names;
// 	metadata_catalog_timestep (run_file_id, all_timestep_file_names);

// 	metadata_catalog_all_timesteps_with_type_var_substr(all_timestep_file_names, type_name, var_name_substr, matching_timestep_entries);

// }


void metadata_catalog_all_timesteps_with_var_attributes_with_type_var_substr(vector<string> all_timestep_file_names
						  ,string type_name
						  ,string var_name_substr
						  ,vector<string> &matching_timesteps
						  )
{
	add_timing_point(MD_CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_START);

	if(matching_timesteps.size() >= 0) {
		matching_timesteps.clear();
	}

	herr_t status;

	string upper_var_name_substr = to_upper(var_name_substr);
	string upper_type_name = to_upper(type_name);

	for (string timestep_filename : all_timestep_file_names) {
		// cout << "filename: " << timestep_filename << endl;
		hid_t file_id = H5Fopen(timestep_filename.c_str(), H5F_ACC_RDWR, H5P_DEFAULT); assert(file_id >= 0);

    	hid_t var_attr_table_id = H5PTopen(file_id, "Var Attribute Table"); assert(var_attr_table_id >= 0);


		status = H5PTcreate_index(var_attr_table_id); assert(status >= 0);

		var_attribute_c_str rec_attr;

		while( H5PTget_next(var_attr_table_id, (size_t)1, &rec_attr) >= 0) {
			//case insensitive comparison
			if( to_upper(rec_attr.type_name) == upper_type_name && to_upper(rec_attr.var_name).find(upper_var_name_substr) != string::npos ) {
				matching_timesteps.push_back(timestep_filename);
				free_var_attr(rec_attr);
				break;
			}
			free_var_attr(rec_attr);
		}
	    
		H5PTclose(var_attr_table_id);
		H5Fclose(file_id);
	}
	add_timing_point(MD_CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_DONE);
}


void metadata_catalog_all_timesteps_with_var_attributes_with_type_var_substr_dims(vector<string> all_timestep_file_names
						  ,string type_name
						  ,string var_name_substr
						  ,md_dim_bounds query_dims
						  ,vector<string> &matching_timesteps
						  )
{
	add_timing_point(MD_CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_DIMS_START);

	if(matching_timesteps.size() >= 0) {
		matching_timesteps.clear();
	}

	herr_t status;

	string upper_var_name_substr = to_upper(var_name_substr);
	string upper_type_name = to_upper(type_name);

	for (string timestep_filename : all_timestep_file_names) {
		// cout << "filename: " << timestep_filename << endl;
		hid_t file_id = H5Fopen(timestep_filename.c_str(), H5F_ACC_RDWR, H5P_DEFAULT); assert(file_id >= 0);

    	hid_t var_attr_table_id = H5PTopen(file_id, "Var Attribute Table"); assert(var_attr_table_id >= 0);

		status = H5PTcreate_index(var_attr_table_id); assert(status >= 0);

		var_attribute_c_str rec_attr;

		while( H5PTget_next(var_attr_table_id, (size_t)1, &rec_attr) >= 0) {
			//case insensitive comparison
			if( to_upper(rec_attr.type_name) == upper_type_name && dims_overlap(rec_attr, query_dims) &&
				to_upper(rec_attr.var_name).find(upper_var_name_substr) != string::npos ) {
				matching_timesteps.push_back(timestep_filename);
				free_var_attr(rec_attr);
				break;
			}
			free_var_attr(rec_attr);
		}
	    
		H5PTclose(var_attr_table_id);
		H5Fclose(file_id);
	}
	add_timing_point(MD_CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_DIMS_DONE);
}


void metadata_catalog_all_timesteps_with_var (vector<string> all_timestep_file_names
						  ,string var_name
						  ,vector<string> &matching_timesteps
						  )
{
	add_timing_point(MD_CATALOG_ALL_TIMESTEPS_WITH_VAR_START);

	if(matching_timesteps.size() >= 0) {
		matching_timesteps.clear();
	}

	herr_t status;
	int MAX_NAME = 1024;
	ssize_t name_len;

	string upper_var_name = to_upper(var_name);

	for (string timestep_filename : all_timestep_file_names) {
		// cout << "filename: " << timestep_filename << endl;
		hid_t file_id = H5Fopen(timestep_filename.c_str(), H5F_ACC_RDWR, H5P_DEFAULT); assert(file_id >= 0);

    	hid_t group_id = H5Gopen(file_id, "/", H5P_DEFAULT);

		H5G_info_t group_info;

	    //Get all the members of the groups, one at a time.
		status = H5Gget_info( group_id, &group_info ); assert(status >= 0); assert(status >= 0);

		for (hsize_t i = 0; i < group_info.nlinks; i++) {
			char var_name[MAX_NAME];

	    	H5O_info_t object_info;
			herr_t status = H5Oget_info_by_idx( group_id, ".", H5_INDEX_NAME, H5_ITER_INC, i, &object_info, H5P_DEFAULT );
			if( object_info.type == H5O_TYPE_DATASET) {
				ssize_t name_len = H5Lget_name_by_idx( group_id, ".", H5_INDEX_NAME, H5_ITER_INC, i, var_name, MAX_NAME, H5P_DEFAULT ); 

				if ( to_upper(var_name) == upper_var_name) {
					matching_timesteps.push_back(timestep_filename);
					// cout << "found match" << endl;
					break;
				}
			}
		}
		H5Gclose(group_id);
		H5Fclose(file_id);
	}
	add_timing_point(MD_CATALOG_ALL_TIMESTEPS_WITH_VAR_DONE);
}

void metadata_catalog_all_timesteps_with_var_attributes_with_type_var(vector<string> all_timestep_file_names
						  ,string type_name
						  ,string var_name
						  ,vector<string> &matching_timesteps
						  )
{
	add_timing_point(MD_CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_START);

	if(matching_timesteps.size() >= 0) {
		matching_timesteps.clear();
	}

	herr_t status;

	string upper_var_name = to_upper(var_name);
	string upper_type_name = to_upper(type_name);

	for (string timestep_filename : all_timestep_file_names) {
		// cout << "filename: " << timestep_filename << endl;
		hid_t file_id = H5Fopen(timestep_filename.c_str(), H5F_ACC_RDWR, H5P_DEFAULT); assert(file_id >= 0);

    	hid_t var_attr_table_id = H5PTopen(file_id, "Var Attribute Table"); assert(var_attr_table_id >= 0);


		status = H5PTcreate_index(var_attr_table_id); assert(status >= 0);

		var_attribute_c_str rec_attr;

		while( H5PTget_next(var_attr_table_id, (size_t)1, &rec_attr) >= 0) {
			//case insensitive comparison
			if( to_upper(rec_attr.type_name) == upper_type_name && to_upper(rec_attr.var_name) == upper_var_name ) {
				matching_timesteps.push_back(timestep_filename);
				free_var_attr(rec_attr);
				break;
			}
			free_var_attr(rec_attr);
		}
	    
		H5PTclose(var_attr_table_id);
		H5Fclose(file_id);
	}
	add_timing_point(MD_CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_DONE);
}

void metadata_catalog_all_timesteps_with_var_attributes_with_type_var_dims(vector<string> all_timestep_file_names
						  ,string type_name
						  ,string var_name
						  ,md_dim_bounds query_dims
						  ,vector<string> &matching_timesteps
						  )
{
	add_timing_point(MD_CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_DIMS_START);

	if(matching_timesteps.size() >= 0) {
		matching_timesteps.clear();
	}

	herr_t status;

	string upper_var_name = to_upper(var_name);
	string upper_type_name = to_upper(type_name);

	for (string timestep_filename : all_timestep_file_names) {
		// cout << "filename: " << timestep_filename << endl;
		hid_t file_id = H5Fopen(timestep_filename.c_str(), H5F_ACC_RDWR, H5P_DEFAULT); assert(file_id >= 0);

    	hid_t var_attr_table_id = H5PTopen(file_id, "Var Attribute Table"); assert(var_attr_table_id >= 0);


		status = H5PTcreate_index(var_attr_table_id); assert(status >= 0);

		var_attribute_c_str rec_attr;

		while( H5PTget_next(var_attr_table_id, (size_t)1, &rec_attr) >= 0) {
			//case insensitive comparison
			if( to_upper(rec_attr.type_name) == upper_type_name && to_upper(rec_attr.var_name) == upper_var_name
					&& dims_overlap(rec_attr, query_dims) ) {
				matching_timesteps.push_back(timestep_filename);
				free_var_attr(rec_attr);
				break;
			}
			free_var_attr(rec_attr);
		}
	    
		H5PTclose(var_attr_table_id);
		H5Fclose(file_id);
	}
	add_timing_point(MD_CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_DIMS_DONE);
}



void metadata_catalog_all_run_attributes (hid_t run_attr_table_id
	                      ,vector<non_var_attribute_str> &attrs
						  ) 
{	
	add_timing_point(MD_CATALOG_ALL_RUN_ATTRIBUTES_START);

	metadata_catalog_all_timestep_attributes (run_attr_table_id, attrs, false);

	add_timing_point(MD_CATALOG_ALL_RUN_ATTRIBUTES_DONE);
}

void metadata_catalog_all_run_attributes_with_type_in_run (hid_t run_attr_table_id
						  ,string type_name
	                      ,vector<non_var_attribute_str> &attrs
						  ) 
{
	add_timing_point(MD_CATALOG_ALL_RUN_ATTRIBUTES_WITH_TYPE_START);

	metadata_catalog_all_timestep_attributes_with_type_in_timestep (run_attr_table_id, type_name, attrs, false);

	add_timing_point(MD_CATALOG_ALL_RUN_ATTRIBUTES_WITH_TYPE_DONE);
}


void metadata_create_and_close_chunked_var(uint32_t num_dims, hsize_t *var_dims, hsize_t *chunk_dims,
					hid_t file_id, const string  var_name
					) 
{
	add_timing_point(MD_CREATE_VAR_START);

	/*
     * Create the var and chunk data space 
     */
    hid_t var_data_space = H5Screate_simple(num_dims, var_dims, NULL); assert(var_data_space >= 0);


    /*
     * Create chunked dataset.
     */
    //create a property list for dataset creation
    hid_t property_list_id = H5Pcreate(H5P_DATASET_CREATE); assert(property_list_id >= 0);

    //add to the property list the size of a chunk
    herr_t status = H5Pset_chunk(property_list_id, num_dims, chunk_dims); assert(status >= 0);


    hid_t var_id = H5Dcreate(file_id, var_name.c_str(), H5T_NATIVE_DOUBLE, var_data_space,
			H5P_DEFAULT, property_list_id, H5P_DEFAULT);  assert(var_id >= 0);

    status = H5Dclose(var_id); assert(status >= 0);
    status = H5Pclose(property_list_id); assert(status >= 0);
    status = H5Sclose(var_data_space); assert(status >= 0);

	add_timing_point(MD_CREATE_VAR_DONE);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void open_file_for_read(string full_file_name, hid_t &file_id)
{
    file_id = H5Fopen(full_file_name.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT); assert(file_id >= 0);
}


void open_file_for_write(string full_file_name, hid_t &file_id)
{
    file_id = H5Fopen(full_file_name.c_str(), H5F_ACC_RDWR, H5P_DEFAULT); assert(file_id >= 0);
}

static string get_timestep_file_name(string run_name, string job_id, uint64_t timestep_id)
{	
	return (run_name + "/" + job_id + "/" + to_string(timestep_id) );
}

void open_timestep_file_for_write(string run_name, string job_id, uint64_t timestep_id, hid_t &timestep_file_id)
{
    open_file_for_write(get_timestep_file_name(run_name, job_id, timestep_id), timestep_file_id);
}


void open_var_attr_table(hid_t timestep_file_id, hid_t &var_attr_table_id)
{
	var_attr_table_id = H5PTopen(timestep_file_id, "Var Attribute Table"); assert(var_attr_table_id >= 0);

}

void open_timestep_attr_table(hid_t timestep_file_id, hid_t &timestep_attr_table_id)
{
    timestep_attr_table_id = H5PTopen(timestep_file_id, "Timestep Attribute Table"); assert(timestep_attr_table_id >= 0);
}

void open_var(hid_t timestep_file_id, string var_name, uint32_t var_version, hid_t &var_id)
{
	string VARNAME = var_name + to_string(var_version);

  	var_id = H5Dopen(timestep_file_id, VARNAME.c_str(),H5P_DEFAULT); assert(var_id >= 0);
}



void open_timestep_file_and_var_attr_table_for_write(string full_timestep_file_name, 
									hid_t &timestep_file_id, hid_t &var_attr_table_id
									)
{
   	open_file_for_write(full_timestep_file_name, timestep_file_id);

    open_var_attr_table(timestep_file_id, var_attr_table_id);
}

void open_timestep_file_and_var_attr_table_for_write(string run_name, string job_id, uint64_t timestep_id, 
									hid_t &timestep_file_id, hid_t &var_attr_table_id
									)
{
   	open_timestep_file_for_write(run_name, job_id, timestep_id, timestep_file_id);

    open_var_attr_table(timestep_file_id, var_attr_table_id);
}

void open_timestep_file_and_timestep_attr_table_for_write(string full_timestep_file_name, 
									hid_t &timestep_file_id, hid_t &timestep_attr_table_id
									)
{
    open_file_for_write(full_timestep_file_name, timestep_file_id);
	
	open_timestep_attr_table(timestep_file_id, timestep_attr_table_id);
}

void open_timestep_file_and_timestep_attr_table_for_write(string run_name, string job_id, uint64_t timestep_id, 
									hid_t &timestep_file_id, hid_t &timestep_attr_table_id
									)
{
    open_timestep_file_for_write(run_name, job_id, timestep_id, timestep_file_id);
	
	open_timestep_attr_table(timestep_file_id, timestep_attr_table_id);
}


void open_timestep_file_and_attr_tables_for_write(string full_timestep_file_name, 
									md_timestep_entry &timestep
									)
{
    open_file_for_write(full_timestep_file_name, timestep.file_id);
	
	open_timestep_attr_table(timestep.file_id, timestep.timestep_attr_table_id);

    open_var_attr_table(timestep.file_id, timestep.var_attr_table_id); 
}

void open_timestep_file_and_attr_tables_for_write(string run_name, string job_id, uint64_t timestep_id, 
									md_timestep_entry &timestep
									)
{
    open_timestep_file_for_write(run_name, job_id, timestep_id, timestep.file_id);
	
	open_timestep_attr_table(timestep.file_id, timestep.timestep_attr_table_id);

    open_var_attr_table(timestep.file_id, timestep.var_attr_table_id); 
}



void open_timestep_file_and_var_attr_table_for_read(string full_timestep_file_name, 
									hid_t &timestep_file_id, hid_t &var_attr_table_id
									)
{
   	open_file_for_read(full_timestep_file_name, timestep_file_id);

    open_var_attr_table(timestep_file_id, var_attr_table_id);
}

void open_timestep_file_and_var_attr_table_for_read(string run_name, string job_id, uint64_t timestep_id, 
									hid_t &timestep_file_id, hid_t &var_attr_table_id
									)
{
   	open_file_for_read(get_timestep_file_name(run_name, job_id, timestep_id), timestep_file_id);

    open_var_attr_table(timestep_file_id, var_attr_table_id);
}

void open_timestep_file_and_timestep_attr_table_for_read(string full_timestep_file_name, 
									hid_t &timestep_file_id, hid_t &timestep_attr_table_id
									)
{
    open_file_for_read(full_timestep_file_name, timestep_file_id);
	
	open_timestep_attr_table(timestep_file_id, timestep_attr_table_id);
}

void open_timestep_file_and_timestep_attr_table_for_read(string run_name, string job_id, uint64_t timestep_id, 
									hid_t &timestep_file_id, hid_t &timestep_attr_table_id
									)
{
    open_file_for_read(get_timestep_file_name(run_name, job_id, timestep_id), timestep_file_id);
	
	open_timestep_attr_table(timestep_file_id, timestep_attr_table_id);
}


void open_timestep_file_and_attr_tables_for_read(string full_timestep_file_name, 
									md_timestep_entry &timestep
									)
{
    open_file_for_read(full_timestep_file_name, timestep.file_id);
	
	open_timestep_attr_table(timestep.file_id, timestep.timestep_attr_table_id);

    open_var_attr_table(timestep.file_id, timestep.var_attr_table_id); 
}

void open_timestep_file_and_attr_tables_for_read(string run_name, string job_id, uint64_t timestep_id, 
									md_timestep_entry &timestep
									)
{
    open_file_for_read(get_timestep_file_name(run_name, job_id, timestep_id), timestep.file_id);
	
	open_timestep_attr_table(timestep.file_id, timestep.timestep_attr_table_id);

    open_var_attr_table(timestep.file_id, timestep.var_attr_table_id); 
}





void close_timestep(md_timestep_entry timestep) {
	herr_t status;
	// cout << "just clsoed timestep" << endl;
	status = H5PTclose(timestep.var_attr_table_id); assert(status >= 0);
	status = H5PTclose(timestep.timestep_attr_table_id); assert(status >= 0);
	status = H5Fclose(timestep.file_id); assert(status >= 0);
}

void close_timestep_file_and_attr_table(hid_t file_id, hid_t attr_table_id) {
	herr_t status;
	status = H5PTclose(attr_table_id); assert(status >= 0);
	status = H5Fclose(file_id); assert(status >= 0);
}

void close_timestep_file(hid_t file_id) {
	herr_t status = H5Fclose(file_id); assert(status >= 0);
}



void open_run_for_read (string run_name, string job_id, hid_t &run_file_id) 
{    
   	string FILENAME = run_name + "/" + job_id + "/run";
	//extreme_debug_log << "FILENAME: " << FILENAME << endl;

    open_file_for_read(FILENAME, run_file_id);
}


void open_run_for_write (string run_name, string job_id, hid_t &run_file_id) 
{    
   	string FILENAME = run_name + "/" + job_id + "/run";
	//extreme_debug_log << "FILENAME: " << FILENAME << endl;

    open_file_for_write(FILENAME, run_file_id);
}



void open_run_attr_table (hid_t run_file_id, hid_t &run_attr_table_id) 
{    
    run_attr_table_id = H5PTopen(run_file_id, "Run Attribute Table"); assert(run_attr_table_id >= 0);

}

void open_run_and_attr_table_for_write (string run_name, string job_id, hid_t &run_file_id, hid_t &run_attr_table_id) 
{    
   	open_run_for_write(run_name, job_id, run_file_id);

	open_run_attr_table (run_file_id, run_attr_table_id);

}

void open_run_and_attr_table_for_read (string run_name, string job_id, hid_t &run_file_id, hid_t &run_attr_table_id) 
{    
   	open_run_for_read(run_name, job_id, run_file_id);

	open_run_attr_table (run_file_id, run_attr_table_id);

}

void close_run (hid_t run_file_id) 
{
	herr_t status = H5Fclose(run_file_id); assert(status >= 0);
}

void close_run_and_attr_table (hid_t run_file_id, hid_t run_attr_table_id) 
{    
	herr_t status;
	status = H5PTclose(run_attr_table_id); assert(status >= 0);
	status = H5Fclose(run_file_id); assert(status >= 0);
}

