/**************************************************************************
**       Title: examples of elliptic models
**    $RCSfile$
**   $Revision: 2745 $$Name$
**       $Date: 2007-12-05 21:08:12 +0100 (Wed, 05 Dec 2007) $
**   Copyright: GPL $Author: nolte $
** Description: implementation of default elliptic models for
**              use in solvers such as FEOp. A model is a class providing
**              all data functions of a problem. The model is assumed to
**              be instantiated once and passed to solvers. It is depending
**              on a traits class providing typedefs.
**
**************************************************************************/
#ifndef DUNE_ELLIPTICMODEL_HH
#define DUNE_ELLIPTICMODEL_HH

#include <sstream>

namespace Dune
{

  struct PoissonModelProperties
  {
    enum { hasDirichletValues = true };
    enum { hasNeumannValues = true };
    enum { hasRobinValues = true };
    enum { hasGeneralizedNeumannValues = true };
    enum { hasConvectiveFlux = false };
    enum { hasMass = false };
    enum { hasSource = true };
  };

  /** \class PoissonModel
   *  \brief The PoissonModel class provides a default model for an elliptic
   *  problem to be handled by FEOp
   *
   *  The model problem simply is
   *  \f[ - \mathrm{div} \nabla u = n \pi^2 \prod_{i=1}^n \sin( \pi x_i ). \f]
   *  Using homogeneous Dirichlet boundary values, the exact solution on the unit
   *  square is \f$u( x ) = \prod_{i=1}^n \sin( \pi x_i ) \f$
   *
   *  All types are extracted from the TraisImp, which defaults to
   *  DefaultElementMatrixTraits.
   */
  template< class FunctionSpaceImp >
  class PoissonModel
  : public LinearEllipticModelDefault
    < FunctionSpaceImp, PoissonModel< FunctionSpaceImp >, PoissonModelProperties >
  {
  public:
    typedef FunctionSpaceImp FunctionSpaceType;

    typedef PoissonModelProperties Properties;

  private:
    typedef PoissonModel< FunctionSpaceType > ThisType;
    typedef LinearEllipticModelDefault
      < FunctionSpaceType, ThisType, Properties >
      BaseType;

  public:
    typedef typename BaseType :: BoundaryType BoundaryType;

    typedef typename FunctionSpaceType :: DomainType DomainType;
    typedef typename FunctionSpaceType :: RangeType RangeType;
    typedef typename FunctionSpaceType :: JacobianRangeType JacobianRangeType;

    typedef typename FunctionSpaceType :: DomainFieldType DomainFieldType;
    typedef typename FunctionSpaceType :: RangeFieldType RangeFieldType;

  public:
    using BaseType :: diffusiveFlux;
    using BaseType :: source;

  public:
    template< class IntersectionType >
    inline BoundaryType boundaryType( const IntersectionType &intersection ) const
    {
        return BaseType :: Dirichlet;
    }

    template< class IntersectionType, class QuadratureType >
    inline void dirichletValues( const IntersectionType &intersection,
                                 const QuadratureType &quadrature,
                                 int p,
                                 RangeType &ret ) const
    {
      typedef typename IntersectionType::Entity EntityType;

      const int dimension = DomainType::dimension;

      const DomainType &x = intersection.inside()->geometry().global( quadrature.point( p ) );

      ret[ 0 ] = 1.0;
      for( int i = 0; i < dimension; ++i )
        ret[ 0 ] *= sin( M_PI * x[ i ] );
    }

    template< class IntersectionType, class QuadratureType >
    inline void neumannValues( const IntersectionType &intersection,
                               const QuadratureType &quadrature,
                               int p,
                               RangeType &ret ) const
    {
      std :: cout << "Neumann boundary values are not implemented." << std :: endl;
      assert( false );

      ret[ 0 ] = 0.0;
    }

    //! determine robin value in a boundary point used in a quadrature
    template< class IntersectionType, class QuadratureType >
    inline void robinValues( const IntersectionType &intersection,
                             const QuadratureType &quadrature,
                             int p,
                             RangeType &ret ) const
    {
      std :: cout << "Robin boundary values are not implemented." << std :: endl;
      assert( false );

      //const DomainType &x = entity.geometry().global( quadrature.point( p ) );

      ret[ 0 ] = 0.0;
    }

    template< class EntityType, class PointType >
    inline void source ( const EntityType &entity,
                         const PointType &x,
                         RangeType &ret ) const
    {
      const int dimension = DomainType :: dimension;

      const DomainType &global = entity.geometry().global( coordinate( x ) );

      ret[ 0 ] = (dimension * M_PI * M_PI);
      for( int i = 0; i < dimension; ++i )
        ret[ 0 ] *= sin( M_PI * global[ i ] );
    }

    template< class EntityType, class PointType >
    inline void diffusiveFlux ( const EntityType &entity,
                                const PointType &x,
                                const JacobianRangeType &gradient,
                                JacobianRangeType &flux ) const
    {
      flux = gradient;
    }

    template< class IntersectionType, class QuadratureType >
    inline double robinAlpha( const IntersectionType &intersection,
                              const QuadratureType &quadrature,
                              int p ) const
    {
      return 1.0;
    }
  };  // end of PoissonModel class



