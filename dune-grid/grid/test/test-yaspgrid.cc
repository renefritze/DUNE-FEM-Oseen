// $Id: test-yaspgrid.cc 4034 2008-03-06 14:14:56Z robertk $

#include <config.h>

#include <iostream>

#include <dune/grid/yaspgrid.hh>

#include "gridcheck.cc"
#include "checkcommunicate.cc"
#include "checkgeometryinfather.cc"
#include "checkintersectionit.cc"


int rank;

template <int dim>
void check_yasp() {
  typedef Dune::FieldVector<int,dim> iTupel;
  typedef Dune::FieldVector<double,dim> fTupel;
  typedef Dune::FieldVector<bool,dim> bTupel;

  std::cout << std::endl << "YaspGrid<" << dim << "," << dim << ">" 
	    << std::endl << std::endl;

  fTupel Len; Len = 1.0;
  iTupel s; s = 3;
  bTupel p; p = false;
  int overlap = 1;

#if HAVE_MPI
  Dune::YaspGrid<dim,dim> grid(MPI_COMM_WORLD,Len,s,p,overlap);
#else
  Dune::YaspGrid<dim,dim> grid(Len,s,p,overlap);
#endif

  grid.globalRefine(2);
  
  gridcheck(grid);

  // check communication interface 
  checkCommunication(grid,-1,Dune::dvverb);
  for(int l=0; l<=grid.maxLevel(); ++l)
    checkCommunication(grid,l,Dune::dvverb);

  // check the method geometryInFather()
  checkGeometryInFather(grid);
  // check the intersection iterator and the geometries it returns
  checkIntersectionIterator(grid);
};

int main (int argc , char **argv) {
  try {
#if HAVE_MPI
    // initialize MPI
    MPI_Init(&argc,&argv);

    // get own rank
    MPI_Comm_rank(MPI_COMM_WORLD,&rank);
#endif    

    check_yasp<1>();
    check_yasp<2>();
    check_yasp<3>();

  } catch (Dune::Exception &e) {
    std::cerr << e << std::endl;
    return 1;
  } catch (...) {
    std::cerr << "Generic exception!" << std::endl;
    return 2;
  }
  
#if HAVE_MPI
  // Terminate MPI
  MPI_Finalize();
#endif

  return 0;
};
