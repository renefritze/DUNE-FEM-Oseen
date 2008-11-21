namespace Dune {
template <int dim,int dimworld> 
inline SGrid<dim,dimworld>* 
 MacroGrid :: 
 Impl< SGrid<dim,dimworld> > ::
 generate(MacroGrid& mg,const char* filename, MPICommunicatorType ) {
  mg.element=Cube; 
  std::ifstream gridin(filename);
  IntervalBlock interval(gridin);
  if(!interval.isactive()) {
    DUNE_THROW(DGFException,
               "Macrofile " << filename << " must have Intervall-Block "
               << "to be used to initialize SGrid!\n" 
               << "No alternative File-Format defined");
  }
  mg.dimw = interval.dimw();
  if (mg.dimw != dimworld) {
    DUNE_THROW(DGFException,
               "Macrofile " << filename << " is for dimension " << mg.dimw 
               << " and connot be used to initialize an SGrid of dimension "
               << dimworld);
  }
  
  FieldVector<double,dimworld> start;
  FieldVector<double,dimworld> upper;
  FieldVector<int,dimworld>    anz;
  
  for (int i=0; i<dimworld; ++i) 
  {
    start[i] = interval.start(i);
    upper[i]  = start[i] + interval.length(i);
    anz[i]  = interval.segments(i);
  }
  // SGrid gets number of cells in each dircetion
  // position of origin of the cube
  // position of the upper right corner of the cube 
  return new SGrid<dim,dimworld>(anz,start,upper);
}
}
