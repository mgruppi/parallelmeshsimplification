#include "Surface.h"
#include <fstream>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cmath>

//CGAL includes
#include <CGAL/Simple_cartesian.h>
#include<CGAL/intersections.h>

typedef CGAL::Simple_cartesian<double> K;
typedef K::Point_3 Point3;
typedef K::Triangle_3 Triangle3;


Surface::Surface(string inputFile)
{

  time_faces = 0;
  time_edges = 0;
  time_point=0;
  failed_collapses = 0;
  timespec t0, t1, t;
  clock_gettime(CLOCK_REALTIME, &t0);
  readSurface(inputFile);
  clock_gettime(CLOCK_REALTIME, &t1);
  t = diff(t0,t1);
  cout << greentty << "Time_read_input_and_init_data: " << t.tv_sec*1000 + t.tv_nsec/1000000 << deftty << endl;
}

Surface::~Surface()
{

}

void Surface::readSurface(string inputFile)
{

  input_path = inputFile;

  FILE *fin = fopen(inputFile.c_str(), "r");


  int success;
  //Read OFF word
  char buffer[256];
  success = fscanf(fin, "%s\n", buffer);

  if(success == EOF)
  {
    cerr << "Could not read input file.\n";
    exit(1);
  }

  int edge_zero; //Number of edges always 0, just read it from file
  success = fscanf(fin, "%d %d %d", &numVertices, &numFaces, &edge_zero);
  if(success == EOF)
  {
    cerr << "Could not read input file.\n";
    exit(1);
  }

  //Reading vertices

  for(int i = 0; i < numVertices; i++)
  {
    double xin;
    double yin;
    double zin;
    success = fscanf(fin, "%lf %lf %lf", &xin, &yin, &zin);
    if(success == EOF)
    {
      cerr << "Could not read input file.\n";
      exit(1);
    }
    Point* aux = new Point(i, xin, yin, zin);

    //Set bounding box
    bbox.minx = std::min(xin,bbox.minx);
    bbox.maxx = std::max(xin,bbox.maxx);
    bbox.miny = std::min(yin,bbox.miny);
    bbox.maxy = std::max(yin,bbox.maxy);
    bbox.minz = std::min(zin,bbox.minz);
    bbox.maxz = std::max(zin,bbox.maxz);

    m_points.push_back(aux);
    aux->pointer = m_points.end();

  }

  //Set vertex pointers
  for(vector<Point*>::iterator it = m_points.begin(); it != m_points.end(); ++it)
  {
    (*it)->pointer = it;
  }
  cerr << m_points.size() << " vertices read.\n";



  //Faces
  for(int i=0; i<numFaces;++i)
  {

   int vertices, v1, v2, v3;
   success = fscanf(fin, "%d %d %d %d", &vertices, &v1,&v2,&v3);
   if(success == EOF)
   {
     cerr << "Could not read input file.\n";
     exit(1);
   }
   Face* faux = new Face(i);
   m_faces.push_back(faux);
   faux->addPoint(m_points.at(v1));
   faux->addPoint(m_points.at(v2));
   faux->addPoint(m_points.at(v3));



   //Add face to Point list of faces
   m_points.at(v1)->addFace(faux);
   m_points.at(v2)->addFace(faux);
   m_points.at(v3)->addFace(faux);

  }
  //Set pointer to vector positions
  for(vector<Face*>::iterator it = m_faces.begin(); it != m_faces.end(); ++it)
  {
    (*it)->pointer = it;
  }

  cerr << m_faces.size() << " faces read.\n";

  //Generate edges

  fclose(fin);
}

void Surface::dumpBoundingBox()
{
  ofstream fout("bbox.off");

  fout << "OFF\n";
  fout << 8 << " " << 6 << " 0\n";
  fout << fixed << setprecision(6);

  //Vertices
  fout << bbox.minx << " " << bbox.miny << " " << bbox.minz << endl;
  fout << bbox.minx + bbox.getXLen() << " " << bbox.miny << " " << bbox.minz << endl;
  fout << bbox.minx + bbox.getXLen() << " " << bbox.miny + bbox.getYLen() << " " << bbox.minz << endl;
  fout << bbox.minx << " " << bbox.miny + bbox.getYLen() << " " << bbox.minz << endl;

  fout << bbox.minx << " " << bbox.miny << " " << bbox.minz + bbox.getZLen() << endl;
  fout << bbox.minx + bbox.getXLen() << " " << bbox.miny << " " << bbox.minz + bbox.getZLen()  << endl;
  fout << bbox.minx + bbox.getXLen() << " " << bbox.miny + bbox.getYLen() << " " << bbox.minz + bbox.getZLen()  << endl;
  fout << bbox.minx << " " << bbox.miny + bbox.getYLen() << " " << bbox.minz + bbox.getZLen()  << endl;

  //Faces
  fout << "4 0 1 2 3\n";
  fout << "4 1 5 6 2\n";
  fout << "4 4 5 6 7\n";
  fout << "4 4 0 3 7\n";
  fout << "4 3 2 6 7\n";
  fout << "4 1 5 6 2\n";

  fout.close();



}

