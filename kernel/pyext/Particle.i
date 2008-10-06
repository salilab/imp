namespace IMP {
  %extend Particle {
    // Two particles count as equal in Python only if their memory
    // addresses are the same
    bool __eq__(const Particle *other) {
      return self == other;
    }
  }
}

%include "IMP/Particle.h"
