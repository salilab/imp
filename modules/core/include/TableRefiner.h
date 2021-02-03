/**
 *  \file IMP/core/TableRefiner.h
 *  \brief A lookup based particle refiner
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 */

#ifndef IMPCORE_TABLE_REFINER_H
#define IMPCORE_TABLE_REFINER_H

#include <IMP/core/core_config.h>

#include <IMP/Refiner.h>
#include <boost/unordered_map.hpp>
IMPCORE_BEGIN_NAMESPACE

//! A lookup based particle refiner
/** Each particle is refined by returning the list
    of particles stored for it in a table.
 */
class IMPCOREEXPORT TableRefiner : public Refiner {
  boost::unordered_map<Particle *, Particles> map_;

 public:
  //! Initialize it with an empty table
  TableRefiner();

  //! Add a mapping to the table
  void add_particle(Particle *p, const ParticlesTemp &ps);

  //! Remove a mapping from the table
  void remove_particle(Particle *p);

  //! Set the mapping for a particular particle
  void set_particle(Particle *p, const ParticlesTemp &ps);

  virtual bool get_can_refine(Particle *) const IMP_OVERRIDE;
  virtual const ParticlesTemp get_refined(Particle *) const
      IMP_OVERRIDE;
#ifndef SWIG
  using Refiner::get_refined;
#endif
  virtual ModelObjectsTemp do_get_inputs(
      Model *m, const ParticleIndexes &pis) const IMP_OVERRIDE;
  IMP_OBJECT_METHODS(TableRefiner);
};

IMPCORE_END_NAMESPACE

#endif /* IMPCORE_TABLE_REFINER_H */
