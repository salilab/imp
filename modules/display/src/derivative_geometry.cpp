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

XYZDerivativeGeometry::XYZDerivativeGeometry(core::XYZDecorator d,
                                             Float radius): d_(d),
                                                            radius_(radius){
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
  return radius_;
}


RigidBodyDerivativeGeometry
::RigidBodyDerivativeGeometry(core::RigidBodyDecorator d,
                              ParticleRefiner *pr): pr_(pr), d_(d){
  xyzcolor_=Color(1,0,0);
  qcolor_=Color(0,1,0);
  ccolor_=Color(0,0,1);
}

RigidBodyDerivativeGeometry::~RigidBodyDerivativeGeometry(){}

void RigidBodyDerivativeGeometry::show(std::ostream &out) const {
  out << "DerivativeGeometryExtractor" << std::endl;
}

Geometries RigidBodyDerivativeGeometry::get_geometry() const {
  Particles ms= pr_->get_refined(d_.get_particle());
  Geometries ret;
  algebra::Transformation3D otr= d_.get_transformation();
  algebra::VectorD<4> rderiv= d_.get_rotational_derivatives();
  algebra::Vector3D tderiv= d_.get_derivatives();
  algebra::VectorD<4> rot = otr.get_rotation().get_quaternion();
  IMP_LOG(TERSE, "Old rotation was " << rot << std::endl);

  Float scale=.1;
  algebra::VectorD<4> dv=rderiv;
  if (dv.get_squared_magnitude() != 0) {
    dv= scale*dv.get_unit_vector();
  }
  rot+= dv;
  rot= rot.get_unit_vector();
  algebra::Rotation3D r(rot[0], rot[1], rot[2], rot[3]);
  IMP_LOG(TERSE, "Derivative was " << tderiv << std::endl);
  IMP_LOG(TERSE, "New rotation is " << rot << std::endl);

  FloatPair xr= d_.get_particle()->get_model()
    ->get_range(core::XYZDecorator::get_xyz_keys()[0]);
  Float wid= xr.second-xr.first;
  algebra::Vector3D stderiv= scale*tderiv*wid;
  algebra::Transformation3D ntr(algebra::Rotation3D(rot[0], rot[1],
                                                    rot[2], rot[3]),
                                stderiv+otr.get_translation());
  for (unsigned int i=0; i< ms.size(); ++i) {
    core::RigidMemberDecorator dm(ms[i], d_.get_traits());
    CylinderGeometry *tr
      = new CylinderGeometry(algebra::Cylinder3D(dm.get_coordinates(),
                                                 dm.get_coordinates()+tderiv,
                                                 0));
    tr->set_color(xyzcolor_);
    ret.push_back(tr);
    algebra::Vector3D ic= r.rotate(dm.get_internal_coordinates())
      + d_.get_coordinates();
    CylinderGeometry *rtr
      = new CylinderGeometry(algebra::Cylinder3D(dm.get_coordinates(),
                                                 ic,
                                                 0));
    rtr->set_color(qcolor_);
    ret.push_back(rtr);
    CylinderGeometry *nrtr
      = new CylinderGeometry(algebra::Cylinder3D(dm.get_coordinates(),
                             ntr.transform(dm.get_internal_coordinates()),
                                                 0));
    nrtr->set_color(ccolor_);
    ret.push_back(nrtr);
  }
  pr_->cleanup_refined(d_.get_particle(), ms);
  return ret;
}

IMPDISPLAY_END_NAMESPACE
