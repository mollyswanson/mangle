#!/bin/sh
# © M E C Swanson 2008
#plot the angular mask described by a polygon file in .list format
#using the matlab mapping toolbox
#optional 3rd argument turns on drawing black outlines around each polygon
#if specifying the full path to a file (rather than just running in the directory 
#containing the file you want to plot), put the path in double-quotes as shown below.
#graphmask.sh <infile> <outfile> [<outlines>]
#EXAMPLES: 
#no outlines: graphmask.sh "dr4/safe0/sdss_dr4safe0_mask.list" "dr4/safe0/sdss_dr4safe0_mask.eps"
#outlines: graphmask.sh allsky_3s.list allsky_3s.eps on

if [ "$MANGLESCRIPTSDIR" = "" ] ; then
    MANGLESCRIPTSDIR="../../scripts"
fi

matlab -nodisplay -r addpath\(\'$MANGLESCRIPTSDIR\'\)\;graphmask\(\'$1\',\'$2\',\'$3\'\)
echo all done!
