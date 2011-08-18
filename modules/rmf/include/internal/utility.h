/**
 *  \file IMP/utility.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPRMF_INTERNAL_UTILITY_H
#define IMPRMF_INTERNAL_UTILITY_H
#include <stdexcept>
#include <sstream>
IMPRMF_BEGIN_INTERNAL_NAMESPACE
IMPRMF_END_INTERNAL_NAMESPACE

#if defined(_MSC_VER)
#  define IMP_RMF_FUNCTION __FUNCTION__
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ < 199901L
# if __GNUC__ >= 2
#  define IMP_RMF_FUNCTION __FUNCTION__
# else
#  define IMP_RMF_FUNCTION "<unknown>"
# endif
#else
# define IMP_RMF_FUNCTION __func__
#endif


#define IMP_RMF_USAGE_CHECK(check, message)             \
  do {                                                  \
    if (!(check)) {                                     \
      std::ostringstream oss;                           \
      oss << "Usage check failed: " << #check << "\n"   \
          << message;                                   \
      throw std::runtime_error(oss.str().c_str());      \
    }                                                   \
  } while (false)

#define IMP_RMF_INTERNAL_CHECK(check, message)\
  do {                                                          \
    if (!(check)) {                                             \
      std::ostringstream oss;                                   \
      oss << "Internal check failed: " << #check << "\n"        \
          << message << "\nThis is a bug in the RMF library"    \
          << " please report it.";                              \
      throw std::logic_error(oss.str().c_str());                \
    }                                                           \
  } while (false)

#define IMP_RMF_IF_CHECK\
  if (true)

#define IMP_RMF_NOT_IMPLEMENTED\
  IMP_RMF_THROW("The function " << IMP_RMF_FUNCTION << " is not implemented", \
                std::logic_error)

#define IMP_RMF_UNUSED(variable) if (0) std::cout << variable;


#define IMP_RMF_THROW(m,e) do {  \
    std::ostringstream oss;      \
    oss << m;                    \
    throw e(oss.str().c_str());  \
  } while (false)

/** Call a function and throw an exception if the return values is bad */
#define IMP_HDF5_CALL(v) IMP_RMF_USAGE_CHECK((v)>=0, "Error calling "<< (#v))



#endif /* IMPRMF_INTERNAL_UTILITY_H */
