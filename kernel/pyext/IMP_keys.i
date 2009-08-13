%include "IMP/utility.h"
%include "IMP/Key.h"

namespace IMP
{
  %template(FloatKeyBase) ::IMP::KeyBase<0, true>;
  %template(IntKeyBase) ::IMP::KeyBase<1, true>;
  %template(StringKeyBase) ::IMP::KeyBase<2, true>;
  %template(ParticleKeyBase) ::IMP::KeyBase<3, true>;
}
