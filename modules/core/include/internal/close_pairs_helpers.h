/**
 *  \file internal/close_pairs_helpers.h
 *  \brief utilities for rigid bodies.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 */

#ifndef IMPCORE_INTERNAL_CLOSE_PAIRS_HELPERS_H
#define IMPCORE_INTERNAL_CLOSE_PAIRS_HELPERS_H

#include "../core_config.h"
#include "../BoxSweepClosePairsFinder.h"
#include "../GridClosePairsFinder.h"
#include "grid_close_pairs_impl.h"
#include "rigid_body_tree.h"
#include "../XYZR.h"
#include "CoreListPairContainer.h"

IMPCORE_BEGIN_INTERNAL_NAMESPACE

inline ClosePairsFinder *default_cpf(unsigned int ) {
  return new GridClosePairsFinder();
}

struct IsInactive {
  bool operator()(const ParticlePair &p) const {
    return !p[0]->get_is_active() || !p[1]->get_is_active();
  }
};
inline CoreListPairContainer *get_list(PairContainer *pc) {
  return dynamic_cast<CoreListPairContainer *>(pc);
}


template <class C>
inline void filter_close_pairs(C *c, ParticleIndexPairs &ps) {
  for (typename C::PairFilterConstIterator it=c->pair_filters_begin();
       it != c->pair_filters_end(); ++it) {
    (*it)->filter_in_place(c->get_model(), ps);
  }
}


inline void fix_order(ParticleIndexPairs &pips) {
  for (unsigned int i=0; i< pips.size(); ++i) {
    if (pips[i][0] > pips[i][1]) {
      std::swap(pips[i][0], pips[i][1]);
    }
  }
}
/*
// Check that they are unique by checking order
struct SameParticle {
  ParticleIndexes ppt_;
  SameParticle(const ParticleIndexes &ppt): ppt_(ppt){
    std::sort(ppt_.begin(), ppt_.end());
  }
  SameParticle(){}
  bool operator()(const ParticleIndexPair& pp) {
    if (ppt_.empty()) {
      return pp[0] == pp[1];
    } else {
      if (binary_search(ppt_.begin(), ppt_.end(), pp[0])
          && binary_search(ppt_.begin(), ppt_.end(), pp[1])) {
        return !(pp[0] < pp[1]);
      } else {
        return false;
      }
    }
  }
  };*/

// Check that they are unique by checking order
struct SameParticle {
  SameParticle(){
  }
  bool operator()(const ParticleIndexPair& pp) {
    return pp[0] == pp[1];
  }
};

/*

inline void filter_same(ParticleIndexPairs &c,
                        const ParticleIndexes &moved) {
  c.erase(std::remove_if(c.begin(), c.end(),
                         SameParticle(moved)),
          c.end());
          }*/

inline void filter_same(ParticleIndexPairs &c) {
  c.erase(std::remove_if(c.begin(), c.end(),
                         SameParticle()),
          c.end());
}


inline bool get_are_close(Model *m, const PairFilters &filters,
                          ParticleIndex a, ParticleIndex b,
                          double distance) {
  XYZ da(m, a);
  XYZ db(m, b);
  Float ra= XYZR(m, a).get_radius();
  Float rb= XYZR(m, b).get_radius();
  Float sr= ra+rb+distance;
  for (unsigned int i=0; i< 3; ++i) {
    double delta=std::abs(da.get_coordinate(i) - db.get_coordinate(i));
    if (delta >= sr) {
      return false;
    }
  }
  return get_interiors_intersect(algebra::Sphere3D(da.get_coordinates(),
                                                     ra+distance),
                                 algebra::Sphere3D(db.get_coordinates(), rb))
    && !get_filters_contains(m, filters, ParticleIndexPair(a,b));
}

struct FarParticle {
  double d_;
  Model *m_;
  FarParticle( Model *m,
               double d): d_(d), m_(m){}
  bool operator()(const ParticleIndexPair& pp) const {
    return !get_are_close(m_, PairFilters(), pp[0], pp[1], d_);
  }
};

inline void filter_far(Model *m, ParticleIndexPairs &c, double d) {
  c.erase(std::remove_if(c.begin(), c.end(),
                         FarParticle(m, d)),
          c.end());
}


