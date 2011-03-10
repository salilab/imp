/**
 *  \file IMP/hdf5/KeyCategory.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/hdf5/restraint_io.h>
#include <IMP/hdf5/operations.h>
#include <IMP/scoped.h>
IMPHDF5_BEGIN_NAMESPACE

namespace {
  void add_restraint_internal(Restraint *r,
                              NodeHandle parent) {
    NodeHandle cur= parent.add_child(r->get_name(), FEATURE);
    cur.set_association(r);
    //
    ParticlesTemp ip= r->get_input_particles();
    FloatKey sk= get_or_add_key<FloatTraits>(parent.get_root_handle(),
                                             Feature, "score", true);
    double s=r->evaluate(false);
    cur.set_value(sk, s, 0);

    for (unsigned int i=0; i< ip.size(); ++i) {
      std::ostringstream oss;
      oss << "representation" << i;
      NodeIDKey ik= get_or_add_key<NodeIDTraits>(parent.get_root_handle(),
                                                Feature, oss.str(), false);
      NodeID id= parent.get_root_handle()
        .get_node_handle_from_association(ip[i])
        .get_id();
      cur.set_value(ik, id);
    }
    Restraints rd= r->get_instant_decomposition();
    if (rd.size() >1) {
      for (unsigned int i=0; i< rd.size(); ++i) {
        ScopedRestraint sr(rd[i], r->get_model()->get_root_restraint_set());
        add_restraint_internal(rd[i], cur);
      }
    }
  }
}

void write_restraint(Restraint *r,
                   RootHandle parent) {
  add_restraint_internal(r, parent);
}

namespace {
  void save_restraint_internal(Restraint *r,
                               RootHandle f,
                               int frame) {
    NodeHandle rn= f.get_node_handle_from_association(r);
    FloatKey sk= get_or_add_key<FloatTraits>(f,
                                             Feature, "score", true);
    double s=r->evaluate(false);
    rn.set_value(sk, s, frame);
    Restraints rd= r->get_instant_decomposition();
    if (rd.size() >1) {
      for (unsigned int i=0; i< rd.size(); ++i) {
        ScopedRestraint sr(rd[i], r->get_model()->get_root_restraint_set());
        save_restraint_internal(rd[i], f, frame);
      }
    }
  }
}

void save_restraint(Restraint *r,
                    RootHandle f, int frame) {
  save_restraint_internal(r, f, frame);
}



IMPHDF5_END_NAMESPACE
