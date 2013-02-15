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


//! Removes the ScoreState when the RAII object is destroyed
/** It is templated so it can act as a general pointer
    to the score state.
*/
template <class SS>
class GenericScopedScoreState: public base::RAII {
  base::Pointer<SS> ss_;
public:
  IMP_RAII(GenericScopedScoreState, (SS *ss, Model *m),{}, {
      ss_=ss;
      m->add_score_state(ss);
    }, {
      if (ss_ && ss_->get_is_part_of_model()) {
        IMP_CHECK_OBJECT(ss_);
        IMP_CHECK_OBJECT(ss_->get_model());
        ss_->get_model()->remove_score_state(ss_);
        ss_=nullptr;
      }
    },{
      if (ss_) out << "(Scoped " <<ss_->get_name() << ")";
      else out << "(Unset scoped score state)";
    });
  bool get_is_set() const {return ss_;}
#ifndef SWIG
  const SS* operator->() const {return ss_;}
  const SS& operator*() const {return *ss_;}
  SS* operator->() {return ss_;}
  SS& operator*() {return *ss_;}
#endif
};

//! Removes the Restraint when the RAII object is destroyed
/** It is templated so it can act as a general pointer
    to the restraint.
    \deprecated{With the advent of the ScoringFunction, this
    should not be needed.}
*/
template <class SS>
class GenericScopedRestraint: public base::RAII {
  base::Pointer<SS> ss_;
  base::Pointer<RestraintSet> rs_;
public:
  IMP_RAII(GenericScopedRestraint, (SS *ss, RestraintSet *rs),{}, {
      ss_=ss;
      rs_=rs;
      rs_->add_restraint(ss);
      IMP_DEPRECATED_CLASS(GenericScopedRestraint, ScoringFunction);
    }, {
      if (ss_ && ss_->get_is_part_of_model()) {
        IMP_CHECK_OBJECT(ss_);
        IMP_CHECK_OBJECT(ss_->get_model());
        rs_->remove_restraint(ss_);
        ss_=nullptr;
        rs_=nullptr;
      }
    }, {
      if (ss_) out << "(Scoped " <<ss_->get_name() << ")";
      else out << "(Unset scoped restraint)";
    });
  bool get_is_set() const {return ss_;}
#ifndef SWIG
  const SS* operator->() const {return ss_;}
  const SS& operator*() const {return *ss_;}
  SS* operator->() {return ss_;}
  SS& operator*() {return *ss_;}
#endif
};

//! Removes the Restraint until RAII object is destroyed
/** It is templated so it can act as a general pointer
    to the restraint.
    \deprecated{With the advent of the ScoringFunction, this
    should not be needed.}
*/
template <class SS>
class GenericScopedRemoveRestraint: public base::RAII {
  base::Pointer<SS> ss_;
  base::Pointer<RestraintSet> rs_;
  void cleanup() {
    if (rs_ && rs_->get_is_part_of_model()) {
        IMP_LOG_VERBOSE( "Restoring restraint "
                << ss_->get_name() << " to "
                << rs_->get_name() << std::endl);
        IMP_CHECK_OBJECT(ss_);
        IMP_CHECK_OBJECT(rs_->get_model());
        rs_->add_restraint(ss_);
        ss_=nullptr;
        rs_=nullptr;
      } else if (ss_) {
        IMP_LOG_VERBOSE( "Not restoring restraint "
                << ss_->get_name() << std::endl);
      }
  }
  void setup(Restraint* ss, RestraintSet *rs) {
    ss_=ss;
    rs_=rs;
    rs_->remove_restraint(ss);
    IMP_LOG_VERBOSE( "Removing restraint "
            << ss_->get_name() << " from "
            << rs_->get_name() << std::endl);
  }
public:
  IMP_RAII(GenericScopedRemoveRestraint, (SS *ss, RestraintSet *rs),{}, {
      setup(ss, rs);
      IMP_DEPRECATED_CLASS(GenericScopedRestraint, ScoringFunction);
    }, {
      cleanup();
    }, {
      if (ss_) out << "(Scoped removal of " <<ss_->get_name() << ")";
      else out << "(Unset scoped restraint)";
    });
  bool get_is_set() const {return ss_;}
#ifndef SWIG
  const SS* operator->() const {return ss_;}
  const SS& operator*() const {return *ss_;}
  SS* operator->() {return ss_;}
  SS& operator*() {return *ss_;}
#endif
};


