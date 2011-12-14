#ifndef MOD_ISTLMATRIX_HH
#define MOD_ISTLMATRIX_HH

#include <dune/istl/bcrsmatrix.hh>

namespace Dune {
namespace Stokes {
namespace Integrators {

template <class LittleBlockType,
          class RowDiscreteFunctionImp,
          class ColDiscreteFunctionImp >
class ModifiedImprovedBCRSMatrix : public Dune::BCRSMatrix<LittleBlockType>
{
  public:
    typedef LittleBlockType KK;
    typedef RowDiscreteFunctionImp RowDiscreteFunctionType;
    typedef ColDiscreteFunctionImp ColDiscreteFunctionType;

    typedef Dune::BCRSMatrix<LittleBlockType> BaseType;
    typedef typename BaseType :: RowIterator RowIteratorType ;
    typedef typename BaseType :: ColIterator ColIteratorType ;

    typedef ModifiedImprovedBCRSMatrix< LittleBlockType,
            RowDiscreteFunctionImp, ColDiscreteFunctionImp > ThisType;

    typedef typename BaseType :: size_type size_type;

    //===== type definitions and constants

    //! export the type representing the field
    typedef typename BaseType::field_type field_type;
    typedef field_type Ttype;

    //! export the type representing the components
    typedef typename BaseType::block_type block_type;

    //! export the allocator type
    typedef typename BaseType:: allocator_type allocator_type;

    //! implement row_type with compressed vector
    typedef typename BaseType :: row_type row_type;

    //! increment block level counter
    enum {
      //! The number of blocklevels the matrix contains.
      blocklevel = BaseType :: blocklevel
    };

    /** \brief Iterator for the entries of each row */
    typedef typename BaseType :: ColIterator ColIterator;

    /** \brief Iterator for the entries of each row */
    typedef typename BaseType :: ConstColIterator ConstColIterator;

    /** \brief Const iterator over the matrix rows */
    typedef typename BaseType :: RowIterator RowIterator;

    /** \brief Const iterator over the matrix rows */
    typedef typename BaseType :: ConstRowIterator ConstRowIterator;

    //! type of discrete function space
    typedef typename ColDiscreteFunctionType :: DiscreteFunctionSpaceType ColSpaceType;
    typedef typename RowDiscreteFunctionType :: DiscreteFunctionSpaceType RowSpaceType;

    //! type of row block vector
    typedef typename RowDiscreteFunctionType :: DofStorageType  RowBlockVectorType;

    //! type of column block vector
    typedef typename ColDiscreteFunctionType :: DofStorageType  ColBlockVectorType;

    //! type of communication object
    typedef typename ColSpaceType :: GridType :: Traits :: CollectiveCommunication   CollectiveCommunictionType ;

  protected:
    size_type nz_;

    int localRows_;
    int localCols_;

    typedef typename BaseType :: BuildMode BuildMode ;

  public:
    //! constructor used by ISTLMatrixObject
    ModifiedImprovedBCRSMatrix(size_type rows, size_type cols)
      : BaseType (rows,cols, BaseType :: random)
      , nz_(0)
    {
        std::cerr << boost::format( "mistl Matrix %d - %d --> %d x %d blocks, nz %d\n")
                        % rows % cols % LittleBlockType::rows %  LittleBlockType::cols  % nz_;
    }

    //! constuctor used by ILU preconditioner
    ModifiedImprovedBCRSMatrix(size_type rows, size_type cols, size_type nz)
      : BaseType (rows,cols, BaseType :: random)
      , nz_(nz)
    {
        std::cerr << boost::format( "mistl Matrix %d - %d --> %d x %d blocks, nz %d\n")
                        % rows % cols % LittleBlockType::rows %  LittleBlockType::cols  % nz_;
    }

    //! copy constructor, needed by ISTL preconditioners
    ModifiedImprovedBCRSMatrix( )
      : BaseType ()
      , nz_(0)
    {}

    //! copy constructor, needed by ISTL preconditioners
    ModifiedImprovedBCRSMatrix(const ModifiedImprovedBCRSMatrix& org)
      : BaseType(org)
      , nz_(org.nz_)
      , localRows_(org.localRows_)
      , localCols_(org.localCols_)
    {}

    //! setup matrix entires
    template < class StencilCreatorImp >
    void setup(const RowSpaceType & rowSpace,
               const ColSpaceType& colSpace,
               const StencilCreatorImp& stencil,
               bool verbose = false)
    {

        stencil.create( rowSpace, colSpace, this );

      // in verbose mode some output
      if(verbose)
      {
        std::cout << "ISTLMatrix::setup: finished assembly of matrix structure! \n";
      }
    }

    //! clear Matrix, i.e. set all entires to 0
    void clear()
    {
      RowIteratorType endi=this->end();
      for (RowIteratorType i=this->begin(); i!=endi; ++i)
      {
        ColIteratorType endj = (*i).end();
        for (ColIteratorType j=(*i).begin(); j!=endj; ++j)
        {
          (*j) = 0;
        }
      }
    }

