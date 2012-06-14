#! /bin/sh
# (C) J C Hill 2007

# script to construct, pixelize, and snap the approximate healpix polygons in mangle
# If no outfile is given, healpix polygon file is named automatically and put in masks/healpix/healpix_polys directory
# USAGE: healpixpolys.sh <Nside> <scheme> <p> <r> <polygon_outfile>
# EXAMPLE: healpixpolys.sh 16 s 0 3 
# EXAMPLE: healpixpolys.sh 16 s 0 3 nside16p3s.pol

if [ "$MANGLEBINDIR" = "" ] ; then
    MANGLEBINDIR="../../bin"
fi
if [ "$MANGLESCRIPTSDIR" = "" ] ; then
    MANGLESCRIPTSDIR="../../scripts"
fi
if [ "$MANGLEDATADIR" = "" ] ; then
    MANGLEDATADIR="../../masks"
fi

#check command line arguments
if [ $# -lt 4 ] ; then
    echo >&2 "ERROR: enter Nside value, pixelization scheme, maximum number of polygons per pixel <p>," 
    echo >&2 "resolution <r>, and (optionally) the name of the output file as command line arguments."
    echo >&2 "" 
    echo >&2 "USAGE: healpixpolys.sh <Nside> <scheme> <p> <r> <polygon_outfile>"
    echo >&2 "EXAMPLE: healpixpolys.sh 16 s 0 3" 
    echo >&2 "EXAMPLE: healpixpolys.sh 16 s 0 3 nside16p3s.pol"
    exit 1
fi

if [ "$1" != 0 ]; then
    lognside=0
    for ((  I = 1 ;  I < 8192 ;  I = `expr 2 \* $I`  ))
    do
	if [ "$1" = "$I" ]; then
	    FLAG=1
	    break
	fi
	((lognside++))
    done

    if [ "$FLAG" != 1 ]; then
	echo >&2 "ERROR: <Nside> must be a power of 2."
	echo >&2 "USAGE: healpixpolys.sh <Nside> <scheme> <p> <r> <polygon_outfile>"
	echo >&2 "EXAMPLE: healpixpolys.sh 16 s 0 3" 
	echo >&2 "EXAMPLE: healpixpolys.sh 16 s 0 3 nside16p3s.pol"
	exit 1
    fi
fi

if [ "$1" = 0 ]; then
    POLYS=1
else
    POLYS=`expr 12 \* $1 \* $1`
fi

#write 12 zeros to jhw1
exec 6>&1           # Link file descriptor #6 with stdout.
exec &> jhw1
for i in {1..12}
do 
    echo 0
done
exec 1>&6 6>&-      # Restore stdout and close file descriptor #6.

#generate file with 12*nside^2 zeros by repeatedly catting 4 copies of jhw1 to itself
if [ "$lognside" != 0 ]; then
    for i in $(eval echo "{1..$lognside}"); do cat jhw1 jhw1 jhw1 jhw1 > jhw2 && mv jhw2 jhw1; done
fi

#prepend jhw1 with healpix_weight line
echo healpix_weight $POLYS | cat - jhw1 > jhw
rm jhw1

$MANGLEBINDIR/poly2poly jhw jp || exit
rm jhw
#note that -vo switch is needed in order to keep the correct id numbers (the HEALPix NESTED pixel numbers)

$MANGLEBINDIR/pixelize -P$2$3,$4 -vo jp jpx || exit
rm jp

if [ "$5" = "" ]; then
    if [ ! -d "$MANGLEDATADIR/healpix/healpix_polys" ] ; then
	echo >&2 "ERROR: $MANGLEDATADIR/healpix/healpix_polys not found." 
	echo >&2 "Check that the environment variable MANGLEDATADIR is pointing to" 
        echo >&2 "the appropriate directory (e.g., the mangle 'masks' directory)." 
        exit 1
    fi
    
    if [ "$3" = 0 ]; then 
        outfile="$MANGLEDATADIR/healpix/healpix_polys/nside${1}_p${4}${2}.pol"
    else
        outfile="$MANGLEDATADIR/healpix/healpix_polys/nside${1}_p-1${2}.pol"
    fi
else
    outfile=$5
fi

$MANGLEBINDIR/snap -vo -os jpx $outfile || exit
rm jpx

echo "HEALPix pixels at Nside=$1 written to $outfile."
