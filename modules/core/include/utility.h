/**
 *  \file core/utility.h    \brief Various important functionality
 *                                       for implementing decorators.
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPCORE_UTILITY_H
#define IMPCORE_UTILITY_H

#include "macros.h"
#include "config.h"
#include "XYZR.h"
#include <IMP/base_types.h>
#include <IMP/algebra/Segment3D.h>
#include <IMP/algebra/Transformation3D.h>
#include <IMP/Model.h>
#include <IMP/Particle.h>

IMPCORE_BEGIN_NAMESPACE
//! Get the centroid
IMPCOREEXPORT algebra::VectorD<3> centroid(const XYZs &ps);

IMPCOREEXPORT
algebra::Segment3D diameter(const XYZs &ps);

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