    //! print matrix
    void print(std::ostream & s) const
    {
      std::cout << "Print ISTLMatrix \n";
      ConstRowIterator endi=this->end();
      for (ConstRowIterator i=this->begin(); i!=endi; ++i)
      {
        ConstColIterator endj = (*i).end();
        for (ConstColIterator j=(*i).begin(); j!=endj; ++j)
        {
          s << (*j) << std::endl;
        }
      }
    }

    size_type numNonZeros( size_t row ) const
    {
//           const row_type& row_  = BaseType::row[row];
         return BaseType::nonzeroes();
    }

    size_type rows() const { return BaseType::N()*localRows_ ; }
    size_type cols() const { return BaseType::M()*localCols_ ; }
    field_type operator() ( const unsigned int row, const unsigned int col ) const
    {
    assert( false );
//          if ( BaseType::exists( row, col ) )
//              return *(BaseType::r[row].find(col));
      return 0.0;
    }
    void set( const unsigned int row, const unsigned int col, field_type value )
    {
    assert( false );
    }
    bool find (size_type row, size_type col) const { return BaseType::exists(row, col); }
    void scale( const field_type scalar ) { *this *= scalar; }
};


template <class RowFunctionImp, class ColFunctionImp, class TraitsImp>
class ModifiedISTLMatrixObject;

template <class RowFunctionImp, class ColFunctionImp >
struct ModifiedISTLMatrixTraits
{
    typedef typename ColFunctionImp::DiscreteFunctionSpaceType
        ColSpaceImp;
    typedef typename RowFunctionImp::DiscreteFunctionSpaceType
        RowSpaceImp;
  typedef RowSpaceImp RowSpaceType;
  typedef ColSpaceImp ColumnSpaceType;
  typedef ModifiedISTLMatrixTraits<RowSpaceType,ColumnSpaceType> ThisType;

  template <class OperatorTraits>
  struct MatrixObject
  {
    typedef ModifiedISTLMatrixObject<RowSpaceType,ColumnSpaceType,OperatorTraits> MatrixObjectType;
  };
};

//! MatrixObject handling an istl matrix
template <class RowFunctionImp, class ColFunctionImp, class TraitsImp>
class ModifiedISTLMatrixObject
{
    typedef typename ColFunctionImp::DiscreteFunctionSpaceType
        ColSpaceImp;
    typedef typename RowFunctionImp::DiscreteFunctionSpaceType
        RowSpaceImp;
public:
  //! type of traits
  typedef TraitsImp Traits;

  //! type of stencil defined by operator
  typedef typename Traits :: StencilType StencilType;

  //! type of space defining row structure
  typedef RowSpaceImp RowSpaceType;
  //typedef typename Traits :: RowSpaceType RowSpaceType;
  //! type of space defining column structure
  typedef ColSpaceImp ColumnSpaceType;
  //typedef typename Traits :: ColumnSpaceType ColumnSpaceType;
    typedef RowSpaceImp DomainSpaceType;
    typedef ColSpaceImp RangeSpaceType;
    typedef typename RowSpaceType :: RangeFieldType Ttype;

  //! type of this pointer
  typedef ModifiedISTLMatrixObject<RowFunctionImp,ColFunctionImp,Traits> ThisType;


protected:
  typedef typename RowSpaceType::GridType GridType;

  typedef typename ColumnSpaceType :: EntityType ColumnEntityType ;
  typedef typename RowSpaceType :: EntityType RowEntityType ;

  enum { littleRows = RowSpaceType :: localBlockSize };
  enum { littleCols = ColumnSpaceType :: localBlockSize };

  typedef typename RowSpaceType :: RangeFieldType RangeFieldType;

  typedef FieldMatrix<RangeFieldType, littleRows, littleCols> LittleBlockType;

  typedef RowFunctionImp RowDiscreteFunctionType;
  typedef typename RowDiscreteFunctionType :: LeakPointerType  RowLeakPointerType;
  typedef ColFunctionImp ColumnDiscreteFunctionType;
  typedef typename ColumnDiscreteFunctionType :: LeakPointerType  ColumnLeakPointerType;

  typedef typename RowDiscreteFunctionType :: DofStorageType    RowBlockVectorType;
  typedef typename ColumnDiscreteFunctionType :: DofStorageType ColumnBlockVectorType;

  typedef typename RowSpaceType :: BlockMapperType RowMapperType;
  typedef typename ColumnSpaceType :: BlockMapperType ColMapperType;

public:
  //! type of used matrix
  typedef ModifiedImprovedBCRSMatrix< LittleBlockType ,
                              RowDiscreteFunctionType ,
                              ColumnDiscreteFunctionType > MatrixType;
  typedef typename Traits :: template Adapter < MatrixType > ::  MatrixAdapterType MatrixAdapterType;
  // get preconditioner type from MatrixAdapterType
  typedef ThisType PreconditionMatrixType;
  typedef typename MatrixAdapterType :: ParallelScalarProductType ParallelScalarProductType;

