/**
 *  \file MovedSingletonContainer.h
 *  \brief Keep track of the maximum change of a set of attributes.
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 */

#ifndef IMPCORE_INTERNAL_MOVED_SINGLETON_CONTAINER_H
#define IMPCORE_INTERNAL_MOVED_SINGLETON_CONTAINER_H

#include "../config.h"

#include <IMP/ScoreState.h>
#include <IMP/Particle.h>
#include <IMP/SingletonContainer.h>
#include <IMP/SingletonModifier.h>
#include <IMP/algebra/Sphere3D.h>
#include "../XYZR.h"
#include "../rigid_bodies.h"
#include "singleton_helpers.h"
#include <IMP/internal/container_helpers.h>
#include <vector>

IMPCORE_BEGIN_INTERNAL_NAMESPACE

namespace {
  class SaveXYZRRotValues:public SingletonModifier {
  protected:
    std::vector<std::pair<algebra::SphereD<3>,
                          algebra::Rotation3D> > &values_;
    mutable int i_;
    void do_apply(Particle *p) const {
      XYZR d(p);
      IMP_INTERNAL_CHECK(values_.size() > static_cast<unsigned int>(i_),
                         "Wrong size of values_.");
      values_[i_].first= d.get_sphere();
      if (RigidBody::particle_is_instance(p)) {
        RigidBody rb(p);
        values_[i_].second
          = rb.get_transformation().get_rotation();
      }
      ++i_;
    }
  public:
    typedef std::pair<algebra::SphereD<3>, algebra::Rotation3D> Value;
    SaveXYZRRotValues(std::vector<Value> &values):
      SingletonModifier("SaveXYZRRotValues"), values_(values) {
      i_=0;
    }
    IMP_INTERNAL_SINGLETON_MODIFIER(SaveXYZRRotValues,
                                    do_apply(p);)
  };

  class SaveXYZRValues:public SingletonModifier {
  protected:
    std::vector<algebra::SphereD<3> > &values_;
    mutable int i_;
    void do_apply(Particle* p) const {
      XYZR d(p);
      IMP_INTERNAL_CHECK(values_.size() > static_cast<unsigned int>(i_),
                         "Wrong size of values_.");
      values_[i_]= d.get_sphere();
      ++i_;
    }
  public:
    typedef algebra::SphereD<3> Value;
    SaveXYZRValues(std::vector<algebra::SphereD<3> > &values):
      SingletonModifier("SaveXYZRValues"),
      values_(values) {
      i_=0;
    }
    IMP_INTERNAL_SINGLETON_MODIFIER(SaveXYZRValues,
                                    {
                                      do_apply(p);
                                    })
  };

  template <class P>
  class SaveMovedValues:public P {
    const ParticlesTemp &moved_;
    void do_apply(Particle *p) const {
      if (std::binary_search(moved_.begin(),
                             moved_.end(), p)) {
        P::apply(p);
      } else {
        ++P::i_;
      }
    }
  public:
    SaveMovedValues(std::vector<typename P::Value> &values,
                    const ParticlesTemp &moved):
      P(values),
      moved_(moved){
    }
    IMP_INTERNAL_SINGLETON_MODIFIER(SaveMovedValues,
                                    do_apply(p);)
  };

  inline bool moved_threshold(Particle *p, const algebra::SphereD<3> &old,
                              bool incremental,
                              double threshold, double &dist2, double &dr,
                              double &dist, bool has_dist) {
    if (incremental && !p->get_is_changed()) {
      has_dist=false;
      return false;
    }
    XYZR d(p);
    dist2=(d.get_coordinates()- old.get_center()).get_squared_magnitude();
    dr= std::abs(old.get_radius()-d.get_radius());
    if (square(threshold) > dist2+ dist2*dr + square(dr)) {
      has_dist=false;
      return false;
    } else {
      dist= std::sqrt(dist2);
      has_dist=true;
      return threshold < dist + dr;
    }
  }

