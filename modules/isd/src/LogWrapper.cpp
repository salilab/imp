/**
 *  \file isd/LogWrapper.cpp
 *  \brief Calculate the -Log of a list of restraints.
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/isd/LogWrapper.h>
#include <IMP/internal/scoring_functions.h>
#include <math.h>

IMPISD_BEGIN_NAMESPACE

LogWrapper::LogWrapper(Model *m, double weight,
                       const std::string &name)
  : RestraintSet(m, weight, name) {}

LogWrapper::LogWrapper(Model *m, const std::string &name)
  : RestraintSet(m, name) {}

LogWrapper::LogWrapper(const RestraintsTemp &rs,double weight,
                       const std::string &name)
  : RestraintSet(rs, weight, name) {}

double LogWrapper::unprotected_evaluate(DerivativeAccumulator* accum) const {
    double prob = 1;
    double score = 0;

    for (unsigned int i = 0; i <get_number_of_restraints(); ++i) {
      double rsrval = get_restraint(i)->unprotected_evaluate(accum);
      get_restraint(i)->set_last_score(rsrval);
      prob *= rsrval;
      if (prob<=std::numeric_limits<double>::min()*1000000.0){
        score=score-std::log(prob);
        prob=1.0;
      }
    }

    score=score-std::log(prob);
    if (accum) {
      // derivatives should be accurately handled in the restraints
    }
    return score;
}

double LogWrapper::unprotected_evaluate_moved(
        DerivativeAccumulator* accum, const ParticleIndexes &moved_pis) const {
  // If we only want the score, and only a single particle moved, only
  // evaluate the restraints that depend on that particle, and use the
  // last score for the rest
  if (!accum && moved_pis.size() == 1) {
    const std::set<Restraint *> &rsset
           = get_model()->get_dependent_restraints(moved_pis[0]);

    double prob = 1;
    double score = 0;

    for (unsigned int i = 0; i <get_number_of_restraints(); ++i) {
      Restraint *r = get_restraint(i);
      if (rsset.find(r) == rsset.end()) {
        double last_score = r->get_last_score();
        // If the restraint is new, get the full score
        if (last_score == NO_MAX) {
          double rsrval = r->unprotected_evaluate(accum);
          r->set_last_score(rsrval);
          prob *= rsrval;
        } else {
          prob *= last_score;
        }
      } else {
        double rsrval = r->unprotected_evaluate_moved(accum, moved_pis);
        r->set_last_score(rsrval);
        prob *= rsrval;
      }
      if (prob<=std::numeric_limits<double>::min()*1000000.0){
        score=score-std::log(prob);
        prob=1.0;
      }
    }

    return score-std::log(prob);
  } else {
    return unprotected_evaluate(accum);
  }
}

void LogWrapper::do_add_score_and_derivatives(ScoreAccumulator sa) const {
  // override RestraintSet implementation
  Restraint::do_add_score_and_derivatives(sa);
}

void LogWrapper::do_add_score_and_derivatives_moved(
                ScoreAccumulator sa, const ParticleIndexes &moved_pis) const {
  // override RestraintSet implementation
  Restraint::do_add_score_and_derivatives_moved(sa, moved_pis);
}

void LogWrapper::show_it(std::ostream &out) const {
  IMP_CHECK_OBJECT(this);
  for (RestraintConstIterator it = restraints_begin(); it != restraints_end();
       ++it) {
    (*it)->show(out);
  }
  out << "... end LogWrapper " << get_name() << std::endl;
}

IMPISD_END_NAMESPACE
