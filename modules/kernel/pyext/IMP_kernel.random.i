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

  %template(DoubleRandoms) ::IMP::Vector<double>;
}


%include "IMP/random.h"
