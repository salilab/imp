/**
 *  \file NormalMover.h
 *  \brief A modifier which perturbs a point with a normal distribution.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMPCORE_NORMAL_MOVER_H
#define __IMPCORE_NORMAL_MOVER_H

#include "core_exports.h"
#include "MoverBase.h"

IMPCORE_BEGIN_NAMESPACE

//! Modify a set of continuous variables using a normal distribution.
/** \ingroup mover
 */
class IMPCOREEXPORT NormalMover :public MoverBase
{
public:
  /** \param[in] vars The variables to use (normally the keys for x,y,z)
      \param[in] sigma The standard deviation to use.
      \param[in] ps The particles to perturb.
   */
  NormalMover(const FloatKeys &vars,
              Float sigma, const Particles &ps=Particles());

  void set_sigma(Float sigma) {
    IMP_check(sigma > 0, "Sigma must be positive",
              ValueException);
    stddev_=sigma;
  }
  Float get_sigma() const {
    return stddev_;
  }
protected:
  /** \internal */
  virtual void generate_move(float f);
private:
  Float stddev_;
};

IMPCORE_END_NAMESPACE

#endif  /* __IMPCORE_NORMAL_MOVER_H */
