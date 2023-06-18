/**
 *  \file IMP/isd/FretRestraint.h
 *  \brief FRET_R restraint to use in vivo FRET data.
 *
 *  Copyright 2007-2022 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPISD_FRET_RESTRAINT_H
#define IMPISD_FRET_RESTRAINT_H
#include <IMP/isd/Scale.h>
#include <IMP/isd/isd_config.h>
#include <IMP/Restraint.h>
#include <IMP/isd/FretData.h>
#include <cereal/access.hpp>
#include <cereal/types/vector.hpp>

IMPISD_BEGIN_NAMESPACE
/** A restraint for using in-vivo ensemble FRET data.
    See [Muller et al. Mol Biol Cell
    16, 3341, 2005](https://www.ncbi.nlm.nih.gov/pmc/articles/PMC1165416/)
    for more info.
 */

class IMPISDEXPORT FretRestraint : public Restraint {
  ParticleIndexes pd_;
  ParticleIndexes pa_;
  ParticleIndex prd_;
  algebra::Vector3D GMMterd_;
  algebra::Vector3Ds GMMctrd_;
  ParticleIndex pra_;
  algebra::Vector3D GMMtera_;
  algebra::Vector3Ds GMMctra_;
  ParticleIndex kda_;
  ParticleIndex Ida_;
  ParticleIndex R0_;
  ParticleIndex sumFi_;
  ParticleIndex sigma0_;
  ParticleIndex Pbl_;
  PointerMember<FretData> data_;
  double fexp_;
  double multi_d_;
  int constr_type_;
  Floats GMMsig_;
  Floats GMMw_;
  unsigned Na_;
  mutable Floats power6_;
  std::vector<std::vector<unsigned> > states_;

  friend class cereal::access;

  template<class Archive> void serialize(Archive &ar) {
    ar(cereal::base_class<Restraint>(this), pd_, pa_, prd_, GMMterd_,
       GMMctrd_, pra_, GMMtera_, GMMctra_, kda_, Ida_, R0_, sumFi_, sigma0_,
       Pbl_, data_, fexp_, multi_d_, constr_type_, GMMsig_, GMMw_, Na_,
       power6_, states_);
  }

  IMP_OBJECT_SERIALIZE_DECL(FretRestraint);

  double get_sumFi(double Pbleach) const;
  std::vector<unsigned> get_indices(unsigned index, int dimension) const;

 public:
  //! Create the restraint.
  FretRestraint(ParticlesTemp pd, ParticlesTemp pa,
                Particle *kda, Particle *Ida,
                Particle *R0, Particle *sigma0,
                Particle *Pbl, double fexp, double m_d = 1.0,
                double m_a = 1.0);

  FretRestraint(Particle *kda, Particle *Ida,
                Particle *sumFi, Particle *sigma0,
                Particle *Pbl, double fexp);

  FretRestraint(Particle *prd, algebra::Vector3D GMMterd,
                algebra::Vector3Ds GMMctrd, Floats GMMwd, Floats GMMsigd,
                Particle *pra, algebra::Vector3D GMMtera,
                algebra::Vector3Ds GMMctra, Floats GMMwa, Floats GMMsiga,
                Particle *kda, Particle *Ida,
                Particle *sigma0, Particle *Pbl, FretData *data,
                double fexp);

  FretRestraint() {}

  // get sumFi
  double get_sumFi() const;

  // get average sigma
  double get_average_sigma(double fmod) const;

  // lognormal standard error (according to Wikipedia)
  double get_standard_error() const;

  // get model fret_r (for the two constructors, i.e. type 0 and 1)
  double get_model_fretr() const;

  // get probability
  double get_probability() const;

  // get experimental value
  double get_experimental_value() const {
    return fexp_;
  };

  // set experimental value
  void set_experimental_value(double fexp);

  virtual double unprotected_evaluate(IMP::DerivativeAccumulator *accum)
      const override;
  virtual IMP::ModelObjectsTemp do_get_inputs() const override;
  IMP_OBJECT_METHODS(FretRestraint);

 private:
  double get_model_fretr_type_0() const;
  double get_model_fretr_type_1() const;
  algebra::Vector3Ds get_current_centers(ParticleIndex p,
                                         const algebra::Vector3Ds &ctrs) const;

  algebra::Vector3D get_current_center(ParticleIndex p,
                                       const algebra::Vector3D &ctr) const;
};

IMPISD_END_NAMESPACE

#endif /* IMPISD_FRET_RESTRAINT_H */
