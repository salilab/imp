/**
 *  \file DomainDecorator.cpp
 *  \brief XXXX.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#include "IMP/atom/DomainDecorator.h"


IMPATOM_BEGIN_NAMESPACE

DomainDecorator::~DomainDecorator(){}

const DomainDecorator::Data& DomainDecorator::get_data() {
  static Data data;
  return data;
}

void DomainDecorator::show(std::ostream &out, std::string prefix) const {
  out << prefix << "DomainDecorator: " << get_begin_index()
      << " to " << get_end_index() << std::endl;
}

IMPATOM_END_NAMESPACE
