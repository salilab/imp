/**
 *  \file RMF/Category.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#include <RMF/Key.h>

namespace RMF {
namespace {
  vector<std::string> category_names;
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
  return RMF::get_category(name);
}

std::string Category::get_name() const {
  if (i_ < 0) return "invalid";
  return category_names[i_];
}

const Category physics=get_category("physics");
const Category sequence=get_category("sequence");
const Category shape=get_category("shape");
const Category feature=get_category("feature");
const Category publication=get_category("publication");
const Category bond=get_category("bond");

  const Category Physics=physics;
  const Category Sequence=sequence;
  const Category Shape=shape;
  const Category Feature=feature;
  const Category Publication=publication;

} /* namespace RMF */
