#include <sstream>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <math.h>

#include <my_metadata_client.h>
#include <my_metadata_client_lua_functs.h>


using namespace std;

static bool testing_logging = false;
static bool extreme_debug_logging = false;
static bool zero_rank_logging = false;
static bool error_logging  = true;

static debugLog testing_log = debugLog(testing_logging, zero_rank_logging);
static debugLog error_log = debugLog(error_logging, zero_rank_logging);
static debugLog extreme_debug_log = debugLog(extreme_debug_logging, false);


// void make_real_attr_data (double test_real, string &serial_str) {
//     stringstream ss;
//     boost::archive::text_oarchive oa(ss);
//     oa << test_real;
//     serial_str = ss.str();
// }


// void make_int_attr_data (int test_int, string &serial_str) {
//     stringstream ss;
//     boost::archive::text_oarchive oa(ss);
//     oa << test_int;
//     serial_str = ss.str();
// }

// void make_string_attr_data (const string &test_str, string &serial_str) {
//     stringstream ss;
//     boost::archive::text_oarchive oa(ss);
//     oa << test_str;
//     serial_str = ss.str();
// }


// void make_blob_attr_data (const string &test_string, int test_int, string &serial_str) {
//     stringstream ss;
//     boost::archive::text_oarchive oa(ss);
//     // extreme_debug_log << "when making attr_data, string.size(): " << test_string.size() << endl;
//     oa << test_string;
//     oa << test_int;
//     serial_str = ss.str();
//     // extreme_debug_log << "serial_str: " << serial_str << endl;
// }