struct InList {
  static IntKey key_;
  const ParticlesTemp &ps_;
  InList( const ParticlesTemp &ps): ps_(ps){
  }
  static InList create( ParticlesTemp &ps) {
    /*for (unsigned int i=0; i< ps.size(); ++i) {
      ps[i]->add_attribute(key_, 1);
      }*/
    std::sort(ps.begin(), ps.end());
    return InList(ps);
  }
  static void destroy(InList &) {
    /*for (unsigned int i=0; i< il.ps_.size(); ++i) {
      il.ps_[i]->remove_attribute(key_);
      }*/
  }
  bool operator()(const ParticlePair &pp) const {
    if (std::binary_search(ps_.begin(), ps_.end(), pp[0])) return true;
    else if (std::binary_search(ps_.begin(), ps_.end(), pp[1])) return true;
    return false;
    //return pp[0]->has_attribute(key_) || pp[1]->has_attribute(key_);
  }
};

inline void
reset_moved(Model *m,
            ParticleIndexes &xyzrs_,
            ParticleIndexes &rbs_,
            IMP::compatibility::map<ParticleIndex,
                                            ParticleIndexes>&
            /*constituents_*/,
            algebra::Transformation3Ds&
            rbs_backup_,
            algebra::Vector3Ds &xyzrs_backup_) {
  xyzrs_backup_.resize(xyzrs_.size());
  for (unsigned int i=0; i< xyzrs_.size(); ++i) {
    xyzrs_backup_[i]= m->get_sphere(xyzrs_[i]).get_center();
  }
  rbs_backup_.resize(rbs_.size());
  for (unsigned int i=0; i< rbs_.size(); ++i) {
    rbs_backup_[i]= RigidBody(m, rbs_[i]).get_reference_frame()
      .get_transformation_to();
  }
}


inline void initialize_particles( SingletonContainer*sc,
                                  ObjectKey key,
                                 ParticleIndexes &xyzrs_,
                                 ParticleIndexes &rbs_,
                                  IMP::compatibility::map<ParticleIndex,
                                                          ParticleIndexes>&
                                  constituents_,
                                  algebra::Transformation3Ds&
                                  rbs_backup_,
                                  algebra::Vector3Ds&xyzrs_backup_) {
  IMP_IF_CHECK(USAGE) {
    ParticleIndexes pis = sc->get_indexes();
    IMP::compatibility::set<ParticleIndex> spis(pis.begin(), pis.end());
    IMP_USAGE_CHECK(pis.size() == spis.size(),
                    "Duplicate particle indexes in input");
  }
  IMP_IF_CHECK(USAGE) {
    ParticlesTemp pis = sc->get();
    IMP::compatibility::set<Particle*> spis(pis.begin(), pis.end());
    IMP_USAGE_CHECK(pis.size() == spis.size(), "Duplicate particles in input");
  }
  constituents_.clear();
  xyzrs_.clear();
  rbs_.clear();
  using IMP::operator<<;
  IMP_FOREACH_SINGLETON(sc, {
      IMP_LOG(VERBOSE, "Processing " << _1->get_name()
              << " (" << _1->get_index() << ")" << std::endl);
      if (RigidMember::particle_is_instance(_1)) {
        RigidBody rb=RigidMember(_1).get_rigid_body();
        ParticleIndex pi= rb.get_particle_index();
        rbs_.push_back(rb.get_particle_index());
        if (constituents_.find(pi) == constituents_.end()) {
          constituents_.insert(std::make_pair(pi, ParticleIndexes(1,
                                                 _1->get_index())));
        } else {
          constituents_[pi].push_back(_1->get_index());
        }
        IMP_IF_CHECK(USAGE_AND_INTERNAL) {
          ParticleIndexes cur= constituents_[pi];
          IMP_USAGE_CHECK(std::find(cur.begin(), cur.end(), pi) == cur.end(),
                          "A rigid body cann't be its own constituent.");
          IMP::compatibility::set<ParticleIndex> scur(cur.begin(), cur.end());
          IMP_USAGE_CHECK(cur.size() == scur.size(),
                          "Duplicate constituents for "
                          << sc->get_model()->get_particle(pi)->get_name()
                          << ": " << cur);
        }
      } else {
        xyzrs_.push_back(_1->get_index());
      }
    });
  std::sort(rbs_.begin(), rbs_.end());
  rbs_.erase(std::unique(rbs_.begin(), rbs_.end()), rbs_.end());
  for (unsigned int i=0; i < rbs_.size(); ++i) {
    internal::get_rigid_body_hierarchy(RigidBody(sc->get_model(), rbs_[i]),
                                       constituents_[rbs_[i]],
                                       key);
  }
  reset_moved(sc->get_model(),
              xyzrs_, rbs_, constituents_, rbs_backup_, xyzrs_backup_);
  xyzrs_backup_.clear();
  rbs_backup_.clear();
  IMP_IF_CHECK(USAGE_AND_INTERNAL) {
     for (IMP::compatibility::map<ParticleIndex,
                                  ParticleIndexes>::const_iterator it
         = constituents_.begin();
       it != constituents_.end(); ++it) {
       ParticleIndexes cur= it->second;
       IMP_USAGE_CHECK(std::find(cur.begin(), cur.end(),
                                 it->first) == cur.end(),
                       "A rigid body cann't be its own constituent.");
       IMP::compatibility::set<ParticleIndex> scur(cur.begin(), cur.end());
       IMP_USAGE_CHECK(cur.size() == scur.size(),
                       "Duplicate constituents for "
                       << sc->get_model()->get_particle(it->first)->get_name()
                       << ": " << cur);
     }
  }
}


