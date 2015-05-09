// Simple wrapper to give read-only access to IMP's random number generator

%{
#include <boost/static_assert.hpp>
// needed to ensure that the swig wrappers work
BOOST_STATIC_ASSERT(sizeof(int) == sizeof(boost::int32_t));

%}


namespace boost {
  typedef int int32_t;
  typedef unsigned long int uint64_t;
};


namespace IMP {
  class RandomNumberGenerator {
  public:
    void seed(::boost::int32_t x);
    int operator()();
  };
  const RandomNumberGenerator random_number_generator;
};

%include "std_vector.i"
namespace IMP {
  %template(VectorOfFloats) ::std::vector<float>;
  %template(VectorOfDoubles) ::std::vector<double>;
}

//%apply IMP::Floats &INOUT { IMP::Floats & v };
%include "IMP/random_utils.h"
namespace IMP {
  %template(get_random_floats_normal)   get_random_numbers_normal<float>;
  %template(get_random_doubles_normal)  get_random_numbers_normal<double>;
  %template(get_random_floats_uniform)  get_random_numbers_uniform<float>;
  %template(get_random_doubles_uniform) get_random_numbers_uniform<double>;
 };



%include "IMP/random.h"
