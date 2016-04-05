"""@namespace IMP.pmi.restraints.stereochemistry
Restraints for keeping correct stereochemistry.
"""

from __future__ import print_function
import IMP
import IMP.core
import IMP.algebra
import IMP.atom
import IMP.container
import IMP.isd
import itertools
import IMP.pmi.tools
import IMP.pmi.representation
from operator import itemgetter
from math import pi,log,sqrt
import sys


class ConnectivityRestraint(object):
    """ This class creates a restraint between consecutive TempResidue objects OR an entire
    PMI MOlecule object. """

    def __init__(self,
                 objects,
                 scale=1.0,
                 disorderedlength=False,
                 upperharmonic=True,
                 resolution=1):
        """
        @param objects - a list of hierarchies, PMI TempResidues OR a single Molecule
        @param scale - Riccardo knows what this is
        @param disorderedlength - This flag uses either disordered length
                     calculated for random coil peptides (True) or zero
                     surface-to-surface distance between beads (False)
                     as optimal distance for the sequence connectivity
                     restraint.
        @param upperharmonic - This flag uses either harmonic (False)
                     or upperharmonic (True) in the intra-pair
                     connectivity restraint.
        @parm resolution The resolution to connect things at - only used if you pass PMI objects
        """
        self.label = "None"
        self.weight = 1.0

        hiers = IMP.pmi.tools.input_adaptor(objects,resolution)
        if len(hiers)>1:
            raise Exception("ConnectivityRestraint: only pass stuff from one Molecule, please")
        hiers = hiers[0]

        self.kappa = 10  # No idea what this is
        self.m = list(hiers)[0].get_model()
        SortedSegments = []
        self.rs = IMP.RestraintSet(self.m, "connectivity_restraint")
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

        # connect the particles
        for x in range(len(SortedSegments) - 1):

            last = SortedSegments[x][1]
            first = SortedSegments[x + 1][0]

            apply_restraint = True

            # Apply connectivity runless ALL of the following are true:
            #   - first and last both have RigidBodyMember decorators
            #   - first and last are both RigidMembers
            #   - first and last are part of the same RigidBody object

            # Check for both in a rigid body
            if  IMP.core.RigidBodyMember.get_is_setup(first) and IMP.core.RigidBodyMember.get_is_setup(last) and \
                IMP.core.RigidMember.get_is_setup(first) and IMP.core.RigidMember.get_is_setup(last):

                #Check if the rigid body objects for each particle are the same object.
                #  if so, skip connectivity restraint
                if IMP.core.RigidBodyMember(first).get_rigid_body() == IMP.core.RigidBodyMember(last).get_rigid_body():
                    apply_restraint = False

            if apply_restraint:

                nreslast = len(IMP.pmi.tools.get_residue_indexes(last))
                lastresn = IMP.pmi.tools.get_residue_indexes(last)[-1]
                nresfirst = len(IMP.pmi.tools.get_residue_indexes(first))
                firstresn = IMP.pmi.tools.get_residue_indexes(first)[0]

                residuegap = firstresn - lastresn - 1
                if disorderedlength and (nreslast / 2 + nresfirst / 2 + residuegap) > 20.0:
                    # calculate the distance between the sphere centers using Kohn
                    # PNAS 2004
                    optdist = sqrt(5 / 3) * 1.93 * \
                        (nreslast / 2 + nresfirst / 2 + residuegap) ** 0.6
                    # optdist2=sqrt(5/3)*1.93*((nreslast)**0.6+(nresfirst)**0.6)/2
                    if upperharmonic:
                        hu = IMP.core.HarmonicUpperBound(optdist, self.kappa)
                    else:
                        hu = IMP.core.Harmonic(optdist, self.kappa)
                    dps = IMP.core.DistancePairScore(hu)
                else:  # default
                    optdist = (0.0 + (float(residuegap) + 1.0) * 3.6) * scale
                    if upperharmonic:  # default
                        hu = IMP.core.HarmonicUpperBound(optdist, self.kappa)
                    else:
                        hu = IMP.core.Harmonic(optdist, self.kappa)
                    dps = IMP.core.SphereDistancePairScore(hu)

                pt0 = last.get_particle()
                pt1 = first.get_particle()
                r = IMP.core.PairRestraint(self.m, dps, (pt0.get_index(), pt1.get_index()))

                print("Adding sequence connectivity restraint between", pt0.get_name(), " and ", pt1.get_name(), 'of distance', optdist)
                self.rs.add_restraint(r)


    def set_label(self, label):
        self.label = label

    def get_weight(self):
        return self.weight

    def add_to_model(self):
        IMP.pmi.tools.add_restraint_to_model(self.m, self.rs)

    def get_restraint(self):
        return self.rs

    def set_weight(self, weight):
        self.weight = weight
        self.rs.set_weight(weight)

    def get_output(self):
        self.m.update()
        output = {}
        score = self.weight * self.rs.unprotected_evaluate(None)
        output["_TotalScore"] = str(score)
        output["ConnectivityRestraint_" + self.label] = str(score)
        return output

    def get_num_restraints(self):
        """ Returns number of connectivity restraints """
        return len(self.rs.get_restraints())

    def evaluate(self):
        return self.weight * self.rs.unprotected_evaluate(None)

