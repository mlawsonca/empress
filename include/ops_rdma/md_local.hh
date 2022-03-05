


#ifndef MDLOCAL_HH
#define MDLOCAL_HH

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <sstream>
// #include <vector>

#include <sqlite3.h>
#include <my_metadata_args.h>

// extern void add_timing_point(int catg);


void get_data_int( std::string data, data_range_type range_type, uint64_t &min, uint64_t &max);
void get_data_real( std::string data, data_range_type range_type, long double &min, long double &max);

};

#endif // MDLOCAL_HH
