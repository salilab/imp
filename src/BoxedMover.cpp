/**
 *  \file BoxedMover.cpp
 *  \brief A mover that keeps a particle in a box
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/membrane/BoxedMover.h>
#include <IMP/core.h>
#include <IMP/algebra.h>
#include <IMP/algebra/Vector3D.h>
#include <iostream>

IMPMEMBRANE_BEGIN_NAMESPACE

BoxedMover::BoxedMover(Particle *p, Float max_tr,
                       Float side, Float thick, algebra::VectorD<3> center,
                       std::string cell_type) {
  IMP_LOG(VERBOSE,"start BoxedMover constructor");
  max_tr_ = max_tr;
  side_ = side;
  thick_ = thick;
  center_ = center;
  cell_type_ = cell_type;
  p_ = p;
  oldcoord_=(core::XYZ(p)).get_coordinates();
  IMP_LOG(VERBOSE,"finish mover construction" << std::endl);
}

void BoxedMover::propose_move(Float f) {
  IMP_LOG(VERBOSE,"BoxedMover:: propose move f is  : " << f <<std::endl);
  {
    ::boost::uniform_real<> rand(0,1);
    double fc =rand(random_number_generator);
    if (fc > f) return;
  }

  double sqrt3=sqrt(3.0);
  algebra::VectorD<3> newcoord;
  oldcoord_=core::XYZ(p_).get_coordinates();

  bool outside=true;
  while(outside){
   algebra::VectorD<3> tr_x
     = algebra::get_random_vector_in(algebra::Sphere3D
                (algebra::Vector3D(0.0,0.0,0.0),max_tr_));

   newcoord=oldcoord_+tr_x;

// this conditions must be satisfied regardless of cell_type
   if(fabs(newcoord[2]-center_[2])>thick_/2.0) {continue;}

// hexagonal cell
   if(cell_type_=="hexagon"){
    if(fabs(newcoord[0])>side_) {continue;}
    if(newcoord[1]>=0.0){
     double tmp=std::min(sqrt3/2.0*side_,sqrt3*(newcoord[0]+side_));
     double thr=std::min(tmp,-sqrt3*(newcoord[0]-side_));
     if(newcoord[1]<thr) {outside=false;}
    } else {
     double tmp=std::max(-sqrt3/2.0*side_,-sqrt3*(newcoord[0]+side_));
     double thr=std::max(tmp,sqrt3*(newcoord[0]-side_));
     if(newcoord[1]>thr) {outside=false;}
    }
// rhombus cell
   } else if(cell_type_=="rhombus"){
    if(newcoord[0]<-side_/2.0 || newcoord[0]>side_) {continue;}
    double thr1=std::min(sqrt3/2.0*side_,-sqrt3*(newcoord[0]-side_));
    double thr2=std::max(0.0,-sqrt3*(newcoord[0]+side_));
    if(newcoord[1]<thr1 && newcoord[1]>thr2) {outside=false;}
// square cell
   } else if(cell_type_=="square"){
    if(fabs(newcoord[0])<side_/2.0 && fabs(newcoord[1])<side_/2.0){
     outside=false;
    }
   }
  }
  core::XYZ(p_).set_coordinates(newcoord);
}


void BoxedMover::reset_move() {
  core::XYZ(p_).set_coordinates(oldcoord_);
}


void BoxedMover::do_show(std::ostream &out) const {
  out << "max translation: " << max_tr_ << "\n";
}

IMPMEMBRANE_END_NAMESPACE
