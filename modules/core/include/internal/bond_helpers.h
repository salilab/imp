/**
 *  \file bond_helpers.h
 *  \brief Helpers for the bond decorators.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPCORE_INTERNAL_BOND_HELPERS_H
#define IMPCORE_INTERNAL_BOND_HELPERS_H

#include "../macros.h"

#include <IMP/base_types.h>

IMPCORE_BEGIN_INTERNAL_NAMESPACE

struct BondData {
  IntKey type_;
  IntKey order_;
  FloatKey length_;
  FloatKey stiffness_;
  internal::GraphData graph_;
};

IMPCOREEXPORT BondData &get_bond_data();

IMPCORE_END_INTERNAL_NAMESPACE

#endif  /* IMPCORE_INTERNAL_BOND_HELPERS_H */
