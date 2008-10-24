#ifndef DUNE_GRID_ENTITY_HH
#define DUNE_GRID_ENTITY_HH

#include <dune/common/typetraits.hh>
#include "grid.hh"

namespace Dune
{

/**
   @brief Wrapper class for entities


   Template parameters are:

   - <tt>cd</tt> Codimension of the entity
   - <tt>dim</tt> Dimension of the grid
   - <tt>GridImp</tt> Type that is a model of Dune::Grid
   - <tt>EntityImp</tt> Class template that is a model of Dune::Entity
 

   <H3>Engine Concept</H3>

   This class wraps a object of type EntityImp and forwards all member 
   function calls to corresponding members of this class. In that sense Entity
   defines the interface and EntityImp supplies the implementation.
   For various reasons we do not use an inheritance hierarchy and the
   Barton-Nackman trick here.


   <H3>Specialization</H3>

   The Entity class template is specialized for <tt>cd=0</tt> (elements,
   Dune::Entity<0,dim,GridImp,EntityImp>).
   This case has an extended interface.
   The methods defined in the general template
   are provided by the specialization as well. We did not use inheritance
   because different implementations for different codimensions may be required
   and virtual functions had to be avoided.

   This relation is shown in the following diagram:
   
   \dot
      digraph entity {
         rankdir=LR;
         node [ shape=record, fontname=Helvetica, fontsize=10, height=0.25 ];
         Entity [ label="Dune::Entity\<cd,dim,GridImp,EntityImp\>"
                  style=filled, bgcolor=lightgrey
                  shape=record, URL="\ref Dune::Entity"];
         Element [ label="Dune::Entity\<0,dim,GridImp,EntityImp\>"
                   URL="\ref Dune::Entity<0,dim,GridImp,EntityImp>"];
         Vertex [ label="Dune::Entity\<dim,dim,GridImp,EntityImp\>"
                  URL="\ref Dune::Entity<dim,dim,GridImp,EntityImp>"];
         Entity -> Element [ dirType="back", arrowType="open",
                             style="dashed"
                             fontname=Helvetica, fontsize=8, 
                             label="cd=0" ];
         Entity -> Vertex [ dirType="back", arrowType="open", style="dashed"
                             fontname=Helvetica, fontsize=8, 
                             label="cd=dim" ];
      }
   \enddot


   <H3>View concept</H3>

   Entities can not be created, assigned or otherwise modified outside
   the interface in the user code. They are only accessible by immutable
   iterators provided on the corresponding grid class.

   The only way to modify the entities of a grid is through grid adaptation which
   consists of tagging entities (of codimension 0) for refinement and then
   calling the adapt() method on the grid.


   \ingroup GIEntity
   \nosubgrouping
*/
template<int cd, int dim, class GridImp, template<int,int,class> class EntityImp>
class Entity {
  enum { dimworld = GridImp::dimensionworld };
  typedef typename GridImp::ctype ct;
protected:
  EntityImp<cd,dim,GridImp> realEntity;
public:

  //===========================================================
  /** @name Exported types and constants
   */
  //@{
  //===========================================================
protected:
  // The type of the wrapped implementation, for internal use only
  typedef EntityImp<cd,dim,GridImp> ImplementationType; 

public:
  //! \brief The corresponding geometry type 
  typedef typename GridImp::template Codim<cd>::Geometry Geometry;

  enum {
    //! \brief Know your own codimension. 
    codimension=cd
  };
  enum {
    //! \brief Know the grid dimension.
    dimension=dim
  };
  enum {
    //! \brief Dimensionality of the reference element of the entity. 
    mydimension=dim-cd
  };
  enum {
    //! \brief Know the dimension of world.
    dimensionworld=dimworld
  };

  //! @brief coordinate type of the Grid 
  typedef ct ctype;
  //@}



  //===========================================================
  /** @name Methods shared by entities of all codimensions
   */
  //@{
  //===========================================================

