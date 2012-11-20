/**
 *  \file IMP/rmf/Category.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/rmf/frames.h>
#include <IMP/rmf/links.h>
#include <IMP/rmf/associations.h>
#include <IMP/compatibility/set.h>
#include <RMF/SetCurrentFrame.h>
#include <boost/scoped_ptr.hpp>
IMPRMF_BEGIN_NAMESPACE
namespace {
  compatibility::map<std::string, int> known_linkers;
  unsigned int get_linker_index(std::string st) {
    if (known_linkers.find(st) == known_linkers.end()) {
      int cur= known_linkers.size();
      known_linkers[st]=cur;
      return cur;
    } else {
      return known_linkers.find(st)->second;
    }
  }
}

unsigned int get_load_linker_index(std::string st) {
  return get_linker_index(st)*2;
}
unsigned int get_save_linker_index(std::string st) {
  return get_linker_index(st)*2+1;
}

void load_frame(RMF::FileConstHandle file, unsigned int frame) {
  try {
  file.set_current_frame(frame);
  for (unsigned int i=0; i< known_linkers.size(); ++i) {
    if (file.get_has_associated_data(2*i)) {
      base::Pointer<LoadLink> ll
        = get_load_linker(file, 2*i);
      ll->load(file);
    }
  }
  } catch (const std::exception &e) {
    IMP_THROW(e.what(), IOException);
  }
}

void save_frame(RMF::FileHandle file, unsigned int frame,
                std::string name) {
  try {
  IMP_USAGE_CHECK(frame==file.get_number_of_frames()
                  || frame == file.get_number_of_frames()-1,
                  "Can only write last frame");
  if (frame==file.get_number_of_frames()) {
    file.set_current_frame(file.get_number_of_frames()-1);
    file.get_current_frame().add_child(name,
                                       RMF::FRAME);
  } else {
    file.set_current_frame(frame);
  }
  IMP_INTERNAL_CHECK(file.get_current_frame().get_id().get_index()
                     == frame, "Wrong current frame");
  for (unsigned int i=0; i< known_linkers.size(); ++i) {
    if (file.get_has_associated_data(2*i+1)) {
      base::Pointer<SaveLink> ll
        = get_save_linker(file, 2*i+1);
      ll->save(file);
    }
  }
  IMP_INTERNAL_CHECK(file.get_number_of_frames()>=frame+1,
                     "Found " << file.get_number_of_frames()
                     << " frames after writing frame "
                     << frame);
  file.flush();
  } catch (const std::exception &e) {
    IMP_THROW(e.what(), IOException);
  }
}


IMPRMF_END_NAMESPACE
