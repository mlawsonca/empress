/* 
 * Copyright 2018 National Technology & Engineering Solutions of
 * Sandia, LLC (NTESS). Under the terms of Contract DE-NA0003525 with NTESS,
 * the U.S. Government retains certain rights in this software.
 *
 * The MIT License (MIT)
 * 
 * Copyright (c) 2018 Sandia Corporation
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */


#ifndef STATSFUNCTIONS_HH
#define STATSFUNCTIONS_HH

#include <vector>
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <numeric>
#include <algorithm>

// template<class T>
// T get_sum(std::vector<T> v);

template<class T>
T get_max_vv(std::vector<std::vector<T>> vv)  {
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
// 	auto itv = min_element(v.begin(), v.end(), my_find_min); // find the vector               
// 	return *min_element((*itv).begin(), (*itv).end()); 
// }


template<class T>
T get_min_vv(std::vector<std::vector<T>> vv)  {
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
T get_sum(std::vector<T> v) {
    return( std::accumulate(v.begin(), v.end(), 0.0) );
}

template<class T>
T get_mean(std::vector<T> v) {
    T sum = std::accumulate(v.begin(), v.end(), 0.0);
    return (sum / v.size());
}

template<class T>
T get_variance(std::vector<T> v) {
	if (v.size() < 2) {
		return 0;
	}
	T mean = get_mean(v);
    T sum_squared_dif = 0;
    for(T value : v) {
        sum_squared_dif += (value-mean)*(value-mean);
    }
    return sum_squared_dif/(v.size()-1);
}

template<class T>
T get_std_dev(std::vector<T> v) {
    return sqrt(get_variance(v));
}

template<class T>
T get_median(std::vector<T> v)  {
   std::sort( v.begin(), v.end() );

   if (v.size() % 2 == 0) {
      return (v[(v.size() / 2) - 1] + v[v.size() / 2]) / 2.0;
   } 
   return v[v.size() / 2];
}

template<class T>
T get_max(std::vector<T> v)  {
   return *max_element(v.begin(),v.end());
}

template<class T>
T get_min(std::vector<T> v)  {
   return *min_element(v.begin(),v.end());
}



// T get_mean(std::vector<T> v);
// T get_variance(std::vector<T> v);
// T get_std_dev(std::vector<T> v);
// T get_median(std::vector<T> v);
// T get_max(std::vector<T> v);
// T get_min(std::vector<T> v);

// long T get_long_mean(std::vector<long T> v);
// long T get_long_variance(std::vector<long T> v);
// long T get_long_std_dev(std::vector<long T> v);
// long T get_long_median(std::vector<long T> v);
// long T get_long_max(std::vector<long T> v);
// long T get_long_min(std::vector<long T> v);

// uint64_t get_uint64_t_mean(std::vector<uint64_t> v);
// uint64_t get_uint64_t_variance(std::vector<uint64_t> v);
// uint64_t get_uint64_t_std_dev(std::vector<uint64_t> v);
// uint64_t get_uint64_t_median(std::vector<uint64_t> v);
// uint64_t get_uint64_t_max(std::vector<uint64_t> v);
// uint64_t get_uint64_t_min(std::vector<uint64_t> v);

#endif //STATSFUNCTIONS_HH