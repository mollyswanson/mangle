#! /bin/sh
# (C) J C Hill 2007

# script to construct, pixelize, and snap the approximate healpix polygons in mangle
# If no outfile is given, healpix polygon file is named automatically and put in masks/healpix/healpix_polys directory
# USAGE: healpixpolys.sh <Nside> <scheme> <p> <r> <polygon_outfile>
# EXAMPLES: healpixpolys.sh 16 s 0 3 
# EXAMPLES: healpixpolys.sh 16 s 0 3 nside16p3s.pol

if [ "$MANGLEBINDIR" = "" ] ; then
    MANGLEBINDIR="../../bin"
fi
if [ "$MANGLESCRIPTSDIR" = "" ] ; then
    MANGLESCRIPTSDIR="../../scripts"
fi
if [ "$MANGLEDATADIR" = "" ] ; then
    MANGLEDATADIR="../../masks"
fi

if [ "$1" != 0 ]; then
 for ((  I = 1 ;  I < 8192 ;  I = `expr 2 \* $I`  ))
 do
  if [ "$1" = "$I" ]; then
   FLAG=1
  fi
 done

 if [ "$FLAG" != 1 ]; then
  echo "USAGE: healpixpolys.sh <Nside> <scheme> <p> <r> <polygon_outfile>"
  echo "<Nside> must be a power of 2."
  echo "<scheme> is the pixelization scheme to use; <p> is the number of polygons allowed in each pixel; <r> is the maximum pixelization resolution."
  exit 1
 fi
fi

if [ "$1" = 0 ]; then
 POLYS=1
else
 POLYS=`expr 12 \* $1 \* $1`
fi

echo healpix_weight $POLYS >> jhw
for ((  I = 0 ;  I < POLYS;  I++  ))
do
  echo 0 >> jhw
done

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

$MANGLEBINDIR/snap -vo jpx $outfile || exit
rm jpx

echo "HEALPix pixels at Nside=$1 written to $outfile."
