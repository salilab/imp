/**
 *  \file IMP/rmf/Category.h
 *  \brief Handle read/write of kernel::Model data from/to files.
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/rmf/links.h>
IMPRMF_BEGIN_NAMESPACE

LoadLink::LoadLink(std::string name) : base::Object(name) {
  frame_loaded_ = false;
}
LoadLink::~LoadLink() {
  if (!frame_loaded_) {
    IMP_WARN("No frames were loaded from file \""
             << get_name() << "\" even though objects were linked or created."
             << std::endl);
  }
}
SaveLink::SaveLink(std::string name) : base::Object(name) {
  frame_saved_ = false;
}
SaveLink::~SaveLink() {
  if (!frame_saved_) {
    IMP_WARN("No frames were saved to file \""
             << get_name() << "\" even though objects were added."
             << std::endl);
  }
}

IMPRMF_END_NAMESPACE
