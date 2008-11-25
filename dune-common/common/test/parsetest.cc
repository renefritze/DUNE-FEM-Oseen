// $Id: parsetest.cc 4848 2007-02-16 13:01:46Z sander $

/* test if the common headers can be parsed without errors */
#include "config.h"

/*

to create all the includes the following commandline is pretty handy:

find dune/common/ -maxdepth 1 -name \*.hh -exec echo '#include <{}>' \; | sort
  
 */

#include <dune/common/alignment.hh>
#include <dune/common/array.hh>
#include <dune/common/arraylist.hh>
#include <dune/common/bigunsignedint.hh>
#include <dune/common/bitfield.hh>
#include <dune/common/configparser.hh>
#include <dune/common/debugstream.hh>
#include <dune/common/dlist.hh>
#include <dune/common/enumset.hh>
#include <dune/common/exceptions.hh>
#include <dune/common/fixedarray.hh>
#include <dune/common/fmatrix.hh>
#include <dune/common/fvector.hh>
#include <dune/common/gcd.hh>
#include <dune/common/genericiterator.hh>
#include <dune/common/helpertemplates.hh>
#include <dune/common/interfaces.hh>
#include <dune/common/iteratorfacades.hh>
#include <dune/common/lcm.hh>
#include <dune/common/misc.hh>
#include <dune/common/poolallocator.hh>
#include <dune/common/precision.hh>
#include <dune/common/propertymap.hh>
#include <dune/common/sllist.hh>
#include <dune/common/smartpointer.hh>
#include <dune/common/stack.hh>
#include <dune/common/stdstreams.hh>
#include <dune/common/timer.hh>
#include <dune/common/tuples.hh>
#include <dune/common/typetraits.hh>
#include <dune/common/bartonnackmanifcheck.hh>

int main () {
}
