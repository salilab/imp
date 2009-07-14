/**
 *  \file ExampleRefCounted.cpp
 *  \brief XXXX.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#include "IMP/examples/ExampleRefCounted.h"
#include "IMP/Pointer.h"


IMPEXAMPLES_BEGIN_NAMESPACE

ExampleRefCounted::ExampleRefCounted(const std::vector<double> &data ):
  data_(data){
}

void usage_example() {
  std::vector<double> data(1000, -1);

  // this would not compile
  // ExampleRefCounted rcstack;

  // create a new object and store it in a ref counted pointer
  IMP_NEW(ExampleRefCounted, rc, (data));
  // reference count is 1

  // another object with another copy of the data
  Pointer<ExampleRefCounted> rc_other= new ExampleRefCounted(data);

  // have two pointers point to the object
  Pointer<ExampleRefCounted> rc2=rc;
  // reference count is 2

  // the object is still around since rc2 points to it
  rc=NULL;
  // reference count is 1

  std::cout << rc2->get_data(100);
  // the object will be deleted on exit
}


IMPEXAMPLES_END_NAMESPACE
