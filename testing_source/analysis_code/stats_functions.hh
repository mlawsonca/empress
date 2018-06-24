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

double get_mean(std::vector<double> v);
double get_variance(std::vector<double> v);
double get_std_dev(std::vector<double> v);
double get_median(std::vector<double> v);
double get_max(std::vector<double> v);
double get_min(std::vector<double> v);

long double get_long_mean(std::vector<long double> v);
long double get_long_variance(std::vector<long double> v);
long double get_long_std_dev(std::vector<long double> v);
long double get_long_median(std::vector<long double> v);
long double get_long_max(std::vector<long double> v);
long double get_long_min(std::vector<long double> v);

uint64_t get_uint64_t_mean(std::vector<uint64_t> v);
uint64_t get_uint64_t_variance(std::vector<uint64_t> v);
uint64_t get_uint64_t_std_dev(std::vector<uint64_t> v);
uint64_t get_uint64_t_median(std::vector<uint64_t> v);
uint64_t get_uint64_t_max(std::vector<uint64_t> v);
uint64_t get_uint64_t_min(std::vector<uint64_t> v);

#endif //STATSFUNCTIONS_HH