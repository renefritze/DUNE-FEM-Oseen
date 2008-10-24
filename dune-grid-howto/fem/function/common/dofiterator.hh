#ifndef DUNE_DOFITERATOR_HH
#define DUNE_DOFITERATOR_HH

#include <dune/fem/misc/bartonnackmaninterface.hh>

namespace Dune
{

  /** \class   DofIteratorInterface
   *  \ingroup DofManager
   *  \brief   interface for DoF iterators of discrete functions
   *
   *  The DoF iterator is an efficient way of walking through the DoFs of a
   *  discrete function.
   */
  template< class DofImp, class DofIteratorImp >
  class DofIteratorInterface
  : public BartonNackmanInterface< DofIteratorInterface< DofImp, DofIteratorImp >,
                                   DofIteratorImp >
  {
  public:
    //! type of the DoFs
    typedef DofImp DofType;

    //! type of the implementation (Barton-Nackman)
    typedef DofIteratorImp DofIteratorType;

  private:
    typedef DofIteratorInterface< DofType, DofIteratorType > ThisType;
    typedef BartonNackmanInterface< ThisType, DofIteratorType > BaseType;

  protected:
    using BaseType :: asImp;
    
  public:
    /** \brief assign another DoF iterator to this one
     *
     *  \param[in]  other  DoF iterator to copy
     */
    inline DofIteratorType &operator= ( const DofIteratorType &other )
    {
      CHECK_AND_CALL_INTERFACE_IMPLEMENTATION( asImp().operator=( other ) );
      return asImp();
    }
    
    /** \brief obtain reference to current DoF
     *
     *  \returns a reference to the current DoF
     */
    inline DofType &operator* ()
    {
      CHECK_INTERFACE_IMPLEMENTATION( *asImp() );
      return *asImp();
    }
    
    /** \brief obtain reference to current DoF
     *
     *  \returns a constant reference to the current DoF
     */
    inline const DofType &operator* () const
    {
      CHECK_INTERFACE_IMPLEMENTATION( *asImp() );
      return *asImp();
    }

    inline const DofImp &operator[] ( const int n ) const
    {
      CHECK_INTERFACE_IMPLEMENTATION( asImp()[ n ] );
      return asImp()[ n ];
    }
    
    inline DofImp &operator[] ( const int n )
    {
      CHECK_INTERFACE_IMPLEMENTATION( asImp()[ n ] );
      return asImp()[ n ];
    }

    /** \brief increment the iterator
     *
     *  Lets the iterator point to the next DoF.
     *
     *  \return reference the the incremented iterator (i.e., *this)
     */
    inline DofIteratorType &operator++ ()
    {
      CHECK_AND_CALL_INTERFACE_IMPLEMENTATON( asImp().operator++() );
      return asImp();
    }

    /** \brief check for equality
     *
     *  \param[in]  other  DoF iterator to compare this one to
     * 
     *  \returns \b true if the iterators are the same, \b false otherewise
     */
    inline bool operator== ( const DofIteratorType &other ) const
    {
      CHECK_INTERFACE_IMPLEMENTATION( asImp().operator==( other ) );
      return asImp().operator==( other );
    }

    /** \brief check for inequality
     *
     *  \param[in]  other  DoF iterator to compare this one to
     * 
     *  \returns \b true if the iterators are the different, \b false otherewise
     */
    inline bool operator!= ( const DofIteratorType &other ) const
    {
      CHECK_INTERFACE_IMPLEMENTATION( asImp().operator!=( other ) );
      return asImp().operator!=( other );
    }

    /** \brief get the global number of the current DoF
     *
     *  \return global number of the current DoF
     */
    inline int index () const
    {  
      CHECK_INTERFACE_IMPLEMENTATION( asImp().index() );
      return asImp().index();
    } 

    /** \brief reset iterator to the first position */
    inline void reset () 
    { 
      CHECK_AND_CALL_INTERFACE_IMPLEMENTATION( asImp().reset() );
    }
  }; // end DofIteratorInterface