inline bool
get_if_moved(Model *m, double slack_,
             ParticleIndexes &xyzrs_,
             ParticleIndexes &rbs_,
             IMP::compatibility::map<ParticleIndex,
                                             ParticleIndexes>&
             /*constituents_*/,
             algebra::Transformation3Ds&
             rbs_backup_,
             algebra::Vector3Ds&xyzrs_backup_) {
  IMP_INTERNAL_CHECK(xyzrs_.size()== xyzrs_backup_.size(),
                     "Backup is not a backup");
  const double s22= square(slack_/2);
  for (unsigned int i=0; i< xyzrs_.size(); ++i) {
    double diff2=0;
    for (unsigned int j=0; j< 3; ++j) {
      double diffc2= square(m->get_sphere(xyzrs_[i]).get_center()[j]
                            - xyzrs_backup_[i][j]);
      diff2+=diffc2;
      if (diff2> s22) {
        return true;
      }
    }
  }
  for (unsigned int i=0; i< rbs_.size(); ++i) {
    double diff2=0;
    for (unsigned int j=0; j< 3; ++j) {
      double diffc2= square(m->get_sphere(rbs_[i]).get_center()[j]
                            - rbs_backup_[i].get_translation()[j]);
      diff2+=diffc2;
      if (diff2> s22) {
        return true;
      }
    }
    algebra::Rotation3D nrot=RigidBody(m,
                                       rbs_[i]).get_reference_frame()
      .get_transformation_to().get_rotation();
    algebra::Rotation3D diffrot
      = rbs_backup_[i].get_rotation().get_inverse()*nrot;
    double angle= algebra::get_axis_and_angle(diffrot).second;
    double drot= std::abs(angle*m->get_sphere(rbs_[i]).get_radius());
    if (s22 < square(drot)+drot*std::sqrt(diff2)+ diff2) {
      return true;
    }
  }
  return false;
}



inline void fill_list(Model *m, const PairFilters &filters,
                      ObjectKey key_,
                      double slack_,
                      ParticleIndexes &xyzrs_,
                      ParticleIndexes &rbs_,
                      IMP::compatibility::map<ParticleIndex,
                                             ParticleIndexes>&
                      constituents_,
                      ParticleIndexPairs &cur_list_) {
  IMP_INTERNAL_CHECK(slack_>=0, "Slack must not be negative");
  /*IMP_LOG(VERBOSE, "filling particle list with slack " << slack_
    << " on " << sc_->get_name());*/
  cur_list_.clear();
  internal::ParticleIndexHelper
      ::fill_close_pairs(internal::ParticleIndexHelper
                         ::get_particle_set(xyzrs_.begin(),
                                            xyzrs_.end(),0),
                         internal::ParticleIndexTraits(m, slack_),
                         internal::ParticleIndexPairSink(m, filters,
                                                         cur_list_));
  internal::ParticleIndexHelper
    ::fill_close_pairs(internal::ParticleIndexHelper
                       ::get_particle_set(rbs_.begin(),
                                          rbs_.end(),0),
                       internal::ParticleIndexHelper
                       ::get_particle_set(xyzrs_.begin(),
                                          xyzrs_.end(),1),
                       internal::ParticleIndexTraits(m, slack_),
               internal::RigidBodyParticleParticleIndexPairSink(m,
                                                                filters,
                                                                   cur_list_,
                                                                   key_,
                                                                   slack_,
                                                            constituents_));
  internal::ParticleIndexHelper
    ::fill_close_pairs(internal::ParticleIndexHelper
                       ::get_particle_set(rbs_.begin(),
                                          rbs_.end(),0),
                       internal::ParticleIndexTraits(m, slack_),
               internal::RigidBodyRigidBodyParticleIndexPairSink(m,
                                                                 filters,
                                                                 cur_list_,
                                                                 key_,
                                                                 slack_,
                                                             constituents_));
  IMP_LOG(VERBOSE, "found " << cur_list_.size() << std::endl);
}


