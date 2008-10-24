struct SStruct {
  SStruct(int n1, int n2, double lx, double ly, double hx, double hy)
  {
    n_[0] = n1;
    n_[1] = n2;
    n_[2] = n2;
    l_[0] = lx;
    l_[1] = ly;
    l_[2] = ly;
    h_[0] = hx;
    h_[1] = hy;
    h_[2] = hy;
  }

  SStruct(int n, double h) {
    n_[0] = n;
    n_[1] = n;
    n_[2] = n;
    l_[0] = .0;
    l_[1] = .0; // h/2.0;
    l_[2] = .0; // h/2.0;
    h_[0] = 2.0;
    h_[1] = 2.0; // h/2.0;
    h_[2] = 2.0; // h/2.0;
  }
  SStruct(int n) {
    n_[0] = n;
    n_[1] = n;
    n_[2] = n;
    l_[0] = .0;
    l_[1] = .0; // 0.5/double(n);
    l_[2] = .0; // 0.5/double(n);
    h_[0] = 2.0;
    h_[1] = 2.0; // 0.5/double(n);
    h_[2] = 2.0; // 0.5/double(n);
  }

  int n_[3];
  double l_[3];
  double h_[3];
};
template <class Geometry,int n>
void midPoint(const Geometry& geo, FieldVector<double, n>& result)
{
  result *= 0.0;
  for (int i = 0; i < geo.corners(); ++i) {
    result += geo[i];
  }

  result /= static_cast<double>(geo.corners());
}
template <class Sol, class SpaceType>
void printSGrid(double time, int timestep, const SpaceType& space, Sol& sol)
{
  /*
  typedef typename SpaceType::IteratorType Iterator;
  typedef typename Sol::DiscreteFunctionType DiscreteFunctionType;
  typedef typename DiscreteFunctionType::LocalFunctionType LocalFunctionType;

  std::ostringstream filestream;
  filestream << "sgrid" << timestep;

  std::ofstream ofs(filestream.str().c_str(), std::ios::out);

  ofs << "# " << time << std::endl;

  typename SpaceType::DomainType mid (0.0);
  typename SpaceType::DomainType localMid (0.5);
  typename SpaceType::RangeType result (0.0);

  Iterator endit = space.end();
  for (Iterator it = space.begin(); it != endit; ++it) {
    midPoint(it->geometry(), mid);
    {
      const LocalFunctionType lf = sol.localFunction(*it);
      lf.evaluate(localMid, result);

      ofs << mid << " " << result << "\n";
    }
  }
  ofs << std::endl;
  */
}


template <class StupidFunction,class DFType>
void initialize(const StupidFunction& f,DFType& df)
{
  //- Actual method to project 
  L2ProjectionImpl::project(f, df);
}

template <class DFType>
void printIt(DFType& df)
{
  typedef typename DFType::DofIteratorType DofIt;

  std::cout << "print it\n";
  for (DofIt it = df.dbegin(); it != df.dend(); ++it) {
    std::cout << *it << std::endl;
  }

}


class EocOutput {

  string outputFile;
	
 public:
		
  EocOutput(string name)
  {
    outputFile = name;
    std::ostringstream filestream;
    filestream << outputFile;

    std::ofstream ofs(filestream.str().c_str(), std::ios::out);

    ofs << "\\documentclass[12pt,english]{article}\n"
	<< "\\usepackage[T1]{fontenc}\n"
	<< "\\usepackage[latin1]{inputenc}\n"
	<< "\\usepackage{setspace}\n"
	<< "\\onehalfspacing\n"
	<< "\\makeatletter\n"
	<< "\\providecommand{\\boldsymbol}[1]{\\mbox{\\boldmath $#1$}}\n"
	<< "\\providecommand{\\tabularnewline}{\\\\}\n"
	<< "\\usepackage{babel}\n"
	<< "\\makeatother\n"
	<< "\\begin{document}\n";
				 
    ofs.close();	
  }
	
  void printTexEnd(double totaltime)
  {
    std::ostringstream filestream;
    filestream << outputFile;

    std::ofstream ofs(filestream.str().c_str(), std::ios::app);
		
    ofs  << "\\end{tabular}\\\\\n\n"
	 << "Total time: " << totaltime << "\n"
	 << "\\end{document}\n" << std::endl;
		
    ofs.close();
  }
	
