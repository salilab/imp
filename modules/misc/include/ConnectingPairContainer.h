/**
 *  \file ConnectingPairContainer.h
 *  \brief XXXXXXXXXXXXXX
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 */

#ifndef IMPMISC_CONNECTING_PAIR_CONTAINER_H
#define IMPMISC_CONNECTING_PAIR_CONTAINER_H

#include "config.h"
#include <IMP/core/internal/MovedSingletonContainer.h>
#include <IMP/core/internal/pair_helpers.h>
#include <IMP/PairContainer.h>
#include <IMP/SingletonContainer.h>
#include <IMP/macros.h>
#include <IMP/OptimizerState.h>
#include <IMP/ScoreState.h>

IMPMISC_BEGIN_NAMESPACE

//! A container which keeps a set of pairs that connect a set of spheres.
/** The contents of this container are a set of pairs defining a tree over
    the spheres in the supplied SingletonContainer. This set of edges has
    the property that
    - for particles whose balls do not intersect, if there is a path
    connecting those balls entirely contained in the union of the set of
    all balls, then there is no edge connecting those two particles.

    \unstable{ConnectingPairContainer}
    \untested{ConnectingPairContainer}
 */
class IMPMISCEXPORT ConnectingPairContainer:
#if defined(IMP_DOXYGEN) || defined(SWIG)
public PairContainer
#else
public IMP::core::internal::ListLikePairContainer
#endif
{
  IMP::internal::OwnerPointer<SingletonContainer> sc_;
  IMP::internal::OwnerPointer<core::internal::MovedSingletonContainer> mv_;
  ParticlePairsTemp data_;
  double error_;
  bool mst_;
  IMP_ACTIVE_CONTAINER_DECL(ConnectingPairContainer);
  void initialize(SingletonContainer *sc);
  void fill_list(bool first);
public:
  ConnectingPairContainer(SingletonContainer *sc, double error, bool mst);

#if defined(IMP_DOXYGEN) || defined(SWIG)
  IMP_PAIR_CONTAINER(ConnectingPairContainer, get_module_version_info());
#else
  IMP_LISTLIKE_PAIR_CONTAINER(ConnectingPairContainer,
                              get_module_version_info());
#endif
};


IMPMISC_END_NAMESPACE

#endif  /* IMPMISC_CONNECTING_PAIR_CONTAINER_H */
