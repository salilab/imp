/**
 *  \file DihedralQuadScore.cpp
 *  \brief A Score on the dihedral angle between four particles.
 *
 *  Copyright 2007-2026 IMP Inventors. All rights reserved.
 */

#include <IMP/core/DihedralQuadScore.h>
#include <IMP/core/XYZ.h>
#include <IMP/core/internal/dihedral_helpers.h>
#include <IMP/algebra/Vector3D.h>

#include <IMP/UnaryFunction.h>
#include <boost/tuple/tuple.hpp>
#include <cmath>

IMPCORE_BEGIN_NAMESPACE

DihedralQuadScore::DihedralQuadScore(UnaryFunction *f) : f_(f) {}

Float DihedralQuadScore::evaluate_index(Model *m,
                                        const ParticleIndexQuad &pi,
                                        DerivativeAccumulator *da) const {
  IMP_CHECK_OBJECT(f_.get());
  XYZ d0 = XYZ(m, std::get<0>(pi));
  XYZ d1 = XYZ(m, std::get<1>(pi));
  XYZ d2 = XYZ(m, std::get<2>(pi));
  XYZ d3 = XYZ(m, std::get<3>(pi));

  Float score;

  if (da) {
    algebra::Vector3D derv0, derv1, derv2, derv3;
    double angle = internal::dihedral(
                       d0, d1, d2, d3, &derv0, &derv1, &derv2, &derv3);

    Float deriv;
    boost::tie(score, deriv) = f_->evaluate_with_derivative(angle);
    d0.add_to_derivatives(derv0 * deriv, *da);
    d1.add_to_derivatives(derv1 * deriv, *da);
    d2.add_to_derivatives(derv2 * deriv, *da);
    d3.add_to_derivatives(derv3 * deriv, *da);
  } else {
    double angle = internal::dihedral(
                   d0, d1, d2, d3, nullptr, nullptr, nullptr, nullptr);
    score = f_->evaluate(angle);
  }
  return score;
}

ModelObjectsTemp DihedralQuadScore::do_get_inputs(
    Model *m, const ParticleIndexes &pis) const {
  return IMP::get_particles(m, pis);
}

IMP_OBJECT_SERIALIZE_IMPL(IMP::core::DihedralQuadScore);

IMPCORE_END_NAMESPACE