  void printTexAddError(double error, double prevError, double time, int level, int counter,double averagedt)
  {
    std::ostringstream filestream;
    filestream << outputFile;

    std::ofstream ofs(filestream.str().c_str(), std::ios::app);
		
    if(prevError > 0.0)
      {	       
	ofs <<  "\\hline \n"
	    << level << " & " << error << " & " << log(prevError/error)/M_LN2 << " & " << time << " & " << counter << " & " << averagedt << "\n"
	    << "\\tabularnewline\n"
	    << "\\hline \n";
      }
    else
      {	       
	ofs << "\\begin{tabular}{|c|c|c|c|c|c|}\n"
	    << "\\hline \n"
	    << "Size & $\\left\\Vert u-u_{h}\\right\\Vert _{L_{2}}$ & EOC & CPU & \\#Iterations & a-dt\n"
	    << "\\tabularnewline\n"
	    << "\\hline\n"
	    << "\\hline\n"
	    << level << " & " << error << " & " << "---" << " & " << time << " & " << counter << " & " << averagedt << "\n"
	    << "\\tabularnewline\n"
	    << "\\hline \n";
      }
		
    ofs.close();
  }

		
  void printInput(InitialDataType& u0, GridType& grid,ODEType& ode, 
		  char *arg)
  {
    std::ostringstream filestream;
    filestream << outputFile;

    std::ofstream ofs(filestream.str().c_str(), std::ios::app);

    ofs  << "Grid: " << grid.name() << "\n\n"
	 << "Macrogrid: " << arg << "\\\\\n\n";
		
    ofs.close();
    ode.printmyInfo(outputFile);
    u0.printmyInfo(outputFile);		
  }
};


//Old version of latex output
/*
  void printError(double totaltime, double time[10], double error[10], \
  int maxit, InitialDataType u0, GridType *grid)
  {
  std::ostringstream filestream;
  filestream << "eoc.tex";

  std::ofstream ofs(filestream.str().c_str(), std::ios::out);
	
  ofs << "\\documentclass[12pt,english]{article}\n"
  <<	"\\usepackage[T1]{fontenc}\n"
  <<	"\\usepackage[latin1]{inputenc}\n"
  <<	"\\usepackage{setspace}\n"
  <<	"\\onehalfspacing\n"
  <<	"\\makeatletter\n"
  <<	"\\providecommand{\\boldsymbol}[1]{\\mbox{\\boldmath $#1$}}\n"
  <<	"\\providecommand{\\tabularnewline}{\\\\}\n"
  <<	"\\usepackage{babel}\n"
  <<	"\\makeatother\n"
  <<	"\\begin{document}\n"
  <<	"\\input{eoc" << u0.myName << ".tex}\n\n"
  <<	"Grid: " << transformToGridName(grid->type()) << "\n\n"
  << "Total Time: " << totaltime << "\n\n"
  << "Runge-Kutta Steps: " << rksteps << "\n\n"
  <<	"Polynomial Order: " << order << "\\\\\n\n"
  <<	"\\begin{tabular}{|c|c|c|c|}\n"
  <<	"\\hline \n"
  <<	"h & $\\left\\Vert u-u_{h}\\right\\Vert _{L_{2}}$ & EOC & CPU\n"
  <<	"\\tabularnewline\n"
  <<	"\\hline\n"
  <<	"\\hline \n"
  <<	" & " << error[0] << " & --- & " << time[0] << "\n"
  <<	"\\tabularnewline\n"
  <<	"\\hline \n";
			 
  for(int i=1;i<maxit;++i)
  { 
  ofs <<	"\\hline \n"
  <<	" & " << error[i] << " & " << log( error[i-1]/error[i])/M_LN2 << " & " << time[i] <<"\n"
  <<	"\\tabularnewline\n"
  <<	"\\hline \n";	
  }
				 
  ofs	 <<	"\\end{tabular}\n"
  <<	"\\end{document}\n" << std::endl;
  }
*/
