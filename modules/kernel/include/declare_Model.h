/**
 *  \file IMP/kernel/declare_Model.h
 *  \brief Storage of a model, its restraints,
 *                         constraints and particles.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_DECLARE_MODEL_H
#define IMPKERNEL_DECLARE_MODEL_H

#include <IMP/kernel/kernel_config.h>
#include "ModelObject.h"
#include "declare_ScoringFunction.h"
#include "declare_Restraint.h"
#include "declare_RestraintSet.h"
#include "ScoreState.h"
#include "container_macros.h"
#include "base_types.h"
#include "declare_Particle.h"
#include "Undecorator.h"
#include "internal/AttributeTable.h"
#include "internal/attribute_tables.h"
#include <IMP/base/Object.h>
#include <IMP/base/Pointer.h>
#include <IMP/base/tracking.h>
#include <IMP/base/map.h>
#include <IMP/base/set.h>
#include <IMP/base/tuple_macros.h>
#include <boost/iterator/transform_iterator.hpp>
#include <boost/iterator/filter_iterator.hpp>


#include <limits>


IMPKERNEL_BEGIN_NAMESPACE

class ModelObject;
class Undecorator;

#if !defined(SWIG) && !defined(IMP_DOXYGEN)
namespace internal {
  enum Stage {NOT_EVALUATING, BEFORE_EVALUATING, EVALUATING, AFTER_EVALUATING,
              COMPUTING_DEPENDENCIES};
}
#endif

/** A structure used for returning restraint statistics from the model.*/
/** \headerfile Model.h "IMP/Model.h"
 */
IMP_NAMED_TUPLE_5(RestraintStatistics,RestraintStatisticsList,
                  double, minimum_score,
                  double, maximum_score,
                  double, average_score,
                  double, last_score,
                  double, average_time,);

class Model;

//! Class for storing model, its restraints, constraints, and particles.
/** The Model maintains a standard \imp container for each of Particle,
    ScoreState and Restraint object types.

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

    \headerfile Model.h "IMP/Model.h"
 */
class IMPKERNELEXPORT Model:
#ifdef IMP_DOXYGEN
    public base::Object
#else
  public RestraintSet
#endif
#if !defined(SWIG) && !defined(IMP_DOXYGEN)
  ,public base::Tracker<ModelObject>,
  public internal::Masks,
// The attribute tables provide fast access to e.g. particle attributes, etc.
  public internal::FloatAttributeTable,
  public internal::StringAttributeTable,
  public internal::IntAttributeTable,
  public internal::ObjectAttributeTable,
  public internal::WeakObjectAttributeTable,
  public internal::IntsAttributeTable,
  public internal::ObjectsAttributeTable,
  public internal::ParticleAttributeTable,
  public internal::ParticlesAttributeTable
