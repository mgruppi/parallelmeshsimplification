#include "SimpELEN.h"
#include <iostream>
#include <fstream>
#include <time.h>
#include <algorithm>
#include <assert.h>

double dist(Point* a, Point* b)
{
  return (a->x - b->x)*(a->x - b->x) + (a->y - b->y)*(a->y - b->y) + (a->z - b->z)*(a->z - b->z);
}

SimpELEN::SimpELEN(Surface* so, int nt=0)
{
  s = so;
  nthreads = nt;
}

//Get cell number which p belongs to
int SimpELEN::getGridCell(Point* p)
{
  int cx = (p->x - s->bbox.minx)/dim[0]; //x
  cx = cx - (cx/grid_res);
  int cy = (p->y - s->bbox.miny)/dim[1]; //y
  cy = cy - (cy/grid_res);
  int cz = (p->z - s->bbox.minz)/dim[2]; //z
  cz = cz - (cz/grid_res);
  //Resulting cell is cx + 3*cy + 9*cz
  //cerr << "cx cy cz " << cx << " " << cy << " " << cz << endl;
  return cx + grid_res*cy + grid_res*grid_res*cz;
}

bool SimpELEN::isEntirelyInCell(Edge* e)
{
  int c1 = getGridCell(e->p1);
  int c2 = getGridCell(e->p2);

  if(c1==c2) return true; //Returns cell number if edge is entirely in a single cell
  else return false; //Returns -1 if edge intersects cell

}

bool SimpELEN::isCrownInCell(Point* p)
{
  int c = getGridCell(p);//Get p's cell id
  for(edge_vec_it eit = p->from.begin(); eit != p->from.end(); ++eit)
  {
    //Only test for p2 because edges FROM p have p as p1

    if(getGridCell((*eit)->p2) != c) //If one of p's neighbours is not in cell c then false
      return false;
  }

  for(edge_vec_it eit = p->to.begin(); eit!= p->to.end(); ++eit)
  {
    //Only test for p1 because edges TO p have p as p2
    if(getGridCell((*eit)->p1) != c)
      return false;
  }
  return true;
}

void SimpELEN::initUniformGrid(int res)
{

  //initEdgeCosts(); //create Edges
  //Cellsize for each dimension
  dim[0] = s->bbox.getXLen()/res;
  dim[1] = s->bbox.getYLen()/res;
  dim[2] = s->bbox.getZLen()/res;

  grid_res = res;
  n_cells = grid_res*grid_res*grid_res;
  int ncells = n_cells;
  //cerr << "Allocating ncells \n";
  //cerr << "Ncells " << n_cells << endl;
  //cerr << "Cell Dimensions " << dim[0] << " " << dim[1] << " " << dim[2] << endl;

  cell = new vector<Point*>[n_cells];
  cell_queue = new priority_queue<Edge>[n_cells];
  initial_vertices = new int[n_cells];


  // for(point_vec_it pit = s->m_points.begin(); pit != s->m_points.end(); ++pit)
  // {
  //   unsigned long int cellpos = getGridCell((*pit));
  //   cell[cellpos].push_back(*pit);
  // }

  for(int i = 0 ; i < s->m_points.size(); ++i)
  {
    unsigned long int cellpos = getGridCell(s->m_points[i]);
    cell[cellpos].push_back(s->m_points[i]);
  }

  omp_set_num_threads(nthreads);
  #pragma omp parallel for
  for(int i = 0 ; i < ncells; ++i)
  {
    //cerr << cell[i].size() << endl;
    initial_vertices[i]=cell[i].size();
    for(point_vec_it pit = cell[i].begin(); pit != cell[i].end(); ++pit)
    {
      for(edge_vec_it eit = (*pit)->from.begin(); eit != (*pit)->from.end(); ++eit)
      {
        // /cerr << "eid  " << (*eit)->id << " - " << (*eit)->p1->id << " " <<(*eit)->p2->id << endl;
        //Check if edge is entirely in cell and so are the endpoint crowns
        if(isEntirelyInCell(*eit) && isCrownInCell((*eit)->p1) && isCrownInCell((*eit)->p2))
        {
            cell_queue[i].push(*(*eit));
        }
      }
    }
  }
}

void SimpELEN::resetQueue()
{
  priority_queue<Edge> newq;
  for(int i = 0; i < s->m_edges.size(); ++i)
  {
    if(!s->is_edge_removed[i])
    {
      newq.push(*(s->m_edges[i]));
    }
  }
  newq.swap(edge_queue);
}

