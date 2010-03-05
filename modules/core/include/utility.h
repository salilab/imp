/**
 *  \file core/utility.h    \brief Various important functionality
 *                                       for implementing decorators.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPCORE_UTILITY_H
#define IMPCORE_UTILITY_H

#include "core_macros.h"
#include "core_config.h"
#include "XYZR.h"
#include <IMP/base_types.h>
#include <IMP/algebra/Segment3D.h>
#include <IMP/algebra/Transformation3D.h>
#include <IMP/Model.h>
#include <IMP/Particle.h>

IMPCORE_BEGIN_NAMESPACE
//! Get the centroid
/** Compute the centroid (mean) of a set of particles.
 */
IMPCOREEXPORT algebra::VectorD<3> get_centroid(const XYZsTemp &ps);

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
class EditGuard: public RAII {
  IMP::internal::OwnerPointer<Container> c_;
public:
  IMP_RAII(EditGuard, (Container *c),,
           {
             if (!c->get_is_editing()) {
               c_= IMP::internal::OwnerPointer<Container>(c);
               c_->set_is_editing(true);
             }
           },
           if (c_) c_->set_is_editing(false);
           )
};

IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_UTILITY_H */
