/**
 *  \file   dune_stokes.cc
 *
 *  \brief  brief
 **/

#ifdef HAVE_CONFIG_H
	#include "config.h"
#endif

#ifdef NDEBUG
	#define DNDEBUG
#endif

#include <cstdio>
#if defined(USE_PARDG_ODE_SOLVER) && defined(USE_BFG_CG_SCHEME)
	#warning ("USE_PARDG_ODE_SOLVER enabled, might conflict with custom solvers")
#endif

//the adaption manager might be troublesome with certain gridparts/spaces, so we needed a easy way to disable it
#ifndef ENABLE_ADAPTIVE
    #define ENABLE_ADAPTIVE 1
#endif

#if defined(UGGRID) && defined(DEBUG)
    #warning ("UGGRID in debug mode is likely to produce a segfault")
#endif

#if ! defined(POLORDER)
    #define POLORDER 0
    #warning ("using default polorder 0 for all spaces")
#endif

#if ! defined(PRESSURE_POLORDER)
    #define PRESSURE_POLORDER POLORDER
#endif

#if ! defined(VELOCITY_POLORDER)
    #define VELOCITY_POLORDER POLORDER
#endif

#if ! defined(DIRICHLET_DATA)
	#define DIRICHLET_DATA DirichletData
#endif

#if ( ( defined(SGRID) || defined(ALUGRID_SIMPLEX) ||  defined(ALUGRID_CUBE) ) && ( GRIDDIM == 3 ) ) || defined(UGGRID) || defined(YASPGRID)
    //this is no mistake, ALU is indeed only incompatible in 3d
    #define OLD_DUNE_GRID_VERSION
#endif

#define USE_GRPAE_VISUALISATION (HAVE_GRAPE && !defined( AORTA_PROBLEM ))

#include <vector>
#include <string>

#include <iostream>
#include <cmath>
#include <dune/fem/misc/mpimanager.hh> // An initializer of MPI
#include <dune/common/exceptions.hh> // We use exceptions
#include <dune/grid/common/capabilities.hh>

//!ATTENTION: undef's GRIDDIM
#include <dune/grid/io/file/dgfparser/dgfgridtype.hh> // for the grid

#include <dune/fem/solver/oemsolver/oemsolver.hh>
#include <dune/fem/space/dgspace.hh>
#include <dune/fem/space/combinedspace.hh>
#include <dune/fem/space/dgspace/dgadaptiveleafgridpart.hh>
#include <dune/fem/pass/pass.hh>
#include <dune/fem/function/adaptivefunction.hh> // for AdaptiveDiscreteFunction
#include <dune/stuff/femeoc.hh>
#include <dune/fem/misc/gridwidth.hh>

#include <dune/stokes/discretestokesfunctionspacewrapper.hh>
#include <dune/stokes/discretestokesmodelinterface.hh>
#include <dune/stokes/stokespass.hh>
#include <dune/stokes/boundarydata.hh>

#include <dune/stuff/printing.hh>
#include <dune/stuff/misc.hh>
#include <dune/stuff/logging.hh>
#include <dune/stuff/parametercontainer.hh>
#include <dune/stuff/postprocessing.hh>
#include <dune/stuff/profiler.hh>

#include "analyticaldata.hh"
#include "velocity.hh"
#include "pressure.hh"
#include "problem.hh"
#include "estimator.hh"

#ifndef COMMIT
    #define COMMIT "undefined"
#endif

static const std::string commit_string (COMMIT);

#if ENABLE_MPI
        typedef Dune::CollectiveCommunication< MPI_Comm > CollectiveCommunication;
#else
        typedef Dune::CollectiveCommunication< double > CollectiveCommunication;
#endif

//! return type of getXXX_Permutations()
typedef std::vector<Dune::StabilizationCoefficients>
    CoeffVector;

//! the strings used for column headers in tex output
typedef std::vector<std::string>
    ColumnHeaders;


