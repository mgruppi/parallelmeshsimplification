#ifndef FUNCTIONS_H__
#define FUNCTIONS_H__
#include <vector>
#include <assert.h>
#include <set>
using namespace std;

class Point;
class Face;
class Edge;

typedef vector<Point*>::iterator point_vec_it;
typedef vector<Face*>::iterator face_vec_it;
typedef vector<Edge*>::iterator edge_vec_it;

class Point{
public:
  Point(int nid, double nx, double ny, double nz){
    id = nid;
    x = nx;
    y = ny;
    z = nz;
    removed = false;
  }

  bool operator==(Point& p)
  {
    if(id == p.id)
    {
      return true;
    }
    return false;
  }

  bool operator<(Point& p)
  {
      return (id < p.id);
  }

  void addFace(Face* f)
  {
    faces.push_back(f);
  }

  bool isValid() //Return whether the point has faces assign to it
  {
    return (faces.size() > 0);
  }

  int id;
  double x;
  double y;
  double z;
  bool removed;
  std::vector<Point*>::iterator pointer; //Iterator to the list of points (vertices)
  vector<Face*> faces;
  vector<Edge*> edges;
  vector<Edge*> to;
  vector<Edge*> from;
  vector<int> i_faces;
  vector<int> i_from;
  vector<int> i_to;
  double Q[4][4];

};

class Edge
{
public:
  int id;
  double cost;
  bool removed;
  Point* p1;
  Point* p2;
  Point* placement;
  std::vector<Edge*>::iterator pointer; //Iterator to list of edges
  std::vector<Face*> faces;

  Edge(Point* pa, Point* pb)
  {
    p1 = pa;
    p2 = pb;
    removed = false;
    placement = new Point(0, p2->x,p2->y,p2->z);

  }

  void addFace(Face* f)
  {
    assert(faces.size() < 2);
    faces.push_back(f);
  }

  bool operator==(Edge& e) const{
    if (p1 == e.p1 && p2 == e.p2)
    {
      return true;
    }
    else return false;
  }

  //An edge comes first if its cost is smaller
  bool operator<(const Edge e) const
  { //We want higher costs at the top of the priority_queue, that's why we invert operator <
    return cost > e.cost;
  }

};

class Face{
public:
  int id;
  vector<Point*> points;
  vector<int> f_points;
  std::vector<Face*>::iterator pointer; // Iterator to the list of faces
  bool removed;

  Face(int nid)
  {
    id = nid;
    removed = false;
  }

  bool operator==(Face& f)
  {
    if(id == f.id)
    {
      return true;
    }
    return false;
  }

  void addPoint(Point* p)
  {
    points.push_back(p);
  }

  bool isValid() //If face has vertices assigned to it
  {
    return (points.size() > 0);
  }

};


#endif
