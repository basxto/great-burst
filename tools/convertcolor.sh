#!/bin/bash
if [ ${#1} -ne 7 ] || [ "${1:0:1}" != '#' ]; then
	>&2 echo "Usage: $0 #579BDF"
	exit 1
fi
hex=${1^^}
r="0x${hex:1:2}"
g="0x${hex:3:2}"
b="0x${hex:5:2}"
echo "(RGB($((r*31/255)), $((g*31/255)), $((b*31/255))))"