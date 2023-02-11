/**
 *  \file Cosine.cpp  \brief Cosine function.
 *
 *  Copyright 2007-2022 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/core/Cosine.h>

#include <cmath>

IMPCORE_BEGIN_NAMESPACE

double Cosine::evaluate(double feature) const {
  return std::abs(force_constant_) -
         force_constant_ * std::cos(periodicity_ * feature + phase_);
}

DerivativePair Cosine::evaluate_with_derivative(double feature) const {
  Float deriv = force_constant_ * periodicity_ *
                std::sin(periodicity_ * feature + phase_);
  return std::make_pair(evaluate(feature), deriv);
}

void Cosine::do_show(std::ostream &out) const {
  out << "amplitude " << force_constant_ << "\nperiodicity " << periodicity_
      << "\nphase " << phase_ << std::endl;
}

IMPCORE_END_NAMESPACE

// Allow polymorphic serialization of Cosine to/from binary archives
#include <boost/serialization/export.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>

#ifndef SWIG
BOOST_CLASS_EXPORT_GUID(IMP::core::Cosine, "core.Cosine")
#endif