class ExcludedVolumeSphere(object):
    """A class to create an excluded volume restraint for a set of particles at a given resolution.
    Can be initialized as a bipartite restraint between two sets of particles.
    # Potential additional function: Variable resolution for each PMI object.  Perhaps passing selection_tuples
    with (PMI_object, resolution)
    """

    def __init__(self,
                 representation=None,
                 included_objects=None,
                 other_objects=None,
                 resolution=1000,
                 kappa=1.0):
        """Constructor.
        @param representation DEPRECATED - just pass objects
        @param included_objects Can be one of the following inputs:
               IMP Hierarchy, PMI System/State/Molecule/TempResidue, or a list/set of them
        @param other_objects Initializes a bipartite restraint between included_objects and other_objects
               Same format as included_objects
        @param resolution The resolution particles at which to impose the restraint.
               By default, the coarsest particles will be chosen.
               If a number is chosen, for each particle, the closest
               resolution will be used (see IMP.atom.Selection).
        @param kappa Restraint strength
        """

        self.weight = 1.0
        self.kappa = kappa
        self.label = "None"
        self.cpc = None
        bipartite = False

        # gather IMP hierarchies from input objects
        hierarchies = IMP.pmi.tools.input_adaptor(included_objects,
                                                  resolution,
                                                  flatten=True)
        included_ps = []
        if other_objects is not None:
            bipartite = True
            other_hierarchies = IMP.pmi.tools.input_adaptor(other_objects,
                                                            resolution,
                                                            flatten=True)
            other_ps = []

        # perform selection
        if representation is None:
            if hierarchies is None:
                raise Exception("Must at least pass included objects")
            self.mdl = hierarchies[0].get_model()
            included_ps = [h.get_particle() for h in hierarchies]
            if bipartite:
                other_ps = [h.get_particle() for h in other_hierarchies]
        elif type(representation) is IMP.pmi.representation.Representation or \
            type(representation) is IMP.pmi.representation.SimplifiedModel:
            self.mdl = representation.m
            included_ps = IMP.pmi.tools.select(
                representation,
                resolution=resolution,
                hierarchies=hierarchies)
            if bipartite:
                other_particles = IMP.pmi.tools.select(
                    representation,
                    resolution=resolution,
                    hierarchies=other_hierarchies)
        else:
            raise Exception("Must pass Representation or included_objects")

        # setup score
        self.rs = IMP.RestraintSet(self.mdl, 'excluded_volume')
        ssps = IMP.core.SoftSpherePairScore(self.kappa)
        lsa = IMP.container.ListSingletonContainer(self.mdl)
        lsa.add(IMP.get_indexes(included_ps))

        # setup close pair container
        if not bipartite:
            rbcpf = IMP.core.RigidClosePairsFinder()
            self.cpc = IMP.container.ClosePairContainer(lsa, 0.0, rbcpf, 10.0)
            evr = IMP.container.PairsRestraint(ssps, self.cpc)
        else:
            other_lsa = IMP.container.ListSingletonContainer(self.mdl)
            other_lsa.add_particles(other_ps)
            self.cpc = IMP.container.CloseBipartitePairContainer(
                lsa,
                other_lsa,
                0.0,
                10.0)
            evr = IMP.container.PairsRestraint(ssps, self.cpc)

        self.rs.add_restraint(evr)

    def add_excluded_particle_pairs(self, excluded_particle_pairs):
        # add pairs to be filtered when calculating the score
        lpc = IMP.container.ListPairContainer(self.mdl)
        lpc.add(IMP.get_indexes(excluded_particle_pairs))
        icpf = IMP.container.InContainerPairFilter(lpc)
        self.cpc.add_pair_filter(icpf)

    def set_label(self, label):
        self.label = label

    def add_to_model(self):
        IMP.pmi.tools.add_restraint_to_model(self.mdl, self.rs)

    def get_restraint(self):
        return self.rs

    def set_weight(self, weight):
        self.weight = weight
        self.rs.set_weight(weight)

    def get_output(self):
        self.mdl.update()
        output = {}
        score = self.weight * self.rs.unprotected_evaluate(None)
        output["_TotalScore"] = str(score)
        output["ExcludedVolumeSphere_" + self.label] = str(score)
        return output

    def evaluate(self):
        return self.weight * self.rs.unprotected_evaluate(None)

class AtomicHelixRestraint(object):
    """Enforce current dihedrals for a selection"""
    def __init__(self,
                 hierarchy,
                 selection_tuple,
                 strength=10.0):
        """Constructor
        @param hierarchy the root node
        @param selection_tuple (start,stop, molname, copynum=0)
        @param strength the restraint strength
        """
        self.mdl = hierarchy.get_model()
        self.rs = IMP.RestraintSet(self.mdl)
        self.weight = 1.0
        self.label = ''
        start = selection_tuple[0]
        stop = selection_tuple[1]
        mol = selection_tuple[2]
        copy_index = 0
        if len(selection_tuple)>3:
            copy_index = selection_tuple[3]

        sel0 = IMP.atom.Selection(hierarchy,molecule=mol,copy_index=copy_index)
        mol_hier = sel0.get_selected_particles(with_representation=False)[0]
        def get_atom(idx,name):
            sel = IMP.atom.Selection(mol_hier,residue_index=idx,atom_type=IMP.atom.AtomType(name))
            return IMP.atom.Atom(sel.get_selected_particles()[0])

        #create two dihedrals: C(-1)-N-CA-C and a N - CA - C - N(+1)
        for res_idx in range(start,stop+1-2):
            a1 = get_atom(res_idx,'C')
            a2 = get_atom(res_idx+1,'N')
            a3 = get_atom(res_idx+1,'CA')
            a4 = get_atom(res_idx+1,'C')
            a5 = get_atom(res_idx+2,'N')

            angle1 = IMP.pmi.get_dihedral_angle(a1,a2,a3,a4)
            hr = IMP.core.Harmonic(angle1,strength)
            self.rs.add_restraint(IMP.core.DihedralRestraint(self.mdl,hr,a1,a2,a3,a4))
            angle2 = IMP.pmi.get_dihedral_angle(a2,a3,a4,a5)
            hr = IMP.core.Harmonic(angle2,strength)
            self.rs.add_restraint(IMP.core.DihedralRestraint(self.mdl,hr,a2,a3,a4,a5))

    def set_label(self, label):
        self.label = label

    def get_weight(self):
        return self.weight

    def add_to_model(self):
        IMP.pmi.tools.add_restraint_to_model(self.mdl, self.rs)

    def get_restraint(self):
        return self.rs

    def set_weight(self, weight):
        self.weight = weight
        self.rs.set_weight(weight)

    def get_number_of_restraints(self):
        """ Returns number of connectivity restraints """
        return len(self.rs.get_restraints())

    def evaluate(self):
        return self.weight * self.rs.unprotected_evaluate(None)

    def get_output(self):
        self.mdl.update()
        output = {}
        score = self.weight * self.rs.unprotected_evaluate(None)
        output["_TotalScore"] = str(score)
        output["AtomicHelixRestraint_" + self.label] = str(score)
        return output

