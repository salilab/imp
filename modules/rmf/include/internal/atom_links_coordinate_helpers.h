/**
 *  \file IMP/rmf/atom_links.h
 *  \brief Handle read/write of kernel::Model data from/to files.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPRMF_ATOM_LINKS_COORDINATE_HELPERS_H
#define IMPRMF_ATOM_LINKS_COORDINATE_HELPERS_H

#include <IMP/rmf/rmf_config.h>
#include <RMF/NodeHandle.h>
#include <RMF/FileHandle.h>
#include <RMF/decorators.h>
#include <IMP/algebra/Transformation3D.h>
IMPRMF_BEGIN_INTERNAL_NAMESPACE

inline void copy_to_frame_reference_frame(
    const algebra::Transformation3D &tr, RMF::NodeHandle n,
    RMF::decorator::ReferenceFrameFactory rff) {
  RMF::decorator::ReferenceFrame rf = rff.get(n);
  algebra::Vector3D translation = tr.get_translation();
  algebra::Vector4D quaternion = tr.get_rotation().get_quaternion();
  rf.set_frame_translation(RMF::Vector3(translation));
  rf.set_frame_rotation(RMF::Vector4(quaternion));
}
inline void copy_to_frame_particle(
    const algebra::Vector3D &v, RMF::NodeHandle n,
    RMF::decorator::IntermediateParticleFactory ipf) {
  RMF::decorator::IntermediateParticle ip = ipf.get(n);
  ip.set_frame_coordinates(RMF::Vector3(v));
}
inline void copy_to_static_reference_frame(
    const algebra::Transformation3D &tr, RMF::NodeHandle n,
    RMF::decorator::ReferenceFrameFactory rff) {
  RMF::decorator::ReferenceFrame rf = rff.get(n);
  algebra::Vector3D translation = tr.get_translation();
  algebra::Vector4D quaternion = tr.get_rotation().get_quaternion();
  rf.set_static_translation(RMF::Vector3(translation));
  rf.set_static_rotation(RMF::Vector4(quaternion));
}
inline void copy_to_static_particle(
    const algebra::Vector3D &v, RMF::NodeHandle n,
    RMF::decorator::IntermediateParticleFactory ipf) {
  RMF::decorator::IntermediateParticle ip = ipf.get(n);
  ip.set_static_coordinates(RMF::Vector3(v));
}
inline algebra::Vector3D get_coordinates(
    RMF::NodeConstHandle n,
    RMF::decorator::IntermediateParticleFactory ipf) {
  RMF::Vector3 coords = ipf.get(n).get_coordinates();
  return algebra::Vector3D(coords);
}
inline algebra::Transformation3D get_transformation(
    RMF::NodeConstHandle n, RMF::decorator::ReferenceFrameFactory rfcf) {
  RMF::Vector3 coords = rfcf.get(n).get_translation();
  RMF::Vector4 quat = rfcf.get(n).get_rotation();
  return algebra::Transformation3D(algebra::Rotation3D(algebra::Vector4D(quat)),
                                   algebra::Vector3D(coords));
}

IMPRMF_END_INTERNAL_NAMESPACE

#endif /* IMPRMF_ATOM_LINKS_COORDINATE_HELPERS_H */
