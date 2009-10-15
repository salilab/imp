/**
 *  \file MaximumChangeScoreState.cpp
 *  \brief Keep track of the maximumimum change of a set of attributes.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 */


#include <IMP/core/MaximumChangeScoreState.h>
#include "IMP/SingletonModifier.h"
#include <IMP/internal/utility.h>

#include <algorithm>
#include <sstream>

IMPCORE_BEGIN_NAMESPACE

namespace {

  class RecordValues:public SingletonModifier {
    std::vector<Floats> &values_;
    FloatKeys &keys_;
    mutable int i_;
  public:
    RecordValues(std::vector<Floats> &values, FloatKeys &keys): values_(values),
                                                                keys_(keys){
      i_=0;
    }
    IMP_INTERNAL_SINGLETON_MODIFIER(RecordValues, get_module_version_info(),
                                    {
    values_[i_].resize(keys_.size());
    for (unsigned int i=0; i< keys_.size(); ++i) {
      values_[i_][i]= p->get_value(keys_[i]);
    }
    ++i_;
  })
  };

  class CompareValues:public SingletonModifier {
    std::vector<Floats> &values_;
    FloatKeys &keys_;
    mutable double change_;
    mutable int i_;
  public:
    CompareValues(std::vector<Floats> &values,
                  FloatKeys &keys): values_(values),
                                    keys_(keys){
      change_=0;
      i_=0;
    }
    IMP_INTERNAL_SINGLETON_MODIFIER(CompareValues, get_module_version_info(),
                                    {
                                      for (unsigned int i=0;
                                           i< keys_.size(); ++i) {
                                        change_= std::max(change_,
                           std::abs(values_[i_][i]-p->get_value(keys_[i])));
                                      }
                                      ++i_;
                                    });
    double get_change() const {return change_;}
  };
}

MaximumChangeScoreState::MaximumChangeScoreState(SingletonContainer *pc,
                                                 const FloatKeys &keys):
  keys_(keys), pc_(pc)
{
  reset();
}

void MaximumChangeScoreState::do_before_evaluate()
{
  IMP_CHECK_OBJECT(pc_);
  if (rev_ != pc_->get_revision()) {
    reset();
    maximum_change_= std::numeric_limits<double>::max();
  } else {
    IMP_NEW(CompareValues,  cv, (orig_values_, keys_));
    pc_->apply(cv);
    maximum_change_= cv->get_change();
  }
  IMP_LOG(TERSE, "MaximumChange update got " << maximum_change_ << std::endl);
}

void MaximumChangeScoreState::do_after_evaluate(DerivativeAccumulator *) {
}


void MaximumChangeScoreState::reset()
{
  maximum_change_=0;
  orig_values_.clear();
  orig_values_.resize(pc_->get_number_of_particles());
  IMP_NEW(RecordValues, rv, (orig_values_, keys_));
  pc_->apply(rv);
  rev_=pc_->get_revision();
}

ParticlesList MaximumChangeScoreState::get_interacting_particles() const {
  return ParticlesList();
}

ParticlesTemp MaximumChangeScoreState::get_read_particles() const {
  return ParticlesTemp(pc_->particles_begin(), pc_->particles_end());
}

ParticlesTemp MaximumChangeScoreState::get_write_particles() const {
  return ParticlesTemp();
}

void MaximumChangeScoreState::show(std::ostream &out) const
{
  out << "MaximumChangeScoreState" << std::endl;
}

IMPCORE_END_NAMESPACE