void SimpELEN::simplify(int goal, int gridres=1)
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

        bool collapsed = s->collapse(e);

        if(collapsed)
        {
          clock_gettime(CLOCK_REALTIME,&tu0);
          vr++;
          updateEdgeCosts(e.p2, i);
          currentEdgeCost[e.id] = INF; // Edge has been removed
          #pragma omp atomic
          vertices_removed++;

          clock_gettime(CLOCK_REALTIME,&tu1);
          tu = diff (tu0,tu1);
          time_updating+=getNanoseconds(tu);
        }
      }
      //cerr << "Vertices removed: " << vr << endl;
    }
    if(gridres>=2) gridres/=2;
  }


  // while(vertices_removed < goal && !edge_queue.empty())
  // {
  //   timespec ti0,ti1,ti;
  //   clock_gettime(CLOCK_REALTIME,&ti0);
  //
  //   Edge e = edge_queue.top();
  //   edge_queue.pop();
  //   //cout << "heap size: " << edge_queue.size() << endl;
  //   if(s->is_edge_removed[e.id] || e.cost != currentEdgeCost[e.id] || e.p1->faces.empty() || e.p2->faces.empty()) //Edge cost has changed, ignore this heap node
  //   {
  //     //cout << redtty << "discarding e " << e.id << " " << e.p1->id << " " << e.p2->id << " " << e.cost <<   deftty << endl;
  //     failed_pop++;
  //
  //     if(s->is_edge_removed[e.id])
  //       failed_removed++;
  //     else if (e.cost!=currentEdgeCost[e.id])
  //       failed_cost++;
  //
  //     clock_gettime(CLOCK_REALTIME,&ti1);
  //     ti = diff(ti0,ti1);
  //     time_iterating+= getNanoseconds(ti);
  //
  //
  //     clock_gettime(CLOCK_REALTIME,&tr0);
  //     // if(edges_outdated >= s->m_edges.size()*0.20)
  //     // {
  //     //     //cerr << "Queue size " << edge_queue.size() << " resetting\n";
  //     //     resetQueue();
  //     //     edges_outdated = 0;
  //     // }
  //     clock_gettime(CLOCK_REALTIME,&tr1);
  //     tr = diff(tr0,tr1);
  //     time_resetting+=getNanoseconds(tr);
  //
  //
  //     continue;
  //   }
  //
  //   timespec tc0,tc1,tc;
  //   clock_gettime(CLOCK_REALTIME, &tc0);
  //   bool collapsed = s->collapse(e);
  //   clock_gettime(CLOCK_REALTIME, &tc1);
  //   tc = diff(tc0,tc1);
  //   time_collapsing += getNanoseconds(tc);
  //
  //   if(collapsed)
  //   {
  //       timespec tu0, tu1;
  //       clock_gettime(CLOCK_REALTIME,&tu0);
  //       updateEdgeCosts(e.p2);
  //       clock_gettime(CLOCK_REALTIME,&tu1);
  //       tu = diff (tu0,tu1);
  //       time_updating += getNanoseconds(tu);
  //       currentEdgeCost[e.id] = INF; // Edge has been removed
  //       vertices_removed++;
  //   }
  // }
  clock_gettime(CLOCK_REALTIME, &t1);
  t = diff(t0,t1);


  cout << bluetty << "Time_simplify: " << getNanoseconds(t)/1000000 << deftty << endl;
  cout << bluetty << "Time_updating: " << time_updating/1000000 << deftty << endl;
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

void SimpELEN::setPlacement(Edge* e)
{
  double x = (e->p1->x + e->p2->x)/2;
  double y = (e->p1->y + e->p2->y)/2;
  double z = (e->p1->z + e->p2->z)/2;
  e->placement->x = x;
  e->placement->y = y;
  e->placement->z = z;

}

double SimpELEN::getCost(Edge* e)
{
  setPlacement(e);
  double cost = dist(e->p1,e->p2);
  return cost;
}


