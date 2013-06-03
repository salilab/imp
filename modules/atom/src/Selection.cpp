/**
 *  \file atom/hierarchy_tools.cpp
 *  \brief A decorator for a point particle that has an electrostatic charge.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/atom/hierarchy_tools.h"
#include <IMP/SingletonContainer.h>
#include <IMP/SingletonPredicate.h>
#include <IMP/algebra/vector_generators.h>
#include <IMP/atom/Atom.h>
#include <IMP/atom/Chain.h>
#include <IMP/atom/Copy.h>
#include <IMP/atom/Domain.h>
#include <IMP/atom/Fragment.h>
#include <IMP/atom/Mass.h>
#include <IMP/atom/Molecule.h>
#include <IMP/atom/Residue.h>
#include <IMP/atom/bond_decorators.h>
#include <IMP/atom/distance.h>
#include <IMP/atom/estimates.h>
#include <IMP/constants.h>
#include <IMP/container/AllBipartitePairContainer.h>
#include <IMP/container/ConnectingPairContainer.h>
#include <IMP/container/ListSingletonContainer.h>
#include <IMP/container/generic.h>
#include <IMP/core/ClosePairsPairScore.h>
#include <IMP/core/ConnectivityRestraint.h>
#include <IMP/core/CoverRefined.h>
#include <IMP/core/DistancePairScore.h>
#include <IMP/core/ExcludedVolumeRestraint.h>
#include <IMP/core/Harmonic.h>
#include <IMP/core/PairRestraint.h>
#include <IMP/core/SphereDistancePairScore.h>
#include <IMP/core/TableRefiner.h>
#include <IMP/generic.h>
#include <algorithm>
IMPATOM_BEGIN_NAMESPACE

Selection::Selection() : radius_(-1) { m_ = nullptr; }
Selection::Selection(Particle *h) : radius_(-1) {
  set_hierarchies(h->get_model(), ParticleIndexes(1, h->get_index()));
}
Selection::Selection(Hierarchy h) : radius_(-1) {
  set_hierarchies(h.get_model(), ParticleIndexes(1, h.get_particle_index()));
}
Selection::Selection(Model *m, const ParticleIndexes &pis) : radius_(-1) {
  set_hierarchies(m, pis);
}
Selection::Selection(const Hierarchies &h) : radius_(-1) {
  if (h.empty()) {
    m_ = nullptr;
    return;
  } else {
    set_hierarchies(h[0].get_model(), IMP::internal::get_index(h));
  }
}
Selection::Selection(const ParticlesTemp &h) : radius_(-1) {
  if (h.empty()) {
    m_ = nullptr;
    return;
  } else {
    set_hierarchies(h[0]->get_model(), IMP::get_indexes(h));
  }
}
// for C++
Selection::Selection(Hierarchy h, std::string molname, int residue_index)
    : radius_(-1) {
  set_hierarchies(h.get_model(), ParticleIndexes(1, h.get_particle_index()));
  set_molecules(Strings(1, molname));
  set_residue_indexes(Ints(1, residue_index));
}

void Selection::set_hierarchies(Model *m, const ParticleIndexes &pi) {
  m_ = m;
  h_ = pi;
  for (unsigned int i = 0; i < pi.size(); ++i) {
    Hierarchy h(m_, pi[i]);
    IMP_USAGE_CHECK(h.get_is_valid(true),
                    "Hierarchy " << h << " is not valid.");
  }
}

namespace {
#define IMP_ATOM_SELECTION_PRED(Name, DataType, check)                         \
  class Name##SingletonPredicate : public SingletonPredicate {                 \
    DataType data_;                                                            \
                                                                               \
   public:                                                                     \
    Name##SingletonPredicate(const DataType &data,                             \
                             std::string name = #Name "SingletonPredicate%1%") \
        : SingletonPredicate(name), data_(data) {}                             \
    IMP_INDEX_SINGLETON_PREDICATE(Name##SingletonPredicate, check, {           \
      ModelObjectsTemp ret;                                                    \
      ret += IMP::get_particles(m, pi);                                        \
      return ret;                                                              \
    });                                                                        \
  };

bool get_is_residue_index_match(const Ints &data, Model *m, ParticleIndex pi) {
  if (Residue::particle_is_instance(m, pi)) {
    return std::binary_search(data.begin(), data.end(),
                              Residue(m, pi).get_index());
  }
  if (Fragment::particle_is_instance(m, pi)) {
    Ints cur = Fragment(m, pi).get_residue_indexes();
    Ints si;
    std::set_intersection(data.begin(), data.end(), cur.begin(), cur.end(),
                          std::back_inserter(si));
    return !si.empty();
  } else if (Domain::particle_is_instance(m, pi)) {
    int lb = Domain(m, pi).get_begin_index();
    int ub = Domain(m, pi).get_end_index();
    return std::lower_bound(data.begin(), data.end(), lb) !=
           std::upper_bound(data.begin(), data.end(), ub);
  }
  return false;
}

IMP_ATOM_SELECTION_PRED(ResidueIndex, Ints, {
  bool this_matches = get_is_residue_index_match(data_, m, pi);
  if (!this_matches) return 0;
  if (Hierarchy(m, pi).get_number_of_children() > 0) {
    // if any children match, push it until then
    for (unsigned int i = 0; i < Hierarchy(m, pi).get_number_of_children();
         ++i) {
      ParticleIndex cpi = Hierarchy(m, pi).get_child(i).get_particle_index();
      bool cur = get_is_residue_index_match(data_, m, cpi);
      if (cur) {
        return 0;
      }
    }
  }
  return 1;
});
IMP_ATOM_SELECTION_PRED(MoleculeName, Strings, {
  if (Molecule::particle_is_instance(m, pi)) {
    return std::binary_search(data_.begin(), data_.end(),
                              m->get_particle_name(pi));
  }
  return 0;
});
IMP_ATOM_SELECTION_PRED(ResidueType, ResidueTypes, {
  if (Residue::particle_is_instance(m, pi)) {
    return std::binary_search(data_.begin(), data_.end(),
                              Residue(m, pi).get_residue_type());
  }
  return 0;
});

IMP_ATOM_SELECTION_PRED(ChainID, std::string, {
  if (Chain::particle_is_instance(m, pi)) {
    return std::binary_search(data_.begin(), data_.end(),
                              Chain(m, pi).get_id());
  }
  return 0;
});
IMP_ATOM_SELECTION_PRED(AtomType, AtomTypes, {
  if (Atom::particle_is_instance(m, pi)) {
    return std::binary_search(data_.begin(), data_.end(),
                              Atom(m, pi).get_atom_type());
  }
  return 0;
});
IMP_ATOM_SELECTION_PRED(DomainName, Strings, {
  if (Domain::particle_is_instance(m, pi)) {
    return std::binary_search(data_.begin(), data_.end(),
                              m->get_particle_name(pi));
  }
  return 0;
});
IMP_ATOM_SELECTION_PRED(CopyIndex, Ints, {
  if (Copy::particle_is_instance(m, pi)) {
    return std::binary_search(data_.begin(), data_.end(),
                              Copy(m, pi).get_copy_index());
  }
  return 0;
});
IMP_ATOM_SELECTION_PRED(Type, core::ParticleTypes, {
  if (core::Typed::particle_is_instance(m, pi)) {
    return std::binary_search(data_.begin(), data_.end(),
                              core::Typed(m, pi).get_type());
  }
  return 0;
});
#define IMP_ATOM_SELECTION_MATCH_TYPE(Type, type, UCTYPE)          \
  if(Type::particle_is_instance(m, pi)) {                          \
    return std::binary_search(data_.begin(), data_.end(), UCTYPE); \
  }

IMP_ATOM_SELECTION_PRED(HierarchyType, Ints, {
  IMP_FOREACH_HIERARCHY_TYPE_STATEMENTS(IMP_ATOM_SELECTION_MATCH_TYPE);
  return 0;
});

bool get_is_terminus(Model *m, ParticleIndex pi, int t) {
  if (Atom::particle_is_instance(m, pi)) {
    // ignore order with atoms
    Atom a(m, pi);
    if (t == Selection::C && a.get_atom_type() != AT_C) {
      return false;
    }
    if (t == Selection::N && a.get_atom_type() != AT_N) {
      return false;
    }
  }
  Hierarchy cur(m, pi);
  Hierarchy p = cur.get_parent();
  if (!p) return true;
  if (!IMP::atom::Chain::particle_is_instance(p) &&
      !IMP::atom::Molecule::particle_is_instance(p)) {
    return get_is_terminus(m, p.get_particle_index(), t);
  }
  unsigned int i = cur.get_child_index();
  IMP_INTERNAL_CHECK(p.get_child(i) == cur, "Cur isn't the ith child");
  if (t == Selection::C && i + 1 != p.get_number_of_children()) {
    return false;
  }
  if (t == Selection::N && i != 0) {
    return false;
  }
  return true;
}

IMP_ATOM_SELECTION_PRED(Terminus, Int, {
  Hierarchy cur(m, pi);
  if (cur.get_number_of_children() > 0)
    return 0;
  else
    return get_is_terminus(m, pi, data_);
});

}
std::pair<boost::dynamic_bitset<>, ParticleIndexes> Selection::search(
    Model *m, ParticleIndex pi, boost::dynamic_bitset<> parent) const {
  IMP_LOG_VERBOSE("Searching " << m->get_particle_name(pi) << " missing "
                               << parent.count() << std::endl);
  for (unsigned int i = 0; i < predicates_.size(); ++i) {
    if (parent[i]) {
      if (predicates_[i]->get_value_index(m, pi)) {
        parent.reset(i);
      }
    }
  }
  Hierarchy cur(m, pi);
  ParticleIndexes children;
  bool children_covered = true;
  boost::dynamic_bitset<> ret = parent;
  for (unsigned int i = 0; i < cur.get_number_of_children(); ++i) {
    std::pair<boost::dynamic_bitset<>, ParticleIndexes> curr =
        search(m, cur.get_child(i).get_particle_index(), parent);
    ret &= curr.first;
    if (curr.first.none()) {
      if (curr.second.empty()) {
        children_covered = false;
      } else if (curr.first.none()) {
        children += curr.second;
      }
    }
  }
  IMP_LOG_VERBOSE("Found " << m->get_particle_name(pi) << " missing "
                           << parent.count() << " selected " << children.size()
                           << " missing children " << children_covered
                           << std::endl);
  if (ret.none()) {
    if (children_covered && !children.empty()) {
      return std::make_pair(ret, children);
    } else {
      if (core::XYZR::particle_is_instance(m, pi)) {
        if (core::XYZR(m, pi).get_radius() > radius_) {
          return std::make_pair(ret, ParticleIndexes(1, pi));
        }
      }
    }
  }
  return std::make_pair(ret, ParticleIndexes());
}

ParticlesTemp Selection::get_selected_particles() const {
  return IMP::get_particles(m_, get_selected_particle_indexes());
}

ParticleIndexes Selection::get_selected_particle_indexes() const {
  if (h_.empty()) return ParticleIndexes();
  for (unsigned int i = 0; i < predicates_.size(); ++i) {
    predicates_[i]->set_was_used(true);
  }
  ParticleIndexes ret;
  // Dynamic bitsets support .none(), but not .all(), so start with all
  // true.
  int sz = predicates_.size();
  boost::dynamic_bitset<> base(sz);
  base.set();
  IMP_LOG_TERSE("Processing selection on "
                << h_ << " with predicates " << predicates_ << std::endl);
  for (unsigned int i = 0; i < h_.size(); ++i) {
    ret += search(m_, h_[i], base).second;
  }
  return ret;
}

Hierarchies Selection::get_hierarchies() const {
  Hierarchies ret(h_.size());
  for (unsigned int i = 0; i < h_.size(); ++i) {
    ret[i] = Hierarchy(m_, h_[i]);
  }
  return ret;
}

void Selection::set_molecules(Strings mols) {
  std::sort(mols.begin(), mols.end());
  predicates_.push_back(new MoleculeNameSingletonPredicate(mols));
}
void Selection::set_terminus(Terminus t) {
  predicates_.push_back(new TerminusSingletonPredicate(t));
}
void Selection::set_chains(std::string chains) {
  std::sort(chains.begin(), chains.end());
  predicates_.push_back(new ChainIDSingletonPredicate(chains));
}
void Selection::set_residue_indexes(Ints indexes) {
  std::sort(indexes.begin(), indexes.end());
  predicates_.push_back(new ResidueIndexSingletonPredicate(indexes));
}
void Selection::set_atom_types(AtomTypes types) {
  std::sort(types.begin(), types.end());
  predicates_.push_back(new AtomTypeSingletonPredicate(types));
}
void Selection::set_residue_types(ResidueTypes types) {
  std::sort(types.begin(), types.end());
  predicates_.push_back(new ResidueTypeSingletonPredicate(types));
}
void Selection::set_domains(Strings names) {
  std::sort(names.begin(), names.end());
  predicates_.push_back(new DomainNameSingletonPredicate(names));
}
void Selection::set_molecule(std::string mol) {
  set_molecules(Strings(1, mol));
}
void Selection::set_chain(char c) { set_chains(std::string(1, c)); }
void Selection::set_residue_index(int i) { set_residue_indexes(Ints(1, i)); }
void Selection::set_atom_type(AtomType types) {
  set_atom_types(AtomTypes(1, types));
}
void Selection::set_residue_type(ResidueType type) {
  set_residue_types(ResidueTypes(1, type));
}
void Selection::set_domain(std::string name) { set_domains(Strings(1, name)); }
void Selection::set_copy_index(unsigned int copy) {
  set_copy_indexes(Ints(1, copy));
}
void Selection::set_copy_indexes(Ints copies) {
  std::sort(copies.begin(), copies.end());
  predicates_.push_back(new CopyIndexSingletonPredicate(copies));
}
void Selection::set_particle_type(core::ParticleType t) {
  set_particle_types(core::ParticleTypes(1, t));
}
void Selection::set_particle_types(core::ParticleTypes t) {
  std::sort(t.begin(), t.end());
  predicates_.push_back(new TypeSingletonPredicate(t));
}
void Selection::set_hierarchy_types(Ints types) {
  std::sort(types.begin(), types.end());
  predicates_.push_back(new HierarchyTypeSingletonPredicate(types));
}

void Selection::show(std::ostream &out) const { out << "Selection on " << h_; }

namespace {
template <class PS>
Restraint *create_distance_restraint(const Selection &n0, const Selection &n1,
                                     PS *ps, std::string name) {
  ParticlesTemp p0 = n0.get_selected_particles();
  ParticlesTemp p1 = n1.get_selected_particles();
  IMP_IF_CHECK(USAGE) {
    IMP::base::set<Particle *> all(p0.begin(), p0.end());
    all.insert(p1.begin(), p1.end());
    IMP_USAGE_CHECK(all.size() == p0.size() + p1.size(),
                    "The two selections cannot overlap.");
  }
  Pointer<Restraint> ret;
  IMP_USAGE_CHECK(!p0.empty(),
                  "Selection " << n0 << " does not refer to any particles.");
  IMP_USAGE_CHECK(!p1.empty(),
                  "Selection " << n1 << " does not refer to any particles.");
  if (p1.size() == 1 && p0.size() == 1) {
    IMP_LOG_TERSE("Creating distance restraint between "
                  << p0[0]->get_name() << " and " << p1[0]->get_name()
                  << std::endl);
    ret = IMP::create_restraint(ps, ParticlePair(p0[0], p1[0]), name);
  } else {
    IMP_LOG_TERSE("Creating distance restraint between " << n0 << " and " << n1
                                                         << std::endl);
    /*if (p0.size()+p1.size() < 100) {
      ret=new core::KClosePairsRestraint(ps,
                                         p0, p1, 1,
                                         "Atom k distance restraint %1%");
                                         } else {*/
    Pointer<core::TableRefiner> r = new core::TableRefiner();
    r->add_particle(p0[0], p0);
    r->add_particle(p1[0], p1);
    IMP_NEW(core::KClosePairsPairScore, nps, (ps, r, 1));
    ret = IMP::create_restraint(nps.get(), ParticlePair(p0[0], p1[0]), name);
    //}
  }
  return ret.release();
}
}

