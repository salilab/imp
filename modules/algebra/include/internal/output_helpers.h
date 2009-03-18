/**
 *  \file output_helpers.h
 *  \brief manipulation of text, and Interconversion between text and numbers
 *  Copyright 2007-9 Sali Lab. All rights reserved.
*/

#ifndef IMPALGEBRA_INTERNAL_OUTPUT_HELPERS_H
#define IMPALGEBRA_INTERNAL_OUTPUT_HELPERS_H

#include "../config.h"
#include "IMP/base_types.h"

#include <cstdio>
#include <string>
#include <vector>
#include <string.h>

IMPALGEBRA_BEGIN_INTERNAL_NAMESPACE

//! Best precision for a Float number.
/*
 * This function returns the best precision to be used in a "printf" format if
 * this number is to fit in a given width. It returns -1 if the exponential
 * format is advised.
 */
IMPALGEBRAEXPORT int best_precision(double F, Int width);


/** Formated Float to string conversion.
 * If precision==0 the precision is automatically computed in such a way that
 * the number fits the width (exponential format might be chosen). If
 * precision==-1 then the exponential format is forced. If width==0 then the
 * minimum width is used.
 */
IMPALGEBRAEXPORT std::string float_to_string(double F, Int width = 8,
Int prec = 0);


IMPALGEBRA_END_INTERNAL_NAMESPACE

#endif  /* IMPALGEBRA_INTERNAL_OUTPUT_HELPERS_H */
