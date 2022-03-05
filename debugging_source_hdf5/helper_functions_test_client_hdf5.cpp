#include <helper_functions_test_client_hdf5.hh>
#include <set>
// #include <sstream>
// #include <boost/archive/text_iarchive.hpp>
// #include <boost/archive/text_oarchive.hpp>

using namespace std;

static bool testing_logging = true;
static bool extreme_debug_logging = false;
static bool debug_logging = false;
static bool zero_rank_logging = false;

static debugLog testing_log = debugLog(testing_logging, zero_rank_logging);
static debugLog extreme_debug_log = debugLog(extreme_debug_logging, false);
static debugLog debug_log = debugLog(debug_logging, false);



// void metadata_catalog_type(hid_t var_attr_table_id
//                       ,std::vector<string> &type_names
//                      )
// {

// }



// static void catalog_all_var_attributes_with_type_name ( hid_t var_attr_table_id, uint64_t timestep_id, uint64_t type_name );



void catalog_all_var_attributes ( hid_t var_attr_table_id, uint64_t timestep_id, md_dim_bounds query_dims) 
{
    vector<var_attribute_str> attr_entries;

   metadata_catalog_all_var_attributes (var_attr_table_id, attr_entries);
        testing_log << "using var attrs funct: attributes associated with timestep_id " << timestep_id << endl;
        print_var_attribute_list (attr_entries);


    metadata_catalog_all_var_attributes_with_dims(var_attr_table_id, query_dims, attr_entries);
        testing_log << "using var attrs dims funct: attributes associated with timestep_id " << timestep_id << " overlapping with query_dims";
        print_md_dim_bounds(query_dims);
        print_var_attribute_list (attr_entries);


}

void catalog_all_var_attributes_with_type ( hid_t var_attr_table_id, uint64_t timestep_id, string type_name, md_dim_bounds query_dims ) {

    vector<var_attribute_str> attr_entries;

    metadata_catalog_all_var_attributes_with_type (var_attr_table_id, type_name, attr_entries);
        testing_log << "using var attrs type name ver funct: attributes associated with timestep_id " << 
            timestep_id << " and type " << to_upper(type_name) << endl;
        print_var_attribute_list (attr_entries);


    metadata_catalog_all_var_attributes_with_type_dims (var_attr_table_id, type_name, query_dims, attr_entries);

        testing_log << "using var attrs type dims name ver funct: attributes associated timestep_id " << 
            timestep_id << " and type " << to_upper(type_name) << 
            " overlapping with dims: ";
        print_md_dim_bounds(query_dims);
        print_var_attribute_list (attr_entries);

}

void catalog_all_var_attributes_with_var ( hid_t var_attr_table_id, uint64_t timestep_id, string var_name, md_dim_bounds query_dims ) 
{

    vector<var_attribute_str> attr_entries;


    metadata_catalog_all_var_attributes_with_var (var_attr_table_id, var_name, attr_entries);

        testing_log << "using var attrs by var name ver funct: attributes associated with timestep_id " << timestep_id <<
        	" and var " << to_upper(var_name) << endl;
         print_var_attribute_list (attr_entries);
  
    // retrieveObjNamesAndDataForAttrCatalog(data_outputs, var_attr_table_id, new_run, attr_entries );
    // }

    metadata_catalog_all_var_attributes_with_var_dims (var_attr_table_id, var_name, query_dims, attr_entries);
        testing_log << "using var attr by var dims id funct: attributes associated with timestep_id " << 
            timestep_id << " and var " << to_upper(var_name) << " overlapping with dims: ";
        print_md_dim_bounds(query_dims);
        print_var_attribute_list (attr_entries);

    // retrieveObjNamesAndDataForAttrCatalog(data_outputs, var_attr_table_id, new_run, attr_entries );
    // }
}



