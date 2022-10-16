#!/bin/bash

# the first argument is the path of sd2-trackgen
trackgen=$1
#trackgen=~/speed-dreams-code/release/bin/sd2-trackgen

temp_dir=$(mktemp -d)
pwd=`pwd`

check () {
	$trackgen -c $1 -n $2 $3 -i $pwd/../../../data/tracks/$1/$2 -o $temp_dir -a >& /dev/null

	diff $pwd/../../../data/tracks/$1/$2/$2.ac $temp_dir/$2.ac > /dev/null

	status=$?
	[ $status -eq 0 ] && echo "$2 good" || echo "$2 bad"
}

# add tracks here as they are updated
check circuit tuna
check development no-barrier-testtrack
check development showroom
check development straight-10
check development testtrack
check dirt mixed-2
check road e-track-4 -B
check speedway a-speedway

rm -rf temp_dir
