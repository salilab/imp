/**
 *  \file IMP/atom/Selection.cpp
 *  \brief Select a subset of a hierarchy.
 *
 *  Copyright 2007-2017 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/atom/hierarchy_tools.h"
#include <IMP/SingletonContainer.h>
#include <IMP/algebra/vector_generators.h>
#include <IMP/atom/Atom.h>
#include <IMP/atom/Chain.h>
#include <IMP/atom/Copy.h>
#include <IMP/atom/Domain.h>
#include <IMP/atom/Fragment.h>
#include <IMP/atom/Mass.h>
#include <IMP/atom/Molecule.h>
#include <IMP/atom/Residue.h>
#include <IMP/atom/Representation.h>
#include <IMP/atom/State.h>
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
#include <boost/unordered_set.hpp>
IMPATOM_BEGIN_NAMESPACE

namespace {
  inline std::ostream &show_predicate(internal::SelectionPredicate *p,
                                      std::ostream &out = std::cout) {
    IMP_PRINT_TREE(out, internal::SelectionPredicate*, p,
                   n->get_number_of_children(), n->get_child, n->show(out));
    return out;
  }

  internal::SelectionPredicate::MatchType get_match_return(bool v) {
    if (v) return internal::SelectionPredicate::MATCH_WITH_CHILDREN;
    else return internal::SelectionPredicate::MISMATCH;
  }

  //! Reverse a match
  class NotSelectionPredicate : public internal::SelectionPredicate {
    Pointer<SelectionPredicate> predicate_;
  public:
    NotSelectionPredicate(SelectionPredicate *predicate,
                          std::string name = "NotSelectionPredicate%1%")
          : internal::SelectionPredicate(name), predicate_(predicate) {}

    virtual unsigned get_number_of_children() const IMP_OVERRIDE {
      return 1;
    }
    virtual SelectionPredicate *get_child(unsigned) const IMP_OVERRIDE {
      return predicate_;
    }

    virtual SelectionPredicate *clone(bool) IMP_OVERRIDE {
      set_was_used(true);
      return new NotSelectionPredicate(predicate_->clone(false));
    }

    virtual int setup_bitset(int index) IMP_OVERRIDE {
      index = internal::SelectionPredicate::setup_bitset(index);
      /* Set index for subpredicate */
      index = predicate_->setup_bitset(index);
      return index;
    }

    virtual ModelObjectsTemp do_get_inputs(
          Model *m, const ParticleIndexes &pis) const
          IMP_OVERRIDE {
      return IMP::get_particles(m, pis);
    }

    virtual MatchType do_get_value_index(Model *m, ParticleIndex pi,
                                         boost::dynamic_bitset<> &bs)
                                             const IMP_OVERRIDE {
      MatchType v = predicate_->get_value_index(m, pi, bs);
      switch(v) {
      case NO_MATCH:
        /* If the subpredicate doesn't match, we do... but we don't want to
           cache this result, since child particles may match */
        return MATCH_SELF_ONLY;
      case MATCH_WITH_CHILDREN:
        return MISMATCH;
      case MISMATCH:
        return MATCH_WITH_CHILDREN;
      default:
        /* The opposite of a non-cached match (MATCH_SELF_ONLY)
           is a "no match" */
        return NO_MATCH;
      }
    }

    IMP_OBJECT_METHODS(NotSelectionPredicate);
  };

  //! Match only if every subpredicate matches (or there are no subpredicates)
  class AndSelectionPredicate : public internal::ListSelectionPredicate {
  public:
    AndSelectionPredicate(std::string name = "AndSelectionPredicate%1%")
          : internal::ListSelectionPredicate(name) {}

    virtual SelectionPredicate *clone(bool toplevel) IMP_OVERRIDE {
      set_was_used(true);
      // If only one predicate and we're not the top level,
      // no need to keep ourself around
      if (!toplevel && predicates_.size() == 1) {
        return predicates_[0]->clone(false);
      } else {
        Pointer<ListSelectionPredicate> a = new AndSelectionPredicate();
        clone_predicates(a);
        return a.release();
      }
    }

    virtual MatchType do_get_value_index(Model *m, ParticleIndex pi,
                                         boost::dynamic_bitset<> &bs)
                                             const IMP_OVERRIDE {
      // Empty list matches everything
      if (predicates_.size() == 0) {
        return MATCH_WITH_CHILDREN;
      }
      bool no_match = false, cached_match = true;
      IMP_FOREACH(internal::SelectionPredicate *p, predicates_) {
        MatchType v = p->get_value_index(m, pi, bs);
        if (v == MISMATCH) {
          return MISMATCH;
        } else if (v == NO_MATCH) {
          no_match = true;
        } else if (v == MATCH_SELF_ONLY) {
          cached_match = false;
        }
      }
      if (no_match) {
        return NO_MATCH;
      } else if (cached_match) {
        return MATCH_WITH_CHILDREN;
      } else {
        return MATCH_SELF_ONLY;
      }
    }

    IMP_OBJECT_METHODS(AndSelectionPredicate);
  };

  //! Match if any subpredicate matches (or there are no subpredicates)
  class OrSelectionPredicate : public internal::ListSelectionPredicate {
  public:
    OrSelectionPredicate(std::string name = "OrSelectionPredicate%1%")
          : internal::ListSelectionPredicate(name) {}

    virtual SelectionPredicate *clone(bool) IMP_OVERRIDE {
      set_was_used(true);
      Pointer<ListSelectionPredicate> a = new OrSelectionPredicate();
      clone_predicates(a);
      return a.release();
    }

    virtual MatchType do_get_value_index(Model *m, ParticleIndex pi,
                                         boost::dynamic_bitset<> &bs)
                                             const IMP_OVERRIDE {
      // Empty list matches everything
      if (predicates_.size() == 0) {
        return MATCH_WITH_CHILDREN;
      }
      bool no_match = false, no_cached_match = false;
      IMP_FOREACH(internal::SelectionPredicate *p, predicates_) {
        MatchType v = p->get_value_index(m, pi, bs);
        if (v == MATCH_WITH_CHILDREN) {
          return MATCH_WITH_CHILDREN;
        } else if (v == NO_MATCH) {
          no_match = true;
        } else if (v == MATCH_SELF_ONLY) {
          no_cached_match = true;
        }
      }
      if (no_cached_match) {
        return MATCH_SELF_ONLY;
      } else if (no_match) {
        return NO_MATCH;
      } else {
        return MISMATCH;
      }
    }

    IMP_OBJECT_METHODS(OrSelectionPredicate);
  };

  //! Match if an odd number of subpredicates match (or there are none)
  class XorSelectionPredicate : public internal::ListSelectionPredicate {
  public:
    XorSelectionPredicate(std::string name = "XorSelectionPredicate%1%")
          : internal::ListSelectionPredicate(name) {}

    virtual SelectionPredicate *clone(bool) IMP_OVERRIDE {
      set_was_used(true);
      Pointer<ListSelectionPredicate> a = new XorSelectionPredicate();
      clone_predicates(a);
      return a.release();
    }

    virtual MatchType do_get_value_index(Model *m, ParticleIndex pi,
                                         boost::dynamic_bitset<> &bs)
                                             const IMP_OVERRIDE {
      // Empty list matches everything
      if (predicates_.size() == 0) {
        return MATCH_WITH_CHILDREN;
      }
      bool no_match = false, match = false;
      IMP_FOREACH(internal::SelectionPredicate *p, predicates_) {
        MatchType v = p->get_value_index(m, pi, bs);
        if (v == MATCH_WITH_CHILDREN || v == MATCH_SELF_ONLY) {
          match = !match;
        }
        if (v == NO_MATCH || v == MATCH_SELF_ONLY) {
          no_match = true;
        }
      }
      if (match) {
        /* Note that we *don't* want to cache matches from this predicate,
           since a child might match another subpredicate and so flip this
           predicate's match bit */
        return MATCH_SELF_ONLY;
      } else if (no_match) {
        return NO_MATCH;
      } else {
        return MISMATCH;
      }
    }

    IMP_OBJECT_METHODS(XorSelectionPredicate);
  };
}

