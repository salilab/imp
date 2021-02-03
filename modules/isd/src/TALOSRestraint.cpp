/**
 *  \file TALOSRestraint.cpp \brief TALOS dihedral restraint between four
 *                                     particles.
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/isd/TALOSRestraint.h>
#include <IMP/isd/Scale.h>
#include <IMP/isd/vonMisesSufficient.h>

#include <IMP/core/XYZ.h>
#include <IMP/core/internal/dihedral_helpers.h>
#include <IMP/algebra/Vector3D.h>

#include <IMP/Particle.h>
#include <IMP/Model.h>
#include <IMP/log.h>

#include <math.h>
#include <boost/tuple/tuple.hpp>

IMPISD_BEGIN_NAMESPACE

TALOSRestraint::TALOSRestraint(Model *m, Particles p,
                               Floats data, Particle *kappa)
    : Restraint(m, "TALOSRestraint%1%"), kappa_(kappa) {
  if (p.size() != 4) {
    IMP_THROW("please provide a list with 4 particles!", ModelException);
  }
  p_[0] = static_cast<Particle *>(p[0]);
  p_[1] = static_cast<Particle *>(p[1]);
  p_[2] = static_cast<Particle *>(p[2]);
  p_[3] = static_cast<Particle *>(p[3]);
  // create von Mises
  double kappaval = Scale(kappa_).get_scale();
  mises_ = new vonMisesSufficient(0, data, kappaval);
  // mises_->set_was_used(true);
}

TALOSRestraint::TALOSRestraint(Model *m, Particles p,
                               unsigned N, double R0, double chiexp,
                               Particle *kappa)
    : Restraint(m, "TALOSRestraint%1%"), kappa_(kappa) {
  if (p.size() != 4) {
    IMP_THROW("please provide a list with 4 particles!", ModelException);
  }
  p_[0] = static_cast<Particle *>(p[0]);
  p_[1] = static_cast<Particle *>(p[1]);
  p_[2] = static_cast<Particle *>(p[2]);
  p_[3] = static_cast<Particle *>(p[3]);
  // create von Mises
  double kappaval = Scale(kappa_).get_scale();
  mises_ = new vonMisesSufficient(0, N, R0, chiexp, kappaval);
  // mises_->set_was_used(true);
}

TALOSRestraint::TALOSRestraint(Model *m, Particle *p1,
                               Particle *p2, Particle *p3,
                               Particle *p4, Floats data,
                               Particle *kappa)
    : Restraint(m, "TALOSRestraint%1%"), kappa_(kappa) {
  p_[0] = static_cast<Particle *>(p1);
  p_[1] = static_cast<Particle *>(p2);
  p_[2] = static_cast<Particle *>(p3);
  p_[3] = static_cast<Particle *>(p4);
  // create von Mises
  double kappaval = Scale(kappa_).get_scale();
  mises_ = new vonMisesSufficient(0, data, kappaval);
  // mises_->set_was_used(true);
}

TALOSRestraint::TALOSRestraint(Model *m, Particle *p1,
                               Particle *p2, Particle *p3,
                               Particle *p4, unsigned N, double R0,
                               double chiexp, Particle *kappa)
    : Restraint(m, "TALOSRestraint%1%"), kappa_(kappa) {
  p_[0] = static_cast<Particle *>(p1);
  p_[1] = static_cast<Particle *>(p2);
  p_[2] = static_cast<Particle *>(p3);
  p_[3] = static_cast<Particle *>(p4);
  // create von Mises
  double kappaval = Scale(kappa_).get_scale();
  mises_ = new vonMisesSufficient(0, N, R0, chiexp, kappaval);
  // mises_->set_was_used(true);
}

//! Calculate the score for this dihedral restraint.
/** \param[in] accum If not nullptr, use this object to accumulate
    partial first
    derivatives.
    \return Current score.
 */
double TALOSRestraint::unprotected_evaluate(DerivativeAccumulator *accum)
    const {
  core::XYZ d0(p_[0]);
  core::XYZ d1(p_[1]);
  core::XYZ d2(p_[2]);
  core::XYZ d3(p_[3]);
  Scale kappascale(kappa_);
  double kappaval = kappascale.get_scale();

  // get angle
  algebra::VectorD<3> derv0, derv1, derv2, derv3;
  double angle;
  if (accum) {
    angle = core::internal::dihedral(d0, d1, d2, d3, &derv0, &derv1, &derv2,
                                     &derv3);
  } else {
    angle = core::internal::dihedral(d0, d1, d2, d3, nullptr, nullptr, nullptr,
                                     nullptr);
  }

  // score current angle
  mises_->set_x(angle);
  mises_->set_kappa(kappaval);
  double score = mises_->evaluate();

  // store derivatives if necessary
  if (accum) {

    double deriv = mises_->evaluate_derivative_x();
    d0.add_to_derivatives(derv0 * deriv, *accum);
    d1.add_to_derivatives(derv1 * deriv, *accum);
    d2.add_to_derivatives(derv2 * deriv, *accum);
    d3.add_to_derivatives(derv3 * deriv, *accum);

    kappascale.add_to_scale_derivative(mises_->evaluate_derivative_kappa(),
                                       *accum);
  }
  return score;
}

ModelObjectsTemp TALOSRestraint::do_get_inputs() const {
  ParticlesTemp ret(p_, p_ + 4);
  ret.push_back(kappa_);
  return ret;
}

IMPISD_END_NAMESPACE