class ResidueBondRestraint(object):
    """ Add bond restraint between pair of consecutive
    residues/beads to enforce the stereochemistry.
    """
    def __init__(self,
                 representation,
                 selection_tuple,
                 distance=3.78,
                 strength=10.0,
                 jitter=None):
        """Constructor
        @param representation
        @param selection_tuple Requested selection
        @param distance Resting distance for restraint
        @param strength Bond constant
        @param jitter Defines the +- added to the optimal distance in the harmonic well restraint
                      used to increase the tolerance
        """
        self.m = representation.prot.get_model()
        self.rs = IMP.RestraintSet(self.m, "Bonds")
        self.weight = 1
        self.label = "None"
        self.pairslist = []

        particles = IMP.pmi.tools.select_by_tuple(
            representation,
            selection_tuple,
            resolution=1)

        if not jitter:
            ts = IMP.core.Harmonic(distance, strength)
        else:
            ts = IMP.core.HarmonicWell(
                (distance - jitter, distance + jitter), strength)

        for ps in IMP.pmi.tools.sublist_iterator(particles, 2, 2):
            pair = []
            if len(ps) != 2:
                raise ValueError("wrong length of pair")
            for p in ps:
                if not IMP.atom.Residue.get_is_setup(p):
                    raise TypeError("not a residue")
                else:
                    pair.append(p)
            print("ResidueBondRestraint: adding a restraint between %s %s" % (pair[0].get_name(), pair[1].get_name()))
            self.rs.add_restraint(
                IMP.core.DistanceRestraint(self.m, ts, pair[0], pair[1]))
            self.pairslist.append(IMP.ParticlePair(pair[0], pair[1]))
            self.pairslist.append(IMP.ParticlePair(pair[1], pair[0]))

    def set_label(self, label):
        self.label = label
        self.rs.set_name(label)
        for r in self.rs.get_restraints():
            r.set_name(label)

    def add_to_model(self):
        IMP.pmi.tools.add_restraint_to_model(self.m, self.rs)

    def get_restraint(self):
        return self.rs

    def set_weight(self, weight):
        self.weight = weight
        self.rs.set_weight(weight)

    def get_excluded_pairs(self):
        return self.pairslist

    def get_output(self):
        self.m.update()
        output = {}
        score = self.weight * self.rs.unprotected_evaluate(None)
        output["_TotalScore"] = str(score)
        output["ResidueBondRestraint_" + self.label] = str(score)
        return output


class ResidueAngleRestraint(object):
    """Add angular restraint between triplets of consecutive
    residues/beads to enforce the stereochemistry.
    """
    def __init__(self,
                 representation,
                 selection_tuple,
                 anglemin=100.0,
                 anglemax=140.0,
                 strength=10.0):
        self.m = representation.prot.get_model()
        self.rs = IMP.RestraintSet(self.m, "Angles")
        self.weight = 1
        self.label = "None"
        self.pairslist = []

        particles = IMP.pmi.tools.select_by_tuple(
            representation,
            selection_tuple,
            resolution=1)

        ts = IMP.core.HarmonicWell(
            (pi * anglemin / 180.0,
             pi * anglemax / 180.0),
            strength)

        for ps in IMP.pmi.tools.sublist_iterator(particles, 3, 3):
            triplet = []
            if len(ps) != 3:
                raise ValueError("wrong length of triplet")
            for p in ps:
                if not IMP.atom.Residue.get_is_setup(p):
                    raise TypeError("not a residue")
                else:
                    triplet.append(p)
            print("ResidueAngleRestraint: adding a restraint between %s %s %s" % (triplet[0].get_name(), triplet[1].get_name(), triplet[2].get_name()))
            self.rs.add_restraint(
                IMP.core.AngleRestraint(triplet[0].get_model(), ts,
                                        triplet[0],
                                        triplet[1],
                                        triplet[2]))
            self.pairslist.append(IMP.ParticlePair(triplet[0], triplet[2]))
            self.pairslist.append(IMP.ParticlePair(triplet[2], triplet[0]))

    def set_label(self, label):
        self.label = label
        self.rs.set_name(label)
        for r in self.rs.get_restraints():
            r.set_name(label)

    def add_to_model(self):
        IMP.pmi.tools.add_restraint_to_model(self.m, self.rs)

    def get_restraint(self):
        return self.rs

    def set_weight(self, weight):
        self.weight = weight
        self.rs.set_weight(weight)

    def get_excluded_pairs(self):
        return self.pairslist

    def get_output(self):
        self.m.update()
        output = {}
        score = self.weight * self.rs.unprotected_evaluate(None)
        output["_TotalScore"] = str(score)
        output["ResidueAngleRestraint_" + self.label] = str(score)
        return output


