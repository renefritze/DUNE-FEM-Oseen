
namespace Dune {

  //- Trilinear mapping (from alu3dmappings.hh)
  inline TrilinearMapping :: 
  TrilinearMapping (const coord_t& p0, const coord_t& p1,
                    const coord_t& p2, const coord_t& p3, 
                    const coord_t& p4, const coord_t& p5, 
                    const coord_t& p6, const coord_t& p7) 
  {
    buildMapping(p0,p1,p2,p3,p4,p5,p6,p7);
    return ;   
  }

  template <class vector_t> 
  inline void TrilinearMapping ::
  buildMapping(const vector_t& p0, const vector_t& p1,
               const vector_t& p2, const vector_t& p3,
               const vector_t& p4, const vector_t& p5,
               const vector_t& p6, const vector_t& p7)
  {
    // build mapping 
    a [0][0] = p0 [0] ;
    a [0][1] = p0 [1] ;
    a [0][2] = p0 [2] ;
    a [1][0] = p1 [0] - p0 [0] ;
    a [1][1] = p1 [1] - p0 [1] ;
    a [1][2] = p1 [2] - p0 [2] ;
    a [2][0] = p2 [0] - p0 [0] ;
    a [2][1] = p2 [1] - p0 [1] ;
    a [2][2] = p2 [2] - p0 [2] ;
    a [3][0] = p4 [0] - p0 [0] ;
    a [3][1] = p4 [1] - p0 [1] ;
    a [3][2] = p4 [2] - p0 [2] ;
    a [4][0] = p3 [0] - p2 [0] - a [1][0] ;
    a [4][1] = p3 [1] - p2 [1] - a [1][1] ;
    a [4][2] = p3 [2] - p2 [2] - a [1][2] ;
    a [5][0] = p6 [0] - p4 [0] - a [2][0] ;
    a [5][1] = p6 [1] - p4 [1] - a [2][1] ;
    a [5][2] = p6 [2] - p4 [2] - a [2][2] ;
    a [6][0] = p5 [0] - p1 [0] - a [3][0] ;
    a [6][1] = p5 [1] - p1 [1] - a [3][1] ;
    a [6][2] = p5 [2] - p1 [2] - a [3][2] ;
    a [7][0] = p7 [0] - p5 [0] + p4 [0] - p6 [0] - p3 [0] + p1 [0] + a [2][0] ;
    a [7][1] = p7 [1] - p5 [1] + p4 [1] - p6 [1] - p3 [1] + p1 [1] + a [2][1] ;
    a [7][2] = p7 [2] - p5 [2] + p4 [2] - p6 [2] - p3 [2] + p1 [2] + a [2][2] ;

    { 
      double sum = 0.0;
      // sum all factor from non-linear terms 
      for(int i=4; i<8; ++i)
      {
        for(int j=0; j<3; ++j)
        {
          sum += fabs(a[i][j]);
        }
      }

      // mapping is affine when all higher terms are zero
      affine_ = (sum < _epsilon);
    }

    // initialize flags  
    calcedDet_ = calcedInv_ = false;

    return ;   
  }

  inline TrilinearMapping :: TrilinearMapping (const TrilinearMapping & map)
  {
    // copy mapping 
    for (int i = 0 ; i < 8 ; ++i)
      for (int j = 0 ; j < 3 ; ++j) 
        a [i][j] = map.a [i][j] ;
    // copy flags 
    affine_ = map.affine_;
    calcedDet_ = calcedInv_ = false;
    return ;
  }

  inline const FieldMatrix<double, 3, 3>& 
  TrilinearMapping::jacobianInverseTransposed(const coord_t& p) 
  {
    // calculate inverse if not calculated or not affine 
    inverse (p);

    // return reference when already calculated 
    if( calcedInv_ ) 
    {
      return Dfi;
    }
    else 
    {
      // make a copy since Dfi could change during world2map
      invTransposed_ = Dfi;
      return invTransposed_;
    }
  } 

  inline void TrilinearMapping :: 
  map2world(const coord_t& p, coord_t& world) const 
  {
    map2world(p[0], p[1], p[2], world);
    return ;
  }

