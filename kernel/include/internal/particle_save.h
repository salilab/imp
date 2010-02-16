/**
 *  \file internal/particle_save.h
 *  \brief Various useful utilities
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 */

#ifndef IMP_INTERNAL_PARTICLE_SAVE_H
#define IMP_INTERNAL_PARTICLE_SAVE_H

#include "../Particle.h"
#include <iostream>
#include <vector>


IMP_BEGIN_INTERNAL_NAMESPACE


// skips empty lines
class LineStream{
  std::istream &in_;
  std::vector<int> indents_;
  std::vector<std::string> lines_;
  bool has_indent(std::string str, unsigned int indent) {
    for (unsigned int i=0; i< indent; ++i) {
      if (i== str.size()) return false;
      if (str[i] != ' ') return false;
    }
    if (str[indent] ==' ') {
      IMP_THROW("Excessive indent (not " << indent
                << ") on line " << str,
                IOException);
    }
    return true;
  }
  bool not_white(char buf[]) const {
    for (int i=0; buf[i] != '\0'; ++i) {
      if (buf[i] != ' ') return true;
    }
    return false;
  }
  int get_indent(std::string str) {
    for (unsigned int i=0; i< str.size(); ++i) {
      if (str[i] != ' ') return i;
    }
    return -1;
  }
  void fill_buffer() {
    while (lines_.empty()) {
      char buf[3000];
      in_.getline(buf, 3000);
      if (!in_) return;
      if (buf[0] == '#') continue;
      if (not_white(buf)) {
        IMP_LOG(VERBOSE, "Found line " << buf << std::endl);
        lines_.push_back(buf);
      }
    }
  }
public:
  typedef std::pair<std::string, std::string> LinePair;
  LineStream(std::istream &init): in_(init), indents_(1,0){}
  void pop_indent() {
    IMP_INTERNAL_CHECK(indents_.size() >1,
                           "Too many pops");
    indents_.pop_back();
    IMP_LOG(VERBOSE, "Indent is now " << indents_.back() << std::endl);
  }
  void push_indent() {
    fill_buffer();
    if (!lines_.empty()) {
      int id=(get_indent(lines_.back()));
      if (id == indents_.back()) ++id;
      indents_.push_back(std::max(id, indents_.back()+1));
    }
  }
  LinePair get_line() {
    fill_buffer();
    if (lines_.empty()) {
      IMP_LOG(VERBOSE, "Hit end of line buffer"<<std::endl);
      return LinePair();
    }
    if (!has_indent(lines_.back(), indents_.back())) {
      IMP_LOG(VERBOSE, "Line does not have indent of "
              << indents_.back() << ": " << lines_.back() << std::endl);
      return LinePair();
    }
    std::string ret(lines_.back(), indents_.back());
    lines_.pop_back();
    std::size_t pos= ret.find_first_of(":");
    if (pos == std::string::npos) {
      IMP_THROW("No colon in line", IOException);
    }
    std::string key(ret, 0, pos), value;
    if (ret.size() > pos+2) {
      value= std::string(ret, pos+2);
    }
    return std::make_pair(key, value);
  }
};

class IMPEXPORT ParticleData {
  friend class ParticleDiff;
  typedef ArrayStorage<FloatAttributeTableTraits> FloatTable;
  typedef ArrayStorage<internal::BoolAttributeTableTraits>
    OptimizedTable;
  typedef ArrayStorage<IntAttributeTableTraits> IntTable;
  typedef ArrayStorage<StringAttributeTableTraits> StringTable;
  typedef RefCountedStorage<ParticlesAttributeTableTraits> ParticleTable;
  typedef RefCountedStorage<ObjectsAttributeTableTraits> ObjectTable;
  std::string name_;
  FloatTable floats_;
  OptimizedTable optimizeds_;
  IntTable ints_;
  StringTable strings_;
  ParticleTable particles_;
  ObjectTable objects_;
 public:
  ParticleData(){}
  //! Store the data from the particle
  ParticleData(Particle *p);
  //! overwrite the particle
  void apply(Particle *p) const;
  void show(std::ostream &out=std::cout) const;
  void write_yaml(std::ostream &out,
                  const std::map<Particle*,
                  unsigned int> &particles) const;
  void read_yaml(LineStream &in,
                 const std::map<unsigned int, Particle*> &particles);
};
IMP_OUTPUT_OPERATOR(ParticleData);

class IMPEXPORT ParticleDiff {
  typedef std::pair<FloatKey, Float> FloatPair;
  typedef std::pair<IntKey, Int> IntPair;
  typedef std::pair<StringKey, String> StringPair;
  typedef std::pair<ParticleKey, Pointer<Particle> > ParticlePair;
  typedef std::pair<ObjectKey, Pointer<Object> > ObjectPair;
  std::vector<FloatPair> floats_a_;
  std::vector<FloatKey> floats_r_;
  std::vector<FloatKey> optimizeds_a_, optimizeds_r_;
  std::vector<IntPair> ints_a_;
  std::vector<IntKey> ints_r_;
  std::vector<StringPair> strings_a_;
  std::vector<StringKey> strings_r_;
  std::vector<ParticlePair> particles_a_;
  std::vector<ParticleKey> particles_r_;
  std::vector<ObjectPair> objects_a_;
  std::vector<ObjectKey> objects_r_;
 public:
  ParticleDiff(){}
  //! Compute the diff between the two
  ParticleDiff(const ParticleData &base, Particle *p);
  void apply(Particle *p) const;
  void show(std::ostream &out=std::cout) const;
};
IMP_OUTPUT_OPERATOR(ParticleDiff);


IMP_END_INTERNAL_NAMESPACE

#endif  /* IMP_INTERNAL_PARTICLE_SAVE_H */