class ResidueDihedralRestraint(object):
    """Add dihedral restraints between quadruplet of consecutive
    residues/beads to enforce the stereochemistry.
    Give as input a string of "C" and "T", meaning cys (0+-40) or trans (180+-40)
    dihedral. The length of the string must be \#residue-3.
    Without the string, the dihedral will be assumed trans.
    """
    def __init__(
            self,
            representation,
            selection_tuple,
            stringsequence=None,
            strength=10.0):
        self.m = representation.prot.get_model()
        self.rs = IMP.RestraintSet(self.m, "Angles")
        self.weight = 1
        self.label = "None"
        self.pairslist = []

        particles = IMP.pmi.tools.select_by_tuple(
            representation,
            selection_tuple,
            resolution=1)

        if stringsequence is None:
            stringsequence = "T" * (len(particles) - 3)

        for n, ps in enumerate(IMP.pmi.tools.sublist_iterator(particles, 4, 4)):
            quadruplet = []
            if len(ps) != 4:
                raise ValueError("wrong length of quadruplet")
            for p in ps:
                if not IMP.atom.Residue.get_is_setup(p):
                    raise TypeError("not a residue")
                else:
                    quadruplet.append(p)
            dihedraltype = stringsequence[n]
            if dihedraltype == "C":
                anglemin = -20.0
                anglemax = 20.0
                ts = IMP.core.HarmonicWell(
                    (pi * anglemin / 180.0,
                     pi * anglemax / 180.0),
                    strength)
                print("ResidueDihedralRestraint: adding a CYS restraint between %s %s %s %s" % (quadruplet[0].get_name(), quadruplet[1].get_name(),
                                                                                                quadruplet[2].get_name(), quadruplet[3].get_name()))
            if dihedraltype == "T":
                anglemin = 180 - 70.0
                anglemax = 180 + 70.0
                ts = IMP.core.HarmonicWell(
                    (pi * anglemin / 180.0,
                     pi * anglemax / 180.0),
                    strength)
                print("ResidueDihedralRestraint: adding a TRANS restraint between %s %s %s %s" % (quadruplet[0].get_name(), quadruplet[1].get_name(),
                                                                                                  quadruplet[2].get_name(), quadruplet[3].get_name()))
            self.rs.add_restraint(
                IMP.core.DihedralRestraint(ts,
                                           quadruplet[0],
                                           quadruplet[1],
                                           quadruplet[2],
                                           quadruplet[3]))
            self.pairslist.append(
                IMP.ParticlePair(quadruplet[0], quadruplet[3]))
            self.pairslist.append(
                IMP.ParticlePair(quadruplet[3], quadruplet[0]))

    def set_label(self, label):
        self.label = label
        self.rs.set_name(label)
        for r in self.rs.get_restraints():
            r.set_name(label)

    def add_to_model(self):
        IMP.pmi.tools.add_restraint_to_model(self.m, self.rs)

    def get_restraint(self):
        return self.rs

    def set_weight(self, weight):
        self.weight = weight
        self.rs.set_weight(weight)

    def get_excluded_pairs(self):
        return self.pairslist

    def get_output(self):
        self.m.update()
        output = {}
        score = self.weight * self.rs.unprotected_evaluate(None)
        output["_TotalScore"] = str(score)
        output["ResidueDihedralRestraint_" + self.label] = str(score)
        return output

