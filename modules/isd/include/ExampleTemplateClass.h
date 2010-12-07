/**
 *  \file isd/ExampleTemplateClass.h
 *  \brief Show how to manage a template class with python.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPISD_ISD_TEMPLATE_CLASS_H
#define IMPISD_ISD_TEMPLATE_CLASS_H

#include "isd_config.h"

#include <IMP/utility.h>
#include <IMP/algebra/VectorD.h>

IMPISD_BEGIN_NAMESPACE

//! A line segment templated on the dimension.
/** It inherits from IMP::InvalidDefault since it, like the underlying
    algebra::VectorD data, is not initialized when the default
    constructor is used. Since it is a small class, it is designed to
    be allocated on the stack and to be passed by value (or
    const &).

    The class should be named SegmentD, but it is an isd.

    The following command indicates to doxygen to mark the class
    as being one whose members are left uninitialized by the
    default constructor:
    \ingroup uninitialized_default

    The source code is as follows:
    \include ExampleTemplateClass.h
*/
template <unsigned int D>
class ExampleTemplateClass
{
  IMP::algebra::VectorD<D> eps_[2];
public:
  ExampleTemplateClass(){}
  /** Since it is a simple object, there is no reason to provide
      methods to change the data.
  */
  ExampleTemplateClass(const IMP::algebra::VectorD<D> &a,
                       const IMP::algebra::VectorD<D> &b){
    eps_[0]= a;
    eps_[1]= b;
  }
  //! Get one of the endpoints
  const IMP::algebra::VectorD<D>& get_point(unsigned int i) const {
    IMP_USAGE_CHECK(i < 2, "The endpoint index can only be 0 or 1");
    return eps_[i];
  }
};

// Make it so the C++ operator<< can be used. The _D means that it is
// is templated on the dimension. See the docs for other, related macros.
IMP_OUTPUT_OPERATOR_D(ExampleTemplateClass);

IMPISD_END_NAMESPACE

#endif  /* IMPISD_ISD_TEMPLATE_CLASS_H */
