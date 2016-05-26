#include "SimpQEM.h"
#include <algorithm>

void initMatrix(int q[4][4])
{
  for(int i = 0 ; i < 4; ++i)
    for(int j = 0 ; j < 4; ++j)
      q[i][j]=0;
}

SimpQEM::SimpQEM(Surface* sur, int n)
: SimpELEN(sur,n)
{


}

void SimpQEM::simplify(int goal, int gridres=1)
{

  cerr << "Initializing edge costs.\n";
  time_updating = 0;
  time_collapsing = 0;
  time_skipping = 0;
  time_resetting = 0;
  time_iterating = 0;
  failed_pop = 0;
  failed_removed = 0;
  failed_cost = 0;
  edges_outdated = 0;
  unsigned long time_grid = 0;
  timespec t0,t1,t,tu,tu0,tu1;
  timespec tr0,tr1,tr; //time for resetting queue
  timespec tgrid0,tgrid1,tgrid;//Time for constructing grid
  clock_gettime(CLOCK_REALTIME, &t0);
  initQuadrics();
  initEdgeCosts();
  clock_gettime(CLOCK_REALTIME, &t1);
  t = diff(t0,t1);
  cout << greentty << "Time_init_edges: " << getMilliseconds(t) << deftty << endl;
  int vertices_removed = 0;
  cerr << orangetty<< "Target vertex count: " << s->m_points.size() - goal << deftty<< endl;


  clock_gettime(CLOCK_REALTIME, &t0);
  while(vertices_removed < goal)
  {
    clock_gettime(CLOCK_REALTIME,&tgrid0);
    initUniformGrid(gridres);
    clock_gettime(CLOCK_REALTIME,&tgrid1);
    tgrid = diff(tgrid0,tgrid1);
    time_grid += getNanoseconds(tgrid);
    cout << greentty << "Grid: " << gridres << endl;
    cout << lightcyantty << "Removed: " << vertices_removed << endl;
    cout << lightgreentty << "Time_init_grid: " << getMilliseconds(tgrid) << deftty << endl;


    omp_set_num_threads(nthreads);

    #pragma omp parallel for
    for(int i = 0; i < n_cells; ++i)
    {
      int vr = 0;
      if(cell[i].empty() || cell_queue[i].empty()) continue;

      //cerr << "Simplifying cell " << i << " - " << cell_queue[i].size() << " edges" <<  endl;

      while(vr < initial_vertices[i]/gridres && vertices_removed < goal && !cell_queue[i].empty())
      {

        Edge e = cell_queue[i].top();
        cell_queue[i].pop();

        //Skip edge if it's been removed or its cost has changed.
        if(s->is_edge_removed[e.id] || e.cost != currentEdgeCost[e.id] || e.p1->faces.empty() || e.p2->faces.empty())
        {
            //failed_pop++;
            continue;
        }

        double tempQ[4][4];
        copyQuadrics(tempQ,e.p1->Q);
        sumQuadrics(tempQ,e.p2->Q);
        bool collapsed = s->collapse(e);
        if(collapsed)
        {

          copyQuadrics(e.p2->Q,tempQ);
          vr++;
          clock_gettime(CLOCK_REALTIME,&tu0);
          updateEdgeCosts(e.p2, i);
          clock_gettime(CLOCK_REALTIME,&tu1);
          tu = diff(tu0,tu1);
          time_updating += getNanoseconds(tu);
          currentEdgeCost[e.id] = INF; // Edge has been removed
          #pragma omp atomic
          vertices_removed++;
        }
      }
      //cerr << "Vertices removed: " << vr << endl;
    }
    if(gridres>=2) gridres/=2;
  }

  clock_gettime(CLOCK_REALTIME, &t1);
  t = diff(t0,t1);


  cout << bluetty << "Time_simplify: " << getNanoseconds(t)/1000000 << deftty << endl;
  cout << bluetty << "Time_updating: " << time_updating/1000000 << deftty << endl;
  cout << yellowtty << "Time_get_error: " << time_error/1000000 << deftty << endl;
  cout << lightbluetty << "Time_quadrics: " << time_quadrics/1000000 << deftty << endl;
  cout << lightpurpletty << "Time_other: " << time_other/1000000 << deftty << endl;
  // cout << yellowtty << "Time_iterating: " << time_iterating/1000000 << deftty << endl;
  // cout << lightbluetty << "Time_collapsing: " << time_collapsing/1000000 << deftty << endl;
  // cout << lightpurpletty << "\tRemoving faces: " << s->time_faces/1000000 << deftty<<endl;
  // cout << lightpurpletty << "\tRemoving edges: " << s->time_edges/1000000 << deftty<<endl;
  // cout << lightpurpletty << "\tRemoving points: " << s->time_point/1000000 << deftty<<endl;
  // cout << redtty << "Time skipping: " << time_skipping/1000000 << deftty << endl;
  // cout << cyantty << "Time resetting queue: " << time_resetting/1000000 << deftty << endl;
  // cout << lightredtty << "Failed pops: " << failed_pop << " " << failed_removed << "(removed) - " << failed_cost << "(cost)" << deftty<<endl;
  // cout << lightredtty << "Failed collapses: " << s->failed_collapses << deftty << endl;
  cout << cyantty << "Left in queue: " << edge_queue.size() << deftty << endl;
}

