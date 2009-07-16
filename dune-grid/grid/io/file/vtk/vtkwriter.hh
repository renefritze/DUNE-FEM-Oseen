// $Id: vtkwriter.hh 4506 2008-10-28 14:01:28Z robertk $

#ifndef DUNE_VTKWRITER_HH
#define DUNE_VTKWRITER_HH

#include <iostream>
#include <fstream>
#include <vector>
#include <list>
#include <string.h>
#include <dune/common/exceptions.hh>
#include <dune/common/iteratorfacades.hh>
#include <dune/grid/common/mcmgmapper.hh>
#include <dune/grid/common/referenceelements.hh>


// namespace base64
// {
// #include"cencode.c"
// }

/** @file
    @author Peter Bastian
    @brief Provides file i/o for the visualization toolkit
*/

/** 
    put vtk io intro here ...
*/


namespace Dune
{
    /** \brief options for VTK output
        \ingroup VTK */
  struct VTKOptions
  {
    enum OutputType {
      /** @brief Output to the file is in ascii. */
      ascii,
      /** @brief Output to the file is binary. */
      binary, 
      /** @brief Ouput is appended to the binary file. */
      binaryappended
    };
    enum DataMode {
      conforming, nonconforming
    };
  };


  // map type to name in data array
  template<class T>
  struct VTKTypeNameTraits {
    std::string operator () (){
      return "";
    }
  };

  template<>
  struct VTKTypeNameTraits<char> {
    std::string operator () () {
      return "Int8";
    }
    typedef int PrintType;
  };

  template<>
  struct VTKTypeNameTraits<unsigned char> {
    std::string operator () () {
      return "UInt8";
    }   
    typedef int PrintType;
  };

  template<>
  struct VTKTypeNameTraits<short> {
    std::string operator () () {
      return "Int16";
    }   
    typedef short PrintType;
  };

  template<>
  struct VTKTypeNameTraits<unsigned short> {
    std::string operator () () {
      return "UInt16";
    }   
    typedef unsigned short PrintType;
  };

  template<>
  struct VTKTypeNameTraits<int> {
    std::string operator () () {
      return "Int32";
    }   
    typedef int PrintType;
  };

  template<>
  struct VTKTypeNameTraits<unsigned int> {
    std::string operator () () {
      return "UInt32";
    }   
    typedef unsigned int PrintType;
  };

  template<>
  struct VTKTypeNameTraits<float> {
    std::string operator () () {
      return "Float32";
    }   
    typedef float PrintType;
  };

  template<>
  struct VTKTypeNameTraits<double> {
    std::string operator () () {
      return "Float64";
    }   
    typedef double PrintType;
  };


  /** 
   * @brief Writer for the ouput of grid functions in the vtk format.
   * @ingroup VTK
   *
   * Writes arbitrary grid functions (living on cells or vertices of a grid)
   * to a file suitable for easy visualization with 
   * <a href="http://public.kitware.com/VTK/">The Visualization Toolkit (VTK)</a>.
   */
  template<class GridImp, class IS=typename GridImp::template Codim<0>::LeafIndexSet>
  class VTKWriter {
    template<int dim>
    struct P0Layout
    {
      bool contains (Dune::GeometryType gt)
        {
            return gt.dim()==dim;
        }
    };
    
    template<int dim>
    struct P1Layout
    {
      bool contains (Dune::GeometryType gt)
        {
            return gt.dim()==0;
        }
    };    

    // extract types
    enum {n=GridImp::dimension};
    enum {w=GridImp::dimensionworld};
    typedef typename GridImp::ctype DT;
    typedef typename GridImp::Traits::template Codim<0>::Entity Entity;
    typedef typename GridImp::Traits::template Codim<0>::Entity Cell;
    typedef typename GridImp::Traits::template Codim<n>::Entity Vertex;
    typedef IS IndexSet;
    static const PartitionIteratorType vtkPartition = InteriorBorder_Partition;
    typedef typename IS::template Codim<0>::template Partition<vtkPartition>::Iterator GridCellIterator;
    typedef typename IS::template Codim<n>::template Partition<vtkPartition>::Iterator GridVertexIterator;
    typedef MultipleCodimMultipleGeomTypeMapper<GridImp,IS,P1Layout> VertexMapper;
  public:

      /** \brief A base class for grid functions with any return type and dimension
          \ingroup VTK

          Trick : use double as return type
      */
    class VTKFunction
    {
    public:
      //! return number of components
      virtual int ncomps () const = 0;

      //! evaluate single component comp in the entity e at local coordinates xi
      /*! Evaluate the function in an entity at local coordinates.
        @param[in]  comp   number of component to be evaluated
        @param[in]  e      reference to grid entity of codimension 0
        @param[in]  xi     point in local coordinates of the reference element of e
        \return            value of the component
      */
      virtual double evaluate (int comp, const Entity& e, const Dune::FieldVector<DT,n>& xi) const = 0;

      //! get name
      virtual std::string name () const = 0;

      //! virtual destructor
      virtual ~VTKFunction () {}
    };

  private:
    typedef typename std::list<VTKFunction*>::iterator FunctionIterator;
    
    class CellIterator :
      public ForwardIteratorFacade<CellIterator, Entity, Entity&, int>
    {
      GridCellIterator git;
      GridCellIterator gend;
    public:
      CellIterator(const GridCellIterator & x, const GridCellIterator & end) : git(x), gend(end) {};
      void increment ()
        {
          ++git;
          while (git!=gend && git->partitionType()!=InteriorEntity) ++git;
        }
      bool equals (const CellIterator & cit) const
        {
          return git == cit.git;
        }
      Entity& dereference() const
        {
          return *git;
        }
      const FieldVector<DT,n> position() const
        {
          return ReferenceElements<DT,n>::general(git->type()).position(0,0);
        }
    };
    CellIterator cellBegin() const
      {
        return CellIterator(is.template begin<0,vtkPartition>(), is.template end<0,vtkPartition>());
      }
    CellIterator cellEnd() const
      {
        return CellIterator(is.template end<0,vtkPartition>(), is.template end<0,vtkPartition>());
      }
    
