#! /bin/sh
# © M E C Swanson 2008
#script to set mangle environment variables
#USAGE: type 'source setup_mangle_environment.sh' in mangle 'scripts' directory
#Or use 'source <MANGLEDIR>/scripts/setup_mangle_environment.sh <MANGLEDIR>'
#where <MANGLEDIR> is the path to the base mangle directory, e.g., /home/username/mangle2.0
#
#To run this script automatically when you start your shell, add the following line
#to your .bashrc (or .tcshrc, or .cshrc, or .login, or .profile) (replace <MANGLEDIR>
#with the path to your mangle installation):
#
#source <MANGLEDIR>/scripts/setup_mangle_environment.sh <MANGLEDIR>


#if running in the 'scripts' directory, set the value of $MANGLEDIR to be the 
#name of the directory one level higher
CURRENTDIR=`basename $PWD`
if [ "$CURRENTDIR" = "scripts" ]; then
    cd ..
    MANGLEDIR="$PWD"
    cd $CURRENTDIR
#otherwise use the path in the first command-line argument as $MANGLEDIR
else
    MANGLEDIR=$1
fi

MANGLEBINDIR="$MANGLEDIR/bin"
MANGLESCRIPTSDIR="$MANGLEDIR/scripts"
MANGLEDATADIR="$MANGLEDIR/masks"

#check to make sure directories exist
if [ ! -d $MANGLEBINDIR ]; then
    echo >&2 "ERROR: The directory $MANGLEBINDIR does not exist"
fi
if [ ! -d $MANGLESCRIPTSDIR ]; then
    echo >&2 "ERROR: The directory $MANGLESCRIPTSDIR does not exist"
fi
if [ ! -d $MANGLEDATADIR ]; then
    echo >&2 "ERROR: The directory $MANGLEDATADIR does not exist"
fi
if [ ! -d $MANGLEBINDIR ] || [ ! -d $MANGLESCRIPTSDIR ] || [ ! -d $MANGLEDATADIR ]; then
    echo >&2 ""
    echo >&2 "USAGE: type 'source setup_mangle_environment.sh' in mangle 'scripts' directory."
    echo >&2 "Or use 'source <MANGLEDIR>/scripts/setup_mangle_environment.sh <MANGLEDIR>'"
    echo >&2 "where <MANGLEDIR> is the path to the base mangle directory," 
    echo >&2 "e.g., /home/username/mangle2.0"
    return
fi
 
CURRENTSHELL=`basename $SHELL`

#export environment variables and put bin and scripts directories in the path
case $CURRENTSHELL in
    sh|bash|ksh)
	export MANGLEBINDIR
	export MANGLESCRIPTSDIR
	export MANGLEDATADIR
	export PATH=$PATH:$MANGLEBINDIR:$MANGLESCRIPTSDIR
	;;
    csh|tcsh)
	setenv MANGLEBINDIR $MANGLEBINDIR
	setenv MANGLESCRIPTSDIR $MANGLESCRIPTSDIR
	setenv MANGLEBINDIR $MANGLESCRIPTSDIR    
	setenv PATH $PATH:$MANGLEBINDIR:$MANGLESCRIPTSDIR
	;;
esac


