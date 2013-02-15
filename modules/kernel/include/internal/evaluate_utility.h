/**
 *  \file internal/utility.h
 *  \brief Various useful utilities
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPKERNEL_INTERNAL_EVALUATE_UTILITY_H
#define IMPKERNEL_INTERNAL_EVALUATE_UTILITY_H

#include <IMP/kernel/kernel_config.h>
#include "../Particle.h"
#include "../ModelObject.h"
#include "../input_output.h"
#include <boost/dynamic_bitset.hpp>

#if IMP_HAS_CHECKS >= IMP_INTERNAL
#define IMP_SF_SET_ONLY(mask, inputs)                           \
  {                                                             \
    ParticlesTemp cur=IMP::kernel::get_input_particles(inputs);         \
    mask.reset();                                               \
    for (unsigned int i=0; i< cur.size(); ++i) {                \
      mask.set(get_as_unsigned_int(cur[i]->get_index()));       \
    }                                                           \
  }

#else
#define IMP_SF_SET_ONLY(mask, inputs)

#endif



#endif  /* IMPKERNEL_INTERNAL_EVALUATE_UTILITY_H */
