/**
 *  \file Model.h   \brief Storage of a model, its restraints,
 *                         constraints and particles.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMP_MODEL_H
#define IMP_MODEL_H

#include "kernel_config.h"
#include "Object.h"
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
#include <IMP/algebra/SphereD.h>
#include <boost/iterator/transform_iterator.hpp>
#include <boost/iterator/filter_iterator.hpp>
#include <vector>


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

#ifndef SWIG
#define IMP_MODEL_IMPORT(Base)                  \
  using Base::add_attribute;                    \
  using Base::add_cache_attribute;              \
  using Base::remove_attribute;                 \
  using Base::get_has_attribute;                \
  using Base::set_attribute;                    \
  using Base::get_attribute;                    \
  using Base::access_attribute
#else
#define IMP_MODEL_IMPORT(Base)
#endif


#if !defined(IMP_DOXYGEN) && !defined(SWIG)
template <class Traits>
class BasicAttributeTable {
public:
  typedef typename Traits::Key Key;
private:
  typename std::vector<typename Traits::Container > data_;
  compatibility::set<Key> caches_;

   void do_add_attribute(Key k, ParticleIndex particle,
                           typename Traits::PassValue value) {
     IMP_USAGE_CHECK(Traits::get_is_valid(value), "Can't set to invalid value: "
                     << value << " for attribute " << k);
    if (data_.size() <= k.get_index()) {
      data_.resize(k.get_index()+1);
    }
    if (data_[k.get_index()].size() <= static_cast<unsigned int>(particle)) {
      data_[k.get_index()].resize(particle+1, Traits::get_invalid());
    }
    data_[k.get_index()][particle]=value;
  }
public:
  void swap_with(BasicAttributeTable<Traits> &o) {
    IMP_SWAP_MEMBER(data_);
    IMP_SWAP_MEMBER(caches_);
  }
  BasicAttributeTable(){}

  void add_attribute(Key k, ParticleIndex particle,
                     typename Traits::PassValue value) {
    do_add_attribute(k, particle, value);
  }
  void add_cache_attribute(Key k, ParticleIndex particle,
                           typename Traits::PassValue value) {
    caches_.insert(k);
    do_add_attribute(k, particle, value);
  }
  void clear_caches(ParticleIndex particle) {
    for (typename compatibility::set<Key>::const_iterator it=caches_.begin();
         it != caches_.end(); ++it) {
      if (data_.size() > static_cast<unsigned int>(it->get_index())
          && data_[it->get_index()].size()
          > static_cast<unsigned int>(particle)) {
        data_[it->get_index()][particle]= Traits::get_invalid();
      }
    }
  }
  void remove_attribute(Key k, ParticleIndex particle) {
    IMP_USAGE_CHECK(get_has_attribute(k, particle),
                    "Can't remove attribute if it isn't there");
    data_[k.get_index()][particle]=Traits::get_invalid();
  }
  bool get_has_attribute(Key k, ParticleIndex particle) const {
    if (data_.size() <= k.get_index()) return false;
    else if (data_[k.get_index()].size()
             <= static_cast<unsigned int>(particle)) return false;
    else return Traits::get_is_valid(data_[k.get_index()][particle]);
  }
  void set_attribute(Key k, ParticleIndex particle,
                     typename Traits::PassValue value) {
    IMP_USAGE_CHECK(get_has_attribute(k, particle),
                    "Setting invalid attribute: " << k
                    << " of particle " << particle);
    data_[k.get_index()][particle]= value;
  }
  typename Traits::PassValue get_attribute(Key k,
                                           ParticleIndex particle) const {
    IMP_USAGE_CHECK(get_has_attribute(k, particle),
                    "Requested invalid attribute: " << k
                    << " of particle " << particle);
    return data_[k.get_index()][particle];
  }
  typename Traits::Container::reference access_attribute(Key k,
                                           ParticleIndex particle) {
    IMP_USAGE_CHECK(get_has_attribute(k, particle),
                    "Requested invalid attribute: " << k
                    << " of particle " << particle);
    return data_[k.get_index()][particle];
  }
  std::pair<typename Traits::Value,
            typename Traits::Value> get_range_internal(Key k) const {
    std::pair<typename Traits::Value,
              typename Traits::Value>  ret;
    IMP_USAGE_CHECK(data_.size() > k.get_index(),
                    "Cannot request range of an unused key.");
    if (data_[k.get_index()].size()==0) return ret;
    ret.first=data_[k.get_index()][0];
    ret.second=ret.first;
    for (unsigned int i=1; i< data_[k.get_index()].size(); ++i) {
      if (Traits::get_is_valid(data_[k.get_index()][i])) {
        ret.first=Traits::min(ret.first, data_[k.get_index()][i]);
        ret.second=Traits::max(ret.second, data_[k.get_index()][i]);
      }
    }
    return ret;
  }
  void clear_attributes(ParticleIndex particle) {
    for (unsigned int i=0; i< data_.size(); ++i) {
      if (data_[i].size() > static_cast<unsigned int>(particle)) {
        data_[i][particle]= Traits::get_invalid();
      }
    }
  }

  std::vector<Key> get_attribute_keys(ParticleIndex particle) const {
    std::vector<Key> ret;
    for (unsigned int i=0; i< data_.size(); ++i) {
      if (data_[i].size() > static_cast<unsigned int>(particle)
          && Traits::get_is_valid(data_[i][particle])) {
        ret.push_back(Key(i));
      }
    }
    return ret;
  }
  void fill(typename Traits::PassValue value) {
    for (unsigned int i=0; i< data_.size(); ++i) {
      std::fill(data_[i].begin(), data_[i].end(), value);
    }
  }
  unsigned int size() const {return data_.size();}
  unsigned int size(unsigned int i) const {return data_[i].size();}
};
IMP_SWAP_1(BasicAttributeTable);



class FloatAttributeTable {
  //std::vector<algebra::Sphere3D> spheres_;
  //std::vector<algebra::Sphere3D> sphere_derivatives_;
  std::vector<algebra::SphereD<3> > spheres_;
  std::vector<algebra::SphereD<3> > sphere_derivatives_;
  BasicAttributeTable<internal::FloatAttributeTableTraits> data_;
  BasicAttributeTable<internal::FloatAttributeTableTraits> derivatives_;
  // make use bitset
  BasicAttributeTable<internal::BoolAttributeTableTraits> optimizeds_;
  std::vector<FloatRange> ranges_;
  algebra::SphereD<3> get_invalid_sphere() const {
    double iv= internal::FloatAttributeTableTraits::get_invalid();
    algebra::SphereD<3> ivs(algebra::VectorD<3>(iv, iv, iv), iv);
    return ivs;
  }
public:
  void swap_with(FloatAttributeTable&o) {
    using IMP::swap;
    using std::swap;
    IMP_SWAP_MEMBER(spheres_);
    IMP_SWAP_MEMBER(sphere_derivatives_);
    IMP_SWAP_MEMBER(data_);
    IMP_SWAP_MEMBER(derivatives_);
    IMP_SWAP_MEMBER(optimizeds_);
  }
  FloatAttributeTable(){}

  // make sure you know what you are doing
  algebra::Sphere3D& get_sphere(ParticleIndex particle) {
    return spheres_[particle];
  }

  void add_to_coordinate_derivatives(ParticleIndex particle,
                                     const algebra::Vector3D &v,
                                     const DerivativeAccumulator &da) {
    IMP_USAGE_CHECK(get_has_attribute(FloatKey(0), particle),
                    "Particle does not have coordinates");
    sphere_derivatives_[particle][0]+=da(v[0]);
    sphere_derivatives_[particle][1]+=da(v[1]);
    sphere_derivatives_[particle][2]+=da(v[2]);
  }
  const algebra::Vector3D&
  get_coordinate_derivatives(ParticleIndex particle) const {
    IMP_USAGE_CHECK(get_has_attribute(FloatKey(0), particle),
                    "Particle does not have coordinates");
    return sphere_derivatives_[particle].get_center();
    }
  void zero_derivatives() {
    /*std::fill(sphere_derivatives_.begin(), sphere_derivatives_.end(),
      algebra::Sphere3D(algebra::Vector3D(0,0,0), 0));*/
    // make more efficient
    std::fill(sphere_derivatives_.begin(),
              sphere_derivatives_.end(),
              algebra::SphereD<3>(algebra::VectorD<3>(0,0,0),0));
    derivatives_.fill(0);
  }
  void clear_caches(ParticleIndex ) {
  }
  void add_cache_attribute(FloatKey , ParticleIndex, double ){
    IMP_NOT_IMPLEMENTED;
  }
  void remove_attribute(FloatKey k, ParticleIndex particle) {
    IMP_USAGE_CHECK(get_has_attribute(k, particle),
                    "Can't remove attribute that isn't there");
    if (k.get_index() < 4) {
      spheres_[particle][k.get_index()]
        = internal::FloatAttributeTableTraits::get_invalid();
      sphere_derivatives_[particle][k.get_index()]
        = internal::FloatAttributeTableTraits::get_invalid();
    } else {
      data_.remove_attribute(FloatKey(k.get_index()-4), particle);
      derivatives_.remove_attribute(FloatKey(k.get_index()-4), particle);
    }
    if (optimizeds_.get_has_attribute(k, particle)) {
      optimizeds_.remove_attribute(k, particle);
    }
  }
  bool get_is_optimized(FloatKey k, ParticleIndex particle) const {
    return optimizeds_.get_has_attribute(k, particle);
  }
  // check NOT_EVALUATING
  void set_is_optimized(FloatKey k, ParticleIndex particle, bool tf) {
    if (tf && !optimizeds_.get_has_attribute(k, particle)) {
      optimizeds_.add_attribute(k, particle, true);
    } else if (!tf && optimizeds_.get_has_attribute(k, particle)){
      optimizeds_.remove_attribute(k, particle);
    }
  }
  // check AFTER_EVALUATE, NOT_EVALUATING
  double get_derivative(FloatKey k, ParticleIndex particle) const {
    IMP_USAGE_CHECK(get_has_attribute(k, particle),
                    "Can't get derivative that isn't there");
    if (k.get_index() < 4) {
      return sphere_derivatives_[particle][k.get_index()];
    } else {
      return derivatives_.get_attribute(FloatKey(k.get_index()-4), particle);
    }
  }
  // check can change EVALUATE, AFTER_EVALUATE< NOT_EVALUATING
  void add_to_derivative(FloatKey k, ParticleIndex particle, double v,
                         const DerivativeAccumulator &da) {
    IMP_USAGE_CHECK(get_has_attribute(k, particle),
                    "Can't get derivative that isn't there");
    if (k.get_index() < 4) {
      sphere_derivatives_[particle][k.get_index()]+=da(v);;
    } else {
      FloatKey nk(k.get_index()-4);
      derivatives_.set_attribute(nk, particle,
                                 derivatives_.get_attribute(nk,
                                                            particle)+da(v));
    }
  }
  void add_attribute(FloatKey k, ParticleIndex particle, double v,
                     bool opt=false) {
    IMP_USAGE_CHECK(!get_has_attribute(k, particle),
                    "Can't add attribute that is there");
    if (k.get_index() <4) {
      if (spheres_.size() <= static_cast<size_t>(particle)) {
        spheres_.resize(particle+1, get_invalid_sphere());
        sphere_derivatives_.resize(particle+1, get_invalid_sphere());
      }
      spheres_[particle][k.get_index()]=v;
    } else {
      FloatKey nk(k.get_index()-4);
      data_.add_attribute(nk, particle, v);
      derivatives_.add_attribute(nk, particle, 0);
    }
    if (opt) optimizeds_.add_attribute(k, particle, true);
    ranges_.resize(std::max(ranges_.size(),
                            static_cast<size_t>(k.get_index()+1)),
                   FloatRange(-std::numeric_limits<double>::max(),
                               std::numeric_limits<double>::max()));
    IMP_USAGE_CHECK(get_has_attribute(k, particle),
                    "Can't attribute was not added");
  }
  bool get_has_attribute(FloatKey k, ParticleIndex particle) const {
    if (k.get_index() < 4) {
      if (spheres_.size() <= static_cast<size_t>(particle)) return false;
      else if (!internal::FloatAttributeTableTraits
               ::get_is_valid(spheres_[particle][k.get_index()])){
        return false;
      }
      return true;
    } else {
      return data_.get_has_attribute(FloatKey(k.get_index()-4), particle);
    }
  }
  void set_attribute(FloatKey k, ParticleIndex particle,
                     double v) {
    IMP_USAGE_CHECK(internal::FloatAttributeTableTraits::get_is_valid(v),
                    "Can't set attribute to invalid value");
    IMP_USAGE_CHECK(get_has_attribute(k, particle),
                    "Can't set attribute that is not there");
    if (k.get_index() <4) {
      spheres_[particle][k.get_index()]=v;
    } else {
      data_.set_attribute(FloatKey(k.get_index()-4), particle, v);
    }
  }
  double get_attribute(FloatKey k,
                       ParticleIndex particle) const {
    IMP_USAGE_CHECK(get_has_attribute(k, particle),
                    "Can't get attribute that is not there");
    if (k.get_index()<4) {
      return spheres_[particle][k.get_index()];
    } else {
      return data_.get_attribute(FloatKey(k.get_index()-4), particle);
    }
  }
  double& access_attribute(FloatKey k,
                       ParticleIndex particle) {
    IMP_USAGE_CHECK(get_has_attribute(k, particle),
                    "Can't get attribute that is not there");
    if (k.get_index()<4) {
      return spheres_[particle][k.get_index()];
    } else {
      return data_.access_attribute(FloatKey(k.get_index()-4), particle);
    }
  }
  struct FloatIndex
  {
    ParticleIndex p_;
    FloatKey k_;
    FloatIndex(FloatKey k, ParticleIndex p): p_(p), k_(k){}
    FloatIndex() {}
  };
  std::vector<FloatIndex> get_optimized_attributes() const {
    std::vector<FloatIndex> ret;
    for (unsigned int i=0; i< optimizeds_.size(); ++i) {
      for (unsigned int j=0; j< optimizeds_.size(i); ++j) {
        if (optimizeds_.get_has_attribute(FloatKey(i), j)) {
          ret.push_back(FloatIndex(FloatKey(i), j));
        }
      }
    }
    return ret;
  }
  void set_range(FloatKey k, FloatRange fr) {
    ranges_[k.get_index()]=fr;
  }
  FloatRange get_range(FloatKey k) {
    FloatRange ret= ranges_[k.get_index()];
    if (ret.first==-std::numeric_limits<double>::max()) {
      if (k.get_index() <4) {
        std::swap(ret.first, ret.second);
        for (unsigned int i=0; i< spheres_.size(); ++i) {
          if (internal::FloatAttributeTableTraits
              ::get_is_valid(spheres_[i][k.get_index()])) {
            ret.first= std::min(ret.first, spheres_[i][k.get_index()]);
            ret.second= std::max(ret.second, spheres_[i][k.get_index()]);
          }
        }
        return ret;
      } else {
        return data_.get_range_internal(FloatKey(k.get_index()-4));
      }
    } else {
      return ret;
    }
  }
  void clear_attributes(ParticleIndex particle) {
    if (spheres_.size()> static_cast<size_t>(particle)) {
      spheres_[particle]= get_invalid_sphere();
      sphere_derivatives_[particle]=get_invalid_sphere();
    }
    data_.clear_attributes(particle);
    derivatives_.clear_attributes(particle);
    optimizeds_.clear_attributes(particle);
  }
  std::vector<FloatKey> get_attribute_keys(ParticleIndex particle) const {
    std::vector<FloatKey> ret=data_.get_attribute_keys(particle);
    for (unsigned int i=0; i< ret.size(); ++i) {
      ret[i]= FloatKey(ret[i].get_index()+4);
    }
    for (unsigned int i=0; i< 4; ++i) {
      if (get_has_attribute(FloatKey(i),particle)) {
        ret.push_back(FloatKey(i));
      }
    }
    return ret;
  }
};

