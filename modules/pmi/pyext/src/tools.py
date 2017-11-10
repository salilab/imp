#!/usr/bin/env python

"""@namespace IMP.pmi.tools
   Miscellaneous utilities.
"""

from __future__ import print_function
import IMP
import IMP.algebra
import IMP.isd
import IMP.pmi
import IMP.pmi.topology
import collections
import itertools
from math import log,pi,sqrt,exp
import sys,os
import random
import ast
import time
import RMF
import IMP.rmf
from collections import defaultdict
try:
    from collections import OrderedDict
except ImportError:
    from IMP.pmi._compat_collections import OrderedDict

def _add_pmi_provenance(p):
    """Tag the given particle as being created by the current version of PMI."""
    IMP.core.add_imp_provenance(p)
    IMP.core.add_software_provenance(p, name="IMP PMI module",
                                     version=IMP.pmi.get_module_version(),
                                     location="https://integrativemodeling.org")
    IMP.core.add_script_provenance(p)

def _get_restraint_set_key():
    if not hasattr(_get_restraint_set_key, 'pmi_rs_key'):
        _get_restraint_set_key.pmi_rs_key = IMP.ModelKey("PMI restraints")
    return _get_restraint_set_key.pmi_rs_key

def _add_restraint_set(model, mk):
    rs = IMP.RestraintSet(model, "All PMI restraints")
    model.add_data(mk, rs)
    return rs

def add_restraint_to_model(model, restraint):
    """Add a PMI restraint to the model.
       Since Model.add_restraint() no longer exists (in modern IMP restraints
       should be added to a ScoringFunction instead) store them instead in
       a RestraintSet, and keep a reference to it in the Model."""
    mk = _get_restraint_set_key()
    if model.get_has_data(mk):
        rs = IMP.RestraintSet.get_from(model.get_data(mk))
    else:
        rs = _add_restraint_set(model, mk)
    rs.add_restraint(restraint)

def get_restraint_set(model):
    """Get a RestraintSet containing all PMI restraints added to the model"""
    mk = _get_restraint_set_key()
    if not model.get_has_data(mk):
        print("WARNING: no restraints added to model yet")
        _add_restraint_set(model, mk)
    return IMP.RestraintSet.get_from(model.get_data(mk))

class Stopwatch(object):
    """Collect timing information.
       Add an instance of this class to outputobjects to get timing information
       in a stat file."""

    def __init__(self, isdelta=True):
        """Constructor.
           @param isdelta if True (the default) then report the time since the
                  last use of this class; if False, report cumulative time."""
        self.starttime = time.clock()
        self.label = "None"
        self.isdelta = isdelta

    def set_label(self, labelstr):
        self.label = labelstr

    def get_output(self):
        output = {}
        if self.isdelta:
            newtime = time.clock()
            output["Stopwatch_" + self.label + "_delta_seconds"] \
                    = str(newtime - self.starttime)
            self.starttime = newtime
        else:
            output["Stopwatch_" + self.label + "_elapsed_seconds"] \
                    = str(time.clock() - self.starttime)
        return output


class SetupNuisance(object):

    def __init__(self, m, initialvalue, minvalue, maxvalue, isoptimized=True):

        nuisance = IMP.isd.Scale.setup_particle(IMP.Particle(m), initialvalue)
        if minvalue:
            nuisance.set_lower(minvalue)
        if maxvalue:
            nuisance.set_upper(maxvalue)

        # m.add_score_state(IMP.core.SingletonConstraint(IMP.isd.NuisanceRangeModifier(),None,nuisance))
        nuisance.set_is_optimized(nuisance.get_nuisance_key(), isoptimized)
        self.nuisance = nuisance

    def get_particle(self):
        return self.nuisance


class SetupWeight(object):

    def __init__(self, m, isoptimized=True):
        pw = IMP.Particle(m)
        self.weight = IMP.isd.Weight.setup_particle(pw)
        self.weight.set_weights_are_optimized(True)

    def get_particle(self):
        return self.weight


class SetupSurface(object):

    def __init__(self, m, center, normal, isoptimized=True):
        p = IMP.Particle(m)
        self.surface = IMP.core.Surface.setup_particle(p, center, normal)
        self.surface.set_coordinates_are_optimized(isoptimized)
        self.surface.set_normal_is_optimized(isoptimized)

    def get_particle(self):
        return self.surface


@IMP.deprecated_object(2.8,
        "If you use this class please let the PMI developers know.")
class ParticleToSampleFilter(object):
    def __init__(self, sampled_objects):
        self.sampled_objects=sampled_objects
        self.filters=[]

    def add_filter(self,filter_string):
        self.filters.append(filter_string)

    def get_particles_to_sample(self):
        particles_to_sample={}
        for so in self.sampled_objects:
            ps_dict=so.get_particles_to_sample()
            for key in ps_dict:
                for f in self.filters:
                    if f in key:
                        if key not in particles_to_sample:
                            particles_to_sample[key]=ps_dict[key]
                        else:
                            particles_to_sample[key]+=ps_dict[key]
        return particles_to_sample

class ParticleToSampleList(object):

    def __init__(self, label="None"):

        self.dictionary_particle_type = {}
        self.dictionary_particle_transformation = {}
        self.dictionary_particle_name = {}
        self.label = label

    def add_particle(
        self,
        particle,
        particle_type,
        particle_transformation,
            name):
        if not particle_type in ["Rigid_Bodies", "Floppy_Bodies", "Nuisances", "X_coord", "Weights", "Surfaces"]:
            raise TypeError("not the right particle type")
        else:
            self.dictionary_particle_type[particle] = particle_type
            if particle_type == "Rigid_Bodies":
                if type(particle_transformation) == tuple and len(particle_transformation) == 2 and type(particle_transformation[0]) == float and type(particle_transformation[1]) == float:
                    self.dictionary_particle_transformation[
                        particle] = particle_transformation
                    self.dictionary_particle_name[particle] = name
                else:
                    raise TypeError("ParticleToSampleList: not the right transformation format for Rigid_Bodies, should be a tuple of floats")
            elif particle_type == "Surfaces":
                if type(particle_transformation) == tuple and len(particle_transformation) == 3 and all(isinstance(x, float) for x in particle_transformation):
                    self.dictionary_particle_transformation[
                        particle] = particle_transformation
                    self.dictionary_particle_name[particle] = name
                else:
                    raise TypeError("ParticleToSampleList: not the right transformation format for Surfaces, should be a tuple of floats")
            else:
                if type(particle_transformation) == float:
                    self.dictionary_particle_transformation[
                        particle] = particle_transformation
                    self.dictionary_particle_name[particle] = name
                else:
                    raise TypeError("ParticleToSampleList: not the right transformation format, should be a float")

    def get_particles_to_sample(self):
        ps = {}
        for particle in self.dictionary_particle_type:
            key = self.dictionary_particle_type[
                particle] + "ParticleToSampleList_" + self.dictionary_particle_name[particle] + "_" + self.label
            value = (
                [particle],
                self.dictionary_particle_transformation[particle])
            ps[key] = value
        return ps


class Variance(object):

    def __init__(self, model, tau, niter, prot, th_profile, write_data=False):

        self.model = model
        self.write_data = write_data
        self.tau = tau
        self.niter = niter
        #! select particles from the model
        particles = IMP.atom.get_by_type(prot, IMP.atom.ATOM_TYPE)
        self.particles = particles
        # store reference coordinates and theoretical profile
        self.refpos = [IMP.core.XYZ(p).get_coordinates() for p in particles]
        self.model_profile = th_profile

    def perturb_particles(self, perturb=True):
        for i, p in enumerate(self.particles):
            newpos = array(self.refpos[i])
            if perturb:
                newpos += random.normal(0, self.tau, 3)
            newpos = IMP.algebra.Vector3D(newpos)
            IMP.core.XYZ(p).set_coordinates(newpos)

    def get_profile(self):
        model_profile = self.model_profile
        p = model_profile.calculate_profile(self.particles, IMP.saxs.CA_ATOMS)
        return array([model_profile.get_intensity(i) for i in
                      range(model_profile.size())])

    def init_variances(self):
        # create placeholders
        N = self.model_profile.size()
        a = self.profiles[0][:]
        self.m = matrix(a).T  # Nx1
        self.V = self.m * self.m.T
        self.normm = linalg.norm(self.m)
        self.normV = linalg.norm(self.V)

    def update_variances(self):
        a = matrix(self.profiles[-1])  # 1xN
        n = float(len(self.profiles))
        self.m = a.T / n + (n - 1) / n * self.m
        self.V = a.T * a + self.V
        self.oldnormm = self.normm
        self.oldnormV = self.normV
        self.normm = linalg.norm(self.m)
        self.normV = linalg.norm(self.V)
        self.diffm = (self.oldnormm - self.normm) / self.oldnormm
        self.diffV = (self.oldnormV - self.normV) / self.oldnormV

    def get_direct_stats(self, a):
        nq = len(a[0])
        nprof = len(a)
        m = [0] * nq
        for prof in a:
            for q, I in enumerate(prof):
                m[q] += I
        m = array(m) / nprof
        V = matrix(a)
        V = V.T * V
        Sigma = (matrix(a - m))
        Sigma = Sigma.T * Sigma / (nprof - 1)
        mi = matrix(diag(1. / m))
        Sigmarel = mi.T * Sigma * mi
        return m, V, Sigma, Sigmarel

    def store_data(self):
        if not os.path.isdir('data'):
            os.mkdir('data')
        profiles = matrix(self.profiles)
        self.directm, self.directV, self.Sigma, self.Sigmarel = \
            self.get_direct_stats(array(profiles))
        directV = self.directV
        # print "V comparison",(linalg.norm(directV-self.V)/self.normV)
        save('data/profiles', profiles)
        # absolute profile differences
        fl = open('data/profiles.dat', 'w')
        for i, l in enumerate(array(profiles).T):
            self.model_profile.get_q(i)
            fl.write('%s ' % i)
            for k in l:
                fl.write('%s ' % (k - self.directm[i]))
            fl.write('\n')
        # relative profile differences
        fl = open('data/profiles_rel.dat', 'w')
        for i, l in enumerate(array(profiles).T):
            self.model_profile.get_q(i)
            fl.write('%s ' % i)
            for k in l:
                fl.write('%s ' % ((k - self.directm[i]) / self.directm[i]))
            fl.write('\n')
        save('data/m', self.directm)
        save('data/V', self.directV)
        Sigma = self.Sigma
        save('data/Sigma', Sigma)
        # Sigma matrix
        fl = open('data/Sigma.dat', 'w')
        model_profile = self.model_profile
        for i in range(model_profile.size()):
            qi = model_profile.get_q(i)
            for j in range(model_profile.size()):
                qj = model_profile.get_q(j)
                vij = self.Sigma[i, j]
                fl.write('%s %s %s\n' % (qi, qj, vij))
            fl.write('\n')
        # Sigma eigenvalues
        fl = open('data/eigenvals', 'w')
        for i in linalg.eigvalsh(Sigma):
            fl.write('%s\n' % i)
        Sigmarel = self.Sigmarel
        save('data/Sigmarel', Sigmarel)
        # Sigmarel matrix
        fl = open('data/Sigmarel.dat', 'w')
        model_profile = self.model_profile
        for i in range(model_profile.size()):
            qi = model_profile.get_q(i)
            for j in range(model_profile.size()):
                qj = model_profile.get_q(j)
                vij = self.Sigmarel[i, j]
                fl.write('%s %s %s\n' % (qi, qj, vij))
            fl.write('\n')
        # Sigma eigenvalues
        fl = open('data/eigenvals_rel', 'w')
        for i in linalg.eigvalsh(Sigmarel):
            fl.write('%s\n' % i)
        # mean profile
        fl = open('data/mean.dat', 'w')
        for i in range(len(self.directm)):
            qi = self.model_profile.get_q(i)
            fl.write('%s ' % qi)
            fl.write('%s ' % self.directm[i])
            fl.write('%s ' % sqrt(self.Sigma[i, i]))
            fl.write('\n')

    def try_chol(self, jitter):
        Sigma = self.Sigma
        try:
            linalg.cholesky(Sigma + matrix(eye(len(Sigma))) * jitter)
        except linalg.LinAlgError:
            print("Decomposition failed with jitter =", jitter)
            return
        print("Successful decomposition with jitter =", jitter)

    def run(self):
        self.profiles = [self.get_profile()]
        # self.init_variances()
        for n in range(self.niter):
            self.perturb_particles()
            self.profiles.append(self.get_profile())
            # self.update_variances()
            #profiles = matrix(self.profiles)
            # print n,self.diffm,self.diffV
        # print
        #
        if self.write_data:
            self.store_data()
        # self.try_chol(0.)
        # for i in logspace(-7,0,num=8):
        #    self.try_chol(i)

    def get_cov(self, relative=True):
        if not relative:
            return self.Sigma
        else:
            return self.Sigmarel

    #-------------------------------

