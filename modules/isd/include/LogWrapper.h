/**
 *  \file IMP/isd/LogWrapper.h
 *  \brief Calculate the -Log of a list of restraints.
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPISD_LOG_WRAPPER_H
#define IMPISD_LOG_WRAPPER_H

#include "isd_config.h"
#include <IMP/container_macros.h>
#include <IMP/RestraintSet.h>

IMPISD_BEGIN_NAMESPACE

//! Calculate the -Log of a list of restraints.
class IMPISDEXPORT LogWrapper : public RestraintSet {
  void show_it(std::ostream &out) const;

   public:
    //! Create an empty set that is registered with the model
    LogWrapper(Model *m, double weight, const std::string &name = "LogWrapper %1%");
    //! Create an empty set that is registered with the model
    LogWrapper(Model *m, const std::string &name = "LogWrapper %1%");
    //! Create a set that is registered with the model
    LogWrapper(const RestraintsTemp &rs, double weight,
               const std::string &name = "LogWrapper %1%");

    virtual double unprotected_evaluate(
        IMP::DerivativeAccumulator* accum) const IMP_OVERRIDE;
    void do_add_score_and_derivatives(ScoreAccumulator sa) const;

    IMP_OBJECT_METHODS(LogWrapper);

};

IMPISD_END_NAMESPACE

#endif /* IMPISD_LOG_WRAPPER_H */
