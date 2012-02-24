/**
 *  \file Model.h   \brief Storage of a model, its restraints,
 *                         constraints and particles.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_MODEL_H
#define IMPKERNEL_MODEL_H

#include "kernel_config.h"
#include "Object.h"
#include "Pointer.h"
#include "Restraint.h"
#include "RestraintSet.h"
#include "ScoreState.h"
#include "container_macros.h"
#include "base_types.h"
#include "VersionInfo.h"
#include "Particle.h"
#include "compatibility/map.h"
#include "compatibility/set.h"
#include "internal/AttributeTable.h"
#include "internal/attribute_tables.h"
#include <boost/iterator/transform_iterator.hpp>
#include <boost/iterator/filter_iterator.hpp>


#include <limits>


IMP_BEGIN_NAMESPACE
#if !defined(SWIG) && !defined(IMP_DOXYGEN)
namespace internal {
  enum Stage {NOT_EVALUATING, BEFORE_EVALUATING, EVALUATING, AFTER_EVALUATING};
}
#endif

/** A structure used for returning restraint statistics from the model.*/
class RestraintStatistics {
#if !defined(IMP_DOXYGEN) && !defined(SWIG)
public:
#endif
  double minimum_score;
  double maximum_score;
  double average_score;
  double last_score;
  double average_time;
public:
  RestraintStatistics(){};
  double get_minimum_score() const {return minimum_score;}
  double get_maximum_score() const {return maximum_score;}
  double get_average_score() const {return average_score;}
  double get_last_score() const {return last_score;}
  double get_average_time() const {return average_time;}
  IMP_SHOWABLE_INLINE(RestraintStatistics, {
      out << "minimum score= " << minimum_score << "\n";
      out << "maximum score= " << maximum_score << "\n";
      out << "average score= " << average_score << "\n";
      out << "last score= " << last_score << "\n";
      out << "average time= " << average_time << "\n";
    });
};

class Model;

/** An evaluation cache stores information for efficient evaluation of
    a set of restraints. Store it an reuse it when you have one or
    more restraints you are evaluating over and over again.
*/
class IMPEXPORT EvaluationCache {
  friend class Model;
  ScoreStates ss_;
  Restraints rs_;
  Floats weights_;

  EvaluationCache(const ScoreStatesTemp &ss,
                  const RestraintsTemp &rs,
                  const Floats &floats);
 public:
  EvaluationCache(){}
  EvaluationCache(const RestraintsTemp &rs);
  EvaluationCache(const RestraintsTemp &rs, double weight);
  RestraintsTemp get_restraints() const {
    return RestraintsTemp(rs_.begin(), rs_.end());
  }
  IMP_SHOWABLE_INLINE(EvaluationCache, out << rs_);
  Model *get_model() const {return rs_[0]->get_model();}
};
IMP_VALUES(EvaluationCache, EvaluationCaches);

IMP_VALUES(RestraintStatistics, RestraintStatisticsList);

//! Class for storing model, its restraints, constraints, and particles.
/** The Model maintains a standard \imp container for each of Particle,
    ScoreState and Restraint object types.

    \note Think carefully about using the iterators over the entire set
          of Particles or Restraints. Most operations should be done using
          a user-passed set of Particles or Restraints instead.
 */
class IMPEXPORT Model:
  public RestraintSet
#if !defined(SWIG) && !defined(IMP_DOXYGEN)
  , public internal::Masks,
  public internal::FloatAttributeTable,
  public internal::StringAttributeTable,
  public internal::IntAttributeTable,
  public internal::ObjectAttributeTable,
  public internal::IntsAttributeTable,
  public internal::ObjectsAttributeTable,
  public internal::ParticleAttributeTable,
  public internal::ParticlesAttributeTable
