/**
 *  \file core/utility.h    \brief Various important functionality
 *                                       for implementing decorators.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPCORE_UTILITY_H
#define IMPCORE_UTILITY_H

#include "macros.h"
#include "config.h"
#include <IMP/base_types.h>
#include <IMP/algebra/Segment3D.h>
#include <IMP/algebra/Transformation3D.h>
#include <IMP/Model.h>
#include <IMP/Particle.h>

IMPCORE_BEGIN_NAMESPACE
//! Get the centroid
IMPCOREEXPORT algebra::Vector3D centroid(const Particles &ps);

#ifndef IMP_NO_DEPRECATED
//! Transform a molecule
/**
   \deprecated see SingletonFunctor for a replacement
 */
IMPCOREEXPORT
void transform(const Particles &ps,
               const algebra::Transformation3D &t);
#endif // IMP_NO_DEPRECATED
IMPCOREEXPORT
algebra::Segment3D diameter(const Particles &ps);

//! RAII class for objects with batch editing modes
/** This object sets the editing mode to true if the object is not
    being edited when it is created. If it changed the editing mode
    on creation, the mode is set to false when the object is
    destroyed.
    \code
    {
      EditGuard<ListSingletonContainer> guard(lsc);
      for (unsigned int i=0; i< ps.size(); ++i) {
        if (is_good(ps[i])) lsc->add_particle(ps[i]);
      }
      // guard is destoyed end closes the editing session
     }
     \endcode
     \see FilteredListSingletonContainer
     \see FilteredListPairContainer
     \see ListSingletonContainer
     \see ListPairContainer
*/
template <class Container>
class EditGuard {
  Pointer<Container> c_;
public:
  EditGuard(Container *c) {
    if (!c->get_is_editing()) {
      c_= Pointer<Container>(c);
      c_->set_is_editing(true);
    }
  }
  ~EditGuard() {
    if (c_) c_->set_is_editing(false);
  }
};

IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_UTILITY_H */
