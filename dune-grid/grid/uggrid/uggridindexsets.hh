#ifndef DUNE_UGGRID_INDEXSETS_HH
#define DUNE_UGGRID_INDEXSETS_HH

/** \file
    \brief The index and id sets for the UGGrid class
*/

#include <vector>
#include <set>

#include <dune/grid/common/grid.hh>

namespace Dune {

template <class GridImp> 
struct UGGridLevelIndexSetTypes
{
  //! The types
  template<int cd>
  struct Codim
  {
	template<PartitionIteratorType pitype>
	struct Partition
	{
	  typedef typename GridImp::Traits::template Codim<cd>::template Partition<pitype>::LevelIterator Iterator;
	};
  };
};


template<class GridImp>
class UGGridLevelIndexSet : public IndexSetDefaultImplementation<GridImp,UGGridLevelIndexSet<GridImp>,UGGridLevelIndexSetTypes<GridImp> >
{
  enum {dim = GridImp::dimension};
  typedef IndexSetDefaultImplementation<GridImp,UGGridLevelIndexSet<GridImp>,UGGridLevelIndexSetTypes<GridImp> > Base;

public:

    /** \brief Default constructor

    Unfortunately we can't force the user to init grid_ and level_, because
    UGGridLevelIndexSets are meant to be stored in an array.

    \todo I want to make this constructor private, but I can't, because
    it is called by UGGrid through a std::vector::resize()
    */
    UGGridLevelIndexSet () {}

  //! get index of an entity
  template<int cd>
  int index (const typename GridImp::Traits::template Codim<cd>::Entity& e) const 
  {
      return UG_NS<dim>::levelIndex(grid_->getRealImplementation(e).target_);
  }

  //! get index of subEntity of a codim 0 entity
  template<int cc>
  int subIndex (const typename GridImp::Traits::template Codim<0>::Entity& e, int i) const
  {
      if (cc==dim)
          return UG_NS<dim>::levelIndex(UG_NS<dim>::Corner(grid_->getRealImplementation(e).target_,
                                                           UGGridRenumberer<dim>::verticesDUNEtoUG(i,e.type())));

      if (cc==0)
          return UG_NS<dim>::levelIndex(grid_->getRealImplementation(e).target_);

      if (cc==dim-1) {
          int a=ReferenceElements<double,dim>::general(e.type()).subEntity(i,dim-1,0,dim);	
          int b=ReferenceElements<double,dim>::general(e.type()).subEntity(i,dim-1,1,dim);
          return UG_NS<dim>::levelIndex(UG_NS<dim>::GetEdge(UG_NS<dim>::Corner(grid_->getRealImplementation(e).target_,
                                                                               UGGridRenumberer<dim>::verticesDUNEtoUG(a,e.type())),
                                                            UG_NS<dim>::Corner(grid_->getRealImplementation(e).target_,
                                                                               UGGridRenumberer<dim>::verticesDUNEtoUG(b,e.type()))));
      }

      if (cc==1)
          return UG_NS<dim>::levelIndex(UG_NS<dim>::SideVector(grid_->getRealImplementation(e).target_,
                                                               UGGridRenumberer<dim>::facesDUNEtoUG(i,e.type())));
      
      DUNE_THROW(GridError, "UGGrid<" << dim << "," << dim << ">::subIndex isn't implemented for cc==" << cc );
  }


    //! get number of entities of given codim, type and on this level
    int size (int codim) const {
	  if (codim==0)
		return numSimplices_+numPyramids_+numPrisms_+numCubes_;
	  if (codim==dim)
		return numVertices_;
	  if (codim==dim-1)
		return numEdges_;
	  if (codim==1)
		return numTriFaces_+numQuadFaces_;
	  DUNE_THROW(NotImplemented, "wrong codim!");
    }

  //! get number of entities of given codim, type and on this level
  int size (GeometryType type) const
  {
      int codim = GridImp::dimension-type.dim();

	if (codim==0) {
            if (type.isSimplex())
		return numSimplices_;
            else if (type.isPyramid())
		return numPyramids_;
            else if (type.isPrism())
		return numPrisms_;
            else if (type.isCube())
		return numCubes_;
            else 
		return 0;
	  
	}

	if (codim==dim) {
	  return numVertices_;
	} 
	if (codim==dim-1) {
	  return numEdges_;
	}
	if (codim==1) {
            if (type.isSimplex())
		return numTriFaces_;
            else if (type.isCube())
		return numQuadFaces_;
            else
		return 0;
	}

	DUNE_THROW(NotImplemented, "Wrong codim!");
  }

