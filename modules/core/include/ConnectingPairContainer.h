/**
 *  \file ConnectingPairContainer.h
 *  \brief A container which has pairs which ensure a set is connected
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 */

#ifndef IMPCORE_CONNECTING_PAIR_CONTAINER_H
#define IMPCORE_CONNECTING_PAIR_CONTAINER_H

#include "config.h"
#include <IMP/core/internal/MovedSingletonContainer.h>
#include <IMP/core/internal/pair_helpers.h>
#include <IMP/PairContainer.h>
#include <IMP/SingletonContainer.h>
#include <IMP/macros.h>
#include <IMP/OptimizerState.h>
#include <IMP/ScoreState.h>

IMPCORE_BEGIN_NAMESPACE

//! A container which keeps a set of pairs that connect a set of spheres.
/** This container is for implementing connectivity when
    - there is no ambiguity about the set of particles being connected
    - the distance score used in the restraint is a monotonic function of the
    distance between the two particles.

    The contents of this container are a set of pairs defining a tree over
    the spheres in the supplied SingletonContainer. This set of edges has
    the property that
    - for particles whose balls do not intersect, if there is a path
    connecting those balls entirely contained in the union of the set of
    all balls, then there is no edge connecting those two particles.


    It is significantly more efficient than the ConnectivityRestraint
    but less flexible.
    \see {ConnectivityRestraint}
 */
class IMPCOREEXPORT ConnectingPairContainer:
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
  /** For efficiency, the set of edges is only updated occasionally. The
   error parameter determines how far particles need to move before
   the set of edges is updated.*/
  ConnectingPairContainer(SingletonContainer *sc, double error, bool mst);

#if defined(IMP_DOXYGEN) || defined(SWIG)
  IMP_PAIR_CONTAINER(ConnectingPairContainer, get_module_version_info());
#else
  IMP_LISTLIKE_PAIR_CONTAINER(ConnectingPairContainer,
                              get_module_version_info());
#endif
};


IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_CONNECTING_PAIR_CONTAINER_H */
