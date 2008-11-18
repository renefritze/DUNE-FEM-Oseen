/**
 *  \file   postprocessing.hh
 *  \brief  postprocessing.hh
 **/

#ifndef POSTPROCESSING_HH
#define POSTPROCESSING_HH

#include <dune/fem/operator/lagrangeinterpolation.hh>
#include <dune/fem/io/file/datawriter.hh>

#include "logging.hh"
#include "problem.hh"
#include "parametercontainer.hh"

template <  class ProblemImp,
            class GridPartImp,
            class DiscreteVelocityFunctionImp,
            class DiscretePressureFunctionImp >
class PostProcessor
{
    public:
        typedef ProblemImp
            ProblemType;
        typedef typename ProblemType::VelocityType
            ContinuousVelocityType;

        typedef GridPartImp
            GridPartType;
        typedef typename GridPartType::GridType
            GridType;

        typedef DiscreteVelocityFunctionImp
            DiscreteVelocityFunctionType;
        typedef typename DiscreteVelocityFunctionType::DiscreteFunctionSpaceType
            DiscreteVelocityFunctionSpaceType;

        typedef DiscretePressureFunctionImp
            DiscretePressureFunctionType;
        typedef typename DiscretePressureFunctionType::DiscreteFunctionSpaceType
            DiscretePressureFunctionSpaceType;



        PostProcessor( const ProblemType& problem, const GridPartType& gridPart,
                        const DiscreteVelocityFunctionSpaceType& velocity_space,
                        const DiscretePressureFunctionSpaceType& press_space)
            : problem_( problem ),
            gridPart_( gridPart ),
            velocitySpace_ ( velocity_space ),
            discreteExactVelocity_( "u_exact", velocity_space ),
            discreteExactForce_( "f_exact", velocity_space ),
            discreteExactDirichlet_( "gd_exact", velocity_space ),
            discreteExactPressure_( "p_exact", press_space )
        {

        }

        ~PostProcessor()
        {
        }

        void assembleExactSolution()
        {
            Dune::LagrangeInterpolation< DiscreteVelocityFunctionType >::interpolateFunction( problem_.velocity(), discreteExactVelocity_ );
            Dune::LagrangeInterpolation< DiscreteVelocityFunctionType >::interpolateFunction( problem_.dirichletData(), discreteExactDirichlet_ );
            Dune::LagrangeInterpolation< DiscreteVelocityFunctionType >::interpolateFunction( problem_.force(), discreteExactForce_ );
            Dune::LagrangeInterpolation< DiscretePressureFunctionType >::interpolateFunction( problem_.velocity(), discreteExactPressure_ );
        }

        void save( const GridType& grid )
        {
            typedef Dune::Tuple<  DiscreteVelocityFunctionType*, DiscreteVelocityFunctionType*,
                            DiscreteVelocityFunctionType*, DiscretePressureFunctionType*>
                    IOTupleType;
            IOTupleType dataTup (   &discreteExactVelocity_,
                                    &discreteExactForce_,
                                    &discreteExactDirichlet_,
                                    &discreteExactPressure_     );

            typedef Dune::DataWriter<GridType, IOTupleType> DataWriterType;
            //DataWriterType dataWriter( *grid, filename, dataTup, startTime, endTime );
            DataWriterType dataWriter( grid, Parameters().ParameterFilename(), dataTup, 0, 0 );

            dataWriter.write(0.0, 0);
        }

    private:
        const ProblemType& problem_;
        //ContinuousVelocityType& continuousVelocity_;
        const GridPartType& gridPart_;
        const DiscreteVelocityFunctionSpaceType& velocitySpace_;
        DiscreteVelocityFunctionType discreteExactVelocity_;
        DiscreteVelocityFunctionType discreteExactForce_;
        DiscreteVelocityFunctionType discreteExactDirichlet_;
        DiscretePressureFunctionType discreteExactPressure_;
};

#endif // end of postprocessing.hh