  //! The level of this entity
  int level () const { return realEntity.level(); }

  //! Partition type of this entity
  PartitionType partitionType () const { return realEntity.partitionType(); }

  /*! \brief Each entity encapsulates an object of type
  Dune::Geometry<dimension-codimension,dimensionworld,...> that
  gives (among other things) the map from a reference element to world coordinates.
  This method delivers a const reference to such a geometry.
  
  \note Be careful when storing such references. If the state
  of any object is changed, e.g. an iterator is advanced, there
  is no guarantee that the reference remains valid.
  */
  const Geometry& geometry () const { return realEntity.geometry(); }
  //@}

    /** \brief Return the name of the reference element. The type can
    be used to access the Dune::ReferenceElement.
   */
    GeometryType type () const { return realEntity.type(); };

  //===========================================================
  /** @name Interface for the implementor
   */
  //@{
  //===========================================================

  //! Copy constructor from EntityImp
  explicit Entity(const EntityImp<cd,dim,GridImp> & e) : realEntity(e) {};  

  /* not part of the interface but maybe in later versions
     \brief Id of the boundary which is associated with the entity, 
   returns 0 for inner entities, arbitrary int otherwise 
  */
  //int boundaryId () const { return realEntity.boundaryId(); }

  //@}

protected:
  typedef typename remove_const<GridImp>::type mutableGridImp;
  
  //===========================================================
  /** @name Protected methods
   */
  //@{
  //===========================================================

  // give the GridDefaultImplementation class access to the realImp 
  friend class GridDefaultImplementation<
            GridImp::dimension, GridImp::dimensionworld,
            typename GridImp::ctype,
            typename GridImp::GridFamily> ;
  
  //! return reference to the real implementation 
  EntityImp<cd,dim,GridImp> & getRealImp() { return realEntity; }
  //! return reference to the real implementation 
  const EntityImp<cd,dim,GridImp> & getRealImp() const { return realEntity; }

protected:  
  /** hide copy constructor */
  Entity(const Entity& rhs) : realEntity(rhs.realEntity) {};
  /** hide assignement operator */
  Entity & operator = (const Entity& rhs) { 
      realEntity = rhs.realEntity; 
      return *this;
  }
  //@}
};

/**
   @brief Template specialization of Dune::Entity for Elements (codim==0)

   @see Dune::Entity (general version) for the full documentation
   
   \ingroup GIEntity
   \nosubgrouping
*/
template<int dim, class GridImp, template<int,int,class> class EntityImp>
class Entity <0,dim,GridImp,EntityImp> 
{
  enum { dimworld = GridImp::dimensionworld };
  typedef typename GridImp::ctype ct;

protected:
  typedef typename remove_const<GridImp>::type mutableGridImp;

  EntityImp<0,dim,GridImp> realEntity;

  // The type of the wrapped implementation, for internal use only
  typedef EntityImp<0,dim,GridImp> ImplementationType; 
public:

  //===========================================================
  /** @name Exported types and constants
   */
  //@{
  //===========================================================

  /** \brief The geometry type of this entity */
  typedef typename GridImp::template Codim<0>::Geometry Geometry;

  /** \brief The geometry type of this entity when the geometry is expressed
   embedded in the father element.  This differs from Geometry only when
  dim != dimworld.*/
  typedef typename GridImp::template Codim<0>::LocalGeometry LocalGeometry;

  /** \brief EntityPointer types of the different codimensions */
  template <int cd>
  struct Codim
  {
    typedef typename GridImp::template Codim<cd>::EntityPointer EntityPointer;
  };

    /** \brief The codim==0 EntityPointer type */
  typedef typename GridImp::template Codim<0>::EntityPointer EntityPointer;

  /** \brief The LeafIntersectionIterator type*/
  typedef typename GridImp::template Codim<0>::LeafIntersectionIterator LeafIntersectionIterator;

