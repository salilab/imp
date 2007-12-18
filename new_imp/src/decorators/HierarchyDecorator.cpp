/**
 *  \file HierarchyDecorator.cpp   \brief Decorator for helping deal with
 *                                        a hierarchy.
 *
 *  Copyright 2007 Sali Lab. All rights reserved.
 *
 */

#include <sstream>

#include "IMP/decorators/HierarchyDecorator.h"
#include "IMP/decorators/NameDecorator.h"

namespace IMP
{

IMP_DECORATOR_ARRAY_DEF(Hierarchy, child, Int);
IntKey HierarchyDecorator::parent_key_;
IntKey HierarchyDecorator::parent_index_key_;


void HierarchyDecorator::validate_node() const
{
  //get_particle()->get_model()->show(std::cerr);
  if (has_parent()) {
    int i= get_parent_index();
    IMP_assert(i >= 0, "The parent index must be positive if it is there");
    This p= get_parent();
    IMP_assert(p.get_particle() != get_particle(),
               "A particle can't be its own parent " << *p.get_particle());
    IMP_assert(p.get_child_index(get_particle()) == i,
               "Incorrect parent index in particle "
               << *get_particle());
  }
  for (unsigned int i=0; i< get_number_of_children(); ++i) {
    IMP_assert(get_particle()->has_attribute(get_child_key(i)),
               "Recorded and actual number of children don't match -- "
               "too few actual");
  }
  IMP_assert(!get_particle()->has_attribute(
                 get_child_key(get_number_of_children())),
             "Recorded and actual number of children don't match -- "
             "too many actual")
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


int HierarchyDecorator::get_child_index(Particle *c) const
{
  for (unsigned int i=0; i< get_number_of_children(); ++i ) {
    if (get_child(i) == HierarchyDecorator(c)) return i;
  }
  return -1;
}



unsigned int HierarchyDecorator::add_child(HierarchyDecorator hd)
{
  IMP_assert(hd.get_particle() != get_particle(),
             "A particle can't be its own child " << *this << std::endl 
             << hd);
  //std::cerr << *get_particle() << std::endl;
  //std::cerr << *p << std::endl;
  //std::cerr << "changing " << std::endl;
  //std::cerr << "before\n";
  //show_hierarchy<NameDecorator>(*this, std::cerr);
  int pi= get_particle()->get_index().get_index();
  int ci= hd.get_particle()->get_index().get_index();
  int nc= internal_add_child(ci);

  hd.get_particle()->add_attribute(parent_index_key_, nc);
  hd.get_particle()->add_attribute(parent_key_, pi);
  //std::cerr << "after\n";
  //show_hierarchy<NameDecorator>(*this, std::cerr);
  return nc;
}


IMP_DECORATOR_INITIALIZE(HierarchyDecorator, DecoratorBase,
                         {
                           parent_key_ = IntKey("hierarchy_parent");
                           parent_index_key_ = IntKey("hiearchy_parent_index");
                           IMP_DECORATOR_ARRAY_INIT(HierarchyDecorator,
                                                    child, Int);
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


} // namespace IMP
