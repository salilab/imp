"""@namespace IMP.pmi.restraints.stereochemistry
Restraints for keeping correct stereochemistry.
"""

from __future__ import print_function
import IMP
import IMP.core
import IMP.algebra
import IMP.atom
import IMP.container
import itertools
import sys
import IMP.pmi.tools
try:
    import IMP.isd2
    noisd2 = False
except:
    noisd2 = True
try:
    import IMP.isd_emxl
    no_isd_emxl = False
except:
    no_isd_emxl = True


class ExcludedVolumeSphere(object):

    '''
    all leaves of the input hierarchies will be input in the
    restraint. If other_hierarchies is defined, then a Bipartite container
    between "hierarchies" and "other_hierarchies" leaves is initialized
    '''

    def __init__(self, representation,
                 hierarchies=None,
                 other_hierarchies=None,
                 resolution=None, kappa=1.0):

        self.m = representation.prot.get_model()
        self.rs = IMP.RestraintSet(self.m, 'excluded_volume')
        self.weight = 1.0
        self.kappa = kappa

        self.label = "None"
        self.cpc = None

        ssps = IMP.core.SoftSpherePairScore(self.kappa)
        lsa = IMP.container.ListSingletonContainer(self.m)

        particles = IMP.pmi.tools.select(
            representation,
            resolution=resolution,
            hierarchies=hierarchies)

        lsa.add(IMP.get_indexes(particles))

        if other_hierarchies is None:
            rbcpf = IMP.core.RigidClosePairsFinder()
            self.cpc = IMP.container.ClosePairContainer(lsa, 0.0, rbcpf, 10.0)
            evr = IMP.container.PairsRestraint(ssps, self.cpc)

        else:
            other_lsa = IMP.container.ListSingletonContainer(self.m)
            other_particles = IMP.pmi.tools.select(
                representation,
                resolution=resolution,
                hierarchies=other_hierarchies)
            other_lsa.add_particles(particles)
            self.cpc = IMP.container.CloseBipartitePairContainer(
                lsa,
                other_lsa,
                0.0,
                10.0)
            evr = IMP.container.PairsRestraint(ssps, self.cpc)

        self.rs.add_restraint(evr)

    def add_excluded_particle_pairs(self, excluded_particle_pairs):
        # add pairs to be filtered when calculating  the score
        lpc = IMP.container.ListPairContainer(self.m)
        lpc.add(IMP.get_indexes(excluded_particle_pairs))
        icpf = IMP.container.InContainerPairFilter(lpc)
        self.cpc.add_pair_filter(icpf)

    def set_label(self, label):
        self.label = label

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
        output["ExcludedVolumeSphere_" + self.label] = str(score)
        return output