class SecondaryStructure(object):
    """Experimental, requires isd_emxl for now"""
    def __init__(self,
                 representation,
                 selection_tuple,
                 ssstring,
                 mixture=False,
                 nativeness=1.0,
                 kt_caff=0.1):

        if no_isd_emxl:
            raise ValueError("IMP.isd_emxl is needed")

        # check that the secondary structure string
        # is compatible with the ssstring

        self.particles = IMP.pmi.tools.select_by_tuple(
            representation,
            selection_tuple,
            resolution=1)
        self.m = representation.prot.get_model()
        self.dihe_dict = {}
        self.ang_dict = {}
        self.do_mix = {}
        self.anglfilename = IMP.isd_emxl.get_data_path("CAAngleRestraint.dat")
        self.dihefilename = IMP.isd_emxl.get_data_path(
            "CADihedralRestraint.dat")
        self.nativeness = nativeness
        self.kt_caff = kt_caff
        self.anglrs = IMP.RestraintSet(self.m, "Angles")
        self.dihers = IMP.RestraintSet(self.m, "Dihedrals")
        self.bondrs = IMP.RestraintSet(self.m, "Bonds")
        self.label = "None"

        if len(self.particles) != len(ssstring):
            print(len(self.particles), len(ssstring))
            print("SecondaryStructure: residue range and SS string incompatible")
        self.ssstring = ssstring

        (bondrslist, anglrslist, diherslist,
         pairslist) = self.get_CA_force_field()
        self.pairslist = pairslist

        # print anglrslist, diherslist, bondrslist, self.particles
        self.anglrs.add_restraints(anglrslist)
        self.dihers.add_restraints(diherslist)
        self.bondrs.add_restraints(bondrslist)

    def set_label(self, label):
        self.label = label

    def add_to_model(self):
        IMP.pmi.tools.add_restraint_to_model(self.m, self.anglrs)
        IMP.pmi.tools.add_restraint_to_model(self.m, self.dihers)
        IMP.pmi.tools.add_restraint_to_model(self.m, self.bondrs)

    def get_CA_force_field(self):
        bondrslist = []
        anglrslist = []
        diherslist = []
        pairslist = []
        # add bonds
        for res in range(0, len(self.particles) - 1):

            ps = self.particles[res:res + 2]
            pairslist.append(IMP.ParticlePair(ps[0], ps[1]))
            pairslist.append(IMP.ParticlePair(ps[1], ps[0]))
            br = self.get_distance_restraint(ps[0], ps[1], 3.78, 416.0)
            br.set_name('Bond_restraint')
            bondrslist.append(br)
        # add dihedrals
        for res in range(0, len(self.particles) - 4):

            # if res not in dihe_dict: continue
            # get the appropriate parameters
            # get the particles
            ps = self.particles[res:res + 5]
            [phi0,
             phi1,
             score_dih] = self.read_potential_dihedral(
                self.ssstring[res:res + 4],
                True)
            pairslist.append(IMP.ParticlePair(ps[0], ps[3]))
            pairslist.append(IMP.ParticlePair(ps[3], ps[0]))
            pairslist.append(IMP.ParticlePair(ps[1], ps[4]))
            pairslist.append(IMP.ParticlePair(ps[4], ps[1]))
            dr = IMP.isd_emxl.CADihedralRestraint(
                ps[0],
                ps[1],
                ps[2],
                ps[3],
                ps[4],
                phi0,
                phi1,
                score_dih)
            dr.set_name('Dihedral restraint')
            diherslist.append(dr)
        # add angles
        for res in range(0, len(self.particles) - 2):
            ps = self.particles[res:res + 3]
            [psi, score_ang] = self.read_potential_angle(
                self.ssstring[res:res + 2], True)
            pairslist.append(IMP.ParticlePair(ps[0], ps[2]))
            pairslist.append(IMP.ParticlePair(ps[2], ps[0]))
            dr = IMP.isd_emxl.CAAngleRestraint(
                ps[0],
                ps[1],
                ps[2],
                psi,
                score_ang)
            dr.set_name('Angle restraint')
            anglrslist.append(dr)
        return (bondrslist, anglrslist, diherslist, pairslist)

    def read_potential_dihedral(self, string, mix=False):
    # read potentials for dihedral
        score_dih = []
        phi0 = []
        phi1 = []
        for i in range(0, 36):
            phi0.append(i * 10.0 / 180.0 * pi)
            phi1.append(i * 10.0 / 180.0 * pi)
            for j in range(0, 36):
                score_dih.append(0.0)
        # open file
        if not mix:
            f = open(self.dihefilename, 'r')
            for line in f.readlines():
                riga = (line.strip()).split()
                if (len(riga) == 4 and riga[0] == string):
                    ii = int(float(riga[1]) / 10.0)
                    jj = int(float(riga[2]) / 10.0)
                    score_dih[ii * len(phi0) + jj] = - \
                        self.kt_caff * self.log(float(riga[3]))
            f.close()
        if mix:
            # mix random coil and native secondary structure
            counts = []
            for i in range(0, 36):
                for j in range(0, 36):
                    counts.append(1.0)
            f = open(self.dihefilename, 'r')
            for line in f.readlines():
                riga = (line.strip()).split()
                if (len(riga) == 4 and riga[0] == string):
                    ii = int(float(riga[1]) / 10.0)
                    jj = int(float(riga[2]) / 10.0)
                    counts[ii * len(phi0) + jj] += self.nativeness * \
                        float(riga[3])
                if (len(riga) == 4 and riga[0] == "-----"):
                    ii = int(float(riga[1]) / 10.0)
                    jj = int(float(riga[2]) / 10.0)
                    counts[ii * len(phi0) + jj] += (1.0 - self.nativeness) * \
                        float(riga[3])
            f.close()
            for i in range(len(counts)):
                score_dih[i] = -self.kt_caff * self.log(counts[i])
        return [phi0, phi1, score_dih]

    def read_potential_angle(self, string, mix=False):
    # read potentials for angles
        score_ang = []
        psi = []
        for i in range(0, 180):
            psi.append(i / 180.0 * pi)
            score_ang.append(0.0)
        # read file
        if not mix:
            f = open(self.anglfilename, 'r')
            for line in f.readlines():
                riga = (line.strip()).split()
                if (len(riga) == 3 and riga[0] == string):
                    ii = int(riga[1])
                    score_ang[ii] = -self.kt_caff * self.log(float(riga[2]))
            f.close()
        if mix:
            # mix random coil and native secondary structure
            counts = []
            for i in range(0, 180):
                counts.append(1.0)

            f = open(self.anglfilename, 'r')
            for line in f.readlines():
                riga = (line.strip()).split()
                if (len(riga) == 3 and riga[0] == string):
                    ii = int(riga[1])
                    counts[ii] += self.nativeness * float(riga[2])
                if (len(riga) == 3 and riga[0] == "---"):
                    ii = int(riga[1])
                    counts[ii] += (1.0 - self.nativeness) * float(riga[2])
            f.close()
            for i in range(0, 180):
                score_ang[i] = -self.kt_caff * self.log(counts[i])
        return [psi, score_ang]

    def get_excluded_pairs(self):
        return self.pairslist

    def get_restraint(self):
        tmprs = IMP.RestraintSet(self.m, 'tmp')
        tmprs.add_restraint(self.anglrs)
        tmprs.add_restraint(self.dihers)
        tmprs.add_restraint(self.bondrs)
        return tmprs

    def get_distance_restraint(self, p0, p1, d0, kappa):
        h = IMP.core.Harmonic(d0, kappa)
        dps = IMP.core.DistancePairScore(h)
        pr = IMP.core.PairRestraint(self.m, dps, IMP.ParticlePair(p0, p1))
        return pr

    def get_output(self):
        output = {}
        self.m.update()
        score_angle = self.anglrs.unprotected_evaluate(None)
        score_dihers = self.dihers.unprotected_evaluate(None)
        score_bondrs = self.bondrs.unprotected_evaluate(None)
        output["_TotalScore"] = str(score_angle + score_dihers + score_bondrs)

        output["SecondaryStructure_Angles_" + self.label] = str(score_angle)
        output["SecondaryStructure_Dihedrals_" +
               self.label] = str(score_dihers)
        output["SecondaryStructure_Bonds_" + self.label] = str(score_bondrs)
        return output


