#ifndef DUNE_EXAMPLEDISCRETEMODELS_HH
#define DUNE_EXAMPLEDISCRETEMODELS_HH

#include <dune/fem/pass/dgpass.hh>
#include <dune/fem/pass/discretemodel.hh>
#include <dune/fem/pass/selection.hh>
#include <dune/fem/solver/timeprovider.hh>
#include <dune/fem/pass/limitpass.hh>

// Dune includes
#include <dune/common/utility.hh>
#include <dune/fem/gridpart/gridpart.hh>
#include <dune/fem/space/dgspace/dgadaptiveleafgridpart.hh>

//- local includes 
#include <dune/fem/space/dgspace.hh>
#include <dune/fem/space/combinedspace.hh>

#include <dune/fem/function/adaptivefunction.hh>
#if HAVE_DUNE_ISTL
#include <dune/fem/function/blockvectorfunction.hh>
#endif
#include <dune/fem/quadrature/cachequad.hh>

//*************************************************************
namespace Dune {  

  // forward declaration of Models
  template <class Model, class NumFlux, int polOrd,
           int passId1 = -1 >
    class AdvDiffDModel1;

  template <class Model, class NumFlux, int polOrd,
           int passId1 = -1, int passId2 = -1 >
    class AdvDiffDModel2; // with advection and diffusion


  // General Traits class
  template <class Model,int dimRange,int polOrd>
  class PassTraits {
  public:
    typedef typename Model::Traits ModelTraits;
    typedef typename ModelTraits::GridPartType GridPartType;
    typedef typename GridPartType :: GridType GridType;
    enum { dimDomain = Model::Traits::dimDomain };

    typedef CachingQuadrature<GridPartType,0> VolumeQuadratureType;
    typedef CachingQuadrature<GridPartType,1> FaceQuadratureType;

    typedef FunctionSpace<double, double, dimDomain, 1> FunctionSpaceType; 
    typedef DiscontinuousGalerkinSpace<FunctionSpaceType, GridPartType, 
               polOrd,CachingStorage> SingleDiscreteFunctionSpaceType;
    // Allow generalization to systems
    typedef CombinedSpace<SingleDiscreteFunctionSpaceType, dimRange> 
            DiscreteFunctionSpaceType; 
    typedef AdaptiveDiscreteFunction<DiscreteFunctionSpaceType> DestinationType;
  };

  /*********************************************
   * Discrete model traits                     *
   ********************************************/
  template <class Model, class NumFlux, int polOrd, int passId1 = -1 >
  struct AdvDiffTraits1 
  {
    typedef typename Model::Traits ModelTraits;
    typedef typename ModelTraits::GridType GridType;

    enum { dimRange = ModelTraits::dimGradRange };
    enum { dimDomain = ModelTraits::dimDomain };

    typedef PassTraits<Model,dimRange,polOrd> Traits;
    typedef typename Traits::FunctionSpaceType FunctionSpaceType;

    typedef typename Traits::VolumeQuadratureType VolumeQuadratureType;
    typedef typename Traits::FaceQuadratureType FaceQuadratureType;
    typedef typename Traits::GridPartType GridPartType;

    typedef typename Traits::DiscreteFunctionSpaceType DiscreteFunctionSpaceType;
    typedef typename Traits::DestinationType DestinationType;
    typedef DestinationType DiscreteFunctionType;

    typedef typename ModelTraits::DomainType DomainType;
    typedef typename DestinationType::RangeType RangeType;
    typedef typename DestinationType::JacobianRangeType JacobianRangeType;

    typedef AdvDiffDModel1<Model, NumFlux, polOrd, passId1> DiscreteModelType;
  };

  template <class Model, class NumFlux, int polOrd,
	    int passId1 = -1, int passId2 = -1>
  struct AdvDiffTraits2
  {
    typedef typename Model::Traits ModelTraits;
    typedef typename ModelTraits::GridType GridType;

    enum { dimRange = ModelTraits::dimRange };
    enum { dimDomain = ModelTraits::dimDomain };

    typedef PassTraits<Model,dimRange,polOrd> Traits;
    typedef typename Traits::FunctionSpaceType FunctionSpaceType;
    
    typedef typename Traits::VolumeQuadratureType VolumeQuadratureType;
    typedef typename Traits::FaceQuadratureType FaceQuadratureType;
    typedef typename Traits::GridPartType GridPartType;
    typedef typename Traits::DiscreteFunctionSpaceType DiscreteFunctionSpaceType;
    typedef typename Traits::DestinationType DestinationType;
    typedef DestinationType DiscreteFunctionType;

    typedef typename DestinationType::DomainType DomainType;
    typedef typename DestinationType::RangeType RangeType;
    typedef typename DestinationType::JacobianRangeType JacobianRangeType;

    typedef AdvDiffDModel2<Model, NumFlux, polOrd, passId1, passId2> DiscreteModelType;
  };

