/**
 *  \file MultipleBinormalRestraint.cpp
 *  \brief Modeller-style multiple binormal (phi/psi) restraint.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/modeller/MultipleBinormalRestraint.h>
#include <IMP/constants.h>
#include <IMP/core/XYZ.h>
#include <IMP/core/internal/dihedral_helpers.h>

#include <cmath>

IMPMODELLER_BEGIN_NAMESPACE

namespace {
  // RT in kcal/mol
  const double RT = 0.5900991;
}

double BinormalTerm::evaluate(const double dihedral[2], double &sin1,
                              double &sin2, double &cos1, double &cos2,
                              double &rho) const
{
  sin1 = std::sin(dihedral[0] - means_.first);
  sin2 = std::sin(dihedral[1] - means_.second);
  cos1 = std::cos(dihedral[0] - means_.first);
  cos2 = std::cos(dihedral[1] - means_.second);

  rho = 1.0 - correlation_ * correlation_;

  double arg = ((1.0 - cos1) / stdevs_.first / stdevs_.first
                + (1.0 - cos2) / stdevs_.second / stdevs_.second
                - correlation_ * sin1 * sin2 / stdevs_.first / stdevs_.second)
               / rho;

  // Avoid underflow in exp
  arg = std::min(arg, 80.0);

  return weight_ / (2.0 * PI * stdevs_.first * stdevs_.second * std::sqrt(rho))
         * std::exp(-arg);
}

MultipleBinormalRestraint::MultipleBinormalRestraint(
    const ParticleQuad &q1, const ParticleQuad &q2) :
    kernel::Restraint(q1[0]->get_model(), "MultipleBinormalRestraint%1%"),
    terms_(), q1_(q1),
    q2_(q2)
{
}

double MultipleBinormalRestraint::unprotected_evaluate(
                                DerivativeAccumulator *accum) const
{
  core::XYZ d0[4], d1[4];
  for (int i = 0; i < 4; ++i) {
    d0[i] = core::XYZ(q1_[i]);
    d1[i] = core::XYZ(q2_[i]);
  }

  double all_terms = 0.;

  if (accum) {
    Float dihedral[2];
    algebra::Vector3D derv0[4], derv1[4];
    dihedral[0] = core::internal::dihedral(d0[0], d0[1], d0[2], d0[3],
                                           &derv0[0], &derv0[1],
                                           &derv0[2], &derv0[3]);
    dihedral[1] = core::internal::dihedral(d1[0], d1[1], d1[2], d1[3],
                                           &derv1[0], &derv1[1],
                                           &derv1[2], &derv1[3]);

    double all_derivs1 = 0., all_derivs2 = 0.;
    for (std::vector<BinormalTerm>::const_iterator term = terms_.begin();
         term != terms_.end(); ++term) {
      double sin1, sin2, cos1, cos2, rho;
      double thisterm = term->evaluate(dihedral, sin1, sin2, cos1, cos2, rho);

      all_terms += thisterm;

      double deriv_weight = thisterm / rho;
      all_derivs1 += deriv_weight / term->stdevs_.first
                     * (sin1 / term->stdevs_.first - term->correlation_
                        * cos1 * sin2 / term->stdevs_.second);
      all_derivs2 += deriv_weight / term->stdevs_.second
                     * (sin2 / term->stdevs_.second - term->correlation_
                        * cos2 * sin1 / term->stdevs_.first);
    }
    all_derivs1 *= RT / all_terms;
    all_derivs2 *= RT / all_terms;
    for (int i = 0; i < 4; ++i) {
      d0[i].add_to_derivatives(all_derivs1 * derv0[i], *accum);
      d1[i].add_to_derivatives(all_derivs2 * derv1[i], *accum);
    }
  } else {
    Float dihedral[2];
    dihedral[0] = core::internal::dihedral(d0[0], d0[1], d0[2], d0[3],
                                           nullptr, nullptr, nullptr, nullptr);
    dihedral[1] = core::internal::dihedral(d1[0], d1[1], d1[2], d1[3],
                                           nullptr, nullptr, nullptr, nullptr);

    for (std::vector<BinormalTerm>::const_iterator term = terms_.begin();
         term != terms_.end(); ++term) {
      double sin1, sin2, cos1, cos2, rho;
      all_terms += term->evaluate(dihedral, sin1, sin2, cos1, cos2, rho);
    }
  }

  return -RT * std::log(all_terms);
}

ModelObjectsTemp MultipleBinormalRestraint::do_get_inputs() const {
  ModelObjectsTemp r(8);
  r[0] = q1_[0]; r[1] = q1_[1]; r[2] = q1_[2]; r[3] = q1_[3];
  r[4] = q2_[0]; r[5] = q2_[1]; r[6] = q2_[2]; r[7] = q2_[3];
  return r;
}

IMPMODELLER_END_NAMESPACE
