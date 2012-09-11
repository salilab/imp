/**
 * \file ColorCoder \brief
 *
 * Copyright 2007-2010 Sali Lab. All rights reserved.
 *
 */
#include "ColorCoder.h"

int ColorCoder::diff_ = 30;

IMP::algebra::Vector3D ColorCoder::color_modules_[6] = {
  IMP::algebra::Vector3D(0,0,1),
  IMP::algebra::Vector3D(0,1,0),
  IMP::algebra::Vector3D(1,0,0),
  IMP::algebra::Vector3D(0,1,1),
  IMP::algebra::Vector3D(1,0,1),
  IMP::algebra::Vector3D(1,1,0)
};
