/**
 *  \file BallMover.cpp
 *  \brief A mover that keeps a particle in a box
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/symmetry/BallMover.h>
#include <IMP/log_macros.h>
#include <IMP/core/XYZ.h>
#include <IMP/algebra.h>

IMPSYMMETRY_BEGIN_NAMESPACE

BallMover::BallMover(Particle *p, Particles ps, Float max_tr,
                     algebra::Vector3Ds ctrs, algebra::Transformation3Ds trs)
    : MonteCarloMover(p->get_model(), "BallMover%1%"),
      p_(p),
      ps_(ps),
      max_tr_(max_tr),
      ctrs_(ctrs),
      trs_(trs) {
  // check that p_ is among ps
}

core::MonteCarloMoverResult BallMover::do_propose() {
  IMP_OBJECT_LOG;

  // random displacement
  algebra::Vector3D displacement = algebra::get_random_vector_in(
      algebra::Sphere3D(algebra::Vector3D(0.0, 0.0, 0.0), max_tr_));

  // store old coordinates of master particle
  oldcoord_ = core::XYZ(p_).get_coordinates();

  // master particle coordinates after displacement
  algebra::Vector3D nc = oldcoord_ + displacement;

  // find center of the closest cell
  double mindist = 1.0e+24;
  unsigned icell = 0;
  for (unsigned i = 0; i < ctrs_.size(); ++i) {
    // calculate distance between nc and cell center
    double dist = algebra::get_l2_norm(nc - ctrs_[i]);
    // find minimum distance
    if (dist < mindist) {
      mindist = dist;
      icell = i;
    }
  }

  // find inverse transformation
  algebra::Transformation3D cell_tr = trs_[icell].get_inverse();

  // set new coordinates for master particle
  core::XYZ(p_).set_coordinates(cell_tr.get_transformed(nc));

  // set new coordinates for slave particles
  oldcoords_.clear();
  for (unsigned i = 0; i < ps_.size(); ++i) {
    core::XYZ xyz = core::XYZ(ps_[i]);
    algebra::Vector3D oc = xyz.get_coordinates();
    // store old coordinates
    oldcoords_.push_back(oc);
    // apply transformation
    algebra::Vector3D nc = cell_tr.get_transformed(oc);
    xyz.set_coordinates(nc);
  }

  ParticlesTemp ret;
  ret.push_back(p_);
  ret.insert(ret.end(), ps_.begin(), ps_.end());

  return core::MonteCarloMoverResult(get_indexes(ret), 1.0);
}

ModelObjectsTemp BallMover::do_get_inputs() const {
  ParticlesTemp ret;
  ret.push_back(p_);
  ret.insert(ret.end(), ps_.begin(), ps_.end());
  return ret;
}

void BallMover::do_reject() {
  // master particle old coordinates
  core::XYZ(p_).set_coordinates(oldcoord_);
  // slave particles old coordinates
  for (unsigned i = 0; i < ps_.size(); ++i) {
    core::XYZ(ps_[i]).set_coordinates(oldcoords_[i]);
  }
}

IMPSYMMETRY_END_NAMESPACE
