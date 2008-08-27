/**
 *  \file HierarchyDecorator.cpp   \brief Decorator for helping deal with
 *                                        a hierarchy.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#include "IMP/decorators/HierarchyDecorator.h"
#include "IMP/decorators/NameDecorator.h"

#include <sstream>

namespace IMP
{

IMP_DECORATOR_ARRAY_DEF(HierarchyDecorator, child,
                        internal::ChildArrayTraits);
namespace internal
{
ParticleKey ChildArrayTraits::parent_key_;
IntKey ChildArrayTraits::parent_index_key_;
}

void HierarchyDecorator::validate_node() const
{
  //get_particle()->get_model()->show(std::cerr);
  if (has_parent()) {
    IMP_assert(get_parent_index() >= 0,
               "The parent index must be positive if it is there");
    This p= get_parent();
    IMP_assert(p.get_particle() != get_particle(),
               "A particle can't be its own parent " << *p.get_particle());
    IMP_assert(p.get_child_index(get_particle()) == get_parent_index(),
               "Incorrect parent index in particle "
               << *get_particle());
  }
}

void HierarchyDecorator::show(std::ostream &, std::string) const
{
}


unsigned int count_hierarchy(HierarchyDecorator h)
{
  HierarchyCounter hc;
  depth_first_traversal(h,hc);
  return hc.get_count();
}




namespace internal
{

struct AssertHierarchy: public HierarchyVisitor
{
  bool visit(Particle *p) {
    HierarchyDecorator d= HierarchyDecorator::cast(p);
    d.validate_node();
    return true;
  }
};

} // namespace internal


void HierarchyDecorator::validate() const
{
  //std::cerr << "Checking hierarchy" << std::endl;
  internal::AssertHierarchy ah;
  depth_first_traversal(*this, ah);
}


int HierarchyDecorator::get_child_index(HierarchyDecorator c) const
{
  for (unsigned int i=0; i< get_number_of_children(); ++i ) {
    if (get_child(i) == c) return i;
  }
  return -1;
}


IMP_DECORATOR_INITIALIZE(HierarchyDecorator, DecoratorBase,
                         {
                           internal::ChildArrayTraits::parent_key_
                             = ParticleKey("hierarchy_parent");
                           internal::ChildArrayTraits::parent_index_key_
                             = IntKey("hiearchy_parent_index");
                           IMP_DECORATOR_ARRAY_INIT(HierarchyDecorator,
                                                    child);
                         })


void breadth_first_traversal(HierarchyDecorator d, HierarchyVisitor &f)
{
  std::deque<HierarchyDecorator> stack;
  stack.push_back(d);
  //d.show(std::cerr);
  do {
    HierarchyDecorator cur= stack.front();
    stack.pop_front();
    if (f.visit(cur.get_particle())) {
      //std::cerr << "Visiting particle " << cur.get_particle() << std::endl;
      for (int i=cur.get_number_of_children()-1; i>=0; --i) {
        stack.push_back(cur.get_child(i));
      }
    }
  } while (!stack.empty());
}

void depth_first_traversal(HierarchyDecorator d, HierarchyVisitor &f)
{
  std::vector<HierarchyDecorator> stack;
  stack.push_back(d);
  do {
    HierarchyDecorator cur= stack.back();
    stack.pop_back();
    if (f.visit(cur.get_particle())) {
      for (int i=cur.get_number_of_children()-1; i>=0; --i) {
        stack.push_back(cur.get_child(i));
      }
    }
  } while (!stack.empty());
}



namespace
{

struct MHDMatchingLeaves
{
  bool operator()(Particle *p) const {
    HierarchyDecorator mhd(p);
    return mhd.get_number_of_children()==0;
  }
};

} // namespace


Particles
hierarchy_get_leaves(HierarchyDecorator mhd)
{
  Particles out;
  hierarchy_gather(mhd, MHDMatchingLeaves(),
                   std::back_inserter(out));
  return out;
}

BondDecorators hierarchy_get_internal_bonds(HierarchyDecorator mhd)
{
  Particles ps= hierarchy_get_all_descendants(mhd);
  std::set<Particle*> sps(ps.begin(), ps.end());
  BondDecorators ret;
  for (unsigned int i=0; i< ps.size(); ++i) {
    if (IMP::BondedDecorator::is_instance_of(ps[i])){
      IMP::BondedDecorator b(ps[i]);
      for (unsigned int i=0; i< b.get_number_of_bonds(); ++i) {
        Particle *op= b.get_bonded(i).get_particle();
        if (op < ps[i]
            && sps.find(op) != sps.end()) {
          ret.push_back(b.get_bond(i));
        }
      }
    }
  }
  return ret;
}


namespace
{

struct MHDMatchingAll
{
  bool operator()(Particle *) const {
    return true;
  }
};

} // namespace

Particles
hierarchy_get_all_descendants(HierarchyDecorator mhd)
{
  Particles out;
  hierarchy_gather(mhd, MHDMatchingAll(),
                   std::back_inserter(out));
  return out;
}

} // namespace IMP
