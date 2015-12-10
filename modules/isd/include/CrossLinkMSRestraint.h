/**
 *  \file IMP/isd/CrossLinkMSRestraint.h
 *  \brief A pmf based likelihood function
 *  with prior knowledge on the false positive rate.
 *
 *  Copyright 2007-2015 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPISD_CROSS_LINK_MSRESTRAINT_H
#define IMPISD_CROSS_LINK_MSRESTRAINT_H
#include "isd_config.h"
#include <IMP/Restraint.h>
#include <IMP/particle_index.h>

IMPISD_BEGIN_NAMESPACE

//! A restraint for ambiguous cross-linking MS data and multiple state approach.
/** It marginalizes the false positive rate and depends on the expected fpr and
    an uncertainty parameter beta.
 */
class IMPISDEXPORT CrossLinkMSRestraint : public Restraint {

    IMP::ParticleIndexPairs ppis_;
    IMP::ParticleIndexPairs sigmass_;
    IMP::ParticleIndex lengthi_;
    IMP::ParticleIndexes psis_;
    double length_;
    double slope_;
    int constr_;

    double sphere_cap(float r1, float r2, float d) const;

   public:
    //! Create the restraint.
    /** Restraints should store the particles they are to act on,
        preferably in a Singleton or PairContainer as appropriate.
     */

    CrossLinkMSRestraint(IMP::Model* m, double length,
                         std::string name = "CrossLinkMSRestraint%1%");

    CrossLinkMSRestraint(IMP::Model* m,
                         IMP::ParticleIndexAdaptor lengthi,
                         std::string name = "CrossLinkMSRestraint%1%");

    CrossLinkMSRestraint(IMP::Model* m, double length, double slope,
                         std::string name = "CrossLinkMSRestraint%1%");

    void add_contribution(const IMP::ParticleIndexPair& pps,
                          const IMP::ParticleIndexPair& sigmas,
                          IMP::ParticleIndex psi) {
        ppis_.push_back(pps);
        sigmass_.push_back(sigmas);
        psis_.push_back(psi);
    }

    double get_probability() const;

    unsigned int get_number_of_contributions() const { return ppis_.size(); }

    /** This macro declares the basic needed methods: evaluate and show
     */
    virtual double unprotected_evaluate(
        IMP::DerivativeAccumulator* accum) const IMP_OVERRIDE;
    virtual IMP::ModelObjectsTemp do_get_inputs() const IMP_OVERRIDE;
    IMP_OBJECT_METHODS(CrossLinkMSRestraint);
};

IMPISD_END_NAMESPACE

#endif /* IMPISD_CROSS_LINK_MSRESTRAINT_H */
