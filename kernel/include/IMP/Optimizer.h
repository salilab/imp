/**
 *  \file Optimizer.h     \brief Base class for all optimizers.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMP_OPTIMIZER_H
#define __IMP_OPTIMIZER_H

#include "IMP_config.h"
#include "base_types.h"
#include "VersionInfo.h"
#include "Object.h"
#include "utility.h"
#include "Model.h"
#include "Particle.h"
#include "Pointer.h"

#include <limits>

namespace IMP
{

class OptimizerState;

typedef std::vector<OptimizerState*> OptimizerStates;

/** \defgroup optimizer Optimizers
    Optimizers of various sorts.
  */

//! Base class for all optimizers.
/** \note There is currently no optimizer support for constraints
    (e.g. rigid bodies).
 */
class IMPDLLEXPORT Optimizer: public Object
{
public:
  Optimizer();
  virtual ~Optimizer();

  /** Optimize the model
      \param[in] max_steps The maximum number of steps to take.
      \return The final score.
   */
  virtual Float optimize(unsigned int max_steps) = 0;

  //! \return version and authorship information.
  virtual VersionInfo get_version_info() const = 0;

  //! Get the model being optimized
  Model *get_model() const {
    return model_.get();
  }

  //! Set the model being optimized
  /**
     \note The model is not owned by the optimizer and so is not
     deleted when the optimizer is deleted. Further, the Optimizer
     does not prevent the model from being deleted when all python
     references go away.
   */
  void set_model(Model *m) {model_=m;}

  virtual void show(std::ostream &out= std::cout) const {
    out << "Some optimizer" << std::endl;
  }

  IMP_CONTAINER(OptimizerState, optimizer_state, OptimizerStateIndex);

protected:
  //! Update optimizer state, should be called at each successful step
  void update_states();

  //! An index to an optimized particle attribute
  struct FloatIndex
  {
    /**
       \todo mac gcc breaks on the protection and friends here
     */
    friend class Optimizer;
    friend class FloatIndexIterator;
    Model::ParticleIterator p_;
    Particle::OptimizedKeyIterator fk_;
    FloatIndex(Model::ParticleIterator p): p_(p){}
  public:
    FloatIndex() {}
  };


  //! An interator through the optimized attributes
  /** The value type is an FloatIndex
   */
  class FloatIndexIterator
   {
    typedef FloatIndexIterator This;
    Model::ParticleIterator pe_;
    mutable FloatIndex i_;

    void search_valid() const {
      while (i_.fk_ == (*i_.p_)->optimized_keys_end()) {
        if (i_.fk_ == (*i_.p_)->optimized_keys_end()) {
          ++i_.p_;
          if (i_.p_== pe_) return;
          else {
            i_.fk_= (*i_.p_)->optimized_keys_begin();
          }
        } else {
          ++i_.fk_;
        }
      }
      IMP_assert(i_.p_ != pe_, "Should have just returned");
      IMP_assert(i_.fk_ != (*i_.p_)->optimized_keys_end(),
                 "Broken iterator end");
      IMP_assert((*i_.p_)->get_is_optimized(*i_.fk_),
                   "Why did the loop end?");
    }
    void find_next() const {
      ++i_.fk_;
      search_valid();
    }
  public:
    FloatIndexIterator(Model::ParticleIterator pc,
                       Model::ParticleIterator pe): pe_(pe), i_(pc) {
      if (pc != pe) {
        i_.fk_= (*pc)->optimized_keys_begin();
        search_valid();
      }
    }
    typedef FloatIndex value_type;
    typedef FloatIndex& reference;
    typedef FloatIndex* pointer;
    typedef std::forward_iterator_tag iterator_category;
    typedef int difference_type;

    const This &operator++() {
      find_next();
      return *this;
    }
    reference operator*() const {
      IMP_assert((*i_.p_)->get_is_optimized(*i_.fk_),
                 "The iterator is broken");
      return i_;
    }
    pointer operator->() const {
      return &i_;
    }
    bool operator==(const This &o) const {
      if (i_.p_ != o.i_.p_) return false;
      if (i_.p_== pe_) return o.i_.p_ ==pe_;
      else return i_.fk_ == o.i_.fk_;
    }
    bool operator!=(const This &o) const {
      return !operator==(o);
    }
  };

  //! Iterate through the optimized attributes
  FloatIndexIterator float_indexes_begin() {
    return FloatIndexIterator(model_->particles_begin(),
                              model_->particles_end());
  }

  FloatIndexIterator float_indexes_end() {
    return FloatIndexIterator(model_->particles_end(),
                              model_->particles_end());
  }

  //! Set the value of an optimized attribute
  /** The attribute must be optimized or an ErrorException is thrown.
   */
  void set_value(FloatIndex fi, Float v) {
    IMP_assert(fi.p_ != model_->particles_end(),
               "Out of range FloatIndex in Optimizer");
    IMP_assert((*fi.p_)->get_is_optimized(*fi.fk_),
               "Keep your mits off unoptimized attributes "
               << (*fi.p_)->get_index() << " " << *fi.fk_ << std::endl);
    (*fi.p_)->set_value(*fi.fk_, v);
  }

  //! Get the value of an optimized attribute
  Float get_value(FloatIndex fi) const {
    /* cast to const needed here to help MSVC */
    IMP_assert(static_cast<Model::ParticleConstIterator>(fi.p_)
               != model_->particles_end(),
               "Out of range FloatIndex in Optimizer");
    return (*fi.p_)->get_value(*fi.fk_);
  }

  //! Get the derivative of an optimized attribute
  Float get_derivative(FloatIndex fi) const {
    IMP_assert(fi.p_ != model_->particles_end(),
               "Out of range FloatIndex in Optimizer");
    return (*fi.p_)->get_derivative(*fi.fk_);
  }

  typedef std::vector<FloatIndex> FloatIndexes;

private:
  Pointer<Model> model_;
};

IMP_OUTPUT_OPERATOR(Optimizer);

} // namespace IMP

#endif  /* __IMP_OPTIMIZER_H */
