#ifndef DUNE_DGFPARSERALU_HH
#define DUNE_DGFPARSERALU_HH

// only include if ALUGrid is used
#if defined ENABLE_ALUGRID
#include <dune/grid/alugrid.hh>
#include "dgfparser.hh"
namespace Dune {
//*********************************
template <int dim,int dimworld>
class MacroGrid::Impl<ALUCubeGrid<dim,dimworld> > {
  typedef MPIHelper::MPICommunicator MPICommunicatorType;
public:
  static ALUCubeGrid<dim,dimworld>* 
  generate(MacroGrid& mg,
	   const char* filename, 
     MPICommunicatorType MPICOMM = MPIHelper::getCommunicator() );
 private:
  inline void
  generateAlu3d(MacroGrid& mg,
		const char* filename, std::string& str, MPICommunicatorType MPICOMM );
};
template <int dim,int dimworld>
class MacroGrid::Impl<ALUSimplexGrid<dim,dimworld> > {
  typedef MPIHelper::MPICommunicator MPICommunicatorType;
  friend class MacroGrid::Impl<ALUConformGrid<dim,dimworld> >;
public:
  static ALUSimplexGrid<dim,dimworld>* 
  generate(MacroGrid& mg,
	   const char* filename,
     MPICommunicatorType MPICOMM = MPIHelper::getCommunicator() );
 protected:
  inline void
  generateAlu3d(MacroGrid& mg,
		const char* filename, std::string& str, MPICommunicatorType MPICOMM );
  // friend MacroGrid::Impl<ALUConformGrid<dim,dimworld> >;
}; 
/* needs new version of alulib */
template <int dim,int dimworld>
class MacroGrid::Impl<ALUConformGrid<dim,dimworld> > {
  typedef MPIHelper::MPICommunicator MPICommunicatorType;
public:
  static ALUConformGrid<dim,dimworld>* 
  generate(MacroGrid& mg,
	   const char* filename,
     MPICommunicatorType MPICOMM = MPIHelper::getCommunicator() );
 private:
  inline void
  generateAlu3d(MacroGrid& mg,
		const char* filename, std::string& str, MPICommunicatorType MPICOMM );
};
template <>
struct DGFGridInfo<ALUCubeGrid<3,3> > {
    static int refineStepsForHalf() {return 1;}
    static double refineWeight() {return pow(0.5,3);}
};
template <int dimworld>
struct DGFGridInfo< ALUSimplexGrid<dimworld,dimworld> > {
    static int refineStepsForHalf() {return 1;}
    static double refineWeight() {return pow(0.5,dimworld);}
};
template <>
struct DGFGridInfo< Dune::ALUConformGrid<2,2> > {
    static int refineStepsForHalf() { return 2; }
    static double refineWeight() { return 0.5; }
};
}
#include "dgfalu.cc"
#endif

#endif