  /*======================================================================*/
  /*!
   *  \class PoissonExactSolution
   *  \brief The class provides the exact solution for the model given by
   *         the PoissonModel class
   *
   *  The function represents u = x(1-x)y(1-y), which is the solution of
   *  the model problem  - div grad u = 2(x(1-x)+ y(1-y)) on
   *  the unit square with homogeneous Dirichlet boundary values.
   *  The function can be used for EOC calculation
   */
  /*======================================================================*/
  template< class FunctionSpaceImp >
  class PoissonExactSolution
  : public Function< FunctionSpaceImp, PoissonExactSolution< FunctionSpaceImp > >
  {
  public:
    typedef FunctionSpaceImp FunctionSpaceType;

  private:
    typedef PoissonExactSolution< FunctionSpaceType > ThisType;
    typedef Function< FunctionSpaceType, ThisType > BaseType;

  public:
    typedef typename FunctionSpaceType :: DomainType DomainType;
    typedef typename FunctionSpaceType :: RangeType RangeType;
    typedef typename FunctionSpaceType :: JacobianRangeType JacobianRangeType;

    typedef typename FunctionSpaceType :: DomainFieldType DomainFieldType;
    typedef typename FunctionSpaceType :: RangeFieldType RangeFieldType;

  public:
    inline PoissonExactSolution ( const FunctionSpaceType &functionSpace )
    : BaseType( functionSpace )
    {
    }

    /*
    //! u(x,y,z) = (x-x^2)*(y-y^2)*(z-z^2)
    void evaluate (const DomainType & x , RangeType & ret) const
    {
      ret = 1.0;
      for(int i=0; i<DomainType::dimension; i++)
          ret *= ( x[i] - SQR(x[i]) );
      //    ret[0] += x[0]+x[1];
      // ret += 1.0;   // add dirichlet-values!!
    }
    */

    //! u( x ) = sin( pi x_1 ) * ... * sin( pi x_n )
    inline void evaluate ( const DomainType &x, RangeType &ret ) const
    {
      enum { dimension = DomainType :: dimension };

      ret[ 0 ] = 1.0;
      for( int i = 0; i < dimension; ++i )
        ret[ 0 ] *= sin( M_PI * x[ i ] );
    }

    inline void evaluate( const DomainType &x, RangeFieldType t, RangeType &ret ) const
    {
      evaluate( x , ret );
    }
  };



