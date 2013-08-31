/**
 *  \file isd/FretRestraint.h
 *  \brief FRET_R restraint
 *  to use in vivo FRET data [Muller et al. Mol Biol Cell 16, 3341, 2005]
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPISD_FRET_RESTRAINT_H
#define IMPISD_FRET_RESTRAINT_H
#include <IMP/isd/Scale.h>
#include "isd_config.h"
#include <IMP/isd/ISDRestraint.h>
#include <IMP/isd/FretData.h>

IMPISD_BEGIN_NAMESPACE
/** A restraint for using in-vivo ensemble FRET data,
    see Muller et al. Mol Biol Cell 16, 3341, 2005 for more info.
 */

class IMPISDEXPORT  FretRestraint : public ISDRestraint
{
    kernel::Particles pd_;
    kernel::Particles pa_;
    base::Pointer<kernel::Particle> prd_;
    algebra::Vector3D  GMMterd_;
    algebra::Vector3Ds GMMctrd_;
    base::Pointer<kernel::Particle> pra_;
    algebra::Vector3D  GMMtera_;
    algebra::Vector3Ds GMMctra_;
    base::Pointer<kernel::Particle> kda_;
    base::Pointer<kernel::Particle> Ida_;
    base::Pointer<kernel::Particle> R0_;
    base::Pointer<kernel::Particle> sumFi_;
    base::Pointer<kernel::Particle> sigma0_;
    base::Pointer<kernel::Particle> Pbl_;
    base::PointerMember<FretData> data_;
    double fexp_;
    double multi_d_;
    int constr_type_;
    Floats GMMsig_;
    Floats GMMw_;
    unsigned Na_;
    mutable Floats power6_;
    std::vector< std::vector<unsigned> > states_;

    double get_sumFi(double Pbleach) const;
    std::vector<unsigned> get_indices(unsigned index, int dimension) const;

public:

  //! Create the restraint.
  /** kernel::Restraints should store the particles they are to act on,
      preferably in a Singleton or PairContainer as appropriate.
   */

  FretRestraint (kernel::Particles pd, kernel::Particles pa,
   kernel::Particle *kda, kernel::Particle *Ida, kernel::Particle *R0, kernel::Particle* sigma0,
   kernel::Particle *Pbl, double fexp, double m_d=1.0, double m_a=1.0);

  FretRestraint
  (kernel::Particle *kda, kernel::Particle *Ida, kernel::Particle *sumFi, kernel::Particle* sigma0,
   kernel::Particle *Pbl, double fexp);

  FretRestraint
  (kernel::Particle *prd, algebra::Vector3D GMMterd,
   algebra::Vector3Ds GMMctrd, Floats GMMwd, Floats GMMsigd,
   kernel::Particle *pra, algebra::Vector3D GMMtera,
   algebra::Vector3Ds GMMctra, Floats GMMwa, Floats GMMsiga,
   kernel::Particle *kda, kernel::Particle *Ida, kernel::Particle* sigma0, kernel::Particle *Pbl,
   FretData *data, double fexp);

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
  double get_experimental_value() const {return fexp_;};

  // set experimental value
  void set_experimental_value(double fexp);

  /** This macro declares the basic needed methods: evaluate and show
   */
  virtual double
  unprotected_evaluate(IMP::kernel::DerivativeAccumulator *accum)
     const IMP_OVERRIDE;
  virtual IMP::kernel::ModelObjectsTemp do_get_inputs() const IMP_OVERRIDE;
  IMP_OBJECT_METHODS(FretRestraint);

private:
  double get_model_fretr_type_0() const;
  double get_model_fretr_type_1() const;
  algebra::Vector3Ds get_current_centers
                            (kernel::Particle *p, const algebra::Vector3Ds& ctrs) const;

  algebra::Vector3D get_current_center
                            (kernel::Particle *p, const algebra::Vector3D& ctr) const;
};

IMPISD_END_NAMESPACE

#endif  /* IMPISD_FRET_RESTRAINT_H */
