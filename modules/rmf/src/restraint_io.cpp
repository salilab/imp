/**
 *  \file IMP/rmf/KeyCategory.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/rmf/restraint_io.h>
#include <IMP/rmf/operations.h>
#include <IMP/scoped.h>
#include <boost/shared_array.hpp>
IMPRMF_BEGIN_NAMESPACE

#define  IMP_HDF5_CREATE_RESTRAINT_KEYS(node)                           \
  RootHandle imp_f=node.get_root_handle();                              \
  FloatKey sk= get_or_add_key<FloatTraits>(imp_f, Feature, "score",     \
                                           true);                       \
  NodeIDsKey nk                                                         \
  = get_or_add_key<NodeIDsTraits>(imp_f, Feature, "representation");

#define IMP_HDF5_ACCEPT_RESTRAINT_KEYS\
  FloatKey sk, NodeIDsKey nk

#define IMP_HDF5_PASS_RESTRAINT_KEYS\
  sk, nk

namespace {

  class  Subset {
    boost::shared_array<Particle*> ps_;
    unsigned int sz_;
    int compare(const Subset &o) const {
      if (sz_ < o.sz_) return -1;
      else if (sz_ > o.sz_) return 1;
      for (unsigned int i=0; i< size(); ++i) {
        if (ps_[i] < o[i]) return -1;
        else if (ps_[i] > o[i]) return 1;
      }
      return 0;
    }
  public:
    Subset(): sz_(0){}
    Subset(ParticlesTemp ps) {
      std::sort(ps.begin(), ps.end());
      ps.erase(std::unique(ps.begin(), ps.end()), ps.end());
      sz_= ps.size();
      ps_.reset(new Particle*[ps.size()]);
      std::copy(ps.begin(), ps.end(), ps_.get());
      IMP_USAGE_CHECK(std::unique(ps.begin(), ps.end()) == ps.end(),
                      "Duplicate particles in set");
      IMP_IF_CHECK(USAGE) {
        for (unsigned int i=0; i< ps.size(); ++i) {
          IMP_CHECK_OBJECT(ps[i]);
        }
      }
    }
    unsigned int size() const {
      return sz_;
    }
    Particle *operator[](unsigned int i) const {
      IMP_USAGE_CHECK( i < sz_, "Out of range");
      return ps_[i];
    }
    typedef Particle** const_iterator;
    const_iterator begin() const {
      return ps_.get();
    }
    const_iterator end() const {
      return ps_.get()+sz_;
    }
    IMP_SHOWABLE(Subset);
    std::string get_name() const;
    IMP_HASHABLE_INLINE(Subset, return boost::hash_range(begin(),
                                                         end()););
    IMP_COMPARISONS(Subset);
  };

  void Subset::show(std::ostream &out) const {
    out << "[";
    for (unsigned int i=0; i< size(); ++i) {
      out << "\"" <<  ps_[i]->get_name() << "\" ";
    }
    out << "]";
  }

  std::string Subset::get_name() const {
    std::ostringstream oss;
    show(oss);
    return oss.str();
  }
#if !defined(__clang__)
  inline std::size_t hash_value(const Subset &t) {
    return t.__hash__();
  }
#endif


  typedef IMP::compatibility::map<Subset, NodeHandle> Index;
  void build_index(NodeHandle parent,
                   Index &nodes,
                   IMP_HDF5_ACCEPT_RESTRAINT_KEYS) {
    IMP_UNUSED(sk);
    NodeHandles children= parent.get_children();
    for (unsigned int i=0; i< children.size(); ++i) {
      ParticlesTemp pt;
      if (children[i].get_has_value(nk)) {
        NodeIDs ids= children[i].get_value(nk);
        for (unsigned int j=0; j< ids.size(); ++j) {
          NodeHandle nh
            = parent.get_root_handle().get_node_handle_from_id(ids[j]);
          Particle *p= reinterpret_cast<Particle*>(nh.get_association());
          pt.push_back(p);
        }
      }
      IMP_USAGE_CHECK(!pt.empty(), "No used particles found. Not so good: "
                      << children[i].get_name());
      Subset s(pt);
      /*std::cout << "Adding index entry for " << s << " to "
        << parent.get_name()
        << std::endl;*/
      nodes[s]=children[i];
    }
  }

  void set_particles(NodeHandle nh,
                     const ParticlesTemp& ps,
                     IMP_HDF5_ACCEPT_RESTRAINT_KEYS) {
    IMP_UNUSED(sk);
    NodeIDs ids(ps.size());
    for (unsigned int i=0; i< ps.size(); ++i) {
      NodeID id
        =nh.get_root_handle().get_node_handle_from_association(ps[i]).get_id();
      ids[i]=id;
    }
    nh.set_value(nk, ids);
  }

  NodeHandle get_child(NodeHandle parent,
                       Restraint *r,
                       Index &nodes,
                       IMP_HDF5_ACCEPT_RESTRAINT_KEYS) {
    ParticlesTemp ip= r->get_input_particles();
    Subset s(ip);
    if (nodes.find(s) == nodes.end()) {
      NodeHandle c= parent.add_child(s.get_name(), FEATURE);
      /*std::cout << "Created node for " << s
        << " under " << parent.get_name() << std::endl;*/
      nodes[s]=c;
      c.set_association(r);
      set_particles(c, ip, IMP_HDF5_PASS_RESTRAINT_KEYS);
      return c;
    } else {
      return nodes[s];
    }
  }


  void add_restraint_internal(Restraint *r,
                              NodeHandle parent,
                              IMP_HDF5_ACCEPT_RESTRAINT_KEYS) {
    NodeHandle cur= parent.add_child(r->get_name(), FEATURE);
    cur.set_association(r);
    //
    ParticlesTemp ip= r->get_input_particles();
    double s=r->evaluate(false);
    cur.set_value(sk, s, 0);

    Restraints rd= r->get_instant_decomposition();
    if (rd.size() >=1 && rd[0] != r) {
      Index index;
      for (unsigned int i=0; i< rd.size(); ++i) {
        //ScopedRestraint sr(rd[i], r->get_model()->get_root_restraint_set());
        rd[i]->set_was_used(true);
        NodeHandle rc=get_child(cur, &*rd[i], index,
                                IMP_HDF5_PASS_RESTRAINT_KEYS);
        double score = rd[i]->unprotected_evaluate(NULL);
        rc.set_value(sk, score, 0);
      }
    }
    set_particles(cur, ip, IMP_HDF5_PASS_RESTRAINT_KEYS);
  }
}

