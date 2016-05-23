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
    //! Set the random number generator seed
    void seed(::boost::int32_t x);

    //! Get the next random value from the generator
    int operator()();
  };
  const RandomNumberGenerator random_number_generator;
};

%include "IMP/random_utils.h"
%inline %{
  // Provide non-templated versions for Python, and return vectors (rather
  // than taking a vector reference)
  namespace IMP {
    Vector<float> get_random_floats_normal(unsigned int n, float mean=0.0,
                                           float stddev=1.0) {
      Vector<float> ret;
      get_random_numbers_normal<float>(ret, n, mean, stddev);
      return ret;
    }
    Vector<double> get_random_doubles_normal(unsigned int n, double mean=0.0,
                                             double stddev=1.0) {
      Vector<double> ret;
      get_random_numbers_normal<double>(ret, n, mean, stddev);
      return ret;
    }
    Vector<float> get_random_floats_uniform(unsigned int n) {
      Vector<float> ret;
      get_random_numbers_uniform<float>(ret, n);
      return ret;
    }
    Vector<double> get_random_doubles_uniform(unsigned int n) {
      Vector<double> ret;
      get_random_numbers_uniform<double>(ret, n);
      return ret;
    }
  }
%}

%include "IMP/random.h"
