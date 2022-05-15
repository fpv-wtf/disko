#!/bin/bash
########################################################################
# Script to install xine including the BLDVB input plugin.
#
# usage: ./build.sh [options]
#
#     -h)        print this help text
#     -m prefix) set prefix for em28xx libmediaclient
#     -i prefix) set installation prefix for xine
########################################################################

libmediaclientprefix="";

function usage() {
    echo "usage: $0 [option]"
    echo
    echo "    -h)        print this help text"
    echo "    -m prefix) set prefix for em28xx libmediaclient"
    echo "    -i prefix) set installation prefix for xine"
}

while getopts  "hm:i:" flag; do
    case $flag in
        h) usage; exit 1;;
        m) libmediaclientprefix="$OPTARG";;
        i) installprefix="--prefix=$OPTARG";;
    esac
done

[ ! -f xine-lib-1.1.16.2.tar.bz2 ] && \
    wget http://prdownloads.sourceforge.net/xine/xine-lib-1.1.16.2.tar.bz2;

[ ! -d xine-lib-1.1.16.2 ] && \
    tar jxf xine-lib-1.1.16.2.tar.bz2;

cd xine-lib-1.1.16.2;

[ ! -f src/input/input_bldvb.cpp ] && \
    patch -p1 < ../xine-lib-1.1.16.2_bldvb.diff;


export LIBMEDIACLIENT_PREFIX="$libmediaclientprefix";
./configure --enable-libmediaclient "$installprefix" && \
    make && \
    make install;
