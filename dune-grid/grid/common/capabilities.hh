#ifndef DUNE_CAPABILITIES_HH
#define DUNE_CAPABILITIES_HH

/** \file
    \brief A set of traits classes to store static information about grid implementation
*/

namespace Dune
{

    /** \brief Contains all capabilities classes */
namespace Capabilities
{

    /** \brief Specialize with 'true' for all codims that a grid implements entities for. (default=false)
    \ingroup GICapabilities
    */
template<class Grid, int codim>
struct hasEntity
{
  static const bool v = false;
};

    /** \brief Specialize with 'true' if implementation supports parallelism. (default=false)
    \ingroup GICapabilities
    */
template<class Grid>
struct isParallel
{
  static const bool v = false;
};

    /** \brief Specialize with 'true' if implementation guarantees conforming level grids. (default=false)
    \ingroup GICapabilities
    */
template<class Grid>
struct isLevelwiseConforming
{
  static const bool v = false;
};

    /** \brief Specialize with 'true' if implementation guarantees a conforming leaf grid. (default=false)
    \ingroup GICapabilities
    */
template<class Grid>
struct isLeafwiseConforming
{
  static const bool v = false;
};

    /** \brief Specialize with 'true' if implementation provides grids with hanging nodes. (default=false)
    \ingroup GICapabilities
    */
template<class Grid>
struct hasHangingNodes
{
  static const bool v = false;
};

    /** \brief Specialize with 'true' if implementation provides backup and restore facilities. (default=false)
    \ingroup GICapabilities
    */
template<class Grid>
struct hasBackupRestoreFacilities
{
  static const bool v = false;
};

    /** \brief Specialize with 'true' if implementation provides unstructured grids. (default=true)
    \ingroup GICapabilities
    */
template <class Grid>
struct IsUnstructured {
  static const bool v = true;
};
  
/*
  forward
  Capabilities::Something<const Grid>
  to
  Capabilities::Something<Grid>
*/

template<class Grid, int codim>
struct hasEntity<const Grid, codim>
{
  static const bool v = Dune::Capabilities::hasEntity<Grid,codim>::v;
};

template<class Grid>
struct isParallel<const Grid>
{
  static const bool v = Dune::Capabilities::isParallel<Grid>::v;
};

template<class Grid>
struct isLevelwiseConforming<const Grid>
{
  static const bool v = Dune::Capabilities::isLevelwiseConforming<Grid>::v;
};

template<class Grid>
struct isLeafwiseConforming<const Grid>
{
  static const bool v = Dune::Capabilities::isLeafwiseConforming<Grid>::v;
};

template<class Grid>
struct hasHangingNodes<const Grid>
{
  static const bool v = Dune::Capabilities::hasHangingNodes<Grid>::v;
};

template<class Grid>
struct hasBackupRestoreFacilities<const Grid> 
{
  static const bool v = Dune::Capabilities::hasBackupRestoreFacilities<Grid>::v;
};

template <class Grid>
struct IsUnstructured<const Grid> {
  static const bool v = Dune::Capabilities::IsUnstructured<Grid>::v;
};
  
}

}

#endif // DUNE_CAPABILITIES_HH
