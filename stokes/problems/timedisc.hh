#ifndef STOKES_PORBLEMS_TIMEDISC_HH
#define STOKES_PORBLEMS_TIMEDISC_HH
#include <dune/fem/function/common/function.hh>
#include <dune/stuff/misc.hh>

static const std::string identifier = "TimeDisc";
static const bool hasExactSolution	= true;
static const double disc_time = 1.0;

template < class FunctionSpaceImp >
class Force : public Dune::Function < FunctionSpaceImp , Force < FunctionSpaceImp > >
{
	public:
		typedef Force< FunctionSpaceImp >
			ThisType;
		typedef Dune::Function< FunctionSpaceImp, ThisType >
			BaseType;
		typedef typename BaseType::DomainType
			DomainType;
		typedef typename BaseType::RangeType
			RangeType;

		Force( const double viscosity, const FunctionSpaceImp& space, const double alpha = 0.0, const double scaling_factor = 1.0 )
			: BaseType ( space ),
			  viscosity_( viscosity ),
			  alpha_( alpha ),
			  scaling_factor_( scaling_factor )
		{}

		~Force()
		{}

		inline void evaluate( const DomainType& arg, RangeType& ret ) const
		{
			dune_static_assert( dim_ == 2  , "__CLASS__ Wrong world dim");
			const double x			= arg[0];
			const double y			= arg[1];
			const double v			= viscosity_;
			//laplce
			ret[0] = -2*std::pow(disc_time,3.0)*v;
			ret[1] = 0;
			//grad p
			ret[0] += disc_time;
			ret[1] += 1;
			//conv
//					  ret[0] += std::pow(disc_time,5.0)*2*x*y;
//					  ret[1] += std::pow(disc_time,5.0)*y*y;
			//dt u
//					  ret[0] += std::pow(disc_time,2.0)*3*y*y;
//					  ret[1] += 2*disc_time*x;

//					  ret *=0;
		}

	private:
		const double viscosity_;
		const double alpha_;
		const double scaling_factor_;
		static const int dim_ = FunctionSpaceImp::dimDomain;
};

template < class FunctionSpaceImp >
class DirichletData : public Dune::Function < FunctionSpaceImp, DirichletData < FunctionSpaceImp > >
{
	public:
		typedef DirichletData< FunctionSpaceImp >
			ThisType;
		typedef Dune::Function< FunctionSpaceImp, ThisType >
			BaseType;
		typedef typename BaseType::DomainType
			DomainType;
		typedef typename BaseType::RangeType
			RangeType;

		DirichletData( const FunctionSpaceImp& space )
			: BaseType( space )
		{}

		 ~DirichletData()
		 {}

		template < class IntersectionType >
		void evaluate( const DomainType& arg, RangeType& ret, const IntersectionType& /*intersection*/ ) const
		{
			ret[0] = std::pow(disc_time,3.0)* arg[1] * arg[1];
			ret[1] = std::pow(disc_time,2.0)* arg[0];
		}

		inline void evaluate( const DomainType& arg, RangeType& ret ) const { assert( false ); }

	private:
		static const int dim_ = FunctionSpaceImp::dimDomain;
};

template < class FunctionSpaceImp  >
class Velocity : public Dune::Function < FunctionSpaceImp , Velocity < FunctionSpaceImp > >
{
	public:
		typedef Velocity< FunctionSpaceImp >
			ThisType;
		typedef Dune::Function< FunctionSpaceImp, ThisType >
			BaseType;
		typedef typename BaseType::DomainType
			DomainType;
		typedef typename BaseType::RangeType
			RangeType;

		Velocity( const FunctionSpaceImp& f_space )
			: BaseType( f_space )
		{}

		~Velocity()
		{}

		inline void evaluate( const DomainType& arg, RangeType& ret ) const
		{
			dune_static_assert( dim_ == 2  , "Wrong world dim");
			ret[0] = std::pow(disc_time,3.0)* arg[1] * arg[1];
			ret[1] = std::pow(disc_time,2.0)* arg[0];
		}

		RangeType operator () ( const DomainType& arg)
		{
			RangeType ret;
			evaluate( arg, ret );
			return ret;
		}

	private:
		static const int dim_ = FunctionSpaceImp::dimDomain;
};

template < class FunctionSpaceImp  >
class Pressure : public Dune::Function < FunctionSpaceImp , Pressure < FunctionSpaceImp > >
{
	public:
		typedef Pressure< FunctionSpaceImp >
			ThisType;
		typedef Dune::Function< FunctionSpaceImp, ThisType >
			BaseType;
		typedef typename BaseType::DomainType
			DomainType;
		typedef typename BaseType::RangeType
			RangeType;

		/**
		 *  \brief constructor
		 *
		 *  doing nothing besides Base init
		 **/
		Pressure( const FunctionSpaceImp& f_space )
			: BaseType( f_space )
		{}

		/**
		 *  \brief  destructor
		 *
		 *  doing nothing
		 **/
		~Pressure()
		{}

		inline void evaluate( const DomainType& arg, RangeType& ret ) const
		{
			ret = disc_time * arg[0] + arg[1] - ( ( disc_time + 1) / 2.0 );
		}

		RangeType operator () ( const DomainType& arg)
		{
			RangeType ret;
			evaluate( arg, ret );
			return ret;
		}

	private:
		static const int dim_ = FunctionSpaceImp::dimDomain;
};

#endif // STOKES_PORBLEMS_TIMEDISC_HH