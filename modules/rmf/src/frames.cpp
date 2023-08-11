/**
 *  \file IMP/rmf/frames.cpp
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2023 IMP Inventors. All rights reserved.
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
    LoadLinks links = internal::get_load_linkers(fh);
    for(LoadLink * ll : links) { ll->load(fh); }
    if (links.empty()) {
      IMP_WARN("Frame " << frame << " loaded from RMF file \"" << fh.get_name()
               << "\" but no IMP objects were updated - perhaps you forgot to "
               << "call, for example, IMP.rmf.link_hierarchies() or "
               << "IMP.rmf.create_hierarchies() first?" << std::endl);
    }
  }
  catch (const std::exception& e) {
    IMP_THROW(e.what(), IOException);
  }
}

RMF::FrameID save_frame(RMF::FileHandle file, std::string name) {
  try {
    file.set_producer("IMP " + get_module_version());
    RMF::FrameID cur = file.add_frame(name, RMF::FRAME);
    SaveLinks links = internal::get_save_linkers(file);
    for(SaveLink * ll : links) {
      ll->save(file);
    }
    if (links.empty()) {
      IMP_WARN("Frame " << cur << " saved to RMF file \"" << file.get_name()
               << "\" but no IMP objects were included - perhaps you forgot to "
               << "call, for example, IMP.rmf.link_hierarchies() or "
               << "IMP.rmf.add_hierarchies() first?" << std::endl);
    }
    file.flush();
    return cur;
  }
  catch (const std::exception& e) {
    IMP_THROW(e.what(), IOException);
  }
}

IMPRMF_END_NAMESPACE