/** \brief one single application of the discretisation and solver

    \param  mpicomm
            mostly useless atm, but mandatory
    \param  refine_level_factor
            integer to be multiplied by Dune::DGFGridInfo< GridType >::refineStepsForHalf()
            to get the used refine level for the constructed grid
    \param  stabil_coeff
            the set of coefficients to be used in the run. Default is used in all run types but StabRun().

**/
RunInfo singleRun(  CollectiveCommunication& mpicomm,
                    int refine_level_factor,
					Dune::StabilizationCoefficients& stabil_coeff );

//! output alert for neg. EOC
typedef std::vector<RunInfo> RunInfoVector;
void eocCheck( const RunInfoVector& runInfos );

/**
 *  \brief  main function
 *
 *  ParameterContainer and Logger setup, select run type from parameter file and go
 *
 *  \param  argc
 *          number of arguments from command line
 *  \param  argv
 *          array of arguments from command line
 **/
int main( int argc, char** argv )
{
  try{

	Dune::MPIManager::initialize(argc, argv);
	//assert( Dune::Capabilities::isParallel< GridType >::v );
	CollectiveCommunication mpicomm( Dune::MPIManager::helper().getCommunicator() );

    /* ********************************************************************** *
     * initialize all the stuff we need                                       *
     * ********************************************************************** */
    if ( argc < 2 ) {
        std::cerr << "\nUsage: " << argv[0] << " parameterfile \n" << "\n\t --- OR --- \n";
        std::cerr << "\nUsage: " << argv[0] << " paramfile:"<<"file" << " more-opts:val ..." << std::endl;
        std::cerr << "\nUsage: " << argv[0] << " -d paramfile "<< "\n\t(for displaying solutions in grape) "<< std::endl;
        Parameters().PrintParameterSpecs( std::cerr );
        std::cerr << std::endl;
        return 2;
    }
#if USE_GRPAE_VISUALISATION
    if ( !strcmp( argv[1], "-d" ) || !strcmp( argv[1], "-r" ) ) {
        return display( argc, argv );
    }
#endif
    if ( !(  Parameters().ReadCommandLine( argc, argv ) ) ) {
        return 1;
    }

    // LOG_NONE = 1, LOG_ERR = 2, LOG_INFO = 4,LOG_DEBUG = 8,LOG_CONSOLE = 16,LOG_FILE = 32
    //--> LOG_ERR | LOG_INFO | LOG_DEBUG | LOG_CONSOLE | LOG_FILE = 62
    const bool useLogger = false;
    Logger().Create( Parameters().getParam( "loglevel",         62,                         useLogger ),
                     Parameters().getParam( "logfile",          std::string("dune_stokes"), useLogger ),
                     Parameters().getParam( "fem.io.logdir",    std::string(),              useLogger )
                    );

    int err = 0;

	profiler().Reset( 1 );
	RunInfoVector rf;
	Dune::StabilizationCoefficients st = Dune::StabilizationCoefficients::getDefaultStabilizationCoefficients();
	st.FactorFromParams( "C11" );
	st.FactorFromParams( "C12" );
	st.FactorFromParams( "D11" );
	st.FactorFromParams( "D12" );
	rf.push_back(singleRun( mpicomm, Parameters().getParam( "minref", 0 ), st ) );
	profiler().Output( mpicomm, rf );

    Logger().Dbg() << "\nRun from: " << commit_string << std::endl;
    return err;
  }



  catch (Dune::Exception &e){
    std::cerr << "Dune reported error: " << e << std::endl;
  }
  catch ( std::bad_alloc& b ) {
      std::cerr << "Memory allocation failed: " << b.what() ;
      Logger().Info().Resume();
      Stuff::meminfo( Logger().Info() );
  }
  catch ( assert_exception& a ) {
      std::cerr << "Exception thrown at:\n" << a.what() << std::endl ;
  }
  catch (...){
    std::cerr << "Unknown exception thrown!" << std::endl;
  }
}

