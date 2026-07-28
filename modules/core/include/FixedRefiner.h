/**
 *  \file IMP/core/FixedRefiner.h
 *  \brief A particle refiner which returns a fixed set of particles
 *
 *  Copyright 2007-2026 IMP Inventors. All rights reserved.
 */

#ifndef IMPCORE_FIXED_REFINER_H
#define IMPCORE_FIXED_REFINER_H

#include <IMP/core/core_config.h>

#include <IMP/PairContainer.h>
#include <IMP/SingletonContainer.h>
#include <IMP/Refiner.h>
#include <cereal/access.hpp>
#include <cereal/types/base_class.hpp>

IMPCORE_BEGIN_NAMESPACE

//! The refiner can refine any particle by returning a fixed set
/**
 */
class IMPCOREEXPORT FixedRefiner : public Refiner {
  Model* m_;
  ParticleIndexes pis_;

  friend class cereal::access;
  template<class Archive> void serialize(Archive &ar) {
    ar(cereal::base_class<Refiner>(this));
    if (std::is_base_of<cereal::detail::OutputArchiveBase, Archive>::value) {
      uint32_t model_id = get_model_id();
      ar(model_id);
    } else {
      uint32_t model_id;
      ar(model_id);
      set_model_from_id(model_id);
    }
    ar(pis_);
  }
  void set_model_from_id(uint32_t model_id);
  uint32_t get_model_id() const;
  IMP_OBJECT_SERIALIZE_DECL(FixedRefiner);

 public:
  //! Store the set of particles
  FixedRefiner(const ParticlesTemp &ps);

  //! Store the set of particle indexes from passed model
  FixedRefiner(Model* m, const ParticleIndexes &pis);

  FixedRefiner() {}

  virtual bool get_can_refine(Particle *) const override
  { return true; }

  //! Returns the fixed set of particles.
  /** \note the passed Particle is ignored.
   */
  virtual const ParticlesTemp get_refined(Particle *p) const
    override;

  //! Returns the fixed set of particles, as indexes.
  /** @note the passed ParticleIndex is ignored.

      @note For FixedRefiner, this is a faster operation than
      get_refined()

      @note It is assumed that the refined particles are also in model m.

   */
  virtual ParticleIndexes get_refined_indexes
    (Model* m, ParticleIndex) const override
  {
    IMP_USAGE_CHECK(m == m_,
                    "mismatching models for refined and coarse particles");
    IMP_UNUSED(m);
    return pis_;
  }

  virtual ParticleIndexes const& get_refined_indexes_by_ref
    (Model *m, ParticleIndex pi) const override
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
  virtual ModelObjectsTemp do_get_inputs(
      Model *m, const ParticleIndexes &pis) const override;
  IMP_OBJECT_METHODS(FixedRefiner);
};

IMPCORE_END_NAMESPACE

#endif /* IMPCORE_FIXED_REFINER_H */
