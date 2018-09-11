#!/bin/bash

filename="/ascldap/users/mlawso/sirius/lib_source/CMakeLists.txt"

debug=false

log ()
{
    if $debug ; then 
        echo "$1" 
    fi
}

cd ~/sirius/generate_functs

new_op_name=false

Attr_Names=()
Var_Names=()
Other_Names=()

#While loop to read line by line
while read -r line; do
    # log $line
    if [[ $line == "add_library(OpsCommon" ]] ; then
        log "new_op_name is true"
        new_op_name=true
    elif [[ $line == ")" && $new_op_name == true ]] ; then
        log "done"
        new_op_name=false
        # exit
    elif [[ $new_op_name == true ]] ; then
        if [[ ! $line == *"#"* ]] ; then
            line=${line#ops/common/Op}
            line=${line%MetaCommon.cpp}    
            line=$(echo $line | sed -re 's/\B([A-Z])/_\1/g' | tr [:lower:] [:upper:])
            log $line

            if [[ $line == *"ATTR"* || $line == *"TYPE"* ]]; then
                log "Op is for types/attrs"
                Attr_Names=("${Attr_Names[@]}" $line)
            elif [[ $line == *"RUN"* || $line == *"VAR"* || $line == *"TIMESTEP"* ]]; then
                log "Op is for vars/datasets"
                Var_Names=("${Var_Names[@]}" $line)
            elif [[ $line == *"SHUTDOWN"* ]]; then
                log "Var is neither types/attrs nor vars/datasets"
                Other_Names=("${Other_Names[@]}" $line)
            else 
            	log "Op is for all types so have assigned to var names"
                Var_Names=("${Var_Names[@]}" $line)
            fi


        fi
    fi
done < "$filename"

# log ${Attr_Names[@]}
# log ${Var_Names[@]}
# log ${Other_Names[@]}

Attr_Names_Sorted=($(for l in ${Attr_Names[@]}; do echo $l; done | sort))
Var_Names_Sorted=($(for l in ${Var_Names[@]}; do echo $l; done | sort))
Other_Names_Sorted=($(for l in ${Other_Names[@]}; do echo $l; done | sort))

# log ${Attr_Names_Sorted[@]}
# log ${Var_Names_Sorted[@]}
# log ${Other_Names_Sorted[@]}

# log ${#Attr_Names_Sorted[@]}
# log ${#Var_Names_Sorted[@]}
# log ${#Other_Names_Sorted[@]}

cd ~/sirius/include/client
sed -i '/server side is/q' md_client_timing_constants.hh

sed -i '/collective calls/q' client_timing_constants_read.hh

starting_var_constant=1000
for l in ${Var_Names_Sorted[@]}; do

# cat >>md_client_timing_constants.hh <<EOL
	cat >>md_client_timing_constants.hh <<-EOL
	//
	    MD_${l}_START = ${starting_var_constant},
	    // OP_${l}_START = `expr ${starting_var_constant} + 1`,
	    OP_${l}_SERIALIZE_MSG_FOR_SERVER = `expr ${starting_var_constant} + 2`,
	    // OP_${l}_CREATE_MSG_FOR_SERVER = `expr ${starting_var_constant} + 3`,
	    OP_${l}_SEND_MSG_TO_SERVER = `expr ${starting_var_constant} + 4`,
	    OP_${l}_RETURN_MSG_RECEIVED_FROM_SERVER = `expr ${starting_var_constant} + 5`,
	    // OP_${l}_PROMISE_VAL_SET_OP_DONE = `expr ${starting_var_constant} + 6`, 
	    MD_${l}_DEARCHIVE_RESPONSE_OP_DONE = `expr ${starting_var_constant} + 7`,

	EOL
    starting_var_constant=`expr $starting_var_constant + 100`
done

# starting_var_constant=`expr '(' $starting_var_constant + 900 ')' / 1000 '*' 1000`

for l in ${Attr_Names_Sorted[@]}; do
# cat >>md_client_timing_constants.hh <<EOL
	cat >>md_client_timing_constants.hh <<-EOL
	//
	    MD_${l}_START = ${starting_var_constant},
	    // OP_${l}_START = `expr ${starting_var_constant} + 1`,
	    OP_${l}_SERIALIZE_MSG_FOR_SERVER = `expr ${starting_var_constant} + 2`,
	    // OP_${l}_CREATE_MSG_FOR_SERVER = `expr ${starting_var_constant} + 3`,
	    OP_${l}_SEND_MSG_TO_SERVER = `expr ${starting_var_constant} + 4`,
	    OP_${l}_RETURN_MSG_RECEIVED_FROM_SERVER = `expr ${starting_var_constant} + 5`,
	    // OP_${l}_PROMISE_VAL_SET_OP_DONE = `expr ${starting_var_constant} + 6`, 
	    MD_${l}_DEARCHIVE_RESPONSE_OP_DONE = `expr ${starting_var_constant} + 7`,

	EOL

	if [[ $l == "CATALOG"*"ATTR"* ]]; then
		if [[ $l == *"RANGE"* ]]; then
			l_max="${l/RANGE/ABOVE_MAX}"
			l_min="${l/RANGE/BELOW_MIN}"

			cat >>client_timing_constants_read.hh <<-EOL
			//
			    ${l}_COLLECTIVE_START = `expr ${starting_var_constant} + 10030`,
			    ${l}_COLLECTIVE_DONE = `expr ${starting_var_constant} + 10031`,

			//
			    ${l_max}_COLLECTIVE_START = `expr ${starting_var_constant} + 10040`,
			    ${l_max}_COLLECTIVE_DONE = `expr ${starting_var_constant} + 10041`,		

			//
			    ${l_min}_COLLECTIVE_START = `expr ${starting_var_constant} + 10050`,
			    ${l_min}_COLLECTIVE_DONE = `expr ${starting_var_constant} + 10051`,
			EOL
		else
			cat >>client_timing_constants_read.hh <<-EOL
			//
			    ${l}_COLLECTIVE_START = `expr ${starting_var_constant} + 10030`,
			    ${l}_COLLECTIVE_DONE = `expr ${starting_var_constant} + 10031`,
			EOL
		fi
	fi
	starting_var_constant=`expr $starting_var_constant + 100`
done


# starting_var_constant=`expr '(' $starting_var_constant + 900 ')' / 1000 '*' 1000`
for l in ${Other_Names_Sorted[@]}; do
	cat >>md_client_timing_constants.hh <<-EOL
	//
	    MD_${l}_START = ${starting_var_constant},
	    // OP_${l}_START = `expr ${starting_var_constant} + 1`,
	    // OP_${l}_CREATE_MSG_FOR_SERVER = `expr ${starting_var_constant} + 3`,
	    // OP_${l}_SEND_MSG_TO_SERVER_OP_DONE = `expr ${starting_var_constant} + 4`,
	    MD_${l}_OP_DONE = `expr ${starting_var_constant} + 7`,

	EOL
	starting_var_constant=`expr $starting_var_constant + 100`
done

# cat >>md_client_timing_constants.hh <<EOL
cat >>md_client_timing_constants.hh <<EOL
};