void print_var_attr_data(uint32_t count, const std::vector<md_catalog_var_attribute_entry> &attr_entries) {

    if(count != attr_entries.size()) {
        error_log << "Error trying to print var attr data. Count: " << count << " while attr_entries.size(): " << attr_entries.size() << endl;
    }

    for (int i=0; i<attr_entries.size(); i++) {


        switch(attr_entries.at(i).data_type) {
            case ATTR_DATA_TYPE_INT : {
                uint64_t deserialized_test_int;

                stringstream sso;
                sso << attr_entries.at(i).data;
                boost::archive::text_iarchive ia(sso);
                ia >> deserialized_test_int;

                cout << "data: int: " << deserialized_test_int << endl; 
                break;
            }
            case ATTR_DATA_TYPE_REAL : {
                long double deserialized_test_real;

                stringstream sso;
                sso << attr_entries.at(i).data;
                boost::archive::text_iarchive ia(sso);
                ia >> deserialized_test_real;

                cout << "data: real: " << deserialized_test_real << endl; 
                break;
            }           
            case ATTR_DATA_TYPE_TEXT : {
                    // string deserialized_test_string;
                cout << "data: text: " << attr_entries.at(i).data << endl; 
                break;
            } 
            case ATTR_DATA_TYPE_BLOB : {
                vector<int> deserialized_vals(2);

                stringstream sso;
                sso << attr_entries.at(i).data;
                boost::archive::text_iarchive ia(sso);
                ia >> deserialized_vals;
                cout << "data: blob: val1: " << deserialized_vals.at(0) << " val2: " << deserialized_vals.at(1) << endl; 
                break;
            }    
            // case ATTR_DATA_TYPE_BLOB : {
            //     string deserialized_test_string;
            //     int deserialized_test_int;

            //     stringstream sso;
            //     sso << attr_entries.at(i).data;
            //     boost::archive::text_iarchive ia(sso);
            //     // extreme_debug_log << "sso: " << sso.str() << endl;
            //     ia >> deserialized_test_string;
            //     ia >> deserialized_test_int;

            //     testing_log << "data: blob: str: " << deserialized_test_string << " int: " << deserialized_test_int << endl; 
            //     // testing_log << "str length: " << deserialized_test_string.size() << endl; 

            //     break;
            // } 
        }
        // extreme_debug_log << "serialized var data: " << attr_entries.at(i).data << endl;

        // testing_log << "data: string: " << deserialized_test_string << " int: " << deserialized_test_int << endl; 
    }
 }

 void print_timestep_attr_data(uint32_t count, const std::vector<md_catalog_timestep_attribute_entry> &attr_entries) {

    if(count != attr_entries.size()) {
        error_log << "Error trying to print var attr data. Count: " << count << " while attr_entries.size(): " << attr_entries.size() << endl;
    }

    for (int i=0; i<attr_entries.size(); i++) {
        switch(attr_entries.at(i).data_type) {
            case ATTR_DATA_TYPE_INT : {
                // string deserialized_test_string;
                stringstream sso;
                sso << attr_entries.at(i).data;
                boost::archive::text_iarchive ia(sso);
                uint64_t deserialized_test_int;
                ia >> deserialized_test_int;
                cout << "data: int: " << deserialized_test_int << endl; 
                break;
            }
            case ATTR_DATA_TYPE_REAL : {
                // string deserialized_test_string;
                stringstream sso;
                sso << attr_entries.at(i).data;
                boost::archive::text_iarchive ia(sso);
                long double deserialized_test_real;
                ia >> deserialized_test_real;
                cout << "data: real: " << deserialized_test_real << endl; 
                break;
            }           
            case ATTR_DATA_TYPE_TEXT : {
                    // string deserialized_test_string;
                cout << "data: text: " << attr_entries.at(i).data << endl; 
                break;
            } 
            case ATTR_DATA_TYPE_BLOB : {
                vector<int> deserialized_vals(2);

                stringstream sso;
                sso << attr_entries.at(i).data;
                boost::archive::text_iarchive ia(sso);
                ia >> deserialized_vals;
                cout << "data: blob: val1: " << deserialized_vals.at(0) << " val2: " << deserialized_vals.at(1) << endl; 
                break;
            }    
            // case ATTR_DATA_TYPE_BLOB : {
            //     // extreme_debug_log << "serialized data: " << attr_entries.at(i).data << endl;

            //     string deserialized_test_string;
            //     int deserialized_test_int;

            //     stringstream sso;
            //     sso << attr_entries.at(i).data;
            //     boost::archive::text_iarchive ia(sso);
            //     ia >> deserialized_test_string;
            //     ia >> deserialized_test_int;

            //     testing_log << "data: blob: str: " << deserialized_test_string << " int: " << deserialized_test_int << endl; 
            //     break;
            // }             
        }
    }
 }   


  void print_run_attr_data(uint32_t count, const std::vector<md_catalog_run_attribute_entry> &attr_entries) {

    if(count != attr_entries.size()) {
        error_log << "Error trying to print var attr data. Count: " << count << " while attr_entries.size(): " << attr_entries.size() << endl;
    }

    for (int i=0; i<attr_entries.size(); i++) {
        switch(attr_entries.at(i).data_type) {
            case ATTR_DATA_TYPE_INT : {
                // string deserialized_test_string;
                stringstream sso;
                sso << attr_entries.at(i).data;
                boost::archive::text_iarchive ia(sso);
                uint64_t deserialized_test_int;
                ia >> deserialized_test_int;
                cout << "data: int: " << deserialized_test_int << endl; 
                break;
            }
            case ATTR_DATA_TYPE_REAL : {
                // string deserialized_test_string;
                stringstream sso;
                sso << attr_entries.at(i).data;
                boost::archive::text_iarchive ia(sso);
                long double deserialized_test_real;
                ia >> deserialized_test_real;
                cout << "data: real: " << deserialized_test_real << endl; 
                break;
            }           
            case ATTR_DATA_TYPE_TEXT : {
                    // string deserialized_test_string;
                cout << "data: text: " << attr_entries.at(i).data << endl; 
                break;
            }
            case ATTR_DATA_TYPE_BLOB : {
                vector<int> deserialized_vals(2);

                stringstream sso;
                sso << attr_entries.at(i).data;
                boost::archive::text_iarchive ia(sso);
                ia >> deserialized_vals;
                cout << "data: blob: val1: " << deserialized_vals.at(0) << " val2: " << deserialized_vals.at(1) << endl; 
                break;
            }              
            // case ATTR_DATA_TYPE_BLOB : {
            //     string deserialized_test_string;
            //     int deserialized_test_int;

            //     stringstream sso;
            //     sso << attr_entries.at(i).data;
            //     boost::archive::text_iarchive ia(sso);
            //     ia >> deserialized_test_string;
            //     ia >> deserialized_test_int;

            //     testing_log << "data: blob: str: " << deserialized_test_string << " int: " << deserialized_test_int << endl; 
            //     break;
            // }    

                                       

        }
    }
 }

