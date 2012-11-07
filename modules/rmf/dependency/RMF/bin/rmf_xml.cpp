/**
 * Copyright 2007-2012 IMP Inventors. All rights reserved.
 */
#include <RMF/FileConstHandle.h>
#include <RMF/NodeConstHandle.h>
#include <RMF/utility.h>
#include "common.h"
#include <sstream>
#include <fstream>

std::string description
="Convert an rmf file into an xml file suitable for opening in a web browser.";
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

template <class TypeT, class Handle>
  bool show_type_data_xml(Handle nh,
                          RMF::Category kc,
                          bool opened, int frame, std::ostream &out) {
    using RMF::operator<<;
    RMF::FileConstHandle rh= nh.get_file();
    std::vector<RMF::Key<TypeT> > keys= rh.get_keys<TypeT>(kc);
    for (unsigned int i=0; i< keys.size(); ++i) {
      //std::cout << "key " << rh.get_name(keys[i]) << std::endl;
      if (rh.get_is_per_frame(keys[i])) {
        if (frame >=0) {
          nh.get_file().set_current_frame(frame);
          if (nh.get_has_value(keys[i])) {
            if (!opened) {
              out << "<" << nh.get_file().get_name(kc) << "\n";
              opened=true;
            }
            out  << get_as_attribute_name(rh.get_name(keys[i])) << "=\"";
            out << nh.get_value(keys[i]) << "\"\n";
          }
        } else {
          int skip=-frame;
          std::ostringstream oss;
          bool some=false;
          for (unsigned int j=0; j< rh.get_number_of_frames(); j+=skip) {
            if (j != 0) {
              oss << " ";
            }
            nh.get_file().set_current_frame(j);
            if (nh.get_has_value(keys[i])) {
              oss << nh.get_value(keys[i]);
              some=true;
            } else {
              oss << "-";
            }
          }
          if (some) {
            if (!opened) {
              out << "<" << nh.get_file().get_name(kc)  << "\n";
              opened=true;
            }
            out << get_as_attribute_name(rh.get_name(keys[i])) << "=\"";
            out << oss.str() << "\"\n";
          } else {
            /*std::cout << "No frames " << rh.get_name(keys[i])
              << " " << rh.get_is_per_frame(keys[i]) << " " << frame
              << " " << nh.get_has_value(keys[i], frame) << std::endl;*/
          }
        }
      } else {
        if (nh.get_has_value(keys[i])) {
          if (!opened) {
            out << "<" << nh.get_file().get_name(kc) << "\n";
            opened=true;
          }
          out  << get_as_attribute_name(rh.get_name(keys[i])) << "=\"";
          out << nh.get_value(keys[i]) << "\"\n";
        }
      }
    }
    return opened;
  }
#define RMF_SHOW_TYPE_DATA_XML(lcname, UCName, PassValue, ReturnValue, \
                                   PassValues, ReturnValues)            \
  opened=show_type_data_xml<RMF::UCName##Traits>(nh, kc, opened, frame, \
                                                 out);

  template <class Handle>
  void show_data_xml(Handle nh,
                     RMF::Category kc,
                     int frame,
                     std::ostream &out) {
    bool opened=false;
    RMF_FOREACH_TYPE(RMF_SHOW_TYPE_DATA_XML);
    if (opened) {
      out << "/>\n";
    }
  }

  void show_hierarchy(RMF::NodeConstHandle nh,
                      const RMF::Categories& cs, int frame,
                      std::set<RMF::NodeConstHandle> &seen,
                      std::ostream &out) {
    out << "<node name=\"" << nh.get_name() << "\" id=\""
        << nh.get_id() << "\" "
        << "type=\"" << RMF::get_type_name(nh.get_type())
        << "\">\n";
    if (seen.find(nh) ==seen.end()) {
      if (verbose) {
        for (unsigned int i=0; i< cs.size(); ++i) {
          show_data_xml(nh, cs[i], frame, out);
        }
      }
      RMF::NodeConstHandles children= nh.get_children();
      for (unsigned int i=0; i< children.size(); ++i) {
        out << "<child>\n";
        show_hierarchy(children[i],cs,  frame, seen, out);
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
    RMF_ADD_FRAMES;
    process_options(argc, argv);

    if (0) {
      std::cout << begin_frame << end_frame << frame_step;
    }

    RMF::FileConstHandle rh= RMF::open_rmf_file_read_only(input);
    std::ostream *out;
    std::ofstream fout;
    if (!output.empty()) {
      fout.open(output.c_str());
      if (!fout) {
        std::cerr << "Error opening file " << output << std::endl;
        return 1;
      }
      out =&fout;
    } else {
      out = &std::cout;
    }
    RMF::Categories cs= rh.get_categories();
    *out << "<?xml version=\"1.0\"?>\n";
    *out << "<rmf>\n";
    *out << "<description>\n";
    *out << rh.get_description() <<std::endl;
    *out << "</description>\n";
    *out << "<path>\n";
    *out << input <<std::endl;
    *out << "</path>\n";
    std::set<RMF::NodeConstHandle> seen;
    show_hierarchy(rh.get_root_node(), cs, frame_option, seen, *out);
    *out << "</rmf>\n";
    return 0;
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
  }
}
