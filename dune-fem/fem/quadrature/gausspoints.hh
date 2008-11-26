#ifndef DUNE_GAUSSPOINTS_HH
#define DUNE_GAUSSPOINTS_HH

#include <cassert>

namespace Dune
{

  /*! \class GaussPts
   *  \ingroup Quadrature
   *  \brief one-dimensional Gauss points and their weights
   *
   *  GaussPtr is an array of one-dimensional Gauss quadratures for the
   *  interval [0,1]. The index of a quadreture equals its number of quadratrue
   *  points (so there is no 0-th quadrature).
   *
   *  \note This class implements the Singleton pattern
   */
  class GaussPts
  {
  public:
    //! number of available quadratures
    enum { MAXP=10 };

    //! highest quadrature order within the array
    enum { highestOrder=19 };
    
  private:
    double G[MAXP+1][MAXP]; // positions of Gauss points
    double W[MAXP+1][MAXP]; // weights associated with points       
    int    O[MAXP+1];       // order of the rule
    
  private:
    /*! \brief constructor initializing the Gauss points for all orders
     */
    GaussPts ();

  public:
    /*! \brief obtain the singleton object
     *
     *  \returns the singleton instance of this class
     */
    inline static const GaussPts &instance()
    {
      static GaussPts gaussPts;
      return gaussPts;
    }

    /*! \brief obtain the i-th point of the m-th quadratre
     *
     *  \param[in]  m  index of the quadrature
     *  \param[in]  i  number of the point within the quadrature (0 <= i < m)
     *
     *  \returns a double in [0,1] representing the i-th Gauss point
     */
    double point ( int m, int i ) const
    {
      assert(m > 0 && i < m);
      return G[m][i];
    }

    /*! \brief obtain the i-th weight of the m-th quadratre
     *
     *  \param[in]  m  index of the quadrature
     *  \param[in]  i  number of the weight within the quadrature (0 <= i < m)
     *
     *  \returns a double representing the weight i-th Gauss point
     */
    double weight ( int m, int i ) const
    {
      assert(m > 0 && i < m);
      return W[m][i];
    }

    /*! \brief obtain the order of the m-th quadratre
     *
     *  \param[in]  m  index of the quadrature
     *
     *  \returns a double representing the weight i-th Gauss point
     */
    int order ( int m ) const
    {
      return O[m];
    }

    /*! \brief a simple power method
     *
     *  \note This method does not use a template meta program
     *
     *  \param[in] y  base \f$y\f$ of the power
     *  \param[in] d  exponent \f$d\f$ of the power
     *
     *  \returns \f$y^d\f$
     */
    int power ( int y, int d ) const
    {
      int m = 1;
      for( int i = 0; i < d; ++i )
        m *= y;
      return m;
    }
  };

} // end namespace Dune

#include "gausspoints_implementation.hh"
#endif