  /*======================================================================*/
  /*!
   *  \class Elliptic2dModel
   *  \brief The Elliptic2dModel class provides a complete model
   *         for an elliptic problem to be handled by FEOp
   *
   *  The model problem is defined on the 2D unit square with
   *        - div ( a grad u - b u) + cu = f
   *            u = g_D on Dirichlet-boundary (upper and lower edge)
   *      (a grad u - bu ) n = g_N on Neuman boundary (left edge)
   *      (a grad u - bu ) n + alpha u= g_R on Robin boundary (right edge)
   *
   *  The data functions are parametrized by nonnegative scalar q,r,s for
   *  switching on/off certain contributions and regulating the stability
   *     alpha = 1
   *     a = [(1+q), -q; -q,  (1+q)]
   *     b = [1; 1] * s * y
   *     c = xy * r
   *     f = 2q + r* x^2y^2 + r* x^2y + s * (x+y+y^2 + 2xy)
   *     g_D = xy+x
   *     g_N = -(1+q)(y+1)
   *     g_R = 2 - s*y^2 + (2+q-s)* y
   *
   *  The solution is simply u = xy+x
   *
   *  All types are extracted from the DefaultElementMatrixTraits class
   *  additionally, the model contains member variables and methods.
   */
  /*======================================================================*/
  template< class FunctionSpaceImp >
  class Elliptic2dModel
  : public LinearEllipticModelDefault< FunctionSpaceImp, Elliptic2dModel< FunctionSpaceImp > >
  {
  public:
    typedef FunctionSpaceImp FunctionSpaceType;

  private:
    typedef Elliptic2dModel< FunctionSpaceType > ThisType;
    typedef LinearEllipticModelDefault< FunctionSpaceType, ThisType > BaseType;

  public:
    typedef typename FunctionSpaceType :: DomainType DomainType;
    typedef typename FunctionSpaceType :: RangeType RangeType;
    typedef typename FunctionSpaceType :: JacobianRangeType JacobianRangeType;

    typedef typename FunctionSpaceType :: DomainFieldType DomainFieldType;
    typedef typename FunctionSpaceType :: RangeFieldType RangeFieldType;

    static const double q = 1.0; // q>0 => non-unit diffusivity
    static const double r = 1.0; // r>0 => mass term activated
    static const double s = 1.0; // 0.001; // s>0 => convective term activated

  public:
    typedef typename BaseType :: BoundaryType BoundaryType;

  public:
    using BaseType :: diffusiveFlux;
    using BaseType :: convectiveFlux;
    using BaseType :: mass;
    using BaseType :: source;

  public:
    //! constructor with functionspace argument such that the space and the
    //! grid is available
    Elliptic2dModel()
    {
      // currently implementation is fitted to 2 dimensions
      assert( dimworld == 2 );
    }

    //! return boundary type of a boundary point p used in a quadrature
    template< class IntersectionType >
    inline BoundaryType boundaryType ( const IntersectionType &intersection ) const
    {
      const int boundaryId = intersection.boundaryId();

      switch( boundaryId )
      {
      case 1:
        return BaseType :: Dirichlet;

      case 2:
        return BaseType :: Neumann;

      case 3:
        return BaseType :: Robin;

      default:
        std :: ostringstream stream;
        stream << "Unknown boundary id: " << boundaryId << ".";
        DUNE_THROW( RangeError, stream.str() );
      }
    }

    //! determine dirichlet value in a boundary point used in a quadrature
    template <class IntersectionType, class QuadratureType >
    inline void dirichletValues ( const IntersectionType &intersection,
                                  const QuadratureType& quad, int p,
                                  RangeType& ret ) const
    {
      const DomainType &x = intersection.inside()->geometry().global( quad.point( p ) );
      ret = x[ 0 ] * (1 + x[ 1 ]);
    }

  //! determine neumann value in a boundary point used in a quadrature
    template <class IntersectionType, class QuadratureType>
    inline void neumannValues ( const IntersectionType &intersection,
                                const QuadratureType& quad, int p,
                                RangeType& ret) const
    {
      const DomainType &x = intersection.inside()->geometry().global( quad.point( p ) );
      ret = -(1 + q) * (x[ 1 ] + 1);
    }

    //! determine robin value in a boundary point used in a quadrature
    template <class IntersectionType, class QuadratureType>
    inline void robinValues ( const IntersectionType &intersection,
                              const QuadratureType& quad, int p,
                              RangeType& ret) const
    {
      const DomainType &x = intersection.inside()->geometry().global( quad.point( p ) );
      ret = 2 - s * SQR( x[ 1 ] ) + (2 + q - s) * x[ 1 ];
      //ret = (2 + q - s * x[ 1 ]) * (1 + x[ 1 ]) - q;
    }

    template< class EntityType, class PointType >
    inline void mass ( const EntityType &entity,
                       const PointType &x,
                       RangeType &ret ) const
    {
      const DomainType &global = entity.geometry().global( coordinate( x ) );
      ret = r * global[ 0 ] * global[ 1 ];
    }

    template< class EntityType, class PointType >
    inline void source( const EntityType &entity,
                        const PointType &x,
                        RangeType &ret ) const
    {
      const DomainType &global = entity.geometry().global( coordinate( x ) );
      ret = 2 * q
            + s * (global[ 0 ] + global[ 1 ]) * (1 + global[ 1 ])
            + s * global[ 0 ] * global[ 1 ]
            + r * SQR( global[ 0 ] ) * global[ 1 ] * (1 + global[ 1 ]);
    }

    template< class EntityType, class PointType >
    inline void diffusiveFlux ( const EntityType &entity,
                                const PointType &x,
                                const JacobianRangeType &gradphi,
                                JacobianRangeType &ret ) const
    {
      ret[ 0 ][ 0 ] = (1 + q) * gradphi[ 0 ][ 0 ] - q * gradphi[ 0 ][ 1 ];
      ret[ 0 ][ 1 ] = (1 + q) * gradphi[ 0 ][ 1 ] - q * gradphi[ 0 ][ 0 ];
    }

    template< class EntityType, class PointType >
    inline void convectiveFlux( const EntityType &entity,
                                const PointType &x,
                                const RangeType &phi,
                                JacobianRangeType &ret ) const
    {
      const DomainType &global = entity.geometry().global( coordinate( x ) );
      ret[ 0 ][ 0 ] = -global[ 1 ] * s * phi[ 0 ];
      ret[ 0 ][ 1 ] = -global[ 1 ] * s * phi[ 0 ];
    }

    //! the coefficient for robin boundary condition
    template< class IntersectionType, class QuadratureType >
    inline RangeFieldType robinAlpha ( const IntersectionType &intersection,
                                       const QuadratureType &quadrature,
                                       int pt ) const
    {
      return 1;
    }
  };  // end of Elliptic2dModel class



