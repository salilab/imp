/**
 * \file IntAttributeMapScoreState.h
 * \brief Maintain a map between int attribute values and particles.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPSEARCH_INT_ATTRIBUTE_MAP_SCORE_STATE_H
#define IMPSEARCH_INT_ATTRIBUTE_MAP_SCORE_STATE_H

#include "AttributeMap1ScoreState.h"

IMPSEARCH_BEGIN_NAMESPACE

//! Maintain a mapping between int attribute values and particles.
/** This class is an instantiation of AttributeMap1ScoreState so that
    the map can be used in python as swig can't deduce the constructor
    arguments by itself.
 */
class IMPSEARCHEXPORT IntAttributeMapScoreState:
  public AttributeMap1ScoreState<Int>
{
public:
    //! Create the score state searching one attribute.
    /** The first argument is the list of particles to search,
     the remainder are the attributes to use. The number of key arguments
     must match the number of provided value types in the template
     parameter list.
     */
  IntAttributeMapScoreState(IMP::SingletonContainer* pc,
                            IntKey k0);

  ~IntAttributeMapScoreState();
};
IMPSEARCH_END_NAMESPACE

#endif  /* IMPSEARCH_INT_ATTRIBUTE_MAP_SCORE_STATE_H */
