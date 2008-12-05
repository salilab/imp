/**
 *  \file ParticleRefiner.h   \brief Refine a particle into a list of particles.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#ifndef IMP_PARTICLE_REFINER_H
#define IMP_PARTICLE_REFINER_H

#include "IMP_config.h"
#include "base_types.h"
#include "VersionInfo.h"
#include "RefCountedObject.h"

IMP_BEGIN_NAMESPACE

class Particle;
class DerivativeAccumulator;

//! Abstract class to implement hierarchical methods.
/** The job of this class is to take a single particle and, if
    appropriate, return a list of particles.
*/
class IMPDLLEXPORT ParticleRefiner : public RefCountedObject
{
public:
  ParticleRefiner() {}
  //! Return true if this refiner can refine that particle
  /** This should not throw, so be careful what fields are touched.
   */
  virtual bool get_can_refine(Particle *a) const {return false;}
  //! Refine the passed particle into a set of particles.
  /** As a precondition can_refine_particle(a) should be true.
   */
  virtual Particles get_refined(Particle *a) const;

  //! Cleanup after refining
  /** If da is non-NULL then the derivatives should be propagated
      to the appropriate particles. This method is only called when
      the caller is done with the particles, so the particles
      can be destroyed if they are temporary.
   */
  virtual void cleanup_refined(Particle *a, Particles &b,
                               DerivativeAccumulator *da=0) const {}
  //! Print information about the refiner
  /** It should end in a new line. */
  virtual void show(std::ostream &out=std::cout) const {
    out << "ParticleRefiner base" << std::endl;
  };

  virtual IMP::VersionInfo get_version_info() const =0;

  IMP_REF_COUNTED_DESTRUCTOR(ParticleRefiner)
};

typedef std::vector<ParticleRefiner*> ParticleRefiners;
typedef Index<ParticleRefiner> ParticleRefinerIndex;

IMP_OUTPUT_OPERATOR(ParticleRefiner);

IMP_END_NAMESPACE

#endif  /* IMP_PARTICLE_REFINER_H */
