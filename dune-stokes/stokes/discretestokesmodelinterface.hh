/**
 *  \file   discretestokesmodelinterface.hh
 *
 *  \brief  containing a class DiscreteStokesModelInterface
 **/
#ifndef DUNE_DISCRESTOKESTEMODELINTERFACE_HH
#define DUNE_DISCRESTOKESTEMODELINTERFACE_HH

#include <dune/common/bartonnackmanifcheck.hh>
#include <dune/common/fvector.hh>

namespace Dune
{

/**
 *  \brief  Interface class for stokes problem definition in the LDG context.
 *  \todo   doc with tex
 **/
template < class DiscreteStokesModelTraits >
class DiscreteStokesModelInterface
{
    public:

        typedef DiscreteStokesModelInterface< DiscreteStokesModelTraits >
            ThisType;

        //! Traits class defined by the user
        typedef DiscreteStokesModelTraits
            Traits;

        //! Implementation type for Barton-Nackman trick
        typedef typename Traits::DiscreteModelType
            DiscreteModelType;

        //! volume quadrature type used in pass
        typedef typename Traits::VolumeQuadratureType
            VolumeQuadratureType;

        //! face quadrature type used in pass
        typedef typename Traits::FaceQuadratureType
            FaceQuadratureType;

        //! Velocity function space
        typedef typename Traits::DiscreteVelocityFunctionSpaceType
            DiscreteVelocityFunctionSpaceType;

        //! Sigma function space
        typedef typename Traits::DiscreteSigmaFunctionSpaceType
            DiscreteSigmaFunctionSpaceType;

        //! Pressure function space
        typedef typename Traits::DiscretePressureFunctionSpaceType
            DiscretePressureFunctionSpaceType;

        //! Coordinate type (world coordinates)
        typedef typename DiscreteVelocityFunctionSpaceType::DomainType
            DomainType;

        //! Vector type of the velocity's discrete function space's range
        typedef typename DiscreteVelocityFunctionSpaceType::RangeType
            VelocityRangeType;

        //! vector type of sigmas' discrete functions space's range
        typedef typename DiscreteSigmaFunctionSpaceType::RangeType
            SigmaRangeType;

        //! Vector type of the pressure's discrete function space's range
        typedef typename DiscretePressureFunctionSpaceType::RangeType
            PressureRangeType;

        //! Type of GridPart
        typedef typename DiscreteVelocityFunctionSpaceType::GridPartType
            GridPartType;

        //! Type of the grid
        typedef typename GridPartType::GridType
            GridType;

        //! Intersection iterator of the grid
        typedef typename GridPartType::IntersectionIteratorType
            IntersectionIteratorType;

        //! Element (codim 0 entity) of the grid
        typedef typename GridType::template Codim<0>::Entity
            EntityType;

        /**
         *  \brief  constructor
         *
         *  doing nothing
         **/
        DiscreteStokesModelInterface()
        {
        }

        /**
         *  \brief  destructor
         *
         *  doing nothing
         **/
        ~DiscreteStokesModelInterface()
        {
        }

        /**
         *  \brief  Returns true if problem has a flux contribution of type
         *          \f$\hat{u}_{\sigma}\f$
         *  \attention  If you let this method return true, make sure to
         *              implement <b>both</b> velocitySigmaFlux and
         *              velocitySigmaBoundaryFlux as well
         **/
        bool hasVelocitySigmaFlux() const
        {
            return asImp().hasVelocitySigmaFlux();
        }

        /**
         *  \brief  Returns true if problem has a flux contribution of type
         *          \f$\hat{u}_{p}\f$
         *  \attention  If you let this method return true, make sure to
         *              implement <b>both</b> velocityPressureFlux and
         *              velocityPressureBoundaryFlux as well.
         **/
        bool hasVelocityPressureFlux() const
        {
            return asImp().hasVelocityPressureFlux();
        }

        /**
         *  \brief  Returns true if problem has a flux contribution of type
         *          \f$\hat{p}\f$
         *  \attention  If you let this method return true, make sure to
         *              implement <b>both</b> pressureFlux and
         *              pressureBoundaryFlux as well.
         **/
        bool hasPressureFlux() const
        {
            return asImp().hasPressureFlux();
        }

        /**
         *  \brief  Returns true if problem has a flux contribution of type
         *          \f$\hat{\sigma}\f$
         *  \attention  If you let this method return true, make sure to
         *              implement <b>both</b> sigmaFlux and
         *              sigmaBoundaryFlux as well.
         **/
        bool hasSigmaFlux() const
        {
            return asImp().hasSigmaFlux;
        }

