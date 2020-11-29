#!/bin/bash

if [ "$#" -ne 5 ] 
	then echo "Wrong count of arguments";
	exit;
fi

if [ ! -f $1 ]
	then echo "wrong diseasesFile " 
	exit 1;
fi
if [ ! -f $2 ]
	then echo "wrong countriesFile " 
	exit 1;
fi
if [ $4 -lt "1" ]
	then echo "wrong numFilesPerDirectory " 
	exit 1;
fi
if [ $5 -lt "1" ]
	then echo "wrong numRecordsPerFile " 
	exit 1;
fi

if [ ! -d $3 ] 
	then mkdir -p $3;
fi;
enterExit="ENTER
EXIT"
diseaseArray=();
while read line; do
	diseaseArray+=($line);
done < $1
diseaseNum=${#diseaseArray[*]} 

en_ex=($enterExit)    
filename=$2
while read line; do
	mkdir -p $3/$line
	ARRAY=()
	for ((i=0; i < $4; i++)) 
	do
		path="$3/$line/$((RANDOM%30+1))-$((RANDOM%12+1))-$((RANDOM%10+2010))";
		touch $path;
		for ((j=0; j < $5; j++))
		do
			count=${#ARRAY[*]}
			
			if [ ${#ARRAY[@]} -eq 0 ]; then
   				second="ENTER"
				#echo "array:"
				#echo "${ARRAY[*]}" 
			else
				second=${en_ex[$((RANDOM%2))]}
			fi
			if [[ "$second" == "EXIT" ]]; then
				#echo $second
				#echo "id from array"
    			id=${ARRAY[$((RANDOM%count))]}
				#echo $id;
			else
				id=$RANDOM;
				while [[ " ${ARRAY[@]} " =~ " ${id} " ]]; do
   					id=$RANDOM;
			done
			fi
			name_size=$(( ( RANDOM % 14 )  + 2 ))
			ARRAY+=($id);
			nameCap=$(cat /dev/urandom | tr -dc 'A-Z' | head -c 1)
			name=$(cat /dev/urandom | tr -dc 'a-z' | head -c $name_size)

			lname_size=$(( ( RANDOM % 14 )  + 2 ))
			lnameCap=$(cat /dev/urandom | tr -dc 'A-Z' | head -c 1)
			lname=$(cat /dev/urandom | tr -dc 'a-z' | head -c $lname_size)
			age=$(( RANDOM % 121 )) 

			echo "$id $second $nameCap$name $lnameCap$lname ${diseaseArray[$((RANDOM%diseaseNum))]} $age"  >> $path
		done
	done
done < $filename


