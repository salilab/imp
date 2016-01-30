"""@namespace IMP.pmi.restraints
   Classes to handle different kinds of restraints.

PMI restraints generally wrap IMP restraints. Typical features in PMI restraints are:
 - Easy setup: for example, you can usually create one with a PMI [Molecule](@ref IMP::pmi::topology::Molecule) or a slice from one.
 - Fast setup from data files. For example you can set up the CrossLinkingMassSpectrometryRestraint by reading in a crosslink file into a [database](@ref IMP::pmi::io::crosslink::CrossLinkDataBase) (see [example](@ref examples/cross-link/cross-link_ms.ipynb)).
 - Useful output: reporting functions which are put into log files when running [ReplicaExchange](@ref IMP::pmi::macros::ReplicaExchange0).
"""

import IMP
import IMP.pmi
import IMP.pmi.tools

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
