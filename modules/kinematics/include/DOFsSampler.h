/**
 * \file kinematics/DOFsSampler.h \brief
 *
 * Copyright 2007-2010 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPKINEMATICS_DO_FS_SAMPLER_H
#define IMPKINEMATICS_DO_FS_SAMPLER_H

#include "kinematics_config.h"
#include "DOFValues.h"

IMPKINEMATICS_BEGIN_NAMESPACE

/**
   a base class for sampling certain combinations of degrees of freedom
 */
class DOFsSampler : public IMP::base::Object {
  IMP_OBJECT_METHODS(DOFsSampler);

 public:
  /** Constructs the dofs sampler over specified dofs
  */
  DOFsSampler(DOFs dofs)
      : IMP::base::Object("IMP_KINEMATICS_DOFSSAMPLER"),
        dofs_(dofs),
        last_sample_(DOFValues(dofs)) {}

  /**
     @return a sample on the dofs over which this class samples
   */
  DOFValues get_sample() const {
    last_sample_ = do_get_sample();
    return last_sample_;
  }

  // TODO: should it be a class function? it is not strictly related to the
  //       class
  /**
     apply set of dof values in values over objects associated with
     this DOFsSampler (e.g., joints)
   */
  virtual void apply(const DOFValues& values) = 0;

  /**
     calls apply(), using values from the last sample returned by
     get_sample() (or values of initial dofs provided to constructor,
     if no sample was taken yet)
   */
  void apply_last_sample() { apply(last_sample_); }

  /**
     make a new sample of the dofs over which this class samples,
     and apply them to objects associated with this sampler (e.g.
     joints)
   */
  void sample_and_apply() { apply(get_sample()); }

  /** Returns the dofs over which this sampler works
   */
  DOFs const& get_dofs() const { return dofs_; }

  /** Returns the i'th dof over which this sampler works
   */
  DOF const* get_dof(unsigned int i) const {
    IMP_USAGE_CHECK(i < dofs_.size(),
                    "Accessing out-of-range dof in DOFsSampler");
    return dofs_[i];
  }

  unsigned int get_number_of_dofs() const { return dofs_.size(); }

 protected:
  virtual DOFValues do_get_sample() const = 0;

 private:
  //! DOFs over which to sample
  DOFs dofs_;

  //! It is assumed that get_sample() implementations set this to its return
  //! value
  mutable DOFValues last_sample_;
};

IMP_OBJECTS(DOFsSampler, DOFsSamplers);

IMPKINEMATICS_END_NAMESPACE

#endif /* IMPKINEMATICS_DO_FS_SAMPLER_H */
