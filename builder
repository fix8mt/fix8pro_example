#!/bin/bash
#############################################################################################
#   ____                      __      ____
#  /\  _`\   __             /'_ `\   /\  _`\
#  \ \ \L\_\/\_\    __  _  /\ \L\ \  \ \ \L\ \ _ __    ___
#   \ \  _\/\/\ \  /\ \/'\ \/_> _ <_  \ \ ,__//\`'__\ / __`\
#    \ \ \/  \ \ \ \/>  </   /\ \L\ \  \ \ \/ \ \ \/ /\ \L\ \
#     \ \_\   \ \_\ /\_/\_\  \ \____/   \ \_\  \ \_\ \ \____/
#      \/_/    \/_/ \//\/_/   \/___/     \/_/   \/_/  \/___/
#
#              Fix8Pro FIX Engine and Framework
#
# Copyright (C) 2010-21 Fix8 Market Technologies Pty Ltd (ABN 29 167 027 198)
# All Rights Reserved. [http://www.fix8mt.com] <heretohelp@fix8mt.com>
#
# THIS FILE IS PROPRIETARY AND  CONFIDENTIAL. NO PART OF THIS FILE MAY BE REPRODUCED,  STORED
# IN A RETRIEVAL SYSTEM,  OR TRANSMITTED, IN ANY FORM OR ANY MEANS,  ELECTRONIC, PHOTOSTATIC,
# RECORDED OR OTHERWISE, WITHOUT THE PRIOR AND  EXPRESS WRITTEN  PERMISSION  OF  FIX8  MARKET
# TECHNOLOGIES PTY LTD.
#
#############################################################################################

copyright()
{
	dy=`date +%y`
	echo "Fix8Pro Samples Builder - Version 1.0"
	echo "Copyright (c) 2010-$dy Fix8 Market Technologies Pty Ltd (ABN 29 167 027 198). All Rights Reserved. [https://www.fix8mt.com] <heretohelp@fix8mt.com>"
}

usage()
{
	cat <<-EOF

USAGE: $0 [--option[=val]]

    You will prompted for required settings not passed on command line

OPTIONS

    -c --config       debug or relwithdebinfo, default is ${config}
    -l --license      set FIX8PRO license file path, default is ${license} or taken from \$FIX8PRO_LICENSE_FILE
    -r --root         path to fix8pro install, default is ${root}
    -h --help         this screen
    -V --verbose      be verbose when building

EXAMPLES

    builder --config=debug --license=~/fix8pro-dev.lic --root=~/src/fix8pro/latest
    builder --config=relwithdebinfo --verbose
    builder --license=\$FIX8PRO_LICENSE_FILE --root=~/src/fix8pro/latest

EOF
	return;
}

defpath="/opt/fix8pro/latest"
root=${defpath}
license="${defpath}/fix8pro-dev.lic"
if [ "$FIX8PRO_LICENSE_FILE" != "" ]; then
  license="$FIX8PRO_LICENSE_FILE"
fi
config="debug"
verbose="OFF"
removeold=0

get_abs_filename() {
  # $1 : relative filename
  echo "$(cd "$(dirname "$1")" && pwd)/$(basename "$1")"
}

while [ "$1" != "" ]; do
	param=$(echo $1 | awk -F= '{print $1}')
	val=$(echo $1 | sed 's/^[^=]*=//g')
	case $param in
		--config | -c) config=$val ;;
		--license | -l) license=$val ;;
		--root | -r) root=$val ;;
		--help | -h) copyright; usage; exit 0 ;;
		--verbose | -V) verbose="ON" ;;
		*) echo "ERROR: unknown parameter \"$param\""
			usage; exit 1 ;;
	esac
	shift
done

# locate and copy cmake file in script directory
cmdir=$(echo $(dirname $(readlink -m "$0")))
cmdcmake="$cmdir/builder.cmake"
if [ "$cmdir" = "$(pwd)" ]; then
	echo "WARNING: This script should not be run from the script source directory."
fi

echo "Fix8Pro Examples Builder"
if [ -z "${root}" ]; then
	echo -n "Enter full path to your Fix8Pro base install directory [$root]: "
	read fix8
	if [ -z "${root}" ]; then
		root=$defpath
	fi
fi
echo
mkdir -p .build_${config}
cd .build_${config} || exit
cmake -DCMAKE_VERBOSE_MAKEFILE=$verbose -DCMAKE_BUILD_TYPE="${config}" -DFIX8PRO_ROOT="${root}" -DFIX8PRO_LICENSE_FILE="${license}" ..
make -j install
cd ..
