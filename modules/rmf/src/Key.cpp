/**
 *  \file IMP/rmf/Category.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/rmf/Key.h>

namespace rmf {
namespace {;
  std::vector<std::string> category_names;
  Category get_category(std::string name) {
    IMP_RMF_USAGE_CHECK(name.find('_')==std::string::npos,
                        "Category names cannot contain '_'");
    for (unsigned int i=0; i < category_names.size(); ++i) {
      if (category_names[i]==name) {
        return Category(i);
      }
    }
    category_names.push_back(name);
    return Category(category_names.size()-1);
  }
}

Category Category::get_category(std::string name) {
  return rmf::get_category(name);
}

std::string Category::get_name() const {
  if (i_ < 0) return "invalid";
  return category_names[i_];
}

const Category Physics=get_category("physics");
const Category Sequence=get_category("sequence");
const Category Bond=get_category("bond");
const Category Shape=get_category("shape");
const Category Feature=get_category("feature");

} // namespace rmf

// there is something wrong with the standards checks (or at least
// the errors they produce
IMP_RMF_BEGIN_NAMESPACE
IMP_RMF_END_NAMESPACE
