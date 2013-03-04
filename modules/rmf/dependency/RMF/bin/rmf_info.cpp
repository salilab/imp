/**
 * Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#include <RMF/FileHandle.h>
#include <sstream>
#include "common.h"

namespace {
std::string description("Print out information about categories and keys.");


template <class Traits>
struct GetCount {
  static int get_count(RMF::FileConstHandle rh,
                       RMF::Key<Traits>     k) {
    return get_count(rh.get_root_node(), k);
  }
  static int get_count(RMF::NodeConstHandle rh,
                       RMF::Key<Traits>     k) {
    int cur = 0;
    if (rh.get_has_value(k)) ++cur;
    RMF::NodeConstHandles nhs = rh.get_children();
    for (unsigned int i = 0; i < nhs.size(); ++i) {
      cur += get_count(nhs[i], k);
    }
    return cur;
  }
};


template <class Traits>
void show_key_info(RMF::FileConstHandle rh,
                   RMF::Category        cat,
                   std::string          name,
                   std::ostream         &out) {
  std::vector<RMF::Key<Traits> > keys = rh.get_keys<Traits>(cat);
  for (unsigned int i = 0; i < keys.size(); ++i) {
    out << "    " << rh.get_name(keys[i]);
    out << ", " << name;
    out << ", " << GetCount<Traits>::get_count(rh,
                                               keys[i]) << " uses"
        << std::endl;
  }
}



#define RMF_SHOW_TYPE_DATA_INFO(lcname, UCName, PassValue, ReturnValue, \
                                PassValues, ReturnValues)               \
  show_key_info<RMF::UCName##Traits>(rh, categories[i], #lcname, out);

int count(RMF::NodeConstHandle nh, std::set<RMF::NodeConstHandle> &seen) {
  int ret = 0;
  if (seen.find(nh) == seen.end()) {
    ++ret;
    seen.insert(nh);
  } else {
    return 0;
  }
  RMF::NodeConstHandles ch = nh.get_children();
  for (unsigned int i = 0; i < ch.size(); ++i) {
    ret += count(ch[i], seen);
  }
  return ret;
}

void show_info(RMF::FileConstHandle rh, std::ostream &out) {
  out << "Nodes:" << std::endl;
  std::set<RMF::NodeConstHandle> seen;
  out << "  number: " << count(rh.get_root_node(), seen) << std::endl;
  std::vector<RMF::Category > categories
    = rh.get_categories();
  for (unsigned int i = 0; i < categories.size(); ++i) {
    out << "  " << rh.get_name(categories[i]) << ":" << std::endl;
    RMF_FOREACH_TYPE(RMF_SHOW_TYPE_DATA_INFO);
  }
}

int frame = 0;
}

int main(int argc, char **argv) {
  try {
    options.add_options()
      ("frame,f", boost::program_options::value< int >(&frame),
      "Frame to use");
    RMF_ADD_INPUT_FILE("rmf");
    process_options(argc, argv);


    RMF::FileConstHandle rh = RMF::open_rmf_file_read_only(input);
    if (!rh.get_description().empty()) {
      std::cout << "description: " << rh.get_description();
    }
    std::cout << "frames: " << rh.get_number_of_frames() << std::endl;
    show_info(rh, std::cout);
    return 0;
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
  }
}
