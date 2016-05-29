#include "Surface.h"

class SimpVertexClustering
{
public:
  Surface* s;
  int gdim;
  double dim[3]; //Cell length of each dimension
  vector<Point*>* cell; //Array of cells

  SimpVertexClustering(Surface* surf, int grid_dim);
  ~SimpVertexClustering();
  //Operations
  void initCells();
  void simplifyClusters();
  void setPlacement(Edge& e);

};
