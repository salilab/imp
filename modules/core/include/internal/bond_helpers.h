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

extern IMPCOREEXPORT internal::GraphData bond_graph_data_;
extern IMPCOREEXPORT  bool bond_keys_initialized_;
extern IMPCOREEXPORT  IntKey bond_type_key_;
extern IMPCOREEXPORT  IntKey bond_order_key_;
extern IMPCOREEXPORT  FloatKey bond_length_key_;
extern IMPCOREEXPORT  FloatKey bond_stiffness_key_;


IMPCORE_END_INTERNAL_NAMESPACE

#endif  /* IMPCORE_INTERNAL_BOND_HELPERS_H */
