/**
 *  \file TableRefiner.h
 *  \brief A lookup based particle refiner
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 */

#ifndef IMPCORE_TABLE_REFINER_H
#define IMPCORE_TABLE_REFINER_H

#include "config.h"

#include "internal/version_info.h"
#include <IMP/Refiner.h>
#include <map>

IMPCORE_BEGIN_NAMESPACE

//! A lookup based particle refiner
/** Each particle is refined by returning the list
    of particles stored for it in a table.
 */
class IMPCOREEXPORT TableRefiner: public Refiner
{
  std::map<Particle*, Particles> map_;
public:
  //! Initialize it with an empty table
  TableRefiner();

  virtual ~TableRefiner();

  //! Add a mapping to the table
  void add_particle(Particle *p, const Particles &ps);

  //! Remove a mapping from the table
  void remove_particle(Particle *p);

  //! Set the mapping for a particular particle
  void set_particle(Particle *p, const Particles &ps);

  virtual const Particles get_refined(Particle *p) const {
    IMP_assert(map_.find(p) != map_.end(),
               "Particle is not found in table to refine");
    return map_.find(p)->second;
  }

  IMP_REFINER(internal::version_info)
};


IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_TABLE_REFINER_H */
