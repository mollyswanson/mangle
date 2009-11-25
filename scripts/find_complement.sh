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
mtol=$3
snap="$4 $5 $6"
dres=3
dscheme="s"

#check command line arguments
if [ "$mask" = "" ] || [ "$complement" = "" ] ; then
    echo >&2 "ERROR: enter the input and output polygon files as command line arguments."
    echo >&2 "" 
    echo >&2 "USAGE: find_complement.sh <mask> <complement>"
    echo >&2 "EXAMPLE:find_complement.sh holes.pol complement.pol"
    echo >&2 "or to use non-default values for snap and multiple intersection tolerances:" 
    echo >&2 "USAGE: find_complement.sh <mask> <complement> mtol_argument snap_tol_arguments"
    echo >&2 "EXAMPLE:find_complement.sh holes.pol complement.pol -m1e-8 -a.027 -b.027 -t.027"
    exit 1
fi

head -n 100 $mask > jmaskhead

#grab pixelization info from input files
awk '/pixelization/{print $0}' < jmaskhead > jpix
res=`awk '{print substr($2, 1, length($2)-1)}' < jpix`
scheme=`awk '{print substr($2, length($2))}' < jpix`
rm jpix jmaskhead

#if input file is unpixelized, pixelize it
#if input file is pixelized to a fixed resolution, use it as is.
if [ "$res" = "" ]; then
    res=$dres
    scheme=$dscheme
    echo ""
    echo "Pixelizing $1 ..."
    $MANGLEBINDIR/pixelize $mtol -P${scheme}0,$res $mask jp || exit  
    echo ""
elif [ "$res" = -1 ] ; then
    res=$dres
    scheme=$dscheme
    echo "WARNING: cannot take the complement of a mask pixelized adaptively."
    echo "Pixelizing your mask using a fixed resolution:"
    echo ""
    echo "Pixelizing $1 ..."
    $MANGLEBINDIR/pixelize $mtol -P${scheme}0,$res $mask jp || exit   
    echo ""
else
    cp $mask jp
fi

#check for appropriate allsky file, and generate it if it's not there:
allsky=$MANGLEDATADIR/allsky/allsky$res$scheme.pol
if [ ! -e $allsky ] ; then
     $MANGLESCRIPTSDIR/make_allsky.sh $res $scheme $mtol $snap
fi

#check if input file is snapped
snapped=`awk '/snapped/{print $1}' < $mask`

#if input file isn't snapped, snap it
if [ ! "$snapped" = "snapped" ]; then
    echo "Snapping $1 ..."
    $MANGLEBINDIR/snap $snap $mtol jp jps || exit
    rm jp
else
    mv jp jps
fi

#set weight of all polygons in mask to zero
echo 0 > jw0
echo "$MANGLEBINDIR/weight $mtol -zjw0 $mask jw"
$MANGLEBINDIR/weight $mtol -zjw0 jps jw || exit
rm jps

#balkanize the full sky with the zero-weighted mask to find the complement
echo "$MANGLEBINDIR/balkanize $mtol $allsky jw jb"
$MANGLEBINDIR/balkanize $mtol $allsky jw jb || exit
rm jw

#unify to get rid of zero weight polygons
echo "$MANGLEBINDIR/unify $mtol jb $complement"
$MANGLEBINDIR/unify $mtol jb $complement || exit
rm jb

echo "Complement of $mask written to ${complement}."
