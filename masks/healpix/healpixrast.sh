#! /bin/sh
# (C) J C Hill 2007

# script to rasterize an arbitrary input mask against the approximate healpix polygons
# in mangle at any value of Nside

# USAGE: healpixrast.sh <polygon_infile> <Nside> <polygon_outfile>
# EXAMPLE: healpixrast.sh mask.pol 16 rast_mask_nside16.pol

if [ "$2" != 0 ]; then
 for ((  I = 1 ;  I < 8192 ;  I = `expr 2 \* $I`  ))
 do
  if [ "$2" = "$I" ]; then
   FLAG=1
  fi
 done

 if [ "$FLAG" != 1 ]; then
  echo "USAGE: healpixrast.sh <polygon_infile> <Nside> <polygon_outfile>"
  echo "<Nside> must be a power of 2."
  exit 1
 fi
fi


if [ "$2" -gt 32 ]; then
   healpixfile=$MANGLEDATADIR/healpix/healpix_polys/nside$2_p8s.pol
   pix=8
   if [ ! -e $healpixfile ] ; then
    echo "ERROR: file $healpixfile does not exist."
    echo "You can download it from the website (recommended)"
    echo "or you can create it by running"
    echo "sh healpixpolys.sh $2 s 0 8 healpix_polys/nside$2_p8s.pol"
    echo "in $MANGLEDATADIR/healpix."
    echo ""
    exit 1
   fi
fi

if [ "$2" = 16 ] || [ "$2" = 32 ]; then
   healpixfile=$MANGLEDATADIR/healpix/healpix_polys/nside$2_p5s.pol
   pix=5
   if [ ! -e $healpixfile ] ; then
    echo "ERROR: file $healpixfile does not exist."
    echo "You can create it quickly by running"
    echo "sh healpixpolys.sh $2 s 0 5 healpix_polys/nside$2_p5s.pol"
    echo "in $MANGLEDATADIR/healpix."
    echo ""
    exit 1
   fi
fi

if [ "$2" -lt 16 ]; then
   healpixfile=$MANGLEDATADIR/healpix/healpix_polys/nside$2_p3s.pol
   pix=3
   if [ ! -e $healpixfile ] ; then
    echo "ERROR: file $healpixfile does not exist."
    echo "You can create it quickly by running"
    echo "sh healpixpolys.sh $2 s 0 3 healpix_polys/nside$2_p3s.pol"
    echo "in $MANGLEDATADIR/healpix."
    echo ""
    exit 1
   fi
fi


echo ""
echo "Pixelizing $1 ..."
$MANGLEBINDIR/pixelize -Ps0,$pix $1 jp

echo ""
echo "Snapping $1 ..."
$MANGLEBINDIR/snap jp jps
rm jp

echo ""
echo "Rasterizing $1 against the Nside=$2 approximate HEALPix pixels..."
$MANGLEBINDIR/rasterize $healpixfile jps $3
rm jps

echo "Rasterized mask written to $3."
