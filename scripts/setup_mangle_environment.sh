# © M E C Swanson 2008
#script to set mangle environment variables
#USAGE: type 'source setup_mangle_environment.sh' in mangle 'scripts' directory
#Or use 'source <MANGLEDIR>scripts/setup_mangle_environment.sh <MANGLEDIR>'
#where <MANGLEDIR> is the path to the base mangle directory, e.g., /home/username/mangle2.0/
#
#To run this script automatically when you start your shell, add the following line
#to your .bashrc (or .tcshrc, or .cshrc, or .login, or .profile) (replace <MANGLEDIR>
#with the path to your mangle installation):
#
#source <MANGLEDIR>/scripts/setup_mangle_environment.sh <MANGLEDIR>

${1}scripts/make_setup_script.sh $1 || return
source setup.sh
/bin/rm setup.sh