    class VertexIterator :
      public ForwardIteratorFacade<VertexIterator, Entity, Entity&, int>
    {
      GridCellIterator git;
      GridCellIterator gend;
      VTKOptions::DataMode datamode;
      int index;
      const VertexMapper & vertexmapper;
      std::vector<bool> visited;
      const std::vector<int> & number;
      int offset;
    protected:
      void basicIncrement ()
        {
          if (git == gend) return;
          index++;
          if (index == git->template count<n>()) {
            offset += git->template count<n>();
            index = 0;
            ++git;
            if(git == gend) return;
            while (git->partitionType()!=InteriorEntity) 
            {
              ++git;
              if(git == gend) return;
            }
          }
        }
    public:
      VertexIterator(const GridCellIterator & x,
                     const GridCellIterator & end,
                     const VTKOptions::DataMode & dm,
                     const VertexMapper & vm,
                     const std::vector<int> & num) :
        git(x), gend(end), datamode(dm), index(0),
        vertexmapper(vm), visited(vm.size(), false),
        number(num), offset(0)
        {
          if (datamode == VTKOptions::conforming && git != gend)
            visited[vertexmapper.template map<n>(*git,index)] = true;
        };
      void increment ()
        {
          switch (datamode)
          {
          case VTKOptions::conforming:
            while(visited[vertexmapper.template map<n>(*git,index)])
            {
              basicIncrement();
              if (git == gend) return;
            }
            visited[vertexmapper.template map<n>(*git,index)] = true;
            break;
          case VTKOptions::nonconforming:
            basicIncrement();
            break;
          }
       }
      bool equals (const VertexIterator & cit) const
        {
          return git == cit.git
            && index == cit.index && datamode == cit.datamode;
        }
      Entity& dereference() const
        {
          return *git;
        }
      int id () const
        {
          switch (datamode)
          {
          case VTKOptions::conforming:
            return
              number[vertexmapper.template map<n>(*git,renumber(*git,index))];
          case VTKOptions::nonconforming:
            return offset + renumber(*git,index);
          default:
            DUNE_THROW(IOError,"VTKWriter: unsupported DataMode" << datamode);
          }
        }
      int localindex () const
        {
          return index;
        }
      const FieldVector<DT,n> & position () const
        {
          return ReferenceElements<DT,n>::general(git->type()).position(index,n);
        }
    };    
    VertexIterator vertexBegin() const
      {
        return VertexIterator(is.template begin<0,vtkPartition>(),
                              is.template end<0,vtkPartition>(),
                              datamode, *vertexmapper, number);
      }
    VertexIterator vertexEnd() const
      {
        return VertexIterator(is.template end<0,vtkPartition>(),
                              is.template end<0,vtkPartition>(),
                              datamode, *vertexmapper, number);
      }    
    
    class CornerIterator :
      public ForwardIteratorFacade<CornerIterator, Entity, Entity&, int>
    {
      GridCellIterator git;
      GridCellIterator gend;
      VTKOptions::DataMode datamode;
      int index;
      const VertexMapper & vertexmapper;
      std::vector<bool> visited;
      const std::vector<int> & number;
      int offset;
    protected:
      void basicIncrement ()
        {
          if (git == gend) return;
          index++;
          if (index == git->template count<n>()) {
            offset += git->template count<n>();
            index = 0;
            ++git;
            if (git == gend) return;
            while (git->partitionType()!=InteriorEntity) 
            {
              ++git;
              if (git == gend) return;
            }
          }
        }
    public:
      CornerIterator(const GridCellIterator & x,
                     const GridCellIterator & end,
                     const VTKOptions::DataMode & dm,
                     const VertexMapper & vm,
                     const std::vector<int> & num) :
        git(x), gend(end), datamode(dm), index(0),
        vertexmapper(vm),
        number(num), offset(0) {};
      void increment ()
        {
          basicIncrement();
        }
      bool equals (const CornerIterator & cit) const
        {
          return git == cit.git
            && index == cit.index && datamode == cit.datamode;
        }
      Entity& dereference() const
        {
          return *git;
        }
      int id () const
        {
          switch (datamode)
          {
          case VTKOptions::conforming:
            return
              number[vertexmapper.template map<n>(*git,renumber(*git,index))];
          case VTKOptions::nonconforming:
            return offset + renumber(*git,index);
          default:
            DUNE_THROW(IOError,"VTKWriter: unsupported DataMode" << datamode);
          }
        }
      int localindex () const
        {
          return index;
        }
    };    
    CornerIterator cornerBegin() const
      {
        return CornerIterator(is.template begin<0,vtkPartition>(),
                              is.template end<0,vtkPartition>(),
                              datamode, *vertexmapper, number);
      }
    CornerIterator cornerEnd() const
      {
        return CornerIterator(is.template end<0,vtkPartition>(),
                              is.template end<0,vtkPartition>(),
                              datamode, *vertexmapper, number);
      }    
        
      /** \brief take a vector and interpret it as cell data
          \ingroup VTK
      */
    template<class V>
    class P0VectorWrapper : public VTKFunction  
    {
      typedef MultipleCodimMultipleGeomTypeMapper<GridImp,IS,P0Layout> VM0;
    public:
      //! return number of components
      virtual int ncomps () const
        {
          return 1;
        }

      //! evaluate
      virtual double evaluate (int comp, const Entity& e, const Dune::FieldVector<DT,n>& xi) const
        {
          return v[mapper.map(e)];
        }
      
      //! get name
      virtual std::string name () const
        {
          return s;
        }

      //! construct from a vector and a name
      P0VectorWrapper (const GridImp& g_, const IS& is_, const V& v_, std::string s_) 
        : g(g_), is(is_), v(v_), s(s_), mapper(g_,is_)
        {
          if (v.size()!=(unsigned int)mapper.size())
            DUNE_THROW(IOError,"VTKWriter::P0VectorWrapper: size mismatch");
        }

      virtual ~P0VectorWrapper() {}
      
    private:
      const GridImp& g;
      const IS& is;
      const V& v;
      std::string s;
      VM0 mapper;
    };

      /** \brief take a vector and interpret it as vertex data
          \ingroup VTK
      */
    template<class V>
    class P1VectorWrapper : public VTKFunction  
    {
      typedef MultipleCodimMultipleGeomTypeMapper<GridImp,IS,P1Layout> VM1;
    public:
      //! return number of components
      virtual int ncomps () const
        {
          return 1;
        }

