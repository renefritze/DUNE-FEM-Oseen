// $Id: test-ug.cc 3776 2007-07-30 14:09:51Z sander $

#include <config.h>

#include <iostream>

/*

  Instantiate UG-Grid and feed it to the generic gridcheck()

*/

#include <dune/grid/uggrid.hh>
#include <dune/doc/grids/ugascplusplus/ugtestgrids.hh>

#include "gridcheck.cc"
#include "checkgeometryinfather.cc"
#include "checkintersectionit.cc"

// Test parallel interface if a parallel UG is used
#ifdef ModelP  
#include <mpi.h>
#include "checkcommunicate.cc"
#endif

using namespace Dune;

class ArcOfCircle : public Dune::BoundarySegment<2>
{
    public:

    ArcOfCircle(const Dune::FieldVector<double,2>& center, double radius,
                double fromAngle, double toAngle)
    : center_(center), radius_(radius), fromAngle_(fromAngle), toAngle_(toAngle)
    {}

    Dune::FieldVector<double,2> operator()(const Dune::FieldVector<double,1>& local) const {

        double angle = fromAngle_ + local[0]*(toAngle_ - fromAngle_);

        Dune::FieldVector<double,2> result = center_;
        result[0] += radius_ * std::cos(angle);
        result[1] += radius_ * std::sin(angle);

        return result;
    }

    Dune::FieldVector<double,2> center_;

    double radius_;

    double fromAngle_;

    double toAngle_;
};


void makeHalfCircleQuad(Dune::UGGrid<2>& grid, bool parametrization)
{
    using namespace Dune;

    grid.createBegin();

    // /////////////////////////////
    //   Create boundary segments
    // /////////////////////////////
    if (parametrization) {

        FieldVector<double,2> center(0);
        center[1] = 15;
        
        std::vector<unsigned int> vertices(2);
        
        vertices[0] = 1;  vertices[1] = 2;
        grid.insertBoundarySegment(vertices, new ArcOfCircle(center, 15, M_PI, M_PI*4/3));
        
        vertices[0] = 2;  vertices[1] = 3;
        grid.insertBoundarySegment(vertices, new ArcOfCircle(center, 15, M_PI*4/3, M_PI*5/3));
        
        vertices[0] = 3;  vertices[1] = 0;
        grid.insertBoundarySegment(vertices, new ArcOfCircle(center, 15, M_PI*5/3, M_PI*2));

    }

   // ///////////////////////
    //   Insert vertices
    // ///////////////////////
    FieldVector<double,2> pos;

    pos[0] = 15;  pos[1] = 15;
    grid.insertVertex(pos);

    pos[0] = -15; pos[1] = 15;
    grid.insertVertex(pos);

    pos[0] = -7.5; pos[1] = 2.00962;
    grid.insertVertex(pos);

    pos[0] = 7.5; pos[1] = 2.00962;
    grid.insertVertex(pos);
    // /////////////////
    // Insert elements
    // /////////////////

    std::vector<unsigned int> cornerIDs(4);
    cornerIDs[0] = 0;
    cornerIDs[1] = 1;
    cornerIDs[2] = 3;
    cornerIDs[3] = 2;

    grid.insertElement(GeometryType(GeometryType::cube,2), cornerIDs);

    // //////////////////////////////////////
    //   Finish initialization
    // //////////////////////////////////////
    grid.createEnd();

}


template <class GridType >
void markOne ( GridType & grid , int num , int ref )
{
  typedef typename GridType::template Codim<0>::LeafIterator LeafIterator;

  int count = 0;
  
  LeafIterator endit = grid.template leafend  <0> ();
  for(LeafIterator it = grid.template leafbegin<0> (); it != endit ; ++it )
  {
    if(num == count) grid.mark( ref, it );
    count++;
  }

  grid.preAdapt();
  grid.adapt();
  grid.postAdapt();
}

void generalTests(bool greenClosure)
{
    // /////////////////////////////////////////////////////////////////
    //   Prelude: the UGGrid implementation relies on the face that std::array,
    //   Dune::FieldVector, and C-arrays have the same memory layout.  Here
    //   we make sure this is really so.
    // /////////////////////////////////////////////////////////////////

    double cArray[3] = {1,2,3};

    for (int i=0; i<3; i++) {
        assert(cArray[i] == (*((FieldVector<double,3>*)&cArray))[i]);
        assert(cArray[i] == (*((array<double,3>*)&cArray))[i]);
    }

    // //////////////////////////////////////////////////////////
    //   Make some grids for testing
    // //////////////////////////////////////////////////////////

    Dune::UGGrid<2> grid2d;
    Dune::UGGrid<3> grid3d;
    
    make2DTestGrid(grid2d);
    make3DTestGrid(grid3d);
    
    // Switch of the green closure, if requested
    if (!greenClosure) {
        grid2d.setClosureType(UGGrid<2>::NONE);
        grid3d.setClosureType(UGGrid<3>::NONE);
    }

    // check macro grid 
    gridcheck(grid2d);
    gridcheck(grid3d);
    
    // create hybrid grid 
    markOne(grid2d,0,1) ;
    markOne(grid3d,0,1) ;
    
    gridcheck(grid2d);
    gridcheck(grid3d);
    
    grid2d.globalRefine(1);
    grid3d.globalRefine(1);
    gridcheck(grid2d);
    gridcheck(grid3d);
    
    // check the method geometryInFather()
    checkGeometryInFather(grid2d);
    checkGeometryInFather(grid3d);
    
    // check the intersection iterator
    checkIntersectionIterator(grid2d);
    checkIntersectionIterator(grid3d);
    
#ifdef ModelP
    // check communication interface 
    checkCommunication(grid2d,-1,Dune::dvverb);
    for(int l=0; l<=grid2d.maxLevel(); ++l)
        checkCommunication(grid2d,l,Dune::dvverb);
#endif
    
}