void SimpQEM::initQuadrics()
{
  //Quadric Q (4x4 matrix) is the sum of all planes tangent to a vertex v (Garland, 97).
  //Get planes of every vertex faces
  for(point_vec_it pit = s->m_points.begin(); pit != s->m_points.end(); ++pit)
  {
    double Kp[4][4];
    //Get each faces plane
    for(face_vec_it fit = (*pit)->faces.begin(); fit != (*pit)->faces.end(); ++fit)
    {
      //Calculate vectors v0v1 and v0v2
      //v0v1
      double x = (*fit)->points[1]->x - (*fit)->points[0]->x;
      double y = (*fit)->points[1]->y - (*fit)->points[0]->y;
      double z = (*fit)->points[1]->z - (*fit)->points[0]->z;

      Vector3f* v0v1 = new Vector3f(x,y,z);

      //v0v2
      x = (*fit)->points[2]->x - (*fit)->points[0]->x;
      y = (*fit)->points[2]->y - (*fit)->points[0]->y;
      z = (*fit)->points[2]->z - (*fit)->points[0]->z;

      Vector3f* v0v2 = new Vector3f(x,y,z);
      Vector3f* vv = v0v1->cross(v0v2);
      vv->normalize();//Normalize so that x² + y² + z² = 1
      //Apply v0 to find parameter d of equation
      double d = vv->x*(*fit)->points[0]->x + vv->y*(*fit)->points[0]->y + vv->z*(*fit)->points[0]->z;
      double plane_eq[4] = {vv->x, vv->y, vv->z, d};

      //For this plane, the fundamental quadric Kp is the product of vectors plane_eq and plane_eq(transposed) (garland97)
      for(int i = 0; i < 4; ++i)
      {
          for(int j = 0 ; j < 4; ++j)
          {
              Kp[i][j] = plane_eq[i]*plane_eq[j];
          }
      }

      delete v0v1;
      delete v0v2;
      delete vv;

    }
    sumQuadrics((*pit)->Q,Kp);
  }
}

void SimpQEM::initEdgeCosts()
{
  cerr << "Init edges.\n";
  //Iterate over every face generating respective edges
  int eid = 0;

  //avoid duplicates by only creating edges such that v1->id < v2->id
  for(vector<Face*>::iterator it = s->m_faces.begin(); it != s->m_faces.end(); ++it)
  {
          std::vector<Point*> vec;
          vec.push_back((*it)->points[0]);
          vec.push_back((*it)->points[1]);
          vec.push_back((*it)->points[2]);
          std::sort(vec.begin(),vec.end());

          int v[3][2]= {{0,1},{0,2},{1,2}};

          //We'll have half-edges vec(0,1) vec(0,2) vec(1,2)
          //Half-edge (0,1)
          for(int i = 0 ; i < 3; ++i)
          {

            Edge* eaux = new Edge(vec[v[i][0]],vec[v[i][1]]);
            bool found_edge = false;
            for(edge_vec_it eit = eaux->p1->from.begin(); eit!=eaux->p1->from.end(); ++eit)
            {
              //cerr << eaux->p1->id << " - " << (*eit)->p1->id << endl;
              //cerr << eaux->p2->id << " , " << (*eit)->p2->id << endl;
              if((*eit)->p2->id == eaux->p2->id){
                found_edge = true;
                //cerr << "Found it!!\n";
                //Add this face to the edge
                (*eit)->addFace(*it);
                continue;
              }
            }
            if(!found_edge)
            {
              eaux->id = eid;
              eaux->cost = getCost(eaux);
              eaux->addFace(*it);
              edge_queue.push(*eaux);
              eaux->p1->from.push_back(eaux);
              eaux->p2->to.push_back(eaux);
              // eaux->p1->i_from.push_back(eaux->id);
              // eaux->p2->i_to.push_back(eaux->id);
              currentEdgeCost.push_back(eaux->cost);
              currentEdgePoints.push_back(pair<int,int>(eaux->p1->id,eaux->p2->id));
              s->m_edges.push_back(eaux);
              s->is_edge_removed.push_back(false);
              eid++;
            }
          }
  }
  total_edges = eid;
  cerr <<"Edges: " << total_edges << endl;
}

