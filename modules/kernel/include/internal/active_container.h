/**
 *  \file container_helpers.h
 *  \brief Internal helpers for container classes.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_INTERNAL_ACTIVE_CONTAINER_H
#define IMPKERNEL_INTERNAL_ACTIVE_CONTAINER_H

#include "../ScoreState.h"


IMP_BEGIN_INTERNAL_NAMESPACE


#define IMP_ACTIVE_CONTAINER_DECL(Name)                         \
  private:                                                      \
  unsigned int eval_update_;                                    \
  class Ticker: public ScoreState {                             \
    Name *back_;                                                \
  public:                                                       \
  Ticker(Name *n): ScoreState(n->get_name()+" updater"),        \
                   back_(n){                                    \
    n->eval_update_=std::numeric_limits<unsigned int>::max();   \
  }                                                             \
  IMP_SCORE_STATE(Ticker);                                      \
  };                                                            \
  friend class Ticker;                                          \
  GenericScopedScoreState<Ticker> ticker_;                      \
  unsigned int get_last_update_evaluation() const {             \
  return eval_update_;                                          \
  }                                                             \
  virtual void do_before_evaluate();                            \
  virtual void do_after_evaluate();                             \
  bool get_has_model() const { return ticker_.get_is_set();}    \
  virtual ParticlesTemp get_state_input_particles() const;      \
  virtual ContainersTemp get_state_input_containers() const;    \
  void initialize_active_container(Model *m);                   \
public:                                                         \
 void set_log_level(LogLevel l)

#define IMP_ACTIVE_CONTAINER_DEF(Name, extra_objects_log)               \
  void Name::Ticker::do_before_evaluate() {                             \
    IMP_CHECK_OBJECT(back_);                                            \
    back_->do_before_evaluate();                                        \
    back_->eval_update_= back_->get_model()->get_evaluation();          \
  }                                                                     \
  void Name::Ticker::do_after_evaluate(DerivativeAccumulator*) {        \
    IMP_CHECK_OBJECT(back_);                                            \
    back_->do_after_evaluate();                                         \
  }                                                                     \
  ContainersTemp Name::Ticker::get_input_containers() const {           \
    IMP_IF_CHECK(USAGE) {                                               \
      std::string name= back_->get_name();                              \
      {Pointer<Name> rc(back_);}                                        \
      IMP_INTERNAL_CHECK((back_)->get_is_valid(),                       \
                         "Container " << name                           \
                         << " is not ref counted properly"              \
                         << " bad things will happen.");                \
    }                                                                   \
    return back_->get_state_input_containers();                         \
  }                                                                     \
  ContainersTemp Name::Ticker::get_output_containers() const {          \
    return ContainersTemp(1, back_);                                    \
  }                                                                     \
  ParticlesTemp Name::Ticker::get_input_particles() const {             \
    return back_->get_state_input_particles();                          \
  }                                                                     \
  ParticlesTemp Name::Ticker::get_output_particles() const {            \
    return ParticlesTemp();                                             \
  }                                                                     \
  void Name::Ticker::do_show(std::ostream &out) const {                 \
    out << "back is " << *back_ << std::endl;                           \
  }                                                                     \
  void Name::initialize_active_container(Model *m) {                    \
    IMP_INTERNAL_CHECK(!get_has_model(),                                \
                       "Can only set the model once.");                 \
    IMP_LOG(TERSE, "Setting up score state for container "              \
            << get_name()<< std::endl);                                 \
    IMP_INTERNAL_CHECK(get_model(),                                     \
                       "No active updating of add/remove containers."); \
    ticker_.set(new Ticker(this), m);                                   \
  }                                                                     \
  void Name::set_log_level(LogLevel l) {                                \
    Object::set_log_level(l);                                           \
    ticker_->set_log_level(l);                                          \
    extra_objects_log;                                                  \
  }                                                                     \
  IMP_REQUIRE_SEMICOLON_NAMESPACE




IMP_END_INTERNAL_NAMESPACE

#endif /* IMPKERNEL_INTERNAL_ACTIVE_CONTAINER_H */