#endif
{
 public:
#if !defined(SWIG) && !defined(IMP_DOXYGEN)
  IMP_MODEL_IMPORT(internal::FloatAttributeTable);
  IMP_MODEL_IMPORT(internal::StringAttributeTable);
  IMP_MODEL_IMPORT(internal::IntAttributeTable);
  IMP_MODEL_IMPORT(internal::ObjectAttributeTable);
  IMP_MODEL_IMPORT(internal::WeakObjectAttributeTable);
  IMP_MODEL_IMPORT(internal::IntsAttributeTable);
  IMP_MODEL_IMPORT(internal::ObjectsAttributeTable);
  IMP_MODEL_IMPORT(internal::ParticleAttributeTable);
  IMP_MODEL_IMPORT(internal::ParticlesAttributeTable);
#endif
  /** Clear all the cache attributes of a given particle.*/
  void clear_particle_caches(ParticleIndex pi) {
    internal::FloatAttributeTable::clear_caches(pi);
    internal::StringAttributeTable::clear_caches(pi);
    internal::IntAttributeTable::clear_caches(pi);
    internal::ObjectAttributeTable::clear_caches(pi);
    internal::WeakObjectAttributeTable::clear_caches(pi);
    internal::IntsAttributeTable::clear_caches(pi);
    internal::ObjectsAttributeTable::clear_caches(pi);
    internal::ParticleAttributeTable::clear_caches(pi);
    internal::ParticlesAttributeTable::clear_caches(pi);
  }
private:
  typedef base::Tracker<ModelObject> ModelObjectTracker;
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
  mutable base::map<base::Object*, Statistics> stats_data_;

  // basic representation
  base::map<FloatKey, FloatRange> ranges_;
  ParticleIndexes free_particles_;
  unsigned int next_particle_;
  base::IndexVector<ParticleIndexTag, base::Pointer<Particle> > particle_index_;
  base::IndexVector<ParticleIndexTag, Undecorators > undecorators_index_;
  base::Vector<base::OwnerPointer<base::Object> > model_data_;
  bool dependencies_dirty_;
  DependencyGraph dependency_graph_;
  DependencyGraphVertexIndex dependency_graph_index_;
  base::map<ModelObject*, ScoreStatesTemp> required_score_states_;
  void compute_required_score_states();
#if !defined(IMP_DOXYGEN) && !defined(SWIG)
  // things the evaluate template functions need, can't be bothered with friends
public:
  bool first_call_;
  void validate_computed_derivatives() const{}
  void compute_dependencies();
  internal::Stage cur_stage_;
  unsigned int eval_count_;
  bool has_good_score_;
  void before_evaluate(const ScoreStatesTemp &states);
  void after_evaluate(const ScoreStatesTemp &states, bool calc_derivs);

  bool gather_statistics_;

  void add_to_restraint_evaluate(Restraint *r, double t, double score) const;
  void clear_caches();
  internal::Stage get_stage() const {
    return cur_stage_;
  }
  unsigned int get_evaluation() const {
    IMP_USAGE_CHECK(get_stage() != internal::NOT_EVALUATING,
                    "Can only call get_evaluation() during evaluation");
    return eval_count_;
  }
  void add_particle_internal(Particle *p, bool set_name);
#endif

 private:
  void cleanup();
  void show_it(std::ostream &out) const;
  // statistics
  void add_to_update_before_time(ScoreState *s, double t) const;
  void add_to_update_after_time(ScoreState *s, double t) const;


  // dependencies
  RestraintsTemp scoring_restraints_;
  ScoreStatesTemp ordered_score_states_;

public:
  /** Construct an empty model */
  Model(std::string name="Model %1%");

  //! Add particle to the model
  ParticleIndex add_particle(std::string name);

#ifndef IMP_DOXYGEN
  const DependencyGraph& get_dependency_graph();
  const DependencyGraphVertexIndex& get_dependency_graph_vertex_index();
  const ScoreStatesTemp& get_required_score_states(ModelObject *o);
#endif

  /** Add the passed Undecorator to the particle.*/
  void add_undecorator(ParticleIndex pi, Undecorator *d);

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
                obj->set_model(this);
                obj->set_was_used(true);
                IMP_LOG_VERBOSE( "Added score state " << obj->get_name()
                        << std::endl);
                IMP_IF_CHECK(base::USAGE) {
                  base::set<ScoreState*> in(score_states_begin(),
                                           score_states_end());
                  IMP_USAGE_CHECK(in.size() == get_number_of_score_states(),
                                  "Score state already in model "
                                  << obj->get_name());
                }
              },{},
              {obj->set_model(nullptr);});
  /**@}*/

 public:

#if !defined(IMP_DOXYGEN)
  ModelObjectsTemp get_optimized_particles() const;

  RestraintSet *get_root_restraint_set();

  bool get_has_dependencies() const;
  double get_maximum_score(Restraint *r) const;
  void set_maximum_score(Restraint *r, double s);
  void set_maximum_score(double s);
  double get_maximum_score() const;

  // make sure the whole model is updated for backwards compat
  /**
     @param tf - calculate derivatives if true
  */
  double evaluate(bool tf, bool warn=true);
#endif

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

  //! Create a scoring function object from the model restraints
  /** Create a scoring function from the model restraints,
      which can be used to evaluate the score over this model
      particles.
   */
  ScoringFunction* create_model_scoring_function();

  //! Sometimes it is useful to be able to make sure the model is up to date
  /** This method updates all the state but does not necessarily compute the
      score. Use this to make sure that your containers and rigid bodies are
      up to date.
  */
  void update();

  IMP_OBJECT_METHODS(Model)

#ifndef IMP_DOXYGEN
  /** Remove a particle from the Model. The particle will then be inactive and
      cannot be used for anything and all data stored in the particle is lost.
  */
    void remove_particle(Particle *p);
  /** Make sure that we don't cache the ScoringFunction so as not to create
      a ref count loop.*/
  virtual ScoringFunction *create_scoring_function(double weight=1.0,
                                                   double max
                                                   = NO_MAX) const IMP_OVERRIDE;
#endif
 /** Remove a particle from the Model. The particle will then be inactive and
      cannot be used for anything and all data stored in the particle is lost.
  */
  void remove_particle(ParticleIndex pi);

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
  bool get_gather_statistics() const {return gather_statistics_;}
  void show_all_statistics(std::ostream &out=std::cout) const;
  void show_restraint_time_statistics(std::ostream &out=std::cout) const;
  void show_restraint_score_statistics(std::ostream &out=std::cout) const;
  RestraintStatistics get_restraint_statistics(Restraint *r) const;
  void show_score_state_time_statistics(std::ostream &out=std::cout) const;
  /** @} */

