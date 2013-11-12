/**
 *  \file IMP/rmf/Category.h
 *  \brief Handle read/write of kernel::Model data from/to files.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/rmf/frames.h>
#include <IMP/rmf/links.h>
#include <IMP/rmf/internal/link_helpers.h>
#include <IMP/rmf/associations.h>
#include <IMP/rmf/links.h>
#include <IMP/base/set.h>
#include <RMF/SetCurrentFrame.h>
#include <boost/scoped_ptr.hpp>

IMPRMF_BEGIN_NAMESPACE
void load_frame(RMF::FileConstHandle fh, int frame) {
  try {
    RMF::FrameConstHandle fr = fh.get_frame(RMF::FrameID(frame));
    fr.set_as_current_frame();
    IMP_FOREACH(LoadLink * ll, internal::get_load_linkers(fh)) { ll->load(fh); }
  }
  catch (const std::exception& e) {
    IMP_THROW(e.what(), IOException);
  }
}

void save_frame(RMF::FileHandle file, int frame, std::string name) {
  try {
    IMP_USAGE_CHECK(
        frame == static_cast<int>(file.get_number_of_frames()) ||
            frame == static_cast<int>(file.get_number_of_frames()) - 1,
        "Can only write last frame");
    if (frame == static_cast<int>(file.get_number_of_frames())) {
      RMF::FrameHandle fr;
      if (frame != 0) {
        fr = file.get_frame(file.get_number_of_frames() - 1);
      } else {
        fr = file.get_root_frame();
      }
      fr.add_child(name, RMF::FRAME).set_as_current_frame();
    } else if (RMF::FrameID(frame) == RMF::ALL_FRAMES) {
      file.get_root_frame().set_as_current_frame();
    } else {
      RMF::FrameHandle fr = file.get_frame(frame);
      fr.set_as_current_frame();
    }
    IMP_INTERNAL_CHECK(static_cast<int>(file.get_number_of_frames()) >= frame,
                       "Not enough frames");
    IMP_INTERNAL_CHECK(file.get_current_frame().get_id().get_index() ==
                           static_cast<int>(frame),
                       "Wrong current frame");
    IMP_FOREACH(SaveLink * ll, internal::get_save_linkers(file)) {
      ll->save(file);
    }
    IMP_INTERNAL_CHECK(
        static_cast<int>(file.get_number_of_frames()) >= frame + 1,
        "Found " << file.get_number_of_frames()
                 << " frames after writing frame " << frame);
    file.flush();
  }
  catch (const std::exception& e) {
    IMP_THROW(e.what(), IOException);
  }
}

IMPRMF_END_NAMESPACE
