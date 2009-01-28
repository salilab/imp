/**
 *  \file HierarchyDecorator.h     \brief Decorator for helping deal with
 *                                        a hierarchy.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPCORE_HIERARCHY_DECORATOR_H
#define IMPCORE_HIERARCHY_DECORATOR_H

#include "config.h"
#include "utility.h"
#include "bond_decorators.h"
#include "internal/hierarchy_helpers.h"
#include "internal/ArrayOnAttributesHelper.h"

#include <IMP/SingletonModifier.h>
#include <IMP/Particle.h>
#include <IMP/Model.h>
#include <IMP/DecoratorBase.h>

#include <limits>
#include <vector>
#include <deque>

IMPCORE_BEGIN_NAMESPACE

/** \defgroup hierarchy Hierarchies of particles
    These functions and classes aid in manipulating particles representing
    molecules at multiple levels.
 */

class HierarchyDecorator;

//! Define the type for a type of hierarchy
/** The hierarchy class is identified by the passed string so two
    hierarchies created with the same initialization string will be
    the same.

    This example shows how to make and use a custom hierarchy:
    \verbinclude custom_hierarchy.py
*/
class IMPCOREEXPORT HierarchyTraits
#ifndef SWIG
: public internal::ArrayOnAttributesHelper<ParticleKey, Particle*>
#endif
{
  friend class HierarchyDecorator;
  typedef internal::ArrayOnAttributesHelper<ParticleKey, Particle*> P;

  ParticleKey parent_key_;
  IntKey parent_index_key_;

  template <class HD>
  void on_add(Particle * p, HD d, unsigned int i) {
    d.get_particle()->add_attribute(parent_key_, p);
    d.get_particle()->add_attribute(parent_index_key_, i);
  }
  void on_change(Particle *, Particle* p, unsigned int oi,
                        unsigned int ni) {
    p->set_value(parent_index_key_, ni);
  }
  template <class HD>
  void on_remove(Particle *, HD d) {
    d.get_particle()->remove_attribute(parent_index_key_);
    d.get_particle()->remove_attribute(parent_key_);
  }
  template <class HD>
  Particle *get_value(HD d) {
    return d.get_particle();
  }
  template <class HD>
  unsigned int get_index(Particle *, HD d) {
    return d.get_parent_index();
  }
  // otherwise it is masked
  using P::get_value;

  template <class T>
  void audit_value(T t) const {
    IMP_check(t.get_hierarchy_type() == get_name(),
              "Mixing hierarchies of type " << get_name()
              << " and type " << t.get_hierarchy_type(),
              ValueException);
  }

  const HierarchyDecorator wrap(Particle* p) const;

public:
  HierarchyTraits(){}
  //! Create a HierarchyTraits with the given name
  HierarchyTraits(std::string name);
  //! Get the name used to identify this traits.
  std::string get_name() const {
    return get_prefix();
  }
};




//! A visitor for traversal of a hierarchy
/** This works from both C++ and Python
    \ingroup hierarchy
    \ingroup decorators
    \relates HierarchyDecorator
 */
class IMPCOREEXPORT HierarchyVisitor
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



//! A which applies a singleton modifier to each Particle in a hierarchy
/** This works from both C++ and Python
    \ingroup hierarchy
    \ingroup decorators
    \relates SingletonModifier
    \relates HierarchyDecorator
 */
class IMPCOREEXPORT ModifierVisitor: public HierarchyVisitor
{
  Pointer<SingletonModifier> sm_;
public:
  ModifierVisitor(SingletonModifier *sm): sm_(sm) {}
  virtual bool visit(Particle *p) {
    sm_->apply(p);
    return true;
  }
  virtual ~ModifierVisitor() {}
};






//! A decorator for helping deal with a hierarchy.
/** \ingroup hierarchy
 */
class IMPCOREEXPORT HierarchyDecorator: public DecoratorBase
{
  typedef DecoratorBase P;

