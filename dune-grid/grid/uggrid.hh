#ifndef DUNE_UGGRID_HH
#define DUNE_UGGRID_HH

/** \file 
 * \brief The UGGrid class
 */

#include <dune/grid/common/capabilities.hh>
#include <dune/grid/common/grid.hh>
#include <dune/grid/common/boundarysegment.hh>
#include <dune/common/collectivecommunication.hh>
#include <dune/common/deprecated.hh>
#include <dune/common/static_assert.hh>

/* The following lines including the necessary UG headers are somewhat
   tricky.  Here's what's happening:
   UG can support two- and three-dimensional grids.  You choose be setting
   either _2 oder _3 while compiling.  This changes all sorts of stuff, in
   particular data structures in the headers.
   UG was never supposed to provide 2d and 3d grids at the same time. 
   However, when compiling it as c++, the dimension-dependent parts are
   wrapped up cleanly in the namespaces UG::D2 and UG::D3, respectively.  That
   way it is possible to link together the UG lib for 2d and the one for 3d.
   But we also need the headers twice!  Once with _2 set and once with _3!
   So here we go:*/

/* The following define tells the UG headers that we want access to a few
   special fields, for example the extra index fields in the element data structures. */
#define FOR_DUNE

// Set UG's space-dimension flag to 2d
#define _2
// And include all necessary UG headers
#include "uggrid/ugincludes.hh"

// Wrap a few large UG macros by functions before they get undef'ed away.
// Here: The 2d-version of the macros
#define UG_DIM 2
#include "uggrid/ugwrapper.hh"
#undef UG_DIM

// UG defines a whole load of preprocessor macros.  ug_undefs.hh undefines
// them all, so we don't get name clashes.
#ifdef UG_LGMDOMAIN
#include "uggrid/ug_undefs_lgm_seq.hh"
#else
#include "uggrid/ug_undefs.hh"
#endif
#undef _2

/* Now we're done with 2d, and we can do the whole thing over again for 3d */

/* All macros set by UG have been unset.  This includes the macros that ensure
   single inclusion of headers.  We can thus include them again.  However, we
   only want to include those headers again that contain dimension-dependent stuff.
   Therefore, we set a few single-inclusion defines manually before including
   ugincludes.hh again.
*/
#define UGTYPES_H
#define __HEAPS__
#define __UGENV__
#define __PARGM_H__
#define __DEVICESH__
#define __SM__

#define _3
#include "uggrid/ugincludes.hh"

// Wrap a few large UG macros by functions before they get undef'ed away.
// This time it's the 3d-versions.
#define UG_DIM 3
#include "uggrid/ugwrapper.hh"
#undef UG_DIM

// undef all macros defined by UG
#ifdef UG_LGMDOMAIN
#include "uggrid/ug_undefs_lgm_seq.hh"
#else
#include "uggrid/ug_undefs.hh"
#endif

#undef _3
#undef FOR_DUNE

// The components of the UGGrid interface
#include "uggrid/uggridgeometry.hh"
#include "uggrid/uggridentity.hh"
#include "uggrid/uggridentitypointer.hh"
#include "uggrid/ugintersectionit.hh"
#include "uggrid/uggridleveliterator.hh"
#include "uggrid/uggridleafiterator.hh"
#include "uggrid/uggridhieriterator.hh"
#include "uggrid/uggridindexsets.hh"

