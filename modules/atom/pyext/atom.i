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
%include "kernel/pyext/IMP_decorators.i"

%include "modules/atom/pyext/atom_config.i"

%include "std_vector.i"
%include "std_string.i"
%include "std_except.i"



/* Get definitions of kernel base classes (but do not wrap) */
%import "kernel/pyext/kernel.i"
%import "kernel/pyext/IMP_keys.i"
%import "modules/algebra/pyext/algebra.i"
%import "modules/core/pyext/core.i"


// must be before hierarchy
%include "IMP/atom/bond_decorators.h"

// it is used as a base class
%include "IMP/atom/Hierarchy.h"

/* Wrap our own classes */
%include "IMP/atom/estimates.h"
%include "IMP/atom/BondEndpointsRefiner.h"
%include "IMP/atom/BondPairContainer.h"
%include "IMP/atom/BondPairFilter.h"
%include "IMP/atom/BondSingletonScore.h"
%include "IMP/atom/CoverBond.h"
%include "IMP/atom/BrownianDynamics.h"
%include "IMP/atom/Diffusion.h"
%include "IMP/atom/Chain.h"
%include "IMP/atom/Domain.h"
%include "IMP/atom/MolecularDynamics.h"
%include "IMP/atom/VelocityScalingOptimizerState.h"
%include "IMP/atom/selectors.h"
%include "IMP/atom/pdb.h"
%include "IMP/atom/Fragment.h"
%include "IMP/atom/SimulationParameters.h"


namespace IMP {
  namespace atom {
    %template(show_molecular_hierarchy) IMP::core::show<IMP::atom::Hierarchy>;
   // swig has random, perplexing issues if these are higher in the file
   %template(AtomTypeBase) ::IMP::KeyBase<IMP_ATOM_TYPE_INDEX>;
   %template(ResidueTypeBase) ::IMP::KeyBase<IMP_RESIDUE_TYPE_INDEX>;
  }
}
%include "IMP/atom/Atom.h"
%include "IMP/atom/Residue.h"
%include "IMP/atom/ForceFieldParameters.h"
%include "IMP/atom/CharmmParameters.h"
%include "IMP/atom/force_fields.h"



namespace IMP {
  namespace atom {
   // must go after the above headers
   %template(HIBase) ::IMP::DecoratorsWithImplicitTraits< IMP::atom::Hierarchy,
                      IMP::core::GenericHierarchies>;
   %template(Hierarchies) ::IMP::Decorators< IMP::atom::Hierarchy,
                                            ::IMP::DecoratorsWithImplicitTraits< IMP::atom::Hierarchy,
                      IMP::core::GenericHierarchies> >;
   %template(HierarchyVector) ::std::vector<IMP::atom::Hierarchy>;
   IMP_DECORATORS(Atom, Atoms, Hierarchies)
   IMP_DECORATORS(Residue, Residues, Hierarchies)
   IMP_DECORATORS(Fragment, Fragments, Hierarchies)
   IMP_DECORATORS(Domain, Domains, Hierarchies)
   IMP_DECORATORS(Chain, Chains, Hierarchies)
   IMP_DECORATORS(Bond, Bonds, Particles)
   IMP_DECORATORS(Bonded, Bondeds, Particles)
   IMP_DECORATORS(Diffusion, Diffusions, core::XYZs)
   IMP_DECORATORS(SimulationParameters, SimulationParameterss, Particles)
  }
}
