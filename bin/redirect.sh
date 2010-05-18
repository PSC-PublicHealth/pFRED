#!/bin/sh
#
# automated redirection to a new repository
# Nathan Stone (2010)
#
# CVS/Root contents look like:
# username@fully.qualified.domain.name:/full/path/to/cvsroot

usage(){
    echo "Usage: $0 [-r]"
    echo " -r : revert back to original repository"
    echo ""
    echo "NOTE:"
    echo " - Invoke this script from your FRED directory"
    echo " - be sure to add \"GSSAPIDelegateCredentials yes\" to ~/.ssh/config"
    exit 0
}

if [ "$1" = "-r" ]; then revert=1 ; fi
if [ "$1" = "-h" -o "$1" = "--help" ]; then usage ; fi

if [ ! -e CVS/Repository ]; then usage ; fi
repodir=`cat CVS/Repository 2>/dev/null`
if [ "$repodir" != "FRED" ]; then usage ; fi

# use $LOGNAME as an initial guess for the remote username
username=$LOGNAME
host=repository.psc.edu
path=/afs/psc.edu/projects/midas/cvsrep

if [ "$revert" != "1" ]; then 
    echo -n "Remote username [<return> to use \"$username\"] "
    read ANS
    if [ "$ANS" != "" ]; then
	username=$ANS
	echo "using: \"$ANS\""
    fi
fi

# create a viable CVS/Root file in temp space
tempfile=.root
echo "$username@$host:$path" > $tempfile

# find all CVS/Root files and replace them
# (make a backup to allow future recovery)
for dir in `find . -name CVS` ; do
    if [ -d $dir -a -e $dir/Root ]; then
	root=$dir/Root

	if [ "$revert" != "1" ]; then 
	    # don't overwrite the original... to allow true revert
	    if [ ! -e $root.backup ]; then
		cp -p $root $root.backup
		cp $tempfile $root
	    fi
	else
	    if [ ! -e $root.backup ]; then
		echo "no $root.backup available"
	    else
		cp -p $root.backup $root
	    fi
	fi
    fi
done

# get rid of the evidence
rm -f $tempfile