def get_random_cross_link_dataset(representation,
                                  resolution=1.0,
                                  number_of_cross_links=10,
                                  ambiguity_probability=0.1,
                                  confidence_score_range=[0,100],
                                  avoid_same_particles=False):
    '''Return a random cross-link dataset as a string.
    Every line is a residue pair, together with UniqueIdentifier
    and XL score.'''

    residue_pairs=get_random_residue_pairs(representation, resolution, number_of_cross_links, avoid_same_particles=avoid_same_particles)

    unique_identifier=0
    cmin=float(min(confidence_score_range))
    cmax=float(max(confidence_score_range))

    dataset="#\n"

    for (name1, r1, name2, r2) in residue_pairs:
        if random.random() > ambiguity_probability:
            unique_identifier+=1
        score=random.random()*(cmax-cmin)+cmin
        dataset+=str(name1)+" "+str(name2)+" "+str(r1)+" "+str(r2)+" "+str(score)+" "+str(unique_identifier)+"\n"

    return dataset


    #-------------------------------

def get_cross_link_data(directory, filename, dist, omega, sigma,
                        don=None, doff=None, prior=0, type_of_profile="gofr"):

    (distmin, distmax, ndist) = dist
    (omegamin, omegamax, nomega) = omega
    (sigmamin, sigmamax, nsigma) = sigma

    filen = IMP.isd.get_data_path("CrossLinkPMFs.dict")
    with open(filen) as xlpot:
        dictionary = ast.literal_eval(xlpot.readline())

    xpot = dictionary[directory][filename]["distance"]
    pot = dictionary[directory][filename][type_of_profile]

    dist_grid = get_grid(distmin, distmax, ndist, False)
    omega_grid = get_log_grid(omegamin, omegamax, nomega)
    sigma_grid = get_log_grid(sigmamin, sigmamax, nsigma)

    if not don is None and not doff is None:
        xlmsdata = IMP.isd.CrossLinkData(
            dist_grid,
            omega_grid,
            sigma_grid,
            xpot,
            pot,
            don,
            doff,
            prior)
    else:
        xlmsdata = IMP.isd.CrossLinkData(
            dist_grid,
            omega_grid,
            sigma_grid,
            xpot,
            pot)
    return xlmsdata

    #-------------------------------


def get_cross_link_data_from_length(length, xxx_todo_changeme3, xxx_todo_changeme4, xxx_todo_changeme5):
    (distmin, distmax, ndist) = xxx_todo_changeme3
    (omegamin, omegamax, nomega) = xxx_todo_changeme4
    (sigmamin, sigmamax, nsigma) = xxx_todo_changeme5

    dist_grid = get_grid(distmin, distmax, ndist, False)
    omega_grid = get_log_grid(omegamin, omegamax, nomega)
    sigma_grid = get_log_grid(sigmamin, sigmamax, nsigma)

    xlmsdata = IMP.isd.CrossLinkData(dist_grid, omega_grid, sigma_grid, length)
    return xlmsdata


def get_grid(gmin, gmax, ngrid, boundaries):
    grid = []
    dx = (gmax - gmin) / float(ngrid)
    for i in range(0, ngrid + 1):
        if(not boundaries and i == 0):
            continue
        if(not boundaries and i == ngrid):
            continue
        grid.append(gmin + float(i) * dx)
    return grid

    #-------------------------------


def get_log_grid(gmin, gmax, ngrid):
    grid = []
    for i in range(0, ngrid + 1):
        grid.append(gmin * exp(float(i) / ngrid * log(gmax / gmin)))
    return grid

    #-------------------------------


def cross_link_db_filter_parser(inputstring):
    '''
    example '"{ID_Score}" > 28 AND "{Sample}" ==
     "%10_1%" OR ":Sample}" == "%10_2%" OR ":Sample}"
    == "%10_3%" OR ":Sample}" == "%8_1%" OR ":Sample}" == "%8_2%"'
    '''

    import pyparsing as pp

    operator = pp.Regex(">=|<=|!=|>|<|==|in").setName("operator")
    value = pp.QuotedString(
        '"') | pp.Regex(
        r"[+-]?\d+(:?\.\d*)?(:?[eE][+-]?\d+)?")
    identifier = pp.Word(pp.alphas, pp.alphanums + "_")
    comparison_term = identifier | value
    condition = pp.Group(comparison_term + operator + comparison_term)

    expr = pp.operatorPrecedence(condition, [
                                ("OR", 2, pp.opAssoc.LEFT, ),
        ("AND", 2, pp.opAssoc.LEFT, ),
    ])

    parsedstring = str(expr.parseString(inputstring)) \
        .replace("[", "(") \
        .replace("]", ")") \
        .replace(",", " ") \
        .replace("'", " ") \
        .replace("%", "'") \
        .replace("{", "float(entry['") \
        .replace("}", "'])") \
        .replace(":", "str(entry['") \
        .replace("}", "'])") \
        .replace("AND", "and") \
        .replace("OR", "or")
    return parsedstring


def open_file_or_inline_text(filename):
    try:
        fl = open(filename, "r")
    except IOError:
        fl = filename.split("\n")
    return fl


def get_drmsd(prot0, prot1):
    drmsd = 0.
    npairs = 0.
    for i in range(0, len(prot0) - 1):
        for j in range(i + 1, len(prot0)):
            dist0 = IMP.core.get_distance(prot0[i], prot0[j])
            dist1 = IMP.core.get_distance(prot1[i], prot1[j])
            drmsd += (dist0 - dist1) ** 2
            npairs += 1.
    return math.sqrt(drmsd / npairs)

    #-------------------------------


def get_ids_from_fasta_file(fastafile):
    ids = []
    with open(fastafile) as ff:
        for l in ff:
            if l[0] == ">":
                ids.append(l[1:-1])
    return ids


def get_closest_residue_position(hier, resindex, terminus="N"):
    '''
    this function works with plain hierarchies, as read from the pdb,
    no multi-scale hierarchies
    '''
    p = []
    niter = 0
    while len(p) == 0:
        niter += 1
        sel = IMP.atom.Selection(hier, residue_index=resindex,
                                 atom_type=IMP.atom.AT_CA)

        if terminus == "N":
            resindex += 1
        if terminus == "C":
            resindex -= 1

        if niter >= 10000:
            print("get_closest_residue_position: exiting while loop without result")
            break
        p = sel.get_selected_particles()

    if len(p) == 1:
        return IMP.core.XYZ(p[0]).get_coordinates()
    elif len(p) == 0:
        print("get_closest_residue_position: got NO residues for hierarchy %s and residue %i" % (hier, resindex))
        raise Exception("get_closest_residue_position: got NO residues for hierarchy %s and residue %i" % (
            hier, resindex))
    else:
        raise ValueError("got multiple residues for hierarchy %s and residue %i; the list of particles is %s" % (hier, resindex, str([pp.get_name() for pp in p])))

def get_terminal_residue(representation, hier, terminus="C", resolution=1):
    '''
    Get the particle of the terminal residue at the GIVEN resolution
    (NOTE: not the closest resolution!).
    To get the terminal residue at the closest resolution use:
    particles=IMP.pmi.tools.select_by_tuple(representation,molecule_name)
    particles[0] and particles[-1] will be the first and last particles
    corresponding to the two termini.
    It is needed for instance to determine the last residue of a pdb.
    @param hier hierarchy containing the terminal residue
    @param terminus either 'N' or 'C'
    @param resolution resolution to use.
    '''
    termresidue = None
    termparticle = None

    ps=select(representation,
           resolution=resolution,
           hierarchies=[hier])

    for p in ps:
        if IMP.pmi.Resolution(p).get_resolution() == resolution:
            residues = IMP.pmi.tools.get_residue_indexes(p)
            if terminus == "C":
                if termresidue is None:
                    termresidue = max(residues)
                    termparticle = p
                elif max(residues) >= termresidue:
                    termresidue = max(residues)
                    termparticle = p
            elif terminus == "N":
                if termresidue is None:
                    termresidue = min(residues)
                    termparticle = p
                elif min(residues) <= termresidue:
                    termresidue = min(residues)
                    termparticle = p
            else:
                raise ValueError("terminus argument should be either N or C")
    return termparticle

def get_terminal_residue_position(representation, hier, terminus="C",
                                  resolution=1):
    """Get XYZ coordinates of the terminal residue at the GIVEN resolution"""
    p = get_terminal_residue(representation, hier, terminus, resolution)
    return IMP.core.XYZ(p).get_coordinates()

def get_residue_gaps_in_hierarchy(hierarchy, start, end):
    '''
    Return the residue index gaps and contiguous segments in the hierarchy.

    @param hierarchy hierarchy to examine
    @param start first residue index
    @param end last residue index

    @return A list of lists of the form
            [[1,100,"cont"],[101,120,"gap"],[121,200,"cont"]]
    '''
    gaps = []
    for n, rindex in enumerate(range(start, end + 1)):
        sel = IMP.atom.Selection(hierarchy, residue_index=rindex,
                                 atom_type=IMP.atom.AT_CA)

        if len(sel.get_selected_particles()) == 0:
            if n == 0:
                # set the initial condition
                rindexgap = start
                rindexcont = start - 1
            if rindexgap == rindex - 1:
                # residue is contiguous with the previously discovered gap
                gaps[-1][1] += 1
            else:
                # residue is not contiguous with the previously discovered gap
                # hence create a new gap tuple
                gaps.append([rindex, rindex, "gap"])
            # update the index of the last residue gap
            rindexgap = rindex
        else:
            if n == 0:
                # set the initial condition
                rindexgap = start - 1
                rindexcont = start
            if rindexcont == rindex - 1:
                # residue is contiguous with the previously discovered
                # continuous part
                gaps[-1][1] += 1
            else:
                # residue is not contiguous with the previously discovered continuous part
                # hence create a new cont tuple
                gaps.append([rindex, rindex, "cont"])
            # update the index of the last residue gap
            rindexcont = rindex
    return gaps


