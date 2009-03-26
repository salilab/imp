/**
 *  \file PDBOptimizerState.h
 *  \brief Write geometry to a file during optimization
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 */

#ifndef IMPDISPLAY_PDB_OPTIMIZER_STATE_H
#define IMPDISPLAY_PDB_OPTIMIZER_STATE_H

#include "config.h"
#include "Writer.h"
#include "internal/version_info.h"
#include <IMP/OptimizerState.h>
#include <IMP/SingletonContainer.h>
#include <IMP/display/geometry.h>
#include <IMP/Pointer.h>
#include <vector>
#include <IMP/atom/MolecularHierarchyDecorator.h>

IMPDISPLAY_BEGIN_NAMESPACE

//! Write molecules in pdb format periodically
/** The State writes a series of files generated from a
    printf-style format string.
    \verbinclude log.py
    \ingroup log
 */
class IMPDISPLAYEXPORT PDBOptimizerState: public OptimizerState
{
  unsigned int step_;
  unsigned int skip_steps_;
  std::string name_template_;

  atom::MolecularHierarchyDecorators mh_;
public:
  //! Write files using name_template as a template (must have a %d in it)
  PDBOptimizerState(const atom::MolecularHierarchyDecorators &mh,
                    std::string name_template);

  virtual ~PDBOptimizerState();

  //! Set the number of steps to skip
  void set_skip_steps(unsigned int i) {
    skip_steps_=i;
  }

  IMP_OPTIMIZER_STATE(internal::version_info)
};


IMPDISPLAY_END_NAMESPACE

#endif  /* IMPDISPLAY_PDB_OPTIMIZER_STATE_H */