  inline void TrilinearMapping :: 
  map2world(const double x, const double y, 
            const double z, coord_t& world ) const 
  {
    const double yz  = y * z ;
    const double xz  = x * z ;
    const double xy  = x * y ;
    const double xyz = x * yz ;
    world [0] = a [0][0] + a [1][0] * x + a [2][0] * y + a [3][0] * z + a [4][0] * xy + a [5][0] * yz + a [6][0] * xz + a [7][0] * xyz ;
    world [1] = a [0][1] + a [1][1] * x + a [2][1] * y + a [3][1] * z + a [4][1] * xy + a [5][1] * yz + a [6][1] * xz + a [7][1] * xyz ;
    world [2] = a [0][2] + a [1][2] * x + a [2][2] * y + a [3][2] * z + a [4][2] * xy + a [5][2] * yz + a [6][2] * xz + a [7][2] * xyz ;
    return ;
  }

  inline void TrilinearMapping :: linear(const coord_t& p ) 
  {
    linear(p[0], p[1], p[2]);
  }

  inline void TrilinearMapping :: linear(const double x,
                                         const double y,
                                         const double z) 
  {
    const double yz = y * z ;
    const double xz = x * z ;
    const double xy = x * y ;

    // derivatives with respect to x 
    Df[0][0] = a[1][0] + y * a[4][0] + z * a[6][0] + yz * a[7][0] ;
    Df[1][0] = a[1][1] + y * a[4][1] + z * a[6][1] + yz * a[7][1] ;
    Df[2][0] = a[1][2] + y * a[4][2] + z * a[6][2] + yz * a[7][2] ;

    // derivatives with respect to y 
    Df[0][1] = a[2][0] + x * a[4][0] + z * a[5][0] + xz * a[7][0] ;
    Df[1][1] = a[2][1] + x * a[4][1] + z * a[5][1] + xz * a[7][1] ;
    Df[2][1] = a[2][2] + x * a[4][2] + z * a[5][2] + xz * a[7][2] ;

    // derivatives with respect to z 
    Df[0][2] = a[3][0] + y * a[5][0] + x * a[6][0] + xy * a[7][0] ;
    Df[1][2] = a[3][1] + y * a[5][1] + x * a[6][1] + xy * a[7][1] ;
    Df[2][2] = a[3][2] + y * a[5][2] + x * a[6][2] + xy * a[7][2] ;
  }

  inline double TrilinearMapping :: det(const coord_t& point ) 
  {
    // use cached value of determinant 
    if( calcedDet_ ) return DetDf;
    
    //  Determinante der Abbildung f:[-1,1]^3 -> Hexaeder im Punkt point.
    linear (point) ;

    // code generated by maple 
    const double t4  = Df[0][0] * Df[1][1];
    const double t6  = Df[0][0] * Df[1][2];
    const double t8  = Df[0][1] * Df[1][0];
    const double t10 = Df[0][2] * Df[1][0];
    const double t12 = Df[0][1] * Df[2][0];
    const double t14 = Df[0][2] * Df[2][0];

    // determinant 
    DetDf = (t4*Df[2][2]-t6*Df[2][1]-t8*Df[2][2]+
            t10*Df[2][1]+t12*Df[1][2]-t14*Df[1][1]);

    assert( DetDf > 0 );

    // set calced det to affine (true if affine false otherwise)
    calcedDet_ = affine_ ;
    return DetDf;
  }

  inline void TrilinearMapping :: inverse(const coord_t& point) 
  {
    // return when inverse already calculated 
    if( calcedInv_ ) return ;

    //  Kramer - Regel, det() rechnet Df und DetDf neu aus.
    const double val = 1.0 / det(point) ;

    // calculate inverse^T 
    Dfi[0][0] = ( Df[1][1] * Df[2][2] - Df[1][2] * Df[2][1] ) * val ;
    Dfi[1][0] = ( Df[0][2] * Df[2][1] - Df[0][1] * Df[2][2] ) * val ;
    Dfi[2][0] = ( Df[0][1] * Df[1][2] - Df[0][2] * Df[1][1] ) * val ;
    Dfi[0][1] = ( Df[1][2] * Df[2][0] - Df[1][0] * Df[2][2] ) * val ;
    Dfi[1][1] = ( Df[0][0] * Df[2][2] - Df[0][2] * Df[2][0] ) * val ;
    Dfi[2][1] = ( Df[0][2] * Df[1][0] - Df[0][0] * Df[1][2] ) * val ;
    Dfi[0][2] = ( Df[1][0] * Df[2][1] - Df[1][1] * Df[2][0] ) * val ;
    Dfi[1][2] = ( Df[0][1] * Df[2][0] - Df[0][0] * Df[2][1] ) * val ;
    Dfi[2][2] = ( Df[0][0] * Df[1][1] - Df[0][1] * Df[1][0] ) * val ;

    // set calcedInv_ to affine (true if affine false otherwise)
    calcedInv_ = affine_; 
    return ;
  }

