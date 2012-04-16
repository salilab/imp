#!/bin/bash -x
# External PatchDock program can be downloaded from http://bioinfo3d.cs.tau.ac.il/PatchDock/

# This needs to be set
IMP=$IMP
PATCHDOCK_DIR=$PatchDock

$IMP/tools/imppy.sh python patchdock_multifit.py \
  -b $PATCHDOCK_DIR/patch_dock.Linux \
  -p input/1oel_subunit.pdb \
  -m input/1oel_subunit.pdb.ms \
  -d input/1oel_10.mrc \
  -l $PATCHDOCK_DIR/chem.lib \
  -o imp_patchdock.output
