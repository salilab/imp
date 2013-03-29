/**
 *  \file RMF/Category.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <RMF/utility.h>
#include <RMF/NodeConstHandle.h>
#include <RMF/NodeHandle.h>
#include <RMF/FileConstHandle.h>
#include <RMF/FileHandle.h>
#include <RMF/internal/set.h>
#include <RMF/internal/utility.h>
#include <RMF/decorators.h>

RMF_ENABLE_WARNINGS

namespace RMF {
namespace {
void copy_structure(NodeConstHandle in, NodeHandle out,
                    const internal::set<NodeConstHandle>* set) {
  // we already saw this part of the dag
  if (in.get_has_association()) return;
  in.set_association(out.get_id());
  NodeConstHandles ch = in.get_children();
  for (unsigned int i = 0; i < ch.size(); ++i) {
    if (set && set->find(ch[i]) == set->end()) continue;
    NodeHandle nn = out.add_child(ch[i].get_name(), ch[i].get_type());
    copy_structure(ch[i], nn, set);
  }
}
void link_structure(NodeConstHandle in, NodeHandle out) {
  // to deal with aliases
  if (in.get_has_association()) return;
  in.set_association(out.get_id());
  NodeConstHandles ch = in.get_children();
  NodeHandles och = out.get_children();
  for (unsigned int i = 0; i < ch.size(); ++i) {
    link_structure(ch[i], och[i]);
  }
}
}

void copy_structure(FileConstHandle in, FileHandle out) {
  copy_structure(in.get_root_node(), out.get_root_node(), NULL);
}
void copy_structure(const NodeConstHandles& in, FileHandle out) {
  if (in.empty()) return;
  internal::set<NodeConstHandle> inputset(in.begin(),
                                          in.end());
  copy_structure(in[0].get_file().get_root_node(),
                 out.get_root_node(), &inputset);
}

void link_structure(FileConstHandle in, FileHandle out) {
  link_structure(in.get_root_node(), out.get_root_node());
}
namespace {
#define RMF_COPY_FRAME_1(lcname, Ucname, PassValue, \
                         ReturnValue,               \
                         PassValues, ReturnValues)  \
  copy_node_frame_type<Ucname##Traits>(in, out,     \
                                       incats, outcats)
#define RMF_COPY_FRAME_D(lcname, Ucname, PassValue, \
                         ReturnValue,               \
                         PassValues, ReturnValues)  \
  copy_set_frame_type<Ucname##Traits>(in, out,      \
                                      incats, outcats)

template <class TypeTraits>
void copy_node_frame_type_node(NodeConstHandle                in,
                               NodeHandle                     out,
                               const std::vector<Key<TypeTraits> > &inkeys,
                               const std::vector<Key<TypeTraits> > &outkeys) {
  if (!in.get_has_association()) return;
  for (unsigned int i = 0; i < inkeys.size(); ++i) {
    if (in.get_has_frame_value(inkeys[i])) {
      out.set_value(outkeys[i], in.get_value(inkeys[i]));
    }
  }
  NodeConstHandles inch = in.get_children();
  NodeHandles outch = out.get_children();
  for (unsigned int i = 0; i < inch.size(); ++i) {
    copy_node_frame_type_node(inch[i], outch[i],
                              inkeys, outkeys);
  }
}

template <class TypeTraits>
void copy_node_frame_type(FileConstHandle in, FileHandle out,
                          Categories incats, Categories outcats) {
  std::vector<Key<TypeTraits> > inkeys;
  std::vector<Key<TypeTraits> > outkeys;
  for (unsigned int i = 0; i < incats.size(); ++i) {
    std::vector<Key<TypeTraits> > cinkeys = in.get_keys<TypeTraits>(incats[i]);
    inkeys.insert(inkeys.end(), cinkeys.begin(), cinkeys.end());
    for (unsigned int j = 0; j < cinkeys.size(); ++j) {
      outkeys.push_back(out.get_key<TypeTraits>(outcats[i],
                                                in.get_name(cinkeys[j])));
    }
  }
    copy_node_frame_type_node(in.get_root_node(), out.get_root_node(),
                            inkeys, outkeys);
}

void copy_node_frame(FileConstHandle in, FileHandle out) {
  Categories incats = in.get_categories();
  Categories outcats;
  for (unsigned int i = 0; i < incats.size(); ++i) {
    outcats.push_back(out.get_category(in.get_name(incats[i])));
  }
  RMF_FOREACH_TYPE(RMF_COPY_FRAME_1);
}
}

void copy_frame(FileConstHandle in, FileHandle out) {
  copy_node_frame(in, out);
}



void copy_values(FileConstHandle in, FileHandle out) {
  // do something less dumb pater
  for (unsigned int i = 0; i < in.get_number_of_frames(); ++i) {
    in.set_current_frame(i);
    out.set_current_frame(i);
    copy_frame(in, out);
  }
}


namespace {
bool get_equal_node_structure(NodeConstHandle in, NodeConstHandle out,
                              bool print_diff) {
  bool ret = true;
  if (in.get_type() != out.get_type()) {
    if (print_diff) {
      std::cout << "Node types differ at " << in
                << " vs " << out << std::endl;
    }
    ret = false;
  }
  if (in.get_name() != out.get_name()) {
    if (print_diff) {
      std::cout << "Node names differ at " << in
                << " vs " << out << std::endl;
    }
    ret = false;
  }
  NodeConstHandles inch = in.get_children();
  NodeConstHandles outch = out.get_children();
  if (inch.size() != outch.size()) {
    if (print_diff) {
      std::cout << "Node number of children differ at " << in
                << " vs " << out << std::endl;
    }
    ret = false;
  }
  for (unsigned int i = 0; i < std::min(inch.size(), outch.size()); ++i) {
    ret = get_equal_node_structure(inch[i], outch[i], print_diff) && ret;
  }
  return ret;
}
}

bool get_equal_structure(FileConstHandle in, FileConstHandle out,
                         bool print_diff) {
  bool ret = true;
  ret = get_equal_node_structure(in.get_root_node(),
                                 out.get_root_node(), print_diff) && ret;
  return ret;
}


namespace {
#define RMF_EQUAL_FRAME_1(lcname, Ucname, PassValue,               \
                          ReturnValue,                             \
                          PassValues, ReturnValues)                \
  ret = get_equal_node_frame_type<Ucname##Traits>(in, out,         \
                                                  incats, outcats, \
                                                  print_diff) && ret
#define RMF_EQUAL_FRAME_D(lcname, Ucname, PassValue,              \
                          ReturnValue,                            \
                          PassValues, ReturnValues)               \
  ret = get_equal_set_frame_type<Ucname##Traits>(in, out,         \
                                                 incats, outcats, \
                                                 print_diff) && ret

template <class TypeTraits>
bool get_equal_node_frame_type_node(NodeConstHandle                in,
                                    NodeConstHandle                out,
                                    const std::vector<Key<TypeTraits> > &inkeys,
                                    const std::vector<Key<TypeTraits> > &outkeys,
                                    bool                           print_diff) {
  bool ret = true;
  for (unsigned int i = 0; i < inkeys.size(); ++i) {
    if (in.get_has_frame_value(inkeys[i])
        != out.get_has_frame_value(outkeys[i])) {
      if (print_diff) {
        std::cout << "Node differ about having value "
                  << in.get_file().get_name(inkeys[i]) << " at "
                  << in << " and " << out << std::endl;
      }
      ret = false;
    } else if (in.get_has_frame_value(inkeys[i])
               && in.get_value(inkeys[i])
               != out.get_value(outkeys[i])) {
      if (print_diff) {
        std::cout << "Node differ about value "
                  << in.get_file().get_name(inkeys[i]) << " at "
                  << in << " and " << out << " "
                  << in.get_value(inkeys[i])
                  << " != "
                  << out.get_value(outkeys[i]) << std::endl;
      }
      ret = false;
    }
  }
  NodeConstHandles inch = in.get_children();
  NodeConstHandles outch = out.get_children();
  for (unsigned int i = 0; i < inch.size(); ++i) {
    ret = get_equal_node_frame_type_node(inch[i], outch[i],
                                         inkeys, outkeys, print_diff) && ret;
  }
  return ret;
}

template <class TypeTraits>
bool get_equal_node_frame_type(FileConstHandle in, FileConstHandle out,
                               Categories incats, Categories outcats,
                               bool print_diff) {
  std::vector<Key<TypeTraits> > inkeys;
  std::vector<Key<TypeTraits> > outkeys;
  for (unsigned int i = 0; i < incats.size(); ++i) {
    std::vector<Key<TypeTraits> > cinkeys = in.get_keys<TypeTraits>(incats[i]);
    inkeys.insert(inkeys.end(), cinkeys.begin(), cinkeys.end());
    for (unsigned int j = 0; j < cinkeys.size(); ++j) {
      outkeys.push_back(out.get_key<TypeTraits>
                          ( outcats[i],
                          in.get_name(cinkeys[j])));
    }
  }
  return get_equal_node_frame_type_node(in.get_root_node(),
                                        out.get_root_node(),
                                        inkeys, outkeys, print_diff);
}

bool get_equal_node_frame(FileConstHandle in, FileConstHandle out,
                          bool print_diff) {
  Categories incats = in.get_categories();
  Categories outcats;
  for (unsigned int i = 0; i < incats.size(); ++i) {
    outcats.push_back(out.get_category(in.get_name(incats[i])));
  }
  bool ret = true;
  RMF_FOREACH_TYPE(RMF_EQUAL_FRAME_1);
  return ret;
}

}


bool get_equal_frame(FileConstHandle in, FileConstHandle out,
                     bool print_diff) {
  bool ret = true;
  ret = get_equal_node_frame(in, out, print_diff) && ret;
  return ret;
}



void add_child_alias(AliasFactory    af,
                     NodeHandle      parent,
                     NodeConstHandle child) {
  internal::add_child_alias(af, parent, child);
}

void test_throw_exception() {
  RMF_THROW(Message("Test exception"), UsageException);
}

} /* namespace RMF */

RMF_DISABLE_WARNINGS
