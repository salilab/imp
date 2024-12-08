/**
 *  \file internal/mmcif.h
 *  \brief Utility functions for handling mmCIF files.
 *
 *  Copyright 2007-2024 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPATOM_INTERNAL_MMCIF_H
#define IMPATOM_INTERNAL_MMCIF_H

#include <IMP/atom/atom_config.h>
#include <IMP/algebra/Vector3D.h>
#include <IMP/atom/Atom.h>

IMPATOM_BEGIN_INTERNAL_NAMESPACE

//! Write the atom_site table to an mmCIF file
class IMPATOMEXPORT AtomSiteDumper {
  std::ofstream &out_file_;
  bool header_written_;

  void write_header();

public:
  AtomSiteDumper(std::ofstream &out_file);

  //! Write a single row in the atom_site loop
  void write(const algebra::Vector3D &v, const Atom &a, int model_num);
};

IMPATOM_END_INTERNAL_NAMESPACE

#endif /* IMPATOM_INTERNAL_MMCIF_H */