void generate_data_for_proc(uint64_t ny, uint64_t nz,
                        int rank, const vector<md_dim_bounds> &bounding_box, vector<double> &data_vct, 
                        uint64_t ndx, uint64_t ndy, uint64_t ndz, uint64_t chunk_vol) 
{

    int number = ny;

    int y_digits = 0;
    while (number > 0) {
        number /= 10;
        y_digits++;
    }
    y_digits++;

    int z_digits = 0;
    int number2 = nz;

    while (number2 > 0) {
        number2 /= 10;
        z_digits++;
    }
    z_digits++;

    extreme_debug_log << "ndx: " << ndx << " ndy: " << ndy << " ndz: " << ndz << endl;

    // uint32_t offset = z + NZ(y + NY*x)
    // uint32_t offset = z1 + output.nz*(y1 + output.ny*x1);
    // extreme_debug_log << "starting offset: " << offset << " rank: " << rank << endl;

    extreme_debug_log << "z1: " << bounding_box.at(2).min << " y1: " << bounding_box.at(1).min << " x1: " << bounding_box.at(0).min << endl;

    uint64_t x1 = bounding_box.at(0).min;
    uint64_t y1 = bounding_box.at(1).min;
    uint64_t z1 = bounding_box.at(2).min;

    // extreme_debug_log << "chunk vol: " << chunk_vol << endl;

    for(int i=0; i<chunk_vol; i++) {

        uint64_t z = z1 + i % ndz;  
        uint64_t y = y1 + (i / ndz)%ndy;
        // uint32_t x = (i+offset) / (ndz * ndy);
        uint64_t x = x1 + i / (ndz * ndy);

        double x_portion_of_value = (x+1) * pow(10.0, y_digits);
        // extreme_debug_log << "y: " << y << endl;
        double y_portion_of_value = (y+1);
        double z_portion_of_value = (z+1)/pow(10.0, z_digits-1);
        // extreme_debug_log << "x_portion_of_value: " << x_portion_of_value << " y_portion_of_value: " << y_portion_of_value << " z_portion_of_value: " << z_portion_of_value << endl;

        double val = (x+1) * pow(10.0, y_digits) + (y+1) +(z+1)/pow(10.0, z_digits-1);
        if(rank == 1 && i == chunk_vol-1){
            extreme_debug_log << "val: " << val << " x_portion_of_value: " << x_portion_of_value << " y_portion_of_value: " << y_portion_of_value << " z_portion_of_value: " << z_portion_of_value << endl;

            // extreme_debug_log << "z_portion_of_value: " << z_portion_of_value << " val: " << val << endl;
        }
        data_vct[i] = val;
    }
}

void generate_data_for_proc(const md_catalog_var_entry &var, 
                        int rank, const vector<md_dim_bounds> &bounding_box, vector<double> &data_vct, 
                        uint64_t ndx, uint64_t ndy, uint64_t ndz, uint64_t chunk_vol) 
{

    // data_vct.reserve(chunk_vol);
    uint64_t ny = var.dims.at(1).max - var.dims.at(1).min + 1;
    uint64_t nz = var.dims.at(2).max - var.dims.at(2).min + 1;

   	generate_data_for_proc(ny, nz, rank, bounding_box, data_vct, ndx, ndy, ndz, chunk_vol);

}


void generate_data_for_proc2D(const md_catalog_var_entry &var, 
                        int rank, const vector<md_dim_bounds> &bounding_box, vector<double> &data_vct, 
                        uint64_t ndx, uint64_t ndy, uint64_t chunk_vol) 
{
    uint64_t ny = var.dims.at(1).max - var.dims.at(1).min + 1;

    int number = ny;

    int y_digits = 0;
    while (number > 0) {
        number /= 10;
        y_digits++;
    }
    y_digits++;

    extreme_debug_log << "ndx: " << ndx << " ndy: " << ndy <<  endl;

    extreme_debug_log << "y1: " << bounding_box.at(1).min << " x1: " << bounding_box.at(0).min << endl;

    uint64_t x1 = bounding_box.at(0).min;
    uint64_t y1 = bounding_box.at(1).min;

    for(int i=0; i<chunk_vol; i++) {

        uint64_t y = y1 + i%ndy;
        uint64_t x = x1 + i / (ndy);

        double x_portion_of_value = (x+1) * pow(10.0, y_digits);
        double y_portion_of_value = (y+1);

        double val = x_portion_of_value + y_portion_of_value;
        if(rank == 1 && i == chunk_vol-1){
            extreme_debug_log << "val: " << val << " x_portion_of_value: " << x_portion_of_value << " y_portion_of_value: " << y_portion_of_value << endl;
        }
        data_vct[i] = val;
    }

    // extreme_debug_log << "data_vct.size(): " << 
}


