/**
 * \file ColorCoder \brief
 *
 * Copyright 2007-2010 Sali Lab. All rights reserved.
 *
 */
#ifndef IMP_COLOR_CODER_H
#define IMP_COLOR_CODER_H

#include <IMP/algebra/Vector3D.h>

class ColorCoder {
public:
  static void get_color_for_id(int &r, int &g, int &b, int id) {
    int mod = id %6;
    int i = id / 6;
    IMP::algebra::Vector3D colors = color_modules_[mod] * (255 - i * diff_);
    r = (int)colors[0]; g = (int)colors[1]; b = (int)colors[2];
  }

  static void set_number(int number) {
    if(number < 6) { diff_ = 150; return; }
    diff_ = 150 / (number/6);
  }

  static void html_hex_color(char *out_color, int id) {
    int r,g,b;
    get_color_for_id(r, g, b, id);
    sprintf(out_color, "%02X%02X%02X", r, g, b);
  }

  static void jmol_dec_color(char *out_color, int id) {
    int r,g,b;
    get_color_for_id(r, g, b, id);
    sprintf(out_color, "[%d ,%d ,%d]", r, g, b);
  }

  static int diff_;
  static IMP::algebra::Vector3D color_modules_[6];
};

#endif /* IMP_COLOR_CODER_H */
