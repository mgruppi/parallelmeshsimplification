#ifndef SIMPQEM_H__
#define SIMPQEM_H__

#include "SimpELEN.h"


class SimpQEM : public SimpELEN
{
public:


  long int time_error = 0;
  long int time_quadrics = 0;
  long int time_other = 0;

  //Methods
  SimpQEM(Surface*, int);

  //Operations
  void initQuadrics(); //Initialize quadric matrix for every vertex
  void initEdgeCosts();
  void simplify(int, int);
  void updateEdgeCosts(Point* v);
  void updateEdgeCosts(Point* v, int c); //Update costs for v in cell c

  double getCost(Edge* e);
  double getCost(Point* p);
  double getError(double v[4], double Q[4][4]);


};


#endif //SIMPQEM_H__
