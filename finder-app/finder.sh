#!/bin/bash

# Check for errors if arguments are invalid
if [ ! $# -eq 2 ]
then
	echo Invalid Parameters
	exit 1

# Check for errors if directory not found
elif [ ! -d $1 ]
then
	echo $1 Cannot be found
	exit 1

fi

	echo pass

# Number of files in the directory and all subdirectories
FILECOUNT=$(find $1 -type f | wc -l)

# Number of matching lines found in respective files
LINESCOUNT=$(grep -r $2 $1 | wc -l)

# print message
echo The number of files are $FILECOUNT and the number of matching lines are $LINESCOUNT 

