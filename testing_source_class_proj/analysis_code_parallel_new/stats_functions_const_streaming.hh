



#ifndef STATSFUNCTIONS_HH
#define STATSFUNCTIONS_HH

#include <vector>
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <numeric>
#include <algorithm>
#include <iostream>
// template<class T>
// T get_sum(std::vector<T> v);

template<class T>
T get_max_vv(const std::vector<std::vector<T>> &vv)  {
    T max = 0;
    for( std::vector<T> v : vv ) {
        if(v.size() > 0) {
            T val = *max_element(v.begin(),v.end());
            if (val > max) {
                max = val;
            }
        }
    }
    return max;
}

// template<class T>
// T get_min(std::vector<std::vector<T>> v)  {
//  auto itv = min_element(v.begin(), v.end(), my_find_min); // find the vector               
//  return *min_element((*itv).begin(), (*itv).end()); 
// }


template<class T>
T get_min_vv(const std::vector<std::vector<T>> &vv)  {
    T min = 1000000000;
    for( std::vector<T> v : vv ) {
        if(v.size() > 0) {
            T val = *min_element(v.begin(),v.end());
            if (val < min) {
                min = val;
            }
        }
    }
    return min;
}


template<class T>
T get_sum(const std::vector<T> &v) {
    return( std::accumulate(v.begin(), v.end(), 0.0) );
}

template<class T>
T get_mean(const std::vector<T> &v) {
    T sum = std::accumulate(v.begin(), v.end(), 0.0);
    // std::cout << "about to get mean, sum: " << sum << " v.size(): " << v.size() << std::endl;
    return (sum / v.size());
}

template<class T>
T get_variance(const std::vector<T> &v) {
    if (v.size() < 2) {
        return 0;
    }
    T mean = get_mean(v);
    T sum_squared_dif = 0;
    for(T value : v) {
        sum_squared_dif += (value-mean)*(value-mean);
    }
    // std::cout << "about to get variance, sum: " << sum_squared_dif << " v.size()-1: " << v.size()-1 << std::endl;

    return sum_squared_dif/(v.size()-1);
}

template<class T>
T get_std_dev(const std::vector<T> &v) {
    return sqrt(get_variance(v));
}


template<class T>
T get_median(std::vector<T> v)  {
   if(v.size() == 0) {
    return 0;
   }
   std::sort( v.begin(), v.end() );

   if (v.size() % 2 == 0) {
      return (v[(v.size() / 2) - 1] + v[v.size() / 2]) / 2.0;
   } 
   return v[v.size() / 2];
}

template<class T>
T get_max(const std::vector<T> &v)  {
   return *max_element(v.begin(),v.end());
}

template<class T>
T get_min(const std::vector<T> &v)  {
   return *min_element(v.begin(),v.end());
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
template<class T>
T get_std_dev(T variance) {
    return sqrt(variance);
}




//alternate method (likely more stable) for calculating streaming variance and average
//only difference: stats.variance still needs to be divided by num_values or num_values-1 when done
// template<class T, class T2>
// void add_value_streaming(T2 &stats, T value) {
//     uint64_t old_avg = stats.avg;
//     stats.num_values += 1;
//     uint64_t delta, delta2;
//     if(value > stats.avg) {
//         delta = value - stats.avg;
//         stats.avg += (delta / stats.num_values);
//         delta2 = value - stats.avg;       
//     }
//     else {
//         delta = stats.avg - value;
//         stats.avg -= (delta / stats.num_values);
//         delta2 = stats.avg - value;
//     }
//     stats.variance += (delta * delta2);
// }



template<class T, class T2>
void add_value_streaming(T2 &stats, T value) {
    if(stats.num_values == 0) {
        stats.avg = value;
        stats.variance = 0;
        stats.num_values = 1;
        stats.min = value;
        stats.max = value;
    }
    else {
        long double old_avg = stats.avg;

        stats.num_values += 1;
        stats.avg = stats.avg + (value - stats.avg)/stats.num_values;
        //do we want to use a biased on unbiased estimator?
        // stats.variance = (stats.variance + (value - old_avg)*(value-stats.avg)) / (stats.num_values-1);
        stats.variance = (stats.variance + (value - old_avg)*(value-stats.avg)) / (stats.num_values);

        if(value < stats.min) {
            stats.min = value;
        }
        else if(value > stats.max) {
            stats.max = value;
        }
    }

    // stats.last_value = value;
}

// template<class T, class T2>
// void add_value_streaming(T2 &stats, T value) {
//     T old_avg = stats.avg;

//     stats.num_values += 1;
//     stats.avg = stats.avg + (value - stats.avg)/stats.num_values;
//     stats.variance = stats.variance + (value - old_avg)*(value-stats.avg);

//     if(value < stats.min) {
//         stats.min = value;
//     }
//     else if(value > stats.max) {
//         stats.max = value;
//     }
// }

template<class T, class T2>
void update_stats_streaming(T2 &stats, const std::vector<T> &v) {

    int i = 0;
    if(stats.num_values == 0 && v.size() > 0) {
        stats.avg = v[0];
        stats.variance = 0;
        stats.num_values = 1;
        stats.min = v[0];
        stats.max = v[0];
        i++;
    }
    for(int j = i; j < v.size(); j++) {
        add_value_streaming(stats, v[j]);
    }

    // v.clear();
}


struct stats {
    long double max;
    long double min;
    long double avg;
    long double variance;
    uint64_t num_values = 0;

    // long double last_value = 0;


    uint64_t size() const {
        return num_values;
    }

    void push_back(long double value) {
        add_value_streaming(*this, value);
    }

    // long double back() {
    //     return last_value;
    // }

    long double sum() {
        return avg*num_values;
    }

    void clear() {
        num_values = 0;
    }
};



struct stats_size {
    uint64_t max;
    uint64_t min;
    long double avg;
    long double variance;
    // uint64_t avg;
    // uint64_t variance;
    uint64_t num_values = 0;

    // uint64_t last_value = 0;

    uint64_t size() const {
        return num_values;
    }

    void push_back(uint64_t value) {
        add_value_streaming(*this, value);
    }

    // uint64_t back() {
    //     return last_value;
    // }

    uint64_t sum() {
        return avg*num_values;
    }

    void clear() {
        num_values = 0;
    }
};

void combine_stats(stats &aggregate_stats, stats &new_stats);
// void combine_stats(stats_size &aggregate_stats, stats_size &new_stats);

void combine_4D(std::vector<std::vector<std::vector<stats>>> &v1, std::vector<std::vector<std::vector<stats>>> &v2);
void combine_3D(std::vector<std::vector<stats>> &v1, std::vector<std::vector<stats>> &v2);
void combine_2D(std::vector<stats> &v1, std::vector<stats> &v2);
void combine_1D(stats &v1, stats &v2);

void combine_4D(std::vector<std::vector<std::vector<stats_size>>> &v1, std::vector<std::vector<std::vector<stats_size>>> &v2);
void combine_3D(std::vector<std::vector<stats_size>> &v1, std::vector<std::vector<stats_size>> &v2);
void combine_2D(std::vector<stats_size> &v1, std::vector<stats_size> &v2);
void combine_1D(stats_size &v1, stats_size &v2);


#endif //STATSFUNCTIONS_HH