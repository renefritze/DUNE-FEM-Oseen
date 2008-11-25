#ifndef DUNE_FEM_QUADRATUREIMP_HH
#define DUNE_FEM_QUADRATUREIMP_HH

#include <cassert>

#include <dune/common/fvector.hh>
#include <dune/common/geometrytype.hh>

#include <dune/fem/storage/array.hh>
#include <dune/fem/quadrature/idprovider.hh>

namespace Dune
{

  /** \class IntegrationPointListImp
   *  \ingroup Quadrature
   *  \brief Generic implementation of an IntegrationPointList
   *
   *  An integration point list is simply a list of points, given in local
   *  coordinates, i.e., coordinates within the reference element.
   *
   *  \note Integration point lists do not change over time. It can safely
   *        be assumed that they always return the same points in the same
   *        order.
   */
  template< typename FieldImp, int dim >
  class IntegrationPointListImp
  {
  public:
    //! field type
    typedef FieldImp FieldType;

  private:
    typedef IntegrationPointListImp< FieldType, dim > ThisType;

  public:
    //! type of local coordinates
    typedef FieldVector< FieldType, dim > CoordinateType;
    
    //! to be revised, look at caching quad 
    enum { codimension = 0 };
    
  private:
    // vector holding the coordinates for each point
    DynamicArray< CoordinateType > points_;

    // identifier of the integration point list
    const size_t id_;
    
  protected:
    /** \brief Constructor
     *
     *  The constructor simply creates an empty point list and stores the
     *  specified identifier.
     * 
     *  \note The constructors of derived classes should fill the integration
     *        point list via addIntegrationPoint.
     *
     *  \note The identifier of an integration point list must be globally
     *        unique. Even integration point lists for different dimensions
     *        must have different identifiers.
     *
     *  \param[in]  id  unique identifier of the integration point list
     *                  (provided by QuadratureProvider)
     */
    inline explicit IntegrationPointListImp( size_t id )
    : points_(),
      id_( id )
    {
    }
   
  private:
    // Copying is forbidden
    IntegrationPointListImp( const IntegrationPointListImp& );

  public:
    // Virtual destructor
    virtual ~IntegrationPointListImp ()
    {
    }

    /** \brief obtain coordinates of i-th integration point
     *
     *  This method returns a reference to the coordinates of the i-th
     *  integration point for 0 <= i < nop(). The integration point is given
     *  in local coordinates, i.e., coordinates with respect to the reference
     *  element.
     * 
     *  \param[in]  i  number of the integration point, 0 <= i < nop()
     *
     *  \returns reference to i-th integration point
     */
    inline const CoordinateType &point ( size_t i ) const
    {
      assert( i < nop() );
      return points_[ i ];
    }

    /** \brief obtain the number of integration points
     *
     *  \returns number of integration points within this list
     */
    size_t nop () const
    {
      return points_.size();
    }

    /** \brief obtain the identifier of the integration point list
     * 
     *  The identifier of an integration point list must be globally unique.
     *  Even integration point lists for different dimensions must have
     *  different identifiers.
     *
     *  \note Quadratures are considered distinct if they differ in one of the
     *        following points: geometry type, order, dimension or implementation.
     * 
     *  \returns globally unique identifier of the integration point list
     */
    size_t id () const
    {
      return id_;
    }

    /** \brief obtain order of the integration point list
     *
     *  The order of a quadrature is the maximal polynomial degree that is
     *  guaranteed to be integrated exactly by the quadrature.
     *
     *  In case of an integration point list, the definition of this value is
     *  left to the implementor.
     *
     *  \returns the order of the integration point list
     */
    virtual int order() const = 0;

    /** \brief obtain GeometryType for this integration point list
     *
     *  Integration point lists are specified in local coordinates, i.e.,
     *  coordinates with respect to the reference element. Hence, each 
     *  integration point list is only valid for one type of geometry, i.e.,
     *  for one reference element. The type can be retrieved via this method.
     *
     *  \returns GeometryType for this integration point list
     */
    virtual GeometryType geometry () const = 0;

