namespace Dune {
template <int dim,int dimworld> 
inline YaspGrid<dim,dimworld>* 
 MacroGrid :: 
 Impl<YaspGrid<dim,dimworld> > ::
 generate(MacroGrid& mg,const char* filename, MPICommunicatorType MPICOMM) 
 {
  mg.element=Cube; 
  std::ifstream gridin(filename);
  IntervalBlock interval(gridin);
  if(!interval.isactive()) {
    DUNE_THROW(DGFException,
               "Macrofile " << filename << " must have Intervall-Block "
               << "to be used to initialize YaspGrid!\n" 
               << "No alternative File-Format defined");
  }
  mg.dimw = interval.dimw();
  if (mg.dimw != dimworld) {
    DUNE_THROW(DGFException,
               "Macrofile " << filename << " is for dimension " << mg.dimw 
               << " and connot be used to initialize an YaspGrid of dimension "
               << dimworld);
  }
  // get grid parameters 
  GridParameterBlock grdParam(gridin, true);
  
  FieldVector<double,dimworld> lang;
  FieldVector<int,dimworld>    anz;
  FieldVector<bool,dimworld>   per(false);

  for (int i=0;i<dimworld;i++) 
  {
    // check that start point is > 0.0
    if( interval.start(i) < 0.0 ) 
    {
      DUNE_THROW(DGFException,"YaspGrid cannot handle grids with left lower corner below zero!");
    }

    // set parameter for yaspgrid 
    lang[i] = interval.length(i);
    anz[i]  = interval.segments(i);
    per[i]  = grdParam.isPeriodic(i);
  }

  #if HAVE_MPI
    return new YaspGrid<dim,dimworld>(MPICOMM,lang, anz, per , grdParam.overlap() );
  #else
    return new YaspGrid<dim,dimworld>(lang, anz, per , grdParam.overlap() );
  #endif
}
}