#ifdef IMP_DOXYGEN
  /** \name Accessing attributes
      All the attribute data associated with each Particle is stored in the
      Model. For each type of attribute, there are the methods detailed below
      (where, eg, TypeKey is FloatKey or StringKey)
      @{
  */
  /** \pre get_has_attribute(attribute_key, particle) is false*/
  void add_attribute(TypeKey attribute_key, ParticleIndex particle, Type value);

  /** \pre get_has_attribute(attribute_key, particle) is true*/
  void remove_attribute(TypeKey attribute_key, ParticleIndex particle);

  bool get_has_attribute(TypeKey attribute_key, ParticleIndex particle) const;

  /** \pre get_has_attribute(attribute_key, particle) is true*/
  void set_attribute(TypeKey attribute_key, ParticleIndex particle, Type value);

  /** \pre get_has_attribute(attribute_key, particle) is true*/
  Type get_attribute(TypeKey attribute_key, ParticleIndex particle);

  /** Cache attributes, unklike normal attributes, can be added during
      evaluation. They are also cleared by the clear_cache_attributes() method.
      Cache attributes should be used when one is adding data to a particle
      to aid scoring (eg cache the rigid body collision acceleration structure).

      When some pertinent aspect of the particle changes, the clear method
      should
      be called (yes, this is a bit vague). Examples where it should be cleared
      include changing the set of members of a core::RigidBody or their
      coordinates, changing the members of an atom::Hierarchy.
  */
  void add_cache_attribute(TypeKey attribute_key, ParticleIndex particle,
                           Type value);

  //! Optimized attributes are the parameters of the model
  /** They will be modified by the samplers and optimizers.
   */
  void set_is_optimized(TypeKey attribute_key, ParticleIndex particle,
                        bool true_or_false);
  /** @} */
#endif

  // kind of icky
#ifdef SWIG
#define IMP_MODEL_ATTRIBUTE_METHODS(Type, Value)                        \
  void add_attribute(Type##Key attribute_key,                           \
                     ParticleIndex particle, Value value);              \
  void remove_attribute(Type##Key attribute_key,                        \
                        ParticleIndex particle);                        \
  bool get_has_attribute(Type##Key attribute_key,                       \
                         ParticleIndex particle) const;                 \
  void set_attribute(Type##Key attribute_key,                           \
                     ParticleIndex particle, Value value);              \
  Value get_attribute(Type##Key attribute_key,                         \
                       ParticleIndex particle);                         \
  void add_cache_attribute(Type##Key attribute_key,                     \
                           ParticleIndex particle,                      \
                           Value value)

  IMP_MODEL_ATTRIBUTE_METHODS(Float, Float);
  IMP_MODEL_ATTRIBUTE_METHODS(Int, Int);
  IMP_MODEL_ATTRIBUTE_METHODS(Ints, Ints);
  IMP_MODEL_ATTRIBUTE_METHODS(String, String);
  IMP_MODEL_ATTRIBUTE_METHODS(ParticleIndexes, ParticleIndexes);
  IMP_MODEL_ATTRIBUTE_METHODS(ParticleIndex, ParticleIndex);
  IMP_MODEL_ATTRIBUTE_METHODS(Object, Object*);
  IMP_MODEL_ATTRIBUTE_METHODS(WeakObject, Object*);
  void set_is_optimized(FloatKey, ParticleIndex, bool);
#endif

  /** \name Model Data
      Arbitrary non-particle data can be associated with the Model. This can
      be used for Model-level caches and such.
      @{ */
  void add_data(ModelKey mk, base::Object *o);
  base::Object *get_data(ModelKey mk) const;
  void remove_data(ModelKey mk);
  bool get_has_data(ModelKey mk) const;
  /** @} */

/** @name Methods to debug particles
      It is sometimes useful to inspect the list of all particles when
      debugging. These methods allow you to do that.
      \note Only use this if you really know what you are doing as
      Particles can be added to the object from many different places.

      The value type for the iterators is a Particle*.
      @{
   */
  unsigned int get_number_of_particles() const;
  ParticlesTemp get_particles() const;
  inline Particle* get_particle(ParticleIndex p) const ;
#ifndef SWIG
#ifdef IMP_DOXYGEN
  class ParticleIterator;
#else
  struct NotNull{
    bool operator()(const base::Pointer<Particle>& p) {
      return p;
    }
  };
  typedef boost::filter_iterator<NotNull,
    base::Vector<base::Pointer<Particle> >
      ::const_iterator> ParticleIterator;

#endif
  ParticleIterator particles_begin() const;
  ParticleIterator particles_end() const;
#endif
  /** @} */
  /** Get all the ModelObjects associated with this Model.
   */
  ModelObjectsTemp get_model_objects() const;
};

IMPKERNEL_END_NAMESPACE

#endif  /* IMPKERNEL_DECLARE_MODEL_H */