void SimpELEN::updateEdgeCosts(Point* v, int i)
{
  edges_outdated += v->from.size() + v->to.size();
  timespec t,t0,t1;
  long int tcost = 0;

  for(vector<Edge*>::iterator eit = v->from.begin(); eit != v->from.end(); ++ eit)
  {
    if(isEntirelyInCell(*eit) && isCrownInCell((*eit)->p1) && isCrownInCell((*eit)->p2))
    {
      //cerr << "Edge update " << (*eit)->id << " - " << (*eit)->p1->id << " " << (*eit)->p2->id << endl;
      clock_gettime(CLOCK_REALTIME,&t0);
      (*eit)->cost = getCost((*eit));
      clock_gettime(CLOCK_REALTIME,&t1);
      t = diff (t0,t1);
      tcost+=getNanoseconds(t);
      currentEdgeCost[(*eit)->id] = (*eit)->cost;
      //pair<int,int> pp((*eit)->p1->id,(*eit)->p2->id);
      //currentEdgePoints[(*eit)->id] = pp;
      cell_queue[i].push(*(*eit));
    }

  }
  for(vector<Edge*>::iterator eit = v->to.begin(); eit != v->to.end(); ++ eit)
  {
    if(isEntirelyInCell(*eit) && isCrownInCell((*eit)->p1) && isCrownInCell((*eit)->p2))
    {
      //cerr << "Edge update " << (*eit)->id << " - " << (*eit)->p1->id << " " << (*eit)->p2->id << endl;
      clock_gettime(CLOCK_REALTIME,&t0);
      (*eit)->cost = getCost((*eit));
      clock_gettime(CLOCK_REALTIME,&t1);
      t = diff (t0,t1);
      tcost+=getNanoseconds(t);
      currentEdgeCost[(*eit)->id] = (*eit)->cost;
      //pair<int,int> pp((*eit)->p1->id,(*eit)->p2->id);
      //currentEdgePoints[(*eit)->id] = pp;
      cell_queue[i].push(*(*eit));
    }
  }
  cout << "one edge update: " << tcost << endl;
}

void SimpELEN::initEdgeCosts()
{
  cerr << "Init edges.\n";
  //Iterate over every face generating respective edges
  int eid = 0;

  //cerr << "costs - ";
  //TODO: avoid duplicates by only creating edges such that v1->id < v2->id
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
              eaux->cost = getCost(eaux);
              eaux->id = eid;
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






        // //First edge keep p1 and p2 ascending ids
        // if((*it)->points[0]->id < (*it)->points[1]->id)
        // {
        //   Edge* eaux = new Edge((*it)->points[0], (*it)->points[1]);
        //   eaux->cost = getCost(eaux);
        //   eaux->id = eid;
        //   edge_queue.push(*eaux);
        //   eaux->p1->from.push_back(eaux);
        //   eaux->p2->to.push_back(eaux);
        //   currentEdgeCost.push_back(eaux->cost);
        //   eid++;
        // }
        // else
        // {
        //   Edge* eaux = new Edge((*it)->points[1], (*it)->points[0]);
        //   eaux->cost = getCost(eaux);
        //   eaux->id = eid;
        //   edge_queue.push(*eaux);
        //   eaux->p1->from.push_back(eaux);
        //   eaux->p2->to.push_back(eaux);
        //   currentEdgeCost.push_back(eaux->cost);
        //   eid++;
        // }
        //
        //
        // //Second edge
        // if((*it)->points[0]->id < (*it)->points[2]->id)
        // {
        //   Edge* eaux = new Edge((*it)->points[0], (*it)->points[2]);
        //   eaux->cost = getCost(eaux);
        //   eaux->id = eid;
        //   edge_queue.push(*eaux);
        //   eaux->p1->from.push_back(eaux);
        //   eaux->p2->to.push_back(eaux);
        //   currentEdgeCost.push_back(eaux->cost);
        //   eid++;
        // }
        // else
        // {
        //   Edge* eaux = new Edge((*it)->points[2], (*it)->points[0]);
        //   eaux->cost = getCost(eaux);
        //   eaux->id = eid;
        //   edge_queue.push(*eaux);
        //   eaux->p1->from.push_back(eaux);
        //   eaux->p2->to.push_back(eaux);
        //   currentEdgeCost.push_back(eaux->cost);
        //   eid++;
        // }
        //
        //
        // //Third edge
        // if((*it)->points[1]->id < (*it)->points[2]->id)
        // {
        //   Edge* eaux = new Edge((*it)->points[1], (*it)->points[2]);
        //   eaux->cost = getCost(eaux);
        //   eaux->id = eid;
        //   edge_queue.push(*eaux);
        //   eaux->p1->from.push_back(eaux);
        //   eaux->p2->to.push_back(eaux);
        //   currentEdgeCost.push_back(eaux->cost);
        //   eid++;
        // }
        // else
        // {
        //   Edge* eaux = new Edge((*it)->points[2], (*it)->points[1]);
        //   eaux->cost = getCost(eaux);
        //   eaux->id = eid;
        //   edge_queue.push(*eaux);
        //   eaux->p1->from.push_back(eaux);
        //   eaux->p2->to.push_back(eaux);
        //   currentEdgeCost.push_back(eaux->cost);
        //   eid++;
        // }

  }
  total_edges = eid;
  cerr <<"Edges: " << total_edges << endl;
}
