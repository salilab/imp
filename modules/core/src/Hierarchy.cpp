/**
 *  \file Hierarchy.cpp   \brief Decorator for helping deal with
 *                                        a hierarchy.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#include <IMP/core/Hierarchy.h>

#include <sstream>

IMPCORE_BEGIN_NAMESPACE

const HierarchyTraits& Hierarchy::get_default_traits() {
  static HierarchyTraits ret("hierarchy");
  return ret;
}

HierarchyTraits::HierarchyTraits(std::string name): P(name),
                         parent_key_((name+"_parent").c_str()),
                         parent_index_key_((name+"_parent_index").c_str()){
}


/*Hierarchy::Hierarchy(Particle *p,
                                       HierarchyTraits traits): P(p),
                                                                traits_(traits){
}

Hierarchy::Hierarchy(HierarchyTraits traits): traits_(traits){
}*/


void Hierarchy::validate_node() const
{
  //get_particle()->get_model()->show(std::cerr);
  if (has_parent()) {
    IMP_assert(get_parent_index() >= 0,
               "The parent index must be positive if it is there");
    This p= get_parent();
    IMP_assert(p.get_particle() != get_particle(),
               "A particle can't be its own parent " << *p.get_particle());
    IMP_assert(p.get_child_index(*this) == get_parent_index(),
               "Incorrect parent index in particle "
               << *get_particle());
  }
}

void Hierarchy::show(std::ostream &out) const
{
  out << "Hierarchy";
}


unsigned int count_hierarchy(Hierarchy h)
{
  HierarchyCounter hc;
  depth_first_traversal(h,hc);
  return hc.get_count();
}




namespace internal
{

struct AssertHierarchy: public HierarchyVisitor
{
  AssertHierarchy(){}
  bool visit(Hierarchy p) {
    p.validate_node();
    return true;
  }
};

} // namespace internal


void Hierarchy::validate() const
{
  //std::cerr << "Checking hierarchy" << std::endl;
  internal::AssertHierarchy ah;
  depth_first_traversal(*this, ah);
}


int Hierarchy::get_child_index(Hierarchy c) const
{
  IMP_check(traits_.get_name() == c.traits_.get_name(),
            "Attemping to mix hierarchy of type " << traits_.get_name()
            << " with one of type " << c.traits_.get_name(),
            InvalidStateException);
  for (unsigned int i=0; i< get_number_of_children(); ++i ) {
    if (get_child(i) == c) return i;
  }
  return -1;
}


void breadth_first_traversal(Hierarchy d, HierarchyVisitor &f)
{
  std::deque<Hierarchy> stack;
  stack.push_back(d);
  //d.show(std::cerr);
  do {
    Hierarchy cur= stack.front();
    stack.pop_front();
    if (f.visit(cur)) {
      //std::cerr << "Visiting particle " << cur.get_particle() << std::endl;
      for (int i=cur.get_number_of_children()-1; i>=0; --i) {
        stack.push_back(cur.get_child(i));
      }
    }
  } while (!stack.empty());
}

void depth_first_traversal(Hierarchy d, HierarchyVisitor &f)
{
  std::vector<Hierarchy> stack;
  stack.push_back(d);
  do {
    Hierarchy cur= stack.back();
    stack.pop_back();
    if (f.visit(cur)) {
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
  HierarchyTraits traits_;
  MHDMatchingLeaves(HierarchyTraits tr): traits_(tr){}
  bool operator()(Particle *p) const {
    Hierarchy mhd(p, traits_);
    return mhd.get_number_of_children()==0;
  }
};

} // namespace


GenericHierarchies
get_leaves(Hierarchy mhd)
{
  GenericHierarchies out;
  gather(mhd, MHDMatchingLeaves(mhd.get_traits()),
                   std::back_inserter(out));
  return out;
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

GenericHierarchies
get_all_descendants(Hierarchy mhd)
{
  GenericHierarchies out;
  gather(mhd, MHDMatchingAll(),
         std::back_inserter(out));
  return out;
}

IMPCORE_END_NAMESPACE
