/**
 *  \file ChildrenRefiner.h
 *  \brief Return the hierarchy children of a particle.
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 */

#ifndef IMPCORE_CHILDREN_REFINER_H
#define IMPCORE_CHILDREN_REFINER_H

#include "config.h"
#include "Hierarchy.h"

#include <IMP/Refiner.h>

IMPCORE_BEGIN_NAMESPACE

class HierarchyTraits;

//! Return the hierarchy children of a particle.
/** \ingroup hierarchy
    A simple example using is
    \htmlinclude cover_particles.py.html
    \see Hierarchy
    \see Hierarchy
*/
class IMPCOREEXPORT ChildrenRefiner : public Refiner
{

  HierarchyTraits traits_;
public:
  //! Create a refiner for a particular type of hierarchy
  ChildrenRefiner(HierarchyTraits tr);

  IMP_REFINER(ChildrenRefiner, get_module_version_info());
};

IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_CHILDREN_REFINER_H */
