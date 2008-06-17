#!/bin/tcsh
# USAGE:   plotmap.sh <Nside_out> <fits_infile> <gif_outfile>
# EXAMPLE: plotmap.sh 512 qaz.fits qaz.gif
echo ""
echo "NOTE: This script requires a HEALPix installation"
echo "Creating $3..."
#echo 'WARNING: plotmap disabled'
#exit

set res     = $1
set infile  = $2
set outfile = qaz_plotmap.fits

echo 'nside_out = '$res   >qaz_ud_grade.dat
echo 'infile = '$infile >>qaz_ud_grade.dat
echo 'outfile = '$outfile >>qaz_ud_grade.dat
if -f $outfile /bin/rm $outfile
ud_grade qaz_ud_grade.dat

if -f $3 /bin/rm $3

# default mangle limits (weights range from 0 to 1)
map2gif -inp qaz_plotmap.fits -out $3 -bar .true. -add 0 -min 0 -max 1

# default WMAP limits
#map2gif -inp qaz_plotmap.fits -out $1.gif -bar .true. -add 0.2 -min 0 -max 0.4

rm qaz_plotmap.fits qaz_ud_grade.dat