void create_timestep(string run_name, string job_id, uint64_t timestep_id, hid_t run_file_id, string type0_name, string type1_name,
					string var0_name, string var1_name)   
{
	herr_t status;
	// hid_t timestep_file_id, timestep_attr_table_id, var_attr_table_id;
	md_timestep_entry timestep;

	uint64_t d0_min, d0_max, d1_min, d1_max, d2_min, d2_max;

	string attr_data;
	vector<var_attribute_str> attrs;

    uint32_t num_dims_var_0 = 2;
    hsize_t dims_var_0[num_dims_var_0] = {10, 10};
	
	uint32_t num_dims_var_1 = 3;
    hsize_t dims_var_1[num_dims_var_1] = {100, 100, 100};


    metadata_create_timestep (run_name, job_id, timestep_id, run_file_id, timestep);
    testing_log << "First timestep id is " << timestep_id << endl;

    // hsize_t dims_chunk_0[num_dims_var_0] = {5, 5};
    // metadata_create_and_close_var_and_chunk(num_dims_var_0, dims_var_0, dims_chunk_0, file_id, var0_name);
    metadata_create_and_close_var(num_dims_var_0, dims_var_0, timestep.file_id, var0_name);
    testing_log << "First var id is 0" << endl;


   // //do for each var
   //  write_output(data_outputs, new_run, new_var); 
   //      return RC_ERR;
   //  }


    metadata_create_and_close_var(num_dims_var_1, dims_var_1, timestep.file_id, var1_name);
    // hsize_t dims_chunk_1[num_dims_var_1] = {10, 10, 10};
    // metadata_create_and_close_var_and_chunk(num_dims_var_1, dims_var_1, dims_chunk_1, file_id, var1_name);
    testing_log << "Second var id is 1" << endl;

   // //do for each var
   //  write_output(data_outputs, new_run, new_var2); 
   //      return RC_ERR;
   //  }

//-------------------------------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------------------------------
//TYPE AND ATTRIBUTE SETUP

    d0_min = 1;
    d0_max = 5;
    d1_min = 2;
    d1_max = 5;
    d2_min = 0;
    d2_max = 0;

    string str("testNULnew0",11);
    make_attr_data(str, 9, attr_data);

    attrs.push_back ( var_attribute_str (type0_name, var0_name, 2, d0_min, d0_max, d1_min,
    		d1_max, d2_min, d2_max, attr_data) );


    d0_min = 3; 
    d0_max = 9;
    d1_min = 4;
    d1_max = 9;
    // d2_min = 5;
    // d2_max = 10;

    string str2("testNULnew1",11);
    make_attr_data(str2, 10, attr_data);

    attrs.push_back ( var_attribute_str (type1_name, var0_name, 2, d0_min, d0_max, d1_min,
    		d1_max, d2_min, d2_max, attr_data) );
  

    d0_min = 0;
    d0_max = 9;
    d1_min = 0;
    d1_max = 9;
    d2_min = 0;
    d2_max = 9;

    string str3("testNULnew2",11);
    make_attr_data(str3, 11, attr_data);

    attrs.push_back ( var_attribute_str (type0_name, var1_name, 3, d0_min, d0_max, d1_min,
    		d1_max, d2_min, d2_max, attr_data) );


    d0_min = 10;
    d0_max = 19;
    d1_min = 10;
    d1_max = 19;
    d2_min = 10;
    d2_max = 19;

    string str4("testNULnew3",11);
    make_attr_data(str4, 12, attr_data);
  
    attrs.push_back ( var_attribute_str (type1_name, var1_name, 3, d0_min, d0_max, d1_min,
    		d1_max, d2_min, d2_max, attr_data) );

    d0_min = 20;
    d0_max = 29;
    d1_min = 20;
    d1_max = 29;
    d2_min = 20;
    d2_max = 29;

    string str5("testNULnew4",11);
    make_attr_data(str5, 13, attr_data);

    attrs.push_back ( var_attribute_str (type1_name, var1_name, 3, d0_min, d0_max, d1_min,
    		d1_max, d2_min, d2_max, attr_data) );


	metadata_insert_var_attribute_batch(timestep.var_attr_table_id, attrs );

	close_timestep(timestep);
}


