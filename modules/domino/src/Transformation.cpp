/**
 *  \file XYZ.cpp   \brief Transformation decorator.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#include <IMP/domino/Transformation.h>
#include <boost/assign/list_of.hpp>
#include <IMP/core/XYZ.h>

IMPDOMINO_BEGIN_NAMESPACE

void Transformation::show(std::ostream &out) const
{
  algebra::VectorD<8> v;
  Particle *p=get_particle();
  v[0]=p->get_value(get_rotation_key(0));
  v[1]=p->get_value(get_rotation_key(1));
  v[2]=p->get_value(get_rotation_key(2));
  v[3]=p->get_value(get_rotation_key(3));
  v[4]=p->get_value(get_translation_key(0));
  v[5]=p->get_value(get_translation_key(1));
  v[6]=p->get_value(get_translation_key(2));
  v[7]=p->get_value(get_transformation_index_key());
  out << "(" <<algebra::commas_io(v)<<")";
}

const FloatKeys& Transformation::get_rotation_keys() {
  static FloatKeys rks=boost::assign::list_of(FloatKey("a"))
    (FloatKey("b"))
    (FloatKey("c"))
    (FloatKey("d"));
  return rks;
}
const FloatKeys& Transformation::get_translation_keys() {
  return IMP::core::XYZ::get_xyz_keys();
}
const IntKey& Transformation::get_transformation_index_key() {
  return IMP::IntKey("trans_ind");
}
//! get a Transformation3D object from the decorator
algebra::Transformation3D Transformation::get_transformation() {
  Particle *p=get_particle();
  return algebra::Transformation3D(
       algebra::Rotation3D(p->get_value(get_rotation_key(0)),
                           p->get_value(get_rotation_key(1)),
                           p->get_value(get_rotation_key(2)),
                           p->get_value(get_rotation_key(3))),
       algebra::Vector3D(p->get_value(get_translation_key(0)),
                         p->get_value(get_translation_key(1)),
                         p->get_value(get_translation_key(2))));
}
IMPDOMINO_END_NAMESPACE
