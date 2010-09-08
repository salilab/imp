/**
 *  \file IMP/scoped.h    \brief Various general useful functions for IMP.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMP_SCOPED_H
#define IMP_SCOPED_H

#include "macros.h"
#include "kernel_config.h"
#include "RestraintSet.h"
#include "FailureHandler.h"

IMP_BEGIN_NAMESPACE


//! Removes the ScoreState when the RAII object is destroyed
/** It is templated so it can act as a general pointer
    to the score state.
*/template <class SS>
class GenericScopedScoreState {
  Pointer<SS> ss_;
public:
  IMP_RAII(GenericScopedScoreState, (SS *ss, Model *m),{}, {
      ss_=ss;
      m->add_score_state(ss);
    }, {
      if (ss_ && ss_->get_has_model()) {
        IMP_CHECK_OBJECT(ss_);
        IMP_CHECK_OBJECT(ss_->get_model());
        ss_->get_model()->remove_score_state(ss_);
        ss_=NULL;
      }
    });
  bool get_is_set() const {return ss_;}
#ifndef SWIG
  const SS* operator->() const {return ss_;}
  const SS& operator*() const {return *ss_;}
  SS* operator->() {return ss_;}
  SS& operator*() {return *ss_;}
#endif
  IMP_SHOWABLE_INLINE(GenericScopedScoreState, {
      if (ss_) out << "(Scoped " <<ss_->get_name() << ")";
      else out << "(Unset scoped score state)";
    });
};

//! Removes the Restraint when the RAII object is destroyed
/** It is templated so it can act as a general pointer
    to the restraint.
*/
template <class SS>
class GenericScopedRestraint {
  Pointer<SS> ss_;
  Pointer<RestraintSet> rs_;
public:
  IMP_RAII(GenericScopedRestraint, (SS *ss, RestraintSet *rs),{}, {
      ss_=ss;
      rs_=rs;
      rs_->add_restraint(ss);
    }, {
      if (ss_ && ss_->get_is_part_of_model()) {
        IMP_CHECK_OBJECT(ss_);
        IMP_CHECK_OBJECT(ss_->get_model());
        rs_->remove_restraint(ss_);
        ss_=NULL;
        rs_=NULL;
      }
    });
  bool get_is_set() const {return ss_;}
#ifndef SWIG
  const SS* operator->() const {return ss_;}
  const SS& operator*() const {return *ss_;}
  SS* operator->() {return ss_;}
  SS& operator*() {return *ss_;}
#endif
  IMP_SHOWABLE_INLINE(GenericScopedRestraint, {
      if (ss_) out << "(Scoped " <<ss_->get_name() << ")";
      else out << "(Unset scoped restraint)";
    });
};

//! Removes the Restraint until RAII object is destroyed
/** It is templated so it can act as a general pointer
    to the restraint.
*/
template <class SS>
class GenericScopedRemoveRestraint {
  Pointer<SS> ss_;
  Pointer<RestraintSet> rs_;
public:
  IMP_RAII(GenericScopedRemoveRestraint, (SS *ss, RestraintSet *rs),{}, {
      ss_=ss;
      rs_=rs;
      rs_->remove_restraint(ss);
    }, {
      if (rs_ && rs_->get_is_part_of_model()) {
        IMP_CHECK_OBJECT(ss_);
        IMP_CHECK_OBJECT(rs_->get_model());
        rs_->add_restraint(ss_);
        ss_=NULL;
        rs_=NULL;
      }
    });
  bool get_is_set() const {return ss_;}
#ifndef SWIG
  const SS* operator->() const {return ss_;}
  const SS& operator*() const {return *ss_;}
  SS* operator->() {return ss_;}
  SS& operator*() {return *ss_;}
#endif
  IMP_SHOWABLE_INLINE(GenericScopedRemoveRestraint, {
      if (ss_) out << "(Scoped removal of " <<ss_->get_name() << ")";
      else out << "(Unset scoped restraint)";
    });
};

//! Remove a score state when the object goes out of scope
typedef GenericScopedScoreState<ScoreState> ScopedScoreState;
//! Remove a restraint when the object goes out of scope
typedef GenericScopedRestraint<Restraint> ScopedRestraint;
//! Remove a restraint until the object goes out of scope
typedef GenericScopedRemoveRestraint<Restraint> ScopedRemoveRestraint;




//! Control a scope-dependent failure handler
/** The failure handler is added on construction and removed
    on destruction.
*/
class ScopedFailureHandler {
  FailureHandler* fh_;
public:
  IMP_RAII(ScopedFailureHandler, (FailureHandler *fh),
           {fh_=NULL;},
           {
             fh_=fh;
             if (fh_) add_failure_handler(fh_);
           },
           {
             if (fh_) remove_failure_handler(fh_);
             fh_=NULL;
           }
           );
};

IMP_END_NAMESPACE

#endif  /* IMP_SCOPED_H */