class ResidueBondRestraint(object):

    '''
    add bond restraint between pair of consecutive
    residues/beads to enforce the stereochemistry.
    '''
    import IMP.pmi.tools
    from math import pi as pi

    def __init__(
        self,
        representation,
        selection_tuple,
        distance=3.78,
        strength=10.0,
            jitter=None):
        '''
        jitter: defines the +- added to the optimal distance in the harmonic well restraint
                used to increase the tolerance
        '''
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

    '''
    add angular restraint between triplets of consecutive
    residues/beads to enforce the stereochemistry.
    '''
    import IMP.pmi.tools
    from math import pi as pi

    def __init__(
        self,
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
            (self.pi * anglemin / 180.0,
             self.pi * anglemax / 180.0),
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

    '''
    Add dihedral restraints between quadruplet of consecutive
    residues/beads to enforce the stereochemistry.
    Give as input a string of "C" and "T", meaning cys (0+-40) or trans (180+-40)
    dihedral. The length of the string must be \#residue-3.
    Without the string, the dihedral will be assumed trans.
    '''
    import IMP.pmi.tools
    from math import pi as pi

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
                    (self.pi * anglemin / 180.0,
                     self.pi * anglemax / 180.0),
                    strength)
                print("ResidueDihedralRestraint: adding a CYS restraint between %s %s %s %s" % (quadruplet[0].get_name(), quadruplet[1].get_name(),
                                                                                                quadruplet[2].get_name(), quadruplet[3].get_name()))
            if dihedraltype == "T":
                anglemin = 180 - 70.0
                anglemax = 180 + 70.0
                ts = IMP.core.HarmonicWell(
                    (self.pi * anglemin / 180.0,
                     self.pi * anglemax / 180.0),
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


#
class SecondaryStructure(object):

    from math import pi
    from math import log

    def __init__(
        self,
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
            phi0.append(i * 10.0 / 180.0 * self.pi)
            phi1.append(i * 10.0 / 180.0 * self.pi)
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
            psi.append(i / 180.0 * self.pi)
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

    '''
    add harmonic restraints between all pairs
    '''
    import IMP.pmi.tools
    from math import pi as pi

    def __init__(self,representation,
                 selection_tuples,
                 resolution=1,
                 strength=0.01,
                 dist_cutoff=10.0,
                 ca_only=True):
        '''
        ca_only: only applies for resolution 0
        '''
        self.m = representation.prot.get_model()
        self.rs = IMP.RestraintSet(self.m, "ElasticNetwork")
        self.weight = 1
        self.label = "None"
        self.pairslist = []

        particles = []
        for st in selection_tuples:
            print('selecting with',st)
            for p in IMP.pmi.tools.select_by_tuple(representation,st,resolution=resolution):
                if (resolution==0 and ca_only and IMP.atom.Atom(p).get_atom_type()!=IMP.atom.AtomType("CA")):
                    continue
                else:
                    particles.append(p.get_particle())


        gcpf = IMP.core.GridClosePairsFinder()
        gcpf.set_distance(dist_cutoff)
        particleindexes=IMP.get_indexes(particles)
        pairs=gcpf.get_close_pairs(   self.m,
                                particleindexes,
                                particleindexes)

        for pair in pairs:
            p1=self.m.get_particle(pair[0])
            p2=self.m.get_particle(pair[1])
            if p1==p2:
                print("%s and %s are the same particle" % (p1.get_name(),p2.get_name()))
                continue
            if(IMP.core.RigidMember.get_is_setup(p1) and
                   IMP.core.RigidMember.get_is_setup(p2) and
                   IMP.core.RigidMember(p1).get_rigid_body() ==
                   IMP.core.RigidMember(p2).get_rigid_body()):
                print("%s and %s are in the same rigid body" % (p1.get_name(),p2.get_name()))
                continue

            distance=IMP.algebra.get_distance(IMP.core.XYZ(p1).get_coordinates(),
                                                IMP.core.XYZ(p2).get_coordinates())

            ts=IMP.core.HarmonicDistancePairScore(distance,strength)
            print("ElasticNetworkConstraint: adding a restraint between %s and %s with distance %.3f" % (p1.get_name(),p2.get_name(),distance))
            self.rs.add_restraint(IMP.core.PairRestraint(self.m, ts, (p1, p2)))
            self.pairslist.append(IMP.ParticlePair(p1, p2))
            self.pairslist.append(IMP.ParticlePair(p1, p2))
        print('created',self.rs.get_number_of_restraints(),'restraints')

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

    '''
    add charmm force field
    '''
    import IMP.pmi.tools
    from math import pi as pi
    import IMP.isd

    def __init__(self,representation,ff_temp=300.0):

        kB = (1.381 * 6.02214) / 4184.0

        self.m=representation.prot.get_model()
        self.bonds_rs = IMP.RestraintSet(self.m, 1.0 / (kB * ff_temp), 'bonds')
        self.nonbonded_rs = IMP.RestraintSet(self.m, 1.0 / (kB * ff_temp), 'NONBONDED')
        self.weight=1
        self.label="None"


        #root=representation.prot.get_children()[0].get_children()[0].get_children()[0]
        root=representation.prot

        ### charmm setup
        ff = IMP.atom.get_heavy_atom_CHARMM_parameters()
        topology = ff.create_topology(root)
        topology.apply_default_patches()
        topology.setup_hierarchy(root)
        r = IMP.atom.CHARMMStereochemistryRestraint(root, topology)
        self.bonds_rs.add_restraint(r)
        ff.add_radii(root)
        ff.add_well_depths(root)
        atoms=IMP.atom.get_leaves(root)

        ### non-bonded forces
        cont = IMP.container.ListSingletonContainer(atoms)
        self.nbl = IMP.container.ClosePairContainer(cont, 4.0)
        self.nbl.add_pair_filter(r.get_pair_filter())
        #sf = IMP.atom.ForceSwitch(6.0, 7.0)
        #pairscore = IMP.atom.LennardJonesPairScore(sf)
        pairscore = IMP.isd.RepulsiveDistancePairScore(0,1)
        pr=IMP.container.PairsRestraint(pairscore, self.nbl)
        self.nonbonded_rs.add_restraint(pr)

        #self.scoring_function = IMP.core.RestraintsScoringFunction([r,pr])

        print('CHARMM is set up')

    def set_label(self, label):
        self.label = label
        self.rs.set_name(label)
        for r in self.rs.get_restraints():
            r.set_name(label)

    def add_to_model(self):
        IMP.pmi.tools.add_restraint_to_model(self.m, self.bonds_rs)
        IMP.pmi.tools.add_restraint_to_model(self.m, self.nonbonded_rs)

    def get_restraint(self):
        return self.rs

    def get_close_pair_container(self):
        return self.nbl

    def set_weight(self, weight):
        self.weight = weight
        self.rs.set_weight(weight)

    def get_output(self):
        self.m.update()
        output = {}
        bonds_score = self.weight * self.bonds_rs.unprotected_evaluate(None)
        nonbonded_score = self.weight * self.nonbonded_rs.unprotected_evaluate(None)
        score=bonds_score+nonbonded_score
        output["_TotalScore"] = str(score)
        output["CHARMM_BONDS"] = str(bonds_score)
        output["CHARMM_NONBONDED"] = str(nonbonded_score)
        return output


class PseudoAtomicRestraint(object):

    '''
    add bonds and improper dihedral restraints for the CBs
    '''
    import IMP.pmi.tools
    from math import pi as pi

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