        /**
         *  \brief  Returns true if problem has a force contribution \f$f\f$
         *  \attention  If you let this method return true, make sure to
         *              implement force as well.
         **/
        bool hasForce() const
        {
            return asImp().hasForce();
        }

        /**
         *  \brief
         *  \tparam FaceDomainType
         *          domain type on given face
         *  \tparam LocalVelocityFunctionType
         *          type of local function (of type velocity)
         *  \param  it
         *          faceiterator
         *  \param  time
         *          global time
         *  \param  x
         *          point to evaluate at
         *  \param  uInner
         *          local function (of type velocity) on given entity
         *  \param  uOuter
         *          local function (of type velocity) on neighbour of
         *          given entity
         *  \todo   latex doc
         **/
        template < class FaceDomainType >
        void velocitySigmaFlux(         const IntersectionIteratorType& it,
                                        const double time,
                                        const FaceDomainType& x,
                                        const VelocityRangeType& uInner,
                                        const VelocityRangeType& uOuter,
                                        VelocityRangeType& uContribInner,
                                        VelocityRangeType& uContribOuter,
                                        VelocityRangeType& emptyContribInner,
                                        VelocityRangeType& emptyContribOuter )
        {
            CHECK_AND_CALL_INTERFACE_IMPLEMENTATION(
                asImp().velocitySigmaFlux(  it,
                                            time,
                                            x,
                                            uInner,
                                            uOuter,
                                            uContribInner,
                                            uContribOuter,
                                            emptyContribInner,
                                            emptyContribOuter ) );
        }

        /**
         *  \brief
         *  \todo   doc like velocitySigmaFlux
         **/
        template < class FaceDomainType >
        void velocitySigmaBoundaryFlux( const IntersectionIteratorType& it,
                                        const double time,
                                        const FaceDomainType& x,
                                        const VelocityRangeType& uInner,
                                        const VelocityRangeType& uOuter,
                                        VelocityRangeType& uContribInner,
                                        VelocityRangeType& uContribOuter,
                                        VelocityRangeType& emptyContribInner,
                                        VelocityRangeType& emptyContribOuter )
        {
            CHECK_AND_CALL_INTERFACE_IMPLEMENTATION(
                asImp().velocitySigmaBoundaryFlux(
                                        it,
                                        time,
                                        x,
                                        uInner,
                                        uOuter,
                                        uContribInner,
                                        uContribOuter,
                                        emptyContribInner,
                                        emptyContribOuter ) );
        }

        /**
         *  \brief
         *  \todo   latex doc
         **/
        template < class FaceDomainType >
        void velocityPressureFlux(      const IntersectionIteratorType& it,
                                        const double time,
                                        const FaceDomainType& x,
                                        const VelocityRangeType& uInner,
                                        const VelocityRangeType& uOuter,
                                        const PressureRangeType& pInner,
                                        const PressureRangeType& pOuter,
                                        VelocityRangeType& uContribInner,
                                        VelocityRangeType& uContribOuter,
                                        VelocityRangeType& pContribInner,
                                        VelocityRangeType& pContribOuter,
                                        VelocityRangeType& emptyContribInner,
                                        VelocityRangeType& emptyContribOuter )
        {
            CHECK_AND_CALL_INTERFACE_IMPLEMENTATION(
                asImp().velocityPressureFlux(   it,
                                                time,
                                                x,
                                                uInner,
                                                uOuter,
                                                pInner,
                                                pOuter,
                                                uContribInner,
                                                uContribOuter,
                                                pContribInner,
                                                pContribOuter,
                                                emptyContribInner,
                                                emptyContribOuter) );
        }

        /**
         *  \brief
         *  \todo   doc like velocityPressureFlux
         **/
        template < class FaceDomainType >
        void velocityPressureBoundaryFlux(
                                    const IntersectionIteratorType& it,
                                    const double time,
                                    const FaceDomainType& x,
                                    const VelocityRangeType& uInner,
                                    const VelocityRangeType& uOuter,
                                    const PressureRangeType& pInner,
                                    const PressureRangeType& pOuter,
                                    VelocityRangeType& uContribInner,
                                    VelocityRangeType& uContribOuter,
                                    VelocityRangeType& pContribInner,
                                    VelocityRangeType& pContribOuter,
                                    VelocityRangeType& emptyContribInner,
                                    VelocityRangeType& emptyContribOuter )
        {
            CHECK_AND_CALL_INTERFACE_IMPLEMENTATION(
                asImp().velocityPressureBoundaryFlux(
                                    it,
                                    time,
                                    x,
                                    uInner,
                                    uOuter,
                                    pInner,
                                    pOuter,
                                    uContribInner,
                                    uContribOuter,
                                    pContribInner,
                                    pContribOuter,
                                    emptyContribInner,
                                    emptyContribOuter ) );
        }