  /** \brief The LevelIntersectionIterator type*/
  typedef typename GridImp::template Codim<0>::LevelIntersectionIterator LevelIntersectionIterator;

    /** \brief The HierarchicIterator type*/
  typedef typename GridImp::template Codim<0>::HierarchicIterator HierarchicIterator;
  
  enum { 
  //! Know your own codimension
  codimension=0 
  };
  enum { 
  //! Know the grid's dimension
  dimension=dim 
  };
  enum { 
    /** \brief Know dimension of the entity */
  mydimension=dim 
  };
  enum { 
  //! Know the world dimension
  dimensionworld=dimworld 
  };
  //! Type used for coordinates in grid module
  typedef ct ctype;
  //@}


  //===========================================================
  /** @name Methods shared by entities of all codimensions
   */
  //@{
  //===========================================================

  //! @copydoc Dune::Entity::level()
  int level () const { return realEntity.level(); }

  //! @copydoc Dune::Entity::partitionType()
  PartitionType partitionType () const { return realEntity.partitionType(); }

  //! @copydoc Dune::Entity::geometry()
  const Geometry& geometry () const { return realEntity.geometry(); }
  //@}

    /** \brief Return the name of the reference element. The type can
        be used to access the Dune::ReferenceElement.
    */
    GeometryType type () const { return realEntity.type(); };

  //===========================================================
  /** @name Extended interface of entities of codimension 0
   */
  //@{
  //===========================================================

  /**\brief Number of subentities with codimension <tt>cc</tt>. This method is in
   principle redundant because this information can be obtained via the reference
   element of the geometry. It is there for efficiency reasons and to make
   the interface self-contained.
   */
  template<int cc> int count () const { return realEntity.count<cc>(); }

  /** \brief Access to subentity <tt>i</tt> of codimension <tt>cc</tt>.
   */ 
  template<int cc> typename Codim<cc>::EntityPointer entity (int i) const
    {
      return realEntity.entity<cc>(i);
    }
 
  /**\brief Access to intersections with neighboring leaf elements. 
     A neighbor is an entity of codimension 0
     which has an intersection of codimension 1 in common with this entity. 
   Access to those neighbors is provided using the IntersectionIterator. 
     This method returns an iterator refering to the first neighbor.
  */
  LeafIntersectionIterator ileafbegin () const
    {
      return realEntity.ileafbegin();
    }
  
  /**\brief Reference to an IntersectionIterator one past the last intersection
  */
  LeafIntersectionIterator ileafend () const
    {
      return realEntity.ileafend();
    }

  /**\brief Intra-level access to intersections with neighboring elements. 
     A neighbor is an entity of codimension 0
     which has an intersection of codimension 1 in common with this entity. 
   Access to those neighbors is provided using the IntersectionIterator. 
     This method returns an iterator refering to the first neighbor.
  */
  LevelIntersectionIterator ilevelbegin () const
    {
      return realEntity.ilevelbegin();
    }
  
  /**\brief Reference to an IntersectionIterator one past the last intersection
  */
  LevelIntersectionIterator ilevelend () const
    {
      return realEntity.ilevelend();
    }

  /**\brief Inter-level access to father entity on the next-coarser grid.
   The given entity resulted directly from a subdivision of its father
   entity. For the macro elements dereferencing the EntityPointer is undefined.
  */
  EntityPointer father () const
    {
      return realEntity.father();
    }

  //! Returns true if the entity is contained in the leaf grid
  bool isLeaf () const
    {
      return realEntity.isLeaf();
    }

  /** @brief Returns true if element is of regular type in red/green type refinement. 
    In bisection or hanging node refinement this is always true.
  */ 
  bool isRegular() const { return realEntity.isRegular(); }