class map(object):

    def __init__(self):
        self.map = {}

    def set_map_element(self, xvalue, yvalue):
        self.map[xvalue] = yvalue

    def get_map_element(self, invalue):
        if type(invalue) == float:
            n = 0
            mindist = 1
            for x in self.map:
                dist = (invalue - x) * (invalue - x)

                if n == 0:
                    mindist = dist
                    minx = x
                if dist < mindist:
                    mindist = dist
                    minx = x
                n += 1
            return self.map[minx]
        elif type(invalue) == str:
            return self.map[invalue]
        else:
            raise TypeError("wrong type for map")


def select(representation,
           resolution=None,
           hierarchies=None,
           selection_arguments=None,
           name=None,
           name_is_ambiguous=False,
           first_residue=None,
           last_residue=None,
           residue=None,
           representation_type=None):
    '''
    this function uses representation=SimplifiedModel
    it returns the corresponding selected particles
    representation_type="Beads", "Res:X", "Densities", "Representation", "Molecule"
    '''

    if resolution is None:
        allparticles = IMP.atom.get_leaves(representation.prot)
    resolution_particles = None
    hierarchies_particles = None
    names_particles = None
    residue_range_particles = None
    residue_particles = None
    representation_type_particles = None

    if not resolution is None:
        resolution_particles = []
        hs = representation.get_hierarchies_at_given_resolution(resolution)
        for h in hs:
            resolution_particles += IMP.atom.get_leaves(h)

    if not hierarchies is None:
        hierarchies_particles = []
        for h in hierarchies:
            hierarchies_particles += IMP.atom.get_leaves(h)

    if not name is None:
        names_particles = []
        if name_is_ambiguous:
            for namekey in representation.hier_dict:
                if name in namekey:
                    names_particles += IMP.atom.get_leaves(
                        representation.hier_dict[namekey])
        elif name in representation.hier_dict:
            names_particles += IMP.atom.get_leaves(representation.hier_dict[name])
        else:
            print("select: component %s is not there" % name)

    if not first_residue is None and not last_residue is None:
        sel = IMP.atom.Selection(representation.prot,
                                 residue_indexes=range(first_residue, last_residue + 1))
        residue_range_particles = [IMP.atom.Hierarchy(p)
                                   for p in sel.get_selected_particles()]

    if not residue is None:
        sel = IMP.atom.Selection(representation.prot, residue_index=residue)
        residue_particles = [IMP.atom.Hierarchy(p)
                             for p in sel.get_selected_particles()]

    if not representation_type is None:
        representation_type_particles = []
        if representation_type == "Molecule":
            for name in representation.hier_representation:
                for repr_type in representation.hier_representation[name]:
                    if repr_type == "Beads" or "Res:" in repr_type:
                        h = representation.hier_representation[name][repr_type]
                        representation_type_particles += IMP.atom.get_leaves(h)

        elif representation_type == "PDB":
            for name in representation.hier_representation:
                for repr_type in representation.hier_representation[name]:
                    if repr_type == "Res:" in repr_type:
                        h = representation.hier_representation[name][repr_type]
                        representation_type_particles += IMP.atom.get_leaves(h)

        else:
            for name in representation.hier_representation:
                h = representation.hier_representation[
                    name][
                    representation_type]
                representation_type_particles += IMP.atom.get_leaves(h)

    selections = [hierarchies_particles, names_particles,
                  residue_range_particles, residue_particles, representation_type_particles]

    if resolution is None:
        selected_particles = set(allparticles)
    else:
        selected_particles = set(resolution_particles)

    for s in selections:
        if not s is None:
            selected_particles = (set(s) & selected_particles)

    return list(selected_particles)


def select_by_tuple(
    representation,
    tupleselection,
    resolution=None,
        name_is_ambiguous=False):
    if isinstance(tupleselection, tuple) and len(tupleselection) == 3:
        particles = IMP.pmi.tools.select(representation, resolution=resolution,
                                         name=tupleselection[2],
                                         first_residue=tupleselection[0],
                                         last_residue=tupleselection[1],
                                         name_is_ambiguous=name_is_ambiguous)
    elif isinstance(tupleselection, str):
        particles = IMP.pmi.tools.select(representation, resolution=resolution,
                                         name=tupleselection,
                                         name_is_ambiguous=name_is_ambiguous)
    else:
        raise ValueError('you passed something bad to select_by_tuple()')
    # now order the result by residue number
    particles = IMP.pmi.tools.sort_by_residues(particles)

    return particles

def select_by_tuple_2(hier,tuple_selection,resolution):
    """New tuple format: molname OR (start,stop,molname,copynum,statenum)
    Copy and state are optional. Can also use 'None' for them which will get all.
    You can also pass -1 for stop which will go to the end.
    Returns the particles
    """
    kwds = {} # going to accumulate keywords
    kwds['resolution'] = resolution
    if type(tuple_selection) is str:
        kwds['molecule'] = tuple_selection
    elif type(tuple_selection) is tuple:
        rbegin = tuple_selection[0]
        rend = tuple_selection[1]
        kwds['molecule'] = tuple_selection[2]
        try:
            copynum = tuple_selection[3]
            if copynum is not None:
                kwds['copy_index'] = copynum
        except:
            pass
        try:
            statenum = tuple_selection[4]
            if statenum is not None:
                kwds['state_index'] = statenum
        except:
            pass
        if rend==-1:
            if rbegin>1:
                s = IMP.atom.Selection(hier,**kwds)
                s -= IMP.atom.Selection(hier,
                                        residue_indexes=range(1,rbegin),
                                        **kwds)
                return s.get_selected_particles()
        else:
            kwds['residue_indexes'] = range(rbegin,rend+1)
    s = IMP.atom.Selection(hier,**kwds)
    return s.get_selected_particles()



def get_db_from_csv(csvfilename):
    import csv
    outputlist = []
    with open(csvfilename) as fh:
        csvr = csv.DictReader(fh)
        for l in csvr:
            outputlist.append(l)
    return outputlist


class HierarchyDatabase(object):
    """Store the representations for a system."""

    def __init__(self):
        self.db = {}
        # this dictionary map a particle to its root hierarchy
        self.root_hierarchy_dict = {}
        self.preroot_fragment_hierarchy_dict = {}
        self.particle_to_name = {}
        self.model = None

    def add_name(self, name):
        if name not in self.db:
            self.db[name] = {}

    def add_residue_number(self, name, resn):
        resn = int(resn)
        self.add_name(name)
        if resn not in self.db[name]:
            self.db[name][resn] = {}

    def add_resolution(self, name, resn, resolution):
        resn = int(resn)
        resolution = float(resolution)
        self.add_name(name)
        self.add_residue_number(name, resn)
        if resolution not in self.db[name][resn]:
            self.db[name][resn][resolution] = []

    def add_particles(self, name, resn, resolution, particles):
        resn = int(resn)
        resolution = float(resolution)
        self.add_name(name)
        self.add_residue_number(name, resn)
        self.add_resolution(name, resn, resolution)
        self.db[name][resn][resolution] += particles
        for p in particles:
            (rh, prf) = self.get_root_hierarchy(p)
            self.root_hierarchy_dict[p] = rh
            self.preroot_fragment_hierarchy_dict[p] = prf
            self.particle_to_name[p] = name
        if self.model is None:
            self.model = particles[0].get_model()

    def get_model(self):
        return self.model

    def get_names(self):
        names = list(self.db.keys())
        names.sort()
        return names

    def get_particles(self, name, resn, resolution):
        resn = int(resn)
        resolution = float(resolution)
        return self.db[name][resn][resolution]

    def get_particles_at_closest_resolution(self, name, resn, resolution):
        resn = int(resn)
        resolution = float(resolution)
        closestres = min(self.get_residue_resolutions(name, resn),
                         key=lambda x: abs(float(x) - float(resolution)))
        return self.get_particles(name, resn, closestres)

    def get_residue_resolutions(self, name, resn):
        resn = int(resn)
        resolutions = list(self.db[name][resn].keys())
        resolutions.sort()
        return resolutions

    def get_molecule_resolutions(self, name):
        resolutions = set()
        for resn in self.db[name]:
            resolutions.update(list(self.db[name][resn].keys()))
        resolutions.sort()
        return resolutions

    def get_residue_numbers(self, name):
        residue_numbers = list(self.db[name].keys())
        residue_numbers.sort()
        return residue_numbers

    def get_particles_by_resolution(self, name, resolution):
        resolution = float(resolution)
        particles = []
        for resn in self.get_residue_numbers(name):
            result = self.get_particles_at_closest_resolution(
                name,
                resn,
                resolution)
            pstemp = [p for p in result if p not in particles]
            particles += pstemp
        return particles

    def get_all_particles_by_resolution(self, resolution):
        resolution = float(resolution)
        particles = []
        for name in self.get_names():
            particles += self.get_particles_by_resolution(name, resolution)
        return particles

    def get_root_hierarchy(self, particle):
        prerootfragment = particle
        while IMP.atom.Atom.get_is_setup(particle) or \
            IMP.atom.Residue.get_is_setup(particle) or \
                IMP.atom.Fragment.get_is_setup(particle):
            if IMP.atom.Atom.get_is_setup(particle):
                p = IMP.atom.Atom(particle).get_parent()
            elif IMP.atom.Residue.get_is_setup(particle):
                p = IMP.atom.Residue(particle).get_parent()
            elif IMP.atom.Fragment.get_is_setup(particle):
                p = IMP.atom.Fragment(particle).get_parent()
            prerootfragment = particle
            particle = p
        return (
            (IMP.atom.Hierarchy(particle), IMP.atom.Hierarchy(prerootfragment))
        )

    def get_all_root_hierarchies_by_resolution(self, resolution):
        hierarchies = []
        resolution = float(resolution)
        particles = self.get_all_particles_by_resolution(resolution)
        for p in particles:
            rh = self.root_hierarchy_dict[p]
            if rh not in hierarchies:
                hierarchies.append(IMP.atom.Hierarchy(rh))
        return hierarchies

    def get_preroot_fragments_by_resolution(self, name, resolution):
        fragments = []
        resolution = float(resolution)
        particles = self.get_particles_by_resolution(name, resolution)
        for p in particles:
            fr = self.preroot_fragment_hierarchy_dict[p]
            if fr not in fragments:
                fragments.append(fr)
        return fragments

    def show(self, name):
        print(name)
        for resn in self.get_residue_numbers(name):
            print(resn)
            for resolution in self.get_residue_resolutions(name, resn):
                print("----", resolution)
                for p in self.get_particles(name, resn, resolution):
                    print("--------", p.get_name())


