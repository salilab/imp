/**
 *  \file PDBOptimizerState.h
 *  \brief Write geometry to a file during optimization
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 */

#ifndef IMPDISPLAY_PDB_OPTIMIZER_STATE_H
#define IMPDISPLAY_PDB_OPTIMIZER_STATE_H

#include "config.h"
#include "Writer.h"
#include <IMP/OptimizerState.h>
#include <IMP/SingletonContainer.h>
#include <IMP/display/geometry.h>
#include <IMP/Pointer.h>
#include <vector>
#include <IMP/atom/Hierarchy.h>
#include <IMP/internal/utility.h>

IMPDISPLAY_BEGIN_NAMESPACE

//! Write molecules in pdb format periodically
/** The State writes a series of files generated from a
    printf-style format string.
    \verbinclude log.py
    \ingroup logging
 */
class IMPDISPLAYEXPORT PDBOptimizerState: public OptimizerState
{
  std::string name_template_;

  atom::Hierarchies mh_;
public:
  //! Write files using name_template as a template (must have a %d in it)
  PDBOptimizerState(const atom::Hierarchies &mh,
                    std::string name_template);

  IMP_PERIODIC_OPTIMIZER_STATE(PDBOptimizerState)
};


IMPDISPLAY_END_NAMESPACE

#endif  /* IMPDISPLAY_PDB_OPTIMIZER_STATE_H */