      //! evaluate
      virtual double evaluate (int comp, const Entity& e, const Dune::FieldVector<DT,n>& xi) const
        {
          double min=1E100;
          int imin=-1;
          Dune::GeometryType gt = e.type();
          for (int i=0; i<e.template count<n>(); ++i)
          {
            Dune::FieldVector<DT,n> 
              local = Dune::ReferenceElements<DT,n>::general(gt).position(i,n);
            local -= xi;
            if (local.infinity_norm()<min)
            {
              min = local.infinity_norm();
              imin = i;
            }
          }
          return v[mapper.template map<n>(e,imin)];
        }
      
      //! get name
      virtual std::string name () const
        {
          return s;
        }

      //! construct from a vector and a name
      P1VectorWrapper (const GridImp& g_, const IS& is_, const V& v_, std::string s_) 
        : g(g_), is(is_), v(v_), s(s_), mapper(g_,is_)
        {
          if (v.size()!=(unsigned int)mapper.size())
            DUNE_THROW(IOError,"VTKWriter::P1VectorWrapper: size mismatch");
        }

      virtual ~P1VectorWrapper() {}
      
    private:
      const GridImp& g;
      const IS& is;
      const V& v;
      std::string s;
      VM1 mapper;
    };

  public:
    /**
     * @brief Constructs a VTKWriter working on the leaf index set of a grid.
     *
     * All functions are supposed to live on the leaf elements of the grid.
     * E. g. you could use a VTKWriter constructed like this for the 
     * visualization of the solution.
     * @param g The grid where the functions to be visualized live.
     * @param dm The data mode??
     */
    VTKWriter (const GridImp& g, VTKOptions::DataMode dm = VTKOptions::conforming) :
      grid(g), is(grid.leafIndexSet()), datamode(dm)
      {
        indentCount = 0;
        numPerLine = 4*3; //should be a multiple of 3 !
      }
 
    /**
     * @brief Construct a VTKWriter working on a specific index set of a grid.
     * 
     * 
     * @param g The grid where the functions to be visualized live.
     * @param i The index set the grid functions live on. (E. g. a level index set.)
     * @param dm The data mode.
     */
    VTKWriter (const GridImp& g, const IndexSet& i, VTKOptions::DataMode dm = VTKOptions::conforming) :
      grid(g), is(i), datamode(dm)
      {
        indentCount = 0;
        numPerLine = 4*3; //should be a multiple of 3 !
      }

    /**
     * @brief Add a grid function that lives on the cells of the grid to the visualization.
     * @param p The function to visualize.
     */
    void addCellData (VTKFunction* p)
      {
        celldata.push_back(p);
      }

    /**
     * @brief Add a grid function (represented by container) that lives on the cells of 
     * the grid to the visualization.
     *
     * The container has to have random access via operator[] (e. g. std::vector). The 
     * value of the grid function for an arbitrary element
     * will be accessed by calling operator[] with the id of the element.
     *
     * @param v The container with the values of the grid function for each cell.
     * @param name A name to indentify the grid function.
     */
    template<class V>
    void addCellData (const V& v, std::string name)
      {
        VTKFunction* p = new P0VectorWrapper<V>(grid,is,v,name);
        celldata.push_back(p);
      }

    /**
     * @brief Add a grid function that lives on the vertices of the grid to the visualization.
     * @param p The function to visualize.
     */
    void addVertexData (VTKFunction* p)
      {
        vertexdata.push_back(p);
      }

    /**
     * @brief Add a grid function (represented by container) that lives on the cells of the 
     * grid to the visualization output.
     *
     * The container has to have random access via operator[] (e. g. std::vector). The value 
     * of the grid function for an arbitrary element
     * will be accessed by calling operator[] with the id of the element.
     *
     * @param v The container with the values of the grid function for each cell.
     * @param name A name to indentify the grid function.
     */
    template<class V>
    void addVertexData (const V& v, std::string name)
      {
        VTKFunction* p = new P1VectorWrapper<V>(grid,is,v,name);
        vertexdata.push_back(p);
      }

    //! clear list of registered functions
    void clear ()
      {
        for (FunctionIterator it=celldata.begin();
             it!=celldata.end(); ++it)
          delete *it;
        celldata.clear();
        for (FunctionIterator it=vertexdata.begin();
             it!=vertexdata.end(); ++it)
          delete *it;
        vertexdata.clear();
      }

    //! destructor
    ~VTKWriter ()
      {
        this->clear();
      }

    /**
     * @brief write output; interface might change later
     * @param name The name of the file to write to.
     * @param ot The output type for the file (default value is ascii).
     */
    void write (const char* name, VTKOptions::OutputType ot = VTKOptions::ascii)
    {
      write( name, ot, grid.comm().size(), grid.comm().rank() );
    }

    //! write parallel output; interface might change later
    /**
     * @brief write parallel output; interface might change later
     * @param name The name of the file to write to.
     * @param path path to write data to 
     * @param extendpath path to write single processor data to 
               (if extendpath = "" then no extra path is used) 
     * @param ot The output type for the file (default value is ascii).
     */
    void pwrite (const char* name,  const char* path, const char* extendpath, 
                 VTKOptions::OutputType ot = VTKOptions::ascii)
    {
      pwrite( name, path, extendpath, ot , grid.comm().size(), grid.comm().rank() );
    }

  protected:    
    enum { MAX_CHAR_LENGTH = 4096 };
    
