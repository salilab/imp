/**
 *  \file Hierarchy.cpp   \brief Decorator for helping deal with
 *                                        a hierarchy.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/core/Hierarchy.h>

#include <sstream>

IMPCORE_BEGIN_NAMESPACE

const HierarchyTraits& Hierarchy::get_default_traits() {
  static HierarchyTraits ret("hierarchy");
  return ret;
}

HierarchyTraits::HierarchyTraits(std::string name): P(name)
{
  P::get_data().parent_key_=ParticleKey((name+"_parent").c_str());
  P::get_data().parent_index_key_= IntKey((name+"_parent_index").c_str());
  P::get_data().cache_key_= ObjectKey((name+"_cache").c_str());
}


void Hierarchy::show(std::ostream &out) const
{
  out << "Hierarchy";
}

namespace {
  class HierarchyCache: public Object {
  public:
    ParticlesTemp leaves;
    HierarchyCache(): Object("HierarchyCache"){}
    IMP_OBJECT(HierarchyCache);
  };
  void HierarchyCache::do_show(std::ostream &out) const {
    out << "HierarchyCache";
  }

  HierarchyCache* get_cache(Hierarchy h) {
    ObjectKey k= h.get_traits().get_data().cache_key_;
    if (!h->has_attribute(k)) return NULL;
    Object *o= h->get_value(k);
    return dynamic_cast<HierarchyCache*>(o);
  }

  HierarchyCache* rebuild_cache(Hierarchy h) {
    Pointer<HierarchyCache> c= new HierarchyCache();
    h.get_particle()->add_cache_attribute(h.get_traits().get_data().cache_key_,
                                          c);
    c->leaves= get_leaves(h);
    return c;
  }
}



const ParticlesTemp& Hierarchy::get_leaves() const {
  HierarchyCache *c= get_cache(*this);
  if (!c) {
    c=rebuild_cache(*this);
  }
  return c->leaves;
}

unsigned int count_hierarchy(Hierarchy h)
{
  HierarchyCounter hc;
  visit_depth_first(h,hc);
  return hc.get_count();
}




int Hierarchy::get_child_index(Hierarchy c) const
{
  IMP_USAGE_CHECK(get_decorator_traits().get_name()
                  == c.get_decorator_traits().get_name(),
            "Attemping to mix hierarchy of type "
                  << get_decorator_traits().get_name()
            << " with one of type " << c.get_decorator_traits().get_name());
  for (unsigned int i=0; i< get_number_of_children(); ++i ) {
    if (get_child(i) == c) return i;
  }
  return -1;
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


GenericHierarchiesTemp
get_leaves(Hierarchy mhd)
{
  GenericHierarchiesTemp out;
  gather(mhd, MHDMatchingLeaves(mhd.get_traits()),
                   std::back_inserter(out));
  return out;
}



namespace
{

struct MHDNotMatchingLeaves
{
  HierarchyTraits traits_;
  MHDNotMatchingLeaves(HierarchyTraits tr): traits_(tr){}
  bool operator()(Particle *p) const {
    Hierarchy mhd(p, traits_);
    return mhd.get_number_of_children()!=0;
  }
};

} // namespace


GenericHierarchiesTemp
get_internal(Hierarchy mhd)
{
  GenericHierarchiesTemp out;
  gather(mhd, MHDNotMatchingLeaves(mhd.get_traits()),
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

GenericHierarchiesTemp
get_all_descendants(Hierarchy mhd)
{
  GenericHierarchiesTemp out;
  gather(mhd, MHDMatchingAll(),
         std::back_inserter(out));
  return out;
}

IMPCORE_END_NAMESPACE
