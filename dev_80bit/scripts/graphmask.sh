#!/bin/sh
# © M E C Swanson 2007
#plot the angular mask described by a polygon file in .list format
#using the matlab mapping toolbox
#optional 3rd argument turns on drawing black outlines around each polygon
#graphmask.sh <infile> <outfile> [<outlines>]
#EXAMPLES: 
#no outlines: graphmask.sh "dr4/safe0/sdss_dr4safe0_mask.list" "dr4/safe0/sdss_dr4safe0_mask.eps"
#outlines: graphmask.sh "allsky_3s.list" "allsky_3s.eps" on


matlab -nodisplay -r graphmask\(\'$1\',\'$2\',\'$3\'\)
echo all done!
