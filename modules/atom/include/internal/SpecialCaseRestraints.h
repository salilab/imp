/**
 *  \file SpecialCaseRestraints.h
 *  \brief Keep track of the maximum change of a set of attributes.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 */

#ifndef IMPATOM_INTERNAL_SPECIAL_CASE_RESTRAINTS_H
#define IMPATOM_INTERNAL_SPECIAL_CASE_RESTRAINTS_H

#include "../atom_config.h"

#include <IMP/core/DistancePairScore.h>
#include <IMP/core/SphereDistancePairScore.h>
#include <IMP/core/Harmonic.h>
#include "../BondSingletonScore.h"
#include <IMP/RestraintSet.h>
#include <IMP/scoped.h>
#include <boost/ptr_container/ptr_vector.hpp>

#ifndef IMP_DOXYGEN
IMP_BEGIN_NAMESPACE
bool
get_is_static_container(Container *c,
                        const DependencyGraph &dg,
                        const ParticlesTemp &pst);
IMP_END_NAMESPACE
#endif

IMPATOM_BEGIN_INTERNAL_NAMESPACE

/*
  Go through restraints and pick out certain types (particles
  attached by harmonics, for example) and give the called an
  opportunity to replace those with something else. A return
  value of true means they have been handled and should be removed.
*/
class IMPATOMEXPORT SpecialCaseRestraints {
  boost::ptr_vector< ScopedRemoveRestraint> restraints_;
  ParticlesTemp ps_;
  DependencyGraph dg_;

  bool get_harmonic_info(PairScore*ps, const ParticlePair &pp,
                         double &x0,
                         double &k) {
    if (dynamic_cast<core::HarmonicDistancePairScore*>(ps)) {
      core::HarmonicDistancePairScore *hdps
        = dynamic_cast<core::HarmonicDistancePairScore*>(ps);
      x0= hdps->get_rest_length();
      k= hdps->get_stiffness();
      return true;
    } else if (dynamic_cast<core::DistancePairScore*>(ps))  {
      core::DistancePairScore *dps= dynamic_cast<core::DistancePairScore*>(ps);
      UnaryFunction *uf= dps->get_unary_function();
      if (dynamic_cast<IMP::core::Harmonic*>(uf)) {
        IMP::core::Harmonic *h= dynamic_cast<IMP::core::Harmonic*>(uf);
        k= h->get_k();
        x0= h->get_mean();
        return true;
      }
    } else if (dynamic_cast<core::SphereDistancePairScore*>(ps))  {
      core::SphereDistancePairScore *dps
        = dynamic_cast<core::SphereDistancePairScore*>(ps);
      UnaryFunction *uf= dps->get_unary_function();
      if (dynamic_cast<IMP::core::Harmonic*>(uf)) {
        IMP::core::Harmonic *h= dynamic_cast<IMP::core::Harmonic*>(uf);
        k= h->get_k();
        x0= h->get_mean()+ core::XYZR(pp[0]).get_radius()
          + core::XYZR(pp[1]).get_radius();
        return true;
      }
    }
    return false;
  }
  bool get_harmonic_info(SingletonScore*ps, Particle *p,
                         ParticlePair &pp,
                         double &x0,
                         double &k) {
    if (dynamic_cast<atom::BondSingletonScore*>(ps)) {
      atom::BondSingletonScore *bss
        = dynamic_cast<atom::BondSingletonScore*>(ps);
      UnaryFunction *f= bss->get_unary_function();
      if  (dynamic_cast<core::Harmonic*>(f)) {
        core::Harmonic *h= dynamic_cast<core::Harmonic*>(f);
        x0=h->get_mean();
        k= h->get_k();
        atom::Bond b(p);
        x0-= b.get_length();
        k*= b.get_stiffness();
        pp= ParticlePair(b.get_bonded(0),
                         b.get_bonded(1));
        return true;
      }
    }
    return false;
  }