  protected:
    /** \brief Adds an integration point to the list
     *
     *  This method allows derived classes to add integration points to the
     *  list. This mehtod should only be used within the constructor of the
     *  derived class.
     */
    void addIntegrationPoint( const CoordinateType &point )
    {
      //points_.push_back( point );
      points_.append( point );
    }
  };



  /** \class QuadratureImp
   *  \ingroup Quadrature
   *  \brief Generic implementation of a Dune quadrature.
   *
   *  A Dune Quadrature is nothing but a list of integration points (see also
   *  IntegrationPointsListImp) and their respective weights.
   *
   *  \note Quadratures do not change over time. It can safely be assumed that
   *        they always return the same points in the same order.
   */
  template< typename FieldImp, int dim >
  class QuadratureImp
  : public IntegrationPointListImp< FieldImp, dim >
  {
  public:
    //! field type
    typedef FieldImp FieldType;
  
  private:
    typedef QuadratureImp< FieldType, dim > ThisType;
    typedef IntegrationPointListImp< FieldType, dim > BaseType;

  public:
    //! type of local coordinates
    typedef typename BaseType :: CoordinateType CoordinateType;

  private:
    // vector holding weights of each integration point 
    DynamicArray< FieldType > weights_;
    //std :: vector< FieldType > weights_;
 
  protected:
    /** \brief Constructor
     *
     *  The constructor simply creates an empty quadrature and stores the
     *  specified identifier.
     * 
     *  \note The constructors of derived classes should fill the quadrature
     *        via addQuadraturePoint
     *
     *  \note The identifier of an integration point list must be globally
     *        unique. Even integration point lists for different dimensions
     *        must have different identifiers.
     *
     *  \param[in]  id  unique identifier of the quadrature (provided by
     *                  QuadratureProvider)
     */
    inline explicit QuadratureImp( size_t id )
    : BaseType( id ),
      weights_()
    {
    }
   
  private:
    // Copying is forbidden
    QuadratureImp ( const QuadratureImp& );

  public:
    virtual ~QuadratureImp ()
    {
    }
    
    /** \brief obtain weight of i-th integration point
     *
     *  This method returns the weight of the i-th integration point for
     *  0 <= i < nop() within the quadrature.
     *
     *  \note The integration point can be obtained via the point() method.
     * 
     *  \note The quadrature weights sum up to the volume of the reference
     *        element.
     *
     *  \param[in]  i  number of the integration point, 0 <= i < nop()
     *
     *  \returns weight of the i-th integration point
     */
    const FieldType &weight ( size_t i ) const
    {
      return weights_[ i ];
    }

  private:
    // Disallow use of addIntegrationPoint for quadratures
    inline void addIntegrationPoint ( const CoordinateType &point )
    {
      BaseType :: addIntegrationPoint( point );
    }

  protected:
    /** \brief Adds a point-weight pair to the quadrature
     *
     *  This method allows derived classes to add quadrature points (and their
     *  respective weights) to the list. This mehtod should only be used within
     *  the constructor of the derived class.
     */
    inline void addQuadraturePoint ( const CoordinateType &point,
                                     const FieldType weight )
    {
      addIntegrationPoint( point );
      //weights_.push_back( weight );
      weights_.append( weight );
    }
  };



  // \brief Allows injection of arbitrary points as quadrature points.
  // Useful to test some features of the quadrature framework in isolation
  // and with known input data. Each TestQuadrature object gets its own
  // unique id.
  template <class ct, int dim>
  class TestQuadrature : public QuadratureImp<ct, dim>
  {
  public:
    typedef FieldVector<ct, dim> CoordinateType;

    // dummy value
    enum { maxOrder_ = 10 };

  public:
    //! Constructor
    TestQuadrature(const GeometryType& geo, int order);

    //! Adds new quadrature point/weight pair
    void newQuadraturePoint(const CoordinateType& c, ct weight);

    //! Desired geometry
    virtual GeometryType geometry() const { return geo_; }

    //! Dummy order method
    virtual int order() const { return order_; }
    
    //! Dummy max order method
    static size_t maxOrder() { return maxOrder_; }
    
  private:
    GeometryType geo_;
    int order_;
  };

} // end namespace Dune

#include "quadratureimp_inline.hh"

#endif
