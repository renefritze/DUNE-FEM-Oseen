#ifndef DUNE_UG_TESTGRIDS_HH
#define DUNE_UG_TESTGRIDS_HH

/** \file
    \author Oliver Sander
    \brief Provides C++ code that create to UGGrids suitable for unit tests.

    We need to provide code because currently the only reader that can read
    general UGGrids is the AmiraMesh reader.  However, that library is still
    not easily available.
*/

#include <dune/grid/uggrid.hh>

namespace Dune {

    void make2DTestGrid(Dune::UGGrid<2>& grid)
    {
        // Start grid creation
        grid.createBegin();
        
        // The list of grid vertex positions
        int numVertices = 16;
        
        double vertices[16][2] = {{0, 0},
                                  {0.5, 0},
                                  {0.5, 0.5},
                                  {0, 0.5},
                                  {0.25, 0},
                                  {0.5, 0.25},
                                  {0.25, 0.5},
                                  {0, 0.25},
                                  {0.25, 0.25},
                                  {1, 0},
                                  {1, 0.5},
                                  {0.75, 0.25},
                                  {1, 1},
                                  {0.5, 1},
                                  {0, 1},
                                  {0.25, 0.75}};
        
        // Create the grid vertices
        for (int i=0; i<numVertices; i++) {
            Dune::FieldVector<double,2> pos;
            pos[0] = vertices[i][0];
            pos[1] = vertices[i][1];
            grid.insertVertex(pos);
        }
        
        // Create the triangle elements
        int numTriangles = 2;
        unsigned int triangles[2][3] = {{9, 10, 11},
                                        {15, 13, 14}};
        
        for (int i=0; i<numTriangles; i++) {
            std::vector<unsigned int> cornerIDs(3);
            for (int j=0; j<3; j++)
                cornerIDs[j] = triangles[i][j];
            grid.insertElement(Dune::GeometryType(Dune::GeometryType::simplex,2),cornerIDs);
        }
        
        // Create the quadrilateral elements
        int numQuadrilaterals = 9;
        unsigned int quadrilaterals[9][4] = {{0, 4, 7, 8},
                                             {4, 1, 8, 5},
                                             {8, 5, 6, 2},
                                             {7, 8, 3, 6},
                                             {1, 9, 5, 11},
                                             {5, 11, 2, 10},
                                             {2, 10, 13, 12},
                                             {3, 6, 14, 15},
                                             {6, 2, 15, 13}};
        
        for (int i=0; i<numQuadrilaterals; i++) {
            std::vector<unsigned int> cornerIDs(4);
            for (int j=0; j<4; j++)
                cornerIDs[j] = quadrilaterals[i][j];
            grid.insertElement(Dune::GeometryType(Dune::GeometryType::cube,2),cornerIDs);
        }
        
        // Finish initialization
        grid.createEnd();
    }
    
    
    void make3DTestGrid(Dune::UGGrid<3>& grid)
    {
        // Start grid creation
        grid.createBegin();
        
        // The list of grid vertex positions
        int numVertices = 61;
        
        double vertices[61][3] = {{0, 0, 0},
                                  {0.5, 0, 0},
                                  {0.5, 0.5, 0},
                                  {0, 0.5, 0},
                                  {0, 0, 0.5},
                                  {0.5, 0, 0.5},
                                  {0.5, 0.5, 0.5},
                                  {0, 0.5, 0.5},
                                  {0.25, 0, 0},
                                  {0.5, 0.25, 0},
                                  {0.25, 0.5, 0},
                                  {0, 0.25, 0},
                                  {0, 0, 0.25},
                                  {0.5, 0, 0.25},
                                  {0.5, 0.5, 0.25},
                                  {0, 0.5, 0.25},
                                  {0.25, 0, 0.5},
                                  {0.5, 0.25, 0.5},
                                  {0.25, 0.5, 0.5},
                                  {0, 0.25, 0.5},
                                  {0.25, 0.25, 0},
                                  {0.25, 0, 0.25},
                                  {0.5, 0.25, 0.25},
                                  {0.25, 0.5, 0.25},
                                  {0, 0.25, 0.25},
                                  {0.25, 0.25, 0.5},
                                  {0.25, 0.25, 0.25},
                                  {1, 0, 0},
                                  {1, 0.5, 0},
                                  {1, 0, 0.5},
                                  {1, 0.5, 0.5},
                                  {0.75, 0.25, 0.25},
                                  {1, 1, 0},
                                  {0.5, 1, 0},
                                  {1, 1, 0.5},
                                  {0.5, 1, 0.5},
                                  {0.75, 0.75, 0.25},
                                  {0, 1, 0},
                                  {0, 1, 0.5},
                                  {0.25, 0.75, 0.25},
                                  {0, 0, 1},
                                  {0.5, 0, 1},
                                  {0.5, 0.5, 1},
                                  {0, 0.5, 1},
                                  {0.25, 0.25, 0.75},
                                  {1, 0, 1},
                                  {1, 0.5, 1},
                                  {0.75, 0.25, 0.75},
                                  {1, 1, 1},
                                  {0.5, 1, 1},
                                  {0, 1, 1},
                                  {0.25, 0.75, 0.75},
                                  {1.5, 0, 0},
                                  {1.5, 0.5, 0},
                                  {1.5, 1, 0},
                                  {1.5, 0, 0.5},
                                  {1.5, 0.5, 0.5},
                                  {1.5, 1, 0.5},
                                  {1.5, 0, 1},
                                  {1.5, 0.5, 1},
                                  {1.5, 1, 1}};
        
        // Create the grid vertices
        for (int i=0; i<numVertices; i++) {
            Dune::FieldVector<double,3> pos;
            for (int j=0; j<3; j++)
                pos[j] = vertices[i][j];
            grid.insertVertex(pos);
        }
        
        
        
        // Create the tetrahedron elements
        int numTetrahedra = 54;
        unsigned int tetrahedra[54][4] = {{10, 29, 3, 32},
                                          {10, 2, 28, 32},
                                          {10, 28, 29, 32},
                                          {14, 28, 2, 32},
                                          {14, 6, 30, 32},
                                          {14, 30, 28, 32},
                                          {15, 31, 7, 32},
                                          {15, 3, 29, 32},
                                          {15, 29, 31, 32},
                                          {18, 30, 6, 32},
                                          {18, 7, 31, 32},
                                          {18, 31, 30, 32},
                                          {15, 29, 3, 37},
                                          {15, 7, 31, 37},
                                          {15, 31, 29, 37},
                                          {15, 36, 7, 37},
                                          {15, 3, 34, 37},
                                          {15, 34, 36, 37},
                                          {11, 38, 4, 40},
                                          {11, 3, 34, 40},
                                          {11, 34, 38, 40},
                                          {15, 34, 3, 40},
                                          {15, 7, 36, 40},
                                          {15, 36, 34, 40},
                                          {16, 39, 8, 40},
                                          {16, 4, 38, 40},
                                          {16, 38, 39, 40},
                                          {19, 36, 7, 40},
                                          {19, 8, 39, 40},
                                          {19, 39, 36, 40},
                                          {17, 42, 6, 45},
                                          {17, 5, 41, 45},
                                          {17, 41, 42, 45},
                                          {18, 43, 7, 45},
                                          {18, 6, 42, 45},
                                          {18, 42, 43, 45},
                                          {19, 44, 8, 45},
                                          {19, 7, 43, 45},
                                          {19, 43, 44, 45},
                                          {20, 41, 5, 45},
                                          {20, 8, 44, 45},
                                          {20, 44, 41, 45},
                                          {18, 31, 7, 48},
                                          {18, 6, 30, 48},
                                          {18, 30, 31, 48},
                                          {18, 42, 6, 48},
                                          {18, 7, 43, 48},
                                          {18, 43, 42, 48},
                                          {19, 39, 8, 52},
                                          {19, 7, 36, 52},
                                          {19, 36, 39, 52},
                                          {19, 43, 7, 52},
                                          {19, 8, 44, 52},
                                          {19, 44, 43, 52}};
        
        for (int i=0; i<numTetrahedra; i++) {
            std::vector<unsigned int> cornerIDs(4);
            for (int j=0; j<4; j++)
                cornerIDs[j] = tetrahedra[i][j]-1;
            grid.insertElement(Dune::GeometryType(Dune::GeometryType::simplex,3),cornerIDs);
        }
        
        // Create the pyramid elements
        int numPyramids = 27;
        unsigned int pyramids[27][5] = {{28, 30, 31, 29, 32},
                                        {10, 23, 14, 2, 32},
                                        {14, 23, 18, 6, 32},
                                        {18, 23, 15, 7, 32},
                                        {15, 23, 10, 3, 32},
                                        {3, 29, 33, 34, 37},
                                        {29, 31, 35, 33, 37},
                                        {33, 35, 36, 34, 37},
                                        {7, 36, 35, 31, 37},
                                        {11, 24, 15, 3, 40},
                                        {15, 24, 19, 7, 40},
                                        {19, 24, 16, 8, 40},
                                        {16, 24, 11, 4, 40},
                                        {34, 36, 39, 38, 40},
                                        {20, 26, 19, 8, 45},
                                        {19, 26, 18, 7, 45},
                                        {18, 26, 17, 6, 45},
                                        {17, 26, 20, 5, 45},
                                        {41, 44, 43, 42, 45},
                                        {6, 42, 46, 30, 48},
                                        {30, 46, 47, 31, 48},
                                        {31, 47, 43, 7, 48},
                                        {42, 43, 47, 46, 48},
                                        {7, 43, 50, 36, 52},
                                        {36, 50, 51, 39, 52},
                                        {39, 51, 44, 8, 52},
                                        {44, 51, 50, 43, 52}};
        
        for (int i=0; i<numPyramids; i++) {
            std::vector<unsigned int> cornerIDs(5);
            for (int j=0; j<5; j++)
                cornerIDs[j] = pyramids[i][j]-1;
            grid.insertElement(Dune::GeometryType(Dune::GeometryType::pyramid,3),cornerIDs);
        }
        
        // Create the prism elements
        int numPrisms = 8;
        unsigned int prisms[8][6] = {{28, 53, 29, 30, 56, 31},
                                     {53, 54, 29, 56, 57, 31},
                                     {30, 56, 31, 46, 59, 47},
                                     {56, 57, 31, 59, 60, 47},
                                     {29, 54, 33, 31, 57, 35},
                                     {54, 55, 33, 57, 58, 35},
                                     {31, 57, 35, 47, 60, 49},
                                     {57, 58, 35, 60, 61, 49}};
        
        
        for (int i=0; i<numPrisms; i++) {
            std::vector<unsigned int> cornerIDs(6);
            for (int j=0; j<6; j++)
                cornerIDs[j] = prisms[i][j]-1;
            grid.insertElement(Dune::GeometryType(Dune::GeometryType::prism,3),cornerIDs);
        }
        
        // Create the hexahedron elements
        int numHexahedra = 9;
        unsigned int hexahedra[9][8] = {{1, 9, 12, 21, 13, 22, 25, 27},
                                        {9, 2, 21, 10, 22, 14, 27, 23},
                                        {21, 10, 11, 3, 27, 23, 24, 15},
                                        {12, 21, 4, 11, 25, 27, 16, 24},
                                        {13, 22, 25, 27, 5, 17, 20, 26},
                                        {22, 14, 27, 23, 17, 6, 26, 18},
                                        {27, 23, 24, 15, 26, 18, 19, 7},
                                        {25, 27, 16, 24, 20, 26, 8, 19},
                                        {7, 31, 36, 35, 43, 47, 50, 49}};
        
        
        for (int i=0; i<numHexahedra; i++) {
            std::vector<unsigned int> cornerIDs(8);
            for (int j=0; j<8; j++)
                cornerIDs[j] = hexahedra[i][j]-1;
            grid.insertElement(Dune::GeometryType(Dune::GeometryType::cube,3),cornerIDs);
        }
        
        // Finish initialization
        grid.createEnd();
    }
    
}

#endif
