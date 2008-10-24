#ifndef DUNE_UGINTERSECTIONIT_HH
#define DUNE_UGINTERSECTIONIT_HH

#include <dune/common/sllist.hh>
/** \file
 * \brief The UGGridIntersectionIterator class
 */

namespace Dune {

//**********************************************************************
//
// --UGGridIntersectionIterator
// --IntersectionIterator
  /** \brief Iterator over all element neighbors
   * \ingroup UGGrid
  Mesh entities of codimension 0 ("elements") allow to visit all neighbors, where
  a neighbor is an entity of codimension 0 which has a common entity of codimension 1
  These neighbors are accessed via a IntersectionIterator. This allows the implement
  non-matching meshes. The number of neigbors may be different from the number
  of an element!
 */
template<class GridImp>
class UGGridLevelIntersectionIterator : 
        public IntersectionIteratorDefaultImplementation <GridImp,UGGridLevelIntersectionIterator>
{

    enum {dim=GridImp::dimension};

    enum {dimworld=GridImp::dimensionworld};

    friend class UGGridEntity<0,dim,GridImp>;

    // The type used to store coordinates
    typedef typename GridImp::ctype UGCtype;

public:

    typedef typename GridImp::template Codim<0>::EntityPointer EntityPointer;
  typedef typename GridImp::template Codim<1>::Geometry Geometry;
  typedef typename GridImp::template Codim<1>::LocalGeometry LocalGeometry;
    typedef typename GridImp::template Codim<0>::Entity Entity;

    /** The default Constructor makes empty Iterator 
        \todo Should be private
    */
    UGGridLevelIntersectionIterator(typename UG_NS<dim>::Element* center, int nb)
	: center_(center), neighborCount_(nb)
    {
    }

  //! The Destructor 
  ~UGGridLevelIntersectionIterator() {};

  //! equality
    bool equals(const UGGridLevelIntersectionIterator<GridImp>& i) const {
        return center_==i.center_ && neighborCount_ == i.neighborCount_;
    }

  //! prefix increment
  void increment() {
        neighborCount_++;
  }

  //! return EntityPointer to the Entity on the inside of this intersection
  //! (that is the Entity where we started this Iterator)
  EntityPointer inside() const {
      return UGGridEntityPointer<0,GridImp>(center_);
  }

  //! return EntityPointer to the Entity on the outside of this intersection
  //! (that is the neighboring Entity)
  EntityPointer outside() const {

        typename UG_NS<dim>::Element* otherelem = UG_NS<dim>::NbElem(center_, neighborCount_);

        if (otherelem==0) 
            DUNE_THROW(GridError,"no neighbor found in outside()");

        return UGGridEntityPointer<0,GridImp>(otherelem);
    }

  //! return true if intersection is with boundary. \todo connection with 
  //! boundary information, processor/outer boundary
  bool boundary () const {
      return UG_NS<dim>::Side_On_Bnd(center_, neighborCount_);
  }

  //! return true if across the edge an neighbor on this level exists
  bool neighbor () const {
      return UG_NS<dim>::NbElem(center_, neighborCount_) != NULL;
  }

  //! return information about the Boundary 
  int boundaryId () const {
    return 1;
  }
      
  //! intersection of codimension 1 of this neighbor with element where
  //! iteration started. 
  //! Here returned element is in LOCAL coordinates of the element
  //! where iteration started.
  const LocalGeometry& intersectionSelfLocal () const;

  //! intersection of codimension 1 of this neighbor with element where iteration started. 
  //! Here returned element is in GLOBAL coordinates of the element where iteration started.
  const Geometry& intersectionGlobal () const;

  //! intersection of codimension 1 of this neighbor with element where iteration started. 
  //! Here returned element is in LOCAL coordinates of neighbor
  const  LocalGeometry& intersectionNeighborLocal () const;

  //! local number of codim 1 entity in self where intersection is contained in 
  int numberInSelf () const {
      return UGGridRenumberer<dim>::facesUGtoDUNE(neighborCount_, UG_NS<dimworld>::Sides_Of_Elem(center_));
  }

  //! local number of codim 1 entity in neighbor where intersection is contained
  int numberInNeighbor () const;
  
  //! return outer normal
    const FieldVector<UGCtype, GridImp::dimensionworld>& outerNormal (const FieldVector<UGCtype, GridImp::dimension-1>& local) const;

private:
  //**********************************************************
  //  private methods 
  //**********************************************************

  //! vector storing the outer normal 
    mutable FieldVector<UGCtype, dimworld> outerNormal_;

  //! pointer to element holding the self_local and self_global information.
  //! This element is created on demand.
    mutable UGMakeableGeometry<dim-1,dim,GridImp> selfLocal_;
    mutable UGMakeableGeometry<dim-1,dim,GridImp> neighLocal_;
 
  //! pointer to element holding the neighbor_global and neighbor_local 
  //! information.
    mutable UGMakeableGeometry<dim-1,dimworld,GridImp> neighGlob_;

    //! The UG element the iterator was created from
    typename UG_NS<dim>::Element *center_;

    //! count on which neighbor we are lookin' at. Note that this is interpreted in UG's ordering!
    int neighborCount_;

};


template<class GridImp>
class UGGridLeafIntersectionIterator : 
        public IntersectionIteratorDefaultImplementation <GridImp,UGGridLeafIntersectionIterator>
{

    enum {dim=GridImp::dimension};

    enum {dimworld=GridImp::dimensionworld};

    friend class UGGridEntity<0,dim,GridImp>;

    // The type used to store coordinates
    typedef typename GridImp::ctype UGCtype;

    // An element face identfied by an element and a face number
    typedef std::pair<typename UG_NS<dim>::Element*, int> Face;

public:

    typedef typename GridImp::template Codim<0>::EntityPointer EntityPointer;
  typedef typename GridImp::template Codim<1>::Geometry Geometry;
  typedef typename GridImp::template Codim<1>::LocalGeometry LocalGeometry;
    typedef typename GridImp::template Codim<0>::Entity Entity;

    UGGridLeafIntersectionIterator(typename UG_NS<dim>::Element* center, int nb)
	: center_(center), neighborCount_(nb), subNeighborCount_(0)
    {
        if (neighborCount_ < UG_NS<dim>::Sides_Of_Elem(center_))
            constructLeafSubfaces();
    }

  //! equality
    bool equals(const UGGridLeafIntersectionIterator<GridImp>& other) const {
        return center_           == other.center_ 
            && neighborCount_    == other.neighborCount_
            && subNeighborCount_ == other.subNeighborCount_;
    }

  //! prefix increment
  void increment() {

      subNeighborCount_++;

      if (subNeighborCount_ >= leafSubFaces_.size() ) {

          neighborCount_++;
          subNeighborCount_ = 0;

          if (neighborCount_ < UG_NS<dim>::Sides_Of_Elem(center_))
              constructLeafSubfaces();

      }

  }

  //! return EntityPointer to the Entity on the inside of this intersection
  //! (that is the Entity where we started this Iterator)
  EntityPointer inside() const {
      return UGGridEntityPointer<0,GridImp>(center_);
  }

  //! return EntityPointer to the Entity on the outside of this intersection
  //! (that is the neighboring Entity)
  EntityPointer outside() const {
      
        typename UG_NS<dim>::Element* otherelem = leafSubFaces_[subNeighborCount_].first;

        if (otherelem==0) 
            DUNE_THROW(GridError,"no neighbor found in outside()");

        return UGGridEntityPointer<0,GridImp>(otherelem);
    }

  //! return true if intersection is with boundary.
  bool boundary () const {
      return UG_NS<dim>::Side_On_Bnd(center_, neighborCount_);
  }

  //! return true if a neighbor element exists across this intersection
  bool neighbor () const {
      return leafSubFaces_[subNeighborCount_].first != NULL;
  }

  //! return information about the Boundary 
  int boundaryId () const {
    return 1;
  }
      
  //! intersection of codimension 1 of this neighbor with element where
  //! iteration started. 
  //! Here returned element is in LOCAL coordinates of the element
  //! where iteration started.
  const LocalGeometry& intersectionSelfLocal () const;

  //! intersection of codimension 1 of this neighbor with element where iteration started. 
  //! Here returned element is in GLOBAL coordinates of the element where iteration started.
  const Geometry& intersectionGlobal () const;

  //! intersection of codimension 1 of this neighbor with element where iteration started. 
  //! Here returned element is in LOCAL coordinates of neighbor
  const  LocalGeometry& intersectionNeighborLocal () const;

  //! local number of codim 1 entity in self where intersection is contained in 
  int numberInSelf () const {
      return UGGridRenumberer<dim>::facesUGtoDUNE(neighborCount_, UG_NS<dimworld>::Sides_Of_Elem(center_));
  }

  //! local number of codim 1 entity in neighbor where intersection is contained
  int numberInNeighbor () const;
  
  //! return outer normal, this should be dependent on local 
  //! coordinates for higher order boundary 
    const FieldVector<UGCtype, GridImp::dimensionworld>& outerNormal (const FieldVector<UGCtype, GridImp::dimension-1>& local) const;

private:
  //**********************************************************
  //  private methods 
  //**********************************************************

    int numberInNeighbor(const typename UG_NS<dim>::Element* me, const typename UG_NS<dim>::Element* other) const {
        const int nSides = UG_NS<dim>::Sides_Of_Elem(other);
        
        for (int i=0; i<nSides; i++)
            if (UG_NS<dim>::NbElem(other,i) == me)
                return i;

        // this point should not be reached, otherwise throw exception
        DUNE_THROW(InvalidStateException,"no consitency in numberInNeighbor");
        return -1;
  }
    
    void constructLeafSubfaces();
    
    //! vector storing the outer normal 
    mutable FieldVector<UGCtype, dimworld> outerNormal_;
    
    //! pointer to element holding the self_local and self_global information.
    //! This element is created on demand.
    mutable UGMakeableGeometry<dim-1,dim,GridImp> selfLocal_;
    mutable UGMakeableGeometry<dim-1,dim,GridImp> neighLocal_;
    
    //! pointer to element holding the neighbor_global and neighbor_local 
  //! information.
    mutable UGMakeableGeometry<dim-1,dimworld,GridImp> neighGlob_;

    //! The UG element the iterator was created from
    typename UG_NS<dim>::Element *center_;

    //! count on which neighbor we are lookin' at. Note that this is interpreted in UG's ordering!
    int neighborCount_;

    //! For nonconforming intersection: which intersection within the face given
    //! by neighborCount_ are we looking at?
    unsigned int subNeighborCount_;

    std::vector<Face> leafSubFaces_;

};

}  // namespace Dune

#endif
