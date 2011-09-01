/**
 *  \file IMP/rmf/Category.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/rmf/particle_io.h>
#include <IMP/rmf/RootHandle.h>
#include <IMP/rmf/NodeHandle.h>
#include <IMP/rmf/internal/imp_operations.h>

IMPRMF_BEGIN_NAMESPACE
using namespace RMF;

void add_particle(RootHandle fh, Particle* ps) {
  NodeHandle n= fh.add_child(ps->get_name(), CUSTOM);
  Category IMP= fh.add_category("IMP");
  n.set_association(ps);
  {
    IMP::FloatKeys fks= ps->get_float_keys();
    for (unsigned int i=0; i< fks.size(); ++i) {
      bool mf=false;
      if (fks[i]== IMP::core::XYZ::get_xyz_keys()[0]
          || fks[i]== IMP::core::XYZ::get_xyz_keys()[1]
          || fks[i]== IMP::core::XYZ::get_xyz_keys()[2]) {
        mf=true;
      }
      RMF::FloatKey fk= internal::get_or_add_key<FloatTraits>(fh, IMP,
                                               fks[i].get_string(), mf);
      n.set_value(fk, ps->get_value(fks[i]));
    }
  }
  {
    IMP::IntKeys fks= ps->get_int_keys();
    for (unsigned int i=0; i< fks.size(); ++i) {
      RMF::IntKey fk
        = internal::get_or_add_key<IntTraits>(fh, IMP, fks[i].get_string(),
                                              false);
      n.set_value(fk, ps->get_value(fks[i]));
    }
  }
  {
    IMP::StringKeys fks= ps->get_string_keys();
    for (unsigned int i=0; i< fks.size(); ++i) {
      RMF::StringKey fk
        = internal::get_or_add_key<StringTraits>(fh, IMP,
                                                 fks[i].get_string(),
                                                 false);
      n.set_value(fk, ps->get_value(fks[i]));
    }
  }
  {
    IMP::ParticleKeys fks= ps->get_particle_keys();
    for (unsigned int i=0; i< fks.size(); ++i) {
      RMF::IndexKey fk
        = internal::get_or_add_key<IndexTraits>(fh, IMP, fks[i].get_string(),
                                                false);
      NodeHandle nh= fh.get_node_handle_from_association(ps->get_value(fks[i]));
      if (nh== NodeHandle()) {
        IMP_THROW("Particle " << ps->get_name()
                  << " references particle "
                  << ps->get_value(fks[i])->get_name()
                  << " which is not already part of the file.",
                  IOException);
      }
      n.set_value(fk, nh.get_id().get_index());
    }
  }
}

ParticlesTemp create_particles(RootHandle fh, Model *m) {
  NodeHandles ch= fh.get_children();
  Category IMP= fh.add_category("IMP");
  ParticlesTemp ret;
  for (unsigned int i=0; i< ch.size(); ++i) {
    NodeHandle cur= ch[i];
    if (ch[i].get_type()==CUSTOM) {
      bool has_data=false;
      IMP_NEW(Particle, p, (m));
      ch[i].set_association(p);
      {
        RMF::FloatKeys fks= fh.get_keys<FloatTraits>(IMP);
        for (unsigned int i=0; i< fks.size(); ++i) {
          if (cur.get_has_value(fks[i])) {
            has_data=true;
            p->add_attribute(IMP::FloatKey(fh.get_name(fks[i])),
                             cur.get_value(fks[i]), false);
          }
        }
      }
      {
        RMF::IntKeys fks= fh.get_keys<IntTraits>(IMP);
        for (unsigned int i=0; i< fks.size(); ++i) {
          if (cur.get_has_value(fks[i])) {
            has_data=true;
            p->add_attribute(IMP::IntKey(fh.get_name(fks[i])),
                         cur.get_value(fks[i]));
          }
        }
      }
      {
        RMF::StringKeys fks= fh.get_keys<StringTraits>(IMP);
        for (unsigned int i=0; i< fks.size(); ++i) {
          if (cur.get_has_value(fks[i])) {
            has_data=true;
            p->add_attribute(IMP::StringKey(fh.get_name(fks[i])),
                             cur.get_value(fks[i]));
          }
        }
      }
      {
        RMF::IndexKeys fks= fh.get_keys<IndexTraits>(IMP);
        for (unsigned int i=0; i< fks.size(); ++i) {
          if (cur.get_has_value(fks[i])) {
            has_data=true;
            NodeHandle nh
              = fh.get_node_handle_from_id(NodeID(cur.get_value(fks[i])));
            Particle *op= reinterpret_cast<Particle*>(nh.get_association());
            IMP_CHECK_OBJECT(op);
            p->add_attribute(IMP::ParticleKey(fh.get_name(fks[i])),
                             op);
          }
        }
      }
      if (has_data) {
        ret.push_back(p.release());
      } else {
        m->remove_particle(p);
      }
    }
  }
  return ret;
}
IMPRMF_END_NAMESPACE
