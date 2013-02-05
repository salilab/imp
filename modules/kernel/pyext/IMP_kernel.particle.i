%pythoncode %{
def _check_particle(p, a):
   if (not p.get_is_active()):
      raise ValueError("Inactive Particle")
   if (type(a)() == a):
      raise IndexError("Cannot use default Index")
   if (not p.has_attribute(a)):
      raise IndexError("Particle does not have attribute")
%}



%include "IMP/kernel/declare_Particle.h"