namespace Dune {

template<int dim, int dimworld>
struct UGGridFamily
{
  typedef GridTraits<dim,dimworld,Dune::UGGrid<dim>,
                     UGGridGeometry,
                     UGGridEntity,
                     UGGridEntityPointer,
                     UGGridLevelIterator,
                     UGGridLeafIntersectionIterator, // leaf  intersection iterartor
                     UGGridLevelIntersectionIterator, // level intersection iterartor
                     UGGridHierarchicIterator,
                     UGGridLeafIterator,
                     UGGridLevelIndexSet< const UGGrid<dim> >,
                     UGGridLevelIndexSetTypes< const UGGrid<dim> >,
                     UGGridLeafIndexSet< const UGGrid<dim> >,
                     UGGridLeafIndexSetTypes< const UGGrid<dim> >,
                     UGGridIdSet< const UGGrid<dim>, false >,
                     unsigned int,
                     UGGridIdSet< const UGGrid<dim>, true >,
                     unsigned int,
                     CollectiveCommunication<Dune::UGGrid<dim> > > 
  Traits;
};


//**********************************************************************
//
// --UGGrid
//
//**********************************************************************

/**
   \brief [<em> provides \ref Dune::Grid </em>]
   \brief Provides the meshes of the finite element toolbox UG.
   \brief (http://sit.iwr.uni-heidelberg.de/~ug).
   \ingroup GridImplementations

  This is the implementation of the grid interface
  using the UG grid management system.  UG provides conforming grids
  in two and three space dimensions.  The grids can be mixed, i.e.
  2d grids can contain triangles and quadrilaterals and 3d grids can
  contain tetrahedra and hexahedra and also pyramids and prisms.
  The grid refinement rules are very flexible.  Local adaptive red/green
  refinement is the default, but a special method in the UGGrid class
  allows you to directly access a number of anisotropic refinements rules.
  Last but not least, the UG grid manager is completely parallelized,
  and you can use boundaries parametrized by either analytical expressions
  or high-resolution piecewise linear surfaces.

  To use this module you need the UG library.  See the 
  DUNE installation notes
  on how to obtain and install it.

  In your %Dune application, you can now instantiate objects of the
  type UGGrid<2> or UGGrid<3>.  You can have more than one, if
  you choose.  It is even possible to have 2d and 3d grids at the same
  time, even though the original UG system never intended to support
  this!

  Please send any questions, suggestions, or bug reports to
  sander@math.fu-berlin.de

  For installation instructions see http://www.dune-project.org/doc/contrib-software.html#uggrid .
*/
template <int dim>
class UGGrid : public GridDefaultImplementation  <dim, dim, double, UGGridFamily<dim,dim> >
{
    friend class UGGridGeometry<0,dim,const UGGrid<dim> >;
    friend class UGGridGeometry<dim,dim,const UGGrid<dim> >;
    friend class UGGridGeometry<1,2,const UGGrid<dim> >;
    friend class UGGridGeometry<2,3,const UGGrid<dim> >;

    friend class UGGridEntity <0,dim,const UGGrid<dim> >;
    friend class UGGridEntity <dim,dim,const UGGrid<dim> >;
    friend class UGGridHierarchicIterator<const UGGrid<dim> >;
    friend class UGGridLeafIntersectionIterator<const UGGrid<dim> >;
    friend class UGGridLevelIntersectionIterator<const UGGrid<dim> >;

    friend class UGGridLevelIndexSet<const UGGrid<dim> >;
    friend class UGGridLeafIndexSet<const UGGrid<dim> >;
    friend class UGGridIdSet<const UGGrid<dim>, false >;
    friend class UGGridIdSet<const UGGrid<dim>, true >;

    template <int codim_, PartitionIteratorType PiType_, class GridImp_>
    friend class UGGridLeafIterator;
    template <int codim_, int dim_, class GridImp_, template<int,int,class> class EntityImp_>
    friend class Entity;

    /** \brief UGGrid is only implemented for 2 and 3 dimension */
    dune_static_assert(dim==2 || dim==3, "Use UGGrid only for 2d and 3d!");

    // The different instantiations are mutual friends so they can access
    // each others numOfUGGrids field
    friend class UGGrid<2>;
    friend class UGGrid<3>;
    //**********************************************************
      // The Interface Methods
      //**********************************************************
public:  
    //! type of the used GridFamily for this grid 
      typedef UGGridFamily<dim,dim>  GridFamily;

    //! the Traits 
    typedef typename UGGridFamily<dim,dim>::Traits Traits;

    //! The type used to store coordinates
    typedef UG::DOUBLE ctype;

    /** \brief Constructor with control over UG's memory requirements 
     *
     * \param heapSize The size of UG's internal memory in megabytes for this grid.
     */
    UGGrid(unsigned int heapSize=500);

     /** \brief Constructor with control over UG's memory requirements 
     *
     * \param heapSize The size of UG's internal memory in megabytes for this grid.
     * \param envHeapSize The size of UG's environment heap, also in megabytes.  
     \deprecated This constructor will be removed in the next release, because
     the second argument is not necessary anymore.
     */
    UGGrid(unsigned int heapSize, unsigned int envHeapSize) DUNE_DEPRECATED;

    //! Destructor 
    ~UGGrid();
   
     //! Return maximum level defined in this grid. Levels are numbered
     //! 0 ... maxlevel with 0 the coarsest level.  
     int maxLevel() const;
     
    //! Iterator to first entity of given codim on level
    template<int codim>
    typename Traits::template Codim<codim>::LevelIterator lbegin (int level) const;

    //! one past the end on this level
    template<int codim>
    typename Traits::template Codim<codim>::LevelIterator lend (int level) const;

    //! Iterator to first entity of given codim on level
    template<int codim, PartitionIteratorType PiType>
    typename Traits::template Codim<codim>::template Partition<PiType>::LevelIterator lbegin (int level) const;