    // write data for size and rank  
    void write (const char* name, VTKOptions::OutputType ot,
                const int mySize, const int myRank)
      {
        // make data mode visible to private functions
        outputtype=ot;

        // reset byte counter for binary appended output
        bytecount = 0;

        if (mySize==1)
        {
          std::ofstream file;
          char fullname[MAX_CHAR_LENGTH];
          if (n>1)
          {
            sprintf(fullname,"%s.vtu",name);
          }
          else
            sprintf(fullname,"%s.vtp",name);
          if (outputtype==VTKOptions::binaryappended)
            file.open(fullname,std::ios::binary);
          else
            file.open(fullname);
          writeDataFile(file);
          file.close();
        }
        else
        {
          std::ofstream file;
          char fullname[MAX_CHAR_LENGTH];
          if (n>1)
            sprintf(fullname,"s%04d:p%04d:%s.vtu",mySize,myRank,name);
          else
            sprintf(fullname,"s%04d:p%04d:%s.vtp",mySize,myRank,name);
          if (outputtype==VTKOptions::binaryappended)
            file.open(fullname,std::ios::binary);
          else
            file.open(fullname);
          writeDataFile(file);
          file.close();
          grid.comm().barrier();
          if (myRank==0)
          {
            if (n>1)
              sprintf(fullname,"s%04d:%s.pvtu",mySize,name);
            else
              sprintf(fullname,"s%04d:%s.pvtp",mySize,name);
            file.open(fullname);
            writeParallelHeader(file,name,".",mySize);
            file.close();
          }
          grid.comm().barrier();
        }
      }

    //! write output; interface might change later
    void pwrite (const char* name,  const char* path, const char* extendpath, 
                 VTKOptions::OutputType ot,
                 const int mySize, const int myRank)
      {
        // make data mode visible to private functions
        outputtype=ot;

        // reset byte counter for binary appended output
        bytecount = 0;

        // do some magic because paraview can only cope with relative pathes to piece files
        std::ofstream file;
        char piecepath[MAX_CHAR_LENGTH];
        char relpiecepath[MAX_CHAR_LENGTH];
        int n=strlen(path);
        int m=strlen(extendpath);
        if (n>0 && path[0]=='/' && path[n-1]=='/')
        {
          // 1) path is an absolute path to the directory where the pvtu file will be placed
          // 2) extendpath is an absolute path from "/" where the pieces are placed
          // 3) pieces are addressed relative in the pvtu files
          if (m==0)
          {
            // write pieces to root :-)
            piecepath[0] = '/';
            piecepath[1] = '\0';
          }
          else
          {
            // make piecepath absolute with trailing "/"
            char *p=piecepath;
            if (extendpath[0]!='/')
            {
              *p = '/';
              p++;
            }
            for (int i=0; i<m; i++)
            {
              *p = extendpath[i];
              p++;
            }
            if (*(p-1)!='/')
            {
              *p = '/';
              p++;
            }
            *p = '\0';
          }
          // path and piecepath are either "/" or have leading and trailing /
          // count slashes in path 
          int k=0;
          const char *p=path;
          while (*p!='\0')
          {
            if (*p=='/') k++;
            p++;
          }
          char *pp = relpiecepath;
          if (k>1)
          {
            for (int i=0; i<k; i++)
            {
              *pp='.'; pp++; *pp='.'; pp++; *pp='/'; pp++; 
            }
          }
          // now copy the extendpath
          for (int i=0; i<m; i++)
          {
            if (i==0 && extendpath[i]=='/') continue;
            *pp = extendpath[i];
            pp++;
          }
          if ( pp!=relpiecepath && (*(pp-1)!='/') )
          {
            *pp = '/';
            pp++;
          }
          *pp = '\0';
        }
        else
        {
          // 1) path is a relative path to the directory where pvtu files are placed
          // 2) extendpath is relative to where the pvtu files are and there the pieces are placed
          if (n==0 || m==0)
            sprintf(piecepath,"%s%s",path,extendpath);
          else
          {
            // both are non-zero
            if (path[n-1]!='/' && extendpath[0]!='/')
              sprintf(piecepath,"%s/%s",path,extendpath);
            else
              sprintf(piecepath,"%s%s",path,extendpath);
          }
          // the pieces are relative to the pvtu files
          sprintf(relpiecepath,"%s",extendpath);
        }
        char fullname[2*MAX_CHAR_LENGTH];
        if (n>1)
          sprintf(fullname,"%s/s%04d:p%04d:%s.vtu",piecepath,mySize,myRank,name);
        else
          sprintf(fullname,"%s/s%04d:p%04d:%s.vtp",piecepath,mySize,myRank,name);
        if (outputtype==VTKOptions::binaryappended)
          file.open(fullname,std::ios::binary);
        else
          file.open(fullname);
        writeDataFile(file);
        file.close();
        grid.comm().barrier();
        if (myRank==0)
        {
          if (n>1)
            sprintf(fullname,"%s/s%04d:%s.pvtu",path,mySize,name);
          else
            sprintf(fullname,"%s/s%04d:%s.pvtp",path,mySize,name);
          file.open(fullname);
          writeParallelHeader(file,name,relpiecepath,mySize);
          file.close();
        }
        grid.comm().barrier();
      }

  private:

    enum VTKGeometryType
    {
      vtkLine = 3,
      vtkTriangle = 5,
      vtkQuadrilateral = 9,
      vtkTetrahedron = 10,
      vtkHexahedron = 12,
      vtkPrism = 13,
      vtkPyramid = 14
    };
    
    //! mapping from GeometryType to VTKGeometryType
    static VTKGeometryType vtkType(const Dune::GeometryType & t)
      {
        if (t.isLine())
          return vtkLine;
        if (t.isTriangle())
          return vtkTriangle;
        if (t.isQuadrilateral())
          return vtkQuadrilateral;
        if (t.isTetrahedron())
          return vtkTetrahedron;
        if (t.isPyramid())
          return vtkPyramid;
        if (t.isPrism())
          return vtkPrism;
        if (t.isHexahedron())
          return vtkHexahedron;
        DUNE_THROW(IOError,"VTKWriter: unsupported GeometryType " << t);
      }

