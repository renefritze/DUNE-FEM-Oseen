#ifndef BCRSTRAITS_HH
#define BCRSTRAITS_HH

#include <dune/fem/operator/2order/dgmatrixsetup.hh>
#include <dune/stokes/integrators/mod_istlmatrix.hh>

namespace Dune {
namespace Stokes {
namespace Integrators {

/**
 * @brief Class for storing the sparsity pattern of a sparse matrix.
 *
 * A sparsity pattern stores nonzero entries (or entries which might be nonzero)
 * in a matrix @f$A=(a_{ij})_{i,j} @f$,
 * i.e. all entries @f$(i,j)@f$ in the matrix with
 * @f[(a_{ij})_{i,j}\neq 0\quad\forall i\in\{1,\ldots,m\},j\in\{1,\ldots,n\}@f]
 * where @f$m@f$ is the number of rows and  @f$n@f$ is the number of columns
 * of the matrix @f$A@f$.
 *
 * Normally, we want to use this class for storing overlapping degrees of freedom
 * of local basis function.
 */
class SparsityPattern
{
public:
  typedef SparsityPattern
    ThisType;

  //! Type for saving the sparsity pattern.
  typedef std::vector< std::set< unsigned int > >
    SparsityPatternContainerType;

  //! Type for iterating through a row.
  typedef std::set< unsigned int >::const_iterator
    NonZeroColIterator;



  /**
   * @brief Constructor storing the row size.
   *
   * @param rowSize Number of rows for the sparsity pattern.
   */
  SparsityPattern( unsigned int rowSize )
    : sparsityPattern_( rowSize ), sizeN_( rowSize )
  {
  }

  /**
   * @brief Inserts a nonzero entry.
   *
   * @param row The row number for the nonzero entry.
   * @param col The column number for the nonzero entry.
   */
  void insert( unsigned int row, unsigned int col )
  {
      ASSERT_LT( row, sparsityPattern_.size() );
      sparsityPattern_[row].insert( col );
  }

  /**
   * @brief Removes a nonzero entry.
   *
   * @param row The row number for the nonzero entry.
   * @param col The column number for the nonzero entry.
   */
  void erase( unsigned int row, unsigned int col )
  {
      ASSERT_LT( row, sparsityPattern_.size() );
    sparsityPattern_[row].erase( col );
  }

  /**
   * @brief Checks whether block is zero.
   *
   * @param row The row number.
   * @param col The column number.
   */
  bool isZero( unsigned int row, unsigned int col )
  {
      ASSERT_LT( row, sparsityPattern_.size() );
    return ( sparsityPattern_[row].count( col ) == 0 );
  }

  /**
   * @brief Counts the number of nonzeros in a row (counted in blocks).
   *
   * @param row The row number in the matrix, where we want to count the nonzero entries.
   */
  unsigned int countNonZeros( unsigned int row )
  {
      ASSERT_LT( row, sparsityPattern_.size() );
    return sparsityPattern_[row].size();
  }

  /**
   * @brief Returns the number of rows (counted in blocks).
   */
  unsigned int size() const
  {
    return sizeN_;
  }

  /**
   * @brief Returns the number of rows (counted in blocks).
   */
  unsigned int N() const
  {
    return sizeN_;
  }

  /**
   *  @brief Gets pointer to the first nonzero entry .
   *
   *  @param row The row number.
   */
  NonZeroColIterator begin( unsigned int row )
  {
      assert( row < sparsityPattern_.size() );
    return sparsityPattern_[row].begin();
  }

  /**
   *  @brief Gets pointer pointing behind the last nonzero entry.
   *
   *  @param row The row number.
   */
  NonZeroColIterator end( unsigned int row )
  {
      assert( row < sparsityPattern_.size() );
    return sparsityPattern_[row].end();
  }

private:
  //! copy constructor
  SparsityPattern( const ThisType& );

  //! assignment operator
  ThisType& operator=( const ThisType& );