static void get_obj_lengths(const md_catalog_var_entry &var, 
                    uint64_t &x_width, uint64_t &last_x_width, uint64_t ndx, uint64_t chunk_size) 
{

    uint64_t ceph_obj_size = 8000000; //todo 

    extreme_debug_log << "chunk size: " << chunk_size << endl;
    uint32_t num_objs_per_chunk = round(chunk_size / ceph_obj_size);
    if(num_objs_per_chunk <= 0) {
        num_objs_per_chunk = 1;
    }
    extreme_debug_log << "num_objs_per_chunk: " << num_objs_per_chunk << endl;
    x_width = round(ndx / num_objs_per_chunk);
    if(x_width <= 0) {
        x_width = 1;
    }
    extreme_debug_log << "x_width: " << x_width << endl;
    last_x_width = ndx - x_width * (num_objs_per_chunk-1);
    if(last_x_width <= 0) {
        num_objs_per_chunk = num_objs_per_chunk + floor( (last_x_width-1) / x_width);
        last_x_width = ndx - x_width * (num_objs_per_chunk-1);
    }
}


int write_output(std::map <string, vector<double>> &data_outputs, int rank, const md_catalog_run_entry &run, const std::string &objector_funct_name,
                const md_catalog_var_entry &var, const vector<md_dim_bounds> &bounding_box, vector<double> &data_vct) 
{
    int rc;

    uint64_t ndx = (var.dims[0].max - var.dims[0].min + 1) / run.npx; //ndx = nx / npx
    uint64_t ndy = 1; 
    uint64_t ndz = 1; 
    uint64_t chunk_vol;
    uint64_t chunk_size;

    int num_ranks;
    if(var.num_dims == 3) {
        num_ranks = run.npx*run.npy*run.npz;
        ndy = (var.dims[1].max - var.dims[1].min + 1) / run.npy; //ndx = nx / npx
        ndz = (var.dims[2].max - var.dims[2].min + 1) / run.npz; //ndx = nx / npx
    }
    else if(var.num_dims == 2) {
        num_ranks = run.npx*run.npy;
        ndy = (var.dims[1].max - var.dims[1].min + 1) / run.npy; //ndx = nx / npx
    }
    else if(var.num_dims == 1) {
        num_ranks = run.npx;
    }
    chunk_vol = ndx * ndy * ndz;
    chunk_size = chunk_vol * var.data_size;
    extreme_debug_log << "num_ranks: " << num_ranks << endl;
    extreme_debug_log << "ndx: " << ndx << " ndy: " << ndy << " ndz: " << ndz << endl;
    extreme_debug_log << "nx: " << (var.dims[0].max - var.dims[0].min) << " ny: " << (var.dims[1].max - var.dims[1].min) << " ndz: " << (var.dims[2].max - var.dims[2].min) << endl;

        
    std::vector<string> obj_names;

    uint64_t x_width;
    uint64_t last_x_width;

    if(var.num_dims == 3) {
        if (extreme_debug_logging) {
            printf("Bounding box 2: (%d, %d, %d),(%d, %d, %d)\n",bounding_box[0].min,bounding_box[1].min,bounding_box[2].min,bounding_box[0].max,bounding_box[1].max,bounding_box[2].max);
        }
    }

    //retrieve the object names for the proc's chunk
    rc = boundingBoxToObjNames(run, var, bounding_box, obj_names);
    if (rc != RC_OK) {
        error_log << "Error doing the bounding box to obj names, returning \n";
        return RC_ERR;
    }

    extreme_debug_log << "about to try generating data \n";
    data_vct.resize(chunk_vol); 

    //generate the data for the process (which will later be read in from the data file)
    if(var.num_dims == 3) {
        generate_data_for_proc(var, rank, bounding_box, data_vct, ndx, ndy, ndz, chunk_vol);
    }
    else if (var.num_dims == 2) {
        generate_data_for_proc2D(var, rank, bounding_box, data_vct, ndx, ndy, chunk_vol);            
    }
    extreme_debug_log << "data_vct.size(): " << data_vct.size() << endl;

    //figure out where to slice the data vector for each object
    get_obj_lengths(var, x_width, last_x_width, ndx, chunk_size);
    extreme_debug_log << "x_width: " << x_width << " last_x_width: " << last_x_width << endl;


    //slice the data array for each object
    for(int i=0; i<obj_names.size(); i++) {
        extreme_debug_log << "obj names.size(): " << obj_names.size() << endl;

        if(i != obj_names.size()-1) {

            vector<double>::const_iterator first = data_vct.begin() + x_width * ndy * ndz * i; 
            vector<double>::const_iterator last = data_vct.begin() + x_width * ndy * ndz * (i+1);
            vector<double> temp(first, last);
            extreme_debug_log << "temp.size(): " << temp.size() << endl;

            extreme_debug_log << "obj name: " << obj_names[i] << " val 0: " << temp[0] << endl;
            data_outputs[obj_names[i]] = temp;
            extreme_debug_log << "storing data for " << obj_names[i] << endl;
        }
        else {
            vector<double>::const_iterator first = data_vct.end() - last_x_width * ndy * ndz;
            vector<double>::const_iterator last = data_vct.end();
            vector<double> temp(first, last);

            data_outputs[obj_names[i]] = temp;
            extreme_debug_log << "temp.size(): " << temp.size() << endl;

            extreme_debug_log << "storing data for " << obj_names[i] << endl;

        }
    }
    extreme_debug_log << "rank: " << rank << endl;
    return rc;
}


