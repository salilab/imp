/**
 *  \file PDBOptimizerState.h
 *  \brief Write geometry to a file during optimization
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 */

#ifndef IMPDISPLAY_PDB_OPTIMIZER_STATE_H
#define IMPDISPLAY_PDB_OPTIMIZER_STATE_H

#include "display_config.h"
#include "Writer.h"
#include <IMP/OptimizerState.h>
#include <IMP/SingletonContainer.h>
#include <IMP/display/geometry.h>
#include <IMP/Pointer.h>
#include <vector>
#include <IMP/atom/pdb.h>
#include <IMP/internal/utility.h>

IMPDISPLAY_BEGIN_NAMESPACE
#ifndef IMP_DOXYGEN
typedef atom::WritePDBOptimizerState PDBOptimizerState;
#endif

IMPDISPLAY_END_NAMESPACE

#endif  /* IMPDISPLAY_PDB_OPTIMIZER_STATE_H */
