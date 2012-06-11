/**
 *  \file RMF/Category.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#include <RMF/NodeConstHandle.h>
#include <boost/tuple/tuple.hpp>
#include <RMF/Category.h>
#include <RMF/FileHandle.h>
#include <RMF/decorators.h>

namespace RMF {

NodeConstHandle::NodeConstHandle(int node, internal::SharedData *shared):
  node_(node), shared_(shared) {
}

FileConstHandle NodeConstHandle::get_file() const {
  return FileConstHandle(shared_.get());
}

vector<NodeConstHandle> NodeConstHandle::get_children() const {
  Ints children= shared_->get_children(node_);
  vector<NodeConstHandle> ret(children.size());
  for (unsigned int i=0; i< ret.size(); ++i) {
    ret[i]= NodeConstHandle(children[i], shared_.get());
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
  case ALIAS:
    return "alias";
  default:
    return "unknown";
  }
}

namespace {
  template <class Types, class Type>
  void show_clean(std::string prefix, std::string name,
                  const Types &t, Type nv, int frame, int end_frame, int step,
                  std::ostream &out) {
    Types tout;
    bool found=false;
    if (step==-1) step=1;
    for ( int i=frame; i< end_frame; i+=step) {
      tout.push_back(t[i]);
      if (t[i] != nv) {
        found=true;
      }
    }
    if (found) {
      out << std::endl << prefix
          << name << ": [";
      for (unsigned int i=0; i< tout.size(); ++i) {
        if (i!= 0) {
          out << ", ";
        }
        if (tout[i]==nv) {
          out << "-";
        } else{
          out << Showable(tout[i]);
        }
      }
      out << "]";
    }
  }


  template <class KT>
  void show_data(NodeConstHandle n,
                 std::ostream &out,
                 const vector<KT> &ks,
                 int frame, int end_frame, int step,
                 std::string prefix) {
    using std::operator<<;
    for (unsigned int i=0; i< ks.size(); ++i) {
      if (!n.get_file().get_is_per_frame(ks[i])
          && n.get_has_value(ks[i])) {
        out << std::endl << prefix
            << n.get_file().get_name(ks[i]) << ": "
            << Showable(n.get_value(ks[i]));
      } else if (end_frame==-1) {
        if (n.get_file().get_is_per_frame(ks[i])
            && n.get_has_value(ks[i], frame)) {
          out << std::endl << prefix
              << n.get_file().get_name(ks[i]) << ": "
              << Showable(n.get_value(ks[i], frame));
        }
      } else {
        show_clean(prefix, n.get_file().get_name(ks[i]),
                   n.get_all_values(ks[i]), KT::TypeTraits::get_null_value(),
                   frame, end_frame, step, out);
      }
    }
  }

  void show_node(NodeConstHandle n, std::ostream &out,
                 std::string prefix="") {
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
                 int frame, int end_frame, int step,
                 std::string prefix) {
    using std::operator<<;
    if (n.get_type()== ALIAS) {
      show_node(n, out);
    } else {
      show_node(n, out);
      show_data(n, out, fks, frame, end_frame, step, prefix+"  ");
      show_data(n, out, iks, frame, end_frame, step, prefix+"  ");
      show_data(n, out, xks, frame, end_frame, step, prefix+"  ");
      show_data(n, out, sks, frame, end_frame, step, prefix+"  ");
      show_data(n, out, nks, frame, end_frame, step, prefix+"  ");
      show_data(n, out, fsks, frame, end_frame, step, prefix+"  ");
      show_data(n, out, isks, frame, end_frame, step, prefix+"  ");
      show_data(n, out, xsks, frame, end_frame, step, prefix+"  ");
      show_data(n, out, ssks, frame, end_frame, step, prefix+"  ");
      show_data(n, out, nsks, frame, end_frame, step, prefix+"  ");
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
                            AliasConstFactory aliascf,
                            int frame,
                            std::string ) {
    using std::operator<<;
    out<< "\"" << n.get_name() << "\" [" << get_type_name(n.get_type())
       << ": ";
    if (ccf.get_is(n, frame)) out <<" color" ;
    if (pcf.get_is(n, frame)) out <<" particle" ;
    if (ipcf.get_is(n, frame)) out <<" iparticle" ;
    if (rpcf.get_is(n, frame)) out <<" rigid" ;
    if (scf.get_is(n, frame)) out <<" score" ;
    if (bcf.get_is(n, frame)) out <<" ball" ;
    if (cycf.get_is(n, frame)) out <<" cylinder" ;
    if (segcf.get_is(n, frame)) out <<" segment" ;
    if (rcf.get_is(n, frame)) out <<" residue" ;
    if (acf.get_is(n, frame)) out <<" atom" ;
    if (chaincf.get_is(n, frame)) out <<" chain" ;
    if (fragcf.get_is(n, frame)) out <<" domain" ;
    if (copycf.get_is(n, frame)) out <<" copy" ;
    if (typedcf.get_is(n, frame)) out <<" typed" ;
    if (diffusercf.get_is(n, frame)) out <<" diffuser" ;
    if (aliascf.get_is(n, frame)) out <<" alias" ;
    out << "]";
  }

  template <class TypeT>
  vector< Key<TypeT, 1> > get_keys(FileConstHandle f) {
    Categories kcs= f.get_categories();
    vector<Key<TypeT, 1> > ret;
    for (unsigned int i=0; i< kcs.size(); ++i) {
      vector<Key<TypeT, 1> > curp=f.get_keys<TypeT, 1>(kcs[i]);
      ret.insert(ret.end(), curp.begin(), curp.end());
    }
    return ret;
  }

// Note that older g++ is confused by queue.back().get<2>()
#define IMP_RMF_PRINT_TREE(stream, NodeType, start, num_children,       \
                           get_children, show)                             \
  {                                                                     \
    vector<boost::tuple<std::string, std::string, NodeType> >           \
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
      vector<NodeType> children=get_children;                           \
      if (children.size()>0) stream << " + ";                           \
      else stream << " - ";                                             \
      show;                                                             \
      stream  << std::endl;                                             \
      for (int i=static_cast<int>(children.size())-1; i>=0; --i) {      \
        queue.push_back(boost::make_tuple(prefix1+" ",                  \
                                          prefix1+" ", children[i]));   \
      }                                                                 \
    } while (!queue.empty());                                           \
  }                                                                     \

}

void show_hierarchy(NodeConstHandle root,
                    std::ostream &out) {
  using std::operator<<;
  IMP_RMF_PRINT_TREE(out, NodeConstHandle, root, n.get_children().size(),
                 n.get_children(),
                     show_node(n, out));
}

  void show_hierarchy_with_values(NodeConstHandle root,
                                  unsigned int frame,
                                  int end_frame, int step,
                                  std::ostream &out) {
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
  fks=get_keys<FloatTraits>(root.get_file());
  iks=get_keys<IntTraits>(root.get_file());
  xks=get_keys<IndexTraits>(root.get_file());
  sks=get_keys<StringTraits>(root.get_file());
  nks=get_keys<NodeIDTraits>(root.get_file());
  fsks=get_keys<FloatsTraits>(root.get_file());
  isks=get_keys<IntsTraits>(root.get_file());
  xsks=get_keys<IndexesTraits>(root.get_file());
  ssks=get_keys<StringsTraits>(root.get_file());
  nsks=get_keys<NodeIDsTraits>(root.get_file());
  using std::operator<<;
  IMP_RMF_PRINT_TREE(out, NodeConstHandle, root, n.get_children().size(),
                 n.get_children(),
                     show_node(n, out, fks, fsks, iks, isks, xks, xsks,
                               sks, ssks, nks, nsks, frame, end_frame, step,
                               prefix0+"   "));
  }


void show_hierarchy_with_decorators(NodeConstHandle root,
                                    bool ,
                                    unsigned int frame,
                                    std::ostream &out) {
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
  AliasConstFactory aliascf(root.get_file());
  using std::operator<<;
  IMP_RMF_PRINT_TREE(out, NodeConstHandle, root, n.get_children().size(),
                 n.get_children(),
                     show_node_decorators(n, out, ccf, pcf, ipcf, rpcf, scf,
                                          bcf, cycf, segcf, rcf, acf,
                                          chaincf, fragcf, copycf,
                                          diffusercf, typedcf, aliascf, frame,
                           prefix0+"   "));
}



NodeConstHandles get_children_resolving_aliases(NodeConstHandle nh) {
  AliasConstFactory saf(nh.get_file());
  NodeConstHandles ret= nh.get_children();
  for (unsigned int i=0; i< ret.size(); ++i) {
    if (ret[i].get_type()== ALIAS && saf.get_is(ret[i])) {
      ret[i]= saf.get(ret[i]).get_aliased();
    }
  }
  return ret;
}


} /* namespace RMF */
