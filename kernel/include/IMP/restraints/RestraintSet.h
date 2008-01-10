/**
 *  \file RestraintSet.h     \brief Used to hold a set of related restraints.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMP_RESTRAINT_SET_H
#define __IMP_RESTRAINT_SET_H

#include <string>

#include "../IMP_config.h"
#include "../Restraint.h"

namespace IMP
{

//! Used to hold a set of related restraints
class IMPDLLEXPORT RestraintSet : public Restraint
{
public:
  RestraintSet(const std::string& name=std::string());
  ~RestraintSet();

  IMP_RESTRAINT("0.5", "Daniel Russel")

  IMP_CONTAINER(Restraint, restraint, RestraintIndex);

 public:

  //! Set weight for all restraints contained by this set.
  /** Setting the weight to 0 disables the restraints in the set.

      \param[in] weight The new value of the weight.
    */
  void set_weight(Float weight) { weight_ = weight; }

  //! Get weight for all restraints contained by this set.
  Float get_weight() { return weight_; }

  //! Called when at least one particle has been inactivated.
  /** Check each restraint to see if it changes its active status.
   */
  virtual void check_particles_active();

protected:

  //! Weight for all restraints.
  Float weight_;

  std::string name_;
};

} // namespace IMP

#endif  /* __IMP_RESTRAINT_SET_H */
