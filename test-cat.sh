#! /bin/sh -pe

# Copyright (C) 2000 by Martin Pool

# Test that our file buffers are operating properly.

source ${srcdir:-.}/testfns.sh $0 $@

files=`echo $srcdir/INSTALL`

sizes="1 2 3 4 7 8 9 10 33 63 64 65 100 200 2000 10000"

new=$tmpdir/new.tmp

for from in $files
do
    for size in $sizes
    do
	for opt in "" "-l"
	do
	    run_test hsfilebufcat $opt -b $size <$from >$new
	    run_test cmp $from $new
	done
    done
done
    
    