static void getFirstAndLastIndex(const md_catalog_run_entry &run, const md_catalog_var_entry &var, const vector<uint64_t> &offsets_and_counts, 
                        uint64_t &first_index, uint64_t &last_index)
{

    uint64_t ndx = (var.dims[0].max - var.dims[0].min + 1) / run.npx; //ndx = nx / npx
    uint64_t ndy = (var.dims.at(1).max - var.dims.at(1).min + 1) / run.npy;
    uint64_t ndz = (var.dims.at(2).max - var.dims.at(2).min + 1) / run.npz;
    extreme_debug_log << "ndx: " << ndx << " ndy: " << ndy << " ndz: " << ndz << endl;

    uint64_t start_x = offsets_and_counts.at(0);
    uint64_t start_y = offsets_and_counts.at(2);
    uint64_t start_z = offsets_and_counts.at(4);
    uint64_t count_x = offsets_and_counts.at(1);
    uint64_t count_y = offsets_and_counts.at(3);
    uint64_t count_z = offsets_and_counts.at(5);

    if(extreme_debug_logging) {
        printf("Offsets and counts: x: (%d, %d), y: (%d, %d), z: (%d, %d)\n",offsets_and_counts[0],offsets_and_counts[1],offsets_and_counts[2],offsets_and_counts[3],offsets_and_counts[4],offsets_and_counts[5]);
    }


    uint64_t end_x = start_x + count_x - 1;
    uint64_t end_y = start_y + count_y - 1;
    uint64_t end_z = start_z + count_z - 1;

    first_index = start_z + ndz*(start_y + ndy*start_x);
    last_index = end_z + ndz*(end_y + ndy*end_x);
    extreme_debug_log << "last_index: " << last_index << endl;
    extreme_debug_log << "end_z: " << end_z << " end_y: " << end_y << " end_x: " << end_x << endl;

}


static void getFirstAndLastIndex2D(const md_catalog_run_entry &run, const md_catalog_var_entry &var, const vector<uint64_t> &offsets_and_counts, 
                        uint64_t &first_index, uint64_t &last_index)
{
    extreme_debug_log << "var.dims.size(): " << var.dims.size() << endl;

    uint64_t ndx = (var.dims[0].max - var.dims[0].min + 1) / run.npx; //ndx = nx / npx
    uint64_t ndy = (var.dims.at(1).max - var.dims.at(1).min + 1) / run.npy;
    extreme_debug_log << "ndx: " << ndx << " ndy: " << ndy << endl;

    // extreme_debug_log << "got here \n";

    uint64_t start_x = offsets_and_counts.at(0);
    uint64_t start_y = offsets_and_counts.at(2);
    uint64_t count_x = offsets_and_counts.at(1);
    uint64_t count_y = offsets_and_counts.at(3);
    if(extreme_debug_logging) {
        printf("Offsets and counts: x: (%d, %d), y: (%d, %d)\n",offsets_and_counts[0],offsets_and_counts[1],offsets_and_counts[2],offsets_and_counts[3]);
    }

    uint64_t end_x = start_x + count_x - 1;
    uint64_t end_y = start_y + count_y - 1;

    first_index = start_y + ndy*start_x;
    last_index = end_y + ndy*end_x;
    extreme_debug_log << "end_y: " << end_y << " end_x: " << end_x << endl;
}

