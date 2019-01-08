/**
 *  \file isd/LogWrapper.cpp
 *  \brief Calculate the -Log of a list of restraints.
 *
 *  Copyright 2007-2019 IMP Inventors. All rights reserved.
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
      prob *= get_restraint(i)->unprotected_evaluate(accum);
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

void LogWrapper::do_add_score_and_derivatives(ScoreAccumulator sa) const {
  // override RestraintSet implementation
  Restraint::do_add_score_and_derivatives(sa);
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
