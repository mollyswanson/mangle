#! /bin/sh

user=`whoami`
names=`finger $user | fgrep "ame:" | sed 's/.*: *\([^ ]*\)[^:]*/\1/'`
for name in ${names}; do break; done
echo "Hello $name, watch me make harmonics for the 2dF 100k survey."
echo "If this works, I will be impressed.  It may take a while to complete."

# to make verbose
quiet=
# to make quiet
#quiet=-q

# snap tolerances
snaptols="-a2 -b2 -t2"

#multiple intersection tolerance
mtol="-m1e-5"

#pixelization: pixelize to default max resolution
pix="-P0"

# maximum harmonic number
lmax=20

mangledir=../../bin/

# North

if [ -z "$quiet" ]; then
    echo "===========================================";
    echo "                   NORTH                   ";
fi

# name of output file to contain 2dF 100k North polygons
npol=2df100k_north.pol

# -vo says to use old polygon ids, which are field numbers
echo "${mangledir}pixelize $quiet $mtol $pix -vo ngp_fields.dat jnfp"
${mangledir}pixelize $quiet $mtol $pix -vo ngp_fields.dat jnfp || exit
echo "${mangledir}pixelize $quiet $mtol $pix -vo ngpholes.dat jnhp"
${mangledir}pixelize $quiet $mtol $pix -vo ngpholes.dat jnhp || exit
echo "${mangledir}pixelize $quiet $mtol $pix -vo centres.ngp jncp"
${mangledir}pixelize $quiet $mtol $pix -vo centres.ngp jncp || exit
echo "${mangledir}snap $quiet $snaptols $mtol -vo jnfp jnf"
${mangledir}snap $quiet $snaptols $mtol -vo jnfp jnf || exit
echo "${mangledir}snap $quiet $snaptols $mtol -vo jnhp jnh"
${mangledir}snap $quiet $snaptols $mtol -vo jnhp jnh || exit
# -n intersects holes with their parent fields
echo "${mangledir}poly2poly $quiet $mtol -n jnh jnf jnhf"
${mangledir}poly2poly $quiet $mtol -n jnh jnf jnhf || exit
echo "${mangledir}balkanize $quiet $mtol jncp jnhf jnb"
${mangledir}balkanize $quiet $mtol jncp jnhf jnb || exit
echo "${mangledir}weight $quiet $mtol -z2dF100k jnb jnw"
${mangledir}weight $quiet $mtol -z2dF100k jnb jnw || exit
echo "${mangledir}unify $quiet $mtol jnw $npol"
${mangledir}unify $quiet $mtol jnw $npol || exit
echo "Polygons for 2dF 100k North are in $npol"

# South

if [ -z "$quiet" ]; then
    echo "===========================================";
    echo "                   SOUTH                   ";
fi

# name of output file to contain 2dF 100k South polygons
spol=2df100k_south.pol

# -vo says to use old polygon ids, which are field numbers
echo "${mangledir}pixelize $quiet $mtol $pix -vo sgp_fields.dat jsfp"
${mangledir}pixelize $quiet $mtol $pix -vo sgp_fields.dat jsfp || exit
echo "${mangledir}pixelize $quiet $mtol $pix -vo sgpholes.dat jshp"
${mangledir}pixelize $quiet $mtol $pix -vo sgpholes.dat jshp || exit
echo "${mangledir}pixelize $quiet $mtol $pix -vo centres.sgp jscp"
${mangledir}pixelize $quiet $mtol $pix -vo centres.sgp jscp || exit
echo "${mangledir}pixelize $quiet $mtol $pix -vo centres.ran jrcp"
${mangledir}pixelize $quiet $mtol $pix -vo centres.ran jrcp || exit
echo "${mangledir}snap $quiet $snaptols $mtol -vo jsfp jsf"
${mangledir}snap $quiet $snaptols $mtol -vo jsfp jsf || exit
echo "${mangledir}snap $quiet $snaptols $mtol -vo jshp jsh"
${mangledir}snap $quiet $snaptols $mtol -vo jshp jsh || exit
# -n intersects holes with their parent fields
echo "${mangledir}poly2poly $quiet $mtol -n jsh jsf jshf"
${mangledir}poly2poly $quiet $mtol -n jsh jsf jshf || exit
echo "${mangledir}balkanize $quiet $mtol centres.sgp centres.ran jshf jsb"
${mangledir}balkanize $quiet $mtol jscp jrcp jshf jsb || exit
echo "${mangledir}weight $quiet $mtol -z2dF100k jsb jsw"
${mangledir}weight $quiet $mtol -z2dF100k jsb jsw || exit
echo "${mangledir}unify $quiet $mtol jsw $spol"
${mangledir}unify $quiet $mtol jsw $spol || exit
echo "Polygons for 2dF 100k South are in $spol"

# Harmonics

if [ -z "$quiet" ]; then
    echo "===========================================";
    echo "                 THE WORLD                 ";
fi

# name of output file to contain harmonics
wlm=2df100k.wlm

echo "${mangledir}harmonize $quiet $mtol -l$lmax $npol $spol $wlm"
${mangledir}harmonize $quiet $mtol -l$lmax $npol $spol $wlm || exit
echo "Harmonics for 2dF 100k mask up to l = $lmax are in $wlm"

# Map

# name of output file to contain map
map=2df100k.map

echo "${mangledir}map $quiet -w$wlm azel.dat $map"
${mangledir}map $quiet -w$wlm azel.dat $map || exit
echo "Map of 2dF 100k mask up to l = $lmax is in $map"

# Vertices

# name of output file to contain vertices
vert=2df100k.vrt

echo "${mangledir}poly2poly -ov $mtol $quiet $npol $spol $vert"
${mangledir}poly2poly -ov $quiet $mtol $npol $spol $vert || exit
echo "Vertices of 2dF 100k mask are in $vert"

# Graphics

# name of output file to contain graphics
grph=2df100k.grph

# number of points per (2 pi) along each edge of a polygon
pts_per_twopi=30

echo "${mangledir}poly2poly -og$pts_per_twopi $quiet $mtol $npol $spol $grph"
${mangledir}poly2poly -og$pts_per_twopi $quiet $mtol $npol $spol $grph || exit
echo "Data suitable for plotting polygons of the 2dF 100k mask are in $grph:"
echo "each line is a sequence of az, el points delineating the perimeter of a polygon."

# for plotting with the matlab script
# name of output file to contain list format
list=2df100k.list
echo "${mangledir}poly2poly -ol$pts_per_twopi $quiet $mtol $npol $spol $list"
${mangledir}poly2poly -ol$pts_per_twopi $quiet $mtol $npol $spol $list || exit
echo "Data suitable for plotting polygons of the 2dF 100k mask in matlab with the mapping toolbox are in $list"
echo "Use the graphmask matlab function available on the mangle website to plot the mask."


# remove temporary files
rm j[nsr]*

if [ -z "$quiet" ]; then
    echo "===========================================";
    echo "the universe?"
fi

if [ "$quiet" ]; then
    echo ""
    echo "If you'd like to repeat that with the sound on,"
    echo "please turn off the quiet button in 2df100k and try again."
fi
