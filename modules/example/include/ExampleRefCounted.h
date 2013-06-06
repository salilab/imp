/**
 *  \file IMP/example/ExampleRefCounted.h
 *  \brief An example showing how to make a simple ref counted object
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPEXAMPLE_EXAMPLE_REF_COUNTED_H
#define IMPEXAMPLE_EXAMPLE_REF_COUNTED_H

#include <IMP/example/example_config.h>

#include <IMP/RefCounted.h>
#include <IMP/macros.h>
#include <IMP/base/Object.h>
#include <IMP/base/Pointer.h>
#include <vector>

IMPEXAMPLE_BEGIN_NAMESPACE

//! An example simple object which is reference counted.
/** Only IMP::Pointer objects should be used to store pointers to
    instances of these objects.

    The source code is as follows:
    \include ExampleRefCounted.h
    \include ExampleRefCounted.cpp
 */
class IMPEXAMPLEEXPORT ExampleRefCounted : public base::RefCounted {
  Floats data_;

 public:
  ExampleRefCounted(const Floats &data);

  double get_data(unsigned int i) const {
    IMP_USAGE_CHECK(i < data_.size(), "Index " << i << " out of range.");
    return data_[i];
  }

  IMP_SHOWABLE_INLINE(ExampleRefCounted, out << data_.size());

  /* Make sure that it can't be allocated on the stack
     The macro defines an empty destructor. In general,
     you want destructors to be empty since they are hard
     to maintain.
  */
  IMP_REF_COUNTED_DESTRUCTOR(ExampleRefCounted);
};

typedef base::Vector<base::Pointer<ExampleRefCounted> > ExampleRefCounteds;
typedef base::Vector<base::WeakPointer<ExampleRefCounted> >
          ExampleRefCountedsTemp;

IMPEXAMPLE_END_NAMESPACE

#endif /* IMPEXAMPLE_EXAMPLE_REF_COUNTED_H */
