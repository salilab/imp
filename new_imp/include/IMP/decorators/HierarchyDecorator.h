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
  IMP_DECORATOR(HierarchyDecorator);
protected:
  static bool keys_initialized_;
  static IntKey parent_key_;
  static IntKey parent_index_key_;
  static IntKey num_children_key_;
  static std::vector<IntKey> child_keys_;

  static void generate_child_keys(unsigned int i);

  static const IntKey get_child_key(unsigned int i) {
    if (i >= child_keys_.size()) generate_child_keys(i);
    return child_keys_[i];
  }


  static bool has_required_attributes(Particle *p) {
    return true;
  }
  static void add_required_attributes(Particle *p) {

  }
  static void initialize_static_data() ;
public:

  //! Get the parent Particle* or NULL if it has no parent.
  Particle *get_parent() const {
    IMP_DECORATOR_GET(parent_key_, Int,
                      return get_model()->get_particle(ParticleIndex(VALUE)),
                      return NULL);
  }

  //! Get a HierarchyDecorator wrapping the parent particle
  /** \return the parent particle, or HierarchyDecorator() if it has no parent.
    */
  This get_parent_decorator() const {
    IMP_DECORATOR_GET(parent_key_, Int,
                      return cast(get_model()->
                                  get_particle(ParticleIndex(VALUE))),
                      return This());
  }

  //! Get the number of children.
  unsigned int get_number_of_children() const {
    IMP_DECORATOR_GET(num_children_key_, Int, return VALUE, return 0);
  }

  //!  Get a Particle* of the ith child or NULL if it does not have this parent
  Particle *get_child(unsigned int i) const {
    IMP_DECORATOR_GET(get_child_key(i), Int,
                      return get_model()->get_particle(ParticleIndex(VALUE)),
                      return NULL);
  }

  //! Get a HierarchyDecorator of the ith child
  /** \return decorator of the ith child, or HierarchyDecorator() if it
              does not have this parent
   */
  This get_child_decorator(unsigned int i) const {
    IMP_DECORATOR_GET(get_child_key(i), Int,
                      return cast(get_model()->
                                  get_particle(ParticleIndex(VALUE))),
                      return This());
  }

  //! Get the index of this particle in the list of children
  /** \return index in the list of children of the parent, or -1 if
                    it does not have a parent.
   */
  unsigned int get_parent_index() const {
    IMP_DECORATOR_GET(parent_index_key_, Int, return VALUE, return -1);
  }

  //! Return true if it has a parent.
  bool has_parent() const {
    return get_particle()->has_attribute(parent_key_);
  }

  //! Add the particle as the last child.
  void add_child(Particle *p);

  void show(std::ostream &out, int level=0) const ;

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
      stack.push_back(cur.get_child_decorator(i));
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
      stack.push_back(cur.get_child_decorator(i));
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