  /**********************************************
   * Discrete Models for LDG Passes             *
   *********************************************/
  template <class Model, class NumFlux, int polOrd, int passId1>
  class AdvDiffDModel1 :
    public DiscreteModelDefault
      <AdvDiffTraits1<Model, NumFlux, polOrd, passId1>, passId1 > 
  {
    // This type definition allows a convenient access to arguments of passes.
    Int2Type<passId1> u0Var;
  public:
    typedef AdvDiffTraits1<Model, NumFlux, polOrd, passId1> Traits;
    
    typedef FieldVector<double, Traits::dimDomain> DomainType;
    typedef FieldVector<double, Traits::dimDomain-1> FaceDomainType;
    typedef typename Traits::RangeType RangeType;
    typedef typename Traits::GridType GridType;
    typedef typename Traits::JacobianRangeType JacobianRangeType;
    typedef typename Traits::GridPartType::IntersectionIteratorType IntersectionIterator;
    typedef typename GridType::template Codim<0>::Entity EntityType;
    
  public:
    /**
     * @brief constructor
     */
    AdvDiffDModel1(const Model& mod,
                   const NumFlux& numf) :
      model_(mod),
      numflux_(numf),
      // Set CFL number for diffusion
      cflDiffinv_(2.*(polOrd+1.))
    {}

    bool hasSource() const { return false; }
    bool hasFlux() const { return true; }
    
    /**
     * @brief method required by LocalDGPass
     *
     * @param it intersection
     * @param time current time given by TimeProvider
     * @param x coordinate of required evaluation local to \c it
     * @param uLeft DOF evaluation on this side of \c it
     * @param uRight DOF evaluation on the other side of \c it
     * @param gLeft result for this side of \c it
     * @param gRight result for the other side of \c it
     * @return time step estimate 
     */ 
    template <class ArgumentTuple>
    double numericalFlux(IntersectionIterator& it,
                         double time, const FaceDomainType& x,
                         const ArgumentTuple& uLeft, 
                         const ArgumentTuple& uRight,
                         RangeType& gLeft,
                         RangeType& gRight)
    { 
      const DomainType normal = it.integrationOuterNormal(x);

      JacobianRangeType diffmatrix(0.0);
      RangeType diffflux(0.);
      /* central differences (might be suboptimal) */
      model_.diffusion1(*it.inside(),  /* get entity */
                        time,          /* for time dependent diffusion */
                        it.intersectionSelfLocal().global(x), 
                                       /* midpoint of intersection */
                        uLeft[u0Var],  /* { u_(x^-) } */
                        diffmatrix     /* return diffusion tensor */
                        );
      diffmatrix.umv(normal,diffflux);
      model_.diffusion1(*it.inside(),  /* get entity */
                        time,          /* for time dependent diffusion */
                        it.intersectionSelfLocal().global(x), 
                                       /* midpoint of intersection */
                        uRight[u0Var], /* { u_(x^+) } */
                        diffmatrix     /* return diffusion tensor */
                        );
      diffmatrix.umv(normal,diffflux);
      diffflux*=0.5;

      gLeft = diffflux;
      gRight = diffflux;

      // upper bound for the next time step length
      return model_.diffusionTimeStep()*cflDiffinv_;
    }

    /**
     * @brief same as numericalFlux() but for the boundary
     */
    template <class ArgumentTuple>
    double boundaryFlux(IntersectionIterator& it,
                        double time, const FaceDomainType& x,
                        const ArgumentTuple& uLeft,
                        RangeType& gLeft)
    {
      const DomainType normal = it.integrationOuterNormal(x);

      typedef typename ArgumentTuple::template Get<passId1>::Type UType;

      JacobianRangeType diffmatrix;
      gLeft*=0.;
      if (model_.hasBoundaryValue(it,time,x)) {
        // get the boundary value and use it to compute the flux
        UType uRight;
        // uRight == boundary Value
	model_.boundaryValue(it, time, x, uLeft[u0Var], uRight);
        // diffmatrix == sqrt(eps) * uRight
	model_.diffusion1(*it.inside(), time,
			 it.intersectionSelfLocal().global(x),
			 uRight, diffmatrix);
      } else {
        model_.diffusion1(*it.inside(), time,
                         it.intersectionSelfLocal().global(x),
                         uLeft[u0Var], diffmatrix);
      }
      diffmatrix.umv(normal,gLeft);

      return model_.diffusionTimeStep()*cflDiffinv_/2; 
    }

    /**
     * @brief method required by LocalDGPass
     */
    template <class ArgumentTuple>
    void analyticalFlux(EntityType& en,
                        double time, const DomainType& x,
                        const ArgumentTuple& u, JacobianRangeType& f)
    { 
      typedef typename ElementType<0, ArgumentTuple>::Type UType;
      model_.diffusion1(en, time, x, u[u0Var], f);
    }

  private:
    const Model& model_;
    const NumFlux& numflux_;
    const double cflDiffinv_;
  };

