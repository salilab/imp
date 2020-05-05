/**
 *  \file file.cpp
 *  \brief Get directories used by IMP.
 *
 *  Copyright 2007-2020 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/io.h>
#include <boost/unordered_map.hpp>
#include <IMP/Particle.h>
#include <IMP/Model.h>
#include <IMP/Decorator.h>
#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/stream.hpp>

IMPKERNEL_BEGIN_NAMESPACE

namespace {
void write_particles_to_buffer(const ParticlesTemp &particles,
                               const FloatKeys &keys, char *buf,
                               unsigned int size) {
  IMP_USAGE_CHECK(
      size >= particles.size() * keys.size() * sizeof(double),
      "Not enough space: " << size << " vs "
                           << particles.size() * keys.size() * sizeof(double));
  boost::iostreams::stream<boost::iostreams::array_sink> in(buf, size);
  for (unsigned int i = 0; i < particles.size(); ++i) {
    for (unsigned int j = 0; j < keys.size(); ++j) {
      double value = 0;
      if (particles[i]->has_attribute(keys[j])) {
        value = particles[i]->get_value(keys[j]);
      }
      in.write(reinterpret_cast<char *>(&value), sizeof(double));
      if (!in) {
        IMP_THROW("Error reading writing to buffer", IOException);
      }
    }
  }
}
void read_particles_from_buffer(const char *buffer, unsigned int size,
                                const ParticlesTemp &particles,
                                const FloatKeys &keys) {
  IMP_USAGE_CHECK(size == particles.size() * keys.size() * sizeof(double),
                  "Not enough data to read: "
                      << size << " vs "
                      << particles.size() * keys.size() * sizeof(double));
  boost::iostreams::stream<boost::iostreams::array_source> in(buffer, size);
  for (unsigned int i = 0; i < particles.size(); ++i) {
    for (unsigned int j = 0; j < keys.size(); ++j) {
      double value;
      in.read(reinterpret_cast<char *>(&value), sizeof(double));
      if (!in) {
        IMP_THROW("Error reading from buffer", IOException);
      }
      if (particles[i]->has_attribute(keys[j])) {
        particles[i]->set_value(keys[j], value);
      }
    }
  }
}
}

Vector<char> write_particles_to_buffer(const ParticlesTemp &particles,
                                             const FloatKeys &keys) {
  if (particles.empty() || keys.empty()) {
    return Vector<char>();
  }
  unsigned int size = particles.size() * keys.size() * sizeof(double);
  Vector<char> ret(size);
  write_particles_to_buffer(particles, keys, &ret.front(), size);
  return ret;
}
void read_particles_from_buffer(const Vector<char> &buffer,
                                const ParticlesTemp &particles,
                                const FloatKeys &keys) {
  if (particles.empty() || keys.empty()) {
    return;
  }
  read_particles_from_buffer(&buffer.front(), buffer.size() * sizeof(double),
                             particles, keys);
  Model *m = particles[0]->get_model();
  ParticleIndexes pis = m->get_particle_indexes();
  for (ParticleIndexes::iterator pi = pis.begin(); pi != pis.end();
       ++pi) {
    IMP::check_particle(m, *pi);
  }
}

IMPKERNEL_END_NAMESPACE
