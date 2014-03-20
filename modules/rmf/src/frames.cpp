/**
 *  \file IMP/rmf/Category.h
 *  \brief Handle read/write of kernel::Model data from/to files.
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/rmf/frames.h>
#include <IMP/rmf/links.h>
#include <IMP/rmf/internal/link_helpers.h>
#include <IMP/rmf/associations.h>
#include <IMP/rmf/links.h>
#include <boost/unordered_set.hpp>
#include <RMF/SetCurrentFrame.h>
#include <boost/scoped_ptr.hpp>

IMPRMF_BEGIN_NAMESPACE
void load_frame(RMF::FileConstHandle fh, RMF::FrameID frame) {
  try {
    fh.set_current_frame(frame);
    IMP_FOREACH(LoadLink * ll, internal::get_load_linkers(fh)) { ll->load(fh); }
  }
  catch (const std::exception& e) {
    IMP_THROW(e.what(), IOException);
  }
}

RMF::FrameID save_frame(RMF::FileHandle file, std::string name) {
  try {
    file.set_producer("IMP " + get_module_version());
    RMF::FrameID cur = file.add_frame(name, RMF::FRAME);
    IMP_FOREACH(SaveLink * ll, internal::get_save_linkers(file)) {
      ll->save(file);
    }
    file.flush();
    return cur;
  }
  catch (const std::exception& e) {
    IMP_THROW(e.what(), IOException);
  }
}

IMPRMF_END_NAMESPACE
