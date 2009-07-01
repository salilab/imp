/**
 * \file AnchorPointSubset.h
 * \brief Holds a discrete sampling set of
         anchor points and indexes the attributes
 *
 */

#ifndef IMPMISC_ANCHOR_POINT_SUBSET_H
#define IMPMISC_ANCHOR_POINT_SUBSET_H

#include "config.h"
#include "misc_exports.h"
#include "internal/misc_version_info.h"

#include <IMP/Particle.h>
#include <map>
#include <IMP/domino/DiscreteSet.h>
#include  <sstream>
#include "IMP/base_types.h"
#include <IMP/core/XYZ.h>
IMPMISC_BEGIN_NAMESPACE
//! Holds a set of anchor points and indexes the attributes
class IMPMISCEXPORT AnchorPointSubset : public domino::DiscreteSet
{
public:
  AnchorPointSubset();
  Float get_state_val_x(unsigned int i) const;
  Float get_state_val_y(unsigned int i) const;
  Float get_state_val_z(unsigned int i) const;
};
IMPMISC_END_NAMESPACE

#endif  /* IMPMISC_ANCHOR_POINT_SUBSET_H */
