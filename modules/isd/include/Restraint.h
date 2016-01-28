/**
 *  \file IMP/isd/Restraint.h
 *
 *  Copyright 2007-2016 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPISD_RESTRAINT_H
#define IMPISD_RESTRAINT_H
#include "isd_config.h"
#include <IMP/Restraint.h>
#include <string>

IMPISD_BEGIN_NAMESPACE
/** A base class for ISD Restraints.
 */

class IMPISDEXPORT Restraint : public IMP::Restraint {

 public:
  //! Create the restraint.
  /** Restraints should store the particles they are to act on,
      preferably in a Singleton or PairContainer as appropriate.
   */

    Restraint(IMP::Model *m, std::string name);

    virtual double get_probability() const;

    virtual double unprotected_evaluate(IMP::DerivativeAccumulator *accum)
      const IMP_OVERRIDE;

    virtual IMP::ModelObjectsTemp do_get_inputs() const IMP_OVERRIDE;

    IMP_OBJECT_METHODS(Restraint);
    
};

IMP_OBJECTS(Restraint,Restraints);

IMPISD_END_NAMESPACE

#endif /* IMPISD_RESTRAINT_H */
