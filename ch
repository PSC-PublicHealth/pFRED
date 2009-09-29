#!/bin/csh -f
#
#	file: ch
#
# csh file for updating the params file

if ( $#argv != 1 && $#argv != 2 && $#argv != 3) then
	echo "Not enough arguments."
	echo "USAGE: ch parameter-name [ parameter-value [params-file] ]"
	exit 1
endif

if ( $#argv == 1 ) then
#
# remove the named param from the params file,
# this reverting to the default value in params.def
#
  set x="$1 = "
  grep "^$x" params.def > ,ch.tmp
  if (-z ,ch.tmp) then
    echo $1 not a valid parameter
  else
    grep "^$x" params > ,ch.tmp
    if (-z ,ch.tmp) then
    else
      set x="$1 = .*"
      sed "/^$x/d" params > ,ch.tmp
      mv ,ch.tmp params
    endif
  endif
  rm -f ,ch.tmp

else
 if ( $#argv == 2 ) then
  set p="params"
 else
  set p=$3
 endif
#
# change the line for this param in the param file.
# if it doesn't occur, add a line to the params file.
#
  set x="$1 = "
  set y="$1 = $2"
  grep "^$x" params.def > ,ch.tmp
  if (-z ,ch.tmp) then
    echo $1 not a valid parameter
  else
    grep "^$x" $p > ,ch.tmp
    if (-z ,ch.tmp) then
      echo $y >> $p
    else
      set x="$1 = .*"
      sed "/^$x/s//$y/" $p > ,ch.tmp
      mv ,ch.tmp $p
    endif
    grep "$y" $p
  endif
  rm -f ,ch.tmp
endif


