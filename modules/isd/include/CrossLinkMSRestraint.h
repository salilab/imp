/**
 *  \file IMP/isd/CrossLinkMSRestraint.h
 *  \brief A pmf based likelihood function
 *  with prior knowledge on the false positive rate.
 *
 *  Copyright 2007-2019 IMP Inventors. All rights reserved.
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
    bool get_log_prob_;

    double sphere_cap(float r1, float r2, float d) const;

   public:
    //! Create the restraint.
    CrossLinkMSRestraint(IMP::Model* m, double length,
                         bool get_log_prob = false,
                         std::string name = "CrossLinkMSRestraint%1%");

    CrossLinkMSRestraint(IMP::Model* m,
                         IMP::ParticleIndexAdaptor lengthi,
                         bool get_log_prob = false,                         
                         std::string name = "CrossLinkMSRestraint%1%");

    CrossLinkMSRestraint(IMP::Model* m, double length, double slope,
                         bool get_log_prob = false,
                         std::string name = "CrossLinkMSRestraint%1%");

    void add_contribution(const IMP::ParticleIndexPair& pps,
                          const IMP::ParticleIndexPair& sigmas,
                          IMP::ParticleIndex psi) {
        ppis_.push_back(pps);
        sigmass_.push_back(sigmas);
        psis_.push_back(psi);
    }

    double get_probability() const;

    //! Get the particle indexes from a contribution
    ParticleIndexPair get_contribution_particle_indexes(int i) const { return ppis_[i]; }

    unsigned int get_number_of_contributions() const { return ppis_.size(); }

    virtual double unprotected_evaluate(
        IMP::DerivativeAccumulator* accum) const IMP_OVERRIDE;
    virtual IMP::ModelObjectsTemp do_get_inputs() const IMP_OVERRIDE;
    IMP_OBJECT_METHODS(CrossLinkMSRestraint);
};

IMPISD_END_NAMESPACE

#endif /* IMPISD_CROSS_LINK_MSRESTRAINT_H */