  template <class MatrixObjectImp>
  class LocalMatrix;

  struct LocalMatrixTraits
  {
    typedef RowSpaceType DomainSpaceType ;
    typedef ColumnSpaceType RangeSpaceType;
    typedef typename RowSpaceType :: RangeFieldType RangeFieldType;
    typedef LocalMatrix<ThisType> LocalMatrixType;
    typedef typename MatrixType:: block_type LittleBlockType;
  };

  //! LocalMatrix
  template <class MatrixObjectImp>
  class LocalMatrix : public LocalMatrixDefault<LocalMatrixTraits>
  {
  public:
    //! type of base class
    typedef LocalMatrixDefault<LocalMatrixTraits> BaseType;

    //! type of matrix object
    typedef MatrixObjectImp MatrixObjectType;
    //! type of matrix
    typedef typename MatrixObjectImp :: MatrixType MatrixType;
    //! type of little blocks
    typedef typename MatrixType:: block_type LittleBlockType;
    //! type of entries of little blocks
    typedef typename RowSpaceType :: RangeFieldType DofType;

    typedef typename MatrixType::row_type RowType;

    //! type of row mapper
    typedef typename MatrixObjectType :: RowMapperType RowMapperType;
    //! type of col mapper
    typedef typename MatrixObjectType :: ColMapperType ColMapperType;

  private:
    // special mapper omiting block size
    const RowMapperType& rowMapper_;
    const ColMapperType& colMapper_;

    // number of local matrices
    int numRows_;
    int numCols_;

    // vector with pointers to local matrices
    typedef std::vector<LittleBlockType *> LittleMatrixRowStorageType ;
    std::vector< LittleMatrixRowStorageType > matrices_;

    // matrix to build
    const MatrixObjectType& matrixObj_;

    // type of actual geometry
    GeometryType geomType_;

  public:
    LocalMatrix(const MatrixObjectType & mObj,
                const RowSpaceType & rowSpace,
                const ColumnSpaceType & colSpace)
      : BaseType( rowSpace, colSpace )
      , rowMapper_(mObj.rowMapper())
      , colMapper_(mObj.colMapper())
      , numRows_( rowMapper_.maxNumDofs() )
      , numCols_( colMapper_.maxNumDofs() )
      , matrixObj_(mObj)
      , geomType_(GeometryType::simplex,0)
    {
    }

    void init(const RowEntityType & rowEntity,
              const ColumnEntityType & colEntity)
    {
      if( geomType_ != rowEntity.type() )
      {
        // initialize base functions sets
        BaseType :: init ( rowEntity , colEntity );

        geomType_ = rowEntity.type();
        numRows_  = rowMapper_.numDofs(rowEntity);
        numCols_  = colMapper_.numDofs(colEntity);
        matrices_.resize( numRows_ );
      }

      MatrixType& matrix = matrixObj_.matrix();
      typedef typename RowMapperType :: DofMapIteratorType RowMapIteratorType ;
      typedef typename ColMapperType :: DofMapIteratorType ColMapIteratorType ;

      const RowMapIteratorType endrow = rowMapper_.end( rowEntity );
      for( RowMapIteratorType row = rowMapper_.begin( rowEntity );
           row != endrow; ++row )
      {
        LittleMatrixRowStorageType& localMatRow = matrices_[ row.local() ];
        localMatRow.resize( numCols_ );

        // get row
        RowType& matRow = matrix[ row.global() ];

        const ColMapIteratorType endcol = colMapper_.end( colEntity );
        for( ColMapIteratorType col = colMapper_.begin( colEntity );
             col != endcol; ++col )
        {
          assert( matrix.exists( row.global(), col.global() ) );
          localMatRow[ col.local() ] = &matRow[ col.global() ];
        }
      }
    }

    LocalMatrix(const LocalMatrix& org)
      : BaseType( org )
      , rowMapper_(org.rowMapper_)
      , colMapper_(org.colMapper_)
      , numRows_( org.numRows_ )
      , numCols_( org.numCols_ )
      , matrices_(org.matrices_)
      , matrixObj_(org.matrixObj_)
      , geomType_(org.geomType_)
    {
    }

  private:
    // check whether given (row,col) pair is valid
    void check(int localRow, int localCol) const
    {
      const size_t row = (int) localRow / littleRows;
      const size_t col = (int) localCol / littleCols;
      const int lRow = localRow%littleRows;
      const int lCol = localCol%littleCols;
      ASSERT_LT( row , matrices_.size() ) ;
      ASSERT_LT( col , matrices_[row].size() );
      ASSERT_LT( lRow , littleRows );
      ASSERT_LT( lCol , littleCols );
    }

    DofType& getValue(const int localRow, const int localCol)
    {
      const int row = (int) localRow / littleRows;
      const int col = (int) localCol / littleCols;
      const int lRow = localRow%littleRows;
      const int lCol = localCol%littleCols;
      return (*matrices_[row][col])[lRow][lCol];
    }

