/**
 *  \file CloseBipartitePairContainer.cpp   \brief A list of
 *ParticlePairs.
 *
 *  Copyright 2007-2016 IMP Inventors. Close rights reserved.
 *
 */

#include "IMP/container/CloseBipartitePairContainer.h"
#include <IMP/PairContainer.h>
#include <IMP/PairModifier.h>

IMPCONTAINER_BEGIN_NAMESPACE

CloseBipartitePairContainer::CloseBipartitePairContainer(
    SingletonContainerAdaptor a, SingletonContainerAdaptor b, double distance,
    double slack, std::string name)
    : P(a, b, distance, slack, name) {
  a.set_name_if_default("CloseBipartitePairContainerInput0%1%");
  b.set_name_if_default("CloseBipartitePairContainerInput1%1%");
}

CloseBipartitePairContainer::CloseBipartitePairContainer(
    SingletonContainerAdaptor a, SingletonContainerAdaptor b, double distance,
    core::ClosePairsFinder *, double slack, std::string name)
    : P(a, b, distance, slack, name) {
  a.set_name_if_default("CloseBipartitePairContainerInput0%1%");
  b.set_name_if_default("CloseBipartitePairContainerInput1%1%");
}
IMPCONTAINER_END_NAMESPACE
