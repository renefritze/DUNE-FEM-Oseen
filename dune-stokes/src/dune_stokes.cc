/** \file dune_stokes.cc
    \brief  brief
 **/

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
#include <iostream>
#include <memory> //not including this gives error of undefined autopointer in dgfparser.hh
#include <dune/common/mpihelper.hh> // An initializer of MPI
#include <dune/common/exceptions.hh> // We use exceptions
#include <dune/grid/io/file/dgfparser/dgfgridtype.hh> // for the grid

//#include "traits.hh"
#include "parametercontainer.hh"
//#include "logging.hh"
#include "problem.hh"

/**
 *  \brief main function
 *
 *  \c more main function
 *
 *  \attention attention
 *
 *  \param argc number of arguments from command line
 *  \param argv array of arguments from command line
 **/
int main( int argc, char** argv )
{
  try{
    //Maybe initialize Mpi
    //Dune::MPIHelper& helper = Dune::MPIHelper::instance(argc, argv);

    /* ********************************************************************** *
     * initialize all the stuff we need                                       *
     * ********************************************************************** */
    ParameterContainer parameters( argc, argv );
    if ( !( parameters.ReadCommandLine() ) ) {
        return 1;
    }
    if ( !( parameters.SetUp() ) ) {
        return 1;
    }
    else {
        parameters.Print( std::cout );
    }
    //Logger().Create( Logging::LOG_CONSOLE | Logging::LOG_DEBUG | Logging::LOG_INFO );

    /* ********************************************************************** *
     * initialize the grid                                                    *
     * ********************************************************************** */
    Dune::GridPtr<GridType> gridptr( "grid.dgf" );

    /* ********************************************************************** *
     * initialize the analytical problem                                      *
     * ********************************************************************** */
    Velocity< GridType > velocity();
    enum {
        grid_dimension = GridType::dimensionworld
    };
    Dune::FieldVector< double, grid_dimension > x;
    x[0] = 0.5;
    x[1] = 0.5;
    std::cout << "TEST";


    return 0;
  }
  catch (Dune::Exception &e){
    std::cerr << "Dune reported error: " << e << std::endl;
  }
  catch (...){
    std::cerr << "Unknown exception thrown!" << std::endl;
  }
}