    //! one past the end on this level
    template<int codim, PartitionIteratorType PiType>
    typename Traits::template Codim<codim>::template Partition<PiType>::LevelIterator lend (int level) const;

    //! Iterator to first leaf entity of given codim
    template<int codim>
    typename Traits::template Codim<codim>::LeafIterator leafbegin() const {
        return typename Traits::template Codim<codim>::template Partition<All_Partition>::LeafIterator(*this);
    }

    //! one past the end of the sequence of leaf entities
    template<int codim>
    typename Traits::template Codim<codim>::LeafIterator leafend() const {
        return UGGridLeafIterator<codim,All_Partition, const UGGrid<dim> >();
    }

    //! Iterator to first leaf entity of given codim
    template<int codim, PartitionIteratorType PiType>
    typename Traits::template Codim<codim>::template Partition<PiType>::LeafIterator leafbegin() const {
        return typename Traits::template Codim<codim>::template Partition<PiType>::LeafIterator(*this);
    }

    //! one past the end of the sequence of leaf entities
    template<int codim, PartitionIteratorType PiType>
    typename Traits::template Codim<codim>::template Partition<PiType>::LeafIterator leafend() const {
        return UGGridLeafIterator<codim,PiType, const UGGrid<dim> >();
    }

    /** \brief Number of grid entities per level and codim
     */
    int size (int level, int codim) const;

  //! number of leaf entities per codim in this process
  int size (int codim) const
  {
      return leafIndexSet().size(codim);
  }

  //! number of entities per level and geometry type in this process
  int size (int level, GeometryType type) const
  {
	return this->levelIndexSet(level).size(type);
  }

  //! number of leaf entities per geometry type in this process
  int size (GeometryType type) const
  {
	return this->leafIndexSet().size(type);
  }

    /** \brief Access to the GlobalIdSet */
    const typename Traits::GlobalIdSet& globalIdSet() const
    {
	return globalIdSet_;
    }
    
    /** \brief Access to the LocalIdSet */
    const typename Traits::LocalIdSet& localIdSet() const
    {
	return localIdSet_;
    }
    
    /** \brief Access to the LevelIndexSets */
    const typename Traits::LevelIndexSet& levelIndexSet(int level) const
    {
        if (level<0 || level>maxLevel())
            DUNE_THROW(GridError, "levelIndexSet of nonexisting level " << level << " requested!");
	return *levelIndexSets_[level];
    }
    
    /** \brief Access to the LeafIndexSet */
    const typename Traits::LeafIndexSet& leafIndexSet() const
    {
	return leafIndexSet_;
    }

    /** @name Grid Refinement Methods */
    /*@{*/

    /** \brief Mark element for refinement
        \param refCount <ul>
        <li> 1: mark for red refinement </li>
        <li> -1: mark for coarsening </li>
        <li> 0: delete a possible refinement mark </li>
        </ul>
        \param e Element to be marked
    \return <ul>
        <li> true, if element was marked </li>
        <li> false, if nothing changed </li>
        </ul>
     */
    bool mark(int refCount, const typename Traits::template Codim<0>::EntityPointer & e );

    /** \brief Mark method accepting a UG refinement rule

    \param e Pointer to the element to be marked for refinement
    \param rule One of the UG refinement rules
    \param side If rule==UG::D2::BLUE (one quadrilateral is split into two rectangles)
    you can choose the orientation of the cut by setting side==0 or side==1
     */
    bool mark(const typename Traits::template Codim<0>::EntityPointer & e, 
              typename UG_NS<dim>::RefinementRule rule,
              int side=0);

    /** \brief Query whether element is marked for refinement */
    int getMark(const typename Traits::template Codim<0>::EntityPointer& e) const;

    /** \brief returns true, if some elements might be coarsend during grid
     adaption, here always returns true */
    bool preAdapt();
    
    //! Triggers the grid refinement process
    bool adapt();

    /** \brief Clean up refinement markers */
    void postAdapt();
    /*@}*/

    /** \brief return name of the grid */
    std::string name () const { return "UGGrid"; };

    /** \brief Size of the overlap on the leaf level */
    unsigned int overlapSize(int codim) const {
        return 0;
    }

    /** \brief Size of the ghost cell layer on the leaf level */
    unsigned int ghostSize(int codim) const {
        return (codim==0) ? 1 : 0;
    }

    /** \brief Size of the overlap on a given level */
    unsigned int overlapSize(int level, int codim) const {
        return 0;
    }

