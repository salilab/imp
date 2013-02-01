/**
 *  \file IMP/rmf/RMFWriter.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPRMF_RMF_WRITER_H
#define IMPRMF_RMF_WRITER_H

#include <IMP/rmf/rmf_config.h>
#include <RMF/FileHandle.h>
#include <IMP/display/Writer.h>
#include <IMP/display/display_macros.h>
#include <IMP/base/set.h>

IMPRMF_BEGIN_NAMESPACE
#if 0
/** Only one frame is handled. Sorry. Kind of hard to fix.*/
class IMPRMFEXPORT RMFWriter: public display::Writer {
  RMF::FileHandle rh_;
  base::set<display::Geometry*> added_;
  void do_add_geometry(Geometry* g);
  void do_add_geometry(const Geometries &g);
  void on_set_frame();
 public:
  RMFWriter(RMF::FileHandle rh);
  IMP_WRITER(RMFWriter);
};
#endif
IMPRMF_END_NAMESPACE

#endif /* IMPRMF_RMF_WRITER_H */
