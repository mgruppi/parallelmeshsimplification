#ifndef SIMPQEM_H__
#define SIMPQEM_H__

#include "SimpELEN.h"


class SimpQEM : public SimpELEN
{
public:


  //Methods
  SimpQEM(Surface*, int);

  //Operations
  void initQuadrics(); //Initialize quadric matrix for every vertex
  void initEdgeCosts();
  void simplify(int, int);
  void updateEdgeCosts(Point* v);
  double getCost(Edge* e);


};


#endif //SIMPQEM_H__
