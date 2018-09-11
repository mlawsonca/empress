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

write_timing_constants() {

	log "parameter 1 is $1"

	sed -i '/{/q' $1

	# sed -i '/collective calls/q' client_timing_constants_read.hh

	err_timing_constant=10000
	num_since_line_break=0
	for code in ${Err_Codes[@]}; do

		cat >>$1 <<-EOL
		    ${code} = ${err_timing_constant},
		EOL
	    err_timing_constant=`expr $err_timing_constant + 1`
	    num_since_line_break=`expr $num_since_line_break + 1`
		if (( $num_since_line_break%10 >= 6 )) ; then
		# echo $timing_constant
		# echo $code
		# err_timing_constant=`expr '(' $err_timing_constant + 9 ')' / 10 '*' 10`
			num_since_line_break=0
	#  		cat >>$1 <<-EOL
			
		# EOL
			echo "" >> $1

		fi
	done
	# cat >>$1 <<-EOL
		
	# EOL
	echo "" >> $1

	timing_constant=0
	num_since_line_break=0
	for code in ${Reg_Codes[@]}; do

		cat >>$1 <<-EOL
		    ${code} = ${timing_constant},
		EOL
	    timing_constant=`expr $timing_constant + 1`
	    num_since_line_break=`expr $num_since_line_break + 1`
	    if [[ $code == *"DONE"* ]]; then
	    	# && $(( $timing_constant%10 )) >= 5 ]]
	    	if (( $num_since_line_break%10 >= 6 )) ; then
	    		# timing_constant=`expr '(' $timing_constant + 9 ')' / 10 '*' 10`
	   #  		cat >>$1 <<-EOL
	   			num_since_line_break=0
					
				# EOL
				echo "" >> $1

	    	fi
	    fi

	done
	echo "" >> $1

	for code in ${Collective_Op_Codes[@]}; do
		end_code="${code/START/DONE}"
		# echo "end_code: $end_code"
		cat >>$1 <<-EOL
		    ${code} = ${timing_constant},
		    ${end_code} = `expr ${timing_constant} + 1`,
		EOL
	    timing_constant=`expr $timing_constant + 2`
	    num_since_line_break=`expr $num_since_line_break + 2`
    	# && $(( $timing_constant%10 )) >= 5 ]]
    	if (( $num_since_line_break%10 >= 6 )) ; then
    		# timing_constant=`expr '(' $timing_constant + 9 ')' / 10 '*' 10`
   #  		cat >>$1 <<-EOL
   			num_since_line_break=0
				
			# EOL
			echo "" >> $1

    	fi

	done
	echo "" >> $1


	special_count=${#Special_Codes[@]}
	last_code=9999
	spec_code=$(( last_code-special_count+1 ))

	for code in ${Special_Codes[@]}; do

		cat >>$1 <<-EOL
		    ${code} = ${spec_code},
		EOL
	    spec_code=`expr $spec_code + 1`
	done


	md_timing_constant=1000
	for code in ${Op_Codes_Sorted[@]}; do
		end_code="${code/START/DONE}"
		# echo "end_code: $end_code"
		cat >>$1 <<-EOL
		    ${code} = ${md_timing_constant},
		    ${end_code} = `expr ${md_timing_constant} + 1`,
		EOL
		# if [[ $code == "MD"*"DONE" ]]; then
			md_timing_constant=`expr '(' $md_timing_constant + 100 ')' / 100 '*' 100`
			echo "" >> $1
		# else
	    	# md_timing_constant=`expr $md_timing_constant + 1`
	    # fi
	done
	echo "" >> $1


	cat >>$1 <<-EOL
	};

	#endif 

	EOL
}



# filename="/ascldap/users/mlawso/sirius/testing_source/testing_harness_new_write.cpp"

# for filename in ${testing_harness_file_names[@]}; do
# 	echo $filename
# done

# for filename in ${timing_constant_file_names[@]}; do
# 	echo $filename
# done

# echo ${testing_harness_file_names[0]}

find_timing_points() {
	while read -r line; do
	    if [[ $line == "add_timing_point"* && ! $line == *"{"* ]] ; then
	    	if [[ $line == [A-Z] ]] ; then
			    echo yes
			fi

			code=${line#*(}
			code=${code%)*}
			code_all_caps=${code^^}
			if [[ $code_all_caps != $code ]]; then
				echo "found a problem $code"
				continue
			fi

			# echo $code

	        if [[ $code == *"ERR"* ]]; then
	        	if [[ ! " ${Err_Codes[@]} " =~ " ${code} "  ]]; then
	        		eval Err_Codes=("${Err_Codes[@]}" $code)
	        	else
	        		echo "found dupl $code" 
	        	fi

	        elif [[ $code == "BOUNDING_BOX_TO_OBJ_NAMES" ]]; then
	         	if [[ ! " ${Special_Codes[@]} " =~ " ${code} " ]]; then
	        		eval Special_Codes=("${Special_Codes[@]}" $code)
	        		log "found special code: $code"
	        	else
	        		echo "found dupl $code" 
	        	fi   

	        elif [[ $code == "MD_"* ]]; then
	      		if [[ $code != *"DONE" ]]; then
		         	if [[ ! " ${Op_Codes[@]} " =~ " ${code} " ]]; then
		        		eval Op_Codes=("${Op_Codes[@]}" $code)
		        		log "found op code: $code"
		        	else
		        		echo "found dupl $code" 
		        	fi  
		        fi 
	        elif [[ $code == "CATALOG_"*"COLLECTIVE"* ]]; then
	      		if [[ $code != *"DONE" ]]; then
		         	if [[ ! " ${Collective_Op_Codes[@]} " =~ " ${code} " ]]; then
		        		eval Collective_Op_Codes=("${Collective_Op_Codes[@]}" $code)
		        		log "found op code: $code"
		        	else
		        		echo "found dupl $code" 
		        	fi  
		        fi 
	        else 
	         	if [[ ! " ${Reg_Codes[@]} " =~ " ${code} " ]]; then
	        		eval Reg_Codes=("${Reg_Codes[@]}" $code)
	        	else
	        		echo "found dupl $code"
	        	fi          	
	        fi


	        # fi
	    fi
	done < "$filename"
}