Restraint *create_distance_restraint(const Selection &n0, const Selection &n1,
                                     double x0, double k, std::string name) {
  IMP_NEW(core::HarmonicSphereDistancePairScore, ps, (x0, k));
  return create_distance_restraint(n0, n1, ps.get(), name);
}

Restraint *create_connectivity_restraint(const Selections &s, double x0,
                                         double k, std::string name) {
  if (s.size() < 2) return nullptr;
  if (s.size() == 2) {
    IMP_NEW(core::HarmonicUpperBoundSphereDistancePairScore, ps, (x0, k));
    Restraint *r = create_distance_restraint(s[0], s[1], ps.get(), name);
    return r;
  } else {
    unsigned int max = 0;
    for (unsigned int i = 0; i < s.size(); ++i) {
      max = std::max(
          static_cast<unsigned int>(s[i].get_selected_particles().size()), max);
    }
    if (max == 1) {
      // special case all singletons
      ParticlesTemp particles;
      for (unsigned int i = 0; i < s.size(); ++i) {
        IMP_USAGE_CHECK(!s[i].get_selected_particles().empty(),
                        "No particles selected");
        particles.push_back(s[i].get_selected_particles()[0]);
      }
      IMP_NEW(core::HarmonicUpperBoundSphereDistancePairScore, hdps, (x0, k));
      IMP_NEW(container::ListSingletonContainer, lsc, (particles));
      IMP_NEW(container::ConnectingPairContainer, cpc, (lsc, 0));
      Pointer<Restraint> cr =
          container::create_restraint(hdps.get(), cpc.get(), name);
      return cr.release();
    } else {
      IMP_NEW(core::TableRefiner, tr, ());
      ParticlesTemp rps;
      for (unsigned int i = 0; i < s.size(); ++i) {
        ParticlesTemp ps = s[i].get_selected_particles();
        IMP_USAGE_CHECK(
            !ps.empty(),
            "Selection " << s[i] << " does not contain any particles.");
        tr->add_particle(ps[0], ps);
        rps.push_back(ps[0]);
      }
      IMP_NEW(core::HarmonicUpperBoundSphereDistancePairScore, hdps, (x0, k));
      Pointer<PairScore> ps;
      IMP_LOG_TERSE("Using closest pair score." << std::endl);
      ps = new core::KClosePairsPairScore(hdps, tr);
      IMP_NEW(IMP::internal::InternalListSingletonContainer, lsc,
              (rps[0]->get_model(), "Connectivity particles"));
      lsc->set(IMP::internal::get_index(rps));
      IMP_NEW(core::ConnectivityRestraint, cr, (ps, lsc));
      cr->set_name(name);
      return cr.release();
    }
  }
}

