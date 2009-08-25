/**
 *  \file CoverBond.h
 *  \brief Cover a bond with a sphere
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 */

#ifndef IMPATOM_COVER_BOND_H
#define IMPATOM_COVER_BOND_H

#include "config.h"

#include <IMP/PairContainer.h>
#include <IMP/SingletonContainer.h>
#include <IMP/SingletonModifier.h>

IMPATOM_BEGIN_NAMESPACE

//! Cover a bond with a sphere
/** This is a version of core::CoverRefined optimized for bonds. It is
    about 4x faster than the general purpose one.
 */
class IMPATOMEXPORT CoverBond: public SingletonModifier
{
public:
  CoverBond();

  IMP_SINGLETON_MODIFIER(CoverBond, get_module_version_info())
};


IMPATOM_END_NAMESPACE

#endif  /* IMPATOM_COVER_BOND_H */