int write_output(std::map <string, vector<double>> &data_outputs, int rank, const md_catalog_run_entry &run, const std::string &objector_funct_name,
                const md_catalog_var_entry &var, const vector<md_dim_bounds> &bounding_box, const vector<double> &data_vct) 
{
    int rc;

    uint64_t ndx = (var.dims[0].max - var.dims[0].min + 1) / run.npx; //ndx = nx / npx
    uint64_t ndy = 1; 
    uint64_t ndz = 1; 
    uint64_t chunk_vol;
    uint64_t chunk_size;

    int num_ranks;
    if(var.num_dims == 3) {
        num_ranks = run.npx*run.npy*run.npz;
        ndy = (var.dims[1].max - var.dims[1].min + 1) / run.npy; //ndx = nx / npx
        ndz = (var.dims[2].max - var.dims[2].min + 1) / run.npz; //ndx = nx / npx
    }
    else if(var.num_dims == 2) {
        num_ranks = run.npx*run.npy;
        ndy = (var.dims[1].max - var.dims[1].min + 1) / run.npy; //ndx = nx / npx
    }
    else if(var.num_dims == 1) {
        num_ranks = run.npx;
    }
    chunk_vol = ndx * ndy * ndz;
    chunk_size = chunk_vol * var.data_size;
    extreme_debug_log << "num_ranks: " << num_ranks << endl;
    extreme_debug_log << "ndx: " << ndx << " ndy: " << ndy << " ndz: " << ndz << endl;
    extreme_debug_log << "nx: " << (var.dims[0].max - var.dims[0].min) << " ny: " << (var.dims[1].max - var.dims[1].min) << " ndz: " << (var.dims[2].max - var.dims[2].min) << endl;

        
    std::vector<string> obj_names;

    uint64_t x_width;
    uint64_t last_x_width;

    if(var.num_dims == 3) {
        if (extreme_debug_logging) {
            printf("Bounding box 2: (%d, %d, %d),(%d, %d, %d)\n",bounding_box[0].min,bounding_box[1].min,bounding_box[2].min,bounding_box[0].max,bounding_box[1].max,bounding_box[2].max);
        }
    }

    //retrieve the object names for the proc's chunk
    rc = boundingBoxToObjNames(run, var, bounding_box, obj_names);
    if (rc != RC_OK) {
        error_log << "Error doing the bounding box to obj names, returning \n";
        return RC_ERR;
    }

    extreme_debug_log << "about to try generating data \n";

    //figure out where to slice the data vector for each object
    get_obj_lengths(var, x_width, last_x_width, ndx, chunk_size);
    extreme_debug_log << "x_width: " << x_width << " last_x_width: " << last_x_width << endl;


    //slice the data array for each object
    for(int i=0; i<obj_names.size(); i++) {
        extreme_debug_log << "obj names.size(): " << obj_names.size() << endl;

        if(i != obj_names.size()-1) {

            vector<double>::const_iterator first = data_vct.begin() + x_width * ndy * ndz * i; 
            vector<double>::const_iterator last = data_vct.begin() + x_width * ndy * ndz * (i+1);
            vector<double> temp(first, last);
            extreme_debug_log << "temp.size(): " << temp.size() << endl;

            extreme_debug_log << "obj name: " << obj_names[i] << " val 0: " << temp[0] << endl;
            data_outputs[obj_names[i]] = temp;
            extreme_debug_log << "storing data for " << obj_names[i] << endl;
        }
        else {
            vector<double>::const_iterator first = data_vct.end() - last_x_width * ndy * ndz;
            vector<double>::const_iterator last = data_vct.end();
            vector<double> temp(first, last);

            data_outputs[obj_names[i]] = temp;
            extreme_debug_log << "temp.size(): " << temp.size() << endl;

            extreme_debug_log << "storing data for " << obj_names[i] << endl;

        }
    }
    extreme_debug_log << "rank: " << rank << endl;
    return rc;
}