int main (int argc , char **argv) try
{

#ifdef ModelP
    // initialize MPI
    MPI_Init(&argc,&argv);
#endif

    // ////////////////////////////////////////////////////////////////////////
    //  Do the standard grid test for a 2d and a 3d UGGrid
    // ////////////////////////////////////////////////////////////////////////

    // Do the general tests for red/green refinement
    std::cout << "Testing UGGrid<2> and UGGrid<3> with red/green refinement" << std::endl;
    generalTests(true);

    // Do the general tests for nonconforming refinement
    std::cout << "Testing UGGrid<2> and UGGrid<3> with nonconforming refinement" << std::endl;
    generalTests(false);

    // ////////////////////////////////////////////////////////////////////////
    //   Check whether geometryInFather returns equal results with and
    //   without parametrized boundaries
    // ////////////////////////////////////////////////////////////////////////

    Dune::UGGrid<2> gridWithParametrization, gridWithoutParametrization;

    // make grids
    makeHalfCircleQuad(gridWithoutParametrization, false);
    makeHalfCircleQuad(gridWithParametrization, true);

    // make grids again just to check this is possible
#if 0
    makeHalfCircleQuad(gridWithoutParametrization, false);
    makeHalfCircleQuad(gridWithParametrization, true);
#endif

    gridWithParametrization.globalRefine(1);
    gridWithoutParametrization.globalRefine(1);

    typedef Dune::UGGrid<2>::Codim<0>::LevelIterator ElementIterator;
    ElementIterator eIt    = gridWithParametrization.lbegin<0>(1);
    ElementIterator eWoIt  = gridWithoutParametrization.lbegin<0>(1);
    ElementIterator eEndIt = gridWithParametrization.lend<0>(1);

    for (; eIt!=eEndIt; ++eIt, ++eWoIt) {

        // The grids where constructed identically and they are traversed identically
        // Thus their respective output from geometryInFather should be the same
        for (int i=0; i<eIt->geometry().corners(); i++) {

            Dune::FieldVector<double,2> diff = eIt->geometryInFather()[i] - eWoIt->geometryInFather()[i];
            
            if ( diff.two_norm() > 1e-5 )
                DUNE_THROW(Dune::GridError, "output of geometryInFather() depends on boundary parametrization!");

        }

    }

    // ////////////////////////////////////////////////////////////////////////
    //   Check whether copies of elements have the same global ID
    // ////////////////////////////////////////////////////////////////////////

    {
        std::cout << "Testing if copies of elements have the same id." << std::endl;
        Dune::UGGrid<2> locallyRefinedGrid;
        
        locallyRefinedGrid.setRefinementType(Dune::UGGrid<2>::COPY);
        
        typedef Dune::UGGrid<2>::Codim<0>::LevelIterator ElementIterator;
        typedef Dune::UGGrid<2>::Codim<0>::HierarchicIterator HierarchicIterator;
        typedef Dune::UGGrid<2>::Traits::GlobalIdSet GlobalIdSet;
        typedef Dune::UGGrid<2>::Traits::LocalIdSet LocalIdSet;
        
        // make grids
        makeHalfCircleQuad(locallyRefinedGrid, false);
        
        markOne(locallyRefinedGrid,0,1);
        markOne(locallyRefinedGrid,0,1);
        
        const GlobalIdSet& globalIdSet = locallyRefinedGrid.globalIdSet();
        const LocalIdSet&  localIdSet  = locallyRefinedGrid.localIdSet();
        
        for (int level=0; level<locallyRefinedGrid.maxLevel(); ++level)
        {
            ElementIterator eIt = locallyRefinedGrid.lbegin<0>(level);
            ElementIterator eEnd = locallyRefinedGrid.lend<0>(level);
            for(; eIt!=eEnd; ++eIt)
            {
                int children = 0;
                GlobalIdSet::IdType globalChildId;
                LocalIdSet::IdType  localChildId;
                
                HierarchicIterator hIt = eIt->hbegin(level+1);
                HierarchicIterator hEnd = eIt->hend(level+1);
                
                
                for( ;hIt!=hEnd; ++hIt)
                {
                    globalChildId = globalIdSet.id<0>(*hIt);
                    localChildId =  localIdSet.id<0>(*hIt);
                    ++children;
                }
                if (children != 1) 
                    continue;

                if (globalIdSet.id<0>(*eIt) != globalChildId)
                    DUNE_THROW(Dune::GridError, "Copy of element has different globalId!");

                if (localIdSet.id<0>(*eIt) != localChildId)
                    DUNE_THROW(Dune::GridError, "Copy of element has different localId!");
            }
        }
    }
    

#ifdef ModelP
    // Terminate MPI
    MPI_Finalize();
#endif

  return 0;
} catch (Dune::Exception& e) {
    std::cerr << e << std::endl;
    return 1;
 } catch (...) {
    std::cerr << "Generic exception!" << std::endl;
    return 2;
 }