  /**\brief Provides information how this element has been subdivided from 
   its father element. 
   The returned LocalGeometry is a model of Dune::Geometry<dimension,dimension,...>
   mapping from the reference element of the given element to the reference
   element of the father element.
     This is sufficient to interpolate all degrees of freedom in the 
   conforming case. Nonconforming may require access to neighbors of father and
     computations with local coordinates.
     On the fly case is somewhat inefficient since degrees of freedom
   may be visited several times.
     If we store interpolation matrices, this is tolerable. We assume that on-the-fly
     implementation of interpolation is only done for simple discretizations.
  */
  const LocalGeometry& geometryInFather () const
    {
      return realEntity.geometryInFather();
    }

  /**\brief Inter-level access to elements that resulted from (recursive)
   subdivision of this element. 

   \param[in] maxlevel Iterator does not stop at elements with level greater than maxlevel.
   \return Iterator to the first son (level is not greater than maxlevel)
  */
  HierarchicIterator hbegin (int maxlevel) const
    {
      return realEntity.hbegin(maxlevel);
    }

  /** \brief Returns iterator to one past the last son element
  */
  HierarchicIterator hend (int maxlevel) const
    {
      return realEntity.hend(maxlevel);
    }

  /**\brief Returns true, if entity was refined during last adaptation cycle 
  */
  bool wasRefined () const { return realEntity.wasRefined(); }
  
  /**\brief Returns true, if entity might be coarsened during next adaption cycle 
  */
  bool mightBeCoarsened () const { return realEntity.mightBeCoarsened (); }

  //===========================================================
  /** @name Interface for the implementor
   */
  //@{
  //===========================================================
  /**\brief Returns true, if entity has intersections with boundary, see
       default implementation  
  */
  bool hasBoundaryIntersections () const { return realEntity.hasBoundaryIntersections(); }    

  //! Copy constructor from EntityImp
  explicit Entity(const EntityImp<0,dim,GridImp> & e) : realEntity(e) {};  

  // @copydoc Dune::Entity::boundaryId()
  // maybe available in later versions 
  //int boundaryId () const { return realEntity.boundaryId(); }

  /* not part of the interface, mybe in later versions 
     \brief The boundaryId of the i-th subentity of codimension <tt>cc</tt>
     
     This does the same as <code>entity<cc>(i).boundaryId()</code>, but it is
     usually a lot faster.
   */
  /*
  template <int cc> int subBoundaryId  ( int i ) const
    {
      return realEntity.subBoundaryId<cc>(i);
    }
  */

  //@}


protected:
  //===========================================================
  /** @name Protected methods
   */
  //@{
  //===========================================================

  // give the GridDefaultImplementation class access to the realImp 
  friend class GridDefaultImplementation<
            GridImp::dimension, GridImp::dimensionworld,
            typename GridImp::ctype,
            typename GridImp::GridFamily> ;

  //! return reference to the real implementation 
  EntityImp<0,dim,GridImp> & getRealImp() { return realEntity; }
  //! return reference to the real implementation 
  const EntityImp<0,dim,GridImp> & getRealImp() const { return realEntity; }

protected:
  /** hide copy constructor */
  Entity(const Entity& rhs) : realEntity(rhs.realEntity) {};
  /** hide assignement operator */
  Entity & operator = (const Entity& rhs) {
    realEntity = rhs.realEntity;
    return *this;
  };
  //@}
};


//********************************************************************
/**
   @brief Default Implementations for EntityImp

   EntityDefaultImplementation provides default implementations for Entity which uses
   the implemented interface which has to be done by the user. 

   @note this is the general version, but there is a specialization for cd=0

   @ingroup GridDevel
*/
template<int cd, int dim, class GridImp, template<int,int,class> class EntityImp>
class EntityDefaultImplementation 
{
  enum { dimworld = GridImp::dimensionworld };
  typedef typename GridImp::ctype ct;
public:
  //! know your own codimension
  enum { codimension=cd };

  //! know your own dimension
  enum { dimension=dim };

  /** \brief Know dimension of the entity */
  enum { mydimension=dim-cd };

