/**
 *  \file IMP/isd/CrossLinkMSRestraint.h
 *  \brief A pmf based likelihood function
 *  with prior knowledge on the false positive rate.
 *
 *  Copyright 2007-2022 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPISD_CROSS_LINK_MSRESTRAINT_H
#define IMPISD_CROSS_LINK_MSRESTRAINT_H

#include <IMP/isd/isd_config.h>
#include <IMP/Restraint.h>
#include <IMP/particle_index.h>
#include <IMP/isd/Scale.h>
#include <cereal/access.hpp>
#include <cereal/types/base_class.hpp>
#include <cereal/types/polymorphic.hpp>

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
    std::string protein1_, protein2_;
    int residue1_, residue2_;

    friend class cereal::access;

    template<class Archive> void serialize(Archive &ar) {
      ar(cereal::base_class<Restraint>(this),
         ppis_, sigmass_, lengthi_, psis_, length_, slope_,
         constr_, get_log_prob_, protein1_, protein2_, residue1_, residue2_);
    }

    double sphere_cap(float r1, float r2, float d) const;

    IMP_OBJECT_SERIALIZE_DECL(CrossLinkMSRestraint);

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

    CrossLinkMSRestraint() {}

    /** \name Source information
        These methods get or set the protein name and residue index of
        each end of the cross-link, as identified in the experiment
        (typically these will correspond to the information in a CSV
        file or similar). This information is not used in modeling, but
        is written into output RMF files and is needed to generate mmCIF files.
        %{
      */
    //! Set protein name for one end of the cross-link
    void set_source_protein1(std::string protein1) { protein1_ = protein1; }

    //! Get protein name for one end of the cross-link
    std::string get_source_protein1() const { return protein1_; }

    //! Set protein name for one end of the cross-link
    void set_source_protein2(std::string protein2) { protein2_ = protein2; }

    //! Get protein name for one end of the cross-link
    std::string get_source_protein2() const { return protein2_; }

    //! Set residue number for one end of the cross-link
    void set_source_residue1(int residue1) { residue1_ = residue1; }

    //! Get residue number for one end of the cross-link
    int get_source_residue1() const { return residue1_; }

    //! Set residue number for one end of the cross-link
    void set_source_residue2(int residue2) { residue2_ = residue2; }

    //! Get residue number for one end of the cross-link
    int get_source_residue2() const { return residue2_; }
    /** @} */

    void add_contribution(const IMP::ParticleIndexPair& pps,
                          const IMP::ParticleIndexPair& sigmas,
                          IMP::ParticleIndex psi) {
        ppis_.push_back(pps);
        sigmass_.push_back(sigmas);
        psis_.push_back(psi);
    }

    double get_probability() const;

    //! Get the length of this restraint
    /** Note that if the restraint was constructed with a length Scale,
        this can change during sampling. */
    double get_length() const {
      if (constr_ == 1) {
        return isd::Scale(get_model(), lengthi_).get_scale();
      } else {
        return length_;
      }
    }

    //! Return true iff the length is variable (stored in a Scale)
    bool get_is_length_variable() const {
      return constr_ == 1;
    }

    //! Return true iff the restraint has a slope
    bool get_has_slope() const {
      return constr_ == 2;
    }

    double get_slope() const { return slope_; }

    bool get_log_prob() const { return get_log_prob_; }

    //! Get the sigma particle indexes from a contribution
    ParticleIndexPair get_contribution_sigma_indexes(int i) const {
      return sigmass_[i];
    }

    //! Get the psi particle index from a contribution
    ParticleIndex get_contribution_psi_index(int i) const {
      return psis_[i];
    }

    //! Get the particle indexes from a contribution
    ParticleIndexPair get_contribution_particle_indexes(int i) const {
      return ppis_[i];
    }

    unsigned int get_number_of_contributions() const { return ppis_.size(); }

    virtual double unprotected_evaluate(
        IMP::DerivativeAccumulator* accum) const override;
    virtual IMP::ModelObjectsTemp do_get_inputs() const override;

    //! \return Information for writing to RMF files
    RestraintInfo *get_static_info() const override;

    IMP_OBJECT_METHODS(CrossLinkMSRestraint);
};

IMPISD_END_NAMESPACE

#endif /* IMPISD_CROSS_LINK_MSRESTRAINT_H */
