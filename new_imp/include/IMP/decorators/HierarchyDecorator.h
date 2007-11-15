/**
 *  \file HierarchyDecorator.h     \brief Decorator for helping deal with
 *                                        a hierarchy.
 *
 *  Copyright 2007 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMP_HIERARCHY_DECORATOR_H
#define __IMP_HIERARCHY_DECORATOR_H

#include "../Particle.h"
#include "../Model.h"
#include "utility.h"
#include <vector>
#include <deque>

namespace IMP
{

//! A decorator for helping deal with a hierarchy.
class IMPDLLEXPORT HierarchyDecorator
{
  IMP_DECORATOR(HierarchyDecorator, return true || p,  ++p);
  IMP_DECORATOR_ARRAY_DECL(child, Int)
protected:
  static bool keys_initialized_;
  static IntKey parent_key_;
  static IntKey parent_index_key_;

public:

  //! Get a HierarchyDecorator wrapping the parent particle
  /** \return the parent particle, or HierarchyDecorator() if it has no parent.
    */
  This get_parent() const {
    IMP_DECORATOR_GET(parent_key_, Int,
                      return cast(get_model()->
                                  get_particle(ParticleIndex(VALUE))),
                      return This());
  }

  //! Get the number of children.
  unsigned int get_number_of_children() const {
    return internal_get_number_of_child();
  }

  //! Get a HierarchyDecorator of the ith child
  /** \return decorator of the ith child, or thrown and IndexException if it
              does not have this child
   */
  This get_child(unsigned int i) const {
    return cast(get_model()->
                get_particle(ParticleIndex(internal_get_child(i))));

  }

  //! Get the index of this particle in the list of children
  /** \return index in the list of children of the parent, or -1 if
                    it does not have a parent.
   */
  int get_parent_index() const {
    IMP_DECORATOR_GET(parent_index_key_, Int, return VALUE, return -1);
  }

  //! Return true if it has a parent.
  bool has_parent() const {
    return get_particle()->has_attribute(parent_key_);
  }

  //! Add the particle as the last child.
  unsigned int add_child(HierarchyDecorator hd);

  void show(std::ostream &out, int level) const;

  //! Get the index of a specific child in this particle.
  /** This takes linear time.
      \return the index, or -1 if there is no such child.
   */
  int get_child_index(Particle *c) const ;

  //! Do some simple validity checks on this node in the hierarchy
  void assert_is_valid() const ;
};


//! Apply functor F to each particle, traversing the hierarchy breadth first.
template <class F>
void breadth_first_traversal(HierarchyDecorator d, const F& f)
{
  std::deque<HierarchyDecorator> stack;
  stack.push_back(d);
  //d.show(std::cerr);
  do {
    HierarchyDecorator cur= stack.front();
    stack.pop_front();
    f(cur.get_particle());
    //std::cerr << "Visiting particle " << cur.get_particle() << std::endl;
    for (int i=0; i<cur.get_number_of_children(); ++i) {
      stack.push_back(cur.get_child(i));
    }
  } while (!stack.empty());
}


//! Apply functor F to each particle, traversing the hierarchy depth first.
template <class F>
void depth_first_traversal(HierarchyDecorator d, const F& f)
{
  std::vector<HierarchyDecorator> stack;
  stack.push_back(d);
  //d.show(std::cerr);
  do {
    HierarchyDecorator cur= stack.back();
    stack.pop_back();
    f(cur.get_particle());
    //std::cerr << "Visiting particle " << cur.get_particle() << std::endl;
    for (unsigned int i=0; i< cur.get_number_of_children(); ++i) {
      stack.push_back(cur.get_child(i));
    }
  } while (!stack.empty());
}

//! Convenience method to check the hierarchy.
/** \note This is just here to avoid writing Python test code that
          duplicates the C++ code.
 */
IMPDLLEXPORT void assert_hierarchy(Particle *p);
}

#endif  /* __IMP_HIERARCHY_DECORATOR_H */
