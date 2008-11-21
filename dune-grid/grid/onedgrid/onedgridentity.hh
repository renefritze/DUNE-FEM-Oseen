#ifndef DUNE_ONE_D_GRID_ENTITY_HH
#define DUNE_ONE_D_GRID_ENTITY_HH

#include <dune/common/fixedarray.hh>

/** \file
 * \brief The OneDGridEntity class and its specializations
 */

namespace Dune {  

    // forward declarations
    template <class GridImp>
    class OneDGridLeafIntersectionIterator;
    template <class GridImp>
    class OneDGridLevelIntersectionIterator;
    template <int mydim, class GridImp>
    class OneDMakeableGeometry;

template <int mydim>
class OneDEntityImp {};

template <>
class OneDEntityImp<0>
{
public:

    OneDEntityImp(int level, double pos) : pos_(pos), level_(level), son_(NULL), pred_(NULL), succ_(NULL)
    {}

    OneDEntityImp(int level, const FieldVector<double, 1>& pos, unsigned int id) 
        : pos_(pos), id_(id), level_(level), son_(NULL), pred_(NULL), succ_(NULL)
    {}
    //private: 
    bool isLeaf() const {
        return son_==NULL;
    }

    FieldVector<double, 1> pos_;

    //! entity number 
    unsigned int levelIndex_;
    
    unsigned int leafIndex_;

    unsigned int id_;

    //! level
    int level_;

    //! Son vertex on the next finer grid
    OneDEntityImp<0>* son_;

    //!
    OneDEntityImp<0>* pred_;
    
    OneDEntityImp<0>* succ_;
    

};


template <>
class OneDEntityImp<1>
{
public:
    enum AdaptationState { NONE , COARSEN , REFINED };

    OneDEntityImp(int level, unsigned int id) 
        : id_(id), level_(level), 
          markState_(NONE), adaptationState_(NONE),
          pred_(NULL), succ_(NULL)
    {
        sons_[0] = sons_[1] = NULL;
    }

    bool isLeaf() const {
        assert( (sons_[0]==NULL && sons_[1]==NULL) || (sons_[0]!=NULL && sons_[1]!=NULL) );
        return sons_[0]==NULL && sons_[1]==NULL;
    }

    array<OneDEntityImp<1>*, 2> sons_;

    OneDEntityImp<1>* father_;

    OneDEntityImp<0>* vertex_[2];

    //! element number 
    unsigned int levelIndex_;
    
    unsigned int leafIndex_;

    /** \brief Unique and persistent id for elements */
    unsigned int id_;

    //! the level of the entity
    int level_;
    
    /** \brief Stores requests for refinement and coarsening */
    AdaptationState markState_;

    /** \brief Stores information about prospective refinement and coarsening
        for use in the interface method state() */
    AdaptationState adaptationState_;

    /** \brief Predecessor in the doubly linked list of elements */
    OneDEntityImp<1>* pred_;

    /** \brief Successor in the doubly linked list of elements */
    OneDEntityImp<1>* succ_;
    
};


template<int cd, int dim, class GridImp>
class OneDEntityWrapper :
  public GridImp::template Codim<cd>::Entity
{
public:

    OneDEntityWrapper() :
        GridImp::template Codim<cd>::Entity (OneDGridEntity<cd, dim, GridImp>())
    {}

    void setToTarget(OneDEntityImp<dim-cd>* target) {
        this->realEntity.setToTarget(target);
    }

    OneDEntityImp<dim-cd>* target() {return this->realEntity.target_;}
};


//**********************************************************************
//
// --OneDGridEntity
// --Entity
//
/** \brief The implementation of entities in a OneDGrid
 \ingroup OneDGrid

  A Grid is a container of grid entities. An entity is parametrized by the codimension.
  An entity of codimension c in dimension d is a d-c dimensional object.

 */
template<int cd, int dim, class GridImp>
class OneDGridEntity : 
        public EntityDefaultImplementation <cd,dim,GridImp,OneDGridEntity>
{

    template <int codim_, PartitionIteratorType PiType_, class GridImp_>
    friend class OneDGridLevelIterator;

    friend class OneDGrid;

    //! Constructor with a given grid level
    OneDGridEntity(int level, double coord) : geo_(coord), target_(NULL) {}

public:
    //! Constructor with a given grid level
    OneDGridEntity() : target_(NULL) {}

    typedef typename GridImp::template Codim<cd>::Geometry Geometry;
    typedef typename GridImp::template Codim<cd>::EntityPointer EntityPointer;

  //! level of this element
    int level () const {return target_->level_;}

  //! only interior entities
  PartitionType partitionType () const { return InteriorEntity; }

    unsigned int levelIndex() const {return target_->levelIndex_;}

    unsigned int leafIndex() const {return target_->leafIndex_;}

    unsigned int globalId() const {return target_->id_;}

    //! return the element type identifier (segment)
    GeometryType type () const {return GeometryType(0);}

    /*! Intra-element access to entities of codimension cc > codim. Return number of entities
      with codimension cc.
    */
    //!< Default codim 1 Faces and codim == dim Vertices 
    template<int cc> int count () const; 
    
  //! Provide access to mesh entity i of given codimension. Entities
  //!  are numbered 0 ... count<cc>()-1
    template<int cc> 
    OneDGridLevelIterator<cc,All_Partition, GridImp> entity (int i);

  //! geometry of this entity
    const Geometry& geometry () const {return geo_;}

    void setToTarget(OneDEntityImp<0>* target) {
        target_ = target;
        geo_.setToTarget(target);
    }

    //! the current geometry
    OneDMakeableGeometry<dim-cd,GridImp> geo_;

    OneDEntityImp<0>* target_;

};

//***********************
//  
//  --OneDGridEntity
//  --0Entity
//
//***********************