void Surface::saveOFF(string output)
{
  ofstream fout(output.c_str());


  fout << "OFF\n";
  //Include 0 in the end for edges
  //cerr << "Output: " << valid_points.size() << " vertices - " << valid_faces.size() << " faces. \n";
  //fout << valid_points.size() << " " << valid_faces.size() << " 0\n";
  point_vec_it pend = remove_if(m_points.begin(), m_points.end(),[](const Point* p) {return (p->removed || p->faces.empty());});//lambda
  m_points.erase(pend, m_points.end());

  face_vec_it fend = remove_if(m_faces.begin(),m_faces.end(),[](const Face* f){return f->removed;});//lambda function
  m_faces.erase(fend,m_faces.end());
  cerr << "Output: " << m_points.size() << " vertices - " << m_faces.size() << " faces.\n";

  fout << m_points.size() << " " << m_faces.size() << " 0" << endl;
 for(unsigned int i = 0 ; i < m_points.size(); ++i)
  {
    if(m_points[i]->faces.size() == 0 && !m_points[i]->removed)
    {
        cerr << "WARNING: Empty vertex: " << m_points[i]->id << endl;
        //m_points.erase(m_points.begin()+i);
        //i--;
        //continue;

    }
    m_points.at(i)->id = i;
  }

  //Write points
  for(std::vector<Point*>::iterator it = m_points.begin(); it != m_points.end(); ++it)
  {
    fout << fixed << setprecision(6) << (*it)->x << " " << (*it)->y << " " << (*it)->z << endl;
  }


  //cerr << "ok\n";
  //Write faces
  for(std::vector<Face*>::iterator it = m_faces.begin(); it!= m_faces.end(); ++it)
  {
    //cerr << "f " << (*it)->id << " " << (*it)->points.size() << endl;
    fout << "3" << " " << (*it)->points.at(0)->id << " " << (*it)->points.at(1)->id << " " << (*it)->points.at(2)->id << endl;
  }

  fout.close();
}

void Surface::printVertices()
{
  cout << "Printing list of vertices: \n";
  std::vector<Point*>::iterator it = m_points.begin();
  while(it!=m_points.end())
  {
    cout << (*it)->x << " " << (*it)->y << " " << (*it)->z << endl;
    it++;
  }
}

void Surface::printFaces()
{
  cout << "Printing list of faces: \n";
  for(std::vector<Face*>::iterator it = m_faces.begin(); it!=m_faces.end(); ++it)
  {
    cout << (*it)->points.at(0) << " " << (*it)->points.at(1) << " " << (*it)->points.at(2) << endl;
  }
}

void Surface::removePoint(Point* p)
{
  //Remove vertex
  p->faces.clear();
  p->removed = true;
  // vector<Point*>::iterator pit;
  // //(*pit)->pointer = pit;
  // for(pit=m_points.erase(p->pointer); pit != m_points.end(); ++pit)
  // {
  //   (*pit)->pointer = pit;
  // }
}

void Surface::removeFace(Face* f)
{

//  cerr << "removing face: " << f->id << endl;
//  cerr << "Points: ";
//  for(int j = 0 ; j < f->points.size(); ++j){
//    cerr << f->points[j]->id << " ";
//  }
//  cerr <<endl;

  //Remove f from the list of faces of every vertex in f
//  int i = 0;

if(f->removed)
{
  //Face has been removed by another edge
  return;
}

  for(vector<Point*>::iterator it= f->points.begin(); it != f->points.end(); ++it)
  {


      //cerr<< "p " << (*it)->id << endl;
      for(vector<Face*>::iterator it2 = (*it)->faces.begin(); it2 != (*it)->faces.end(); ++ it2)
      {

        if((*it2) == f)
        {
          //cerr << "removed from " << (*it)->id << endl;


          (*it)->faces.erase(it2);
          break;
        }
      }

  }

  //Remove face from general list of faces

  // vector<Face*>::iterator fit;
  // //Update pointers
  // for(fit = m_faces.erase(f->pointer); fit != m_faces.end(); ++fit)
  // {
  //   (*fit)->pointer = fit;
  // }

  f->removed = true;

}

