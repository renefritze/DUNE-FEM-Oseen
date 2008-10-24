namespace Dune {
class DGFPrintInfo {
 public:
  DGFPrintInfo(std::string name) : out((name+".log").c_str()) {
    // out << "Grid generated from file " << name << std::endl;
    out << "DGF parser started" << std::endl;
  }
  void finish() {
    out << "Sucsessful" << std::endl;
  }
  template <class Block>
  void block(const Block& block) {
    out << "Using " << Block::ID << " block" << std::endl;
  }
  void print(std::string mesg) {
    out << mesg << std::endl;
  }
  void step1(int dimw,int nofvtx,int nofel) {
    out << "Step 1 finished: " << std::endl;
    out << "                 " << dimw << "d" << std::endl;
    out << "                 " << nofvtx << " verticies" << std::endl;
    out << "                 " << nofel << " elements" << std::endl;
  }
  void step2(int bndseg,int totalbndseg,
       int bnddomain,int defsegs,int remaining) {
    out << "Step 2 finished: " << std::endl;
    out << "                 " << bndseg  
  << " bnd-segs read in BoundarySegment block" << std::endl;
    out << "                 " << totalbndseg 
  << " bnd-segs in grid" << std::endl;
    out << "                 " << bnddomain
  << " bnd-segs a boundary domain" << std::endl;
    out << "                 " << defsegs  
  << " bnd-seg assigned default value" << std::endl;
    out << "                 " << remaining 
  << " bnd-segs have not been assigned an id!" << std::endl;
  }
  void cube2simplex(DuneGridFormatParser::element_t el) {
    if (el == DuneGridFormatParser::General)
      out << "Simplex block found, thus converting "
    << "cube grid to simplex grid" << std::endl;
    else
      out << "Element type should be simplex, thus converting "
    << "cube grid to simplex grid" << std::endl;
  }
  void automatic() {
    out << "Automatic grid generation" << std::endl;
  }
 private:
  std::ofstream out;
};

// Output to Alberta macrogridfile (2d/3d)
inline void DuneGridFormatParser::writeAlberta(std::ostream& out) {
// writes an output file for in gird type Alberta
  out << "DIM: " << dimw
      << "\n" << "DIM_OF_WORLD: " 
      << dimw << "\n" 
      << "\nnumber of vertices: " 
      << nofvtx <<  "\nnumber of elements: " 
      << nofelements << std::endl; 
  out <<  "\nvertex coordinates: " <<  std::endl;
  for (int n=0;n<nofvtx;n++) {
    for (int j=0;j<dimw;j++) {
      out << vtx[n][j] << " ";
    }
    out << std::endl;
  }
  out << "\nelement vertices: "  << std::endl;
  for (int n=0;n<nofelements;n++) {
    for (int j=0;j<dimw+1;j++) {
      // riesiger Hack! die make6 methode erzeugt nur jeden zweiten Tetraeder 
      // richtig, bei den geraden Tetras muessen die letzten beide Knoten
      // vertauscht werden... 
      if ( cube2simplex && 
          dimw==3 && n%2==0) {
        if (j==2) out << " " << elements[n][3] << " ";
        else if (j==3) out << " " << elements[n][2] << " ";
        else if (j==1) out << " " << elements[n][1] << " ";
        else if (j==0) out << " " << elements[n][0] << " ";
      }
      else 
        out << " " << elements[n][j] << " ";
    }
    out << std::endl;
  }
  out << "\nelement boundaries: "  << std::endl;
  std::map<DGFEntityKey<int>,int>::iterator pos;
  for( int simpl=0; simpl < nofelements ; simpl++) { 
    for (int i =0 ; i< dimw+1 ; i++){
      DGFEntityKey<int> key2(elements[simpl],dimw,i+1);
      pos=facemap.find(key2); 
      if (pos==facemap.end())
        out << "0 ";
      else {
        if (pos->second == 0)
          out << "E ";
        else
          out << pos->second << " ";
      }
    }     
    out << " " << std::endl;
  }  
}

// Output to ALU macrogridfile (3d tetra/hexa)
inline void DuneGridFormatParser::writeAlu(std::ostream& out) 
{
  // wirtes an output file in grid type ALU
  if (dimw==3) {
    if (simplexgrid)
      out << "!Tetrahedra" << std::endl;
    else
      out << "!Hexahedra" << std::endl;
  }
  if (dimw==2) {
    if (!simplexgrid) {
      DUNE_THROW(IOError, "ALU can only handle simplex grids in 2d!");
    }
    out << "!Triangles" << std::endl;
  }
  dverb << "Writing vertices...";
  out << nofvtx << std::endl;
  for (int n=0;n<nofvtx;n++) {
    for (int j=0;j<dimw;j++) {
      out << vtx[n][j] << " ";
    }
    out << std::endl;
  }
  dverb << "done" << std::endl;
  dverb.flush();
  dverb << "Writing Simplices...";
  out << nofelements << std::endl;
  for (int n=0;n<nofelements;n++) {
    if (simplexgrid) {
      for (size_t j=0;j<elements[n].size();j++) 
      {
        out << " " << elements[n][j] << " ";
      }
    } else 
    {
      std::vector<unsigned int> el;
      for (size_t j=0;j<elements[n].size();j++) 
        el.push_back((elements[n][j]));
      if (el.size()==8) 
      {
        unsigned int tmp = el[2];
        el[2] = el[3];
        el[3] = tmp;
        tmp = el[6];
        el[6] = el[7];
        el[7] = tmp;    
      }  
      for (size_t j=0;j<el.size();j++) {
        out << " " << el[j] << " ";
      }
    }
    out << std::endl;
  }
  dverb << "done" << std::endl;
  dverb.flush();
  dverb << "Writing Boundary...";
  out << facemap.size() << std::endl;
  std::map<DGFEntityKey<int>,int>::iterator pos;
  for(pos= facemap.begin(); pos!=facemap.end(); ++pos)
  {
    if (pos->second == 0)
      out << "E ";
    else
      out << -pos->second << " ";
    if (dimw == 3) 
      out << pos->first.size() << " ";
    for (int i=0;i<pos->first.size();i++)
    {
      out << pos->first.origKey(i) << " ";
    }
    out << std::endl;
  }
  if (dimw == 3)
    for (int n=0;n<nofvtx;n++) {
      out << n << " " << -1 << std::endl;
    }
  dverb << "done" << std::endl;
  dverb.flush(); 
}
// Output to Tetgen/Triangle poly-file 
  inline void DuneGridFormatParser::writeTetgenPoly(std::string& name,
                std::string& params) 
{
  if (dimw==2) 
  {
    if (facemap.size()+elements.size()>0) 
    {
      name += ".poly";
      params = " -Ap ";
    }
    else 
    {
      name += ".node";
      params = "";
    }
    info->print("writting poly file "+name);
    std::ofstream polys(name.c_str());
    writeTetgenPoly(polys);
  } 
  else 
  {
    if (facemap.size()>0 && elements.size()==0) 
    {
      name += ".poly";
      info->print("writting poly file "+name);
      std::ofstream polys(name.c_str());
      writeTetgenPoly(polys);
      params = " -p ";
    }
    else 
    {
      {
        std::string tmpname = name;
        tmpname += ".node";
        std::ofstream out(tmpname.c_str());
        int nr = 0;
        dverb << "Writing vertices...";
        out << nofvtx << " " << dimw << " " << nofvtxparams << " 0" << std::endl;
        for (int n=0;n<nofvtx;n++) 
        {
          out << nr++ << "   ";
          for (int j=0;j<dimw;j++) 
          {
            out << vtx[n][j] << " ";
          }
          for (int j=0;j<nofvtxparams;++j) 
          {
            out << vtxParams[n][j] << " ";
          }
          out << std::endl;
        }
      }
      {
  std::string tmpname = name;
  tmpname += ".ele";
  std::ofstream out(tmpname.c_str());
  int nr = 0;
  dverb << "Writing elements...";
  out << elements.size() << " 4 " << nofelparams << std::endl;
  for (size_t n=0;n<elements.size();n++) {
    out << nr++ << "   ";
    for (int j=0;j<4;j++) {
      out << elements[n][j] << " ";
    }
    for (int j=0;j<nofelparams;++j) {
      out << elParams[n][j] << " ";
    }
    out << std::endl;
  }
      }
      {
  std::string tmpname = name;
  tmpname += ".face";
  std::ofstream out(tmpname.c_str());
  std::map<DGFEntityKey<int>,int>::iterator pos;
  int nr = 0;
  dverb << "Writing boundary faces...";
  out << facemap.size() << " 1 " << std::endl;
  for(pos= facemap.begin(); pos!=facemap.end(); ++pos){
    out << nr++ << " ";
    for (int i=0;i<pos->first.size();i++)
      out << pos->first.origKey(i) << " ";
    out << pos->second;
    out << std::endl;
  }
      }
      name += ".node";
      if (elements.size()>0)
  params = " -r ";
      else
  params = "";
    }
  }
}
// Output to Tetgen/Triangle poly-file 
inline void DuneGridFormatParser::writeTetgenPoly(std::ostream& out) {
  int nr = 0;
  dverb << "Writing vertices...";
  out << nofvtx << " " << dimw << " " << nofvtxparams << " 0" << std::endl;
  for (int n=0;n<nofvtx;n++) {
    out << nr++ << "   ";
    for (int j=0;j<dimw;j++) {
      out << vtx[n][j] << " ";
    }
    for (int j=0;j<nofvtxparams;++j) {
      out << vtxParams[n][j] << " ";
    }
    out << std::endl;
  }
  dverb << "done" << std::endl;
  dverb.flush();
  dverb << "Writing Segments...";
  out << facemap.size()+elements.size()*3 << " 1 " << std::endl;
  std::map<DGFEntityKey<int>,int>::iterator pos;
  nr = 0;
  for(size_t i=0; i<elements.size(); ++i) {
    for (int k=0;k<3;k++)
      out << nr++ << " "
    << elements[i][(k+1)%3] << " " << elements[i][(k+2)%3] << " 0\n";
  }
  for(pos= facemap.begin(); pos!=facemap.end(); ++pos){
    if (dimw == 3) {
      out << "1 0 " << pos->second << std::endl;
      out << pos->first.size() << " ";
    }
    else out << nr << " ";
    for (int i=0;i<pos->first.size();i++)
      out << pos->first.origKey(i) << " ";
    if (dimw==2)
      out << pos->second;
    out << std::endl;
    nr++;
  }
  out << "0" << std::endl;
  if (nofelparams>0) {
    assert(dimw==2);
    out << elements.size()*nofelparams << std::endl;
    int nr = 0;
    for(size_t i=0; i<elements.size(); ++i) {
      double coord[2] = {0,0};
      for (int j=0;j<3;j++) {
  coord[0] += vtx[elements[i][j]][0];
  coord[1] += vtx[elements[i][j]][1];
      }
      coord[0] /= 3.;
      coord[1] /= 3.;
      for (int j=0;j<nofelparams;j++) 
  out << nr++ << " " 
      << coord[0] << " " << coord[1] << " " << elParams[i][j] << std::endl;
    }
  } else
    out << 0 << std::endl;
  dverb << "done" << std::endl;
  dverb.flush(); 
}

// read the DGF file and store vertex/element/bound structure
inline bool DuneGridFormatParser::readDuneGrid(std::istream& gridin) 
{
  static const std::string dgfid("DGF");
  std::string idline;
  std::getline(gridin,idline);
  makeupcase(idline);
  std::stringstream idstream(idline);
  std::string id; 
  idstream >> id; 
  cube2simplex = false;
  // compare id to DGF keyword 
  if ( id != dgfid ) 
  {
    std::cerr << "Couldn't find 'DGF' keyword, file is not a DuneGridFormat file ... exiting parser! \n";
    return false;
  }// not a DGF file, prehaps native file format

  info = new DGFPrintInfo("dgfparser");

  dimw=-1;
  IntervalBlock interval(gridin);

  vtxoffset=0;

  // generate cartesian grid?
  if(interval.isactive()) 
  { 
    info->automatic();
    VertexBlock bvtx(gridin,dimw); 
    nofvtx=0;
    if (bvtx.isactive()) 
    {
      nofvtx=bvtx.get(vtx,vtxParams,nofvtxparams);
      info->block(bvtx);
    } 
    info->block(interval);
    cube2simplex = true;
    dimw = interval.dimw();
    simplexgrid = (element == Simplex);
    if (element == General) 
    {
      SimplexBlock bsimplex(gridin,-1,-1,dimw);
      simplexgrid = bsimplex.isactive();
      info->cube2simplex(element);
    } 
    else 
    {
      simplexgrid = (element == Simplex);
    }
    interval.get(vtx,nofvtx,elements,nofelements);
    // nofelements=interval.getHexa(elements);
    if(simplexgrid) {
      nofelements=SimplexBlock::cube2simplex(vtx,elements,elParams);
    }

    // remove copies of vertices 
    removeCopies();
  }   
  else { // ok: grid generation by hand...
    VertexBlock bvtx(gridin,dimw); 
    if (bvtx.isactive()) {
      nofvtx=bvtx.get(vtx,vtxParams,nofvtxparams);
      info->block(bvtx);
      vtxoffset=bvtx.offset();
    } 
    nofelements=0;
    SimplexBlock bsimplex(gridin,nofvtx,vtxoffset,dimw);
    CubeBlock bcube(gridin,nofvtx,vtxoffset,dimw);
    if (bcube.isactive() && element!=Simplex) {
      info->block(bcube);
      nofelements=bcube.get(elements,elParams,nofelparams);    
      if (bsimplex.isactive() && element==General) 
      {
        // make simplex grid
        info->cube2simplex(element);
        nofelements=SimplexBlock::cube2simplex(vtx,elements,elParams);
        simplexgrid=true;
      }
      else
        simplexgrid=false;
    }
    else {
      simplexgrid=true;
      if (bsimplex.isactive()) {
        nofelements+=bsimplex.get(elements,elParams,nofelparams);
        if (dimw == 2) 
          for (size_t i=0;i<elements.size();i++) {
            testTriang(i);
          }
      }
      if (nofelements==0 && bcube.isactive()) {
        info->block(bcube);
        info->cube2simplex(element);
        nofelements=bcube.get(elements,elParams,nofelparams);    
        // make simplex grid
        nofelements=SimplexBlock::cube2simplex(vtx,elements,elParams);
        cube2simplex = true; // needed by AlbertaGrid to write correct simplex info
      }
      else if (bsimplex.isactive()){
        info->block(bsimplex);
      }
    }
  }

  info->step1(dimw,vtx.size(),elements.size());
  // test for tetgen/triangle block (only if simplex-grid allowed)
  if (element!=Cube && SimplexGenerationBlock(gridin).isactive()) {
    if (!interval.isactive())
      generateBoundaries(gridin,true);
    info->automatic();
    simplexgrid=true;
    nofelements=0;
    generateSimplexGrid(gridin);
  } 
  generateBoundaries(gridin,!interval.isactive());
  if (nofelements<=0) {
    DUNE_THROW(DGFException,
         "An Error occured while reading element information"
         << "from the DGF file - no elements found!");
  }  
  info->finish();
  delete info;
  info = 0;
  // we made it - 
  // although prehaps a few boundary segments are still without id :-<
  return true;
}
inline void DuneGridFormatParser::removeCopies() {
  std::vector<int> map(vtx.size());
  std::vector<int> shift(vtx.size());
  for (size_t i=0;i<vtx.size();i++) {
    map[i]=i;
    shift[i]=0;
  }
  nofvtx = vtx.size();
  for (size_t i=0;i<vtx.size();i++) {
    if ((size_t)map[i]!=i) continue;
    for (size_t j=i+1;j<vtx.size();j++) {
      double len=pow(vtx[i][0]-vtx[j][0],2.);
      for (int p=1;p<dimw;p++) 
  len+=pow(vtx[i][p]-vtx[j][p],2.);
      if (len<1e-10) {
  map[j]=i;
  for (size_t k=j+1;k<vtx.size();k++) 
    shift[k]++;
  nofvtx--;
      }
    }
  }
  for (size_t i=0;i<elements.size();i++) {
    for (size_t j=0;j<elements[i].size();j++) {
      elements[i][j]=map[elements[i][j]];
      elements[i][j]-=shift[elements[i][j]];
    }
  }
  for (size_t j=0;j<vtx.size();j++) {
    vtx[j-shift[j]]=vtx[j];
  }
  vtx.resize(nofvtx);
  assert(vtx.size()==size_t(nofvtx));
}
inline void DuneGridFormatParser::
       generateBoundaries(std::istream& gridin,bool bndseg) {
  // **************************************************
  // up to here:
  // filled vertex and element block, now look at boundaries...
  // **************************************************
  // first read macrogrid segments... 
  if (bndseg)
  {
    BoundarySegBlock segbound(gridin, nofvtx,dimw,simplexgrid);     
    if (segbound.isactive()) 
    {
      info->block(segbound);
      nofbound=segbound.get(facemap,(nofelements>0),vtxoffset);
    }
  }
  
  // if no boundary elements, return 
  if (nofelements==0) return;
  
  std::map<DGFEntityKey<int>,int>::iterator pos;
  // now add all boundary faces
  {
    for(int simpl=0; simpl < nofelements ; simpl++) 
    { 
      for (int i =0 ; i<ElementFaceUtil::nofFaces(dimw,elements[simpl])  ; i++)
      {
        DGFEntityKey<int> key2 = ElementFaceUtil::generateFace(dimw,elements[simpl],i);
  
        pos=facemap.find(key2);
        if(pos == facemap.end())
        {
          facemap[key2]=0;
        }
        else if (pos->second==0 || 
                 pos->first.origKeySet()) 
        { // face found twice
          facemap.erase(pos);
        }
        else { // use original key as given in key2
          int value = pos->second;
          facemap.erase(pos);
          facemap[key2] = value;
        }
      }
    }
  }
  // remove unused boundary faces added through boundaryseg block or cube2simplex conversion
  {
    std::map<DGFEntityKey<int>,int>::iterator pos = facemap.begin();
    while (pos!=facemap.end()) {
      if (!pos->first.origKeySet()) {
        std::map<DGFEntityKey<int>,int>::iterator pos1 = pos;
        ++pos;
        facemap.erase(pos1);
      }
      else 
        ++pos;
    }
  }
  
  // now try to assign boundary ids...
  int remainingBndSegs = 0;
  int defaultBndSegs = 0;
  int inbnddomain = 0;
  BoundaryDomBlock dombound(gridin, dimw);     
  if (dombound.isactive()){
    info->block(dombound);
    for (; dombound.ok(); dombound.next()) {
      for(pos=facemap.begin(); pos!=facemap.end(); ++pos){
        if(pos->second == 0) {
          // if an edge of a simplex is inside the domain it has the value zero
          bool isinside=true;
          for (int i=0;i<pos->first.size();i++) 
          {
            if (!dombound.inside(vtx[pos->first[i]])) 
            {
              isinside=false;
              break;
            }
          }
          if (isinside) 
          {
            pos->second = dombound.id(); 
            inbnddomain++;
          }
        }
      }
    }
    // now assign default value to remaining segments - if one was given:
    if (dombound.defaultValueGiven()) {
      info->print("Default boundary ID found");
      for(pos=facemap.begin(); pos!=facemap.end(); ++pos){
        if(pos->second == 0) {
          pos->second = dombound.defaultValue();
    defaultBndSegs++;
        }
      }
    } else {
      for(pos=facemap.begin(); pos!=facemap.end(); ++pos){
        if(pos->second == 0) {
          remainingBndSegs++;
        }
      }
    }
  } else {
    for(pos=facemap.begin(); pos!=facemap.end(); ++pos){
      if(pos->second == 0) {
        remainingBndSegs++;
      }
    }
  }
  info->step2(nofbound,facemap.size(),inbnddomain,defaultBndSegs,remainingBndSegs);
}
/*************************************************************
  caller to tetgen/triangle
****************************************************/
inline void DuneGridFormatParser::generateSimplexGrid(std::istream& gridin) {
  SimplexGenerationBlock para(gridin);     
  info->block(para);

  std::string name = "gridparserfile.polylists.tmp";
  const std::string prefixname = name;
  const std::string inname = "gridparserfile.polylists.tmp";
  std::string params;

  if(!para.hasfile()) {
    {
      writeTetgenPoly(name,params);
    }
  }
  else {
    if (para.filetype().size()==0) {
      readTetgenTriangle(para.filename());
      return;
    }
    dimw = para.dimension();
    if (dimw!=2 && dimw!=3 && dimw!=-1) {
      DUNE_THROW(DGFException,
                 "SimplexGen can only generate 2d or 3d meshes but not in "
                 << dimw << " dimensions!");
    } else if (dimw==-1) {
      DUNE_THROW(DGFException,
                 "SimplexGen: connot determine dimension of grid," <<
                 " include parameter DIMENSION in the "
                 << "Simplexgeneration-Block");
    }
  }
  int call_nr = 1;
  if (dimw==2) 
  {
    std::stringstream command;
    std::string suffix;

    if (para.haspath()) 
      command << para.path() << "/";
    command << "triangle -ej " << params;
    if(para.hasfile())
    {
      name = para.filename();
      suffix = "."+para.filetype();
      command << " " << para.parameter() << " ";
    }
    else 
    {
      suffix = "";
    }

    if (para.minAngle()>0) 
      command << "-q" << para.minAngle() << " ";
    if (para.maxArea()>0) 
      command << "-a" << para.maxArea() << " ";
    command << name << suffix; 
    dverb << "Calling : " << command.str() << std::endl;
    info->print("Calling : "+command.str());
    system(command.str().c_str());
    if (para.display()) {
      std::stringstream command;
      if (para.haspath()) 
        command << para.path() << "/";
      command << "showme " << name; // << ".1.ele"; 
      dverb << "Calling : " << command.str() << std::endl;
      system(command.str().c_str());      
    }
  }
  else if (dimw==3) 
  {
    {// first call
      std::stringstream command;
      std::string suffix;

      if (para.haspath()) 
        command << para.path() << "/";
      command << "tetgen " << params;
      
      if(para.hasfile())
      {
        name = para.filename();
        suffix = "."+para.filetype();
        command << " " << para.parameter() << " ";
      }
      else 
      {
        suffix = "";
      }

      command << name << suffix;
      dverb << "Calling : " << command.str() << std::endl;
      info->print("Calling : "+command.str());
      system(command.str().c_str());
    }
    if (para.minAngle()>0 || para.maxArea()>0)
    {// second call
      info->print("Quality enhancement:");
      call_nr = 2;
      std::stringstream command;
      if (para.haspath()) 
        command << para.path() << "/";
      
      command << "tetgen -r";
      if (para.minAngle()>0) 
        command << "q" << para.minAngle();
      
      if (para.maxArea()>0) 
        command << "a" << para.maxArea();

      command << " " << inname << ".1"; 
      dverb << "Calling : " << command.str() << std::endl;
      info->print("Calling : "+command.str());
      system(command.str().c_str());
    }
    if (para.display()) 
    {
      std::stringstream command;
      if (para.haspath()) 
        command << para.path() << "/";

      command << "tetview-linux " << prefixname << "." << call_nr << ".ele"; 
      dverb << "Calling : " << command.str() << std::endl;
      system(command.str().c_str());      
    }
  }
  std::stringstream polyname;
  polyname << inname << "." << call_nr;
  readTetgenTriangle(polyname.str());
  info->print("Automatic grid generation finished");
}
  
inline void DuneGridFormatParser::readTetgenTriangle(std::string name) {
  int offset,bnd;
  std::string nodename = name + ".node";
  std::string elename = name + ".ele";
  std::string polyname = name + ((dimw==2)?".edge":".face");
  dverb << "opening " << nodename << "\n";
  std::ifstream node(nodename.c_str());
  if (!node) {
    DUNE_THROW(DGFException,
               "could not find file " << nodename
               << " prehaps something went wrong with Tetgen/Triangle?");
  }
  dverb << "opening " << elename << "\n";
  std::ifstream ele(elename.c_str());
  if (!ele) {
    DUNE_THROW(DGFException,
               "could not find file " << elename
               << " prehaps something went wrong with Tetgen/Triangle?");
  }
  {
    dverb << "calculating offset from " << name << " .... offset = ";
    node >> nofvtx >> dimw >> nofvtxparams >> bnd;
    // offset is 0 by default 
    // the offset it the difference of the first vertex number to zero 
    node >> offset; 
    dverb << offset << " \n";
    node.seekg(0);
  }  
  {
    int tmp;
    // first token is number of vertex which should equal i
    node >> nofvtx >> dimw >> nofvtxparams >> bnd;
    vtx.resize(nofvtx);
    if (nofvtxparams>0)
      vtxParams.resize(nofvtx);
    for (int i=0;i<nofvtx;i++) {
      vtx[i].resize(dimw);
      int nr;
      node >> nr; 
      // first token is number of vertex which should equal i
      assert(nr-offset==i); 
      for (int v=0;v<dimw;v++)
  node >> vtx[i][v];
      if (nofvtxparams>0) {
  vtxParams[i].resize(nofvtxparams);
  for (int p=0;p<nofvtxparams;p++)
    node >> vtxParams[i][p];
      }
      for (int p=0;p<bnd;p++)
  node >> tmp;
    }
  }
  {
    int tmp;
    ele >> nofelements >> tmp >> nofelparams;
    elements.resize(nofelements);
    if (nofelparams>0)
      elParams.resize(nofelements);
    for (int i=0;i<nofelements;i++) {
      elements[i].resize(dimw+1);
      int nr;
      ele >> nr;
      assert(nr-offset==i);
      for (int v=0;v<dimw+1;v++) {
        int elno; 
  ele >> elno;
  elements[i][v] = elno - offset;
      }
      if (nofelparams>0) {
  elParams[i].resize(nofelparams);
  for (int p=0;p<nofelparams;p++)
    ele >> elParams[i][p];
      }
    }
  }
  dverb << "opening " << polyname << "\n";
  std::ifstream poly(polyname.c_str());
  if (poly) {
  /*
  if (dimw==2) {
    poly >> tmp >> tmp >> tmp >> tmp;
  }
  */
  {
    int noffaces,params;
    poly >> noffaces >> params;
    if (params>0) {
      assert(params==1);
      facemap.clear();
      for (int i=0;i<noffaces;i++) {
  std::vector<int> p(dimw);
  int nr;
  poly >> nr;
  for (size_t k=0;k<p.size();k++)
    poly >> p[k]; 
  poly >> params;
  if (params!=0) {
    DGFEntityKey<int> key(p,false);
    facemap[key]=params;
  }
      }
    }
  } 
  }
}
/***************************
  Helper methods mostly only for simplex grids
 ***************************/
inline void
DuneGridFormatParser::setOrientation(int use1,int use2,
        orientation_t orientation) {
  if (element == Cube) {
    std::cerr << "Reorientation is only implemented for simplex grid!" 
              << std::endl;
    return;
  }
  if (dimw==2) {
    for (int i=0;i<nofelements;i++) {
      if (elements[i].size()!=size_t(dimw+1))
  continue;
      double o=testTriang(i);
      if (o*orientation<0) {  // wrong orientation
        // dverb << "Reorientation of simplex " << i << std::endl;
        int tmp=elements[i][use1];
        elements[i][use1] = elements[i][use2];
        elements[i][use2] = tmp;
      }
    }
  }
  else if (dimw==3) 
  {
    const ReferenceSimplex<double,3> refElem; 
    for (int i=0;i<nofelements;i++) 
    {
      if (elements[i].size()!=size_t(dimw+1))
        continue;

      std::vector<double>& p0 = vtx[elements[i][1]];
      std::vector<double>& p1 = vtx[elements[i][2]];
      std::vector<double>& p2 = vtx[elements[i][3]];
      std::vector<double>& q  = vtx[elements[i][0]];
      
      double n[3];
      n[0] = -((p1[1]-p0[1]) *(p2[2]-p0[2]) - (p2[1]-p0[1]) *(p1[2]-p0[2])) ;
      n[1] = -((p1[2]-p0[2]) *(p2[0]-p0[0]) - (p2[2]-p0[2]) *(p1[0]-p0[0])) ;
      n[2] = -((p1[0]-p0[0]) *(p2[1]-p0[1]) - (p2[0]-p0[0]) *(p1[1]-p0[1])) ;
      double test = n[0]*(q[0]-p0[0])+n[1]*(q[1]-p0[1])+n[2]*(q[2]-p0[2]);      
      bool reorient = (test*orientation<0);
      if (reorient) 
      {
        // reorient element first 
        int tmp=elements[i][use1];
        elements[i][use1] = elements[i][use2];
        elements[i][use2] = tmp;

        // for all faces, check reorient 
        for(int k=0; k<refElem.size(1); ++k) 
        {
          int numVerts = refElem.size(k,1,dimw);
          std::vector<int> face(numVerts);
          // get face vertices 
          for(int j=0; j<numVerts; ++j) 
          {
            int vx = refElem.subEntity(k,1,j,dimw);
            face[j] = elements[i][vx];
          }

          {
            // key is now right oriented 
            DGFEntityKey<int> key(face);

            typedef facemap_t :: iterator iterator;
            
            iterator bndFace = facemap.find(key);
            if(bndFace != facemap.end())
            {
              // delete old key, and store new key 
              int bndId = bndFace->second;
              facemap.erase(bndFace);
              facemap[key] = bndId;
            }
          }
        }

      }
    }
  }
}
inline void
DuneGridFormatParser::setRefinement(int use1,int use2,int is1,int is2) {
  if (element == Cube) {
    std::cerr << "Computing refinement vertex is only implemented for 2d simplex grid!" 
              << std::endl;
    return;
  }
  if (use1>use2){
    int tmp=use1;
    use1=use2;
    use2=tmp;
  }
  if (is1>is2) {
    int tmp=is1;
    is1=is2;
    is2=tmp;
  }
  for (int i=0;i<nofelements;i++) {
    if (elements[i].size()!=size_t(dimw+1))
      continue;
    double maxlen=0.0;
    int vtx1 = is1;
    int vtx2 = is2;
    if (vtx1==-1 || vtx2==-1)
      for (int l=0;l<dimw+1;l++) {
        int idxl=elements[i][l];
        for (int k=l+1;k<dimw+1;k++) {
          int idxk=elements[i][k];
          double len=pow(vtx[idxk][0]-vtx[idxl][0],2.);
          for (int p=1;p<dimw;p++)
            len+=pow(vtx[idxk][p]-vtx[idxl][p],2.);
          if (len>maxlen) {
            vtx1=l;
            vtx2=k;
            maxlen=len;
          }
        }
      }
    int swapped=0;
    if (vtx1!=use1) {
      int tmp=elements[i][vtx1];
      elements[i][vtx1]=elements[i][use1];
      elements[i][use1]=tmp;
      swapped++;
    }
    if (vtx2!=use2) {
      int tmp=elements[i][vtx2];
      elements[i][vtx2]=elements[i][use2];
      elements[i][use2]=tmp;
      swapped++;
    }
    if (swapped==1) {
      int tmp=elements[i][use2];
      elements[i][use2]=elements[i][use1];
      elements[i][use1]=tmp;
    }
  }
}
inline double 
DuneGridFormatParser:: testTriang(int snr) {
  double o = 
    (vtx[elements[snr][1]][0]-vtx[elements[snr][0]][0])*
    (vtx[elements[snr][2]][1]-vtx[elements[snr][1]][1])-
    (vtx[elements[snr][1]][1]-vtx[elements[snr][0]][1])*
    (vtx[elements[snr][2]][0]-vtx[elements[snr][1]][0]);
  if (fabs(o)<1e-10) {
    DUNE_THROW(DGFException,
               "Simplex number " << snr << " with vertex numbers "
               << "(" << elements[snr][0] 
               << "," << elements[snr][1] 
               << "," << elements[snr][2] << ")"
               << " has zero volume!");
  }
  return o;
}
}