  public:
    const DofType get(const int localRow, const int localCol) const
    {
      const int row = (int) localRow / littleRows;
      const int col = (int) localCol / littleCols;
      const int lRow = localRow%littleRows;
      const int lCol = localCol%littleCols;
      return (*matrices_[row][col])[lRow][lCol];
    }

    void scale (const DofType& scalar)
    {
      for(size_t i=0; i<matrices_.size(); ++i)
        for(size_t j=0; j<matrices_[i].size(); ++j)
          (*matrices_[i][j]) *= scalar;
    }

    void add(const int localRow, const int localCol , const DofType value)
    {
#ifndef NDEBUG
      check(localRow,localCol);
#endif
      getValue(localRow,localCol) += value;
    }

    void set(const int localRow, const int localCol , const DofType value)
    {
#ifndef NDEBUG
      check(localRow,localCol);
#endif
      getValue(localRow,localCol) = value;
    }

    //! make unit row (all zero, diagonal entry 1.0 )
    void unitRow(const int localRow)
    {
      const int row = (int) localRow / littleRows;
      const int lRow = localRow%littleRows;

      // clear row
      doClearRow( row, lRow );

      // set diagonal entry to 1
      (*matrices_[row][row])[lRow][lRow] = 1;
    }

    //! clear all entries belonging to local matrix
    void clear ()
    {
      for(int i=0; i<matrices_.size(); ++i)
        for(int j=0; j<matrices_[i].size(); ++j)
          (*matrices_[i][j]) = (DofType) 0;
    }

    //! set matrix row to zero
    void clearRow ( const int localRow )
    {
      const int row = (int) localRow / littleRows;
      const int lRow = localRow%littleRows;

      // clear the row
      doClearRow( row, lRow );
    }

    //! empty as the little matrices are already sorted
    void resort ()
    {}

protected:
    //! set matrix row to zero
    void doClearRow ( const int row, const int lRow )
    {
      // get number of columns
      const int col = this->columns();
      for(int localCol=0; localCol<col; ++localCol)
      {
        const int col = (int) localCol / littleCols;
        const int lCol = localCol%littleCols;
        (*matrices_[row][col])[lRow][lCol] = 0;
      }
    }

  }; // end of class LocalMatrix

public:
  //! type of local matrix
  typedef LocalMatrix<ThisType> ObjectType;
  typedef ThisType LocalMatrixFactoryType;
  typedef ObjectStack< LocalMatrixFactoryType > LocalMatrixStackType;
  //! type of local matrix
  typedef LocalMatrixWrapper< LocalMatrixStackType > LocalMatrixType;

protected:
  const RowSpaceType & rowSpace_;
  const ColumnSpaceType & colSpace_;

  // sepcial row mapper
  RowMapperType& rowMapper_;
  // special col mapper
  ColMapperType& colMapper_;

  int size_;

  int sequence_;

  mutable MatrixType* matrix_;

  ParallelScalarProductType scp_;

  int numIterations_;
  double relaxFactor_;

  enum PreConder_Id { none  = 0 , // no preconditioner
                      ssor  = 1 , // SSOR preconditioner
                      sor   = 2 , // SOR preconditioner
                      ilu_0 = 3 , // ILU-0 preconditioner
                      ilu_n = 4 , // ILU-n preconditioner
                      gauss_seidel= 5 , // Gauss-Seidel preconditioner
                      jacobi = 6,  // Jacobi preconditioner
                      amg_ilu_0 = 7,  // AMG with ILU-0 smoother
                      amg_ilu_n = 8,  // AMG with ILU-n smoother
                      amg_jacobi = 9  // AMG with Jacobi smoother
  };

  PreConder_Id preconditioning_;

  mutable LocalMatrixStackType localMatrixStack_;

  mutable MatrixAdapterType* matrixAdap_;
  mutable RowBlockVectorType* Arg_;
  mutable ColumnBlockVectorType* Dest_;

  // prohibit copy constructor
  ModifiedISTLMatrixObject(const ModifiedISTLMatrixObject&);
public:
  //! constructor
  //! \param rowSpace space defining row structure
  //! \param colSpace space defining column structure
  //! \param paramfile parameter file to read variables
  //!         - Preconditioning: {0,1,2,3,4,5,6} put -1 to get info
  //!         - Pre-iteration: number of iteration of preconditioner
  //!         - Pre-relaxation: relaxation factor
  ModifiedISTLMatrixObject ( const RowSpaceType &rowSpace,
                     const ColumnSpaceType &colSpace,
                     const std :: string &paramfile = "" )
    : rowSpace_(rowSpace)
    , colSpace_(colSpace)
    // create scp to have at least one instance
    // otherwise instance will be deleted during setup
    // get new mappers with number of dofs without considerung block size
    , rowMapper_( rowSpace.blockMapper() )
    , colMapper_( colSpace.blockMapper() )
    , size_(-1)
    , sequence_(-1)
    , matrix_(0)
    , scp_(colSpace_)
    , numIterations_(5)
    , relaxFactor_(1.1)
    , preconditioning_(none)
    , localMatrixStack_( *this )
    , matrixAdap_(0)
    , Arg_(0)
    , Dest_(0)
  {
    int preCon = 0;
    if(paramfile != "")
    {
      DUNE_THROW(InvalidStateException,"ModifiedISTLMatrixObject: old parameter method disabled");
    }
    else
    {
      static const std::string preConTable[]
        = { "none", "ssor", "sor", "ilu-0", "ilu-n", "gauss-seidel", "jacobi",
          "amg-ilu-0", "amg-ilu-n", "amg-jacobi" };
      preCon         = Parameter::getEnum( "istl.preconditioning.method", preConTable, preCon );
      numIterations_ = Parameter::getValue( "istl.preconditioning.iterations", numIterations_ );
      relaxFactor_   = Parameter::getValue( "istl.preconditioning.relaxation", relaxFactor_ );
    }

    if( preCon >= 0 && preCon <= 9)
      preconditioning_ = (PreConder_Id) preCon;
    else
      preConErrorMsg(preCon);
    assert( rowMapper_.size() == colMapper_.size() );
  }

public:
  //! destructor
  ~ModifiedISTLMatrixObject()
  {
    removeObj();
  }

