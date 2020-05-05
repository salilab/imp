/**
 *  \file IMP/em2d/internal/Image2D.cpp
 *  \brief A basic image class
 *
 *  Copyright 2007-2020 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/em2d/internal/Image2D.h>

IMPEM2D_BEGIN_INTERNAL_NAMESPACE

void read_pgm_header(std::ifstream &infile, bool &binary_fmt, int &width,
                     int &height, int &maxval) {
  char c;
  char ftype = ' ';
  const int MAX_LENGTH = 1000;
  char line[MAX_LENGTH];
  while (infile.get(c)) {
    switch (c) {
      case '#':  // if comment line, read the whole line and move on
        infile.getline(line, MAX_LENGTH);
        break;
      case 'P':  // format magick number
        infile.get(ftype);
        infile.getline(line, MAX_LENGTH);
        break;
      default:  // return the first character
        infile.putback(c);
        break;
    }
    if (ftype != ' ' && isdigit(c)) break;  // magick number read and the next line is numbers
  }

  infile >> width >> height >> maxval;
  IMP_LOG_VERBOSE("Image width = " << width << " height = " << height
                  << " max color val = " << maxval << std::endl);
  // magic number P5 = binary PGM; P2 = text PGM
  binary_fmt = (ftype == '5');
}

IMPEM2D_END_INTERNAL_NAMESPACE
