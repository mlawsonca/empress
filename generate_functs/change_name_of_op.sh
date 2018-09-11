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


debug=true

log ()
{
	if $debug ; then 
		echo "$1" 
	fi
}

echo "Type the current name of the op in upper camel case (minus 'OP') and then press [ENTER]"
read op_name_upper_camel_case

echo "Type the desired name of the op in upper camel case and then press [ENTER]"
read desired_op_name_upper_camel_case

echo "Type the name of the struct you want returned from the function in snake case (e.g., var_attribute ) and then press [ENTER]"
read struct

change_struct=true


op_name_underscores=$(echo $op_name_upper_camel_case | sed -re 's/\B([A-Z])/_\1/g' | tr [:upper:] [:lower:])
op_name_all_caps=${op_name_underscores^^}
op_name_all_caps_no_space=${op_name_all_caps//_}
log $op_name_underscores
log $op_name_all_caps
log $op_name_all_caps_no_space

client_op_name=Op${op_name_upper_camel_case}MetaClient.cpp
server_op_name=Op${op_name_upper_camel_case}MetaServer.cpp
common_op_name=Op${op_name_upper_camel_case}MetaCommon.cpp
common_op_header_name=Op${op_name_upper_camel_case}MetaCommon.hh

new_client_op_name=Op${desired_op_name_upper_camel_case}MetaClient.cpp
new_server_op_name=Op${desired_op_name_upper_camel_case}MetaServer.cpp
new_common_op_name=Op${desired_op_name_upper_camel_case}MetaCommon.cpp
new_common_op_header_name=Op${desired_op_name_upper_camel_case}MetaCommon.hh


log "client op name: $client_op_name"
log "server op name: $server_op_name"
log "common op name: $common_op_name"
log "common header name: $common_op_header_name"


desired_op_name_underscores=$(echo $desired_op_name_upper_camel_case | sed -re 's/\B([A-Z])/_\1/g' | tr [:upper:] [:lower:])
desired_op_name_all_caps=${desired_op_name_underscores^^}
desired_op_name_all_caps_no_space=${desired_op_name_all_caps//_}


log "desired_op_name_underscores: $desired_op_name_underscores"
log "desired_op_name_all_caps: $desired_op_name_all_caps"
log "desired_op_name_all_caps_no_space: $desired_op_name_all_caps_no_space"
log "done"

log "struct: $struct"

if [[ "${struct}" == "" ]]; then
	echo "sticking with current struct"
	change_struct=false
elif [[ "${struct}" != "var_attribute" && "${struct}" != "timestep_attribute" && 
		"${struct}" != "run_attribute" && "${struct}" != "var" && "${struct}" != "type" &&
		"${struct}" != "timestep" && "${struct}" != "run"   ]]; then
		echo "error. the given struct (${struct}) is not an option"
		exit
fi

log "modifying client op"
cd ~/sirius/lib_source/ops/client 
if [ ! -e "$client_op_name" ]; then
	echo "error. $client_op_name does not exists"
	exit
else
	log "found the client op!"

	mv $client_op_name $new_client_op_name
	# s/old-word/new-word/g
	sed -i -e "s/${op_name_upper_camel_case}/${desired_op_name_upper_camel_case}/g" $new_client_op_name
	sed -i -e "s/${op_name_underscores}/${desired_op_name_underscores}/g" $new_client_op_name
	sed -i -e "s/${op_name_all_caps}/${desired_op_name_all_caps}/g" $new_client_op_name
	sed -i -e "s/${op_name_all_caps_no_space}/${desired_op_name_all_caps_no_space}/g" $new_client_op_name

	if $change_struct ; then 
		sed -i "s/struct .#_entry/struct md_catalog_${struct}_entry/g" $new_client_op_name
	fi

	log "word to replace: ${op_name_upper_camel_case}"
	log "word to replace with: ${desired_op_name_upper_camel_case}"
	log "file to replace on: $new_client_op_name "
fi

cd ../server  


cd ~/sirius/lib_source/ops/server 
if [ ! -e "$server_op_name" ]; then
	echo "error. $server_op_name does not exists"
	exit
else
	log "found the server op!"

	mv $server_op_name $new_server_op_name
	sed -i -e "s/${op_name_upper_camel_case}/${desired_op_name_upper_camel_case}/g" $new_server_op_name
	sed -i -e "s/${op_name_underscores}/${desired_op_name_underscores}/g" $new_server_op_name
	sed -i -e "s/${op_name_all_caps}/${desired_op_name_all_caps}/g" $new_server_op_name
	sed -i -e "s/${op_name_all_caps_no_space}/${desired_op_name_all_caps_no_space}/g" $new_server_op_name

	if $change_struct ; then 
		sed -i "s/struct .#_entry/struct md_catalog_${struct}_entry/g" $new_server_op_name
	fi
fi



cd ~/sirius/lib_source/ops/common 
if [ ! -e "$common_op_name" ]; then
	echo "error. $common_op_name does not exists"
	exit
else
	log "found the common op!"

	mv $common_op_name $new_common_op_name
	sed -i -e "s/${op_name_upper_camel_case}/${desired_op_name_upper_camel_case}/g" $new_common_op_name
	sed -i -e "s/${op_name_underscores}/${desired_op_name_underscores}/g" $new_common_op_name
	sed -i -e "s/${op_name_all_caps}/${desired_op_name_all_caps}/g" $new_common_op_name
	sed -i -e "s/${op_name_all_caps_no_space}/${desired_op_name_all_caps_no_space}/g" $new_common_op_name

	if $change_struct ; then 
		sed -i "s/struct .#_entry/struct md_catalog_${struct}_entry/g" $new_common_op_name
	fi
fi

cd ~/sirius/include/ops 
if [ ! -e "$common_op_header_name" ]; then
	echo "error. $common_op_header_name does not exists"
	exit
else
	log "found the common header!"

	mv $common_op_header_name $new_common_op_header_name
	sed -i -e "s/${op_name_upper_camel_case}/${desired_op_name_upper_camel_case}/g" $new_common_op_header_name
	sed -i -e "s/${op_name_underscores}/${desired_op_name_underscores}/g" $new_common_op_header_name
	sed -i -e "s/${op_name_all_caps}/${desired_op_name_all_caps}/g" $new_common_op_header_name
	sed -i -e "s/${op_name_all_caps_no_space}/${desired_op_name_all_caps_no_space}/g" $new_common_op_header_name

	if $change_struct ; then 
		sed -i "s/struct .#_entry/struct md_catalog_${struct}_entry/g" $new_common_op_header_name
	fi
fi


cd ~/sirius/include/ops

sed -i -e "s/#include <$common_op_header_name>/#include <$new_common_op_header_name>/g" "libops.hh"


cd ~/sirius/lib_source
# echo $full_common_ops_path
#REMINDER: SED CAN TAKE ANY CHARACTER AS A DELIMITER, SO IF YOUR VARIABLE CONTAINS SLASHES, USE A
#DIF DELIMETER
sed -i -e "s@ops/common/$common_op_name@ops/common/$new_common_op_name@g" "CMakeLists.txt"
sed -i -e "s@ops/server/$server_op_name@ops/server/$new_server_op_name@g" "CMakeLists.txt"
sed -i -e "s@ops/client/$client_op_name@ops/client/$new_client_op_name@g" "CMakeLists.txt"

cd ~/sirius/my_md_source
sed -i -e "s@opbox::RegisterOp<Op${op_name_upper_camel_case}@opbox::RegisterOp<Op${desired_op_name_upper_camel_case}@g" "my_metadata_server.C"
sed -i -e "s@opbox::RegisterOp<Op${op_name_upper_camel_case}@opbox::RegisterOp<Op${desired_op_name_upper_camel_case}@g" "my_metadata_client.C"

#make the md_client function
md_funct=$(sed -n "/metadata_${op_name_underscores} (/,/end of funct/p" my_metadata_client.C)
# md_funct=$(sed -n "/metadata_${op_name_underscores} (/,/return return_value;/p" my_metadata_client.C)

echo "$md_funct" >> "temp.txt"
# echo -e "}\n" >> "temp.txt"

sed -i -e "s/${op_name_upper_camel_case}/${desired_op_name_upper_camel_case}/g" "temp.txt"
sed -i -e "s/${op_name_underscores}/${desired_op_name_underscores}/g" "temp.txt"
sed -i -e "s/${op_name_all_caps}/${desired_op_name_all_caps}/g" "temp.txt"
sed -i -e "s/${op_name_all_caps_no_space}/${desired_op_name_all_caps_no_space}/g" "temp.txt"

if $change_struct ; then 
	sed -i "s/struct .#_entry/struct md_catalog_${struct}_entry/g" "temp.txt"
fi

#deletes the old md_funct
sed -i "/metadata_${op_name_underscores}/,/end of funct/d" my_metadata_client.C


header=$(sed -n "/metadata_${desired_op_name_underscores} (/,/)/p" temp.txt)
cat "temp.txt" >> "my_metadata_client.C"
rm -f "temp.txt"

#add the declaration for the new md function to the header
cd ~/sirius/include/client
#delete last line
sed -i "/#endif/d" "my_metadata_client.h"
echo "${header};" >> "my_metadata_client.h"
echo "" >> "my_metadata_client.h"
echo "#endif" >> "my_metadata_client.h"

#deletes the declaration for the old function name
sed -i "/metadata_${op_name_underscores}/,/);/d" "my_metadata_client.h"


cd ~/sirius/include/common
#changes the args struct to the correct name
sed -i -e "s/md_${op_name_underscores}_args/md_${desired_op_name_underscores}_args/g" "my_metadata_args.h"


/bin/bash ~/sirius/generate_functs/make_timing_constants.bash

echo "Reminder: Don't forget to adjust the parameters to the client funct, client funct header, and op args"


