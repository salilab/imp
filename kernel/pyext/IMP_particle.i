%pythoncode %{
def _check_particle(p, a):
   if (not p.get_is_active()):
      raise ValueError("Inactive Particle")
   if (type(a)() == a):
      raise IndexError("Cannot use default Index")
   if (not p.has_attribute(a)):
      raise IndexError("Particle does not have attribute")
%}

namespace IMP {
  %extend Particle {
    // Two particles count as equal in Python only if their memory
    // addresses are the same
    bool __eq__(const Particle *other) {
      return self == other;
    }
/*    bool __eq__(IMP::Decorator other) {
      return self == other;
    }*/
  }

  // need to special case particle so can't add this to macro
  %pythonprepend Particle::get_value %{
        try:
          parg=self
          karg=args[0]
        except NameError:
          parg=args[0]
          karg=args[1]
        _check_particle(parg, karg)
  %}
  %pythonprepend Particle::get_is_optimized %{
        try:
          parg=self
          karg=args[0]
        except NameError:
          parg=args[0]
          karg=args[1]
        _check_particle(parg, karg)
  %}
  %pythonprepend Particle::set_is_optimized %{
        try:
          parg=self
          karg=args[0]
        except NameError:
          parg=args[0]
          karg=args[1]
        _check_particle(parg, karg)
  %}
  %pythonprepend Particle::set_value %{
        try:
          parg=self
          karg=args[0]
        except NameError:
          parg=args[0]
          karg=args[1]
        _check_particle(parg, karg)
  %}
  %pythonprepend Particle::add_to_derivative %{
        try:
          parg=self
          karg=args[0]
        except NameError:
          parg=args[0]
          karg=args[1]
        _check_particle(parg, karg)
  %}
  %pythonprepend Particle::get_derivative %{
        try:
          parg=self
          karg=args[0]
        except NameError:
          parg=args[0]
          karg=args[1]
        _check_particle(parg, karg)
  %}
  %pythonprepend Particle::add_attribute %{
        # special case since we don't want to check that the attribute is there
        try:
          parg=self
          karg=args[0]
        except NameError:
          parg=args[0]
          karg=args[1]
        if (not parg.get_is_active()):
           raise ValueError("Inactive Particle")
        elif (type(karg)() == karg):
           raise IndexError("Cannot use default Index")
        elif (parg.has_attribute(karg)):
           raise IndexError("Particle already has attribute")

  %}

}

%include "IMP/Particle.h"
