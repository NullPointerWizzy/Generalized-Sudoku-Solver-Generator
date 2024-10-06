#!/bin/sh

BINARY_DIR=binaries

TIMEOUT=300

for exec in ${BINARY_DIR}/*-sudoku; do
    echo "$(basename ${exec})"
    echo "====================="
    total_time=0;
    for level in level-0[1-5] ; do
	echo -n "$(basename ${level}): ";
	start=$(date +%s);
	timeout $TIMEOUT ./${exec} --all ${level}/*.sku > /dev/null
	ret="$?";
	end=$(date +%s);
	if [ "${ret}" -eq "0" ] ; then
	    echo "Success! ($((${end} - ${start})))";
	    total_time=$((${total_time} + (${end} - ${start})))
	else
	    echo "Failed!";
	fi;
    done;
    echo "Total time: ${total_time}"
    echo "";
done
