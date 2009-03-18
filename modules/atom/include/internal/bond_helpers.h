/**
 *  \file bond_helpers.h
 *  \brief Helpers for the bond decorators.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPATOM_INTERNAL_BOND_HELPERS_H
#define IMPATOM_INTERNAL_BOND_HELPERS_H

#include "../config.h"

#include <IMP/base_types.h>

IMPATOM_BEGIN_INTERNAL_NAMESPACE

struct BondData {
  IntKey type_;
  IntKey order_;
  FloatKey length_;
  FloatKey stiffness_;
  IMP::core::internal::GraphData graph_;
};

IMPATOMEXPORT BondData &get_bond_data();

IMPATOM_END_INTERNAL_NAMESPACE

#endif  /* IMPATOM_INTERNAL_BOND_HELPERS_H */