// int retrieve_obj_names_and_data(std::map <string, vector<double>> data_outputs, const md_catalog_run_entry &run, 
//                       const md_catalog_var_entry &var, const vector<md_dim_bounds> &bounding_box,    
//                       vector<string> &obj_names, vector<uint64_t> &offsets_and_counts

// 					  ) 
int retrieve_obj_names_and_data(std::map <string, vector<double>> data_outputs, const md_catalog_run_entry &run, 
                      const md_catalog_var_entry &var, const vector<md_dim_bounds> &bounding_box  
					  ) 
{
    int rc;

    string errmsg;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////this is just for the data/debugging purposes////////////////////////////////////////////////////////////////////////////////////
    uint64_t ny = 0;
    uint64_t nz = 0;

    if (var.num_dims > 1) {
        ny = (var.dims.at(1).max - var.dims.at(1).min + 1);
    }
    if (var.num_dims > 2) {
        nz = (var.dims.at(2).max - var.dims.at(2).min + 1);
    }

    // extreme_debug_log << "got here 2\n";

    int number = ny;
    int y_digits = 0;
    while (number > 0) {
        number /= 10;
        y_digits++;
    }
    y_digits++;

    int z_digits = 0;
    int number2 = nz;

    while (number2 > 0) {
        number2 /= 10;
        z_digits++;
    }
    z_digits++;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        // extreme_debug_log << "got here 4 \n";

	vector<string> obj_names;
	vector<uint64_t> offsets_and_counts;

    extreme_debug_log << "bounding box size: " << bounding_box.size() << endl;
    // printf("Bounding box 3: (%d, %d, %d),(%d, %d, %d)\n",bounding_box[0].min,bounding_box[1].min,bounding_box[2].min,bounding_box[0].max,bounding_box[1].max,bounding_box[2].max);

    // extreme_debug_log << "got here 3\n";

    rc = boundingBoxToObjNamesAndCounts(run, var, bounding_box, obj_names, offsets_and_counts);
    if (rc != RC_OK) {
        error_log << "error with boundingBoxToObjNamesAndCounts \n";
        return rc;
    }

    // extreme_debug_log << "got here 5 \n";

    // printf("Bounding box: (%d, %d, %d),(%d, %d, %d)\n",bounding_box[0].min,bounding_box[1].min,bounding_box[2].min,bounding_box[0].max,bounding_box[1].max,bounding_box[2].max);

    // extreme_debug_log << "obj_names.size(): " << obj_names.size() << endl;
    // extreme_debug_log << "offsets_and_counts.size(): " << offsets_and_counts.size() << endl;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////this is just for the data/debugging purposes////////////////////////////////////////////////////////////////////////////////////
    for(int i=0; i<obj_names.size(); i++) {
        vector<uint64_t>::const_iterator first = offsets_and_counts.begin() + (2*var.num_dims)*i; 
        vector<uint64_t>::const_iterator last = offsets_and_counts.begin() + (2*var.num_dims)*(i+1);
        vector<uint64_t> my_offsets_and_counts(first, last);

        if(extreme_debug_logging) {
            printf("Offsets and counts: x: (%d, %d), y: (%d, %d), z: (%d, %d)\n",my_offsets_and_counts[0],my_offsets_and_counts[1],my_offsets_and_counts[2],my_offsets_and_counts[3],my_offsets_and_counts[4],my_offsets_and_counts[5]);
        }

        vector<double> data_vct;
        extreme_debug_log << "obj name to find: " << obj_names[i] << endl;
        if(data_outputs.find(obj_names[i]) != data_outputs.end()) {
            data_vct = data_outputs.find(obj_names[i])->second;
            extreme_debug_log << "found the data vector \n";
        }
        else {
            error_log << "error. could not find given object name \n";
            error_log << "obj_name: " << obj_names[i] << endl;

            error_log << "map.size(): " << data_outputs.size() << " and map: \n";
            for (std::map<string,vector<double>>::iterator it=data_outputs.begin(); it!=data_outputs.end(); ++it) {
              string obj_name = it->first;
              vector<double> temp_ary = it->second;
              error_log << "obj_name: " << obj_name << endl;
            }
            return RC_ERR;
        }

        //note: this is just for debug purposes/////////////////////////////////////////////////////////////////////////////////////////////
        uint64_t first_index;
        uint64_t last_index;

        if(var.num_dims == 3) {
            getFirstAndLastIndex(run, var, my_offsets_and_counts, first_index, last_index);
        }
        else if(var.num_dims == 2) {
            getFirstAndLastIndex2D(run, var, my_offsets_and_counts, first_index, last_index);
        }
        extreme_debug_log << "first_index: " << first_index << endl;
        extreme_debug_log << "last_index: " << last_index << endl;
        extreme_debug_log << "data_vct.size(): " << data_vct.size() << endl;

        double first_val = data_vct.at(first_index);
        double last_val = data_vct.at(last_index);
        extreme_debug_log << "first val: " << first_val << " last val: " << last_val << endl;
        extreme_debug_log << "obj name:" << obj_names[i].c_str() << endl;

        uint64_t x1 = first_val / pow(10.0, y_digits) - 1;
        uint64_t x2 = last_val / pow(10.0, y_digits) - 1;
        uint64_t y1 = first_val - (x1+1)* pow(10.0, y_digits) -1;
        uint64_t y2 = last_val - (x2+1)* pow(10.0, y_digits) -1;
        // uint64_t y1 = (int)(first_val-1) % ny;
        // uint64_t y2 = (int)(last_val-1) % ny;
        extreme_debug_log << "ny: " << ny << " first_val-1: " << first_val-1 << " (first_val-1) mod ny: " << (int)(first_val-1) % ny << endl;

        if(var.num_dims == 3) {
            uint64_t z1 = round((first_val - (int)first_val)*pow(10.0, z_digits-1) -1);
            uint64_t z2 = round((last_val - (int)last_val)*pow(10.0, z_digits-1) -1);
            printf("data range for obj %d is: (%d, %d, %d),(%d, %d, %d)\n",i,x1,y1,z1,x2,y2,z2);  
        }
        else if(var.num_dims == 2) {
            printf("data range for obj %d is: (%d, %d),(%d, %d)\n",i,x1,y1,x2,y2);              
        }
        
    }
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    return RC_OK;
}



