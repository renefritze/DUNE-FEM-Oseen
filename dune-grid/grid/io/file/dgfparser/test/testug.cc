#include <config.h>
#include <iostream> 
#include <string>

#include <dune/grid/io/file/dgfparser/dgfparser.hh>

// use grid check 
#include <dune/grid/test/gridcheck.cc>

using namespace Dune;

int main(int argc, char ** argv, char ** envp) 
try {
  std::cout << std::endl << "start UGGrid test" << std::endl;
    
  // this method calls MPI_Init, if MPI is enabled
  MPIHelper::instance(argc,argv);

  {
    std::cout << std::endl << "check UGGrid<2>" << std::endl;
    typedef UGGrid<2> GridType;
    std::string filename(SRCDIR"/examplegrid5.dgf");
    GridPtr<GridType> gridptr(filename); 

    // run grid check to check grid 
    gridcheck(*gridptr);
  }

  {
    std::cout << std::endl << "check UGGrid<3>" << std::endl;
    typedef UGGrid<3> GridType; 
    std::string filename(SRCDIR"/examplegrid6.dgf");
    GridPtr<GridType> gridptr(filename); 

    // run grid check to check grid 
    gridcheck(*gridptr);
  }

  return 0;
}
catch (Dune::Exception &e) {
  std::cerr << e << std::endl;
  return 1;
}
catch (...) {
  std::cerr << "Generic exception!" << std::endl;
  return 1;
}
