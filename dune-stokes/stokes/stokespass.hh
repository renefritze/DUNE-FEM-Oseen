/** \file stokespass.hh
    \brief  stokespass.hh
 **/

#ifndef STOKESPASS_HH
#define STOKESPASS_HH

#include <dune/fem/pass/pass.hh>
#include <dune/fem/operator/matrix/spmatrix.hh>
#include <dune/fem/space/dgspace.hh>
#include <dune/fem/quadrature/caching/twistutility.hh>
#include <dune/fem/misc/l2norm.hh>

#include <dune/stokes/saddlepoint_inverse_operator.hh>


#ifndef NLOG // if we want logging, should be removed in the end
    #include <dune/stuff/printing.hh>
    #include <dune/stuff/misc.hh>
    #include <dune/stuff/logging.hh>
#endif

#include <dune/stuff/grid.hh>
#include <dune/stuff/functions.hh>

#include <dune/stuff/profiler.hh>

#ifdef CHEAT
    #if (POLORDER > 0)
        #warning "gradient reconstruction disabled because polorder > 0"
        #undef CHEAT
    #else
        #include "discretegradientpass.hh"
    #endif
#endif

namespace Dune
{

/**
 *  \brief  StokesPass
 *
 *             more
 *  \todo   doc
 **/
template <  class DiscreteModelImp,
            class PreviousPassImp,
            int PassID = 0 >
class StokesPass
    : public Pass < DiscreteModelImp, PreviousPassImp, PassID >
{
    public:
        //! own type
        typedef StokesPass< DiscreteModelImp, PreviousPassImp, PassID >
            ThisType;

        //! base type
        typedef Pass < DiscreteModelImp, PreviousPassImp, PassID >
            BaseType;

        //! previous pass type
        typedef PreviousPassImp
            PreviousPassType;

        //! discrete model type
        typedef DiscreteModelImp
            DiscreteModelType;

        //! volume quadrature type
        typedef typename DiscreteModelType::VolumeQuadratureType
            VolumeQuadratureType;

        //! face quadrature type
        typedef typename DiscreteModelType::FaceQuadratureType
            FaceQuadratureType;

        //! type of discrete function space wrapper
        typedef typename DiscreteModelType::DiscreteStokesFunctionSpaceWrapperType
            DiscreteStokesFunctionSpaceWrapperType;

        //! discrete function wrapper type
        typedef typename DiscreteModelType::DiscreteStokesFunctionWrapperType
            DiscreteStokesFunctionWrapperType;

        //! discrete function type for the velocity
        typedef typename DiscreteStokesFunctionWrapperType::DiscreteVelocityFunctionType
            DiscreteVelocityFunctionType;

        //! discrete function space type for the velocity
        typedef typename DiscreteVelocityFunctionType::DiscreteFunctionSpaceType
            DiscreteVelocityFunctionSpaceType;

        //! discrete function type for sigma
        typedef typename DiscreteModelType::DiscreteSigmaFunctionType
            DiscreteSigmaFunctionType;

        //! discrete function space type for sigma
        typedef typename DiscreteSigmaFunctionType::DiscreteFunctionSpaceType
            DiscreteSigmaFunctionSpaceType;

        //! discrete fucntion type for the pressure
        typedef typename DiscreteStokesFunctionWrapperType::DiscretePressureFunctionType
            DiscretePressureFunctionType;

        //! discrete function space type for the pressure
        typedef typename DiscretePressureFunctionType::DiscreteFunctionSpaceType
            DiscretePressureFunctionSpaceType;

        //! Coordinate type on the element
        typedef typename DiscreteVelocityFunctionSpaceType::DomainType
            ElementCoordinateType;

        //! Coordinate type on an intersection
        typedef typename FaceQuadratureType::LocalCoordinateType
            IntersectionCoordinateType;

        //! Vector type of the velocity's discrete function space's range
        typedef typename DiscreteVelocityFunctionSpaceType::RangeType
            VelocityRangeType;

        typedef typename DiscreteVelocityFunctionSpaceType::BaseFunctionSetType::JacobianRangeType
            VelocityJacobianRangeType;

        //! vector type of sigmas' discrete functions space's range
        typedef typename DiscreteSigmaFunctionSpaceType::RangeType
            SigmaRangeType;

        typedef typename DiscreteSigmaFunctionSpaceType::BaseFunctionSetType::JacobianRangeType
            SigmaJacobianRangeType;

        //! Vector type of the pressure's discrete function space's range
        typedef typename DiscretePressureFunctionSpaceType::RangeType
            PressureRangeType;

        typedef typename DiscretePressureFunctionSpaceType::BaseFunctionSetType::JacobianRangeType
            PressureJacobianRangeType;

        //! Type of GridPart
        typedef typename DiscreteVelocityFunctionSpaceType::GridPartType
            GridPartType;

        //! Intersection iterator of the gridpart
        typedef typename GridPartType::IntersectionIteratorType
            IntersectionIteratorType;

        //! local coordinate type on an intersection
        typedef typename FaceQuadratureType::LocalCoordinateType
            LocalIntersectionCoordinateType;

        //! entity iterator of the gridpart
        typedef typename GridPartType::template Codim< 0 >::IteratorType
            EntityIteratorType;

        //! type of the grid
        typedef typename GridPartType::GridType
            GridType;

        //! type of codim 0 entity
        typedef typename GridType::template Codim< 0 >::Entity
            EntityType;

        //! type of the used solver
        typedef NestedCgSaddlepointInverseOperator< ThisType >
            InvOpType;

#ifdef USE_ALTERNATIVE_SOLVER
        typedef SaddlepointInverseOperator< ThisType >
            AltInvOpType;
#endif

        //! polynomial order for the discrete sigma function space
        static const int sigmaSpaceOrder
            = DiscreteModelType::sigmaSpaceOrder;
        //! polynomial order for the discrete velocity function space
        static const int velocitySpaceOrder
            = DiscreteModelType::velocitySpaceOrder;
        //! polynomial order for the discrete pressure function space
        static const int pressureSpaceOrder
            = DiscreteModelType::pressureSpaceOrder;


        /**
         *  \name typedefs needed for interface compliance
         *  \{
         **/
        typedef typename BaseType::DestinationType
            DestinationType;

        typedef typename BaseType::DomainType
            DomainType;

        typedef typename BaseType::RangeType
            RangeType;

        typedef typename BaseType::TotalArgumentType
            TotalArgumentType;
        /**
         *  \}
         **/

        /**
         *  \brief  constructor
         *  \todo   doc
         **/
        StokesPass( PreviousPassType& prevPass,
                    DiscreteModelType& discreteModel,
                    GridPartType& gridPart,
                    DiscreteStokesFunctionSpaceWrapperType& spaceWrapper )
            : BaseType( prevPass ),
            discreteModel_( discreteModel ),
            gridPart_( gridPart ),
            spaceWrapper_( spaceWrapper ),
            velocitySpace_( spaceWrapper.discreteVelocitySpace() ),
            pressureSpace_( spaceWrapper.discretePressureSpace() ),
            sigmaSpace_( gridPart )
        {}

        /**
         *  \brief  empty constructor
         **/
        StokesPass()
        {}

        //! used in Postprocessing to get refs to gridparts, spaces
        const DiscreteStokesFunctionSpaceWrapperType& GetFunctionSpaceWrapper() const
        {
            return spaceWrapper_;
        }

        /**
         *  \todo doc
         *  \attention  think about quadrature orders
         **/
        virtual void apply( const DomainType &arg, RangeType &dest) const
        {

            // entity and geometry types
            typedef typename EntityType::Geometry
                EntityGeometryType;
            typedef typename Dune::FieldMatrix< typename EntityGeometryType::ctype,
                                                EntityGeometryType::coorddimension,
                                                EntityGeometryType::mydimension >
                JacobianInverseTransposedType;
//
//            profiler().StartTiming("Pass");
//            profiler().StartTiming("Pass -- ASSEMBLE");
            // viscosity
            const double mu = discreteModel_.viscosity();

            // matrices
            // M\in R^{M\times M}
            typedef SparseRowMatrixObject<  DiscreteSigmaFunctionSpaceType,
                                            DiscreteSigmaFunctionSpaceType >
                MInversMatrixType;
            MInversMatrixType MInversMatrix( sigmaSpace_, sigmaSpace_ );
            MInversMatrix.reserve();
////MInversMatrixType MMatrix( sigmaSpace_, sigmaSpace_ );
////MMatrix.reserve();
            // W\in R^{M\times L}
            typedef SparseRowMatrixObject<  DiscreteSigmaFunctionSpaceType,
                                            DiscreteVelocityFunctionSpaceType >
                WmatrixType;
            WmatrixType Wmatrix( sigmaSpace_, velocitySpace_ );
            Wmatrix.reserve();
            // X\in R^{L\times M}
            typedef SparseRowMatrixObject<  DiscreteVelocityFunctionSpaceType,
                                            DiscreteSigmaFunctionSpaceType >
                XmatrixType;
            XmatrixType Xmatrix( velocitySpace_, sigmaSpace_ );
            Xmatrix.reserve();
            // Y\in R^{L\times L}
            typedef SparseRowMatrixObject<  DiscreteVelocityFunctionSpaceType,
                                            DiscreteVelocityFunctionSpaceType >
                YmatrixType;
            YmatrixType Ymatrix( velocitySpace_, velocitySpace_ );
            Ymatrix.reserve();
            // Z\in R^{L\times K}
            typedef SparseRowMatrixObject<  DiscreteVelocityFunctionSpaceType,
                                            DiscretePressureFunctionSpaceType >
                ZmatrixType;
            ZmatrixType Zmatrix( velocitySpace_, pressureSpace_ );
            Zmatrix.reserve();
            // E\in R^{K\times L}
            typedef SparseRowMatrixObject<  DiscretePressureFunctionSpaceType,
                                            DiscreteVelocityFunctionSpaceType >
                EmatrixType;
            EmatrixType Ematrix( pressureSpace_, velocitySpace_ );
            Ematrix.reserve();
            // R\in R^{K\times K}
            typedef SparseRowMatrixObject<  DiscretePressureFunctionSpaceType,
                                            DiscretePressureFunctionSpaceType >
                RmatrixType;
            RmatrixType Rmatrix( pressureSpace_, pressureSpace_ );
            Rmatrix.reserve();

            // local matrices
            // M\in R^{M\times M}
            typedef typename MInversMatrixType::LocalMatrixType
                LocalMInversMatrixType;
            // W\in R^{M\times L}
            typedef typename WmatrixType::LocalMatrixType
                LocalWmatrixType;
            // X\in R^{L\times M}
            typedef typename XmatrixType::LocalMatrixType
                LocalXmatrixType;
            // Y\in R^{L\times L}
            typedef typename YmatrixType::LocalMatrixType
                LocalYmatrixType;
            // Z\in R^{L\times K}
            typedef typename ZmatrixType::LocalMatrixType
                LocalZmatrixType;
            // E\in R^{K\times L}
            typedef typename EmatrixType::LocalMatrixType
                LocalEmatrixType;
            // R\in R^{K\times K}
            typedef typename RmatrixType::LocalMatrixType
                LocalRmatrixType;

            // right hand sides
            // H_{1}\in R^{M}
            DiscreteSigmaFunctionType H1rhs( "H1", sigmaSpace_ );
            H1rhs.clear();

            // H_{2}\in R^{L}
            DiscreteVelocityFunctionType H2rhs( "H2", velocitySpace_ );
            H2rhs.clear();

            // H_{3}\in R^{K}
            DiscretePressureFunctionType H3rhs( "H3", pressureSpace_ );
            H3rhs.clear();
//
////#ifdef CHEAT
////            DiscreteSigmaFunctionType exactH1rhs( "exact H1", sigmaSpace_ );
////            DiscreteSigmaFunctionType tmpH1rhs( "tmp H1", sigmaSpace_ );
////            exactH1rhs.clear();
////            tmpH1rhs.clear();
////
////            DiscreteVelocityFunctionType exactH2rhs( "exact H2", velocitySpace_ );
////            DiscreteVelocityFunctionType tmpH2rhs( "tmp H2", velocitySpace_ );
////            exactH2rhs.clear();
////            tmpH2rhs.clear();
////
////            DiscretePressureFunctionType exactH3rhs( "exact H3", pressureSpace_ );
////            DiscretePressureFunctionType tmpH3rhs( "tmp H3", pressureSpace_ );
////            exactH3rhs.clear();
////            tmpH3rhs.clear();
////#endif
            // local right hand sides
            // H_{1}\in R^{M}
            typedef typename DiscreteSigmaFunctionType::LocalFunctionType
                LocalH1rhsType;
            // H_{2}\in R^{L}
            typedef typename DiscreteVelocityFunctionType::LocalFunctionType
                LocalH2rhsType;
            // H_{3}\in R^{K}
            typedef typename DiscretePressureFunctionType::LocalFunctionType
                LocalH3rhsType;
//
            // base functions
            // of type sigma
            typedef typename DiscreteSigmaFunctionSpaceType::BaseFunctionSetType
                SigmaBaseFunctionSetType;
////            typedef typename SigmaBaseFunctionSetType::JacobianRangeType
////                SigmaJacobianRangeType;
            // of type u
            typedef typename DiscreteVelocityFunctionSpaceType::BaseFunctionSetType
                VelocityBaseFunctionSetType;
            // of type p
            typedef typename DiscretePressureFunctionSpaceType::BaseFunctionSetType
                PressureBaseFunctionSetType;

            // eps
            const double eps = Parameters().getParam( "eps", 1.0e-14 );

#ifndef NLOG
            // logging stuff
            Logging::LogStream& infoStream = Logger().Info();
            Logging::LogStream& debugStream = Logger().Info();
//            Logging::LogStream& debugStream = Logger().Dbg();
            bool entityOutput = false;
            bool intersectionOutput = false;
            const int outputEntity = 0;
            const int outputIntersection = -1;
            int entityNR = 0;
            int intersectionNR = 0;
            int numberOfEntities = 0;
            int numberOfIntersections = 0;
            int numberOfBoundaryIntersections = 0;
            int numberOfInnerIntersections = 0;
            const bool Mprint = Parameters().getParam( "Mprint", false );
            const bool Wprint = Parameters().getParam( "Wprint", false );
            const bool Xprint = Parameters().getParam( "Xprint", false );
            const bool Yprint = Parameters().getParam( "Yprint", false );
            const bool Zprint = Parameters().getParam( "Zprint", false );
            const bool Eprint = Parameters().getParam( "Eprint", false );
            const bool Rprint = Parameters().getParam( "Rprint", false );
            const bool H1print = Parameters().getParam( "H1print", false );
            const bool H2print = Parameters().getParam( "H2print", false );
            const bool H3print = Parameters().getParam( "H3print", false );
            const bool allOutput = Parameters().getParam( "allOutput", false );
            const bool Mdebug = Parameters().getParam( "Mdebug", false );
            const bool Wdebug = Parameters().getParam( "Wdebug", false );
            const bool Xdebug = Parameters().getParam( "Xdebug", false );
            const bool Ydebug = Parameters().getParam( "Ydebug", false );
            const bool Zdebug = Parameters().getParam( "Zdebug", false );
            const bool Edebug = Parameters().getParam( "Edebug", false );
            const bool Rdebug = Parameters().getParam( "Rdebug", false );
            const bool H1debug = Parameters().getParam( "H1debug", false );
            const bool H2debug = Parameters().getParam( "H2debug", false );
            const bool H3debug = Parameters().getParam( "H3debug", false );
            int fivePercentOfEntities = 0;
            int fivePercents = 0;
            infoStream << "this is StokesPass::apply()" << std::endl;
//
////#ifdef CHEAT
////            // init the gradient pass
////            typedef Dune::DiscreteGradientModelTraits<  GridPartType,
////                                                        DiscreteSigmaFunctionType >
////                DiscreteGradientModelTraitsImp;
////
////            typedef Dune::DiscreteGradientModel< DiscreteGradientModelTraitsImp >
////                DiscreteGradientModelType;
////
////
////            typedef Dune::StartPass< DiscreteVelocityFunctionType, -1 >
////                StartPassType;
////
////            typedef Dune::DiscreteGradientPass< DiscreteGradientModelType,
////                                                StartPassType,
////                                                0 >
////                DiscreteGradientPassType;
////            DiscreteGradientModelType discreteGradientModel;
////            StartPassType startPass;
////            DiscreteGradientPassType discreteGradientPass(  startPass,
////                                                            discreteGradientModel,
////                                                            gridPart_ );
////            // get the exact solution out of the argument
////            DiscreteVelocityFunctionType discreteExactVelocity = arg.discreteVelocity();
////            DiscreteSigmaFunctionType computedVelocityGradient( "computed velocity gradient", sigmaSpace_ );
////            computedVelocityGradient.clear();
////            discreteGradientPass.apply( discreteExactVelocity, computedVelocityGradient );
////            DiscretePressureFunctionType discreteExactPressure = arg.discretePressure();
////		double cg_min,cg_max;
////		Stuff::getMinMaxOfDiscreteFunction( computedVelocityGradient, cg_min, cg_max );
////		Stuff::oneLinePrint( infoStream, computedVelocityGradient );
////		Stuff::oneLinePrint( infoStream, arg.discreteVelocity() );
////		Stuff::oneLinePrint( infoStream, arg.discretePressure() );
////#endif
//
            // do an empty grid walk to get informations
            double maxGridWidth( 0.0 );
            EntityIteratorType entityItEndLog = velocitySpace_.end();
            for (   EntityIteratorType entityItLog = velocitySpace_.begin();
                    entityItLog != entityItEndLog;
                    ++entityItLog ) {
                const EntityType& entity = *entityItLog;
                // count entities
                ++numberOfEntities;
                // walk the intersections
                IntersectionIteratorType intItEnd = gridPart_.iend( entity );
                for (   IntersectionIteratorType intIt = gridPart_.ibegin( entity );
                        intIt != intItEnd;
                        ++intIt ) {
                    // count intersections
                    ++numberOfIntersections;
                    maxGridWidth = Stuff::getLenghtOfIntersection( intIt ) > maxGridWidth ? Stuff::getLenghtOfIntersection( intIt ) : maxGridWidth;
                    // if we are inside the grid
                    if ( intIt.neighbor() && !intIt.boundary() ) {
                        // count inner intersections
                        ++numberOfInnerIntersections;
                    }
                    // if we are on the boundary of the grid
                    if ( !intIt.neighbor() && intIt.boundary() ) {
                        // count boundary intersections
                        ++numberOfBoundaryIntersections;
                    }
                }
            }
            if ( numberOfEntities > 19 ) {
                infoStream << "found " << numberOfEntities << " entities," << std::endl;
                infoStream << "found " << numberOfIntersections << " intersections," << std::endl;
                infoStream << "      " << numberOfInnerIntersections << " intersections inside and" << std::endl;
                infoStream << "      " << numberOfBoundaryIntersections << " intersections on the boundary." << std::endl;
                infoStream << "      maxGridWidth is " << maxGridWidth << std::endl;
                infoStream << "- starting gridwalk" << std::endl;
                fivePercentOfEntities = int( std::floor(double(numberOfEntities) / double(20)));
                infoStream << "  [ assembling         ]" << std::endl;
                infoStream << "  [";
            } else {
                infoStream << "found " << numberOfEntities << " entities," << std::endl;
                infoStream << "found " << numberOfIntersections << " intersections," << std::endl;
                infoStream << "      " << numberOfInnerIntersections << " intersections inside and" << std::endl;
                infoStream << "      " << numberOfBoundaryIntersections << " intersections on the boundary." << std::endl;
                infoStream << "      maxGridWidth is " << maxGridWidth << std::endl;
                infoStream << "- starting gridwalk" << std::endl;
            }
            infoStream.Suspend();
#endif
            // walk the grid
            EntityIteratorType entityItEnd = velocitySpace_.end();
            for (   EntityIteratorType entityIt = velocitySpace_.begin();
                    entityIt != entityItEnd;
                    ++entityIt ) {

                // get entity and geometry
                const EntityType& entity = *entityIt;
                const EntityGeometryType& geometry = entity.geometry();

                // get local matrices for the volume integral
                LocalMInversMatrixType localMInversMatrixElement = MInversMatrix.localMatrix( entity, entity );
//                LocalMInversMatrixType localMMatrixElement = MMatrix.localMatrix( entity, entity );
                LocalWmatrixType localWmatrixElement = Wmatrix.localMatrix( entity, entity );
                LocalXmatrixType localXmatrixElement = Xmatrix.localMatrix( entity, entity );
                LocalYmatrixType localYmatrixElement = Ymatrix.localMatrix( entity, entity );
                LocalZmatrixType localZmatrixElement = Zmatrix.localMatrix( entity, entity );
                LocalEmatrixType localEmatrixElement = Ematrix.localMatrix( entity, entity );
                LocalRmatrixType localRmatrixElement = Rmatrix.localMatrix( entity, entity );

                // get local right hand sides
                LocalH1rhsType localH1rhs = H1rhs.localFunction( entity );
                LocalH2rhsType localH2rhs = H2rhs.localFunction( entity );
                LocalH3rhsType localH3rhs = H3rhs.localFunction( entity );

                // get basefunctionsets
                const SigmaBaseFunctionSetType sigmaBaseFunctionSetElement = sigmaSpace_.baseFunctionSet( entity );
                const VelocityBaseFunctionSetType velocityBaseFunctionSetElement = velocitySpace_.baseFunctionSet( entity );
                const PressureBaseFunctionSetType pressureBaseFunctionSetElement = pressureSpace_.baseFunctionSet( entity );
                const int numSigmaBaseFunctionsElement = sigmaBaseFunctionSetElement.numBaseFunctions();
                const int numVelocityBaseFunctionsElement = velocityBaseFunctionSetElement.numBaseFunctions();
                const int numPressureBaseFunctionsElement = pressureBaseFunctionSetElement.numBaseFunctions();

                // get quadrature
                const VolumeQuadratureType volumeQuadratureElement( entity,
                                                                    ( 2 * pressureSpaceOrder ) + 1 );
//                const VolumeQuadratureType volumeQuadratureElement( entity,
//                                                                    3 );
#ifndef NLOG
                if ( numberOfEntities > 19 ) {
                    if ( ( entityNR % fivePercentOfEntities ) == 0 ) {
                        if ( fivePercents < 20 ) {
                            infoStream.Resume();
                            infoStream << "=";
                            infoStream.Flush();
                            infoStream.Suspend();
                            ++fivePercents;
                        }
                    }
                }
                debugStream.Suspend(); // disable logging
//                if ( outputEntity == entityNR ) entityOutput = true;
                if ( allOutput ) entityOutput = true;
                if ( entityOutput ) debugStream.Resume(); // enable logging
                debugStream << "  - numSigmaBaseFunctionsElement: " << numSigmaBaseFunctionsElement << std::endl;
                debugStream << "  - numVelocityBaseFunctionsElement: " << numVelocityBaseFunctionsElement << std::endl;
                debugStream << "  - numPressureBaseFunctionsElement: " << numPressureBaseFunctionsElement << std::endl;
                debugStream << "  - == start calculations on entity " << entityNR << std::endl;
                debugStream << "    - entity " << entityNR << " has " << geometry.corners() << " corners:";
                for ( int i = 0; i < geometry.corners(); ++i ) {
                    const VelocityRangeType corner = geometry[i];
                    Stuff::printFieldVector( corner, "corner_"+Stuff::toString(i), debugStream, "      " );
                }
                debugStream << std::endl;
                bool Moutput = false;
                bool Woutput = false;
                bool Xoutput = false;
                bool Youtput = false;
                bool Zoutput = false;
                bool Eoutput = false;
                bool Routput = false;
                bool H1output = false;
                bool H2output = false;
                bool H3output = false;
                // we want logging at the following base functions
                const int logBaseI = Parameters().getParam( "logBaseI", 0 );
                const int logBaseJ = Parameters().getParam( "logBaseJ", 0 );
                debugStream.Suspend(); // disable logging
#endif
//#ifndef SKIP_VOLUME_INTEGRALS
                // compute volume integrals

                // needed for element volume during faceloop
                double elementVolume( 0.0 );

                // do loop over all quadrature points
                for ( int quad = 0; quad < volumeQuadratureElement.nop(); ++quad ) {

                    // quadrture point in reference element coordinates
                    const ElementCoordinateType x = volumeQuadratureElement.point( quad );
                    // quadrature point in world coordinates
                    const ElementCoordinateType xWorld = geometry.global( x );
                    // quadrature weight
                    const double integrationWeight = volumeQuadratureElement.weight( quad );
                    // integration factor
                    elementVolume = geometry.integrationElement( x );

                    // do loop over all SigmaBaseFunctions
                    for ( int m = 0; m < numSigmaBaseFunctionsElement; ++m ) {

                        // evalute the SigmaBaseFunctionSet
                        SigmaRangeType tau( 0.0 );
                        sigmaBaseFunctionSetElement.evaluate( m, x, tau );

                        // do loop over all VelocityBaseFunctions
                        for ( int l = 0; l < numVelocityBaseFunctionsElement; ++l ) {

                            // evaluate the VelocityBaseFunctionSet
                            VelocityRangeType v( 0.0 );
                            velocityBaseFunctionSetElement.evaluate( l, x, v );

//                            // do H2 assembling only once
//                            if ( m == 0 ) {
//
//                                // evaluate f
//                                VelocityRangeType f( 0.0 );
//                                discreteModel_.force( 0.0, xWorld, f );
//                                // add to H2_{l}
//                                localH2rhs[ l ] += elementVolume
//                                    * integrationWeight
//                                    * velocityBaseFunctionSetElement.evaluateSingle( l, x, f );
//
//                            } // done H2 assembling only once

                            // prepare v for divergence multiplication
                            SigmaJacobianRangeType vForDivergenceMult( 0.0 );
                            vForDivergenceMult[ 0 ][ 0 ] = v[ 0 ];
                            vForDivergenceMult[ 1 ][ 1 ] = v[ 0 ];
                            vForDivergenceMult[ 2 ][ 0 ] = v[ 1 ];
                            vForDivergenceMult[ 3 ][ 1 ] = v[ 1 ];

                            // compute \tfrac{1}{|T_{tt}|} \int_{T_{tt}} - \nabla\cdot \tau \cdot v dx
                            const double MinusDivergenceOfTauTimesV = -1.0
                                * integrationWeight
                                * sigmaBaseFunctionSetElement.evaluateGradientSingle( m, entity, x, vForDivergenceMult );

                            // compute \int_{T_{tt}} \tau : \nabla\cdot v
                            const double GradientOfVTimesTau = integrationWeight
                                * elementVolume
                                * velocityBaseFunctionSetElement.evaluateGradientSingle( l, entity, x, tau );

                            // add to local matrices
                            localXmatrixElement.add( l, m, GradientOfVTimesTau );
//                            localWmatrixElement.add( m, l, MinusDivergenceOfTauTimesV );

                            // do pressure-stuff only once
                            if ( m == 0 ) {

                                // do loop over all PressureBaseFunctions
                                for ( int k = 0; k <  numPressureBaseFunctionsElement; ++k ) {

                                    // evaluate the PressureBaseFunctionSet
                                    PressureRangeType q( 0.0 );
                                    pressureBaseFunctionSetElement.evaluate( k, x, q );

                                    // prepare q for divergence multiplication
                                    VelocityJacobianRangeType qForDivergenceMult( 0.0 );
                                    qForDivergenceMult[ 0 ][ 0 ] = q;
                                    qForDivergenceMult[ 1 ][ 1 ] = q;

                                    // compute \int_{T_{tt}} - q \nabla\cdot v
                                    const double minusQTimesDivergenceOfV = -1.0
                                        * integrationWeight
                                        * elementVolume
                                        * velocityBaseFunctionSetElement.evaluateGradientSingle( l, entity, x, qForDivergenceMult );

                                    // compute \int_{T_{tt}} \nabla q \cdot v
                                    PressureJacobianRangeType vForGradientMult( 0.0 );
                                    vForGradientMult[ 0 ] = v;
                                    const double vTimesGradientOfQ = integrationWeight
                                        * elementVolume
                                        * pressureBaseFunctionSetElement.evaluateGradientSingle( k, entity, x, vForGradientMult );

                                    // add to local matrices
                                    localZmatrixElement.add( l, k, minusQTimesDivergenceOfV );
//                                    localEmatrixElement.add( k, l, vTimesGradientOfQ );

                                } // done loop over all PressureBaseFunctions

                            } // done pressure-stuff only once

                        } // done loop over all VelocityBaseFunctions

                    } // done loop over all SigmaBaseFunctions

                } // done loop over all quadrature points

                //                                                     // we will call this one
                // (M^{-1})_{i,j} = (\int_{T}\tau_{j}:\tau_{i}dx)^{-1} // Minvs' volume integral
#ifndef NLOG
                if ( Mdebug ) {
                    debugStream.Resume(); // enable logging
                    debugStream << "    = M volume =================" << std::endl;
                    debugStream.Suspend();
                }
#endif
                for ( int i = 0; i < numSigmaBaseFunctionsElement; ++i ) {
                    for ( int j = 0; j < numSigmaBaseFunctionsElement; ++j ) {
                        double M_i_j = 0.0;
#ifndef NLOG
//                        if ( ( i == logBaseI ) && ( j == logBaseJ ) ) Moutput = true;
//                        if ( allOutput ) Moutput = true;
//                        if ( Mdebug ) Moutput = true;
                        if ( entityOutput && Moutput ) debugStream.Resume(); // enable logging
                        debugStream << "    = M ========================" << std::endl;
                        debugStream << "    basefunctions " << i << " " << j << std::endl;
                        debugStream << "    volumeQuadrature.nop() " << volumeQuadratureElement.nop() << std::endl;
#endif
                        // sum over all quadrature points
                        for ( int quad = 0; quad < volumeQuadratureElement.nop(); ++quad ) {
                            // get x
                            const ElementCoordinateType x = volumeQuadratureElement.point( quad );
                            const ElementCoordinateType xGlobal = geometry.global( x );
                            // get the integration factor
                            const double elementVolume = geometry.integrationElement( x );
                            // get the quadrature weight
                            const double integrationWeight = volumeQuadratureElement.weight( quad );
                            // compute \tau_{i}:\tau_{j}
                            SigmaRangeType tau_i( 0.0 );
                            SigmaRangeType tau_j( 0.0 );
                            sigmaBaseFunctionSetElement.evaluate( i, x, tau_i );
                            sigmaBaseFunctionSetElement.evaluate( j, x, tau_j );
                            const double tau_times_tau = colonProduct( tau_i, tau_j );
                            // compute M_i_j
                            M_i_j += elementVolume
                                * integrationWeight
                                * tau_times_tau;
#ifndef NLOG
                            debugStream << "    - quadPoint " << quad;
                            Stuff::printFieldVector( x, "x", debugStream, "      " );
                            Stuff::printFieldVector( xGlobal, "xGlobal", debugStream, "      " );
                            debugStream << "\n        - elementVolume: " << elementVolume << std::endl;
                            debugStream << "        - integrationWeight: " << integrationWeight;
                            Stuff::printFieldMatrix( tau_i, "tau_i", debugStream, "      " );
                            Stuff::printFieldMatrix( tau_j, "tau_j", debugStream, "      " );
                            debugStream << "\n        - tau_times_tau: " << tau_times_tau << std::endl;
                            debugStream << "        - M_" << i << "_" << j << "+=: " << M_i_j << std::endl;
#endif
                        } // done sum over quadrature points
                        // if small, should be zero
                        if ( fabs( M_i_j ) < eps ) {
                            M_i_j = 0.0;
////localMMatrixElement.add( i, j, M_i_j );
                        } // else invert
                        else {
////localMMatrixElement.add( i, j, M_i_j );
                            M_i_j = 1.0 / M_i_j;
                        }
////#ifndef NLOG
////                        if ( Mdebug && ( M_i_j > 0.0 ) ) {
////                            debugStream.Resume();
////                            debugStream << "      tau_" << i << ", tau_" << j << std::endl
////                                        << "        M( ";
////                        }
////#endif
                        // add to matrix
                        localMInversMatrixElement.add( i, j, M_i_j );
////#ifndef NLOG
////                        if ( Mdebug && ( M_i_j > 0.0 ) ) {
////                            debugStream << " ) += " << M_i_j << std::endl;
////                        }
////                        Moutput = false;
////                        debugStream.Suspend(); // disable logging
////#endif
                    }
                } // done computing Minvs' volume integral

                //                                                        // we will call this one
                // (W)_{i,j} += \int_{T}v_{j}\cdot(\nabla\cdot\tau_{i})dx // W's volume integral
                //                                                        // see also "W's entitity surface integral", "W's neighbour surface integral" and "W's boundary integral" below
#ifndef NLOG
                if ( Wdebug ) {
                    debugStream.Resume(); // enable logging
                    debugStream << "    = W volume =================" << std::endl;
                    debugStream.Suspend();
                }
#endif
                for ( int i = 0; i < numSigmaBaseFunctionsElement; ++i ) {
                    for ( int j = 0; j < numVelocityBaseFunctionsElement; ++j ) {
                        double W_i_j = 0.0;
#ifndef NLOG
                        if ( ( i == logBaseI ) && ( j == logBaseJ ) ) Woutput = true;
                        if ( ( ( i == logBaseI ) && ( j == logBaseJ ) ) && Wdebug ) Woutput = true;
                        if ( allOutput ) Woutput = true;
                        if ( Wdebug ) Woutput = true;
                        if ( entityOutput && Woutput ) debugStream.Resume(); // enable logging
                        debugStream << "    = W ========================" << std::endl;
                        debugStream << "    basefunctions " << i << " " << j << std::endl;
                        debugStream << "    volumeQuadrature.nop() " << volumeQuadratureElement.nop() << std::endl;
#endif
                        // sum over all quadrature points
                        for ( int quad = 0; quad < volumeQuadratureElement.nop(); ++quad ) {
                            // get x
                            const ElementCoordinateType x = volumeQuadratureElement.point( quad );
//                            const ElementCoordinateType xGlobal = geometry.global( x );
                            // get the integration factor
                            const double elementVolume = geometry.integrationElement( x );
                            // get the quadrature weight
                            const double integrationWeight = volumeQuadratureElement.weight( quad );
                            // compute v_j^t \cdot ( \nabla \cdot \tau_i^t )
                            VelocityRangeType v_j( 0.0 );
                            velocityBaseFunctionSetElement.evaluate( j, x, v_j );
//                            SigmaJacobianRangeType gradient_of_tau_i( 0.0 );
//                            sigmaBaseFunctionSetElement.jacobian( i, x, gradient_of_tau_i );
//                            const VelocityRangeType divergence_of_tau_i_untransposed = sigmaDivergenceOutOfGradient( gradient_of_tau_i );
//                            VelocityRangeType divergence_of_tau_i( 0.0 );
//                            const JacobianInverseTransposedType jacobianInverseTransposed = geometry.jacobianInverseTransposed( x );
//                            jacobianInverseTransposed.umv( divergence_of_tau_i_untransposed, divergence_of_tau_i );
                            const double divergence_of_tau_times_v = sigmaBaseFunctionSetElement.evaluateGradientSingle( i, entity, x, prepareVelocityRangeTypeForSigmaDivergence( v_j ) );
                            W_i_j += elementVolume
                                * integrationWeight
                                * divergence_of_tau_times_v;
//#ifndef NLOG
//                            debugStream.Resume(); // enable logging
//                            Stuff::printFieldMatrix( jacobianInverseTransposed, "jacobianInverseTransposed", debugStream, "      " );
//                            debugStream << "    - quadPoint " << quad;
//                            Stuff::printFieldVector( x, "x", debugStream, "      " );
////                            Stuff::printFieldVector( xGlobal, "xGlobal", debugStream, "      " );
//                            debugStream << "\n        - elementVolume: " << elementVolume << std::endl;
//                            debugStream << "        - integrationWeight: " << integrationWeight;
//                            Stuff::printFieldMatrix( gradient_of_tau_i, "gradient_of_tau_i", debugStream, "      " );
//                            Stuff::printFieldVector( divergence_of_tau_i, "divergence_of_tau_i", debugStream, "      " );
//                            Stuff::printFieldVector( v_j, "v_j", debugStream, "      " );
//                            Stuff::printFieldVector( divergence_of_tau_i_untransposed, "divergence_of_tau_i_untransposed", debugStream, "      " );
//                            debugStream << "\n        - v_j_times_divergence_of_tau_i: " << v_j_times_divergence_of_tau_i << std::endl;
//                            debugStream << "\n        - W_" << i << "_" << j << "+=: " << W_i_j << std::endl;
//                            debugStream.Suspend();
//#endif
                        } // done sum over quadrature points
                        // if small, should be zero
                        if ( fabs( W_i_j ) < eps ) {
                            W_i_j = 0.0;
                        }
//#ifndef NLOG
//                        if ( Wdebug && ( W_i_j > 0.0 ) ) {
//                            debugStream.Resume();
//                            debugStream << "      W( ";
//                        }
//#ifdef LOTS_OF_DEBUG
//                        else if ( Wdebug ) {
//                            debugStream.Resume();
//                            debugStream << "      W( ";
//                        }
//#endif
//#endif
                        // add to matrix
                        localWmatrixElement.add( i, j, W_i_j );
//#ifndef NLOG
//                        if ( Wdebug && ( W_i_j > 0.0 ) ) {
//                            debugStream << " ) += " << W_i_j << std::endl
//                                        << "                 entity " << entityNR << ", tau_" << i << ", v_" << j << ", W volume" << std::endl;
//                        }
//#ifdef LOTS_OF_DEBUG
//                        else if ( Wdebug ) {
//                            debugStream << " ) += " << W_i_j << std::endl
//                                        << "                 entity " << entityNR << ", tau_" << i << ", v_" << j << ", W volume" << std::endl;
//                        }
//#endif
//                        Woutput = false;
//                        debugStream.Suspend(); // disable logging
//#endif
                    }
                } // done computing W's volume integral
//
//                //                                                  // we will call this one
//                // (X)_{i,j} += \mu\int_{T}\tau_{j}:\nabla v_{i} dx // X's volume integral
//                //                                                  // see also "X's entitity surface integral", "X's neighbour surface integral" and "X's boundary integral" below
////#ifndef NLOG
////                if ( Xdebug ) {
////                    debugStream.Resume(); // enable logging
////                    debugStream << "    = X volume =================" << std::endl;
////                    debugStream.Suspend();
////                }
////#endif
//                for ( int i = 0; i < numVelocityBaseFunctionsElement; ++i ) {
//                    for ( int j = 0; j < numSigmaBaseFunctionsElement; ++j ) {
//                        double X_i_j = 0.0;
////#ifndef NLOG
//////                        if ( ( i == logBaseI ) && ( j == logBaseJ ) ) Xoutput = true;
//////                        if ( allOutput ) Xoutput = true;
//////                        if ( Xdebug ) Xoutput = true;
////                        if ( entityOutput && Xoutput ) debugStream.Resume(); // enable logging
////                        debugStream << "    = X ========================" << std::endl;
////                        debugStream << "    basefunctions " << i << " " << j << std::endl;
////                        debugStream << "    volumeQuadrature.nop() " << volumeQuadratureElement.nop() << std::endl;
////#endif
//                        // sum over all quadrature points
//                        for ( int quad = 0; quad < volumeQuadratureElement.nop(); ++quad ) {
//                            // get x
//                            const ElementCoordinateType x = volumeQuadratureElement.point( quad );
//                            // get the integration factor
//                            const double elementVolume = geometry.integrationElement( x );
//                            // get the quadrature weight
//                            const double integrationWeight = volumeQuadratureElement.weight( quad );
////                            // compute \tau_{j}:\nabla v_{i}
//                            SigmaRangeType gradient_of_v_i_untransposed( 0.0 );
//                            SigmaRangeType gradient_of_v_i( 0.0 );
//                            velocityBaseFunctionSetElement.jacobian( i, x, gradient_of_v_i_untransposed );
//                            const JacobianInverseTransposedType jacobianInverseTransposed = geometry.jacobianInverseTransposed( x );
//                            jacobianInverseTransposed.umv( gradient_of_v_i_untransposed, gradient_of_v_i );
//                            SigmaRangeType tau_j( 0.0 );
//                            sigmaBaseFunctionSetElement.evaluate( j, x, tau_j );
//                            const double gradient_of_v_times_t =
//                                colonProduct( gradient_of_v_i, tau_j );
//                            X_i_j += elementVolume
//                                * integrationWeight
//                                * mu
//                                * gradient_of_v_times_t;
////#ifndef NLOG
////                            debugStream << "    - quadPoint " << quad;
////                            Stuff::printFieldVector( x, "x", debugStream, "      " );
////                            debugStream << "\n        - elementVolume: " << elementVolume << std::endl;
////                            debugStream << "        - integrationWeight: " << integrationWeight;
////                            Stuff::printFieldVector( gradient_of_v_i, "gradient of v_i", debugStream, "      " );
////                            Stuff::printFieldMatrix( tau_j, "tau_j", debugStream, "      " );
////                            debugStream << "\n        - tau_j_times_gradient_v_i: " << tau_j_times_gradient_v_i << std::endl;
////                            debugStream << "        - X_" << i << "_" << j << "+=: " << X_i_j << std::endl;
////#endif
//                        } // done sum over quadrature points
//                        // if small, should be zero
//                        if ( fabs( X_i_j ) < eps ) {
//                            X_i_j = 0.0;
//                        }
////#ifndef NLOG
////                        if ( Xdebug && ( X_i_j > 0.0 ) ) {
////                            debugStream.Resume();
////                            debugStream << "      v_" << i << ", tau_" << j << std::endl
////                                        << "        X( ";
////                        }
////#ifdef LOTS_OF_DEBUG
////                        else if ( Xdebug ) {
////                            debugStream.Resume();
////                            debugStream << "      v_" << i << ", tau_" << j << std::endl
////                                        << "        X( ";
////                        }
////#endif
////#endif
//                        // add to matrix
//                        localXmatrixElement.add( i, j, X_i_j );
////#ifndef NLOG
////                        if ( Xdebug && ( X_i_j > 0.0 ) ) {
////                            debugStream << " ) += " << X_i_j << std::endl;
////                        }
////#ifdef LOTS_OF_DEBUG
////                        else if ( Xdebug ) {
////                            debugStream << " ) += " << X_i_j << std::endl;
////                        }
////#endif
////                        Xoutput = false;
////                        debugStream.Suspend(); // disable logging
////#endif
//                    }
//                } // done computing X's volume integral
//
//                //                                                  // we will call this one
//                // (Z)_{i,j} += -\int_{T}q_{j}(\nabla\cdot v_{i})dx // Z's volume integral
//                //                                                  // see also "Z's entitity surface integral", "Z's neighbour surface integral" and "Z's boundary integral" below
////#ifndef NLOG
////                if ( Zdebug ) {
////                    debugStream.Resume(); // enable logging
////                    debugStream << "    = Z volume =================" << std::endl;
////                    debugStream.Suspend();
////                }
////#endif
//                for ( int i = 0; i < numVelocityBaseFunctionsElement; ++i ) {
//                    for ( int j = 0; j < numPressureBaseFunctionsElement; ++j ) {
//                        double Z_i_j = 0.0;
////#ifndef NLOG
//////                        if ( ( i == logBaseI ) && ( j == logBaseJ ) ) Zoutput = true;
//////                        if ( allOutput ) Zoutput = true;
//////                        if ( Zdebug ) Zoutput = true;
////                        if ( entityOutput && Zoutput ) debugStream.Resume(); // enable logging
////                        debugStream << "    = Z ========================" << std::endl;
////                        debugStream << "    basefunctions " << i << " " << j << std::endl;
////                        debugStream << "    volumeQuadrature.nop() " << volumeQuadratureElement.nop() << std::endl;
////#endif
//                        // sum over all quadratur points
//                        for ( int quad = 0; quad < volumeQuadratureElement.nop(); ++ quad ) {
//                            // get x
//                            const ElementCoordinateType x = volumeQuadratureElement.point( quad );
//                            // get the integration factor
//                            const double elementVolume = geometry.integrationElement( x );
//                            // get the quadrature weight
//                            const double integrationWeight = volumeQuadratureElement.weight( quad );
////                            // compute q_{j}\cdot(\nabla\cdot v_i)
//                            PressureRangeType q_j( 0.0 );
//                            pressureBaseFunctionSetElement.evaluate( j, x, q_j );
////                            SigmaRangeType gradient_of_v_i_untransposed( 0.0 );
////                            velocityBaseFunctionSetElement.jacobian( i, x, gradient_of_v_i_untransposed );
////                            const JacobianInverseTransposedType jacobianInverseTransposed = geometry.jacobianInverseTransposed( x );
////                            SigmaRangeType gradient_of_v_i( 0.0 );
////                            jacobianInverseTransposed.umv( gradient_of_v_i_untransposed, gradient_of_v_i );
////                            const double divergence_of_v_i = velocityDivergenceOutOfGradient( gradient_of_v_i );
//                            const double divergence_of_v_times_q = velocityBaseFunctionSetElement.evaluateGradientSingle( i, entity, x, preparePressureRangeTypeForVelocityDivergence( q_j ) );
//                            Z_i_j += -1.0
//                                * elementVolume
//                                * integrationWeight
//                                * divergence_of_v_times_q;
////#ifndef NLOG
////                            debugStream << "    - quadPoint " << quad;
////                            Stuff::printFieldVector( x, "x", debugStream, "      " );
////                            debugStream << "\n        - elementVolume: " << elementVolume << std::endl;
////                            debugStream << "        - integrationWeight: " << integrationWeight;
////                            Stuff::printFieldMatrix( gradient_of_v_i, "gradient_of_v_i", debugStream, "      " );
////                            Stuff::printFieldVector( q_j, "q_j", debugStream, "      " );
////                            debugStream << "\n        - q_j_times_divergence_of_v_i: " << q_j_times_divergence_of_v_i << std::endl;
////                            debugStream << "        - Z_" << i << "_" << j << "+=: " << Z_i_j << std::endl;
////#endif
//                        } // done sum over all quadrature points
//                        // if small, should be zero
//                        if ( fabs( Z_i_j ) < eps ) {
//                            Z_i_j = 0.0;
//                        }
////#ifndef NLOG
////                        if ( Zdebug && ( Z_i_j > 0.0 ) ) {
////                            debugStream.Resume();
////                            debugStream << "      v_" << i << ", q_" << j << std::endl
////                                        << "        Z( ";
////                        }
////#ifdef LOTS_OF_DEBUG
////                        else if ( Zdebug ) {
////                            debugStream.Resume();
////                            debugStream << "      v_" << i << ", q_" << j << std::endl
////                                        << "        Z( ";
////                        }
////#endif
////#endif
//                        // add to matrix
//                        localZmatrixElement.add( i, j, Z_i_j );
////#ifndef NLOG
////                        if ( Zdebug && ( Z_i_j > 0.0 ) ) {
////                            debugStream << " ) += " << Z_i_j << std::endl;
////                        }
////#ifdef LOTS_OF_DEBUG
////                        else if ( Zdebug ) {
////                            debugStream << " ) += " << Z_i_j << std::endl;
////                        }
////#endif
////                        Zoutput = false;
////                        debugStream.Suspend(); // disable logging
////#endif
//                    }
//                } // done computing Z's volume integral

                //                                    // we will call this one
                // (H2)_{j} += \int_{T}f\cdot v_{j}dx // H2's volume integral
                //                                    // see also "H2's boundary integral" further down
//                if ( discreteModel_.hasForce() ) {
                    for ( int j = 0; j < numVelocityBaseFunctionsElement; ++j ) {
                        double H2_j = 0.0;
#ifndef NLOG
    //                    if ( ( j == logBaseJ ) ) H2output = true;
//                        if ( allOutput ) H2output = true;
                        if ( H2debug ) H2output = true;
                        if ( entityOutput && H2output ) debugStream.Resume(); // enable logging
                        debugStream << "    = H2 =======================" << std::endl;
                        debugStream << "    basefunction " << " " << j << std::endl;
                        debugStream << "    volumeQuadrature.nop() " << volumeQuadratureElement.nop() << std::endl;
#endif
                        // sum over all quadratur points
                        for ( int quad = 0; quad < volumeQuadratureElement.nop(); ++ quad ) {
                            // get x
                            const ElementCoordinateType x = volumeQuadratureElement.point( quad );
                            const VelocityRangeType xWorld = geometry.global( x );

                            // get the integration factor
                            const double elementVolume = geometry.integrationElement( x );
                            // get the quadrature weight
                            const double integrationWeight = volumeQuadratureElement.weight( quad );
                            // compute f\cdot v_j
                            VelocityRangeType v_j( 0.0 );
                            VelocityRangeType f( 0.0 );
                            velocityBaseFunctionSetElement.evaluate( j, x, v_j );
                            discreteModel_.force( 0.0, xWorld, f );
                            const double f_times_v_j = f * v_j;
                            H2_j += elementVolume
                                * integrationWeight
                                * f_times_v_j;
#ifndef NLOG
                            debugStream << "    - quadPoint " << quad;
                            Stuff::printFieldVector( x, "x", debugStream, "      " );
                            Stuff::printFieldVector( xWorld, "xWorld", debugStream, "      " );
                            debugStream << "\n        - elementVolume: " << elementVolume << std::endl;
                            debugStream << "        - integrationWeight: " << integrationWeight;
                            Stuff::printFieldVector( f, "f", debugStream, "      " );
                            Stuff::printFieldVector( v_j, "v_j", debugStream, "      " );
                            debugStream << "\n        - f_times_v_j: " << f_times_v_j << std::endl;
                            debugStream << "        - H2_" << j << "+=: " << H2_j << std::endl;
#endif
                        } // done sum over all quadrature points
                        // if small, should be zero
                        if ( fabs( H2_j ) < eps ) {
                            H2_j = 0.0;
                        }
                        // add to rhs
                        localH2rhs[ j ] += H2_j;
#ifndef NLOG
                        H2output = false;
                        debugStream.Suspend(); // disable logging
#endif
                    } // done computing H2's volume integral
//                }

                //                                                // we will call this one
                // (E)_{i,j} += -\int_{T}v_{j}\cdot\nabla q_{i}dx // E's volume integral
                //                                                // see also "E's entitity surface integral", "E's neighbour surface integral" and "E's boundary integral" below
#ifndef NLOG
                if ( Edebug ) {
                    debugStream.Resume(); // enable logging
                    debugStream << "    = E volume =================" << std::endl;
                    debugStream.Suspend();
                }
#endif
                for ( int i = 0; i < numPressureBaseFunctionsElement; ++i ) {
                    for ( int j = 0; j < numVelocityBaseFunctionsElement; ++j ) {
                        double E_i_j = 0.0;
#ifndef NLOG
//                        if ( ( i == logBaseI ) && ( j == logBaseJ ) ) Eoutput = true;
//                        if ( allOutput ) Eoutput = true;
//                        if ( Edebug ) Eoutput = true;
                        if ( entityOutput && Eoutput ) debugStream.Resume(); // enable logging
                        debugStream << "    = E ========================" << std::endl;
                        debugStream << "    basefunctions " << i << " " << j << std::endl;
                        debugStream << "    volumeQuadrature.nop() " << volumeQuadratureElement.nop() << std::endl;
#endif
                        // sum over all quadratur points
                        for ( int quad = 0; quad < volumeQuadratureElement.nop(); ++ quad ) {
                            // get x
                            ElementCoordinateType x = volumeQuadratureElement.point( quad );
                            // get the integration factor
                            double elementVolume = geometry.integrationElement( x );
                            // get the quadrature weight
                            double integrationWeight = volumeQuadratureElement.weight( quad );
                            // compute v_{j}\cdot(\nabla q_i)
                            VelocityRangeType v_j( 0.0 );
                            velocityBaseFunctionSetElement.evaluate( j, x, v_j );
                            typename DiscretePressureFunctionSpaceType::JacobianRangeType jacobian_of_q_i( 0.0 );
                            pressureBaseFunctionSetElement.jacobian( i, x, jacobian_of_q_i );
                            const VelocityRangeType gradient_of_q_i_untransposed( jacobian_of_q_i[0] );
                            const JacobianInverseTransposedType jacobianInverseTransposed = geometry.jacobianInverseTransposed( x );
                            VelocityRangeType gradient_of_q_i( 0.0 );
                            jacobianInverseTransposed.umv( gradient_of_q_i_untransposed, gradient_of_q_i );
                            const double gradient_of_q_times_v = gradient_of_q_i * v_j;
                            E_i_j += -1.0
                                * elementVolume
                                * integrationWeight
                                * gradient_of_q_times_v;
//#ifndef NLOG
//                            debugStream << "    - quadPoint " << quad;
//                            Stuff::printFieldVector( x, "x", debugStream, "      " );
//                            debugStream << "\n        - elementVolume: " << elementVolume << std::endl;
//                            debugStream << "        - integrationWeight: " << integrationWeight;
//                            Stuff::printFieldVector( gradient_of_q_i, "gradient_of_q_i", debugStream, "      " );
//                            Stuff::printFieldVector( v_j, "v_j", debugStream, "      " );
//                            debugStream << "\n        - v_j_times_gradient_of_q_i: " << v_j_times_gradient_of_q_i << std::endl;
//                            debugStream << "        - E_" << i << "_" << j << "+=: " << E_i_j << std::endl;
//#endif
                        } // done sum over all quadrature points
                        // if small, should be zero
                        if ( fabs( E_i_j ) < eps ) {
                            E_i_j = 0.0;
                        }
//#ifndef NLOG
//                        if ( Edebug && ( E_i_j > 0.0 ) ) {
//                            debugStream.Resume();
//                            debugStream << "      q_" << i << ", v_" << j << std::endl
//                                        << "        E( ";
//                        }
//#ifdef LOTS_OF_DEBUG
//                        else if ( Edebug ) {
//                            debugStream.Resume();
//                            debugStream << "      q_" << i << ", v_" << j << std::endl
//                                        << "        E( ";
//                        }
//#endif
//#endif
                        // add to matrix
                        localEmatrixElement.add( i, j, E_i_j );
//#ifndef NLOG
//                        if ( Edebug && ( E_i_j > 0.0 ) ) {
//                            debugStream << " ) += " << E_i_j << std::endl;
//                        }
//#ifdef LOTS_OF_DEBUG
//                        else if ( Edebug ) {
//                            debugStream << " ) += " << E_i_j << std::endl;
//                        }
//#endif
//                        Eoutput = false;
//                        debugStream.Suspend(); // disable logging
//#endif
                    }
                } // done computing E's volume integral
//#endif //SKIP VOLUME
//#ifndef SKIP_SURFACE_INTEGRALS
                // walk the intersections
                IntersectionIteratorType intItEnd = gridPart_.iend( entity );
                for (   IntersectionIteratorType intIt = gridPart_.ibegin( entity );
                        intIt != intItEnd;
                        ++intIt ) {
#ifndef NLOG
//                    if ( ( outputIntersection == intersectionNR ) && entityOutput ) intersectionOutput = true;
                    if ( entityOutput ) intersectionOutput = true;
                    if ( intersectionOutput ) debugStream.Resume(); // enable logging
                    debugStream << "    - ==== start calculations on intersection " << intersectionNR << std::endl;
#endif

                    // get intersection geometry
                    typedef typename IntersectionIteratorType::Geometry
                        IntersectionGeometryType;
//                    const IntersectionGeometryType& intersectionGeoemtry = intIt.intersectionSelfLocal();
                    const IntersectionGeometryType& intersectionGeoemtry = intIt.intersectionGlobal();
#ifndef NLOG
                    // get corners
                    debugStream << "      - intersection " << intersectionNR << " has " << intersectionGeoemtry.corners() << " corners:";
                    for ( int i = 0; i < intersectionGeoemtry.corners(); ++i ) {
                        const VelocityRangeType corner = intersectionGeoemtry[i];
                        Stuff::printFieldVector( corner, "corner_"+Stuff::toString(i), debugStream, "        " );
                    }
                    debugStream << "\n        length of intersection " << intersectionNR << " is " << Stuff::getLenghtOfIntersection( intIt ) << std::endl;
                    debugStream.Suspend(); // disable logging
#endif

                    // get intersection quadrature, seen from inside
                    const FaceQuadratureType faceQuadratureElement( gridPart_,
                                                                    intIt,
                                                                    ( 2 * pressureSpaceOrder ) + 1,
                                                                    FaceQuadratureType::INSIDE );
////                    const FaceQuadratureType faceQuadratureElement( gridPart_,
////                                                                    intIt,
////                                                                    3,
////                                                                    FaceQuadratureType::INSIDE );

                    // get flux coefficients
                    const double lengthOfIntersection = getLenghtOfIntersection( intIt );
                    const double C_11 = 1.0 / lengthOfIntersection;
                    const double D_11 = lengthOfIntersection;

                    // if we are inside the grid
                    if ( intIt.neighbor() && !intIt.boundary() ) {
                        // get neighbour
                        const typename IntersectionIteratorType::EntityPointer neighbourPtr = intIt.outside();
                        const EntityType& neighbour = *neighbourPtr;

                        // get local matrices for the surface integrals
                        LocalMInversMatrixType localMInversMatrixNeighbour = MInversMatrix.localMatrix( entity, neighbour );
                        LocalWmatrixType localWmatrixNeighbour = Wmatrix.localMatrix( neighbour, entity );
                        LocalXmatrixType localXmatrixNeighbour = Xmatrix.localMatrix( entity, neighbour );
                        LocalYmatrixType localYmatrixNeighbour = Ymatrix.localMatrix( neighbour, entity );
                        LocalZmatrixType localZmatrixNeighbour = Zmatrix.localMatrix( entity, neighbour );
                        LocalEmatrixType localEmatrixNeighbour = Ematrix.localMatrix( neighbour, entity );
                        LocalRmatrixType localRmatrixNeighbour = Rmatrix.localMatrix( entity, neighbour );

                        // get basefunctionsets
                        const SigmaBaseFunctionSetType sigmaBaseFunctionSetNeighbour = sigmaSpace_.baseFunctionSet( neighbour );
                        const VelocityBaseFunctionSetType velocityBaseFunctionSetNeighbour = velocitySpace_.baseFunctionSet( neighbour );
                        const PressureBaseFunctionSetType pressureBaseFunctionSetNeighbour = pressureSpace_.baseFunctionSet( neighbour );
                        const int numSigmaBaseFunctionsNeighbour = sigmaBaseFunctionSetNeighbour.numBaseFunctions();
                        const int numVelocityBaseFunctionsNeighbour = velocityBaseFunctionSetNeighbour.numBaseFunctions();
                        const int numPressureBaseFunctionsNeighbour = pressureBaseFunctionSetNeighbour.numBaseFunctions();

                        // get intersection quadrature, seen from outside
                        const FaceQuadratureType faceQuadratureNeighbour(   gridPart_,
                                                                            intIt,
                                                                            ( 2 * pressureSpaceOrder ) + 1,
                                                                            FaceQuadratureType::OUTSIDE );
////                        const FaceQuadratureType faceQuadratureNeighbour(   gridPart_,
////                                                                            intIt,
////                                                                            3,
////                                                                            FaceQuadratureType::OUTSIDE );

                        // compute surface integrals

                        // do loop over all quadrature points
                        for ( int quad = 0; quad < faceQuadratureElement.nop(); ++quad ) {

                            // quadrture point in reference element coordinates ssen from inside
                            const ElementCoordinateType xInner = faceQuadratureElement.point( quad );
                            // quadrture point in reference element coordinates ssen from outside
                            const ElementCoordinateType xOuter = faceQuadratureNeighbour.point( quad );
                            // quadrature point in world coordinates
                            const ElementCoordinateType xWorld = geometry.global( xInner );
                            // quadrature point in codim< 1 > reference element coordinates
                            const LocalIntersectionCoordinateType xLocal = faceQuadratureElement.localPoint( quad );
                            // quadrature weight
                            const double integrationWeight = faceQuadratureElement.weight( quad );
                            // integration factor
                            const double faceVolume = intersectionGeoemtry.integrationElement( xLocal );
                            // unit outer normal
                            const VelocityRangeType normal = intIt.unitOuterNormal( faceQuadratureElement.localPoint( quad ) );

                            // flux constants
                            const double c11 = 1.0 / faceVolume;
                            const double d11 = faceVolume;

                            // do loop over all VelocityBaseFunctions
                            for ( int l = 0; l < numVelocityBaseFunctionsElement; ++l ) {

                                // evaluate VelocityBaseFunctions on Element
                                VelocityRangeType vElement( 0.0 );
                                velocityBaseFunctionSetElement.evaluate( l, xInner, vElement );
                                // evaluate VelocityBaseFunctions on Neighbour
                                VelocityRangeType vNeighbour( 0.0 );
                                velocityBaseFunctionSetNeighbour.evaluate( l, xOuter, vNeighbour );

                                //evaluate fluxes
                                SigmaRangeType normalTensorHalfVElement = dyadicProduct( normal, vElement );
                                normalTensorHalfVElement *= 0.5;

                                SigmaRangeType normalTensorHalfVNeighbour = dyadicProduct( normal, vNeighbour );
                                normalTensorHalfVNeighbour *= 0.5;

                                VelocityRangeType minusOneOverHTimesVElement = vElement;
                                minusOneOverHTimesVElement *= ( -1.0 * c11 );

                                VelocityRangeType minusOneOverHTimesVNeighbour = vNeighbour;
                                minusOneOverHTimesVNeighbour *= ( -1.0 * c11 );

                                double normalTimesHalfVElement = normal * vElement;
                                normalTimesHalfVElement *= 0.5;

                                double normalTimesHalfVNeighbour = normal * vNeighbour;
                                normalTimesHalfVNeighbour *= 0.5;

                                // do loop over all SigmaBaseFunctions
                                for ( int m = 0; m < numSigmaBaseFunctionsElement; ++m ) {

                                    // evaluate SigmaBaseFunctions on Element
                                    SigmaRangeType tauElement( 0.0 );
                                    sigmaBaseFunctionSetElement.evaluate( m, xInner, tauElement );

                                    // evaluate SigmaBaseFunctions on Neighbour
                                    SigmaRangeType tauNeighbour( 0.0 );
                                    sigmaBaseFunctionSetElement.evaluate( m, xOuter, tauNeighbour );

                                    // evaluate the fluxes
                                    VelocityRangeType halfTauElementTimesNormal( 0.0 );
                                    tauElement.umv( normal, halfTauElementTimesNormal );
                                    halfTauElementTimesNormal *= 0.5;

                                    VelocityRangeType halfTauNeighbourTimesNormal( 0.0 );
                                    tauElement.umv( normal, halfTauNeighbourTimesNormal );
                                    halfTauNeighbourTimesNormal *= 0.5;

                                    // compute |T| * W_en
                                    const double W_en = integrationWeight
                                        * faceVolume
                                        * sigmaBaseFunctionSetElement.evaluateSingle( m, xInner, normalTensorHalfVElement )
                                        * elementVolume;

                                    // compute |T| * W_nb
                                    const double W_nb = integrationWeight
                                        * faceVolume
                                        * sigmaBaseFunctionSetElement.evaluateSingle( m, xInner, normalTensorHalfVNeighbour )
                                        * elementVolume;

                                    // add to local matrices
//                                    localWmatrixElement.add( m, l, W_en );
//                                    localWmatrixNeighbour.add( m, l, W_nb );

                                    // compute X_en
                                    const double X_en = -1.0
                                        * integrationWeight
                                        * faceVolume
                                        * velocityBaseFunctionSetElement.evaluateSingle( l, xInner, halfTauElementTimesNormal );

                                    // compute X_nb
                                    const double X_nb = -1.0
                                        * integrationWeight
                                        * faceVolume
                                        * velocityBaseFunctionSetElement.evaluateSingle( l, xInner, halfTauNeighbourTimesNormal );

                                    // add to local matrices
                                    localXmatrixElement.add( l, m, X_en );
                                    localXmatrixNeighbour.add( l, m, X_nb );

                                } // done loop over all SigmaBaseFunctions

                                // do loop over all PressureBaseFunctions
                                for ( int k = 0; k < numPressureBaseFunctionsElement; ++k ) {

                                    // evalute PressureBaseFunctions on Element
                                    PressureRangeType qElement( 0.0 );
                                    pressureBaseFunctionSetElement.evaluate( k, xInner, qElement );

                                    // evalute PressureBaseFunctions on Neighbour
                                    PressureRangeType qNeighbour( 0.0 );
                                    pressureBaseFunctionSetNeighbour.evaluate( k, xOuter, qNeighbour );

                                    // evaluate fluxes
                                    VelocityRangeType halfQElementTimesNormal = normal;
                                    halfQElementTimesNormal *= ( qElement * 0.5 );

                                    VelocityRangeType halfQNeighbourTimesNormal = normal;
                                    halfQNeighbourTimesNormal *= ( qNeighbour * 0.5 );

                                    // compute Z_en
                                    const double Z_en = integrationWeight
                                        * faceVolume
                                        * velocityBaseFunctionSetElement.evaluateSingle( l, xInner, halfQElementTimesNormal );

                                    // compute Z_nb
                                    const double Z_nb = integrationWeight
                                        * faceVolume
                                        * velocityBaseFunctionSetElement.evaluateSingle( l, xInner, halfQNeighbourTimesNormal );

                                    // add to local matrices
                                    localZmatrixElement.add( l, k, Z_en );
                                    localZmatrixNeighbour.add( l, k, Z_nb );

                                    // compute E_en
                                    const double E_en = -1.0
                                        * integrationWeight
                                        * faceVolume
                                        * pressureBaseFunctionSetElement.evaluateSingle( k, xInner, normalTimesHalfVElement );

                                    // compute E_nb
                                    const double E_nb = -1.0
                                        * integrationWeight
                                        * faceVolume
                                        * pressureBaseFunctionSetElement.evaluateSingle( k, xInner, normalTimesHalfVNeighbour );

                                    // add to local matrices
//                                    localEmatrixElement.add( k, l, E_en );
//                                    localEmatrixNeighbour.add( k, l, E_nb );

                                    // do R calculation only for one l
                                    if ( l == 0 ) {

                                        // evaluate fluxes
                                        PressureRangeType d11TimesQElement = qElement;
                                        d11TimesQElement *= d11;

                                        PressureRangeType d11TimesQNeighbour = qNeighbour;
                                        d11TimesQNeighbour *= ( -1.0 * d11 );

                                        // do loop over all PressureBaseFunctions
                                        for ( int kk = 0; kk < numPressureBaseFunctionsElement; ++kk ) {

                                            // compute R_en
                                            const double R_en = integrationWeight
                                                * faceVolume
                                                * pressureBaseFunctionSetElement.evaluateSingle( kk, xInner, d11TimesQElement );

                                            // compute R_nb
                                            const double R_nb = integrationWeight
                                                * faceVolume
                                                * pressureBaseFunctionSetElement.evaluateSingle( kk, xInner, d11TimesQNeighbour );

                                            // add to local matrices
                                            localRmatrixElement.add( kk, k, R_en );
                                            localRmatrixNeighbour.add( kk, k, R_nb );

                                        } // done loop over all PressureBaseFunctions

                                    } // done R calculation only for one l

                                } // done loop over all PressureBaseFunctions

                                // do loop over all VelocityBaseFunctions
                                for ( int ll = 0; ll < numVelocityBaseFunctionsElement; ++ll ) {

                                    // evaluate the VelocityBaseFunctionSet
                                    VelocityRangeType v( 0.0 );
                                    velocityBaseFunctionSetElement.evaluate( ll, xInner, v );

                                    // compute Y_en
                                    const double Y_en = -1.0
                                        * integrationWeight
                                        * faceVolume
                                        * ( minusOneOverHTimesVElement * v );

                                    // compute Y_nb
                                    const double Y_nb = -1.0
                                        * integrationWeight
                                        * faceVolume
                                        * ( minusOneOverHTimesVNeighbour * v );

                                    // add to local matrices
//                                    localYmatrixElement.add( ll, l, Y_en );
//                                    localYmatrixNeighbour.add( ll, l, Y_nb );

                                } // done loop over all VelocityBaseFunctions

                            } // done loop over all VelocityBaseFunctions

                        } // done loop over all quadrature points

                        //                                                                                                               // we will call this one
                        // (W)_{i,j} += \int_{\varepsilon\in \Epsilon_{I}^{T}}-\hat{u}_{\sigma}^{U^{+}}(v_{j})\cdot\tau_{i}\cdot n_{T}ds // W's element surface integral
                        //           += \int_{\varepsilon\in \Epsilon_{I}^{T}}-\hat{u}_{\sigma}^{U^{-}}(v_{j})\cdot\tau_{i}\cdot n_{T}ds // W's neighbour surface integral
                        //                                                                                                               // see also "W's boundary integral" below
                        //                                                                                                               // and "W's volume integral" above
//#ifndef NLOG
//                        if ( Wdebug ) {
//                            debugStream.Resume(); // enable logging
//                            debugStream << "      = W surface =======================" << std::endl;
//                            debugStream.Suspend();
//                        }
//#endif
//                        if ( discreteModel_.hasVelocitySigmaFlux() ) {
                            for ( int j = 0; j < numVelocityBaseFunctionsElement; ++j ) {
                                // compute W's element surface integral
                                for ( int i = 0; i < numSigmaBaseFunctionsElement; ++i ) {
                                    double W_i_j = 0.0;
#ifndef NLOG
//                                    if ( ( i == logBaseI ) && ( j == logBaseJ ) ) Woutput = true;
//                                    if ( ( ( i == logBaseI ) && ( j == logBaseJ ) ) && Wdebug ) Woutput = true;
//                                    if ( allOutput ) Woutput = true;
//                                    if ( Wdebug ) Woutput = true;
                                    if ( intersectionOutput && Woutput ) debugStream.Resume(); // enable logging
                                    debugStream << "      = W element ======================" << std::endl;
                                    debugStream << "      basefunctions " << i << " " << j << std::endl;
                                    debugStream << "      faceQuadratureElement.nop() " << faceQuadratureElement.nop() << std::endl;
#endif
                                    // sum over all quadrature points
                                    for ( int quad = 0; quad < faceQuadratureElement.nop(); ++quad ) {
                                        // get x in codim<0> and codim<1> coordinates
                                        const ElementCoordinateType x = faceQuadratureElement.point( quad );
                                        const LocalIntersectionCoordinateType localX = faceQuadratureElement.localPoint( quad );
                                        // get the integration factor
                                        const double elementVolume = intersectionGeoemtry.integrationElement( localX );
                                        // get the quadrature weight
                                        const double integrationWeight = faceQuadratureElement.weight( quad );
//                                        // compute \hat{u}_{\sigma}^{U^{+}}(v_{j})\cdot\tau_{j}\cdot n_{T}
                                        const VelocityRangeType outerNormal = intIt.unitOuterNormal( localX );
                                        SigmaRangeType tau_i( 0.0 );
                                        sigmaBaseFunctionSetElement.evaluate( i, x, tau_i );
                                        VelocityRangeType v_j( 0.0 );
                                        velocityBaseFunctionSetElement.evaluate( j, x, v_j );
                                        VelocityRangeType tau_times_normal( 0.0 );
                                        tau_i.mv( outerNormal, tau_times_normal );
                                        double v_times_tau_times_normal = v_j * tau_times_normal;
//                                        VelocityRangeType u_sigma_u_plus_flux( 0.0 );
//                                        discreteModel_.velocitySigmaFlux(   intIt,
//                                                                            0.0,
//                                                                            localX,
//                                                                            DiscreteModelType::inside,
//                                                                            v_j,
//                                                                            u_sigma_u_plus_flux );
//                                        VelocityRangeType tau_i_times_n_t( 0.0 );
//                                        tau_i.mv( outerNormal, tau_i_times_n_t );
//                                        const double flux_times_tau_i_times_n_t = u_sigma_u_plus_flux * tau_i_times_n_t;
                                        W_i_j += -0.5
                                            * elementVolume
                                            * integrationWeight
                                            * v_times_tau_times_normal;
//#ifndef NLOG
//                                        debugStream << "      - quadPoint " << quad;
//                                        Stuff::printFieldVector( x, "x", debugStream, "        " );
//                                        Stuff::printFieldVector( localX, "localX", debugStream, "          " );
//                                        Stuff::printFieldVector( outerNormal, "outerNormal", debugStream, "          " );
//                                        debugStream << "\n        - elementVolume: " << elementVolume << std::endl;
//                                        debugStream << "        - integrationWeight: " << integrationWeight;
//                                        Stuff::printFieldMatrix( tau_i, "tau_i", debugStream, "        " );
//                                        Stuff::printFieldVector( v_j, "v_j", debugStream, "        " );
//                                        Stuff::printFieldVector( u_sigma_u_plus_flux, "u_sigma_u_plus_flux", debugStream, "        " );
//                                        Stuff::printFieldVector( tau_i_times_n_t, "tau_i_times_n_t", debugStream, "        " );
//                                        debugStream << "\n          - flux_times_tau_i_times_n_t: " << flux_times_tau_i_times_n_t << std::endl;
//                                        debugStream << "          - W_" << i << "_" << j << "+=: " << W_i_j << std::endl;
//#endif
                                    } // done sum over all quadrature points
                                    // if small, should be zero
                                    if ( fabs( W_i_j ) < eps ) {
                                        W_i_j = 0.0;
                                    }
//#ifndef NLOG
//                                    if ( Wdebug && ( W_i_j > 0.0 ) ) {
//                                        debugStream.Resume();
//                                        debugStream << "      W( ";
//                                    }
//#ifdef LOTS_OF_DEBUG
//                                    else if ( Wdebug ) {
//                                        debugStream.Resume();
//                                        debugStream << "      W( ";
//                                    }
//#endif
//#endif
                                    // add to matrix
                                    localWmatrixElement.add( i, j, W_i_j );
//#ifndef NLOG
//                                    if ( Wdebug && ( W_i_j > 0.0 ) ) {
//                                        debugStream << " ) += " << W_i_j << std::endl
//                                                    << "                 entity " << entityNR << ", intersection " << intersectionNR << ", tau_" << i << ", v_" << j << ", W element" << std::endl;
//                                    }
//#ifdef LOTS_OF_DEBUG
//                                    else if ( Wdebug ) {
//                                        debugStream << " ) += " << W_i_j << std::endl
//                                                    << "                 entity " << entityNR << ", intersection " << intersectionNR << ", tau_" << i << ", v_" << j << ", W element" << std::endl;
//                                    }
//#endif
//                                    Woutput = false;
//                                    debugStream.Suspend(); // disable logging
//#endif
                                } // done computing W's element surface integral
                                // compute W's neighbour surface integral
                                for ( int i = 0; i < numSigmaBaseFunctionsNeighbour; ++i ) {
                                    double W_i_j = 0.0;
#ifndef NLOG
//                                    if ( ( i == logBaseI ) && ( j == logBaseJ ) ) Woutput = true;
//                                    if ( ( ( i == logBaseI ) && ( j == logBaseJ ) ) && Wdebug ) Woutput = true;
//                                    if ( allOutput ) Woutput = true;
//                                    if ( Wdebug ) Woutput = true;
                                    if ( intersectionOutput && Woutput ) debugStream.Resume(); // enable logging
                                    debugStream << "      = W neighbour ====================" << std::endl;
                                    debugStream << "      basefunctions " << i << " " << j << std::endl;
                                    debugStream << "      faceQuadratureNeighbour.nop() " << faceQuadratureNeighbour.nop() << std::endl;
#endif
                                    // sum over all quadrature points
                                    for ( int quad = 0; quad < faceQuadratureElement.nop(); ++quad ) {
                                        // get x in codim<0> and codim<1> coordinates
                                        const ElementCoordinateType xInside = faceQuadratureElement.point( quad );
                                        const ElementCoordinateType xOutside = faceQuadratureNeighbour.point( quad );
                                        const LocalIntersectionCoordinateType localX = faceQuadratureElement.localPoint( quad );
                                        // get the integration factor
                                        const double elementVolume = intersectionGeoemtry.integrationElement( localX );
                                        // get the quadrature weight
                                        const double integrationWeight = faceQuadratureElement.weight( quad );
//                                        // compute \hat{u}_{\sigma}^{U^{-}}(v_{j})\cdot\tau_{j}\cdot n_{T}
                                        const VelocityRangeType outerNormal = intIt.unitOuterNormal( localX );
                                        SigmaRangeType tau_i( 0.0 );
                                        sigmaBaseFunctionSetNeighbour.evaluate( i, xOutside, tau_i );
                                        VelocityRangeType v_j( 0.0 );
                                        velocityBaseFunctionSetElement.evaluate( j, xInside, v_j );
                                        VelocityRangeType tau_times_normal( 0.0 );
                                        tau_i.mv( outerNormal, tau_times_normal );
                                        double v_times_tau_times_normal = v_j * tau_times_normal;
//                                        VelocityRangeType u_sigma_u_plus_flux( 0.0 );
//                                        discreteModel_.velocitySigmaFlux(   intIt,
//                                                                            0.0,
//                                                                            localX,
//                                                                            DiscreteModelType::inside,
//                                                                            v_j,
//                                                                            u_sigma_u_plus_flux );
//                                        VelocityRangeType tau_i_times_n_t( 0.0 );
//                                        tau_i.mv( outerNormal, tau_i_times_n_t );
//                                        const double flux_times_tau_i_times_n_t = u_sigma_u_plus_flux * tau_i_times_n_t;
                                        W_i_j += 0.5
                                            * elementVolume
                                            * integrationWeight
                                            * v_times_tau_times_normal;
//#ifndef NLOG
//                                        debugStream << "      - quadPoint " << quad;
////                                        Stuff::printFieldVector( x, "x", debugStream, "        " );
//                                        Stuff::printFieldVector( localX, "localX", debugStream, "          " );
//                                        Stuff::printFieldVector( outerNormal, "outerNormal", debugStream, "          " );
//                                        debugStream << "\n        - elementVolume: " << elementVolume << std::endl;
//                                        debugStream << "        - integrationWeight: " << integrationWeight;
//                                        Stuff::printFieldMatrix( tau_i, "tau_i", debugStream, "        " );
//                                        Stuff::printFieldVector( v_j, "v_j", debugStream, "        " );
//                                        Stuff::printFieldVector( u_sigma_u_plus_flux, "u_sigma_u_plus_flux", debugStream, "        " );
//                                        Stuff::printFieldVector( tau_i_times_n_t, "tau_i_times_n_t", debugStream, "        " );
//                                        debugStream << "\n          - flux_times_tau_i_times_n_t: " << flux_times_tau_i_times_n_t << std::endl;
//                                        debugStream << "          - W_" << i << "_" << j << "+=: " << W_i_j << std::endl;
//#endif
                                    } // done sum over all quadrature points
                                    // if small, should be zero
                                    if ( fabs( W_i_j ) < eps ) {
                                        W_i_j = 0.0;
                                    }
//#ifndef NLOG
//                                    if ( Wdebug && ( W_i_j > 0.0 ) ) {
//                                        debugStream.Resume();
//                                        debugStream << "      W( ";
//                                    }
//#ifdef LOTS_OF_DEBUG
//                                    else if ( Wdebug ) {
//                                        debugStream.Resume();
//                                        debugStream << "      W( ";
//                                    }
//#endif
//#endif
                                    // add to matrix
                                    localWmatrixNeighbour.add( i, j, W_i_j );
//#ifndef NLOG
//                                    if ( Wdebug && ( W_i_j > 0.0 ) ) {
//                                        debugStream << " ) += " << W_i_j << std::endl
//                                                    << "                 entity " << entityNR << ", intersection " << intersectionNR << ", tau_" << i << ", v_" << j << ", W neighbour" << std::endl;
//                                    }
//#ifdef LOTS_OF_DEBUG
//                                    else if ( Wdebug ) {
//                                        debugStream << " ) += " << W_i_j << std::endl
//                                                    << "                 entity " << entityNR << ", intersection " << intersectionNR << ", tau_" << i << ", v_" << j << ", W neighbour" << std::endl;
//                                    }
//#endif
//                                    Woutput = false;
//                                    debugStream.Suspend(); // disable logging
//#endif
                                } // done computing W's neighbour surface integral
                            } // done computing W's surface integrals
//                        }
//
//
//                        //                                                                                                                   // we will call this one
//                        // (X)_{i,j} += \int_{\varepsilon\in\Epsilon_{I}^{T}}-\mu v_{i}\cdot\hat{\sigma}^{\sigma^{+}}(\tau_{j})\cdot n_{t}ds // X's element sourface integral
//                        //           += \int_{\varepsilon\in\Epsilon_{I}^{T}}-\mu v_{i}\cdot\hat{\sigma}^{\sigma^{-}}(\tau_{j})\cdot n_{t}ds // X's neighbour sourface integral
//                        //                                                                                                                   // see also "X's boundary integral" below
//                        //                                                                                                                   // and "X's volume integral" above
////#ifndef NLOG
////                        if ( Xdebug ) {
////                            debugStream.Resume(); // enable logging
////                            debugStream << "      = X surface =======================" << std::endl;
////                            debugStream.Suspend();
////                        }
////#endif
////                        if ( discreteModel_.hasSigmaFlux() ) {
//                            for ( int j = 0; j < numSigmaBaseFunctionsElement; ++j ) {
//                                // compute X's element sourface integral
//                                for ( int i = 0; i < numVelocityBaseFunctionsElement; ++i ) {
//                                    double X_i_j = 0.0;
////#ifndef NLOG
////    //                                if ( ( i == logBaseI ) && ( j == logBaseJ ) ) Xoutput = true;
//////                                    if ( allOutput ) Xoutput = true;
//////                                    if ( Xdebug ) Xoutput = true;
////                                    if ( intersectionOutput && Xoutput ) debugStream.Resume(); // enable logging
////                                    debugStream << "      = X element ======================" << std::endl;
////                                    debugStream << "      basefunctions " << i << " " << j << std::endl;
////                                    debugStream << "      volumeQuadratureElement.nop() " << volumeQuadratureElement.nop() << std::endl;
////#endif
//                                    // sum over all quadrature points
//                                    for ( int quad = 0; quad < faceQuadratureElement.nop(); ++quad ) {
//                                        // get x in codim<0> and codim<1> coordinates
//                                        const ElementCoordinateType x = faceQuadratureElement.point( quad );
//                                        const LocalIntersectionCoordinateType localX = faceQuadratureElement.localPoint( quad );
//                                        // get the integration factor
//                                        const double elementVolume = intersectionGeoemtry.integrationElement( localX );
//                                        // get the quadrature weight
//                                        const double integrationWeight = faceQuadratureElement.weight( quad );
////                                        // compute -\mu v_{i}\cdot\hat{\sigma}^{\sigma^{+}}(\tau_{j})\cdot n_{t}
//                                        const VelocityRangeType outerNormal = intIt.unitOuterNormal( localX );
//                                        SigmaRangeType tau_j( 0.0 );
//                                        sigmaBaseFunctionSetElement.evaluate( j, x, tau_j );
//                                        VelocityRangeType v_i( 0.0 );
//                                        velocityBaseFunctionSetElement.evaluate( i, x, v_i );
//                                        VelocityRangeType tau_times_normal( 0.0 );
//                                        tau_j.mv( outerNormal, tau_times_normal );
//                                        const double v_times_tau_times_normal = v_i * tau_times_normal;
////                                        SigmaRangeType sigma_sigma_plus_flux( 0.0 );
////                                        discreteModel_.sigmaFlux(   intIt,
////                                                                    0.0,
////                                                                    localX,
////                                                                    DiscreteModelType::inside,
////                                                                    tau_j,
////                                                                    sigma_sigma_plus_flux );
////                                        VelocityRangeType flux_times_n_t( 0.0 );
////                                        sigma_sigma_plus_flux.mv( outerNormal, flux_times_n_t );
////                                        const double v_i_times_flux_times_n_t = v_i * flux_times_n_t;
//                                        X_i_j += -0.5
//                                            * elementVolume
//                                            * integrationWeight
//                                            * mu
//                                            * v_times_tau_times_normal;
////#ifndef NLOG
////                                        debugStream << "      - quadPoint " << quad;
////                                        Stuff::printFieldVector( x, "x", debugStream, "        " );
////                                        Stuff::printFieldVector( localX, "localX", debugStream, "          " );
////                                        Stuff::printFieldVector( outerNormal, "outerNormal", debugStream, "          " );
////                                        debugStream << "\n        - elementVolume: " << elementVolume << std::endl;
////                                        debugStream << "        - integrationWeight: " << integrationWeight;
////                                        Stuff::printFieldVector( v_i, "v_i", debugStream, "        " );
////                                        Stuff::printFieldMatrix( tau_j, "tau_j", debugStream, "        " );
////                                        Stuff::printFieldMatrix( sigma_sigma_plus_flux, "sigma_sigma_plus_flux", debugStream, "        " );
////                                        Stuff::printFieldVector( flux_times_n_t, "flux_times_n_t", debugStream, "        " );
////                                        debugStream << "\n          - v_i_times_flux_times_n_t: " << v_i_times_flux_times_n_t << std::endl;
////                                        debugStream << "          - X_" << i << "_" << j << "+=: " << X_i_j << std::endl;
////#endif
//                                    } // done sum over all quadrature points
//                                    // if small, should be zero
//                                    if ( fabs( X_i_j ) < eps ) {
//                                        X_i_j = 0.0;
//                                    }
////#ifndef NLOG
////                                    if ( Xdebug && ( X_i_j > 0.0 ) ) {
////                                        debugStream.Resume();
////                                        debugStream << "      v_" << i << ", tau_" << j << " (element)" << std::endl
////                                                    << "        X( ";
////                                    }
////#ifdef LOTS_OF_DEBUG
////                                    else if ( Xdebug ) {
////                                        debugStream.Resume();
////                                        debugStream << "      v_" << i << ", tau_" << j << " (element)" << std::endl
////                                                    << "        X( ";
////                                    }
////#endif
////#endif
//                                    // add to matrix
//                                    localXmatrixElement.add( i, j, X_i_j );
////#ifndef NLOG
////                                    if ( Xdebug && ( X_i_j > 0.0 ) ) {
////                                        debugStream << " ) += " << X_i_j << std::endl;
////                                    }
////#ifdef LOTS_OF_DEBUG
////                                    else if ( Xdebug ) {
////                                        debugStream << " ) += " << X_i_j << std::endl;
////                                    }
////#endif
////                                    Xoutput = false;
////                                    debugStream.Suspend(); // disable logging
////#endif
//                                } // done computing X's element sourface integral
//                                // compute X's neighbour sourface integral
//                                for ( int i = 0; i < numVelocityBaseFunctionsNeighbour; ++i ) {
//                                    double X_i_j = 0.0;
////#ifndef NLOG
////    //                                if ( ( i == logBaseI ) && ( j == logBaseJ ) ) Xoutput = true;
////    //                                if ( allOutput ) Xoutput = true;
//////                                    if ( Xdebug ) Xoutput = true;
////                                    if ( intersectionOutput && Xoutput ) debugStream.Resume(); // enable logging
////                                    debugStream << "      = X neighbour ====================" << std::endl;
////                                    debugStream << "      basefunctions " << i << " " << j << std::endl;
////                                    debugStream << "      faceQuadratureNeighbour.nop() " << faceQuadratureNeighbour.nop() << std::endl;
////#endif
//                                    // sum over all quadrature points
//                                    for ( int quad = 0; quad < faceQuadratureNeighbour.nop(); ++quad ) {
//                                        // get x codim<0> and codim<1> coordinates
//                                        const ElementCoordinateType xInside = faceQuadratureElement.point( quad );
//                                        const ElementCoordinateType xOutside = faceQuadratureNeighbour.point( quad );
//                                        const LocalIntersectionCoordinateType localX = faceQuadratureNeighbour.localPoint( quad );
//                                        // get the integration factor
//                                        const double elementVolume = intersectionGeoemtry.integrationElement( localX );
//                                        // get the quadrature weight
//                                        const double integrationWeight = faceQuadratureElement.weight( quad );
////                                        // compute -\mu v_{i}\cdot\hat{\sigma}^{\sigma^{-}}(\tau_{j})\cdot n_{t}
//                                        const VelocityRangeType outerNormal = intIt.unitOuterNormal( localX );
//                                        SigmaRangeType tau_j( 0.0 );
//                                        sigmaBaseFunctionSetElement.evaluate( j, xInside, tau_j );
//                                        VelocityRangeType v_i( 0.0 );
//                                        velocityBaseFunctionSetNeighbour.evaluate( i, xOutside, v_i );
//                                        VelocityRangeType tau_times_normal( 0.0 );
//                                        tau_j.mv( outerNormal, tau_times_normal );
//                                        const double v_times_tau_times_normal = v_i * tau_times_normal;
////                                        SigmaRangeType sigma_sigma_minus_flux( 0.0 );
////                                        discreteModel_.sigmaFlux(   intIt,
////                                                                    0.0,
////                                                                    localX,
////                                                                    DiscreteModelType::inside,
////                                                                    tau_j,
////                                                                    sigma_sigma_minus_flux );
////                                        VelocityRangeType flux_times_n_t( 0.0 );
////                                        sigma_sigma_minus_flux.mv( outerNormal, flux_times_n_t );
////                                        const double v_i_times_flux_times_n_t = v_i * flux_times_n_t;
//                                        X_i_j += 0.5
//                                            * elementVolume
//                                            * integrationWeight
//                                            * mu
//                                            * v_times_tau_times_normal;
////#ifndef NLOG
////                                        debugStream << "      - quadPoint " << quad;
//////                                        Stuff::printFieldVector( x, "x", debugStream, "        " );
////                                        Stuff::printFieldVector( localX, "localX", debugStream, "          " );
////                                        Stuff::printFieldVector( outerNormal, "outerNormal", debugStream, "          " );
////                                        debugStream << "\n        - elementVolume: " << elementVolume << std::endl;
////                                        debugStream << "        - integrationWeight: " << integrationWeight;
////                                        Stuff::printFieldVector( v_i, "v_i", debugStream, "        " );
////                                        Stuff::printFieldMatrix( tau_j, "tau_j", debugStream, "        " );
////                                        Stuff::printFieldMatrix( sigma_sigma_minus_flux, "sigma_sigma_minus_flux", debugStream, "        " );
////                                        Stuff::printFieldVector( flux_times_n_t, "flux_times_n_t", debugStream, "        " );
////                                        debugStream << "\n          - v_i_times_flux_times_n_t: " << v_i_times_flux_times_n_t << std::endl;
////                                        debugStream << "          - X_" << i << "_" << j << "+=: " << X_i_j << std::endl;
////#endif
//                                    } // done sum over all quadrature points
//                                    // if small, should be zero
//                                    if ( fabs( X_i_j ) < eps ) {
//                                        X_i_j = 0.0;
//                                    }
////#ifndef NLOG
////                                    if ( Xdebug && ( X_i_j > 0.0 ) ) {
////                                        debugStream.Resume();
////                                        debugStream << "      v_" << i << ", tau_" << j << " (neighbour)" << std::endl
////                                                    << "        X( ";
////                                    }
////#ifdef LOTS_OF_DEBUG
////                                    else if ( Xdebug ) {
////                                        debugStream.Resume();
////                                        debugStream << "      v_" << i << ", tau_" << j << " (neighbour)" << std::endl
////                                                    << "        X( ";
////                                    }
////#endif
////#endif
//                                    // add to matrix
//                                    localXmatrixNeighbour.add( i, j, X_i_j );
////#ifndef NLOG
////                                    if ( Xdebug && ( X_i_j > 0.0 ) ) {
////                                        debugStream << " ) += " << X_i_j << std::endl;
////                                    }
////#ifdef LOTS_OF_DEBUG
////                                    else if ( Xdebug ) {
////                                        debugStream << " ) += " << X_i_j << std::endl;
////                                    }
////#endif
////                                    Xoutput = false;
////                                    debugStream.Suspend(); // disable logging
////#endif
//                                } // done computing X's neighbour sourface integral
//                            } // done computing X's sourface integrals
////                        }
//
                        //                                                                                                         // we call this one
                        // (Y)_{i,j} += \int_{\varepsilon\in\Epsilon_{I}^{T}}-\mu v_{i}\cdot\hat{\sigma}^{U{+}}(v{j})\cdot n_{t}ds // Y's element surface integral
                        //           += \int_{\varepsilon\in\Epsilon_{I}^{T}}-\mu v_{i}\cdot\hat{\sigma}^{U{-}}(v{j})\cdot n_{t}ds // Y's neighbour surface integral
                        //                                                                                                         // see also "Y's boundary integral" below
//#ifndef NLOG
//                        if ( Ydebug ) {
//                            debugStream.Resume(); // enable logging
//                            debugStream << "      = Y surface =======================" << std::endl;
//                            debugStream.Suspend();
//                        }
//#endif
//                        if ( discreteModel_.hasSigmaFlux() ) {
                            for ( int j = 0; j < numVelocityBaseFunctionsElement; ++j ) {
                                // compute Y's element surface integral
                                for ( int i = 0; i < numVelocityBaseFunctionsElement; ++i ) {
                                    double Y_i_j = 0.0;
#ifndef NLOG
//                                    if ( ( i == logBaseI ) && ( j == logBaseJ ) ) Youtput = true;
//                                    if ( allOutput ) Youtput = true;
//                                    if ( Ydebug ) Youtput = true;
                                    if ( intersectionOutput && Youtput ) debugStream.Resume(); // enable logging
                                    debugStream << "      = Y element ======================" << std::endl;
                                    debugStream << "      basefunctions " << i << " " << j << std::endl;
                                    debugStream << "      faceQuadratureElement.nop() " << faceQuadratureElement.nop() << std::endl;
#endif
                                    // sum over all quadrature points
                                    for ( int quad = 0; quad < faceQuadratureElement.nop(); ++quad ) {
                                        // get x codim<0> and codim<1> coordinates
                                        const ElementCoordinateType x = faceQuadratureElement.point( quad );
                                        const LocalIntersectionCoordinateType localX = faceQuadratureElement.localPoint( quad );
                                        // get the integration factor
                                        const double elementVolume = intersectionGeoemtry.integrationElement( localX );
                                        // get the quadrature weight
                                        const double integrationWeight = faceQuadratureElement.weight( quad );
                                        // compute -\mu v_{i}\cdot\hat{\sigma}^{U{+}}(v{j})\cdot n_{t}
                                        const VelocityRangeType outerNormal = intIt.unitOuterNormal( localX );
                                        VelocityRangeType v_j( 0.0 );
                                        velocityBaseFunctionSetElement.evaluate( j, x, v_j );
                                        VelocityRangeType v_i( 0.0 );
                                        velocityBaseFunctionSetElement.evaluate( i, x, v_i );
                                        const double v_times_v = v_i * v_j;
//                                        SigmaRangeType sigma_u_plus_flux( 0.0 );
//                                        discreteModel_.sigmaFlux(   intIt,
//                                                                    0.0,
//                                                                    localX,
//                                                                    DiscreteModelType::inside,
//                                                                    v_j,
//                                                                    sigma_u_plus_flux );
//                                        VelocityRangeType flux_times_n_t( 0.0 );
//                                        sigma_u_plus_flux.mv( outerNormal, flux_times_n_t );
//                                        const double v_i_times_flux_times_n_t = v_i * flux_times_n_t;
                                        Y_i_j += ( 1.0 / getLenghtOfIntersection( intIt ) )
                                            * elementVolume
                                            * integrationWeight
                                            * mu
                                            * v_times_v;
//#ifndef NLOG
//                                        debugStream << "      - quadPoint " << quad;
//                                        Stuff::printFieldVector( x, "x", debugStream, "        " );
//                                        Stuff::printFieldVector( localX, "localX", debugStream, "          " );
//                                        Stuff::printFieldVector( outerNormal, "outerNormal", debugStream, "          " );
//                                        debugStream << "\n        - elementVolume: " << elementVolume << std::endl;
//                                        debugStream << "        - integrationWeight: " << integrationWeight;
//                                        Stuff::printFieldVector( v_i, "v_i", debugStream, "        " );
//                                        Stuff::printFieldVector( v_j, "v_j", debugStream, "        " );
//                                        Stuff::printFieldMatrix( sigma_u_plus_flux, "sigma_u_plus_flux", debugStream, "        " );
//                                        Stuff::printFieldVector( flux_times_n_t, "flux_times_n_t", debugStream, "        " );
//                                        debugStream << "\n          - v_i_times_flux_times_n_t: " << v_i_times_flux_times_n_t << std::endl;
//                                        debugStream << "          - Y_" << i << "_" << j << "+=: " << Y_i_j << std::endl;
//#endif
                                    } // done sum over all quadrature points
                                    // if small, should be zero
                                    if ( fabs( Y_i_j ) < eps ) {
                                        Y_i_j = 0.0;
                                    }
//#ifndef NLOG
//                                    if ( Ydebug && ( Y_i_j > 0.0 ) ) {
//                                        debugStream.Resume();
//                                        debugStream << "      v_" << i << ", v_" << j << " (element)" << std::endl
//                                                    << "        Y( ";
//                                    }
//#ifdef LOTS_OF_DEBUG
//                                    else if ( Ydebug ) {
//                                        debugStream.Resume();
//                                        debugStream << "      v_" << i << ", v_" << j << " (element)" << std::endl
//                                                    << "        Y( ";
//                                    }
//#endif
//#endif
                                    // add to matrix
                                    localYmatrixElement.add( i, j, Y_i_j );
//#ifndef NLOG
//                                    if ( Ydebug && ( Y_i_j > 0.0 ) ) {
//                                        debugStream << " ) += " << Y_i_j << std::endl;
//                                    }
//#ifdef LOTS_OF_DEBUG
//                                    else if ( Ydebug ) {
//                                        debugStream << " ) += " << Y_i_j << std::endl;
//                                    }
//#endif
//                                    Youtput = false;
//                                    debugStream.Suspend(); // disable logging
//#endif
                                } // done computing Y's element surface integral
                                // compute Y's neighbour surface integral
                                for ( int i = 0; i < numVelocityBaseFunctionsNeighbour; ++i ) {
                                    double Y_i_j = 0.0;
//#ifndef NLOG
//    //                                if ( ( i == logBaseI ) && ( j == logBaseJ ) ) Youtput = true;
//    //                                if ( allOutput ) Youtput = true;
////                                    if ( Ydebug ) Youtput = true;
//                                    if ( intersectionOutput && Youtput ) debugStream.Resume(); // enable logging
//                                    debugStream << "      = Y neighbour ====================" << std::endl;
//                                    debugStream << "      basefunctions " << i << " " << j << std::endl;
//                                    debugStream << "      faceQuadratureNeighbour.nop() " << faceQuadratureNeighbour.nop() << std::endl;
//#endif
                                    // sum over all quadrature points
                                    for ( int quad = 0; quad < faceQuadratureNeighbour.nop(); ++quad ) {
                                        // get x codim<0> and codim<1> coordinates
                                        const ElementCoordinateType xInside = faceQuadratureElement.point( quad );
                                        const ElementCoordinateType xOutside = faceQuadratureNeighbour.point( quad );
                                        const LocalIntersectionCoordinateType localX = faceQuadratureNeighbour.localPoint( quad );
                                        // get the integration factor
                                        const double elementVolume = intersectionGeoemtry.integrationElement( localX );
                                        // get the quadrature weight
                                        const double integrationWeight = faceQuadratureNeighbour.weight( quad );
//                                        // compute -\mu v_{i}\cdot\hat{\sigma}^{U{-}}(v{j})\cdot n_{t}
                                        const VelocityRangeType outerNormal = intIt.unitOuterNormal( localX );
                                        VelocityRangeType v_i( 0.0 );
                                        velocityBaseFunctionSetNeighbour.evaluate( i, xOutside, v_i );
                                        VelocityRangeType v_j( 0.0 );
                                        velocityBaseFunctionSetElement.evaluate( j, xInside, v_j );
                                        const double v_times_v = v_i * v_j;
//                                        SigmaRangeType sigma_u_minus_flux( 0.0 );
//                                        discreteModel_.sigmaFlux(   intIt,
//                                                                    0.0,
//                                                                    localX,
//                                                                    DiscreteModelType::inside,
//                                                                    v_j,
//                                                                    sigma_u_minus_flux );
//                                        VelocityRangeType flux_times_n_t( 0.0 );
//                                        sigma_u_minus_flux.mv( outerNormal, flux_times_n_t );
//                                        const double v_i_times_flux_times_n_t = v_i * flux_times_n_t;
                                        Y_i_j += ( -1.0 / getLenghtOfIntersection( intIt ) )
                                            * elementVolume
                                            * integrationWeight
                                            * mu
                                            * v_times_v;
//#ifndef NLOG
//                                        debugStream << "      - quadPoint " << quad;
////                                        Stuff::printFieldVector( x, "x", debugStream, "        " );
//                                        Stuff::printFieldVector( localX, "localX", debugStream, "          " );
//                                        Stuff::printFieldVector( outerNormal, "outerNormal", debugStream, "          " );
//                                        debugStream << "\n        - elementVolume: " << elementVolume << std::endl;
//                                        debugStream << "        - integrationWeight: " << integrationWeight;
//                                        Stuff::printFieldVector( v_i, "v_i", debugStream, "        " );
//                                        Stuff::printFieldVector( v_j, "v_j", debugStream, "        " );
//                                        Stuff::printFieldMatrix( sigma_u_minus_flux, "sigma_u_minus_flux", debugStream, "        " );
//                                        Stuff::printFieldVector( flux_times_n_t, "flux_times_n_t", debugStream, "        " );
//                                        debugStream << "\n          - v_i_times_flux_times_n_t: " << v_i_times_flux_times_n_t << std::endl;
//                                        debugStream << "          - Y_" << i << "_" << j << "+=: " << Y_i_j << std::endl;
//#endif
                                    } // done sum over all quadrature points
                                    // if small, should be zero
                                    if ( fabs( Y_i_j ) < eps ) {
                                        Y_i_j = 0.0;
                                    }
//#ifndef NLOG
//                                    if ( Ydebug && ( Y_i_j > 0.0 ) ) {
//                                        debugStream.Resume();
//                                        debugStream << "      v_" << i << ", v_" << j << " (neighbour)" << std::endl
//                                                    << "        Y( ";
//                                    }
//#ifdef LOTS_OF_DEBUG
//                                    else if ( Ydebug ) {
//                                        debugStream.Resume();
//                                        debugStream << "      v_" << i << ", v_" << j << " (neighbour)" << std::endl
//                                                    << "        Y( ";
//                                    }
//#endif
//#endif
                                    // add to matrix
                                    localYmatrixNeighbour.add( i, j, Y_i_j );
//#ifndef NLOG
//                                    if ( Ydebug && ( Y_i_j > 0.0 ) ) {
//                                        debugStream << " ) += " << Y_i_j << std::endl;
//                                    }
//#ifdef LOTS_OF_DEBUG
//                                    else if ( Ydebug ) {
//                                        debugStream << " ) += " << Y_i_j << std::endl;
//                                    }
//#endif
//                                    Youtput = false;
//                                    debugStream.Suspend(); // disable logging
//#endif
                                } // done computing Y's neighbour surface integral
                            } // done computing Y's surface integrals
//                        }
//
//                        //                                                                                                  // we will call this one
//                        // (Z)_{i,j} += \int_{\varepsilon\in\Epsilon_{I}^{T}}\hat{p}^{P^{+}}(q_{j})\cdot v_{i}\cdot n_{T}ds // Z's element surface integral
//                        //           += \int_{\varepsilon\in\Epsilon_{I}^{T}}\hat{p}^{P^{-}}(q_{j})\cdot v_{i}\cdot n_{T}ds // Z's neighbour surface integral
//                        //                                                                                                  // see also "Z's boundary integral" below
//                        //                                                                                                  // and "Z's volume integral" above
////#ifndef NLOG
////                        if ( Zdebug ) {
////                            debugStream.Resume(); // enable logging
////                            debugStream << "      = Z surface =======================" << std::endl;
////                            debugStream.Suspend();
////                        }
////#endif
////                        if ( discreteModel_.hasPressureFlux() ) {
//                            for ( int j = 0; j < numPressureBaseFunctionsElement; ++j ) {
//                                // compute Z's element surface integral
//                                for ( int i = 0; i < numVelocityBaseFunctionsElement; ++i ) {
//                                    double Z_i_j = 0.0;
////#ifndef NLOG
////    //                                if ( ( i == logBaseI ) && ( j == logBaseJ ) ) Zoutput = true;
////    //                                if ( allOutput ) Zoutput = true;
//////                                    if ( Zdebug ) Zoutput = true;
////                                    if ( intersectionOutput && Zoutput ) debugStream.Resume(); // enable logging
////                                    debugStream << "      = Z element ======================" << std::endl;
////                                    debugStream << "      basefunctions " << i << " " << j << std::endl;
////                                    debugStream << "      faceQuadratureElement.nop() " << faceQuadratureElement.nop() << std::endl;
////#endif
//                                    // sum over all quadrature points
//                                    for ( int quad = 0; quad < faceQuadratureElement.nop(); ++quad ) {
//                                        // get x codim<0> and codim<1> coordinates
//                                        const ElementCoordinateType x = faceQuadratureElement.point( quad );
//                                        const LocalIntersectionCoordinateType localX = faceQuadratureElement.localPoint( quad );
//                                        // get the integration factor
//                                        const double elementVolume = intersectionGeoemtry.integrationElement( localX );
//                                        // get the quadrature weight
//                                        const double integrationWeight = faceQuadratureElement.weight( quad );
////                                        // compute \hat{p}^{P^{+}}(q_{j})\cdot v_{i}\cdot n_{T}
//                                        const VelocityRangeType outerNormal = intIt.unitOuterNormal( localX );
//                                        VelocityRangeType v_i( 0.0 );
//                                        velocityBaseFunctionSetElement.evaluate( i, x, v_i );
//                                        PressureRangeType q_j( 0.0 );
//                                        pressureBaseFunctionSetElement.evaluate( j, x, q_j );
//                                        const double v_times_normal = v_i * outerNormal;
//                                        const double q_times_v_times_normal = q_j * v_times_normal;
////                                        PressureRangeType p_p_plus_flux( 0.0 );
////                                        discreteModel_.pressureFlux(    intIt,
////                                                                        0.0,
////                                                                        localX,
////                                                                        DiscreteModelType::inside,
////                                                                        q_j,
////                                                                        p_p_plus_flux );
//                                        Z_i_j += 0.5
//                                            * elementVolume
//                                            * integrationWeight
//                                            * q_times_v_times_normal;
////#ifndef NLOG
////                                        debugStream << "      - quadPoint " << quad;
////                                        Stuff::printFieldVector( x, "x", debugStream, "        " );
////                                        Stuff::printFieldVector( localX, "localX", debugStream, "          " );
////                                        Stuff::printFieldVector( outerNormal, "outerNormal", debugStream, "          " );
////                                        debugStream << "\n        - elementVolume: " << elementVolume << std::endl;
////                                        debugStream << "        - integrationWeight: " << integrationWeight;
////                                        Stuff::printFieldVector( v_i, "v_i", debugStream, "        " );
////                                        Stuff::printFieldVector( q_j, "q_j", debugStream, "        " );
////                                        debugStream << "\n          - v_i_times_n_t: " << v_i_times_n_t << std::endl;
////                                        debugStream << "\n          - p_p_plus_flux: " << p_p_plus_flux << std::endl;
////                                        debugStream << "          - Z_" << i << "_" << j << "+=: " << Z_i_j << std::endl;
////#endif
//                                    } // done sum over all quadrature points
//                                    // if small, should be zero
//                                    if ( fabs( Z_i_j ) < eps ) {
//                                        Z_i_j = 0.0;
//                                    }
////#ifndef NLOG
////                                    if ( Zdebug && ( Z_i_j > 0.0 ) ) {
////                                        debugStream.Resume();
////                                        debugStream << "      v_" << i << ", q_" << j << " (element)" << std::endl
////                                                    << "        Z( ";
////                                    }
////#ifdef LOTS_OF_DEBUG
////                                    else if ( Zdebug ) {
////                                        debugStream.Resume();
////                                        debugStream << "      v_" << i << ", q_" << j << " (element)" << std::endl
////                                                    << "        Z( ";
////                                    }
////#endif
////#endif
//                                    // add to matrix
//                                    localZmatrixElement.add( i, j, Z_i_j );
////#ifndef NLOG
////                                    if ( Zdebug && ( Z_i_j > 0.0 ) ) {
////                                        debugStream << " ) += " << Z_i_j << std::endl;
////                                    }
////#ifdef LOTS_OF_DEBUG
////                                    else if ( Zdebug ) {
////                                        debugStream << " ) += " << Z_i_j << std::endl;
////                                    }
////#endif
////                                    Zoutput = false;
////                                    debugStream.Suspend(); // disable logging
////#endif
//                                } // done computing Z's element surface integral
//                                // compute Z's neighbour surface integral
//                                for ( int i = 0; i < numVelocityBaseFunctionsNeighbour; ++i ) {
//                                    double Z_i_j = 0.0;
////#ifndef NLOG
////    //                                if ( ( i == logBaseI ) && ( j == logBaseJ ) ) Zoutput = true;
////    //                                if ( allOutput ) Zoutput = true;
//////                                    if ( Zdebug ) Zoutput = true;
////                                    if ( intersectionOutput && Zoutput ) debugStream.Resume(); // enable logging
////                                    debugStream << "      = Z neighbour ====================" << std::endl;
////                                    debugStream << "      basefunctions " << i << " " << j << std::endl;
////                                    debugStream << "      faceQuadratureNeighbour.nop() " << faceQuadratureNeighbour.nop() << std::endl;
////#endif
//                                    // sum over all quadrature points
//                                    for ( int quad = 0; quad < faceQuadratureNeighbour.nop(); ++quad ) {
//                                        // get x codim<0> and codim<1> coordinates
//                                        const ElementCoordinateType xInside = faceQuadratureElement.point( quad );
//                                        const ElementCoordinateType xOutside = faceQuadratureNeighbour.point( quad );
//                                        const LocalIntersectionCoordinateType localX = faceQuadratureNeighbour.localPoint( quad );
//                                        // get the integration factor
//                                        const double elementVolume = intersectionGeoemtry.integrationElement( localX );
//                                        // get the quadrature weight
//                                        const double integrationWeight = faceQuadratureNeighbour.weight( quad );
////                                        // compute \hat{p}^{P^{+}}(q_{j})\cdot v_{i}\cdot n_{T}
//                                        const VelocityRangeType outerNormal = intIt.unitOuterNormal( localX );
//                                        VelocityRangeType v_i( 0.0 );
//                                        velocityBaseFunctionSetNeighbour.evaluate( i, xOutside, v_i );
//                                        PressureRangeType q_j( 0.0 );
//                                        pressureBaseFunctionSetElement.evaluate( j, xInside, q_j );
//                                        const double v_times_normal = v_i * outerNormal;
//                                        const double q_times_v_times_normal = q_j * v_times_normal;
////                                        PressureRangeType p_p_minus_flux( 0.0 );
////                                        discreteModel_.pressureFlux(    intIt,
////                                                                        0.0,
////                                                                        localX,
////                                                                        DiscreteModelType::inside,
////                                                                        q_j,
////                                                                        p_p_minus_flux );
//                                        Z_i_j += -0.5
//                                            * elementVolume
//                                            * integrationWeight
//                                            * q_times_v_times_normal;
////#ifndef NLOG
////                                        debugStream << "      - quadPoint " << quad;
//////                                        Stuff::printFieldVector( x, "x", debugStream, "        " );
////                                        Stuff::printFieldVector( localX, "localX", debugStream, "          " );
////                                        Stuff::printFieldVector( outerNormal, "outerNormal", debugStream, "          " );
////                                        debugStream << "\n        - elementVolume: " << elementVolume << std::endl;
////                                        debugStream << "        - integrationWeight: " << integrationWeight;
////                                        Stuff::printFieldVector( v_i, "v_i", debugStream, "        " );
////                                        Stuff::printFieldVector( q_j, "q_j", debugStream, "        " );
////                                        debugStream << "\n          - v_i_times_n_t: " << v_i_times_n_t << std::endl;
////                                        debugStream << "\n          - p_p_minus_flux: " << p_p_minus_flux << std::endl;
////                                        debugStream << "          - Z_" << i << "_" << j << "+=: " << Z_i_j << std::endl;
////#endif
//                                    } // done sum over all quadrature points
//                                    // if small, should be zero
//                                    if ( fabs( Z_i_j ) < eps ) {
//                                        Z_i_j = 0.0;
//                                    }
////#ifndef NLOG
////                                    if ( Zdebug && ( Z_i_j > 0.0 ) ) {
////                                        debugStream.Resume();
////                                        debugStream << "      v_" << i << ", q_" << j << " (neighbour)" << std::endl
////                                                    << "        Z( ";
////                                    }
////#ifdef LOTS_OF_DEBUG
////                                    else if ( Zdebug ) {
////                                        debugStream.Resume();
////                                        debugStream << "      v_" << i << ", q_" << j << " (neighbour)" << std::endl
////                                                    << "        Z( ";
////                                    }
////#endif
////#endif
//                                    // add to matrix
//                                    localZmatrixNeighbour.add( i, j, Z_i_j );
////#ifndef NLOG
////                                    if ( Zdebug && ( Z_i_j > 0.0 ) ) {
////                                        debugStream << " ) += " << Z_i_j << std::endl;
////                                    }
////#ifdef LOTS_OF_DEBUG
////                                    else if ( Zdebug ) {
////                                        debugStream << " ) += " << Z_i_j << std::endl;
////                                    }
////#endif
////                                    Zoutput = false;
////                                    debugStream.Suspend(); // disable logging
////#endif
//                                } // done computing Z's neighbour surface integral
//                            } // done computing Z's surface integrals
////                        }
//
                        //                                                                                                // we will call this one
                        // (E)_{i,j} += \int_{\varepsilon\in\Epsilon_{I}^{T}}\hat{u}_{p}^{U^{+}}(v_{j})\cdot n_{T}q_{i}ds // E's element surface integral
                        //           += \int_{\varepsilon\in\Epsilon_{I}^{T}}\hat{u}_{p}^{U^{-}}(v_{j})\cdot n_{T}q_{i}ds // E's neighbour surface integral
                        //                                                                                                // see also "E's boundary integral" below
                        //                                                                                                // and "E's volume integral" above
#ifndef NLOG
                        if ( Edebug ) {
                            debugStream.Resume(); // enable logging
                            debugStream << "      = E surface =======================" << std::endl;
                            debugStream.Suspend();
                        }
#endif
//                        if ( discreteModel_.hasVelocityPressureFlux() ) {
                            for ( int j = 0; j < numVelocityBaseFunctionsElement; ++j ) {
                                // compute E's element surface integral
                                for ( int i = 0; i < numPressureBaseFunctionsElement; ++i ) {
                                    double E_i_j = 0.0;
#ifndef NLOG
//                                    if ( ( i == logBaseI ) && ( j == logBaseJ ) ) Eoutput = true;
//                                    if ( allOutput ) Eoutput = true;
//                                    if ( Edebug ) Eoutput = true;
                                    if ( intersectionOutput && Eoutput ) debugStream.Resume(); // enable logging
                                    debugStream << "      = E element ======================" << std::endl;
                                    debugStream << "      basefunctions " << i << " " << j << std::endl;
                                    debugStream << "      faceQuadratureElement.nop() " << faceQuadratureElement.nop() << std::endl;
#endif
                                    // sum over all quadrature points
                                    for ( int quad = 0; quad < faceQuadratureElement.nop(); ++quad ) {
                                        // get x codim<0> and codim<1> coordinates
                                        const ElementCoordinateType x = faceQuadratureElement.point( quad );
                                        const LocalIntersectionCoordinateType localX = faceQuadratureElement.localPoint( quad );
                                        // get the integration factor
                                        const double elementVolume = intersectionGeoemtry.integrationElement( localX );
                                        // get the quadrature weight
                                        const double integrationWeight = faceQuadratureElement.weight( quad );
//                                        // compute \hat{u}_{p}^{U^{+}}(v_{j})\cdot n_{T}q_{i}
                                        const VelocityRangeType outerNormal = intIt.unitOuterNormal( localX );
                                        VelocityRangeType v_j( 0.0 );
                                        velocityBaseFunctionSetElement.evaluate( j, x, v_j );
                                        PressureRangeType q_i( 0.0 );
                                        pressureBaseFunctionSetElement.evaluate( i, x, q_i );
                                        const double v_times_normal = v_j * outerNormal;
                                        const double q_times_v_times_normal = q_i * v_times_normal;
//                                        VelocityRangeType u_p_u_plus_flux( 0.0 );
//                                        discreteModel_.velocityPressureFlux(    intIt,
//                                                                                0.0,
//                                                                                localX,
//                                                                                DiscreteModelType::inside,
//                                                                                v_j,
//                                                                                u_p_u_plus_flux );
//                                        const double flux_times_n_t = u_p_u_plus_flux * outerNormal;
//                                        const double flux_times_n_t_times_q_i = q_i * flux_times_n_t;
                                        E_i_j += 0.5
                                            * elementVolume
                                            * integrationWeight
                                            * q_times_v_times_normal;
//#ifndef NLOG
//                                        debugStream << "      - quadPoint " << quad;
//                                        Stuff::printFieldVector( x, "x", debugStream, "        " );
//                                        Stuff::printFieldVector( localX, "localX", debugStream, "          " );
//                                        Stuff::printFieldVector( outerNormal, "outerNormal", debugStream, "          " );
//                                        debugStream << "\n        - elementVolume: " << elementVolume << std::endl;
//                                        debugStream << "        - integrationWeight: " << integrationWeight;
//                                        Stuff::printFieldVector( q_i, "q_i", debugStream, "        " );
//                                        Stuff::printFieldVector( v_j, "v_j", debugStream, "        " );
//                                        Stuff::printFieldVector( u_p_u_plus_flux, "u_p_u_plus_flux", debugStream, "        " );
//                                        debugStream << "\n          - flux_times_n_t: " << flux_times_n_t << std::endl;
//                                        debugStream << "\n          - flux_times_n_t_times_q_i: " << flux_times_n_t_times_q_i << std::endl;
//                                        debugStream << "          - E_" << i << "_" << j << "+=: " << E_i_j << std::endl;
//#endif
                                    } // done sum over all quadrature points
                                    // if small, should be zero
                                    if ( fabs( E_i_j ) < eps ) {
                                        E_i_j = 0.0;
                                    }
//#ifndef NLOG
//                                    if ( Edebug && ( E_i_j > 0.0 ) ) {
//                                        debugStream.Resume();
//                                        debugStream << "      q_" << i << ", v_" << j << " (element)" << std::endl
//                                                    << "        E( ";
//                                    }
//#ifdef LOTS_OF_DEBUG
//                                    else if ( Edebug ) {
//                                        debugStream.Resume();
//                                        debugStream << "      q_" << i << ", v_" << j << " (element)" << std::endl
//                                                    << "        E( ";
//                                    }
//#endif
//#endif
                                    // add to matrix
                                    localEmatrixElement.add( i, j, E_i_j );
//#ifndef NLOG
//                                    if ( Edebug && ( E_i_j > 0.0 ) ) {
//                                        debugStream << " ) += " << E_i_j << std::endl;
//                                    }
//#ifdef LOTS_OF_DEBUG
//                                    else if ( Edebug ) {
//                                        debugStream << " ) += " << E_i_j << std::endl;
//                                    }
//#endif
//                                    Eoutput = false;
//                                    debugStream.Suspend(); // disable logging
//#endif
                                } // done computing E's element surface integral
                                // compute E's neighbour surface integral
                                for ( int i = 0; i < numPressureBaseFunctionsNeighbour; ++i ) {
                                    double E_i_j = 0.0;
#ifndef NLOG
    //                                if ( ( i == logBaseI ) && ( j == logBaseJ ) ) Eoutput = true;
    //                                if ( allOutput ) Eoutput = true;
//                                    if ( Edebug ) Eoutput = true;
                                    if ( intersectionOutput && Eoutput ) debugStream.Resume(); // enable logging
                                    debugStream << "      = E neighbour ====================" << std::endl;
                                    debugStream << "      basefunctions " << i << " " << j << std::endl;
                                    debugStream << "      faceQuadratureNeighbour.nop() " << faceQuadratureNeighbour.nop() << std::endl;
#endif
                                    // sum over all quadrature points
                                    for ( int quad = 0; quad < faceQuadratureNeighbour.nop(); ++quad ) {
                                        // get x codim<0> and codim<1> coordinates
                                        const ElementCoordinateType xInside = faceQuadratureElement.point( quad );
                                        const ElementCoordinateType xOutside = faceQuadratureNeighbour.point( quad );
                                        const LocalIntersectionCoordinateType localX = faceQuadratureNeighbour.localPoint( quad );
                                        // get the integration factor
                                        const double elementVolume = intersectionGeoemtry.integrationElement( localX );
                                        // get the quadrature weight
                                        const double integrationWeight = faceQuadratureNeighbour.weight( quad );
//                                        // compute \hat{u}_{p}^{U^{-}}(v_{j})\cdot n_{T}q_{i}
                                        const VelocityRangeType outerNormal = intIt.unitOuterNormal( localX );
                                        VelocityRangeType v_j( 0.0 );
                                        velocityBaseFunctionSetElement.evaluate( j, xInside, v_j );
                                        PressureRangeType q_i( 0.0 );
                                        pressureBaseFunctionSetNeighbour.evaluate( i, xOutside, q_i );
                                        const double v_times_normal = v_j * outerNormal;
                                        const double q_times_v_times_normal = q_i * v_times_normal;
//                                        VelocityRangeType u_p_u_minus_flux( 0.0 );
//                                        discreteModel_.velocityPressureFlux(    intIt,
//                                                                                0.0,
//                                                                                localX,
//                                                                                DiscreteModelType::inside,
//                                                                                v_j,
//                                                                                u_p_u_minus_flux );
//                                        const double flux_times_n_t = u_p_u_minus_flux * outerNormal;
//                                        const double flux_times_n_t_times_q_i = q_i * flux_times_n_t;
                                        E_i_j += -0.5
                                            * elementVolume
                                            * integrationWeight
                                            * q_times_v_times_normal;
//#ifndef NLOG
//                                        debugStream << "      - quadPoint " << quad;
////                                        Stuff::printFieldVector( x, "x", debugStream, "        " );
//                                        Stuff::printFieldVector( localX, "localX", debugStream, "          " );
//                                        Stuff::printFieldVector( outerNormal, "outerNormal", debugStream, "          " );
//                                        debugStream << "\n        - elementVolume: " << elementVolume << std::endl;
//                                        debugStream << "        - integrationWeight: " << integrationWeight;
//                                        Stuff::printFieldVector( q_i, "q_i", debugStream, "        " );
//                                        Stuff::printFieldVector( v_j, "v_j", debugStream, "        " );
//                                        Stuff::printFieldVector( u_p_u_minus_flux, "u_p_u_minus_flux", debugStream, "        " );
//                                        debugStream << "\n          - flux_times_n_t: " << flux_times_n_t << std::endl;
//                                        debugStream << "\n          - flux_times_n_t_times_q_i: " << flux_times_n_t_times_q_i << std::endl;
//                                        debugStream << "          - E_" << i << "_" << j << "+=: " << E_i_j << std::endl;
//#endif
                                    } // done sum over all quadrature points
                                    // if small, should be zero
                                    if ( fabs( E_i_j ) < eps ) {
                                        E_i_j = 0.0;
                                    }
//#ifndef NLOG
//                                    if ( Edebug && ( E_i_j > 0.0 ) ) {
//                                        debugStream.Resume();
//                                        debugStream << "      q_" << i << ", v_" << j << " (neighbour)" << std::endl
//                                                    << "        E( ";
//                                    }
//#ifdef LOTS_OF_DEBUG
//                                    else if ( Edebug ) {
//                                        debugStream.Resume();
//                                        debugStream << "      q_" << i << ", v_" << j << " (neighbour)" << std::endl
//                                                    << "        E( ";
//                                    }
//#endif
//#endif
                                    // add to matrix
                                    localEmatrixNeighbour.add( i, j, E_i_j );
//#ifndef NLOG
//                                    if ( Edebug && ( E_i_j > 0.0 ) ) {
//                                        debugStream << " ) += " << E_i_j << std::endl;
//                                    }
//#ifdef LOTS_OF_DEBUG
//                                    else if ( Edebug ) {
//                                        debugStream << " ) += " << E_i_j << std::endl;
//                                    }
//#endif
//                                    Eoutput = false;
//                                    debugStream.Suspend(); // disable logging
//#endif
                                } // done computing E's neighbour surface integral
                            } // done computing E's surface integrals
//                        }
//
//                        //                                                                                                // we will call this one
//                        // (R)_{i,j} += \int_{\varepsilon\in\Epsilon_{I}^{T}}\hat{u}_{p}^{P^{+}}(q_{j})\cdot n_{T}q_{i}ds // R's element surface integral
//                        //           += \int_{\varepsilon\in\Epsilon_{I}^{T}}\hat{u}_{p}^{P^{-}}(q_{j})\cdot n_{T}q_{i}ds // R's neighbour surface integral
//                        //                                                                                                // see also "R's boundary integral" below
////#ifndef NLOG
////                        if ( Rdebug ) {
////                            debugStream.Resume(); // enable logging
////                            debugStream << "      = R surface =======================" << std::endl;
////                            debugStream.Suspend();
////                        }
////#endif
////                        if ( discreteModel_.hasVelocityPressureFlux() ) {
//                            for ( int j = 0; j < numPressureBaseFunctionsElement; ++j ) {
//                                // compute R's element surface integral
//                                for ( int i = 0; i < numPressureBaseFunctionsElement; ++i ) {
//                                    double R_i_j = 0.0;
////#ifndef NLOG
////    //                                if ( ( i == logBaseI ) && ( j == logBaseJ ) ) Routput = true;
////    //                                if ( allOutput ) Routput = true;
//////                                    if ( Rdebug ) Routput = true;
////                                    if ( intersectionOutput && Routput ) debugStream.Resume(); // enable logging
////                                    debugStream << "      = R element ======================" << std::endl;
////                                    debugStream << "      basefunctions " << i << " " << j << std::endl;
////                                    debugStream << "      faceQuadratureElement.nop() " << faceQuadratureElement.nop() << std::endl;
////#endif
//                                    // sum over all quadrature points
//                                    for ( int quad = 0; quad < faceQuadratureElement.nop(); ++quad ) {
//                                        // get x codim<0> and codim<1> coordinates
//                                        const ElementCoordinateType x = faceQuadratureElement.point( quad );
//                                        const LocalIntersectionCoordinateType localX = faceQuadratureElement.localPoint( quad );
//                                        // get the integration factor
//                                        const double elementVolume = intersectionGeoemtry.integrationElement( localX );
//                                        // get the quadrature weight
//                                        const double integrationWeight = faceQuadratureElement.weight( quad );
////                                        // compute \hat{u}_{p}^{P^{+}}(q_{j})\cdot n_{T}q_{i}
//                                        const VelocityRangeType outerNormal = intIt.unitOuterNormal( localX );
//                                        PressureRangeType q_j( 0.0 );
//                                        pressureBaseFunctionSetElement.evaluate( j, x, q_j );
//                                        PressureRangeType q_i( 0.0 );
//                                        pressureBaseFunctionSetElement.evaluate( i, x, q_i );
//                                        const double q_times_q = q_i * q_j;
////                                        VelocityRangeType u_p_p_plus_flux( 0.0 );
////                                        discreteModel_.velocityPressureFlux(    intIt,
////                                                                                0.0,
////                                                                                localX,
////                                                                                DiscreteModelType::inside,
////                                                                                q_j,
////                                                                                u_p_p_plus_flux );
////                                        const double flux_times_n_t = u_p_p_plus_flux
////                                            * outerNormal;
////                                        const double flux_times_n_t_times_q_i = q_i * flux_times_n_t;
//                                        R_i_j += getLenghtOfIntersection( intIt )
//                                            * elementVolume
//                                            * integrationWeight
//                                            * q_times_q;
////#ifndef NLOG
////                                        debugStream << "      - quadPoint " << quad;
////                                        Stuff::printFieldVector( x, "x", debugStream, "        " );
////                                        Stuff::printFieldVector( localX, "localX", debugStream, "          " );
////                                        Stuff::printFieldVector( outerNormal, "outerNormal", debugStream, "          " );
////                                        debugStream << "\n        - elementVolume: " << elementVolume << std::endl;
////                                        debugStream << "        - integrationWeight: " << integrationWeight;
////                                        Stuff::printFieldVector( q_i, "q_i", debugStream, "        " );
////                                        Stuff::printFieldVector( q_j, "q_j", debugStream, "        " );
////                                        Stuff::printFieldVector( u_p_p_plus_flux, "u_p_p_plus_flux", debugStream, "        " );
////                                        debugStream << "\n          - flux_times_n_t: " << flux_times_n_t << std::endl;
////                                        debugStream << "\n          - flux_times_n_t_times_q_i: " << flux_times_n_t_times_q_i << std::endl;
////                                        debugStream << "          - R_" << i << "_" << j << "+=: " << R_i_j << std::endl;
////#endif
//                                    } // done sum over all quadrature points
//                                    // if small, should be zero
//                                    if ( fabs( R_i_j ) < eps ) {
//                                        R_i_j = 0.0;
//                                    }
////#ifndef NLOG
////                                    if ( Rdebug && ( R_i_j > 0.0 ) ) {
////                                        debugStream.Resume();
////                                        debugStream << "      q_" << i << ", q_" << j << " (element)" << std::endl
////                                                    << "        R( ";
////                                    }
////#ifdef LOTS_OF_DEBUG
////                                    else if ( Rdebug ) {
////                                        debugStream.Resume();
////                                        debugStream << "      q_" << i << ", q_" << j << " (element)" << std::endl
////                                                    << "        R( ";
////                                    }
////#endif
////#endif
//                                    // add to matrix
//                                    localRmatrixElement.add( i, j, R_i_j );
////#ifndef NLOG
////                                    if ( Rdebug && ( R_i_j > 0.0 ) ) {
////                                        debugStream << " ) += " << R_i_j << std::endl;
////                                    }
////#ifdef LOTS_OF_DEBUG
////                                    else if ( Rdebug ) {
////                                        debugStream << " ) += " << R_i_j << std::endl;
////                                    }
////#endif
////                                    Routput = false;
////                                    debugStream.Suspend(); // disable logging
////#endif
//                                } // done computing R's element surface integral
////                                // compute R's neighbour surface integral
//                                for ( int i = 0; i < numPressureBaseFunctionsNeighbour; ++i ) {
//                                    double R_i_j = 0.0;
////#ifndef NLOG
////    //                                if ( ( i == logBaseI ) && ( j == logBaseJ ) ) Routput = true;
////    //                                if ( allOutput ) Routput = true;
//////                                    if ( Rdebug ) Routput = true;
////                                    if ( intersectionOutput && Routput ) debugStream.Resume(); // enable logging
////                                    debugStream << "      = R neighbour ====================" << std::endl;
////                                    debugStream << "      basefunctions " << i << " " << j << std::endl;
////                                    debugStream << "      faceQuadratureNeighbour.nop() " << faceQuadratureNeighbour.nop() << std::endl;
////#endif
//                                    // sum over all quadrature points
//                                    for ( int quad = 0; quad < faceQuadratureNeighbour.nop(); ++quad ) {
//                                        // get x codim<0> and codim<1> coordinates
//                                        const ElementCoordinateType xInside = faceQuadratureElement.point( quad );
//                                        const ElementCoordinateType xOutside = faceQuadratureNeighbour.point( quad );
//                                        const LocalIntersectionCoordinateType localX = faceQuadratureNeighbour.localPoint( quad );
//                                        // get the integration factor
//                                        const double elementVolume = intersectionGeoemtry.integrationElement( localX );
//                                        // get the quadrature weight
//                                        const double integrationWeight = faceQuadratureNeighbour.weight( quad );
////                                        // compute \hat{u}_{p}^{P^{-}}(q_{j})\cdot n_{T}q_{i}
//                                        const VelocityRangeType outerNormal = intIt.unitOuterNormal( localX );
//                                        PressureRangeType q_j( 0.0 );
//                                        pressureBaseFunctionSetElement.evaluate( j, xInside, q_j );
//                                        PressureRangeType q_i( 0.0 );
//                                        pressureBaseFunctionSetNeighbour.evaluate( i, xOutside, q_i );
//                                        const double q_times_q = q_i * q_j;
////                                        VelocityRangeType u_p_p_minus_flux( 0.0 );
////                                        discreteModel_.velocityPressureFlux(    intIt,
////                                                                                0.0,
////                                                                                localX,
////                                                                                DiscreteModelType::outside,
////                                                                                q_j,
////                                                                                u_p_p_minus_flux );
////                                        const double flux_times_n_t = u_p_p_minus_flux
////                                            * outerNormal;
////                                        const double flux_times_n_t_times_q_i = q_i * flux_times_n_t;
//                                        R_i_j += getLenghtOfIntersection( intIt )
//                                            * elementVolume
//                                            * integrationWeight
//                                            * q_times_q;
////#ifndef NLOG
////                                        debugStream << "      - quadPoint " << quad;
//////                                        Stuff::printFieldVector( x, "x", debugStream, "        " );
////                                        Stuff::printFieldVector( localX, "localX", debugStream, "          " );
////                                        Stuff::printFieldVector( outerNormal, "outerNormal", debugStream, "          " );
////                                        debugStream << "\n        - elementVolume: " << elementVolume << std::endl;
////                                        debugStream << "        - integrationWeight: " << integrationWeight;
////                                        Stuff::printFieldVector( q_i, "q_i", debugStream, "        " );
////                                        Stuff::printFieldVector( q_j, "q_j", debugStream, "        " );
////                                        Stuff::printFieldVector( u_p_p_minus_flux, "u_p_p_minus_flux", debugStream, "        " );
////                                        debugStream << "\n          - flux_times_n_t: " << flux_times_n_t << std::endl;
////                                        debugStream << "\n          - flux_times_n_t_times_q_i: " << flux_times_n_t_times_q_i << std::endl;
////                                        debugStream << "          - R_" << i << "_" << j << "+=: " << R_i_j << std::endl;
////#endif
//                                    } // done sum over all quadrature points
//                                    // if small, should be zero
//                                    if ( fabs( R_i_j ) < eps ) {
//                                        R_i_j = 0.0;
//                                    }
////#ifndef NLOG
////                                    if ( Rdebug && ( R_i_j > 0.0 ) ) {
////                                        debugStream.Resume();
////                                        debugStream << "      q_" << i << ", q_" << j << " (neighbour)" << std::endl
////                                                    << "        R( ";
////                                    }
////#ifdef LOTS_OF_DEBUG
////                                    else if ( Rdebug ) {
////                                        debugStream.Resume();
////                                        debugStream << "      q_" << i << ", q_" << j << " (neighbour)" << std::endl
////                                                    << "        R( ";
////                                    }
////#endif
////#endif
//                                    // add to matrix
//                                    localRmatrixNeighbour.add( i, j, R_i_j );
////#ifndef NLOG
////                                    if ( Rdebug && ( R_i_j > 0.0 ) ) {
////                                        debugStream << " ) += " << R_i_j << std::endl;
////                                    }
////#ifdef LOTS_OF_DEBUG
////                                    else if ( Rdebug ) {
////                                        debugStream << " ) += " << R_i_j << std::endl;
////                                    }
////#endif
////                                    Routput = false;
////                                    debugStream.Suspend(); // disable logging
////#endif
//                                } // done computing R's neighbour surface integral
//                            } // done computing R's surface integrals
////                        }
                    } // done with those inside the grid

                    // if we are on the boundary of the grid
                    if ( !intIt.neighbor() && intIt.boundary() ) {

                        // do loop over all quadrature points
                        for ( int quad = 0; quad < faceQuadratureElement.nop(); ++quad ) {

                            // quadrture point in reference element coordinates
                            const ElementCoordinateType x = faceQuadratureElement.point( quad );
                            // quadrature point in world coordinates
                            const ElementCoordinateType xWorld = geometry.global( x );
                            // quadrature point in codim< 1 > reference element coordinates
                            const LocalIntersectionCoordinateType xLocal = faceQuadratureElement.localPoint( quad );
                            // quadrature weight
                            const double integrationWeight = faceQuadratureElement.weight( quad );
                            // integration factor
                            const double faceVolume = intersectionGeoemtry.integrationElement( xLocal );
                            // unit outer normal
                            const VelocityRangeType normal = intIt.unitOuterNormal( faceQuadratureElement.localPoint( quad ) );

                            // flux constants
                            const double c11 = 1.0 / faceVolume;
                            const double d11 = faceVolume;

                            // dirichlet values
                            VelocityRangeType gD( 0.0 );
                            discreteModel_.dirichletData( 0.0, xWorld, gD );

                            // evaluate fluxes
                            VelocityRangeType minusC11TimesgD = gD;
                            minusC11TimesgD *= c11;

                            // do loop over all VelocityBaseFunctions
                            for ( int l = 0; l < numVelocityBaseFunctionsElement; ++l ) {

                                // evaluate the VelocityBaseFunctionSet
                                VelocityRangeType v( 0.0 );
                                velocityBaseFunctionSetElement.evaluate( l, x, v );

//                                // compute H2rhs and add to local function
//                                localH2rhs[ l ] += integrationWeight
//                                    * faceVolume
//                                    * velocityBaseFunctionSetElement.evaluateSingle( l, x, minusC11TimesgD );

                                // do loop over all SigmaBaseFunctions
                                for ( int m = 0; m < numSigmaBaseFunctionsElement; ++m ) {

                                    // evaluate the SigmaBaseFunctionSet
                                    SigmaRangeType tau( 0.0 );
                                    sigmaBaseFunctionSetElement.evaluate( m, x, tau );

                                    // compute X_bnd
                                    VelocityRangeType tauTimesNormal( 0.0 );
                                    tau.umv( normal, tauTimesNormal );
                                    const double X_bnd = -1.0
                                        * integrationWeight
                                        * faceVolume
                                        * velocityBaseFunctionSetElement.evaluateSingle( l, x, tauTimesNormal );

                                    // add to local matrices
                                    localXmatrixElement.add( l, m, X_bnd );

//                                    // compute H1rhs
//                                    localH1rhs[ m ] += integrationWeight
//                                        * faceVolume
//                                        * sigmaBaseFunctionSetElement.evaluateSingle( m, x, dyadicProduct( gD, normal) );

                                } // done loop over all SigmaBaseFunctions

                                // do loop over all VelocityBaseFunction
                                for ( int ll = 0; ll < numVelocityBaseFunctionsElement; ++ll ) {

                                    // calculate Y_bnd
                                    const double Y_bnd = c11
                                        * integrationWeight
                                        * faceVolume
                                        * velocityBaseFunctionSetElement.evaluateSingle( ll, x, v );

                                    // add to local matrices
//                                    localYmatrixElement.add( ll, l, Y_bnd );

                                } // done loop over all VelocityBaseFunction

                                // do loop over all PressureBaseFunctions
                                for ( int k = 0; k < numPressureBaseFunctionsElement; ++k ) {

                                    // evaluate the PressureBaseFunctionSet
                                    PressureRangeType q( 0.0 );
                                    pressureBaseFunctionSetElement.evaluate( k, x, q );

                                    // compute Z_bnd
                                    VelocityRangeType normalTimesQ = normal;
                                    normalTimesQ *= q;
                                    const double Z_bnd = integrationWeight
                                        * faceVolume
                                        * velocityBaseFunctionSetElement.evaluateSingle( l, x, normalTimesQ );

                                    // add to local matrices
                                    localZmatrixElement.add( l, k, Z_bnd );

//                                    // do H3rhs assembling only once
//                                    if ( l == 0 ) {
//
//                                        // compute H3rhs
//                                        const double gDTimesNormal = gD * normal;
//                                        localH3rhs[ k ] += integrationWeight
//                                            * faceVolume
//                                            * pressureBaseFunctionSetElement.evaluateSingle( k, x, gDTimesNormal );
//
//                                    } // done H3rhs assembling only once

                                } // done loop over all PressureBaseFunctions

                            } // done loop over all VelocityBaseFunctions

                        } // done loop over all quadrature points

//
//                        //                                                                                                               // we wil call this one
//                        // (W)_{i,j} += \int_{\varepsilon\in \Epsilon_{D}^{T}}-\hat{u}_{\sigma}^{U^{+}}(v_{j})\cdot\tau_{i}\cdot n_{T}ds // W's boundary integral
//                        //                                                                                                               // see also "W's volume integral", "W's element surface integral" and "W's neighbour surface integral" above
////#ifndef NLOG
////                        if ( Wdebug ) {
////                            debugStream.Resume(); // enable logging
////                            debugStream << "      = W boundary =====================" << std::endl;
////                            debugStream.Suspend();
////                        }
////#endif
////                        if ( discreteModel_.hasVelocitySigmaFlux() ) {
////                            for ( int i = 0; i < numSigmaBaseFunctionsElement; ++i ) {
////                                for ( int j = 0; j < numVelocityBaseFunctionsElement; ++j ) {
////                                    double W_i_j = 0.0;
////#ifndef NLOG
////                                    if ( ( ( i == logBaseI ) && ( j == logBaseJ ) ) && Wdebug ) Woutput = true;
////    //                                if ( allOutput ) Woutput = true;
//////                                    if ( Wprint ) Woutput = true;
////                                    if ( intersectionOutput && Woutput ) debugStream.Resume(); // enable logging
////                                    debugStream << "      = W boundary =====================" << std::endl;
////                                    debugStream << "      basefunctions " << i << " " << j << std::endl;
////                                    debugStream << "      faceQuadratureElement.nop() " << faceQuadratureElement.nop() << std::endl;
////#endif
////                                    // sum over all quadrature points
////                                    for ( int quad = 0; quad < faceQuadratureElement.nop(); ++quad ) {
////                                        // get x codim<0> and codim<1> coordinates
////                                        const ElementCoordinateType x = faceQuadratureElement.point( quad );
////                                        const LocalIntersectionCoordinateType localX = faceQuadratureElement.localPoint( quad );
////                                        // get the integration factor
////                                        const double elementVolume = intersectionGeoemtry.integrationElement( localX );
////                                        // get the quadrature weight
////                                        const double integrationWeight = faceQuadratureElement.weight( quad );
////                                        // compute \hat{u}_{\sigma}^{U^{+}}(v_{j})\cdot\tau_{j}\cdot n_{T}
////                                        const VelocityRangeType outerNormal = intIt.unitOuterNormal( localX );
////                                        SigmaRangeType tau_i( 0.0 );
////                                        VelocityRangeType v_j( 0.0 );
////                                        VelocityRangeType u_sigma_u_plus_flux( 0.0 );
////                                        sigmaBaseFunctionSetElement.evaluate( i, x, tau_i );
////                                        velocityBaseFunctionSetElement.evaluate( j, x, v_j );
////                                        discreteModel_.velocitySigmaBoundaryFlux(   intIt,
////                                                                                    0.0,
////                                                                                    localX,
////                                                                                    v_j,
////                                                                                    u_sigma_u_plus_flux );
////                                        VelocityRangeType tau_i_times_n_t( 0.0 );
////                                        tau_i.mv( outerNormal, tau_i_times_n_t );
////                                        const double flux_times_tau_i_times_n_t = u_sigma_u_plus_flux * tau_i_times_n_t;
////                                        W_i_j += -1.0
////                                            * elementVolume
////                                            * integrationWeight
////                                            * flux_times_tau_i_times_n_t;
////#ifndef NLOG
////                                        debugStream << "      - quadPoint " << quad;
////                                        Stuff::printFieldVector( x, "x", debugStream, "        " );
////                                        Stuff::printFieldVector( localX, "localX", debugStream, "          " );
////                                        Stuff::printFieldVector( outerNormal, "outerNormal", debugStream, "          " );
////                                        debugStream << "\n        - elementVolume: " << elementVolume << std::endl;
////                                        debugStream << "        - integrationWeight: " << integrationWeight;
////                                        Stuff::printFieldMatrix( tau_i, "tau_i", debugStream, "        " );
////                                        Stuff::printFieldVector( v_j, "v_j", debugStream, "        " );
////                                        Stuff::printFieldVector( u_sigma_u_plus_flux, "u_sigma_u_plus_flux", debugStream, "        " );
////                                        Stuff::printFieldVector( tau_i_times_n_t, "tau_i_times_n_t", debugStream, "        " );
////                                        debugStream << "\n          - flux_times_tau_i_times_n_t: " << flux_times_tau_i_times_n_t << std::endl;
////                                        debugStream << "          - W_" << i << "_" << j << "+=: " << W_i_j << std::endl;
////#endif
////                                    } // done sum over all quadrature points
////                                    // if small, should be zero
////                                    if ( fabs( W_i_j ) < eps ) {
////                                        W_i_j = 0.0;
////                                    }
////#ifndef NLOG
////                                    if ( Wdebug && ( W_i_j > 0.0 ) ) {
////                                        debugStream.Resume();
////                                        debugStream << "      W( ";
////                                    }
////#ifdef LOTS_OF_DEBUG
////                                    else if ( Wdebug ) {
////                                        debugStream.Resume();
////                                        debugStream << "      W( ";
////                                    }
////#endif
////#endif
////                                    // add to matrix
////                                    localWmatrixElement.add( i, j, W_i_j );
////#ifndef NLOG
////                                    if ( Wdebug && ( W_i_j > 0.0 ) ) {
////                                        debugStream << " ) += " << W_i_j << std::endl
////                                                    << "                 entity " << entityNR << ", intersection " << intersectionNR << ", tau_" << i << ", v_" << j << ", W boundary" << std::endl;
////                                    }
////#ifdef LOTS_OF_DEBUG
////                                    else if ( Wdebug ) {
////                                        debugStream << " ) += " << W_i_j << std::endl
////                                                    << "                 entity " << entityNR << ", intersection " << intersectionNR << ", tau_" << i << ", v_" << j << ", W neighbour" << std::endl;
////                                    }
////#endif
////                                    Woutput = false;
////                                    debugStream.Suspend(); // disable logging
////#endif
////                                }
////                            } // done computing W's boundary integral
////                        }

                        //                                                                                                    // we will call this one
                        // (H1)_{j} = \int_{\varepsilon\in\Epsilon_{D}^{T}}\hat{u}_{\sigma}^{RHS}()\cdot\tau_{j}\cdot n_{T}ds // H1's boundary integral
                        if ( discreteModel_.hasVelocitySigmaFlux() ) {
                            for ( int j = 0; j < numSigmaBaseFunctionsElement; ++j ) {
                                double H1_j = 0.0;
#ifndef NLOG
    //                            if ( j == logBaseJ ) H1output = true;
//                                if ( allOutput ) H1output = true;
                                if ( H1debug ) H1output = true;
                                if ( intersectionOutput && H1output ) debugStream.Resume(); // enable logging
                                debugStream << "      = H1 boundary ====================" << std::endl;
                                debugStream << "      basefunction " << j << std::endl;
                                debugStream << "      faceQuadratureElement.nop() " << faceQuadratureElement.nop() << std::endl;
#endif
                                // sum over all quadrature points
                                for ( int quad = 0; quad < faceQuadratureElement.nop(); ++quad ) {
                                    // get x codim<0> and codim<1> coordinates
                                    const ElementCoordinateType x = faceQuadratureElement.point( quad );
                                    const VelocityRangeType globalX = geometry.global( x );
                                    const LocalIntersectionCoordinateType localX = faceQuadratureElement.localPoint( quad );
                                    // get the integration factor
                                    const double elementVolume = intersectionGeoemtry.integrationElement( localX );
                                    // get the quadrature weight
                                    const double integrationWeight = faceQuadratureElement.weight( quad );
                                    // compute \hat{u}_{\sigma}^{RHS}()\cdot\tau_{j}\cdot n_{T}
                                    const VelocityRangeType outerNormal = intIt.unitOuterNormal( localX );
                                    SigmaRangeType tau_j( 0.0 );
                                    sigmaBaseFunctionSetElement.evaluate( j, x, tau_j );
                                    VelocityRangeType tau_j_times_n_t( 0.0 );
                                    tau_j.mv( outerNormal, tau_j_times_n_t );
                                    VelocityRangeType u_sigma_rhs_flux( 0.0 );
//                                    discreteModel_. velocitySigmaBoundaryFlux(  intIt,
//                                                                                0.0,
//                                                                                localX,
//                                                                                u_sigma_rhs_flux );
                                    // some preparations
                                    const VelocityRangeType xIntersectionGlobal = intIt.intersectionSelfLocal().global( localX );
                                    const VelocityRangeType xWorld = geometry.global( xIntersectionGlobal );
                                    // contribution to rhs ( from inside entity )
                                    discreteModel_.dirichletData( 0.0, xWorld,  u_sigma_rhs_flux );
                                    const double flux_times_tau_j_times_n_t = u_sigma_rhs_flux * tau_j_times_n_t;
                                    H1_j += elementVolume
                                        * integrationWeight
                                        * flux_times_tau_j_times_n_t;
#ifndef NLOG
                                    debugStream << "      - quadPoint " << quad;
                                    Stuff::printFieldVector( x, "x", debugStream, "        " );
                                    Stuff::printFieldVector( localX, "localX", debugStream, "        " );
                                    Stuff::printFieldVector( globalX, "globalX", debugStream, "        " );
                                    Stuff::printFieldVector( outerNormal, "outerNormal", debugStream, "        " );
                                    debugStream << "\n        - elementVolume: " << elementVolume << std::endl;
                                    debugStream << "        - integrationWeight: " << integrationWeight;
                                    Stuff::printFieldMatrix( tau_j, "tau_j", debugStream, "        " );
                                    Stuff::printFieldVector( u_sigma_rhs_flux, "u_sigma_rhs_flux", debugStream, "        " );
                                    Stuff::printFieldVector( tau_j_times_n_t, "tau_j_times_n_t", debugStream, "        " );
                                    debugStream << "\n        - flux_times_tau_j_times_n_t: " << flux_times_tau_j_times_n_t << std::endl;
                                    debugStream << "        - H1_" << j << "+=: " << H1_j << std::endl;
#endif
                                } // done sum over all quadrature points
                                // if small, should be zero
                                if ( fabs( H1_j ) < eps ) {
                                    H1_j = 0.0;
                                }
                                // add to rhs
                                localH1rhs[ j ] += H1_j;
#ifndef NLOG
                                H1output = false;
                                debugStream.Suspend(); // disable logging
#endif
                            } // done computing H1's boundary integral
                        }
//
//                        //                                                                                                                   // we will call this one
//                        // (X)_{i,j} += \int_{\varepsilon\in\Epsilon_{D}^{T}}-\mu v_{i}\cdot\hat{\sigma}^{\sigma^{+}}(\tau_{j})\cdot n_{t}ds // X's boundary integral
//                        //                                                                                                                   // see also "X's volume integral", "X's element surface integral" and "X's neighbour surface integral" above
////#ifndef NLOG
////                        if ( Xdebug ) {
////                            debugStream.Resume(); // enable logging
////                            debugStream << "      = X boundary =====================" << std::endl;
////                            debugStream.Suspend();
////                        }
////#endif
////                        if ( discreteModel_.hasSigmaFlux() ) {
//                            for ( int i = 0; i < numVelocityBaseFunctionsElement; ++i ) {
//                                for ( int j = 0; j < numSigmaBaseFunctionsElement; ++j ) {
//                                    double X_i_j = 0.0;
////#ifndef NLOG
////    //                                if ( ( i == logBaseI ) && ( j == logBaseJ ) ) Xoutput = true;
//////                                    if ( allOutput ) Xoutput = true;
//////                                    if ( Xdebug ) Xoutput = true;
////                                    if ( intersectionOutput && Xoutput ) debugStream.Resume(); // enable logging
////                                    debugStream << "      = X boundary =====================" << std::endl;
////                                    debugStream << "      basefunctions " << i << " " << j << std::endl;
////                                    debugStream << "      faceQuadratureElement.nop() " << faceQuadratureElement.nop() << std::endl;
////#endif
//                                    // sum over all quadrature points
//                                    for ( int quad = 0; quad < faceQuadratureElement.nop(); ++quad ) {
//                                        // get x codim<0> and codim<1> coordinates
//                                        const ElementCoordinateType x = faceQuadratureElement.point( quad );
//                                        const LocalIntersectionCoordinateType localX = faceQuadratureElement.localPoint( quad );
//                                        // get the integration factor
//                                        const double elementVolume = intersectionGeoemtry.integrationElement( localX );
//                                        // get the quadrature weight
//                                        const double integrationWeight = faceQuadratureElement.weight( quad );
////                                        // compute -\mu v_{i}\cdot\hat{\sigma}^{\sigma^{+}}(\tau_{j})\cdot n_{t}
//                                        const VelocityRangeType outerNormal = intIt.unitOuterNormal( localX );
//                                        SigmaRangeType tau_j( 0.0 );
//                                        sigmaBaseFunctionSetElement.evaluate( j, x, tau_j );
//                                        VelocityRangeType v_i( 0.0 );
//                                        velocityBaseFunctionSetElement.evaluate( i, x, v_i );
//                                        VelocityRangeType tau_times_normal( 0.0 );
//                                        tau_j.mv( outerNormal, tau_times_normal );
//                                        const double v_times_tau_times_normal
//                                            = v_i * tau_times_normal;
////                                        SigmaRangeType sigma_sigma_plus_flux( 0.0 );
////                                        discreteModel_.sigmaBoundaryFlux(   intIt,
////                                                                            0.0,
////                                                                            localX,
////                                                                            tau_j,
////                                                                            sigma_sigma_plus_flux );
////                                        VelocityRangeType flux_times_n_t( 0.0 );
////                                        sigma_sigma_plus_flux.mv( outerNormal, flux_times_n_t );
////                                        const double v_i_times_flux_times_n_t = v_i * flux_times_n_t;
//                                        X_i_j += -1.0
//                                            * elementVolume
//                                            * integrationWeight
//                                            * mu
//                                            * v_times_tau_times_normal;
////#ifndef NLOG
////                                        debugStream << "      - quadPoint " << quad;
////                                        Stuff::printFieldVector( x, "x", debugStream, "        " );
////                                        Stuff::printFieldVector( localX, "localX", debugStream, "          " );
////                                        Stuff::printFieldVector( outerNormal, "outerNormal", debugStream, "          " );
////                                        debugStream << "\n        - elementVolume: " << elementVolume << std::endl;
////                                        debugStream << "        - integrationWeight: " << integrationWeight;
////                                        Stuff::printFieldVector( v_i, "v_i", debugStream, "        " );
////                                        Stuff::printFieldMatrix( tau_j, "tau_j", debugStream, "        " );
////                                        Stuff::printFieldMatrix( sigma_sigma_plus_flux, "sigma_sigma_plus_flux", debugStream, "        " );
////                                        Stuff::printFieldVector( flux_times_n_t, "flux_times_n_t", debugStream, "        " );
////                                        debugStream << "\n          - v_i_times_flux_times_n_t: " << v_i_times_flux_times_n_t << std::endl;
////                                        debugStream << "          - X_" << i << "_" << j << "+=: " << X_i_j << std::endl;
////#endif
//                                    } // done sum over all quadrature points
//                                    // if small, should be zero
//                                    if ( fabs( X_i_j ) < eps ) {
//                                        X_i_j = 0.0;
//                                    }
////#ifndef NLOG
////                                    if ( Xdebug && ( X_i_j > 0.0 ) ) {
////                                        debugStream.Resume();
////                                        debugStream << "      v_" << i << ", tau_" << j << std::endl
////                                                    << "        X( ";
////                                    }
////#ifdef LOTS_OF_DEBUG
////                                    else if ( Xdebug ) {
////                                        debugStream.Resume();
////                                        debugStream << "      v_" << i << ", tau_" << j << std::endl
////                                                    << "        X( ";
////                                    }
////#endif
////#endif
//                                    // add to matrix
//                                    localXmatrixElement.add( i, j, X_i_j );
////#ifndef NLOG
////                                    if ( Xdebug && ( X_i_j > 0.0 ) ) {
////                                        debugStream << " ) += " << X_i_j << std::endl;
////                                    }
////#ifdef LOTS_OF_DEBUG
////                                    else if ( Xdebug ) {
////                                        debugStream << " ) += " << X_i_j << std::endl;
////                                    }
////#endif
////                                    Xoutput = false;
////                                    debugStream.Suspend(); // disable logging
////#endif
//                                }
//                            } // done computing X's boundary integral
////                        }
//
                        //                                                                                                           // we will call this one
                        // (Y)_{i,j} += \int_{\varepsilon\in\Epsilon_{D}^{T}}-\mu v_{i}\cdot\hat{\sigma}^{U^{+}}(v_{j})\cdot n_{t}ds // Y's boundary integral
                        //                                                                                                           // see also "Y's element surface integral" and "Y's neighbour surface integral" above
#ifndef NLOG
                        if ( Ydebug ) {
                            debugStream.Resume(); // enable logging
                            debugStream << "      = Y boundary =====================" << std::endl;
                            debugStream.Suspend();
                        }
#endif
//                        if ( discreteModel_.hasSigmaFlux() ) {
                            for ( int i = 0; i < numVelocityBaseFunctionsElement; ++i ) {
                                for ( int j = 0; j < numVelocityBaseFunctionsElement; ++j ) {
                                    double Y_i_j = 0.0;
#ifndef NLOG
//                                    if ( ( i == logBaseI ) && ( j == logBaseJ ) ) Youtput = true;
//                                    if ( allOutput ) Youtput = true;
//                                    if ( Ydebug ) Youtput = true;
                                    if ( intersectionOutput && Youtput ) debugStream.Resume(); // enable logging
                                    debugStream << "      = Y boundary =====================" << std::endl;
                                    debugStream << "      basefunctions " << i << " " << j << std::endl;
                                    debugStream << "      faceQuadratureElement.nop() " << faceQuadratureElement.nop() << std::endl;
#endif
                                    // sum over all quadrature points
                                    for ( int quad = 0; quad < faceQuadratureElement.nop(); ++quad ) {
                                        // get x codim<0> and codim<1> coordinates
                                        const ElementCoordinateType x = faceQuadratureElement.point( quad );
                                        const LocalIntersectionCoordinateType localX = faceQuadratureElement.localPoint( quad );
                                        // get the integration factor
                                        const double elementVolume = intersectionGeoemtry.integrationElement( localX );
                                        // get the quadrature weight
                                        const double integrationWeight = faceQuadratureElement.weight( quad );
                                        // compute -\mu v_{i}\cdot\hat{\sigma}^{U^{+}}(v_{j})\cdot n_{t}
                                        const VelocityRangeType outerNormal = intIt.unitOuterNormal( localX );
                                        VelocityRangeType v_j( 0.0 );
                                        velocityBaseFunctionSetElement.evaluate( j, x, v_j );
                                        VelocityRangeType v_i( 0.0 );
                                        velocityBaseFunctionSetElement.evaluate( i, x, v_i );
                                        const double v_times_v = v_i * v_j;
//                                        SigmaRangeType sigma_u_plus_flux( 0.0 );
//                                        discreteModel_.sigmaBoundaryFlux(   intIt,
//                                                                            0.0,
//                                                                            localX,
//                                                                            v_j,
//                                                                            sigma_u_plus_flux );
//                                        VelocityRangeType flux_times_n_t( 0.0 );
//                                        sigma_u_plus_flux.mv( outerNormal, flux_times_n_t );
//                                        const double v_i_times_flux_times_n_t = v_i * flux_times_n_t;
                                        Y_i_j += ( 1.0 / getLenghtOfIntersection( intIt ) )
                                            * elementVolume
                                            * integrationWeight
                                            * mu
                                            * v_times_v;
//#ifndef NLOG
//                                        debugStream << "      - quadPoint " << quad;
//                                        Stuff::printFieldVector( x, "x", debugStream, "        " );
//                                        Stuff::printFieldVector( localX, "localX", debugStream, "          " );
//                                        Stuff::printFieldVector( outerNormal, "outerNormal", debugStream, "          " );
//                                        debugStream << "\n        - elementVolume: " << elementVolume << std::endl;
//                                        debugStream << "        - integrationWeight: " << integrationWeight;
//                                        Stuff::printFieldVector( v_i, "v_i", debugStream, "        " );
//                                        Stuff::printFieldVector( v_j, "v_j", debugStream, "        " );
//                                        Stuff::printFieldMatrix( sigma_u_plus_flux, "sigma_u_plus_flux", debugStream, "        " );
//                                        Stuff::printFieldVector( flux_times_n_t, "flux_times_n_t", debugStream, "        " );
//                                        debugStream << "\n          - v_i_times_flux_times_n_t: " << v_i_times_flux_times_n_t << std::endl;
//                                        debugStream << "          - Y_" << i << "_" << j << "+=: " << Y_i_j << std::endl;
//#endif
                                    } // done sum over all quadrature points
                                    // if small, should be zero
                                    if ( fabs( Y_i_j ) < eps ) {
                                        Y_i_j = 0.0;
                                    }
//#ifndef NLOG
//                                    if ( Ydebug && ( Y_i_j > 0.0 ) ) {
//                                        debugStream.Resume();
//                                        debugStream << "      v_" << i << ", v_" << j << std::endl
//                                                    << "        Y( ";
//                                    }
//#ifdef LOTS_OF_DEBUG
//                                    else if ( Ydebug ) {
//                                        debugStream.Resume();
//                                        debugStream << "      v_" << i << ", v_" << j << std::endl
//                                                    << "        Y( ";
//                                    }
//#endif
//#endif
                                    // add to matrix
                                    localYmatrixElement.add( i, j, Y_i_j );
//#ifndef NLOG
//                                    if ( Ydebug && ( Y_i_j > 0.0 ) ) {
//                                        debugStream << " ) += " << Y_i_j << std::endl;
//                                    }
//#ifdef LOTS_OF_DEBUG
//                                    else if ( Ydebug ) {
//                                        debugStream << " ) += " << Y_i_j << std::endl;
//                                    }
//#endif
//                                    Youtput = false;
//                                    debugStream.Suspend(); // disable logging
//#endif
                                }
                            } // done computing Y's boundary integral
//                        }
//
//                        //                                                                                                  // we will call this one
//                        // (Z)_{i,j} += \int_{\varepsilon\in\Epsilon_{D}^{T}}\hat{p}^{P^{+}}(q_{j})\cdot v_{i}\cdot n_{T}ds // Z's boundary integral
//                        //                                                                                                  // see also "Z's volume integral", "Z's element surface integral" and "Z's neighbour surface integral" above
////#ifndef NLOG
////                        if ( Zdebug ) {
////                            debugStream.Resume(); // enable logging
////                            debugStream << "      = Z boundary =====================" << std::endl;
////                            debugStream.Suspend();
////                        }
////#endif
////                        if ( discreteModel_.hasPressureFlux() ) {
//                            for ( int i = 0; i < numVelocityBaseFunctionsElement; ++i ) {
//                                // compute the boundary integral
//                                for ( int j = 0; j < numPressureBaseFunctionsElement; ++j ) {
//                                    double Z_i_j = 0.0;
////#ifndef NLOG
////    //                                if ( ( i == logBaseI ) && ( j == logBaseJ ) ) Zoutput = true;
////    //                                if ( allOutput ) Zoutput = true;
//////                                    if ( Zdebug ) Zoutput = true;
////                                    if ( intersectionOutput && Zoutput ) debugStream.Resume(); // enable logging
////                                    debugStream << "      = Z boundary =====================" << std::endl;
////                                    debugStream << "      basefunctions " << i << " " << j << std::endl;
////                                    debugStream << "      faceQuadratureElement.nop() " << faceQuadratureElement.nop() << std::endl;
////#endif
//                                    // sum over all quadrature points
//                                    for ( int quad = 0; quad < faceQuadratureElement.nop(); ++quad ) {
//                                        // get x codim<0> and codim<1> coordinates
//                                        const ElementCoordinateType x = faceQuadratureElement.point( quad );
//                                        const LocalIntersectionCoordinateType localX = faceQuadratureElement.localPoint( quad );
//                                        // get the integration factor
//                                        const double elementVolume = intersectionGeoemtry.integrationElement( localX );
//                                        // get the quadrature weight
//                                        const double integrationWeight = faceQuadratureElement.weight( quad );
////                                        // compute \hat{p}^{P^{+}}(q_{j})\cdot v_{i}\cdot n_{T}
//                                        const VelocityRangeType outerNormal = intIt.unitOuterNormal( localX );
//                                        VelocityRangeType v_i( 0.0 );
//                                        velocityBaseFunctionSetElement.evaluate( i, x, v_i );
//                                        PressureRangeType q_j( 0.0 );
//                                        pressureBaseFunctionSetElement.evaluate( j, x, q_j );
//                                        const double v_times_normal = v_i * outerNormal;
//                                        const double q_times_v_times_normal = q_j * v_times_normal;
////                                        PressureRangeType p_p_plus_flux( 0.0 );
////                                        discreteModel_.pressureBoundaryFlux(    intIt,
////                                                                        0.0,
////                                                                        localX,
////                                                                        q_j,
////                                                                        p_p_plus_flux );
//                                        Z_i_j += elementVolume
//                                            * integrationWeight
//                                            * q_times_v_times_normal;
////#ifndef NLOG
////                                        debugStream << "      - quadPoint " << quad;
////                                        Stuff::printFieldVector( x, "x", debugStream, "        " );
////                                        Stuff::printFieldVector( localX, "localX", debugStream, "          " );
////                                        Stuff::printFieldVector( outerNormal, "outerNormal", debugStream, "          " );
////                                        debugStream << "\n        - elementVolume: " << elementVolume << std::endl;
////                                        debugStream << "        - integrationWeight: " << integrationWeight;
////                                        Stuff::printFieldVector( v_i, "v_i", debugStream, "        " );
////                                        Stuff::printFieldVector( q_j, "q_j", debugStream, "        " );
////                                        debugStream << "\n          - v_i_times_n_t: " << v_i_times_n_t << std::endl;
////                                        debugStream << "\n          - p_p_plus_flux: " << p_p_plus_flux << std::endl;
////                                        debugStream << "          - Z_" << i << "_" << j << "+=: " << Z_i_j << std::endl;
////#endif
//                                    } // done sum over all quadrature points
//                                    // if small, should be zero
//                                    if ( fabs( Z_i_j ) < eps ) {
//                                        Z_i_j = 0.0;
//                                    }
////#ifndef NLOG
////                                    if ( Zdebug && ( Z_i_j > 0.0 ) ) {
////                                        debugStream.Resume();
////                                        debugStream << "      v_" << i << ", q_" << j << std::endl
////                                                    << "        Z( ";
////                                    }
////#ifdef LOTS_OF_DEBUG
////                                    else if ( Zdebug ) {
////                                        debugStream.Resume();
////                                        debugStream << "      v_" << i << ", q_" << j << std::endl
////                                                    << "        Z( ";
////                                    }
////#endif
////#endif
//                                    // add to matrix
//                                    localZmatrixElement.add( i, j, Z_i_j );
////#ifndef NLOG
////                                    if ( Zdebug && ( Z_i_j > 0.0 ) ) {
////                                        debugStream << " ) += " << Z_i_j << std::endl;
////                                    }
////#ifdef LOTS_OF_DEBUG
////                                    else if ( Zdebug ) {
////                                        debugStream << " ) += " << Z_i_j << std::endl;
////                                    }
////#endif
////                                    Zoutput = false;
////                                    debugStream.Suspend(); // disable logging
////#endif
//                                }
//                            } // done computing Z's boundary integral
////                        }

                        //                                                                                                                 // we will call this one
                        // (H2)_{j} += \int_{\varepsilon\in\Epsilon_{D}^{T}}\left( \mu v_{j}\cdot\hat{\sigma}^{RHS}()\cdot n_{T}ds         // H2's 1st boundary integral
                        //                                                         -\hat{p}^{RHS}()\cdot v_{j}\cdot n_{T}ds        \right) // H2's 2nd boundary integral
                        //                                                                                                                 // see also "H2's volume integral" above
                        if ( discreteModel_.hasSigmaFlux() && discreteModel_.hasPressureFlux() ) {
                            for ( int j = 0; j < numVelocityBaseFunctionsElement; ++j ) {
                                double H2_j = 0.0;
#ifndef NLOG
    //                            if ( j == logBaseJ ) H2output = true;
//                                if ( allOutput ) H2output = true;
                                if ( H2debug ) H2output = true;
                                if ( intersectionOutput && H2output ) debugStream.Resume(); // enable logging
                                debugStream << "      = H2 boundary ====================" << std::endl;
                                debugStream << "      basefunction " << j << std::endl;
                                debugStream << "      faceQuadratureElement.nop() " << faceQuadratureElement.nop() << std::endl;
#endif
                                // sum over all quadrature points
                                for ( int quad = 0; quad < faceQuadratureElement.nop(); ++quad ) {
                                    // get x codim<0> and codim<1> coordinates
                                    const ElementCoordinateType x = faceQuadratureElement.point( quad );
                                    const LocalIntersectionCoordinateType localX = faceQuadratureElement.localPoint( quad );
                                    const VelocityRangeType globalX = geometry.global( x );
                                    // get the integration factor
                                    const double elementVolume = intersectionGeoemtry.integrationElement( localX );
                                    // get the quadrature weight
                                    const double integrationWeight = faceQuadratureElement.weight( quad );
                                    // prepare
                                    const VelocityRangeType outerNormal = intIt.unitOuterNormal( localX );
                                    VelocityRangeType v_j( 0.0 );
                                    velocityBaseFunctionSetElement.evaluate( j, x, v_j );
                                    // compute \mu v_{j}\cdot\hat{\sigma}^{RHS}()\cdot n_{T}
                                    if ( discreteModel_.hasSigmaFlux() ) {
                                        const VelocityRangeType xIntersectionGlobal = intIt.intersectionSelfLocal().global( localX );
                                        const VelocityRangeType xWorld = geometry.global( xIntersectionGlobal );
                                        VelocityRangeType gD( 0.0 );
                                        discreteModel_.dirichletData( 0.0, xWorld, gD );
                                        SigmaRangeType sigma_rhs_flux( 0.0 );
                                        sigma_rhs_flux = dyadicProduct( gD, outerNormal );
                                        sigma_rhs_flux *= C_11;
//                                        discreteModel_.sigmaBoundaryFlux(   intIt,
//                                                                            0.0,
//                                                                            localX,
//                                                                            sigma_rhs_flux );
                                        VelocityRangeType flux_times_n_t( 0.0 );
                                        sigma_rhs_flux.mv( outerNormal, flux_times_n_t );
                                        const double v_j_times_flux_times_n_t = v_j * flux_times_n_t;
                                        H2_j += elementVolume
                                            * integrationWeight
                                            * mu
                                            * v_j_times_flux_times_n_t;
#ifndef NLOG
                                        debugStream << "      - quadPoint " << quad;
                                        Stuff::printFieldVector( x, "x", debugStream, "        " );
                                        Stuff::printFieldVector( localX, "localX", debugStream, "        " );
                                        Stuff::printFieldVector( globalX, "globalX", debugStream, "        " );
                                        Stuff::printFieldVector( outerNormal, "outerNormal", debugStream, "        " );
                                        debugStream << "\n        - elementVolume: " << elementVolume << std::endl;
                                        debugStream << "        - integrationWeight: " << integrationWeight;
                                        Stuff::printFieldVector( v_j, "v_j", debugStream, "        " );
                                        Stuff::printFieldMatrix( sigma_rhs_flux, "sigma_rhs_flux", debugStream, "        " );
                                        Stuff::printFieldVector( flux_times_n_t, "flux_times_n_t", debugStream, "        " );
                                        debugStream << "\n        - v_j_times_flux_times_n_t: " << v_j_times_flux_times_n_t << std::endl;
                                        debugStream << "        - H2_" << j << "+=: " << H2_j;
#endif
                                    }
                                    // done computing H2's 1st boundary integral
                                    // compute -\hat{p}^{RHS}()\cdot v_{j}\cdot n_{T}
                                    if ( discreteModel_.hasPressureFlux() ) {
                                        const double v_j_times_n_t = v_j * outerNormal;
                                        PressureRangeType p_rhs_flux( 0.0 );
//                                        discreteModel_.pressureBoundaryFlux(    intIt,
//                                                                                0.0,
//                                                                                localX,
//                                                                                p_rhs_flux );
                                        const double flux_times_v_j_times_n_t = p_rhs_flux * v_j_times_n_t;
                                        H2_j += -1.0
                                            * elementVolume
                                            * integrationWeight
                                            * flux_times_v_j_times_n_t;
#ifndef NLOG
                                        Stuff::printFieldVector( p_rhs_flux, "p_rhs_flux", debugStream, "        " );
                                        debugStream << "\n        - flux_times_v_j_times_n_t: " << flux_times_v_j_times_n_t << std::endl;
                                        debugStream << "        - H2_" << j << "+=: " << H2_j << std::endl;
#endif
                                    }
                                    // done computing H2's 2nd boundary integral
                                } // done sum over all quadrature points
                                // if small, should be zero
                                if ( fabs( H2_j ) < eps ) {
                                    H2_j = 0.0;
                                }
                                // add to rhs
                                localH2rhs[ j ] += H2_j;
#ifndef NLOG
                                H2output = false;
                                debugStream.Suspend(); // disable logging
#endif
                            } // done computing H2's boundary integrals
                        }
//
//                        //                                                                                               // we will call this one
//                        // (E)_{i,j} += \int_{\varepsilon\in\Epsilon_{D}^{T}}\hat{u}_{p}^{U^{+}}(v_{j}\cdot n_{T}q_{i}ds // E's boundary integral
//                        //                                                                                               // see also "E's volume integral", "E's element surface integral" and "E's neighbour surface integral" above
////#ifndef NLOG
////                        if ( Edebug ) {
////                            debugStream.Resume(); // enable logging
////                            debugStream << "      = E boundary =====================" << std::endl;
////                            debugStream.Suspend();
////                        }
////#endif
////                        if ( discreteModel_.hasVelocityPressureFlux() ) {
////                            for ( int i = 0; i < numPressureBaseFunctionsElement; ++i ) {
////                                // compute the boundary integral
////                                for ( int j = 0; j < numVelocityBaseFunctionsElement; ++j ) {
////                                    double E_i_j = 0.0;
////#ifndef NLOG
////    //                                if ( ( i == logBaseI ) && ( j == logBaseJ ) ) Eoutput = true;
////    //                                if ( allOutput ) Eoutput = true;
//////                                    if ( Edebug ) Eoutput = true;
////                                    if ( intersectionOutput && Eoutput ) debugStream.Resume(); // enable logging
////                                    debugStream << "      = E boundary =====================" << std::endl;
////                                    debugStream << "      basefunctions " << i << " " << j << std::endl;
////                                    debugStream << "      faceQuadratureElement.nop() " << faceQuadratureElement.nop() << std::endl;
////#endif
////                                    // sum over all quadrature points
////                                    for ( int quad = 0; quad < faceQuadratureElement.nop(); ++quad ) {
////                                        // get x codim<0> and codim<1> coordinates
////                                        const ElementCoordinateType x = faceQuadratureElement.point( quad );
////                                        const LocalIntersectionCoordinateType localX = faceQuadratureElement.localPoint( quad );
////                                        // get the integration factor
////                                        const double elementVolume = intersectionGeoemtry.integrationElement( localX );
////                                        // get the quadrature weight
////                                        const double integrationWeight = faceQuadratureElement.weight( quad );
////                                        // compute \hat{u}_{p}^{U^{+}}(v_{j})\cdot n_{T}q_{i}
////                                        const VelocityRangeType outerNormal = intIt.unitOuterNormal( localX );
////                                        VelocityRangeType v_j( 0.0 );
////                                        velocityBaseFunctionSetElement.evaluate( j, x, v_j );
////                                        VelocityRangeType u_p_u_plus_flux( 0.0 );
////                                        discreteModel_.velocityPressureBoundaryFlux(    intIt,
////                                                                                0.0,
////                                                                                localX,
////                                                                                v_j,
////                                                                                u_p_u_plus_flux );
////                                        const double flux_times_n_t = u_p_u_plus_flux * outerNormal;
////                                        PressureRangeType q_i( 0.0 );
////                                        pressureBaseFunctionSetElement.evaluate( i, x, q_i );
////                                        const double flux_times_n_t_times_q_i = q_i * flux_times_n_t;
////                                        E_i_j += elementVolume
////                                            * integrationWeight
////                                            * flux_times_n_t_times_q_i;
////#ifndef NLOG
////                                        debugStream << "      - quadPoint " << quad;
////                                        Stuff::printFieldVector( x, "x", debugStream, "        " );
////                                        Stuff::printFieldVector( localX, "localX", debugStream, "          " );
////                                        Stuff::printFieldVector( outerNormal, "outerNormal", debugStream, "          " );
////                                        debugStream << "\n        - elementVolume: " << elementVolume << std::endl;
////                                        debugStream << "        - integrationWeight: " << integrationWeight;
////                                        Stuff::printFieldVector( q_i, "q_i", debugStream, "        " );
////                                        Stuff::printFieldVector( v_j, "v_j", debugStream, "        " );
////                                        Stuff::printFieldVector( u_p_u_plus_flux, "u_p_u_plus_flux", debugStream, "        " );
////                                        debugStream << "\n          - flux_times_n_t: " << flux_times_n_t << std::endl;
////                                        debugStream << "\n          - flux_times_n_t_times_q_i: " << flux_times_n_t_times_q_i << std::endl;
////                                        debugStream << "          - E_" << i << "_" << j << "+=: " << E_i_j << std::endl;
////#endif
////                                    } // done sum over all quadrature points
////                                    // if small, should be zero
////                                    if ( fabs( E_i_j ) < eps ) {
////                                        E_i_j = 0.0;
////                                    }
////#ifndef NLOG
////                                    if ( Edebug && ( E_i_j > 0.0 ) ) {
////                                        debugStream.Resume();
////                                        debugStream << "      q_" << i << ", v_" << j << std::endl
////                                                    << "        E( ";
////                                    }
////#ifdef LOTS_OF_DEBUG
////                                    else if ( Edebug ) {
////                                        debugStream.Resume();
////                                        debugStream << "      q_" << i << ", v_" << j << std::endl
////                                                    << "        E( ";
////                                    }
////#endif
////#endif
////                                    // add to matrix
////                                    localEmatrixElement.add( i, j, E_i_j );
////#ifndef NLOG
////                                    if ( Edebug && ( E_i_j > 0.0 ) ) {
////                                        debugStream << " ) += " << E_i_j << std::endl;
////                                    }
////#ifdef LOTS_OF_DEBUG
////                                    else if ( Edebug ) {
////                                        debugStream << " ) += " << E_i_j << std::endl;
////                                    }
////#endif
////                                    Eoutput = false;
////                                    debugStream.Suspend(); // disable logging
////#endif
////                                }
////                            } // done computing E's boundary integral
////                        }
//
//                        //                                                                                                // we call this one
//                        // (R)_{i,j} += \int_{\varepsilon\in\Epsilon_{D}^{T}}\hat{u}_{p}^{P^{+}}(q_{j})\cdot n_{t}q_{i}ds // R's boundary integral
//                        //                                                                                                // see also "R's element surface integral" and "R's neighbour surface integral" above
////#ifndef NLOG
////                        if ( Rdebug ) {
////                            debugStream.Resume(); // enable logging
////                            debugStream << "      = R boundary =====================" << std::endl;
////                            debugStream.Suspend();
////                        }
////#endif
////                        if ( discreteModel_.hasVelocityPressureFlux() ) {
////                            for ( int i = 0; i < numPressureBaseFunctionsElement; ++i ) {
////                                for ( int j = 0; j < numPressureBaseFunctionsElement; ++j ) {
////                                    double R_i_j = 0.0;
////#ifndef NLOG
////    //                                if ( ( i == logBaseI ) && ( j == logBaseJ ) ) Routput = true;
////    //                                if ( allOutput ) Routput = true;
//////                                    if ( Rdebug ) Routput = true;
////                                    if ( intersectionOutput && Routput ) debugStream.Resume(); // enable logging
////                                    debugStream << "      = R boundary =====================" << std::endl;
////                                    debugStream << "      basefunctions " << i << " " << j << std::endl;
////                                    debugStream << "      faceQuadratureElement.nop() " << faceQuadratureElement.nop() << std::endl;
////#endif
////                                    // sum over all quadrature points
////                                    for ( int quad = 0; quad < faceQuadratureElement.nop(); ++quad ) {
////                                        // get x codim<0> and codim<1> coordinates
////                                        const ElementCoordinateType x = faceQuadratureElement.point( quad );
////                                        const LocalIntersectionCoordinateType localX = faceQuadratureElement.localPoint( quad );
////                                        // get the integration factor
////                                        const double elementVolume = intersectionGeoemtry.integrationElement( localX );
////                                        // get the quadrature weight
////                                        const double integrationWeight = faceQuadratureElement.weight( quad );
////                                        // compute \hat{u}_{p}^{P^{+}}(q_{j})\cdot n_{T}q_{i}
////                                        const VelocityRangeType outerNormal = intIt.unitOuterNormal( localX );
////                                        PressureRangeType q_j( 0.0 );
////                                        pressureBaseFunctionSetElement.evaluate( j, x, q_j );
////                                        VelocityRangeType u_p_p_plus_flux( 0.0 );
////                                        discreteModel_.velocityPressureBoundaryFlux(    intIt,
////                                                                                        0.0,
////                                                                                        localX,
////                                                                                        q_j,
////                                                                                        u_p_p_plus_flux );
////                                        const double flux_times_n_t = u_p_p_plus_flux
////                                            * outerNormal;
////                                        PressureRangeType q_i( 0.0 );
////                                        pressureBaseFunctionSetElement.evaluate( i, x, q_i );
////                                        const double flux_times_n_t_times_q_i = q_i * flux_times_n_t;
////                                        R_i_j += elementVolume
////                                            * integrationWeight
////                                            * flux_times_n_t_times_q_i;
////#ifndef NLOG
////                                        debugStream << "      - quadPoint " << quad;
////                                        Stuff::printFieldVector( x, "x", debugStream, "        " );
////                                        Stuff::printFieldVector( localX, "localX", debugStream, "          " );
////                                        Stuff::printFieldVector( outerNormal, "outerNormal", debugStream, "          " );
////                                        debugStream << "\n        - elementVolume: " << elementVolume << std::endl;
////                                        debugStream << "        - integrationWeight: " << integrationWeight;
////                                        Stuff::printFieldVector( q_i, "q_i", debugStream, "        " );
////                                        Stuff::printFieldVector( q_j, "q_j", debugStream, "        " );
////                                        Stuff::printFieldVector( u_p_p_plus_flux, "u_p_p_plus_flux", debugStream, "        " );
////                                        debugStream << "\n          - flux_times_n_t: " << flux_times_n_t << std::endl;
////                                        debugStream << "\n          - flux_times_n_t_times_q_i: " << flux_times_n_t_times_q_i << std::endl;
////                                        debugStream << "          - R_" << i << "_" << j << "+=: " << R_i_j << std::endl;
////#endif
////                                    } // done sum over all quadrature points
////                                    // if small, should be zero
////                                    if ( fabs( R_i_j ) < eps ) {
////                                        R_i_j = 0.0;
////                                    }
////#ifndef NLOG
////                                    if ( Rdebug && ( R_i_j > 0.0 ) ) {
////                                        debugStream.Resume();
////                                        debugStream << "      q_" << i << ", q_" << j << std::endl
////                                                    << "        R( ";
////                                    }
////#ifdef LOTS_OF_DEBUG
////                                    else if ( Rdebug ) {
////                                        debugStream.Resume();
////                                        debugStream << "      q_" << i << ", q_" << j << std::endl
////                                                    << "        R( ";
////                                    }
////#endif
////#endif
////                                    // add to matrix
////                                    localRmatrixElement.add( i, j, R_i_j );
////#ifndef NLOG
////                                    if ( Rdebug && ( R_i_j > 0.0 ) ) {
////                                        debugStream << " ) += " << R_i_j << std::endl;
////                                    }
////#ifdef LOTS_OF_DEBUG
////                                    else if ( Rdebug ) {
////                                        debugStream << " ) += " << R_i_j << std::endl;
////                                    }
////#endif
////                                    Routput = false;
////                                    debugStream.Suspend(); // disable logging
////#endif
////                                }
////                            } // done computing R's boundary integral
////                        }

                        //                                                                                        // we will call this one
                        // (H3)_{j} = \int_{\varepsilon\in\Epsilon_{D}^{T}}-\hat{u}_{p}^{RHS}()\cdot n_{T}q_{j}ds // H3's boundary integral
                        if ( discreteModel_.hasVelocityPressureFlux() ) {
                            for ( int j = 0; j < numPressureBaseFunctionsElement; ++j ) {
                                double H3_j = 0.0;
#ifndef NLOG
//                                if ( j == logBaseJ ) H3output = true;
//                                if ( allOutput ) H3output = true;
//                                if ( H3debug ) H3output = true;
                                if ( intersectionOutput && H3output ) debugStream.Resume(); // enable logging
                                debugStream << "      = H3 boundary ====================" << std::endl;
                                debugStream << "      basefunction " << j << std::endl;
                                debugStream << "      faceQuadratureElement.nop() " << faceQuadratureElement.nop() << std::endl;
#endif
                                // sum over all quadrature points
                                for ( int quad = 0; quad < faceQuadratureElement.nop(); ++quad ) {
                                    // get x codim<0> and codim<1> coordinates
                                    const ElementCoordinateType x = faceQuadratureElement.point( quad );
                                    const LocalIntersectionCoordinateType localX = faceQuadratureElement.localPoint( quad );
                                    const VelocityRangeType xWorld = geometry.global( x );
                                    // get the integration factor
                                    const double elementVolume = intersectionGeoemtry.integrationElement( localX );
                                    // get the quadrature weight
                                    const double integrationWeight = faceQuadratureElement.weight( quad );
                                    // compute -\hat{u}_{p}^{RHS}()\cdot n_{T}q_{j}
                                    const VelocityRangeType outerNormal = intIt.unitOuterNormal( localX );
                                    VelocityRangeType gD( 0.0 );
                                    discreteModel_.dirichletData( 0.0, xWorld, gD );
                                    VelocityRangeType u_p_rhs_flux = gD;
//                                    discreteModel_.velocityPressureBoundaryFlux( intIt,
//                                                                                0.0,
//                                                                                localX,
//                                                                                u_p_rhs_flux );
                                    const double flux_times_n_t = u_p_rhs_flux
                                        * outerNormal;
                                    PressureRangeType q_j( 0.0 );
                                    pressureBaseFunctionSetElement.evaluate( j, x, q_j );
                                    const double flux_times_n_t_times_q_j = q_j
                                        * flux_times_n_t;
                                    H3_j += elementVolume
                                        * integrationWeight
                                        * flux_times_n_t_times_q_j;
#ifndef NLOG
                                    debugStream << "      - quadPoint " << quad;
                                    Stuff::printFieldVector( x, "x", debugStream, "        " );
                                    Stuff::printFieldVector( localX, "localX", debugStream, "        " );
                                    Stuff::printFieldVector( outerNormal, "outerNormal", debugStream, "        " );
                                    debugStream << "\n        - elementVolume: " << elementVolume << std::endl;
                                    debugStream << "        - integrationWeight: " << integrationWeight;
                                    Stuff::printFieldVector( q_j, "q_j", debugStream, "        " );
                                    Stuff::printFieldVector( u_p_rhs_flux, "u_p_rhs_flux", debugStream, "        " );
                                    debugStream << "\n        - flux_times_n_t: " << flux_times_n_t << std::endl;
                                    debugStream << "\n        - flux_times_n_t_times_q_j: " << flux_times_n_t_times_q_j << std::endl;
                                    debugStream << "        - H3_" << j << "+=: " << H3_j << std::endl;
#endif
                                } // done sum over all quadrature points
//                                // if small, should be zero
//                                if ( fabs( H3_j ) < eps ) {
//                                    H3_j = 0.0;
//                                }
                                // add to rhs
                                localH3rhs[ j ] += H3_j;
#ifndef NLOG
                                H3output = false;
                                debugStream.Suspend(); // disable logging
#endif
                            } // done computing H3's boundary integral
                        }

                    } // done with those on the boundary
#ifndef NLOG
                    if ( intersectionOutput ) debugStream.Resume(); // enable logging
                    debugStream << "    - ==== done calculations on intersection " << intersectionNR << std::endl;
                    debugStream.Suspend(); // disable logging
                    intersectionOutput = false;
                    ++intersectionNR;
#endif
                } // done walking the neighbours
//#endif //no_surface_ints

#ifndef NLOG
                intersectionNR = 0;
                if ( entityOutput ) debugStream.Resume(); // enable logging
                debugStream << "  - == done calculations on entity " << entityNR << std::endl;
                debugStream.Suspend(); // disable logging
                entityOutput = false;
                ++entityNR;
#endif
            } // done walking the grid


#ifndef NLOG

////#ifdef CHEAT
////            // compute the artificial right hand sides, should be the right ones
////            // H1
//////            debugStream = Logger().Info();
////
////            debugStream.Resume();
////            debugStream << "  - computing artificial right hand sides" << std::endl;
////            debugStream << "    - H1" << std::endl;
////            Wmatrix.matrix().apply( discreteExactVelocity, tmpH1rhs );
////            exactH1rhs += tmpH1rhs;
////            tmpH1rhs.clear();
////            MMatrix.matrix().apply( computedVelocityGradient, tmpH1rhs );
////            exactH1rhs += tmpH1rhs;
////            tmpH1rhs.clear();
////            // H2
////            debugStream << "    - H2" << std::endl;
////            Zmatrix.matrix().apply( discreteExactPressure, tmpH2rhs );
////            exactH2rhs += tmpH2rhs;
////            tmpH2rhs.clear();
////            Ymatrix.matrix().apply( discreteExactVelocity, tmpH2rhs );
////            exactH2rhs += tmpH2rhs;
////            tmpH2rhs.clear();
////            Xmatrix.matrix().apply( computedVelocityGradient, tmpH2rhs );
////            exactH2rhs += tmpH2rhs;
////            tmpH2rhs.clear();
////            // H3
////            debugStream << "    - H3" << std::endl;
////            Rmatrix.matrix().apply( discreteExactPressure, tmpH3rhs );
////            exactH3rhs += tmpH3rhs;
////            tmpH3rhs.clear();
////            Ematrix.matrix().apply( discreteExactVelocity, tmpH3rhs );
////            exactH3rhs += tmpH3rhs;
////            tmpH3rhs.clear();
////            debugStream << "  - done computing artificial rihgt hand sides" << std::endl;
////#endif

            infoStream.Resume();
            if ( numberOfEntities > 19 ) {
                infoStream << "]";
            }
            infoStream << "\n- gridwalk done" << std::endl << std::endl;
            infoStream.Suspend();

            if ( Mprint || Wprint || Xprint || Yprint || Zprint || Eprint || Rprint || H1print || H2print || H3print ) {
                debugStream.Resume();
                debugStream << "- printing matrices" << std::endl;
                if ( Mprint ) {
                    debugStream << " - = Minvers ======" << std::endl;
                    debugStream.Log( &MInversMatrixType::MatrixType::print,  MInversMatrix.matrix() );
                }
//if ( Mprint ) {
//    debugStream << " - = M ============" << std::endl;
//    debugStream.Log( &MInversMatrixType::MatrixType::print,  MMatrix.matrix() );
//}
                if ( Wprint ) {
                    debugStream << " - = W ============" << std::endl;
                    debugStream.Log( &WmatrixType::MatrixType::print,  Wmatrix.matrix() );
                }
                if ( Xprint ) {
                    debugStream << " - = X ============" << std::endl;
                    debugStream.Log( &XmatrixType::MatrixType::print,  Xmatrix.matrix() );
                }
                if ( Yprint ) {
                    debugStream << " - = Y ============" << std::endl;
                    debugStream.Log( &YmatrixType::MatrixType::print,  Ymatrix.matrix() );
                }
                if ( Zprint ) {
                    debugStream << " - = Z ============" << std::endl;
                    debugStream.Log( &ZmatrixType::MatrixType::print,  Zmatrix.matrix() );
                }
                if ( Eprint ) {
                    debugStream << " - = E ============" << std::endl;
                    debugStream.Log( &EmatrixType::MatrixType::print,  Ematrix.matrix() );
                }
                if ( Rprint ) {
                    debugStream << " - = R ============" << std::endl;
                    debugStream.Log( &RmatrixType::MatrixType::print,  Rmatrix.matrix() );
                }
                if ( H1print ) {
                    debugStream << " - = H1 ===========" << std::endl;
                    debugStream.Log( &DiscreteSigmaFunctionType::print, H1rhs );
//                    Stuff::oneLinePrint( debugStream, H1rhs );
                }
//#ifdef CHEAT
//                if ( H1print ) {
//                    debugStream << " - = exact H1 =====" << std::endl;
//                    debugStream.Log( &DiscreteSigmaFunctionType::print, exactH1rhs );
//                    Stuff::oneLinePrint( debugStream, exactH1rhs );
//                }
//#endif
                if ( H2print ) {
                    debugStream << " - = H2 ===========" << std::endl;
                    debugStream.Log( &DiscreteVelocityFunctionType::print, H2rhs );
//                    Stuff::oneLinePrint( debugStream, H2rhs );
                }
//#ifdef CHEAT
//                if ( H2print ) {
//                    debugStream << " - = exact H2 =====" << std::endl;
//                    debugStream.Log( &DiscreteVelocityFunctionType::print, exactH2rhs );
//                    Stuff::oneLinePrint( debugStream, exactH2rhs );
//                }
//#endif
                if ( H3print ) {
                    debugStream << " - = H3 ===========" << std::endl;
                    debugStream.Log( &DiscretePressureFunctionType::print, H3rhs );
//                    Stuff::oneLinePrint( debugStream, H3rhs );
                }
//#ifdef CHEAT
//                if ( H3print ) {
//                    debugStream << " - = exact H3 =====" << std::endl;
//                    debugStream.Log( &DiscretePressureFunctionType::print, exactH3rhs );
//                    Stuff::oneLinePrint( debugStream, exactH3rhs );
//                }
//#endif
                debugStream << std::endl;
            }

//            double H1Min = 0.0;
//            double H1Max = 0.0;
//            double H2Min = 0.0;
//            double H2Max = 0.0;
//            double H3Min = 0.0;
//            double H3Max = 0.0;
//            Stuff::getMinMaxOfDiscreteFunction( H1rhs, H1Min, H1Max );
//            Stuff::getMinMaxOfDiscreteFunction( H2rhs, H2Min, H2Max );
//            Stuff::getMinMaxOfDiscreteFunction( H3rhs, H3Min, H3Max );
//            double H1avg = Stuff::getFuncAvg( H1rhs );
//            double H2avg = Stuff::getFuncAvg( H2rhs );
//            double H3avg = Stuff::getFuncAvg( H3rhs );
//
//#ifdef CHEAT
//            double exactH1Min = 0.0;
//            double exactH1Max = 0.0;
//            double exactH2Min = 0.0;
//            double exactH2Max = 0.0;
//            double exactH3Min = 0.0;
//            double exactH3Max = 0.0;
//            Stuff::getMinMaxOfDiscreteFunction( exactH1rhs, exactH1Min, exactH1Max );
//            Stuff::getMinMaxOfDiscreteFunction( exactH2rhs, exactH2Min, exactH2Max );
//            Stuff::getMinMaxOfDiscreteFunction( exactH3rhs, exactH3Min, exactH3Max );
//            double eH1avg = Stuff::getFuncAvg( exactH1rhs );
//            double eH2avg = Stuff::getFuncAvg( exactH2rhs );
//            double eH3avg = Stuff::getFuncAvg( exactH3rhs );
//            double tol = Parameters().getParam( "diff-tolerance", 0.01 );
//            double h1_diffs = Stuff::getNumDiffDofs( H1rhs, exactH1rhs, tol );
//            double h2_diffs = Stuff::getNumDiffDofs( H2rhs, exactH2rhs, tol );
//            double h3_diffs = Stuff::getNumDiffDofs( H3rhs, exactH3rhs, tol );
//            Dune::L2Norm< GridPartType > l2 ( exactH3rhs.space().gridPart() );
//            tmpH1rhs.assign( exactH1rhs );
//            tmpH1rhs -= H1rhs;
//            double h1_err = l2.norm( tmpH1rhs );
//            tmpH2rhs.assign( exactH2rhs );
//            tmpH2rhs -= H2rhs;
//            double h2_err = l2.norm( tmpH2rhs );
//            tmpH3rhs.assign( exactH3rhs );
//            tmpH3rhs -= H3rhs;
//            double h3_err = l2.norm( tmpH3rhs );
//#endif
//            debugStream << "- printing infos" << std::endl
//                        << "  - H1" << std::endl
//                        << "    min: " << H1Min << std::endl
//                        << "    max: " << H1Max << std::endl
//                        << "    avg: " << H1avg << std::endl
//#ifdef CHEAT
//                        << "  - exact H1" << std::endl
//                        << "    min: " << exactH1Min << std::endl
//                        << "    max: " << exactH1Max << std::endl
//                        << "    avg: " << eH1avg << std::endl
//                        << "    err: " << h1_err << std::endl
//                        << "   #dif: " << h1_diffs << " / " << H1rhs.size()
//                                << " (" << 100 * h1_diffs / (double) H1rhs.size() << "%)" << std::endl
//#endif
//                        << "  - H2" << std::endl
//                        << "    min: " << H2Min << std::endl
//                        << "    max: " << H2Max << std::endl
//                        << "    avg: " << H2avg << std::endl
//#ifdef CHEAT
//                        << "  - exact H2" << std::endl
//                        << "    min: " << exactH2Min << std::endl
//                        << "    max: " << exactH2Max << std::endl
//                        << "    avg: " << eH2avg << std::endl
//                        << "    err: " << h2_err << std::endl
//                        << "   #dif: " << h2_diffs << " / " << H2rhs.size()
//                                << " (" << 100 * h2_diffs / (double) H2rhs.size() << "%)" << std::endl
//#endif
//                        << "  - H3" << std::endl
//                        << "    min: " << H3Min << std::endl
//                        << "    max: " << H3Max << std::endl
//                        << "    avg: " << H3avg << std::endl
//#ifdef CHEAT
//                        << "  - exact H3" << std::endl
//                        << "    min: " << exactH3Min << std::endl
//                        << "    max: " << exactH3Max << std::endl
//                        << "    avg: " << eH3avg << std::endl
//                        << "    err: " << h3_err << std::endl
//                        << "   #dif: " << h3_diffs << " / " << H3rhs.size()
//                                << " (" << 100 * h3_diffs / (double) H3rhs.size() << "%)" << std::endl
//                        << " - " << numberOfBoundaryIntersections << " intersections on the boundary." << std::endl
//#endif
//                        << std::endl;


            // do the matlab logging stuff
            Logging::MatlabLogStream& matlabLogStream = Logger().Matlab();
            Stuff::printSparseRowMatrixMatlabStyle( MInversMatrix.matrix(), "M_invers", matlabLogStream );
            Stuff::printSparseRowMatrixMatlabStyle( Wmatrix.matrix(), "W", matlabLogStream );
            Stuff::printSparseRowMatrixMatlabStyle( Xmatrix.matrix(), "X", matlabLogStream );
            Stuff::printSparseRowMatrixMatlabStyle( Ymatrix.matrix(), "Y", matlabLogStream );
            Stuff::printSparseRowMatrixMatlabStyle( Zmatrix.matrix(), "Z", matlabLogStream );
            Stuff::printSparseRowMatrixMatlabStyle( Ematrix.matrix(), "E", matlabLogStream );
            Stuff::printSparseRowMatrixMatlabStyle( Rmatrix.matrix(), "R", matlabLogStream );
            Stuff::printDiscreteFunctionMatlabStyle( H1rhs, "H1", matlabLogStream );
            Stuff::printDiscreteFunctionMatlabStyle( H2rhs, "H2", matlabLogStream );
            Stuff::printDiscreteFunctionMatlabStyle( H3rhs, "H3", matlabLogStream );
//            matlabLogStream << "\nA = Y - X * M_invers * W;" << std::endl;
//            matlabLogStream << "B = Z;" << std::endl;
//            matlabLogStream << "B_T = - E;" << std::endl;
//            matlabLogStream << "C = R;" << std::endl;
//            matlabLogStream << "F = H2 - X * M_invers * H1;" << std::endl;
//            matlabLogStream << "G = - H3;" << std::endl;
//            matlabLogStream << "A_invers = inv( A );" << std::endl;
//            matlabLogStream << "schur_S = B_T * A_invers * B + C;" << std::endl;
//            matlabLogStream << "schur_f = B_T * A_invers * F - G;" << std::endl;
//            matlabLogStream << "p = schur_S \\ schur_f;" << std::endl;
//            matlabLogStream << "u = A_invers * ( F - B * p );" << std::endl;
            matlabLogStream << "mirko_W = -(1/M_invers(1,1)) .* (mirko_W);" << std::endl;
            matlabLogStream << "mirko_E = -(mirko_E);" << std::endl;
            matlabLogStream << "mirko_H1 = (1/M_invers(1,1)) .* (mirko_H1);" << std::endl;
            matlabLogStream << "fprintf(1, 'norm( W - mirko_W ) = %d\\n', norm( W - mirko_W ) );\n" << std::endl;
            matlabLogStream << "fprintf(1, 'norm( X - mirko_X ) = %d\\n', norm( X - mirko_X ) );\n" << std::endl;
            matlabLogStream << "fprintf(1, 'norm( Y - mirko_Y ) = %d\\n', norm( Y - mirko_Y ) );\n" << std::endl;
            matlabLogStream << "fprintf(1, 'norm( Z - mirko_Z ) = %d\\n', norm( Z - mirko_Z ) );\n" << std::endl;
            matlabLogStream << "fprintf(1, 'norm( E - mirko_E ) = %d\\n', norm( E - mirko_E ) );\n" << std::endl;
            matlabLogStream << "fprintf(1, 'norm( R - mirko_R ) = %d\\n', norm( R - mirko_R ) );\n" << std::endl;
            matlabLogStream << "fprintf(1, 'norm( H1 - mirko_H1 ) = %d\\n', norm( H1 - mirko_H1 ) );\n" << std::endl;
            matlabLogStream << "fprintf(1, 'norm( H2 - mirko_H2 ) = %d\\n', norm( H2 - mirko_H2 ) );\n" << std::endl;
//            matlabLogStream << "fprintf(1, 'norm( H2 - H2_model ) = %d\\n', norm( H2 - H2_model ) );\n" << std::endl;
            matlabLogStream << "fprintf(1, 'norm( H3 - mirko_H3 ) = %d\\n', norm( H3 - mirko_H3 ) );\n" << std::endl;
//            matlabLogStream << "fprintf(1, 'norm( A - mirko_A ) = %d\\n', norm( A - mirko_A, inf ) );\n" << std::endl;
//            matlabLogStream << "fprintf(1, 'norm( B - mirko_B ) = %d\\n', norm( B - mirko_B, inf ) );\n" << std::endl;
//            matlabLogStream << "fprintf(1, 'norm( B_T - mirko_B_T ) = %d\\n', norm( B_T - mirko_B_T, inf ) );\n" << std::endl;
//            matlabLogStream << "fprintf(1, 'norm( C - mirko_C ) = %d\\n', norm( C - mirko_C, inf ) );\n" << std::endl;
//            matlabLogStream << "fprintf(1, 'norm( F - mirko_F ) = %d\\n', norm( F - mirko_F, inf ) );\n" << std::endl;
//            matlabLogStream << "fprintf(1, 'norm( G - mirko_G ) = %d\\n', norm( G - mirko_G, inf ) );\n" << std::endl;
//            matlabLogStream << "%fprintf(1, 'norm( schur_S - mirko_schur_S ) = %d\\n', norm( schur_S - mirko_schur_S ) );\n" << std::endl;
//            matlabLogStream << "%fprintf(1, 'norm( schur_f - mirko_schur_f ) = %d\\n', norm( schur_f - mirko_schur_f ) );\n" << std::endl;
//            matlabLogStream << "fprintf(1, 'norm( p - mirko_p ) = %d\\n', norm( p - p ) );\n" << std::endl;
//            matlabLogStream << "fprintf(1, 'norm( u - mirko_u ) = %d\\n', norm( u - mirko_u ) );\n" << std::endl;
//#endif
//            matlabLogStream << "\nA = Y - X * M_invers * W;" << std::endl;
//            matlabLogStream << "B = Z;" << std::endl;
//            matlabLogStream << "B_T = - E;" << std::endl;
//            matlabLogStream << "C = R;" << std::endl;
//            matlabLogStream << "F = H2 - X * M_invers * H1;" << std::endl;
//            matlabLogStream << "G = - H3;" << std::endl;
//            matlabLogStream << "%A_invers = inv( A );" << std::endl;
//            matlabLogStream << "%schur_S = B_T * A_invers * B + C;" << std::endl;
//            matlabLogStream << "%schur_f = B_T * A_invers * F - G;" << std::endl;
//            matlabLogStream << "%p = schur_S \\ schur_f;" << std::endl;
//            matlabLogStream << "%u = A_invers * ( F - B * p );\n" << std::endl;
//            matlabLogStream << "%fprintf(1, 'Condition A: %d\\n', cond( A ) );\n" << std::endl;
//            matlabLogStream << "%fprintf(1, 'Condition S: %d\\n', cond( schur_S ) );\n" << std::endl;
//
//            Stuff::GridWalk<GridPartType> gw( gridPart_ );
//            typedef Logging::MatlabLogStream
//                FunctorStream;
//            FunctorStream& functorStream = matlabLogStream;
//            Stuff::GridWalk<DiscreteVelocityFunctionSpaceType> gw( velocitySpace_ );
//            Stuff::LocalMatrixPrintFunctor< EmatrixType,FunctorStream> f_E ( Ematrix, functorStream, "E" );
//            Stuff::LocalMatrixPrintFunctor< WmatrixType,FunctorStream> f_W ( Wmatrix, functorStream, "W" );
//            Stuff::LocalMatrixPrintFunctor< XmatrixType,FunctorStream> f_X ( Xmatrix, functorStream, "X" );
//            Stuff::LocalMatrixPrintFunctor< YmatrixType,FunctorStream> f_Y ( Ymatrix, functorStream, "Y" );
//            Stuff::LocalMatrixPrintFunctor< ZmatrixType,FunctorStream> f_Z ( Zmatrix, functorStream, "Z" );
//            Stuff::LocalMatrixPrintFunctor< RmatrixType,FunctorStream> f_R ( Rmatrix, functorStream, "R" );
//            if( Wprint ) {
//                gw( f_W );
//            }
//            if( Xprint ) {
//                gw( f_X );
//            }
//            if( Yprint ) {
//                gw( f_Y );
//            }
//            if( Zprint ) {
//                gw( f_Z );
//            }
//            if( Eprint ) {
//                gw( f_E );
//            }
//            if( Rprint ) {
//                gw( f_R );
//            }
#endif //NLOG
//
//            profiler().StopTiming("Pass -- ASSEMBLE");
//            profiler().StartTiming("Pass -- SOLVER");
//            InvOpType op;
//#ifdef USE_ALTERNATIVE_SOLVER
//            AltInvOpType m_op;
//            if ( Parameters().getParam( "alternative-solve", false ) )
//                m_op.solve( arg, dest, Xmatrix, MInversMatrix, Ymatrix, Ematrix, Rmatrix, Zmatrix, Wmatrix, H1rhs, H2rhs, H3rhs );
//            else
//#endif
//                op.solve( arg, dest, Xmatrix, MInversMatrix, Ymatrix, Ematrix, Rmatrix, Zmatrix, Wmatrix, H1rhs, H2rhs, H3rhs );
//
//#if 0 //too complex to incorporate right now
//#ifndef CHEAT
//            op.solve( arg, dest, Xmatrix, MInversMatrix, Ymatrix, Ematrix, Rmatrix, Zmatrix, Wmatrix, H1rhs, H2rhs, H3rhs );
//#else
//            if ( Parameters().getParam( "use-cheat", true ) ) {
//#ifndef NLOG
//                infoStream.Resume();
//                infoStream << "solving with cheated rhs" << std::endl;
//                infoStream.Suspend();
//#endif
//                op.solve( arg, dest, Xmatrix, MInversMatrix, Ymatrix, Ematrix, Rmatrix, Zmatrix, Wmatrix, exactH1rhs, exactH2rhs, exactH3rhs );
//            }
//            else {
//#ifndef NLOG
//                infoStream.Resume();
//                infoStream << "solving with \"normal\" rhs" << std::endl;
//                infoStream.Suspend();
//#endif
//                op.solve( arg, dest, Xmatrix, MInversMatrix, Ymatrix, Ematrix, Rmatrix, Zmatrix, Wmatrix, H1rhs, H2rhs, H3rhs );
//            }
//#endif
//#endif //end if 0
//
//#ifndef NLOG
//            if ( Parameters().getParam( "solution-print", true ) ) {
//                Stuff::oneLinePrint( infoStream, dest.discretePressure() );
//                Stuff::oneLinePrint( infoStream, dest.discreteVelocity() );
//            }
//
////            debugStream.Resume();
////            Stuff::oneLinePrint( debugStream, dest.discretePressure() );
//#endif
//            profiler().StopTiming("Pass -- SOLVER");
//            profiler().StopTiming("Pass");

        } // end of apply

        virtual void compute( const TotalArgumentType &arg, DestinationType &dest) const
        {}

        virtual void allocateLocalMemory()
        {}

    private:
        DiscreteModelType& discreteModel_;
        GridPartType& gridPart_;
        DiscreteStokesFunctionSpaceWrapperType& spaceWrapper_;
        DiscreteVelocityFunctionSpaceType& velocitySpace_;
        DiscretePressureFunctionSpaceType& pressureSpace_;
        DiscreteSigmaFunctionSpaceType sigmaSpace_;

        /**
         *  \todo   doc
         **/
        double colonProduct(    const SigmaRangeType& arg1,
                                const SigmaRangeType& arg2 ) const
        {
            assert( arg1.rowdim() == arg2.coldim() );
            double ret = 0.0;
            // iterators
            typedef typename SigmaRangeType::ConstRowIterator
                ConstRowIteratorType;
            typedef typename SigmaRangeType::row_type::ConstIterator
                ConstIteratorType;
            ConstRowIteratorType arg1RowItEnd = arg1.end();
            ConstRowIteratorType arg2RowItEnd = arg2.end();
            ConstRowIteratorType arg2RowIt = arg2.begin();
            for (   ConstRowIteratorType arg1RowIt = arg1.begin();
                    arg1RowIt != arg1RowItEnd, arg2RowIt != arg2RowItEnd;
                    ++arg1RowIt, ++arg2RowIt ) {
                ConstIteratorType row1ItEnd = arg1RowIt->end();
                ConstIteratorType row2ItEnd = arg2RowIt->end();
                ConstIteratorType row2It = arg2RowIt->begin();
                for (   ConstIteratorType row1It = arg1RowIt->begin();
                        row1It != row1ItEnd, row2It != row2ItEnd;
                        ++row1It, ++row2It ) {
                    ret += *row1It * *row2It;
                }
            }
            return ret;
        }

//        /**
//         *  \todo   doc
//         *  \tparam C   SigmaJacobianRangeType
//         **/
//        template < class C >
//        VelocityRangeType sigmaDivergenceOutOfGradient( const C& arg ) const
//        {
//            VelocityRangeType ret( 0.0 );
//            const int dim = arg[0].dim();
//            for ( int i = 0; i < dim; ++i ) {
//                for ( int j = 0; j < dim; ++j ) {
//                    const VelocityRangeType gradientRow = arg[( dim * i ) + j];
//                    ret[i] += gradientRow[ i ];
//                }
//            }
//            return ret;
//        }
//
//        /**
//         *  \todo   doc
//         **/
//        double velocityDivergenceOutOfGradient( const SigmaRangeType& arg ) const
//        {
//            double ret( 0.0 );
//            typedef typename SigmaRangeType::ConstRowIterator
//                ConstRowIteratorType;
//            ConstRowIteratorType rowItEnd = arg.end();
//            int i = 0;
//            for (   ConstRowIteratorType rowIt = arg.begin();
//                    rowIt != rowItEnd;
//                    ++rowIt ) {
//                ret += (*rowIt)[i];
//                ++i;
//            }
//            return ret;
//        }

                /**
         *  \brief  dyadic product
         *
         *          Implements \f$\left(arg_{1} \otimes arg_{2}\right)_{i,j}:={arg_{1}}_{i} {arg_{2}}_{j}\f$
         **/
        SigmaRangeType dyadicProduct(   const VelocityRangeType& arg1,
                                        const VelocityRangeType& arg2 ) const
        {
            SigmaRangeType ret( 0.0 );
            typedef typename SigmaRangeType::RowIterator
                MatrixRowIteratorType;
            typedef typename VelocityRangeType::ConstIterator
                ConstVectorIteratorType;
            typedef typename VelocityRangeType::Iterator
                VectorIteratorType;
            MatrixRowIteratorType rItEnd = ret.end();
            ConstVectorIteratorType arg1It = arg1.begin();
            for ( MatrixRowIteratorType rIt = ret.begin(); rIt != rItEnd; ++rIt ) {
                ConstVectorIteratorType arg2It = arg2.begin();
                VectorIteratorType vItEnd = rIt->end();
                for (   VectorIteratorType vIt = rIt->begin();
                        vIt != vItEnd;
                        ++vIt ) {
                    *vIt = *arg1It * *arg2It;
                    ++arg2It;
                }
                ++arg1It;
            }
            return ret;
        }

        /**
         *  \brief  calculates length of given intersection in world coordinates
         *  \tparam IntersectionIteratorType
         *          IntersectionIteratorType
         *  \param[in]  intIt
         *          intersection
         *  \return length of intersection
         **/
        template < class IntersectionIteratorType >
        double getLenghtOfIntersection( const IntersectionIteratorType& intIt ) const
        {
            typedef typename IntersectionIteratorType::Geometry
                IntersectionGeometryType;
            const IntersectionGeometryType& intersectionGeoemtry = intIt.intersectionGlobal();
            assert( intersectionGeoemtry.corners() == 2 );
            typedef typename IntersectionIteratorType::ctype
                ctype;
            const int dimworld = IntersectionIteratorType::dimensionworld;
            typedef Dune::FieldVector< ctype, dimworld >
                DomainType;
            const DomainType cornerOne = intersectionGeoemtry[0];
            const DomainType cornerTwo = intersectionGeoemtry[1];
            const DomainType difference = cornerOne - cornerTwo;
            return difference.two_norm();
        }

        // VelocityRangeType is expected to be a FieldVector,
        // SigmaJacobianRangeType to be a Matrixmapping and
        // SigmaJacobianRangeType[i] to be a FieldVector
        SigmaJacobianRangeType prepareVelocityRangeTypeForSigmaDivergence( const VelocityRangeType& arg ) const
        {
            SigmaJacobianRangeType ret( 0.0 );
            assert( arg.dim() == ret[0].dim() );
            for ( int i = 0; i < arg.dim() ; ++i ) {
                for ( int j = 0; j < arg.dim(); ++j ) {
                    VelocityRangeType row( 0.0 );
                    row[ j ] = arg[ i ];
                    ret[ i * arg.dim() + j ] = row;
                }
            }
            return ret;
        }

        VelocityJacobianRangeType preparePressureRangeTypeForVelocityDivergence( const PressureRangeType& arg ) const
        {
            VelocityJacobianRangeType ret( 0.0 );
            for ( int i = 0; i < ret[0].dim(); ++i ) {
                VelocityRangeType row( 0.0 );
                row[ i ] = arg;
                ret[ i ] = row;
            }
        }


};

}
#endif  // end of stokespass.hh