Restraint *create_connectivity_restraint(const Selections &s, double k,
                                         std::string name) {
  return create_connectivity_restraint(s, 0, k, name);
}

Restraint *create_internal_connectivity_restraint(const Selection &ss,
                                                  double x0, double k,
                                                  std::string name) {
  ParticlesTemp s = ss.get_selected_particles();
  if (s.size() < 2) return nullptr;
  if (s.size() == 2) {
    IMP_NEW(core::HarmonicUpperBoundSphereDistancePairScore, ps, (x0, k));
    IMP_NEW(core::PairRestraint, r, (ps, ParticlePair(s[0], s[1]), name));
    return r.release();
  } else {
    IMP_NEW(core::HarmonicUpperBoundSphereDistancePairScore, hdps, (x0, k));
    IMP_NEW(container::ListSingletonContainer, lsc, (s));
    IMP_NEW(container::ConnectingPairContainer, cpc, (lsc, 0));
    Pointer<Restraint> cr =
        container::create_restraint(hdps.get(), cpc.get(), name);
    return cr.release();
  }
}

Restraint *create_internal_connectivity_restraint(const Selection &s, double k,
                                                  std::string name) {
  return create_internal_connectivity_restraint(s, 0, k, name);
}

Restraint *create_excluded_volume_restraint(const Selections &ss) {
  ParticlesTemp ps;
  for (unsigned int i = 0; i < ss.size(); ++i) {
    ParticlesTemp cps = ss[i].get_selected_particles();
    IMP_IF_LOG(TERSE) {
      IMP_LOG_TERSE("Found ");
      for (unsigned int i = 0; i < cps.size(); ++i) {
        IMP_LOG_TERSE(cps[i]->get_name() << " ");
      }
      IMP_LOG_TERSE(std::endl);
    }
    ps.insert(ps.end(), cps.begin(), cps.end());
  }
  IMP_NEW(IMP::internal::InternalListSingletonContainer, lsc,
          (ps[0]->get_model(), "Hierarchy EV particles"));
  lsc->set(IMP::internal::get_index(ps));
  IMP_NEW(core::ExcludedVolumeRestraint, evr, (lsc));
  evr->set_name("Hierarchy EV");
  return evr.release();
}

