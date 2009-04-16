/**
 *  \file model_io.cpp   \brief YAML io support.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#include "IMP/core/model_io.h"
#include <set>
#include <map>
#include <algorithm>

IMPCORE_BEGIN_NAMESPACE

namespace {
  struct DefaultWrite {
    template <class T>
    std::ostream & operator()(const T t,
                              std::ostream &out) const {
      return out << t;
    }
  };
  struct ParticleWrite {
    ParticleWrite(){}
    std::ostream & operator()(Particle *p,
                              std::ostream &out) const {
      return out << p;
    }
  };
  struct FloatWrite {
    std::ostream & operator()(Float t,
                              std::ostream &out) const {
      // set format better
      return out << t;
    }
  };

  template <class It, class Write>
  void write_attributes(std::string indent,
                        Particle *p,
                        It b, It e, Write w,
                        std::ostream &out) {
    for (It c= b; c != e; ++c) {
      /** \todo should escape things properly */
      out << indent << c->get_string() << ": ";
      w(p->get_value(*c), out) << "\n";
    }
  }

  // skips empty lines
  struct LineStream{
    std::istream &in;
    std::vector<std::string> line;
    bool has_indent(std::string str, unsigned int indent) {
      for (unsigned int i=0; i< indent; ++i) {
        if (i== str.size()) return false;
        if (str[i] != ' ') return false;
      }
      return true;
    }
    bool not_white(char buf[]) const {
      for (int i=0; buf[i] != '\0'; ++i) {
        if (buf[i] != ' ') return true;
      }
      return false;
    }

    typedef std::pair<std::string, std::string> LinePair;

    LineStream(std::istream &init): in(init){}
    operator bool() const {return !line.empty() || static_cast<bool>(in);}
    LinePair get_line(unsigned int min_indent) {
      while (line.empty()) {
        char buf[3000];
        in.getline(buf, 3000);
        if (!in) return LinePair();
        if (buf[0] == '#') continue;
        if (not_white(buf)) {
          line.push_back(buf);
        }
      }
      if (has_indent(line.back(), min_indent)) {
        std::string ret(line.back(), min_indent);
        line.pop_back();
        std::size_t pos= ret.find_first_of(":");
        if (pos == std::string::npos) {
          throw InvalidStateException("No colon in line");
        }
        IMP_LOG(VERBOSE, "Found colon at position " << pos
                << " of \"" << ret << "\"" <<std::endl);
        std::string key(ret, 0, pos), value;
        if (ret.size() > pos+2) {
          value= std::string(ret, pos+2);
        }
        return std::make_pair(key, value);
      } else {
        IMP_LOG(VERBOSE, "Line \"" << line.back() << "\" lacks "
                << min_indent << " spaces" << std::endl);
        return LinePair();
      }
    }
    void push_line(std::string s) {
      if (s.empty()) return;
      for (unsigned int i=0; i< s.size(); ++i) {
        if (s[i] != ' ') {
          line.push_back(s);
          return;
        }
      }
    }

    int get_next_indent() {
      std::string buf;
      if (line.empty()) {
        char cbuf[1000];
        in.getline(cbuf, 1000);
        buf= std::string(cbuf);
      } else {
        buf=line.back();
        line.pop_back();
      }
      if (buf.empty()) return 0;
      unsigned int i=0;
      for (; i < buf.size() && buf[i] == ' '; ++i) {
      }
      push_line(buf);
      return i;
    }
  };

  template <class K, class V>
  struct DefaultRead {
    void operator()(Particle *p, std::string key, std::string value) const {
      IMP_LOG(VERBOSE,
              "Reading values from pair " << key << " "
              << value << std::endl);
      K k(key.c_str());
      std::istringstream iss(value.c_str());
      V v;
      iss >> v;
      IMP_check(iss, "Error reading value. Got " << v , ValueException);
      p->set_value(k, v);
    }
  };

  struct ParticleRead {
    std::set<Particle*> unused_;
    std::map<std::string, Particle *> used_;
    ParticleRead(const Particles &particles):
    unused_(particles.begin(), particles.end()){}
    void add_particle(std::string nm, Particle *p) {
      if (unused_.find(p) != unused_.end()) {
        unused_.erase(p);
        used_[nm]= p;
      }
    }
    void operator()(Particle *p, std::string key, std::string value) {
      IMP_LOG(VERBOSE,
              "Reading values from pair " << key << " "
              << value << std::endl);
      ParticleKey k(key.c_str());
      if (used_.find(value) != used_.end()) {
      } else {
        used_[value]=*unused_.begin();
        unused_.erase(unused_.begin());
      }
      Particle *op= used_[value];
      p->set_value(k, op);
    }
  };

  template <class Read>
  void read_attributes(Particle *p, LineStream &in,
                       int indent,
                       Read read) {
    IMP_LOG(VERBOSE, "Reading attributes " << indent << std::endl);
    int nindent= in.get_next_indent();
    if (nindent <= indent) return;
    indent=nindent;
    IMP_LOG(VERBOSE, "Required indent is " << indent<< std::endl);
    do {
      LineStream::LinePair lp = in.get_line(indent);
      if (lp.first.empty()) {
        IMP_LOG(VERBOSE, "Done reading attributes" << std::endl);
        return;
      }
      read(p, lp.first, lp.second);

    } while (true);
  }

  void read(Model *m, Particle *p, ParticleRead &pr,
            LineStream &in,
            unsigned int indent) {
    LineStream::LinePair lp=in.get_line(indent);
    if (lp.first.empty()) return;
    //IMP_LOG(VERBOSE, "Got line " << buf << std::endl);
    //IMP_check(in, "Error reading particle line from yaml", ValueException);
    IMP_check(lp.first== "particle", "Error reading particle line: \""
              << lp.first << "\" got " << lp.first, InvalidStateException);
    IMP_check(!lp.second.empty(), "Couldn't read id", InvalidStateException);
    IMP_LOG(VERBOSE, "Reading particle " << lp.second << std::endl);
    pr.add_particle(lp.second, p);
    unsigned int nindent= in.get_next_indent();
    if (nindent <= indent) return;
    indent=nindent;
    while (in) {
      LineStream::LinePair lp=in.get_line(indent);
      if (lp.first.empty()) break;

      IMP_LOG(VERBOSE, "Looking for attributes in line " << lp.first << ": "
              << lp.second << std::endl);
      if (lp.first.compare("name")==0) {
        p->set_name(lp.second);
      } else if (lp.first.compare("float-attributes")==0) {
        read_attributes(p, in, indent, DefaultRead<FloatKey, Float>());
      } else if (lp.first.compare("int-attributes")==0) {
        read_attributes(p, in, indent, DefaultRead<IntKey, Int>());
      } else if (lp.first.compare("string-attributes")==0) {
        read_attributes(p, in, indent, DefaultRead<StringKey, String>());
      } else if (lp.first.compare("particle-attributes")==0) {
        read_attributes(p, in, indent, pr);
      } else {
        break;
      }
    }
    IMP_LOG(VERBOSE, "Done reading particle " << lp.second << std::endl);
  }
}

