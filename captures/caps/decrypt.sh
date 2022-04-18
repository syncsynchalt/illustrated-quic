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
rm -f /tmp/working.$$
perl -p0777 -e 's/(\0\0)*$//gs; s/.{'${adlen}'}(.*).{16}/\1/s' < ${infile} > /tmp/working.$$
cat /tmp/working.$$ | ./aes_128_gcm_decrypt ${iv} ${recordnum} ${key} ${recdata} ${authtag} > /tmp/out.$$
cat /tmp/out.$$ > ${outfile}

filesize=$(du -k /tmp/working.$$ | cut -f1)
if [[ $filesize -lt 7 ]]; then
    msg=$(cat /tmp/working.$$ | xxd -p | perl -pe 's/../& /g')
else
    start=$(head -c 3 /tmp/working.$$ | xxd -p | perl -pe 's/../& /g')
    end=$(tail -c 3 /tmp/working.$$ | xxd -p | perl -pe 's/../& /g')
    msg="$start ... $end"
fi

echo --------- start copy into template -------
echo '$ key='$key
echo '$ iv='$iv
echo "### from this record"
echo '$ recdata='$recdata
echo '$ authtag='$authtag
echo '$ recordnum='$recordnum
echo "### may need to add -I and -L flags for include and lib dirs"
echo '$ cc -o aes_128_gcm_decrypt aes_128_gcm_decrypt.c -lssl -lcrypto'
echo '$ echo '"$msg" | xxd -r -p > /tmp/msg1
echo '$ cat /tmp/msg1 \'
echo '  | ./aes_128_gcm_decrypt $iv $recordnum $key $recdata $authtag \'
echo '  | hexdump -C'
echo
hexdump -C /tmp/out.$$ | head
echo --------- end copy into template -------

rm -f /tmp/out.$$ /tmp/working.$$
