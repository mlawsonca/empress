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

#include "stats_functions.hh"
// #include <stdio.h>
#include <math.h>
#include <stdint.h>
#include <numeric>
#include <algorithm>
// #include <sys/types.h>
#include "stats_functions.hh"

using namespace std;

//common functions used by dirman, server and client analysis:
double get_mean(vector<double> v) {
    double sum = std::accumulate(v.begin(), v.end(), 0.0);
    return (sum / v.size());
}

double get_variance(vector<double> v) {
	double mean = get_mean(v);
    double sum_squared_dif = 0;
    for(double value : v) {
        sum_squared_dif += (value-mean)*(value-mean);
    }
    return sum_squared_dif/(v.size()-1);
}

double get_std_dev(vector<double> v) {
    return sqrt(get_variance(v));
}


double get_median(vector<double> v)  {
   std::sort( v.begin(), v.end() );

   if (v.size() % 2 == 0) {
      return (v[(v.size() / 2) - 1] + v[v.size() / 2]) / 2.0;
   } 
   return v[v.size() / 2];
}

double get_max(vector<double> v)  {
   return *max_element(v.begin(),v.end());
}

double get_min(vector<double> v)  {
   return *min_element(v.begin(),v.end());
}

//common functions used by dirman, server and client analysis:
long double get_long_mean(vector<long double> v) {
    long double sum = std::accumulate(v.begin(), v.end(), 0.0);
    return (sum / v.size());
}

long double get_long_variance(vector<long double> v) {
	long double mean = get_long_mean(v);
    long double sum_squared_dif = 0;
    for(double value : v) {
        sum_squared_dif += (value-mean)*(value-mean);
    }
    return sum_squared_dif/(v.size()-1);
}

long double get_long_std_dev(vector<long double> v) {
    return sqrt(get_long_variance(v));
}


long double get_long_median(vector<long double> v)  {
   std::sort( v.begin(), v.end() );

   if (v.size() % 2 == 0) {
      return (v[(v.size() / 2) - 1] + v[v.size() / 2]) / 2.0;
   } 
   return v[v.size() / 2];
}

long double get_long_max(vector<long double> v)  {
   return *max_element(v.begin(),v.end());
}

long double get_long_min(vector<long double> v)  {
   return *min_element(v.begin(),v.end());
}


//common functions used by dirman, server and client analysis:
uint64_t get_uint64_t_mean(vector<uint64_t> v) {
    uint64_t sum = std::accumulate(v.begin(), v.end(), 0.0);
    return (sum / v.size());
}

uint64_t get_uint64_t_variance(vector<uint64_t> v) {
  uint64_t mean = get_uint64_t_mean(v);
    uint64_t sum_squared_dif = 0;
    for(uint64_t value : v) {
        sum_squared_dif += (value-mean)*(value-mean);
    }
    return sum_squared_dif/(v.size()-1);
}

uint64_t get_uint64_t_std_dev(vector<uint64_t> v) {
    return sqrt(get_uint64_t_variance(v));
}


uint64_t get_uint64_t_median(vector<uint64_t> v)  {
   std::sort( v.begin(), v.end() );

   if (v.size() % 2 == 0) {
      return (v[(v.size() / 2) - 1] + v[v.size() / 2]) / 2.0;
   } 
   return v[v.size() / 2];
}

uint64_t get_uint64_t_max(vector<uint64_t> v)  {
   return *max_element(v.begin(),v.end());
}

uint64_t get_uint64_t_min(vector<uint64_t> v)  {
   return *min_element(v.begin(),v.end());
}