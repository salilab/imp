/**
 *  \file HierarchyDecorator.h     \brief Decorator for helping deal with
 *                                        a hierarchy.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMP_HIERARCHY_DECORATOR_H
#define __IMP_HIERARCHY_DECORATOR_H

#include <limits>
#include <vector>
#include <deque>

#include "../Particle.h"
#include "../Model.h"
#include "../DecoratorBase.h"
#include "utility.h"

namespace IMP
{


/** \defgroup hierarchy Hierarchies of particles
    These functions and classes aid in manipulating particles representing
    molecules at multiple levels.
 */

//! A visitor for traversal of a hierarchy
/** This works from both C++ and Python
    \ingroup hierarchy
 */
class IMPDLLEXPORT HierarchyVisitor
{
public:
  HierarchyVisitor() {}
  //! Return true if the traversal should visit this node's children
  /** The const is needed to make memory management simple. Just declare
      internal data mutable.
   */
  virtual bool visit(Particle *p) = 0;
  virtual ~HierarchyVisitor() {}
};


//! A decorator for helping deal with a hierarchy.
/** \ingroup hierarchy
 */
class IMPDLLEXPORT HierarchyDecorator: public DecoratorBase
{
  IMP_DECORATOR(HierarchyDecorator, DecoratorBase, return true || p,  ++p);
  IMP_DECORATOR_ARRAY_DECL(child, Particle, Particle*, NULL)
protected:
  static ParticleKey parent_key_;
  static IntKey parent_index_key_;

public:

  //! Get a HierarchyDecorator wrapping the parent particle
  /** \return the parent particle, or HierarchyDecorator()
      if it has no parent.
   */
  This get_parent() const {
    IMP_DECORATOR_GET(parent_key_, Particle*,
                      return VALUE,
                      return This());
  }

  //! Get the number of children.
  unsigned int get_number_of_children() const {
    // defined by the array macro
    return internal_get_number_of_child();
  }

