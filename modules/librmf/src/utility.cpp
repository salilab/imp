/**
 *  \file RMF/Category.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#include <RMF/utility.h>
#include <RMF/NodeConstHandle.h>
#include <RMF/NodeHandle.h>
#include <RMF/FileConstHandle.h>
#include <RMF/FileHandle.h>
#include <RMF/internal/map.h>

namespace RMF {
  namespace {
    void copy_structure(NodeConstHandle in, NodeHandle out,
                        internal::map<NodeConstHandle, NodeHandle> &map){
      map[in]=out;
      NodeConstHandles ch=in.get_children();
      for (unsigned int i=0; i< ch.size(); ++i) {
        NodeHandle nn=out.add_child(ch[i].get_name(), ch[i].get_type());
        copy_structure(ch[i], nn, map);
      }
    }
    template <int Arity>
    void copy_sets(const vector<NodeSetConstHandle<Arity> > &in,
                   FileHandle out,
                   const internal::map<NodeConstHandle, NodeHandle> &map) {
      for (unsigned int i=0; i< in.size(); ++i) {
        NodeHandles nhs;
        for (unsigned int j=0; j < Arity; ++j) {
          NodeConstHandle id= in[i].get_node(j);
          nhs.push_back(map.find(id)->second);
        }
        out.add_node_set<Arity>(nhs, in[i].get_type());
      }
    }
  }
  void copy_structure(FileConstHandle in, FileHandle out){
    internal::map<NodeConstHandle, NodeHandle> map;
    copy_structure(in.get_root_node(), out.get_root_node(), map);

    copy_sets(in.get_node_pairs(), out, map);
    copy_sets(in.get_node_triplets(), out, map);
    copy_sets(in.get_node_quads(), out, map);
  }

  namespace {
#define IMP_RMF_COPY_FRAME_1(lcname, Ucname, PassValue,                 \
                             ReturnValue,                               \
                             PassValues, ReturnValues)                  \
    copy_node_frame_type<Ucname##Traits>(in, out, inframe, outframe,    \
                                         incats, outcats)
#define IMP_RMF_COPY_FRAME_D(lcname, Ucname, PassValue,                 \
                             ReturnValue,                               \
                             PassValues, ReturnValues)                  \
    copy_set_frame_type<Ucname##Traits>(in, out, inframe, outframe,     \
                                      incats, outcats)

    template <class TypeTraits>
    void copy_node_frame_type_node(NodeConstHandle in,
                                   NodeHandle out,
                                   unsigned int inframe, unsigned int outframe,
                                   const vector<Key<TypeTraits, 1> > &inkeys,
                                   const vector<Key<TypeTraits, 1> > &outkeys) {
      for (unsigned int i=0; i< inkeys.size(); ++i) {
        if (in.get_has_value(inkeys[i], inframe)) {
          out.set_value(outkeys[i], in.get_value(inkeys[i], inframe), outframe);
        }
      }
      NodeConstHandles inch=in.get_children();
      NodeHandles outch=out.get_children();
      for (unsigned int i=0; i< inch.size(); ++i) {
        copy_node_frame_type_node(inch[i],outch[i], inframe,outframe,
                                  inkeys, outkeys);
      }
    }

    template <class TypeTraits>
    void copy_node_frame_type(FileConstHandle in, FileHandle out,
                              unsigned int inframe, unsigned int outframe,
                              Categories incats, Categories outcats) {
      vector<Key<TypeTraits, 1> > inkeys;
      vector<Key<TypeTraits, 1> > outkeys;
      for (unsigned int i=0; i< incats.size(); ++i) {
        vector<Key<TypeTraits, 1> > cinkeys= in.get_keys<TypeTraits>(incats[i]);
        inkeys.insert(inkeys.end(), cinkeys.begin(), cinkeys.end());
        for (unsigned int j=0; j < cinkeys.size(); ++j) {
          outkeys.push_back(get_key_always<TypeTraits>
                            (out, outcats[i],
                             in.get_name(cinkeys[j]),
                             cinkeys[j].get_is_per_frame()));
        }
      }
      copy_node_frame_type_node(in.get_root_node(), out.get_root_node(),
                                inframe, outframe, inkeys, outkeys);
    }

    void copy_node_frame(FileConstHandle in, FileHandle out,
                         unsigned int inframe, unsigned int outframe) {
      Categories incats= in.get_categories();
      Categories outcats;
      for (unsigned int i=0; i< incats.size(); ++i) {
        outcats.push_back(get_category_always<1>(out, in.get_name(incats[i])));
      }
      IMP_RMF_FOREACH_TYPE(IMP_RMF_COPY_FRAME_1);
    }


    template <class TypeTraits, int Arity>
     void copy_set_frame_type(FileConstHandle in, FileHandle out,
                              unsigned int inframe, unsigned int outframe,
                              const vector<CategoryD<Arity> >& incats,
                              const vector<CategoryD<Arity> >& outcats) {
      vector<Key<TypeTraits, Arity> > inkeys;
      vector<Key<TypeTraits, Arity> > outkeys;
      for (unsigned int i=0; i< incats.size(); ++i) {
        vector<Key<TypeTraits, Arity> > cinkeys
          = in.get_keys<TypeTraits>(incats[i]);
        inkeys.insert(inkeys.end(), cinkeys.begin(), cinkeys.end());
        for (unsigned int j=0; j < cinkeys.size(); ++j) {
          outkeys.push_back(get_key_always<TypeTraits>
                            (out, outcats[i],
                             in.get_name(cinkeys[j]),
                             cinkeys[j].get_is_per_frame()));
        }
      }
      vector<NodeSetConstHandle<Arity> > insets= in.get_node_sets<Arity>();
      vector<NodeSetHandle<Arity> > outsets= out.get_node_sets<Arity>();
      for (unsigned int i=0; i< insets.size(); ++i) {
        for (unsigned int j=0; j< inkeys.size(); ++j) {
          if (insets[i].get_has_value(inkeys[j], inframe)) {
            outsets[i].set_value(outkeys[j],
                                 insets[i].get_value(inkeys[i], inframe),
                                 outframe);
          }
        }
      }
    }

    template <int Arity>
    void copy_set_frame(FileConstHandle in, FileHandle out,
                        unsigned int inframe, unsigned int outframe) {
      vector<CategoryD<Arity> > incats= in.get_categories<Arity>();
      vector<CategoryD<Arity> > outcats;
      for (unsigned int i=0; i< incats.size(); ++i) {
        outcats.push_back(get_category_always<Arity>(out,
                                                     in.get_name(incats[i])));
      }
      IMP_RMF_FOREACH_TYPE(IMP_RMF_COPY_FRAME_D);
    }
  }

  void copy_frame(FileConstHandle in, FileHandle out,
                  unsigned int inframe, unsigned int outframe) {
    copy_node_frame(in, out, inframe, outframe);
    copy_set_frame<2>(in, out, inframe, outframe);
    copy_set_frame<3>(in, out, inframe, outframe);
    copy_set_frame<4>(in, out, inframe, outframe);
  }


namespace {
bool get_equal_node_structure(NodeConstHandle in, NodeConstHandle out,
                         bool print_diff) {
  bool ret=true;
  if (in.get_type() != out.get_type()) {
    if (print_diff) {
      std::cout << "Node types differ at " << in
                << " vs " << out << std::endl;
    }
    ret=false;
  }
  if (in.get_name() != out.get_name()) {
    if (print_diff) {
      std::cout << "Node names differ at " << in
                << " vs " << out << std::endl;
    }
    ret=false;
  }
  NodeConstHandles inch=in.get_children();
  NodeConstHandles outch= out.get_children();
  if (inch.size() != outch.size()) {
    if (print_diff) {
      std::cout << "Node number of children differ at " << in
                << " vs " << out << std::endl;
    }
    ret=false;
  }
  for (unsigned int i=0; i< std::min(inch.size(), outch.size()); ++i) {
    ret = get_equal_node_structure(inch[i], outch[i], print_diff) && ret;
  }
  return ret;
}

  template <int Arity>
  bool get_equal_set_structure(FileConstHandle in, FileConstHandle out,
                               bool print_diff) {
    vector<NodeSetConstHandle<Arity> > insets=in.get_node_sets<Arity>();
    vector<NodeSetConstHandle<Arity> > outsets=out.get_node_sets<Arity>();
    bool ret=true;
    if (insets.size() != outsets.size()) {
      if (print_diff) {
        std::cout << "Node sets sizes differ " << Arity << std::endl;
      }
      ret=false;
    }
    for (unsigned int i=0; i< std::min(insets.size(), outsets.size()); ++i) {
      if (insets[i].get_type() != outsets[i].get_type()) {
        if (print_diff) {
          std::cout << "Node set types differ at " << insets[i]
                    << " vs " << outsets[i] << std::endl;
        }
        ret=false;
      }
    }
    return ret;
  }
}

bool get_equal_structure(FileConstHandle in, FileConstHandle out,
                         bool print_diff) {
  bool ret=true;
  ret=get_equal_node_structure(in.get_root_node(),
                               out.get_root_node(), print_diff) &&ret;
  ret=get_equal_set_structure<2>(in, out, print_diff) &&ret;
  ret=get_equal_set_structure<3>(in, out, print_diff) &&ret;
  ret=get_equal_set_structure<4>(in, out, print_diff) &&ret;
  return ret;
}


  namespace {
#define IMP_RMF_EQUAL_FRAME_1(lcname, Ucname, PassValue,                 \
                             ReturnValue,                               \
                             PassValues, ReturnValues)                  \
    ret=get_equal_node_frame_type<Ucname##Traits>(in, out, inframe, outframe, \
                                                  incats, outcats,      \
                                                  print_diff) &&ret
#define IMP_RMF_EQUAL_FRAME_D(lcname, Ucname, PassValue,                \
                             ReturnValue,                               \
                             PassValues, ReturnValues)                  \
    ret=get_equal_set_frame_type<Ucname##Traits>(in, out, inframe, outframe, \
                                                 incats, outcats,       \
                                                 print_diff)&&ret

    template <class TypeTraits>
    bool get_equal_node_frame_type_node(NodeConstHandle in,
                                        NodeConstHandle out,
                                        unsigned int inframe,
                                        unsigned int outframe,
                                    const vector<Key<TypeTraits, 1> > &inkeys,
                                    const vector<Key<TypeTraits, 1> > &outkeys,
                                        bool print_diff) {
      bool ret=true;
      for (unsigned int i=0; i< inkeys.size(); ++i) {
          if (in.get_has_value(inkeys[i], inframe)
              != out.get_has_value(outkeys[i], outframe)) {
            if (print_diff) {
              std::cout << "Node sets differ about having value "
                        << in.get_file().get_name(inkeys[i]) << " at "
                        << in << " and " << out << std::endl;
            }
            ret=false;
          } else if (in.get_value(inkeys[i], inframe)
                     != out.get_value(outkeys[i], outframe)) {
            if (print_diff) {
              std::cout << "Node sets differ about value "
                        << in.get_file().get_name(inkeys[i]) << " at "
                        << in << " and " << out << std::endl;
            }
            ret=false;
          }
      }
      NodeConstHandles inch=in.get_children();
      NodeConstHandles outch=out.get_children();
      for (unsigned int i=0; i< inch.size(); ++i) {
        ret=get_equal_node_frame_type_node(inch[i],outch[i], inframe,outframe,
                                      inkeys, outkeys, print_diff) && ret;
      }
      return ret;
    }

    template <class TypeTraits>
    bool get_equal_node_frame_type(FileConstHandle in, FileConstHandle out,
                                   unsigned int inframe, unsigned int outframe,
                                   Categories incats, Categories outcats,
                                   bool print_diff) {
      vector<Key<TypeTraits, 1> > inkeys;
      vector<Key<TypeTraits, 1> > outkeys;
      for (unsigned int i=0; i< incats.size(); ++i) {
        vector<Key<TypeTraits, 1> > cinkeys= in.get_keys<TypeTraits>(incats[i]);
        inkeys.insert(inkeys.end(), cinkeys.begin(), cinkeys.end());
        for (unsigned int j=0; j < cinkeys.size(); ++j) {
          outkeys.push_back(out.get_key<TypeTraits>
                            ( outcats[i],
                              in.get_name(cinkeys[j]),
                              cinkeys[j].get_is_per_frame()));
        }
      }
      return get_equal_node_frame_type_node(in.get_root_node(),
                                            out.get_root_node(),
                                            inframe, outframe,
                                            inkeys, outkeys, print_diff);
    }

    bool get_equal_node_frame(FileConstHandle in, FileConstHandle out,
                              unsigned int inframe, unsigned int outframe,
                              bool print_diff) {
      Categories incats= in.get_categories();
      Categories outcats;
      for (unsigned int i=0; i< incats.size(); ++i) {
        outcats.push_back(out.get_category<1>(in.get_name(incats[i])));
      }
      bool ret=true;
      IMP_RMF_FOREACH_TYPE(IMP_RMF_EQUAL_FRAME_1);
      return ret;
    }


    template <class TypeTraits, int Arity>
     bool get_equal_set_frame_type(FileConstHandle in, FileConstHandle out,
                                   unsigned int inframe, unsigned int outframe,
                                   const vector<CategoryD<Arity> >& incats,
                                   const vector<CategoryD<Arity> >& outcats,
                                   bool print_diff) {
      vector<Key<TypeTraits, Arity> > inkeys;
      vector<Key<TypeTraits, Arity> > outkeys;
      for (unsigned int i=0; i< incats.size(); ++i) {
        vector<Key<TypeTraits, Arity> > cinkeys
          = in.get_keys<TypeTraits>(incats[i]);
        inkeys.insert(inkeys.end(), cinkeys.begin(), cinkeys.end());
        for (unsigned int j=0; j < cinkeys.size(); ++j) {
          outkeys.push_back(out.get_key<TypeTraits>
                            (outcats[i],
                             in.get_name(cinkeys[j]),
                             cinkeys[j].get_is_per_frame()));
        }
      }
      bool ret=true;
      vector<NodeSetConstHandle<Arity> > insets= in.get_node_sets<Arity>();
      vector<NodeSetConstHandle<Arity> > outsets= out.get_node_sets<Arity>();
      for (unsigned int i=0; i< insets.size(); ++i) {
        for (unsigned int j=0; j< inkeys.size(); ++j) {
          if (insets[i].get_has_value(inkeys[j], inframe)
              != outsets[i].get_has_value(outkeys[j], outframe)) {
            if (print_diff) {
              std::cout << "Node sets differ about having value "
                        << in.get_name(inkeys[i]) << " at "
                        << insets[i] << " and " << outsets[i] << std::endl;
            }
            ret=false;
          } else if (insets[i].get_value(inkeys[j], inframe)
                     != outsets[i].get_value(outkeys[j], outframe)) {
            if (print_diff) {
              std::cout << "Node sets differ about value "
                        << in.get_name(inkeys[j]) << " at "
                        << insets[i] << " and " << outsets[i] << std::endl;
            }
            ret=false;
          }
        }
      }
      return ret;
    }

    template <int Arity>
    bool get_equal_set_frame(FileConstHandle in, FileConstHandle out,
                             unsigned int inframe, unsigned int outframe,
                             bool print_diff) {
      vector<CategoryD<Arity> > incats= in.get_categories<Arity>();
      vector<CategoryD<Arity> > outcats;
      for (unsigned int i=0; i< incats.size(); ++i) {
        outcats.push_back(out.get_category<Arity>(in.get_name(incats[i])));
      }
      bool ret=true;
      IMP_RMF_FOREACH_TYPE(IMP_RMF_EQUAL_FRAME_D);
      return ret;
    }
  }


bool get_equal_frame(FileConstHandle in, FileConstHandle out,
                     unsigned int inframe, unsigned int outframe,
                     bool print_diff) {
  bool ret=true;
  ret= get_equal_node_frame(in, out, inframe, outframe, print_diff) && ret;
  ret= get_equal_set_frame<2>(in, out, inframe, outframe, print_diff) && ret;
  ret= get_equal_set_frame<3>(in, out, inframe, outframe, print_diff) && ret;
  ret= get_equal_set_frame<4>(in, out, inframe, outframe, print_diff) && ret;
  return ret;
}
} /* namespace RMF */