  /*======================================================================*/
  /*!
   *  \class Elliptic2dExactSolution
   *  \brief The class provides the exact solution for the model given by
   *         the Elliptic2dModel class
   *
   *  The function represents u = x y + x, which is the solution of
   *  the model problem on the unit square with inhomogeneous Dirichlet
   *  Neumann and Dirichlet boundary values.
   *  Function can be used for EOC calculation
   */
  /*======================================================================*/
  template< class FunctionSpaceImp >
  class Elliptic2dExactSolution
  : public Function< FunctionSpaceImp, Elliptic2dExactSolution< FunctionSpaceImp > >
  {
  public:
    typedef FunctionSpaceImp FunctionSpaceType;

  private:
    typedef Elliptic2dExactSolution< FunctionSpaceType > ThisType;
    typedef Function< FunctionSpaceType, ThisType > BaseType;

  public:
    typedef typename FunctionSpaceType :: DomainType DomainType;
    typedef typename FunctionSpaceType :: RangeType RangeType;
    typedef typename FunctionSpaceType :: JacobianRangeType JacobianRangeType;

    typedef typename FunctionSpaceType :: DomainFieldType DomainFieldType;
    typedef typename FunctionSpaceType :: RangeFieldType RangeFieldType;

  public:
    inline Elliptic2dExactSolution ( const FunctionSpaceType &functionSpace )
    : BaseType( functionSpace )
    {
    }

    void evaluate ( const DomainType &x, RangeType &ret ) const
    {
      ret = x[0]*x[1] + x[0];
    }

    void evaluate (const DomainType & x , RangeFieldType time , RangeType & ret) const
    {
      evaluate ( x , ret );
    }
  }; // end class Elliptic2dExactSolution