def get_prot_name_from_particle(p, list_of_names):
    '''Return the component name provided a particle and a list of names'''
    root = p
    protname = root.get_name()
    is_a_bead = False
    while not protname in list_of_names:
        root0 = root.get_parent()
        if root0 == IMP.atom.Hierarchy():
            return (None, None)
        protname = root0.get_name()

        # check if that is a bead
        # this piece of code might be dangerous if
        # the hierarchy was called Bead :)
        if "Beads" in protname:
            is_a_bead = True
        root = root0
    return (protname, is_a_bead)


def get_residue_indexes(hier):
    '''
    Retrieve the residue indexes for the given particle.

    The particle must be an instance of Fragment,Residue, Atom or Molecule
    or else returns an empty list
    '''
    resind = []
    if IMP.atom.Fragment.get_is_setup(hier):
        resind = IMP.atom.Fragment(hier).get_residue_indexes()
    elif IMP.atom.Residue.get_is_setup(hier):
        resind = [IMP.atom.Residue(hier).get_index()]
    elif IMP.atom.Atom.get_is_setup(hier):
        a = IMP.atom.Atom(hier)
        resind = [IMP.atom.Residue(a.get_parent()).get_index()]
    elif IMP.atom.Molecule.get_is_setup(hier):
        resind_tmp=IMP.pmi.tools.OrderedSet()
        for lv in IMP.atom.get_leaves(hier):
            if IMP.atom.Fragment.get_is_setup(lv) or \
               IMP.atom.Residue.get_is_setup(lv) or \
               IMP.atom.Atom.get_is_setup(lv):
                for ind in get_residue_indexes(lv): resind_tmp.add(ind)
        resind=list(resind_tmp)
    else:
        resind = []
    return resind


def sort_by_residues(particles):
    particles_residues = [(p, IMP.pmi.tools.get_residue_indexes(p))
                          for p in particles]
    sorted_particles_residues = sorted(
        particles_residues,
        key=lambda tup: tup[1])
    particles = [p[0] for p in sorted_particles_residues]
    return particles


def get_residue_to_particle_map(particles):
    # this function returns a dictionary that map particles to residue indexes
    particles = sort_by_residues(particles)
    particles_residues = [(p, IMP.pmi.tools.get_residue_indexes(p))
                          for p in particles]
    return dict(zip(particles_residues, particles))

#
# Parallel Computation
#


def scatter_and_gather(data):
    """Synchronize data over a parallel run"""
    from mpi4py import MPI
    comm = MPI.COMM_WORLD
    rank = comm.Get_rank()
    number_of_processes = comm.size
    comm.Barrier()
    if rank != 0:
        comm.send(data, dest=0, tag=11)

    elif rank == 0:
        for i in range(1, number_of_processes):
            data_tmp = comm.recv(source=i, tag=11)
            if type(data) == list:
                data += data_tmp
            elif type(data) == dict:
                data.update(data_tmp)
            else:
                raise TypeError("data not supported, use list or dictionaries")

        for i in range(1, number_of_processes):
            comm.send(data, dest=i, tag=11)

    if rank != 0:
        data = comm.recv(source=0, tag=11)
    return data

def scatter_and_gather_dict_append(data):
    """Synchronize data over a parallel run"""
    from mpi4py import MPI
    comm = MPI.COMM_WORLD
    rank = comm.Get_rank()
    number_of_processes = comm.size
    comm.Barrier()
    if rank != 0:
        comm.send(data, dest=0, tag=11)
    elif rank == 0:
        for i in range(1, number_of_processes):
            data_tmp = comm.recv(source=i, tag=11)
            for k in data:
                data[k]+=data_tmp[k]

        for i in range(1, number_of_processes):
            comm.send(data, dest=i, tag=11)

    if rank != 0:
        data = comm.recv(source=0, tag=11)
    return data


#
### Lists and iterators
#

def sublist_iterator(l, lmin=None, lmax=None):
    '''
    Yield all sublists of length >= lmin and <= lmax
    '''
    if lmin is None:
        lmin = 0
    if lmax is None:
        lmax = len(l)
    n = len(l) + 1
    for i in range(n):
        for j in range(i + 1, n):
            if len(l[i:j]) <= lmax and len(l[i:j]) >= lmin:
                yield l[i:j]


def flatten_list(l):
    return [item for sublist in l for item in sublist]


def list_chunks_iterator(list, length):
    """ Yield successive length-sized chunks from a list.
    """
    for i in range(0, len(list), length):
        yield list[i:i + length]


def chunk_list_into_segments(seq, num):
    seq = list(seq)
    avg = len(seq) / float(num)
    out = []
    last = 0.0

    while last < len(seq):
        out.append(seq[int(last):int(last + avg)])
        last += avg

    return out


class Segments(object):

    ''' This class stores integers
    in ordered compact lists eg:
    [[1,2,3],[6,7,8]]
    the methods help splitting and merging the internal lists
    Example:
    s=Segments([1,2,3]) is [[1,2,3]]
    s.add(4) is [[1,2,3,4]] (add right)
    s.add(3) is [[1,2,3,4]] (item already existing)
    s.add(7) is [[1,2,3,4],[7]] (new list)
    s.add([8,9]) is [[1,2,3,4],[7,8,9]]  (add item right)
    s.add([5,6]) is [[1,2,3,4,5,6,7,8,9]]  (merge)
    s.remove(3) is [[1,2],[4,5,6,7,8,9]]  (split)
    etc.
    '''

    def __init__(self,index):
        '''index can be a integer or a list of integers '''
        if type(index) is int:
            self.segs=[[index]]
        elif type(index) is list:
            self.segs=[[index[0]]]
            for i in index[1:]:
                self.add(i)

    def add(self,index):
        '''index can be a integer or a list of integers '''
        if type(index) is int:
            mergeleft=None
            mergeright=None
            for n,s in enumerate(self.segs):
                if index in s:
                    return 0
                else:
                    if s[0]-index==1:
                        mergeleft=n
                    if index-s[-1]==1:
                        mergeright=n
            if mergeright is None and mergeleft is None:
                self.segs.append([index])
            if not mergeright is None and mergeleft is None:
                self.segs[mergeright].append(index)
            if not mergeleft is None and  mergeright is None:
                self.segs[mergeleft]=[index]+self.segs[mergeleft]
            if not mergeleft is None and not mergeright is None:
                self.segs[mergeright]=self.segs[mergeright]+[index]+self.segs[mergeleft]
                del self.segs[mergeleft]

            for n in range(len(self.segs)):
                self.segs[n].sort()

            self.segs.sort(key=lambda tup: tup[0])

        elif type(index) is list:
            for i in index:
                self.add(i)

    def remove(self,index):
        '''index can be a integer'''
        for n,s in enumerate(self.segs):
            if index in s:
                if s[0]==index:
                    self.segs[n]=s[1:]
                elif s[-1]==index:
                    self.segs[n]=s[:-1]
                else:
                    i=self.segs[n].index(index)
                    self.segs[n]=s[:i]
                    self.segs.append(s[i+1:])
        for n in range(len(self.segs)):
            self.segs[n].sort()
            if len(self.segs[n])==0:
                del self.segs[n]
        self.segs.sort(key=lambda tup: tup[0])

    def get_flatten(self):
        ''' Returns a flatten list '''
        return [item for sublist in self.segs for item in sublist]

    def __repr__(self):
        ret_tmp="["
        for seg in self.segs:
            ret_tmp+=str(seg[0])+"-"+str(seg[-1])+","
        ret=ret_tmp[:-1]+"]"
        return ret



#
# COORDINATE MANIPULATION
#


def translate_hierarchy(hierarchy, translation_vector):
    '''
    Apply a translation to a hierarchy along the input vector.
    '''
    rbs = set()
    xyzs = set()
    if type(translation_vector) == list:
        transformation = IMP.algebra.Transformation3D(
            IMP.algebra.Vector3D(translation_vector))
    else:
        transformation = IMP.algebra.Transformation3D(translation_vector)
    for p in IMP.atom.get_leaves(hierarchy):
        if IMP.core.RigidBody.get_is_setup(p):
            rbs.add(IMP.core.RigidBody(p))
        elif IMP.core.RigidMember.get_is_setup(p):
            rb = IMP.core.RigidMember(p).get_rigid_body()
            rbs.add(rb)
        else:
            xyzs.add(p)
    for xyz in xyzs:
        IMP.core.transform(IMP.core.XYZ(xyz), transformation)
    for rb in rbs:
        IMP.core.transform(rb, transformation)


def translate_hierarchies(hierarchies, translation_vector):
    for h in hierarchies:
        IMP.pmi.tools.translate_hierarchy(h, translation_vector)


def translate_hierarchies_to_reference_frame(hierarchies):
    xc = 0
    yc = 0
    zc = 0
    nc = 0
    for h in hierarchies:
        for p in IMP.atom.get_leaves(h):
            coor = IMP.core.XYZ(p).get_coordinates()
            nc += 1
            xc += coor[0]
            yc += coor[1]
            zc += coor[2]
    xc = xc / nc
    yc = yc / nc
    zc = zc / nc
    IMP.pmi.tools.translate_hierarchies(hierarchies, (-xc, -yc, -zc))


#
# Tools to simulate data
#

def normal_density_function(expected_value, sigma, x):
    return (
        1 / math.sqrt(2 * math.pi) / sigma *
        math.exp(-(x - expected_value) ** 2 / 2 / sigma / sigma)
    )


def log_normal_density_function(expected_value, sigma, x):
    return (
        1 / math.sqrt(2 * math.pi) / sigma / x *
        math.exp(-(math.log(x / expected_value) ** 2 / 2 / sigma / sigma))
    )


def get_random_residue_pairs(representation, resolution,
                             number,
                             max_distance=None,
                             avoid_same_particles=False,
                             names=None):

    particles = []
    if names is None:
        names=list(representation.hier_dict.keys())

    for name in names:
        prot = representation.hier_dict[name]
        particles += select(representation,name=name,resolution=resolution)
    random_residue_pairs = []
    while len(random_residue_pairs)<=number:
        p1 = random.choice(particles)
        p2 = random.choice(particles)
        if max_distance is not None and \
           core.get_distance(core.XYZ(p1), core.XYZ(p2)) > max_distance:
            continue
        r1 = random.choice(IMP.pmi.tools.get_residue_indexes(p1))
        r2 = random.choice(IMP.pmi.tools.get_residue_indexes(p2))
        if r1==r2 and avoid_same_particles: continue
        name1 = representation.get_prot_name_from_particle(p1)
        name2 = representation.get_prot_name_from_particle(p2)
        random_residue_pairs.append((name1, r1, name2, r2))

    return random_residue_pairs


