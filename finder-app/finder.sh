#!/bin/bash

# Author: Divyesh Patel
# File: finder.sh

# First argument is a path to directory	
filesdir=$1

# Second argument is a text string which is searched within directory
searchstr=$2


# Check for errors if arguments are invalid
if [ ! $# -eq 2 ]
then
	echo Invalid Parameters, $# entered 2 expected
	exit 1

# Check for errors if directory not found
elif [ ! -d $filesdir ]
then
	echo $filesdir Cannot be found
	exit 1

fi


# Number of files in the directory and all subdirectories
FILECOUNT=$(find $filesdir -type f | wc -l)

# Number of matching lines found in respective files
LINESCOUNT=$(grep -r "$searchstr" $filesdir | wc -l)

# print message
echo The number of files are $FILECOUNT and the number of matching lines are $LINESCOUNT 

exit 0