IMP_SWAP(FloatAttributeTable);

typedef BasicAttributeTable<internal::StringAttributeTableTraits>
StringAttributeTable;
typedef BasicAttributeTable<internal::IntAttributeTableTraits>
IntAttributeTable;
typedef BasicAttributeTable<internal::ObjectAttributeTableTraits>
ObjectAttributeTable;
typedef BasicAttributeTable<internal::IntsAttributeTableTraits>
IntsAttributeTable;
typedef BasicAttributeTable<internal::ObjectsAttributeTableTraits>
ObjectsAttributeTable;
typedef BasicAttributeTable<internal::ParticleAttributeTableTraits>
ParticleAttributeTable;
typedef BasicAttributeTable<internal::ParticlesAttributeTableTraits>
ParticlesAttributeTable;
#endif

IMP_VALUES(RestraintStatistics, RestraintStatisticsList);

//! Class for storing model, its restraints, constraints, and particles.
/** The Model maintains a standard \imp container for each of Particle,
    ScoreState and Restraint object types.

    \note Think carefully about using the iterators over the entire set
          of Particles or Restraints. Most operations should be done using
          a user-passed set of Particles or Restraints instead.
 */
class IMPEXPORT Model:
  public Object
#if !defined(SWIG) && !defined(IMP_DOXYGEN)
  , public FloatAttributeTable,
  public StringAttributeTable,
  public IntAttributeTable,
  public ObjectAttributeTable,
  public IntsAttributeTable,
  public ObjectsAttributeTable,
  public ParticleAttributeTable,
  public ParticlesAttributeTable
