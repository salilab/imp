/**
 *  \file NormalMover.h    
 *  \brief A modifier which perturbs a point with a gaussian.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMP_NORMAL_MOVER_H
#define __IMP_NORMAL_MOVER_H

#include "../../IMP_config.h"
#include "../MoverBase.h"

namespace IMP
{

//! Modify a set of continuous variables.
/** The variables are perturbed within a ball of the 
    given radius.
    \ingroup mover
 */
class IMPDLLEXPORT NormalMover :public MoverBase
{
public:
  NormalMover(const Particles &pis, const FloatKeys &vars,
              Float stdev);
  void set_particles(const Particles &ps) {
    MoverBase::clear_particles();
    MoverBase::add_particles(ps);
  }
protected:
   virtual void generate_move(float f);
private:
  Float stddev_;
};

} // namespace IMP

#endif  /* __IMP_NORMAL_MOVER_H */
