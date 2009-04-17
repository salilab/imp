// Simple wrapper to give read-only access to IMP's random number generator

namespace IMP {
  class RandomNumberGenerator {
  public:
    void seed(int x);
    int operator()();
  };
  const RandomNumberGenerator random_number_generator;
}
