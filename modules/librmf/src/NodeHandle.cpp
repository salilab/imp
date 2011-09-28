/**
 *  \file RMF/Category.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#include <RMF/NodeHandle.h>
#include <boost/tuple/tuple.hpp>
#include <RMF/Category.h>
#include <RMF/RootHandle.h>

namespace RMF {

NodeHandle::NodeHandle(int node, internal::SharedData *shared):
  node_(node), shared_(shared) {
}

NodeHandle NodeHandle::add_child(std::string name, NodeType t) {
  return NodeHandle(shared_->add_child(node_, name, t), shared_.get());
}


void NodeHandle::set_association(void *d, bool overwrite) {
  shared_->set_association(node_, d, overwrite);
}
void* NodeHandle::get_association() const {
  return shared_->get_association(node_);
}

RootHandle NodeHandle::get_root_handle() const {
  return RootHandle(shared_.get());
}

std::vector<NodeHandle> NodeHandle::get_children() const {
  Ints children= shared_->get_children(node_);
  std::vector<NodeHandle> ret(children.size());
  for (unsigned int i=0; i< ret.size(); ++i) {
    ret[i]= NodeHandle(children[ret.size()-i-1], shared_.get());
  }
  return ret;
}

std::string get_type_name(NodeType t) {
  switch (t) {
  case ROOT:
    return "root";
  case REPRESENTATION:
    return "rep";
  case GEOMETRY:
    return "geom";
  case FEATURE:
    return "feat";
  default:
    return "unknown";
  }
}

namespace {
  template <class KT>
  void show_data(NodeHandle n,
                 std::ostream &out,
                 const std::vector<KT> &ks,
                 int frame, std::string prefix) {
    for (unsigned int i=0; i< ks.size(); ++i) {
      if ((n.get_root_handle().get_is_per_frame(ks[i])
           && n.get_has_value(ks[i], frame))
          || (!n.get_root_handle().get_is_per_frame(ks[i])
              && n.get_has_value(ks[i]))) {
        if (n.get_root_handle().get_is_per_frame(ks[i])) {
          out << std::endl << prefix
              << n.get_root_handle().get_name(ks[i]) << ": "
              << n.get_value(ks[i], frame)
              << " (" << n.get_root_handle().get_number_of_frames(ks[i])
              << ")";
        } else {
          out << std::endl << prefix
              << n.get_root_handle().get_name(ks[i]) << ": "
              << n.get_value(ks[i]);
        }
      }
    }
  }

  void show_node(NodeHandle n, std::ostream &out,
                 FloatKeys fks, IntKeys iks,
                 IndexKeys xks, StringKeys sks,
                 int frame,
                 std::string prefix) {
    out << get_type_name(n.get_type()) << " "
        << n.get_id() << " " << n.get_name();
    show_data(n, out, fks, frame, prefix);
    show_data(n, out, iks, frame, prefix);
    show_data(n, out, xks, frame, prefix);
    show_data(n, out, sks, frame, prefix);
  }

  template <class TypeT>
  std::vector< Key<TypeT> > get_keys(RootHandle f) {
    Categories kcs= f.get_categories();
    std::vector<Key<TypeT> > ret;
    for (unsigned int i=0; i< kcs.size(); ++i) {
      std::vector<Key<TypeT> > curp=f.get_keys<TypeT>(kcs[i]);
      ret.insert(ret.end(), curp.begin(), curp.end());
    }
    return ret;
  }
}

// Note that older g++ is confused by queue.back().get<2>()
#define IMP_RMF_PRINT_TREE(stream, NodeType, start, num_children,       \
                       get_child, show)                                 \
  {                                                                     \
    std::vector<boost::tuple<std::string, std::string, NodeType> >      \
      queue;                                                            \
    queue.push_back(boost::make_tuple(std::string(),                    \
                                      std::string(), start));           \
    do {                                                                \
      boost::tuple<std::string, std::string, NodeType> &back = queue.back(); \
      NodeType n= back.get<2>();                                        \
      std::string prefix0= back.get<0>();                               \
      std::string prefix1= back.get<1>();                               \
      queue.pop_back();                                                 \
      stream << prefix0;                                                \
      unsigned int nc= num_children;                                    \
      if (nc>0) stream << " + ";                                        \
      else stream << " - ";                                             \
      show;                                                             \
      stream  << std::endl;                                             \
      for (int i=static_cast<int>(nc-1); i>=0; --i) {                   \
        queue.push_back(boost::make_tuple(prefix1+" ",                  \
                                          prefix1+" ", get_child(i)));  \
      }                                                                 \
    } while (!queue.empty());                                           \
  }                                                                     \


void show_hierarchy(NodeHandle root,
                    std::ostream &out,
                    bool verbose,
                    unsigned int frame) {
  FloatKeys fks;
  IntKeys iks;
  IndexKeys xks;
  StringKeys sks;
  if (verbose) {
    fks=get_keys<FloatTraits>(root.get_root_handle());
    iks=get_keys<IntTraits>(root.get_root_handle());
    xks=get_keys<IndexTraits>(root.get_root_handle());
    sks=get_keys<StringTraits>(root.get_root_handle());
  }
  IMP_RMF_PRINT_TREE(out, NodeHandle, root, n.get_children().size(),
                 n.get_children().at,
                 show_node(n, out, fks, iks, xks, sks, frame,
                           prefix0+"   "));
}

} /* namespace RMF */