  inline void TrilinearMapping::world2map (const coord_t& wld , coord_t& map ) 
  {
    //  Newton - Iteration zum Invertieren der Abbildung f.
    double err = 10.0 * _epsilon ;
#ifndef NDEBUG
    int count = 0 ;
#endif
    map [0] = map [1] = map [2] = .0 ;
    coord_t upd ;
    do {
      // do mapping 
      map2world (map, upd) ;
      // get inverse 
      inverse ( map ) ;
      const double u0 = upd [0] - wld [0] ;
      const double u1 = upd [1] - wld [1] ;
      const double u2 = upd [2] - wld [2] ;

      // jacobian is stored as transposed 
      const double c0 = Dfi [0][0] * u0 + Dfi [1][0] * u1 + Dfi [2][0] * u2 ;
      const double c1 = Dfi [0][1] * u0 + Dfi [1][1] * u1 + Dfi [2][1] * u2 ;
      const double c2 = Dfi [0][2] * u0 + Dfi [1][2] * u1 + Dfi [2][2] * u2 ;
      map [0] -= c0 ;
      map [1] -= c1 ;
      map [2] -= c2 ;
      err = std::fabs (c0) + std::fabs (c1) + std::fabs (c2) ;
      assert (count ++ < 1000) ;
    } while (err > _epsilon) ;
    return ;
  }

  inline bool TrilinearMapping :: affine () const
  {
    return affine_;
  }

  //- Bilinear surface mapping
  // Constructor for FieldVectors 
  inline BilinearSurfaceMapping :: 
  BilinearSurfaceMapping (double x)
  {
    double p[3] = {x,x,x};
    //initialize with zero
    buildMapping(p,p,p,p);
  }

  //- Bilinear surface mapping
  // Constructor for FieldVectors 
  inline BilinearSurfaceMapping :: 
  BilinearSurfaceMapping (const coord3_t& x0, const coord3_t& x1, 
                          const coord3_t& x2, const coord3_t& x3) 
  {
    buildMapping(x0,x1,x2,x3);
  }

  // Constructor for double[3]
  inline BilinearSurfaceMapping :: 
  BilinearSurfaceMapping (const double3_t & x0, const double3_t & x1, 
                          const double3_t & x2, const double3_t & x3) 
  {
    buildMapping(x0,x1,x2,x3);
  }

  // the real constructor, this can be called fro FieldVectors 
  // and double[3], we dont have to convert one type 
  template <class vector_t>
  inline void BilinearSurfaceMapping :: 
  buildMapping  (const vector_t & _p0, const vector_t & _p1, 
                 const vector_t & _p2, const vector_t & _p3) 
  {
    _b [0][0] = _p0 [0] ;
    _b [0][1] = _p0 [1] ;
    _b [0][2] = _p0 [2] ;
    _b [1][0] = _p1 [0] - _p0 [0] ;
    _b [1][1] = _p1 [1] - _p0 [1] ;
    _b [1][2] = _p1 [2] - _p0 [2] ;
    _b [2][0] = _p2 [0] - _p0 [0] ;
    _b [2][1] = _p2 [1] - _p0 [1] ;
    _b [2][2] = _p2 [2] - _p0 [2] ;
    _b [3][0] = _p3 [0] - _p2 [0] - _b [1][0] ;
    _b [3][1] = _p3 [1] - _p2 [1] - _b [1][1] ;
    _b [3][2] = _p3 [2] - _p2 [2] - _b [1][2] ;

    _n [0][0] = _b [1][1] * _b [2][2] - _b [1][2] * _b [2][1] ;
    _n [0][1] = _b [1][2] * _b [2][0] - _b [1][0] * _b [2][2] ;
    _n [0][2] = _b [1][0] * _b [2][1] - _b [1][1] * _b [2][0] ;
    _n [1][0] = _b [1][1] * _b [3][2] - _b [1][2] * _b [3][1] ;
    _n [1][1] = _b [1][2] * _b [3][0] - _b [1][0] * _b [3][2] ;
    _n [1][2] = _b [1][0] * _b [3][1] - _b [1][1] * _b [3][0] ;
    _n [2][0] = _b [3][1] * _b [2][2] - _b [3][2] * _b [2][1] ;
    _n [2][1] = _b [3][2] * _b [2][0] - _b [3][0] * _b [2][2] ; 
    _n [2][2] = _b [3][0] * _b [2][1] - _b [3][1] * _b [2][0] ;


    { 
      double sum = 0.0;
      // sum all factor from non-linear terms 
      for(int j=0; j<3; ++j)
      {
        sum += fabs(_b[3][j]);
      }
      
      // mapping is affine when all higher terms are zero
      _affine = (sum < _epsilon);
    }

    // initialize flags 
    _calcedDet = _calcedInv = false ;
    
    return ;
  }