#endif
{
 public:
#ifndef SWIG
  IMP_MODEL_IMPORT(internal::FloatAttributeTable);
  IMP_MODEL_IMPORT(internal::StringAttributeTable);
  IMP_MODEL_IMPORT(internal::IntAttributeTable);
  IMP_MODEL_IMPORT(internal::ObjectAttributeTable);
  IMP_MODEL_IMPORT(internal::IntsAttributeTable);
  IMP_MODEL_IMPORT(internal::ObjectsAttributeTable);
  IMP_MODEL_IMPORT(internal::ParticleAttributeTable);
  IMP_MODEL_IMPORT(internal::ParticlesAttributeTable);
#endif
  void clear_particle_caches(ParticleIndex pi) {
    internal::FloatAttributeTable::clear_caches(pi);
    internal::StringAttributeTable::clear_caches(pi);
    internal::IntAttributeTable::clear_caches(pi);
    internal::ObjectAttributeTable::clear_caches(pi);
    internal::IntsAttributeTable::clear_caches(pi);
    internal::ObjectsAttributeTable::clear_caches(pi);
    internal::ParticleAttributeTable::clear_caches(pi);
    internal::ParticlesAttributeTable::clear_caches(pi);
  }
private:
  struct Statistics {
    double total_time_;
    double total_time_after_;
    unsigned int calls_;
    double total_value_;
    double min_value_;
    double max_value_;
    double last_value_;
    Statistics(): total_time_(0), total_time_after_(0),
                  calls_(0), total_value_(0),
                  min_value_(std::numeric_limits<double>::max()),
                  max_value_(-std::numeric_limits<double>::max()),
                  last_value_(-1)
    {}
  };
  mutable compatibility::map<Object*, Statistics> stats_data_;

  // basic representation
  std::map<FloatKey, FloatRange> ranges_;
  compatibility::set<Restraint*> tracked_restraints_;
  double max_score_;
  vector<std::pair<Object*, Object*> > extra_edges_;

  ParticleIndexes free_particles_;
  unsigned int next_particle_;
  base::IndexVector<ParticleIndexTag, Pointer<Particle> > particle_index_;
  vector<OwnerPointer<Object> > model_data_;
#if !defined(IMP_DOXYGEN) && !defined(SWIG)
  // things the evaluate template functions need, can't be bothered with friends
public:
  bool first_call_;
  void validate_computed_derivatives() const{}
  void compute_dependencies() const;
  bool get_has_dependencies() const {
    return has_dependencies_;
  }
  mutable internal::Stage cur_stage_;
  unsigned int eval_count_;
  mutable bool has_good_score_;
  void before_evaluate(const ScoreStatesTemp &states) const;
  void after_evaluate(const ScoreStatesTemp &states, bool calc_derivs) const;

  bool gather_statistics_;

  void add_to_restraint_evaluate(Restraint *r, double t, double score) const;
#endif
 private:
  // statistics
  void add_to_update_before_time(ScoreState *s, double t) const;
  void add_to_update_after_time(ScoreState *s, double t) const;



  // dependencies
  mutable bool has_dependencies_;
  mutable RestraintsTemp scoring_restraints_;
  mutable ScoreStatesTemp ordered_score_states_;

  // other
  /* Allow Model::ScoreStateDataWrapper class to call the private
     ScoreState::set_model() function (older g++ and MSVC do not support
     member classes as friends) */
  static void set_score_state_model(ScoreState *ss, Model *model);

  void do_show(std::ostream& out) const;

#if defined(SWIG)
 public:
#else
 private:
#ifndef IMP_DOXYGEN
  template <class T, class E> friend struct base::internal::RefStuff;
#endif
#endif

  virtual ~Model();
public:
#if !defined(IMP_DOXYGEN)
#ifndef SWIG
  internal::Stage get_stage() const {
    return cur_stage_;
  }
  unsigned int get_evaluation() const {
    IMP_USAGE_CHECK(get_stage() != internal::NOT_EVALUATING,
                    "Can only call get_evaluation() during evaluation");
    return eval_count_;
  }
#endif
  // It is annoying to get the friend call right for VC
  void reset_dependencies();
  ScoreStatesTemp get_score_states(const RestraintsTemp &rs) const;
  ScoreStatesTemp get_ordered_score_states() const {
    return ordered_score_states_;
  }
#endif

  /** Construct an empty model */
  Model(std::string name="Model %1%");


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
  IMP_LIST_ACTION(public, ScoreState, ScoreStates,
                  score_state, score_states, ScoreState*, ScoreStates,
              {IMP_INTERNAL_CHECK(cur_stage_== internal::NOT_EVALUATING,
                     "The set of score states cannot be changed during"
                                  << "evaluation.");
                Model::set_score_state_model(obj, this);
                obj->set_was_used(true);
                IMP_LOG(VERBOSE, "Added score state " << obj->get_name()
                        << std::endl);
                IMP_IF_CHECK(USAGE) {
                  compatibility::set<ScoreState*> in(score_states_begin(),
                                           score_states_end());
                  IMP_USAGE_CHECK(in.size() == get_number_of_score_states(),
                                  "Score state already in model "
                                  << obj->get_name());
                }
              },{reset_dependencies();},
              {Model::set_score_state_model(obj, nullptr);
               if(container) container->reset_dependencies(); });
  /**@}*/

  /** @name Restraints

      The Model scores the current configuration using the stored Restraint
      objects. Use the methods below to manipulate the list.

      The value type for the iterators is a Restraint*.
   */
  /**@{*/
  double get_weight(Restraint *r) const;
  RestraintSet *get_root_restraint_set() const {
    return const_cast<Model*>(this);
  }
  /**@}*/
#ifndef IMP_DOXYGEN
 /** \name Tracked restraints

     All restraints are in this list, whether or not they are
     part of the scoring function.
  */
  void add_tracked_restraint(Restraint *r);
  void remove_tracked_restraint(Restraint *r);
  bool get_is_tracked_restraint(Restraint *r) const;
  RestraintsTemp get_tracked_restraints() const;
  /** @} */
#endif
 public:

  /** \name Filtering
      We are typically only interested in "good" conformations of
      the model. These are described by specifying maximum scores
      per restraint (Restraint::set_maximum_score())
      and for the whole model. Samplers, optimizers
      etc are free to ignore configurations they encounter which
      go outside these bounds.
      @{
  */
#if !defined(IMP_DOXYGEN)
  double get_maximum_score(Restraint *r) const {
    return r->get_maximum_score();
  }
  void set_maximum_score(Restraint *r, double s) {
    r->set_maximum_score(s);
  }
#endif
  void set_maximum_score(double s);
  double get_maximum_score() const {
    return max_score_;
  }
  //! Return true if thelast evaluate satisfied the thresholds
  /** Currently this ignores maximum scores on restraint sets. Sorry.
   */
  bool get_has_good_score() const;
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

  /** Use this to simplify code.*/
  EvaluationCache get_evaluation_cache() const;


  //! Evaluate a subset of the restraints
  /** The passed restraints must have been added to this model already
      and must not be RestraintSets.

      \note Not all ScoreStates are updated during this call, only the
      ones which are needed to make sure the particles read by the
      restraints are up-to-date. The list of ScoreStates that needs to
      be updated for each restraint is currently recomputed when new
      score states are added, but not when the dependencies of
      Restraints or ScoreStates change. This can be fixed if requested.
  */
  Floats evaluate( const EvaluationCache&cache,
                   bool calc_derivs);

  //! Evaluate a subset of the restraints
  /** In contrast to other evaluate methods,
      this method is free to shortcut evaluation and return a very
      large score if the total score at any point exceeds max or if
      any of the restraints exceed their Restraint::get_maximum().

      See evaluate(RestraintsTemp,Floats,bool) for more
      information.
  */
  Floats evaluate_if_good( const EvaluationCache &cache,
                           bool calc_derivs);
  /** Evaluate, returning the score if it below the max value.
      Otherwise return a number above max. The restraint maxima
      are ignored.
  */
  Floats evaluate_if_below( const EvaluationCache &cache,
                            bool calc_derivs, double max);


  //! Sometimes it is useful to be able to make sure the model is up to date
  /** This method updates all the state but does not necessarily compute the
      score.
  */
  void update();

#ifndef IMP_DOXYGEN
  VersionInfo get_version_info() const {
    IMP_CHECK_OBJECT(this);
    return IMP::VersionInfo("IMP", get_module_version());
  }

  std::string get_type_name() const {
    return "Model";
  }
#endif

  void remove_particle(Particle *p);

  /** \name Statistics

      The Model can gather various statistics about the restraints and
      score states used. To use this feature, first turn on statistics
      gather and then run your optimization (or just call evaluate).

      \note Telling the model not to gather statistics does not clear
      existing statistics.
      @{
  */
  void clear_all_statistics();
  void set_gather_statistics(bool tf);
  void show_all_statistics(std::ostream &out=std::cout) const;
  void show_restraint_time_statistics(std::ostream &out=std::cout) const;
  void show_restraint_score_statistics(std::ostream &out=std::cout) const;
  RestraintStatistics get_restraint_statistics(Restraint *r) const;
  void show_score_state_time_statistics(std::ostream &out=std::cout) const;
  /** @} */

  /** \name Model Data
      Arbitrary non-particle data can be associated with the Model. This can
      be used for Model-level caches and such.
      @{ */
  void add_data(ModelKey mk, Object *o);
  Object *get_data(ModelKey mk) const;
  void remove_data(ModelKey mk);
  bool get_has_data(ModelKey mk) const;
  /** @} */

#ifndef IMP_DOXYGEN
  /** Sometimes there are dependencies among score states that require
      an ordering that cannot be derived automatically. For example score
      states that read and write the same set of particles and have to
      do so in a certain order.
  */
  void add_dependency_edge(ScoreState *from, ScoreState *to);
#endif

#if !defined(IMP_DOXYGEN) && !defined(SWIG)
  const vector<std::pair<Object*, Object*> >&
    get_extra_dependency_edges() const {
    return extra_edges_;
  }
  void add_particle_internal(Particle *p, bool set_name);
#endif
/** @name Methods to debug particles
      It is sometimes useful to inspect the list of all particles when
      debugging. These methods allow you to do that.
      \note Only use this if you really know what you are doing as
      Particles can be added to the object from many different places.

      The value type for the iterators is a Particle*.
      @{
   */
  unsigned int get_number_of_particles() const {
    return get_particles().size();
  }
  ParticlesTemp get_particles() const;
  Particle* get_particle(ParticleIndex p) const  {
    IMP_USAGE_CHECK(particle_index_.size() > get_as_unsigned_int(p),
                    "Invalid particle requested");
    IMP_USAGE_CHECK(particle_index_[p],
                    "Invalid particle requested");
    return particle_index_[p];
  }
#ifndef SWIG
#ifdef IMP_DOXYGEN
  class ParticleIterator;
#else
  struct NotNull{
    bool operator()(const Pointer<Particle>& p) {
      return p;
    }
  };
  typedef boost::filter_iterator<NotNull,
                                 compatibility
                                 ::vector<Pointer<Particle> >
                                 ::const_iterator> ParticleIterator;

#endif
  ParticleIterator particles_begin() const {
    return ParticleIterator(NotNull(), particle_index_.begin(),
                            particle_index_.end());
  }
  ParticleIterator particles_end() const {
    return ParticleIterator(NotNull(), particle_index_.end(),
                            particle_index_.end());
}
#endif
  /** @} */
};

