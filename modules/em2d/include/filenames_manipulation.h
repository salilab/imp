/**
 *  \file filenames_manipulation.h
 *  \brief Generation of projections using the central section theorem
 * Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPEM2D_FILENAMES_MANIPULATION_H
#define IMPEM2D_FILENAMES_MANIPULATION_H

#include "IMP/exception.h"
#include "IMP/base_types.h"
#include <iostream>
#include <fstream>

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/exception.hpp>
#include <boost/version.hpp>

IMPEM2D_BEGIN_NAMESPACE

//! Reads a selection file
/*!
  \param[in] fn Name of the selection file. First column is are file names
            and second are 1 (file is selected) or 0 (file ignored)
  \return List of the selected names.
*/
inline Strings read_selection_file(String fn) {
  String name;
  Strings names;
  std::ifstream in;
  int not_ignored;
  in.open(fn.c_str(), std::ios::in);
  if (!in) {
    IMP_THROW("Unable to read file " << fn,
              IOException);
  }

  while(in >> name >> not_ignored) {
    if (not_ignored) {
      names.push_back(name);
    }
  }
  in.close();
  return names;
}

//! generates consecutive filenames: basic_name-i.extension
//! Adds zeros at the front of the number when neccessary
inline Strings create_filenames(unsigned long number,
                                  String basic_name, String extension) {
  // Number of digits to use
  int digits=0;
  unsigned long count=1;
  while(count <= number) {
    digits+=1;
    count*=10;
  }
  Strings proj_names(number);

  for (unsigned int i=0;i<number;++i) {
    std::ostringstream  strm;
    strm << basic_name << "-";
    strm.fill('0');
    strm.width(digits);
    strm << i;
    strm << "." << extension;
    proj_names[i]=strm.str();
  }
  return proj_names;
}


IMPEM2D_END_NAMESPACE

#endif /* IMPEM2D_FILENAMES_MANIPULATION_H */
