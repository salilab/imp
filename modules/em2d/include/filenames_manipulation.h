/**
 *  \file filenames_manipulation.h
 *  \brief Generation of projections using the central section theorem
 * Copyright 2007-2010 IMP Inventors. All rights reserved.
 */

#ifndef IMPEM2D_FILENAMES_MANIPULATION_H
#define IMPEM2D_FILENAMES_MANIPULATION_H

#include "IMP/exception.h"
#include <boost/filesystem/path.hpp>
#include <boost/version.hpp>

IMPEM2D_BEGIN_NAMESPACE

//! Reads a selection file, first column is a file name second is 1 if the
//! file is selected, 0 if ignored
IMPEM2DEXPORT inline Strings read_selection_file(String fn) {
  String name;
  Strings names;
  boost::filesystem::path fnp(fn);
#if BOOST_VERSION >= 103400
  boost::filesystem::path dir= fnp.remove_filename();
#else
  boost::filesystem::path dir= fnp.branch_path();
#endif
  std::ifstream in;
  int not_ignored;
  in.open(fn.c_str(), std::ios::in);
  if (!in) {
    IMP_THROW("Unable to read file " << fn,
              IOException);
  }

  while(in >> name >> not_ignored) {
    if (not_ignored) {
      boost::filesystem::path path=dir/boost::filesystem::path(name);
#if BOOST_VERSION >= 103400
      std::string str= path.file_string();
#else
      std::string str= path.native_file_string();
#endif
      names.push_back(str);
    }
  }
  in.close();
  return names;
}

//! generates consecutive filenames: basic_name-i.extension
IMPEM2DEXPORT inline Strings generate_filenames(unsigned long number,
                                            String basic_name,
                                            String extension) {
  Strings proj_names(number);
  for (unsigned int i=0;i<number;++i) {
    std::ostringstream  strm;
    strm << basic_name << "-" << i << "." << extension;
    proj_names[i]=strm.str();
  }
  return proj_names;
}

IMPEM2D_END_NAMESPACE

#endif /* IMPEM2D_FILENAMES_MANIPULATION_H */
