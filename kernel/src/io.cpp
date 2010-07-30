/**
 *  \file file.cpp
 *  \brief Get directories used by IMP.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/io.h>
#include <IMP/internal/particle_save.h>
#ifdef IMP_USE_NETCDF
#include <netcdfcpp.h>
#endif

IMP_BEGIN_NAMESPACE

IMPEXPORT void write_model(Model *,
                           const ParticlesTemp &particles,
                           TextOutput out) {
  write_model(particles, out);
}

IMPEXPORT void write_model(const ParticlesTemp &particles,
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

IMPEXPORT void write_model(const ParticlesTemp &particles,
                           const FloatKeys &keys,
                           TextOutput out) {
  std::map<Particle*, unsigned int> to;
  for (unsigned int i=0; i< particles.size(); ++i) {
    to[particles[i]]=i;
  }
  std::map<unsigned int, internal::ParticleData> data;
  for (std::map<Particle*, unsigned int>::const_iterator it= to.begin();
       it != to.end(); ++it) {
    data[it->second]= internal::ParticleData(it->first, keys);
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
                          Model *) {
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


IMPEXPORT void read_model(TextInput in,
                          const ParticlesTemp &particles,
                          const FloatKeys &keys) {
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
    it->second.apply(it->first, keys);
  }
}


IMPEXPORT void read_model(TextInput in, Model *m) {
  ParticlesTemp ps(m->particles_begin(), m->particles_end());
  read_model(in, ps, m);
}

#ifdef IMP_USE_NETCDF

void write_binary_model(const ParticlesTemp &particles,
                        const FloatKeys &keys,
                        std::string filename,
                        int frame) {
  NcFile::FileMode mode;
  // replace on 0 also
  if (frame >0) {
    mode=NcFile::Write;
  } else {
    mode=NcFile::Replace;
  }
  NcFile f(filename.c_str(), mode, NULL, 0, NcFile::Netcdf4);
  if (!f.is_valid()) {
    IMP_THROW("Unable to open file " << filename << " for writing",
              IOException);
  }
  const NcDim* dims[2];
  if (frame > 0) {
    dims[0]= f.get_dim("particles");
    dims[1]= f.get_dim("values");
    if (static_cast<unsigned int>(dims[0]->size()) != particles.size()) {
      IMP_THROW("Number of particles (" << particles.size()
                << ") does not match expected (" << dims[0]->size()
                << ")", IOException);
    }
    if (static_cast<unsigned int>(dims[1]->size()) != keys.size()) {
      IMP_THROW("Number of keys (" << keys.size()
                << ") does not match expected (" << dims[1]->size()
                << ")", IOException);
    }
  } else {
    dims[0]= f.add_dim("particles", particles.size());
    dims[1]= f.add_dim("values", keys.size());
  }
  NcVar *cur=NULL;
  if (frame >=0) {
    std::ostringstream oss;
    oss << frame;
    cur = f.add_var(oss.str().c_str(), ncDouble, 2, dims);
  } else {
    cur = f.add_var("data", ncDouble, 2, dims);
  }
  boost::scoped_array<double> values(new double[particles.size()*keys.size()]);
  for (unsigned int i=0; i< particles.size(); ++i) {
    for (unsigned int j=0; j< keys.size(); ++j) {
      values[i*keys.size()+j]= particles[i]->get_value(keys[j]);
    }
  }
  cur->put(values.get(), particles.size(), keys.size());
}

void read_binary_model(NcFile &f,
                       const ParticlesTemp &particles,
                       const FloatKeys &keys,
                       int var_index) {
  if (var_index >= f.num_vars()) {
    IMP_THROW("Illegal component of file requested " << var_index
              << ">=" << f.num_vars(), IOException);
  }
  NcVar *data=f.get_var(var_index);
  boost::scoped_array<double> values(new double[particles.size()*keys.size()]);
  data->get(values.get(), particles.size(), keys.size());
  for (unsigned int i=0; i< particles.size(); ++i) {
    for (unsigned int j=0; j< keys.size(); ++j) {
      particles[i]->set_value(keys[j], values[i*keys.size()+j]);
    }
  }
}

void read_binary_model(std::string filename,
                       const ParticlesTemp &particles,
                       const FloatKeys &keys,
                       int frame) {
  NcFile f(filename.c_str(), NcFile::ReadOnly,
           NULL, 0, NcFile::Netcdf4);
  if (!f.is_valid()) {
    IMP_THROW("Unable to open file " << filename << " for reading",
              IOException);
  }
  int index=-1;
  if (frame>=0) {
    std::ostringstream oss;
    oss << frame;\
    for ( int i=0; i< f.num_vars(); ++i) {
      NcVar *v= f.get_var(i);
      if (std::string(v->name())== oss.str()) {
        index=i;
        break;
      }
    }
    if (index==-1) {
      std::string vars;
      for ( int i=0; i< f.num_vars(); ++i) {
        NcVar *v= f.get_var(i);
        vars= vars+" "+std::string(v->name());
      }
      IMP_THROW("Unable to find " << oss.str()
                << " found frames are " << vars, IOException);
    }
  } else {
    index=0;
  }
  read_binary_model(f, particles, keys, index);
}
#endif



DumpModelOnFailure
::DumpModelOnFailure(Model *m,
                     TextOutput out): m_(m),
                                      file_name_(out){}

void DumpModelOnFailure::handle_failure() {
  write_model(m_, file_name_);
}



void DumpModelOnFailure::do_show(std::ostream &) const {
}


IMP_END_NAMESPACE
