/**
 *  \file IMP/hdf5/KeyCategory.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/hdf5/Key.h>

IMPHDF5_BEGIN_NAMESPACE
namespace {;
  std::vector<std::string> category_names;
}

KeyCategory KeyCategory::add_key_category(std::string name) {
  category_names.push_back(name);
  return KeyCategory(category_names.size()-1);
}

std::string KeyCategory::get_name() const {
  if (i_ < 0) return "invalid";
  return category_names[i_];
}

const KeyCategory Physics=KeyCategory::add_key_category("physics");
const KeyCategory Sequence=KeyCategory::add_key_category("sequence");
const KeyCategory Bond=KeyCategory::add_key_category("bond");
const KeyCategory Shape=KeyCategory::add_key_category("shape");
const KeyCategory Feature=KeyCategory::add_key_category("feature");

IMPHDF5_END_NAMESPACE