    //! write header file in parallel case to stream
    void writeParallelHeader (std::ostream& s, const char* piecename, const char* piecepath,
                              int mySize = -1) 
      {
        mySize = (mySize < 0) ? grid.comm().size() : mySize;

        // xml header
        s << "<?xml version=\"1.0\"?>" << std::endl;

        // VTKFile
        if (n>1)
          s << "<VTKFile type=\"PUnstructuredGrid\" version=\"0.1\" byte_order=\"LittleEndian\">" << std::endl;
        else
          s << "<VTKFile type=\"PPolyData\" version=\"0.1\" byte_order=\"LittleEndian\">" << std::endl;
        indentUp();

        // PUnstructuredGrid
        indent(s); 
        if (n>1)
          s << "<PUnstructuredGrid GhostLevel=\"0\">" << std::endl;
        else
          s << "<PPolyData GhostLevel=\"0\">" << std::endl;
        indentUp();

        // PPointData
        indent(s); s << "<PPointData";
        for (FunctionIterator it=vertexdata.begin(); it!=vertexdata.end(); ++it)
          if ((*it)->ncomps()==1)
          {
            s << " Scalars=\"" << (*it)->name() << "\"" ;
            break;
          }
        for (FunctionIterator it=vertexdata.begin(); it!=vertexdata.end(); ++it)
          if ((*it)->ncomps()>1)
          {
            s << " Vectors=\"" << (*it)->name() << "\"" ;
            break;
          }
        s << ">" << std::endl;
        indentUp();
        for (FunctionIterator it=vertexdata.begin(); it!=vertexdata.end(); ++it)
        {
          indent(s); s << "<PDataArray type=\"Float32\" Name=\"" << (*it)->name() << "\" ";
          s << "NumberOfComponents=\"" << ((*it)->ncomps()>1?3:1) << "\" ";
          if (outputtype==VTKOptions::ascii)
            s << "format=\"ascii\"/>" << std::endl;
          if (outputtype==VTKOptions::binary)
            s << "format=\"binary\"/>" << std::endl;
          if (outputtype==VTKOptions::binaryappended)
            s << "format=\"appended\"/>" << std::endl;
        }
        indentDown();
        indent(s); s << "</PPointData>" << std::endl;

        // PCellData
        indent(s); s << "<PCellData";
        for (FunctionIterator it=celldata.begin(); it!=celldata.end(); ++it)
          if ((*it)->ncomps()==1)
          {
            s << " Scalars=\"" << (*it)->name() << "\"" ;
            break;
          }
        for (FunctionIterator it=celldata.begin(); it!=celldata.end(); ++it)
          if ((*it)->ncomps()>1)
          {
            s << " Vectors=\"" << (*it)->name() << "\"" ;
            break;
          }
        s << ">" << std::endl;
        indentUp();
        for (FunctionIterator it=celldata.begin(); it!=celldata.end(); ++it)
        {
          indent(s); s << "<PDataArray type=\"Float32\" Name=\"" << (*it)->name() << "\" ";
          s << "NumberOfComponents=\"" << ((*it)->ncomps()>1?3:1) << "\" ";
          if (outputtype==VTKOptions::ascii)
            s << "format=\"ascii\"/>" << std::endl;
          if (outputtype==VTKOptions::binary)
            s << "format=\"binary\"/>" << std::endl;
          if (outputtype==VTKOptions::binaryappended)
            s << "format=\"appended\"/>" << std::endl;
        }
        indentDown();
        indent(s); s << "</PCellData>" << std::endl;

        // PPoints
        indent(s); s << "<PPoints>" << std::endl;
        indentUp();
        indent(s); s << "<PDataArray type=\"Float32\" Name=\"Coordinates\" NumberOfComponents=\"" << "3" << "\" ";
        if (outputtype==VTKOptions::ascii)
          s << "format=\"ascii\"/>" << std::endl;
        if (outputtype==VTKOptions::binary)
          s << "format=\"binary\"/>" << std::endl;
        if (outputtype==VTKOptions::binaryappended)
          s << "format=\"appended\"/>" << std::endl;
        indentDown();
        indent(s); s << "</PPoints>" << std::endl;

        // Pieces
        for (int i=0; i<mySize; i++)
        {
          char fullname[128];
          if (n>1)
            sprintf(fullname,"%s/s%04d:p%04d:%s.vtu",piecepath,mySize,i,piecename);
          else
            sprintf(fullname,"%s/s%04d:p%04d:%s.vtp",piecepath,mySize,i,piecename);
          indent(s); s << "<Piece Source=\"" << fullname << "\"/>" << std::endl;
        }

        // /PUnstructuredGrid
        indentDown();
        indent(s); 
        if (n>1)
          s << "</PUnstructuredGrid>" << std::endl;
        else
          s << "</PPolyData>" << std::endl;

        // /VTKFile
        indentDown();
        s << "</VTKFile>" << std::endl;   
      }

    //! write data file to stream
    void writeDataFile (std::ostream& s)
      {
        // xml header
        s << "<?xml version=\"1.0\"?>" << std::endl;

        // VTKFile
        if (n>1)
          s << "<VTKFile type=\"UnstructuredGrid\" version=\"0.1\" byte_order=\"LittleEndian\">" << std::endl;
        else
          s << "<VTKFile type=\"PolyData\" version=\"0.1\" byte_order=\"LittleEndian\">" << std::endl;
        indentUp();

        // Grid characteristics
        vertexmapper = new VertexMapper(grid,is);
        if (datamode == VTKOptions::conforming)
        {
          number.resize(vertexmapper->size());
          for (std::vector<int>::size_type i=0; i<number.size(); i++) number[i] = -1;
        }
        nvertices = 0;
        ncells = 0;
        ncorners = 0;
        for (CellIterator it=cellBegin(); it!=cellEnd(); ++it)
        {
          ncells++;
          for (int i=0; i<it->template count<n>(); ++i)
          {
            ncorners++;
            if (datamode == VTKOptions::conforming)
            {
              int alpha = vertexmapper->template map<n>(*it,i);
              if (number[alpha]<0)
                number[alpha] = nvertices++;
            }
            else
            {
              nvertices++;
            }
          }
        }

        // UnstructuredGrid
        indent(s);
        if (n>1)
          s << "<UnstructuredGrid>" << std::endl;
        else
          s << "<PolyData>" << std::endl;
        indentUp();

        // Piece
        indent(s);
        if (n>1)
          s << "<Piece NumberOfPoints=\"" << nvertices << "\" NumberOfCells=\"" << ncells << "\">" << std::endl;
        else
          s << "<Piece NumberOfPoints=\"" << nvertices << "\""
            << " NumberOfVerts=\"0\""
            << " NumberOfLines=\"" << ncells << "\">" 
            << " NumberOfPolys=\"0\"" << std::endl;
        indentUp();

        // PointData
        writeVertexData(s);

        // CellData
        writeCellData(s);

        // Points
        writeGridPoints(s);

        // Cells
        writeGridCells(s);

        // /Piece
        indentDown();
        indent(s); s << "</Piece>" << std::endl;

        // /UnstructuredGrid
        indentDown();
        indent(s); 
        if (n>1)
          s << "</UnstructuredGrid>" << std::endl;
        else
          s << "</PolyData>" << std::endl;

        // write appended binary dat section
        if (outputtype==VTKOptions::binaryappended)
          writeAppendedData(s);

        // /VTKFile
        indentDown();
        s << "</VTKFile>" << std::endl;

        delete vertexmapper; number.clear();
      }

