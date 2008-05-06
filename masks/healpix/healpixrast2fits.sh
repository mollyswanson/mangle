#! /bin/sh
# (C) J C Hill 2007

# script to rasterize a mask against the approximate HEALPix polygons and then convert the output
# into a HEALPix FITS file and (optional) then plot it using map2gif (plot REQUIRES HEALPIX)

# USAGE: healpixrast2fits.sh <polygon_infile> <Nside> <fits_outfile> <Nside_out> <gif_outfile>
# EXAMPLE: healpixrast2fits.sh mask.pol 16 rast_mask_nside16.fits 8 rast_mask_nside16.gif

if [ "$2" != 0 ]; then
 for ((  I = 1 ;  I < 8192 ;  I = `expr 2 \* $I`  ))
 do
  if [ "$2" = "$I" ]; then
   FLAG=1
  fi
 done

 if [ "$FLAG" != 1 ]; then
  echo "USAGE: healpixrast2fits.sh <polygon_infile> <Nside> <fits_outfile> <Nside_out> <gif_outfile>"
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
$MANGLEBINDIR/rasterize -H $healpixfile jps jhw
rm jps

# use sed to remove the "healpix_weight N" (the first line) of the healpix_weight file
sed '1d' jhw > jhd
rm jhw

datfitsbin=$MANGLEDATADIR/healpix/healpix_conversion_scripts/dat2fits_binary.x
if [ ! -e $datfitsbin ] ; then
 echo "ERROR: binary $datfitsbin does not exist."
 echo "You can create it by compiling the fortran file"
 echo "$MANGLEDATADIR/healpix/healpix_conversion_scripts/dat2fits_binary.f as follows:"
 echo "f77 dat2fits_binary.f -o dat2fits_binary.x libcfitsio.a"
 echo "NOTE: You will need to obtain and install the library file libcfitsio.a"
 echo "from http://heasarc.nasa.gov/fitsio/fitsio.html"
 echo "in order to compile the fortran file successfully."
 exit 1
fi

$MANGLEDATADIR/healpix/healpix_conversion_scripts/call $datfitsbin 1 $2 jhd $3
rm jhd

echo "Rasterized mask FITS file written to $3."

OUT=$4
OUT=${OUT:=0}

if [ "$OUT" == 0 ] ; then
 echo "No gif specifications present"
 echo "Finished!"
 rm args.dat
fi

if [ "$OUT" != 0 ]; then
 for ((  I = 1 ;  I < 8192 ;  I = `expr 2 \* $I`  ))
 do
  if [ "$4" = "$I" ]; then
   FLAGG=1
  fi
 done

 if [ "$FLAGG" != 1 ]; then
  echo "USAGE: healpixrast2fits.sh <polygon_infile> <Nside> <fits_outfile> <Nside_out> <gif_outfile>"
  echo "<Nside_out> must be a power of 2."
  rm args.dat
  exit 1
 fi

 $MANGLEDATADIR/healpix/healpix_conversion_scripts/plotmap.sh $4 $3 $5

 echo "Rasterized mask image file written to $5."

 rm args.dat qaz_plotmap.fits qaz_ud_grade.dat
fi