  //! know your own dimension of world
  enum { dimensionworld=dimworld };

  //! define type used for coordinates in grid module
  typedef ct ctype;

    /** \brief Return the name of the reference element. The type can
        be used to access the Dune::ReferenceElement.
    */
    GeometryType type () const { return asImp().geometry().type(); };

private:
  //!  Barton-Nackman trick 
  EntityImp<cd,dim,GridImp>& asImp () 
  { 
  return static_cast<EntityImp<cd,dim,GridImp>&>(*this); 
  }
  const EntityImp<cd,dim,GridImp>& asImp () const 
  { 
  return static_cast<const EntityImp<cd,dim,GridImp>&>(*this); 
  }
}; // end EntityDefaultImplementation

//********************************************************************
/**
   @brief Default Implementations for EntityImp (Elements [cd=0])

   EntityDefaultImplementation provides default implementations for Entity which uses
   the implemented interface which has to be done by the user. 

   @note
   this specialization has an extended interface compared to the general case

   @ingroup GridDevel
*/
template<int dim, class GridImp, template<int,int,class> class EntityImp>
class EntityDefaultImplementation <0,dim,GridImp,EntityImp> 
{
  enum { dimworld = GridImp::dimensionworld };
  typedef typename GridImp::ctype ct;

public:
  //! know your own codimension
  enum { codimension=0 };

  //! know your own dimension
  enum { dimension=dim };

  /** \brief Know dimension of the entity */
  enum { mydimension=dim };

  //! know your own dimension of world
  enum { dimensionworld=dimworld };

  //! define type used for coordinates in grid module
  typedef ct ctype;

  /** @brief Returns true if element is of regular type in red/green type refinement. 
    In bisection or hanging node refinement this is always true.
  */ 
  bool isRegular() const { return true; }

    /** \brief Return the name of the reference element. The type can
        be used to access the Dune::ReferenceElement.
    */
    GeometryType type () const { return asImp().geometry().type(); };

  /* maybe in later versions 
   * \brief Default implementation for access to boundaryId of sub entities 
   *
   * Default implementation for access to boundaryId via interface method
   * entity<codim>.boundaryId(), default is very slow, but works, can be
   * overloaded be the actual grid implementation.
   */
  /*
  template <int cc> int subBoundaryId  ( int i ) const
    {
      return (asImp().template entity<cc>(i))->boundaryId();
    }
  */

  /**\brief Returns true, if entity was refined during last adaptation cycle 
  */
  bool wasRefined () const { return false; }

  /**\brief Returns true, if entity might be coarsened during next adaption cycle 
  */
  bool mightBeCoarsened () const { return false; }

  /**\brief Returns true, if entity has intersections with boundary,
     this implementation uses the Level- and LeafIntersectionIterator to
     check for boundary intersections
  */
  bool hasBoundaryIntersections () const 
  {
    {
      typedef typename GridImp :: template Codim<0> :: LevelIntersectionIterator
        IntersectionIterator; 
      IntersectionIterator end = asImp().ilevelend();
      for(IntersectionIterator it = asImp().ilevelbegin(); it != end; ++it)
      {
        if( it.boundary() ) return true;
      }
    }
    
    {
      typedef typename GridImp :: template Codim<0> :: LeafIntersectionIterator
        IntersectionIterator; 
      IntersectionIterator end = asImp().ileafend();
      for(IntersectionIterator it = asImp().ileafbegin(); it != end; ++it)
      {
        if( it.boundary() ) return true;
      }
    }
    
    return false;
  }

private:
  //  Barton-Nackman trick 
  EntityImp<0,dim,GridImp>& asImp () { return static_cast<EntityImp<0,dim,GridImp>&>(*this); }
  const EntityImp<0,dim,GridImp>& asImp () const { return static_cast<const EntityImp<0,dim,GridImp>&>(*this); }
};

}

#endif // DUNE_GRID_ENTITY_HH