  //! return reference to system matrix
  MatrixType & matrix() const
  {
    assert( matrix_ );
    return *matrix_;
  }

  void printTexInfo(std::ostream& out) const
  {
    out << "ISTL MatrixObj: ";
    out << " preconditioner = " << preconditionName() ;
    out  << "\\\\ \n";
  }

  //! return matrix adapter object
  std::string preconditionName() const
  {
    std::stringstream tmp ;
    // no preconditioner
    switch (preconditioning_)
    {
      case ssor : tmp << "SSOR"; break;
      case sor  : tmp << "SOR"; break;
      case ilu_0: tmp << "ILU-0"; break;
      case ilu_n: tmp << "ILU-n"; break;
      case gauss_seidel : tmp << "Gauss-Seidel"; break;
      case jacobi: tmp << "Jacobi"; break;
      default: tmp << "None"; break;
    }

    if( preconditioning_ != ilu_0 )
    {
      tmp << " n=" << numIterations_;
    }
    tmp << " relax=" << relaxFactor_ ;
    return tmp.str();
  }

  template <class PreconditionerType>
  MatrixAdapterType
  createMatrixAdapter(const PreconditionerType* preconditioning,
                      size_t numIterations) const
  {
    typedef typename MatrixAdapterType :: PreconditionAdapterType PreConType;
    PreConType preconAdapter(matrix(), numIterations, relaxFactor_, preconditioning );
    return MatrixAdapterType(matrix(), rowSpace_, colSpace_, preconAdapter );
  }

  template <class PreconditionerType>
  MatrixAdapterType
  createAMGMatrixAdapter(const PreconditionerType* preconditioning,
                         size_t numIterations) const
  {
    typedef typename MatrixAdapterType :: PreconditionAdapterType PreConType;
    PreConType preconAdapter(matrix(), numIterations, relaxFactor_, preconditioning, rowSpace_.grid().comm() );
    return MatrixAdapterType(matrix(), rowSpace_, colSpace_, preconAdapter );
  }

  //! return matrix adapter object
  const MatrixAdapterType& matrixAdapter() const
  {
    if( matrixAdap_ == 0 )
      matrixAdap_ = new MatrixAdapterType( matrixAdapterObject() );
    return *matrixAdap_;
  }

