#ifndef __SURFACE_H__
#define __SURFACE_H__
#include <vector>
#include <list>
#include <string>
#include <queue>
#include <time.h>
//=====================
//Local
#include "Classes.h"
#include "Vector3f.h"
#include "common.h"
//=====================


#define METRIC_ELEN 1
#define METRIC_QEM 2

const int INF = 9999;
using namespace std;

struct BoundingBox
{
  double minx=INF, maxx=0;
  double miny=INF, maxy=0;
  double minz=INF, maxz=0;

  double getYLen()
  {
    return maxy - miny;
  }
  double getXLen()
  {
    return maxx - minx;
  }
  double getZLen()
  {
    return maxz - minz;
  }
};


class Surface
{

public:
  Surface(string inputFile);
  Surface(){}
  ~Surface();

  long int time_faces;
  long int time_edges;
  long int time_point;
  int failed_collapses;

  //General
  void readSurface(string inputFile);
  void printVertices();
  void printFaces();
  void saveOFF(string); //Save output file
  void dumpBoundingBox();

  //Mesh Operations
  void removePoint(Point* p); //Remove point p and all its faces from the surface
  void removeFace(Face* f); // Remove face f from the surface
  void removeEdge(Edge* e);
  void mergeVertices(Point* p1, Point* p2); //Merge p1 and p2. The resulting vertex is placed at p2's position. p1 is removed.
  bool collapse(Edge&);
  void simplify(float goal, const int);

  //ELen (Edge length) simplification operations
  void ELEN_computeEdgeCosts(); //Compute all edges costs and construct edge_queue
  void ELEN_updateEdgeCost(Point* v);
  double ELEN_computeCost(Edge* e);
  void ELEN_setPlacement(Edge* e); //Determine the placement vertex of an edge e


  //QEM simplification operations
  void initQuadrics();


  //Attributes
  BoundingBox bbox;
  vector<double> currentEdgeCost;
  priority_queue<Edge> edge_queue;
  int numVertices;
  int numFaces;
  vector<Point*> m_points;
  vector<Face*> m_faces;
  vector<Edge*> m_edges;
  vector<int> is_edge_removed;

  //file
  string input_path;


private:


};

#endif // __SURFACE_H__
