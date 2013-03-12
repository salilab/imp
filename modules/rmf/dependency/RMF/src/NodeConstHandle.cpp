/**
 *  \file RMF/Category.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <RMF/NodeConstHandle.h>
#include <boost/tuple/tuple.hpp>
#include <RMF/Category.h>
#include <RMF/FileHandle.h>
#include <RMF/decorators.h>

RMF_ENABLE_WARNINGS

RMF_VECTOR_DEF(NodeConstHandle);

namespace RMF {

NodeConstHandle::NodeConstHandle(int node, internal::SharedData *shared):
  node_(node), shared_(shared) {
}

FileConstHandle NodeConstHandle::get_file() const {
  return FileConstHandle(shared_.get());
}

std::vector<NodeConstHandle> NodeConstHandle::get_children() const {
  try {
    Ints children = shared_->get_children(node_);
    std::vector<NodeConstHandle> ret(children.size());
    for (unsigned int i = 0; i < ret.size(); ++i) {
      ret[i] = NodeConstHandle(children[i], shared_.get());
    }
    return ret;
  } RMF_NODE_CATCH();
}

#define RMF_HDF5_NODE_CONST_KEY_TYPE_METHODS_DEF(lcname, UCName, PassValue,          \
                                                 ReturnValue,                        \
                                                 PassValues, ReturnValues)           \
  ReturnValue NodeConstHandle::get_value(UCName##Key k) const {                      \
    try {                                                                            \
      RMF_USAGE_CHECK(get_has_value(k),                                              \
                      internal::get_error_message("Node ", get_name(),               \
                                                  " does not have a value for key ", \
                                                  shared_->get_name(k)));            \
      return get_value_always(k);                                                    \
    } RMF_NODE_CATCH_KEY(k, );                                                       \
  }                                                                                  \
  ReturnValues NodeConstHandle::get_all_values(UCName##Key k) const {                \
    try {                                                                            \
      return shared_->get_all_values(node_, k);                                      \
    } RMF_NODE_CATCH_KEY(k, );                                                       \
  }                                                                                  \
  ReturnValue NodeConstHandle::get_value_always(UCName##Key k) const {               \
    try {                                                                            \
      return shared_->get_value(node_, k);                                           \
    } RMF_NODE_CATCH_KEY(k, );                                                       \
  }                                                                                  \
  bool NodeConstHandle::get_has_value(UCName##Key k) const {                         \
    return !UCName##Traits::get_is_null_value(get_value_always(k));                  \
  }                                                                                  \
  ReturnValues NodeConstHandle::get_values_always(const UCName##Key##s & k) const {  \
    try {                                                                            \
      return shared_->get_values(node_, k);                                          \
    } RMF_NODE_CATCH();                                                              \
  }                                                                                  \
  ReturnValues NodeConstHandle::get_values(const UCName##Key##s & k) const {         \
    try {                                                                            \
      RMF_USAGE_CHECK(get_has_value(k[0]),                                           \
                      internal::get_error_message("Node ", get_name(),               \
                                                  " does not have a value for key ", \
                                                  shared_->get_name(k[0])));         \
      return get_values_always(k);                                                   \
    } RMF_NODE_CATCH();                                                              \
  }                                                                                  \
  bool NodeConstHandle::get_has_frame_value(UCName##Key k) const {                   \
    try {                                                                            \
      return shared_->get_has_frame_value(node_, k);                                 \
    } RMF_NODE_CATCH();                                                              \
  }

RMF_FOREACH_TYPE(RMF_HDF5_NODE_CONST_KEY_TYPE_METHODS_DEF);


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
  case ALIAS:
    return "alias";
  case BOND:
    return "bond";
  case CUSTOM:
    return "custom";
  case ORGANIZATIONAL:
    return "organizational";
  default:
    return "unknown";
  }
}

std::ostream &operator<<(std::ostream &out,
                         NodeType     t) {
  out << get_type_name(t);
  return out;
}
std::istream &operator>>(std::istream &in,
                         NodeType     &t) {
  std::string token;
  in >> token;
  for (NodeType i = ROOT; i < LINK; i = NodeType(i + 1)) {
    if (token == get_type_name(i)) {
      t = i;
      return in;
    }
  }
  t = CUSTOM;
  return in;
}

namespace {
template <class KT>
void show_data(NodeConstHandle  n,
               std::ostream     &out,
               const std::vector<KT> &ks,
               std::string      prefix) {
  using std::operator<<;
  for (unsigned int i = 0; i < ks.size(); ++i) {
    if (n.get_has_value(ks[i])) {
      out << std::endl << prefix
          << n.get_file().get_name(ks[i]) << ": "
          << Showable(n.get_value(ks[i]));
    }
  }
}

void show_node(NodeConstHandle n, std::ostream &out,
               std::string prefix = "") {
  using std::operator<<;
  out << prefix << "\"" << n.get_name()
      << "\" [" << get_type_name(n.get_type()) << "]";
}
void show_node(NodeConstHandle n, std::ostream &out,
               FloatKeys fks, FloatsKeys fsks,
               IntKeys iks, IntsKeys isks,
               IndexKeys xks, IndexesKeys xsks,
               StringKeys sks, StringsKeys ssks,
               NodeIDKeys nks, NodeIDsKeys nsks,
               std::string prefix) {
  using std::operator<<;
  if (true) {
    show_node(n, out);
    show_data(n, out, fks,  prefix + "  ");
    show_data(n, out, iks,  prefix + "  ");
    show_data(n, out, xks,  prefix + "  ");
    show_data(n, out, sks,  prefix + "  ");
    show_data(n, out, nks,  prefix + "  ");
    show_data(n, out, fsks, prefix + "  ");
    show_data(n, out, isks, prefix + "  ");
    show_data(n, out, xsks, prefix + "  ");
    show_data(n, out, ssks, prefix + "  ");
    show_data(n, out, nsks, prefix + "  ");
  }
}

void show_node_decorators(NodeConstHandle n, std::ostream &out,
                          ColoredConstFactory ccf,
                          ParticleConstFactory pcf,
                          IntermediateParticleConstFactory ipcf,
                          RigidParticleConstFactory rpcf,
                          ScoreConstFactory scf,
                          BallConstFactory bcf,
                          CylinderConstFactory cycf,
                          SegmentConstFactory segcf,
                          ResidueConstFactory rcf,
                          AtomConstFactory acf,
                          ChainConstFactory chaincf,
                          DomainConstFactory fragcf,
                          CopyConstFactory copycf,
                          DiffuserConstFactory diffusercf,
                          TypedConstFactory typedcf,
                          std::string ) {
  using std::operator<<;
  out << "\"" << n.get_name() << "\" [" << get_type_name(n.get_type())
      << ": ";
  if (ccf.get_is(n)) out << " color";
  if (pcf.get_is(n)) out << " particle";
  if (ipcf.get_is(n)) out << " iparticle";
  if (rpcf.get_is(n)) out << " rigid";
  if (scf.get_is(n)) out << " score";
  if (bcf.get_is(n)) out << " ball";
  if (cycf.get_is(n)) out << " cylinder";
  if (segcf.get_is(n)) out << " segment";
  if (rcf.get_is(n)) out << " residue";
  if (acf.get_is(n)) out << " atom";
  if (chaincf.get_is(n)) out << " chain";
  if (fragcf.get_is(n)) out << " domain";
  if (copycf.get_is(n)) out << " copy";
  if (typedcf.get_is(n)) out << " typed";
  if (diffusercf.get_is(n)) out << " diffuser";
  out << "]";
}

template <class TypeT>
std::vector< Key<TypeT> > get_keys(FileConstHandle f) {
  Categories kcs = f.get_categories();
  std::vector<Key<TypeT> > ret;
  for (unsigned int i = 0; i < kcs.size(); ++i) {
    std::vector<Key<TypeT> > curp = f.get_keys<TypeT>(kcs[i]);
    ret.insert(ret.end(), curp.begin(), curp.end());
  }
  return ret;
}

// Note that older g++ is confused by queue.back().get<2>()
#define RMF_PRINT_TREE(stream, NodeType, start, num_children,                \
                       get_children, show)                                   \
  {                                                                          \
    std::vector<boost::tuple<std::string, std::string, NodeType> >      \
    queue;                                                                   \
    queue.push_back(boost::make_tuple(std::string(),                         \
                                      std::string(), start));                \
    do {                                                                     \
      boost::tuple<std::string, std::string, NodeType> &back = queue.back(); \
      NodeType n = back.get<2>();                                            \
      std::string prefix0 = back.get<0>();                                   \
      std::string prefix1 = back.get<1>();                                   \
      queue.pop_back();                                                      \
      stream << prefix0;                                                     \
      std::vector<NodeType> children = get_children;                    \
      if (children.size() > 0) stream << " + ";                              \
      else stream << " - ";                                                  \
      show;                                                                  \
      stream << std::endl;                                                   \
      for (int i = static_cast<int>(children.size()) - 1; i >= 0; --i) {     \
        queue.push_back(boost::make_tuple(prefix1 + " ",                     \
                                          prefix1 + " ", children[i]));      \
      }                                                                      \
    } while (!queue.empty());                                                \
  }                                                                          \

}

void show_hierarchy(NodeConstHandle root,
                    std::ostream    &out) {
  using std::operator<<;
  RMF_PRINT_TREE(out, NodeConstHandle, root, n.get_children().size(),
                 n.get_children(),
                 show_node(n, out));
}

void show_hierarchy_with_values(NodeConstHandle root,
                                std::ostream    &out) {
  FloatKeys fks;
  IntKeys iks;
  IndexKeys xks;
  StringKeys sks;
  NodeIDKeys nks;
  FloatsKeys fsks;
  IntsKeys isks;
  IndexesKeys xsks;
  StringsKeys ssks;
  NodeIDsKeys nsks;
  fks = get_keys<FloatTraits>(root.get_file());
  iks = get_keys<IntTraits>(root.get_file());
  xks = get_keys<IndexTraits>(root.get_file());
  sks = get_keys<StringTraits>(root.get_file());
  nks = get_keys<NodeIDTraits>(root.get_file());
  fsks = get_keys<FloatsTraits>(root.get_file());
  isks = get_keys<IntsTraits>(root.get_file());
  xsks = get_keys<IndexesTraits>(root.get_file());
  ssks = get_keys<StringsTraits>(root.get_file());
  nsks = get_keys<NodeIDsTraits>(root.get_file());
  using std::operator<<;
  RMF_PRINT_TREE(out, NodeConstHandle, root, n.get_children().size(),
                 n.get_children(),
                 show_node(n, out, fks, fsks, iks, isks, xks, xsks,
                           sks, ssks, nks, nsks,
                           prefix0 + "   "));
}


void show_hierarchy_with_decorators(NodeConstHandle root,
                                    bool,
                                    std::ostream    &out) {
  ColoredConstFactory ccf(root.get_file());
  ParticleConstFactory pcf(root.get_file());
  IntermediateParticleConstFactory ipcf(root.get_file());
  RigidParticleConstFactory rpcf(root.get_file());
  ScoreConstFactory scf(root.get_file());
  BallConstFactory bcf(root.get_file());
  CylinderConstFactory cycf(root.get_file());
  SegmentConstFactory segcf(root.get_file());
  ResidueConstFactory rcf(root.get_file());
  AtomConstFactory acf(root.get_file());
  ChainConstFactory chaincf(root.get_file());
  DomainConstFactory fragcf(root.get_file());
  CopyConstFactory copycf(root.get_file());
  DiffuserConstFactory diffusercf(root.get_file());
  TypedConstFactory typedcf(root.get_file());
  using std::operator<<;
  RMF_PRINT_TREE(out, NodeConstHandle, root, n.get_children().size(),
                 n.get_children(),
                 show_node_decorators(n, out, ccf, pcf, ipcf, rpcf, scf,
                                      bcf, cycf, segcf, rcf, acf,
                                      chaincf, fragcf, copycf,
                                      diffusercf, typedcf,
                                      prefix0 + "   "));
}

} /* namespace RMF */

RMF_DISABLE_WARNINGS

