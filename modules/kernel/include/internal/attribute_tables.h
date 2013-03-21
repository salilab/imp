/**
 *  \file interna/constants.h    \brief Various useful constants.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_INTERNAL_ATTRIBUTE_TABLES_H
#define IMPKERNEL_INTERNAL_ATTRIBUTE_TABLES_H

#include <IMP/kernel/kernel_config.h>
#include <boost/dynamic_bitset.hpp>
#include "../Key.h"
#include "../utility.h"
#include "../FloatIndex.h"
#include "input_output_exception.h"
#include <IMP/base/exception.h>
#include <IMP/base/check_macros.h>
#include <IMP/base/log.h>
#include <IMP/algebra/Sphere3D.h>


#if IMP_HAS_CHECKS >= IMP_INTERNAL
#define IMP_CHECK_MASK(mask, particle_index, key, operation, entity)    \
  IMP_USAGE_CHECK(!mask || mask->size() >                               \
                  get_as_unsigned_int(particle_index),                  \
                  "For some reason the mask is too small.");            \
  if (mask && !(*mask)[get_as_unsigned_int(particle_index)]) {          \
    throw InputOutputException(particle_index.get_index(),        \
                               InputOutputException::operation, \
                               InputOutputException::entity, \
                               key.get_string());                         \
  }

#else
#define IMP_CHECK_MASK(mask, particle_index, key, operation, entity)
#endif

#ifdef _OPENMP
#define IMP_ACCUMULATE(dest, increment)         \
  {                                             \
    double &val=(dest);                         \
    double pv=(increment);                      \
    IMP_PRAGMA(omp atomic)                      \
      val+=pv;                                  \
  }
#else
#define IMP_ACCUMULATE(dest, increment)\
  dest+=increment
#endif

IMPKERNEL_BEGIN_INTERNAL_NAMESPACE

typedef boost::dynamic_bitset<> Mask;

template <class Traits>
class BasicAttributeTable {
public:
  typedef typename Traits::Key Key;
private:
  base::Vector<typename Traits::Container > data_;
#if IMP_HAS_CHECKS >= IMP_INTERNAL
  Mask *read_mask_, *write_mask_, *add_remove_mask_;
#endif
  base::set<Key> caches_;

   void do_add_attribute(Key k, ParticleIndex particle,
                           typename Traits::PassValue value) {
     IMP_USAGE_CHECK(Traits::get_is_valid(value), "Can't set to invalid value: "
                     << value << " for attribute " << k);
    if (data_.size() <= k.get_index()) {
      data_.resize(k.get_index()+1);
    }
    base::resize_to_fit(data_[k.get_index()], particle,
                        Traits::get_invalid());
    data_[k.get_index()][particle]=value;
  }
public:
  void swap_with(BasicAttributeTable<Traits> &o) {
    IMP_SWAP_MEMBER(data_);
    IMP_SWAP_MEMBER(caches_);
  }

#if IMP_HAS_CHECKS >= IMP_INTERNAL
  void set_masks(Mask *read_mask,
                 Mask *write_mask,
                 Mask *add_remove_mask) {
    read_mask_=read_mask;
    write_mask_=write_mask;
    add_remove_mask_=add_remove_mask;
  }
#endif

  BasicAttributeTable()
#if IMP_HAS_CHECKS >= IMP_INTERNAL
  : read_mask_(nullptr), write_mask_(nullptr), add_remove_mask_(nullptr)
#endif
  {}

  void add_attribute(Key k, ParticleIndex particle,
                     typename Traits::PassValue value) {
    IMP_CHECK_MASK(add_remove_mask_, particle, k, ADD, ATTRIBUTE);
    do_add_attribute(k, particle, value);
  }
  void add_cache_attribute(Key k, ParticleIndex particle,
                           typename Traits::PassValue value) {
IMP_OMP_PRAGMA(critical(imp_cache))
    {
    caches_.insert(k);
    do_add_attribute(k, particle, value);
  }
  }
  void clear_caches(ParticleIndex particle) {
IMP_OMP_PRAGMA(critical(imp_cache))
    for (typename base::set<Key>::const_iterator it=caches_.begin();
         it != caches_.end(); ++it) {
      if (data_.size() > it->get_index()
          && data_[it->get_index()].size() > get_as_unsigned_int(particle)) {
        data_[it->get_index()][particle]
            = Traits::get_invalid();
      }
    }
  }
  void remove_attribute(Key k, ParticleIndex particle) {
    IMP_CHECK_MASK(add_remove_mask_, particle, k, REMOVE, ATTRIBUTE);
    IMP_USAGE_CHECK(get_has_attribute(k, particle),
                    "Can't remove attribute if it isn't there");
    data_[k.get_index()][particle]=Traits::get_invalid();
  }
  bool get_has_attribute(Key k, ParticleIndex particle) const {
    if (data_.size() <= k.get_index()) return false;
    else if (data_[k.get_index()].size()
             <= get_as_unsigned_int(particle)) return false;
    else return Traits::get_is_valid(data_[k.get_index()]
                                     [particle]);
  }
  void set_attribute(Key k, ParticleIndex particle,
                     typename Traits::PassValue value) {
#if IMP_HAS_CHECKS >= IMP_INTERNAL
    if (caches_.find(k)==caches_.end()) {
      IMP_CHECK_MASK( write_mask_, particle, k, SET, ATTRIBUTE);
    }
#endif
    IMP_USAGE_CHECK(get_has_attribute(k, particle),
                    "Setting invalid attribute: " << k
                    << " of particle " << particle);
    IMP_USAGE_CHECK(value != Traits::get_invalid(),
                    "Cannot set attribute to value of " << Traits::get_invalid()
                    << " as it is reserved for a null value.");
    data_[k.get_index()][particle]= value;
  }
  typename Traits::PassValue get_attribute(Key k,
                                           ParticleIndex particle,
                                           bool checked=true) const {
    if (checked) {
      IMP_CHECK_MASK(read_mask_, particle, k, GET, ATTRIBUTE);
    }
    IMP_USAGE_CHECK(get_has_attribute(k, particle),
                    "Requested invalid attribute: " << k
                    << " of particle " << particle);
    return data_[k.get_index()][particle];
  }
  typename Traits::Container::reference access_attribute(Key k,
                                                       ParticleIndex particle) {
    IMP_CHECK_MASK(write_mask_, particle, k, SET, ATTRIBUTE);
    IMP_USAGE_CHECK(get_has_attribute(k, particle),
                    "Requested invalid attribute: " << k
                    << " of particle " << particle);
    return data_[k.get_index()][particle];
  }
  std::pair<typename Traits::Value,
            typename Traits::Value> get_range_internal(Key k) const {
    std::pair<typename Traits::Value,
              typename Traits::Value>  ret;
    IMP_USAGE_CHECK(data_.size() > k.get_index()
                    && data_[k.get_index()].size()!=0,
                    "Cannot request range of an unused key.");
    bool init=false;
    for (unsigned int i=0; i< data_[k.get_index()].size(); ++i) {
      if (Traits::get_is_valid(data_[k.get_index()][ParticleIndex(i)])) {
        if (!init) {
          ret.first= data_[k.get_index()][ParticleIndex(i)];
          ret.second= data_[k.get_index()][ParticleIndex(i)];
          init=true;
        } else {
          ret.first=Traits::min(ret.first,
                                data_[k.get_index()][ParticleIndex(i)]);
          ret.second=Traits::max(ret.second,
                                 data_[k.get_index()][ParticleIndex(i)]);
        }
      }
    }
    return ret;
  }
  void clear_attributes(ParticleIndex particle) {
    IMP_CHECK_MASK(add_remove_mask_, particle, Key(0), REMOVE, ATTRIBUTE);
    for (unsigned int i=0; i< data_.size(); ++i) {
      if (data_[i].size() > get_as_unsigned_int(particle)) {
        data_[i][particle]= Traits::get_invalid();
      }
    }
  }

  IMP::base::Vector<Key>
  get_attribute_keys(ParticleIndex particle) const {
    base::Vector<Key> ret;
    for (unsigned int i=0; i< data_.size(); ++i) {
      if (data_[i].size() > get_as_unsigned_int(particle)
          && Traits::get_is_valid(data_[i][particle])) {
        ret.push_back(Key(i));
      }
    }
    return ret;
  }
  void fill(typename Traits::PassValue value) {
    for (unsigned int i=0; i< data_.size(); ++i) {
      std::fill(data_[i].begin(), data_[i].end(), value);
    }
  }
  unsigned int size() const {return data_.size();}
  unsigned int size(unsigned int i) const {return data_[i].size();}
};
IMP_SWAP_1(BasicAttributeTable);



class FloatAttributeTable {
  //vector<algebra::Sphere3D> spheres_;
  //vector<algebra::Sphere3D> sphere_derivatives_;
  base::IndexVector<ParticleIndexTag, algebra::Sphere3D> spheres_;
  base::IndexVector<ParticleIndexTag, algebra::Sphere3D> sphere_derivatives_;
  base::IndexVector<ParticleIndexTag, algebra::Vector3D> internal_coordinates_;
  base::IndexVector<ParticleIndexTag, algebra::Vector3D>
  internal_coordinate_derivatives_;
  BasicAttributeTable<internal::FloatAttributeTableTraits> data_;
  BasicAttributeTable<internal::FloatAttributeTableTraits> derivatives_;
  // make use bitset
  BasicAttributeTable<internal::BoolAttributeTableTraits> optimizeds_;
  FloatRanges ranges_;
#if IMP_HAS_CHECKS >= IMP_INTERNAL
  Mask *read_mask_, *write_mask_,*add_remove_mask_,
                *read_derivatives_mask_, *write_derivatives_mask_;
#endif
  algebra::Sphere3D get_invalid_sphere() const {
    double iv= internal::FloatAttributeTableTraits::get_invalid();
    algebra::Sphere3D ivs(algebra::Vector3D(iv, iv, iv), iv);
    return ivs;
  }

public:
  void swap_with(FloatAttributeTable&o) {
    using std::swap;
    IMP_SWAP_MEMBER(spheres_);
    IMP_SWAP_MEMBER(sphere_derivatives_);
    IMP_SWAP_MEMBER(data_);
    IMP_SWAP_MEMBER(derivatives_);
    IMP_SWAP_MEMBER(optimizeds_);
    IMP_SWAP_MEMBER(internal_coordinates_);
    IMP_SWAP_MEMBER(internal_coordinate_derivatives_);
  }
  FloatAttributeTable()
#if IMP_HAS_CHECKS >= IMP_INTERNAL
  : read_mask_(nullptr), write_mask_(nullptr),
    add_remove_mask_(nullptr),
    read_derivatives_mask_(nullptr),
    write_derivatives_mask_(nullptr)
#endif
{}
#if IMP_HAS_CHECKS >= IMP_INTERNAL
  void set_masks(Mask *read_mask,
                 Mask *write_mask,
                 Mask *add_remove_mask,
                 Mask *read_derivatives_mask,
                 Mask *write_derivatives_mask) {
    data_.set_masks(read_mask, write_mask, add_remove_mask);
    derivatives_.set_masks(read_derivatives_mask, write_derivatives_mask,
                           add_remove_mask);
    optimizeds_.set_masks(read_mask, write_mask, add_remove_mask);
    read_mask_=read_mask;
    write_mask_=write_mask;
    add_remove_mask_=add_remove_mask;
    read_derivatives_mask_=read_derivatives_mask;
    write_derivatives_mask_=write_derivatives_mask;
  }
#endif

  // make sure you know what you are doing
  algebra::Sphere3D& get_sphere(ParticleIndex particle) {
    IMP_CHECK_MASK(read_mask_, particle, FloatKey(0), GET, ATTRIBUTE);
    return spheres_[particle];
  }

  algebra::Vector3D& get_internal_coordinates(ParticleIndex particle) {
    IMP_CHECK_MASK(read_mask_, particle, FloatKey(5), GET, ATTRIBUTE);
    IMP_USAGE_CHECK(internal_coordinates_[particle][0]
                    !=internal::FloatAttributeTableTraits::get_invalid(),
                    "No internal coordinates");
    IMP_USAGE_CHECK(internal_coordinates_[particle][1]
                    !=internal::FloatAttributeTableTraits::get_invalid(),
                    "No internal coordinates");
    IMP_USAGE_CHECK(internal_coordinates_[particle][2]
                    !=internal::FloatAttributeTableTraits::get_invalid(),
                    "No internal coordinates");
    return internal_coordinates_[particle];
  }


  void add_to_coordinate_derivatives(ParticleIndex particle,
                                     const algebra::Vector3D &v,
                                     const DerivativeAccumulator &da) {
    IMP_CHECK_MASK(write_derivatives_mask_, particle, FloatKey(0),
                   SET, DERIVATIVE);
    IMP_USAGE_CHECK(get_has_attribute(FloatKey(0), particle),
                    "Particle does not have coordinates");
    IMP_ACCUMULATE(sphere_derivatives_[particle][0], da(v[0]));
    IMP_ACCUMULATE(sphere_derivatives_[particle][1], da(v[1]));
    IMP_ACCUMULATE(sphere_derivatives_[particle][2], da(v[2]));
  }

  void add_to_internal_coordinate_derivatives(ParticleIndex particle,
                                     const algebra::Vector3D &v,
                                     const DerivativeAccumulator &da) {
    IMP_CHECK_MASK(write_derivatives_mask_, particle, FloatKey(4),
                   SET, DERIVATIVE);
    IMP_USAGE_CHECK(get_has_attribute(FloatKey(0), particle),
                    "Particle does not have coordinates");
    IMP_ACCUMULATE(internal_coordinate_derivatives_[particle][0],
        da(v[0]));
    IMP_ACCUMULATE(internal_coordinate_derivatives_[particle][1],
        da(v[1]));
    IMP_ACCUMULATE(internal_coordinate_derivatives_[particle][2],
        da(v[2]));
  }

  const algebra::Vector3D&
  get_coordinate_derivatives(ParticleIndex particle) const {
    IMP_CHECK_MASK(read_derivatives_mask_, particle, FloatKey(0),
                   GET, DERIVATIVE);
    IMP_USAGE_CHECK(get_has_attribute(FloatKey(0), particle),
                    "Particle does not have coordinates");
    return sphere_derivatives_[particle].get_center();
    }
  void zero_derivatives() {
    /*std::fill(sphere_derivatives_.begin(), sphere_derivatives_.end(),
      algebra::Sphere3D(algebra::Vector3D(0,0,0), 0));*/
    // make more efficient
    std::fill(sphere_derivatives_.begin(),
              sphere_derivatives_.end(),
              algebra::Sphere3D(algebra::Vector3D(0,0,0),0));
    std::fill(internal_coordinate_derivatives_.begin(),
              internal_coordinate_derivatives_.end(),
              algebra::Vector3D(0,0,0));
    derivatives_.fill(0);
  }
  void clear_caches(ParticleIndex ) {
  }
  void add_cache_attribute(FloatKey , ParticleIndex, double ){
    IMP_NOT_IMPLEMENTED;
  }
  void remove_attribute(FloatKey k, ParticleIndex particle) {
    IMP_CHECK_MASK(add_remove_mask_, particle,k,
                   REMOVE, ATTRIBUTE);
    if (k.get_index() < 4) {
      IMP_CHECK_MASK(add_remove_mask_, particle, k,
                     REMOVE, ATTRIBUTE);
      spheres_[particle][k.get_index()]
        = internal::FloatAttributeTableTraits::get_invalid();
      sphere_derivatives_[particle][k.get_index()]
        = internal::FloatAttributeTableTraits::get_invalid();
    } else if (k.get_index() < 7) {
      IMP_CHECK_MASK(add_remove_mask_, particle, k,
                     REMOVE, ATTRIBUTE);
      internal_coordinates_[particle][k.get_index()-4]
        = internal::FloatAttributeTableTraits::get_invalid();
      internal_coordinate_derivatives_[particle]
          [k.get_index()-4]
        = internal::FloatAttributeTableTraits::get_invalid();
    } else {
      data_.remove_attribute(FloatKey(k.get_index()-7), particle);
      derivatives_.remove_attribute(FloatKey(k.get_index()-7), particle);
    }
    if (optimizeds_.get_has_attribute(k, particle)) {
      optimizeds_.remove_attribute(k, particle);
    }
  }
  bool get_is_optimized(FloatKey k, ParticleIndex particle) const {
    return optimizeds_.get_has_attribute(k, particle);
  }
  // check NOT_EVALUATING
  void set_is_optimized(FloatKey k, ParticleIndex particle, bool tf) {
    if (tf && !optimizeds_.get_has_attribute(k, particle)) {
      optimizeds_.add_attribute(k, particle, true);
    } else if (!tf && optimizeds_.get_has_attribute(k, particle)){
      optimizeds_.remove_attribute(k, particle);
    }
  }
  // check AFTER_EVALUATE, NOT_EVALUATING
  double get_derivative(FloatKey k, ParticleIndex particle,
                        bool checked=true) const {
    IMP_USAGE_CHECK(get_has_attribute(k, particle),
                    "Can't get derivative that isn't there");
    if (k.get_index() < 4) {
      if (checked) {
        IMP_CHECK_MASK(read_derivatives_mask_, particle, k,
                       GET, DERIVATIVE);
      }
      return sphere_derivatives_[particle][k.get_index()];
    } else if (k.get_index() < 7) {
      if (checked) {
        IMP_CHECK_MASK(read_derivatives_mask_, particle, k,
                       GET, DERIVATIVE);
      }
      return internal_coordinate_derivatives_[particle]
          [k.get_index()-4];
    } else {
      return derivatives_.get_attribute(FloatKey(k.get_index()-7), particle,
                                        checked);
    }
  }
  // check can change EVALUATE, AFTER_EVALUATE< NOT_EVALUATING
  void add_to_derivative(FloatKey k, ParticleIndex particle, double v,
                         const DerivativeAccumulator &da) {
    IMP_USAGE_CHECK(get_has_attribute(k, particle),
                    "Can't get derivative that isn't there");
    if (k.get_index() < 4) {
      IMP_CHECK_MASK(write_derivatives_mask_, particle, k,
                     SET, DERIVATIVE);
      sphere_derivatives_[particle][k.get_index()]+=da(v);;
    } else if (k.get_index() < 7) {
      IMP_CHECK_MASK(write_derivatives_mask_, particle, k,
                     SET, DERIVATIVE);
      internal_coordinate_derivatives_[particle]
          [k.get_index()-4]+=da(v);;
    } else {
      FloatKey nk(k.get_index()-7);
      IMP_ACCUMULATE(derivatives_.access_attribute(nk, particle),
                    da(v));
    }
  }
  void add_attribute(FloatKey k, ParticleIndex particle, double v,
                     bool opt=false) {
    IMP_CHECK_MASK(add_remove_mask_, particle, k,
                   ADD, ATTRIBUTE);
    IMP_USAGE_CHECK(!get_has_attribute(k, particle),
                    "Can't add attribute that is there");
    IMP_USAGE_CHECK(FloatAttributeTableTraits::get_is_valid(v),
                    "Can't set float attribute to " << v
                    << " that is a special value.");
    if (k.get_index() <4) {
      if (spheres_.size() <= get_as_unsigned_int(particle)) {
        spheres_.resize(get_as_unsigned_int(particle)+1, get_invalid_sphere());
        sphere_derivatives_.resize(get_as_unsigned_int(particle)+1,
                                   get_invalid_sphere());
      }
      spheres_[particle][k.get_index()]=v;
    } else if (k.get_index() <7) {
      if (internal_coordinates_.size() <= get_as_unsigned_int(particle)) {
        internal_coordinates_.resize(get_as_unsigned_int(particle)+1,
                                     get_invalid_sphere().get_center());
        internal_coordinate_derivatives_.
            resize(get_as_unsigned_int(particle)+1,
                 get_invalid_sphere().get_center());
      }
      internal_coordinates_[particle][k.get_index()-4]=v;
    } else {
      FloatKey nk(k.get_index()-7);
      data_.add_attribute(nk, particle, v);
      derivatives_.add_attribute(nk, particle, 0);
    }
    if (opt) optimizeds_.add_attribute(k, particle, true);
    ranges_.resize(std::max(ranges_.size(),
                            static_cast<size_t>(k.get_index()+1)),
                   FloatRange(-std::numeric_limits<double>::max(),
                               std::numeric_limits<double>::max()));
    IMP_USAGE_CHECK(get_has_attribute(k, particle),
                    "Can't attribute was not added");
  }
  bool get_has_attribute(FloatKey k, ParticleIndex particle) const {
    if (k.get_index() < 4) {
      if (spheres_.size() <= get_as_unsigned_int(particle)) return false;
      else if (!internal::FloatAttributeTableTraits
               ::get_is_valid(spheres_[particle]
                              [k.get_index()])){
        return false;
      }
      return true;
    } else if (k.get_index() < 7) {
      if (internal_coordinates_.size() <= get_as_unsigned_int(particle)) {
        return false;
      }
      else if (!internal::FloatAttributeTableTraits
               ::get_is_valid(internal_coordinates_
                              [particle]
                              [k.get_index()-4])){
        return false;
      }
      return true;
    } else {
      return data_.get_has_attribute(FloatKey(k.get_index()-7), particle);
    }
  }
  void set_attribute(FloatKey k, ParticleIndex particle,
                     double v) {
    IMP_CHECK_MASK(write_mask_, particle, k,
                   SET, ATTRIBUTE);
    IMP_USAGE_CHECK(internal::FloatAttributeTableTraits::get_is_valid(v),
                    "Can't set attribute to invalid value");
    IMP_USAGE_CHECK(get_has_attribute(k, particle),
                    "Can't set attribute that is not there");
    if (k.get_index() <4) {
      spheres_[particle][k.get_index()]=v;
    } else if (k.get_index() <7) {
      internal_coordinates_[particle][k.get_index()-4]=v;
    } else {
      data_.set_attribute(FloatKey(k.get_index()-7), particle, v);
    }
  }
  double get_attribute(FloatKey k,
                       ParticleIndex particle,
                       bool checked=true) const {
    if (checked) {
      IMP_CHECK_MASK(read_mask_, particle, k,
                     GET, ATTRIBUTE);
    }
    IMP_USAGE_CHECK(get_has_attribute(k, particle),
                    "Can't get attribute that is not there");
    if (k.get_index()<4) {
      return spheres_[particle][k.get_index()];
    } else if (k.get_index()<7) {
      return internal_coordinates_[particle][k.get_index()-4];
    } else {
      return data_.get_attribute(FloatKey(k.get_index()-7), particle, checked);
    }
  }
  double& access_attribute(FloatKey k,
                       ParticleIndex particle) {
    IMP_CHECK_MASK(write_mask_, particle, k,
                   SET, ATTRIBUTE);
    IMP_USAGE_CHECK(get_has_attribute(k, particle),
                    "Can't get attribute that is not there");
    if (k.get_index()<4) {
      return spheres_[particle][k.get_index()];
    } else if (k.get_index()<7) {
      return spheres_[particle][k.get_index()-4];
    } else {
      return data_.access_attribute(FloatKey(k.get_index()-7), particle);
    }
  }
 FloatIndexes get_optimized_attributes() const {
    FloatIndexes ret;
    for (unsigned int i=0; i< optimizeds_.size(); ++i) {
      for (unsigned int j=0; j< optimizeds_.size(i); ++j) {
        if (optimizeds_.get_has_attribute(FloatKey(i), ParticleIndex(j))) {
          ret.push_back(FloatIndex(ParticleIndex(j), FloatKey(i)));
        }
      }
    }
    return ret;
  }
  void set_range(FloatKey k, FloatRange fr) {
    ranges_[k.get_index()]=fr;
  }
  FloatRange get_range(FloatKey k) {
    FloatRange ret= ranges_[k.get_index()];
    if (ret.first==-std::numeric_limits<double>::max()) {
      if (k.get_index() <4) {
        std::swap(ret.first, ret.second);
        for (unsigned int i=0; i< spheres_.size(); ++i) {
          if (internal::FloatAttributeTableTraits
              ::get_is_valid(spheres_[ParticleIndex(i)][k.get_index()])) {
            ret.first= std::min(ret.first, spheres_[ParticleIndex(i)]
                                [k.get_index()]);
            ret.second= std::max(ret.second, spheres_[ParticleIndex(i)]
                                 [k.get_index()]);
          }
        }
        return ret;
      } else if (k.get_index() < 7) {
        std::swap(ret.first, ret.second);
        for (unsigned int i=0; i< internal_coordinates_.size(); ++i) {
          if (internal::FloatAttributeTableTraits
              ::get_is_valid(internal_coordinates_[ParticleIndex(i)]
                             [k.get_index()-4])) {
            ret.first= std::min(ret.first,
                                internal_coordinates_[ParticleIndex(i)]
                                [k.get_index()-4]);
            ret.second= std::max(ret.second,
                                 internal_coordinates_[ParticleIndex(i)]
                                 [k.get_index()-4]);
          }
        }
        return ret;

      } else {
        return data_.get_range_internal(FloatKey(k.get_index()-7));
      }
    } else {
      return ret;
    }
  }
  void clear_attributes(ParticleIndex particle) {
    if (spheres_.size()> get_as_unsigned_int(particle)) {
      spheres_[particle]= get_invalid_sphere();
      sphere_derivatives_[particle]=get_invalid_sphere();
    }
    if (internal_coordinates_.size()> get_as_unsigned_int(particle)) {
      internal_coordinates_[particle]
          = get_invalid_sphere().get_center();
      internal_coordinate_derivatives_[particle]
        =get_invalid_sphere().get_center();
    }
    data_.clear_attributes(particle);
    derivatives_.clear_attributes(particle);
    optimizeds_.clear_attributes(particle);
  }
  FloatKeys get_attribute_keys(ParticleIndex particle) const {
    FloatKeys ret=data_.get_attribute_keys(particle);
    for (unsigned int i=0; i< ret.size(); ++i) {
      ret[i]= FloatKey(ret[i].get_index()+7);
    }
    for (unsigned int i=0; i< 7; ++i) {
      if (get_has_attribute(FloatKey(i),particle)) {
        ret.push_back(FloatKey(i));
      }
    }
    return ret;
  }
  unsigned int size() const {return data_.size()+7;}
  unsigned int size(int i) const {

    if (i < 4) {
      return spheres_.size();
    } else if (i < 7) {
      return internal_coordinates_.size();
    } else {
      return data_.size(i-7);
    }
  }
};

