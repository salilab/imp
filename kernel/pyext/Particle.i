namespace IMP {
  %extend Particle {
    %pythoncode {
      def __eq__(self, other):
          """Particles are equal only if they are the same particle.
             This gives the same behavior as the C++ code."""
          return self.this == other.this
    }
  }
}

%include "IMP/Particle.h"
