#include"config.h"
#include<dune/istl/matrixutils.hh>
#include<dune/istl/bcrsmatrix.hh>
#include<dune/common/fmatrix.hh>
#include<dune/common/stdstreams.hh>
#include"laplacian.hh"

int main(int argc, char** argv)
{
  Dune::FieldMatrix<double,4,7> fmatrix;
  
  int ret=0;
  
  if(4*7!=countNonZeros(fmatrix)){
    Dune::derr<<"Counting nonzeros of fieldMatrix failed!"<<std::endl;
    ret++;
  }

  const int N=4;
  
  typedef Dune::BCRSMatrix<Dune::FieldMatrix<double,1,1> > BMatrix;
  
  BMatrix laplace;
  setupLaplacian(laplace, N);

  if(N*N*5-4*2-(N-2)*4!=countNonZeros(laplace)){
    ++ret;
    Dune::derr<<"Counting nonzeros of BCRSMatrix failed!"<<std::endl;
  }
  
  Dune::BCRSMatrix<Dune::FieldMatrix<double,4,7> > blaplace;
  setupLaplacian(blaplace,N);

  if((N*N*5-4*2-(N-2)*4)*4*7!=countNonZeros(blaplace)){
    ++ret;
    Dune::derr<<"Counting nonzeros of block BCRSMatrix failed!"<<std::endl;
  }

  Dune::BCRSMatrix<Dune::FieldMatrix<double,4,7> > bblaplace;
  bblaplace.setSize(N*N,N*N, N*N*5);
  bblaplace.setBuildMode(Dune::BCRSMatrix<Dune::FieldMatrix<double,4,7> >::row_wise);
  setupLaplacian(bblaplace,N);

  if((N*N*5-4*2-(N-2)*4)*4*7!=countNonZeros(bblaplace)){
    ++ret;
    Dune::derr<<"Counting nonzeros of block BCRSMatrix failed!"<<std::endl;
  }
  
}
