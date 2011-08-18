/**
 * Copyright 2007-2011 IMP Inventors. All rights reserved.
 */
#include <IMP/rmf/atom_io.h>
#include <IMP/rmf/RootHandle.h>
#include <IMP/internal/graph_utility.h>
#include <boost/timer.hpp>
#include <IMP/benchmark/utility.h>
#include <IMP/benchmark/benchmark_macros.h>
#include <sstream>

namespace {
  template <class T>
  double count(T t) {
    return t;
  }
  double count(std::string nm) {
    return nm.size();
  }
  double count(IMP::rmf::NodeID nm) {
    return nm.get_index();
  }

  template <class TypeT>
  double show_type_data_xml(IMP::rmf::NodeHandle nh,
                          IMP::rmf::Category kc) {
    double ret=0;
    IMP::rmf::RootHandle rh= nh.get_root_handle();
    std::vector<IMP::rmf::Key<TypeT> > keys= rh.get_keys<TypeT>(kc);
    for (unsigned int i=0; i< keys.size(); ++i) {
      if (nh.get_has_value(keys[i])) {
        ret+= count(nh.get_value(keys[i]));
        ret+= rh.get_name(keys[i]).size();
      }
    }
    return ret;
  }
  double show_data_xml(IMP::rmf::NodeHandle nh,
                     IMP::rmf::Category kc) {
    double ret=0;
    ret+=show_type_data_xml<IMP::rmf::IntTraits>(nh, kc);
    ret+=show_type_data_xml<IMP::rmf::FloatTraits>(nh, kc);
    ret+=show_type_data_xml<IMP::rmf::IndexTraits>(nh, kc);
    ret+=show_type_data_xml<IMP::rmf::StringTraits>(nh, kc);
    ret+=show_type_data_xml<IMP::rmf::NodeIDTraits>(nh, kc);
    ret+=show_type_data_xml<IMP::rmf::DataSetTraits>(nh, kc);
    return ret;
  }

  double show_xml(IMP::rmf::NodeHandle nh) {
    double ret=0;
    ret+=nh.get_name().size();
    ret+=nh.get_id().get_index();
    ret+=show_data_xml(nh, IMP::rmf::Physics);
    ret+=show_data_xml(nh, IMP::rmf::Sequence);
    ret+=show_data_xml(nh, IMP::rmf::Shape);
    ret+=show_data_xml(nh, IMP::rmf::Feature);
    IMP::rmf::NodeHandles children= nh.get_children();
    for (unsigned int i=0; i< children.size(); ++i) {
      ret+=show_xml(children[i]);
    }
    return ret;
  }
}


double traverse(std::string name) {
  double ret=0;
  IMP::rmf::RootHandle rh= IMP::rmf::open_rmf_file(name);
  ret+=show_xml(rh);
  if (rh.get_number_of_bonds() >0) {
    for (unsigned int i=0; i< rh.get_number_of_bonds(); ++i) {
      std::pair<IMP::rmf::NodeHandle, IMP::rmf::NodeHandle> handles
        = rh.get_bond(i);
      ret+=handles.first.get_id().get_index();
      ret+=handles.second.get_id().get_index();
    }
  }
  return ret;
}

int main(int, char **) {
   double runtime, dist=0;
    IMP_TIME(
             {
               dist+=traverse(IMP::benchmark::get_data_path("rnapii.rmf"));
             }, runtime);
    /*std::cout << "TEST1 (decorator_access)  took " << runtime
      << " (" << dist << ")"<< std::endl;*/
    IMP::benchmark::report("rmf large", runtime, dist);

  return 0;
}
