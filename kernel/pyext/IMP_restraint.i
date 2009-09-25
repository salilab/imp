namespace IMP {
  %extend Restraint {
    // Two restraints count as equal in Python only if their memory
    // addresses are the same
    bool __eq__(const Restraint &other) {
      return self == &other;
    }
  }
}

%include "IMP/Restraint.h"
