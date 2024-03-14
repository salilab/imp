/**
 *  \file IMP/Flag.h
 *  \brief Various general useful macros for IMP.
 *
 *  Copyright 2007-2022 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_INTERNAL_FLAG_H
#define IMPKERNEL_INTERNAL_FLAG_H

#include <IMP/kernel_config.h>
#include <boost/program_options.hpp>
#include <type_traits>
#include <boost/type_traits/is_enum.hpp>
#include <boost/noncopyable.hpp>
#include <string>

IMPKERNEL_BEGIN_INTERNAL_NAMESPACE

template <class T, bool ENABLED, class Enabled = void>
class FlagImpl : public T, public boost::noncopyable {};

template <bool ENABLED>
class FlagImpl<bool, ENABLED> : public boost::noncopyable {
  bool v_;

 public:
  FlagImpl(boost::program_options::options_description &opt, std::string name,
           std::string description, bool default_value)
      : v_(default_value) {
    if (!ENABLED) description += " Disabled at compile time.";
    opt.add_options()(name.c_str(),
                      boost::program_options::value<bool>(&v_)->zero_tokens(),
                      description.c_str());
  }
  operator bool() const { return v_; }
  operator bool &() { return v_; }
  FlagImpl<bool, ENABLED> &operator=(bool o) {
    v_ = o;
    return *this;
  }
};

template <class T, bool ENABLED>
class FlagImpl<T, ENABLED, typename std::enable_if<
   !std::is_enum<T>::value && !std::is_fundamental<T>::value>::type>
        : public T, public boost::noncopyable {
 public:
  FlagImpl(boost::program_options::options_description &opt, std::string name,
           std::string description, T default_value)
      : T(default_value) {
    if (!ENABLED) description += " Disabled at compile time.";
    opt.add_options()(name.c_str(), boost::program_options::value<T>(this)
                                        ->default_value(*this),
                      description.c_str());
  }
};

template <class T, bool ENABLED>
class FlagImpl<T, ENABLED, typename std::enable_if<
      std::is_fundamental<T>::value || std::is_enum<T>::value>::type>
            : public boost::noncopyable {
  T v_;

 public:
  FlagImpl(boost::program_options::options_description &opt, std::string name,
           std::string description, T default_value)
      : v_(default_value) {
    if (!ENABLED) description += " Disabled at compile time.";
    opt.add_options()(name.c_str(),
                      boost::program_options::value<T>(&v_)->default_value(v_),
                      description.c_str());
  }
  operator T() const { return v_; }
  operator T &() { return v_; }
  FlagImpl<T, ENABLED> &operator=(T o) {
    v_ = o;
    return *this;
  }
};

IMPKERNEL_END_INTERNAL_NAMESPACE

#endif /* IMPKERNEL_INTERNAL_FLAG_H */
