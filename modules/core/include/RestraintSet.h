/**
 *  \file RestraintSet.h     \brief Used to hold a set of related restraints.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPCORE_RESTRAINT_SET_H
#define IMPCORE_RESTRAINT_SET_H

#include "config.h"
#include "internal/version_info.h"

#include <IMP/Restraint.h>

#include <string>

IMPCORE_BEGIN_NAMESPACE

//! Container used to hold a set of restraints
/**
 */
class IMPCOREEXPORT RestraintSet : public Restraint
{
public:
  //! Create an empty set
  RestraintSet(const std::string& name=std::string());
  ~RestraintSet();

  IMP_RESTRAINT(internal::version_info)

  IMP_LIST(public, Restraint, restraint, Restraint*);

 public:

  //! Set weight for all restraints contained by this set.
  /** Setting the weight to 0 disables the restraints in the set.

      \param[in] weight The new value of the weight.
    */
  void set_weight(Float weight) { weight_ = weight; }

  //! Get weight for all restraints contained by this set.
  Float get_weight() const { return weight_; }

  virtual ParticlesList get_interacting_particles() const;

private:

  //! Weight for all restraints.
  Float weight_;

  std::string name_;
};

IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_RESTRAINT_SET_H */