void SimpQEM::updateEdgeCosts(Point* v, int i)
{
  timespec t,t0,t1;

  //cout << "To|From: " << v->to.size() << "|" << v->from.size() << endl;
  for(vector<Edge*>::iterator eit = v->from.begin(); eit != v->from.end(); ++ eit)
  {
    gettime(t0);
    if(isEntirelyInCell(*eit) && isCrownInCell((*eit)->p1) && isCrownInCell((*eit)->p2))
    {
      //cerr << "Edge update " << (*eit)->id << " - " << (*eit)->p1->id << " " << (*eit)->p2->id << endl;

      (*eit)->cost = getCost((*eit));


      currentEdgeCost[(*eit)->id] = (*eit)->cost;
      //pair<int,int> pp((*eit)->p1->id,(*eit)->p2->id);
      //currentEdgePoints[(*eit)->id] = pp;
      cell_queue[i].push(*(*eit));
    }
    gettime(t1);
    t = diff (t0,t1);
    time_other+=getNanoseconds(t);

  }
  for(vector<Edge*>::iterator eit = v->to.begin(); eit != v->to.end(); ++ eit)
  {
    gettime(t0);
    if(isEntirelyInCell(*eit) && isCrownInCell((*eit)->p1) && isCrownInCell((*eit)->p2))
    {
      //cerr << "Edge update " << (*eit)->id << " - " << (*eit)->p1->id << " " << (*eit)->p2->id << endl;

      (*eit)->cost = getCost((*eit));


      currentEdgeCost[(*eit)->id] = (*eit)->cost;
      //pair<int,int> pp((*eit)->p1->id,(*eit)->p2->id);
      //currentEdgePoints[(*eit)->id] = pp;
      cell_queue[i].push(*(*eit));
    }
    gettime(t1);
    t = diff (t0,t1);
    time_other+=getNanoseconds(t);
  }
  //cout << "Update one edge: " << tcost << endl;

}


double SimpQEM::getError(double v[4], double Q[4][4])
{
  //timespec t,t0,t1;
  //gettime(t0);
  double vQ[4] = {0};
  double cost = 0;

  //vT(row vector) dot Q (4x4 matrix)
  for(int i = 0; i < 4; ++i)
  {
    for(int j = 0; j< 4 ; ++j)
    {
      vQ[i] += v[j]*Q[j][i];
    }
  }
  //vQ (row vector) dot v (column vector)
  for(int i = 0; i < 4; ++i)
  {
    cost+=vQ[i]*v[i];
  }
  // gettime(t1);
  // t = diff(t0,t1);
  // time_error+=getNanoseconds(t);
  return cost;
}

double SimpQEM::getCost(Point* p)
{
  double v[4] = {p->x, p->y, p->z, 1};
  double vQ[4] = {0};
  double cost = 0;

  return getError(v,p->Q);
}

double SimpQEM::getCost(Edge* e)
{
  timespec t,t0,t1;
  timespec te,te0,te1;
  gettime(t0);
  SimpELEN::setPlacement(e);
  //Error cost is given by vTQv quere v is placement vertex;
  copyQuadrics(e->placement->Q,e->p1->Q);
  sumQuadrics(e->placement->Q,e->p2->Q);
  gettime(t1);
  t = diff (t0,t1);
  time_quadrics+=getNanoseconds(t);

  gettime(te0);
  double c = getCost(e->placement);
  gettime(te1);
  te = diff(te0,te1);
  time_error += getNanoseconds(te);

  //cout << "Eid|Cost: " << e->id << "|"<<c<<endl;
  return c;

}
