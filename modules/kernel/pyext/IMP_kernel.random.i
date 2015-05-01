// Simple wrapper to give read-only access to IMP's random number generator

%{
#include <boost/static_assert.hpp>
// needed to ensure that the swig wrappers work
BOOST_STATIC_ASSERT(sizeof(int) == sizeof(boost::int32_t));

%}

namespace boost {
  typedef int int32_t;
  typedef unsigned long int uint64_t;
}

namespace IMP {
  class RandomNumberGenerator {
  public:
    void seed(::boost::int32_t x);
    int operator()();
  };
  const RandomNumberGenerator random_number_generator;
}

// support for get_random_numbers_normal, copied from random.h
// and its use of C array of double numbers
%include "carrays.i"
%array_class(double, doubleArray);
%array_class(float, floatArray);

%include "IMP/random.h"
