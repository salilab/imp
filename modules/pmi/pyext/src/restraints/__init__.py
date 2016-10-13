"""@namespace IMP.pmi.restraints
   Classes to handle different kinds of restraints.

PMI restraints generally wrap IMP restraints. Typical features in PMI restraints are:
 - Easy setup: for example, you can usually create one with a PMI [Molecule](@ref IMP::pmi::topology::Molecule) or a slice from one.
 - Fast setup from data files. For example you can set up the [CrossLinkingMassSpectrometryRestraint](@ref IMP::pmi::restraints::crosslinking::CrossLinkingMassSpectrometryRestraint) by reading in a crosslink file into a [database](@ref IMP::pmi::io::crosslink::CrossLinkDataBase).
 - Useful output: reporting functions which are put into log files when running [ReplicaExchange](@ref IMP::pmi::macros::ReplicaExchange0).
"""

import IMP
import IMP.pmi
import IMP.pmi.tools


class RestraintBase(object):

    """Base class for PMI restraints, which wrap `IMP.Restraint`(s)."""

    def __init__(self, m, name=None, label=None, weight=1.):
        """Constructor.
        @param m The model object
        @param name The name of the primary restraint set that is wrapped.
                    This is used for outputs and particle/restraint names
                    and should be set by the child class.
        @param label A unique label to be used in outputs and
                     particle/restraint names.
        @param weight The weight to apply to all internal restraints.
        """
        self.m = m
        self.restraint_sets = []
        self._label_is_set = False
        self.weight = weight
        self._label = None
        self._label_suffix = ""
        self.set_label(label)

        if not name:
            self.name = self.__class__.__name__
        else:
            self.name = str(name)

        self.rs = self._create_restraint_set(name=None)

    def set_label(self, label):
        """Set the unique label used in outputs and particle/restraint names.
        @param label Label
        """
        if self._label_is_set:
            raise ValueError("Label has already been set.")
        if not label:
            self._label = ""
            self._label_suffix = ""
        else:
            self._label = str(label)
            self._label_suffix = "_" + self._label
            self._label_is_set = True

    @property
    def label(self):
        return self._label

    def set_weight(self, weight):
        """Set the weight to apply to all internal restraints.
        @param weight Weight
        """
        self.weight = weight
        for rs in self.restraint_sets:
            rs.set_weight(self.weight)

    def add_to_model(self):
        """Add the restraint to the model."""
        self._label_is_set = True
        for rs in self.restraint_sets:
            IMP.pmi.tools.add_restraint_to_model(self.m, rs)

    def evaluate(self):
        """Evaluate the score of the restraint."""
        self._label_is_set = True
        return self.weight * self.rs.unprotected_evaluate(None)

    def get_restraint_set(self):
        """Get the primary restraint set."""
        self._label_is_set = True
        return self.rs

    def get_restraint(self):
        """Get the primary restraint set. Identical to `get_restraint_set`."""
        return self.get_restraint_set()

    def get_restraint_for_rmf(self):
        """Get the restraint for visualization in an RMF file."""
        self._label_is_set = True
        return self.rs

    def get_particles_to_sample(self):
        """Get any created particles which should be sampled."""
        self._label_is_set = True
        return {}

    def get_output(self):
        """Get outputs to write to stat files."""
        output = {}
        self.m.update()
        score = self.evaluate()
        output["_TotalScore"] = str(score)

        suffix = "_Score" + self._label_suffix
        for rs in self.restraint_sets:
            out_name = rs.get_name() + suffix
            output[out_name] = str(
                self.weight * rs.unprotected_evaluate(None))
        return output

    def _create_restraint_set(self, name=None):
        """Create ``IMP.RestraintSet``."""
        if not name:
            name = self.name
        else:
            name = self.name + "_" + str(name)
        rs = IMP.RestraintSet(self.m, name)
        rs.set_weight(self.weight)
        self.restraint_sets.append(rs)
        return rs


