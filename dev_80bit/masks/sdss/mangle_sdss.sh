#! /bin/sh
# © M E C Swanson 2007
#script to combine window and holes of SDSS, as provided by the NYU VAGC:
# http://sdss.physics.nyu.edu/vagc/
#the window and mask files used by this script are in 
#lss/<release>/<cut>/<number>/lss/
#window.<release><cut><number>.ply and mask.<release><cut><number>.ply
#USAGE: mangle_sdss.sh <release> <cut><number>
#EXAMPLE:mangle_sdss.sh dr6 safe0

sample=$1
cuts=$2

user=`whoami`
names=`finger $user | fgrep "ame:" | sed 's/.*: *\([^ ]*\)[^:]*/\1/'`
for name in ${names}; do break; done
echo "Hello $name, watch me combine the window function and holes for the SDSS survey."

# to make verbose
quiet=
# to make quiet
#quiet=-q

#to assign new id numbers
old=
#to keep old id numbers
#old=-vo

#to pixelize dynamically
#pix=-Pd
#to pixelize everything to a maximum resolution
pix=-Pd0,6

filedir=$MANGLEDATADIR/sdss/$sample/$cuts/

cd $filedir

# name of output file to contain sdss polygons
pol=sdss_${sample}${cuts}_res6d.pol
list=sdss_${sample}${cuts}_res6d.list
eps=sdss_${sample}${cuts}_res6d.eps
fields=window.${sample}${cuts}.ply
mask=mask.${sample}${cuts}.ply
holes=holes.${sample}${cuts}.ply

echo 0 > jw
echo "$MANGLEBINDIR/weight -zjw $mask $holes"
$MANGLEBINDIR/weight -zjw $mask $holes

echo "$MANGLEBINDIR/snap -S $quiet $fields $holes jfhs"
$MANGLEBINDIR/snap -S $quiet $fields $holes jfhs || exit
echo "$MANGLEBINDIR/pixelize $quiet $old $pix jfhs jfhp"
$MANGLEBINDIR/pixelize $quiet $old $pix jfhs jfhp || exit
echo "$MANGLEBINDIR/snap $quiet $old jfhp jfh"
$MANGLEBINDIR/snap $quiet $old jfhp jfh || exit
echo "$MANGLEBINDIR/balkanize $quiet $old jfh jb"
$MANGLEBINDIR/balkanize $quiet $old jfh jb || exit
echo "$MANGLEBINDIR/unify $quiet $old jb $pol"
$MANGLEBINDIR/unify $quiet $old jb $pol || exit

echo "Polygons for SDSS $1 $2 are in $pol"

#uncomment this to plot the mask using the Matlab mapping toolbox
#$MANGLEBINDIR/poly2poly $quiet $old -ol30 $pol $list
#$MANGLESCRIPTSDIR/graphmask.sh $list $eps

rm j*