    void writeCellData (std::ostream& s)
      {
        indent(s); s << "<CellData";
        for (FunctionIterator it=celldata.begin(); it!=celldata.end(); ++it)
          if ((*it)->ncomps()==1)
          {
            s << " Scalars=\"" << (*it)->name() << "\"" ;
            break;
          }
        for (FunctionIterator it=celldata.begin(); it!=celldata.end(); ++it)
          if ((*it)->ncomps()>1)
          {
            s << " Vectors=\"" << (*it)->name() << "\"" ;
            break;
          }
        s << ">" << std::endl;
        indentUp();
        for (FunctionIterator it=celldata.begin(); it!=celldata.end(); ++it)
        {
          VTKDataArrayWriter<float> *p=0;
          if (outputtype==VTKOptions::ascii)
            p = new VTKAsciiDataArrayWriter<float>(s,(*it)->name(),(*it)->ncomps());
          if (outputtype==VTKOptions::binary)       
            p = new VTKBinaryDataArrayWriter<float>(s,(*it)->name(),(*it)->ncomps(),(*it)->ncomps()*ncells); 
          if (outputtype==VTKOptions::binaryappended)       
            p = new VTKBinaryAppendedDataArrayWriter<float>(s,(*it)->name(),(*it)->ncomps(),bytecount); 
          for (CellIterator i=cellBegin(); i!=cellEnd(); ++i)
            for (int j=0; j<(*it)->ncomps(); j++)
              p->write((*it)->evaluate(j,*i,i.position()));
          delete p;
        }
        indentDown();
        indent(s); s << "</CellData>" << std::endl;
      }

    void writeVertexData (std::ostream& s)
      {
        indent(s); s << "<PointData";
        for (FunctionIterator it=vertexdata.begin(); it!=vertexdata.end(); ++it)
          if ((*it)->ncomps()==1)
          {
            s << " Scalars=\"" << (*it)->name() << "\"" ;
            break;
          }
        for (FunctionIterator it=vertexdata.begin(); it!=vertexdata.end(); ++it)
          if ((*it)->ncomps()>1)
          {
            s << " Vectors=\"" << (*it)->name() << "\"" ;
            break;
          }
        s << ">" << std::endl;
        indentUp();
        for (FunctionIterator it=vertexdata.begin(); it!=vertexdata.end(); ++it)
        {
          VTKDataArrayWriter<float> *p=0;
          if (outputtype==VTKOptions::ascii)
            p = new VTKAsciiDataArrayWriter<float>(s,(*it)->name(),(*it)->ncomps()); 
          if (outputtype==VTKOptions::binary)
            p = new VTKBinaryDataArrayWriter<float>(s,(*it)->name(),(*it)->ncomps(),(*it)->ncomps()*nvertices); 
          if (outputtype==VTKOptions::binaryappended)
            p = new VTKBinaryAppendedDataArrayWriter<float>(s,(*it)->name(),(*it)->ncomps(),bytecount);
          for (VertexIterator vit=vertexBegin(); vit!=vertexEnd(); ++vit)
          {
            for (int j=0; j<(*it)->ncomps(); j++)
              p->write((*it)->evaluate(j,*vit,vit.position()));
			//vtk file format: a vector data always should have 3 comps(with 3rd comp = 0 in 2D case)
			if((*it)->ncomps()==2)
			  p->write(0.0);
          }
          delete p;
        }
        indentDown();
        indent(s); s << "</PointData>" << std::endl;
      }

    void writeGridPoints (std::ostream& s)
      {
        indent(s); s << "<Points>" << std::endl;
        indentUp();

        VTKDataArrayWriter<float> *p=0;
        if (outputtype==VTKOptions::ascii)
          p = new VTKAsciiDataArrayWriter<float>(s,"Coordinates",3);
        if (outputtype==VTKOptions::binary)
          p = new VTKBinaryDataArrayWriter<float>(s,"Coordinates",3,3*nvertices);
        if (outputtype==VTKOptions::binaryappended)
          p = new VTKBinaryAppendedDataArrayWriter<float>(s,"Coordinates",3,bytecount);
        VertexIterator vEnd = vertexEnd();
        for (VertexIterator vit=vertexBegin(); vit!=vEnd; ++vit)
        {
          int dimw=w;
          for (int j=0; j<std::min(dimw,3); j++)
            p->write(vit->geometry()[vit.localindex()][j]);
          for (int j=std::min(dimw,3); j<3; j++)
            p->write(0.0);
        }
        delete p;
      
        indentDown();
        indent(s); s << "</Points>" << std::endl;
      }

