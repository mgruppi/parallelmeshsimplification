#include "SimpVertexClustering.h"
#include <iostream>
using namespace std;

SimpVertexClustering::SimpVertexClustering(Surface* surf, int grid_dim)
{
  gdim = grid_dim;
  s = surf;
  dim[0] = s->bbox.getXLen()/grid_dim;
  dim[1] = s->bbox.getYLen()/grid_dim;
  dim[2] = s->bbox.getZLen()/grid_dim;
  cell = new vector<Point*>[gdim*gdim*gdim];
}
SimpVertexClustering::~SimpVertexClustering()
{
  delete cell;
}

void SimpVertexClustering::initCells()
{
  //Init cells
  for(point_vec_it pit = s->m_points.begin(); pit != s->m_points.end(); ++pit)
  {
    int cx = ((*pit)->x - s->bbox.minx)/dim[0]; //x
    int cy = ((*pit)->y - s->bbox.miny)/dim[1]; //y
    int cz = ((*pit)->z - s->bbox.minz)/dim[2]; //z
    //Resulting cell is cx + 3*cy + 9*cz
    int cellpos = cx + dim[0]*cy + dim[0]*dim[1]*cz;
    cell[cellpos].push_back(*pit);
  }
}

void SimpVertexClustering::setPlacement(Edge& e)
{
  double x = (e.p1->x + e.p2->x)/2;
  double y = (e.p1->y + e.p2->y)/2;
  double z = (e.p1->z + e.p2->z)/2;
  Point* p = new Point(0,x,y,z);

  e.placement = p;
}

void SimpVertexClustering::simplifyClusters()
{
  int clusters = 0;
  for(int i = 0 ; i < gdim*gdim*gdim; ++i)
  {
    if(cell[i].size()>=2)
    {
      clusters++;
      cerr << "Cluster " << i << ": " << cell[i].size() << " vertices.\n";
      for(point_vec_it pit = cell[i].begin(); pit != cell[i].end(); ++pit)
        cerr << (*pit)->id << " ";
      cerr << endl;
      for(point_vec_it pit = cell[i].begin(); pit != cell[i].end()-1; ++pit)
      {
        Edge e((*pit),*(pit+1));

        s->collapse(e);
      }
    }

    else continue;
  }
  cerr << "No. of clusters: " << clusters << endl;
}
