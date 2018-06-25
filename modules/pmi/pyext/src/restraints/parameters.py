"""@namespace IMP.pmi.restraints.parameters
Restraints for parameters.
"""

import IMP
import IMP.isd
import IMP.pmi.tools
import IMP.pmi.restraints


class WeightRestraint(IMP.pmi.restraints.RestraintBase):

    """Wrapper for an ``IMP.isd.WeightRestraint``"""

    def __init__(self, w, lower, upper, kappa, label=None, weight=1.):
        """Create restraint on weight parameter.
        @param w ``IMP.isd.Weight`` particle
        @param lower Lower bound on weight restraint
        @param upper Upper bound on weight restraint
        @param kappa Strength of restraint
        @param label A unique label to be used in outputs
        @param weight The weight to apply to all internal restraints
        """
        self.w = w
        model = self.w.get_model()
        super(WeightRestraint, self).__init__(model, label=label,
                                              weight=weight)
        self.lower = lower
        self.upper = upper
        self.kappa = kappa
        self.rs.add_restraint(
            IMP.isd.WeightRestraint(
                self.w,
                self.lower,
                self.upper,
                self.kappa))


class JeffreysPrior(IMP.pmi.restraints.RestraintBase):

    """Wrapper for ``IMP.isd.JeffreysRestraint``"""

    def __init__(self, nuisance, label=None, weight=1.):
        """Create Jeffreys prior.
        @param nuisance Nuisance parameter
        @param label A unique label to be used in outputs
        @param weight The weight to apply to all internal restraints
        """
        model = nuisance.get_model()
        super(JeffreysPrior, self).__init__(model, label=label, weight=weight)
        jp = IMP.isd.JeffreysRestraint(self.model, nuisance)
        self.rs.add_restraint(jp)
