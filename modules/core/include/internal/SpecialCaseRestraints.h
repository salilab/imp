/**
 *  \file SpecialCaseRestraints.h
 *  \brief Keep track of the maximum change of a set of attributes.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 */

#ifndef IMPCORE_INTERNAL_SPECIAL_CASE_RESTRAINTS_H
#define IMPCORE_INTERNAL_SPECIAL_CASE_RESTRAINTS_H

#include "../core_config.h"

#include "../DistancePairScore.h"
#include "../SphereDistancePairScore.h"
#include "../Harmonic.h"
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

IMPCORE_BEGIN_INTERNAL_NAMESPACE

/*
  Go through restraints and pick out certain types (particles
  attached by harmonics, for example) and give the called an
  opportunity to replace those with something else. A return
  value of true means they have been handled and should be removed.
*/
class IMPCOREEXPORT SpecialCaseRestraints {
  boost::ptr_vector< ScopedRemoveRestraint> restraints_;
  ParticlesTemp ps_;
  DependencyGraph dg_;
 public:
  SpecialCaseRestraints(Model *m, const ParticlesTemp &ps);
  template <class Harmonic, class Harmonics, class EV>
    void add_restraint_set(RestraintSet *rs,
                           Harmonic fh, Harmonics fhs,
                           EV fev) {
    Restraints rss(rs->restraints_begin(), rs->restraints_end());
    for (unsigned int i=0; i < rss.size(); ++i) {
      IMP_LOG(VERBOSE, "Inspecting restraint " << rss[i]->get_name()
              << std::endl);
      Restraint *r= rss[i];
      if (dynamic_cast<PairScoreRestraint*>(r)) {
        PairScoreRestraint*pr= dynamic_cast<PairScoreRestraint*>(r);
        PairScore *ps= pr->get_score();
        HarmonicDistancePairScore *hdps
          = dynamic_cast<core::HarmonicDistancePairScore*>(ps);
        if (hdps) {
          IMP_LOG(TERSE, "Handling restraint " << pr->get_name() << std::endl);
          double x0= hdps->get_rest_length();
          double k= hdps->get_stiffness();
          if (fh(pr->get_argument(), x0, k)) {
            restraints_.push_back(new ScopedRemoveRestraint(pr,rs));
          }
        } else {
          DistancePairScore *dps= dynamic_cast<DistancePairScore*>(ps);
          if (dps) {
            UnaryFunction *uf= dps->get_unary_function();
            IMP::core::Harmonic *h= dynamic_cast<IMP::core::Harmonic*>(uf);
            if (h) {
              double k= h->get_k();
              double x0= h->get_mean();
              if (fh(pr->get_argument(), x0, k)) {
                restraints_.push_back(new ScopedRemoveRestraint(pr,rs));
              }
            }
          }
        }
      } else if (dynamic_cast<PairsScoreRestraint*>(r)) {
        PairsScoreRestraint*pr
          = dynamic_cast<PairsScoreRestraint*>(r);
        ContainersTemp ct= pr->get_input_containers();
        PairScore *pscore= pr->get_score();
        if (ct.size()==1 && get_is_static_container(ct[0], dg_, ps_)) {
          core::HarmonicDistancePairScore *hdps
            = dynamic_cast<core::HarmonicDistancePairScore*>(pscore);
          if (hdps) {
            IMP_LOG(TERSE, "Handling restraint " << pr->get_name()
                    << std::endl);
            double x0= hdps->get_rest_length();
            double k= hdps->get_stiffness();
            ParticlePairsTemp ppt= pr->get_arguments();
            if (fhs(ppt, x0, k)) {
              restraints_.push_back(new ScopedRemoveRestraint(pr,rs));
            }
          }
        } else {
          if (dynamic_cast<core::SoftSpherePairScore*>(pscore)) {
            IMP_LOG(TERSE, "Handling restraint " << pr->get_name()
                    << std::endl);
            if (fev()) {
              restraints_.push_back(new ScopedRemoveRestraint(pr,rs));
            }
          }
        }
      }
    }
  }
};


IMPCORE_END_INTERNAL_NAMESPACE

#endif  /* IMPCORE_INTERNAL_SPECIAL_CASE_RESTRAINTS_H */
