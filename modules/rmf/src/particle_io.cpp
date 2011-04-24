/**
 *  \file IMP/rmf/KeyCategory.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/rmf/particle_io.h>
#include <IMP/rmf/operations.h>
#include <IMP/rmf/RootHandle.h>
#include <IMP/rmf/NodeHandle.h>

IMPRMF_BEGIN_NAMESPACE


void add_particle(RootHandle fh, Particle* ps) {
  NodeHandle n= fh.add_child(ps->get_name(), CUSTOM);
  n.set_association(ps);
  for (Particle::FloatKeyIterator it= ps->float_keys_begin();
       it != ps->float_keys_end(); ++it) {
    bool mf=false;
    if (*it== IMP::core::XYZ::get_xyz_keys()[0]
        || *it== IMP::core::XYZ::get_xyz_keys()[1]
        || *it== IMP::core::XYZ::get_xyz_keys()[2]) {
      mf=true;
    }
    FloatKey fk= get_or_add_key<FloatTraits>(fh, IMP, it->get_string(), mf);
    n.set_value(fk, ps->get_value(*it));
  }
  for (Particle::IntKeyIterator it= ps->int_keys_begin();
       it != ps->int_keys_end(); ++it) {
    IntKey fk= get_or_add_key<IntTraits>(fh, IMP, it->get_string(), false);
    n.set_value(fk, ps->get_value(*it));
  }
  for (Particle::StringKeyIterator it= ps->string_keys_begin();
       it != ps->string_keys_end(); ++it) {
    StringKey fk= get_or_add_key<StringTraits>(fh, IMP, it->get_string(),
                                               false);
    n.set_value(fk, ps->get_value(*it));
  }
  for (Particle::ParticleKeyIterator it= ps->particle_keys_begin();
       it != ps->particle_keys_end(); ++it) {
    IndexKey fk= get_or_add_key<IndexTraits>(fh, IMP, it->get_string(),
                                               false);
    NodeHandle nh= fh.get_node_handle_from_association(ps->get_value(*it));
    if (nh== NodeHandle()) {
      IMP_THROW("Particle " << ps->get_name()
                << " references particle " << ps->get_value(*it)->get_name()
                << " which is not already part of the file.",
                IOException);
    }
    n.set_value(fk, nh.get_id().get_index());
  }
}

ParticlesTemp create_particles(RootHandle fh, Model *m) {
  std::vector<NodeHandle> ch= fh.get_children();
  ParticlesTemp ret;
  for (unsigned int i=0; i< ch.size(); ++i) {
    NodeHandle cur= ch[i];
    if (ch[i].get_type()==CUSTOM) {
      bool has_data=false;
      IMP_NEW(Particle, p, (m));
      ch[i].set_association(p);
      {
        std::vector<FloatKey> fks= fh.get_keys<FloatTraits>(IMP);
        for (unsigned int i=0; i< fks.size(); ++i) {
          if (cur.get_has_value(fks[i])) {
            has_data=true;
            p->add_attribute(IMP::FloatKey(fh.get_name(fks[i])),
                             cur.get_value(fks[i]), false);
          }
        }
      }
      {
        std::vector<IntKey> fks= fh.get_keys<IntTraits>(IMP);
        for (unsigned int i=0; i< fks.size(); ++i) {
          if (cur.get_has_value(fks[i])) {
            has_data=true;
            p->add_attribute(IMP::IntKey(fh.get_name(fks[i])),
                         cur.get_value(fks[i]));
          }
        }
      }
      {
        std::vector<StringKey> fks= fh.get_keys<StringTraits>(IMP);
        for (unsigned int i=0; i< fks.size(); ++i) {
          if (cur.get_has_value(fks[i])) {
            has_data=true;
            p->add_attribute(IMP::StringKey(fh.get_name(fks[i])),
                             cur.get_value(fks[i]));
          }
        }
      }
      {
        std::vector<IndexKey> fks= fh.get_keys<IndexTraits>(IMP);
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

const KeyCategory IMP=KeyCategory::add_key_category("IMP");

IMPRMF_END_NAMESPACE
