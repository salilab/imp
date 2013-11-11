/**
 *  \file IMP/kernel/scoped.h
 *  \brief Various general useful functions for IMP.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_SCOPED_H
#define IMPKERNEL_SCOPED_H

#include <IMP/kernel/kernel_config.h>
#include "RestraintSet.h"
#include "ScoreState.h"
#include "Model.h"
#include <IMP/base/RAII.h>
#include <IMP/base/deprecation.h>
#include <IMP/base/deprecation_macros.h>
#include <IMP/base/Pointer.h>
#include <IMP/base/raii_macros.h>
#include <IMP/base/check_macros.h>
#include <IMP/base/log_macros.h>

IMPKERNEL_BEGIN_NAMESPACE

/** Add a cache attribute to a particle and then remove it
    when this goes out of scope.
*/
template <class Key, class Value>
class ScopedAddCacheAttribute : public base::RAII {
  base::Pointer<Model> m_;
  ParticleIndex pi_;
  Key key_;

 public:
  IMP_RAII(ScopedAddCacheAttribute, (Particle* p, Key key, const Value& value),
           { pi_ = base::get_invalid_index<ParticleIndexTag>(); },
           {
             m_ = p->get_model();
             pi_ = p->get_index();
             key_ = key;
             m_->add_cache_attribute(key_, pi_, value);
           },
           {
             if (pi_ != base::get_invalid_index<ParticleIndexTag>()) {
               m_->remove_attribute(key_, pi_);
             }
           }, );
};

/** Set an attribute to a given value and restore the old
    value when this goes out of scope..
*/
template <class Key, class Value>
class ScopedSetAttribute : public base::RAII {
  base::Pointer<Model> m_;
  ParticleIndex pi_;
  Key key_;
  Value old_;

 public:
  IMP_RAII(ScopedSetAttribute, (Particle* p, Key key, const Value& value),
           { pi_ = base::get_invalid_index<ParticleIndexTag>(); },
           {
             m_ = p->get_model();
             pi_ = p->get_index();
             key_ = key;
             old_ = m_->get_attribute(key_, pi_);
             m_->set_attribute(key_, pi_, value);
           },
           {
             if (pi_ != base::get_invalid_index<ParticleIndexTag>()) {
               m_->set_attribute(key_, pi_, old_);
             }
           }, );
};

typedef ScopedSetAttribute<FloatKey, Float> ScopedSetFloatAttribute;

IMPKERNEL_END_NAMESPACE

#endif /* IMPKERNEL_SCOPED_H */
