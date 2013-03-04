/**
 * Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#include <RMF/FileConstHandle.h>
#include <RMF/NodeConstHandle.h>
#include <RMF/utility.h>
#include "common.h"
#include <sstream>
#include <fstream>

namespace {
std::string description
  = "Convert an rmf file into an xml file suitable for opening in a web browser.";


std::string get_as_attribute_name(std::string name) {
  std::vector<char> data(name.begin(), name.end());
  std::vector<char>::iterator cur = data.begin();
  do {
    cur = std::find(cur, data.end(), ' ');
    if (cur == data.end()) {
      break;
    } else {
      *cur = '_';
    }
  } while (true);
  return std::string(data.begin(), data.end());
}

template <class TypeT, class Handle>
bool show_type_data_xml(Handle nh,
                        RMF::Category kc,
                        bool opened, std::ostream &out) {
  using RMF::operator<<;
  RMF::FileConstHandle rh = nh.get_file();
  std::vector<RMF::Key<TypeT> > keys = rh.get_keys<TypeT>(kc);
  for (unsigned int i = 0; i < keys.size(); ++i) {
    //std::cout << "key " << rh.get_name(keys[i]) << std::endl;
    if (nh.get_has_value(keys[i])) {
      if (!opened) {
        out << "<" << nh.get_file().get_name(kc) << "\n";
        opened = true;
      }
      out << get_as_attribute_name(rh.get_name(keys[i])) << "=\"";
      out << nh.get_value(keys[i]) << "\"\n";
    }
  }
  return opened;
}
#define RMF_SHOW_TYPE_DATA_XML(lcname, UCName, PassValue, ReturnValue, \
                               PassValues, ReturnValues)               \
  opened = show_type_data_xml<RMF::UCName##Traits>(nh, kc, opened,     \
                                                   out);

template <class Handle>
void show_data_xml(Handle        nh,
                   RMF::Category kc,
                   std::ostream  &out) {
  bool opened = false;
  RMF_FOREACH_TYPE(RMF_SHOW_TYPE_DATA_XML);
  if (opened) {
    out << "/>\n";
  }
}

void show_hierarchy(RMF::NodeConstHandle           nh,
                    const RMF::Categories          & cs,
                    std::set<RMF::NodeConstHandle> &seen,
                    std::ostream                   &out) {
  out << "<node name=\"" << nh.get_name() << "\" id=\""
      << nh.get_id() << "\" "
      << "type=\"" << RMF::get_type_name(nh.get_type())
      << "\">\n";
  if (seen.find(nh) == seen.end()) {
    if (verbose) {
      for (unsigned int i = 0; i < cs.size(); ++i) {
        show_data_xml(nh, cs[i], out);
      }
    }
    RMF::NodeConstHandles children = nh.get_children();
    for (unsigned int i = 0; i < children.size(); ++i) {
      out << "<child>\n";
      show_hierarchy(children[i], cs, seen, out);
      out << "</child>\n";
    }
    seen.insert(nh);
  }
  out << "</node>" << std::endl;
}
}




int main(int argc, char **argv) {
  try {
    RMF_ADD_INPUT_FILE("rmf");
    RMF_ADD_OUTPUT_FILE("xml");
    int frame = 0;
    options.add_options() ("frame,f",                                    \
                           boost::program_options::value< int >(&frame), \
                           "Frame to use, if -1 just show static data");

    process_options(argc, argv);

    RMF::FileConstHandle rh = RMF::open_rmf_file_read_only(input);
    std::ostream *out;
    std::ofstream fout;
    if (!output.empty()) {
      fout.open(output.c_str());
      if (!fout) {
        std::cerr << "Error opening file " << output << std::endl;
        return 1;
      }
      out = &fout;
    } else {
      out = &std::cout;
    }
    RMF::Categories cs = rh.get_categories();
    *out << "<?xml version=\"1.0\"?>\n";
    *out << "<rmf>\n";
    *out << "<description>\n";
    *out << rh.get_description() << std::endl;
    *out << "</description>\n";
    *out << "<path>\n";
    *out << input << std::endl;
    *out << "</path>\n";
    std::set<RMF::NodeConstHandle> seen;
    rh.set_current_frame(frame);
    show_hierarchy(rh.get_root_node(), cs, seen, *out);
    *out << "</rmf>\n";
    return 0;
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
  }
}
