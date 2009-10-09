/**
 *  \file Fragment.h
 *  \brief A decorator for associating a Hierachy piece
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 */

#ifndef IMPATOM_FRAGMENT_H
#define IMPATOM_FRAGMENT_H

#include "config.h"
#include "Hierarchy.h"
#include <IMP/Decorator.h>
#include <IMP/core/internal/ArrayOnAttributesHelper.h>

IMPATOM_BEGIN_NAMESPACE

//! A decorator to associate a particle with a part of a protein/DNA/RNA
/** The decorator stores an optional list of resdiue indexes.
    \unstable{Fragment}
    \untested{Fragment}
 */
class IMPATOMEXPORT Fragment: public Hierarchy
{

  struct Traits
    : public IMP::core::internal::ArrayOnAttributesHelper<IntKey, Int>
  {Traits(std::string str)
    : IMP::core::internal::ArrayOnAttributesHelper<IntKey, Int>(str){}
  };
  IMP_DECORATOR_ARRAY_DECL(private, Fragment, ResidueBegin,
                           residue_begin,
                           residue_begins, btraits_, int, Ints)
  IMP_DECORATOR_ARRAY_DECL(private, Fragment, ResidueEnd,
                           residue_end,
                           residue_ends, etraits_, int, Ints)
    static Traits btraits_;
    static Traits etraits_;
  //! Add a set of residues to this Fragment
  /** The values passed is intepreted as a range. That is,
      the Fragment will then contain [begin, end).
   */
  void add_residue_indexes(int begin, int end);
public:
  static Fragment setup_particle(Particle *p) {
    add_required_attributes_for_residue_begin(p, btraits_);
    add_required_attributes_for_residue_end(p, etraits_);
    if (!Hierarchy::particle_is_instance(p)) {
      Hierarchy::setup_particle(p,
                     Hierarchy::FRAGMENT);
    }
    return Fragment(p);
  }

  //! Create a domain by copying from o
  static Fragment setup_particle(Particle *p, Fragment o) {
    add_required_attributes_for_residue_begin(p, btraits_);
    add_required_attributes_for_residue_end(p, etraits_);
    if (!Hierarchy::particle_is_instance(p)) {
      Hierarchy::setup_particle(p,
                        Hierarchy::FRAGMENT);
    }
    Fragment f(p);
    for (unsigned int i=0; i< o.get_number_of_residue_begins(); ++i) {
      f.add_residue_begin(o.get_residue_begin(i));
      f.add_residue_end(o.get_residue_end(i));
    }
    return Fragment(p);
  }

  virtual ~Fragment();

  static bool particle_is_instance(Particle *p) {
    return has_required_attributes_for_residue_begin(p, btraits_)
      && Hierarchy::particle_is_instance(p);
  }


  //! Add the residues whose indexes are listed in the passed vector
  void set_residue_indexes( Ints o);

  Ints get_residue_indexes() const;

  //! Return true if this fragment contains a given residue
  /** This could be made more efficient. */
  bool get_contains_residue(int rindex) {
    for (unsigned int i=0; i< get_number_of_residue_begins(); ++i) {
      if (get_residue_begin(i) <= rindex) {
        if (get_residue_end(i) > rindex) return true;
      }
    }
    return false;
  }

#if !defined(IMP_DOXYGEN)
#if  !defined(SWIG)
  class ResidueIndexIterator {
    friend class Fragment;
    const Fragment *f_;
    int i_, j_;
    mutable int v_;
  public:
    typedef int value_type;
    typedef const int &reference;
    typedef const int* pointer;
    typedef std::forward_iterator_tag iterator_category;
    typedef int difference_type;
    ResidueIndexIterator(const Fragment * f, int index): f_(f) {
      i_=index;
      j_=0;
    }
    const ResidueIndexIterator& operator++() {
      ++j_;
      int diff= f_->get_residue_end(i_)- f_->get_residue_begin(i_);
      if (diff == j_) {
        ++i_;
        j_=0;
      }
      return *this;
    }
    ResidueIndexIterator operator++(int) {
      ResidueIndexIterator it= *this;
      operator++();
      return it;
    }
    int operator*() const {
      return f_->get_residue_begin(i_) + j_;
    }
    const int& operator->() const {
      v_= operator*();
      return v_;
    }
    bool operator==(const ResidueIndexIterator &o) const {
      IMP_assert(f_ == o.f_, "Can't compare iterators from different "
                 << "containers");
      return i_ == o.i_ && j_== o.j_;
    }
    bool operator!=(const ResidueIndexIterator &o) const {
      return !operator==(o);
    }
  };
#endif // SWIG
#else
  class ResidueIndexIterator;
#endif

#if !defined(SWIG)
  ResidueIndexIterator residue_indexes_begin() const {
    return ResidueIndexIterator(this, 0);
  }

  ResidueIndexIterator residue_indexes_end() const {
    return ResidueIndexIterator(this, get_number_of_residue_ends());
  }
#endif

  IMP_DECORATOR(Fragment, Hierarchy)
};


typedef Decorators<Fragment, Hierarchies> Fragments;

IMPATOM_END_NAMESPACE

#endif  /* IMPATOM_FRAGMENT_H */
