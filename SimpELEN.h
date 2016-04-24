#ifndef SimpELEN_H
#define SimpELEN_H
#include "Surface.h"
#include <omp.h>

class SimpELEN
{
public:

  //Attributes
  Surface* s;
  priority_queue<Edge> edge_queue;
  vector<double> currentEdgeCost;
  vector<pair<int,int>> currentEdgePoints;
  int* initial_vertices; //How many vertices are initially inside each uniformgrid cell

  vector<Point*>* cell; //Uniform grid cells
  priority_queue<Edge>* cell_queue; //Priority_queue for each cell in the grid

  int* edges_in;
  int* edges_full_in;
  int* edges_collapsable;

  long int time_collapsing;
  long int time_updating;
  long int time_skipping;
  long int time_resetting;
  long int time_clear_queue;
  long int time_iterating;

  int failed_pop;
  int failed_removed;
  int failed_cost;
  int edges_outdated;

  int grid_res; // grid has N x N x N cells
  int n_cells;
  double dim[3]; //dimension of the grid x,y,z
  int total_edges = 0;
  int nthreads; //Number of OpenMP threads to run

  //Methods
  SimpELEN(Surface*, int);

  //Operations
  void setPlacement(Edge* e); //Set placement vertex after collapsing edge e
  void updateEdgeCosts(Point* v, int); //Update edge costs for every edge of v
  void updateEdgeCosts(Point* v);//For serial run
  double getCost(Edge* e); //Updates the cost for edge e
  void initEdgeCosts(); //Compute initial costs and constructs edge_queue
  void simplify(int, int);
  void resetQueue();

  //UNIFORM GRID
  void initUniformGrid(int res);
  int getGridCell(Point* p);
  bool isCrownInCell(Point* p); //Checks whether p's crown (neighbours) is inside the same cell as p
  bool isEntirelyInCell(Edge* e);
};



#endif //SimpELEN_H
