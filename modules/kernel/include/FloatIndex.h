/**
 *  \file IMP/FloatIndex.h
 *  \brief A FloatIndex identifies an optimized attribute in a model.
 *
 *  Copyright 2007-2026 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_FLOAT_INDEX_H
#define IMPKERNEL_FLOAT_INDEX_H

#include <IMP/kernel_config.h>
#include "base_types.h"
#include "particle_index.h"
#include <IMP/tuple_macros.h>

IMPKERNEL_BEGIN_NAMESPACE

class Model;

//! A FloatIndex identifies an optimized attribute in a model.
/** This can be a single Float attribute, or a single element in an
    optimized Vector3D or Vector4D, depending on the constructor used.
 */
class IMPKERNELEXPORT FloatIndex : public Value {
  FloatKey fk_;
  Vector3DDerivKey v3k_;
  Vector4DDerivKey v4k_;
  int dim_;
  int index_;
  ParticleIndex pi_;
public:
  FloatIndex(IMP::ParticleIndex pi, IMP::FloatKey k)
      : fk_(k), dim_(0), pi_(pi) {}

  FloatIndex(IMP::ParticleIndex pi, IMP::Vector3DDerivKey k, int index)
      : v3k_(k), dim_(3), index_(index), pi_(pi) {}

  FloatIndex(IMP::ParticleIndex pi, IMP::Vector4DDerivKey k, int index)
      : v4k_(k), dim_(4), index_(index), pi_(pi) {}

  FloatKey get_float_key() const { return fk_; }

  void set_value(Model *m, Float v) const;

  Float get_value(Model *m) const;

  Float get_derivative(Model *m) const;

  IMP_SHOWABLE_INLINE(FloatIndex, out << "(" << pi_ << ")");
};

IMP_VALUES(FloatIndex, FloatIndexes);

IMPKERNEL_END_NAMESPACE

#endif /* IMPKERNEL_FLOAT_INDEX_H */
