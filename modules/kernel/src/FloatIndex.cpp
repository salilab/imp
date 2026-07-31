/**
 *  \file FloatIndex.cpp
 *  \brief Identifies an optimized attribute in a model.
 *
 *  Copyright 2007-2026 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/FloatIndex.h>
#include <IMP/Model.h>

IMPKERNEL_BEGIN_NAMESPACE

void FloatIndex::set_value(Model *m, Float v) const
{
  switch (dim_) {
  case 3:
    m->access_attribute(v3k_, pi_)[index_] = v;
    break;
  case 4:
    m->access_attribute(v4k_, pi_)[index_] = v;
    break;
  default:
    m->set_attribute(fk_, pi_, v);
    break;
  }
}

Float FloatIndex::get_value(Model *m) const
{
  switch (dim_) {
  case 3:
    return m->get_attribute(v3k_, pi_)[index_];
  case 4:
    return m->get_attribute(v4k_, pi_)[index_];
  default:
    return m->get_attribute(fk_, pi_);
  }
}

Float FloatIndex::get_derivative(Model *m) const
{
  switch (dim_) {
  case 3:
    return m->get_derivative(v3k_, pi_)[index_];
  case 4:
    return m->get_derivative(v4k_, pi_)[index_];
  default:
    return m->get_derivative(fk_, pi_);
  }
}

IMPKERNEL_END_NAMESPACE