//! Removes the ScoreState until RAII object is destroyed
/** It is templated so it can act as a general pointer
    to the restraint.
*/
template <class SS>
class GenericScopedRemoveScoreState: public base::RAII {
  base::Pointer<SS> ss_;
  base::Pointer<Model> rs_;
  void cleanup() {
    if (rs_) {
        IMP_LOG_VERBOSE( "Restoring restraint "
                << ss_->get_name() << " to "
                << rs_->get_name() << std::endl);
        IMP_CHECK_OBJECT(ss_);
        IMP_CHECK_OBJECT(rs_);
        rs_->add_score_state(ss_);
        ss_=nullptr;
        rs_=nullptr;
      }
  }
  void setup(ScoreState* ss, Model *rs) {
    ss_=ss;
    rs_=rs;
    rs_->remove_score_state(ss);
    IMP_LOG_VERBOSE( "Removing restraint "
            << ss_->get_name() << " from "
            << rs_->get_name() << std::endl);
  }
public:
  IMP_RAII(GenericScopedRemoveScoreState, (SS *ss, Model *rs),{}, {
      setup(ss, rs);
    }, {
      cleanup();
    }, {
      if (ss_) out << "(Scoped removal of " <<ss_->get_name() << ")";
      else out << "(Unset scoped restraint)";
    });
  bool get_is_set() const {return ss_;}
#ifndef SWIG
  const SS* operator->() const {return ss_;}
  const SS& operator*() const {return *ss_;}
  SS* operator->() {return ss_;}
  SS& operator*() {return *ss_;}
#endif
};

//! Remove a score state when the object goes out of scope
typedef GenericScopedScoreState<ScoreState> ScopedScoreState;
//! Remove a restraint when the object goes out of scope
typedef GenericScopedRestraint<Restraint> ScopedRestraint;
//! Remove a restraint until the object goes out of scope
typedef GenericScopedRemoveRestraint<Restraint> ScopedRemoveRestraint;
//! Remove a score state until the object goes out of scope
typedef GenericScopedRemoveScoreState<ScoreState> ScopedRemoveScoreState;

/** Add a cache attribute to a particle and then remove it
    when this goes out of scope.
*/
template <class Key, class Value>
class ScopedAddCacheAttribute: public base::RAII {
  base::Pointer<Model> m_;
  ParticleIndex pi_;
  Key key_;
public:
  IMP_RAII(ScopedAddCacheAttribute, (Particle *p,
                                  Key key, const Value &value),
           {pi_=base::get_invalid_index<ParticleIndexTag>();},
           {
             m_=p->get_model();
             pi_=p->get_index();
             key_=key;
             m_->add_cache_attribute(key_, pi_, value);
           },
           {
             if (pi_ !=base::get_invalid_index<ParticleIndexTag>()) {
               m_->remove_attribute(key_, pi_);
             }
           },);
};


/** Set an attribute to a given value and restore the old
    value when this goes out of scope..
*/
template <class Key, class Value>
class ScopedSetAttribute: public base::RAII {
  base::Pointer<Model> m_;
  ParticleIndex pi_;
  Key key_;
  Value old_;
public:
  IMP_RAII(ScopedSetAttribute, (Particle *p,
                                Key key, const Value &value),
           {pi_=base::get_invalid_index<ParticleIndexTag>();},
           {
             m_=p->get_model();
             pi_=p->get_index();
             key_=key;
             old_= m_->get_attribute(key_, pi_);
             m_->set_attribute(key_, pi_, value);
           },
           {
             if (pi_ != base::get_invalid_index<ParticleIndexTag>()) {
               m_->set_attribute(key_, pi_, old_);
             }
           },);
};

typedef ScopedSetAttribute<FloatKey, Float> ScopedSetFloatAttribute;


IMPKERNEL_END_NAMESPACE

#endif  /* IMPKERNEL_SCOPED_H */
