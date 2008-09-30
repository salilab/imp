/**
 *  \file ConstantRestraint.h    \brief Constant restraint.
 *
 *  Just return a constant.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMP_CONSTANT_RESTRAINT_H
#define __IMP_CONSTANT_RESTRAINT_H

#include "../IMP_config.h"
#include "../Restraint.h"
#include "../internal/kernel_version_info.h"

IMP_BEGIN_NAMESPACE

class PairScore;

//! Return a constant value.
/** This restraint is mostly for testing, but can also be used to make
    the total score look nicer..

    \ingroup restraint
 */
class IMPDLLEXPORT ConstantRestraint : public Restraint
{
  Float v_;
public:
  ConstantRestraint(Float v);

  IMP_RESTRAINT(internal::kernel_version_info)
};

IMP_END_NAMESPACE

#endif /* __IMP_CONSTANT_RESTRAINT_H */
