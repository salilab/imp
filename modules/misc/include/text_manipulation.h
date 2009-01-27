/**
 *  \file text_manipulation.h
 *  \brief manipulation of text, and Interconversion between text and numbers
 *  Copyright 2007-8 Sali Lab. All rights reserved.
*/

#ifndef IMPMISC_TEXT_MANIPULATION_H
#define IMPMISC_TEXT_MANIPULATION_H

#include "config.h"
#include "IMP/base_types.h"

#include <cstdio>
#include <string>
#include <vector>
#include <string.h>

IMPMISC_BEGIN_NAMESPACE

//! Best precision for a Float number.
/*
 * This function returns the best precision to be used in a "printf" format if
 * this number is to fit in a given width. It returns -1 if the exponential
 * format is advised.
 */
IMPMISCEXPORT Int best_precision(Float F, Int width);


/** Formated Float to string conversion.
 * If precision==0 the precision is automatically computed in such a way that
 * the number fits the width (exponential format might be chosen). If
 * precision==-1 then the exponential format is forced. If width==0 then the
 * minimum width is used.
 */
IMPMISCEXPORT String float_to_string(Float F, Int width = 8, Int _prec = 0);


IMPMISC_END_NAMESPACE

#endif  /* IMPMISC_TEXT_MANIPULATION_H */
