/**
 *  \file Sigmoid.h    \brief Sigmoid function.
 *
 *  Copyright 2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPATOM_INTERNAL_SIGMOID_H
#define IMPATOM_INTERNAL_SIGMOID_H

#include <IMP/atom/atom_config.h>
#include <IMP/UnaryFunction.h>
#include <IMP/utility.h>

IMPATOM_BEGIN_INTERNAL_NAMESPACE

//! Sigmoid function
class Sigmoid : public UnaryFunction
{
public:
  /** Create with the given mean and the spring constant k */
  Sigmoid(Float E0, Float Zmid, Float n) :
   E0_(E0), Zmid_(Zmid), n_(n) {}

  IMP_UNARY_FUNCTION_INLINE(Sigmoid,
                            E0_ / ( 1.0 + pow(feature / Zmid_, n_) ),
                            -E0_ * n_ * pow(feature / Zmid_, n_) /
                            ( feature * (1.0 + pow(feature / Zmid_, n_))
                            * (1.0 + pow(feature / Zmid_, n_)) ),
                            "Sigmoid: " << E0_ << " and " << Zmid_
                            << " and " << n_ << std::endl);

private:
  Float E0_;
  Float Zmid_;
  Float n_;
};

IMPATOM_END_INTERNAL_NAMESPACE

#endif  /* IMPATOM_INTERNAL_SIGMOID_H */