  IMP_DECORATOR_ARRAY_DECL(public, child, children, traits_,
                           HierarchyDecorator)
public:
  IMP_DECORATOR_TRAITS(HierarchyDecorator, DecoratorBase,
                       HierarchyTraits, traits,
                       HierarchyDecorator::get_default_traits());


  //! Add the needed attributes to a particle
  static HierarchyDecorator create(Particle *p,
                                   HierarchyTraits traits
                                   =HierarchyDecorator::get_default_traits()) {
    add_required_attributes_for_child(p, traits);
    return HierarchyDecorator(p, traits);
  }

  /** Check if the particle has the needed attributes for a
   cast to succeed */
  static bool is_instance_of(Particle *p,
                             HierarchyTraits traits
                             =HierarchyDecorator::get_default_traits()){
    return has_required_attributes_for_child(p, traits);
  }

  /** \return the parent particle, or HierarchyDecorator()
      if it has no parent.
   */
  This get_parent() const {
    IMP_DECORATOR_GET(traits_.parent_key_, Particle*,
                      return This(VALUE, traits_),
                      return This());
  }

  //! Get the index of this particle in the list of children
  /** \return index in the list of children of the parent, or -1 if
      it does not have a parent.
   */
  int get_parent_index() const {
    IMP_DECORATOR_GET(traits_.parent_index_key_,
                      Int, return VALUE, return -1);
  }

  /** Return true if the parent is not empty */
  bool has_parent() const {
    return get_particle()->has_attribute(traits_.parent_key_);
  }

  //! Get the index of a specific child in this particle.
  /** This takes linear time.
      \note This is mostly useful for debugging as you can always call
      get_parent_index() on the child.
      \return the index, or -1 if there is no such child.
   */
  int get_child_index(HierarchyDecorator c) const;

  //! Do some simple validity checks on this node in the hierarchy
  void validate_node() const;

  //! Do some validity checks on the subhierarchy
  void validate() const;

  //! Return the string identifying this type of hierarchy
  std::string get_hierarchy_type() const {
    return traits_.get_name();
  }

  //! Get the default hierarchy traits
  static const HierarchyTraits& get_default_traits();
  };


IMP_OUTPUT_OPERATOR(HierarchyDecorator);

//! Collect the matching visiting nodes into a container.
/** A node is collected if the function evaluates true.
    \relates HierarchyDecorator
 */
template <class F, class Out>
struct Gather: public HierarchyVisitor
{
  //! initialize with the function and the container
  Gather(F f, Out out): f_(f), out_(out) {}
  bool visit(Particle *p) {
    if (f_(p)) {
      *out_=p;
      ++out_;
    }
    return true;
  }
  //! Return the container
  Out get_out() const {
    return out_;
  }
private:
  F f_;
  Out out_;
};


inline const HierarchyDecorator HierarchyTraits::wrap(Particle* p) const {
  return HierarchyDecorator(p, *this);
}








//! Apply the visitor to each particle,  breadth first.
/** \param[in] d The HierarchyDecorator for the tree in question
    \param[in] v The visitor to be applied. This is passed by reference.
    \ingroup hierarchy
    \relates HierarchyDecorator
 */
IMPCOREEXPORT
void breadth_first_traversal(HierarchyDecorator d,  HierarchyVisitor &v);

//! Depth first traversal of the hierarchy
/** See breadth_first_traversal and HierarchyVisitor for more information
    \ingroup hierarchy
    \relates HierarchyDecorator
 */
IMPCOREEXPORT
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
    \relates HierarchyDecorator
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
    \relates HierarchyDecorator
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
    \relates HierarchyDecorator
 */
template <class PD>
struct HierarchyPrinter
{
  HierarchyPrinter(std::ostream &out,
                   unsigned int max_depth,
                   HierarchyTraits traits
                   = HierarchyDecorator::get_default_traits()): traits_(traits),
                                                                out_(out),
                                                                md_(max_depth)
  {}

