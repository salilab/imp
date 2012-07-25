/**
 * Copyright 2007-2012 IMP Inventors. All rights reserved.
 */
#include <RMF/FileHandle.h>
#include <RMF/FileLock.h>
#include <sstream>
#include "common.h"

std::string description("Print out information about categories and keys.");

template <int Arity, class Traits>
struct GetCount {
  static int get_count(RMF::FileConstHandle rh,
                       RMF::Key<Traits, Arity> k) {
    RMF::vector<RMF::NodeSetConstHandle<Arity> > sets
        = rh.get_node_sets<Arity>();
    int found=0;
    for (unsigned int i=0; i< sets.size(); ++i) {
      if (sets[i].get_has_value(k)) ++found;
    }
    return found;
  }
};

template <class Traits>
struct GetCount<1, Traits> {
  static int get_count(RMF::FileConstHandle rh,
                       RMF::Key<Traits, 1> k) {
    return get_count(rh.get_root_node(), k);
  }
  static int get_count(RMF::NodeConstHandle rh,
                       RMF::Key<Traits, 1> k) {
    int cur=0;
    if (rh.get_has_value(k)) ++cur;
    RMF::NodeConstHandles nhs = rh.get_children();
    for (unsigned int i=0; i< nhs.size(); ++i) {
      cur+= get_count(nhs[i], k);
    }
    return cur;
  }
};


template <int Arity, class Traits>
void show_key_info(RMF::FileConstHandle rh,
                   RMF::CategoryD<Arity> cat,
                   std::string name,
                   std::ostream &out) {
  RMF::vector<RMF::Key<Traits, Arity> > keys = rh.get_keys<Traits>(cat);
  for (unsigned int i=0; i< keys.size(); ++i) {
    out << "    " << rh.get_name(keys[i]);
    if (rh.get_is_per_frame(keys[i])) {
      out << ", frames";
    }
    out << ", " << name;
    out << ", " << GetCount<Arity, Traits>::get_count(rh,
                                                      keys[i]) << " uses"
        << std::endl;
  }
}



#define IMP_RMF_SHOW_TYPE_DATA_INFO(lcname, UCName, PassValue, ReturnValue, \
                                    PassValues, ReturnValues)           \
  show_key_info<Arity, RMF::UCName##Traits>(rh, categories[i], #lcname, out);

template <int Arity>
void show_info(RMF::FileConstHandle rh, std::ostream &out) {
  if (rh.get_number_of_node_sets<Arity>()==0) {
    return;
  }
  if (Arity==1) {
    out << "Nodes:" << std::endl;
  } else {
    out << Arity << "-sets:" << std::endl;
  }
  out << "  number: " << rh.get_number_of_node_sets<Arity>() << std::endl;
  RMF::vector<RMF::CategoryD<Arity> > categories
      = rh.get_categories<Arity>();
  for (unsigned int i=0; i < categories.size(); ++i) {
    out << "  " << rh.get_category_name(categories[i]) << ":" << std::endl;
    IMP_RMF_FOREACH_TYPE(IMP_RMF_SHOW_TYPE_DATA_INFO);
  }
}

int frame=0;

int main(int argc, char **argv) {
  try {
    options.add_options()
      ("frame,f", boost::program_options::value< int >(&frame),
       "Frame to use");
    IMP_ADD_INPUT_FILE("rmf");
    process_options(argc, argv);


    RMF::FileConstHandle rh= RMF::open_rmf_file_read_only(input);
    RMF::FileLock lk(rh);
    if (!rh.get_description().empty()) {
      std::cout << "description: " << rh.get_description();
    }
    std::cout << "frames: " << rh.get_number_of_frames() << std::endl;
    show_info<1>(rh, std::cout);
    show_info<2>(rh, std::cout);
    show_info<3>(rh, std::cout);
    show_info<4>(rh, std::cout);
    return 0;
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
  }
}