  SparsityPatternContainerType sparsityPattern_;
  unsigned int sizeN_;
};

/**
 * @brief This default implementation sets the diagonal elements
 * to nonzero.
 */
class DefaultSparsityPattern
  : public SparsityPattern
{

  /**
   * @brief Constructor setting the diagonal elements to nonzero.
   *
   * @param rowSize Number of rows for the sparsity pattern.
   */
  DefaultSparsityPattern( unsigned int rowSize )
    : SparsityPattern( rowSize )
  {
    for( unsigned int i = 0; i < rowSize; ++i )
      insert( i, i );
  }

}; //end of class DefaultSparsityPattern

/**
  \brief      Static factory class for matrix classes of type Dune::BCRSMatrix.

  \tparam     BlockType Type to construct a Dune::BCRSMatrix representing the type for
              a block, normally a Dune::FieldMatrix.

  \attention  The sparsity pattern is always created for codim 1 contributions as well! This should be optimized
  \todo       See \attention
 */
template < class AnsatzSpaceType, class TestSpaceType >
class BCRSFactory
{
public:
  /** @brief Creates a new BCRSMatrix object and returns a pointer to the
   * allocated object.
   *
   * Matrices have size @f$ H \times H @f$ where @f$H@f$ is the number
   * of degrees of freedom in the discrete function space @f$ { \cal X }_H @f$.
   * The matrices' sparsity pattern is determined by the discrete function
   * space's basefunction overlap.
   *
   * @param dfs The discrete function space @f$ { \cal X }_H @f$.
   */
  template< class MatrixType >
  static void create( const AnsatzSpaceType& ansatzSpace, const TestSpaceType& testSpace, MatrixType* matrix )
  {
    // some types
    typedef typename AnsatzSpaceType::GridPartType
      GridPartType;

    typedef typename GridPartType::template Codim< 0 >::IteratorType
      ElementIteratorType;

    typedef typename GridPartType::GridType::template Codim< 0 >::Entity
      ElementType;

    typedef typename GridPartType::IntersectionIteratorType
      IntersectionIteratorType;

    typedef typename IntersectionIteratorType::Intersection
      IntersectionType;

    typedef typename IntersectionType::EntityPointer
      ElementPointerType;

    const unsigned int ansatzSize = ansatzSpace.size();
    const unsigned int testSize = testSpace.size();

    typedef SparsityPattern
      PatternType;

    PatternType sPattern( ansatzSize/MatrixType::KK::rows );

    // compute sparsity pattern
    // \todo precompile this in linear subspace
    // \todo use constraints for sparsity pattern
    const ElementIteratorType lastElement = ansatzSpace.end();

    for(  ElementIteratorType elementIterator = ansatzSpace.begin();
          elementIterator != lastElement;
          ++elementIterator )
    {
      const ElementType& element = *elementIterator;

      const int elRowIndex = ansatzSpace.blockMapper().mapToGlobal( element, 0 );
      sPattern.insert( elRowIndex, elRowIndex );
      // do loop over all intersections
      const IntersectionIteratorType lastIntersection = ansatzSpace.gridPart().iend( element );
      for( IntersectionIteratorType intIt = ansatzSpace.gridPart().ibegin( element ); intIt != lastIntersection; ++intIt )
      {
        const IntersectionType& intersection = *intIt;
        // if inner intersection
        if( intersection.neighbor() && !intersection.boundary() )
        {
          // get neighbouring entity
          const ElementPointerType neighbourPtr = intersection.outside();
          const ElementType& neighbour = *neighbourPtr;

          const int nbColIndex = testSpace.blockMapper().mapToGlobal( neighbour , 0 );
          const int nbRowIndex = ansatzSpace.blockMapper().mapToGlobal( neighbour , 0 );
          sPattern.insert( elRowIndex, nbColIndex );
          sPattern.insert( nbRowIndex, nbColIndex );
          const int elColIndex = testSpace.blockMapper().mapToGlobal( element , 0 );
          sPattern.insert( nbColIndex, elColIndex );
        } // end if inner intersection
      } // done loop over all intersections
    }

    for( unsigned int i = 0; i < sPattern.size(); ++i )
    {
      matrix->setrowsize( i, sPattern.countNonZeros( i ) );
    }
    matrix->endrowsizes();

    for( unsigned int i = 0; i < sPattern.size(); ++i )
    {
      typedef SparsityPattern::NonZeroColIterator
        ColIterator;
      ColIterator sit = sPattern.begin( i );
      const ColIterator e = sPattern.end( i );
      for( ; sit!=e; ++sit )
      {
        matrix->addindex( i, *sit );
      }
    }
    matrix->endindices();
  } // end method createPtr

}; // end class MatrixFactory<BCRSMatrix<T> >

template< class RowSpace, class ColumnSpace >
struct ModifiedDGMatrixTraits
{
  typedef RowSpace RowSpaceType;
  typedef ColumnSpace ColumnSpaceType;
  typedef BCRSFactory<RowSpaceType,ColumnSpaceType> StencilType;
  typedef Dune::ParallelScalarProduct< ColumnSpaceType > ParallelScalarProductType;

  template< class M >
  struct Adapter
  {
    typedef Dune::DGParallelMatrixAdapter< M > MatrixAdapterType;
  };

};


} //namespace Dune
} //namespace Stokes
} //namespace Integrators


#endif // BCRSTRAITS_HH
