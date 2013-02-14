/**
 * \file ColorCoder.h \brief
 *
 * Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMP_COLOR_CODER_H
#define IMP_COLOR_CODER_H

#include <cstdio>
#include <stdio.h>

class ColorCoder {
public:
  static void get_color_for_id(int &r, int &g, int &b, int id);

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
};

#endif /* IMP_COLOR_CODER_H */
