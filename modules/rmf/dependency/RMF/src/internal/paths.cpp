/**
 *  \file RMF/paths.cpp
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/version.hpp>
#include <string>
#include <vector>
#if BOOST_VERSION <= 104100
#include <cstdio> // for tmpnam()
#endif

#include "RMF/compiler_macros.h"
#include "RMF/internal/paths.h"

RMF_ENABLE_WARNINGS

namespace {

#ifndef _MSC_VER

// Return true iff the provided path is an absolute one
bool isabspath(boost::filesystem::path path) {
  return path.string()[0] == '/';
}

// Return an absolute path for a path (possibly) relative to basedir
boost::filesystem::path abspath(boost::filesystem::path path,
                                boost::filesystem::path basedir) {
  if (isabspath(path)) {
    return path;
  } else {
    return basedir / path;
  }
}

// Get the number of pathname components common to both absolute paths
size_t get_common_prefix(boost::filesystem::path p1,
		         boost::filesystem::path p2) {
  size_t common = 0;
  for (boost::filesystem::path::iterator it1(p1.begin()), it1_end(p1.end()),
       it2(p2.begin()), it2_end(p2.end());
       it1 != it1_end && it2 != it2_end; ++it1, ++it2) {
    if (*it1 == *it2) common++;
  }
  return common;
}

// Get the number of pathname components in the path
size_t count_path_components(boost::filesystem::path p) {
  size_t numcomp = 0;
  for (boost::filesystem::path::iterator it(p.begin()), it_end(p.end());
       it != it_end; ++it) {
    numcomp++;
  }
  return numcomp;
}

// Remove extraneous . and .. entries from the path
boost::filesystem::path normalize(const boost::filesystem::path& p) {
  size_t comp = count_path_components(p);
  std::vector<bool> keep(comp, true);

  size_t i = 0;
  for (boost::filesystem::path::iterator it(p.begin()), it_end(p.end());
       it != it_end; ++it, ++i) {
    if (*it == ".") {
      keep[i] = false;
    } else if (*it == "..") {
      keep[i] = false;
      for (int j = i; j >= 0; --j) {
        if (keep[j]) {
          keep[j] = false;
          break;
        }
      }
    }
  }
  boost::filesystem::path newp;
  i = 0;
  for (boost::filesystem::path::iterator it(p.begin()), it_end(p.end());
       it != it_end; ++it, ++i) {
    if (keep[i]) {
      newp /= *it;
    }
  }
  return newp;
}

// Return a path for p that is relative to the directory containing base
boost::filesystem::path relpath(boost::filesystem::path p,
                                boost::filesystem::path base) {
  boost::filesystem::path cwd = boost::filesystem::current_path();
  boost::filesystem::path absp = abspath(p, cwd);
  boost::filesystem::path absbase = abspath(base.parent_path(), cwd);

  size_t lenbase = count_path_components(absbase);
  size_t common = get_common_prefix(absp, absbase);

  boost::filesystem::path r;
  for (size_t i = 0; i < lenbase - common; ++i) {
    r /= "..";
  }
  size_t pcomp = 0;
  for (boost::filesystem::path::iterator it(absp.begin()), it_end(absp.end());
       it != it_end; ++it, ++pcomp) {
    if (pcomp >= common) {
      r /= *it;
    }
  }
  return r;
}

#endif // _MSC_VER

} // anonymous namespace

namespace RMF {
namespace internal {
std::string get_file_name(std::string path) {
#if BOOST_VERSION >= 104600
  return boost::filesystem::path(path).filename().string();
#else
  return boost::filesystem::path(path).filename();
#endif
}

std::string get_relative_path(std::string base, std::string file) {
#ifdef _MSC_VER
  return file;
#else
  return relpath(boost::filesystem::path(file),
                 boost::filesystem::path(base)).string();
#endif
}

std::string get_absolute_path(std::string base, std::string file) {
#ifdef _MSC_VER
  return file;
#else
  boost::filesystem::path b(base);
  boost::filesystem::path absb = abspath(b.parent_path(),
                                         boost::filesystem::current_path());
  boost::filesystem::path f(file);

  return normalize(abspath(f, absb)).string();
#endif
}

std::string get_unique_path() {
#if BOOST_VERSION > 104100
  boost::filesystem::path temp = boost::filesystem::unique_path();
  return temp.string();
#else
  return tmpnam(NULL);
#endif
}

}  // namespace internal
} /* namespace RMF */

RMF_DISABLE_WARNINGS
