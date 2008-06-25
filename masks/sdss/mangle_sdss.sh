#! /bin/sh
# © M E C Swanson 2007
#script to combine window and holes of SDSS, as provided by the NYU VAGC:
# http://sdss.physics.nyu.edu/vagc/
#the window and mask files used by this script are in 
#lss/<release>/<cut>/<number>/lss/
#window.<release><cut><number>.ply and mask.<release><cut><number>.ply
#USAGE: mangle_sdss.sh <release> <cut><number>
#EXAMPLE:mangle_sdss.sh dr7 safe0

if [ "$MANGLEBINDIR" = "" ] ; then
    MANGLEBINDIR="../../bin"
fi
if [ "$MANGLESCRIPTSDIR" = "" ] ; then
    MANGLESCRIPTSDIR="../../scripts"
fi
if [ "$MANGLEDATADIR" = "" ] ; then
    MANGLEDATADIR="../../masks"
fi

sample=$1
cuts=$2

#check command line arguments
if [ "$sample" = "" ] || [ "$cuts" = "" ] ; then
    echo >&2 "ERROR: enter the SDSS release and cuts to use as command line arguments."
    echo >&2 "" 
    echo >&2 "USAGE: mangle_sdss.sh <release> <cut><number>"
    echo >&2 "EXAMPLE:mangle_sdss.sh dr7 safe0"
    exit 1
fi

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
#pix=
#restag=
#to pixelize everything to fixed resolution
scheme="d"
res=6
pix="-P${scheme}0,${res}"
restag="_res${res}${scheme}"

#uncomment this to put files from different releases in individual directories
#filedir=$MANGLEDATADIR/sdss/$sample/$cuts/
#cd $filedir

# name of output file to contain sdss polygons
pol=sdss_${sample}${cuts}${restag}.pol
grph=sdss_${sample}${cuts}${restag}.grph
list=sdss_${sample}${cuts}${restag}.list
eps=sdss_${sample}${cuts}${restag}.eps
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

# Graphics

# number of points per (2 pi) along each edge of a polygon
pts_per_twopi=30

echo "$MANGLEBINDIR/poly2poly -og$pts_per_twopi $quiet $pol $grph"
$MANGLEBINDIR/poly2poly -og$pts_per_twopi $quiet $pol $grph || exit
echo "Data suitable for plotting polygons of the SDSS $1 $2 mask are in $grph:"
echo "each line is a sequence of az, el points delineating the perimeter of a polygon."

# for plotting with the matlab script

if which matlab ; then
# name of output file to contain matlab graphics

    echo "$MANGLEBINDIR/poly2poly -ol$pts_per_twopi $quiet $pol $list"
    $MANGLEBINDIR/poly2poly -ol$pts_per_twopi $quiet $pol $list || exit
    echo "Data for plotting polygons of the SDSS $1 $2 mask in Matlab are in $list."
    echo "Using Matlab to plot the SDSS $1 $2  mask ..."
    echo "$MANGLESCRIPTSDIR/graphmask.sh $list $eps"
    $MANGLESCRIPTSDIR/graphmask.sh $list $eps "Completeness mask for SDSS $sample $cuts"
    if [ $? -eq 0 ]; then
	echo "Made a figure illustrating the SDSS $1 $2 mask: $eps" 
	echo "Type \"ggv $eps\" or \"gv $eps\" to view the figure."  
###uncomment to automatically plot usng the sm script -- sm tends to get overloaded with the SDSS mask
#    elif which sm ; then
#	echo "Using Supermongo to plot the SDSS $1 $2 mask:"
#	$MANGLESCRIPTSDIR/graphmasksm.sh $grph $eps 0 0 0 0 "Completeness mask for SDSS $sample $cuts"
#	if [ $? -eq 0 ]; then
#	    echo "Made a figure illustrating the SDSS $1 $2 mask: $eps" 
#	    echo "Type \"ggv $eps\" or \"gv $eps\" to view the figure."  
#	    echo "A script is also available to plot mangle files Matlab (with the mapping toolbox)," 
#	    echo "or you can plot $grph using your own favorite plotting tool."
#	fi
    else 
	echo "Scripts are available for plotting mangle polygons in Matlab" 
	echo "(with the mapping toolbox) or Supermongo, or you can plot $grph"
	echo "using your own favorite plotting tool."
    fi
###uncomment to automatically plot usng the sm script -- sm tends to get overloaded with the SDSS mask
#elif which sm ; then
#    echo "Using Supermongo to plot the SDSS $1 $2 mask:"
#    $MANGLESCRIPTSDIR/graphmasksm.sh $grph $eps 0 0 0 0 "Completeness mask for SDSS $sample $cuts"
#    if [ $? -eq 0 ]; then
#	echo "Made a figure illustrating the SDSS $1 $2 mask: $eps" 
#	echo "Type \"ggv $eps\" or \"gv $eps\" to view the figure."  
#	echo "A script is also available to plot mangle files Matlab (with the mapping toolbox)," 
#       echo "or you can plot $grph using your own favorite plotting tool."
#    fi
else
    echo "Scripts are available for plotting mangle polygons in Matlab" 
    echo "(with the mapping toolbox) or Supermongo, or you can plot $grph"
    echo "using your own favorite plotting tool."
fi

rm jw jfhs jfhp jfh jb