  const RowSpaceType& rowSpace() const { return rowSpace_; }
  const ColumnSpaceType& colSpace() const { return colSpace_; }

protected:
  MatrixAdapterType matrixAdapterObject() const
  {

    const size_t procs = rowSpace_.grid().comm().size();

    typedef typename MatrixType :: BaseType ISTLMatrixType ;
    typedef typename MatrixAdapterType :: PreconditionAdapterType PreConType;
    // no preconditioner
    if( preconditioning_ == none )
    {
      return MatrixAdapterType(matrix(), rowSpace_,colSpace_, PreConType() );
    }
    else DUNE_THROW(InvalidStateException,"All preconditioning for istl objects has been disabled");
#ifndef DISABLE_ISTL_PRECONDITIONING
    // SSOR
    else if( preconditioning_ == ssor )
    {
      if( procs > 1 )
        DUNE_THROW(InvalidStateException,"ISTL::SeqSSOR not working in parallel computations");

      typedef SeqSSOR<ISTLMatrixType,RowBlockVectorType,ColumnBlockVectorType> PreconditionerType;
      return createMatrixAdapter( (PreconditionerType*)0, numIterations_ );
    }
    // SOR
    else if(preconditioning_ == sor )
    {
      if( procs > 1 )
        DUNE_THROW(InvalidStateException,"ISTL::SeqSOR not working in parallel computations");

      typedef SeqSOR<ISTLMatrixType,RowBlockVectorType,ColumnBlockVectorType> PreconditionerType;
      return createMatrixAdapter( (PreconditionerType*)0, numIterations_ );
    }
    // ILU-0
    else if(preconditioning_ == ilu_0)
    {
      if( procs > 1 )
        DUNE_THROW(InvalidStateException,"ISTL::SeqILU0 not working in parallel computations");

      typedef FemSeqILU0<ISTLMatrixType,RowBlockVectorType,ColumnBlockVectorType> PreconditionerType;
      return createMatrixAdapter( (PreconditionerType*)0, numIterations_ );
    }
    // ILU-n
    else if(preconditioning_ == ilu_n)
    {
      if( procs > 1 )
        DUNE_THROW(InvalidStateException,"ISTL::SeqILUn not working in parallel computations");

      typedef SeqILUn<ISTLMatrixType,RowBlockVectorType,ColumnBlockVectorType> PreconditionerType;
      return createMatrixAdapter( (PreconditionerType*)0, numIterations_ );
    }
    // Gauss-Seidel
    else if(preconditioning_ == gauss_seidel)
    {
      if( procs > 1 )
        DUNE_THROW(InvalidStateException,"ISTL::SeqGS not working in parallel computations");

      typedef SeqGS<ISTLMatrixType,RowBlockVectorType,ColumnBlockVectorType> PreconditionerType;
      return createMatrixAdapter( (PreconditionerType*)0, numIterations_ );
    }
    // Jacobi
    else if(preconditioning_ == jacobi)
    {
      if( procs > 1 && numIterations_ > 1 )
        DUNE_THROW(InvalidStateException,"ISTL::SeqJac only working with istl.preconditioning.iterations: 1 in parallel computations");
      typedef SeqJac<ISTLMatrixType,RowBlockVectorType,ColumnBlockVectorType> PreconditionerType;
      return createMatrixAdapter( (PreconditionerType*)0, numIterations_ );
    }
    // AMG ILU-0
    else if(preconditioning_ == amg_ilu_0)
    {
      // use original SeqILU0 because of some AMG traits classes.
      typedef SeqILU0<ISTLMatrixType,RowBlockVectorType,ColumnBlockVectorType> PreconditionerType;
      return createAMGMatrixAdapter( (PreconditionerType*)0, numIterations_ );
    }
    // AMG ILU-n
    else if(preconditioning_ == amg_ilu_n)
    {
      typedef SeqILUn<ISTLMatrixType,RowBlockVectorType,ColumnBlockVectorType> PreconditionerType;
      return createAMGMatrixAdapter( (PreconditionerType*)0, numIterations_ );
    }
    // AMG Jacobi
    else if(preconditioning_ == amg_jacobi)
    {
      typedef SeqJac<ISTLMatrixType,RowBlockVectorType,ColumnBlockVectorType> PreconditionerType;
      return createAMGMatrixAdapter( (PreconditionerType*)0, numIterations_ );
    }
    else
    {
      preConErrorMsg(preconditioning_);
    }
#endif

    return MatrixAdapterType(matrix(), rowSpace_, colSpace_, PreConType() );
  }

public:
  //! return true, because in case of no preconditioning we have empty
  //! preconditioner (used by OEM methods)
  bool hasPreconditionMatrix() const { return (preconditioning_ != none); }

  //! return reference to preconditioner object (used by OEM methods)
  const PreconditionMatrixType& preconditionMatrix() const { return *this; }

  //! set all matrix entries to zero
  void clear()
  {
    matrix().clear();
  }

  //! reserve matrix with right size
  void reserve(bool verbose = false)
  {
    // if grid sequence number changed, rebuild matrix
    if(sequence_ != rowSpace_.sequence())
    {
      removeObj();

      StencilType stencil;
      matrix_ = new MatrixType(rowSpace_.size()/LittleBlockType::rows,colSpace_.size()/LittleBlockType::cols);
      matrix().setup(rowSpace_,colSpace_,stencil,verbose);

      sequence_ = rowSpace_.sequence();
    }
  }

  //! we only have right precondition
  bool rightPrecondition() const { return true; }

  //! precondition method for OEM Solvers
  //! not fast but works, double is copied to block vector
  //! and after application copied back
  void precondition(const double* arg, double* dest) const
  {
    createBlockVectors();

    assert( Arg_ );
    assert( Dest_ );

    RowBlockVectorType& Arg = *Arg_;
    ColumnBlockVectorType & Dest = *Dest_;

    // copy from double
    double2Block(arg, Arg);

    // set Dest to zero
    Dest = 0;

    assert( matrixAdap_ );
    // not parameter swaped for preconditioner
    matrixAdap_->preconditionAdapter().apply(Dest , Arg);

    // copy back
    block2Double( Dest , dest);
  }

  //! mult method for OEM Solver
  void multOEM(const double* arg, double* dest) const
  {
    createBlockVectors();

    assert( Arg_ );
    assert( Dest_ );

    RowBlockVectorType& Arg = *Arg_;
    ColumnBlockVectorType & Dest = *Dest_;

    // copy from double
    double2Block(arg, Arg);

    // call mult of matrix adapter
    assert( matrixAdap_ );
    matrixAdap_->apply( Arg, Dest );

    //  copy back
    block2Double( Dest , dest);
  }

