#!/bin/bash -e

if test $# -ne 6; then
  echo "Usage: $0 key iv aadlen recordnum infile outfile" 1>&2
  exit 1
fi

key=$1
iv=$2
adlen=$3
recordnum=$4
infile=$5
outfile=$6

recdata=$(head -c ${adlen} ${infile} | xxd -p)
authtag=$(perl -p0777 -e 's/(\0\0)*$//gs; s/.*(.{16})$/\1/s' ${infile} | xxd -p)
rm -f /tmp/working
perl -p0777 -e 's/(\0\0)*$//gs; s/.{'${adlen}'}(.*).{16}/\1/s' < ${infile} > /tmp/working
cat /tmp/working | ./aes_128_gcm_decrypt ${iv} ${recordnum} ${key} ${recdata} ${authtag} > /tmp/out
cat /tmp/out > ${outfile}
rm -f /tmp/out /tmp/working
