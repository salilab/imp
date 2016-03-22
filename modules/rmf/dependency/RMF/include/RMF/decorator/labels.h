/**
 *  \file RMF/decorator/labels.h
 *  \brief Helper functions for manipulating RMF files.
 *
 *  Copyright 2007-2016 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_DECORATORS_LABELS_H
#define RMF_DECORATORS_LABELS_H

#include <RMF/config.h>
#include <RMF/Label.h>

RMF_ENABLE_WARNINGS
namespace RMF {
namespace decorator {
/** Test and set whether the node marks the root of a molecule. There
    is no associated data. */
class RMFEXPORT Molecule : public Label {
 public:
  Molecule(FileConstHandle fh) : Label(fh, "sequence", "molecule") {}
  Molecule(FileHandle fh) : Label(fh, "sequence", "molecule") {}
};

/** Test and set whether the node marks the root of a assembly. There
    is no associated data. */
class RMFEXPORT Assembly : public Label {
 public:
  Assembly(FileConstHandle fh) : Label(fh, "sequence", "molecule") {}
  Assembly(FileHandle fh) : Label(fh, "sequence", "molecule") {}
};
} /* namespace decorator */
} /* namespace RMF */
RMF_DISABLE_WARNINGS

#endif /* RMF_DECORATORS_LABELS_H */
