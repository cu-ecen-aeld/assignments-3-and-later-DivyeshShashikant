#!/bin/bash

# Author: Divyesh Patel	
# Filename: writer.sh

# Check for error if there are invalid arguments
if [ ! $# -eq 2 ]
then 
	echo Invalid Parameters, 2 expected
	exit 1

fi

# First argument is a full path to a file
writefile=$1

# Second argument is text string to be searched within the file
writestr=$2

# Create the path directory
mkdir -p $(dirname $writefile)

# Create new file with name and path
touch $writefile 

# Check for error if file is created or not
if [ -f $writefile ]	
then
	echo "$writestr" > $writefile
	exit 0
else
	echo File Not Created
	exit 1

fi