void Surface::removeEdge(Edge* e)
{

  if(is_edge_removed[e->id]) return;

  edge_vec_it eit = e->p1->from.begin();
  while(eit != e->p1->from.end())
  {
      if((*eit)->id == e->id)
      {
          eit = e->p1->from.erase(eit);
      }
      else eit++;
  }

  eit = e->p2->to.begin();
  while(eit != e->p2->to.end())
  {
    if((*eit)->id == e->id)
    {
      eit = e->p2->to.erase(eit);
    }
    else eit++;
  }

  is_edge_removed[e->id] = true;
}


bool Surface::collapse(Edge& e)
{

    timespec t0,t1,t;
    clock_gettime(CLOCK_REALTIME,&t0);
    Point* p1 = e.p1;
    Point* p2 = e.p2;

    assert(p1);
    assert(p2);



    if(p1->faces.size() ==0 || p2->faces.size()==0) //Do not simplify boundary corners
    {
        //cerr << "*ERROR: EMPTY VERTEX.\n";
        failed_collapses++;
        //cout << redtty << "failed.\n" << deftty;
        return false;
    }

    //Iterating faces
  vector<Face*> for_removal;
  //clock_gettime(CLOCK_REALTIME,&t0);
    for(vector<Face*>::iterator fit = p1->faces.begin(); fit!=p1->faces.end(); ++fit)
    {
        bool removeface = false;
        vector<Point*>::iterator auxp1;
        for(vector<Point*>::iterator pit = (*fit)->points.begin(); pit != (*fit)->points.end() ; ++pit)
        {
            if((*pit) == p2)
            {
                removeface = true;
                break;
            }
            else if ((*pit) == p1)
            {
                auxp1 = pit;
            }
        }
        if(removeface) //p1 and p2 share face, remove it.
        {
            for_removal.push_back((*fit));
        }
        else //Swap p1 for p2 for this face
        {
            //Face is about to be modified. Checking intersection.
            Point3 p30((*fit)->points[0]->x,(*fit)->points[0]->y,(*fit)->points[0]->z);
            Point3 p31((*fit)->points[1]->x,(*fit)->points[1]->y,(*fit)->points[1]->z);
            Point3 p32((*fit)->points[2]->x,(*fit)->points[2]->y,(*fit)->points[2]->z);

            Triangle3 face(p30,p31,p32);

            for(face_vec_it fit2 = m_faces.begin(); fit2 != m_faces.end(); ++fit2)
            {
              Face* f = (*fit2);
              Point3 pf0(f->points[0]->x,f->points[0]->y,f->points[0]->z);
              Point3 pf1(f->points[1]->x,f->points[1]->y,f->points[1]->z);
              Point3 pf2(f->points[2]->x,f->points[2]->y,f->points[2]->z);
              Triangle3 face2(pf0,pf1,pf2);

              if(CGAL::do_intersect(face,face2))
              {
                cerr << "***Faces " << (*fit)->id << " X " << f->id << endl;
              }
            }

            (*auxp1) = p2;
            p2->faces.push_back((*fit));
        }
    }



    //cerr << "Removing faces: ";
    for(vector<Face*>::iterator it = for_removal.begin(); it != for_removal.end(); ++it)
    {
      //cerr << (*it)->id << " ";
       removeFace(*it);
       (*it)->removed = true;
       //delete (*it);
    }

    //Set position of p2 as midpoint between p1-p2
    p2->x = e.placement->x;
    p2->y = e.placement->y;
    p2->z = e.placement->z;


    clock_gettime(CLOCK_REALTIME,&t1);
    t = diff(t0,t1);
    time_faces+= getNanoseconds(t);


    //TODO: more efficient to use std::remove_if
    clock_gettime(CLOCK_REALTIME,&t0);
    removeEdge(m_edges[e.id]);

    vector<Edge*> edges_to_remove;
    edge_vec_it eit = p1->to.begin();
    //Remove double edges to
    while(eit != p1->to.end())
    {
      Edge* e = (*eit);
      bool found = false;
      edge_vec_it it = p2->to.begin();
      while(it != p2->to.end())
      {
        Edge* e2 = (*it);
        if(e->p1->id == e2->p1->id)
        {
          //cerr << "Found " << e->id << " " << e->p1->id << " " << e->p2->id << endl;
          edges_to_remove.push_back(e);
          found = true;
          break;
        }

        it++;
      }
      if(!found)
      {
        e->p2 = p2;
        if(e->p1->id == e->p2->id)
          edges_to_remove.push_back(e);
      }
      eit++;
    }

    //Remove double edges from
    eit = p1->from.begin();
    while(eit!=p1->from.end())
    {
      Edge* e = (*eit);
      bool found = false;
      edge_vec_it it = p2->from.begin();
      while(it != p2->from.end())
      {
        Edge* e2 = (*it);
        if(e->p2->id == e2->p2->id)
        {
          //cerr << "Found from " << e->id << " " << e->p1->id << " " <<e->p2->id << endl;
          edges_to_remove.push_back(e);
          found =true;
          break;
        }
        it++;
      }
      if(!found)
      {
        e->p1=p2;
        if(e->p1->id == e->p2->id)
          edges_to_remove.push_back(e);
      }
      eit++;
    }

    eit = edges_to_remove.begin();
    while(eit != edges_to_remove.end())
    {
      removeEdge(*eit);
      eit++;
    }

    //Append p1 edges to p2
    p2->to.insert(p2->to.end(), p1->to.begin(), p1->to.end());
    p2->from.insert(p2->from.end(),p1->from.begin(), p1->from.end());

    clock_gettime(CLOCK_REALTIME,&t1);
    t = diff(t0,t1);
    time_edges += getNanoseconds(t);

    //Can remove point p1
    clock_gettime(CLOCK_REALTIME,&t0);
    removePoint(p1);
    clock_gettime(CLOCK_REALTIME,&t1);
    t = diff(t0,t1);
    time_point+=getNanoseconds(t);

    return true;
}


