#!/usr/bin/env python

"""@namespace IMP.pmi.tools
   Miscellaneous utilities.
"""

from __future__ import print_function
import IMP
import IMP.algebra
import collections
import itertools

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

    def __init__(self, isdelta=True):
        global time
        import time

        self.starttime = time.clock()
        self.label = "None"
        self.isdelta = isdelta

    def set_label(self, labelstr):
        self.label = labelstr

    def get_output(self):
        output = {}
        if self.isdelta:
            newtime = time.clock()
            output[
                "Stopwatch_" +
                self.label +
                "_delta_seconds"] = str(
                newtime -
                self.starttime)
            self.starttime = newtime
        else:
            output["Stopwatch_" + self.label +
                   "_elapsed_seconds"] = str(time.clock() - self.starttime)
        return output


class SetupNuisance(object):

    def __init__(self, m, initialvalue, minvalue, maxvalue, isoptimized=True):
        import IMP.isd

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
        import IMP.isd
        pw = IMP.Particle(m)
        self.weight = IMP.isd.Weight.setup_particle(pw)
        self.weight.set_weights_are_optimized(True)

    def get_particle(self):
        return self.weight


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
        if not particle_type in ["Rigid_Bodies", "Floppy_Bodies", "Nuisances", "X_coord", "Weights"]:
            raise TypeError("not the right particle type")
        else:
            self.dictionary_particle_type[particle] = particle_type
            if particle_type == "Rigid_Bodies":
                if type(particle_transformation) == tuple and len(particle_transformation) == 2 and type(particle_transformation[0]) == float and type(particle_transformation[1]) == float:
                    self.dictionary_particle_transformation[
                        particle] = particle_transformation
                    self.dictionary_particle_name[particle] = name
                else:
                    raise TypeError("ParticleToSampleList: not the right transformation format for Rigid_Bodies, should be a tuple a floats")
            else:
                if type(particle_transformation) == float:
                    self.dictionary_particle_transformation[
                        particle] = particle_transformation
                    self.dictionary_particle_name[particle] = name
                else:
                    raise TypeError("ParticleToSampleList: not the right transformation format sould be a float")

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
        global sqrt, os, random
        from math import sqrt
        import os
        import random

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

    residue_pairs=get_random_residue_pairs(representation, resolution, number_of_cross_links, avoid_same_particles)

    from random import random
    unique_identifier=0
    cmin=float(min(confidence_score_range))
    cmax=float(max(confidence_score_range))

    dataset="#\n"

    for (name1, r1, name2, r2) in residue_pairs:
        if random() > ambiguity_probability:
            unique_identifier+=1
        score=random()*(cmax-cmin)+cmin
        dataset+=str(name1)+" "+str(name2)+" "+str(r1)+" "+str(r2)+" "+str(score)+" "+str(unique_identifier)+"\n"

    return dataset


    #-------------------------------

def get_cross_link_data(directory, filename, dist, omega, sigma,
                        don=None, doff=None, prior=0, type_of_profile="gofr"):

    (distmin, distmax, ndist) = dist
    (omegamin, omegamax, nomega) = omega
    (sigmamin, sigmamax, nsigma) = sigma
    import IMP.isd

    filen = IMP.isd.get_data_path("PMFs.dict")
    xlpot = open(filen)

    for line in xlpot:
        dictionary = eval(line)
        break

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
    import IMP.isd

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
    from math import exp, log
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

def get_position_terminal_residue(hier, terminus="C", resolution=1):
    '''
    Get the xyz position of the terminal residue at the given resolution.
    @param hier hierarchy containing the terminal residue
    @param terminus either 'N' or 'C'
    @param resolution resolution to use.
    '''
    termresidue = None
    termparticle = None
    for p in IMP.atom.get_leaves(hier):
        if IMP.pmi.Resolution(p).get_resolution() == resolution:
            residues = IMP.pmi.tools.get_residue_indexes(p)
            if terminus == "C":
                if max(residues) >= termresidue and not termresidue is None:
                    termresidue = max(residues)
                    termparticle = p
                elif termresidue is None:
                    termresidue = max(residues)
                    termparticle = p
            elif terminus == "N":
                if min(residues) <= termresidue and not termresidue is None:
                    termresidue = min(residues)
                    termparticle = p
                elif termresidue is None:
                    termresidue = min(residues)
                    termparticle = p
            else:
                raise ValueError("terminus argument should be either N or C")

    return IMP.core.XYZ(termparticle).get_coordinates()


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


