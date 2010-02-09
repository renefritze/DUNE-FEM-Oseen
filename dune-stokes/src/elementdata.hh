#include<dune/grid/common/referenceelements.hh>
#include<dune/grid/common/mcmgmapper.hh>
#include<dune/grid/io/file/vtk/vtkwriter.hh>
#if HAVE_GRAPE
#include<dune/grid/io/visual/grapedatadisplay.hh>
#endif

//! Parameter for mapper class
template<int dim>
struct P0Layout
{
  bool contains (Dune::GeometryType gt)
  {
	if (gt.dim()==dim) return true;
	return false;
  }
};

// demonstrate attaching data to elements
template<class G, class F>
void elementdata (const G& grid, const F& f)
{
  // the usual stuff
  const int dim = G::dimension;
  const int dimworld = G::dimensionworld;
  typedef typename G::ctype ct;
  typedef typename G::template Codim<0>::LeafIterator ElementLeafIterator;

  // make a mapper for codim 0 entities in the leaf grid
  Dune::LeafMultipleCodimMultipleGeomTypeMapper<G,P0Layout>
      mapper(grid);                                    /*@\label{edh:mapper}@*/

  // allocate a vector for the data
  std::vector<double> c(mapper.size());                /*@\label{edh:c}@*/

  // iterate through all entities of codim 0 at the leafs
  for (ElementLeafIterator it = grid.template leafbegin<0>(); /*@\label{edh:loop0}@*/
	   it!=grid.template leafend<0>(); ++it)
	{
	  // cell geometry type
	  Dune::GeometryType gt = it->type();

	  // cell center in reference element
	  const Dune::FieldVector<ct,dim>&
		local = Dune::ReferenceElements<ct,dim>::general(gt).position(0,0);

	  // get global coordinate of cell center
	  Dune::FieldVector<ct,dimworld> global = it->geometry().global(local);

	  // evaluate functor and store value
	  c[mapper.map(*it)] = f(*it);	       /*@\label{edh:feval}@*/
	}                                              /*@\label{edh:loop1}@*/

  // generate a VTK file
  // Dune::LeafP0Function<G,double> cc(grid,c);
  Dune::VTKWriter<G> vtkwriter(grid);                  /*@\label{edh:vtk0}@*/
  vtkwriter.addCellData(c,"data");
  vtkwriter.write(f.filename().c_str(),Dune::VTKOptions::binaryappended); /*@\label{edh:vtk1}@*/

  // online visualization with Grape
#if HAVE_GRAPE                                         /*@\label{edh:grape0}@*/
  if ( Parameters().getParam("use_grape",false) )
  {
    const int polynomialOrder = 0; // we piecewise constant data
    const int dimRange = 1; // we have scalar data here
    // create instance of data display
    Dune::GrapeDataDisplay<G> grape(grid);
    // display data
    grape.displayVector("concentration", // name of data that appears in grape
                        c,  // data vector
                        grid.leafIndexSet(), // used index set
                        polynomialOrder, // polynomial order of data
                        dimRange); // dimRange of data
  }
#endif                                                 /*@\label{edh:grape1}@*/
}
