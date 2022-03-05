#include "stats_functions_const_streaming.hh"
#include <iostream>

using namespace std;

void combine_stats(stats &aggregate_stats, stats &new_stats) {
    if(new_stats.num_values == 0) {
        return;
    }
    else if(aggregate_stats.num_values == 0) {
        aggregate_stats.max = new_stats.max;
        aggregate_stats.min = new_stats.min;
        aggregate_stats.avg = new_stats.avg;
        aggregate_stats.variance = new_stats.variance;
        aggregate_stats.num_values = new_stats.num_values;

    }
    else {

        long double total_group_avg = (aggregate_stats.num_values*aggregate_stats.avg + new_stats.num_values*new_stats.avg) / (aggregate_stats.num_values + new_stats.num_values);
        long double diff_aggregate_stats = (aggregate_stats.avg - total_group_avg);
        long double diff_new_stats = (new_stats.avg - total_group_avg);

        aggregate_stats.variance = ( aggregate_stats.num_values*(aggregate_stats.variance + pow(diff_aggregate_stats,2)) +
                                   new_stats.num_values*(new_stats.variance + pow(diff_new_stats,2)) ) / (aggregate_stats.num_values + new_stats.num_values);

        aggregate_stats.avg = total_group_avg;

        if(new_stats.max > aggregate_stats.max) {
            aggregate_stats.max = new_stats.max;
        }
        if(new_stats.min < aggregate_stats.min) {
            aggregate_stats.min = new_stats.min;
        }

        aggregate_stats.num_values += new_stats.num_values;

        //reset it so it can be used again
        new_stats.num_values = 0;

    }
}

void combine_stats(stats_size &aggregate_stats, stats_size &new_stats) {
    if(new_stats.num_values == 0) {
        return;
    }
    else if(aggregate_stats.num_values == 0) {
        aggregate_stats.max = new_stats.max;
        aggregate_stats.min = new_stats.min;
        aggregate_stats.avg = new_stats.avg;
        aggregate_stats.variance = new_stats.variance;
        aggregate_stats.num_values = new_stats.num_values;

    }
    else {

        uint64_t total_group_avg = (aggregate_stats.num_values*aggregate_stats.avg + new_stats.num_values*new_stats.avg) / (aggregate_stats.num_values + new_stats.num_values);
        uint64_t diff_aggregate_stats = (aggregate_stats.avg - total_group_avg);
        uint64_t diff_new_stats = (new_stats.avg - total_group_avg);

        aggregate_stats.variance = ( aggregate_stats.num_values*(aggregate_stats.variance + pow(diff_aggregate_stats,2)) +
                                   new_stats.num_values*(new_stats.variance + pow(diff_new_stats,2)) ) / (aggregate_stats.num_values + new_stats.num_values);

        aggregate_stats.avg = total_group_avg;

        if(new_stats.max > aggregate_stats.max) {
            aggregate_stats.max = new_stats.max;
        }
        if(new_stats.min < aggregate_stats.min) {
            aggregate_stats.min = new_stats.min;
        }
        aggregate_stats.num_values += new_stats.num_values;

        //reset it so it can be used again
        new_stats.num_values = 0;
    }
}


void combine_4D(std::vector<std::vector<std::vector<stats>>> &v1, std::vector<std::vector<std::vector<stats>>> &v2)
{
    for(int i = 0; i < v2.size(); i++) {
        for(int j = 0; j < v2[i].size(); j++) {
            for(int k = 0; k < v2[i][j].size(); k++) {
                combine_stats(v1[i][j][k], v2[i][j][k]);
            }
        }
    }
}

void combine_3D(std::vector<std::vector<stats>> &v1, std::vector<std::vector<stats>> &v2)
{

    for(int i = 0; i < v2.size(); i++) {
        for(int j = 0; j < v2[i].size(); j++) {
            combine_stats(v1[i][j], v2[i][j]);
        }
    }
}

void combine_2D(std::vector<stats> &v1, std::vector<stats> &v2)
{

    for(int i = 0; i < v2.size(); i++) {
        combine_stats(v1[i], v2[i]);
    }
}

void combine_1D(stats &v1, stats &v2)
{

    combine_stats(v1, v2);
}

void combine_4D(std::vector<std::vector<std::vector<stats_size>>> &v1, std::vector<std::vector<std::vector<stats_size>>> &v2)
{
    for(int i = 0; i < v2.size(); i++) {
        for(int j = 0; j < v2[i].size(); j++) {
            for(int k = 0; k < v2[i][j].size(); k++) {
                combine_stats(v1[i][j][k], v2[i][j][k]);
            }
        }
    }
}

void combine_3D(std::vector<std::vector<stats_size>> &v1, std::vector<std::vector<stats_size>> &v2)
{

    for(int i = 0; i < v2.size(); i++) {
        for(int j = 0; j < v2[i].size(); j++) {
            combine_stats(v1[i][j], v2[i][j]);
        }
    }
}

void combine_2D(std::vector<stats_size> &v1, std::vector<stats_size> &v2)
{

    for(int i = 0; i < v2.size(); i++) {
        combine_stats(v1[i], v2[i]);
    }
}

void combine_1D(stats_size &v1, stats_size &v2)
{

    combine_stats(v1, v2);
}