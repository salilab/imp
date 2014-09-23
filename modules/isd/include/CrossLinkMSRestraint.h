/**
 *  \file IMP/isd/CrossLinkMSRestraint.h
 *  \brief A pmf based likelihood function
 *  with prior knowledge on the flase positive rate.
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPISD_CROSS_LINK_MSRESTRAINT_H
#define IMPISD_CROSS_LINK_MSRESTRAINT_H
#include "isd_config.h"
#include <IMP/kernel/Restraint.h>

IMPISD_BEGIN_NAMESPACE
/** A restraint for ambiguous cross-linking MS data and multiple state approach.
    It marginalizes the false positive rate and depends on the expected fpr and
    an uncertainty parameter beta.
 */

class IMPISDEXPORT CrossLinkMSRestraint : public kernel::Restraint {

    IMP::kernel::ParticleIndexPairs ppis_;
    IMP::kernel::ParticleIndexPairs sigmass_;
    IMP::kernel::ParticleIndex lengthi_;
    IMP::kernel::ParticleIndexes psis_;
    double length_;
    double slope_;
    int constr_;

    double sphere_cap(float r1, float r2, float d) const;

   public:
    //! Create the restraint.
    /** Restraints should store the particles they are to act on,
        preferably in a Singleton or PairContainer as appropriate.
     */

    CrossLinkMSRestraint(IMP::kernel::Model* m, double length,
                         std::string name = "CrossLinkMSRestraint%1%");

    CrossLinkMSRestraint(IMP::kernel::Model* m,
                         IMP::kernel::ParticleIndexAdaptor lengthi,
                         std::string name = "CrossLinkMSRestraint%1%");

    CrossLinkMSRestraint(IMP::kernel::Model* m, double length, double slope,
                         std::string name = "CrossLinkMSRestraint%1%");

    void add_contribution(const IMP::kernel::ParticleIndexPair& pps,
                          const IMP::kernel::ParticleIndexPair& sigmas,
                          const IMP::kernel::ParticleIndex& psi) {
        ppis_.push_back(pps);
        sigmass_.push_back(sigmas);
        psis_.push_back(psi);
    }

    double get_probability() const;

    unsigned int get_number_of_contributions() const { return ppis_.size(); }

    /** This macro declares the basic needed methods: evaluate and show
     */
    virtual double unprotected_evaluate(
        IMP::kernel::DerivativeAccumulator* accum) const IMP_OVERRIDE;
    virtual IMP::kernel::ModelObjectsTemp do_get_inputs() const IMP_OVERRIDE;
    IMP_OBJECT_METHODS(CrossLinkMSRestraint);
};

IMPISD_END_NAMESPACE

#endif /* IMPISD_CROSS_LINK_MSRESTRAINT_H */
