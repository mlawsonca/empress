#!/bin/bash

debug=true

log ()
{
	if $debug ; then 
		echo "$1" 
	fi
}

echo "Type the name of the new op you want in upper camel case (minus 'OP') and then press [ENTER]"
read op_name_upper_camel_case

echo "Type the name of the op you want it to be copied from in upper camel case. Indicating 'any' for no preference and then press [ENTER]"
read op_to_be_copied_from_upper_camel_case

echo "Type the name of the struct you want returned from the function in snake case (e.g., var_attribute ) and then press [ENTER]"
read struct

change_struct=true

op_name_underscores=$(echo $op_name_upper_camel_case | sed -re 's/\B([A-Z])/_\1/g' | tr [:upper:] [:lower:])
op_name_all_caps=${op_name_underscores^^}
op_name_all_caps_no_space=${op_name_all_caps//_}
log "name with underscores: $op_name_underscores" 
log "name with all caps: $op_name_all_caps" 
log "name with all caps no space: $op_name_all_caps_no_space" 

client_op_name=Op${op_name_upper_camel_case}MetaClient.cpp
server_op_name=Op${op_name_upper_camel_case}MetaServer.cpp
common_op_name=Op${op_name_upper_camel_case}MetaCommon.cpp
common_op_header_name=Op${op_name_upper_camel_case}MetaCommon.hh

log "client op name: $client_op_name"
log "server op name: $server_op_name"
log "common op name: $common_op_name"
log "common header name: $common_op_header_name"

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

if [[ "${op_to_be_copied_from_upper_camel_case,,}" == "any" || "${op_to_be_copied_from_upper_camel_case,,}" == "" ]]; then
	log "using any op"
	op_to_be_copied_from_upper_camel_case="CreateVar"
	op_to_be_copied_from_underscores="create_var"
	op_to_be_copied_from_all_caps="CREATE_VAR"
	op_to_be_copied_from_all_caps_no_space="CREATEVAR"
else
	log "as instructed, looking for op: $op_to_be_copied_from_upper_camel_case"
	op_to_be_copied_from_underscores=$(echo $op_to_be_copied_from_upper_camel_case | sed -re 's/\B([A-Z])/_\1/g' | tr [:upper:] [:lower:])
	op_to_be_copied_from_all_caps=${op_to_be_copied_from_underscores^^}
	op_to_be_copied_from_all_caps_no_space=${op_to_be_copied_from_all_caps//_}
fi

log "op_to_be_copied_from_underscores: $op_to_be_copied_from_underscores"
log "op_to_be_copied_from_all_caps: $op_to_be_copied_from_all_caps"
log "op_to_be_copied_from_all_caps_no_space: $op_to_be_copied_from_all_caps_no_space"
log "done"

log "creating client op"
cd ~/sirius/lib_source/ops/client 
if [ ! -e "Op${op_to_be_copied_from_upper_camel_case}MetaClient.cpp" ]; then
	echo "please indicate a new default op since Op${op_to_be_copied_from_upper_camel_case}MetaClient.cpp does not exist"
	exit
else
	log "found the client!"
	cp Op${op_to_be_copied_from_upper_camel_case}MetaClient.cpp $client_op_name
	# s/old-word/new-word/g
	sed -i -e "s/${op_to_be_copied_from_upper_camel_case}/${op_name_upper_camel_case}/g" $client_op_name
	sed -i -e "s/${op_to_be_copied_from_underscores}/${op_name_underscores}/g" $client_op_name
	sed -i -e "s/${op_to_be_copied_from_all_caps}/${op_name_all_caps}/g" $client_op_name
	sed -i -e "s/${op_to_be_copied_from_all_caps_no_space}/${op_name_all_caps_no_space}/g" $client_op_name
	#changes to new return struct type
	if $change_struct ; then 
		sed -i "s/struct .*_entry/struct md_catalog_${struct}_entry/g" $client_op_name
	fi

	log "word to replace: ${op_to_be_copied_from_upper_camel_case}"
	log "word to replace with: ${op_name_upper_camel_case}"
	log "file to replace on: $client_op_name"
	log "just made file $client_op_name and changed text to use new op name"
	# fi

fi

cd ../server  
if [ ! -e "Op${op_to_be_copied_from_upper_camel_case}MetaServer.cpp" ]; then
	echo "please indicate a new default op since Op${op_to_be_copied_from_upper_camel_case}MetaServer.cpp does not exist"
	exit
else
	log "found the server!"
	cp Op${op_to_be_copied_from_upper_camel_case}MetaServer.cpp $server_op_name
	sed -i -e "s/${op_to_be_copied_from_upper_camel_case}/${op_name_upper_camel_case}/g" $server_op_name
	sed -i -e "s/${op_to_be_copied_from_underscores}/${op_name_underscores}/g" $server_op_name
	sed -i -e "s/${op_to_be_copied_from_all_caps}/${op_name_all_caps}/g" $server_op_name
	sed -i -e "s/${op_to_be_copied_from_all_caps_no_space}/${op_name_all_caps_no_space}/g" $server_op_name

	if $change_struct ; then 
		sed -i "s/struct .*_entry/struct md_catalog_${struct}_entry/g" $server_op_name
	fi
fi

cd ../common  
if [ ! -e "Op${op_to_be_copied_from_upper_camel_case}MetaCommon.cpp" ]; then
	echo "please indicate a new default op since Op${op_to_be_copied_from_upper_camel_case}MetaCommon.cpp does not exist"
	exit
else
	log "found the common!"
	cp Op${op_to_be_copied_from_upper_camel_case}MetaCommon.cpp $common_op_name
	sed -i -e "s/${op_to_be_copied_from_upper_camel_case}/${op_name_upper_camel_case}/g" $common_op_name
	sed -i -e "s/${op_to_be_copied_from_underscores}/${op_name_underscores}/g" $common_op_name
	sed -i -e "s/${op_to_be_copied_from_all_caps}/${op_name_all_caps}/g" $common_op_name
	sed -i -e "s/${op_to_be_copied_from_all_caps_no_space}/${op_name_all_caps_no_space}/g" $common_op_name

	if $change_struct ; then 
		sed -i "s/struct .*_entry/struct md_catalog_${struct}_entry/g" $common_op_name
	fi
fi

cd ~/sirius/include/ops 
if [ ! -e "Op${op_to_be_copied_from_upper_camel_case}MetaCommon.hh" ]; then
	log "please indicate a new default op since Op${op_to_be_copied_from_upper_camel_case}MetaCommon.hh does not exist"
	exit
else
	log "found the common header!"
	cp Op${op_to_be_copied_from_upper_camel_case}MetaCommon.hh $common_op_header_name
	sed -i -e "s/${op_to_be_copied_from_upper_camel_case}/${op_name_upper_camel_case}/g" $common_op_header_name
	sed -i -e "s/${op_to_be_copied_from_underscores}/${op_name_underscores}/g" $common_op_header_name
	sed -i -e "s/${op_to_be_copied_from_all_caps}/${op_name_all_caps}/g" $common_op_header_name
	sed -i -e "s/${op_to_be_copied_from_all_caps_no_space}/${op_name_all_caps_no_space}/g" $common_op_header_name

	if $change_struct ; then 
		sed -i "s/struct .*_entry/struct md_catalog_${struct}_entry/g" $common_op_header_name
	fi
fi


# cd ~/sirius/include/ops

sed -i -e "s/#endif/#include <$common_op_header_name>\\n#endif/g" "libops.hh"


cd ~/sirius/lib_source
# echo $full_common_ops_path
#REMINDER: SED CAN TAKE ANY CHARACTER AS A DELIMITER, SO IF YOUR VARIABLE CONTAINS SLASHES, USE A
#DIF DELIMETER
sed -i -e "s@add_library(OpsCommon@add_library(OpsCommon\\n\\tops/common/$common_op_name@g" "CMakeLists.txt"
sed -i -e "s@add_library(OpsServer@add_library(OpsServer\\n\\tops/server/$server_op_name@g" "CMakeLists.txt"
sed -i -e "s@add_library(OpsClient@add_library(OpsClient\\n\\tops/client/$client_op_name@g" "CMakeLists.txt"


cd ~/sirius/my_md_source
sed -i -e "s@int register_ops () {@int register_ops () {\\n\\topbox::RegisterOp<Op${op_name_upper_camel_case}Meta>();@g" "my_metadata_server.C"
sed -i -e "s@int register_ops () {@int register_ops () {\\n\\topbox::RegisterOp<Op${op_name_upper_camel_case}Meta>();@g" "my_metadata_client.C"

#make the md_client function
# md_funct=$(sed -n "/metadata_${op_to_be_copied_from_underscores} (/,/return return_value;/p" my_metadata_client.C)
md_funct=$(sed -n "/metadata_${op_to_be_copied_from_underscores} (/,/end of funct/p" my_metadata_client.C)

echo "$md_funct" >> "temp.txt"
# echo -e "}\n" >> "temp.txt"

sed -i -e "s/${op_to_be_copied_from_upper_camel_case}/${op_name_upper_camel_case}/g" "temp.txt"
sed -i -e "s/${op_to_be_copied_from_underscores}/${op_name_underscores}/g" "temp.txt"
sed -i -e "s/${op_to_be_copied_from_all_caps}/${op_name_all_caps}/g" "temp.txt"
sed -i -e "s/${op_to_be_copied_from_all_caps_no_space}/${op_name_all_caps_no_space}/g" "temp.txt"
if $change_struct ; then 
	sed -i "s/struct .*_entry/struct md_catalog_${struct}_entry/g" "temp.txt"
fi

header=$(sed -n "/metadata_${op_name_underscores} (/,/)/p" temp.txt)
cat "temp.txt" >> "my_metadata_client.C"
echo "" >> "my_metadata_client.C"
rm -f "temp.txt"

#add the declaration for the new md function to the header
cd ~/sirius/include/client
#delete last line
sed -i "/#endif/d" "my_metadata_client.h"
echo "${header};" >> "my_metadata_client.h"
echo "" >> "my_metadata_client.h"
echo "#endif" >> "my_metadata_client.h"


#adds a copy of the args for the new op
cd ~/sirius/include/common
md_args=$(sed -n "/md_${op_to_be_copied_from_underscores}_args/,/};/p" my_metadata_args.h)
echo "$md_args" >> "temp.txt"
sed -i -e "s/${op_to_be_copied_from_underscores}/${op_name_underscores}/g" "temp.txt"

sed -i "\@#endif //MYMETADATA_ARGS_H@d" "my_metadata_args.h"
cat "temp.txt" >> "my_metadata_args.h"
echo "" >> "my_metadata_args.h"
echo "#endif //MYMETADATA_ARGS_H" >> "my_metadata_args.h"

rm -f "temp.txt"


/bin/bash ~/sirius/generate_functs/make_timing_constants.bash


echo "Reminder: Don't forget to adjust the parameters to the client funct, client funct header, and op args"

# echo "Reminder: You still need to make a md_client function and add it to the md_client's header"
# echo "Reminder: Don't forget to copy the new timing constants over and to make the needed args"



