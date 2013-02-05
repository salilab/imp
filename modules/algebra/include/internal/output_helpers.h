/**
 *  \file output_helpers.h
 *  \brief manipulation of text, and Interconversion between text and numbers
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
*/

#ifndef IMPALGEBRA_INTERNAL_OUTPUT_HELPERS_H
#define IMPALGEBRA_INTERNAL_OUTPUT_HELPERS_H

#include <IMP/algebra/algebra_config.h>
#include <IMP/base/Vector.h>
#include <vector>
#include <string>

IMPALGEBRA_BEGIN_INTERNAL_NAMESPACE

//! Best precision for a Float number.
/*
 * This function returns the best precision to be used in a "printf" format if
 * this number is to fit in a given width. It returns -1 if the exponential
 * format is advised.
 */
IMPALGEBRAEXPORT int best_precision(double F, int width);

//!  Formated Float to string conversion.
/**
 * If precision==0 the precision is automatically computed in such a way that
 * the number fits the width (exponential format might be chosen). If
 * precision==-1 then the exponential format is forced. If width==0 then the
 * minimum width is used.
 */
IMPALGEBRAEXPORT std::string float_to_string(double F, int width = 8,
int prec = 0);

template <class T>
struct IMPALGEBRAEXPORT stdVectorIO
{
  const base::Vector<T> &v_;
  stdVectorIO(const base::Vector<T>& v): v_(v) {}
};

//! output for a vector
template <class T>
inline std::ostream& operator<<(std::ostream& ostrm,
                         const stdVectorIO<T>& v)
{
  unsigned long s = v.v_.size();
  for(unsigned long i = 0; i<s; i++) {
    ostrm << v.v_[i] << " ";
  }
  return ostrm;
}


IMPALGEBRA_END_INTERNAL_NAMESPACE

#endif  /* IMPALGEBRA_INTERNAL_OUTPUT_HELPERS_H */
