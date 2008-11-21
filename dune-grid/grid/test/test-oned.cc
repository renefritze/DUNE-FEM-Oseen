// $Id: test-oned.cc 3337 2006-12-06 16:18:44Z oliver $

#include <config.h>

#include <iostream>
#include <vector>

#include <dune/grid/onedgrid.hh>

#include "gridcheck.cc"
#include "checkgeometryinfather.cc"
#include "checkintersectionit.cc"

int main () try
{

    std::vector<double> coords(6);
    coords[0] = -1;
    coords[1] = -0.4;
    coords[2] = 0.1;
    coords[3] = 0.35;
    coords[4] = 0.38;
    coords[5] = 1;
    
    // extra-environment to check destruction
    {
      std::cout << std::endl << "OneDGrid" << std::endl << std::endl;
      Dune::OneDGrid grid(coords);

      // check macro grid 
      gridcheck(grid);
      
      // create hybrid grid 
      grid.mark(1, grid.leafbegin<0>());
      grid.preAdapt();
      grid.adapt();
      grid.postAdapt();
      checkIntersectionIterator(grid);

      // check the grid again
      gridcheck(grid);

      grid.globalRefine(1);
      gridcheck(grid);

      // check the method geometryInFather()
      checkGeometryInFather(grid);

      // check the intersection iterator
      checkIntersectionIterator(grid);
    };

  return 0;

 } catch (Dune::Exception &e) {
    std::cerr << e << std::endl;
    return 1;
 } catch (...) {
    std::cerr << "Generic exception!" << std::endl;
    return 2;
 }
