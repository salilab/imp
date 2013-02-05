/**
 *  \file internal/utility.h
 *  \brief Various useful utilities
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPKERNEL_INTERNAL_UTILITY_H
#define IMPKERNEL_INTERNAL_UTILITY_H

#include <IMP/kernel/kernel_config.h>
#include "../Particle.h"
#include <boost/format.hpp>
#include <sstream>

IMPKERNEL_BEGIN_INTERNAL_NAMESPACE

inline std::string make_object_name(std::string templ, unsigned int index) {
  std::ostringstream oss;
  boost::format format(templ);
  format.exceptions(boost::io::no_error_bits );
  oss << format %index;
  return oss.str();
}

struct Counter {
  unsigned int count;
  Counter(): count(0){}
  operator unsigned int () const {return count;}
  Counter operator++() {
    ++count;
    return *this;
  }
  void operator=(unsigned int i) {
    count=i;
  }
};

// Note that older g++ is confused by queue.back().get<2>()
#define IMP_PRINT_TREE(stream, NodeType, start, num_children,           \
                       get_child, show)                                 \
  {                                                                     \
    base::Vector<boost::tuple<std::string, std::string, NodeType> >      \
      queue;                                                            \
    queue.push_back(boost::make_tuple(std::string(),                    \
                                      std::string(), start));           \
    do {                                                                \
      boost::tuple<std::string, std::string, NodeType> &back = queue.back(); \
      NodeType n= back.get<2>();                                        \
      std::string prefix0= back.get<0>();                               \
      std::string prefix1= back.get<1>();                               \
      queue.pop_back();                                                 \
      stream << prefix0;                                                \
      unsigned int nc= num_children;                                    \
      if (nc>0) stream << " + ";                                        \
      else stream << " - ";                                             \
      show;                                                             \
      stream  << std::endl;                                             \
      for (int i=static_cast<int>(nc-1); i>=0; --i) {                   \
        queue.push_back(boost::make_tuple(prefix1+" ",                  \
                                          prefix1+" ", get_child(i)));  \
      }                                                                 \
    } while (!queue.empty());                                           \
  }                                                                     \


typedef bool (*ParticleFunction)(Particle*);

IMPKERNELEXPORT
void add_particle_check(ParticleFunction instance, ParticleFunction check);


struct ParticleCheck {
  ParticleCheck(ParticleFunction instance, ParticleFunction check) {
    add_particle_check(instance, check);
  }
};



template <class T>
struct SFSetIt {
  T *t_;
  T old_;
  SFSetIt(T *t, T nv): t_(t), old_(*t){
    *t_=nv;
  }
  ~SFSetIt() {
    *t_= old_;
  }
};


struct SFResetBitset {
  boost::dynamic_bitset<> &bs_;
  bool val_;
  SFResetBitset(boost::dynamic_bitset<> &bs,
              bool val): bs_(bs), val_(val){}
  ~SFResetBitset() {
    if (val_) {
      bs_.set();
    } else {
      bs_.reset();
    }
  }
};

IMPKERNEL_END_INTERNAL_NAMESPACE

#endif  /* IMPKERNEL_INTERNAL_UTILITY_H */
