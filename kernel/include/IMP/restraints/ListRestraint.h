/**
 *  \file ListRestraint.h   
 *  \brief Apply a SingletonScore to each particle in a list.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMP_LIST_RESTRAINT_H
#define __IMP_LIST_RESTRAINT_H

#include <iostream>

#include "../IMP_config.h"
#include "../Restraint.h"

namespace IMP
{

class SingletonScore;

//! Applies a SingletonScore to each Particle in a list.
/** \ingroup restraint
 */
class IMPDLLEXPORT ListRestraint : public Restraint
{
public:
  //! Create the list restraint.
  /** \param[in] ps The list of particles to use in the restraint.
      \param[in] ss The function to apply to each particle.
   */
  ListRestraint(const Particles &ps, SingletonScore *ss);
  virtual ~ListRestraint(){}

  IMP_RESTRAINT("0.5", "Daniel Russel");

  using Restraint::add_particles;
  using Restraint::clear_particles;

protected:
  std::auto_ptr<SingletonScore> ss_;
};

} // namespace IMP

#endif  /* __IMP_LIST_RESTRAINT_H */