def get_random_data_point(
    expected_value,
    ntrials,
    sensitivity,
    sigma,
    outlierprob,
    begin_end_nbins_tuple,
    log=False,
        loggrid=False):
    import random

    begin = begin_end_nbins_tuple[0]
    end = begin_end_nbins_tuple[1]
    nbins = begin_end_nbins_tuple[2]

    if not loggrid:
        fmod_grid = get_grid(begin, end, nbins, True)
    else:
        fmod_grid = get_log_grid(begin, end, nbins)

    norm = 0
    cumul = []
    cumul.append(0)

    a = []
    for i in range(0, ntrials):
        a.append([random.random(), True])

    if sigma != 0.0:
        for j in range(1, len(fmod_grid)):
            fj = fmod_grid[j]
            fjm1 = fmod_grid[j - 1]
            df = fj - fjm1

            if not log:
                pj = normal_density_function(expected_value, sigma, fj)
                pjm1 = normal_density_function(expected_value, sigma, fjm1)
            else:
                pj = log_normal_density_function(expected_value, sigma, fj)
                pjm1 = log_normal_density_function(expected_value, sigma, fjm1)

            norm += (pj + pjm1) / 2.0 * df
            cumul.append(norm)
            # print fj, pj

        random_points = []

        for i in range(len(cumul)):
            # print i,a, cumul[i], norm
            for aa in a:
                if (aa[0] <= cumul[i] / norm and aa[1]):
                    random_points.append(
                        int(fmod_grid[i] / sensitivity) * sensitivity)
                    aa[1] = False

    else:
        random_points = [expected_value] * ntrials

    for i in range(len(random_points)):
        if random.random() < outlierprob:
            a = random.uniform(begin, end)
            random_points[i] = int(a / sensitivity) * sensitivity
    print(random_points)
    '''
    for i in range(ntrials):
      if random.random() > OUTLIERPROB_:
        r=truncnorm.rvs(0.0,1.0,expected_value,BETA_)
        if r>1.0: print r,expected_value,BETA_
      else:
        r=random.random()
      random_points.append(int(r/sensitivity)*sensitivity)
    '''

    rmean = 0.
    rmean2 = 0.
    for r in random_points:
        rmean += r
        rmean2 += r * r

    rmean /= float(ntrials)
    rmean2 /= float(ntrials)
    stddev = math.sqrt(max(rmean2 - rmean * rmean, 0.))
    return rmean, stddev

def print_multicolumn(list_of_strings, ncolumns=2, truncate=40):

    l = list_of_strings

    cols = ncolumns
    # add empty entries after l
    for i in range(len(l) % cols):
        l.append(" ")

    split = [l[i:i + len(l) / cols] for i in range(0, len(l), len(l) / cols)]
    for row in zip(*split):
        print("".join(str.ljust(i, truncate) for i in row))

class ColorChange(object):
    '''Change color code to hexadecimal to rgb'''
    def __init__(self):
        self._NUMERALS = '0123456789abcdefABCDEF'
        self._HEXDEC = dict((v, int(v, 16)) for v in (x+y for x in self._NUMERALS for y in self._NUMERALS))
        self.LOWERCASE, self.UPPERCASE = 'x', 'X'

    def rgb(self,triplet):
        return float(self._HEXDEC[triplet[0:2]]), float(self._HEXDEC[triplet[2:4]]), float(self._HEXDEC[triplet[4:6]])

    def triplet(self,rgb, lettercase=None):
        if lettercase is None: lettercase=self.LOWERCASE
        return format(rgb[0]<<16 | rgb[1]<<8 | rgb[2], '06'+lettercase)

# -------------- Collections --------------- #

class OrderedSet(collections.MutableSet):

    def __init__(self, iterable=None):
        self.end = end = []
        end += [None, end, end]         # sentinel node for doubly linked list
        self.map = {}                   # key --> [key, prev, next]
        if iterable is not None:
            self |= iterable

    def __len__(self):
        return len(self.map)

    def __contains__(self, key):
        return key in self.map

    def add(self, key):
        if key not in self.map:
            end = self.end
            curr = end[1]
            curr[2] = end[1] = self.map[key] = [key, curr, end]

    def discard(self, key):
        if key in self.map:
            key, prev, next = self.map.pop(key)
            prev[2] = next
            next[1] = prev

    def __iter__(self):
        end = self.end
        curr = end[2]
        while curr is not end:
            yield curr[0]
            curr = curr[2]

    def __reversed__(self):
        end = self.end
        curr = end[1]
        while curr is not end:
            yield curr[0]
            curr = curr[1]

    def pop(self, last=True):
        if not self:
            raise KeyError('set is empty')
        if last:
            key = self.end[1][0]
        else:
            key = self.end[2][0]
        self.discard(key)
        return key

    def __repr__(self):
        if not self:
            return '%s()' % (self.__class__.__name__,)
        return '%s(%r)' % (self.__class__.__name__, list(self))

    def __eq__(self, other):
        if isinstance(other, OrderedSet):
            return len(self) == len(other) and list(self) == list(other)
        return set(self) == set(other)


class OrderedDefaultDict(OrderedDict):
    """Store objects in order they were added, but with default type.
    Source: http://stackoverflow.com/a/4127426/2608793
    """
    def __init__(self, *args, **kwargs):
        if not args:
            self.default_factory = None
        else:
            if not (args[0] is None or callable(args[0])):
                raise TypeError('first argument must be callable or None')
            self.default_factory = args[0]
            args = args[1:]
        super(OrderedDefaultDict, self).__init__(*args, **kwargs)

    def __missing__ (self, key):
        if self.default_factory is None:
            raise KeyError(key)
        self[key] = default = self.default_factory()
        return default

    def __reduce__(self):  # optional, for pickle support
        args = (self.default_factory,) if self.default_factory else ()
        return self.__class__, args, None, None, self.iteritems()

# -------------- PMI2 Tools --------------- #

def set_coordinates_from_rmf(hier,rmf_fn,frame_num=0):
    """Extract frame from RMF file and fill coordinates. Must be identical topology.
    @param hier The (System) hierarchy to fill (e.g. after you've built it)
    @param rmf_fn The file to extract from
    @param frame_num The frame number to extract
    """
    rh = RMF.open_rmf_file_read_only(rmf_fn)
    IMP.rmf.link_hierarchies(rh,[hier])
    IMP.rmf.load_frame(rh, RMF.FrameID(frame_num))
    del rh

def input_adaptor(stuff,
                  pmi_resolution=0,
                  flatten=False,
                  selection_tuple=None,
                  warn_about_slices=True):
    """Adapt things for PMI (degrees of freedom, restraints, ...)
    Returns list of list of hierarchies, separated into Molecules if possible.
    The input can be a list, or a list of lists (iterable of ^1 or iterable of ^2)
    (iterable of ^2) Hierarchy -> returns input as list of list of hierarchies,
        only one entry, not grouped by molecules.
    (iterable of ^2) PMI::System/State/Molecule/TempResidue ->
        returns residue hierarchies, grouped in molecules, at requested resolution
    @param stuff Can be one of the following inputs:
           IMP Hierarchy, PMI System/State/Molecule/TempResidue, or a list/set (of list/set) of them.
           Must be uniform input, however. No mixing object types.
    @param pmi_resolution For selecting, only does it if you pass PMI objects. Set it to "all"
          if you want all resolutions!
    @param flatten Set to True if you just want all hierarchies in one list.
    @param warn_about_slices Print a warning if you are requesting only part of a bead.
           Sometimes you just don't care!
    \note since this relies on IMP::atom::Selection, this will not return any objects if they weren't built!
    But there should be no problem if you request unbuilt residues, they should be ignored.
    """

    if stuff is None:
        return stuff

    if hasattr(stuff,'__iter__'):
        if len(stuff)==0:
            return stuff
        thelist=list(stuff)

        # iter of iter of should be ok
        if all(hasattr(el,'__iter__') for el in thelist):
            thelist = [i for sublist in thelist for i in sublist]
        elif any(hasattr(el,'__iter__') for el in thelist):
            raise Exception('input_adaptor: input_object must be a list or a list of lists')

        stuff = thelist
    else:
        stuff = [stuff]

    # check that it is a hierarchy homogenously:
    try:
        is_hierarchy=all(IMP.atom.Hierarchy.get_is_setup(s) for s in stuff)
    except NotImplementedError:
        is_hierarchy=False
    # get the other types homogenously
    is_system=all(isinstance(s, IMP.pmi.topology.System) for s in stuff)
    is_state=all(isinstance(s, IMP.pmi.topology.State) for s in stuff)
    is_molecule=all(isinstance(s, IMP.pmi.topology.Molecule) for s in stuff)
    is_temp_residue=all(isinstance(s, IMP.pmi.topology.TempResidue) for s in stuff)

    # now that things are ok, do selection if requested
    hier_list = []
    pmi_input = False
    if is_system or is_state or is_molecule or is_temp_residue:
        # if PMI, perform selection using gathered indexes
        pmi_input = True
        indexes_per_mol = OrderedDefaultDict(list) #key is Molecule object, value are residues
        if is_system:
            for system in stuff:
                for state in system.get_states():
                    mdict = state.get_molecules()
                    for molname in mdict:
                        for copy in mdict[molname]:
                            indexes_per_mol[copy] += [r.get_index() for r in copy.get_residues()]
        elif is_state:
            for state in stuff:
                mdict = state.get_molecules()
                for molname in mdict:
                    for copy in mdict[molname]:
                        indexes_per_mol[copy] += [r.get_index() for r in copy.get_residues()]
        elif is_molecule:
            for molecule in stuff:
                indexes_per_mol[molecule] += [r.get_index() for r in molecule.get_residues()]
        elif is_temp_residue:
            for tempres in stuff:
                indexes_per_mol[tempres.get_molecule()].append(tempres.get_index())
        for mol in indexes_per_mol:
            if pmi_resolution=='all':
                # because you select from the molecule,
                #  this will start the search from the base resolution
                ps = select_at_all_resolutions(mol.get_hierarchy(),
                                               residue_indexes=indexes_per_mol[mol])
            else:
                sel = IMP.atom.Selection(mol.get_hierarchy(),
                                         resolution=pmi_resolution,
                                         residue_indexes=indexes_per_mol[mol])
                ps = sel.get_selected_particles()

            # check that you don't have any incomplete fragments!
            if warn_about_slices:
                rset = set(indexes_per_mol[mol])
                for p in ps:
                    if IMP.atom.Fragment.get_is_setup(p):
                        fset = set(IMP.atom.Fragment(p).get_residue_indexes())
                        if not fset <= rset:
                            print('BAD')
                            minset = min(fset)
                            maxset = max(fset)
                            found = fset&rset
                            minf = min(found)
                            maxf = max(found)
                            resbreak = maxf if minf==minset else minset-1
                            print('WARNING: You are trying to select only part of the bead %s:%i-%i.\n'
                                  'The residues you requested are %i-%i. You can fix this by:\n'
                                  '1) requesting the whole bead/none of it or\n'
                                  '2) break the bead up by passing bead_extra_breaks=[\'%i\'] in '
                                  'molecule.add_representation()'
                                            %(mol.get_name(),minset,maxset,minf,maxf,resbreak))
            hier_list.append([IMP.atom.Hierarchy(p) for p in ps])
    elif is_hierarchy:
        #check
        ps=[]
        if pmi_resolution=='all':
            for h in stuff:
                ps+=select_at_all_resolutions(h)
        else:
            for h in stuff:
                ps+=IMP.atom.Selection(h,resolution=pmi_resolution).get_selected_particles()
        hier_list=[IMP.atom.Hierarchy(p) for p in ps]
        if not flatten:
            hier_list = [hier_list]
    else:
        raise Exception('input_adaptor: you passed something of wrong type or a list with mixed types')

    if flatten and pmi_input:
        return [h for sublist in hier_list for h in sublist]
    else:
        return hier_list


