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

bool HierarchyDecorator::keys_initialized_=false;
IntKey HierarchyDecorator::parent_key_;
IntKey HierarchyDecorator::parent_index_key_;
IntKey HierarchyDecorator::num_children_key_;
std::vector<IntKey> HierarchyDecorator::child_keys_;

void HierarchyDecorator::assert_is_valid() const
{
  //get_particle()->get_model()->show(std::cerr);
  if (has_parent()) {
    int i= get_parent_index();
    IMP_assert(i >= 0, "The parent index must be positive if it is there");
    This p= get_parent_decorator();
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

void HierarchyDecorator::show(std::ostream &out, int level) const
{
  assert_is_valid();

  for (int i=0; i< level; ++i) {
    out << " ";
  }
  if (get_particle()->has_attribute(num_children_key_)) {
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
    This hd= get_child_decorator(index);
    hd.show(out, level+1);
  }
}

void HierarchyDecorator::generate_child_keys(unsigned int i)
{
  while (!(i < child_keys_.size())) {
    //std::cerr << "generating child key " << i << std::endl;
    std::ostringstream oss;
    oss << "hierarchy_child_" << child_keys_.size();
    child_keys_.push_back(IntKey(oss.str().c_str()));
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



void HierarchyDecorator::add_child(Particle *p)
{
  IMP_assert(p != get_particle(),
             "A particle can't be its own parent");
  //std::cerr << *get_particle() << std::endl;
  //std::cerr << *p << std::endl;
  //std::cerr << "changing " << std::endl;

  int nc= get_number_of_children();
  int pi= get_particle()->get_index().get_index();
  int ci= p->get_index().get_index();
  //std::cerr << "pi is " << pi << " and ci is " << ci << std::endl;
  get_particle()->add_attribute(get_child_key(nc), ci);

  p->add_attribute(parent_index_key_, nc);
  p->add_attribute(parent_key_, pi);
  IMP_DECORATOR_SET(num_children_key_, nc+1);
  //std::cerr << *get_particle() << std::endl;
  //std::cerr << *p << std::endl;
  //assert_hierarchy(get_particle());
}



void HierarchyDecorator::initialize_static_data()
{
  if (keys_initialized_) {
    return;
  } else {
    parent_key_= IntKey("hierarchy_parent");
    parent_index_key_= IntKey("hiearchy_parent_index");
    num_children_key_= IntKey("hierarchy_num_children");
    keys_initialized_=true;
  }
}

} // namespace IMP
