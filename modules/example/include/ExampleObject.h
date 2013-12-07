/**
 *  \file IMP/example/ExampleObject.h
 *  \brief An example showing how to make a simple ref counted object
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPEXAMPLE_EXAMPLE_OBJECT_H
#define IMPEXAMPLE_EXAMPLE_OBJECT_H

#include <IMP/example/example_config.h>

#include <IMP/base/Object.h>
#include <IMP/base/object_macros.h>
#include <IMP/base/types.h>
#include <IMP/base/Pointer.h>
#include <vector>

IMPEXAMPLE_BEGIN_NAMESPACE

//! An example simple object which is reference counted.
/** Only IMP::Pointer objects should be used to store pointers to
    instances of these objects.

    The source code is as follows:
    \include ExampleObject.h
    \include ExampleObject.cpp
 */
class IMPEXAMPLEEXPORT ExampleObject : public base::Object {
  Floats data_;

 public:
  ExampleObject(const Floats &data);

  double get_data(unsigned int i) const {
    IMP_USAGE_CHECK(i < data_.size(), "Index " << i << " out of range.");
    return data_[i];
  }

  /* Make sure that it can't be allocated on the stack
     The macro defines an empty destructor. In general,
     you want destructors to be empty since they are hard
     to maintain.
  */
  IMP_OBJECT_METHODS(ExampleObject);
};

typedef base::Vector<base::Pointer<ExampleObject> > ExampleObjects;
typedef base::Vector<base::WeakPointer<ExampleObject> > ExampleObjectsTemp;

IMPEXAMPLE_END_NAMESPACE

#endif /* IMPEXAMPLE_EXAMPLE_OBJECT_H */
