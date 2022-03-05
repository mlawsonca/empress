#include <vector>
#include <set>
#include <iostream>
#include <algorithm>    // std::sort()
#include <math.h>       /* round, floor, ceil, trunc */
#include "my_metadata_args.h"

using namespace std;

// npx * npy * npz = num_write_procs;
// ndx * ndy * ndz = 5000000;
// npx_r * npy_r * npz_r = num_write_procs / 10;
// ndx * npx = nx;
// ndy * npy = ny;
// ndz * npz = nz;

// nx / npx_r = k1;
// ny / npy_r = k2;
// nz / npz_r = k3;


// implied: (using ndx, ndy, ndz)
// nx / npx = k4;
// ny / npy = k5;
// nz / npz = k6;

extern debugLog debug_log;
extern debugLog extreme_debug_log;

float distance(float x1, float y1,  
            float z1, float x2,  
            float y2, float z2) 
{ 
    float d = sqrt(pow(x2 - x1, 2) +  
                pow(y2 - y1, 2) +  
                pow(z2 - z1, 2) * 1.0); 

    return d;
} 

struct cubic_factor {
    uint32_t x;
    uint32_t y;
    uint32_t z;

    cubic_factor(uint32_t a, uint32_t b, uint32_t c) {
        x = a;
        y = b;
        z = c;
    }

    cubic_factor() { }


    // compare for order.     
    bool operator <(const cubic_factor &factor) const
    {
        //extreme_debug_log << "x: " << x << " y: " << y << " z: " << z << endl;
        //extreme_debug_log << "factor.x: " << factor.x << " factor.y: " << factor.y << " factor.z: " << factor.z << endl;
        //extreme_debug_log << "returning: " << ((x < factor.x) || (y < factor.y) || (z < factor.z)) << endl;
        // return ((x < factor.x) || (y < factor.y) || (z < factor.z));
        return (x < factor.x) || ((x == factor.x) && (y < factor.y)) || ((x == factor.x) && (y == factor.y) && (z < factor.z));
    }
};

struct config {
    cubic_factor nd;
    cubic_factor np;
    cubic_factor np_read;
    uint32_t diff;

    config(cubic_factor a, cubic_factor b, cubic_factor c) {
        nd = a;
        np = b;
        np_read = c;

        uint32_t nd_root = round(cbrt(nd.x*nd.y*nd.z));
        uint32_t np_root = round(cbrt(np.x*np.y*np.z));
        uint32_t np_read_root = round(cbrt(np_read.x * np_read.y * np_read.z));

        diff = distance(nd.x, nd.y, nd.z, nd_root, nd_root, nd_root) + 
               distance(np.x, np.y, np.z, np_root, np_root, np_root) + 
               distance(np_read.x, np_read.y, np_read.z, np_read_root, np_read_root, np_read_root);
        // diff = abs(nd_root - nd.x)+abs(nd_root - nd.y)+abs(nd_root - nd.z)+
        // diff = ( (nd.z - nd.y + 1) * (nd.z-nd.x + 1)) + 10*((np.z - np.y + 1) * (np.z - np.x + 1)) + 100*((np_read.z - np_read.y + 1) * (np_read.z - np_read.x + 1));

        // diff = (nd.z - nd.y + nd.z-nd.x) + 10*(np.z - np.y + np.z - np.x) + 100*(np_read.z - np_read.y + np_read.z - np_read.x);
    }

    bool operator <(const config &other_config) const
    {
        return (diff < other_config.diff);
    }
};


std::ostream& operator << (std::ostream& o, const cubic_factor &factor)
{
    o << "x: " << factor.x << " y: " << factor.y << " z: " << factor.z << endl;
    return o;
}

std::ostream& operator << (std::ostream& o, const config &my_config)
{
    o << "ndx: " << my_config.nd.x << " ndy: " << my_config.nd.y << " ndz: " << my_config.nd.z << endl;
    o << "npx: " << my_config.np.x << " npy: " << my_config.np.y << " npz: " << my_config.np.z << endl;
    o << "npx_read: " << my_config.np_read.x << " npy_read: " << my_config.np_read.y << " npz_read: " << my_config.np_read.z << endl;
    o << "diff: " << my_config.diff << endl;

    // o << "nd: " << my_config.nd << "np: " << my_config.np << "np_read: " << my_config.np_read << endl;
    return o;
}


set<cubic_factor> find_cubic_factors(uint64_t number);
vector<uint32_t> find_factors(uint64_t number);