  inline BilinearSurfaceMapping :: 
  BilinearSurfaceMapping (const BilinearSurfaceMapping & m)
  {
    {
      for (int i = 0 ; i < 4 ; i ++)
      for (int j = 0 ; j < 3 ; j ++ )
        _b [i][j] = m._b [i][j] ;
    }
    {
      for (int i = 0 ; i < 3 ; i ++)
      for (int j = 0 ; j < 3 ; j ++ )
        _n [i][j] = m._n [i][j] ;
    }
    _affine = m._affine;
    // initialize flags 
    _calcedDet = _calcedInv = false ;
    return ;
  }

  inline void BilinearSurfaceMapping :: 
  map2world (const coord2_t& map, coord3_t& wld) const 
  {
    map2world(map[0],map[1],wld);
  }

  inline void BilinearSurfaceMapping :: 
  map2world (const double x, const double y, coord3_t& w) const 
  {
    const double xy = x * y ;
    w[0] = _b [0][0] + x * _b [1][0] + y * _b [2][0] + xy * _b [3][0] ;
    w[1] = _b [0][1] + x * _b [1][1] + y * _b [2][1] + xy * _b [3][1] ;
    w[2] = _b [0][2] + x * _b [1][2] + y * _b [2][2] + xy * _b [3][2] ;
    return ;
  }

  
  inline void BilinearSurfaceMapping :: 
  map2worldnormal (const double x,  
                   const double y,
                   const double z,
                   coord3_t& w) const 
  {
    normal(x,y,normal_);

    const double xy = x * y ;
    w[0] = _b [0][0] + x * _b [1][0] + y * _b [2][0] + xy * _b [3][0] + z*normal_[0];
    w[1] = _b [0][1] + x * _b [1][1] + y * _b [2][1] + xy * _b [3][1] + z*normal_[1];
    w[2] = _b [0][2] + x * _b [1][2] + y * _b [2][2] + xy * _b [3][2] + z*normal_[2];
    return ;
  }

  inline void BilinearSurfaceMapping ::
  map2worldlinear(const double x, const double y, const double z) const
  { 
    normal(x,y,normal_);

    Df[0][0] = _b [1][0] + y * _b [3][0]+ z*_n[1][0] ;
    Df[1][0] = _b [1][1] + y * _b [3][1]+ z*_n[1][1] ;
    Df[2][0] = _b [1][2] + y * _b [3][2]+ z*_n[1][2] ;
    
    Df[0][1] = _b [2][0] + x * _b [3][0]+ z*_n[2][0] ;
    Df[1][1] = _b [2][1] + x * _b [3][1]+ z*_n[2][1] ;
    Df[2][1] = _b [2][2] + x * _b [3][2]+ z*_n[2][2] ;
    
    Df[0][2] = normal_[0];
    Df[1][2] = normal_[1];
    Df[2][2] = normal_[2];
    
    return ;
  }
  

  inline double BilinearSurfaceMapping :: det(const coord3_t& point ) const 
  {
    // return det if already calculated 
    if( _calcedDet ) return DetDf; 
    
    //  Determinante der Abbildung f:[-1,1]^3 -> Hexaeder im Punkt point.
    map2worldlinear (point[0],point[1],point[2]) ;

    // only true for affine mappings 
    _calcedDet = _affine ;

    return (DetDf = Df.determinant());
  }