  //! Get a HierarchyDecorator of the ith child
  /** \return decorator of the ith child, or throw an IndexException if it
      does not have this child
   */
  This get_child(unsigned int i) const {
    // defined by the array macro
    return cast(internal_get_child(i));

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

  //! Get the index of a specific child in this particle.
  /** This takes linear time.
      \return the index, or -1 if there is no such child.
   */
  int get_child_index(Particle *c) const;

  //! Do some simple validity checks on this node in the hierarchy
  void validate_node() const;

  //! Do some validity checks on the subhierarchy
  void validate() const;
};


IMP_OUTPUT_OPERATOR(HierarchyDecorator);



//! Apply the visitor to each particle,  breadth first.
/** \param[in] d The HierarchyDecorator for the tree in question
    \param[in] v The visitor to be applied. This is passed by reference.
    \ingroup hierarchy
 */
IMPDLLEXPORT
void breadth_first_traversal(HierarchyDecorator d,  HierarchyVisitor &v);

//! Depth first traversal of the hierarchy
/** See breadth_first_traversal and HierarchyVisitor for more information
    \ingroup hierarchy
 */
IMPDLLEXPORT
void depth_first_traversal(HierarchyDecorator d,  HierarchyVisitor &v);


//! Apply functor F to each particle, traversing the hierarchy breadth first.
/** This method allows data to be associated with each visited node.
    The data of the parent is passed to each invocation of the child.

    \param[in] d The HierarchyDecorator for the tree in question
    \param[in] f The functor to be applied
    F must define a type Data which is returned by each call.
    The result of the parent call is passed as the second argument
    to the operator() of the child. e.g.
    struct DepthVisitor {
      typedef int result_type;
      result_type operator()(Particle *p, int i) const
      {
        if (p==NULL) return 0;
        else return i+1;
      }
    };
    \param[in] i The data to be used for d (since it has no relevant parent)

    \return A copy of the functor passed in. Use this if you care about
           the functor state.

    \ingroup hierarchy
 */
template <class HD, class F>
F breadth_first_traversal_with_data(HD d, F f, typename F::result_type i)
{
  typedef std::pair<typename F::result_type, HD> DP;
  std::deque<DP > stack;
  stack.push_back(DP(i, d));
  //d.show(std::cerr);
  do {
    DP cur= stack.front();
    stack.pop_front();
    typename F::result_type r= f(cur.second.get_particle(), cur.first);
    //std::cerr << "Visiting particle " << cur.get_particle() << std::endl;
    for (int i=cur.second.get_number_of_children()-1; i>=0; --i) {
      stack.push_back(std::make_pair(r, cur.second.get_child(i)));
    }
  } while (!stack.empty());
  return f;
}


//! Apply functor F to each particle, traversing the hierarchy depth first.
/** See breadth_first_traversal for documentation.
    \ingroup hierarchy
 */
template <class HD, class F>
F depth_first_traversal_with_data(HD d,  F f, typename F::result_type i)
{
  typedef std::pair<typename F::result_type, HD> DP;
  std::vector<DP> stack;
  stack.push_back(DP(i, d));
  //d.show(std::cerr);
  do {
    DP cur= stack.back();
    stack.pop_back();
    typename F::result_type r=f(cur.second.get_particle(), cur.first);
    //std::cerr << "Visiting particle " << cur.get_particle() << std::endl;
    for (int i=cur.second.get_number_of_children()-1; i>=0; --i) {
      stack.push_back(DP(r, cur.second.get_child(i)));
    }
  } while (!stack.empty());
  return f;
}




//! A simple visitor which pretty-prints the hierarchy
/** The template argument NP is the decorator to use to print each node.
    \ingroup hierarchy
 */
template <class PD>
struct HierarchyPrinter
{
  HierarchyPrinter(std::ostream &out,
                   unsigned int max_depth): out_(out), md_(max_depth) {}
  typedef unsigned int result_type;
  int operator()(Particle *p, unsigned int depth) const {
    if (depth > md_) return depth+1;

    HierarchyDecorator hd= HierarchyDecorator::cast(p);
    std::string prefix;
    for (unsigned int i=0; i< depth; ++i) {
      out_ << " ";
      prefix+=" ";
    }
    if (hd == HierarchyDecorator() || hd.get_number_of_children()==0) {
      out_ << "-";
    } else {
      out_ << "+";
    }
    out_ << "Particle " << p->get_index() << std::endl;
    prefix += "  ";
    PD nd= PD::cast(p);
    if (nd != PD()) {
      nd.show(out_, prefix);
    } else {
      out_ << prefix << "*******";
    }
    out_ << std::endl;
    return depth+1;
  }
  std::ostream &out_;
  unsigned int md_;
};


//! Print the hierarchy using a given decorator as to display each node
/** The last argument limits how deep will be printed out.
    \ingroup hierarchy
 */
template <class ND>
std::ostream &show(HierarchyDecorator h, std::ostream &out=std::cout,
                   unsigned int max_depth
                   = std::numeric_limits<unsigned int>::max())
{
  depth_first_traversal_with_data(h, HierarchyPrinter<ND>(out, max_depth), 0);
  return out;
}


//! A simple functor to count the number of particles in a hierarchy.
/** This is a good example of a simple HierarchyVisitor.
    \ingroup hierarchy
 */
struct HierarchyCounter: public HierarchyVisitor
{
  HierarchyCounter(): ct_(0) {}

  //! Increment the counter
  bool visit(Particle*) {
    ++ct_;
    return true;
  }
  //! Return how many nodes have been visited
  unsigned int get_count() const {
    return ct_;
  }
private:

  unsigned int ct_;
};

namespace internal
{

template <class F, class Out>
struct Gather: public HierarchyVisitor
{
  Gather(F f, Out out): f_(f), out_(out) {}
  bool visit(Particle *p) {
    if (f_(p)) {
      *out_=p;
      ++out_;
    }
    return true;
  }
  Out get_out() const {
    return out_;
  }
  F f_;
  Out out_;

};

} // namespace internal

//! Gather all the Particle* in the hierarchy which meet some criteria
/** \ingroup hierarchy
 */
template <class Out, class F>
Out hierarchy_gather(HierarchyDecorator h, F f, Out out)
{
  internal::Gather<F,Out> gather(f,out);
  depth_first_traversal(h, gather);
  return gather.get_out();
}

} // namespace IMP

#endif  /* __IMP_HIERARCHY_DECORATOR_H */
