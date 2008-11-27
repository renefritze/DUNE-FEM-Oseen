#ifndef DUNE_FEM_VERSION_HH
#define DUNE_FEM_VERSION_HH

// These three lines define the dune-fem version.
#define DUNE_FEM_VERSION_MAJOR 0
#define DUNE_FEM_VERSION_MINOR 9
#define DUNE_FEM_VERSION_REVISION 2


// These two lines define the dune-grid version.
#define DUNE_GRID_VERSION_MAJOR 1
#define DUNE_GRID_VERSION_MINOR 1


// Macros for version comparison (dune-fem)
#define DUNE_FEM_VERSION_EQUAL(major,minor) \
  ((DUNE_FEM_VERSION_MAJOR == major) && (DUNE_FEM_VERSION_MINOR == minor))
#define DUNE_FEM_VERSION_EQUAL_REVISON(major,minor,revision) \
  (DUNE_FEM_VERSION_EQUAL(major,minor) \
   && (DUNE_FEM_VERSION_REVISION == revision))

#define DUNE_FEM_VERSION_NEWER(major,minor,revision) \
  ((DUNE_FEM_VERSION_MAJOR > major) \
   || ((DUNE_FEM_VERSION_MAJOR == major) && (DUNE_FEM_VERSION_MINOR > minor)) \
   || ((DUNE_FEM_VERSION_MAJOR == major) && (DUNE_FEM_VERSION_MINOR == minor) \
       && (DUNE_FEM_VERSION_REVISION >= revision)))


// Macros for version comparison (dune-grid)
#define DUNE_GRID_VERSION_EQUAL(major,minor) \
  ((DUNE_GRID_VERSION_MAJOR == major) && (DUNE_GRID_VERSION_MINOR == minor))

#define DUNE_GRID_VERSION_NEWER(major,minor) \
  ((DUNE_GRID_VERSION_MAJOR > major) \
   || ((DUNE_GRID_VERSION_MAJOR == major) && (DUNE_GRID_VERSION_MINOR >= minor)))


#include <sstream>

namespace Dune
{
  
  class DuneFEM
  {
  private:
    typedef DuneFEM ThisType;

  public:
    static const unsigned int MajorVersion = DUNE_FEM_VERSION_MAJOR;
    static const unsigned int MinorVersion = DUNE_FEM_VERSION_MINOR;
    static const unsigned int Revision = DUNE_FEM_VERSION_REVISION;

  private:
    DuneFEM ();
    DuneFEM ( const ThisType &other );

  public:
    static inline std :: string version ()
    {
      std :: ostringstream s;
      s << "dune-fem " << MajorVersion
        << "." << MinorVersion
        << "." << Revision;
      return s.str();
    }

    static inline std :: string version ( unsigned int versionId )
    {
      std :: ostringstream s;
      s << "dune-fem " << (versionId >> 24)
        << "." << ((versionId >> 16) & 0xff)
        << "." << (versionId & 0xffff);
      return s.str();
    }

    static inline unsigned int versionId ( unsigned int majorVersion,
                                           unsigned int minorVersion,
                                           unsigned int revision )
    {
      return (majorVersion << 24) + (minorVersion << 16) + revision;
    }

    static inline unsigned int versionId ()
    {
      return versionId( MajorVersion, MinorVersion, Revision );
    }
  };
  
};

#endif
