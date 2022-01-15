#!/bin/bash

if [ ! $# -eq 2 ]
then
	echo Invalid Parameters
	exit 1

elif [ ! -d $1 ]
then
	echo $1 Cannot be found
	exit 1

fi

	echo pass
	FILECOUNT=$(find $1 -type f | wc -l)
	LINESCOUNT=$(grep -r $2 $1 | wc -l)
	echo The number of files are $FILECOUNT and the number of matching lines are $LINESCOUNT 