int retrieveObjNamesAndDataForAttrCatalog(const std::map <string, vector<double>> &data_outputs, const md_catalog_run_entry &run, 
										 const vector<md_catalog_var_entry> &var_entries,
                                         uint64_t run_id, uint64_t timestep_id,
                                         uint64_t txn_id,
                                         const vector<md_catalog_var_attribute_entry> &attr_entries )

// int retrieveObjNamesAndDataForAttrCatalog(const std::map <string, vector<double>> &data_outputs, const md_catalog_run_entry &run, 
//                                          uint64_t run_id, uint64_t timestep_id,
//                                          uint64_t txn_id,
//                                          const vector<md_catalog_var_attribute_entry> &attr_entries,
//                                          vector<string> &obj_names, vector<uint64_t> &offsets_and_counts
//                                          )
{
    int rc;

    // vector<md_catalog_var_entry> var_entries;
    uint32_t count;

    // extreme_debug_log << "got here 1\n";

    // rc = metadata_catalog_var (server, run_id, timestep_id, txn_id, count, var_entries);
    // if (rc != RC_OK) {
    //     error_log << "Error cataloging the new set of var entries. Proceeding" << endl;
    //     return RC_ERR;
    // }

    for(int i = 0; i < attr_entries.size(); i++) {
        md_catalog_var_entry matching_var;

        md_catalog_var_attribute_entry attr = attr_entries.at(i);

        int index_var=0;
        while (index_var < var_entries.size() && var_entries.at(index_var).var_id != attr.var_id) {
            index_var++;
        }
        if (var_entries.size() <= index_var) {
            error_log << "error. couldn't find the var with id " << attr.var_id << endl;
            print_var_catalog (var_entries.size(), var_entries);
            return RC_ERR;
        }
        // extreme_debug_log << "got here 3 \n";

        matching_var = var_entries.at(index_var);

        // rc = retrieve_obj_names_and_data(data_outputs, run, matching_var, attr.dims, obj_names, offsets_and_counts);
        rc = retrieve_obj_names_and_data(data_outputs, run, matching_var, attr.dims );
        if (rc != RC_OK) {
            error_log << "error in retrieveObjNamesAndDataForAttrCatalog \n";
            return rc;
        }
    }
    return RC_OK;
}






