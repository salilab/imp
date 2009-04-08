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
#include "RefCounted.h"
#include "internal/IndexingIterator.h"

IMP_BEGIN_NAMESPACE

class Particle;
class DerivativeAccumulator;

//! Abstract class to implement hierarchical methods.
/** The job of this class is to take a single particle and, if
    appropriate, return a list of particles.
*/
class IMPEXPORT Refiner : public Object
{
  struct Accessor;
public:
  Refiner() {}
  //! Return true if this refiner can refine that particle
  /** This should not throw, so be careful what fields are touched.
   */
  virtual bool get_can_refine(Particle *a) const {return false;}

  //! Refine the passed particle into a set of particles.
  /** As a precondition can_refine_particle(a) should be true.
   */
  virtual const Particles get_refined(Particle *a) const=0;

  //! Get the ith refined particle.
  /** As a precondition can_refine_particle(a) should be true.
   */
  virtual Particle* get_refined(Particle *a, unsigned int i) const =0;

  /** As a precondition can_refine_particle(a) should be true.
   */
  virtual unsigned int get_number_of_refined(Particle *a) const =0;

  /** @name Iterating through the set of refined particles

      Using iterators can be more efficient, however it is not
      necessarily so.
      @{
  */
  typedef internal::IndexingIterator<Accessor> RefinedIterator;
  RefinedIterator refined_begin(Particle *a) const;
  RefinedIterator refined_end(Particle *a) const;
  /** @} */

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

#ifndef SWIG
struct Refiner::Accessor {
  // can't reference count since swig memory management is broken
  Particle* p_;
  const Refiner* r_;
  Accessor(Particle *p, const Refiner *r): p_(p), r_(r) {}
  Accessor(){}
  typedef Particle *result_type;
  Particle *operator()(unsigned int i) const {
    return r_->get_refined(p_, i);
  }
  bool operator==(const Accessor &o) const {
    return p_==o.p_ && r_==o.r_;
  }
};
#endif

inline Refiner::RefinedIterator Refiner::refined_begin(Particle *a) const {
  return RefinedIterator(Accessor(a, this), 0);
}
inline Refiner::RefinedIterator Refiner::refined_end(Particle *a) const {
  return RefinedIterator(Accessor(a, this), get_number_of_refined(a));
}

IMP_END_NAMESPACE

#endif  /* IMP_REFINER_H */