  /*======================================================================*/
  /*!
   *  \class Elliptic3dModel
   *  \brief The Elliptic3dModel class provides a complete model
   *         for an elliptic problem to be handled by FEOp
   *
   *  The model problem is defined on the 3D unit cube with
   *        - div ( a grad u - b u) + cu = f
   *      (a grad u - bu ) n = g_N on Neuman boundary (x=0 face)
   *      (a grad u - bu ) n + alpha u= g_R on Robin boundary (x=1 face)
   *            u = g_D on Dirichlet-boundary (remaining 4 faces)
   *
   *  The data functions are
   *     alpha = 1
   *     a = [3 -1 -1; -1 3 -1; -1 -1 3]
   *     b = [1; 1; 1] y
   *     c = xy
   *     f = 2z+3y+3x+y^2z+2xyz+xy^2+x^2y^2z+x^2y
   *     g_D = xyz+x
   *     g_N = -3yz-3
   *     g_R = 4yz-2y-z+4-y^2z
   *
   *  The solution is simply u = xyz+x
   *
   *  All types are extracted from the DefaultElementMatrixTraits class
   *  additionally, the model contains member variables and methods.
   */
  /*======================================================================*/
  template< class FunctionSpaceImp >
  class Elliptic3dModel
  : public LinearEllipticModelDefault< FunctionSpaceImp, Elliptic3dModel< FunctionSpaceImp > >
  {
  public:
    typedef FunctionSpaceImp FunctionSpaceType;

  private:
    typedef Elliptic3dModel< FunctionSpaceType > ThisType;
    typedef LinearEllipticModelDefault< FunctionSpaceType, ThisType > BaseType;

  public:
    typedef typename BaseType :: BoundaryType BoundaryType;

    typedef typename FunctionSpaceType :: DomainType DomainType;
    typedef typename FunctionSpaceType :: RangeType RangeType;
    typedef typename FunctionSpaceType :: JacobianRangeType JacobianRangeType;

    typedef typename FunctionSpaceType :: DomainFieldType DomainFieldType;
    typedef typename FunctionSpaceType :: RangeFieldType RangeFieldType;

  public:
    using BaseType :: diffusiveFlux;
    using BaseType :: convectiveFlux;
    using BaseType :: mass;
    using BaseType :: source;

  public:
    //! constructor with functionspace argument such that the space and the
    //! grid is available
    inline Elliptic3dModel ()
    {
      // currently implementation is fitted to 3 dimensions
      assert(dimworld==3);
    }

    //! return boundary type of a boundary point p used in a quadrature
    template< class IntersectionType >
    inline BoundaryType boundaryType( const IntersectionType &intersection ) const
    {
      const int boundaryId = intersection.boundaryId();

      switch( boundaryId )
      {
      case 1:
        return BaseType :: Dirichlet;

      case 2:
        return BaseType :: Neumann;

      case 3:
        return BaseType :: Robin;

      default:
        DUNE_THROW( RangeError, "Unknown boundary id." );
      }
   }

  //! determine dirichlet value in a boundary point used in a quadrature
    template< class IntersectionType, class QuadratureType >
    inline void dirichletValues( const IntersectionType &intersection,
                                 const QuadratureType& quad, int p,
                                 RangeType& ret) const
    {
      const DomainType& glob = intersection.inside()->geometry().global(quad.point(p));
      ret[0] = glob[0] * ( 1.0 + glob[1]*glob[2]);
    }

  //! determine neumann value in a boundary point used in a quadrature
    template< class IntersectionType, class QuadratureType >
    inline void neumannValues( const IntersectionType &intersection,
                               const QuadratureType& quad, int p,
                               RangeType& ret) const
    {
      const DomainType& glob = intersection.inside()->geometry().global(quad.point(p));
      ret[0] = -3.0 * glob[1]*glob[2] - 3.0;
    }

  //! determine robin value in a boundary point used in a quadrature
    template< class IntersectionType, class QuadratureType >
    inline void robinValues( const IntersectionType &intersection,
                             const QuadratureType& quad, int p,
                             RangeType& ret) const
    {
      const DomainType& glob = intersection.inside()->geometry().global(quad.point(p));
      ret[0] = 4 * glob[1] * glob[2] - 2* glob[1] - glob[2] +
          4.0 - SQR(glob[1])*glob[2];
    }

    template< class EntityType, class PointType >
    inline void mass ( const EntityType &entity,
                       const PointType &x,
                       RangeType &ret ) const
    {
      const DomainType &global = entity.geometry().global( coordinate( x ) );
      ret = global[ 0 ] * global[ 1 ];
    }

    template< class EntityType, class PointType >
    inline void source ( const EntityType &entity,
                         const PointType &x,
                         RangeType &ret ) const
    {
      const DomainType &global = entity.geometry().global( coordinate( x ) );
      ret = 2 * global[2] + 3* global[1] + 3 * global[0] +
           SQR(global[1])*global[2] + 2* global[0] * global[1]* global[2] +
                global[0] * SQR(global[1]) + SQR(global[0]*global[1]) * global[2] +
                SQR(global[0]) * global[1];
    }

    template< class EntityType, class PointType >
    inline void diffusiveFlux ( const EntityType &entity,
                                const PointType &x,
                                const JacobianRangeType &gradient,
                                JacobianRangeType &flux ) const
    {
      const DomainType &grad = gradient[ 0 ];
      flux[ 0 ][ 0 ] = 3 * grad[ 0 ] - grad[ 1 ]- grad[ 2 ];
      flux[ 0 ][ 1 ] = -grad[ 0 ] + 3 * grad[ 1 ] - grad[ 2 ];
      flux[ 0 ] [2 ] = -grad[ 0 ] - grad[ 1 ] + 3 * grad[ 2 ];
    }

    template< class EntityType, class PointType >
    inline void convectiveFlux( const EntityType &entity,
                                const PointType &x,
                                const RangeType &phi,
                                JacobianRangeType &ret ) const
    {
      const DomainType global = entity.geometry().global( coordinate( x ) );
      ret[ 0 ][ 0 ] = -global[ 1 ] * phi[ 0 ];
      ret[ 0 ][ 1 ] = -global[ 1 ] * phi[ 0 ];
      ret[ 0 ][ 2 ] = -global[ 1 ] * phi[ 0 ];
    }

    //! the coefficient for robin boundary condition
    template< class IntersectionType, class QuadratureType >
    inline RangeFieldType robinAlpha ( const IntersectionType &intersection,
                                       const QuadratureType &quadrature,
                                       int pt ) const
    {
      return 1;
    }
  };  // end of Elliptic3dModel class

