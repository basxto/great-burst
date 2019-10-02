#!/bin/bash
if [ ${#1} -ne 7 ] || [ "${1:0:1}" != '#' ]; then
	echo "usage: $0 #5797DF"
	exit 1
fi
hex=${1^^}
r="0x${hex:1:2}"
g="0x${hex:3:2}"
b="0x${hex:5:2}"

# 256/32=8
# to gbc
r=$((((r+1)/8)-1))
g=$((((g+1)/8)-1))
b=$((((b+1)/8)-1))
echo "(RGB($r, $g, $b))"

# back to hex
r=$((((r+1)*8)-1))
g=$((((g+1)*8)-1))
b=$((((b+1)*8)-1))
printf '#%02X%02X%02X' $r $g $b