#ifndef DUNE_UGGRID_ENTITY_POINTER_HH
#define DUNE_UGGRID_ENTITY_POINTER_HH


namespace Dune {

/*! Acts as a pointer to an  entities of a given codimension.
 */
template<int codim, class GridImp>
class UGGridEntityPointer :
        public EntityPointerDefaultImplementation <codim, GridImp,
                                     Dune::UGGridEntityPointer<codim,GridImp> >
{
  enum { dim = GridImp::dimension };
public:
  typedef typename GridImp::template Codim<codim>::Entity Entity;
  typedef UGGridEntityPointer<codim,GridImp> Base;

  //! constructor
    UGGridEntityPointer ()  { 
        virtualEntity_.setToTarget(0);
    }

  //! constructor
    UGGridEntityPointer (typename UG_NS<dim>::template Entity<codim>::T* target)
        : virtualEntity_(target) 
    {}

    void setToTarget(typename UG_NS<dim>::template Entity<codim>::T* target) {
        virtualEntity_.setToTarget(target);
    }

  //! equality
    bool equals(const UGGridEntityPointer<codim,GridImp>& i) const {
        return virtualEntity_.getTarget() == i.virtualEntity_.getTarget();
    }

  //! dereferencing
    Entity& dereference() const {
        return virtualEntity_;
    }

  //! ask for level of entity
    int level () const {return virtualEntity_.level();}

protected:

  mutable UGMakeableEntity<codim,dim,GridImp> virtualEntity_;  //!< virtual entity

};


} // end namespace Dune

#endif
