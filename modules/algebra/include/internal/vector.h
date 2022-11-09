/**
 *  \file cgal_predicates.h
 *  \brief predicates implemented using CGAL
 *  Copyright 2007-2022 IMP Inventors. All rights reserved.
*/

#ifndef IMPALGEBRA_INTERNAL_VECTOR_H
#define IMPALGEBRA_INTERNAL_VECTOR_H
#include <IMP/algebra/algebra_config.h>
#include <boost/scoped_array.hpp>
#include <IMP/exception.h>
#include <boost/serialization/access.hpp>
#include <boost/serialization/split_member.hpp>
#include <limits>

IMPALGEBRA_BEGIN_INTERNAL_NAMESPACE

template <class T>
inline T get_null_value() {
  return T();
}

template <>
inline double get_null_value() {
  return std::numeric_limits<double>::quiet_NaN();
}

template <>
inline int get_null_value() {
  return std::numeric_limits<int>::max();
}

template <class T, int D, bool KNOWN_DEFAULT>
class VectorData {
  T storage_[D];

  friend class boost::serialization::access;

  template<class Archive>
  void serialize(Archive &ar, const unsigned int) {
    for (auto &i: storage_) {
      ar & i;
    }
  }

 public:
  unsigned int get_dimension() const { return D; }
  VectorData(int) {}
  VectorData() {
#if IMP_HAS_CHECKS >= IMP_USAGE
    for (unsigned int i = 0; i < D; ++i) {
      storage_[i] = get_null_value<T>();
    }
#else
    if (KNOWN_DEFAULT) {
      for (unsigned int i = 0; i < D; ++i) {
        storage_[i] = get_null_value<T>();
      }
    }
#endif
  }
  template <class It>
  void set_coordinates(It b, It e) {
    IMP_USAGE_CHECK(std::distance(b, e) == D,
                    "Wrong number of coordinates provided.");
    std::copy(b, e, storage_);
  }
  T *get_data() { return storage_; }
  const T *get_data() const { return storage_; }

#ifdef _MSC_VER
  IMP_CXX11_DEFAULT_COPY_CONSTRUCTOR(VectorData);
#else
  VectorData(const VectorData &other) {
    std::copy(other.storage_, other.storage_ + D, storage_);
  }
#endif

  bool get_is_null() const { return storage_[0] >= get_null_value<T>(); }
  ~VectorData() {
#if IMP_HAS_CHECKS >= IMP_USAGE
    for (unsigned int i = 0; i < D; ++i) {
      storage_[i] = get_null_value<T>();
    }
#endif
  }
};

template <class T, bool KNOWN_DEFAULT>
class VectorData<T, -1, KNOWN_DEFAULT> {
  boost::scoped_array<T> storage_;
  unsigned int d_;

  friend class boost::serialization::access;

  template<class Archive>
  void save(Archive &ar, const unsigned int) const {
    ar << d_;
    for (unsigned i = 0; i < d_; ++i) {
      ar << storage_[i];
    }
  }

  template<class Archive>
  void load(Archive &ar, const unsigned int) {
    ar >> d_;
    storage_.reset(new T[d_]);
    T *data = get_data();
    for (unsigned i = 0; i < d_; ++i) {
      ar >> *(data + i);
    }
  }

  BOOST_SERIALIZATION_SPLIT_MEMBER()

 public:
  VectorData(int d) : storage_(new T[d]), d_(d) {}
  VectorData(const VectorData &o) {
    set_coordinates(o.get_data(), o.get_data() + o.get_dimension());
  }
  VectorData &operator=(const VectorData &o) {
    set_coordinates(o.get_data(), o.get_data() + o.get_dimension());
    return *this;
  }
  VectorData() : d_(0) {}
  unsigned int get_dimension() const { return d_; }
  template <class It>
  void set_coordinates(It b, It e) {
    d_ = std::distance(b, e);
    storage_.reset(new T[d_]);
    std::copy(b, e, storage_.get());
  }
  T *get_data() { return storage_.get(); }
  const T *get_data() const { return storage_.get(); }
  bool get_is_null() const { return d_ == 0; }
  ~VectorData() {
#if IMP_HAS_CHECKS >= IMP_USAGE
    for (unsigned int i = 0; i < d_; ++i) {
      storage_[i] = get_null_value<T>();
    }
#endif
  }
};

IMPALGEBRA_END_INTERNAL_NAMESPACE

#endif /* IMPALGEBRA_INTERNAL_VECTOR_H */