Restraint *create_excluded_volume_restraint(const Hierarchies &hs,
                                            double resolution) {
  Selections ss;
  for (unsigned int i = 0; i < hs.size(); ++i) {
    Selection s(hs[i]);
    s.set_target_radius(resolution);
    ss.push_back(s);
  }
  return create_excluded_volume_restraint(ss);
}

core::XYZR create_cover(const Selection &s, std::string name) {
  if (name.empty()) {
    name = "atom cover";
  }
  ParticlesTemp ps = s.get_selected_particles();
  IMP_USAGE_CHECK(!ps.empty(), "No particles selected.");
  Particle *p = new Particle(ps[0]->get_model());
  p->set_name(name);
  core::RigidBody rb;
  for (unsigned int i = 0; i < ps.size(); ++i) {
    if (core::RigidMember::particle_is_instance(ps[i])) {
      if (!rb) {
        rb = core::RigidMember(ps[i]).get_rigid_body();
      } else {
        if (rb != core::RigidMember(ps[i]).get_rigid_body()) {
          rb = core::RigidBody();
          break;
        }
      }
    }
  }

  if (rb) {
    algebra::Sphere3Ds ss;
    for (unsigned int i = 0; i < ps.size(); ++i) {
      ss.push_back(core::XYZR(ps[i]).get_sphere());
    }
    algebra::Sphere3D s = algebra::get_enclosing_sphere(ss);
    core::XYZR d = core::XYZR::setup_particle(p, s);
    rb.add_member(d);
    return d;
  } else {
    core::Cover c = core::Cover::setup_particle(p, core::XYZRs(ps));
    return c;
  }
}

