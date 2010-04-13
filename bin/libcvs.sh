# Homepage: http://code.google.com/p/coderev
# License: GPLv2, see "COPYING"
#
# This library implements cvs operations, see comments in coderev.sh
#
# $Id: libcvs.sh,v 1.1 2010-04-13 15:55:35 nstone Exp $

function cvs_get_banner
{
    echo "CVS"
    return 0
}

function cvs_get_repository
{
    cat CVS/Root
}

function cvs_get_project_path
{
    cat CVS/Repository
}

function cvs_get_working_revision
{
    # CVS doesn't have a global revision number, just fetch revision of the
    # first file or "."
    #
    local pathname="."
    [[ -n $1 ]] && [[ -z $2 ]] && pathname=$1
    cvs st $pathname 2>/dev/null | grep 'Working revision:.*\.' | head -1 \
        | sed 's/.*Working revision://' | awk '{print $1}'
}

function cvs_get_active_list
{
    cvs st $@ | grep File: \
        | awk '$4 != "Up-to-date" && $4 != "Unknown" {print $2}'
}

function cvs_get_diff
{
    local op diff_opt OPTIND OPTARG

    while getopts "r:" op; do
        case $op in
            r) diff_opt="-r $OPTARG" ;;
            ?) echo "Unknown option: -$op" >&2; exit 1;;
        esac
    done
    shift $((OPTIND - 1))

    # patch utility sometimes fails if no context line
    # Issue 2 suffers by keywords in context
    cvs diff -U5 $diff_opt $@
    return 0 # cvs diff return 1 when there're changes
}

function cvs_get_diff_opt
{
    echo "-r $1"
}