        /**
         *  \brief
         *  \todo latex doc
         **/
        template < class FaceDomainType >
        void pressureFlux(  const IntersectionIteratorType& it,
                            const double time,
                            const FaceDomainType& x,
                            const PressureRangeType& pInner,
                            const PressureRangeType& pOuter,
                            PressureRangeType& pContribInner,
                            PressureRangeType& pContribOuter,
                            PressureRangeType& emptyContribInner,
                            PressureRangeType& emptyContribOuter )
        {
            CHECK_AND_CALL_INTERFACE_IMPLEMENTATION(
                asImp().pressureFlux(   it,
                                        time,
                                        x,
                                        pInner,
                                        pOuter,
                                        pContribInner,
                                        pContribOuter,
                                        emptyContribInner,
                                        emptyContribOuter ) );
        }

        /**
         *  \brief
         *  \todo   doc like pressureFlux
         **/
        template < class FaceDomainType >
        void pressureBoundaryFlux(  const IntersectionIteratorType& it,
                                    const double time,
                                    const FaceDomainType& x,
                                    const PressureRangeType& pInner,
                                    const PressureRangeType& pOuter,
                                    PressureRangeType& pContribInner,
                                    PressureRangeType& pContribOuter,
                                    PressureRangeType& emptyContribInner,
                                    PressureRangeType& emptyContribOuter )
        {
            CHECK_AND_CALL_INTERFACE_IMPLEMENTATION(
                asImp().pressureBoundaryFlux(   it,
                                                time,
                                                x,
                                                pInner,
                                                pOuter,
                                                pContribInner,
                                                pContribOuter,
                                                emptyContribInner,
                                                emptyContribOuter) );
        }

        /**
         *  \brief
         *  \todo   latex doc
         **/
        template < class FaceDomainType >
        void sigmaFlux( const IntersectionIteratorType& it,
                        const double time,
                        const FaceDomainType& x,
                        const VelocityRangeType& uInner,
                        const VelocityRangeType& uOuter,
                        const SigmaRangeType& sigmaInner,
                        const SigmaRangeType& sigmaOuter,
                        SigmaRangeType& sigmaContribInner,
                        SigmaRangeType& sigmaContribOuter,
                        SigmaRangeType& uContribInner,
                        SigmaRangeType& uContribOuter,
                        SigmaRangeType& emptyContribInner,
                        SigmaRangeType& emptyContribOuter )
        {
            CHECK_AND_CALL_INTERFACE_IMPLEMENTATION(
                asImp().sigmaFlux(  it,
                                    time,
                                    x,
                                    uInner,
                                    uOuter,
                                    sigmaInner,
                                    sigmaOuter,
                                    sigmaContribInner,
                                    sigmaContribOuter,
                                    uContribInner,
                                    uContribOuter,
                                    emptyContribInner,
                                    emptyContribOuter ) );
        }

        /**
         *  \brief
         *  \todo   doc like sigmaFlux
         **/
        template < class FaceDomainType >
        void sigmaBoundaryFlux( const IntersectionIteratorType& it,
                                const double time,
                                const FaceDomainType& x,
                                const VelocityRangeType& uInner,
                                const VelocityRangeType& uOuter,
                                const SigmaRangeType& sigmaInner,
                                const SigmaRangeType& sigmaOuter,
                                SigmaRangeType& sigmaContribInner,
                                SigmaRangeType& sigmaContribOuter,
                                SigmaRangeType& uContribInner,
                                SigmaRangeType& uContribOuter,
                                SigmaRangeType& emptyContribInner,
                                SigmaRangeType& emptyContribOuter)
        {
            CHECK_AND_CALL_INTERFACE_IMPLEMENTATION(
                asImp().sigmaBoundaryFlux(  it,
                                            time,
                                            x,
                                            uInner,
                                            uOuter,
                                            sigmaInner,
                                            sigmaOuter,
                                            sigmaContribInner,
                                            sigmaContribOuter,
                                            uContribInner,
                                            uContribOuter,
                                            emptyContribInner,
                                            emptyContribOuter) );
        }

