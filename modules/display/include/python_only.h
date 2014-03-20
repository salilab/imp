/**
 *  \file IMP/display/python_only.h   \brief Build dependency graphs on models.
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPDISPLAY_PYTHON_ONLY_H
#define IMPDISPLAY_PYTHON_ONLY_H

#include <IMP/display/display_config.h>

IMPDISPLAY_BEGIN_NAMESPACE

#ifdef IMP_DOXYGEN
/** This writer displays things to a
    [Pivy](https://pypi.python.org/pypi/Pivy) window.

    At the moment it support SphereGeometry and CylinderGeometry.

    This class is under development.
*/
class PivyWriter : public Writer {
 public:
  PivyWriter();
  /** Show the window and start the interactive event loop.
      Normal python execution will resume when the window
      is closed.
  */
  void show();
};
#endif

IMPDISPLAY_END_NAMESPACE

#endif /* IMPDISPLAY_PYTHON_ONLY_H */
