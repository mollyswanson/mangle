#! /bin/sh
# © M E C Swanson 2007
#script to find the complement of a mangle mask
#USAGE: find_complement.sh <mask> <complement>
#EXAMPLE:find_complement.sh holes.pol complement.pol
#input mask can be pixelized beforehand to a fixed resolution
#if the mask is complicated, e.g. pixelize -Ps0,8 holes.pol holes_pixelized.pol

mask=$1
complement=$2

awk '/pixelization/{print $0}' < $mask > jpix
res=`awk '{print substr($2, 1, length($2)-1)}' < jpix`
scheme=`awk '{print substr($2, length($2))}' < jpix`
if [ $res -eq -1 ] ; then
    echo "ERROR: cannot take the complement of a mask pixelized adaptively."
    echo "Pixelize your mask using a fixed resolution, e.g. -Ps0,8, and try again."
exit 1
fi

allsky=$MANGLEDATADIR/allsky/allsky$res$scheme.pol

if [ ! -e $allsky ] ; then
    echo "ERROR: file $allsky does not exist."
    echo "You can create it by running" 
    echo "pixelize -P${scheme}0,${res} allsky.pol allsky$res$scheme.pol" 
    echo "in $MANGLEDATADIR/allsky, or download it from the mangle website."
    exit 1
fi

echo 0 > jw0
echo "$MANGLEBINDIR/snap $mask js" 
$MANGLEBINDIR/snap $mask js || exit
echo "$MANGLEBINDIR/weight -zjw0 js jw"
$MANGLEBINDIR/weight -zjw0 js jw || exit
echo "$MANGLEBINDIR/balkanize $allsky jw jb"
$MANGLEBINDIR/balkanize $allsky jw jb || exit
echo "$MANGLEBINDIR/unify jb $complement"
$MANGLEBINDIR/unify jb $complement || exit

echo "Complement of $mask written to ${complement}."

rm j*
