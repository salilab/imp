/**
 * \file IMP/foxs/ColorCoder.cpp \brief
 *
 * Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/foxs/ColorCoder.h>

IMPFOXS_BEGIN_NAMESPACE

int ColorCoder::diff_ = 30;

void ColorCoder::get_color_for_id(int &r, int &g, int &b, int id) {
  const float color_modules[6][3] = {
      {1, 0, 0}, {0, 0, 1}, {0, 1, 0}, {0, 1, 1}, {1, 0, 1}, {1, 1, 0}};
  int mod = id % 6;
  int i = id / 6;
  int mult = 255 - i * diff_;
  r = (int)(color_modules[mod][0] * mult);
  g = (int)(color_modules[mod][1] * mult);
  b = (int)(color_modules[mod][2] * mult);
}

IMPFOXS_END_NAMESPACE
