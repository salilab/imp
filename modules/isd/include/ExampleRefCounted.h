/**
 *  \file isd/ExampleRefCounted.h
 *  \brief An isd showing how to make a simple ref counted object
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 */

#ifndef IMPISD_ISD_REF_COUNTED_H
#define IMPISD_ISD_REF_COUNTED_H

#include "isd_config.h"

#include <IMP/RefCounted.h>
#include <IMP/macros.h>

IMPISD_BEGIN_NAMESPACE

//! An isd simple object which is reference counted.
/** Only IMP::Pointer objects should be used to store pointers to
    instances of these objects.

    The source code is as follows:
    \include ExampleRefCounted.h
    \include ExampleRefCounted.cpp
 */
class IMPISDEXPORT ExampleRefCounted: public RefCounted
{
  std::vector<double> data_;
public:
  ExampleRefCounted(const std::vector<double> &data);

  double get_data(unsigned int i) const {
    IMP_USAGE_CHECK(i < data_.size(), "Index " << i
              << " out of range.");
    return data_[i];
  }

  /* Make sure that it can't be allocated on the stack
     The macro defines an empty destructor. In general,
     you want destructors to be empty since they are hard
     to maintain.
  */
  IMP_REF_COUNTED_DESTRUCTOR(ExampleRefCounted);
};


IMPISD_END_NAMESPACE

#endif  /* IMPISD_ISD_REF_COUNTED_H */