void transform(Hierarchy h, const algebra::Transformation3D &tr) {
  base::Vector<Hierarchy> stack;
  stack.push_back(h);
  do {
    Hierarchy c = stack.back();
    stack.pop_back();
    if (core::RigidBody::particle_is_instance(c)) {
      core::transform(core::RigidBody(c), tr);
    } else if (core::XYZ::particle_is_instance(c)) {
      core::transform(core::XYZ(c), tr);
    }
    for (unsigned int i = 0; i < c.get_number_of_children(); ++i) {
      stack.push_back(c.get_child(i));
    }
  } while (!stack.empty());
}

double get_mass(const Selection &h) {
  IMP_FUNCTION_LOG;
  double ret = 0;
  ParticlesTemp ps = h.get_selected_particles();
  for (unsigned int i = 0; i < ps.size(); ++i) {
    ret += Mass(ps[i]).get_mass();
  }
  return ret;
}

#ifdef IMP_ALGEBRA_USE_IMP_CGAL

namespace {
algebra::Sphere3Ds get_representation(Selection h) {
  ParticlesTemp leaves = h.get_selected_particles();
  algebra::Sphere3Ds ret(leaves.size());
  for (unsigned int i = 0; i < leaves.size(); ++i) {
    ret[i] = core::XYZR(leaves[i]).get_sphere();
  }
  return ret;
}
}