class _RestraintNuisanceMixin(object):

    """Mix-in to add nuisance particle creation functionality to restraint.

    This class must only be inherited if also inheriting
    IMP.pmi.restraints.RestraintBase.
    """

    def __init__(self, *args, **kwargs):
        super(_RestraintNuisanceMixin, self).__init__(*args, **kwargs)
        self.sampled_nuisances = {}
        self.nuisances = {}

    def _create_nuisance(self, init_val, min_val, max_val, max_trans, name,
                         is_sampled=False):
        """Create nuisance particle.
        @param init_val Initial value of nuisance
        @param min_val Minimum value of nuisance
        @param max_val Maximum value of nuisance
        @param max_trans Maximum move to apply to nuisance
        @param is_sampled Nuisance is a sampled particle
        \see IMP.pmi.tools.SetupNuisance
        """
        nuis = IMP.pmi.tools.SetupNuisance(
            self.m, init_val, min_val, max_val,
            isoptimized=is_sampled).get_particle()
        nuis_name = self.name + "_" + name
        nuis.set_name(nuis_name)
        self.nuisances[nuis_name] = nuis
        if is_sampled:
            self.sampled_nuisances[nuis_name] = (nuis, max_trans)
        return nuis

    def get_particles_to_sample(self):
        """Get any created particles which should be sampled."""
        ps = super(_RestraintNuisanceMixin, self).get_particles_to_sample()
        for name, (nuis, max_trans) in self.sampled_nuisances.items():
            ps["Nuisances_" + name + self._label_suffix] = ([nuis], max_trans)
        return ps

    def get_output(self):
        """Get outputs to write to stat files."""
        output = super(_RestraintNuisanceMixin, self).get_output()
        for nuis_name, nuis in self.nuisances.items():
            output[nuis_name + self._label_suffix] = str(nuis.get_scale())
        return output


class _NuisancesBase(object):

    """This base class is used to provide nuisance setup and interface
    for the ISD cross-link restraints"""

    sigma_dictionary = {}
    psi_dictionary = {}

    def create_length(self):
        """Create a nuisance on the length of the cross-link."""
        lengthinit = 10.0
        self.lengthissampled = True
        lengthminnuis = 0.0000001
        lengthmaxnuis = 1000.0
        lengthmin = 6.0
        lengthmax = 30.0
        lengthtrans = 0.2
        length = IMP.pmi.tools.SetupNuisance(self.m, lengthinit,
                                             lengthminnuis, lengthmaxnuis,
                                             self.lengthissampled
                                             ).get_particle()
        self.rslen.add_restraint(
            IMP.isd.UniformPrior(
                self.m,
                length,
                1000000000.0,
                lengthmax,
                lengthmin))

    def create_sigma(self, resolution):
        """Create a nuisance on the structural uncertainty."""
        if isinstance(resolution, str):
            sigmainit = 2.0
        else:
            sigmainit = resolution + 2.0
        self.sigmaissampled = True
        sigmaminnuis = 0.0000001
        sigmamaxnuis = 1000.0
        sigmamin = 0.01
        sigmamax = 100.0
        sigmatrans = 0.5
        sigma = IMP.pmi.tools.SetupNuisance(self.m, sigmainit, sigmaminnuis,
                                            sigmamaxnuis, self.sigmaissampled
                                            ).get_particle()
        self.sigma_dictionary[resolution] = (
            sigma,
            sigmatrans,
            self.sigmaissampled)
        self.rssig.add_restraint(
            IMP.isd.UniformPrior(
                self.m,
                sigma,
                1000000000.0,
                sigmamax,
                sigmamin))
        # self.rssig.add_restraint(IMP.isd.JeffreysRestraint(self.sigma))

    def get_sigma(self, resolution):
        """Get the nuisance on structural uncertainty."""
        if resolution not in self.sigma_dictionary:
            self.create_sigma(resolution)
        return self.sigma_dictionary[resolution]

    def create_psi(self, value):
        """Create a nuisance on the inconsistency."""
        if isinstance(value, str):
            psiinit = 0.5
        else:
            psiinit = value
        self.psiissampled = True
        psiminnuis = 0.0000001
        psimaxnuis = 0.4999999
        psimin = 0.01
        psimax = 0.49
        psitrans = 0.1
        psi = IMP.pmi.tools.SetupNuisance(self.m, psiinit,
                                          psiminnuis, psimaxnuis,
                                          self.psiissampled).get_particle()
        self.psi_dictionary[value] = (
            psi,
            psitrans,
            self.psiissampled)
        self.rspsi.add_restraint(
            IMP.isd.UniformPrior(
                self.m,
                psi,
                1000000000.0,
                psimax,
                psimin))
        self.rspsi.add_restraint(IMP.isd.JeffreysRestraint(self.m, psi))

    def get_psi(self, value):
        """Get the nuisance on the inconsistency."""
        if value not in self.psi_dictionary:
            self.create_psi(value)
        return self.psi_dictionary[value]
