/**
 *  \file PbcBoxedMover.cpp
 *  \brief A mover that keeps a particle in a box
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/algebra.h>
#include <IMP/algebra/Vector3D.h>
#include <IMP/atom.h>
#include <IMP/core.h>
#include <IMP/isd/Scale.h>
#include <IMP/spb/PbcBoxedMover.h>
#include <iostream>

IMPSPB_BEGIN_NAMESPACE

PbcBoxedMover::PbcBoxedMover(Particle *p, Particles ps, Float max_tr,
                             algebra::Vector3Ds centers,
                             algebra::Transformation3Ds transformations,
                             Particle *px, Particle *py, Particle *pz)
    : MonteCarloMover(p->get_model(), "PbcBoxedMover%1%") {
  p_ = p;
  max_tr_ = max_tr;
  centers_ = centers;
  ps_ = ps;
  transformations_ = transformations;
  // store Scale particles
  px_ = px;
  py_ = py;
  pz_ = pz;
}

algebra::Vector3D PbcBoxedMover::get_vector(algebra::Vector3D center) {
  Float sx = isd::Scale(px_).get_scale();
  Float sy = isd::Scale(py_).get_scale();
  Float sz = isd::Scale(pz_).get_scale();
  algebra::Vector3D newcenter =
      algebra::Vector3D(center[0] * sx, center[1] * sy, center[2] * sz);
  return newcenter;
}

algebra::Transformation3D PbcBoxedMover::get_transformation(
    algebra::Transformation3D trans) {
  algebra::Rotation3D rr = trans.get_rotation();
  algebra::Vector3D tt = trans.get_translation();
  algebra::Transformation3D newtrans(rr, get_vector(tt));
  return newtrans;
}

core::MonteCarloMoverResult PbcBoxedMover::do_propose() {
  algebra::Vector3D tr_x = algebra::get_random_vector_in(
      algebra::Sphere3D(algebra::Vector3D(0.0, 0.0, 0.0), max_tr_));

  algebra::Vector3D newcoord = core::XYZ(p_).get_coordinates() + tr_x;

  // find cell
  double mindist = 1.0e+24;
  unsigned icell = 0;
  for (unsigned int i = 0; i < centers_.size(); ++i) {
    double dist = algebra::get_l2_norm(newcoord - get_vector(centers_[i]));
    if (dist < mindist) {
      mindist = dist;
      icell = i;
    }
  }

  // scale transformations for size of the cell
  algebra::Transformation3D trans =
      get_transformation(transformations_[icell]).get_inverse();

  ParticlesTemp ret;
  if (icell == 0)
    ret.push_back(p_);
  else
    ret = ps_;

  oldcoords_.clear();
  for (unsigned int i = 0; i < ps_.size(); ++i) {
    core::XYZ xyz = core::XYZ(ps_[i]);
    algebra::Vector3D oc = xyz.get_coordinates();
    oldcoords_.push_back(oc);
    algebra::Vector3D trr_x = algebra::Vector3D(0.0, 0.0, 0.0);
    if (ps_[i] == p_) trr_x = tr_x;
    newcoord = trans.get_transformed(oc + trr_x);
    xyz.set_coordinates(newcoord);
  }

  return core::MonteCarloMoverResult(IMP::get_indexes(ret), 1.0);
}

ModelObjectsTemp PbcBoxedMover::do_get_inputs() const {
  IMP::ParticlesTemp ret;
  ret.push_back(p_);
  ret.insert(ret.end(), ps_.begin(), ps_.end());

  ret.push_back(px_);
  ret.push_back(py_);
  ret.push_back(pz_);

  return ret;
}

void PbcBoxedMover::do_reject() {
  for (unsigned int i = 0; i < ps_.size(); ++i) {
    core::XYZ(ps_[i]).set_coordinates(oldcoords_[i]);
  }
}


void PbcBoxedMover::show(std::ostream &out) const {

  out << "max translation: " << max_tr_ << "\n";
}

IMPSPB_END_NAMESPACE
