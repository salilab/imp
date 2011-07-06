/**
 *  \file spb.cpp
 *  \brief SPB in C++
 *
 *  Copyright 2011 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/core.h>
#include <IMP/atom.h>
#include <IMP/algebra.h>
#include <IMP/container.h>
#include <boost/program_options.hpp>
#include <IMP/base_types.h>
#include <string>
#include <iostream>
using namespace IMP;

algebra::Vector2Ds do_compress
(algebra::Vector2Ds points, double xf, double yf)
{
 algebra::Vector2Ds ret=algebra::Vector2Ds();
 for(int i=0;i<points.size();++i){
  ret.push_back(algebra::Vector2D(points[i][0]*xf,points[i][1]*yf));
 }
 return ret;
}

algebra::Vector2Ds do_shear
(algebra::Vector2Ds points, double bh, double h)
{
 algebra::Vector2Ds ret=algebra::Vector2Ds();
 double b=bh/h;
 for(int i=0;i<points.size();++i){
  ret.push_back(algebra::Vector2D(points[i][0]-b*points[i][1],points[i][1]));
 }
 return ret;
}

algebra::Vector3Ds grid_cell(double side, double ds, double z)
{
 algebra::BoundingBox2D bb=algebra::BoundingBox2D(algebra::Vector2D(0.0,0.0),
                                               algebra::Vector2D(side,side));
 algebra::Vector2Ds cur=algebra::get_grid_interior_cover_by_spacing(bb, ds);

 if(cell_type!="square"){
  algebra::Vector2Ds cur_comp=do_compress(cur,1.0,sqrt(3.0)/2.0);
  cur=do_shear(cur_comp,side/2.0,side*sqrt(3.0)/2.0);
 }

 algebra::Vector3Ds positions=algebra::Vector3Ds();
 algebra::Vector3Ds positions2=algebra::Vector3Ds();
 for(int i=0;i<cur.size();++i){
  positions.push_back(algebra::Vector3D(cur[i][0],cur[i][1],z));
 }
 if(cell_type=="hexagon"){
  algebra::Vector3D tra=algebra::Vector3D(0.0,0.0,0.0);
  for(i=0;i<3;++i){
   algebra::Rotation3D rot=
   algebra::get_rotation_about_axis(algebra::Vector3D(0.0,0.0,1.0),
                                    (double)i * 2.0 * IMP::PI / 3.0);
   algebra::Transformation3D tr=algebra::Transformation3D(rot,tra);
   for(j=0;j<positions.size();++j){
    positions2.push_back(tr.get_trasformed(positions[j]));
   }
  }
 }
 positions+=positions2;
 return positions;
}

int main(int  , char **)
{

// various parameters
const double sqrt3=sqrt(3.0);
const double ds=40.0;
double       side=80.0;
const int    niter=3;
bool         do_statistics=true;
bool         do_random=true;
bool         do_save_ass=false;
const int    skip=100;
std::string  cell_type="hexagon"
int          num_cells;
int          num_copies;
double       error_bound;

// cell dependent parameters
if(cell_type=="rhombus"){
 num_cells=21;
 num_copies=2;
 error_bound=1.45*pow(ds,2);
}else if(cell_type=="hexagon"){
 num_cells=7;
 num_copies=6;
 error_bound=1.45*pow(ds,2);
}else if(cell_type=="square"){
 num_cells=9;
 num_copies=6;
 side=sqrt(1.5*pow(side,2)*sqrt3);
 error_bound=pow(ds,2);
}

// create a new model
IMP_NEW(Model,m,());

return 0;
}