debug=false

log ()
{
    if $debug ; then 
        echo "$1" 
    fi
}

# cd ~/sirius/generate_functs

# new_op_name=false


testing_harness_file_names=(
	"/ascldap/users/mlawso/sirius/testing_source/testing_harness_new_write.cpp"
	"/ascldap/users/mlawso/sirius/testing_source/hdf5_helper_functions_write.cpp"
	"/ascldap/users/mlawso/sirius/testing_source_hdf5/testing_harness_write_hdf5.cpp"
	"/ascldap/users/mlawso/sirius/testing_source_hdf5/testing_harness_read_hdf5.cpp"
	"/ascldap/users/mlawso/sirius/testing_source_hdf5/3d_read_for_testing_hdf5.cpp"
	"/ascldap/users/mlawso/sirius/testing_source_hdf5/extra_testing_collective_hdf5.cpp"
	"/ascldap/users/mlawso/sirius/testing_source_hdf5/extra_testing_collective_helper_functions_hdf5.cpp"
	"/ascldap/users/mlawso/sirius/testing_source_hdf5/testing_harness_helper_functions_hdf5.cpp"
	"/ascldap/users/mlawso/sirius/testing_source_hdf5/my_metadata_client_hdf5.cpp"
	# "/ascldap/users/mlawso/sirius/testing_source/testing_harness_new_read.cpp"
	# "/ascldap/users/mlawso/sirius/testing_source/3d_read_for_testing.cpp"
	# "/ascldap/users/mlawso/sirius/testing_source/extra_testing_collective.cpp"
	# "/ascldap/users/mlawso/sirius/testing_source/extra_testing_collective_helper_functions.cpp"
	)

timing_constant_file_names=("/ascldap/users/mlawso/sirius/include/client/client_timing_constants_write.hh"
	"/ascldap/users/mlawso/sirius/include/hdf5/client_timing_constants_write_hdf5.hh"
	"/ascldap/users/mlawso/sirius/include/hdf5/client_timing_constants_read_hdf5.hh"	
	"/ascldap/users/mlawso/sirius/include/hdf5/md_timing_constants_hdf5.hh"
	# "/ascldap/users/mlawso/sirius/include/client/client_timing_constants_read.hh"		
	)

# num_to_eval_simult=(2 1 4 1 4)
num_to_eval_simult=(2 1 5 1)

# for filename in ${testing_harness_file_names[@]}; do

constant_file_names_indx=0
harness_file_names_indx=0
while [ $constant_file_names_indx -lt ${#timing_constant_file_names[@]} ]; do
	# echo $filename

	Err_Codes=()
	Special_Codes=()
	Reg_Codes=()
	Op_Codes=()
	Collective_Op_Codes=()

	iter=0
	while [ $iter -lt ${num_to_eval_simult[$constant_file_names_indx]} ]; do
		filename=${testing_harness_file_names[$harness_file_names_indx]}

		if [[ $filename == *"/testing_harness"* && $filename != *"helper"* ]]; then
			log "testing harness file name: $filename" 
			eval Err_Codes=("${Err_Codes[@]}" "ERR_ARGC")
		else 
			log "NOT testing harness file name: $filename" 
		fi

		iter=$(expr $iter + 1)
		harness_file_names_indx=$(expr $harness_file_names_indx + 1)
		log "constant_file_names_indx: $constant_file_names_indx iter: $iter"

		# find_timing_points $filename Err_Codes Special_Codes Reg_Codes
		find_timing_points

		log "filename: $filename"
		log "Err_Codes: ${Err_Codes[*]}"
		log "Special_Codes: ${Special_Codes[*]}"
		log "Reg_Codes: ${Reg_Codes[*]}"
		log "Op_Codes: ${Op_Codes[*]}"
		log ""

	done

	#While loop to read line by line

	timing_file_name=${timing_constant_file_names[constant_file_names_indx]}
	log $timing_file_name

	Op_Codes_Sorted=($(for l in ${Op_Codes[@]}; do echo $l; done | sort))
	Collective_Op_Codes_Sorted=($(for l in ${Collective_Op_Codes[@]}; do echo $l; done | sort))

	write_timing_constants $timing_file_name

	constant_file_names_indx=$(expr $constant_file_names_indx + 1)
done


# echo "Your new timing constants have been generated in md_client_timing_constants.hh and server_timing_constants.hh. Copy and paste them into the include constants headers"