void Selection::init_predicate() {
  and_predicate_ = new AndSelectionPredicate();
  and_predicate_->set_was_used(true);
  predicate_ = and_predicate_;
}

Selection::Selection() : resolution_(0),representation_type_(BALLS) {
  init_predicate();
  m_ = nullptr;
}

Selection Selection::create_clone() {
  Selection s;
  s.m_ = m_;
  s.h_ = h_;
  s.resolution_ = resolution_;
  s.representation_type_ = representation_type_;
  s.predicate_ = dynamic_cast<internal::ListSelectionPredicate*>
                         (predicate_->clone(true));
  IMP_INTERNAL_CHECK(s.predicate_, "Clone failed");
  s.and_predicate_ = nullptr;
  return s;
}

Selection::Selection(Particle *h) : resolution_(0),representation_type_(BALLS) {
  init_predicate();
  set_hierarchies(h->get_model(), ParticleIndexes(1, h->get_index()));
}
Selection::Selection(Hierarchy h) : resolution_(0),representation_type_(BALLS) {
  init_predicate();
  set_hierarchies(h.get_model(),
                  ParticleIndexes(1, h.get_particle_index()));
}
Selection::Selection(Model *m, const ParticleIndexes &pis)
    : resolution_(0),representation_type_(BALLS) {
  init_predicate();
  set_hierarchies(m, pis);
}
Selection::Selection(const Hierarchies &h) :
  resolution_(0),representation_type_(BALLS) {
  init_predicate();
  if (h.empty()) {
    m_ = nullptr;
    return;
  } else {
    set_hierarchies(h[0].get_model(), IMP::internal::get_index(h));
  }
}
Selection::Selection(const ParticlesTemp &h) :
  resolution_(0),representation_type_(BALLS) {
  init_predicate();
  if (h.empty()) {
    m_ = nullptr;
    return;
  } else {
    set_hierarchies(h[0]->get_model(), IMP::get_indexes(h));
  }
}
// for C++
Selection::Selection(Hierarchy h, std::string molname, int residue_index)
    : resolution_(0),representation_type_(BALLS) {
  init_predicate();
  set_hierarchies(h.get_model(),
                  ParticleIndexes(1, h.get_particle_index()));
  set_molecules(Strings(1, molname));
  set_residue_indexes(Ints(1, residue_index));
}

