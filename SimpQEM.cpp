#include "SimpQEM.h"


SimpQEM::SimpQEM(Surface* sur, int n)
: SimpELEN(sur,n)
{
}

void SimpQEM::simplify(int goal, int gridres=1)
{

  SimpELEN::simplify(goal,gridres);
}

void SimpQEM::initQuadrics()
{

}

void SimpQEM::initEdgeCosts()
{

}

void SimpQEM::updateEdgeCosts(Point* v)
{

}

double getCost(Edge* e)
{

}