  template <class Harmonic, class EV>
    void handle_pair_score_restraint(PairScoreRestraint* pr,
                                     RestraintSet *rs,
                                     Harmonic fh,
                                     EV fev) {
    PairScore *ps= pr->get_score();
    double x0, k;
    if (get_harmonic_info(ps, pr->get_argument(), x0, k)) {
      if (fh(pr->get_argument(), x0, k)) {
        restraints_.push_back(new ScopedRemoveRestraint(pr,rs));
      }
    }
  }
  template <class Harmonic, class EV>
  void handle_pairs_score_restraint(PairsScoreRestraint *pr,
                                     RestraintSet *rs,
                                     Harmonic fh,
                                    EV fev) {
    ContainersTemp ct= pr->get_input_containers();
    PairScore *ps= pr->get_score();
    if (ct.size()==1 && get_is_static_container(ct[0], dg_, ps_)) {
      ParticlePairsTemp ppts= pr->get_arguments();
      bool handled=false;
      for (unsigned int i=0; i< ppts.size(); ++i) {
        double x0, k;
        if (get_harmonic_info(ps, ppts[i], x0,k)) {
          ParticlePair pp= ppts[i];
          handled= fh(pp, x0, k);
        } else {
          IMP_USAGE_CHECK(!handled, "Can't mix harmonics and not.");
        }
      }
      if (handled) {
        restraints_.push_back(new ScopedRemoveRestraint(pr,rs));
      }
    } else {
      if (dynamic_cast<core::SoftSpherePairScore*>(ps)) {
        IMP_LOG(TERSE, "Handling restraint " << pr->get_name()
                << std::endl);
        if (fev()) {
          restraints_.push_back(new ScopedRemoveRestraint(pr,rs));
        }
      }
    }
  }

 template <class Harmonic, class EV>
    void handle_singleton_score_restraint(SingletonScoreRestraint* pr,
                                     RestraintSet *rs,
                                     Harmonic fh,
                                     EV fev) {
    SingletonScore *ps= pr->get_score();
    double x0, k;
    ParticlePair pp;
    if (get_harmonic_info(ps, pr->get_argument(), pp, x0, k)) {
      if (fh(pp, x0, k)) {
        restraints_.push_back(new ScopedRemoveRestraint(pr,rs));
      }
    }
  }
  template <class Harmonic, class EV>
    void handle_singletons_score_restraint(SingletonsScoreRestraint *pr,
                                     RestraintSet *rs,
                                     Harmonic fh,
                                    EV fev) {
    ContainersTemp ct= pr->get_input_containers();
    SingletonScore *ps= pr->get_score();
    if (ct.size()==1 && get_is_static_container(ct[0], dg_, ps_)) {
      ParticlesTemp ppts= pr->get_arguments();
      bool handled=false;
      for (unsigned int i=0; i< ppts.size(); ++i) {
        double x0, k;
        ParticlePair pp;
        if (get_harmonic_info(ps, ppts[i], pp, x0,k)) {
          handled= fh(pp, x0, k);
        } else {
          IMP_USAGE_CHECK(!handled, "Can't mix harmonics and not.");
        }
      }
      if (handled) {
        restraints_.push_back(new ScopedRemoveRestraint(pr,rs));
      }
    }
  }
 public:
  SpecialCaseRestraints(Model *m, const ParticlesTemp &ps);
  template <class Harmonic, class EV>
    void add_restraint_set(RestraintSet *rs,
                           Harmonic fh,
                           EV fev) {
    Restraints rss(rs->restraints_begin(), rs->restraints_end());
    for (unsigned int i=0; i < rss.size(); ++i) {
      IMP_LOG(VERBOSE, "Inspecting restraint " << rss[i]->get_name()
              << std::endl);
      Restraint *r= rss[i];
      if (dynamic_cast<PairScoreRestraint*>(r)) {
        handle_pair_score_restraint(dynamic_cast<PairScoreRestraint*>(r),
                                   rs, fh, fev);
      } else if (dynamic_cast<PairsScoreRestraint*>(r)) {
        handle_pairs_score_restraint(dynamic_cast<PairsScoreRestraint*>(r),
                                     rs, fh, fev);
      } else if (dynamic_cast<SingletonScoreRestraint*>(r)) {
        handle_singleton_score_restraint(
                       dynamic_cast<SingletonScoreRestraint*>(r),
                                          rs, fh, fev);
      } else if (dynamic_cast<SingletonsScoreRestraint*>(r)) {
        handle_singletons_score_restraint(
                        dynamic_cast<SingletonsScoreRestraint*>(r),
                                          rs, fh, fev);
      } else if (dynamic_cast<RestraintSet*>(r)) {
        add_restraint_set(dynamic_cast<RestraintSet*>(r),
                          fh, fev);
      }
    }
  }
};


IMPATOM_END_INTERNAL_NAMESPACE

#endif  /* IMPATOM_INTERNAL_SPECIAL_CASE_RESTRAINTS_H */