  //! apply with discrete functions
  void apply(const RowDiscreteFunctionType& arg,
             ColumnDiscreteFunctionType& dest) const
  {
    createMatrixAdapter();
    assert( matrixAdap_ );
    matrixAdap_->apply( arg.blockVector(), dest.blockVector() );
  }

  //! apply with discrete functions
  void apply(const typename RowDiscreteFunctionType::LeakPointerType& arg,
         typename ColumnDiscreteFunctionType::LeakPointerType& dest) const
  {
    createMatrixAdapter();
    assert( matrixAdap_ );
    matrixAdap_->apply( arg, dest );
  }

//  template <class ArgBlockType, class DestBlockType, class ArgDType, class DestDType>
//  void apply(const Dune::BlockVector<ArgBlockType, ArgDType>& arg,
//           Dune::BlockVector<DestBlockType, DestDType>& dest) const
  void apply(const RowBlockVectorType& arg, ColumnBlockVectorType& dest) const
  {
    multOEM( arg, dest );
  }

  //! apply with arbitrary discrete functions calls multOEM
//  template <class RowDFType, class ColDFType>
//  void apply(const RowDFType& arg, ColDFType& dest) const
//  {
//    multOEM( arg.leakPointer(), dest.leakPointer ());
//  }

  //! mult method of matrix object used by oem solver
  void multOEM(const RowLeakPointerType& arg, ColumnLeakPointerType& dest) const
  {
    createMatrixAdapter();
    assert( matrixAdap_ );
    matrixAdap_->apply( arg.blockVector(), dest.blockVector() );
  }
  void multOEM(const RowBlockVectorType& arg, ColumnBlockVectorType& dest) const
  {
    createMatrixAdapter();
    assert( matrixAdap_ );
    matrixAdap_->apply( arg, dest );
  }
  template <class ArgBlockType, class DestBlockType, class ArgDType, class DestDType>
  void multOEM(const Dune::BlockVector<ArgBlockType, ArgDType>& arg,
           Dune::BlockVector<DestBlockType, DestDType>& dest) const
  {
      createMatrixAdapter();
      assert( matrixAdap_ );
      matrixAdap_->apply( arg, dest );
  }

  //! apply with discrete functions
  void applyAdd(const RowDiscreteFunctionType& arg,
         ColumnDiscreteFunctionType& dest) const
  {
    createMatrixAdapter();
    assert( matrixAdap_ );
    ColumnDiscreteFunctionType tmp( dest );
    matrixAdap_->apply( arg.blockVector(), tmp.blockVector() );
    dest += tmp;
  }

  //!
  template <class ArgBlockType, class DestBlockType, class ArgDType, class DestDType>
  void applyAdd(const Dune::BlockVector<ArgBlockType, ArgDType>& arg,
           Dune::BlockVector<DestBlockType, DestDType>& dest) const
  {
    createMatrixAdapter();
    assert( matrixAdap_ );
    Dune::BlockVector<DestBlockType, DestDType > tmp( dest );
    matrixAdap_->apply( arg, tmp );
    dest += tmp;
  }

  //! mult method of matrix object used by oem solver
  void multOEMAdd(const RowLeakPointerType* arg, ColumnLeakPointerType* dest) const
  {
    createMatrixAdapter();
    assert( matrixAdap_ );
    ColumnLeakPointerType tmp;
    matrixAdap_->apply( arg->blockVector(), tmp->blockVector() );
    dest += tmp;
  }

  //! mult method of matrix object used by oem solver
  template <class ArgDofStorageType, class DestDofStorageType, class ArgRangeFieldType, class DestRangeFieldType>
  void multOEMAdd(const Dune::StraightenBlockVector<ArgDofStorageType,ArgRangeFieldType>& arg,
           Dune::StraightenBlockVector<DestDofStorageType,DestRangeFieldType>& dest ) const
  {
      multOEMAdd( arg.blockVector(), dest.blockVector() );
  }
  template <class ArgDofStorageType, class DestDofStorageType>
  void multOEMAdd(const Dune::BlockVector<ArgDofStorageType> &arg,
           Dune::BlockVector<DestDofStorageType> &dest) const
  {
      createMatrixAdapter();
      assert( matrixAdap_ );
      Dune::BlockVector<DestDofStorageType> tmp( dest );
      matrixAdap_->apply( arg, tmp );
      dest += tmp;
  }

  //! dot method for OEM Solver
  double ddotOEM(const double* v, const double* w) const
  {
    createBlockVectors();

    assert( Arg_ );
    assert( Dest_ );

    RowBlockVectorType&    V = *Arg_;
    ColumnBlockVectorType& W = *Dest_;

    // copy from double
    double2Block(v, V);
    double2Block(w, W);

#if HAVE_MPI
    // in parallel use scalar product of discrete functions
    BlockVectorDiscreteFunction< RowSpaceType    > vF("ddotOEM:vF", rowSpace_, V );
    BlockVectorDiscreteFunction< ColumnSpaceType > wF("ddotOEM:wF", colSpace_, W );
    return vF.scalarProductDofs( wF );
#else
    return V * W;
#endif
  }