  /** \brief Deliver all geometry types used in this grid */
  const std::vector<GeometryType>& geomTypes (int codim) const
  {
	return myTypes_[codim];
  }

  //! one past the end on this level
  template<int cd, PartitionIteratorType pitype>
  typename Base::template Codim<cd>::template Partition<pitype>::Iterator begin () const
  {
	return grid_->template lbegin<cd,pitype>(level_);
  }
  
  //! Iterator to one past the last entity of given codim on level for partition type
  template<int cd, PartitionIteratorType pitype>
  typename Base::template Codim<cd>::template Partition<pitype>::Iterator end () const
  {
	return grid_->template lend<cd,pitype>(level_);
  }

    /** \brief Update the level indices.  This method is called after each grid change */
    void update(const GridImp& grid, int level);

  const GridImp* grid_;
  int level_;

  int numSimplices_;
  int numPyramids_;
  int numPrisms_;
  int numCubes_;
  int numVertices_;
  int numEdges_;
  int numTriFaces_;
  int numQuadFaces_;

  std::vector<GeometryType> myTypes_[dim+1];
};

template <class GridImp> 
struct UGGridLeafIndexSetTypes
{
  //! The types
  template<int cd>
  struct Codim
  {
	template<PartitionIteratorType pitype>
	struct Partition
	{
	  typedef typename GridImp::Traits::template Codim<cd>::template Partition<pitype>::LeafIterator Iterator;
	};
  };
};


template<class GridImp>
class UGGridLeafIndexSet: public IndexSetDefaultImplementation<GridImp,UGGridLeafIndexSet<GridImp>,UGGridLeafIndexSetTypes<GridImp> >
{
  typedef IndexSetDefaultImplementation<GridImp,UGGridLeafIndexSet<GridImp>,UGGridLeafIndexSetTypes<GridImp> > Base;
public:

  /*
    We use the remove_const to extract the Type from the mutable class,
    because the const class is not instatiated yet.
  */
  enum {dim = remove_const<GridImp>::type::dimension};

  //! constructor stores reference to a grid and level
  UGGridLeafIndexSet (const GridImp& g) : grid_(g)
  {
  }

  //! get index of an entity
  /*
    We use the remove_const to extract the Type from the mutable class,
    because the const class is not instatiated yet.
  */
  template<int cd>
  int index (const typename remove_const<GridImp>::type::Traits::template Codim<cd>::Entity& e) const 
  {
      return UG_NS<dim>::leafIndex(grid_.getRealImplementation(e).target_);
  }

  //! get index of subEntity of a codim 0 entity
  /*
    We use the remove_const to extract the Type from the mutable class,
    because the const class is not instatiated yet.
  */
  template<int cc>
  int subIndex (const typename remove_const<GridImp>::type::Traits::template Codim<0>::Entity& e, int i) const
  {
      if (cc==0)
          return UG_NS<dim>::leafIndex(grid_.getRealImplementation(e).target_);
      
      const GeometryType type = e.type();
      
      if (cc==dim)
          return UG_NS<dim>::leafIndex(UG_NS<dim>::Corner(grid_.getRealImplementation(e).target_,
                                                          UGGridRenumberer<dim>::verticesDUNEtoUG(i,type)));

      if (cc==dim-1) {

            int a=ReferenceElements<double,dim>::general(type).subEntity(i,dim-1,0,dim);	
            int b=ReferenceElements<double,dim>::general(type).subEntity(i,dim-1,1,dim);
            return UG_NS<dim>::leafIndex(UG_NS<dim>::GetEdge(UG_NS<dim>::Corner(grid_.getRealImplementation(e).target_,
                                                                                UGGridRenumberer<dim>::verticesDUNEtoUG(a,type)),
                                                             UG_NS<dim>::Corner(grid_.getRealImplementation(e).target_,
                                                                                UGGridRenumberer<dim>::verticesDUNEtoUG(b,type))));
        }

	if (cc==1)
            return UG_NS<dim>::leafIndex(UG_NS<dim>::SideVector(grid_.getRealImplementation(e).target_,
                                                                UGGridRenumberer<dim>::facesDUNEtoUG(i,type)));
        
	DUNE_THROW(GridError, "UGGrid<" << dim << "," << dim << ">::subLeafIndex isn't implemented for cc==" << cc );
  }