//This should be applied to the placement vertex of an edge
//p quadric (Q) must be the sum of the endpoints of the edge
//and p is the placement vertex
double getCost(Point* p)
{
  double v[4] = {p->x, p->y, p->z, 1};
  double vQ[4] = {0};
  double vQv[4] = {0};
  double cost = 0;
  //v(row) dot Q
  for(int i = 0; i < 4; ++i)
  {
    for(int j = 0; j< 4 ; ++j)
    {
      vQ[i] += v[i]*p->Q[j][i];
    }
  }
  for(int i = 0; i < 4; ++i)
  {
    cost+=vQ[i]*v[i];
  }
  return cost;
}


//Compute the initial quadrics for every vertex
void Surface::initQuadrics()
{

    //Quadric Q (4x4 matrix) is the sum of all planes tangent to a vertex v (Garland, 97).
    for(point_vec_it pit = m_points.begin(); pit != m_points.end(); ++pit)
    {

        double Kp[4][4];
        for(face_vec_it fit = (*pit)->faces.begin(); fit!= (*pit)->faces.end(); ++fit)
        {
            // //Calculate vectors v0v1 and v0v2
            // //v0v1
            // double x = (*fit)->points[1]->x - (*fit)->points[0]->x;
            // double y = (*fit)->points[1]->y - (*fit)->points[0]->y;
            // double z = (*fit)->points[1]->z - (*fit)->points[0]->z;
            // Vector3f v0v1(x,y,z);
            //
            // //v0v2
            // x = (*fit)->points[2]->x - (*fit)->points[0]->x;
            // y = (*fit)->points[2]->y - (*fit)->points[0]->y;
            // z = (*fit)->points[2]->z - (*fit)->points[0]->z;
            //
            // Vector3f v0v2(x,y,z);
            // Vector3f* vv = v0v1.cross(v0v2);
            // vv->normalize(); //Normalize so that x� + y� + z� = 1
            // //Apply v0 to find parameter d of equation
            // double d = vv->x*(*fit)->points[0]->x + vv->y*(*fit)->points[0]->y + vv->z*(*fit)->points[0]->z;
            // double plane_eq[4] = {vv->x, vv->y, vv->z, d};
            //
            // //For this plane, the fundamental quadric Kp is the dot product of plane_eq and plane_eq(transposed) (garland97)
            // for(int i = 0; i < 4; ++i)
            // {
            //     for(int j = 0 ; j < 4; ++j)
            //     {
            //         Kp[i][j] = plane_eq[i]*plane_eq[j];
            //     }
            // }
            //
            // delete vv;
        }
        sumQuadrics((*pit)->Q,Kp);
    }

}


void Surface::simplify(float goal, const int METRIC)
{
    int goal_vertices = goal*m_points.size();
    int simplified = 0;
    cerr << "Starting simplification...\n";
    cerr << "Vertices to remove: " << redtty<<goal_vertices;
}
