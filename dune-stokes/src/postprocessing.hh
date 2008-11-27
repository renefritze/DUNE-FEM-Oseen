/**
 *  \file   postprocessing.hh
 *  \brief  postprocessing.hh
 **/

#ifndef POSTPROCESSING_HH
#define POSTPROCESSING_HH

#include <dune/fem/operator/projection/l2projection.hh>
#include <dune/fem/io/file/vtkio.hh>

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
        typedef typename ProblemType::PressureType
            ContinuousPressureType;
        typedef typename ProblemType::ForceType
            ForceType;
        typedef typename ProblemType::DirichletDataType
            DirichletDataType;

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
            discreteExactPressure_( "p_exact", press_space ),
            solutionAssembled_(false)
        {

        }

        ~PostProcessor()
        {
        }

        void assembleExactSolution()
        {
            typedef Dune::L2Projection< double, double, ContinuousVelocityType, DiscreteVelocityFunctionType > ProjectionV;
                ProjectionV projectionV;
            projectionV( problem_.velocity(), discreteExactVelocity_ );

            typedef Dune::L2Projection< double, double, DirichletDataType, DiscreteVelocityFunctionType > ProjectionD;
                ProjectionD projectionD;
            projectionD( problem_.dirichletData(), discreteExactDirichlet_ );

            typedef Dune::L2Projection< double, double, ForceType, DiscreteVelocityFunctionType > ProjectionF;
                ProjectionF projectionF;
            projectionF( problem_.force(), discreteExactForce_ );

            typedef Dune::L2Projection< double, double, ContinuousPressureType, DiscretePressureFunctionType > ProjectionP;
                ProjectionP projectionP;
            projectionP( problem_.pressure(), discreteExactPressure_ );
        }

        void save( const GridType& grid )
        {
            if ( !solutionAssembled_ )
                assembleExactSolution();

            typedef Dune::VTKIO<GridPartType> VTKWriterType;
            VTKWriterType vtkWriter( gridPart_ );
			vtkWriter.addVertexData( discreteExactVelocity_ );
//			vtkWriter.write(( "data/discreteExactVelocity_" ) );
//			vtkWriter.clear();
			vtkWriter.addVertexData( discreteExactPressure_ );
//			vtkWriter.write(( "data/discreteExactPressure_" ) );
//			vtkWriter.clear();
			vtkWriter.addVertexData( discreteExactForce_ );
//			vtkWriter.write(( "data/discreteExactForce_" ) );
//			vtkWriter.clear();
			vtkWriter.addVertexData( discreteExactDirichlet_ );
			//vtkWriter.write(( "data/discreteExactDirichlet_" ) );
			vtkWriter.pwrite( "funcs", "/share/projekte/uni/diplomarbeit/dune-code/dune-stokes/src/data" , "." );
			vtkWriter.write( "data/funcs"  );
			vtkWriter.clear();

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
        bool solutionAssembled_;
};

#endif // end of postprocessing.hh