  // Discrete Model for Pass2
  template <class Model, class NumFlux, int polOrd, int passId1, int passId2>
  class AdvDiffDModel2 : 
    public DiscreteModelDefault
      <AdvDiffTraits2<Model, NumFlux, polOrd, passId1, passId2>, passId1, passId2 > 
  {
    // These type definitions allow a convenient access to arguments of pass.  
    Int2Type<passId1> u0Var;  /*@\label{dm:int2type0}@*/
    Int2Type<passId2> u1Var;  /*@\label{dm:int2type1}@*/
  public:
    typedef AdvDiffTraits2<Model, NumFlux, polOrd, passId1, passId2> Traits;

    typedef FieldVector<double, Traits::dimDomain> DomainType;
    typedef FieldVector<double, Traits::dimDomain-1> FaceDomainType;
    
    typedef typename Traits::RangeType RangeType;
    typedef typename Traits::GridType GridType;
    typedef typename Traits::JacobianRangeType JacobianRangeType;
    typedef typename Traits::GridPartType::IntersectionIteratorType 
      IntersectionIterator;
    typedef typename GridType::template Codim<0>::Entity EntityType;

  public:
    /**
     * @brief constructor
     */
    AdvDiffDModel2(const Model& mod,const NumFlux& numf) :
      model_(mod),
      numflux_(numf)
    {}

    bool hasSource() const { return false; }  /*@\label{dm:hasSource}@*/
    bool hasFlux() const { return true; }

    /**
     * @brief method required by LocalDGPass
     *
     * @param it intersection
     * @param time current time given by TimeProvider
     * @param x coordinate of required evaluation local to \c it
     * @param uLeft DOF evaluation on this side of \c it
     * @param uRight DOF evaluation on the other side of \c it
     * @param gLeft result for this side of \c it
     * @param gRight result for the other side of \c it
     * @return time step estimate 
     */ 
    template <class ArgumentTuple>
    double numericalFlux(IntersectionIterator& it,
                         double time, const FaceDomainType& x,
                         const ArgumentTuple& uLeft, 
                         const ArgumentTuple& uRight,
                         RangeType& gLeft,
                         RangeType& gRight)
    { 
      const DomainType normal = it.integrationOuterNormal(x);


      /*****************************
       * Advection                 *
       ****************************/
      // delegated to numflux_
      double ldt;
      ldt=numflux_.                                /*@\label{dm:upwind}@*/
        numericalFlux(it, time, x, uLeft[u0Var], uRight[u0Var], gLeft, gRight);

      /*****************************
       * Diffusion                 *
       ****************************/
      JacobianRangeType diffmatrix;
      RangeType diffflux(0.);
      /* Central differences */
      model_.                                      /*@\label{dm:int2typeusage}@*/
        diffusion2(*it.inside(), time, it.intersectionSelfLocal().global(x),
                   uLeft[u0Var], uLeft [u1Var], diffmatrix);
      diffmatrix.umv(normal, diffflux);
      model_.
        diffusion2(*it.outside(), time, it.intersectionNeighborLocal().global(x),
                   uRight[u0Var], uRight[u1Var], diffmatrix);
      diffmatrix.umv(normal, diffflux);
      diffflux*=0.5;

      gLeft += diffflux;
      gRight += diffflux;
      return ldt;
    }

    /**
     * @brief same as numericalFlux() but for the boundary
     */
    template <class ArgumentTuple>
    double boundaryFlux(IntersectionIterator& it,
                        double time, const FaceDomainType& x,
                        const ArgumentTuple& uLeft,
                        RangeType& gLeft)
    {
      const DomainType normal = it.integrationOuterNormal(x);

      typedef typename ArgumentTuple::template Get<passId1>::Type UType;

      double ldt=0.0;
      if (model_.hasBoundaryValue(it,time,x)) 
      {
        /*****************************
         * Advection                 *
         ****************************/
        UType uRight;
	RangeType gRight;

        // get the boundary value for upwind discretization
	model_.boundaryValue(it, time, x, uLeft[u0Var], uRight);
	ldt = numflux_.numericalFlux(it, time, x, uLeft[u0Var], uRight, gLeft, gRight);
        
        /*****************************
         * Diffusion                 *
         ****************************/
	JacobianRangeType diffmatrix;
	model_.diffusion2(*it.inside(), time,
                          it.intersectionSelfLocal().global(x),
                          uLeft[u0Var], uLeft[u1Var], diffmatrix);
	diffmatrix.umv(normal, gLeft);
      } else { 
        // not implemented
        assert(false);
      }
      return ldt;
    }

    /**
     * @brief method required by LocalDGPass
     */
    template <class ArgumentTuple>
    void analyticalFlux(EntityType& en,
                        double time, const DomainType& x,
                        const ArgumentTuple& u, JacobianRangeType& f)
    { 
      /*****************************
       * Advection                 *
       ****************************/
      model_.advection(en,time, x, u[u0Var],f);
      /*****************************
       * Diffusion                 *
       ****************************/
      JacobianRangeType diffmatrix;
      model_.diffusion2(en, time, x, u[u0Var], u[u1Var], diffmatrix);
      f += diffmatrix;
    }
  private:
    const Model& model_;
    const NumFlux& numflux_;
  };


}

#endif
