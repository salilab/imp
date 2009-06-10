%module(directors="1") "IMP.atom"

%{
#include "IMP.h"
#include "IMP/core.h"
#include "IMP/algebra.h"
#include "IMP/atom.h"
%}

%include "kernel/pyext/IMP_macros.i"
%include "kernel/pyext/IMP_exceptions.i"
%include "kernel/pyext/IMP_streams.i"

%include "atom_config.i"

%include "std_vector.i"
%include "std_string.i"
%include "std_except.i"



/* Get definitions of kernel base classes (but do not wrap) */
%import "kernel/pyext/IMP.i"
%import "kernel/pyext/IMP_keys.i"
%import "modules/algebra/pyext/algebra.i"
%import "modules/core/pyext/core.i"

/*namespace IMP {
namespace core {
// stupid hack for swig
class XYZ {
public:
void set_x(Float f);
};
}
}*/

// it is used as a base class
%include "IMP/atom/Hierarchy.h"


/* Wrap our own classes */
%include "IMP/atom/estimates.h"
%include "IMP/atom/bond_decorators.h"
%include "IMP/atom/BondEndpointsRefiner.h"
%include "IMP/atom/BondPairContainer.h"
%include "IMP/atom/BondSingletonScore.h"
%include "IMP/atom/BrownianDynamics.h"
%include "IMP/atom/Diffusion.h"
%include "IMP/atom/Chain.h"
%include "IMP/atom/Domain.h"
%include "IMP/atom/MolecularDynamics.h"
%include "IMP/atom/VelocityScalingOptimizerState.h"
%include "IMP/atom/selectors.h"
%include "IMP/atom/pdb.h"
%include "IMP/atom/Fragment.h"

namespace IMP {
  namespace atom {
    %template(show_molecular_hierarchy) IMP::core::show<IMP::atom::Hierarchy>;
    // swig gets scope wrong, I can't fix it
    %template(Bonds) ::std::vector<IMP::atom::Bond>;
  }
}


namespace IMP {
namespace atom {
   // swig has random, perplexing issues if these are higher in the file
   %template(AtomTypeBase) ::IMP::KeyBase<IMP_ATOM_TYPE_INDEX>;
   %template(ResidueTypeBase) ::IMP::KeyBase<IMP_RESIDUE_TYPE_INDEX>;
   %template(Hierarchies) ::std::vector<Hierarchy>;	

}
}
%include "IMP/atom/Atom.h"
%include "IMP/atom/Residue.h"
