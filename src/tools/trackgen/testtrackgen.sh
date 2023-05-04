#!/bin/bash

# the first argument is the path of sd2-trackgen
trackgen=$1
#trackgen=~/speed-dreams-code/release/bin/sd2-trackgen

temp_dir=$(mktemp -d -t trackgen.XXXXXXXX)
pwd=`pwd`

check () {
	$trackgen -c $1 -n $2 $3 -i $pwd/../../../data/tracks/$1/$2 -o $temp_dir -a >& $temp_dir/$2.txt
	status=$?
	if [ $status -eq 0 ]; then
		if  test -f "$pwd/../../../data/tracks/$1/$2/$2.ac"; then
			diff $pwd/../../../data/tracks/$1/$2/$2.ac $temp_dir/$2.ac > $temp_dir/$2.ac.diff
			[ $? -eq 0 ] && echo "$1/$2 GOOD" || echo "$1/$2 BAD"
		else
			echo "$1 $2 no $2.ac file"
		fi
	elif [ $status -eq 3 ]; then
		echo "$1 $2 CRASHED $status"		
		lines=()
		while IFS= read -r line
		do 
			lines+=("$line") 
		done < <(tail -n 3 $temp_dir/$2.txt)
		numLines=${#lines[@]}
		for (( i=0; i<${numLines}; i++ ));
		do
			printf "    %s\n" "${lines[$i]}"
		done
	else
		echo "$1 $2 FAILED $status"
		lines=()
		while IFS= read -r line
		do 
			lines+=("$line") 
		done < <(tail -n 3 $temp_dir/$2.txt)
		numLines=${#lines[@]}
		for (( i=0; i<${numLines}; i++ ));
		do
			printf "    %s\n" "${lines[$i]}"
		done
	fi
}

# add tracks here as they are updated
check circuit tuna
check development no-barrier-testtrack
check development showroom
check development straight-10
check development testtrack
check dirt mixed-2
check road e-track-4
check speedway a-speedway

rm -rf temp_dir
