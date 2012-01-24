/**
 *  \file deprecation.cpp   \brief classes for deprecation.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/internal/pdb.h"
#include "IMP/internal/AttributeTable.h"
#include <iostream>
#include <fstream>
#include <sstream>


IMP_BEGIN_INTERNAL_NAMESPACE

ParticlesTemp create_particles_from_pdb(base::TextInput in,
                                        Model *m) {
  ParticlesTemp ret;
  do {
    char buf[1000];
    in.get_stream().getline(buf, 1000);
    if (!in) break;
    std::istringstream iss(buf+31);
    double x,y,z;
    iss >> x >> y >> z;
    IMP_NEW(Particle, p, (m));
    m->add_attribute(xyzr_keys[0], p->get_index(), 0);
    m->add_attribute(xyzr_keys[1], p->get_index(), 0);
    m->add_attribute(xyzr_keys[2], p->get_index(), 0);
    m->add_attribute(xyzr_keys[3], p->get_index(), 0);
    m->get_sphere(p->get_index())
        = algebra::Sphere3D(algebra::Vector3D(x,y,z),2);
    ret.push_back(p);
  } while (true);
  return ret;
}

IMP_END_INTERNAL_NAMESPACE