#endif //CLIENTTIMINGCONSTANTS_HH

EOL

# cat >>md_client_timing_constants.hh <<EOL
cat >>client_timing_constants_read.hh <<EOL
};

#endif //CLIENTTIMINGCONSTANTSREAD_HH

EOL


cd ~/sirius/include/server
sed -i '/client side is/q' server_timing_constants.hh

starting_var_constant=1050
for l in ${Var_Names_Sorted[@]}; do

cat >>server_timing_constants.hh <<EOL
//
    OP_${l}_START = ${starting_var_constant},
    OP_${l}_DEARCHIVE_MSG_FROM_CLIENT = `expr ${starting_var_constant} + 1`,
    OP_${l}_MD_${l}_STUB = `expr ${starting_var_constant} + 2`,
    OP_${l}_SERIALIZE_MSG_FOR_CLIENT = `expr ${starting_var_constant} + 3`,
    OP_${l}_CREATE_MSG_FOR_CLIENT = `expr ${starting_var_constant} + 4`,
    OP_${l}_SEND_MSG_TO_CLIENT_OP_DONE = `expr ${starting_var_constant} + 5`,

EOL
    starting_var_constant=`expr $starting_var_constant + 100`
done

# starting_var_constant=`expr '(' $starting_var_constant + 900 ')' / 1000 '*' 1000 + 50`

for l in ${Attr_Names_Sorted[@]}; do
	cat >>server_timing_constants.hh <<-EOL
	//
	    OP_${l}_START = ${starting_var_constant},
	    OP_${l}_DEARCHIVE_MSG_FROM_CLIENT = `expr ${starting_var_constant} + 1`,
	    OP_${l}_MD_${l}_STUB = `expr ${starting_var_constant} + 2`,
	    OP_${l}_SERIALIZE_MSG_FOR_CLIENT = `expr ${starting_var_constant} + 3`,
	    OP_${l}_CREATE_MSG_FOR_CLIENT = `expr ${starting_var_constant} + 4`,
	    OP_${l}_SEND_MSG_TO_CLIENT_OP_DONE = `expr ${starting_var_constant} + 5`,

	EOL
    starting_var_constant=`expr $starting_var_constant + 100`
done



cat >>server_timing_constants.hh <<EOL
};

#endif //SERVERTIMINGCONSTANTS_HH

EOL




# echo "Your new timing constants have been generated in md_client_timing_constants.hh and server_timing_constants.hh. Copy and paste them into the include constants headers"





