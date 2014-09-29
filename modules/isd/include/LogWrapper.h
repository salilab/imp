/**
 *  \file IMP/isd/LogWrapper.h
 *  \brief Calculate the -Log of a list of restraints.
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPISD_LOG_WRAPPER_H
#define IMPISD_LOG_WRAPPER_H

#include "isd_config.h"
#include <IMP/kernel/container_macros.h>
#include <IMP/Restraint.h>

IMPISD_BEGIN_NAMESPACE

//! Calculate the -Log of a list of restraints.
class IMPISDEXPORT LogWrapper : public kernel::Restraint {
  void on_add(Restraint *r);
  void on_change();
  static void on_remove(LogWrapper *container, Restraint *r);
  void show_it(std::ostream &out) const;

   public:
    //! Create an empty set that is registered with the model
    LogWrapper(kernel::Model *m, double weight, const std::string &name = "LogWrapper %1%");
    //! Create an empty set that is registered with the model
    LogWrapper(kernel::Model *m, const std::string &name = "LogWrapper %1%");
    //! Create a set that is registered with the model
    LogWrapper(const RestraintsTemp &rs, double weight,
               const std::string &name = "LogWrapper %1%");

    virtual double unprotected_evaluate(
        IMP::kernel::DerivativeAccumulator* accum) const IMP_OVERRIDE;
    virtual IMP::kernel::ModelObjectsTemp do_get_inputs() const IMP_OVERRIDE;
    IMP_OBJECT_METHODS(LogWrapper);

    IMP_LIST_ACTION(public, Restraint, Restraints, restraint, restraints,
                  Restraint *, Restraints, on_add(obj), on_change(),
                  if (container) on_remove(container, obj));
};

IMPISD_END_NAMESPACE

#endif /* IMPISD_LOG_WRAPPER_H */