RunInfo singleRun(  CollectiveCommunication& mpicomm,
                    int refine_level_factor,
					Dune::StabilizationCoefficients& stabil_coeff )
{
    profiler().StartTiming( "SingleRun" );
    Logging::LogStream& infoStream = Logger().Info();
    Logging::LogStream& debugStream = Logger().Dbg();
	stabil_coeff.print( infoStream );
    RunInfo info;

    debugStream << "\nsingleRun( ";
    stabil_coeff.print( debugStream );
    debugStream << " )" << std::endl;

    /* ********************************************************************** *
     * initialize the grid                                                    *
     * ********************************************************************** */
    infoStream << "\n- initialising grid" << std::endl;
    const int gridDim = GridType::dimensionworld;
    static Dune::GridPtr< GridType > gridPtr( Parameters().DgfFilename( gridDim ) );
    static bool firstRun = true;
    int refine_level = ( refine_level_factor  ) * Dune::DGFGridInfo< GridType >::refineStepsForHalf();
    if ( firstRun && refine_level_factor > 0 ) { //since we have a couple of local statics, only do this once, further refinement done in estimator
        refine_level = ( refine_level_factor ) * Dune::DGFGridInfo< GridType >::refineStepsForHalf();
        gridPtr->globalRefine( refine_level );
    }
	const int polOrder = POLORDER;

	typedef Dune::AdaptiveLeafGridPart< GridType >
        GridPartType;
    typedef Dune::DiscreteStokesModelDefaultTraits<
                    GridPartType,
                    Force,
					DefaultDirichletDataTraits<DIRICHLET_DATA>,
                    gridDim,
                    polOrder,
                    VELOCITY_POLORDER,
                    PRESSURE_POLORDER >
        StokesModelTraitsImp;
    typedef Dune::DiscreteStokesModelDefault< StokesModelTraitsImp >
        StokesModelImpType;
    // treat as interface
    typedef Dune::DiscreteStokesModelInterface< StokesModelTraitsImp >
        StokesModelType;
    // function wrapper for the solutions
    typedef StokesModelTraitsImp::DiscreteStokesFunctionSpaceWrapperType
        DiscreteStokesFunctionSpaceWrapperType;
    typedef StokesModelTraitsImp::DiscreteStokesFunctionWrapperType
        DiscreteStokesFunctionWrapperType;

    static GridPartType gridPart( *gridPtr );
    static DiscreteStokesFunctionSpaceWrapperType
        discreteStokesFunctionSpaceWrapper( gridPart );

    static DiscreteStokesFunctionWrapperType
        computedSolutions(  "computed_",
                            discreteStokesFunctionSpaceWrapper,
                            gridPart );
	DiscreteStokesFunctionWrapperType
		dummyFunctions(  "dummy_",
							discreteStokesFunctionSpaceWrapper,
							gridPart );

	info.codim0 = gridPtr->size( 0 );
	const double grid_width = Dune::GridWidth::calcGridWidth( gridPart );
    infoStream << "  - max grid width: " << grid_width << std::endl;
    info.grid_width = grid_width;
    /* ********************************************************************** *
     * initialize problem                                                     *
     * ********************************************************************** */
    infoStream << "\n- initialising problem" << std::endl;


    debugStream << "  - polOrder: " << polOrder << std::endl;
	const double alpha_param = Parameters().getParam( "alpha", 1.0 );
	const double alpha = 1.0;
	const double scale_factor = 1 / alpha_param;
    const double viscosity_param = Parameters().getParam( "viscosity", 1.0 ) ;
	const double viscosity = viscosity_param * scale_factor;
    debugStream << "  - viscosity: " << viscosity << std::endl;
	debugStream << "  - scaling  : " << scale_factor << std::endl;

    typedef StokesModelTraitsImp::AnalyticalForceType
        AnalyticalForceType;
	AnalyticalForceType analyticalForce( viscosity_param, discreteStokesFunctionSpaceWrapper.discreteVelocitySpace(), alpha_param, scale_factor );

    typedef StokesModelTraitsImp::AnalyticalDirichletDataType
        AnalyticalDirichletDataType;
	AnalyticalDirichletDataType analyticalDirichletData =
			StokesModelTraitsImp::AnalyticalDirichletDataTraitsImplementation::getInstance( discreteStokesFunctionSpaceWrapper );

    StokesModelImpType stokesModel( stabil_coeff,
                                    analyticalForce,
                                    analyticalDirichletData,
									viscosity ,
									alpha,
									scale_factor,
									scale_factor );

    /* ********************************************************************** *
     * initialize passes                                                      *
     * ********************************************************************** */
    infoStream << "\n- starting pass" << std::endl;

    typedef Dune::StartPass< DiscreteStokesFunctionWrapperType, -1 >
        StartPassType;
    StartPassType startPass;

	typedef Dune::StokesPass< StokesModelImpType, StartPassType, 0 >
        StokesPassType;
    StokesPassType stokesPass(  startPass,
                                stokesModel,
                                gridPart,
								discreteStokesFunctionSpaceWrapper,
								dummyFunctions.discreteVelocity(),
								false );

    profiler().StartTiming( "Pass -- APPLY" );
	stokesPass.apply( computedSolutions, computedSolutions );
    profiler().StopTiming( "Pass -- APPLY" );
    info.run_time = profiler().GetTiming( "Pass -- APPLY" );
    stokesPass.getRuninfo( info );

    /* ********************************************************************** *
     * Problem postprocessing
     * ********************************************************************** */
    infoStream << "\n- postprocesing" << std::endl;


    profiler().StartTiming( "Problem/Postprocessing" );

#if defined (AORTA_PROBLEM) || defined (COCKBURN_PROBLEM) || defined (GENERALIZED_STOKES_PROBLEM) //bool tpl-param toggles ana-solution output in post-proc
	typedef Problem< gridDim, DiscreteStokesFunctionWrapperType, true, AnalyticalDirichletDataType >
        ProblemType;
#else
	typedef Problem< gridDim, DiscreteStokesFunctionWrapperType, false, AnalyticalDirichletDataType >
        ProblemType;
#endif
	ProblemType problem( viscosity , computedSolutions, analyticalDirichletData );

    typedef PostProcessor< StokesPassType, ProblemType >
        PostProcessorType;

    PostProcessorType postProcessor( discreteStokesFunctionSpaceWrapper, problem );

    postProcessor.save( *gridPtr, computedSolutions, refine_level );
    info.L2Errors = postProcessor.getError();
    typedef Dune::StabilizationCoefficients::ValueType
        Pair;
    info.c11 = Pair( stabil_coeff.Power( "C11" ), stabil_coeff.Factor( "C11" ) );
    info.c12 = Pair( stabil_coeff.Power( "C12" ), stabil_coeff.Factor( "C12" ) );
    info.d11 = Pair( stabil_coeff.Power( "D11" ), stabil_coeff.Factor( "D11" ) );
    info.d12 = Pair( stabil_coeff.Power( "D12" ), stabil_coeff.Factor( "D12" ) );
    info.bfg = Parameters().getParam( "do-bfg", true );
    info.gridname = gridPart.grid().name();
    info.refine_level = refine_level;

    info.polorder_pressure = StokesModelTraitsImp::pressureSpaceOrder;
    info.polorder_sigma = StokesModelTraitsImp::sigmaSpaceOrder;
    info.polorder_velocity = StokesModelTraitsImp::velocitySpaceOrder;

    info.solver_accuracy = Parameters().getParam( "absLimit", 1e-4 );
    info.inner_solver_accuracy = Parameters().getParam( "inner_absLimit", 1e-4 );
    info.bfg_tau = Parameters().getParam( "bfg-tau", 0.1 );

	info.problemIdentifier = StokesProblem::ProblemIdentifier;

    profiler().StopTiming( "Problem/Postprocessing" );
    profiler().StopTiming( "SingleRun" );

    firstRun = false;

    return info;
}

void eocCheck( const RunInfoVector& runInfos )
{
	bool ups = false;
	RunInfoVector::const_iterator it = runInfos.begin();
	RunInfo last = *it;
	++it;
	for ( ; it != runInfos.end(); ++it ) {
		ups = ( last.L2Errors[0] < it->L2Errors[0]
			|| last.L2Errors[1] < it->L2Errors[1] );
		last = *it;
	}
	if ( ups ) {
		Logger().Err() 	<< 	"----------------------------------------------------------\n"
						<<	"-                                                        -\n"
						<<	"-                  negative EOC                          -\n"
						<<	"-                                                        -\n"
						<< 	"----------------------------------------------------------\n"
						<< std::endl;
	}
}
