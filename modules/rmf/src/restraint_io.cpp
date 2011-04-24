/**
 *  \file IMP/rmf/KeyCategory.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/rmf/restraint_io.h>
#include <IMP/rmf/operations.h>
#include <IMP/domino/Subset.h>
#include <IMP/scoped.h>
IMPRMF_BEGIN_NAMESPACE

namespace {
  typedef IMP::internal::Map<IMP::domino::Subset, NodeHandle> Index;
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
      IMP::domino::Subset s(pt);
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
    IMP::domino::Subset s(ip);
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
