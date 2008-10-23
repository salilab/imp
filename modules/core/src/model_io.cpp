/**
 *  \file model_io.cpp   \brief YAML io support.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#include "IMP/core/model_io.h"

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
    std::ostream & operator()(Particle *p,
                              std::ostream &out) const {
      return out << p->get_index().get_index();
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

    LineStream(std::istream &init): in(init){}
    operator bool() const {return !line.empty() || static_cast<bool>(in);}
    std::string get_line(unsigned int min_indent) {
      while (line.empty()) {
        char buf[3000];
        in.getline(buf, 3000);
        if (!in) return std::string();
        if (buf[0] == '#') continue;
        if (not_white(buf)) {
          line.push_back(buf);
        }
      }
      if (has_indent(line.back(), min_indent)) {
        std::string ret(line.back(), min_indent);
        line.pop_back();
        return ret;
      } else {
        IMP_LOG(VERBOSE, "Line \"" << line.back() << "\" lacks "
                << min_indent << " spaces" << std::endl);
        return std::string();
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
    void operator()(Particle *p, std::string key, std::string value) const {
      IMP_LOG(VERBOSE,
              "Reading values from pair " << key << " "
              << value << std::endl);
      ParticleKey k(key.c_str());
      std::istringstream iss(value.c_str());
      int i;
      iss >> i;
      IMP_check(iss, "Error reading value" , ValueException);
      Particle *op= p->get_model()->get_particle(ParticleIndex(i));
      p->set_value(k, op);
    }
  };

  int get_next_indent(LineStream &in) {
    std::string buf= in.get_line(0);
    if (buf.empty()) return 0;
    unsigned int i=0;
    for (; i < buf.size() && buf[i] == ' '; ++i) {
    }
    in.push_line(buf);
    return i;
  }


  template <class Read>
  void read_attributes(Particle *p, LineStream &in,
                       int indent,
                       Read read) {
    IMP_LOG(VERBOSE, "Reading attributes " << indent << std::endl);
    int nindent= get_next_indent(in);
    if (nindent <= indent) return;
    indent=nindent;
    IMP_LOG(VERBOSE, "Required indent is " << indent<< std::endl);
    do {
      std::string buf = in.get_line(indent);
      if (buf.empty()) {
        IMP_LOG(VERBOSE, "Done reading attributes" << std::endl);
        return;
      }
      IMP_check(buf[0] != ' ', "Extra white space on line "
                << buf, InvalidStateException);
      std::istringstream iss(buf.c_str());
      char key[2000];
      iss.get(key, 2000, ':');
      IMP_check(iss, "no : found in line " << buf,
                ValueException);
      char colon;
      iss >> colon;
      IMP_check(colon == ':', "No colon found" << buf,
                ValueException);

      char value[2000];
      iss.getline(value, 2000);
      IMP_check(iss, "Error reading line " << buf,
                ValueException);
      read(p, key, value);

    } while (true);
  }

  void read(Model *m,
                 LineStream &in,
                 unsigned int indent) {
    std::string buf=in.get_line(indent);
    if (buf.empty()) return;
    //IMP_LOG(VERBOSE, "Got line " << buf << std::endl);
    //IMP_check(in, "Error reading particle line from yaml", ValueException);
    int id;
    int nread=sscanf(buf.c_str(), "particle: %d", &id);
    IMP_check(nread==1, "Couldn't read id", InvalidStateException);
    Particle *p= m->get_particle(id);
    IMP_LOG(VERBOSE, "Reading particle " << id << std::endl);
    unsigned int nindent= get_next_indent(in);
    if (nindent <= indent) return;
    indent=nindent;
    while (in) {
      std::string buf=in.get_line(indent);
      if (buf.empty()) break;
      IMP_check(buf[0] != ' ', "Indent error" << buf, InvalidStateException);

      IMP_LOG(VERBOSE, "Looking for attributes in line " << buf << std::endl);
      std::istringstream iss(buf);
      std::string type;
      iss >> type;
      if (type.compare("float-attributes:")==0) {
        read_attributes(p, in, indent, DefaultRead<FloatKey, Float>());
      } else if (type.compare("int-attributes:")==0) {
        read_attributes(p, in, indent, DefaultRead<IntKey, Int>());
      } else if (type.compare("string-attributes:")==0) {
        read_attributes(p, in, indent, DefaultRead<StringKey, String>());
      } else if (type.compare("particle-attributes:")==0) {
        read_attributes(p, in, indent, ParticleRead());
      } else {
        break;
      }
    }
    IMP_LOG(VERBOSE, "Done reading particle " << id << std::endl);
  }
}

static std::string indent_level="  ";

void write(Particle *p,
           std::ostream &out,
           std::string indent) {
  out << indent << "particle: " << p->get_index().get_index() << "\n";
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

void write(Model *m, std::string out) {
  std::ofstream outf(out.c_str());
  IMP_check(outf, "Invalid file name " << out, ValueException);
  write(m, outf, "");
}

std::string write(Model *m) {
  std::ostringstream outf;
  write(m, outf, "");
  return outf.str();
}

void read(std::istream &in,
               Model *m) {
  LineStream r(in);
  unsigned int nread=0;
  do {
    read(m, r, get_next_indent(r));
    ++nread;
  } while (r);
  IMP_check(nread== m->get_number_of_particles(),
            "Read wrong number of particles. Model is corrupted. Bye.",
            ErrorException);
}

void read(std::string in,
               Model *m) {
  std::ifstream iss(in.c_str());
  IMP_check(iss, "Invalid file name " << in, ValueException);
  read(iss, m);
}

void read_from_string(std::string in,
                           Model *m) {
  std::istringstream iss(in);
  read(iss, m);
}

IMPCORE_END_NAMESPACE
