/**
 *  \file IMP/isd/TALOSRestraint.h
 *  \brief TALOS dihedral restraint between four particles.
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPISD_TALOS_RESTRAINT_H
#define IMPISD_TALOS_RESTRAINT_H

#include <IMP/isd/isd_config.h>
#include "Scale.h"
#include "vonMisesSufficient.h"

#include <IMP/kernel/Restraint.h>
#include <IMP/kernel/Particle.h>

IMPISD_BEGIN_NAMESPACE

//! phi/psi dihedral restraint between four particles, using data from TALOS.
/** The likelihood is
       \f[
       f(\chi_{exp}^1,\cdots,\chi_{exp}^N|\chi(X),\kappa,I)
        = \frac{1}{2\pi I_0(\kappa)^N}
        \exp \left(R_0 \kappa \cos (\chi_{exp}-\chi(X)) \right)
      \f]
    where the sufficient statistics are \f$N\f$ the number of observations,
    \f$R\f$ and \f$\chi_{exp}\f$.
    \see vonMisesSufficient.h for further detail.
*/
class IMPISDEXPORT TALOSRestraint : public kernel::Restraint {
 public:
  //! Create restraint from a list of particles and the data.
  /** \param[in] m the Model
      \param[in] p list of 4 particles that make the dihedral angle.
      \param[in] data list of observations for that angle.
      \param[in] kappa Pointer to the \f$\kappa\f$ concentration particle.
   */
  TALOSRestraint(kernel::Model *m, kernel::Particles p, Floats data,
                 kernel::Particle *kappa);

  //! Create restraint from 4 particles and the data.
  /** \param[in] m the Model
      \param[in] p1 Pointer to first particle in dihedral restraint.
      \param[in] p2 Pointer to second particle in dihedral restraint.
      \param[in] p3 Pointer to third particle in dihedral restraint.
      \param[in] p4 Pointer to fourth particle in dihedral restraint.
      \param[in] data list of observations for that angle.
      \param[in] kappa Pointer to the \f$\kappa\f$ concentration particle.
   */
  TALOSRestraint(kernel::Model *m, kernel::Particle *p1, kernel::Particle *p2,
                 kernel::Particle *p3, kernel::Particle *p4, Floats data,
                 kernel::Particle *kappa);

  //! Create restraint from a list of particles and the sufficient statistics.
  /** \param[in] m the Model
      \param[in] p list of 4 particles that make the dihedral angle.
      \param[in] N Number of observations
      \param[in] R0 component on the x axis
      \param[in] chiexp average observed angle.
      \param[in] kappa Pointer to the \f$\kappa\f$ concentration particle.
   */
  TALOSRestraint(kernel::Model *m, kernel::Particles p, unsigned N, double R0,
                 double chiexp, kernel::Particle *kappa);

  //! Create restraint from 4 particles and the sufficient statistics.
  /** \param[in] m the Model
      \param[in] p1 Pointer to first particle in dihedral restraint.
      \param[in] p2 Pointer to second particle in dihedral restraint.
      \param[in] p3 Pointer to third particle in dihedral restraint.
      \param[in] p4 Pointer to fourth particle in dihedral restraint.
      \param[in] N Number of observations
      \param[in] R0 component on the x axis
      \param[in] chiexp average observed angle.
      \param[in] kappa Pointer to the \f$\kappa\f$ concentration particle.
   */
  TALOSRestraint(kernel::Model *m, kernel::Particle *p1, kernel::Particle *p2,
                 kernel::Particle *p3, kernel::Particle *p4, unsigned N,
                 double R0, double chiexp, kernel::Particle *kappa);

  //! Call for probability
  double get_probability() const { return exp(-unprotected_evaluate(nullptr)); }

  double get_R0() const { return mises_->get_R0(); }

  double get_chiexp() const { return mises_->get_chiexp(); }

  virtual double unprotected_evaluate(IMP::kernel::DerivativeAccumulator *accum)
      const IMP_OVERRIDE;
  virtual IMP::kernel::ModelObjectsTemp do_get_inputs() const IMP_OVERRIDE;
  IMP_OBJECT_METHODS(TALOSRestraint);

 private:
  base::Pointer<kernel::Particle> p_[4];
  base::Pointer<kernel::Particle> kappa_;
  base::Pointer<vonMisesSufficient> mises_;
};

IMPISD_END_NAMESPACE

#endif /* IMPISD_TALOS_RESTRAINT_H */
