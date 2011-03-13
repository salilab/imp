/**
 *  \file IMP/hdf5/KeyCategory.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/hdf5/particle_io.h>
IMPHDF5_BEGIN_NAMESPACE


void write_particle(Particle* ps, RootHandle fh) {
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
}

ParticlesTemp read_all_particles(RootHandle fh, Model *m) {
  std::vector<NodeHandle> ch= fh.get_children();
  ParticlesTemp ret;
  for (unsigned int i=0; i< ch.size(); ++i) {
    NodeHandle cur= ch[i];
    if (ch[i].get_type()==CUSTOM) {
      bool has_data=false;
      IMP_NEW(Particle, p, (m));
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

IMPHDF5_END_NAMESPACE