def get_db_from_csv(csvfilename):
    import csv
    outputlist = []
    csvr = csv.DictReader(open(csvfilename, "rU"))
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

    The particle must be an instance of Fragment,Residue or Atom
    '''
    resind = []
    if IMP.atom.Fragment.get_is_setup(hier):
        resind = IMP.atom.Fragment(hier).get_residue_indexes()
    elif IMP.atom.Residue.get_is_setup(hier):
        resind = [IMP.atom.Residue(hier).get_index()]
    elif IMP.atom.Atom.get_is_setup(hier):
        a = IMP.atom.Atom(hier)
        resind = [IMP.atom.Residue(a.get_parent()).get_index()]
    else:
        print("get_residue_indexes> input is not Fragment, Residue or Atom")
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

    from random import choice
    particles = []
    if names is None:
        names=list(representation.hier_dict.keys())

    for name in names:
        prot = representation.hier_dict[name]
        particles += select(representation,name=name,resolution=resolution)
    random_residue_pairs = []
    while len(random_residue_pairs)<=number:
        p1 = choice(particles)
        p2 = choice(particles)
        if p1==p2 and avoid_same_particles: continue
        if IMP.core.get_distance(IMP.core.XYZ(p1),IMP.core.XYZ(p2)) > max_distance: continue
        r1 = choice(IMP.pmi.tools.get_residue_indexes(p1))
        r2 = choice(IMP.pmi.tools.get_residue_indexes(p2))
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

is_already_printed = {}


def print_deprecation_warning(old_name, new_name):
    if old_name not in is_already_printed:
        print("WARNING: " + old_name + " is deprecated, use " + new_name + " instead")
        is_already_printed[old_name] = True


def print_multicolumn(list_of_strings, ncolumns=2, truncate=40):

    l = list_of_strings

    cols = ncolumns
    # add empty entries after l
    for i in range(len(l) % cols):
        l.append(" ")

    split = [l[i:i + len(l) / cols] for i in range(0, len(l), len(l) / cols)]
    for row in zip(*split):
        print("".join(str.ljust(i, truncate) for i in row))


def parse_dssp(dssp_fn, limit_to_chains=''):
    '''Read dssp file and get secondary structure information.
       Values are all PDB residue numbering.
       @return dict of sel tuples
