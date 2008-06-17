#! /bin/sh
# (C) J C Hill 2007
# script to rasterize a mask against the approximate HEALPix polygons and then convert the output
# into a HEALPix FITS file and (optional) then plot it using map2gif (plot REQUIRES HEALPIX)
 
# USAGE: healpixrast2fits.sh <polygon_infile> <Nside> <fits_outfile> <Nside_out> <gif_outfile>
# EXAMPLE: healpixrast2fits.sh mask.pol 16 rast_mask_nside16.fits 8 rast_mask_nside16.gif
# EXAMPLE (without healpix): healpixrast2fits.sh mask.pol 16 rast_mask_nside16.fits

if [ "$MANGLEBINDIR" = "" ] ; then
    MANGLEBINDIR="../../bin"
fi
if [ "$MANGLESCRIPTSDIR" = "" ] ; then
    MANGLESCRIPTSDIR="../../scripts"
fi
if [ "$MANGLEDATADIR" = "" ] ; then
    MANGLEDATADIR="../../masks"
fi

scheme="s"

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
    pix=8
fi
if [ "$2" = 16 ] || [ "$2" = 32 ]; then
    pix=5
fi
if [ "$2" -lt 16 ]; then
    pix=3
fi
healpixfile=$MANGLEDATADIR/healpix/healpix_polys/nside$2_p${pix}${scheme}.pol

if [ ! -e $healpixfile ] ; then
    if [ "$2" -gt 32 ]; then
	echo "ERROR: file $healpixfile does not exist."
	echo "You can download it from the website (recommended)"
	echo "or you can create it by running"
	echo "sh healpixpolys.sh $2 ${scheme} 0 8"
	echo "in $MANGLEDATADIR/healpix."
	echo ""
	exit 1
    else
	echo "Generating HEALPix polygons with healpixpolys.sh ..."
	healpixpolys.sh $2 $scheme 0 $pix || exit
    fi
fi

#grab pixelization info from input file
awk '/pixelization/{print $0}' < $1 > jpix
res1=`awk '{print substr($2, 1, length($2)-1)}' < jpix`
scheme1=`awk '{print substr($2, length($2))}' < jpix`
rm jpix

#if input file is unpixelized, pixelize it and snap it
#if input file is pixelized to the correct resolution, use it as is.
#WARNING: this assumes that if the input polygon file is pixelized, it is also snapped.
if [ "$res1" = "" ]; then
    echo ""
    echo "Pixelizing $1 ..."
    $MANGLEBINDIR/pixelize -P${scheme}0,$pix $1 jp || exit   
    echo ""
    echo "Snapping $1 ..."
    $MANGLEBINDIR/snap jp jps || exit
    rm jp
#if input file is pixelized to a different resolution and scheme,
#warn and pixelize again
elif [ ! "$res1" = "$pix" ] || [ ! "$scheme1" = "$scheme" ]; then
    echo "ALERT: input polygon file is not pixelized with the same resolution"
    echo "and scheme as the provided HEALPix polygons for nside=$2." 
    echo "--> repixelizing input file"
    echo "(This step can be avoided by using an input file pixelized" 
    echo "with resolution $pix and scheme $scheme, using the flag -P${scheme}0,$pix .)"
    echo ""
    echo "Pixelizing $1 ..."
    $MANGLEBINDIR/pixelize -P${scheme}0,$pix $1 jps  || exit 
else
    cp $1 jps
fi

echo ""
echo "Rasterizing $1 against the Nside=$2 approximate HEALPix pixels..."
$MANGLEBINDIR/rasterize -H $healpixfile jps jhw || exit
rm jps

# use sed to remove the "healpix_weight N" (the first line) of the healpix_weight file
sed '1d' jhw > jhd
rm jhw

datfitsbin=$MANGLEBINDIR/dat2fits_binary.x
if [ ! -e $datfitsbin ] ; then
 echo "ERROR: binary $datfitsbin does not exist."
 echo "You can create it by compiling the fortran file"
 echo "$MANGLEDATADIR/healpix/healpix_conversion_scripts/dat2fits_binary.f as follows:"
 echo "g77 dat2fits_binary.f -o dat2fits_binary.x libcfitsio.a"
 echo "NOTE: You will need to obtain and install the library file libcfitsio.a"
 echo "from http://heasarc.nasa.gov/fitsio/fitsio.html"
 echo "in order to compile the fortran file successfully."
 exit 1
fi
	
if [ -e $3 ] ; then
    echo "WARNING: overwriting existing version of $3" 
    /bin/rm $3
fi

$MANGLEBINDIR/call $datfitsbin 1 $2 jhd $3 
rm args.dat
if [ $? -ne 0 ]; then
 echo "ERROR: binary $datfitsbin is not executable on your system."
 echo "You can recompile it on your system as follows:"
 echo "g77 dat2fits_binary.f -o dat2fits_binary.x libcfitsio.a"
 echo "NOTE: You will need to obtain and install the library file libcfitsio.a"
 echo "from http://heasarc.nasa.gov/fitsio/fitsio.html"
 echo "in order to compile the fortran file successfully."
    
 exit 1
fi

rm jhd

if [ -e dat2fitserr.temp ]; then
    rm dat2fitserr.temp
    exit 1
fi

echo "Rasterized mask FITS file written to $3."

OUT=$4
OUT=${OUT:=0}

if [ "$OUT" == 0 ] ; then
 echo "No gif specifications present"
 echo "Finished!"
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
	exit 1
    fi
    
    if which ud_grade && which map2gif ; then
	$MANGLESCRIPTSDIR/plotmap.sh $4 $3 $5
	echo "Rasterized mask image file written to $5."
    else
	echo "ud_grade and/or map2gif not found!"
	echo "In order to plot a gif image of $3, you need to install HEALPix,"
        echo "which is available at http://healpix.jpl.nasa.gov/."
	exit 1
    fi
fi
