#!/bin/bash

if test $# -lt 1 ; then
  echo "Usage: $0 <header>"
  exit 1
fi

CXX="g++"

WORKINGDIR=`pwd`
cd `dirname $0`
SCRIPTSDIR=`pwd`
cd "$SCRIPTSDIR/.."
FEMDIR=`pwd`
HEADER=$1

if ! test -e $HEADER ; then
  echo "'$HEADER' does not exist."
  exit 1
fi

cd $WORKINGDIR
BASEFILE=`mktemp -p . tmp-header-XXXXXX`
MAKEFILE=$BASEFILE.make
CCFILE=$BASEFILE.cc
OFILE=$BASEFILE.o

echo "GRIDTYPE=ALUGRID_SIMPLEX" >> $MAKEFILE
echo "GRIDDIM=2" >> $MAKEFILE
echo ".cc.o:" >> $MAKEFILE
echo -e -n "\t$CXX -c -I$FEMDIR" >> $MAKEFILE
echo ' -I/data/dune_work/private/f_albr01/diplomarbeit_felix_rene/dune-common -I/data/dune_work/private/f_albr01/diplomarbeit_felix_rene/dune-grid -I/data/dune_work/private/f_albr01/diplomarbeit_felix_rene/dune-istl   -I/data/dune_work/private/f_albr01/diplomarbeit_felix_rene/dune-common -I/data/dune_work/private/f_albr01/diplomarbeit_felix_rene/dune-grid -I/data/dune_work/private/f_albr01/diplomarbeit_felix_rene/dune-istl -DGRIDDIM=$(GRIDDIM) -D$(GRIDTYPE) -I/share/dune/Modules/modules_x86_64/grape -I/usr/X11R6/include -pthread -I/share/dune/Modules/modules_x86_64/ALUGrid-1.1_Parallel/include -I/share/dune/Modules/modules_x86_64/ALUGrid-1.1_Parallel/include/serial -I/share/dune/Modules/modules_x86_64/ALUGrid-1.1_Parallel/include/duneinterface -DENABLE_ALUGRID -O0 -o $@ $<' >> $MAKEFILE

echo "#include <config.h>" >> $CCFILE
echo "#include <${HEADER}>" >> $CCFILE
echo "#include <${HEADER}>" >> $CCFILE
echo "int main () {}" >> $CCFILE

make -f $MAKEFILE $OFILE 1>/dev/null
SUCCESS=$?

rm -f $OFILE
rm -f $CCFILE
rm -f $MAKEFILE
rm -f $BASEFILE

exit $SUCCESS