        /**
         *  \brief
         *  \todo   latex doc
         **/
        template < class FaceDomainType >
        void force( const IntersectionIteratorType& it,
                    const double time,
                    const FaceDomainType& x,
                    VelocityRangeType& forceContribInner,
                    VelocityRangeType& forceContribOuter )
        {
            CHECK_AND_CALL_INTERFACE_IMPLEMENTATION(
                asImp().force(
                ) );
        }

    protected:
        //! for Barton-Nackmann trick
        DiscreteModelType& asImp()
        {
            return static_cast<DiscreteModelType&>(*this);
        }

        //! for Barton-Nackmann trick
        const DiscreteModelType& asImp() const
        {
            return static_cast<const DiscreteModelType&>(*this);
        }

};

// forward declaration
template < class DiscreteStokesModelDefaultTraits >
class DiscreteStokesModelDefault;

/**
 *  \brief  traits class for DiscreteStokesModelDefault
 *  \todo   doc
 **/
template < class GridPartImp, int gridDim, int polOrder >
class DiscreteStokesModelDefaultTraits
{
    public:
        //! for Barton-Nackmann trick
        typedef DiscreteStokesModelDefault < DiscreteStokesModelDefaultTraits >
            DiscreteModelType;

        //! we use caching quadratures for the entities
        typedef Dune::CachingQuadrature< GridPartImp, 0 >
            VolumeQuadratureType;

        //! and for the faces
        typedef Dune::CachingQuadrature< GridPartImp, 1 >
            FaceQuadratureType;

        //! function space type for the velocity
        typedef Dune::FunctionSpace< double, double, gridDim, gridDim >
            VelocityFunctionSpaceType;

        //! discrete function type space for the velocity
        typedef Dune::DiscontinuousGalerkinSpace<   VelocityFunctionSpaceType,
                                                    GridPartImp,
                                                    polOrder >
            DiscreteVelocityFunctionSpaceType;

        //! discrete function type for the velocity
        typedef Dune::AdaptiveDiscreteFunction< DiscreteVelocityFunctionSpaceType >
            DiscreteVelocityFunctionType;

        //! function space type for sigma
        typedef Dune::MatrixFunctionSpace<  double,
                                            double,
                                            gridDim,
                                            gridDim,
                                            gridDim >
            SigmaFunctionSpaceType;

        //! discrete function space type for sigma
        typedef Dune::DiscontinuousGalerkinSpace<   SigmaFunctionSpaceType,
                                                    GridPartImp,
                                                    polOrder >
            DiscreteSigmaFunctionSpaceType;

        //! discrete function type for sigma
        typedef Dune::AdaptiveDiscreteFunction< DiscreteSigmaFunctionSpaceType >
            DiscreteSigmaFunctionType;

      //! function space type for the pressure
        typedef Dune::FunctionSpace< double, double, gridDim, 1 >
            PressureFunctionSpaceType;

        //! discrete function space type for the pressure
        typedef Dune::DiscontinuousGalerkinSpace<   PressureFunctionSpaceType,
                                                    GridPartImp,
                                                    polOrder >
            DiscretePressureFunctionSpaceType;

        //! discrete function type for the pressure
        typedef Dune::AdaptiveDiscreteFunction< DiscretePressureFunctionSpaceType >
            DiscretePressureFunctionType;

};


/**
 *  \brief  definition of an ldg method for a stokes problem
 *  \todo   texdoc
 **/
template < class DiscreteStokesModelDefaultTraitsImp >
class DiscreteStokesModelDefault : public DiscreteStokesModelInterface< DiscreteStokesModelDefaultTraitsImp >
{
    public:

        typedef DiscreteStokesModelInterface< DiscreteStokesModelDefaultTraitsImp >
            BaseType;

        typedef typename BaseType::IntersectionIteratorType
            IntersectionIteratorType;

        typedef typename BaseType::VelocityRangeType
            VelocityRangeType;

        typedef typename BaseType::SigmaRangeType
            SigmaRangeType;

        typedef typename BaseType::PressureRangeType
            PressureRangeType;

        /**
         *  \brief  constructor
         *
         *  set \f$C_{11}\in R\f$, \f$C_{12}\in R\f$, \f$D_{11}\in R^{d}\f$,
         *  \f$D_{12}\in R^{d}\f$
         **/
        DiscreteStokesModelDefault()
            : C_12_( 1.0 ),
            D_12_( 1.0 )
        {
            C_11_ = 1.0;
            D_11_ = 1.0;


        }

