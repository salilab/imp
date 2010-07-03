/**
 *  \file Model.h   \brief Storage of a model, its restraints,
 *                         constraints and particles.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMP_MODEL_H
#define IMP_MODEL_H

#include "kernel_config.h"
#include "Object.h"
#include "Particle.h"
#include "Restraint.h"
#include "RestraintSet.h"
#include "ScoreState.h"
#include "container_macros.h"
#include "base_types.h"
#include "VersionInfo.h"
#include <boost/dynamic_bitset.hpp>
#include <boost/graph/adjacency_list.hpp>


#include <limits>

IMP_BEGIN_NAMESPACE

class Particle;


//! Class for storing model, its restraints, constraints, and particles.
/** The Model maintains a standard \imp container for each of Particle,
    ScoreState and Restraint object types.

    \note Think carefully about using the iterators over the entire set
          of Particles or Restraints. Most operations should be done using
          a user-passed set of Particles or Restraints instead.
 */
class IMPEXPORT Model: public Object
{
 public:
#if !defined(SWIG) && !defined(IMP_DOXYGEN)
  enum Stage {NOT_EVALUATING, BEFORE_EVALUATE, EVALUATE, AFTER_EVALUATE};
#endif
private:
  friend class Restraint;
  friend class Particle;
  friend class RestraintSet;
  typedef Particle::Storage ParticleStorage;


  // basic representation
  ParticleStorage particles_;
  bool incremental_update_;
  // true if a regular evaluate needs to be called first
  bool first_incremental_;
  bool last_had_derivatives_;
  std::map<FloatKey, FloatRange> ranges_;
  mutable Stage cur_stage_;
  unsigned int eval_count_;
  internal::OwnerPointer<RestraintSet> rs_;



  // statistics
  bool gather_statistics_;
  void add_to_update_before_time(ScoreState *s, double t) const;
  void add_to_update_after_time(ScoreState *s, double t) const;
  void add_to_restraint_evaluate(Restraint *r, double t, double score) const;



  // evaluation
  void validate_incremental_evaluate(const RestraintsTemp &restraints,
                                     const std::vector<double> &weights,
                                     bool calc_derivs,
                                     double score);
  void validate_computed_derivatives() const;
  void before_evaluate(const ScoreStatesTemp &states) const;
  void after_evaluate(const ScoreStatesTemp &states, bool calc_derivs) const;
  void zero_derivatives(bool shadow_too=false) const;
  double do_evaluate(const RestraintsTemp &restraints,
                     const std::vector<double> &weights,
                     const ScoreStatesTemp &states, bool calc_derivs);
  enum WhichRestraints {ALL, INCREMENTAL, NONINCREMENTAL};
  double do_evaluate_restraints(const RestraintsTemp &restraints,
                                const std::vector<double> &weights,
                                bool calc_derivs,
                                WhichRestraints incremental_restraints,
                                bool incremental_evaluation) const;



  // dependencies
  mutable std::map<Restraint *, int> restraint_index_;
  mutable RestraintsTemp ordered_restraints_;
  mutable std::vector<boost::dynamic_bitset<> > restraint_dependencies_;
  mutable std::vector<double> restraint_weights_;
  mutable ScoreStatesTemp ordered_score_states_;
  void compute_dependencies() const;
  bool get_has_dependencies() const {
    return (!ordered_restraints_.empty()
            || get_number_of_restraints() ==0)
      && ordered_score_states_.size()
      == get_number_of_score_states();
  }
  void reset_dependencies();


  // other
  /* Allow Model::ScoreStateDataWrapper class to call the private
     ScoreState::set_model() function (older g++ and MSVC do not support
     member classes as friends) */
  static void set_score_state_model(ScoreState *ss, Model *model);

  void add_particle_internal(Particle *p) {
    IMP_CHECK_OBJECT(this);
    IMP_CHECK_OBJECT(p);
    p->set_was_used(true);
    particles_.push_back(p);
    p->ps_->iterator_= --particles_.end();
    internal::ref(p);
    // particles will not be backed up properly, so don't do incremental
    first_incremental_=true;
    if (get_is_incremental()) {
      p->setup_incremental();
    }
  }


  void do_show(std::ostream& out) const;

#if defined(SWIG)
 public:
#else
 private:
  IMP_NO_DOXYGEN(template <class T> friend void IMP::internal::unref(T*));
#endif

  virtual ~Model();
public:
#if !defined(SWIG) && !defined(IMP_DOXYGEN)
  Stage get_stage() const {
    return cur_stage_;
  }
  unsigned int get_evaluation() const {
    IMP_USAGE_CHECK(get_stage() != NOT_EVALUATING,
                    "Can only call get_evaluation() during evaluation");
    return eval_count_;
  }
#endif

  /** Construct an empty model */
  Model(std::string name="The Model");