void add_restraint(RootHandle parent, Restraint *r) {
  IMP_FUNCTION_LOG;
  IMP_HDF5_CREATE_RESTRAINT_KEYS(parent);
  add_restraint_internal(r, parent, IMP_HDF5_PASS_RESTRAINT_KEYS);
}

namespace {

  void save_restraint_internal(Restraint *r,
                               RootHandle f,
                               int frame,
                               IMP_HDF5_ACCEPT_RESTRAINT_KEYS) {
    NodeHandle rn= f.get_node_handle_from_association(r);
    Index index;
    build_index(rn, index, IMP_HDF5_PASS_RESTRAINT_KEYS);
    double s=r->evaluate(false);
    rn.set_value(sk, s, frame);
    Restraints rd= r->get_instant_decomposition();
    if (rd.size() >1) {
      for (unsigned int i=0; i< rd.size(); ++i) {
        //ScopedRestraint sr(rd[i], r->get_model()->get_root_restraint_set());
        rd[i]->set_was_used(true);
        NodeHandle rc=get_child(rn, &*rd[i], index,
                                IMP_HDF5_PASS_RESTRAINT_KEYS);
        double score = rd[i]->unprotected_evaluate(NULL);
        rc.set_value(sk, score, frame);
      }
    }
  }
}

void save_frame(RootHandle f, int frame, Restraint *r) {
  IMP_FUNCTION_LOG;
  IMP_HDF5_CREATE_RESTRAINT_KEYS(f);
  save_restraint_internal(r, f, frame, IMP_HDF5_PASS_RESTRAINT_KEYS);
}

ParticlesTemp get_restraint_particles(NodeHandle f,
                                      int frame) {
  IMP_FUNCTION_LOG;
  IMP_HDF5_CREATE_RESTRAINT_KEYS(f);
  IMP_USAGE_CHECK(f.get_type()== FEATURE,
                  "Get restraint particles called on non-restraint node "
                  << f.get_name());
  RootHandle rh=f.get_root_handle();
  NodeIDs ids= f.get_value(nk, frame);
  ParticlesTemp ret(ids.size());
  for (unsigned int i=0; i< ids.size(); ++i) {
    NodeHandle nh= rh.get_node_handle_from_id(ids[i]);
    Particle *p= reinterpret_cast<Particle*>(nh.get_association());
    ret[i]=p;
  }
  return ret;
}



double get_restraint_score(NodeHandle f,
                           int frame) {
  IMP_FUNCTION_LOG;
  IMP_HDF5_CREATE_RESTRAINT_KEYS(f);
  if (f.get_has_value(sk, frame)) {
    return f.get_value(sk, frame);
  } else {
    return -std::numeric_limits<double>::infinity();
  }
}




IMPRMF_END_NAMESPACE