class ElasticNetworkRestraint(object):
    """Add harmonic restraints between all pairs
    """
    def __init__(self,representation=None,
                 selection_tuples=None,
                 resolution=1,
                 strength=0.01,
                 dist_cutoff=10.0,
                 ca_only=True,
                 hierarchy=None):
        """Constructor
        @param representation Representation object
        @param selection_tuples Selecting regions for the restraint [[start,stop,molname,copy_index=0],...]
        @param resolution Resolution for applying restraint
        @param strength Bond strength
        @param dist_cutoff Cutoff for making restraints
        @param ca_only Selects only CAlphas. Only matters if resolution=0.
        @param hierarchy Root hierarchy to select from, use this instead of representation
        """

        particles = []
        if representation is None and hierarchy is not None:
            self.m = hierarchy.get_model()
            for st in selection_tuples:
                copy_index=0
                if len(st)>3:
                    copy_index=st[3]
                if not ca_only:
                    sel = IMP.atom.Selection(hierarchy,molecule=st[2],residue_indexes=range(st[0],st[1]+1),
                                             copy_index=copy_index)
                else:
                    sel = IMP.atom.Selection(hierarchy,molecule=st[2],residue_indexes=range(st[0],st[1]+1),
                                             copy_index=copy_index,
                                             atom_type=IMP.atom.AtomType("CA"))
                particles+=sel.get_selected_particles()
        elif representation is not None and type(representation)==IMP.pmi.representation.Representation:
            self.m = representation.mdl
            for st in selection_tuples:
                print('selecting with',st)
                for p in IMP.pmi.tools.select_by_tuple(representation,st,resolution=resolution):
                    if (resolution==0 and ca_only and IMP.atom.Atom(p).get_atom_type()!=IMP.atom.AtomType("CA")):
                        continue
                    else:
                        particles.append(p.get_particle())
        else:
            raise Exception("must pass representation or hierarchy")

        self.weight = 1
        self.label = "None"
        self.pairslist = []

        # create score
        self.rs = IMP.pmi.create_elastic_network(particles,dist_cutoff,strength)
        for r in self.rs.get_restraints():
            a1,a2 = r.get_inputs()
            self.pairslist.append(IMP.ParticlePair(a1,a2))
            self.pairslist.append(IMP.ParticlePair(a2,a1))
        print('ElasticNetwork: created',self.rs.get_number_of_restraints(),'restraints')

    def set_label(self, label):
        self.label = label
        self.rs.set_name(label)
        for r in self.rs.get_restraints():
            r.set_name(label)

    def add_to_model(self):
        IMP.pmi.tools.add_restraint_to_model(self.m, self.rs)

    def get_restraint(self):
        return self.rs

    def set_weight(self, weight):
        self.weight = weight
        self.rs.set_weight(weight)

    def get_excluded_pairs(self):
        return self.pairslist

    def get_output(self):
        self.m.update()
        output = {}
        score = self.weight * self.rs.unprotected_evaluate(None)
        output["_TotalScore"] = str(score)
        output["ElasticNetworkRestraint_" + self.label] = str(score)
        return output


