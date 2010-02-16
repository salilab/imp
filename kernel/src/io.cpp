/**
 *  \file file.cpp
 *  \brief Get directories used by IMP.
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 *
 */

#include <IMP/io.h>
#include <IMP/internal/particle_save.h>

IMP_BEGIN_NAMESPACE



IMPEXPORT void write_model(Model *m,
                           const ParticlesTemp &particles,
                           TextOutput out) {
  std::map<Particle*, unsigned int> to;
  for (unsigned int i=0; i< particles.size(); ++i) {
    to[particles[i]]=i;
  }
  std::map<unsigned int, internal::ParticleData> data;
  for (std::map<Particle*, unsigned int>::const_iterator it= to.begin();
       it != to.end(); ++it) {
    data[it->second]= internal::ParticleData(it->first);
  }
  for (std::map<unsigned int,
         internal::ParticleData>::const_iterator it= data.begin();
       it != data.end(); ++it) {
    out.get_stream() << "particle: " << it->first << std::endl;
    it->second.write_yaml(out, to);
  }
}
IMPEXPORT void write_model(Model *m,
                           TextOutput out) {
  ParticlesTemp ps(m->particles_begin(), m->particles_end());
  write_model(m, ps, out);
}
IMPEXPORT void read_model(TextInput in,
                          const ParticlesTemp &particles,
                          Model *m) {
  internal::LineStream ls(in);
  std::map<unsigned int, Particle *> from;
  for (unsigned int i=0; i< particles.size(); ++i) {
    from[i]= particles[i];
  }
  std::map<Particle*, internal::ParticleData> data;
  do {
    internal::LineStream::LinePair lp= ls.get_line();
    if (lp.first.empty()) break;
    if (lp.first== "particle") {
      std::istringstream iss(lp.second);
      int i=-1;
      iss >> i;
      if (i<0) {
        IMP_THROW("Error reading from line "
                  << lp.first << ": " << lp.second,
                  IOException);
      }
      if (from.find(i) == from.end()) {
        IMP_THROW("Can't find particle " << i << " in map",
                  IOException);
      }
      data[from.find(i)->second]=internal::ParticleData();
      ls.push_indent();
      data[from.find(i)->second].read_yaml(ls, from);
      ls.pop_indent();
    } else {
      IMP_LOG(TERSE,
              "Found " << lp.first << " when looking for particle"
              << std::endl);
    }
  } while (true);
  IMP_LOG(TERSE, "Read " << data.size() << " particles"
          << std::endl);
  if (data.size() != from.size()) {
    IMP_THROW("Did not read all particles. Expected "
              << from.size() << " got " << data.size(),
              IOException);
  }
  for (std::map<Particle*, internal::ParticleData>::const_iterator
         it= data.begin(); it != data.end(); ++it) {
    it->second.apply(it->first);
  }
}
IMPEXPORT void read_model(TextInput in, Model *m) {
  ParticlesTemp ps(m->particles_begin(), m->particles_end());
  read_model(in, ps, m);
}



DumpModelOnFailure
::DumpModelOnFailure(Model *m,
                     TextOutput out): m_(m),
                                      file_name_(out){}

void DumpModelOnFailure::handle_failure() {
  write_model(m_, file_name_);
}



void DumpModelOnFailure::do_show(std::ostream &out) const {
}


IMP_END_NAMESPACE
