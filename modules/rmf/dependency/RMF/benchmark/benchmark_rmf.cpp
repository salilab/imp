/**
 * Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#include <RMF/FileHandle.h>
#include <boost/timer.hpp>
#include <sstream>

namespace {
template <class T>
double count(T t) {
  return t;
}
double count(std::string nm) {
  return nm.size();
}
double count(RMF::NodeID nm) {
  return nm.get_index();
}

template <class TypeT>
double show_type_data_xml(RMF::NodeConstHandle nh,
                          RMF::Category        kc) {
  double ret = 0;
  RMF::FileConstHandle rh = nh.get_file();
  std::vector< RMF::Key<TypeT> > keys = rh.get_keys<TypeT>(kc);
  for (unsigned int i = 0; i < keys.size(); ++i) {
    if (nh.get_has_value(keys[i])) {
      ret += count(nh.get_value(keys[i]));
      ret += rh.get_name(keys[i]).size();
    }
  }
  return ret;
}
double show_data_xml(RMF::NodeConstHandle nh,
                     RMF::Category        kc) {
  double ret = 0;
  ret += show_type_data_xml< RMF::IntTraits>(nh, kc);
  ret += show_type_data_xml< RMF::FloatTraits>(nh, kc);
  ret += show_type_data_xml< RMF::IndexTraits>(nh, kc);
  ret += show_type_data_xml< RMF::StringTraits>(nh, kc);
  ret += show_type_data_xml< RMF::NodeIDTraits>(nh, kc);
  return ret;
}

double show_xml(RMF::NodeConstHandle  nh,
                const RMF::Categories &kcs) {
  double ret = 0;
  ret += nh.get_name().size();
  ret += nh.get_id().get_index();
  for (unsigned int i = 0; i < kcs.size(); ++i) {
    ret += show_data_xml(nh, kcs[i]);
  }
  RMF::NodeConstHandles children = nh.get_children();
  for (unsigned int i = 0; i < children.size(); ++i) {
    ret += show_xml(children[i], kcs);
  }
  return ret;
}


double traverse(std::string name) {
  double ret = 0;
  RMF::FileConstHandle rh = RMF::open_rmf_file_read_only(name);
  RMF::Categories kcs = rh.get_categories();
  ret += show_xml(rh.get_root_node(), kcs);
  return ret;
}
}

int main(int, char **) {
  try {
    boost::timer timer;
    double dist = traverse("rnapii.rmf");

    /*std::cout << "TEST1 (decorator_access)  took " << runtime
       << " (" << dist << ")"<< std::endl;*/
    std::cout << "rmf c++" << timer.elapsed() << " " << dist << std::endl;
  } catch (const std::exception &e) {
    std::cerr << "Exception thrown: " << e.what() << std::endl;
  }
  return 0;
}
