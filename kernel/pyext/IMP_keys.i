%include "IMP/utility.h"
%include "IMP/Key.h"

namespace IMP
{
  %template(FloatKeyBase) ::IMP::KeyBase<0>;
  %template(IntKeyBase) ::IMP::KeyBase<1>;
  %template(StringKeyBase) ::IMP::KeyBase<2>;
  %template(ParticleKeyBase) ::IMP::KeyBase<3>;
}