IMP_OUTPUT_OPERATOR(Model);

IMP_OBJECTS(Model,Models);

#ifndef IMP_DOXYGEN

IMP_PARTICLE_ATTRIBUTE_TYPE_DEF(Float, float, Float);
IMP_PARTICLE_ATTRIBUTE_TYPE_DEF(Int, int, Int);
IMP_PARTICLE_ATTRIBUTE_TYPE_DEF(String, string, String);
IMP_PARTICLE_ATTRIBUTE_TYPE_DEF(Object, object, Object*);


inline void Particle::add_attribute(FloatKey name,
                                    const Float initial_value, bool optimized){
  IMP_USAGE_CHECK(get_is_active(), "Inactive particle used.");
  get_model()->add_attribute(name, id_, initial_value);
  get_model()->set_is_optimized(name, id_, optimized);
}
inline void Particle::add_to_derivative(FloatKey key, Float value,
                                        const DerivativeAccumulator &da) {
  IMP_USAGE_CHECK(get_is_active(), "Inactive particle used.");
  get_model()->add_to_derivative(key, id_, value, da);
}
inline void Particle::set_is_optimized(FloatKey k, bool tf) {
  IMP_USAGE_CHECK(get_is_active(), "Inactive particle used.");
  return get_model()->set_is_optimized(k, id_, tf);
}
inline bool Particle::get_is_optimized(FloatKey k) const {
  IMP_USAGE_CHECK(get_is_active(), "Inactive particle used.");
  return get_model()->get_is_optimized(k, id_);
}
inline Float Particle::get_derivative(FloatKey name) const {
  IMP_USAGE_CHECK(get_is_active(), "Inactive particle used.");
  return get_model()->get_derivative(name, id_);
}
inline void Particle::add_attribute(ParticleKey k, Particle *v) {
  IMP_USAGE_CHECK(get_is_active(), "Inactive particle used.");
  get_model()->add_attribute(k, id_, v->get_index());
}
inline bool Particle::has_attribute(ParticleKey k) {
  IMP_USAGE_CHECK(get_is_active(), "Inactive particle used.");
  return get_model()->get_has_attribute(k, id_);
}
inline void Particle::set_value(ParticleKey k, Particle *v) {
  IMP_USAGE_CHECK(get_is_active(), "Inactive particle used.");
  get_model()->set_attribute(k, id_, v->get_index());
}
inline Particle *Particle::get_value(ParticleKey k) const {
  IMP_USAGE_CHECK(get_is_active(), "Inactive particle used.");
  return get_model()->get_particle(get_model()->get_attribute(k, id_));
}
inline void Particle::remove_attribute(ParticleKey k) {
  IMP_USAGE_CHECK(get_is_active(), "Inactive particle used.");
  get_model()->remove_attribute(k, id_);
}
inline ParticleKeys Particle::get_particle_keys() const {
  IMP_USAGE_CHECK(get_is_active(), "Inactive particle used.");
  return get_model()->internal::ParticleAttributeTable::get_attribute_keys(id_);
}

#endif
IMP_END_NAMESPACE

#endif  /* IMPKERNEL_MODEL_H */
