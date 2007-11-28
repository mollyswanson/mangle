#! /bin/sh
# © M E C Swanson 2007
#script to find the complement of a mangle mask
#USAGE: find_complement.sh <mask> <complement>
#EXAMPLE:find_complement.sh holes.pol complement.pol
mask=$1
trimmer=$2
outfile=$3
pixargs=$4

pixelize $pixargs $trimmer trimmer_pix
pixelize $pixargs $mask mask_pix

find_complement.sh trimmer_pix trimmer_comp
echo 0 > jw0
weight -zjw0 trimmer_comp jcomp0
balkanize mask_pix jcomp0 jb
unify jb $outfile