    /** \brief Size of the ghost cell layer on a given level */
    unsigned int ghostSize(int level, int codim) const {
        return (codim==0) ? 1 : 0;
    }
    
    /** \brief Distributes this grid over the available nodes in a distributed machine 
     *
     \param minlevel The coarsest grid level that gets distributed
     \param maxlevel does currently get ignored
    */
    void loadBalance(int strategy, int minlevel, int depth, int maxlevel, int minelement);

    typedef GridDefaultImplementation  <dim, dim, double,
            UGGridFamily<dim,dim> > GridDefaultImplementationType; 

    //! also make default implementations of loadBalance useable  
    using GridDefaultImplementationType :: loadBalance;

    /** \brief The communication interface
      @param T: array class holding data associated with the entities
      @param P: type used to gather/scatter data in and out of the message buffer
      @param codim: communicate entites of given codim
      @param if: one of the predifined interface types, throws error if it is not implemented
      @param level: communicate for entities on the given level
      
      Implements a generic communication function sending an object of type P for each entity
      in the intersection of two processors. P has two methods gather and scatter that implement
      the protocol. Therefore P is called the "protocol class".
    */
    template<class T, template<class> class P, int codim>
    void communicate (T& t, InterfaceType iftype, CommunicationDirection dir, int level);

    /*! The new communication interface
      
    communicate objects for all codims on a given level
    */
    template<class DataHandle>
    void communicate (DataHandle& data, InterfaceType iftype, CommunicationDirection dir, int level) const  
    {}

    template<class DataHandle>
    void communicate (DataHandle& data, InterfaceType iftype, CommunicationDirection dir) const  
    {}

	/** dummy collective communication */
	const CollectiveCommunication<UGGrid>& comm () const
	{
	  return ccobj;
	}

    // **********************************************************
    // End of Interface Methods
    // **********************************************************
    
    /** @name Coarse Grid Creation Methods */
    /*@{*/
    
    /** \brief Start the coarse grid creation process 
        \todo This method should delete the grid!
    */
    void createBegin();

    /** \brief End the coarse grid creation process */
    void createEnd();

    /** \brief When UGGrid has been configured to use the LGM domain manager,
        this routine sets up a grid from an LGM and an NG file
    */
    void createLGMGrid(const std::string& name);

    /** \brief Method to insert an arbitrarily shaped boundary segment into a coarse grid
        \param vertices The indices of the vertices of the segment
        \param boundarySegment Class implementing the geometry of the boundary segment.
        The grid object takes control of this object and deallocates it when destructing itself.
     */
    void insertBoundarySegment(const std::vector<unsigned int> vertices,
                               const BoundarySegment<dim>* boundarySegment);

    /** \brief Insert a vertex into the coarse grid */
    void insertVertex(const FieldVector<double,dim>& pos);

    /** \brief Insert an element into the coarse grid
        \param type The GeometryType of the new element
        \param vertices The vertices of the new element, using the DUNE numbering
    */
    void insertElement(GeometryType type,
                       const std::vector<unsigned int>& vertices);
    
    /*@}*/

    /** \brief Rudimentary substitute for a hierarchic iterator on faces
        \param e, elementSide Grid face specified by an element and one of its sides
        \param maxl The finest level that should be traversed by the iterator
        \param children For each subface: element index, elementSide, and level
    */
    void getChildrenOfSubface(typename Traits::template Codim<0>::EntityPointer & e,
                              int elementSide,
                              int maxl, 
                              std::vector<typename Traits::template Codim<0>::EntityPointer>& childElements,
                              std::vector<unsigned char>& childElementSides) const;
    
    /** \brief The different forms of grid refinement that UG supports */
    enum RefinementType {
        /** \brief New level consists only of the refined elements and the closure*/
        LOCAL, 
        /** \brief New level consists of the refined elements and the unrefined ones, too */
        COPY};

    /** \brief Decide whether to add a green closure to locally refined grid sections or not */
    enum ClosureType {
        /** \brief Standard red/green refinement */
        GREEN,
        /** \brief No closure, results in nonconforming meshes */
        NONE};

    /** \brief Sets the type of grid refinement */
    void setRefinementType(RefinementType type) {
        refinementType_ = type;
    }

    /** \brief Sets the type of grid refinement closure */
    void setClosureType(ClosureType type) {
        closureType_ = type;
    }

    /** \brief Collapses the grid hierarchy into a single grid level*/
    void collapse() {
        if (Collapse(multigrid_))
            DUNE_THROW(GridError, "UG" << dim << "d::Collapse() returned error code!");

        setIndices();
    }