    void writeGridCells (std::ostream& s)
      {
        indent(s); 
        if (n>1)
          s << "<Cells>" << std::endl;
        else
          s << "<Lines>" << std::endl;
        indentUp();

        // connectivity
        VTKDataArrayWriter<int> *p1=0;
        if (outputtype==VTKOptions::ascii)
          p1 = new VTKAsciiDataArrayWriter<int>(s,"connectivity",1); 
        if (outputtype==VTKOptions::binary)
          p1 = new VTKBinaryDataArrayWriter<int>(s,"connectivity",1,ncorners); 
        if (outputtype==VTKOptions::binaryappended)
          p1 = new VTKBinaryAppendedDataArrayWriter<int>(s,"connectivity",1,bytecount);
        for (CornerIterator it=cornerBegin(); it!=cornerEnd(); ++it)
          p1->write(it.id());
        delete p1;

        // offsets
        VTKDataArrayWriter<int> *p2=0;
        if (outputtype==VTKOptions::ascii)
          p2 = new VTKAsciiDataArrayWriter<int>(s,"offsets",1);
        if (outputtype==VTKOptions::binary)
          p2 = new VTKBinaryDataArrayWriter<int>(s,"offsets",1,ncells); 
        if (outputtype==VTKOptions::binaryappended)
          p2 = new VTKBinaryAppendedDataArrayWriter<int>(s,"offsets",1,bytecount);
        {
          int offset = 0;
          for (CellIterator it=cellBegin(); it!=cellEnd(); ++it)
          {
            offset += it->template count<n>();
            p2->write(offset);
          }
        }
        delete p2;

        // types
        if (n>1)
        {
          VTKDataArrayWriter<unsigned char> *p3=0; 
          if (outputtype==VTKOptions::ascii)
            p3 = new VTKAsciiDataArrayWriter<unsigned char>(s,"types",1);
          if (outputtype==VTKOptions::binary)
            p3 = new VTKBinaryDataArrayWriter<unsigned char>(s,"types",1,ncells); 
          if (outputtype==VTKOptions::binaryappended)
            p3 = new VTKBinaryAppendedDataArrayWriter<unsigned char>(s,"types",1,bytecount); 
          for (CellIterator it=cellBegin(); it!=cellEnd(); ++it)
          {
            int vtktype = vtkType(it->type());
            p3->write(vtktype);
          }
          delete p3;
        }

        indentDown();
        indent(s); 
        if (n>1)
          s << "</Cells>" << std::endl;
        else
          s << "</Lines>" << std::endl;
      }


    void writeAppendedData (std::ostream& s)
      {
        indent(s); s << "<AppendedData encoding=\"raw\">" << std::endl;
        indentUp();
        indent(s); s << "_"; // indicates start of binary data

        SimpleStream stream(s);

        // write length before each data block
        unsigned int blocklength;

        // point data     
        for (FunctionIterator it=vertexdata.begin(); it!=vertexdata.end(); ++it)
        {
		  
		  blocklength = nvertices * (*it)->ncomps() * sizeof(float);
		  //vtk file format: a vector data always should have 3 comps(with 3rd comp = 0 in 2D case)
		  if((*it)->ncomps()==2)
			blocklength = nvertices * (3) * sizeof(float);
          stream.write(blocklength);
          std::vector<bool> visited(vertexmapper->size(), false);
          for (VertexIterator vit=vertexBegin(); vit!=vertexEnd(); ++vit)
          {
            for (int j=0; j<(*it)->ncomps(); j++)
            {
              float data = (*it)->evaluate(j,*vit,vit.position());
              stream.write(data);
            }
			//vtk file format: a vector data always should have 3 comps(with 3rd comp = 0 in 2D case)
			if((*it)->ncomps()==2){
			  float data=0.0;
			  stream.write(data);}
          }
        }

        // cell data
        for (FunctionIterator it=celldata.begin(); it!=celldata.end(); ++it)
        {
          blocklength = ncells * (*it)->ncomps() * sizeof(float);
          stream.write(blocklength);
          for (CellIterator i=cellBegin(); i!=cellEnd(); ++i)
            for (int j=0; j<(*it)->ncomps(); j++)
            {
              float data = (*it)->evaluate(j,*i,i.position());
              stream.write(data);
            }
        }
      
        // point coordinates
        blocklength = nvertices * 3 * sizeof(float);
        stream.write(blocklength);
        std::vector<bool> visited(vertexmapper->size(), false);
        for (VertexIterator vit=vertexBegin(); vit!=vertexEnd(); ++vit)
        {
          int dimw=w;
          float data;
          for (int j=0; j<std::min(dimw,3); j++)
          {
            data = vit->geometry()[vit.localindex()][j];
            stream.write(data);
          }
          data = 0;
          for (int j=std::min(dimw,3); j<3; j++)
            stream.write(data);
        }
      
        // connectivity
        blocklength = ncorners * sizeof(unsigned int);
        stream.write(blocklength);
        for (CornerIterator it=cornerBegin(); it!=cornerEnd(); ++it)
        {
          stream.write(it.id());
        }

        // offsets
        blocklength = ncells * sizeof(unsigned int);
        stream.write(blocklength);
        {
          int offset = 0;
          for (CellIterator it=cellBegin(); it!=cellEnd(); ++it)
          {
            offset += it->template count<n>();
            stream.write(offset);
          }
        }

        // cell types
        if (n>1)
        {
          blocklength = ncells * sizeof(unsigned char);
          stream.write(blocklength);
          for (CellIterator it=cellBegin(); it!=cellEnd(); ++it)
          {
            unsigned char vtktype = vtkType(it->type());
            stream.write(vtktype);
          }
        }

        s << std::endl;
        indentDown();
        indent(s); s << "</AppendedData>" << std::endl;
      }
    
    // base class for data array writers
    template<class T>
    class VTKDataArrayWriter
    {
    public:
      virtual void write (T data) = 0;
      virtual ~VTKDataArrayWriter () {}
    };

    // a streaming writer for data array tags
    template<class T>
    class VTKAsciiDataArrayWriter : public VTKDataArrayWriter<T>
    {
    public:
      //! make a new data array writer
      VTKAsciiDataArrayWriter (std::ostream& theStream, std::string name, int ncomps) 
        : s(theStream), counter(0), numPerLine(12)
        {
          VTKTypeNameTraits<T> tn;
          s << "<DataArray type=\"" << tn() << "\" Name=\"" << name << "\" ";
		  //vtk file format: a vector data always should have 3 comps(with 3rd comp = 0 in 2D case)
          if (ncomps>3)
            DUNE_THROW(IOError, "VTKWriter does not support more than 3 components");
          s << "NumberOfComponents=\"" << (ncomps>1?3:1) << "\" ";
          s << "format=\"ascii\">" << std::endl;
        }

      //! write one data element to output stream
      void write (T data)
        {
          typedef typename VTKTypeNameTraits<T>::PrintType PT;
          s << (PT) data << " ";
          counter++;
          if (counter%numPerLine==0) s << std::endl;
        }

      //! finish output; writes end tag
      ~VTKAsciiDataArrayWriter ()
        {
          if (counter%numPerLine!=0) s << std::endl;
          s << "</DataArray>" << std::endl;   
        }

    private:
      std::ostream& s;
      int counter;
      int numPerLine;
    };