#endif
{
 public:
  IMP_MODEL_IMPORT(FloatAttributeTable);
  IMP_MODEL_IMPORT(StringAttributeTable);
  IMP_MODEL_IMPORT(IntAttributeTable);
  IMP_MODEL_IMPORT(ObjectAttributeTable);
  IMP_MODEL_IMPORT(IntsAttributeTable);
  IMP_MODEL_IMPORT(ObjectsAttributeTable);
  IMP_MODEL_IMPORT(ParticleAttributeTable);
  IMP_MODEL_IMPORT(ParticlesAttributeTable);
  void clear_particle_caches(ParticleIndex pi) {
    FloatAttributeTable::clear_caches(pi);
    StringAttributeTable::clear_caches(pi);
    IntAttributeTable::clear_caches(pi);
    ObjectAttributeTable::clear_caches(pi);
    IntsAttributeTable::clear_caches(pi);
    ObjectsAttributeTable::clear_caches(pi);
    ParticleAttributeTable::clear_caches(pi);
    ParticlesAttributeTable::clear_caches(pi);
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
  mutable internal::Stage cur_stage_;
  unsigned int eval_count_;
  internal::OwnerPointer<RestraintSet> rs_;
  bool first_call_;
  double max_score_;
  mutable bool has_good_score_;
  std::vector<std::pair<Object*, Object*> > extra_edges_;

  Ints free_particles_;
  unsigned int next_particle_;
  std::vector<Pointer<Particle> > particle_index_;

 private:
  // statistics
  bool gather_statistics_;
  void add_to_update_before_time(ScoreState *s, double t) const;
  void add_to_update_after_time(ScoreState *s, double t) const;
  void add_to_restraint_evaluate(Restraint *r, double t, double score) const;



  void validate_computed_derivatives() const{}
  void before_evaluate(const ScoreStatesTemp &states) const;
  void after_evaluate(const ScoreStatesTemp &states, bool calc_derivs) const;
  Floats do_evaluate(const RestraintsTemp &restraints,
                     const ScoreStatesTemp &states, bool calc_derivs,
                     bool if_good, bool if_max,
                     double max= std::numeric_limits<double>::max());
  Floats do_evaluate_restraints(const RestraintsTemp &restraints,
                                bool calc_derivs,
                                bool if_good, bool if_max,
                                double max= std::numeric_limits<double>::max());
  Floats do_external_evaluate(const RestraintsTemp &restraints,
                              bool calc_derivs,
                              bool if_good, bool if_max,
                              double max= std::numeric_limits<double>::max());


  // dependencies
  mutable RestraintsTemp ordered_restraints_;
  mutable ScoreStatesTemp ordered_score_states_;
  void compute_dependencies() const;
  bool get_has_dependencies() const {
    return (!ordered_restraints_.empty()
            || get_number_of_restraints() ==0)
      && ordered_score_states_.size()
      == get_number_of_score_states();
  }

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
  template <class T, class E> friend struct internal::RefStuff;
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
              {Model::set_score_state_model(obj, NULL);
               if(container) container->reset_dependencies(); });
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
  Floats evaluate( RestraintsTemp restraints,
                   bool calc_derivs);

  //! Evaluate a subset of the restraints
  /** In contrast to other evaluate methods,
      this method is free to shortcut evaluation and return a very
      large score if the total score at any point exceeds max or if
      any of the restraints exceed their Restraint::get_maximum().

      See evaluate(RestraintsTemp,std::vector<double>,bool) for more
      information.
  */
  Floats evaluate_if_good( RestraintsTemp restraints,
                           bool calc_derivs);
  /** Evaluate, returning the score if it below the max value.
      Otherwise return a number above max. The restraint maxima
      are ignored.
  */
  Floats evaluate_if_below( RestraintsTemp restraints,
                            bool calc_derivs, double max);


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

#ifndef IMP_DOXYGEN
  /** Sometimes there are dependencies among score states that require
      an ordering that cannot be derived automatically. For example score
      states that read and write the same set of particles and have to
      do so in a certain order.
  */
  void add_dependency_edge(ScoreState *from, ScoreState *to);
#endif

#if !defined(IMP_DOXYGEN) && !defined(SWIG)
  const std::vector<std::pair<Object*, Object*> >&
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
    IMP_USAGE_CHECK(particle_index_.size() > static_cast<unsigned int>(p),
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
      std::vector<Pointer<Particle> >::const_iterator> ParticleIterator;

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
  return get_model()->ParticleAttributeTable::get_attribute_keys(id_);
}
IMP_END_NAMESPACE

#endif  /* IMP_MODEL_H */