void Selection::set_hierarchies(Model *m,
                                const ParticleIndexes &pi) {
  m_ = m;
  h_ = pi;
  for (unsigned int i = 0; i < pi.size(); ++i) {
    Hierarchy h(m_, pi[i]);
    IMP_USAGE_CHECK(h.get_is_valid(true), "Hierarchy " << h
                                                       << " is not valid.");
  }
}

namespace {
#define IMP_ATOM_SELECTION_PRED(Name, DataType, check)                         \
  class Name##SelectionPredicate : public internal::SelectionPredicate {       \
    DataType data_;                                                            \
                                                                               \
   public:                                                                     \
    Name##SelectionPredicate(const DataType &data,                             \
                             std::string name = #Name "SelectionPredicate%1%") \
        : internal::SelectionPredicate(name), data_(data) {}                   \
    virtual MatchType do_get_value_index(Model *m,                             \
                                   ParticleIndex pi,                           \
                                   boost::dynamic_bitset<> &)                  \
                                            const IMP_OVERRIDE {               \
      check;                                                                   \
    }                                                                          \
    virtual ModelObjectsTemp do_get_inputs(                            \
        Model *m, const ParticleIndexes &pis) const            \
        IMP_OVERRIDE {                                                         \
      return IMP::get_particles(m, pis);                                       \
    }                                                                          \
    IMP_OBJECT_METHODS(Name##SelectionPredicate);                              \
  };

bool get_is_residue_index_match(const Ints &data, Model *m,
                                ParticleIndex pi) {
  if (Residue::get_is_setup(m, pi)) {
    return std::binary_search(data.begin(), data.end(),
                              Residue(m, pi).get_index());
  }
  if (Fragment::get_is_setup(m, pi)) {
    Ints cur = Fragment(m, pi).get_residue_indexes();
    Ints si;
    std::set_intersection(data.begin(), data.end(), cur.begin(), cur.end(),
                          std::back_inserter(si));
    return !si.empty();
  } else if (Domain::get_is_setup(m, pi)) {
    IntRange ir = Domain(m, pi).get_index_range();
    return std::lower_bound(data.begin(), data.end(), ir.first) !=
           std::lower_bound(data.begin(), data.end(), ir.second);
  }
  return false;
}

IMP_ATOM_SELECTION_PRED(ResidueIndex, Ints, {
  bool this_matches = get_is_residue_index_match(data_, m, pi);
  if (!this_matches)
    return NO_MATCH;
  if (Hierarchy(m, pi).get_number_of_children() > 0) {
    // if any children match, push it until then
    for (unsigned int i = 0; i < Hierarchy(m, pi).get_number_of_children();
         ++i) {
      ParticleIndex cpi =
          Hierarchy(m, pi).get_child(i).get_particle_index();
      bool cur = get_is_residue_index_match(data_, m, cpi);
      if (cur) {
        return NO_MATCH;
      }
    }
  }
  return MATCH_WITH_CHILDREN;
});
IMP_ATOM_SELECTION_PRED(MoleculeName, Strings, {
  if (Molecule::get_is_setup(m, pi)) {
    return get_match_return(std::binary_search(data_.begin(), data_.end(),
                                               m->get_particle_name(pi)));
  }
  return NO_MATCH;
});
IMP_ATOM_SELECTION_PRED(ResidueType, ResidueTypes, {
  if (Residue::get_is_setup(m, pi)) {
    return get_match_return(std::binary_search(
        data_.begin(), data_.end(), Residue(m, pi).get_residue_type()));
  }
  return NO_MATCH;
});

IMP_ATOM_SELECTION_PRED(Element, Element, {
  if (Atom::get_is_setup(m, pi)) {
    return get_match_return(data_ == Atom(m, pi).get_element());
  }
  return NO_MATCH;
});

IMP_ATOM_SELECTION_PRED(ChainID, Strings, {
  if (Chain::get_is_setup(m, pi)) {
    return get_match_return(
        std::binary_search(data_.begin(), data_.end(), Chain(m, pi).get_id()));
  }
  return NO_MATCH;
});
IMP_ATOM_SELECTION_PRED(AtomType, AtomTypes, {
  if (Atom::get_is_setup(m, pi)) {
    return get_match_return(std::binary_search(data_.begin(), data_.end(),
                                               Atom(m, pi).get_atom_type()));
  }
  return NO_MATCH;
});
IMP_ATOM_SELECTION_PRED(DomainName, Strings, {
  if (Domain::get_is_setup(m, pi)) {
    return get_match_return(std::binary_search(data_.begin(), data_.end(),
                                               m->get_particle_name(pi)));
  }
  return NO_MATCH;
});
IMP_ATOM_SELECTION_PRED(CopyIndex, Ints, {
  if (Copy::get_is_setup(m, pi)) {
    return get_match_return(std::binary_search(data_.begin(), data_.end(),
                                               Copy(m, pi).get_copy_index()));
  }
  return NO_MATCH;
});
IMP_ATOM_SELECTION_PRED(StateIndex, Ints, {
  if (State::get_is_setup(m, pi)) {
    return get_match_return(std::binary_search(data_.begin(), data_.end(),
                                               State(m, pi).get_state_index()));
  }
  return NO_MATCH;
});
IMP_ATOM_SELECTION_PRED(Type, core::ParticleTypes, {
  if (core::Typed::get_is_setup(m, pi)) {
    if( std::binary_search(data_.begin(), data_.end(),
                           core::Typed(m, pi).get_type())) {
      return MATCH_WITH_CHILDREN;
    }
  }
  return NO_MATCH;
});
#define IMP_ATOM_SELECTION_MATCH_TYPE(Type, type, UCTYPE)       \
  if (Type::get_is_setup(m, pi)) {                              \
    if (std::binary_search(data_.begin(), data_.end(), UCTYPE)) \
      return MATCH_WITH_CHILDREN;                               \
    else                                                        \
      return NO_MATCH;                                          \
  }

IMP_ATOM_SELECTION_PRED(HierarchyType, Ints, {
  IMP_ATOM_FOREACH_HIERARCHY_TYPE_STATEMENTS(IMP_ATOM_SELECTION_MATCH_TYPE);
  return NO_MATCH;
});

bool get_is_terminus(Model *m, ParticleIndex pi, int t) {
  if (Atom::get_is_setup(m, pi)) {
    // Atoms can only be termini if the type matches
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
  // e.g. the terminal residue in a non-terminal fragment is *not* a terminus
  if (!IMP::atom::Chain::get_is_setup(p) &&
      !IMP::atom::Molecule::get_is_setup(p) &&
      !get_is_terminus(m, p.get_particle_index(), t)) {
    return false;
  }
  // Ignore order with atoms
  if (!Atom::get_is_setup(m, pi)) {
    unsigned int i = cur.get_child_index();
    IMP_INTERNAL_CHECK(p.get_child(i) == cur, "Cur isn't the ith child");
    if (t == Selection::C && i + 1 != p.get_number_of_children()) {
      return false;
    }
    if (t == Selection::N && i != 0) {
      return false;
    }
  }
  return true;
}

IMP_ATOM_SELECTION_PRED(Terminus, Int, {
  Hierarchy cur(m, pi);
  if (cur.get_number_of_children() > 0) {
    return NO_MATCH;
  } else if (get_is_terminus(m, pi, data_)) {
    return MATCH_WITH_CHILDREN;
  } else {
    return NO_MATCH;
  }
});

IMP_NAMED_TUPLE_2(ExpandResult, ExpandResults, bool, from_rep,
                  ParticleIndexes, indexes, );

ExpandResult expand_search(Model *m,
                           ParticleIndex pi,
                           double resolution,
                           RepresentationType representation_type) {
  // to handle representations
  ParticleIndexes idxs;
  bool from_rep = false;
  if (Representation::get_is_setup(m, pi)) {
    from_rep = true;
    if (resolution == ALL_RESOLUTIONS) {
      idxs = Representation(m, pi).get_representations(representation_type);
    }
    else {
      Hierarchy tmp = Representation(m, pi).get_representation(resolution,
                                                               representation_type);
      if (tmp) idxs.push_back(tmp);
    }
  }
  else {
    idxs.push_back(pi);
  }
  return ExpandResult(from_rep,idxs);
}

ExpandResults expand_children_search(Model *m,
                                     ParticleIndex pi,
                                     double resolution,
                                     RepresentationType representation_type) {
  Hierarchy h(m, pi);
  ExpandResults ret;
  IMP_FOREACH(Hierarchy c, h.get_children()) {
    ExpandResult r = expand_search(m, c, resolution, representation_type);
    if (r.get_indexes().size()>0) ret.push_back(r);
  }
  return ret;
}
}

Selection::SearchResult Selection::search(
    Model *m, ParticleIndex pi,
    boost::dynamic_bitset<> parent, bool with_representation,
    bool found_rep_node) const {
  IMP_FUNCTION_LOG;
  IMP_LOG_VERBOSE("Searching " << m->get_particle_name(pi) << std::endl);
  internal::SelectionPredicate::MatchType val
                 = predicate_->get_value_index(m, pi, parent);
  if (val == internal::SelectionPredicate::MISMATCH) {
    // nothing can match in this subtree
    return SearchResult(false, ParticleIndexes());
  } else if (val == internal::SelectionPredicate::MATCH_WITH_CHILDREN
             && !with_representation) {
    // terminate search without considering children, if we got a sure match
    // BUT make sure non-BALLS only acceptable if you found a rep in subtree
    if (representation_type_==BALLS || found_rep_node){
      return SearchResult(true, ParticleIndexes(1, pi));
    }
    else return SearchResult(false, ParticleIndexes());
  }
  Hierarchy cur(m, pi);
  ParticleIndexes children;
  ExpandResults cur_children =
    expand_children_search(m, pi, resolution_, representation_type_);
  bool children_covered = true;
  bool matched = (val == internal::SelectionPredicate::MATCH_WITH_CHILDREN
                  || val == internal::SelectionPredicate::MATCH_SELF_ONLY);
  IMP_FOREACH(ExpandResult chlist, cur_children) {
    found_rep_node |= chlist.get_from_rep();
    IMP_FOREACH(ParticleIndex ch, chlist.get_indexes()) {
      SearchResult curr = search(m, ch, parent, with_representation, found_rep_node);
      matched |= curr.get_match();
      if (curr.get_match()) {
        if (curr.get_indexes().empty()) {
          children_covered = false;
        } else {
          children += curr.get_indexes();
        }
      }
    }
  }
  if (matched) {
    IMP_LOG_VERBOSE("Matched " << m->get_particle_name(pi) << " with "
                               << children << " and " << children_covered
                               <<" found rep node: "<<found_rep_node
                               << std::endl);
    if (children_covered && !children.empty()) {
      return SearchResult(true, children);
    }
    else {
      if (representation_type_==BALLS || found_rep_node){
        return SearchResult(true, ParticleIndexes(1, pi));
      }
      else return SearchResult(false, ParticleIndexes());
    }
  }
  return SearchResult(false, ParticleIndexes());
}

ParticlesTemp
Selection::get_selected_particles(bool with_representation) const {
  return IMP::get_particles(m_,
                            get_selected_particle_indexes(with_representation));
}

ParticleIndexes
Selection::get_selected_particle_indexes(bool with_representation) const {
  if (h_.empty()) return ParticleIndexes();
  ParticleIndexes ret;
  // Dynamic bitsets support .none(), but not .all(), so start with all
  // true
  int sz = predicate_->setup_bitset(0);
  boost::dynamic_bitset<> base(sz);
  base.set();
  IMP_LOG_TERSE("Processing selection on " << h_ << " with predicates "
                << std::endl);
  IMP_LOG_WRITE(VERBOSE, show_predicate(predicate_, IMP_STREAM));
  IMP_FOREACH(ParticleIndex pi, h_) {
    ExpandResult res = expand_search(m_, pi, resolution_,
                                     representation_type_);
    IMP_FOREACH(ParticleIndex rpi, res.get_indexes()) {
      ret += search(m_, rpi, base, with_representation, res.get_from_rep()).get_indexes();
    }
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
  add_predicate(new MoleculeNameSelectionPredicate(mols));
}
void Selection::set_terminus(Terminus t) {
  add_predicate(new TerminusSelectionPredicate(t));
}
void Selection::set_element(Element e) {
  add_predicate(new ElementSelectionPredicate(e));
}
void Selection::set_chain_ids(Strings chains) {
  std::sort(chains.begin(), chains.end());
  add_predicate(new ChainIDSelectionPredicate(chains));
}
void Selection::set_residue_indexes(Ints indexes) {
  std::sort(indexes.begin(), indexes.end());
  add_predicate(new ResidueIndexSelectionPredicate(indexes));
}
void Selection::set_atom_types(AtomTypes types) {
  std::sort(types.begin(), types.end());
  add_predicate(new AtomTypeSelectionPredicate(types));
}
void Selection::set_residue_types(ResidueTypes types) {
  std::sort(types.begin(), types.end());
  add_predicate(new ResidueTypeSelectionPredicate(types));
}
void Selection::set_domains(Strings names) {
  std::sort(names.begin(), names.end());
  add_predicate(new DomainNameSelectionPredicate(names));
}
void Selection::set_molecule(std::string mol) {
  set_molecules(Strings(1, mol));
}
void Selection::set_chain_id(std::string c) { set_chain_ids(Strings(1, c)); }
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
  add_predicate(new CopyIndexSelectionPredicate(copies));
}
void Selection::set_state_indexes(Ints copies) {
  std::sort(copies.begin(), copies.end());
  add_predicate(new StateIndexSelectionPredicate(copies));
}
void Selection::set_particle_type(core::ParticleType t) {
  set_particle_types(core::ParticleTypes(1, t));
}
void Selection::set_particle_types(core::ParticleTypes t) {
  std::sort(t.begin(), t.end());
  add_predicate(new TypeSelectionPredicate(t));
}
void Selection::set_hierarchy_types(Ints types) {
  std::sort(types.begin(), types.end());
  add_predicate(new HierarchyTypeSelectionPredicate(types));
}

void Selection::set_intersection(const Selection &s) {
  IMP_USAGE_CHECK(h_ == s.h_,
              "Both Selections must be on the same Hierarchy or Hierarchies");
  if (!dynamic_cast<AndSelectionPredicate*>(predicate_.get())) {
    // Replace top-level predicate with a new AndSelectionPredicate, and make
    // both the existing top-level predicate and the other selection's predicate
    // children of it
    Pointer<internal::ListSelectionPredicate> p
                = new AndSelectionPredicate();
    p->add_predicate(predicate_);
    predicate_ = p;
  }
  predicate_->add_predicate(s.predicate_->clone(false));
}

void Selection::set_union(const Selection &s) {
  IMP_USAGE_CHECK(h_ == s.h_,
              "Both Selections must be on the same Hierarchy or Hierarchies");
  if (!dynamic_cast<OrSelectionPredicate*>(predicate_.get())) {
    // Replace top-level predicate with a new OrSelectionPredicate, and make
    // both the existing top-level predicate and the other selection's predicate
    // children of it
    Pointer<internal::ListSelectionPredicate> p
                = new OrSelectionPredicate();
    p->add_predicate(predicate_);
    predicate_ = p;
  }
  predicate_->add_predicate(s.predicate_->clone(false));
}

void Selection::set_symmetric_difference(const Selection &s) {
  IMP_USAGE_CHECK(h_ == s.h_,
              "Both Selections must be on the same Hierarchy or Hierarchies");
  if (!dynamic_cast<XorSelectionPredicate*>(predicate_.get())) {
    // Replace top-level predicate with a new XorSelectionPredicate, and make
    // both the existing top-level predicate and the other selection's predicate
    // children of it
    Pointer<internal::ListSelectionPredicate> p
                = new XorSelectionPredicate();
    p->add_predicate(predicate_);
    predicate_ = p;
  }
  predicate_->add_predicate(s.predicate_->clone(false));
}

void Selection::set_difference(const Selection &s) {
  IMP_USAGE_CHECK(h_ == s.h_,
              "Both Selections must be on the same Hierarchy or Hierarchies");
  if (!dynamic_cast<AndSelectionPredicate*>(predicate_.get())) {
    // Replace top-level predicate with a new AndSelectionPredicate, and make
    // both the existing top-level predicate and the other selection's predicate
    // (wrapped with a NotSelectionPredicate) children of it
    Pointer<internal::ListSelectionPredicate> p
                = new AndSelectionPredicate();
    p->add_predicate(predicate_);
    predicate_ = p;
  }
  predicate_->add_predicate(new NotSelectionPredicate
                                   (s.predicate_->clone(false)));
}

void Selection::add_predicate(internal::SelectionPredicate *p)
{
  // Take a reference to p so it gets freed if the usage check fails
  Pointer<internal::SelectionPredicate> pp(p);
  IMP_USAGE_CHECK(and_predicate_, "Cannot add predicates to Selection copies");
  and_predicate_->add_predicate(p);
}

void Selection::show(std::ostream &out) const { out << "Selection on " << h_; }

namespace {
template <class PS>
Restraint *create_distance_restraint(const Selection &n0, const Selection &n1,
                                     PS *ps, std::string name) {
  ParticlesTemp p0 = n0.get_selected_particles();
  ParticlesTemp p1 = n1.get_selected_particles();
  IMP_IF_CHECK(USAGE) {
    boost::unordered_set<Particle *> all(p0.begin(), p0.end());
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
    ret = IMP::create_restraint(
        nps.get(), ParticlePair(p0[0], p1[0]), name);
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
  IMP_IF_CHECK(USAGE) {
    boost::unordered_set<ParticleIndex> used;
    IMP_FOREACH(const Selection & sel, s) {
      ParticleIndexes cur = sel.get_selected_particle_indexes();
      int old = used.size();
      IMP_UNUSED(old);
      used.insert(cur.begin(), cur.end());
      IMP_USAGE_CHECK(used.size() == old + cur.size(),
                      "Input selections are not disjoint. This won't work.");
    }
  }

  if (s.size() < 2) return nullptr;
  if (s.size() == 2) {
    IMP_NEW(core::HarmonicUpperBoundSphereDistancePairScore, ps, (x0, k));
    Restraint *r =
        create_distance_restraint(s[0], s[1], ps.get(), name);
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
      IMP_NEW(container::ListSingletonContainer, lsc,
              (particles[0]->get_model(), IMP::get_indexes(particles)));
      IMP_NEW(container::ConnectingPairContainer, cpc, (lsc, 0));
      Pointer<Restraint> cr =
          container::create_restraint(hdps.get(), cpc.get(), name);
      return cr.release();
    } else {
      IMP_NEW(core::TableRefiner, tr, ());
      ParticlesTemp rps;
      for (unsigned int i = 0; i < s.size(); ++i) {
        ParticlesTemp ps = s[i].get_selected_particles();
        IMP_USAGE_CHECK(!ps.empty(), "Selection "
                                         << s[i]
                                         << " does not contain any particles.");
        tr->add_particle(ps[0], ps);
        rps.push_back(ps[0]);
      }
      IMP_NEW(core::HarmonicUpperBoundSphereDistancePairScore, hdps, (x0, k));
      Pointer<PairScore> ps;
      IMP_LOG_TERSE("Using closest pair score." << std::endl);
      ps = new core::KClosePairsPairScore(hdps, tr);
      IMP_NEW(IMP::internal::StaticListContainer<SingletonContainer>,
              lsc, (rps[0]->get_model(), "Connectivity particles"));
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
  ParticleIndexes s = ss.get_selected_particle_indexes();
  if (s.size() < 2) return nullptr;
  Hierarchies h = ss.get_hierarchies();
  Model *m = h[0].get_model();
  if (s.size() == 2) {
    IMP_NEW(core::HarmonicUpperBoundSphereDistancePairScore, ps, (x0, k));
    IMP_NEW(core::PairRestraint, r,
            (m, ps, ParticleIndexPair(s[0], s[1]), name));
    return r.release();
  } else {
    IMP_NEW(core::HarmonicUpperBoundSphereDistancePairScore, hdps, (x0, k));
    IMP_NEW(container::ListSingletonContainer, lsc, (m, s));
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
  IMP_NEW(IMP::internal::StaticListContainer<SingletonContainer>,
          lsc, (ps[0]->get_model(), "Hierarchy EV particles"));
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
    s.set_resolution(resolution);
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
    if (core::RigidMember::get_is_setup(ps[i])) {
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
  Vector<std::pair<int, Hierarchy> > queue;
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
  if (Residue::get_is_setup(h)) {
    return Ints(1, Residue(h).get_index());
  }
  Ints ret;
  if (Domain::get_is_setup(h)) {
    IntRange ir = Domain(h).get_index_range();
    for (int i = ir.first; i < ir.second; ++i) {
      ret.push_back(i);
    }
  } else if (Fragment::get_is_setup(h)) {
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
    if (Residue::get_is_setup(h)) {
      return Residue(h).get_residue_type();
    }
  } while ((h = h.get_parent()));
  IMP_THROW("Hierarchy " << h << " has no residue type.", ValueException);
}
std::string get_chain_id(Hierarchy h) {
  Chain c = get_chain(h);
  if (!c) {
    IMP_THROW("Hierarchy " << h << " has no chain.", ValueException);
  } else {
    return c.get_id();
  }
}
AtomType get_atom_type(Hierarchy h) {
  do {
    if (Atom::get_is_setup(h)) {
      return Atom(h).get_atom_type();
    }
  } while ((h = h.get_parent()));
  IMP_THROW("Hierarchy " << h << " has no atom type.", ValueException);
}
std::string get_domain_name(Hierarchy h) {
  do {
    if (Domain::get_is_setup(h)) {
      return Domain(h)->get_name();
    }
  } while ((h = h.get_parent()));
  IMP_THROW("Hierarchy " << h << " has no domain name.", ValueException);
}
std::string get_molecule_name(Hierarchy h) {
  do {
    if (Molecule::get_is_setup(h)) {
      return h->get_name();
    }
  } while ((h = get_parent_representation(h)));
  IMP_THROW("Hierarchy " << h << " has no molecule name.", ValueException);
}
IMPATOM_END_NAMESPACE