  /** \brief Specialization for codim-0-entities.
   * \ingroup OneDGrid
   *
   * This class embodies the topological parts of elements of the grid.
   * It has an extended interface compared to the general entity class.
   * For example, Entities of codimension 0  allow to visit all neighbors.
   *
   * OneDGrid only implements the case dim==dimworld==1
   */
template<int dim, class GridImp>
class OneDGridEntity<0,dim, GridImp> : 
        public EntityDefaultImplementation<0,dim,GridImp, OneDGridEntity>
{
    friend class OneDGrid;
    template <class GridImp_>
    friend class OneDGridLevelIntersectionIterator;
    template <class GridImp_>
    friend class OneDGridLeafIntersectionIterator;
    friend class OneDGridHierarchicIterator <GridImp>;
    friend class OneDGridLevelIterator <0,All_Partition,GridImp>;

public:
    typedef typename GridImp::template Codim<0>::Geometry Geometry;
    typedef typename GridImp::template Codim<0>::LevelIterator LevelIterator;
    typedef typename GridImp::template Codim<0>::LeafIntersectionIterator LeafIntersectionIterator;
    typedef typename GridImp::template Codim<0>::LevelIntersectionIterator LevelIntersectionIterator;
    typedef typename GridImp::template Codim<0>::HierarchicIterator HierarchicIterator;
    
    //! Default Constructor
    OneDGridEntity() {};


    //! Level of this element
    int level () const {return target_->level_;}

    //! only interior entities
    PartitionType partitionType () const { return InteriorEntity; }

    //! Level index is unique and consecutive per level and codim
    unsigned int levelIndex() const {return target_->levelIndex_;}

    unsigned int leafIndex() const {return target_->leafIndex_;}

    unsigned int globalId() const {return target_->id_;}

    //! Geometry of this entity
    const Geometry& geometry () const {return geo_;}

    //! return the element type identifier (segment)
    GeometryType type () const {return GeometryType(1);}

    /** \brief Return the number of subentities of codimension cc.
     */
    template<int cc> 
    int count () const {
        assert(cc==0 || cc==1);
        return (cc==0) ? 1 : 2;
    }

    /** \brief Return index of sub entity with codim = cc and local number i
     */
    template<int cc> 
    int subLevelIndex (int i) const {
        assert(i==0 || i==1);
        return (cc==0)
            ? target_->levelIndex_
            : target_->vertex_[i]->levelIndex_;
    }

    /** \brief Return leaf index of sub entity with codim = cc and local number i
     */
    template<int cc> 
    int subLeafIndex (int i) const {
        assert(i==0 || i==1);
        return (cc==0) 
            ? target_->leafIndex_
            : target_->vertex_[i]->leafIndex_;
    }

    /** \brief Return leaf index of sub entity with codim = cc and local number i
     */
    template<int cc> 
    int subId (int i) const {
        assert(i==0 || i==1);
        return (cc==0) 
            ? target_->id_
            : target_->vertex_[i]->id_;
    }
    
    /** \brief Provide access to sub entity i of given codimension. Entities
     *  are numbered 0 ... count<cc>()-1
     */
    template<int cc> 
    typename GridImp::template Codim<cc>::EntityPointer entity (int i) const {
        if (cc==0) {
            assert(i==0);
            // The cast is correct when this if clause is executed
            return OneDGridLevelIterator<cc,All_Partition,GridImp>( (OneDEntityImp<1-cc>*)this->target_);
        } else if (cc==1) {
            assert(i==0 || i==1);
            // The cast is correct when this if clause is executed
            return OneDGridLevelIterator<cc,All_Partition,GridImp>( (OneDEntityImp<1-cc>*)this->target_->vertex_[i]);
        }
    }

    LeafIntersectionIterator ileafbegin () const {
        return OneDGridLeafIntersectionIterator<GridImp>(target_, (isLeaf()) ? 0 : 2);
    }

    LevelIntersectionIterator ilevelbegin () const {
        return OneDGridLevelIntersectionIterator<GridImp>(target_, 0);
    }
    
    LeafIntersectionIterator ileafend () const {
        return OneDGridLeafIntersectionIterator<GridImp>(target_);
    }

    LevelIntersectionIterator ilevelend () const {
        return OneDGridLevelIntersectionIterator<GridImp>(target_);
    }
    
    //! returns true if Entity has no children 
    bool isLeaf () const {
        return (target_->sons_[0]==NULL) && (target_->sons_[1]==NULL);
    }
    
    //! Inter-level access to father element on coarser grid. 
    //! Assumes that meshes are nested.
    OneDGridEntityPointer<0, GridImp> father () const {
        return OneDGridEntityPointer<0,GridImp>(target_->father_);
    }
    
    /*! Location of this element relative to the reference element 
      of the father. This is sufficient to interpolate all 
      dofs in conforming case. Nonconforming may require access to 
      neighbors of father and computations with local coordinates.
      On the fly case is somewhat inefficient since dofs  are visited 
      several times. If we store interpolation matrices, this is tolerable. 
      We assume that on-the-fly implementation of numerical algorithms 
      is only done for simple discretizations. Assumes that meshes are nested.
    */
    const Geometry& geometryInFather () const {
        assert(target_->father_);
        assert(target_->father_->sons_[0] == target_ || target_->father_->sons_[1] == target_);

        if (target_->father_->sons_[0] == target_ && target_->father_->sons_[1] == target_) {
            // Copied element?
            geometryInFather_.setPositions(0,1);
        } else if (target_->father_->sons_[0] == target_) {
            // Left son?
            geometryInFather_.setPositions(0,0.5);
        } else {
            // Right son!
            geometryInFather_.setPositions(0.5,1);
        }
            
        return geometryInFather_;
    }

    /*! Inter-level access to son elements on higher levels<=maxlevel.
      This is provided for sparsely stored nested unstructured meshes.
      Returns iterator to first son.
    */
    OneDGridHierarchicIterator<GridImp> hbegin (int maxlevel) const {
        
        OneDGridHierarchicIterator<GridImp> it(maxlevel);

        if (level()<=maxlevel) {

          // Load sons of old target onto the iterator stack
            if (!isLeaf()) {
                typename OneDGridHierarchicIterator<GridImp>::StackEntry se0;
              se0.element = target_->sons_[0];
              se0.level   = level() + 1;
              it.elemStack.push(se0);

              typename OneDGridHierarchicIterator<GridImp>::StackEntry se1;
              se1.element = target_->sons_[1];
              se1.level   = level() + 1;
              it.elemStack.push(se1);
            }

        }

        it.virtualEntity_.setToTarget((it.elemStack.empty()) ? NULL : it.elemStack.top().element);
        
        return it;
    }
    
    //! Returns iterator to one past the last son
    HierarchicIterator hend (int maxlevel) const {
        return HierarchicIterator(maxlevel);
    }
    
    // ***************************************************************
    //  Interface for Adaptation
    // ***************************************************************
      
    /** returns true, if entity might be coarsened during next adaptation cycle */
    bool mightBeCoarsened () const { return target_->adaptationState_ == OneDEntityImp<1> :: COARSEN; }
    /** returns true, if entity was refined during last adaptation cycle */
    bool wasRefined () const { return target_->adaptationState_ == OneDEntityImp<1> :: REFINED; }
    
    void setToTarget(OneDEntityImp<1>* target) {
        target_ = target;
        geo_.setToTarget(target);
    }


    //! the current geometry
    OneDMakeableGeometry<dim,GridImp> geo_;

    mutable OneDMakeableGeometry<dim,GridImp> geometryInFather_;

    OneDEntityImp<1>* target_;

}; // end of OneDGridEntity codim = 0

} // namespace Dune

#endif