  class ListXYZRRotMovedParticles:public SingletonModifier {
    std::vector<std::pair<algebra::SphereD<3>,
                          algebra::Rotation3D> > &values_;
    mutable ParticlesTemp &pt_;
    double threshold_;
    mutable unsigned int i_;
    bool incremental_;
    void do_apply(Particle *p) const {
      double dist2=-1, dr, dist;
      bool has_dist=false; // suppress warning
      IMP_INTERNAL_CHECK(values_.size() > i_,
                         "Wrong size of values_.");
      if (moved_threshold(p, values_[i_].first, incremental_,
                          threshold_, dist2, dr, dist, has_dist)) {
        pt_.push_back(p);
      } else {
        if (!has_dist) {
          dist= std::sqrt(dist2);
        }
        if (RigidBody::particle_is_instance(p)) {
          RigidBody rb(p);
          algebra::Rotation3D rd
            = values_[i_].second/rb.get_transformation().get_rotation();
          algebra::VectorD<3> rv(0,0,XYZR(p).get_radius());
          algebra::VectorD<3> rvr= rd.get_rotated(rv);
          if (dist + (rv-rvr).get_magnitude() > threshold_) {
            pt_.push_back(p);
          }
        }
      }
      ++i_;
    }
  public:
    ListXYZRRotMovedParticles(std::vector<std::pair<algebra::SphereD<3>,
                              algebra::Rotation3D> > &values,
                ParticlesTemp &pt,
                double threshold,
                bool incremental):
      SingletonModifier("ListXYZRRotMoved"),
      values_(values),
      pt_(pt), threshold_(threshold),
      i_(0), incremental_(incremental){
    }

    IMP_INTERNAL_SINGLETON_MODIFIER(ListXYZRRotMovedParticles,
                                    do_apply(p););
  };

  class ListXYZRMovedParticles:public SingletonModifier {
    std::vector<algebra::SphereD<3> > &values_;
    mutable ParticlesTemp &pt_;
    double threshold_;
    mutable unsigned int i_;
    bool incremental_;
    void do_apply(Particle *p ) const {
      double dist2, dr, dist;
      bool has_dist=false;
      if (moved_threshold(p, values_[i_], incremental_,
                          threshold_, dist2, dr, dist, has_dist)) {
        pt_.push_back(p);
      }
      ++i_;
    }
  public:
    ListXYZRMovedParticles(std::vector<algebra::SphereD<3> > &values,
                ParticlesTemp &pt,
                double threshold,
                           bool incremental):
      SingletonModifier("ListXYZRMoved"),
      values_(values),
      pt_(pt), threshold_(threshold),
      i_(0), incremental_(incremental){
    }
    IMP_INTERNAL_SINGLETON_MODIFIER(ListXYZRMovedParticles,
                                    do_apply(p););
  };
}

class IMPCOREEXPORT MovedSingletonContainer: public ListLikeSingletonContainer
{
 protected:
  double threshold_;
  Pointer<SingletonContainer> pc_;
  bool first_call_;
  IMP_ACTIVE_CONTAINER_DECL(MovedSingletonContainer);
  virtual void save()=0;
  virtual void save_moved()=0;
  virtual void update_list()=0;
  using ListLikeSingletonContainer::update_list;
public:
  //! Track the changes with the specified keys.
  MovedSingletonContainer(Model *m,
                          SingletonContainer *pc,
                          double threshold);

  //! Measure differences from the current value.
  void reset();

  //! Reset the positions for the moved particles
  void reset_moved();

  void update() {
    ticker_.get_score_state()->before_evaluate();
  }

  //! Return the container storing the particles
  SingletonContainer *get_singleton_container() const {
    return pc_;
  }
  void set_threshold(double d);

  IMP_LISTLIKE_SINGLETON_CONTAINER(MovedSingletonContainer);
};


template <class Data, class Save, class SaveMoved, class ListMoved>
class MovedSingletonContainerImpl:
  public MovedSingletonContainer
{
  std::vector<Data> backup_;
  virtual void save() {
    //backup_.clear();
    backup_.resize(MovedSingletonContainer
                   ::pc_->get_number_of_particles());
    IMP_NEW(Save, rv, (backup_));
    rv->set_was_used(true);
    MovedSingletonContainer::pc_->apply(rv);
  }
  virtual void save_moved() {
    if (MovedSingletonContainer::pc_->get_number_of_particles() != 0) {
      IMP_NEW(SaveMoved,  cv, (backup_, access()));
      cv->set_was_used(true);
      MovedSingletonContainer::pc_->apply(cv);
    }
  }
  virtual void update_list() {
    if (MovedSingletonContainer::pc_->get_number_of_particles() != 0) {
      bool incr= MovedSingletonContainer
        ::pc_->get_particle(0)->get_model()->get_is_incremental();
      ParticlesTemp ret;
      IMP_NEW(ListMoved,  cv, (backup_, ret,
                               MovedSingletonContainer::threshold_, incr));
      cv->set_was_used(true);
      MovedSingletonContainer::pc_->apply(cv);
      add_to_list(ret);
    }
  }
public:
  //! Track the changes with the specified keys.
  MovedSingletonContainerImpl(Model *m,
                              SingletonContainer *pc,
                              double threshold):
    MovedSingletonContainer(m,pc, threshold){
    if (this==0) {
      // is this legit?
      save();
      save_moved();
      update_list();
    }
  }
};


IMPCORE_END_INTERNAL_NAMESPACE

#endif  /* IMPCORE_INTERNAL_MOVED_SINGLETON_CONTAINER_H */
