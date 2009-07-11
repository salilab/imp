/**
 *  \file log_internal.h     \brief Logging and error reporting support.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#ifndef IMP_INTERNAL_LOG_INTERNAL_H
#define IMP_INTERNAL_LOG_INTERNAL_H

#include "../config.h"

#include <boost/noncopyable.hpp>

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string>

IMP_BEGIN_INTERNAL_NAMESPACE


class IMPEXPORT Log: boost::noncopyable
{
public:
  static Log& get();

  std::ostream &get_stream(unsigned int l) {
    if (target_== 0) {
      return std::cout;
    } else if (target_== 1) {
      return std::cerr;
    } else {
      return fstream_;
    }
  }

  unsigned int get_target() {
    return target_;
  }
  void set_target(unsigned int k) {
    target_=k;
  }
  void set_filename(std::string k) {
    fstream_.open(k.c_str());
    if (!fstream_) {
      std::cerr << "Error opening log file " << k << std::endl;
    }
  }

private:

  Log(unsigned int target)  : target_(target) {}
  unsigned int target_;
  std::ofstream fstream_;
};



IMP_END_INTERNAL_NAMESPACE

#endif  /* IMP_INTERNAL_LOG_INTERNAL_H */
