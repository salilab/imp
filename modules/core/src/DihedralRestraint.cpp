/**
 *  \file DihedralRestraint.cpp \brief Dihedral restraint between four
 *                                     particles.
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/core/DihedralRestraint.h>
#include <IMP/core/XYZ.h>
#include <IMP/core/internal/dihedral_helpers.h>
#include <IMP/algebra/Vector3D.h>

#include <IMP/Particle.h>
#include <IMP/Model.h>
#include <IMP/log.h>

#include <boost/tuple/tuple.hpp>

IMPCORE_BEGIN_NAMESPACE

DihedralRestraint::DihedralRestraint(Model *m, UnaryFunction* score_func,
                    ParticleIndexAdaptor p1,
                    ParticleIndexAdaptor p2,
                    ParticleIndexAdaptor p3,
                    ParticleIndexAdaptor p4)
    : Restraint(m, "DihedralRestraint%1%"), score_func_(score_func) {
  p_[0] = p1;
  p_[1] = p2;
  p_[2] = p3;
  p_[3] = p4;
}

//! Calculate the score for this dihedral restraint.
/** \param[in] accum If not nullptr, use this object to accumulate partial first
                     derivatives.
    \return Current score.
 */
double DihedralRestraint::unprotected_evaluate(DerivativeAccumulator* accum)
    const {
  IMP_CHECK_OBJECT(score_func_);
  Model *m = get_model();
  XYZ d0(m, p_[0]);
  XYZ d1(m, p_[1]);
  XYZ d2(m, p_[2]);
  XYZ d3(m, p_[3]);

  Float score;

  if (accum) {
    algebra::Vector3D derv0, derv1, derv2, derv3;
    double angle =
        internal::dihedral(d0, d1, d2, d3, &derv0, &derv1, &derv2, &derv3);

    Float deriv;
    boost::tie(score, deriv) = score_func_->evaluate_with_derivative(angle);

    d0.add_to_derivatives(derv0 * deriv, *accum);
    d1.add_to_derivatives(derv1 * deriv, *accum);
    d2.add_to_derivatives(derv2 * deriv, *accum);
    d3.add_to_derivatives(derv3 * deriv, *accum);
  } else {
    double angle =
        internal::dihedral(d0, d1, d2, d3, nullptr, nullptr, nullptr, nullptr);
    score = score_func_->evaluate(angle);
  }
  return score;
}

ModelObjectsTemp DihedralRestraint::do_get_inputs() const {
  ModelObjectsTemp ret(4);
  for (unsigned i = 0; i < 4; ++i) {
    ret[i] = get_model()->get_particle(p_[i]);
  }
  return ret;
}

IMPCORE_END_NAMESPACE
