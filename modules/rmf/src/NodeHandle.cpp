/**
 *  \file IMP/rmf/KeyCategory.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/rmf/NodeHandle.h>
#include <IMP/internal/utility.h>
#include <boost/tuple/tuple.hpp>
#include <IMP/rmf/KeyCategory.h>
#include <IMP/rmf/RootHandle.h>
IMPRMF_BEGIN_NAMESPACE
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
    std::vector<Key<TypeT> > ret;
    std::vector<Key<TypeT> > curp=f.get_keys<TypeT>(Physics);
    ret.insert(ret.end(), curp.begin(), curp.end());
    std::vector<Key<TypeT> > curs=f.get_keys<TypeT>(Sequence);
    ret.insert(ret.end(), curs.begin(), curs.end());
    std::vector<Key<TypeT> > curh=f.get_keys<TypeT>(Shape);
    ret.insert(ret.end(), curh.begin(), curh.end());
    std::vector<Key<TypeT> > curf=f.get_keys<TypeT>(Feature);
    ret.insert(ret.end(), curf.begin(), curf.end());
    return ret;
  }
}

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
  IMP_PRINT_TREE(out, NodeHandle, root, n.get_children().size(),
                 n.get_children().at,
                 show_node(n, out, fks, iks, xks, sks, frame,
                           prefix0+"   "));
}

namespace {
  void fill_node_tree(NodeHandle n, int parent,
                      NodeTree &tree) {
    typedef boost::property_map<NodeTree,
                                boost::vertex_name_t>::type VertexMap;
    VertexMap pm= boost::get(boost::vertex_name, tree);
    int vi= add_vertex(tree);
    pm[vi]= n;
    if (parent != -1) {
      boost::add_edge(parent, vi, tree);
    }
    NodeHandles chs= n.get_children();
    for (unsigned int i=0; i< chs.size(); ++i) {
      fill_node_tree(chs[i], vi, tree);
    }
  }
}

NodeTree get_node_tree(NodeHandle n) {
  NodeTree tree;
  fill_node_tree(n, -1, tree);
  return tree;
}

IMPRMF_END_NAMESPACE