def get_sorted_segments(mol):
    """Returns sequence-sorted segments array, each containing the first particle
    the last particle and the first residue index."""

    from operator import itemgetter
    hiers=IMP.pmi.tools.input_adaptor(mol)
    if len(hiers)>1:
        raise Exception("IMP.pmi.tools.get_sorted_segments: only pass stuff from one Molecule, please")
    hiers = hiers[0]
    SortedSegments = []
    for h in hiers:
        try:
            start = IMP.atom.Hierarchy(h).get_children()[0]
        except:
            start = IMP.atom.Hierarchy(h)

        try:
            end = IMP.atom.Hierarchy(h).get_children()[-1]
        except:
            end = IMP.atom.Hierarchy(h)

        startres = IMP.pmi.tools.get_residue_indexes(start)[0]
        endres = IMP.pmi.tools.get_residue_indexes(end)[-1]
        SortedSegments.append((start, end, startres))
    SortedSegments = sorted(SortedSegments, key=itemgetter(2))
    return SortedSegments

def display_bonds(mol):
    """Decorate the sequence-consecutive particles from a PMI2 molecule with a bond,
    so that they appear connected in the rmf file"""
    SortedSegments=get_sorted_segments(mol)
    for x in range(len(SortedSegments) - 1):

        last = SortedSegments[x][1]
        first = SortedSegments[x + 1][0]

        p1 = last.get_particle()
        p2 = first.get_particle()
        if not IMP.atom.Bonded.get_is_setup(p1):
            IMP.atom.Bonded.setup_particle(p1)
        if not IMP.atom.Bonded.get_is_setup(p2):
            IMP.atom.Bonded.setup_particle(p2)

        if not IMP.atom.get_bond(IMP.atom.Bonded(p1),IMP.atom.Bonded(p2)):
            IMP.atom.create_bond(
                IMP.atom.Bonded(p1),
                IMP.atom.Bonded(p2),1)


class ThreeToOneConverter(defaultdict):
    """This class converts three to one letter codes, and return X for any unknown codes"""
    def __init__(self,is_nucleic=False):

        if not is_nucleic:
            threetoone = {'ALA': 'A', 'ARG': 'R', 'ASN': 'N', 'ASP': 'D',
                              'CYS': 'C', 'GLU': 'E', 'GLN': 'Q', 'GLY': 'G',
                              'HIS': 'H', 'ILE': 'I', 'LEU': 'L', 'LYS': 'K',
                              'MET': 'M', 'PHE': 'F', 'PRO': 'P', 'SER': 'S',
                              'THR': 'T', 'TRP': 'W', 'TYR': 'Y', 'VAL': 'V', 'UNK': 'X'}
        else:
            threetoone = {'ADE': 'A', 'URA': 'U', 'CYT': 'C', 'GUA': 'G',
                              'THY': 'T', 'UNK': 'X'}

        defaultdict.__init__(self,lambda: "X", threetoone)




def get_residue_type_from_one_letter_code(code,is_nucleic=False):
    threetoone=ThreeToOneConverter(is_nucleic)
    one_to_three={}
    for k in threetoone:
        one_to_three[threetoone[k]] = k
    return IMP.atom.ResidueType(one_to_three[code])


def get_all_leaves(list_of_hs):
    """ Just get the leaves from a list of hierarchies """
    lvs = list(itertools.chain.from_iterable(IMP.atom.get_leaves(item) for item in list_of_hs))
    return lvs


def select_at_all_resolutions(hier=None,
                              hiers=None,
                              **kwargs):
    """Perform selection using the usual keywords but return ALL resolutions (BEADS and GAUSSIANS).
    Returns in flat list!
    """

    if hiers is None:
        hiers = []
    if hier is not None:
        hiers.append(hier)
    if len(hiers)==0:
        print("WARNING: You passed nothing to select_at_all_resolutions()")
        return []
    ret = OrderedSet()
    for hsel in hiers:
        try:
            htest = IMP.atom.Hierarchy.get_is_setup(hsel)
        except:
            raise Exception('select_at_all_resolutions: you have to pass an IMP Hierarchy')
        if not htest:
            raise Exception('select_at_all_resolutions: you have to pass an IMP Hierarchy')
        if 'resolution' in kwargs or 'representation_type' in kwargs:
            raise Exception("don't pass resolution or representation_type to this function")
        selB = IMP.atom.Selection(hsel,resolution=IMP.atom.ALL_RESOLUTIONS,
                                  representation_type=IMP.atom.BALLS,**kwargs)
        selD = IMP.atom.Selection(hsel,resolution=IMP.atom.ALL_RESOLUTIONS,
                                  representation_type=IMP.atom.DENSITIES,**kwargs)
        ret |= OrderedSet(selB.get_selected_particles())
        ret |= OrderedSet(selD.get_selected_particles())
    return list(ret)


def get_particles_within_zone(hier,
                              target_ps,
                              sel_zone,
                              entire_residues,
                              exclude_backbone):
    """Utility to retrieve particles from a hierarchy within a
    zone around a set of ps.
    @param hier The hierarchy in which to look for neighbors
    @param target_ps The particles for zoning
    @param sel_zone The maximum distance
    @param entire_residues If True, will grab entire residues
    @param exclude_backbone If True, will only return sidechain particles
    """

    test_sel = IMP.atom.Selection(hier)
    backbone_types=['C','N','CB','O']
    if exclude_backbone:
        test_sel -= IMP.atom.Selection(hier,atom_types=[IMP.atom.AtomType(n)
                                                        for n in backbone_types])
    test_ps = test_sel.get_selected_particles()
    nn = IMP.algebra.NearestNeighbor3D([IMP.core.XYZ(p).get_coordinates()
                                         for p in test_ps])
    zone = set()
    for target in target_ps:
        zone|=set(nn.get_in_ball(IMP.core.XYZ(target).get_coordinates(),sel_zone))
    zone_ps = [test_ps[z] for z in zone]
    if entire_residues:
        final_ps = set()
        for z in zone_ps:
            final_ps|=set(IMP.atom.Hierarchy(z).get_parent().get_children())
        zone_ps = [h.get_particle() for h in final_ps]
    return zone_ps


def get_rbs_and_beads(hiers):
    """Returns unique objects in original order"""
    rbs = set()
    beads = []
    rbs_ordered = []
    if not hasattr(hiers,'__iter__'):
        hiers = [hiers]
    for p in get_all_leaves(hiers):
        if IMP.core.RigidMember.get_is_setup(p):
            rb = IMP.core.RigidMember(p).get_rigid_body()
            if rb not in rbs:
                rbs.add(rb)
                rbs_ordered.append(rb)
        elif IMP.core.NonRigidMember.get_is_setup(p):
            rb = IMP.core.NonRigidMember(p).get_rigid_body()
            if rb not in rbs:
                rbs.add(rb)
                rbs_ordered.append(rb)
            beads.append(p)
        else:
            beads.append(p)
    return rbs_ordered,beads

def get_molecules(input_objects):
    "This function returns the parent molecule hierarchies of given objects"
    stuff=input_adaptor(input_objects, pmi_resolution='all',flatten=True)
    molecules=set()
    for h in stuff:
        is_root=False
        is_molecule=False
        while not (is_molecule or is_root):
            root=IMP.atom.get_root(h)
            if root == h:
                is_root=True
            is_molecule=IMP.atom.Molecule.get_is_setup(h)
            if is_molecule:
                molecules.add(IMP.atom.Molecule(h))
            h=h.get_parent()
    return list(molecules)

def get_molecules_dictionary(input_objects):
    moldict=defaultdict(list)
    for mol in IMP.pmi.tools.get_molecules(input_objects):
        name=mol.get_name()
        moldict[name].append(mol)
    return moldict

def get_molecules_dictionary_by_copy(input_objects):
    moldict=defaultdict(dict)
    for mol in IMP.pmi.tools.get_molecules(input_objects):
        name=mol.get_name()
        c=IMP.atom.Copy(mol).get_copy_index()
        moldict[name][c]=mol
    return moldict

def get_selections_dictionary(input_objects):
    moldict=IMP.pmi.tools.get_molecules_dictionary(input_objects)
    seldict=defaultdict(list)
    for name, mols in moldict.items():
        for m in mols:
            seldict[name].append(IMP.atom.Selection(m))
    return seldict

def get_densities(input_objects):
    """Given a list of PMI objects, returns all density hierarchies within
    these objects.  The output of this function can be inputted into
    things such as EM restraints. This function is intended to gather density particles
    appended to molecules (and not other hierarchies which might have been appended to the root node directly).
    """
    # Note that Densities can only be selected at the Root or Molecule level and not at the Leaves level.
    # we'll first get all molecule hierarchies corresponding to the leaves.
    molecules=get_molecules(input_objects)
    densities=[]
    for i in molecules:
        densities+=IMP.atom.Selection(i,representation_type=IMP.atom.DENSITIES).get_selected_particles()
    return densities

