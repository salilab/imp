/**
 *  \file NormalMover.h
 *  \brief A modifier which perturbs a point with a normal distribution.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPCORE_NORMAL_MOVER_H
#define IMPCORE_NORMAL_MOVER_H

#include "config.h"
#include "MoverBase.h"

IMPCORE_BEGIN_NAMESPACE

//! Modify a set of continuous variables using a normal distribution.
/** \see MonteCarlo
 */
class IMPCOREEXPORT NormalMover :public MoverBase
{
public:
  /**  \param[in] sc The set of particles to perturb.
       \param[in] vars The variables to use (normally the keys for x,y,z)
       \param[in] sigma The standard deviation to use.
   */
  NormalMover(SingletonContainer *sc,
              const FloatKeys &vars,
              Float sigma);
  /** */
  void set_sigma(Float sigma) {
    IMP_check(sigma > 0, "Sigma must be positive",
              ValueException);
    stddev_=sigma;
  }
  /** */
  Float get_sigma() const {
    return stddev_;
  }
protected:
  virtual void generate_move(float f);
private:
  Float stddev_;
};

IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_NORMAL_MOVER_H */
