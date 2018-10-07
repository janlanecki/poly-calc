#!/bin/bash

START="START"
STOP=0

if [[ $# -eq 2 ]]; then
	if [[ ! -e $1 ]] || [[ ! -x $1 ]]; then
		echo -e "$1 is not an executable current_file"
		exit 1
	fi

	if [[ ! -d $2 ]]; then
		echo -e "$2 is not a directory"
		exit 1
	fi

else
	echo -e "Wrong number of parameters"
	exit 1
fi

prog="$1"
dir="$2"

for f in "$dir"/*; do
	headline=`head -n 1 "$f"`

	if [[ "$headline" = "$START" ]]; then
		file="$f"
	fi
done

touch temp.txt
touch temp2.txt
touch temp3.txt

lines=`cat "$file" | wc -l`
lines=`expr $lines - 1`

head -n $lines "$file" > temp2.txt
lines=`expr $lines - 1`
tail -n $lines temp2.txt > temp.txt
"$prog" < temp.txt > temp2.txt
cat temp2.txt > temp.txt

while [[ $STOP -eq 0 ]]; do
	current_file=`tail -n 1 "$file"`

	if [[ "$current_file" = "STOP" ]]; then
		cat temp.txt
		rm temp.txt
		rm temp2.txt
		rm temp3.txt
		exit 0
	else
		current_file=`echo -e "$current_file" | cut -c 6-`
	fi

	lines=`cat "$dir/$current_file" | wc -l`
	lines=`expr $lines - 1`

	head -n $lines "$dir/$current_file" > temp3.txt
	cat temp.txt temp3.txt | "$prog" > temp2.txt
	cat temp2.txt > temp.txt
	file="$dir/$current_file"
done