  //! get number of entities of given codim and type 
  int size (GeometryType type) const
  {
      if (type.dim()==GridImp::dimension) {
            if (type.isSimplex())
		return numSimplices_;
            else if (type.isPyramid())
		return numPyramids_;
            else if (type.isPrism())
		return numPrisms_;
            else if (type.isCube())
		return numCubes_;
            else
		return 0;
	}
      if (type.dim()==0) {
	  return numVertices_;
	} 
      if (type.dim()==1) {
	  return numEdges_;
	}
      if (type.isTriangle())
          return numTriFaces_;
      else if (type.isQuadrilateral())
          return numQuadFaces_;
      
      return 0;
  }

  //! get number of entities of given codim  
  int size (int codim) const 
  {
    return Base::size(codim);
  }

  /** deliver all geometry types used in this grid */
  const std::vector<GeometryType>& geomTypes (int codim) const
  {
	return myTypes_[codim];
  }

  //! one past the end on this level
  template<int cd, PartitionIteratorType pitype>
  typename Base::template Codim<cd>::template Partition<pitype>::Iterator begin () const
  {
	return grid_.template leafbegin<cd,pitype>();
  }
  
  //! Iterator to one past the last entity of given codim on level for partition type
  template<int cd, PartitionIteratorType pitype>
  typename Base::template Codim<cd>::template Partition<pitype>::Iterator end () const
  {
	return grid_.template leafend<cd,pitype>();
  }

    /** \brief Update the leaf indices.  This method is called after each grid change. */
    void update();

    const GridImp& grid_;

    /** \brief The lowest level that contains leaf elements

    This corresponds to UG's fullRefineLevel, which is, unfortunately only
    computed if you use some nontrivial UG algebra.  Thus we compute it
    ourselves, and use it to speed up the leaf iterators.
    */
    unsigned int coarsestLevelWithLeafElements_;



  int numSimplices_;
  int numPyramids_;
  int numPrisms_;
  int numCubes_;
  int numVertices_;
  int numEdges_;
  int numTriFaces_;
  int numQuadFaces_;

  std::vector<GeometryType> myTypes_[dim+1];
};


/** \brief Implementation class for the UGGrid Id sets

The UGGridGlobalIdSet and the UGGridLocalIdSet are virtually identical. This
class implements them both at once.  You can select the one you want using
the <tt>Local</tt> template parameter.
\param Local false for GlobalIdSet, true for LocalIdSet
*/
template <class GridImp, bool Local>
class UGGridIdSet : public IdSet<GridImp,UGGridIdSet<GridImp,Local>,unsigned int>
{
    enum {dim = remove_const<GridImp>::type::dimension};

    typedef typename std::pair<const typename UG_NS<dim>::Element*, int> Face;

    /** \brief Look for copy of a face on the next-lower grid level.

    \todo This method is not implemented very efficiently, but I will not put
    further work into it as long as the much-awaited face objects are not included
    in UG.
    \todo This method exists here and again identically in UGGridLocalIdSet */
    static Face getFatherFace(const Face& face) {

        // set up result object
        Face resultFace;
        resultFace.first = UG_NS<dim>::EFather(face.first);

        // If there is no father element then we know there is no father face
        /** \bug This is not true when doing vertical load balancing. */
        if (resultFace.first == NULL)
            return resultFace;

        // Get all corners of the face
        std::set<const typename UG_NS<dim>::Vertex*> corners;

        for (int i=0; i<UG_NS<dim>::Corners_Of_Side(face.first, face.second); i++)
            corners.insert(UG_NS<dim>::Corner(face.first, UG_NS<dim>::Corner_Of_Side(face.first, face.second, i))->myvertex);

        // Loop over all faces of the father element and look for a face that has the same vertices
        for (int i=0; i<UG_NS<dim>::Sides_Of_Elem(resultFace.first); i++) {

            // Copy father face into set
            std::set<const typename UG_NS<dim>::Vertex*> fatherFaceCorners;

            for (int j=0; j<UG_NS<dim>::Corners_Of_Side(resultFace.first, i); j++)
                fatherFaceCorners.insert(UG_NS<dim>::Corner(resultFace.first, UG_NS<dim>::Corner_Of_Side(resultFace.first, i, j))->myvertex);

            // Do the vertex sets match?
            if (corners==fatherFaceCorners) {
                resultFace.second = i;
                return resultFace;
            }

        }

        // No father face found
        resultFace.first = NULL;
        return resultFace;
    }

public:
  //! constructor stores reference to a grid
  UGGridIdSet (const GridImp& g) : grid_(g) {}