static std::string indent_level="  ";

void write(Particle *p,
           std::ostream &out,
           std::string indent) {
  std::ostringstream oss;
  oss<< p;
  out << indent << "particle: " << oss.str() << "\n";
  out << indent << indent_level << "name: " << p->get_name() << "\n";
  out << indent << indent_level << "float-attributes:\n";
  write_attributes(indent+indent_level+"  ",
                   p,
                   p->float_keys_begin(),
                   p->float_keys_end(),
                   FloatWrite(),
                   out);
  out << indent << indent_level << "int-attributes:\n";
  write_attributes(indent+indent_level+"  ",
                   p,
                   p->int_keys_begin(),
                   p->int_keys_end(),
                   DefaultWrite(),
                   out);
  out << indent << indent_level << "string-attributes:\n";
  write_attributes(indent+indent_level+"  ",
                   p,
                   p->string_keys_begin(),
                   p->string_keys_end(),
                   DefaultWrite(),
                   out);
  out << indent << indent_level << "particle-attributes:\n";
  write_attributes(indent+indent_level+"  ",
                   p,
                   p->particle_keys_begin(),
                   p->particle_keys_end(),
                   ParticleWrite(),
                   out);
}

void write(Model *m,
           std::ostream &out,
           std::string indent) {
  for (Model::ParticleIterator pit= m->particles_begin();
       pit != m->particles_end(); ++pit) {
    write(*pit, out, indent);
  }
}

void read(std::istream &in,
          Model *m) {
  LineStream r(in);
  Particles ps(m->particles_begin(), m->particles_end());
  std::reverse(ps.begin(), ps.end());
  ParticleRead pr(ps);
  Model::ParticleIterator pit= m->particles_begin();
  do {
    read(m, *pit, pr, r, r.get_next_indent());
    ++pit;
  } while (r);
  IMP_check(pit== m->particles_end(),
            "Read wrong number of particles. Model is corrupted. Bye.",
            ErrorException);
}

IMPCORE_END_NAMESPACE
