/**
 *  \file IMP/core/FixedRefiner.h
 *  \brief A particle refiner which returns a fixed set of particles
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 */

#ifndef IMPCORE_FIXED_REFINER_H
#define IMPCORE_FIXED_REFINER_H

#include <IMP/core/core_config.h>

#include <IMP/PairContainer.h>
#include <IMP/SingletonContainer.h>
#include <IMP/Refiner.h>

IMPCORE_BEGIN_NAMESPACE

//! The refiner can refine any particle by returning a fixed set
/**
 */
class IMPCOREEXPORT FixedRefiner : public Refiner {
  Model* m_;
  kernel::ParticleIndexes pis_;

 public:
  //! Store the set of particles
  FixedRefiner(const kernel::ParticlesTemp &ps);

  //! Store the set of particle indexes from passed model
  FixedRefiner(Model* m, const kernel::ParticleIndexes &pis);

  virtual bool get_can_refine(kernel::Particle *) const IMP_OVERRIDE
  { return true; }

  //! Returns the fixed set of particles.
  /** Returns the fixed set of particles, regardless of passed particle

      @param p coarse particle to be refined (ignored for FixedRefiner)
   */
  virtual const kernel::ParticlesTemp get_refined(kernel::Particle *p) const
    IMP_OVERRIDE;

  //! Return the indexes of the particles returned by get_refined()
  /** Return the indexes of the particles returned by get_refined()
      for particle pi in model m.

      @param pi coarse particle to be refined

      @note For FixedRefiner, this is a faster operation than
      get_refined()

      @note It is assumed that the refined particles are also in model m.

   */
  virtual kernel::ParticleIndexes get_refined_indexes
    (Model* m, ParticleIndex) const IMP_OVERRIDE
  {
    IMP_USAGE_CHECK(m == m_,
                    "mismatching models for refined and coarse particles");
    IMP_UNUSED(m);
    return pis_;
  }

  virtual ParticleIndexes const& get_refined_indexes_by_ref
    (Model *m, ParticleIndex pi) const
  {
    IMP_USAGE_CHECK(m == m_,
                    "mismatching models for refined and coarse particles");
    IMP_UNUSED(m);
    IMP_UNUSED(pi);
    return pis_;
  }


#ifndef SWIG
  using Refiner::get_refined;
#endif
  virtual kernel::ModelObjectsTemp do_get_inputs(
      kernel::Model *m, const kernel::ParticleIndexes &pis) const IMP_OVERRIDE;
  IMP_OBJECT_METHODS(FixedRefiner);
};

IMPCORE_END_NAMESPACE

#endif /* IMPCORE_FIXED_REFINER_H */