    // a streaming writer for data array tags
    template<class T>
    class VTKBinaryDataArrayWriter : public VTKDataArrayWriter<T>
    {
    public:
      //! make a new data array writer
      VTKBinaryDataArrayWriter (std::ostream& theStream, std::string name, int ncomps, int nitems) 
        : s(theStream),bufsize(4096),n(0)
        {
          DUNE_THROW(IOError, "binary does not work yet, use binaryappended!");
          VTKTypeNameTraits<T> tn;
          s << "<DataArray type=\"" << tn() << "\" Name=\"" << name << "\" ";
		  //vtk file format: a vector data always should have 3 comps(with 3rd comp = 0 in 2D case)
          if (ncomps>3)
            DUNE_THROW(IOError, "VTKWriter does not support more than 3 components");
          s << "NumberOfComponents=\"" << (ncomps>1?3:1) << "\" ";
          s << "format=\"binary\">" << std::endl;
          buffer = new char[bufsize*sizeof(T)];
          code = new char[2*bufsize*sizeof(T)];
          unsigned int size = nitems*sizeof(T);
          char* p = reinterpret_cast<char*>(&size);
          memcpy(buffer+n,p,sizeof(int));
          n += sizeof(int);
          //        base64::base64_init_encodestate(&_state);
        }

      //! write one data element to output stream
      void write (T data)
        {
          if (n+sizeof(T)>bufsize)
          {
            // flush buffer
            //          int codelength = base64::base64_encode_block(buffer,n,code,&_state);
            //          s.write(code,codelength);
            n=0;
          }
          char* p = reinterpret_cast<char*>(&data);
          memcpy(buffer+n,p,sizeof(T));
          n += sizeof(T);
        }

      //! finish output; writes end tag
      ~VTKBinaryDataArrayWriter ()
        {
//      int codelength;
          if (n>0)
          {
            //          codelength = base64::base64_encode_block(buffer,n,code,&_state);
            //          s.write(code,codelength);
          }
          //        codelength = base64::base64_encode_blockend(code,&_state);
//      s.write(code,codelength);
          //        base64::base64_init_encodestate(&_state);
          s << std::endl;
          s << "</DataArray>" << std::endl;
          delete [] code;
          delete [] buffer;
        }

    private:
      std::ostream& s;
      //      base64::base64_encodestate _state;
      size_t bufsize;
      char* buffer;
      char* code;
      int n;
    };

    // a streaming writer for data array tags
    template<class T>
    class VTKBinaryAppendedDataArrayWriter : public VTKDataArrayWriter<T>
    {
    public:
      //! make a new data array writer
      VTKBinaryAppendedDataArrayWriter (std::ostream& theStream, std::string name, int ncomps, unsigned int& bc) 
        : s(theStream),bytecount(bc)
        {
          VTKTypeNameTraits<T> tn;
          s << "<DataArray type=\"" << tn() << "\" Name=\"" << name << "\" ";
		  //vtk file format: a vector data always should have 3 comps(with 3rd comp = 0 in 2D case)
          if (ncomps>3)
            DUNE_THROW(IOError, "VTKWriter does not support more than 3 components");
          s << "NumberOfComponents=\"" << (ncomps>1?3:1) << "\" ";
          s << "format=\"appended\" offset=\""<< bytecount << "\" />" << std::endl;
          bytecount += 4; // header
        }

      //! write one data element to output stream
      void write (T data)
        {
          bytecount += sizeof(T);
        }

    private:
      std::ostream& s;
      unsigned int& bytecount;
    };


    // write out data in binary
    class SimpleStream
    {
    public:
      SimpleStream (std::ostream& theStream)
        : s(theStream)
        {}
      template<class T>
      void write (T data)
        {
          char* p = reinterpret_cast<char*>(&data);
          s.write(p,sizeof(T));
        }
    private:
      std::ostream& s;
    };

    void indentUp ()
      {
        indentCount++;
      }

    void indentDown ()
      {
        indentCount--;
      }

    void indent (std::ostream& s)
      {
        for (int i=0; i<indentCount; i++) 
          s << "  ";
      }

    // renumber VTK -> Dune
    static int renumber (const Entity& e, int i)
      {
        static const int quadRenumbering[4] = {0,1,3,2};
        static const int cubeRenumbering[8] = {0,1,3,2,4,5,7,6};
        static const int prismRenumbering[6] = {0,2,1,3,5,4};
        switch (vtkType(e.type()))
        {
        case vtkQuadrilateral:
          return quadRenumbering[i];
        case vtkHexahedron:
          return cubeRenumbering[i];
        case vtkPrism:
          return prismRenumbering[i];
        default:
          return i;
        }
      }

    // the list of registered functions
    std::list<VTKFunction*> celldata;
    std::list<VTKFunction*> vertexdata;

    // the grid
    const GridImp& grid;

    // the indexset
    const IndexSet& is;

    // intend counter
    int indentCount;
    int numPerLine;

    // temporary grid information
    int ncells;
    int nvertices;
    int ncorners;
    VertexMapper* vertexmapper;
    std::vector<int> number;
    VTKOptions::DataMode datamode;
    VTKOptions::OutputType outputtype;
    unsigned int bytecount;
  };

  /** \brief VTKWriter on the leaf grid
      \ingroup VTK
   */
  template<class G>
  class LeafVTKWriter : public VTKWriter<G,typename G::template Codim<0>::LeafIndexSet>
  {
  public:
      /** \brief Construct a VTK writer for the leaf level of a given grid */
    LeafVTKWriter (const G& grid, VTKOptions::DataMode dm = VTKOptions::conforming)
      : VTKWriter<G,typename G::template Codim<0>::LeafIndexSet>(grid,grid.leafIndexSet(),dm)
      {}
  };

  /** \brief VTKWriter on a given level grid
      \ingroup VTK
   */
  template<class G>
  class LevelVTKWriter : public VTKWriter<G, typename G::template Codim<0>::LevelIndexSet>
  {
  public:
      /** \brief Construct a VTK writer for a certain level of a given grid */
    LevelVTKWriter (const G& grid, int level, VTKOptions::DataMode dm = VTKOptions::conforming)
      : VTKWriter<G,typename G::template Codim<0>::LevelIndexSet>(grid,grid.levelIndexSet(level),dm)
      {}
  };
}
#endif