  /*======================================================================*/
  /*!
   *  \class Elliptic3dExactSolution
   *  \brief The class provides the exact solution for the model given by
   *         the Elliptic2dModel class
   *
   *  The function represents u = x y z + x, which is the solution of
   *  the model problem on the unit cube with inhomogeneous Dirichlet
   *  Neumann and Dirichlet boundary values.
   *  Function can be used for EOC calculation
   */
  /*======================================================================*/
  template< class FunctionSpaceImp >
  class Elliptic3dExactSolution
  : public Function< FunctionSpaceImp, Elliptic3dExactSolution< FunctionSpaceImp > >
  {
  public:
    typedef FunctionSpaceImp FunctionSpaceType;

  private:
    typedef Elliptic3dExactSolution< FunctionSpaceType > ThisType;
    typedef Function< FunctionSpaceType, ThisType > BaseType;

  public:
    typedef typename FunctionSpaceType :: DomainType DomainType;
    typedef typename FunctionSpaceType :: RangeType RangeType;
    typedef typename FunctionSpaceType :: JacobianRangeType JacobianRangeType;

    typedef typename FunctionSpaceType :: DomainFieldType DomainFieldType;
    typedef typename FunctionSpaceType :: RangeFieldType RangeFieldType;

  public:
    inline Elliptic3dExactSolution ( const FunctionSpaceType &functionSpace )
    : BaseType( functionSpace )
    {
    }

    inline void evaluate (const DomainType & x , RangeType & ret) const
    {
      ret = x[0]*x[1]*x[2] + x[0];
    }

    inline void evaluate (const DomainType & x , RangeFieldType time , RangeType & ret) const
    {
      evaluate ( x , ret );
    }
  }; // end class Elliptic3dExactSolution

  struct AortaModelProperties
  {
    enum { hasDirichletValues = true };
    enum { hasNeumannValues = false };
    enum { hasRobinValues = false };
    enum { hasGeneralizedNeumannValues = false };
    enum { hasConvectiveFlux = false };
    enum { hasMass = false };
    enum { hasSource = false };
  };

