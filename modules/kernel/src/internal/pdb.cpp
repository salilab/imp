/**
 *  \file deprecation.cpp   \brief classes for deprecation.
 *
 *  Copyright 2007-2018 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/internal/pdb.h"
#include "IMP/Particle.h"
#include "IMP/Model.h"
#include <iostream>
#include <fstream>
#include <sstream>

IMPKERNEL_BEGIN_INTERNAL_NAMESPACE

ParticleIndexes create_particles_from_pdb(TextInput in, Model *m) {
  ParticleIndexes ret;
  do {
    char buf[1000];
    in.get_stream().getline(buf, 1000);
    if (!in) break;
    if (!(buf[0] == 'A' && buf[1] == 'T' && buf[2] == 'O' && buf[3] == 'M')) {
      continue;
    }
    std::istringstream iss(buf + 31);
    double x, y, z;
    iss >> x >> y >> z;
    ParticleIndex p = m->add_particle("atom");
    m->add_attribute(xyzr_keys[0], p, 0);
    m->add_attribute(xyzr_keys[1], p, 0);
    m->add_attribute(xyzr_keys[2], p, 0);
    m->add_attribute(xyzr_keys[3], p, 0);
    m->get_sphere(p) = algebra::Sphere3D(algebra::Vector3D(x, y, z), 2);
    ret.push_back(p);
  } while (true);
  return ret;
}

IMPKERNEL_END_INTERNAL_NAMESPACE