// static void metadata_create_and_close_var_and_chunk(uint32_t num_dims, hsize_t *var_dims, hsize_t *chunk_dims,
// 					hid_t file_id, const string  var_name,
// 					hid_t &var_id, hid_t &var_data_space, hid_t &chunk_data_space
// 					) 
// void metadata_create_and_close_var_and_chunk(uint32_t num_dims, hsize_t *var_dims, hsize_t *chunk_dims,
// 					hid_t file_id, const string  var_name
// 					) 
// {
// 	/*
//      * Create the var and chunk data space 
//      */
//     hid_t var_data_space = H5Screate_simple(num_dims, var_dims, NULL); assert(var_data_space >= 0);

//     hid_t chunk_data_space  = H5Screate_simple(num_dims, chunk_dims, NULL); assert(chunk_data_space >= 0);

//     /*
//      * Create chunked dataset.
//      */
//     //create a property list for dataset creation
//     hid_t property_list_id = H5Pcreate(H5P_DATASET_CREATE); assert(property_list_id >= 0);

//     //add to the property list the size of a chunk
//     herr_t status = H5Pset_chunk(property_list_id, num_dims, chunk_dims); assert(status >= 0);


//     hid_t var_id = H5Dcreate(file_id, var_name.c_str(), H5T_NATIVE_DOUBLE, var_data_space,
// 			H5P_DEFAULT, property_list_id, H5P_DEFAULT);  assert(var_id >= 0);

//     status = H5Pclose(property_list_id); assert(status >= 0);
//     status = H5Sclose(chunk_data_space); assert(status >= 0);
//     status = H5Sclose(var_data_space); assert(status >= 0);

// }