void find_config(uint32_t num_write_procs, uint32_t num_read_procs, uint32_t num_data_pts_per_proc, 
                uint64_t &my_nx, uint64_t &my_ny, uint64_t &my_nz,
                uint32_t &my_npx, uint32_t &my_npy, uint32_t &my_npz, 
                uint32_t &my_npx_read, uint32_t &my_npy_read, uint32_t &my_npz_read
                ) 
{

    // uint32_t num_write_procs = 4000;
    // uint32_t num_data_pts_per_proc = 5000000;
    // uint32_t num_read_procs = num_write_procs / 10;

    vector<config> configs;


    set<cubic_factor> nd_factors = find_cubic_factors(num_data_pts_per_proc); //ndx, ndy, ndz
    set<cubic_factor> np_factors = find_cubic_factors(num_write_procs); //npx, npy, npz
    set<cubic_factor> np_read_factors = find_cubic_factors(num_read_procs); //npx_r, npy_r, npz_r

    for (std::set<cubic_factor>::iterator it=nd_factors.begin(); it!=nd_factors.end(); ++it) {
        uint32_t ndx = (*it).x;
        uint32_t ndy = (*it).y;
        uint32_t ndz = (*it).z;
        for (std::set<cubic_factor>::iterator it2=np_factors.begin(); it2!=np_factors.end(); ++it2) {
            uint32_t npx = (*it2).x;
            uint32_t npy = (*it2).y;
            uint32_t npz = (*it2).z;

            uint32_t nx = npx * ndx;
            uint32_t ny = npy * ndy;
            uint32_t nz = npz * ndz;
            for (std::set<cubic_factor>::iterator it3=np_read_factors.begin(); it3!=np_read_factors.end(); ++it3) {
                uint32_t npx_r = (*it3).x;
                uint32_t npy_r = (*it3).y;
                uint32_t npz_r = (*it3).z;



                if( (nx%npx_r == 0) && (ny%npy_r == 0) && (nz%npz_r == 0)) {
                    configs.push_back(config((*it), (*it2), (*it3)));
                }
            }
        }
    }
    //debug_log << "configs.size(): " << configs.size() << endl;

    sort(configs.begin(), configs.end());
    // for(int i = 0; i < configs.size(); i++) {
        //debug_log << configs[i] << endl;
    // }

    config best_config = configs[0];

    my_nx = best_config.nd.x * best_config.np.x;
    my_ny = best_config.nd.y * best_config.np.y;
    my_nz = best_config.nd.z * best_config.np.z;
    my_npx = best_config.np.x;
    my_npy = best_config.np.y;
    my_npz = best_config.np.z;
    my_npx_read = best_config.np_read.x;
    my_npy_read = best_config.np_read.y;
    my_npz_read = best_config.np_read.z;

    if(my_npx * my_npy * my_npz != num_write_procs) {
        cout << "error, incorrect number of write procs. supposed to have " << num_write_procs << " but got " << my_npx * my_npy * my_npz << "instead" << endl;
    }
    else {
        cout << "correct, number of write procs: " << num_write_procs << endl;
    }
    if(my_npx_read * my_npy_read * my_npz_read != num_read_procs) {
        cout << "error, incorrect number of read procs. supposed to have " << num_read_procs << " but got " << my_npx_read * my_npy_read * my_npz_read << "instead" << endl;
    }
    else {
        cout << "correct, number of read procs: " << num_read_procs << endl;
    }
    if(my_nx/my_npx * my_ny/my_npy * my_nz/my_npz != num_data_pts_per_proc) {
        cout << "error, incorrect number of data points per proc. supposed to have " << num_data_pts_per_proc << " but got " << 
            (my_nx/my_npx * my_ny/my_npy * my_nz/my_npz) << " instead" << endl;
    }
    else {
        cout << "correct, number of data points per proc: " << num_data_pts_per_proc << endl;
    }          
}

set<cubic_factor> find_cubic_factors(uint64_t number) {
    vector<uint32_t> factors = find_factors(number);
    set<cubic_factor> cubic_factors;

    // cubic_factors.insert(cubic_factor(1, 1, 4000));
    // cubic_factors.insert(cubic_factor(1, 1, 4000));

    for(uint32_t i : factors) {
        for(uint32_t j : factors) {
            //extreme_debug_log << "i: " << i << " j: " << j << endl;

            if(number % (i*j) == 0) {
                //extreme_debug_log << "i*j: " << i*j << endl;
                uint32_t k =  number/(i*j);
                uint32_t arr[] = {i, k, j};
                sort(arr, arr+3);
                //extreme_debug_log << "k: " << k << endl;
                //extreme_debug_log << "attempting to insert " << arr[0] << "," << arr[1] << "," << arr[2] << endl;
                cubic_factors.insert(cubic_factor(arr[0], arr[1], arr[2]));
            }
        }
    }
    //debug_log << "set.size(): " << cubic_factors.size() << endl;
    //debug_log << "factors: " << endl;

    // for (std::set<cubic_factor>::iterator it=cubic_factors.begin(); it!=cubic_factors.end(); ++it) {
    //     std::debug_log << *it;
    // }
    //extreme_debug_log << "returning" << endl;

    return cubic_factors;
}



vector<uint32_t> find_factors(uint64_t number) {
    vector<uint32_t> factors;
    for(uint64_t i = 1; i <= number; ++i)
    {
        //extreme_debug_log << "i: " << i << endl;

        if(number % i == 0) {
            factors.push_back(i);
        }        
    }
    //extreme_debug_log << "returning" << endl;
    return factors;
}