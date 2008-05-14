#! /bin/sh
# © M E C Swanson 2008
#script to find the complement of a mangle mask
#USAGE: find_complement.sh <mask> <complement>
#EXAMPLE:find_complement.sh holes.pol complement.pol
#input mask can be pixelized beforehand to a fixed resolution
#if the mask is complicated, e.g. pixelize -Ps0,8 holes.pol holes_pixelized.pol

if [ "$MANGLEBINDIR" = "" ] ; then
    MANGLEBINDIR="../../bin"
fi
if [ "$MANGLESCRIPTSDIR" = "" ] ; then
    MANGLESCRIPTSDIR="../../scripts"
fi
if [ "$MANGLEDATADIR" = "" ] ; then
    MANGLEDATADIR="../../masks"
fi

mask=$1
complement=$2

#check command line arguments
if [ "$mask" = "" ] || [ "$complement" = "" ] ; then
    echo >&2 "ERROR: enter the input and output polygon files as command line arguments."
    echo >&2 "" 
    echo >&2 "USAGE: find_complement.sh <mask> <complement>"
    echo >&2 "EXAMPLE:find_complement.sh holes.pol complement.pol"
    exit 1
fi

#grab pixelization info from input files
awk '/pixelization/{print $0}' < $mask > jpix
res=`awk '{print substr($2, 1, length($2)-1)}' < jpix`
scheme=`awk '{print substr($2, length($2))}' < jpix`
if [ $res -eq -1 ] ; then
    echo >&2 "ERROR: cannot take the complement of a mask pixelized adaptively."
    echo >&2 "Pixelize your mask using a fixed resolution, e.g. -Ps0,8, and try again."
    exit 1
fi

#check for appropriate allsky file, and generate it if it'
allsky=$MANGLEDATADIR/allsky/allsky$res$scheme.pol
if [ ! -e $allsky ] ; then
    echo "Generating allsky$res$scheme.pol..."
    if [ ! -d "$MANGLEDATADIR/allsky" ] ; then
	echo >&2 "ERROR: $MANGLEDATADIR/allsky not found." 
	echo >&2 "Check that the environment variable MANGLEDATADIR is pointing to" 
        echo >&2 "the appropriate directory (e.g., the mangle 'masks' directory)." 
        exit 1
    fi
    currentdir=$PWD
    cd $MANGLEDATADIR/allsky
    $MANGLEBINDIR/pixelize -P${scheme}0,${res} allsky.pol allsky$res$scheme.pol 
    cd $currentdir
fi

#set weight of all polygons in mask to zero
echo 0 > jw0
echo "$MANGLEBINDIR/weight -zjw0 $mask jw"
$MANGLEBINDIR/weight -zjw0 $mask jw || exit

#balkanize the full sky with the zero-weighted mask to find the complement
echo "$MANGLEBINDIR/balkanize $allsky jw jb"
$MANGLEBINDIR/balkanize $allsky jw jb || exit

#unify to get rid of zero weight polygons
echo "$MANGLEBINDIR/unify jb $complement"
$MANGLEBINDIR/unify jb $complement || exit

echo "Complement of $mask written to ${complement}."

#clean up
rm j*