class CharmmForceFieldRestraint(object):
    """ Enable CHARMM force field """
    def __init__(self,
                 root=None,
                 ff_temp=300.0,
                 zone_ps=None,
                 zone_size=10.0,
                 enable_nonbonded=True,
                 enable_bonded=True,
                 zone_nonbonded=False,
                 representation=None):
        """Setup the CHARMM restraint on a selection. Expecting atoms.
        @param root             The node at which to apply the restraint
        @param ff_temp          The temperature of the force field
        @param zone_ps          Create a zone around this set of particles
               Automatically includes the entire residue (incl. backbone)
        @param zone_size        The size for looking for neighbor residues
        @param enable_nonbonded Allow the repulsive restraint
        @param enable_bonded    Allow the bonded restraint
        @param zone_nonbonded   EXPERIMENTAL: exclude from nonbonded all sidechains that aren't in zone!
        @param representation Legacy representation object
        """

        kB = (1.381 * 6.02214) / 4184.0
        if representation is not None:
            root = representation.prot

        self.mdl = root.get_model()
        self.bonds_rs = IMP.RestraintSet(self.mdl, 1.0 / (kB * ff_temp), 'BONDED')
        self.nonbonded_rs = IMP.RestraintSet(self.mdl, 1.0 / (kB * ff_temp), 'NONBONDED')
        self.weight = 1.0
        self.label = ""

        ### setup topology and bonds etc
        if enable_bonded:
            ff = IMP.atom.get_heavy_atom_CHARMM_parameters()
            topology = ff.create_topology(root)
            topology.apply_default_patches()
            topology.setup_hierarchy(root)
            if zone_ps is not None:
                limit_to_ps=IMP.pmi.topology.get_particles_within_zone(
                    root,
                    zone_ps,
                    zone_size,
                    entire_residues=True,
                    exclude_backbone=False)
                r = IMP.atom.CHARMMStereochemistryRestraint(root,
                                                            topology,
                                                            limit_to_ps)
                self.ps = limit_to_ps
            else:
                r = IMP.atom.CHARMMStereochemistryRestraint(root, topology)
                self.ps = IMP.core.get_leaves(root)
            print('init bonds score',r.unprotected_evaluate(None))
            self.bonds_rs.add_restraint(r)
            ff.add_radii(root)
            ff.add_well_depths(root)

        atoms = IMP.atom.get_by_type(root,IMP.atom.ATOM_TYPE)
        ### non-bonded forces
        if enable_nonbonded:
            if (zone_ps is not None) and zone_nonbonded:
                print('stereochemistry: zone_nonbonded is True')
                # atoms list should only include backbone plus zone_ps!
                backbone_types=['C','N','CB','O']
                sel = IMP.atom.Selection(root,atom_types=[IMP.atom.AtomType(n)
                                                          for n in backbone_types])
                backbone_atoms = sel.get_selected_particles()
                #x = list(set(backbone_atoms)|set(limit_to_ps))
                sel_ps=IMP.pmi.topology.get_particles_within_zone(
                    root,
                    zone_ps,
                    zone_size,
                    entire_residues=True,
                    exclude_backbone=True)

                self.nbl = IMP.container.CloseBipartitePairContainer(
                    IMP.container.ListSingletonContainer(backbone_atoms),
                    IMP.container.ListSingletonContainer(sel_ps),
                    4.0)
            else:
                cont = IMP.container.ListSingletonContainer(self.mdl,atoms)
                self.nbl = IMP.container.ClosePairContainer(cont, 4.0)
            if enable_bonded:
                self.nbl.add_pair_filter(r.get_full_pair_filter())
            pairscore = IMP.isd.RepulsiveDistancePairScore(0,1)
            pr=IMP.container.PairsRestraint(pairscore, self.nbl)
            self.nonbonded_rs.add_restraint(pr)
        print('CHARMM is set up')

    def set_label(self, label):
        self.label = label
        self.rs.set_name(label)
        for r in self.rs.get_restraints():
            r.set_name(label)

    def add_to_model(self):
        IMP.pmi.tools.add_restraint_to_model(self.mdl, self.bonds_rs)
        IMP.pmi.tools.add_restraint_to_model(self.mdl, self.nonbonded_rs)

    def get_restraint(self):
        return self.rs

    def get_close_pair_container(self):
        return self.nbl

    def set_weight(self, weight):
        self.weight = weight
        self.rs.set_weight(weight)

    def get_output(self):
        self.mdl.update()
        output = {}
        bonds_score = self.weight * self.bonds_rs.unprotected_evaluate(None)
        nonbonded_score = self.weight * self.nonbonded_rs.unprotected_evaluate(None)
        score=bonds_score+nonbonded_score
        output["_TotalScore"] = str(score)
        output["CHARMM_BONDS"] = str(bonds_score)
        output["CHARMM_NONBONDED"] = str(nonbonded_score)
        return output