  template< class FunctionSpaceImp >
  class AortaModel
  : public LinearEllipticModelDefault< FunctionSpaceImp, AortaModel < FunctionSpaceImp >, AortaModelProperties >
  {
  public:
    typedef FunctionSpaceImp FunctionSpaceType;

  private:
    typedef AortaModel< FunctionSpaceType > ThisType;
    typedef LinearEllipticModelDefault< FunctionSpaceType, ThisType, AortaModelProperties > BaseType;

  public:
    typedef typename BaseType :: BoundaryType BoundaryType;

    typedef typename FunctionSpaceType :: DomainType DomainType;
    typedef typename FunctionSpaceType :: RangeType RangeType;
    typedef typename FunctionSpaceType :: JacobianRangeType JacobianRangeType;

    typedef typename FunctionSpaceType :: DomainFieldType DomainFieldType;
    typedef typename FunctionSpaceType :: RangeFieldType RangeFieldType;

  public:
    using BaseType :: diffusiveFlux;
    using BaseType :: convectiveFlux;
    using BaseType :: mass;
    using BaseType :: source;

  public:
    //! constructor with functionspace argument such that the space and the
    //! grid is available
    inline AortaModel ()
    {
      // currently implementation is fitted to 3 dimensions
      assert(dimworld==3);
    }

    //! return boundary type of a boundary point p used in a quadrature
    template< class IntersectionType >
    inline BoundaryType boundaryType( const IntersectionType &intersection ) const
    {
//      return BaseType :: Neumann;
      return BaseType :: Dirichlet;
   }

  //! determine dirichlet value in a boundary point used in a quadrature
    template< class IntersectionType, class QuadratureType >
    inline void dirichletValues( const IntersectionType &intersection,
                                 const QuadratureType& quad, int p,
                                 RangeType& ret) const
    {
        assert( false );
        const int boundaryId = intersection.boundaryId();
      double fac = 100.0;
      switch( boundaryId )
      {
      case 1:
        ret[0] = 0; break;

      case 2:
        ret[0] = fac; break;

      case 3:
      case 4:
      case 5:
      case 6:
        ret[0] = -1 * fac; break;

      default:
        DUNE_THROW( RangeError, "Unknown boundary id." );
      }
    }

  //! determine neumann value in a boundary point used in a quadrature
    template< class IntersectionType, class QuadratureType >
    inline void neumannValues( const IntersectionType &intersection,
                               const QuadratureType& quad, int p,
                               RangeType& ret) const
    {
//        assert( false );
      const int boundaryId = intersection.boundaryId();
      double fac = 100.0;
      double f = ret ;
      ret = 1;
      switch( boundaryId )
      {
      case 1:
        ret[0] = 0; break;

      case 2:
        ret[0] = fac; break;

      case 3:
      case 4:
      case 5:
      case 6:
        ret[0] = -1 * fac; break;

      default:
        DUNE_THROW( RangeError, "Unknown boundary id." );
      }
    }

  //! determine robin value in a boundary point used in a quadrature
    template< class IntersectionType, class QuadratureType >
    inline void robinValues( const IntersectionType &intersection,
                             const QuadratureType& quad, int p,
                             RangeType& ret) const
    {
        assert( false );
    }

    template< class EntityType, class PointType >
    inline void mass ( const EntityType &entity,
                       const PointType &x,
                       RangeType &ret ) const
    {
      const DomainType &global = entity.geometry().global( coordinate( x ) );
      ret = global[ 0 ] * global[ 1 ];
      assert( false );
    }

    template< class EntityType, class PointType >
    inline void source ( const EntityType &entity,
                         const PointType &x,
                         RangeType &ret ) const
    {
      const DomainType &global = entity.geometry().global( coordinate( x ) );
      ret = 2 * global[2] + 3* global[1] + 3 * global[0] +
           SQR(global[1])*global[2] + 2* global[0] * global[1]* global[2] +
                global[0] * SQR(global[1]) + SQR(global[0]*global[1]) * global[2] +
                SQR(global[0]) * global[1];
        assert( false );
    }

    template< class EntityType, class PointType >
    inline void diffusiveFlux ( const EntityType &entity,
                                const PointType &x,
                                const JacobianRangeType &gradient,
                                JacobianRangeType &flux ) const
    {
      const DomainType &grad = gradient[ 0 ];
      flux[ 0 ][ 0 ] = 3 * grad[ 0 ] - grad[ 1 ]- grad[ 2 ];
      flux[ 0 ][ 1 ] = -grad[ 0 ] + 3 * grad[ 1 ] - grad[ 2 ];
      flux[ 0 ] [2 ] = -grad[ 0 ] - grad[ 1 ] + 3 * grad[ 2 ];
    }

    template< class EntityType, class PointType >
    inline void convectiveFlux( const EntityType &entity,
                                const PointType &x,
                                const RangeType &phi,
                                JacobianRangeType &ret ) const
    {
      const DomainType global = entity.geometry().global( coordinate( x ) );
      ret[ 0 ][ 0 ] = -global[ 1 ] * phi[ 0 ];
      ret[ 0 ][ 1 ] = -global[ 1 ] * phi[ 0 ];
      ret[ 0 ][ 2 ] = -global[ 1 ] * phi[ 0 ];
    }

    //! the coefficient for robin boundary condition
    template< class IntersectionType, class QuadratureType >
    inline RangeFieldType robinAlpha ( const IntersectionType &intersection,
                                       const QuadratureType &quadrature,
                                       int pt ) const
    {
        assert( false );
    }
  };  // end of AortaModel class

} // end namespace Dune

#endif