def shuffle_configuration(objects,
                          max_translation=300., max_rotation=2.0 * pi,
                          avoidcollision_rb=True, avoidcollision_fb=False,
                          cutoff=10.0, niterations=100,
                          bounding_box=None,
                          excluded_rigid_bodies=[],
                          hierarchies_excluded_from_collision=[],
                          hierarchies_included_in_collision=[],
                          verbose=False,
                          return_debug=False):
    """Shuffle particles. Used to restart the optimization.
    The configuration of the system is initialized by placing each
    rigid body and each bead randomly in a box with a side of
    max_translation angstroms, and far enough from each other to
    prevent any steric clashes. The rigid bodies are also randomly rotated.
    @param objects Can be one of the following inputs:
               IMP Hierarchy, PMI System/State/Molecule/TempResidue, or a list/set of them
    @param max_translation Max translation (rbs and flexible beads)
    @param max_rotation Max rotation (rbs only)
    @param avoidcollision_rb check if the particle/rigid body was
           placed close to another particle; uses the optional
           arguments cutoff and niterations
    @param avoidcollision_fb Advanced. Generally you want this False because it's hard to shuffle beads.
    @param cutoff Distance less than this is a collision
    @param niterations How many times to try avoiding collision
    @param bounding_box Only shuffle particles within this box. Defined by ((x1,y1,z1),(x2,y2,z2)).
    @param excluded_rigid_bodies Don't shuffle these rigid body objects
    @param hierarchies_excluded_from_collision Don't count collision with these bodies
    @param hierarchies_included_in_collision Hierarchies that are not shuffled, but should be included in collision calculation (for fixed regions)
    @param verbose Give more output
    \note Best to only call this function after you've set up degrees of freedom
    For debugging purposes, returns: <shuffled indexes>, <collision avoided indexes>
    """

    ### checking input
    hierarchies = IMP.pmi.tools.input_adaptor(objects,
                                              pmi_resolution='all',
                                              flatten=True)
    rigid_bodies,flexible_beads = get_rbs_and_beads(hierarchies)
    if len(rigid_bodies)>0:
        mdl = rigid_bodies[0].get_model()
    elif len(flexible_beads)>0:
        mdl = flexible_beads[0].get_model()
    else:
        raise Exception("Could not find any particles in the hierarchy")
    if len(rigid_bodies) == 0:
        print("shuffle_configuration: rigid bodies were not intialized")

    ### gather all particles
    gcpf = IMP.core.GridClosePairsFinder()
    gcpf.set_distance(cutoff)

    # Add particles from excluded hierarchies to excluded list
    collision_excluded_hierarchies = IMP.pmi.tools.input_adaptor(hierarchies_excluded_from_collision,
                                              pmi_resolution='all',
                                              flatten=True)

    collision_included_hierarchies = IMP.pmi.tools.input_adaptor(hierarchies_included_in_collision,
                                              pmi_resolution='all',
                                              flatten=True)

    collision_excluded_idxs = set([l.get_particle().get_index() for h in collision_excluded_hierarchies \
                               for l in IMP.core.get_leaves(h)])

    collision_included_idxs = set([l.get_particle().get_index() for h in collision_included_hierarchies \
                               for l in IMP.core.get_leaves(h)])

    # Excluded collision with Gaussians
    all_idxs = [] #expand to representations?
    for p in IMP.pmi.tools.get_all_leaves(hierarchies):
        if IMP.core.XYZ.get_is_setup(p):
            all_idxs.append(p.get_particle_index())
        if IMP.core.Gaussian.get_is_setup(p):
            collision_excluded_idxs.add(p.get_particle_index())

    if bounding_box is not None:
        ((x1, y1, z1), (x2, y2, z2)) = bounding_box
        ub = IMP.algebra.Vector3D(x1, y1, z1)
        lb = IMP.algebra.Vector3D(x2, y2, z2)
        bb = IMP.algebra.BoundingBox3D(ub, lb)

    all_idxs = set(all_idxs) | collision_included_idxs
    all_idxs = all_idxs - collision_excluded_idxs
    debug = []
    print('shuffling', len(rigid_bodies), 'rigid bodies')
    for rb in rigid_bodies:
        if rb not in excluded_rigid_bodies:
            # gather particles to avoid with this transform
            if avoidcollision_rb:
                rb_idxs = set(rb.get_member_particle_indexes()) - \
                          collision_excluded_idxs
                other_idxs = all_idxs - rb_idxs
                if not other_idxs:
                    continue

            # iterate, trying to avoid collisions
            niter = 0
            while niter < niterations:
                rbxyz = (rb.get_x(), rb.get_y(), rb.get_z())

                # local transform
                if bounding_box:
                    translation = IMP.algebra.get_random_vector_in(bb)
                    # First move to origin
                    transformation_orig = IMP.algebra.Transformation3D(IMP.algebra.get_identity_rotation_3d(),
                                                                       -IMP.core.XYZ(rb).get_coordinates())
                    IMP.core.transform(rb, transformation_orig)
                    old_coord = IMP.core.XYZ(rb).get_coordinates()
                    rotation = IMP.algebra.get_random_rotation_3d()
                    transformation = IMP.algebra.Transformation3D(rotation,
                                                                  translation)

                else:
                    transformation = IMP.algebra.get_random_local_transformation(
                        rbxyz,
                        max_translation,
                        max_rotation)

                debug.append([rb, other_idxs if avoidcollision_rb else set()])
                IMP.core.transform(rb, transformation)

                # check collisions
                if avoidcollision_rb:
                    mdl.update()
                    npairs = len(gcpf.get_close_pairs(mdl,
                                                      list(other_idxs),
                                                      list(rb_idxs)))
                    #print("NPAIRS:", npairs)
                    if npairs==0:
                        break
                    else:
                        niter += 1
                        if verbose:
                            print("shuffle_configuration: rigid body placed close to other %d particles, trying again..." % npairs)
                            print("shuffle_configuration: rigid body name: " + rb.get_name())
                        if niter == niterations:
                            raise ValueError("tried the maximum number of iterations to avoid collisions, increase the distance cutoff")
                else:
                    break

    print('shuffling', len(flexible_beads), 'flexible beads')
    for fb in flexible_beads:
        # gather particles to avoid
        if avoidcollision_fb:
            fb_idxs = set(IMP.get_indexes([fb]))
            other_idxs = all_idxs - fb_idxs
            if not other_idxs:
                continue

        # iterate, trying to avoid collisions
        niter = 0
        while niter < niterations:
            if bounding_box:
                translation = IMP.algebra.get_random_vector_in(bb)
                transformation = IMP.algebra.Transformation3D(translation)
            else:
                fbxyz = IMP.core.XYZ(fb).get_coordinates()
                transformation = IMP.algebra.get_random_local_transformation(fbxyz,
                                                                             max_translation,
                                                                             max_rotation)

            # For gaussians, treat this fb as an rb
            if IMP.core.NonRigidMember.get_is_setup(fb):
                xyz=[fb.get_value(IMP.FloatKey(4)), fb.get_value(IMP.FloatKey(5)), fb.get_value(IMP.FloatKey(6))]
                xyz_transformed=transformation.get_transformed(xyz)
                if bounding_box:
                    # Translate to origin
                    fb.set_value(IMP.FloatKey(4),xyz_transformed[0]-xyz[0])
                    fb.set_value(IMP.FloatKey(5),xyz_transformed[1]-xyz[1])
                    fb.set_value(IMP.FloatKey(6),xyz_transformed[2]-xyz[2])
                    debug.append([xyz,other_idxs if avoidcollision_fb else set()])
                    xyz2=[fb.get_value(IMP.FloatKey(4)), fb.get_value(IMP.FloatKey(5)), fb.get_value(IMP.FloatKey(6))]
                else:
                    fb.set_value(IMP.FloatKey(4),xyz_transformed[0])
                    fb.set_value(IMP.FloatKey(5),xyz_transformed[1])
                    fb.set_value(IMP.FloatKey(6),xyz_transformed[2])
                    debug.append([xyz,other_idxs if avoidcollision_fb else set()])
            else:
                d =IMP.core.XYZ(fb)
                if bounding_box:
                    # Translate to origin first
                    IMP.core.transform(d, -d.get_coordinates())
                    d =IMP.core.XYZ(fb)
                debug.append([d,other_idxs if avoidcollision_fb else set()])
                IMP.core.transform(d, transformation)

            if avoidcollision_fb:
                mdl.update()
                npairs = len(gcpf.get_close_pairs(mdl,
                                                  list(other_idxs),
                                                  list(fb_idxs)))

                if npairs==0:
                    break
                else:
                    niter += 1
                    print("shuffle_configuration: floppy body placed close to other %d particles, trying again..." % npairs)
                    if niter == niterations:
                        raise ValueError("tried the maximum number of iterations to avoid collisions, increase the distance cutoff")
            else:
                break
    if return_debug:
        return debug

def color2rgb(colorname):
    """Given a chimera color name, return RGB"""
    d = {'aquamarine': (0.4980392156862745, 1.0, 0.8313725490196079),
         'black': (0.0, 0.0, 0.0),
         'blue': (0.0, 0.0, 1.0),
         'brown': (0.6470588235294118, 0.16470588235294117, 0.16470588235294117),
         'chartreuse': (0.4980392156862745, 1.0, 0.0),
         'coral': (1.0, 0.4980392156862745, 0.3137254901960784),
         'cornflower blue': (0.39215686274509803, 0.5843137254901961, 0.9294117647058824),
         'cyan': (0.0, 1.0, 1.0),
         'dark cyan': (0.0, 0.5450980392156862, 0.5450980392156862),
         'dark gray': (0.6627450980392157, 0.6627450980392157, 0.6627450980392157),
         'dark green': (0.0, 0.39215686274509803, 0.0),
         'dark khaki': (0.7411764705882353, 0.7176470588235294, 0.4196078431372549),
         'dark magenta': (0.5450980392156862, 0.0, 0.5450980392156862),
         'dark olive green': (0.3333333333333333, 0.4196078431372549, 0.1843137254901961),
         'dark red': (0.5450980392156862, 0.0, 0.0),
         'dark slate blue': (0.2823529411764706, 0.23921568627450981, 0.5450980392156862),
         'dark slate gray': (0.1843137254901961, 0.30980392156862746, 0.30980392156862746),
         'deep pink': (1.0, 0.0784313725490196, 0.5764705882352941),
         'deep sky blue': (0.0, 0.7490196078431373, 1.0),
         'dim gray': (0.4117647058823529, 0.4117647058823529, 0.4117647058823529),
         'dodger blue': (0.11764705882352941, 0.5647058823529412, 1.0),
         'firebrick': (0.6980392156862745, 0.13333333333333333, 0.13333333333333333),
         'forest green': (0.13333333333333333, 0.5450980392156862, 0.13333333333333333),
         'gold': (1.0, 0.8431372549019608, 0.0),
         'goldenrod': (0.8549019607843137, 0.6470588235294118, 0.12549019607843137),
         'gray': (0.7450980392156863, 0.7450980392156863, 0.7450980392156863),
         'green': (0.0, 1.0, 0.0),
         'hot pink': (1.0, 0.4117647058823529, 0.7058823529411765),
         'khaki': (0.9411764705882353, 0.9019607843137255, 0.5490196078431373),
         'light blue': (0.6784313725490196, 0.8470588235294118, 0.9019607843137255),
         'light gray': (0.8274509803921568, 0.8274509803921568, 0.8274509803921568),
         'light green': (0.5647058823529412, 0.9333333333333333, 0.5647058823529412),
         'light sea green': (0.12549019607843137, 0.6980392156862745, 0.6666666666666666),
         'lime green': (0.19607843137254902, 0.803921568627451, 0.19607843137254902),
         'magenta': (1.0, 0.0, 1.0),
         'medium blue': (0.19607843137254902, 0.19607843137254902, 0.803921568627451),
         'medium purple': (0.5764705882352941, 0.4392156862745098, 0.8588235294117647),
         'navy blue': (0.0, 0.0, 0.5019607843137255),
         'olive drab': (0.4196078431372549, 0.5568627450980392, 0.13725490196078433),
         'orange red': (1.0, 0.27058823529411763, 0.0),
         'orange': (1.0, 0.4980392156862745, 0.0),
         'orchid': (0.8549019607843137, 0.4392156862745098, 0.8392156862745098),
         'pink': (1.0, 0.7529411764705882, 0.796078431372549),
         'plum': (0.8666666666666667, 0.6274509803921569, 0.8666666666666667),
         'purple': (0.6274509803921569, 0.12549019607843137, 0.9411764705882353),
         'red': (1.0, 0.0, 0.0),
         'rosy brown': (0.7372549019607844, 0.5607843137254902, 0.5607843137254902),
         'salmon': (0.9803921568627451, 0.5019607843137255, 0.4470588235294118),
         'sandy brown': (0.9568627450980393, 0.6431372549019608, 0.3764705882352941),
         'sea green': (0.1803921568627451, 0.5450980392156862, 0.3411764705882353),
         'sienna': (0.6274509803921569, 0.3215686274509804, 0.17647058823529413),
         'sky blue': (0.5294117647058824, 0.807843137254902, 0.9215686274509803),
         'slate gray': (0.4392156862745098, 0.5019607843137255, 0.5647058823529412),
         'spring green': (0.0, 1.0, 0.4980392156862745),
         'steel blue': (0.27450980392156865, 0.5098039215686274, 0.7058823529411765),
         'tan': (0.8235294117647058, 0.7058823529411765, 0.5490196078431373),
         'turquoise': (0.25098039215686274, 0.8784313725490196, 0.8156862745098039),
         'violet red': (0.8156862745098039, 0.12549019607843137, 0.5647058823529412),
         'white': (1.0, 1.0, 1.0),
         'yellow': (1.0, 1.0, 0.0)}
    return d[colorname]

