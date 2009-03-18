/**
 *  \file Refiner.h   \brief Refine a particle into a list of particles.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 */

#ifndef IMP_REFINER_H
#define IMP_REFINER_H

#include "config.h"
#include "base_types.h"
#include "Particle.h"
#include "VersionInfo.h"
#include "RefCountedObject.h"

IMP_BEGIN_NAMESPACE

class Particle;
class DerivativeAccumulator;

//! Abstract class to implement hierarchical methods.
/** The job of this class is to take a single particle and, if
    appropriate, return a list of particles.
*/
class IMPEXPORT Refiner : public RefCountedObject
{
public:
  Refiner() {}
  //! Return true if this refiner can refine that particle
  /** This should not throw, so be careful what fields are touched.
   */
  virtual bool get_can_refine(Particle *a) const {return false;}
  //! Refine the passed particle into a set of particles.
  /** As a precondition can_refine_particle(a) should be true.
   */
  virtual Particles get_refined(Particle *a) const;

  //! Print information about the refiner
  /** It should end in a new line. */
  virtual void show(std::ostream &out=std::cout) const {
    out << "Refiner base" << std::endl;
  };

  //! Get information about the author and version
  virtual IMP::VersionInfo get_version_info() const =0;

  IMP_REF_COUNTED_DESTRUCTOR(Refiner)
};
//! a collection of Refiner objects
typedef std::vector<Refiner*> Refiners;

IMP_OUTPUT_OPERATOR(Refiner);

IMP_END_NAMESPACE

#endif  /* IMP_REFINER_H */