  //! resort row numbering in matrix to have ascending numbering
  void resort()
  {
  }

  //! create precondition matrix does nothing because preconditioner is
  //! created only when requested
  void createPreconditionMatrix()
  {
  }

  //! print matrix
  void print(std::ostream & s) const
  {
    matrix().print(std::cout);
  }

  const RowMapperType& rowMapper() const { return rowMapper_; }
  const ColMapperType& colMapper() const { return colMapper_; }

  //! interface method from LocalMatrixFactory
  ObjectType* newObject() const
  {
    return new ObjectType(*this,
                          rowSpace_,
                          colSpace_);
  }

  //! return local matrix object
  LocalMatrixType localMatrix(const RowEntityType& rowEntity,
                              const ColumnEntityType& colEntity) const
  {
    return LocalMatrixType(localMatrixStack_,rowEntity,colEntity);
  }

protected:
  void preConErrorMsg(int preCon) const
  {
    std::cerr << "ERROR: Wrong precoditioning number (p = " << preCon;
    std::cerr <<") in ModifiedISTLMatrixObject! \n";
    std::cerr <<"Valid values are: \n";
    std::cerr <<"0 == no \n";
    std::cerr <<"1 == SSOR \n";
    std::cerr <<"2 == SOR \n";
    std::cerr <<"3 == ILU-0 \n";
    std::cerr <<"4 == ILU-n \n";
    std::cerr <<"5 == Gauss-Seidel \n";
    std::cerr <<"6 == Jacobi \n";
    assert(false);
    exit(1);
  }

  void removeObj()
  {
    delete Dest_; Dest_ = 0;
    delete Arg_;  Arg_ = 0;
    delete matrixAdap_; matrixAdap_ = 0;
    delete matrix_; matrix_ = 0;
  }

  // copy double to block vector
  void double2Block(const double* arg, RowBlockVectorType& dest) const
  {
    typedef typename RowBlockVectorType :: block_type BlockType;
    const size_t blocks = dest.size();
    int idx = 0;
    for(size_t i=0; i<blocks; ++i)
    {
      BlockType& block = dest[i];
      enum { blockSize = BlockType :: dimension };
      for(int j=0; j<blockSize; ++j, ++idx)
      {
        block[j] = arg[idx];
      }
    }
  }

  // copy block vector to double
  void block2Double(const ColumnBlockVectorType& arg, double* dest) const
  {
    typedef typename ColumnBlockVectorType :: block_type BlockType;
    const size_t blocks = arg.size();
    int idx = 0;
    for(size_t i=0; i<blocks; ++i)
    {
      const BlockType& block = arg[i];
      enum { blockSize = BlockType :: dimension };
      for(int j=0; j<blockSize; ++j, ++idx)
      {
        dest[idx] = block[j];
      }
    }
  }

  void createBlockVectors() const
  {
    if( ! Arg_ || ! Dest_ )
    {
      delete Arg_; delete Dest_;
      Arg_  = new RowBlockVectorType( rowMapper_.size() );
      Dest_ = new ColumnBlockVectorType( colMapper_.size() );
    }

    createMatrixAdapter ();
  }

  void createMatrixAdapter () const
  {
    if( ! matrixAdap_ )
    {
      matrixAdap_ = new MatrixAdapterType(matrixAdapter());
    }
  }

};



// ISTLMatrixOperator
// ------------------

template< class DomainFunction, class RangeFunction, class TraitsImp >
class ISTLMatrixOperator
: public ModifiedISTLMatrixObject< typename DomainFunction::DiscreteFunctionSpaceType, typename RangeFunction::DiscreteFunctionSpaceType, TraitsImp >,
  public Operator< typename DomainFunction::RangeFieldType, typename RangeFunction::RangeFieldType, DomainFunction, RangeFunction >
{
  typedef ISTLMatrixOperator< DomainFunction, RangeFunction, TraitsImp > This;
  typedef ModifiedISTLMatrixObject< typename DomainFunction::DiscreteFunctionSpaceType, typename RangeFunction::DiscreteFunctionSpaceType, TraitsImp > Base;

public:
  typedef typename Base::ColumnSpaceType DomainSpaceType;
  typedef typename Base::RowSpaceType RangeSpaceType;

  using Base::apply;

  ISTLMatrixOperator ( const std::string &name,
                       const DomainSpaceType &domainSpace,
                       const RangeSpaceType &rangeSpace,
                       const std::string &paramfile = "" )
  : Base( domainSpace, rangeSpace, paramfile )
  {}

  virtual void operator() ( const DomainFunction &arg, RangeFunction &dest ) const
  {
    Base::apply( arg, dest );
  }

  const Base &systemMatrix () const
  {
    return *this;
  }
};

} //namespace Dune
} //namespace Stokes
} //namespace Integrators

#endif // MOD_ISTLMATRIX_HH
