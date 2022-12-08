/**
 *  \file isd/LogWrapper.cpp
 *  \brief Calculate the -Log of a list of restraints.
 *
 *  Copyright 2007-2022 IMP Inventors. All rights reserved.
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

namespace {
void add_last_score_restraint(Restraint *r, DerivativeAccumulator *accum,
#if IMP_HAS_CHECKS >= IMP_INTERNAL
                              const ParticleIndexes &moved_pis,
                              const ParticleIndexes &reset_pis,
#endif
                              double &prob, double last_score) {
  // If the restraint is new, get the full score
  if (last_score == NO_MAX) {
    double rsrval = r->unprotected_evaluate(accum);
    r->set_last_score(rsrval);
    prob *= rsrval;
  } else {
#if IMP_HAS_CHECKS >= IMP_INTERNAL
    double rsrval = r->unprotected_evaluate_moved(accum, moved_pis,
                                                  reset_pis);
    IMP_INTERNAL_CHECK_FLOAT_EQUAL(
           rsrval, last_score,
           "Restraint " << *r
           << " changed score even though particles didn't move");
#endif
    prob *= last_score;
  }
}
}

double LogWrapper::unprotected_evaluate_moved(
        DerivativeAccumulator* accum, const ParticleIndexes &moved_pis,
        const ParticleIndexes &reset_pis) const {
  // If we only want the score, and only a single particle moved, only
  // evaluate the restraints that depend on that particle, and use the
  // last score for the rest
  if (!accum && moved_pis.size() == 1 && reset_pis.size() == 0) {
    const std::set<Restraint *> &rsset
           = get_model()->get_dependent_restraints(moved_pis[0]);

    double prob = 1;
    double score = 0;

    for (unsigned int i = 0; i <get_number_of_restraints(); ++i) {
      Restraint *r = get_restraint(i);
      if (rsset.find(r) != rsset.end()) {
        double rsrval = r->unprotected_evaluate_moved(accum, moved_pis,
                                                      reset_pis);
        r->set_last_score(rsrval);
        prob *= rsrval;
      } else {
        add_last_score_restraint(r, accum,
#if IMP_HAS_CHECKS >= IMP_INTERNAL
                                 moved_pis, reset_pis,
#endif
                                 prob, r->get_last_score());
      }
      if (prob<=std::numeric_limits<double>::min()*1000000.0){
        score=score-std::log(prob);
        prob=1.0;
      }
    }

    return score-std::log(prob);
  // If we have *both* moved and reset particles, we need to check each
  // restraint for both
  } else if (!accum && moved_pis.size() == 1 && reset_pis.size() == 1) {
    const std::set<Restraint *> &moved_set
           = get_model()->get_dependent_restraints(moved_pis[0]);
    const std::set<Restraint *> &reset_set
           = get_model()->get_dependent_restraints(reset_pis[0]);

    double prob = 1;
    double score = 0;

    for (unsigned int i = 0; i <get_number_of_restraints(); ++i) {
      Restraint *r = get_restraint(i);
      // must check moved first, since if a given restraint is affected by
      // *both* moved and reset particles, we need to recalculate it
      if (moved_set.find(r) != moved_set.end()) {
        double rsrval;
        // preserve last-last score if reset
        if (reset_set.find(r) != reset_set.end()) {
          double last_last_score = r->get_last_last_score();
          rsrval = r->unprotected_evaluate_moved(accum, moved_pis, reset_pis);
          r->set_last_score(rsrval);
          r->set_last_last_score(last_last_score);
        } else {
          rsrval = r->unprotected_evaluate_moved(accum, moved_pis, reset_pis);
          r->set_last_score(rsrval);
        }
        prob *= rsrval;
      } else if (reset_set.find(r) != reset_set.end()) {
        // If reset, we can use the last-but-one score, unless it is an
        // aggregate restraint (e.g. a RestraintSet), in which case
        // have the restraint itself figure out what to return
        if (r->get_is_aggregate()) {
          double rsrval = r->unprotected_evaluate_moved(
                          accum, moved_pis, reset_pis);
          r->set_last_score(rsrval);
          prob *= rsrval;
        } else {
          double score = r->get_last_last_score();
          add_last_score_restraint(r, accum,
#if IMP_HAS_CHECKS >= IMP_INTERNAL
                                   moved_pis, reset_pis,
#endif
                                   prob, score);
          r->set_last_score(score);
        }
      } else {
        // If not moved, we can use the last score
        add_last_score_restraint(r, accum,
#if IMP_HAS_CHECKS >= IMP_INTERNAL
                                 moved_pis, reset_pis,
#endif
                                 prob, r->get_last_score());
      }
      if (prob<=std::numeric_limits<double>::min()*1000000.0){
        score=score-std::log(prob);
        prob=1.0;
      }
    }
    return score-std::log(prob);
  } else {
    double prob = 1;
    double score = 0;

    for (unsigned int i = 0; i <get_number_of_restraints(); ++i) {
      double rsrval = get_restraint(i)->unprotected_evaluate_moved(
                                           accum, moved_pis, reset_pis);
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
}

void LogWrapper::do_add_score_and_derivatives(ScoreAccumulator sa) const {
  // override RestraintSet implementation
  Restraint::do_add_score_and_derivatives(sa);
}

void LogWrapper::do_add_score_and_derivatives_moved(
                ScoreAccumulator sa, const ParticleIndexes &moved_pis,
                const ParticleIndexes &reset_pis) const {
  // override RestraintSet implementation
  Restraint::do_add_score_and_derivatives_moved(sa, moved_pis, reset_pis);
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
