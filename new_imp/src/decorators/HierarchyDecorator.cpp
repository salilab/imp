/**
 *  \file HierarchyDecorator.cpp   \brief Decorator for helping deal with
 *                                        a hierarchy.
 *
 *  Copyright 2007 Sali Lab. All rights reserved.
 *
 */

#include <IMP/decorators/HierarchyDecorator.h>
#include <IMP/decorators/NameDecorator.h>
#include <sstream>

namespace IMP
{
IMP_DECORATOR_ARRAY_DEF(Hierarchy, child, Int);
bool HierarchyDecorator::keys_initialized_=false;
IntKey HierarchyDecorator::parent_key_;
IntKey HierarchyDecorator::parent_index_key_;


void HierarchyDecorator::assert_is_valid() const
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

void HierarchyDecorator::show(std::ostream &out) const {
  return show(out, 0);
}

void HierarchyDecorator::show(std::ostream &out, int level) const
{
  assert_is_valid();

  for (int i=0; i< level; ++i) {
    out << " ";
  }
  if (get_number_of_children() != 0) {
    out << "+";
  } else {
    out << " ";
  }

  NameDecorator nd= NameDecorator::cast(get_particle());
  if (nd != NameDecorator()) {
    out << nd.get_name();
  } else {
    out << "";
  }

  out << std::endl;
  for (unsigned int index=0; index < get_number_of_children(); ++index) {
    This hd= get_child(index);
    hd.show(out, level+1);
  }
}




namespace internal {

struct AssertHierarchy {
  void operator()(Particle *p) const {
    HierarchyDecorator d= HierarchyDecorator::cast(p);
    d.assert_is_valid();
  }
};

} // namespace internal


void assert_hierarchy(Particle *p)
{
  //std::cerr << "Checking hierarchy" << std::endl;
  HierarchyDecorator h= HierarchyDecorator::cast(p);
  depth_first_traversal(h, internal::AssertHierarchy());
}


int HierarchyDecorator::get_child_index(Particle *c) const
{
  for (unsigned int i=0; i< get_number_of_children(); ++i ) {
    if (get_child(i) == c) return i;
  }
  return -1;
}



unsigned int HierarchyDecorator::add_child(HierarchyDecorator hd)
{
  IMP_assert(hd.get_particle() != get_particle(),
             "A particle can't be its own parent");
  //std::cerr << *get_particle() << std::endl;
  //std::cerr << *p << std::endl;
  //std::cerr << "changing " << std::endl;

  int pi= get_particle()->get_index().get_index();
  int ci= hd.get_particle()->get_index().get_index();
  int nc= internal_add_child(ci);

  hd.get_particle()->add_attribute(parent_index_key_, nc);
  hd.get_particle()->add_attribute(parent_key_, pi);
  return nc;
}



IMPDLLEXPORT void HierarchyDecorator::initialize_static_data()
{
  if (keys_initialized_) {
    return;
  } else {
    parent_key_= IntKey("hierarchy_parent");
    parent_index_key_= IntKey("hiearchy_parent_index");
    IMP_DECORATOR_ARRAY_INIT(HierarchyDecorator, child, Int);
    keys_initialized_=true;
  }
}

} // namespace IMP