IMP_SWAP(FloatAttributeTable);

typedef BasicAttributeTable<internal::StringAttributeTableTraits>
StringAttributeTable;
typedef BasicAttributeTable<internal::IntAttributeTableTraits>
IntAttributeTable;
typedef BasicAttributeTable<internal::ObjectAttributeTableTraits>
ObjectAttributeTable;
typedef BasicAttributeTable<internal::WeakObjectAttributeTableTraits>
WeakObjectAttributeTable;
typedef BasicAttributeTable<internal::IntsAttributeTableTraits>
IntsAttributeTable;
typedef BasicAttributeTable<internal::ObjectsAttributeTableTraits>
ObjectsAttributeTable;
typedef BasicAttributeTable<internal::ParticleAttributeTableTraits>
ParticleAttributeTable;
typedef BasicAttributeTable<internal::ParticlesAttributeTableTraits>
ParticlesAttributeTable;


struct Masks {
#if IMP_HAS_CHECKS >= IMP_INTERNAL
  mutable Mask read_mask_, write_mask_, add_remove_mask_,
    read_derivatives_mask_, write_derivatives_mask_;
#endif
};
IMPKERNEL_END_INTERNAL_NAMESPACE

#ifndef SWIG
#define IMP_MODEL_IMPORT(Base)                  \
  using Base::add_attribute;                    \
  using Base::add_cache_attribute;              \
  using Base::remove_attribute;                 \
  using Base::get_has_attribute;                \
  using Base::set_attribute;                    \
  using Base::get_attribute;                    \
  using Base::access_attribute
#else
#define IMP_MODEL_IMPORT(Base)
#endif

#endif  /* IMPKERNEL_INTERNAL_ATTRIBUTE_TABLES_H */