  /** @name States

      The Model stores a list of ScoreStates which are given an
      opportunity to update the stored Particles and their internal
      state before and after the restraints are evaluated. Use the
      methods below to manipulate the list of ScoreState objects.

      The value type for the iterators is a ScoreState*.

      \note The order of addition of ScoreState objects does not matter.

      \advancedmethod
  */
  /**@{*/
  IMP_LIST(public, ScoreState, score_state, ScoreState*, ScoreStates);
  /**@}*/

  /** @name Restraints

      The Model scores the current configuration using the stored Restraint
      objects. Use the methods below to manipulate the list.

      The value type for the iterators is a Restraint*.
   */
  /**@{*/
  void add_restraint(Restraint *r);
  void remove_restraint(Restraint *r);
  unsigned int get_number_of_restraints() const {
    return rs_->get_number_of_restraints();
  }
  Restraint *get_restraint(unsigned int i) const {
    return rs_->get_restraint(i);
  }
#ifndef SWIG
  typedef RestraintSet::RestraintIterator RestraintIterator;
  RestraintIterator restraints_begin();
  RestraintIterator restraints_end();
  typedef RestraintSet::RestraintConstIterator RestraintConstIterator;
  RestraintConstIterator restraints_begin() const;
  RestraintConstIterator restraints_end() const;
#endif
  double get_weight(Restraint *r) const;
  RestraintSet *get_root_restraint_set() const {
    return rs_;
  }
  /**@}*/
 public:

  //! Remove the particle from this model
  /** Since particles are ref counted the object will still be valid
      until all references are removed, however attributes of
      removed particles cannot be changed or inspected.

      \note It is an error to remove particles from the model during
      Restraint evaluation or ScoreState evaluation. It is OK to
      remove them during OptimizerState updates, although not all
      optimizers support this yet.

  */
  void remove_particle(Particle *p) {
    IMP_OBJECT_LOG;
    IMP_CHECK_OBJECT(this);
    IMP_CHECK_OBJECT(p);
    IMP_USAGE_CHECK(p->get_model() == this,
              "The particle does not belong to this model");
    IMP_LOG(VERBOSE, "Removing particle " << p->get_name()
            << std::endl);
    IMP_INTERNAL_CHECK(get_stage() == Model::NOT_EVALUATING,
               "Particles cannot be removed from the model during evaluation");
    particles_.erase(p->ps_->iterator_);
    p->m_=NULL;
    internal::unref(p);
  }

  //! Sometimes it is useful to put a particle back into a model
  /** When restoring the state of a Model, it is useful to be able to restore
      a particle back to the model it used to be part of.
  */
  void restore_particle(Particle *p) {
    add_particle_internal(p);
  }

  /** @name Methods to debug particles
      It is sometimes useful to inspect the list of all particles when
      debugging. These methods allow you to do that.
      \note Only use this if you really know what you are doing as
      Particles can be added to the object from many different places.

      The value type for the iterators is a Particle*.
   */
  /**@{*/
  unsigned int get_number_of_particles() const {
    return particles_.size();
  }
#ifdef IMP_DOXYGEN
  class ParticleInterator; class ParticleConstIterator;
#else
 typedef ParticleStorage::const_iterator ParticleConstIterator;
 typedef ParticleStorage::iterator ParticleIterator;
#endif
  ParticleIterator particles_begin() {
    return particles_.begin();
  }
  ParticleIterator particles_end() {
    return particles_.end();
  }

#ifndef SWIG
  ParticleConstIterator particles_begin() const {
    return particles_.begin();
  }
  ParticleConstIterator particles_end() const {
    return particles_.end();
  }
#endif

  /** @} */

  /** @name Float Attribute Ranges
      Each Float attribute has an associated range which reflects the
      range of values that it is expected to take on during optimization.
      The optimizer can use these ranges to make the optimization process
      more efficient. By default, the range estimates are simply the
      range of values for that attribute in the various particles, but
      it can be set to another value. For example, an attribute storing
      an angle could have the range set to (0,PI).

      The ranges are not enforced; they are just guidelines. In order to
      enforce ranges, see, for example,
      IMP::example::ExampleSingletonModifier.
      @{
  */
  FloatRange get_range(FloatKey k) const;

  void set_range(FloatKey k, FloatRange range) {
    ranges_[k]=range;
  }
  /** @} */

  /** \name Evaluation

      Evaluation proceeds as follows:
      - ScoreState::before_evaluate() is called on all ScoreStates
      - Restraint::evaluate() is called on all Restraints
      - ScoreState::after_evaluate() is called on all ScoreStates
      The sum of the Restraint::evaluate() return values is returned.

      All evaluate calls throw a ModelException if a Particle attribute
      value becomes invalid (NaN, infinite etc.)

      @{
  */
  //! Evaluate all of the restraints in the model and return the score.
  /** \param[in] calc_derivs If true, also evaluate the first derivatives.
      \return The score.
  */
  virtual double evaluate(bool calc_derivs);

#ifndef IMP_DOXYGEN
  //! Evaluate a subset of the restraints
  /** The passed restraints must have been added to this model already.

      \note Not all ScoreStates are updated during this call, only the
      ones which are needed to make sure the particles read by the
      restraints are up-to-date. The list of ScoreStates that needs to
      be updated for each restraint is currently recomputed when new
      score states are added, but not when the dependencies of
      Restraints or ScoreStates change. This can be fixed if requested.
  */
  double evaluate(const RestraintsTemp &restraints, bool calc_derivs);
#endif

