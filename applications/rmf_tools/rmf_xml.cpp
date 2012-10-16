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

template <class TypeT, int Arity, class Handle>
  bool show_type_data_xml(Handle nh,
                          RMF::CategoryD<Arity> kc,
                          bool opened, int frame, std::ostream &out) {
    using RMF::operator<<;
    RMF::FileConstHandle rh= nh.get_file();
    std::vector<RMF::Key<TypeT, Arity> > keys= rh.get_keys<TypeT>(kc);
    for (unsigned int i=0; i< keys.size(); ++i) {
      //std::cout << "key " << rh.get_name(keys[i]) << std::endl;
      if (rh.get_is_per_frame(keys[i])) {
        if (frame >=0) {
          if (nh.get_has_value(keys[i], frame)) {
            if (!opened) {
              out << "<" << nh.get_file().get_category_name(kc) << "\n";
              opened=true;
            }
            out  << get_as_attribute_name(rh.get_name(keys[i])) << "=\"";
            out << nh.get_value(keys[i], frame) << "\"\n";
          }
        } else {
          int skip=-frame;
          std::ostringstream oss;
          bool some=false;
          for (unsigned int j=0; j< rh.get_number_of_frames(); j+=skip) {
            if (j != 0) {
              oss << " ";
            }
            if (nh.get_has_value(keys[i], j)) {
              oss << nh.get_value(keys[i], j);
              some=true;
            } else {
              oss << "-";
            }
          }
          if (some) {
            if (!opened) {
              out << "<" << nh.get_file().get_category_name(kc)  << "\n";
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
            out << "<" << nh.get_file().get_category_name(kc) << "\n";
            opened=true;
          }
          out  << get_as_attribute_name(rh.get_name(keys[i])) << "=\"";
          out << nh.get_value(keys[i]) << "\"\n";
        }
      }
    }
    return opened;
  }
#define IMP_RMF_SHOW_TYPE_DATA_XML(lcname, UCName, PassValue, ReturnValue, \
                                   PassValues, ReturnValues)            \
  opened=show_type_data_xml<RMF::UCName##Traits, Arity>(nh, kc, opened, frame, \
                                                        out);

template <int Arity, class Handle>
  void show_data_xml(Handle nh,
                     RMF::CategoryD<Arity> kc,
                     int frame,
                     std::ostream &out) {
    bool opened=false;
    IMP_RMF_FOREACH_TYPE(IMP_RMF_SHOW_TYPE_DATA_XML);
    if (opened) {
      out << "/>\n";
    }
  }

  void show_hierarchy(RMF::NodeConstHandle nh,
                      const RMF::Categories& cs, int frame,
                      std::ostream &out) {
    out << "<node name=\"" << nh.get_name() << "\" id=\""
        << nh.get_id() << "\" "
        << "type=\"" << RMF::get_type_name(nh.get_type())
        << "\">\n";
    if (verbose) {
      for (unsigned int i=0; i< cs.size(); ++i) {
        show_data_xml<1>(nh, cs[i], frame, out);
      }
    }
    RMF::NodeConstHandles children= nh.get_children();
    for (unsigned int i=0; i< children.size(); ++i) {
      out << "<child>\n";
      show_hierarchy(children[i],cs,  frame, out);
      out << "</child>\n";
    }
    out << "</node>" << std::endl;
  }
}


template <int Arity>
void show_sets(RMF::FileConstHandle rh,
               const RMF::vector<RMF::CategoryD<Arity> >& cs,
               int frame,
               std::ostream &out) {
  std::vector<RMF::NodeSetConstHandle<Arity> > sets= rh.get_node_sets<Arity>();
  if (!sets.empty()) {
    out << "<sets" << Arity << ">" << std::endl;
    for (unsigned int i=0; i< sets.size(); ++i) {
      out << "<set id=\"" << sets[i].get_id().get_index()
          << "\" type=\"" << RMF::get_set_type_name(sets[i].get_type())
          << "\" members=\"";
      for (unsigned int j=0; j< Arity; ++j) {
        if (j >0) {
          out << ", ";
        }
        out << sets[i].get_node(j).get_id().get_index();
      }
      out << "\">" << std::endl;
      if (verbose) {
        for (unsigned int j=0; j< cs.size(); ++j) {
          show_data_xml<Arity>(sets[i], cs[j], frame, out);
        }
      }
      out << "</set>" << std::endl;
    }
    out << "</sets"<< Arity << ">" << std::endl;
  }
}





int main(int argc, char **argv) {
  try {
    IMP_ADD_INPUT_FILE("rmf");
    IMP_ADD_OUTPUT_FILE("xml");
    IMP_ADD_FRAMES;
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
    show_hierarchy(rh.get_root_node(), cs, frame_option, *out);

    show_sets<2>(rh, rh.get_categories<2>(), frame_option, *out);
    show_sets<3>(rh, rh.get_categories<3>(), frame_option, *out);
    show_sets<4>(rh, rh.get_categories<4>(), frame_option, *out);
    *out << "</rmf>\n";
    return 0;
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
  }
}