  /** \class   DofIteratorDefault
   *  \ingroup DofManager
   *  \brief   default implementation of DofManagerInterface
   */
  template< class DofImp, class DofIteratorImp >
  class DofIteratorDefault
  : public DofIteratorInterface< DofImp, DofIteratorImp >
  {
  public:
    //! type of the DoFs
    typedef DofImp DofType;

    //! type of the implementation (Barton-Nackman)
    typedef DofIteratorImp DofIteratorType;

  private:
    typedef DofIteratorDefault< DofType, DofIteratorType > ThisType;
    typedef DofIteratorInterface< DofType, DofIteratorType > BaseType;

  protected:
    using BaseType :: asImp;
      
  public:
    inline const DofImp &operator[] ( const int n ) const
    {
      DofIteratorType &it = const_cast< DofIteratorType & >( asImp() );
      it.reset();
      for( int i = 0; i < n; ++i )
        ++it; 
      return *asImp();
    }

    inline DofType &operator[] ( const int n )
    {
      asImp().reset();
      for( int i = 0; i < n; ++i )
        ++asImp();
      return *asImp();
    }

    /* \copydoc Dune::DofIteratorInterface::operator!=
     *
     *  \note The default implementation is just
     *  \code
     *  return !operator==( other );
     *  \endcode
     */
    inline bool operator!= ( const DofIteratorType &other ) const
    {
      return !asImp().operator==( other );
    }

    /** \copydoc Dune::DofIteratorInterface::index() const */
    inline int index () const
    {
      DofIteratorType it( asImp() );
      it.reset();
      
      int idx = 0;
      for( ; it != *this; ++it )
        ++idx;

      return idx;
    }
  }; // end class DofIteratorDefault



  /* \class ConstDofIteratorDefault
   * \brief makes a const DoF iterator out of DoF iterator
   */
  template< class DofIteratorImp >
  class ConstDofIteratorDefault
  : public DofIteratorDefault< typename DofIteratorImp :: DofType,
                               DofIteratorImp >
  {
  public:
    //! type of the wrapped DoF iterator
    typedef DofIteratorImp WrappedDofIteratorType;

    //! type of the DoFs
    typedef typename WrappedDofIteratorType :: DofType DofType;

  public:
    typedef ConstDofIteratorDefault< WrappedDofIteratorType > ThisType;
    typedef DofIteratorDefault< DofType, ThisType > BaseType;

  protected:
    WrappedDofIteratorType it_;

  public:
    inline ConstDofIteratorDefault( const WrappedDofIteratorType &it )
    : it_( it )
    {
    }

    inline ConstDofIteratorDefault( const ThisType &other )
    : it_( other.it_ )
    {
    }

    /** \copydoc Dune::DofIteratorInterface::operator= */
    inline const ThisType &operator= ( const ThisType &other )
    {
      it_ = other.it_;
      return *this;
    }

    /** \copydoc Dune::DofIteratorInterface::operator*() const */
    const DofType& operator* () const
    {
      return (*it_); 
    }

    inline const DofType &operator[] ( const int n ) const
    {
      return it_[ n ];
    }
   
    /** \copydoc Dune::DofIteratorInterface::index */
    inline int index () const 
    {
      return it_.index();
    }

    /** \copydoc Dune::DofIteratorInterface::operator++ */
    inline ThisType &operator++ ()
    {
      ++it_;
      return (*this);
    }

    /** \copydoc Dune::DofIteratorInterface::operator== */
    inline bool operator== ( const ThisType &other ) const
    {
      return (it_ == other.it_);
    }
    
    /** \copydoc Dune::DofIteratorInterface::operator!= */
    inline bool operator!= ( const ThisType &other ) const
    {
      return (it_ != other.it_);
    }

    /** \copydoc Dune::DofIteratorInterface::reset */
    inline void reset ()
    {
      it_.reset();
    }

    // note: this method is not in the interface!
    const DofType *vector () const
    {
      return it_.vector();
    }
  }; // end class DofIteratorDefault

} // end namespace Dune 

#endif
