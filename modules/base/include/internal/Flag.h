/**
 *  \file IMP/base/Flag.h
 *  \brief Various general useful macros for IMP.
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPBASE_INTERNAL_FLAG_H
#define IMPBASE_INTERNAL_FLAG_H

#include <IMP/base/base_config.h>
#include <boost/program_options.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/type_traits/is_fundamental.hpp>
#include <boost/type_traits/is_enum.hpp>
#include <boost/mpl/negate.hpp>
#include <boost/mpl/and.hpp>
#include <boost/mpl/or.hpp>
#include <boost/noncopyable.hpp>
#include <string>

IMPBASE_BEGIN_INTERNAL_NAMESPACE

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
class FlagImpl<T, ENABLED, typename boost::enable_if<boost::mpl::and_<
                               boost::mpl::not_<boost::is_enum<T> >,
                               boost::mpl::not_<boost::is_fundamental<T> > > >::
                               type> : public T,
                                       public boost::noncopyable {
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
class FlagImpl<T, ENABLED, typename boost::enable_if<boost::mpl::or_<
                               boost::is_fundamental<T>, boost::is_enum<T> > >::
                               type> : public boost::noncopyable {
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

IMPBASE_END_INTERNAL_NAMESPACE

#endif /* IMPBASE_INTERNAL_FLAG_H */
