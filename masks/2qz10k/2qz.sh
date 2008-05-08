#! /bin/sh

#script to create the angular mask for the 10k releast of the 2dF QSO Redshift Survey
#see http://www.2dfquasar.org/ for more about the survey.
#USAGE: 2qz.sh

user=`whoami`
names=`finger $user | fgrep "ame:" | sed 's/.*: *\([^ ]*\)[^:]*/\1/'`
for name in ${names}; do break; done
echo "Hello $name, watch me make the angular mask and harmonics for the 2QZ 10k survey."

#to pixelize dynamically
#pix=-Pd
#to pixelize everything to maximum resolution
pix=-Ps0,6

# to make verbose
quiet=
# to make quiet
#quiet=-q

# snap tolerances
snaptols="-a2 -b2 -t2"

#multiple intersection tolerance
mtol="-m1e-5"

# maximum harmonic number
lmax=20

mangledir=../../bin/

# North

if [ -z "$quiet" ]; then
    echo "===========================================";
    echo "                 THE NORTH                 ";
fi

# name of output file to contain 2QZ 10k North polygons
npol=2qz_north_pixel.pol

echo "${mangledir}pixelize $pix $quiet $mtol ngp_ukstfld.lims.txt jnup"
${mangledir}pixelize $pix $quiet $mtol ngp_ukstfld.lims.txt jnup || exit
echo "${mangledir}pixelize $pix $quiet $mtol ngp_field_coords.txt jnfp"
${mangledir}pixelize $pix $quiet $mtol ngp_field_coords.txt jnfp || exit
echo "${mangledir}pixelize $pix $quiet $mtol ngp.used.rahole.txt jnhp"
${mangledir}pixelize $pix $quiet $mtol ngp.used.rahole.txt jnhp || exit
echo "${mangledir}snap $quiet $snaptols $mtol jnup jnu"
${mangledir}snap $quiet $snaptols $mtol jnup jnu || exit
echo "${mangledir}snap $quiet $snaptols $mtol jnfp jnf"
${mangledir}snap $quiet $snaptols $mtol jnfp jnf || exit
echo "${mangledir}snap $quiet $snaptols $mtol jnhp jnh"
${mangledir}snap $quiet $snaptols $mtol jnhp jnh || exit
echo "${mangledir}snap $quiet $snaptols $mtol jnu jnf jnh jnufh"
${mangledir}snap $quiet $snaptols $mtol jnu jnf jnh jnufh || exit
echo "${mangledir}balkanize $quiet $mtol jnufh jnb"
${mangledir}balkanize $quiet $mtol jnufh jnb || exit
echo "${mangledir}weight $quiet $mtol -z2QZ10k jnb jnw"
${mangledir}weight $quiet $mtol -z2QZ10k jnb jnw || exit
echo "${mangledir}unify $quiet $mtol jnw $npol"
${mangledir}unify $quiet $mtol jnw $npol || exit
echo "Polygons for 2QZ 10k North are in $npol"

# South

if [ -z "$quiet" ]; then
    echo "===========================================";
    echo "                 THE SOUTH                 ";
fi

# name of output file to contain 2QZ 10k South polygons
spol=2qz_south_pixel.pol

echo "${mangledir}pixelize $pix $quiet $mtol sgp_ukstfld.lims.txt jsup"
${mangledir}pixelize $pix $quiet $mtol sgp_ukstfld.lims.txt jsup || exit
echo "${mangledir}pixelize $pix $quiet $mtol sgp_field_coords.txt jsfp"
${mangledir}pixelize $pix $quiet $mtol sgp_field_coords.txt jsfp || exit
echo "${mangledir}pixelize $pix $quiet $mtol sgp.used.rahole.txt jshp"
${mangledir}pixelize $pix $quiet $mtol sgp.used.rahole.txt jshp || exit
echo "${mangledir}snap $quiet $snaptols $mtol jsup jsu"
${mangledir}snap $quiet $snaptols $mtol jsup jsu || exit
echo "${mangledir}snap $quiet $snaptols $mtol jsfp jsf"
${mangledir}snap $quiet $snaptols $mtol jsfp jsf || exit
echo "${mangledir}snap $quiet $snaptols $mtol jshp jsh"
${mangledir}snap $quiet $snaptols $mtol jshp jsh || exit
echo "${mangledir}snap $quiet $snaptols $mtol jsu jsf jsh jsufh"
${mangledir}snap $quiet $snaptols $mtol jsu jsf jsh jsufh || exit
echo "${mangledir}balkanize $quiet $mtol jsufh jsb"
${mangledir}balkanize $quiet $mtol jsufh jsb || exit
echo "${mangledir}weight $quiet $mtol -z2QZ10k jsb jsw"
${mangledir}weight $quiet $mtol -z2QZ10k jsb jsw || exit
echo "${mangledir}unify $quiet $mtol jsw $spol"
${mangledir}unify $quiet $mtol jsw $spol || exit
echo "Polygons for 2QZ 10k South are in $spol"

# Harmonics

if [ -z "$quiet" ]; then
    echo "===========================================";
    echo "                 THE WORLD                 ";
fi

# name of output file to contain harmonics
wlm=2qz_pixel.wlm

echo "${mangledir}harmonize $quiet -l$lmax $npol $spol $wlm"
${mangledir}harmonize $quiet $mtol -l$lmax $npol $spol $wlm || exit
echo "Harmonics for 2QZ 10k mask up to l = $lmax are in $wlm"

# Map

# name of output file to contain map
map=2qz_pixel.map

echo "${mangledir}map $quiet -w$wlm azel.dat $map"
${mangledir}map $quiet -w$wlm azel.dat $map || exit
echo "Map of 2QZ 10k mask up to l = $lmax is in $map"

# Vertices

# name of output file to contain vertices
vert=2qz_pixel.vrt

echo "${mangledir}poly2poly -ov $quiet $npol $spol $vert"
${mangledir}poly2poly -ov $quiet $mtol $npol $spol $vert || exit
echo "Vertices of 2QZ 10k mask are in $vert"

# Graphics

# name of output file to contain graphics
grph=2qzpixel.grph

# number of points per (2 pi) along each edge of a polygon
pts_per_twopi=30

echo "${mangledir}poly2poly -og$pts_per_twopi $quiet $npol $spol $grph"
${mangledir}poly2poly -og$pts_per_twopi $quiet $mtol $npol $spol $grph || exit
echo "Data suitable for plotting polygons of the 2QZ 10k mask are in $grph:"
echo "each line is a sequence of az, el points delineating the perimeter of a polygon."

# remove temporary files
rm j[ns]*

if [ -z "$quiet" ]; then
    echo "===========================================";
    echo "the universe?"
fi

if [ "$quiet" ]; then
    echo ""
    echo "If you'd like to repeat that with the sound on,"
    echo "please turn off the quiet button in 2qz_pixel and try again."
fi
