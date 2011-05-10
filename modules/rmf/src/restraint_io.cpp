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
    Subset(ParticlesTemp ps):
      ps_(new Particle*[ps.size()]),
      sz_(ps.size()) {
      std::sort(ps.begin(), ps.end());
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

  IMP_VALUES(Subset, Subsets);

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

  inline std::size_t hash_value(const Subset &t) {
    return t.__hash__();
  }


  typedef IMP::internal::Map<Subset, NodeHandle> Index;
  void build_index(NodeHandle parent,
                   NodeIDKeys &fks,
                   Index &nodes) {
    NodeHandles children= parent.get_children();
    for (unsigned int i=0; i< children.size(); ++i) {
      ParticlesTemp pt;
      unsigned int j=0;
      while (true) {
        if (fks.size() <= j) {
          std::ostringstream oss;
          oss << "representation" << j;
          fks.push_back(get_or_add_key<NodeIDTraits>(parent.get_root_handle(),
                                                     Feature, oss.str(),
                                                     false));
        }
        if (children[i].get_has_value(fks[j])) {
          NodeID id= children[i].get_value(fks[j]);
          NodeHandle nh= parent.get_root_handle().get_node_handle_from_id(id);
          Particle *p= reinterpret_cast<Particle*>(nh.get_association());
          pt.push_back(p);
          ++j;
        } else {
          break;
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
                     NodeIDKeys &fks,
                     const ParticlesTemp& ps) {
    for (unsigned int i=0; i< ps.size(); ++i) {
      if (fks.size() <= i) {
        std::ostringstream oss;
        oss << "representation" << i;
        fks.push_back(get_or_add_key<NodeIDTraits>(nh.get_root_handle(),
                                                   Feature, oss.str(), false));
      }
      NodeID id
        =nh.get_root_handle().get_node_handle_from_association(ps[i]).get_id();
      nh.set_value(fks[i], id);
    }
  }

  NodeHandle get_child(NodeHandle parent,
                       NodeIDKeys &fks,
                       Restraint *r,
                       Index &nodes) {
    ParticlesTemp ip= r->get_input_particles();
    Subset s(ip);
    if (nodes.find(s) == nodes.end()) {
      NodeHandle c= parent.add_child(s.get_name(), FEATURE);
      /*std::cout << "Created node for " << s
        << " under " << parent.get_name() << std::endl;*/
      nodes[s]=c;
      c.set_association(r);
      set_particles(c, fks, ip);
      return c;
    } else {
      return nodes[s];
    }
  }


  void add_restraint_internal(Restraint *r,
                              NodeIDKeys &fks,
                              FloatKey sk,
                              NodeHandle parent) {
    NodeHandle cur= parent.add_child(r->get_name(), FEATURE);
    cur.set_association(r);
    //
    ParticlesTemp ip= r->get_input_particles();
    double s=r->evaluate(false);
    cur.set_value(sk, s, 0);

    Restraints rd= r->get_instant_decomposition();
    if (rd.size() >1) {
      Index index;
      for (unsigned int i=0; i< rd.size(); ++i) {
        ScopedRestraint sr(rd[i], r->get_model()->get_root_restraint_set());
        rd[i]->set_was_used(true);
        NodeHandle rc=get_child(cur, fks, &*sr, index);
        double score = sr->evaluate(false);
        rc.set_value(sk, score, 0);
      }
    } else {
      set_particles(cur, fks, ip);
    }
  }
}

void add_restraint(RootHandle parent, Restraint *r) {
  NodeIDKeys fks;
  FloatKey sk= get_or_add_key<FloatTraits>(parent.get_root_handle(),
                                           Feature, "score", true);
  add_restraint_internal(r, fks, sk, parent);
}

namespace {

  void save_restraint_internal(Restraint *r,
                               RootHandle f,
                               NodeIDKeys &fks,
                               FloatKey sk,
                               int frame) {
    NodeHandle rn= f.get_node_handle_from_association(r);
    Index index;
    build_index(rn, fks, index);
    double s=r->evaluate(false);
    rn.set_value(sk, s, frame);
    Restraints rd= r->get_instant_decomposition();
    if (rd.size() >1) {
      for (unsigned int i=0; i< rd.size(); ++i) {
        ScopedRestraint sr(rd[i], r->get_model()->get_root_restraint_set());
        rd[i]->set_was_used(true);
        NodeHandle rc=get_child(rn, fks, &*sr, index);
        double score = sr->evaluate(false);
        rc.set_value(sk, score, frame);
      }
    }
  }
}

void save_frame(RootHandle f, int frame, Restraint *r) {
  NodeIDKeys fks;
  FloatKey sk= get_or_add_key<FloatTraits>(f,
                                           Feature, "score", true);

  save_restraint_internal(r, f, fks, sk, frame);
}

ParticlesTemp get_restraint_particles(NodeHandle f,
                                      NodeIDKeys &ks,
                                      int frame) {
  IMP_USAGE_CHECK(f.get_type()== FEATURE,
                  "Get restraint particles called on non-restraint node "
                  << f.get_name());
  RootHandle rh=f.get_root_handle();
  if (ks.empty()) {
    do {
      std::ostringstream oss;
      oss << "representation" << ks.size();
      if (rh.get_has_key<NodeIDTraits>(Feature, oss.str())) {
        ks.push_back(rh.get_key<NodeIDTraits>(Feature, oss.str()));
      } else {
        break;
      }
    } while (true);
  }
  ParticlesTemp ret;
  for (unsigned int i=0; i< ks.size(); ++i) {
    if (!f.get_has_value(ks[i], frame)) {
      break;
    } else {
      NodeID id= f.get_value(ks[i], frame);
      NodeHandle nh= rh.get_node_handle_from_id(id);
      Particle *p= reinterpret_cast<Particle*>(nh.get_association());
      ret.push_back(p);
    }
  }
  return ret;
}


ParticlesTemp get_restraint_particles(NodeHandle f,
                                      int frame) {
  NodeIDKeys t;
  return get_restraint_particles(f, t, frame);
}

double get_restraint_score(NodeHandle f,
                           FloatKey &fk,
                           int frame) {
  if (fk== FloatKey()) {
    fk= get_or_add_key<FloatTraits>(f.get_root_handle(),
                                    Feature, "score", true);
  }
  if (f.get_has_value(fk, frame)) {
    return f.get_value(fk, frame);
  } else {
    return -std::numeric_limits<double>::infinity();
  }
}


double get_restraint_score(NodeHandle f,
                           int frame) {
  FloatKey sk;
  return get_restraint_score(f, sk, frame);
}


IMPRMF_END_NAMESPACE