  //! define the type used for persistent indices
  typedef unsigned int GlobalIdType;
  typedef unsigned int LocalIdType;

  //! get id of an entity
  /*
    We use the remove_const to extract the Type from the mutable class,
    because the const class is not instatiated yet.
  */
  template<int cd>
  GlobalIdType id (const typename remove_const<GridImp>::type::Traits::template Codim<cd>::Entity& e) const 
  {
#ifdef ModelP
        return grid_.getRealImplementation(e).target_->ge.ddd.gid;
#else
        if (cd==0) {
            // If we're asked for the id of an element, and that element is a copy of its father, then
            // we return the id of the lowest ancestor that the element is a copy from.  That way copies
            // of elements have the same id
            const typename UG_NS<dim>::Element* ancestor = (typename UG_NS<dim>::Element* const)(grid_.getRealImplementation(e).target_);
            /** \todo We should really be using an isCopy() method rather than hasCopy() */
            while (UG_NS<dim>::EFather(ancestor) && UG_NS<dim>::hasCopy(UG_NS<dim>::EFather(ancestor)))
                ancestor = UG_NS<dim>::EFather(ancestor);
            
            return UG_NS<dim>::id(ancestor);
        }
        return UG_NS<dim>::id(grid_.getRealImplementation(e).target_);
#endif
  }

  //! get id of subEntity
  /*
    We use the remove_const to extract the Type from the mutable class,
    because the const class is not instantiated yet.
  */
  template<int cc>
  GlobalIdType subId (const typename remove_const<GridImp>::type::Traits::template Codim<0>::Entity& e, int i) const
  {
      if (cc==0)
          return id<0>(e);

      const typename UG_NS<dim>::Element* target = grid_.getRealImplementation(e).target_;
      GeometryType type = e.type();

      if (dim-cc==1) {
          
          int a=ReferenceElements<double,dim>::general(type).subEntity(i,dim-1,0,dim);	
          int b=ReferenceElements<double,dim>::general(type).subEntity(i,dim-1,1,dim);
          const typename UG_NS<dim>::Edge* edge = UG_NS<dim>::GetEdge(UG_NS<dim>::Corner(target, UGGridRenumberer<dim>::verticesDUNEtoUG(a,type)),
                                                                      UG_NS<dim>::Corner(target, UGGridRenumberer<dim>::verticesDUNEtoUG(b,type)));

          // If this edge is the copy of an edge on a lower level we return the id of that lower
          // edge, because Dune wants entities which are copies of each other to have the same id.
          const typename UG_NS<dim>::Edge* fatherEdge;
          fatherEdge = GetFatherEdge(edge);
          while (fatherEdge) {
              edge = fatherEdge;
              fatherEdge = GetFatherEdge(edge);
          }

#ifdef ModelP
          return (Local) ? edge->id : edge->ddd.gid;
#else
          return edge->id;
#endif
      }
  
      if (cc==1) {  // Faces

          Face face(target, UGGridRenumberer<dim>::facesDUNEtoUG(i,type));

          // If this face is the copy of a face on a lower level we return the id of that lower
          // face, because Dune wants entities which are copies of each other to have the same id.
          Face fatherFace;
          fatherFace = getFatherFace(face);
          while (fatherFace.first) {
              face = fatherFace;
              fatherFace = getFatherFace(face);
          }

#ifdef ModelP
          return (Local) 
              ? UG_NS<dim>::SideVector(face.first, face.second)->id
              : UG_NS<dim>::SideVector(face.first, face.second)->ddd.gid;
#else
          return UG_NS<dim>::SideVector(face.first, face.second)->id;
#endif
      }

      if (cc==dim) {
#ifdef ModelP
          return (Local)
              ? UG_NS<dim>::id(UG_NS<dim>::Corner(target,UGGridRenumberer<dim>::verticesDUNEtoUG(i,type)))
              : UG_NS<dim>::Corner(target, UGGridRenumberer<dim>::verticesDUNEtoUG(i,type))->ddd.gid;
#else
          return UG_NS<dim>::id(UG_NS<dim>::Corner(target,UGGridRenumberer<dim>::verticesDUNEtoUG(i,type)));
#endif
      }
      
      DUNE_THROW(GridError, "UGGrid<" << dim << ">::subGlobalId isn't implemented for cc==" << cc );
  }

    //private:

    /** \todo Should be private */
    void update() {}

  const GridImp& grid_;
};

}  // namespace Dune


#endif
