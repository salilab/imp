/**
 *  \file internal/utility.h
 *  \brief Various useful utilities
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 */

#ifndef IMP_INTERNAL_UTILITY_H
#define IMP_INTERNAL_UTILITY_H

#include "../kernel_config.h"
#include "../Particle.h"
#include <boost/format.hpp>
#include <sstream>

IMP_BEGIN_INTERNAL_NAMESPACE

//! \internal \return true if a passed particle is inactive
struct IsInactiveParticle
{
  bool operator()(Particle *p) const {
    return !p->get_is_active();
  }
};


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
    std::vector<boost::tuple<std::string, std::string, NodeType> >      \
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
        if (i != static_cast<int>(nc-1)) {                              \
          queue.push_back(boost::make_tuple(prefix1+" ",                \
                                         prefix1+" ", get_child(i)));   \
        } else {                                                        \
          queue.push_back(boost::make_tuple(prefix1+" ",                \
                                         prefix1+" ", get_child(i)));   \
        }                                                               \
      }                                                                 \
    } while (!queue.empty());                                           \
  }                                                                     \

IMP_END_INTERNAL_NAMESPACE

#endif  /* IMP_INTERNAL_UTILITY_H */
