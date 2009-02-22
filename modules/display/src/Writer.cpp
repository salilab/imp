/**
 *  \file Writer.cpp
 *  \brief XXXX.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#include "IMP/display/Writer.h"


IMPDISPLAY_BEGIN_NAMESPACE

Writer::Writer( ){
}

Writer::~Writer(){
}

void Writer::show(std::ostream &out) const {
  out << "Writer" << std::endl;
}

IMPDISPLAY_END_NAMESPACE