helix : [ [ ['A',5,7] ] , [['B',15,17]] , ...] two helices A:5-7,B:15-17
beta  : [ [ ['A',1,3] , ['A',100,102] ] , ...] one sheet: A:1-3 & A:100-102
loop  : same format as helix, it's the contiguous loops
'''

    # setup
    sses = {'helix': [],
            'beta': [],
            'loop': []}
    helix_classes = 'GHI'
    strand_classes = 'EB'
    loop_classes = [' ', '', 'T', 'S']
    sse_dict = {}
    for h in helix_classes:
        sse_dict[h] = 'helix'
    for s in strand_classes:
        sse_dict[s] = 'beta'
    for l in loop_classes:
        sse_dict[l] = 'loop'

    # read file and parse
    start = False
    # temporary beta dictionary indexed by DSSP's ID
    beta_dict = collections.defaultdict(list)
    prev_sstype = None
    cur_sse = []
    prev_beta_id = None
    for line in open(dssp_fn, 'r'):
        fields = line.split()
        chain_break = False
        if len(fields) < 2:
            continue
        if fields[1] == "RESIDUE":
            # Start parsing from here
            start = True
            continue
        if not start:
            continue
        if line[9] == " ":
            chain_break = True
        elif limit_to_chains != '' and line[11] not in limit_to_chains:
            break

        # gather line info
        if not chain_break:
            pdb_res_num = int(line[5:10])
            chain = 'chain' + line[11]
            sstype = line[16]
            beta_id = line[33]

        # decide whether to extend or store the SSE
        if prev_sstype is None:
            cur_sse = [pdb_res_num, pdb_res_num, chain]
        elif sstype != prev_sstype or chain_break:
            # add cur_sse to the right place
            if sse_dict[prev_sstype] in ['helix', 'loop']:
                sses[sse_dict[prev_sstype]].append([cur_sse])
            if sse_dict[prev_sstype] == 'beta':
                beta_dict[prev_beta_id].append(cur_sse)
            cur_sse = [pdb_res_num, pdb_res_num, chain]
        else:
            cur_sse[1] = pdb_res_num
        if chain_break:
            prev_sstype = None
            prev_beta_id = None
        else:
            prev_sstype = sstype
            prev_beta_id = beta_id

    # final SSE processing
    if not prev_sstype is None:
        if sse_dict[prev_sstype] in ['helix', 'loop']:
            sses[sse_dict[prev_sstype]].append([cur_sse])
        if sse_dict[prev_sstype] == 'beta':
            beta_dict[prev_beta_id].append(cur_sse)

    # gather betas
    for beta_sheet in beta_dict:
        sses['beta'].append(beta_dict[beta_sheet])

    return sses


def sse_selections_to_chimera_colors(dssp_dict, chimera_model_num=0):
    ''' get chimera command to check if you've correctly made the dssp dictionary
    colors each helix and beta sheet'''
    cmds = {
        'helix': 'color green ',
        'beta': 'color blue ',
        'loop': 'color red '}
    for skey in dssp_dict.keys():
        for sgroup in dssp_dict[skey]:
            for sse in sgroup:
                start, stop, chain = sse
                chain = chain.strip('chain')
                cmds[
                    skey] += '#%i:%s-%s.%s ' % (chimera_model_num, start, stop, chain)
    print('; '.join([cmds[k] for k in cmds]))




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

# -------------- PMI2 Tools --------------- #

def get_hierarchies_from_spec(spec):
    """ Given PMI Molecule/Residue or IMP object or a list of (one type) of them, return IMP hierarchies.
    @param spec Can be one of the following inputs:
                              IMP Selection, Hierarchy,
                              PMI Molecule, Residue, or a list/set
    \note if passed PMI objects like Molecules or Residues, will return ALL resolutions!
    """

    ret = []

    # check for consistent type, make into list
    if hasattr(spec,'__iter__'):
        if len(spec)==0:
            return ret
        type_set = set(type(a) for a in spec)
        if len(type_set)!=1:
            raise Exception('get_hierarchies_from_spec: can only pass one type of object at a time')
        tp = type_set.pop()
        spec = list(spec)
    else:
        tp = type(spec)
        spec = [spec]

    # if PMI object, get all resolutions. otherwise just return the hiers
    if tp==IMP.pmi.topology.Residue:
        mol_map = collections.defaultdict(list)
        for res in spec:
            mol = res.get_molecule()
            mol_map[mol].append(res.get_index())
        for mol in mol_map:
            ret += [IMP.atom.Hierarchy(p) for p in mol.get_particles_at_all_resolutions(mol_map[mol])]
    elif tp==IMP.pmi.topology.Molecule:
        for mol in spec:
            ret += [IMP.atom.Hierarchy(p) for p in mol.get_particles_at_all_resolutions()]
    elif tp==IMP.atom.Selection:
        for sel in spec:
            ret += [IMP.atom.Hierarchy(p) for p in sel.get_selected_particles()]
    elif tp==IMP.atom.Hierarchy:
        ret = spec
    else:
        raise Exception('get_hierarchies_from_spec: passed an unknown object')
    return ret

def get_all_leaves(list_of_hs):
    """ Just get the leaves from a list of hierarchies """
    lvs = list(itertools.chain.from_iterable(IMP.atom.get_leaves(item) for item in list_of_hs))
    return lvs


def select_at_all_resolutions(hier,
                              **kwargs):
    """Perform selection using the usual keywords but return ALL resolutions (BEADS and GAUSSIANS).
    Returns in flat list!
    """

    if type(hier) is not IMP.atom.Hierarchy:
        raise Exception('select_at_all_resolutions: you have to pass an IMP Hierarchy')
    if 'resolution' in kwargs or 'representation_type' in kwargs:
        raise Exception("don't pass resolution or representation_type to this function")

    def get_resolutions(h): #move to C++
        while h:
            if IMP.atom.Representation.get_is_setup(h):
                ret = [IMP.atom.Representation(h).get_resolutions(IMP.atom.BALLS),
                       IMP.atom.Representation(h).get_resolutions(IMP.atom.DENSITIES)]
                return ret
            h = h.get_parent()
        return [[],[]]

    # gather resolutions
    init_sel = IMP.atom.Selection(hier,**kwargs)
    init_ps = init_sel.get_selected_particles()
    all_bead_res = OrderedSet()
    all_density_res = OrderedSet()

    for p in init_ps:
        b,d = get_resolutions(IMP.atom.Hierarchy(p))
        all_bead_res |= OrderedSet(b)
        all_density_res |= OrderedSet(d)

    # final selection
    ret = OrderedSet()
    for res in all_bead_res:
        sel = IMP.atom.Selection(hier,resolution=res,
                                 representation_type=IMP.atom.BALLS,**kwargs)
        ret|=OrderedSet(sel.get_selected_particles())
    for res in all_density_res:
        sel = IMP.atom.Selection(hier,resolution=res,
                                 representation_type=IMP.atom.DENSITIES,**kwargs)
        ret|=OrderedSet(sel.get_selected_particles())
    return list(ret)


def get_rbs_and_beads(hiers):
    """Returns unique objects in original order"""
    rbs = set()
    beads = []
    rbs_ordered = []
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