inline void fill_list(Model *m, const PairFilters &filters,
                      ObjectKey key_,
                      double slack_,
                      ParticleIndexes xyzrs_[],
                      ParticleIndexes rbs_[],
                      IMP::compatibility::map<ParticleIndex,
                                              ParticleIndexes>&
                      constituents_,
                      ParticleIndexPairs &cur_list_) {
  IMP_INTERNAL_CHECK(slack_>=0, "Slack must not be negative");
  /*IMP_LOG(VERBOSE, "filling particle list with slack " << slack_
    << " on " << sc_->get_name());*/
  cur_list_.clear();
  internal::ParticleIndexHelper
      ::fill_close_pairs(internal::ParticleIndexHelper
                         ::get_particle_set(xyzrs_[0].begin(),
                                            xyzrs_[0].end(),0),
                         internal::ParticleIndexHelper
                         ::get_particle_set(xyzrs_[1].begin(),
                                            xyzrs_[1].end(),0),
                         internal::ParticleIndexTraits(m, slack_),
                         internal::ParticleIndexPairSink(m, filters,
                                                         cur_list_));
  internal::ParticleIndexHelper
    ::fill_close_pairs(internal::ParticleIndexHelper
                       ::get_particle_set(rbs_[0].begin(),
                                          rbs_[0].end(),0),
                       internal::ParticleIndexHelper
                       ::get_particle_set(xyzrs_[1].begin(),
                                          xyzrs_[1].end(),1),
                       internal::ParticleIndexTraits(m, slack_),
               internal::RigidBodyParticleParticleIndexPairSink(m,
                                                                filters,
                                                                   cur_list_,
                                                                   key_,
                                                                   slack_,
                                                            constituents_));
  internal::ParticleIndexHelper
    ::fill_close_pairs(internal::ParticleIndexHelper
                       ::get_particle_set(xyzrs_[0].begin(),
                                          xyzrs_[0].end(),0),
                       internal::ParticleIndexHelper
                       ::get_particle_set(rbs_[1].begin(),
                                          rbs_[1].end(),1),
                       internal::ParticleIndexTraits(m, slack_),
               internal::RigidBodyParticleParticleIndexPairSink(m,
                                                                filters,
                                                                   cur_list_,
                                                                   key_,
                                                                   slack_,
                                                            constituents_));
  internal::ParticleIndexHelper
    ::fill_close_pairs(internal::ParticleIndexHelper
                       ::get_particle_set(rbs_[0].begin(),
                                          rbs_[0].end(),0),
                       internal::ParticleIndexHelper
                       ::get_particle_set(rbs_[1].begin(),
                                          rbs_[1].end(),0),
                       internal::ParticleIndexTraits(m, slack_),
               internal::RigidBodyRigidBodyParticleIndexPairSink(m,
                                                                 filters,
                                                                    cur_list_,
                                                                    key_,
                                                                    slack_,
                                                             constituents_));
  IMP_LOG(VERBOSE, "found " << cur_list_.size() << std::endl);
}


inline ParticlesTemp
get_input_particles(Model *m, SingletonContainer *sc_,
                    const PairFilters &filters_,
                    const ParticleIndexes &/*xyzrs_*/,
                    const ParticleIndexes &rbs_,
                    const IMP::compatibility::map<ParticleIndex,
                                            ParticleIndexes>&
                    /*constituents_*/) {
  ParticlesTemp ret= sc_->get_contained_particles();
  ParticlesTemp all;
  for (unsigned int i=0; i< filters_.size(); ++i) {
    for (unsigned int j=0; j< ret.size(); ++j) {
      ParticlesTemp cur= filters_[i]->get_input_particles(ret[j]);
      all.insert(all.end(), cur.begin(), cur.end());
    }
  }
  ret.insert(ret.end(), all.begin(), all.end());
  for (unsigned int i=0; i< rbs_.size(); ++i) {
    ret.push_back(m->get_particle(rbs_[i]));
  }
  return ret;
}
IMPCORE_END_INTERNAL_NAMESPACE

#endif  /* IMPCORE_INTERNAL_CLOSE_PAIRS_HELPERS_H */
