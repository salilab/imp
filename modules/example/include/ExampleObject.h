/**
 *  \file IMP/example/ExampleObject.h
 *  \brief An example showing how to make a simple ref counted object
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 */

#ifndef IMPEXAMPLE_EXAMPLE_OBJECT_H
#define IMPEXAMPLE_EXAMPLE_OBJECT_H

#include <IMP/example/example_config.h>

#include <IMP/Object.h>
#include <IMP/object_macros.h>
#include <IMP/types.h>
#include <IMP/Pointer.h>
#include <vector>

IMPEXAMPLE_BEGIN_NAMESPACE

//! An example simple object which is reference counted.
/** Only IMP::Pointer objects should be used to store pointers to
    instances of these objects.

    The source code is as follows:
    \include ExampleObject.h
    \include ExampleObject.cpp
 */
class IMPEXAMPLEEXPORT ExampleObject : public Object {
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

typedef Vector<Pointer<ExampleObject> > ExampleObjects;
typedef Vector<WeakPointer<ExampleObject> > ExampleObjectsTemp;

IMPEXAMPLE_END_NAMESPACE

#endif /* IMPEXAMPLE_EXAMPLE_OBJECT_H */
