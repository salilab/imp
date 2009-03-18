/**
 * \file AttributeMapScoreState.h
 * \brief Maintain a map between attribute values and particles.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPSEARCH_ATTRIBUTE_MAP_SCORE_STATE_H
#define IMPSEARCH_ATTRIBUTE_MAP_SCORE_STATE_H

#include "config.h"
#include "internal/version_info.h"
#include "internal/map_helpers.h"
#include <IMP/SingletonContainer.h>
#include <IMP/core/config.h>
#include <IMP/Particle.h>
#include <IMP/ScoreState.h>

#include <iostream>
#include <algorithm>

#include <boost/tuple/tuple.hpp>

namespace IMP {
  // for swig
  class SingletonContainer;
}

IMPSEARCH_BEGIN_NAMESPACE

//! Maintain a mapping between attribute values and particles.
/** This class allows you to look of which particles from a set have
    attributes which a particular set of values. That is, if you
    initialize the map with the IntKey("residue index") attribute,
    you can quickly find the residues with that index.

    The template arguments are a boost::tuple containing the
    key types.
*/
template <class KeysT>
class AttributeMapScoreState : public ScoreState
{
  typedef internal::MapTraits<KeysT> Traits;
public:
  typedef KeysT Key;
  typedef typename Traits::Value Value;
private:
  typedef typename Traits::Bin Bin;
  typedef typename std::vector<Bin> Map;
  Map map_;
  Pointer<IMP::SingletonContainer> pc_;
  Key keys_;

  typename Map::const_iterator get_iterator(const Value &vs) const {
    return std::lower_bound(map_.begin(), map_.end(), Bin(vs));
  }

  const Particles &get_particles_internal(const Value &vs) const {
    static Particles empty;
    Bin bvs(vs);
    typename Map::const_iterator it = get_iterator(vs);
    if (it!= map_.end() && *it == bvs) {
      return it->ps_;
    } else {
      return empty;
    }
  }

  Particles &get_particles_nonconst(const Value &vs) {
    Bin bvs(vs);
    typename Map::iterator it = std::lower_bound(map_.begin(), map_.end(),
                                                 bvs);
    IMP_assert(it != map_.end(), "Value not found in map on update");
    IMP_assert(*it == bvs, "Values to not match on map update");
    return it->ps_;
  }

public:

  //! Create the score state searching one attribute.
  /** The first argument is the list of particles to search,
      the remainder are the attributes to use. The number of key arguments
      must match the number of provided value types in the template
      parameter list.
  */
  AttributeMapScoreState(IMP::SingletonContainer* pc,
                         Key key): pc_(pc), keys_(key) {
  }


  //! Find particles which match the given attributes
  const Particles &get_particles(const Value &v)  const {
    return get_particles_internal(v);
  }

  //! Find particles which match the given attributes
  /** It is an error if there is not a unique particle.
   */
  const Particle* get_particle(const Value &v)  const {
    IMP_assert(get_particles_internal(v).size() == 1,
               "There must be exactly one particle.");
    return get_particles_internal(v)[0];
  }

  //! Find all the particles in the range
  /** The range is defined via a lexicographical ordering. The first set
      of arguments is the lower bound and the second set is the upper bound.
      The range is half open (so the upper bound is not included).
   */
  const Particles get_particles(const Value &v0,
                                const Value &v1) const {
    typename Map::const_iterator it =get_iterator(v0), ite= get_iterator(v1);
    Particles ret;
    for (; it != ite; ++it) {
      ret.insert(ret.end(), it->ps_.begin(), it->ps_.end());
    }
    return ret;
  }

  /* We can't use the macro since SWIG won't instantiate do_before_evaluate
     since it doesn't call it directly.
  */
  virtual void show(std::ostream &out=std::cout) const {
    out << "AttributeMapScoreState on "
        << pc_ << " and ";
    Traits::write(keys_, out);
    out << std::endl;
  }
  virtual IMP::VersionInfo get_version_info() const {
    return internal::version_info;
  }


  /* make this public so SWIG instantiates it*/
  virtual void do_before_evaluate() {
    map_.clear();
    map_.reserve(pc_->get_number_of_particles());
    for (IMP::SingletonContainer::ParticleIterator
           it= pc_->particles_begin();
         it != pc_->particles_end(); ++it) {
      Value vs= Traits::get_value(*it, keys_);
      map_.push_back(vs);
    }
    std::sort(map_.begin(), map_.end());
    map_.erase(std::unique(map_.begin(), map_.end()), map_.end());
    for (IMP::SingletonContainer::ParticleIterator
           it= pc_->particles_begin();
         it != pc_->particles_end(); ++it) {
      Value vs= Traits::get_value(*it, keys_);
      get_particles_nonconst(vs).push_back(*it);
    }
  }

  ~AttributeMapScoreState(){}

};

// for swig

//! A map to store a set of particles indexed by an Int attribute
typedef AttributeMapScoreState<boost::tuple<IntKey> > IntMapScoreState;
//! The key to use to set up the map
/** \see IntMapScoreState */
typedef AttributeMapScoreState<boost::tuple<IntKey> >::Key IntMapKey;
//! The value to use to search up the map
/** \see IntMapScoreState */
typedef AttributeMapScoreState<boost::tuple<IntKey> >::Value IntMapValue;


IMPSEARCH_END_NAMESPACE

#endif  /* IMPSEARCH_ATTRIBUTE_MAP_SCORE_STATE_H */
