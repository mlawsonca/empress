#ifndef SQLHELPERFUNCTS_HH
#define SQLHELPERFUNCTS_HH

#include <sqlite3.h>

int sql_retrieve_var_attrs(sqlite3_stmt * stmt, std::vector<md_catalog_var_attribute_entry> &attribute_list);
int sql_retrieve_run_attrs(sqlite3_stmt * stmt, std::vector<md_catalog_run_attribute_entry> &attribute_list);
int sql_retrieve_timestep_attrs(sqlite3_stmt * stmt, std::vector<md_catalog_timestep_attribute_entry> &attribute_list);
int sql_retrieve_timesteps(sqlite3_stmt * stmt, std::vector<md_catalog_timestep_entry> &entries);
int sql_retrieve_types(sqlite3_stmt * stmt, std::vector<md_catalog_type_entry> &entries);
int sql_retrieve_runs(sqlite3_stmt * stmt, std::vector<md_catalog_run_entry> &entries);
int sql_retrieve_vars(sqlite3_stmt * stmt, std::vector<md_catalog_var_entry> &entries);
std::string get_query_beginning();
std::string get_count_query_beginning();

#include <sstream>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

template <class T>
void get_range_values(const std::string &data, data_range_type range_type, T &val1, T &val2)
{
	std::stringstream sso;
	sso << data;
	boost::archive::text_iarchive ia(sso);

	ia >> val1;
	if(range_type == DATA_RANGE) {
		ia >> val2;
	}
	else {
		val2 = val1;
	}

};

#endif //SQLHELPERFUNCTS_HH
