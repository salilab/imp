/**
 *  \file IntAttributeMapScoreState.cpp \brief Map on attribute values.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#include <IMP/search/IntAttributeMapScoreState.h>

IMPSEARCH_BEGIN_NAMESPACE

IntAttributeMapScoreState
::IntAttributeMapScoreState(IMP::core::SingletonContainer *pc,
                            IntKey k): AttributeMap1ScoreState<Int>(pc, k){}

IntAttributeMapScoreState::~IntAttributeMapScoreState(){}

IMPSEARCH_END_NAMESPACE
