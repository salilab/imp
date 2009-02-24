/**
 *  \file derivative_geometry.cpp
 *  \brief XXXX.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#include "IMP/display/derivative_geometry.h"
#include "IMP/display/CylinderGeometry.h"


IMPDISPLAY_BEGIN_NAMESPACE

XYZDerivativeGeometry::XYZDerivativeGeometry(core::XYZDecorator d): d_(d){
}

XYZDerivativeGeometry::~XYZDerivativeGeometry(){}

void XYZDerivativeGeometry::show(std::ostream &out) const {
  out << "XYZDerivativeGeometry" << std::endl;
}

unsigned int XYZDerivativeGeometry::get_dimension() const{
  return 1;
}
unsigned int XYZDerivativeGeometry::get_number_of_vertices() const{
  return 2;
}
algebra::Vector3D XYZDerivativeGeometry::get_vertex(unsigned int i) const {
  if (i==0) return d_.get_coordinates();
  else return d_.get_coordinates()+d_.get_derivatives();
}

Float XYZDerivativeGeometry::get_size() const {
  return 0;
}


RigidBodyDerivativeGeometry
::RigidBodyDerivativeGeometry(core::RigidBodyDecorator d,
                              ParticleRefiner *pr): pr_(pr), d_(d){
  xyzcolor_=Color(1,0,0);
  qcolor_=Color(0,1,0);
}

RigidBodyDerivativeGeometry::~RigidBodyDerivativeGeometry(){}

void RigidBodyDerivativeGeometry::show(std::ostream &out) const {
  out << "DerivativeGeometryExtractor" << std::endl;
}

Geometries RigidBodyDerivativeGeometry::get_geometry() const {
  Particles ms= pr_->get_refined(d_.get_particle());
  Geometries ret;
  algebra::VectorD<4> deriv= d_.get_rotational_derivatives();
  algebra::VectorD<4> rot
    = d_.get_transformation().get_rotation().get_quaternion();
  IMP_LOG(TERSE, "Old rotation was " << rot << std::endl);
  algebra::VectorD<4> dv=deriv;
  if (dv.get_squared_magnitude() != 0) {
    dv= .1*dv.get_unit_vector();
  }
  rot+= dv;
  rot= rot.get_unit_vector();
  algebra::Rotation3D r(rot[0], rot[1], rot[2], rot[3]);
  IMP_LOG(TERSE, "Derivative was " << deriv << std::endl);
  IMP_LOG(TERSE, "New rotation is " << rot << std::endl);
  for (unsigned int i=0; i< ms.size(); ++i) {
    core::RigidMemberDecorator dm(ms[i], d_.get_traits());
    CylinderGeometry *tr= new CylinderGeometry(dm.get_coordinates(),
                    dm.get_coordinates()+d_.get_derivatives(),
                                               0);
    tr->set_color(xyzcolor_);
    ret.push_back(tr);
    algebra::Vector3D ic= r.rotate(dm.get_internal_coordinates())
      + d_.get_coordinates();
    CylinderGeometry *rtr= new CylinderGeometry(dm.get_coordinates(),
                                                ic,
                                                0);
    rtr->set_color(qcolor_);
    ret.push_back(rtr);
  }
  pr_->cleanup_refined(d_.get_particle(), ms);
  return ret;
}

IMPDISPLAY_END_NAMESPACE
