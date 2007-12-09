/**
 *  \file RestraintSet.h     \brief Used to hold a set of related restraints.
 *
 *  Copyright 2007 Sali Lab. All rights reserved.
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

  //! The type to use to retrieve a restraint
  typedef int RestraintIndex;

  //! Set weight for all restraints contained by this set.
  /** \param[in] weight The new value of the weight.
    */
  void set_weight(Float weight) { weight_ = weight; }

  //! Add restraint to the restraint set.
  /** \param[in] restraint The restraint to add to the restraint set.
      \return the index of the newly-added restraint in the restraint set.
   */
  RestraintIndex add_restraint(Restraint *restraint);

  //! Access a restraint in the restraint set.
  /** \param[in] i The RestraintIndex of the restraint to retrieve.
      \exception std::out_of_range the index is out of range.
      \return Pointer to the Restraint.
   */
  Restraint *get_restraint(RestraintIndex i) const;

  //! Return the total number of restraints
  unsigned int number_of_restraints() const {
    return restraints_.size();
  }

  //! Called when at least one particle has been inactivated.
  /** Check each restraint to see if it changes its active status.
   */
  virtual void check_particles_active();

protected:

  //! Restraints to evaluate.
  /** These can be accessed with an iterator by a filter.
      In the case where the restraint is a single simple restraint,
      this vector contains a pointer to itself.
   */
  std::vector<Restraint *> restraints_;

  //! Weight for all restraints.
  Float weight_;
};

} // namespace IMP

#endif  /* __IMP_RESTRAINT_SET_H */
