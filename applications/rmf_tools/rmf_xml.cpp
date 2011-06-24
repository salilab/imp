/**
 * Copyright 2007-2011 IMP Inventors. All rights reserved.
 */
#include <IMP/rmf/atom_io.h>
#include <IMP/rmf/RootHandle.h>
#include <IMP/internal/graph_utility.h>

#include <boost/program_options.hpp>
namespace po = boost::program_options;

std::string input, output;
po::options_description desc("Usage: input_hdf5");
bool help=false;
bool verbose=false;
int frame=0;
void print_help() {
  std::cerr << desc << std::endl;
}

namespace {

  std::string get_as_attribute_name(std::string name) {
    std::vector<char> data(name.begin(), name.end());
    std::vector<char>::iterator cur= data.begin();
    do {
      cur= std::find(cur, data.end(), ' ');
      if (cur== data.end()) {
        break;
      } else {
        *cur='_';
      }
    } while (true);
    return std::string(data.begin(), data.end());
  }

  template <class TypeT>
  bool show_type_data_xml(IMP::rmf::NodeHandle nh,
                          IMP::rmf::KeyCategory kc,
                          bool opened) {
    IMP::rmf::RootHandle rh= nh.get_root_handle();
    std::vector<IMP::rmf::Key<TypeT> > keys= rh.get_keys<TypeT>(kc);
    for (unsigned int i=0; i< keys.size(); ++i) {
      if (nh.get_has_value(keys[i], frame)) {
        if (!opened) {
          std::cout << "<" << kc << "\n";
          opened=true;
        }
        std::cout  << get_as_attribute_name(rh.get_name(keys[i])) << "=\""
                  << nh.get_value(keys[i], frame) << "\"\n";
      }
    }
    return opened;
  }
  void show_data_xml(IMP::rmf::NodeHandle nh,
                     IMP::rmf::KeyCategory kc) {
    bool opened=false;
    opened=show_type_data_xml<IMP::rmf::IntTraits>(nh, kc, opened);
    opened=show_type_data_xml<IMP::rmf::FloatTraits>(nh, kc, opened);
    opened=show_type_data_xml<IMP::rmf::IndexTraits>(nh, kc, opened);
    opened=show_type_data_xml<IMP::rmf::StringTraits>(nh, kc, opened);
    opened=show_type_data_xml<IMP::rmf::NodeIDTraits>(nh, kc, opened);
    opened=show_type_data_xml<IMP::rmf::DataSetTraits>(nh, kc, opened);
    if (opened) {
      std::cout << "/>\n";
    }
  }

  void show_xml(IMP::rmf::NodeHandle nh) {
    std::cout << "<node name=\"" << nh.get_name() << "\" id=\""
              << nh.get_id() << "\" "
              << "type=\"" << IMP::rmf::get_type_name(nh.get_type())
              << "\"/>\n";
    if (verbose) {
      show_data_xml(nh, IMP::rmf::Physics);
      show_data_xml(nh, IMP::rmf::Sequence);
      show_data_xml(nh, IMP::rmf::Shape);
      show_data_xml(nh, IMP::rmf::Feature);
    }
    IMP::rmf::NodeHandles children= nh.get_children();
    for (unsigned int i=0; i< children.size(); ++i) {
      std::cout << "<child>\n";
      show_xml(children[i]);
      std::cout << "</child>\n";
    }
  }
}


int main(int argc, char **argv) {
  desc.add_options()
    ("help,h", "Print the contents of an rmf file to the terminal as xml.")
    ("verbose,v", "Include lots of information about each node.")
    ("frame,f", po::value< int >(&frame),
     "Frame to use")
    ("input-file,i", po::value< std::string >(&input),
     "input hdf5 file");
  po::positional_options_description p;
  p.add("input-file", 1);
  po::variables_map vm;
  po::store(
      po::command_line_parser(argc,argv).options(desc).positional(p).run(), vm);
  po::notify(vm);
  verbose= vm.count("verbose");
  if (vm.count("help") || input.empty()) {
    print_help();
    return 1;
  }
  IMP::rmf::RootHandle rh(input, false);
  std::cout << "<?xml version=\"1.0\"?>\n";
  std::cout << "<rmf>\n";
  std::cout << "<description>\n";
  std::cout << rh.get_description() <<std::endl;
  std::cout << "</description>\n";
  std::cout << "<path>\n";
  std::cout << input <<std::endl;
  std::cout << "</path>\n";
  show_xml(rh);
  if (rh.get_number_of_bonds() >0) {
    std::cout << "<bonds>\n";
    for (unsigned int i=0; i< rh.get_number_of_bonds(); ++i) {
      std::pair<IMP::rmf::NodeHandle, IMP::rmf::NodeHandle> handles
        = rh.get_bond(i);
      std::cout << "<bond id0=\""<< handles.first.get_id()
                << "\" id1=\"" << handles.second.get_id() << "\"/>\n";
    }
    std::cout << "</bonds>\n";
  }
  std::cout << "</rmf>\n";
  return 0;
}