  typedef unsigned int result_type;
  int operator()(Particle *p, unsigned int depth) const {
    if (depth > md_) return depth+1;

    HierarchyDecorator hd= HierarchyDecorator::cast(p, traits_);
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
    out_ << "Particle " << p->get_name() << std::endl;
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
  HierarchyTraits traits_;
  std::ostream &out_;
  unsigned int md_;
};


//! Print the hierarchy using a given decorator as to display each node
/** The last argument limits how deep will be printed out.
    \ingroup hierarchy
    \relates HierarchyDecorator
 */
template <class ND>
std::ostream &show(HierarchyDecorator h, std::ostream &out=std::cout,
                   unsigned int max_depth
                   = std::numeric_limits<unsigned int>::max())
{
  depth_first_traversal_with_data(h, HierarchyPrinter<ND>(out, max_depth,
                                                          h.get_traits()), 0);
  return out;
}


//! A simple functor to count the number of particles in a hierarchy.
/** This is a good example of a simple HierarchyVisitor.
    \ingroup hierarchy
    \relates HierarchyDecorator
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

//! Gather all the Particle* in the hierarchy which meet some criteria
/** \ingroup hierarchy
    \relates HierarchyDecorator
 */
template <class Out, class F>
Out hierarchy_gather(HierarchyDecorator h, F f, Out out)
{
  Gather<F,Out> gather(f,out);
  depth_first_traversal(h, gather);
  return gather.get_out();
}

//! Gather all the Particle* in the hierarchy which match on an attribute
/** \ingroup hierarchy
    \relates HierarchyDecorator
 */
template <class Out, class K, class V>
Out hierarchy_gather_by_attribute(HierarchyDecorator h, K k, V v, Out out)
{
  Gather<internal::MatchAttribute<K, V>,Out>
    gather(internal::MatchAttribute<K,V>(k,v),
           out);
  depth_first_traversal(h, gather);
  return gather.get_out();
}




//! Gather all the Particle* in the hierarchy which match on two attributes
/** \ingroup hierarchy
    \relates HierarchyDecorator
 */
template <class Out, class K0, class V0, class K1, class V1>
Out hierarchy_gather_by_attributes(HierarchyDecorator h, K0 k0,
                                   V0 v0, K1 k1, V1 v1, Out out)
{
  Gather<internal::MatchAttributes<K0, V0, K1, V1>,Out>
    gather(internal::MatchAttributes<K0,V0, K1, V1>(k0,v0, k1, v1),
           out);
  depth_first_traversal(h, gather);
  return gather.get_out();
}


//! Find the first node which matches some criteria
/** \ingroup hierarchy
    \relates HierarchyDecorator
 */
template <class HD, class F>
HD hierarchy_find(HD h, F f)
{
  if (f(h.get_particle())) return h;
  std::vector<HD> stack;
  stack.push_back(h);
  //d.show(std::cerr);
  do {
    HD cur= stack.back();
    stack.pop_back();

    for (int i=cur.get_number_of_children()-1; i>=0; --i) {
      HD hd= cur.get_child(i);
      if (f(hd.get_particle())) {
        return hd;
      } else {
        stack.push_back(hd);
      }
    }
  } while (!stack.empty());
  return HD();
}



//! Get all the leaves of the bit of hierarchy
/**     \relates HierarchyDecorator
 */
IMPCOREEXPORT Particles
hierarchy_get_leaves(HierarchyDecorator mhd);

//! Get the bonds internal to this tree
/**     \relates HierarchyDecorator
 */
IMPCOREEXPORT BondDecorators
hierarchy_get_internal_bonds(HierarchyDecorator mhd);

//! Get all the particles in the subtree
/**     \relates HierarchyDecorator
 */
IMPCOREEXPORT Particles
hierarchy_get_all_descendants(HierarchyDecorator mhd);

IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_HIERARCHY_DECORATOR_H */
