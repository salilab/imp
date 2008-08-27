%include "IMP/Key.h"

namespace IMP
{
  %template(FloatKeyBase) ::IMP::KeyBase<0>;
  %template(IntKeyBase) ::IMP::KeyBase<1>;
  %template(StringKeyBase) ::IMP::KeyBase<2>;
  %template(ParticleKeyBase) ::IMP::KeyBase<3>;
  %template(AtomTypeBase) ::IMP::KeyBase<IMP_ATOM_TYPE_INDEX>;
  %template(ResidueTypeBase) ::IMP::KeyBase<IMP_RESIDUE_TYPE_INDEX>;
}
