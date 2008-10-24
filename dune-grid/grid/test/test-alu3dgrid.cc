#include <config.h>

#include <iostream>
#include <sstream>
#include <string>

#include <dune/common/mpihelper.hh>

#include <dune/grid/io/file/dgfparser/dgfalu.hh>
#include "gridcheck.cc"

#include "checkgeometryinfather.cc"
#include "checkintersectionit.cc"
#include "checkcommunicate.cc"

using namespace Dune;

template <class GridType>
void makeNonConfGrid(GridType &grid,int level,int adapt) {
  int myrank = grid.comm().rank();
  grid.loadBalance();
  grid.globalRefine(level);
  grid.loadBalance();
  for (int i=0;i<adapt;i++) 
  {
    if (myrank==0) 
    {
      typedef typename GridType :: template Codim<0> :: 
            template Partition<Interior_Partition> :: LeafIterator LeafIterator;
      
      LeafIterator endit = grid.template leafend<0,Interior_Partition>   ();
      int nr = 0;
      int size = grid.size(0);
      for(LeafIterator it    = grid.template leafbegin<0,Interior_Partition> ();
          it != endit ; ++it,nr++ ) 
      {
        grid.mark(1,it);
        if (nr>size*0.8) break;
      }
    }
    grid.adapt();
    grid.postAdapt();
    grid.loadBalance();
  }
}

template <class GridType>
void checkIteratorAssignment(GridType & grid) 
{
  // check Iterator assignment
  {
    enum { dim = GridType :: dimension };
    typedef typename GridType :: template Codim<dim> :: LevelIterator
      IteratorType; 

    IteratorType it = grid.template lbegin<dim>(0);
    if( grid.maxLevel() > 0 ) it = grid.template lbegin<dim>(1);
  }

  {
    enum { dim = GridType :: dimension };
    typedef typename GridType :: template Codim<dim> :: LevelIterator
      IteratorType;
    typedef typename GridType::Traits::template Codim<dim>::EntityPointer EntityPointerType; 

    IteratorType it = grid.template lbegin<dim>(0);

    if( it != grid.template lend<dim>(0) )
    {
      assert( it->level() == 0 );
      EntityPointerType p = it; 
      
      assert( p.level()  == 0 );
      assert( p->level() == 0 );

      if( grid.maxLevel() > 0 )
      {
        it = grid.template lbegin<dim>(1);
        p = it;

        assert( it->level() == 1 );
        assert( p.level()   == 1 );
        assert( p->level()  == 1 );
      }
    }
  }
}
template <class GridType> 
void checkLevelIndexNonConform(GridType & grid) 
{
  typedef typename GridType :: template Codim<0> :: LeafIterator
          IteratorType;
  {
    IteratorType end = grid.template leafend<0>();
    for(IteratorType it = grid.template leafbegin<0>(); it!=end; ++it)
    {
      // call index of level index set
      grid.levelIndexSet(it->level()).index(*it);
    }
  }

  {
    IteratorType it = grid.template leafbegin<0>();
    if( it != grid.template leafend<0>() )
    {
      // mark first entity
      grid.mark(1, it);
    }
  }

  grid.preAdapt();
  grid.adapt();
  grid.postAdapt();

  {
    IteratorType end = grid.template leafend<0>();
    for(IteratorType it = grid.template leafbegin<0>(); it!=end; ++it)
    {
      // call index of level index set
      grid.levelIndexSet(it->level()).index(*it);
    }
  }
}