class PseudoAtomicRestraint(object):
    """Add bonds and improper dihedral restraints for the CBs
    """
    def __init__(
        self, rnums, representation, selection_tuple, strength=10.0, kappa=1.0,
            jitter_angle=0.0, jitter_improper=0.0):
        '''
        need to add:
        ca-ca bond
        ca-cb is a constraint, no restraint needed
        ca-ca-ca
        cb-ca-ca-cb
        '''

        self.m = representation.prot.get_model()
        self.rs = IMP.RestraintSet(self.m, "PseudoAtomic")
        self.rset_angles = IMP.RestraintSet(self.m, "PseudoAtomic_Angles")
        self.rset_bonds = IMP.RestraintSet(self.m, "PseudoAtomic_Bonds")
        self.weight = 1
        self.label = "None"
        self.pairslist = []

        # residues=IMP.pmi.tools.select_by_tuple(representation,selection_tuple,resolution=1)
        for rnum in rnums:
            ca, cb = self.get_ca_cb(
                IMP.pmi.tools.select_by_tuple(representation,
                                              (rnum, rnum, 'chainA'), resolution=0))
            if not cb is None:
                nter = False
                cter = False
                ca_prev, cb_prev = self.get_ca_cb(
                    IMP.pmi.tools.select_by_tuple(representation,
                                                  (rnum - 1, rnum - 1, 'chainA'), resolution=0))
                ca_next, cb_next = self.get_ca_cb(
                    IMP.pmi.tools.select_by_tuple(representation,
                                                  (rnum + 1, rnum + 1, 'chainA'), resolution=0))
                if ca_prev is None:
                    nter = True
                else:
                    if ca_next is None:
                        cter = True
                    else:
                        if (nter and cter):
                            continue

                # adding a bond restraint between CA and CB
                # h=IMP.core.Harmonic(6.0,kappa)
                # dps=IMP.core.DistancePairScore(h)
                # pr=IMP.core.PairRestraint(dps,IMP.ParticlePair(ca,cb))
                # self.pairslist.append((ca,cb))
                # self.rset_bonds.add_restraint(pr)

                # creating improper dihedral restraint
                # hus=IMP.core.Harmonic(2.09,kappa)
                hupp = IMP.core.HarmonicUpperBound(
                    2.09 +
                    jitter_angle /
                    0.5,
                    kappa)
                hlow = IMP.core.HarmonicLowerBound(
                    2.09 -
                    jitter_angle /
                    0.5,
                    kappa)
                if not nter:
                    # ar13=IMP.core.AngleRestraint(hus,ca_prev,ca,cb)
                    # self.rset_angles.add_restraint(ar13)
                    ar13u = IMP.core.AngleRestraint(hupp, ca_prev, ca, cb)
                    ar13l = IMP.core.AngleRestraint(hlow, ca_prev, ca, cb)
                    self.rset_angles.add_restraint(ar13u)
                    self.rset_angles.add_restraint(ar13l)
                if not cter:
                    # ar23=IMP.core.AngleRestraint(hus,ca_next,ca,cb)
                    # self.rset_angles.add_restraint(ar23)
                    ar23u = IMP.core.AngleRestraint(hupp, ca_next, ca, cb)
                    ar23l = IMP.core.AngleRestraint(hlow, ca_next, ca, cb)
                    self.rset_angles.add_restraint(ar23u)
                    self.rset_angles.add_restraint(ar23l)
                if not nter and not cter:
                    # hus2=IMP.core.Harmonic(0,kappa)
                    # idr=IMP.core.DihedralRestraint(hus2,ca,ca_prev,ca_next,cb)
                    # self.rset_angles.add_restraint(idr)

                    hus2upp = IMP.core.HarmonicUpperBound(
                        jitter_improper,
                        kappa)
                    hus2low = IMP.core.HarmonicLowerBound(
                        -
                        jitter_improper,
                        kappa)
                    idru = IMP.core.DihedralRestraint(
                        hus2upp,
                        ca,
                        ca_prev,
                        ca_next,
                        cb)
                    idrl = IMP.core.DihedralRestraint(
                        hus2low,
                        ca,
                        ca_prev,
                        ca_next,
                        cb)
                    self.rset_angles.add_restraint(idru)
                    self.rset_angles.add_restraint(idrl)
        self.rs.add_restraint(self.rset_bonds)
        self.rs.add_restraint(self.rset_angles)

    def get_ca_cb(self, atoms):
        ca = None
        cb = None
        for a in atoms:
            if IMP.atom.Atom(a).get_atom_type() == IMP.atom.AtomType("CA"):
                ca = a.get_particle()
            elif IMP.atom.Atom(a).get_atom_type() == IMP.atom.AtomType("CB"):
                cb = a.get_particle()
        return ca, cb

    def set_label(self, label):
        self.label = label
        self.rs.set_name(label)
        for r in self.rs.get_restraints():
            r.set_name(label)

    def add_to_model(self):
        IMP.pmi.tools.add_restraint_to_model(self.m, self.rs)

    def get_restraint(self):
        return self.rs

    def set_weight(self, weight):
        self.weight = weight
        self.rs.set_weight(weight)

    def get_excluded_pairs(self):
        return self.pairslist

    def get_output(self):
        self.m.update()
        output = {}
        score = self.weight * self.rs.unprotected_evaluate(None)
        output["_TotalScore"] = str(score)
        output["PseudoAtomicRestraint_" + self.label] = str(score)
        return output

class SymmetryRestraint(object):
    """Create harmonic restraints between the reference and (transformed) clones.
    \note Wraps IMP::core::TransformedDistancePairScore with an IMP::core::Harmonic
    """
    def __init__(self,
                 references,
                 clones_list,
                 transforms,
                 label='',
                 strength=10.0):
        """Constructor
        @param references List of particles for symmetry reference
        @param clones_list List of lists of clone particles
        @param transforms Transforms moving each selection to the first selection
        @param label Label for output
        @param strength            The elastic bond strength
        \note You will have to perform an IMP::atom::Selection to get the particles you need.
        Will check to make sure the numbers match.
        """
        self.mdl = root.get_model()
        self.rs = IMP.RestraintSet(self.mdl, "Symmetry")
        self.weight = 1
        self.label = label
        if len(clones_list)!=len(transforms):
            raise Exception('Error: There should be as many clones as transforms')

        harmonic = IMP.core.Harmonic(0.,strength)
        for clones,trans in zip(clones_list,transforms):
            if len(clones)!=len(references):
                raise Exception("Error: len(references)!=len(clones)")
            pair_score = IMP.core.TransformedDistancePairScore(harmonic,trans)
            for p0,p1 in zip(references,clones):
                r = IMP.core.PairRestraint(pair_score,(p0,p1))
                self.rs.add_restraint(r)

        print('created symmetry network with',self.rs.get_number_of_restraints(),'restraints')

    def set_label(self, label):
        self.label = label
        self.rs.set_name(label)
        for r in self.rs.get_restraints():
            r.set_name(label)

    def add_to_model(self):
        IMP.pmi.tools.add_restraint_to_model(self.m, self.rs)

    def get_restraint(self):
        return self.rs

    def set_weight(self, weight):
        self.weight = weight
        self.rs.set_weight(weight)

    def get_excluded_pairs(self):
        return self.pairslist

    def get_output(self):
        self.mdl.update()
        output = {}
        score = self.weight * self.rs.unprotected_evaluate(None)
        output["SymmetryRestraint_" + self.label] = str(score)
        output["_TotalScore"] = str(score)
        return output
