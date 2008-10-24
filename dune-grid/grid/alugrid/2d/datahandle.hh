#ifndef DUNE_ALU2DGRIDDATAHANDLE_HH
#define DUNE_ALU2DGRIDDATAHANDLE_HH

//- system includes 
#include <iostream>

//- local includes 
#include "alu2dinclude.hh"

using std::endl;
using std::cout;
using std::flush;

namespace ALU2DSPACENAME {
//struct AdaptRestrictProlong2dType {
//};

/////////////////////////////////////////////////////////////////
//
//  --AdaptRestrictProlong 
//
/////////////////////////////////////////////////////////////////
template <class GridType , class RestrictProlongOperatorType >
 class AdaptRestrictProlong2dImpl : public AdaptRestrictProlong2dType
{
  GridType & grid_;
  typedef Dune :: MakeableInterfaceObject<typename GridType::template Codim<0>::Entity> EntityType;
  typedef typename EntityType :: ImplementationType RealEntityType;
  typedef typename Hmesh_basic::helement_t HElementType ;
  
  EntityType & reFather_;
  EntityType & reSon_;
  RealEntityType & realFather_;
  RealEntityType & realSon_;
 
  //DofManagerType & dm_;
  RestrictProlongOperatorType & rp_;

  int maxlevel_;


public:
  //! Constructor
  AdaptRestrictProlong2dImpl (GridType & grid, 
      EntityType & f, RealEntityType & rf, EntityType & s, RealEntityType & rs
      , RestrictProlongOperatorType & rp) 
    : grid_(grid)
    , reFather_(f)
    , reSon_(s)
    , realFather_(rf) 
    , realSon_(rs) 
    , rp_(rp) 
    , maxlevel_(-1) 
  {
  }

  virtual ~AdaptRestrictProlong2dImpl () 
  {
  }

  //! restrict data , elem is always the father 
  int preCoarsening ( HElementType & elem )
  {
    // set element and then start 
    HElementType * son = elem.down();
    if(elem.level() > maxlevel_) maxlevel_ = elem.level();

    //elem.resetRefinedTag();
    assert( son );
   
    realSon_.setElement(*son);
    realFather_.setElement(elem);
    rp_.restrictLocal(reFather_,reSon_,true);

    son = son->next();
    while( son )
    {
      realSon_.setElement(*son);
      rp_.restrictLocal(reFather_,reSon_,false);
      son = son->next();
    }
    return 0;
  }

  //! prolong data, elem is the father  
  int postRefinement ( HElementType & elem )
  {
    // set element and then start 
    HElementType * son = elem.down();
    assert( son );
    //elem.resetRefinedTag();
   
    realFather_.setElement(elem);
    realSon_.setElement(*son);
    int level = realSon_.level(); 
    if(level > maxlevel_) maxlevel_ = level;
    
    rp_.prolongLocal(reFather_,reSon_, false);

    son = son->next();
    while( son )
    {
      assert( son );
  
      realSon_.setElement(*son);
      rp_.prolongLocal(reFather_,reSon_, false);
      
      son = son->next();
    }
    return 0;
  }

  int maxLevel () const { return maxlevel_; }
};

} // end namespace 
#endif