        /**
         *  \brief  destructor
         *
         *  doing nothing
         **/
        ~DiscreteStokesModelDefault()
        {
        }

        /**
         *  \brief
         *  \todo   doc me
         **/
        bool hasVelocitySigmaFlux() const
        {
            return true;
        }

        /**
         *  \brief
         *  \todo   doc me
         **/
        bool hasVelocityPressureFlux() const
        {
            return true;
        }

        /**
         *  \brief
         *  \todo   doc me
         **/
        bool hasPressureFlux() const
        {
            return true;
        }

        /**
         *  \brief
         *  \todo   doc me
         **/
        bool hasSigmaFlux() const
        {
            return true;
        }

        /**
         *  \brief
         *  \todo   doc me
         **/
        bool hasForce() const
        {
            return true;
        }

        /**
         *  \brief
         *  \todo   doc me
         **/
        template < class FaceDomainType >
        void velocitySigmaFlux(         const IntersectionIteratorType& it,
                                        const double time,
                                        const FaceDomainType& x,
                                        const VelocityRangeType& uInner,
                                        const VelocityRangeType& uOuter,
                                        VelocityRangeType& uContribInner,
                                        VelocityRangeType& uContribOuter,
                                        VelocityRangeType& emptyContribInner,
                                        VelocityRangeType& emptyContribOuter )
        {
            VelocityRangeType outerNormal = it->unitOuterNormal( x );

        }

        /**
         *  \brief
         *  \todo   doc like velocitySigmaFlux
         **/
        void velocitySigmaBoundaryFlux( const IntersectionIteratorType& it,
                                        const double time,
                                        const VelocityRangeType& uInner,
                                        const VelocityRangeType& uOuter,
                                        VelocityRangeType& uContribInner,
                                        VelocityRangeType& uContribOuter,
                                        VelocityRangeType& emptyContribInner,
                                        VelocityRangeType& emptyContribOuter )
        {
        }

        /**
         *  \brief
         *  \todo   doc me
         **/
        void velocityPressureFlux(      const IntersectionIteratorType& it,
                                        const double time,
                                        const VelocityRangeType& uInner,
                                        const VelocityRangeType& uOuter,
                                        const PressureRangeType& pInner,
                                        const PressureRangeType& pOuter,
                                        VelocityRangeType& uContribInner,
                                        VelocityRangeType& uContribOuter,
                                        VelocityRangeType& pContribInner,
                                        VelocityRangeType& pContribOuter,
                                        VelocityRangeType& emptyContribInner,
                                        VelocityRangeType& emptyContribOuter )
        {
        }

        /**
         *  \brief
         *  \todo   doc like velocityPressureFlux
         **/
        void velocityPressureBoundaryFlux(
                                    const IntersectionIteratorType& it,
                                    const double time,
                                    const VelocityRangeType& uInner,
                                    const VelocityRangeType& uOuter,
                                    const PressureRangeType& pInner,
                                    const PressureRangeType& pOuter,
                                    VelocityRangeType& uContribInner,
                                    VelocityRangeType& uContribOuter,
                                    VelocityRangeType& pContribInner,
                                    VelocityRangeType& pContribOuter,
                                    VelocityRangeType& emptyContribInner,
                                    VelocityRangeType& emptyContribOuter )
        {
        }

        /**
         *  \brief
         *  \todo latex doc
         **/
        void pressureFlux(  const IntersectionIteratorType& it,
                            const double time,
                            const PressureRangeType& pInner,
                            const PressureRangeType& pOuter,
                            PressureRangeType& pContribInner,
                            PressureRangeType& pContribOuter,
                            PressureRangeType& emptyContribInner,
                            PressureRangeType& emptyContribOuter )
        {
        }

        /**
         *  \brief
         *  \todo   doc like pressureFlux
         **/
        void pressureBoundaryFlux(  const IntersectionIteratorType& it,
                                    const double time,
                                    const PressureRangeType& pInner,
                                    const PressureRangeType& pOuter,
                                    PressureRangeType& pContribInner,
                                    PressureRangeType& pContribOuter,
                                    PressureRangeType& emptyContribInner,
                                    PressureRangeType& emptyContribOuter )
        {
        }