 //! Sometimes it is useful to be able to make sure the model is up to date
 /** This method updates all the state but does not necessarily compute the
     score.
 */
 void update();

#ifndef IMP_DOXYGEN
  VersionInfo get_version_info() const {
    IMP_CHECK_OBJECT(this);
    return IMP::get_module_version_info();
  }

  std::string get_type_name() const {
    return "Model";
  }
#endif

  /** \name Incremental Updates

      Control whether incremental updates are being used. See
      the \ref incremental "incremental updates" page for a more
      detailed description.
      @{
  */
  /** Turn on or off incremental evaluation. */
  void set_is_incremental(bool tf);

  bool get_is_incremental() const {
    return incremental_update_;
  }
  /** @} */


  /** \name Statistics

      The Model can gather various statistics about the restraints and
      score states used. To use this feature, first turn on statistics
      gather and then run your optimization (or just call evaluate).

      \note Telling the model not to gather statistics does not clear
      existing statistics.
      @{
  */
  void set_gather_statistics(bool tf);
  void show_statistics_summary(std::ostream &out=std::cout) const;
  /** @} */
};

IMP_OUTPUT_OPERATOR(Model);

#ifndef IMP_DOXYGEN
// these require Model be defined

inline void Particle::assert_values_mutable() const {
  IMP_INTERNAL_CHECK(get_model()->get_stage() != Model::EVALUATE,
             "Restraints are not allowed to change attribute values during "
             << "evaluation.");
  IMP_INTERNAL_CHECK(get_model()->get_stage() != Model::AFTER_EVALUATE,
             "ScoreStates are not allowed to change attribute values after "
             << "evaluation.");
#if IMP_BUILD < IMP_FAST
  IMP_IF_CHECK(USAGE_AND_INTERNAL) {
    if (ps_->write_locked_) throw internal::WriteLockedParticleException(this);
  }
#endif
}

inline void Particle::assert_values_readable() const {
#if IMP_BUILD < IMP_FAST
  IMP_IF_CHECK(USAGE_AND_INTERNAL) {
    if (ps_->read_locked_) throw internal::ReadLockedParticleException(this);
  }
#endif
}

inline void Particle::assert_can_change_optimization() const {
  IMP_INTERNAL_CHECK(get_model()->get_stage() == Model::NOT_EVALUATING,
             "The set of optimized attributes cannot be changed during "
             << "evaluation.");
}

inline void Particle::assert_can_change_derivatives() const {
  IMP_INTERNAL_CHECK(get_model()->get_stage() == Model::EVALUATE
             || get_model()->get_stage() == Model::AFTER_EVALUATE
             || get_model()->get_stage() == Model::NOT_EVALUATING,
             "Derivatives can only be changed during restraint "
             << "evaluation and score state after evaluation calls.");
}

inline void Particle::assert_valid_derivatives() const {
  IMP_INTERNAL_CHECK(get_model()->get_stage() == Model::AFTER_EVALUATE
             || get_model()->get_stage() == Model::NOT_EVALUATING,
             "Derivatives can only be changed during restraint "
             << "evaluation and score state after evaluation calls.");
}
#endif

IMP_OBJECTS(Model,Models);




/** \brief A directed graph on the interactions between the various objects in
    the model.

    The vertices are named by the associated Object*. There
    is an edge from a to b, if a is an input to b. For example, there
    is an edge from a particle to a restraint if the restraint directly
    reads the particle.

    See \ref graphs "Graphs in IMP" for more information.
*/
IMP_GRAPH(DependencyGraph, bidirectional, Object*, int);

/** The dependency graph captures the interactions between Restraint,
    ScoreState and Particle objects. The graph has an edge if the source
    of the edge is an input for the target of the edge. eg, there
    is an edge connecting a Container to the Restraint which gets
    its particles from the Container. In order for a given
    Restraint to be evaluated properly, all of the Particles connected
    by a path to the Restraint must be up to date.
*/
IMPEXPORT DependencyGraph
get_dependency_graph(const ScoreStatesTemp &ss,
                     const RestraintsTemp &rs);




#ifndef SWIG

//! Removes the ScoreState when the RAII object is destroyed
template <class SS>
class ScoreStatePointer: public RAII {
  Pointer<SS> ss_;
public:
  IMP_RAII(ScoreStatePointer, (SS *ss, Model *m),{}, {
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
  SS* operator->() const {return ss_;}
  SS& operator*() const {return *ss_;}
};
#endif


IMP_END_NAMESPACE

#endif  /* IMP_MODEL_H */