  inline void BilinearSurfaceMapping :: inverse(const coord3_t& point ) const 
  {
    if( _calcedInv ) return ;

    //  Kramer - Regel, det() rechnet Df und DetDf neu aus.
    const double val = 1.0 / det(point) ;

    Dfi[0][0] = ( Df[1][1] * Df[2][2] - Df[1][2] * Df[2][1] ) * val ;
    Dfi[0][1] = ( Df[0][2] * Df[2][1] - Df[0][1] * Df[2][2] ) * val ;
    Dfi[0][2] = ( Df[0][1] * Df[1][2] - Df[0][2] * Df[1][1] ) * val ;
    Dfi[1][0] = ( Df[1][2] * Df[2][0] - Df[1][0] * Df[2][2] ) * val ;
    Dfi[1][1] = ( Df[0][0] * Df[2][2] - Df[0][2] * Df[2][0] ) * val ;
    Dfi[1][2] = ( Df[0][2] * Df[1][0] - Df[0][0] * Df[1][2] ) * val ;
    Dfi[2][0] = ( Df[1][0] * Df[2][1] - Df[1][1] * Df[2][0] ) * val ;
    Dfi[2][1] = ( Df[0][1] * Df[2][0] - Df[0][0] * Df[2][1] ) * val ;
    Dfi[2][2] = ( Df[0][0] * Df[1][1] - Df[0][1] * Df[1][0] ) * val ;

    // only true for affine mappings 
    _calcedInv = _affine ;
    return ;
  }

  inline const FieldMatrix<double, 2, 2>& 
  BilinearSurfaceMapping::jacobianInverseTransposed(const coord2_t & local) const
  {
    // if calculated return 
    if( _calcedInv ) return invTransposed_;

    map2worldnormal (local[0],local[1],0.0,tmp_); 
    inverse (tmp_) ;

    // calculate transposed inverse 
    invTransposed_[0][0] = Dfi[0][0];
    invTransposed_[0][1] = Dfi[1][0];

    invTransposed_[1][0] = Dfi[0][1];
    invTransposed_[1][1] = Dfi[1][1];
    
    return invTransposed_;
  } 

  inline void BilinearSurfaceMapping::world2map (const coord3_t& wld , coord2_t& map ) const
  {
    //  Newton - Iteration zum Invertieren der Abbildung f.
    double err = 10.0 * _epsilon ;
    coord3_t map_ (0);
#ifndef NDEBUG
    int count = 0 ;
#endif
    coord3_t upd ;
    do {
      // apply mapping 
      map2worldnormal (map_[0],map_[1],map_[2], upd) ;
      // calculate inverse 
      inverse (map_) ;
      const double u0 = upd [0] - wld [0] ;
      const double u1 = upd [1] - wld [1] ;
      const double u2 = upd [2] - wld [2] ;
      const double c0 = Dfi [0][0] * u0 + Dfi [0][1] * u1 + Dfi [0][2] * u2 ;
      const double c1 = Dfi [1][0] * u0 + Dfi [1][1] * u1 + Dfi [1][2] * u2 ;
      const double c2 = Dfi [2][0] * u0 + Dfi [2][1] * u1 + Dfi [2][2] * u2 ;
      map_ [0] -= c0 ;
      map_ [1] -= c1 ;
      map_ [2] -= c2 ;
      err = fabs (c0) + fabs (c1) + fabs (c2) ;
      assert (count ++ < 3000);
    } 
    while (err > _epsilon) ;

    // get local coordinates 
    map[0] = map_[0];
    map[1] = map_[1];
    return ;
  }

  inline void BilinearSurfaceMapping :: 
  normal (const coord2_t& map, coord3_t& norm) const 
  {
    normal(map[0],map[1],norm);
    return ;
  }
 
  inline void BilinearSurfaceMapping :: 
  normal (const double x, const double y, coord3_t& norm) const {
    norm [0] = -(_n [0][0] + _n [1][0] * x + _n [2][0] * y);
    norm [1] = -(_n [0][1] + _n [1][1] * x + _n [2][1] * y);
    norm [2] = -(_n [0][2] + _n [1][2] * x + _n [2][2] * y);
    return ;
  }

} // end namespace Dune
