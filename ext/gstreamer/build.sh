#!/bin/bash -e
########################################################################
# Script to install gstreamer including the disko videosink plugin.
#
# usage: ./build.sh [options]
#
#     -h)        print this help text
#     -i prefix) set installation prefix for gstreamer
########################################################################

function usage() {
    echo "usage: $0 [option]"
    echo
    echo "    -h)        print this help text"
    echo "    -i prefix) set installation prefix for gstreamer"
}

while getopts  "hi:" flag; do
    case $flag in
        h) usage; exit 1;;
        i) installprefix="--prefix=$OPTARG"; 
           export PKG_CONFIG_PATH=$PKG_CONFIG_PATH:$OPTARG/lib/pkgconfig;;
    esac
done

[ ! -f gstreamer-0.10.23.tar.bz2 ] && \
    wget http://gstreamer.freedesktop.org/src/gstreamer/gstreamer-0.10.23.tar.bz2;
    
[ ! -d gstreamer-0.10.23 ] && \
    tar jxf gstreamer-0.10.23.tar.bz2;

cd gstreamer-0.10.23;
./configure "$installprefix" && make && make install;
cd -

[ ! -f gst-plugins-base-0.10.23.tar.bz2 ] && \
    wget http://gstreamer.freedesktop.org/src/gst-plugins-base/gst-plugins-base-0.10.23.tar.bz2;
    
[ ! -d gst-plugins-base-0.10.23 ] && \
    tar jxf gst-plugins-base-0.10.23.tar.bz2;

cd gst-plugins-base-0.10.23;
./configure "$installprefix" && make && make install;
cd -

[ ! -f gst-plugins-bad-0.10.13.tar.bz2 ] && \
    wget http://gstreamer.freedesktop.org/src/gst-plugins-bad/gst-plugins-bad-0.10.13.tar.bz2;

[ ! -d gst-plugins-bad-0.10.13 ] && \
    tar jxf gst-plugins-bad-0.10.13.tar.bz2;

cd gst-plugins-bad-0.10.13;

[ ! -d ext/disko ] && \
    patch -p1 < ../gst-plugins-bad-0.10.13_diskovideosink.diff;
    
./configure "$installprefix" && make && make install;
