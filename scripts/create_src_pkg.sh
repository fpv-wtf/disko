PROJECTNAME=disko
PROJECTVERSION=1.5.0
PNV=$PROJECTNAME-$PROJECTVERSION
TMPDIR=/tmp/$PNV
TARFILE=$PNV.tar

echo "creating source package"

mkdir -p $TMPDIR/doc/conf
cp README Makefile bin.def dep.def shlib.def SConstruct $TMPDIR
cp -R inc $TMPDIR
cp -R src $TMPDIR
cp -R tools $TMPDIR
cp -R share $TMPDIR
cp -R scons $TMPDIR
mkdir $TMPDIR/bin
cp ./doc/conf/$PROJECTNAME.conf $TMPDIR/doc/conf
cd /tmp
tar -cvf $TARFILE ./$PNV/README ./$PNV/Makefile ./$PNV/bin.def ./$PNV/dep.def ./$PNV/shlib.def ./$PNV/SConstruct ./$PNV/doc ./$PNV/share ./$PNV/bin >/dev/null
tar -rf $TARFILE `find ./$PNV/inc -name "*.h" -or -name "*.tmpl"`
tar -rf $TARFILE `find ./$PNV/src -name "*.cpp" -or -name "Makefile" -or -name "SConscript"`
tar -rf $TARFILE `find ./$PNV/tools -name "*.cpp" -or -name "Makefile" -or -name "SConscript"`
tar -rf $TARFILE `find ./$PNV/scons -name "*.py"`
bzip2 $TARFILE
cd - >/dev/null
mv /tmp/$TARFILE.bz2 .
rm -rf $TMPDIR

echo "-> $TARFILE.bz2"