void catalog_all_var_attributes_with_type_var ( hid_t var_attr_table_id, uint64_t timestep_id, string type_name, string var_name, 
										 md_dim_bounds query_dims ) {


	vector<var_attribute_str> attr_entries;

    metadata_catalog_all_var_attributes_with_type_var (var_attr_table_id, type_name, var_name, attr_entries);

    testing_log << "using var attrs with type var by name ver funct: attributes associated with timestep_id " << 
        timestep_id << " and type " << to_upper(type_name) << " and var " << to_upper(var_name) << endl;
    print_var_attribute_list (attr_entries);


    metadata_catalog_all_var_attributes_with_type_var_dims (var_attr_table_id, type_name, var_name, query_dims, attr_entries);

    testing_log << "using var attrs with type var dims by id funct: attributes associated with timestep_id " << 
        timestep_id << " and type " << to_upper(type_name) << " and var " << to_upper(var_name) <<  " overlapping with dims: ";
    print_md_dim_bounds(query_dims);
    print_var_attribute_list (attr_entries);

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////

void create_new_types(md_timestep_entry timestep0, md_timestep_entry timestep1, string var0_name, string var1_name) 
{
    vector<var_attribute_str> timestep0_attrs;
    vector<var_attribute_str> timestep1_attrs;
    string attr_data;

    uint32_t num_dims = 3;
    md_dim_bounds dims = md_dim_bounds(num_dims, 1, 5, 2, 5, 3, 5);
    string str("testNULnew0",9);
    make_attr_data(str, 9, attr_data);

    string type_name = "typey10";
    timestep0_attrs.push_back( var_attribute_str (type_name, var0_name, dims, attr_data) );

    type_name = "typey20";
    timestep1_attrs.push_back( var_attribute_str (type_name, var0_name, dims, attr_data) );


    type_name = "typey30";
    timestep0_attrs.push_back( var_attribute_str (type_name, var1_name, dims, attr_data) );


    type_name = "typey40";
    dims = md_dim_bounds(num_dims, 1, 4, 2, 3, 3, 3);
    timestep0_attrs.push_back( var_attribute_str (type_name, var0_name, dims, attr_data) );


	metadata_insert_var_attribute_batch(timestep0.var_attr_table_id, timestep0_attrs );
	metadata_insert_var_attribute_batch(timestep1.var_attr_table_id, timestep1_attrs );

}

void catalog_all_types ( hid_t var_attr_table_id, uint64_t timestep_id, string var_name, md_dim_bounds query_dims ) 
{

    vector<string> type_names;

    metadata_catalog_all_types_with_var_attributes_in_timestep (var_attr_table_id, type_names);

        testing_log << "using catalog all types in timestep funct: types associated with timestep_id " << 
        	timestep_id << endl;
            print_type_catalog (type_names);

    metadata_catalog_all_types_with_var_attributes_with_var_in_timestep(var_attr_table_id, var_name, type_names);

        testing_log << "using catalog all types with instances on var in timestep funct: types associated with timestep_id " << timestep_id << 
           " var_name: " << var_name << endl;
            print_type_catalog (type_names);

    metadata_catalog_all_types_with_var_attributes_with_var_dims_in_timestep (var_attr_table_id, var_name, query_dims, type_names);

       testing_log << "using catalog all types with instances on var dims in timestep funct: types associated with timestep_id " << timestep_id << 
            " var_name: " << var_name << " overlapping with dims: ";
        	
        	print_md_dim_bounds(query_dims);
            print_type_catalog (type_names);

}







/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////




void catalog_all_var_attributes_with_var_substr ( hid_t file_id, hid_t var_attr_table_id, uint64_t timestep_id, 
					md_dim_bounds query_dims ) 
{

	vector<var_attribute_str> attr_entries;
	vector<md_var_entry> var_entries;

	string full_var_name_substr = "VAR"; 
	string partial_var_name_substr = "VA"; 

    metadata_catalog_var (file_id, var_entries);

    testing_log << "now the number of vars is " << var_entries.size() << endl;
    testing_log << "new var catalog: \n";
    print_var_catalog (var_entries);
 
    metadata_catalog_all_var_attributes (var_attr_table_id, attr_entries);
        testing_log << "var attributes associated with timestep_id " << timestep_id << endl;
        print_var_attribute_list (attr_entries);


    metadata_catalog_all_var_attributes_with_var_substr (var_attr_table_id, partial_var_name_substr, attr_entries);

        testing_log << "using var attrs by var substr funct: attributes associated with timestep_id " << timestep_id << 
        	" and var_name_substr " << partial_var_name_substr << endl;
            print_var_attribute_list (attr_entries);


    metadata_catalog_all_var_attributes_with_var_substr (var_attr_table_id, full_var_name_substr, attr_entries);

        testing_log << "using var attrs by var substr funct: attributes associated with timestep_id " << timestep_id << 
        	" and var_name_substr " << full_var_name_substr << endl;
            print_var_attribute_list (attr_entries);


    metadata_catalog_all_var_attributes_with_var_substr_dims (var_attr_table_id, partial_var_name_substr, query_dims, attr_entries);
        testing_log << "using var attrs by var dims name ver funct: attributes associated with timestep_id " << timestep_id << 
        	" and var_name_substr " << partial_var_name_substr << " overlapping with dims: ";
    		print_md_dim_bounds(query_dims);
            print_var_attribute_list (attr_entries);


    metadata_catalog_all_var_attributes_with_var_substr_dims (var_attr_table_id, full_var_name_substr, query_dims, attr_entries);
        testing_log << "using var attrs by var dims name ver funct: attributes associated with timestep_id " << timestep_id << 
        	" and var_name_substr " << full_var_name_substr << " overlapping with dims: ";
    		print_md_dim_bounds(query_dims);
            print_var_attribute_list (attr_entries);

}


void catalog_all_var_attributes_with_type_var_substr ( hid_t var_attr_table_id, uint64_t timestep_id, 
					string type_name, md_dim_bounds query_dims ) 
{

    vector<var_attribute_str> attr_entries;
    vector<md_var_entry> var_entries;

	// string full_var_name_substr = "var"; 
	// string partial_var_name_substr = "va"; 

	string full_var_name_substr = "VAR"; 
	string partial_var_name_substr = "VA"; 


    metadata_catalog_all_var_attributes_with_type_var_substr (var_attr_table_id, type_name, full_var_name_substr, attr_entries);
        testing_log << "using var attrs with type var by name ver funct: attributes associated with timestep_id " << 
        	timestep_id << " and type_name " << type_name <<
        	" and var_substr " << full_var_name_substr << endl;
            print_var_attribute_list (attr_entries);


    metadata_catalog_all_var_attributes_with_type_var_substr (var_attr_table_id, type_name, partial_var_name_substr, attr_entries);
        testing_log << "using var attrs with type var by name ver funct: attributes associated with timestep_id " << 
        	timestep_id << " and type_name " << type_name <<
        	" and var_substr " << partial_var_name_substr << endl;
            print_var_attribute_list (attr_entries);


    metadata_catalog_all_var_attributes_with_type_var_substr_dims (var_attr_table_id, type_name, 
    	full_var_name_substr, query_dims, attr_entries);
        testing_log << "using metadata_catalog_all_var_attributes_with_type_var_substr_dims funct: attributes associated with timestep_id " << timestep_id << " and type_name " << type_name <<
        	" and var_substr " << full_var_name_substr << " overlapping with dims: ";
        	print_md_dim_bounds(query_dims);
            print_var_attribute_list (attr_entries);

    metadata_catalog_all_var_attributes_with_type_var_substr_dims (var_attr_table_id, type_name, 
    	partial_var_name_substr, query_dims, attr_entries);
        testing_log << "using metadata_catalog_all_var_attributes_with_type_var_substr_dims funct: attributes associated with timestep_id " << timestep_id << " and type_name " << type_name <<
        	" and var_substr " << partial_var_name_substr << " overlapping with dims: ";
        	print_md_dim_bounds(query_dims);
            print_var_attribute_list (attr_entries);

}

void catalog_all_types_substr ( hid_t var_attr_table_id, uint64_t timestep_id, md_dim_bounds query_dims ) 
{

    vector<string> type_names;

	// string full_var_name_substr = "var"; 
	// string partial_var_name_substr = "va"; 
	string full_var_name_substr = "VAR"; 
	string partial_var_name_substr = "VA"; 

    metadata_catalog_all_types_with_var_attributes_in_timestep (var_attr_table_id, type_names);
        testing_log << "using catalog all types in timestep funct: types associated with timestep_id " << timestep_id << endl;
        print_type_catalog (type_names);

    metadata_catalog_all_types_with_var_attributes_with_var_substr_in_timestep (var_attr_table_id, full_var_name_substr, type_names);
        testing_log << "using catalog all types with instances on var str in timestep funct: types associated " << 
        	"with timestep_id " << timestep_id << " and var_name_substr " << full_var_name_substr << endl;
            print_type_catalog (type_names);

    metadata_catalog_all_types_with_var_attributes_with_var_substr_in_timestep (var_attr_table_id, partial_var_name_substr, type_names);
        testing_log << "using catalog all types with instances on var str in timestep funct: types associated " << 
        	"with timestep_id " << timestep_id << " and var_name_substr " << partial_var_name_substr << endl;
            print_type_catalog (type_names);


    metadata_catalog_all_types_with_var_attributes_with_var_substr_dims_in_timestep (var_attr_table_id, full_var_name_substr, query_dims, type_names);
       testing_log << "using catalog all types with instances on var str dims in timestep funct: types associated " << 
       	   "with timestep_id " << timestep_id << 
            " and var_name_substr " << full_var_name_substr << " overlapping with dims: ";
            print_md_dim_bounds(query_dims);
            print_type_catalog (type_names);

    metadata_catalog_all_types_with_var_attributes_with_var_substr_dims_in_timestep (var_attr_table_id, partial_var_name_substr, query_dims, type_names);
       testing_log << "using catalog all types with instances on var str dims in timestep funct: types associated " << 
       	   "with timestep_id " << timestep_id << 
            " and var_name_substr " << partial_var_name_substr << " overlapping with dims: ";
            print_md_dim_bounds(query_dims);
            print_type_catalog (type_names);


}

void create_timestep_attrs(hid_t timestep_attr_table_id0, hid_t timestep_attr_table_id1,
                    string type0_name, string type1_name )   
{

    vector<non_var_attribute_str> all_timestep_attrs_table0;
    vector<non_var_attribute_str> all_timestep_attrs_table1;

    string attr_data;

    string str("testNULnew0",9);
    make_attr_data(str, 9, attr_data);

    // new_attr.data = "9";
    // make_single_val_data (val, attr_data);
    // new_attr.data = val;

    all_timestep_attrs_table0.push_back( non_var_attribute_str(type0_name, attr_data) );

    string str2("testNULnew 1",9);
    make_attr_data(str2, 10, attr_data);
    // new_attr.data = "10";
    // make_single_val_data (val, attr_data);

    all_timestep_attrs_table0.push_back( non_var_attribute_str(type1_name, attr_data) );

    string str3("testNULnew2",9);
    make_attr_data(str3, 11, attr_data);
    // new_attr.data = "11";
    // make_single_val_data (val, attr_data);


    all_timestep_attrs_table1.push_back( non_var_attribute_str(type0_name, attr_data) );

    string str4("testNULnew3",9);
    make_attr_data(str4, 12, attr_data);
    // new_attr.data = "12";

    // make_single_val_data (val, attr_data);
    all_timestep_attrs_table1.push_back( non_var_attribute_str(type1_name, attr_data) );


    string str5("testNULnew4",9);
    make_attr_data(str5, 13, attr_data);
    // new_attr.data = "13";
    // make_single_val_data (val, attr_data);

    all_timestep_attrs_table1.push_back( non_var_attribute_str(type1_name, attr_data) );

    metadata_insert_timestep_attribute_batch(timestep_attr_table_id0, all_timestep_attrs_table0 );

    metadata_insert_timestep_attribute_batch(timestep_attr_table_id1, all_timestep_attrs_table1 );

}


void catalog_all_timestep_attributes ( hid_t timestep_attr_table_id, uint64_t timestep_id, string type_name) {

    vector<non_var_attribute_str> attr_entries;

    metadata_catalog_all_timestep_attributes (timestep_attr_table_id, attr_entries);

        testing_log << "using metadata_catalog_all_timestep_attributes funct: timestep attrs associated with" << 
        	" timestep_id " << timestep_id << endl;
            print_timestep_attribute_list (attr_entries);


    metadata_catalog_all_timestep_attributes_with_type (timestep_attr_table_id, type_name, attr_entries);

	    testing_log << "using metadata_catalog_all_timestep_attributes_with_type funct: timestep attrs associated with" << 
	    	" timestep_id " << timestep_id << " type_name " << type_name << endl;
	    	print_timestep_attribute_list (attr_entries);

}

void catalog_all_timesteps_substr ( hid_t run_file_id, string type_name, md_dim_bounds query_dims ) {


	// string full_var_name_substr = "var"; 
	// string partial_var_name_substr = "va"; 
	string full_var_name_substr = "VAR"; 
	string partial_var_name_substr = "VA"; 

    vector<string> all_timestep_entries;
    vector<string> matching_timestep_entries;

    metadata_catalog_timestep (run_file_id, all_timestep_entries);
    testing_log << "timestep catalog: \n";
    print_timestep_catalog (all_timestep_entries);

    // metadata_catalog_all_timesteps_with_var_substr (run_file_id, full_var_name_substr, timestep_entries);
    metadata_catalog_all_timesteps_with_var_substr (all_timestep_entries, full_var_name_substr, matching_timestep_entries);

        testing_log << "using catalog_all_timesteps_with_var_substr funct: timesteps associated with" << 
        	" var_name_substr " << full_var_name_substr << "\n";
            print_timestep_catalog (matching_timestep_entries);


    metadata_catalog_all_timesteps_with_var_substr (all_timestep_entries, partial_var_name_substr, matching_timestep_entries);

        testing_log << "using catalog_all_timesteps_with_var_substr funct: timesteps associated with" << 
        	" var_name_substr " << partial_var_name_substr << "\n";
            print_timestep_catalog (matching_timestep_entries);


    metadata_catalog_all_timesteps_with_var_attributes_with_type_var_substr (all_timestep_entries, type_name, full_var_name_substr, matching_timestep_entries);

        testing_log << "using metadata_catalog_all_timesteps_with_var_attributes_with_type_var_substr funct: timesteps associated with" << 
        	" type_name " << type_name << 
        	" var_name_substr " << full_var_name_substr << "\n";
            print_timestep_catalog (matching_timestep_entries);


    metadata_catalog_all_timesteps_with_var_attributes_with_type_var_substr (all_timestep_entries, type_name, partial_var_name_substr, matching_timestep_entries);

        testing_log << "using metadata_catalog_all_timesteps_with_var_attributes_with_type_var_substr funct: timesteps associated with" << 
        	" type_name " << type_name << 
        	" var_name_substr " << partial_var_name_substr << "\n";
         	print_timestep_catalog (matching_timestep_entries);


    metadata_catalog_all_timesteps_with_var_attributes_with_type_var_substr_dims (all_timestep_entries, type_name, full_var_name_substr, query_dims, matching_timestep_entries);
        testing_log << "using metadata_catalog_all_timesteps_with_var_attributes_with_type_var_substr_dims funct: timesteps associated with" << ", type_name " << type_name << 
        	" var_name_substr " << full_var_name_substr << " overlapping with dims";
        	print_md_dim_bounds(query_dims);
            print_timestep_catalog (matching_timestep_entries);


    metadata_catalog_all_timesteps_with_var_attributes_with_type_var_substr_dims (all_timestep_entries, type_name, partial_var_name_substr, query_dims, matching_timestep_entries);
        testing_log << "using metadata_catalog_all_timesteps_with_var_attributes_with_type_var_substr_dims funct: timesteps associated with" << ", type_name " << type_name << 
        	" var_name_substr " << partial_var_name_substr << " overlapping with dims";
      		print_md_dim_bounds(query_dims);
        	print_timestep_catalog (matching_timestep_entries);

}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void create_new_timesteps(string run_name, string job_id, uint64_t timestep_id, hid_t run_file_id, string type0_name, 
					string type1_name, string var0_name, string var1_name, hid_t timestep20_var_attr_table_id) 

{
    md_timestep_entry timestep;

    vector<var_attribute_str> attr_entries;

    md_dim_bounds dims;
    dims.num_dims = 3;
    dims.d0_min = 1;
    dims.d0_max = 5;
    dims.d1_min = 2;
    dims.d1_max = 5;
    dims.d2_min = 3;
    dims.d2_max = 5;

    //////////////////////////////////////////////////////////////////

 	metadata_create_timestep (run_name, job_id, timestep_id+10, run_file_id, timestep);
    testing_log << "First new timestep id is " << timestep_id+10 << endl;

    string attr_data;

    string str("testNULnew0",9);
    make_attr_data(str, 9, attr_data);

   	var_attribute_str var_attr = var_attribute_str (type0_name, var0_name, dims, attr_data);

	metadata_insert_var_attribute(timestep.var_attr_table_id, var_attr);

   	metadata_catalog_all_var_attributes (timestep.var_attr_table_id, attr_entries);
        testing_log << "using var attrs funct: attributes associated with timestep_id " << timestep_id+10 << endl;
        print_var_attribute_list (attr_entries);

	close_timestep(timestep);

	////////////////////////////////////////////////////////////////////////
	//this timestep was created earlier! 
 //    metadata_create_timestep (run_name, job_id, timestep_id+20, run_file_id, timestep_file_id, timestep_attr_table_id, var_attr_table_id);
 //    testing_log << "Second new timestep id is " << timestep_id+20 << endl;


   	var_attr = var_attribute_str (type1_name, var0_name, dims, attr_data);

	metadata_insert_var_attribute(timestep20_var_attr_table_id, var_attr);

   	metadata_catalog_all_var_attributes (timestep20_var_attr_table_id, attr_entries);
        testing_log << "using var attrs funct: attributes associated with timestep_id " << timestep_id+20 << endl;
        print_var_attribute_list (attr_entries);


  	////////////////////////////////////////////////////////////////////////


    metadata_create_timestep (run_name, job_id, timestep_id+30, run_file_id, timestep);

    testing_log << "Third new timestep id is " << timestep_id+30 << endl;


   	var_attr = var_attribute_str (type0_name, var1_name, dims, attr_data);

	metadata_insert_var_attribute(timestep.var_attr_table_id, var_attr);

   	metadata_catalog_all_var_attributes (timestep.var_attr_table_id, attr_entries);
        testing_log << "using var attrs funct: attributes associated with timestep_id " << timestep_id+30 << endl;
        print_var_attribute_list (attr_entries);

	close_timestep(timestep);

	////////////////////////////////////////////////////////////////////////


    metadata_create_timestep (run_name, job_id, timestep_id+40, run_file_id, timestep);

    testing_log << "Fourth new timestep id is " << timestep_id+40 << endl;


    dims.d0_min = 1;
    dims.d0_max = 4;
    dims.d1_min = 2;
    dims.d1_max = 3;
    dims.d2_min = 3;
    dims.d2_max = 3;

   	var_attr = var_attribute_str (type0_name, var0_name, dims, attr_data);

	metadata_insert_var_attribute(timestep.var_attr_table_id, var_attr);

   	metadata_catalog_all_var_attributes (timestep.var_attr_table_id, attr_entries);
        testing_log << "using var attrs funct: attributes associated with timestep_id " << timestep_id+40 << endl;
        print_var_attribute_list (attr_entries);

	close_timestep(timestep);

	testing_log << endl;

}




void catalog_all_timesteps ( hid_t run_file_id, string type_name, string var_name, md_dim_bounds query_dims ) 
{

    vector<string> all_timestep_filenames;
    vector<string> matching_timestep_filenames;

    metadata_catalog_timestep (run_file_id, all_timestep_filenames);
    print_timestep_catalog (all_timestep_filenames);


    metadata_catalog_all_timesteps_with_var (all_timestep_filenames, var_name, matching_timestep_filenames);

        testing_log << "using catalog_all_timesteps_with_var funct: timesteps associated, var_name " << var_name << endl;
            print_timestep_catalog (matching_timestep_filenames);


    metadata_catalog_all_timesteps_with_var_attributes_with_type_var (all_timestep_filenames, type_name, var_name, matching_timestep_filenames);

        testing_log << "using metadata_catalog_all_timesteps_with_var_attributes_with_type_var funct: timesteps associated, type_name " << type_name << 
        	", var_name " << var_name << endl;
            print_timestep_catalog (matching_timestep_filenames);


    metadata_catalog_all_timesteps_with_var_attributes_with_type_var_dims (all_timestep_filenames, type_name, var_name, query_dims, matching_timestep_filenames);

        testing_log << "using metadata_catalog_all_timesteps_with_var_attributes_with_type_var_dims funct: timesteps associated, type_name " << type_name << 
        	", var_name " << var_name << " overlapping with dims: ";
        	print_md_dim_bounds(query_dims);
            print_timestep_catalog (matching_timestep_filenames);

}


void create_run_attrs(hid_t run_attr_table_id, string type0_name, string type1_name )   
{

    vector<non_var_attribute_str> all_run_attrs;

    string attr_data;
    string str("testNULnew0",9);
    make_attr_data(str, 9, attr_data);

    all_run_attrs.push_back( non_var_attribute_str(type0_name, attr_data) );

    string str2("testNULnew 1",9);
    make_attr_data(str2, 10, attr_data);


    all_run_attrs.push_back( non_var_attribute_str(type1_name, attr_data) );

    metadata_insert_timestep_attribute_batch(run_attr_table_id, all_run_attrs );


}


void catalog_all_run_attributes ( hid_t run_attr_table_id, string type_name ) {

    vector<non_var_attribute_str> attr_entries;

    metadata_catalog_all_run_attributes (run_attr_table_id, attr_entries);

        testing_log << "using metadata_catalog_all_run_attributes funct: runs attrs " << endl;
            print_run_attribute_list (attr_entries);


    metadata_catalog_all_run_attributes_with_type (run_attr_table_id, type_name, attr_entries);

	    testing_log << "using metadata_catalog_all_run_attributes_with_type funct: run attrs associated with type_name " << type_name << endl;
	    	print_run_attribute_list (attr_entries);
}