double get_volume(const Selection &h) {
  IMP_FUNCTION_LOG;
  IMP_USAGE_CHECK(!h.get_selected_particles().empty(),
                  "No particles selected.");
  return algebra::get_surface_area_and_volume(get_representation(h)).second;
}

double get_surface_area(const Selection &h) {
  IMP_FUNCTION_LOG;
  IMP_USAGE_CHECK(!h.get_selected_particles().empty(),
                  "No particles selected.");
  return algebra::get_surface_area_and_volume(get_representation(h)).first;
}
#endif

double get_radius_of_gyration(const Selection &h) {
  IMP_FUNCTION_LOG;
  IMP_USAGE_CHECK(!h.get_selected_particles().empty(),
                  "No particles selected.");
  return get_radius_of_gyration(h.get_selected_particles());
}

HierarchyTree get_hierarchy_tree(Hierarchy h) {
  HierarchyTree ret;
  typedef boost::property_map<HierarchyTree, boost::vertex_name_t>::type VM;
  VM vm = boost::get(boost::vertex_name, ret);
  base::Vector<std::pair<int, Hierarchy> > queue;
  int v = boost::add_vertex(ret);
  vm[v] = h;
  queue.push_back(std::make_pair(v, h));
  do {
    int v = queue.back().first;
    Hierarchy c = queue.back().second;
    queue.pop_back();
    for (unsigned int i = 0; i < c.get_number_of_children(); ++i) {
      int vc = boost::add_vertex(ret);
      vm[vc] = c.get_child(i);
      boost::add_edge(v, vc, ret);
      queue.push_back(std::make_pair(vc, c.get_child(i)));
    }
  } while (!queue.empty());
  return ret;
}