class Colors(object):
    def __init__(self):
        self.colors={
        "reds":[("maroon","#800000",(128,0,0)),("dark red","#8B0000",(139,0,0)),
        ("brown","#A52A2A",(165,42,42)),("firebrick","#B22222",(178,34,34)),
        ("crimson","#DC143C",(220,20,60)),("red","#FF0000",(255,0,0)),
        ("tomato","#FF6347",(255,99,71)),("coral","#FF7F50",(255,127,80)),
        ("indian red","#CD5C5C",(205,92,92)),("light coral","#F08080",(240,128,128)),
        ("dark salmon","#E9967A",(233,150,122)),("salmon","#FA8072",(250,128,114)),
        ("light salmon","#FFA07A",(255,160,122)),("orange red","#FF4500",(255,69,0)),
        ("dark orange","#FF8C00",(255,140,0))],
        "yellows":[("orange","#FFA500",(255,165,0)),("gold","#FFD700",(255,215,0)),
        ("dark golden rod","#B8860B",(184,134,11)),("golden rod","#DAA520",(218,165,32)),
        ("pale golden rod","#EEE8AA",(238,232,170)),("dark khaki","#BDB76B",(189,183,107)),
        ("khaki","#F0E68C",(240,230,140)),("olive","#808000",(128,128,0)),
        ("yellow","#FFFF00",(255,255,0)),("antique white","#FAEBD7",(250,235,215)),
        ("beige","#F5F5DC",(245,245,220)),("bisque","#FFE4C4",(255,228,196)),
        ("blanched almond","#FFEBCD",(255,235,205)),("wheat","#F5DEB3",(245,222,179)),
        ("corn silk","#FFF8DC",(255,248,220)),("lemon chiffon","#FFFACD",(255,250,205)),
        ("light golden rod yellow","#FAFAD2",(250,250,210)),("light yellow","#FFFFE0",(255,255,224))],
        "greens":[("yellow green","#9ACD32",(154,205,50)),("dark olive green","#556B2F",(85,107,47)),
        ("olive drab","#6B8E23",(107,142,35)),("lawn green","#7CFC00",(124,252,0)),
        ("chart reuse","#7FFF00",(127,255,0)),("green yellow","#ADFF2F",(173,255,47)),
        ("dark green","#006400",(0,100,0)),("green","#008000",(0,128,0)),
        ("forest green","#228B22",(34,139,34)),("lime","#00FF00",(0,255,0)),
        ("lime green","#32CD32",(50,205,50)),("light green","#90EE90",(144,238,144)),
        ("pale green","#98FB98",(152,251,152)),("dark sea green","#8FBC8F",(143,188,143)),
        ("medium spring green","#00FA9A",(0,250,154)),("spring green","#00FF7F",(0,255,127)),
        ("sea green","#2E8B57",(46,139,87)),("medium aqua marine","#66CDAA",(102,205,170)),
        ("medium sea green","#3CB371",(60,179,113)),("light sea green","#20B2AA",(32,178,170)),
        ("dark slate gray","#2F4F4F",(47,79,79)),("teal","#008080",(0,128,128)),
        ("dark cyan","#008B8B",(0,139,139))],
        "blues":[("dark turquoise","#00CED1",(0,206,209)),
        ("turquoise","#40E0D0",(64,224,208)),("medium turquoise","#48D1CC",(72,209,204)),
        ("pale turquoise","#AFEEEE",(175,238,238)),("aqua marine","#7FFFD4",(127,255,212)),
        ("powder blue","#B0E0E6",(176,224,230)),("cadet blue","#5F9EA0",(95,158,160)),
        ("steel blue","#4682B4",(70,130,180)),("corn flower blue","#6495ED",(100,149,237)),
        ("deep sky blue","#00BFFF",(0,191,255)),("dodger blue","#1E90FF",(30,144,255)),
        ("light blue","#ADD8E6",(173,216,230)),("sky blue","#87CEEB",(135,206,235)),
        ("light sky blue","#87CEFA",(135,206,250)),("midnight blue","#191970",(25,25,112)),
        ("navy","#000080",(0,0,128)),("dark blue","#00008B",(0,0,139)),
        ("medium blue","#0000CD",(0,0,205)),("blue","#0000FF",(0,0,255)),("royal blue","#4169E1",(65,105,225)),
        ("aqua","#00FFFF",(0,255,255)),("cyan","#00FFFF",(0,255,255)),("light cyan","#E0FFFF",(224,255,255))],
       "violets":[("blue violet","#8A2BE2",(138,43,226)),("indigo","#4B0082",(75,0,130)),
        ("dark slate blue","#483D8B",(72,61,139)),("slate blue","#6A5ACD",(106,90,205)),
        ("medium slate blue","#7B68EE",(123,104,238)),("medium purple","#9370DB",(147,112,219)),
        ("dark magenta","#8B008B",(139,0,139)),("dark violet","#9400D3",(148,0,211)),
        ("dark orchid","#9932CC",(153,50,204)),("medium orchid","#BA55D3",(186,85,211)),
        ("purple","#800080",(128,0,128)),("thistle","#D8BFD8",(216,191,216)),
        ("plum","#DDA0DD",(221,160,221)),("violet","#EE82EE",(238,130,238)),
        ("magenta / fuchsia","#FF00FF",(255,0,255)),("orchid","#DA70D6",(218,112,214)),
        ("medium violet red","#C71585",(199,21,133)),("pale violet red","#DB7093",(219,112,147)),
        ("deep pink","#FF1493",(255,20,147)),("hot pink","#FF69B4",(255,105,180)),
        ("light pink","#FFB6C1",(255,182,193)),("pink","#FFC0CB",(255,192,203))],
       "browns":[("saddle brown","#8B4513",(139,69,19)),("sienna","#A0522D",(160,82,45)),
        ("chocolate","#D2691E",(210,105,30)),("peru","#CD853F",(205,133,63)),
        ("sandy brown","#F4A460",(244,164,96)),("burly wood","#DEB887",(222,184,135)),
        ("tan","#D2B48C",(210,180,140)),("rosy brown","#BC8F8F",(188,143,143)),
        ("moccasin","#FFE4B5",(255,228,181)),("navajo white","#FFDEAD",(255,222,173)),
        ("peach puff","#FFDAB9",(255,218,185)),("misty rose","#FFE4E1",(255,228,225)),
        ("lavender blush","#FFF0F5",(255,240,245)),("linen","#FAF0E6",(250,240,230)),
        ("old lace","#FDF5E6",(253,245,230)),("papaya whip","#FFEFD5",(255,239,213)),
        ("sea shell","#FFF5EE",(255,245,238))],
       "greys":[("black","#000000",(0,0,0)),("dim gray / dim grey","#696969",(105,105,105)),
        ("gray / grey","#808080",(128,128,128)),("dark gray / dark grey","#A9A9A9",(169,169,169)),
        ("silver","#C0C0C0",(192,192,192)),("light gray / light grey","#D3D3D3",(211,211,211)),
        ("gainsboro","#DCDCDC",(220,220,220)),("white smoke","#F5F5F5",(245,245,245)),
        ("white","#FFFFFF",(255,255,255))]}

    def assign_color_group(self,color_group,representation,component_names):
        for n,p in enumerate(component_names):
            s=IMP.atom.Selection(representation.prot,molecule=p)
            psel=s.get_selected_particles()
            ctuple=self.colors[color_group][n]
            print("Assigning "+p+" to color "+ctuple[0])
            c=ctuple[2]
            color=IMP.display.Color(float(c[0])/255,float(c[1])/255,float(c[2])/255)
            for part in psel:
                if IMP.display.Colored.get_is_setup(part):
                    IMP.display.Colored(part).set_color(color)
                else:
                    IMP.display.Colored.setup_particle(part,color)

    def get_list_distant_colors(self):
        cnames = ['#F0F8FF', '#FAEBD7', '#00FFFF', '#7FFFD4', '#F0FFFF', '#F5F5DC',
        '#FFE4C4', '#000000', '#FFEBCD', '#0000FF', '#8A2BE2', '#A52A2A', '#DEB887',
        '#5F9EA0', '#7FFF00', '#D2691E', '#FF7F50', '#6495ED', '#FFF8DC', '#DC143C',
        '#00FFFF', '#00008B', '#008B8B', '#B8860B', '#A9A9A9', '#006400', '#BDB76B',
        '#8B008B', '#556B2F', '#FF8C00', '#9932CC', '#8B0000', '#E9967A', '#8FBC8F',
        '#483D8B', '#2F4F4F', '#00CED1', '#9400D3', '#FF1493', '#00BFFF', '#696969',
        '#1E90FF', '#B22222', '#FFFAF0', '#228B22', '#FF00FF', '#DCDCDC', '#F8F8FF',
        '#FFD700', '#DAA520', '#808080', '#008000', '#ADFF2F', '#F0FFF0', '#FF69B4',
        '#CD5C5C', '#4B0082', '#FFFFF0', '#F0E68C', '#E6E6FA', '#FFF0F5', '#7CFC00',
        '#FFFACD', '#ADD8E6', '#F08080', '#E0FFFF', '#FAFAD2', '#90EE90', '#D3D3D3',
        '#FFB6C1', '#FFA07A', '#20B2AA', '#87CEFA', '#778899', '#B0C4DE', '#FFFFE0',
        '#00FF00', '#32CD32', '#FAF0E6', '#FF00FF', '#800000', '#66CDAA', '#0000CD',
        '#BA55D3', '#9370DB', '#3CB371', '#7B68EE', '#00FA9A', '#48D1CC', '#C71585',
        '#191970', '#F5FFFA', '#FFE4E1', '#FFE4B5', '#FFDEAD', '#000080', '#FDF5E6',
        '#808000', '#6B8E23', '#FFA500', '#FF4500', '#DA70D6', '#EEE8AA', '#98FB98',
        '#AFEEEE', '#DB7093', '#FFEFD5', '#FFDAB9', '#CD853F', '#FFC0CB', '#DDA0DD',
        '#B0E0E6', '#800080', '#FF0000', '#BC8F8F', '#4169E1', '#8B4513', '#FA8072',
        '#FAA460', '#2E8B57', '#FFF5EE', '#A0522D', '#C0C0C0', '#87CEEB', '#6A5ACD',
        '#708090', '#FFFAFA', '#00FF7F', '#4682B4', '#D2B48C', '#008080', '#D8BFD8',
        '#FF6347', '#40E0D0', '#EE82EE', '#F5DEB3', '#FFFFFF', '#F5F5F5', '#FFFF00',
        '#9ACD32']
        return cnames
