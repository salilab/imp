/**
 *  \file SphericalRestraint.h   \brief Absolute position restraint.
 *
 *  Optimize based on distance from an absolute position.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMP_SPHERICAL_RESTRAINT_H
#define __IMP_SPHERICAL_RESTRAINT_H

#include <list>

#include "../IMP_config.h"
#include "../UnaryFunctor.h"
#include "../Restraint.h"

namespace IMP
{

//! Restrict particle position based on its distance to a point.
class IMPDLLEXPORT SphericalRestraint : public Restraint
{
public:
  // Create the SphericalRestraint.
  /**
     \param[in] p The particle to restrict.
     \param[in] x The x coordinate to take distance to.
     \param[in] y The y coordinate to take distance to.
     \param[in] z The z coordinate to take distance to.
     \param[in] score_func The scoring function. It is deleted in the
                           destructor.
   */
  SphericalRestraint(Particle* p,
                     Float x, Float y, Float z,
                     UnaryFunctor* score_func);
  virtual ~SphericalRestraint();

  IMP_RESTRAINT("0.5", "Daniel Russel");

protected:
  Float center_[3];
  UnaryFunctor* score_func_;
};

} // namespace IMP

#endif /* __IMP_COORDINATE_RESTRAINT_H */
