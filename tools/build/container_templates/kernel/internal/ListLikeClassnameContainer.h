/**
 *  \file internal/core_FUNCTIONNAME_helpers.h
 *  \brief A container for Classnames.
 *
 *  BLURB
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPKERNEL_INTERNAL_LIST_LIKE_CLASSNAME_CONTAINER_H
#define IMPKERNEL_INTERNAL_LIST_LIKE_CLASSNAME_CONTAINER_H

#include <IMP/kernel/kernel_config.h>
#include "../ClassnameContainer.h"
#include "../ClassnameModifier.h"
#include "../ClassnameScore.h"
#include "../scoped.h"
#include "TupleRestraint.h"
#include "container_helpers.h"
#include <IMP/base/thread_macros.h>
#include <algorithm>


IMPKERNEL_BEGIN_INTERNAL_NAMESPACE

class IMPKERNELEXPORT ListLikeClassnameContainer: public ClassnameContainer {
private:
  PLURALINDEXTYPE data_;
protected:
  void swap(PLURALINDEXTYPE &cur) {
    Container::set_is_changed(true);
    IMP::base::swap(data_, cur);
  }
  /*  template <class F>
  void remove_from_list_if(F f) {
    data_.erase(std::remove_if(data_.begin(), data_.end(), f), data_.end());
    Container::set_is_changed(true);
    }*/
  ListLikeClassnameContainer(Model *m, std::string name):
    ClassnameContainer(m,name) {
  }
 public:
  template <class F>
  void apply_generic(const F *f) const {
    validate_readable();
    if (base::get_number_of_threads() > 1) {
      unsigned int tasks=2*base::get_number_of_threads();
      unsigned int chunk_size= std::max<unsigned int>(1U, data_.size()/tasks)+1;
      Model *m= get_model();
      for (unsigned int i=0; i< tasks; ++i) {
        unsigned int lb= i*chunk_size;
        unsigned int ub= std::min<unsigned int>(data_.size(),
                                                (i+1) *chunk_size);
        IMP_TASK((lb, ub, m, f),
                 f->apply_indexes(m, data_,lb, ub),
                 "apply");
      }
IMP_OMP_PRAGMA(taskwait)
    } else {
      f->apply_indexes(get_model(), data_,
                       0, data_.size());
    }
  }

  IMP_IMPLEMENT_INLINE(void do_apply(const ClassnameModifier *sm) const, {
    apply_generic(sm);
  });

  IMP_OBJECT(ListLikeClassnameContainer);

  PLURALINDEXTYPE get_indexes() const {
    return data_;
  }
  IMP_IMPLEMENT_INLINE(bool do_get_provides_access() const, {return true;});
  const PLURALINDEXTYPE& get_access() const {
    return data_;
  }

  typedef PLURALINDEXTYPE::const_iterator const_iterator;
  const_iterator begin() const {
    return data_.begin();
  }
  const_iterator end() const {
    return data_.end();
  }
};


IMPKERNEL_END_INTERNAL_NAMESPACE

#define IMP_LISTLIKE_CLASSNAME_CONTAINER(Name)                         \
  public:                                                               \
  ParticleIndexes get_all_possible_indexes() const;                     \
  ParticlesTemp get_input_particles() const;                            \
  ContainersTemp get_input_containers() const;                          \
  ModelObjectsTemp do_get_inputs() const {                              \
    ModelObjects ret;                                                   \
    ret+=get_input_containers();                                        \
    ret+=get_input_particles();                                         \
    return ret;                                                         \
  }                                                                     \
  void do_before_evaluate();                                            \
  PLURALINDEXTYPE get_range_indexes() const;                     \
  IMP_OBJECT(Name)


#define IMP_LISTLIKE_CLASSNAME_CONTAINER_2(Name)                         \
  public:                                                               \
  virtual ParticleIndexes get_all_possible_indexes() const IMP_OVERRIDE; \
  virtual ModelObjectsTemp do_get_inputs() const IMP_OVERRIDE;          \
  virtual void do_before_evaluate() IMP_OVERRIDE;                       \
  virtual PLURALINDEXTYPE get_range_indexes() const IMP_OVERRIDE;       \
  IMP_OBJECT(Name)


#endif  /* IMPKERNEL_INTERNAL_LIST_LIKE_CLASSNAME_CONTAINER_H */
