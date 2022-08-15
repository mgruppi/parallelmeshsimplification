#include <iostream>
#include <stdlib.h>
#include <string>
#include "Surface.h"
#include "SimpVertexClustering.h"
#include "SimpELEN.h"
#include "SimpQEM.h"

using namespace std;


int main(int argc, char** argv)
{
  if(argc < 6)
  {
    cerr << "*USAGE: Simplify <input file> <fraction of points to remove> <method (elen/qem/vc)> <grid_resolution> <no of threads>.\n";
    exit(1);
  }

  string method = argv[3];

  if(method != "elen" && method != "qem" && method != "vc")
  {
    cerr << "ERROR: Invalid decimation method.\n";
    exit(1);
  }

  Surface* s = new Surface(argv[1]);
  float goal = atof(argv[2]);
  int gridresolution = atoi(argv[4]);
  int nthreads = atoi(argv[5]);




//  Vector3f v1(1,0,0);
//  Vector3f v2(0,1,0);
//  Vector3f v3(0,0,1);
//
//  Vector3f v4 = v1.cross(v2);
//
//  cerr << "v1 x v2 = (" << v4.x << "," << v4.y << "," << v4.z << ")\n";
//  cerr << "v1 . v2 = " << v1.dot(v2) << endl;
//
//  Vector3f v5 = v3.cross(v4);
//  cerr << "v3 x v4 = (" << v5.x << "," << v5.y << "," << v5.z << ")\n";




  if(method == "elen")
  {
    method = "ELEN";
    int goal_vertices = goal*s->m_points.size();
    SimpELEN* elen = new SimpELEN(s, nthreads);
    //elen->initUniformGrid(gridresolution);
    //elen->initEdgeCosts();
    elen->simplify(goal_vertices,gridresolution);
  }
  else if (method == "qem")
  {
    method = "QEM";
    int goal_vertices = goal*s->m_points.size();
    SimpQEM* qem = new SimpQEM(s,nthreads);
    qem->simplify(goal_vertices,gridresolution);
  }
  else if (method == "vc")
  {
    method = "VCLUSTERING";
    cout << "Vertex Clustering not available yet.\n";
    exit(1);
    //SimpVertexClustering* vc = new SimpVertexClustering(s, 5);
    //vc->initCells();
    //vc->simplifyClusters();
  }


  //Vertex Clustering
 // SimpVertexClustering* vc = new SimpVertexClustering(s, 10);
 // vc->initCells();
 // vc->simplifyClusters();



  string sa(argv[1]);
  string sub = sa.substr(0, sa.length()-4);
  int percentage = goal*100;
  string qtd = to_string(percentage);
  string output = sub+qtd+"_"+method+".off";
  cerr << "Writing output to " + output <<endl;
  s->saveOFF(output);
  //s->dumpBoundingBox();

  cerr << redtty << "Bounding box(min): " << s->bbox.minx << " " << s->bbox.miny << " " << s->bbox.minz << endl;
  cerr << redtty << "Bounding box(max): " << s->bbox.maxx << " " << s->bbox.maxy << " " << s->bbox.maxz << endl;

  cerr << "Length: " << s->bbox.getXLen() << " " << s->bbox.getYLen() << " " << s->bbox.getZLen() << endl;
  cerr << deftty;

  //delete vc;
  delete s;

  return 0;
}
