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

    def __init__(self, m, rname=None):
        """Constructor.
        @param m The model object
        @param rname The name of the primary restraint set that is wrapped.
        """
        self.m = m
        self.label = None
        self.weight = 1.
        if rname is None:
            rname = self.__class__.__name__
        self.rs = IMP.RestraintSet(self.m, rname)
        self.restraint_sets = [self.rs]

    def set_weight(self, weight):
        """Set the weight of the restraint.
        @param weight Restraint weight
        """
        self.weight = weight
        self.rs.set_weight(weight)

    def set_label(self, label):
        """Set the label used in outputs.
        @param label Label
        """
        self.label = label

    def add_to_model(self):
        """Add the restraint to the model."""
        IMP.pmi.tools.add_restraint_to_model(self.m, self.rs)

    def evaluate(self):
        """Evaluate the score of the restraint."""
        return self.weight * self.rs.unprotected_evaluate(None)

    def get_restraint_set(self):
        """Get the primary restraint set."""
        return self.rs

    def get_restraint(self):
        """Get the primary restraint set. Identical to `get_restraint_set`."""
        return self.get_restraint_set()

    def get_restraint_for_rmf(self):
        """Get the restraint for visualization in an RMF file."""
        return self.rs

    def get_particles_to_sample(self):
        """Get any created particles which should be sampled."""
        return {}

    def get_output(self):
        """Get outputs to write to stat files."""
        self.m.update()
        output = {}
        score = self.evaluate()
        output["_TotalScore"] = str(score)

        suffix = "_Score"
        if self.label is not None:
            suffix += "_" + str(self.label)

        for rs in self.restraint_sets:
            out_name = rs.get_name() + suffix
            output[out_name] = str(
                self.weight * rs.unprotected_evaluate(None))

        return output


class _NuisancesBase(object):
    ''' This base class is used to provide nuisance setup and interface
    for the ISD cross-link restraints '''
    sigma_dictionary={}
    psi_dictionary={}

    def create_length(self):
        ''' a nuisance on the length of the cross-link '''
        lengthinit = 10.0
        self.lengthissampled = True
        lengthminnuis = 0.0000001
        lengthmaxnuis = 1000.0
        lengthmin = 6.0
        lengthmax = 30.0
        lengthtrans = 0.2
        length = IMP.pmi.tools.SetupNuisance(self.m, lengthinit,
                                                    lengthminnuis,
                                                    lengthmaxnuis,
                                                    lengthissampled).get_particle()
        self.rslen.add_restraint(
            IMP.isd.UniformPrior(
                self.m,
                length,
                1000000000.0,
                lengthmax,
                lengthmin))

    def create_sigma(self, resolution):
        ''' a nuisance on the structural uncertainty '''
        if isinstance(resolution,str):
            sigmainit = 2.0
        else:
            sigmainit = resolution + 2.0
        self.sigmaissampled = True
        sigmaminnuis = 0.0000001
        sigmamaxnuis = 1000.0
        sigmamin = 0.01
        sigmamax = 100.0
        sigmatrans = 0.5
        sigma = IMP.pmi.tools.SetupNuisance(self.m, sigmainit,
                                                 sigmaminnuis, sigmamaxnuis, self.sigmaissampled).get_particle()
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
        if not resolution in self.sigma_dictionary:
            self.create_sigma(resolution)
        return self.sigma_dictionary[resolution]

    def create_psi(self, value):
        ''' a nuisance on the inconsistency '''
        if isinstance(value,str):
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
        if not value in self.psi_dictionary:
            self.create_psi(value)
        return self.psi_dictionary[value]