display::Geometries SelectionGeometry::get_components() const {
  display::Geometries ret;
  ParticlesTemp ps = res_.get_selected_particles();
  for (unsigned int i = 0; i < ps.size(); ++i) {
    if (components_.find(ps[i]) == components_.end()) {
      IMP_NEW(HierarchyGeometry, g, (atom::Hierarchy(ps[i])));
      components_[ps[i]] = g;
      g->set_name(get_name());
      if (get_has_color()) {
        components_[ps[i]]->set_color(get_color());
      }
    }
    ret.push_back(components_.find(ps[i])->second);
  }
  return ret;
}

Hierarchies get_leaves(const Selection &h) {
  Hierarchies ret;
  ParticlesTemp ps = h.get_selected_particles();
  for (unsigned int i = 0; i < ps.size(); ++i) {
    ret += get_leaves(Hierarchy(ps[i]));
  }
  return ret;
}
namespace {
Ints get_tree_residue_indexes(Hierarchy h) {
  if (Residue::particle_is_instance(h)) {
    return Ints(1, Residue(h).get_index());
  }
  Ints ret;
  if (Domain::particle_is_instance(h)) {
    for (int i = Domain(h).get_begin_index(); i < Domain(h).get_end_index();
         ++i) {
      ret.push_back(i);
    }
  } else if (Fragment::particle_is_instance(h)) {
    Ints cur = Fragment(h).get_residue_indexes();
    ret.insert(ret.end(), cur.begin(), cur.end());
  }
  if (ret.empty()) {
    if (h.get_number_of_children() > 0) {
      for (unsigned int i = 0; i < h.get_number_of_children(); ++i) {
        Ints cur = get_tree_residue_indexes(h.get_child(0));
        ret.insert(ret.end(), cur.begin(), cur.end());
      }
    }
  }
  return ret;
}
}

Ints get_residue_indexes(Hierarchy h) {
  do {
    Ints ret = get_tree_residue_indexes(h);
    if (!ret.empty()) return ret;
  } while ((h = h.get_parent()));
  IMP_THROW("Hierarchy " << h << " has no residue index.", ValueException);
}
ResidueType get_residue_type(Hierarchy h) {
  do {
    if (Residue::particle_is_instance(h)) {
      return Residue(h).get_residue_type();
    }
  } while ((h = h.get_parent()));
  IMP_THROW("Hierarchy " << h << " has no residue type.", ValueException);
}
int get_chain_id(Hierarchy h) {
  Chain c = get_chain(h);
  if (!c) {
    IMP_THROW("Hierarchy " << h << " has no chain.", ValueException);
  } else {
    return c.get_id();
  }
}
AtomType get_atom_type(Hierarchy h) {
  do {
    if (Atom::particle_is_instance(h)) {
      return Atom(h).get_atom_type();
    }
  } while ((h = h.get_parent()));
  IMP_THROW("Hierarchy " << h << " has no atom type.", ValueException);
}
std::string get_domain_name(Hierarchy h) {
  do {
    if (Domain::particle_is_instance(h)) {
      return Domain(h)->get_name();
    }
  } while ((h = h.get_parent()));
  IMP_THROW("Hierarchy " << h << " has no domain name.", ValueException);
}
int get_copy_index(Hierarchy h) {
  do {
    if (Copy::particle_is_instance(h)) {
      return Copy(h).get_copy_index();
    }
  } while ((h = h.get_parent()));
  IMP_THROW("Hierarchy " << h << " has number.", ValueException);
}
std::string get_molecule_name(Hierarchy h) {
  do {
    if (Molecule::particle_is_instance(h)) {
      return h->get_name();
    }
  } while ((h = h.get_parent()));
  IMP_THROW("Hierarchy " << h << " has no residue index.", ValueException);
}
IMPATOM_END_NAMESPACE