template <class GridType> 
void checkALUSerial(GridType & grid, int mxl = 2) 
{
  // be careful, each global refine create 8 x maxlevel elements 
  gridcheck(grid);
  for(int i=0; i<mxl; i++) {
    grid.globalRefine( DGFGridInfo<GridType> :: refineStepsForHalf() );
    gridcheck(grid);
  }

  // check also non-conform grids 
  makeNonConfGrid(grid,0,1);
  gridcheck(grid);
  
  // check the method geometryInFather()
  checkGeometryInFather(grid);
  // check the intersection iterator and the geometries it returns
  checkIntersectionIterator(grid);

  // some checks for assignment of iterators 
  checkIteratorAssignment(grid);

  checkLevelIndexNonConform(grid);
}
#if HAVE_MPI
template <class GridType> 
void checkALUParallel(GridType & grid, int gref, int mxl = 3) 
{
  makeNonConfGrid(grid,gref,mxl);

  // -1 stands for leaf check 
  checkCommunication(grid, -1, std::cout);

  for(int l=0; l<= mxl; ++l)
    checkCommunication(grid, l , Dune::dvverb);
}
#else
template <class GridType> 
void checkALUParallel(GridType & grid, int gref, int mxl = 3) 
{
}
#endif

int main (int argc , char **argv) {
  
  // this method calls MPI_Init, if MPI is enabled
  MPIHelper & mpihelper = MPIHelper::instance(argc,argv);
  int myrank = mpihelper.rank(); 
  int mysize = mpihelper.size();

  try {
    /* use grid-file appropriate for dimensions */

    // extra-environment to check destruction
    {
      factorEpsilon = 5.e+5;
      // check empty grid

      if (myrank == 0)
        std::cout << "Check empty grids" << std::endl;
      {
        ALUCubeGrid<3,3> grid;
        checkALUSerial(grid);
      }

      {
        ALUSimplexGrid<3,3> grid;
        checkALUSerial(grid);
      }
     
      // check non-confrom ALUGrid for 2d 
      {
        typedef ALUSimplexGrid<2,2> GridType;
        std::string filename("simplex-testgrid-2-2.dgf");
        GridPtr<GridType> gridPtr(filename);
        checkALUSerial(*gridPtr,2);
      }

      // check confrom ALUGrid for 2d 
      {
        typedef ALUConformGrid<2,2> GridType;
        std::string filename("simplex-testgrid-2-2.dgf");
        GridPtr<GridType> gridPtr(filename);
        checkALUSerial(*gridPtr,2);
      }

      {
        std::string filename;
        if (mysize<=2) 
          filename += "simplex-testgrid-3-3.dgf";
        else 
          filename += "cube-testgrid-3-3-large.dgf";

        typedef ALUCubeGrid<3,3> GridType;
        GridPtr<GridType> gridPtr(filename);
        GridType & grid = *gridPtr;

        if(myrank == 0) 
        {
          std::cout << "Check serial grid" << std::endl;
          checkALUSerial(grid,(mysize == 1) ? 1 : 0);
        }
        
        // perform parallel check only when more then one proc 
        if(mysize > 1)
        {
          if (myrank == 0) std::cout << "Check conform grid" << std::endl;
          checkALUParallel(grid,1,0);
          if (myrank == 0) std::cout << "Check non-conform grid" << std::endl;
          checkALUParallel(grid,0,2);
        }
      }

      {
        std::string filename;
        if (mysize<=2) 
          filename += "simplex-testgrid-3-3.dgf";
        else
          filename += "simplex-testgrid-3-3-large.dgf";
        
        typedef ALUSimplexGrid<3,3> GridType;
        GridPtr<GridType> gridPtr(filename);
        GridType & grid = *gridPtr;

        if (myrank == 0) 
        {
          std::cout << "Check serial grid" << std::endl;
          checkALUSerial(grid,(mysize == 1) ? 1 : 0);
        }

        // perform parallel check only when more then one proc 
        if(mysize > 1)
        {
          if (myrank == 0) std::cout << "Check conform grid" << std::endl;
          checkALUParallel(grid,0,0);  //1,3
          if (myrank == 0) std::cout << "Check non-conform grid" << std::endl;
          checkALUParallel(grid,0,2);  //1,3
        }
      }
    };

  } catch (Dune::Exception &e) {
    std::cerr << e << std::endl;
    return 1;
  } catch (...) {
    std::cerr << "Generic exception!" << std::endl;
    return 2;
  }

  return 0;
}