    /** \brief Sets a vertex to a new position 
    
    Changing a vertex' position changes its position on all grid levels!*/
    void setPosition(typename Traits::template Codim<dim>::EntityPointer& e,
                     const FieldVector<double, dim>& pos);

    /** \brief For a point on the grid boundary return its position on the domain boundary */
    FieldVector<ctype,dim> getBoundaryPosition(const IntersectionIterator<const UGGrid<dim>, UGGridLevelIntersectionIterator>& iIt,
                                               const FieldVector<ctype,dim-1>& localPos) const;

    /** \brief Does uniform refinement
     *
     * \param n Number of uniform refinement steps
     */
    void globalRefine(int n);

    void saveState(const std::string& filename) const;

    void loadState(const std::string& filename);

private:
    /** \brief UG multigrid, which contains the actual grid hierarchy structure */
    typename UG_NS<dim>::MultiGrid* multigrid_;

    /** \brief The classes implementing the geometry of the boundary segments */
    std::vector<const BoundarySegment<dim>*> boundarySegments_;

    /** \brief Buffer for the vertices of each explicitly given boundary segment */
    std::vector<array<unsigned int, dim*2-2> > boundarySegmentVertices_;

  CollectiveCommunication<UGGrid> ccobj;

    // Recomputes entity indices after the grid was changed
    void setIndices();

    // Each UGGrid object has a unique name to identify it in the
    // UG environment structure
    std::string name_;

    // Our set of level indices
    std::vector<UGGridLevelIndexSet<const UGGrid<dim> >*> levelIndexSets_;

    UGGridLeafIndexSet<const UGGrid<dim> > leafIndexSet_;

    UGGridIdSet<const UGGrid<dim>, false > globalIdSet_;

    UGGridIdSet<const UGGrid<dim>, true > localIdSet_;

    //! The type of grid refinement currently in use
    RefinementType refinementType_;

    //! The type of grid refinement closure currently in use
    ClosureType closureType_;

    /** \brief While inserting the elements this array records the number of
        vertices of each element. */
    std::vector<unsigned char> elementTypes_;

    /** \brief While inserting the elements this array records the vertices
        of the elements. */
    std::vector<unsigned int> elementVertices_;

    /** \brief Buffer the vertices until createend() is called */
    std::vector<FieldVector<double, dim> > vertexPositions_;

    /** \brief Number of UGGrids currently in use.
     *
     * This counts the number of UGGrids currently instantiated.  All
     * constructors of UGGrid look at this variable.  If it zero, they
     * initialize UG before proceeding.  Destructors use the same mechanism
     * to safely shut down UG after deleting the last UGGrid object.
     */
    static int numOfUGGrids;

    /** \brief Remember whether some element has been marked for refinement
        ever since the last call to adapt().

        This is here to implement the return value of preAdapt().
    */
    bool someElementHasBeenMarkedForRefinement_;

    /** \brief The size of UG's internal heap in megabytes
     *
     * It is handed over to UG for each new multigrid.
     */
    unsigned int heapsize;


}; // end Class UGGrid

namespace Capabilities
{
  /** \struct hasEntity
  \ingroup UGGrid
  */
  
  /** \struct hasBackupRestoreFacilities
  \ingroup UGGrid
  */
  
  /** \struct IsUnstructured
  \ingroup UGGrid
  */

  /** \brief UGGrid has codim=0 entities (elements)
  \ingroup UGGrid
  */
  template<int dim>
  struct hasEntity< UGGrid<dim>, 0>
  {
    static const bool v = true;
  };

  /** \brief UGGrid has codim=dim entities (vertices)
  \ingroup UGGrid
  */
  template<int dim>
  struct hasEntity< UGGrid<dim>, dim>
  {
    static const bool v = true;
  };
  
  /** \brief UGGrid is parallel
  \ingroup UGGrid
  */
  template<int dim>
  struct isParallel< UGGrid<dim> >
  {
#ifdef ModelP
      static const bool v = true;
#else
      static const bool v = false;
#endif
  };

  /** \brief UGGrid is levelwise conforming
  \ingroup UGGrid
  */
  template<int dim>
  struct isLevelwiseConforming< UGGrid<dim> >
  {
    static const bool v = true;
  };

  /** \brief UGGrid may not be leafwise conforming
  \ingroup UGGrid
  */
  template<int dim>
  struct isLeafwiseConforming< UGGrid<dim> >
  {
    static const bool v = false;
  };

  /** \brief UGGrid does support hanging nodes
  \ingroup UGGrid
  */
  template<int dim>
  struct hasHangingNodes< UGGrid<dim> >
  {
    static const bool v = true;
  };
  
}

} // namespace Dune

#endif