        /**
         *  \brief
         *  \todo   latex doc
         **/
        void sigmaFlux( const IntersectionIteratorType& it,
                        const double time,
                        const VelocityRangeType& uInner,
                        const VelocityRangeType& uOuter,
                        const SigmaRangeType& sigmaInner,
                        const SigmaRangeType& sigmaOuter,
                        SigmaRangeType& sigmaContribInner,
                        SigmaRangeType& sigmaContribOuter,
                        SigmaRangeType& uContribInner,
                        SigmaRangeType& uContribOuter,
                        SigmaRangeType& emptyContribInner,
                        SigmaRangeType& emptyContribOuter )
        {
        }

        /**
         *  \brief
         *  \todo   doc like sigmaFlux
         **/
        void sigmaBoundaryFlux( const IntersectionIteratorType& it,
                                const double time,
                                const VelocityRangeType& uInner,
                                const VelocityRangeType& uOuter,
                                const SigmaRangeType& sigmaInner,
                                const SigmaRangeType& sigmaOuter,
                                SigmaRangeType& sigmaContribInner,
                                SigmaRangeType& sigmaContribOuter,
                                SigmaRangeType& uContribInner,
                                SigmaRangeType& uContribOuter,
                                SigmaRangeType& emptyContribInner,
                                SigmaRangeType& emptyContribOuter)
        {
        }

        /**
         *  \brief
         *  \todo   latex doc
         **/
        template < class FaceDomainType >
        void force( const IntersectionIteratorType& it,
                    const double time,
                    const FaceDomainType& x,
                    VelocityRangeType& forceContribInner,
                    VelocityRangeType& forceContribOuter )
        {
        }

    private:

        double C_11_, D_11_;
        VelocityRangeType C_12_, D_12_;

        /**
         *  \brief  jump for pressure-type functions
         *
         *  \f$\left[\left[\p\right]\right]:=\left(p^{+} + p^{-}\right)n^{+}\in R^{d}\f$,
         *  where \f$n^{+}\f4 is the unit outer normal,
         *  \f$p^{+}\f$ is the value of p on the inside and
         *  \f$p^{-}\f$ the value of p at the outside
         **/
        VelocityRangeType pTypeJump(    const PressureRangeType& pInner,
                                        const PressureRangeType& pOuter,
                                        const VelocityRangeType& outerNormal )
        {
            VelocityRangeType ret( 0.0 );
            ret += outerNormal;
            ret *= ( pInner - pOuter );
            return ret;
        }

        /**
         *  \brief  jump for velocity-type functions
         *  \todo doc like pTypeJump
         **/
        double uTypeJump(   const VelocityRangeType& uInner,
                            const VelocityRangeType& uOuter,
                            const VelocityRangeType& outerNormal )
        {
            return ( uInner - uOuter ) * outerNormal;
        }

        /**
         *  \brief  matrix valued jump for velocity-type functions
         *  \todo   doc like pTypeJump
         **/
        SigmaRangeType uTypeMatrixJump( const VelocityRangeType& uInner,
                                        const VelocityRangeType& uOuter,
                                        const VelocityRangeType& outerNormal )
        {
            SigmaRangeType ret( 0.0 );
            VelocityRangeType uDiff = uInner - uOuter;
            typedef typename SigmaRangeType::RowIterator
                MatrixRowIteratorType;
            typedef typename VelocityRangeType::Iterator
                VectorIteratorType;
            MatrixRowIteratorType rItEnd = ret.end();
            VectorIteratorType uDiffIt = uDiff.begin();
            for ( MatrixRowIteratorType rIt = ret.begin(); rIt != rItEnd; ++rIt ) {
                VectorIteratorType outerNormalIt = outerNormal.begin();
                VectorIteratorType vItEnd = rIt->end();
                for (   VectorIteratorType vIt = rIt->begin();
                        vIt != vItEnd;
                        ++vIt ) {
                    *vIt = *uDiffIt * *outerNormalIt;
                    ++outerNormalIt;
                }
                ++uDiffIt;
            }

        }

        /**
         *  \brief  mean value of two functions (of same type)
         *  \todo   texdoc example
         **/
        template < class DiscreteFunctionImp >
        DiscreteFunctionImp meanValue( const DiscreteFunctionImp& funcInner,
                                    const DiscreteFunctionImp& funcOuter )
        {
            DiscreteFunctionImp ret( 0.0 );
            ret += funcInner;
            ret += funcOuter;
            ret *= 0.5;
            return ret;
        }


};

}; // end of namespace Dune

#endif // end of discretestokesmodelinterface.hh
