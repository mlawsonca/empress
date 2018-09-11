#!/bin/bash

# Copyright 2018 National Technology & Engineering Solutions of
# Sandia, LLC (NTESS). Under the terms of Contract DE-NA0003525 with NTESS,
# the U.S. Government retains certain rights in this software.
#
# The MIT License (MIT)
# 
# Copyright (c) 2018 Sandia Corporation
# 
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
#

debug=false

log ()
{
	if $debug ; then 
		echo "$1" 
	fi
}

echo "Type the name of the new op you want to delete in upper camel case (minus 'OP') and then press [ENTER]"
read op_name_to_delete_upper_camel_case

echo "Are you sure you want to delete $op_name_to_delete_upper_camel_case? Type yes or y to continue deleting"
read answer

if [[ "${answer,,}" != "yes" && "${answer,,}" != "y" ]]; then
	echo "You have decided to abort deleting"
	exit
fi

op_name_to_delete_underscores=$(echo $op_name_to_delete_upper_camel_case | sed -re 's/\B([A-Z])/_\1/g' | tr [:upper:] [:lower:])
op_name_to_delete_all_caps=${op_name_to_delete_upper_camel_case^^}
op_name_to_delete_all_caps_no_space=${op_name_to_delete_all_caps//_}
log $op_name_to_delete_underscores
log $op_name_to_delete_all_caps
log $op_name_to_delete_all_caps_no_space

client_op_name=Op${op_name_to_delete_upper_camel_case}MetaClient.cpp
server_op_name=Op${op_name_to_delete_upper_camel_case}MetaServer.cpp
common_op_name=Op${op_name_to_delete_upper_camel_case}MetaCommon.cpp
common_op_header_name=Op${op_name_to_delete_upper_camel_case}MetaCommon.hh

log "client op name: $client_op_name"
log "server op name: $server_op_name"
log "common op name: $common_op_name"
log "common header name: $common_op_header_name"

log "deleting client op"
cd ~/sirius/lib_source/ops/client 
if [ -e $client_op_name ]; then
	rm -f $client_op_name
# else 
# 	echo "client op was not found"
# 	exit
fi

cd ../server  
if [ -e $server_op_name ]; then
	rm -f $server_op_name
# else 
# 	echo "server op was not found"
# 	exit
fi

cd ../common  
if [ -e $common_op_name ]; then
	rm -f $common_op_name
# else 
# 	echo "common op was not found"
# 	exit
fi

cd ~/sirius/include/ops 
if [ -e $common_op_header_name ]; then
	rm -f $common_op_header_name
# else 
# 	echo "common op header was not found"
# 	exit
fi


sed -i "/#include <$common_op_header_name>/d" "libops.hh"

cd ~/sirius/lib_source
# echo $full_common_ops_path
#REMINDER: SED CAN TAKE ANY CHARACTER AS A DELIMITER, SO IF YOUR VARIABLE CONTAINS SLASHES, USE A
#DIF DELIMETER
sed -i "/$op_name_to_delete_upper_camel_case/d" "CMakeLists.txt"

cd ~/sirius/my_md_source
sed -i "/opbox::RegisterOp<Op${op_name_to_delete_upper_camel_case}Meta>();/d" "my_metadata_server.C"
sed -i "/opbox::RegisterOp<Op${op_name_to_delete_upper_camel_case}Meta>();/d" "my_metadata_client.C"
echo "trying to delete line that contains opbox::RegisterOp<Op${op_name_to_delete_upper_camel_case}Meta>();"

#deletes the md_funct
sed -i "/metadata_${op_name_to_delete_underscores}/,/end of funct/d" my_metadata_client.C

cd ~/sirius/include/client
#deletes the declaration for md client function 
sed -i "/metadata_${op_name_to_delete_underscores}/,/);/d" "my_metadata_client.h"


cd ~/sirius/include/common
#deletes the args struct
sed -i "/md_${op_name_to_delete_underscores}_args/,/};/d" "my_metadata_args.h"


/bin/bash ~/sirius/generate_functs/make_timing_constants.bash


