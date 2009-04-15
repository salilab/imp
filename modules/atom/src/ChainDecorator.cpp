/**
 *  \file ChainDecorator.cpp
 *  \brief XXXX.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#include "IMP/atom/ChainDecorator.h"


IMPATOM_BEGIN_NAMESPACE


IntKey ChainDecorator::get_id_key() {
  static IntKey k("chain");
  return k;
}

void ChainDecorator::show(std::ostream &out, std::string prefix) const {
  out << prefix << "ChainDecorator: " << get_id() << std::endl;
}

IMPATOM_END_NAMESPACE
