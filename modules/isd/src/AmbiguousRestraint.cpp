/**
 *  \file isd/AmbiguousRestraint.cpp
 *  \brief Restrain a list of particle pairs with a lognormal+ISPA.
 *  NOTE: for now, the derivatives are written to all variables.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/isd/AmbiguousRestraint.h>
#include <math.h>

IMPISD_BEGIN_NAMESPACE

AmbiguousRestraint::AmbiguousRestraint(int d, Restraint *r0, Restraint *r1)
: d_(d)
{
    rs_.push_back(r0);
    rs_.push_back(r1);
}

AmbiguousRestraint::AmbiguousRestraint(int d, Restraints rs) : d_(d),rs_(rs) {}

/* Apply the restraint by computing the d-norm
 */
double
AmbiguousRestraint::unprotected_evaluate(DerivativeAccumulator *accum) const
{
    base::Vector<double> enes;
    double ene=0;
    for (unsigned int i=0; i < rs_.size(); ++i) {
        enes.push_back(rs_[i]->unprotected_evaluate(nullptr));
        ene += pow(enes[i], d_);
    }
    ene = pow(ene, 1.0/d_);
    if (accum)
    {
        for (unsigned int i=0; i < rs_.size(); ++i) {
            DerivativeAccumulator a0(*accum,pow(enes[i],d_-1)*pow(ene,1-d_));
            rs_[i]->unprotected_evaluate(&a0);
        }
    }
    return ene;
}

/* Return all particles whose attributes are read by the restraints. To
   do this, ask the pair score what particles it uses.*/
ModelObjectsTemp AmbiguousRestraint::do_get_inputs() const
{
  ModelObjectsTemp ret;
  for (unsigned int i=0; i<rs_.size(); ++i){
    ret+=rs_[i]->get_inputs();
  }
  return ret;
}

IMPISD_END_NAMESPACE
