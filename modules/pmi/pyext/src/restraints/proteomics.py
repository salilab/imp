"""@namespace IMP.pmi.restraints.proteomics
Restraints for handling various kinds of proteomics data.
"""

from __future__ import print_function
import IMP
import IMP.core
import IMP.base
import IMP.algebra
import IMP.atom
import IMP.container


class ConnectivityRestraint(object):

    '''
    generate a connectivity restraint between domains
    setting up the restraint
    example:
    cr=restraints.ConnectivityRestraint(simo,["CCC",(1,100,"TTT"),(100,150,"AAA")])
    cr.add_to_model()
    cr.set_label("CR1")


    Multistate support =No
    Selection type=selection tuple
    Resolution=Yes
    '''

    def __init__(
        self,
        representation,
        selection_tuples,
        kappa=10.0,
        resolution=None,
            label="None"):

        self.weight = 1.0
        self.kappa = kappa
        self.label = label
        if self.label == "None":
            self.label = str(selection_tuples)
        self.m = representation.prot.get_model()
        self.rs = IMP.RestraintSet(self.m, label)

        sels = []

        for s in selection_tuples:
            particles = IMP.pmi.tools.select_by_tuple(representation, s,
                                                      resolution=resolution, name_is_ambiguous=True)
            sel = IMP.atom.Selection(particles)
            sels.append(sel)

        cr = IMP.atom.create_connectivity_restraint(
            sels,
            self.kappa,
            self.label)
        self.rs.add_restraint(cr)

    def set_label(self, label):
        self.label = label
        self.rs.set_name(label)
        for r in self.rs.get_restraints():
            r.set_name(label)

    def add_to_model(self):
        self.m.add_restraint(self.rs)

    def get_restraint(self):
        return self.rs

    def get_restraints(self):
        rlist = []
        for r in self.rs.get_restraints():
            rlist.append(IMP.core.PairRestraint.get_from(r))
        return rlist

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


#
class CompositeRestraint(object):

    '''
    handleparticles is a selection tuple
    compositeparticles is a list of selection tuples
    '''
    import IMP.pmi

    def __init__(
        self,
        representation,
        handleparticles_tuples,
        compositeparticles_tuple_list,
        cut_off=5.0,
        lam=1.0,
        plateau=0.0,
        resolution=None,
            label="None"):

        # composite particles: all particles beside the handle
        self.label = label
        self.m = representation.prot.get_model()
        self.rs = IMP.RestraintSet(self.m, 'cr')

        handleparticles = []
        for s in handleparticles_tuples:
            handleparticles += IMP.pmi.tools.select_by_tuple(representation, s,
                                                             resolution=resolution, name_is_ambiguous=True)

        compositeparticle_list = []
        for list in compositeparticles_tuple_list:
            compositeparticles = []
            for s in list:
                compositeparticles += IMP.pmi.tools.select_by_tuple(
                    representation, s,
                    resolution=resolution, name_is_ambiguous=True)
            compositeparticle_list.append(compositeparticles)

        ln = IMP.pmi.CompositeRestraint(
            self.m,
            handleparticles,
            cut_off,
            lam,
            True,
            plateau)
        for ps in compositeparticle_list:
            # composite particles is a list of list of particles
            ln.add_composite_particle(ps)

        self.rs.add_restraint(ln)

    def set_label(self, label):
        self.label = label

    def add_to_model(self):
        self.m.add_restraint(self.rs)

    def get_output(self):
        self.m.update()
        output = {}
        score = self.rs.unprotected_evaluate(None)
        output["_TotalScore"] = str(score)
        output["CompositeRestraint_" + self.label] = str(score)
        return output


#
class AmbiguousCompositeRestraint(object):

    '''
    this restraint allows ambiguous crosslinking between multiple copies
    excluding between symmetric copies
    It allows name ambiguity
    '''

    def __init__(
        self,
        representation,
        restraints_file,
        cut_off=5.0,
        lam=1.0,
        plateau=0.01,
        resolution=None,
            label="None"):

        self.weight = 1.0
        self.m = representation.prot.get_model()
        self.rs = IMP.RestraintSet(self.m, 'data')
        self.label = "None"
        self.pairs = []

        self.outputlevel = "low"
        self.cut_off = cut_off
        self.lam = lam
        self.plateau = plateau

        fl = IMP.pmi.tools.open_file_or_inline_text(restraints_file)

        for line in fl:

            tokens = line.split()
            # skip character
            if (tokens[0] == "#"):
                continue
            r1 = int(tokens[2])
            c1 = tokens[0]
            r2 = int(tokens[3])
            c2 = tokens[1]

            ps1 = IMP.pmi.tools.select(
                representation,
                resolution=resolution,
                name=c1,
                name_is_ambiguous=True,
                residue=r1)
            hrc1 = [representation.hier_db.particle_to_name[p] for p in ps1]
            ps1nosym = [
                p for p in ps1 if IMP.pmi.Symmetric(
                    p).get_symmetric(
                ) == 0]
            hrc1nosym = [representation.hier_db.particle_to_name[p]
                         for p in ps1nosym]

            if len(ps1) == 0:
                print("AmbiguousCompositeRestraint: WARNING> residue %d of chain %s is not there" % (r1, c1))
                continue

            ps2 = IMP.pmi.tools.select(
                representation,
                resolution=resolution,
                name=c2,
                name_is_ambiguous=True,
                residue=r2)
            hrc2 = [representation.hier_db.particle_to_name[p] for p in ps2]
            ps2nosym = [
                p for p in ps2 if IMP.pmi.Symmetric(
                    p).get_symmetric(
                ) == 0]
            hrc2nosym = [representation.hier_db.particle_to_name[p]
                         for p in ps2nosym]

            if len(ps2) == 0:
                print("AmbiguousCompositeRestraint: WARNING> residue %d of chain %s is not there" % (r2, c2))
                continue

            cr = IMP.pmi.CompositeRestraint(
                self.m,
                ps1nosym,
                self.cut_off,
                self.lam,
                True,
                self.plateau)
            cr.add_composite_particle(ps2)

            self.rs.add_restraint(cr)
            self.pairs.append(
                (ps1nosym,
                 hrc1nosym,
                 c1,
                 r1,
                 ps2,
                 hrc2,
                 c2,
                 r2,
                 cr))

            cr = IMP.pmi.CompositeRestraint(
                self.m,
                ps1,
                self.cut_off,
                self.lam,
                True,
                self.plateau)
            cr.add_composite_particle(ps2nosym)

            self.rs.add_restraint(cr)
            self.pairs.append(
                (ps1,
                 hrc1,
                 c1,
                 r1,
                 ps2nosym,
                 hrc2nosym,
                 c2,
                 r2,
                 cr))

    def plot_restraint(
        self,
        maxdist=100,
            npoints=100):
        import IMP.pmi.output as output

        p1 = IMP.Particle(self.m)
        p2 = IMP.Particle(self.m)
        d1 = IMP.core.XYZR.setup_particle(p1)
        d2 = IMP.core.XYZR.setup_particle(p2)
        cr = IMP.pmi.CompositeRestraint(
            self.m,
            [p1],
            self.cut_off,
            self.lam,
            True,
            self.plateau)
        cr.add_composite_particle([p2])
        dists = []
        scores = []
        for i in range(npoints):
            d2.set_coordinates(
                IMP.algebra.Vector3D(maxdist / npoints * float(i), 0, 0))
            dists.append(IMP.core.get_distance(d1, d2))
            scores.append(cr.unprotected_evaluate(None))
        output.plot_xy_data(dists, scores)

    def set_label(self, label):
        self.label = label
        self.rs.set_name(label)
        for r in self.rs.get_restraints():
            r.set_name(label)

    def add_to_model(self):
        self.m.add_restraint(self.rs)

    def get_hierarchies(self):
        return self.prot

    def get_restraint_sets(self):
        return self.rs

    def get_restraint(self):
        return self.rs

    def set_output_level(self, level="low"):
            # this might be "low" or "high"
        self.outputlevel = level

    def set_weight(self, weight):
        self.weight = weight
        self.rs.set_weight(weight)

    def get_output(self):
        # content of the crosslink database pairs
        # self.pairs.append((p1,p2,dr,r1,c1,r2,c2))
        self.m.update()

        output = {}
        score = self.weight * self.rs.unprotected_evaluate(None)
        output["_TotalScore"] = str(score)
        output["AmbiguousCompositeRestraint_Score_" + self.label] = str(score)
        for n, p in enumerate(self.pairs):

            ps1 = p[0]
            hrc1 = p[1]
            c1 = p[2]
            r1 = p[3]
            ps2 = p[4]
            hrc2 = p[5]
            c2 = p[6]
            r2 = p[7]
            cr = p[8]
            for n1, p1 in enumerate(ps1):
                name1 = hrc1[n1]

                for n2, p2 in enumerate(ps2):
                    name2 = hrc2[n2]
                    d1 = IMP.core.XYZR(p1)
                    d2 = IMP.core.XYZR(p2)
                    label = str(r1) + ":" + name1 + "_" + str(r2) + ":" + name2
                    output["AmbiguousCompositeRestraint_Distance_" +
                           label] = str(IMP.core.get_distance(d1, d2))

            label = str(r1) + ":" + c1 + "_" + str(r2) + ":" + c2
            output["AmbiguousCompositeRestraint_Score_" +
                   label] = str(self.weight * cr.unprotected_evaluate(None))

        return output


#
class SimplifiedPEMAP(object):

    def __init__(
        self,
        representation,
        restraints_file,
        expdistance,
        strength,
            resolution=None):

        self.m = representation.prot.get_model()
        self.rs = IMP.RestraintSet(self.m, 'data')
        self.label = "None"
        self.pairs = []

        self.outputlevel = "low"
        self.expdistance = expdistance
        self.strength = strength

        fl = IMP.pmi.tools.open_file_or_inline_text(restraints_file)

        for line in fl:

            tokens = line.split()
            # skip character
            if (tokens[0] == "#"):
                continue
            r1 = int(tokens[2])
            c1 = tokens[0]
            r2 = int(tokens[3])
            c2 = tokens[1]
            pcc = float(tokens[4])

            ps1 = IMP.pmi.tools.select(
                representation,
                resolution=resolution,
                name=c1,
                name_is_ambiguous=False,
                residue=r1)
            if len(ps1) == 0:
                print("SimplifiedPEMAP: WARNING> residue %d of chain %s is not there (w/ %d %s)" % (r1, c1, r2, c2))
                continue
            if len(ps1) > 1:
                print("SimplifiedPEMAP: WARNING> residue %d of chain %s selected multiple particles" % (r1, c1))
                continue

            ps2 = IMP.pmi.tools.select(
                representation,
                resolution=resolution,
                name=c2,
                name_is_ambiguous=False,
                residue=r2)
            if len(ps2) == 0:
                print("SimplifiedPEMAP: WARNING> residue %d of chain %s is not there (w/ %d %s)" % (r1, c1, r2, c2))
                continue
            if len(ps2) > 1:
                print("SimplifiedPEMAP: WARNING> residue %d of chain %s selected multiple particles" % (r2, c2))
                continue

            p1 = ps1[0]
            p2 = ps2[0]

            # This is harmonic potential for the pE-MAP data
            upperdist = self.get_upper_bond(pcc)
            limit = self.strength * (upperdist + 15) ** 2 + 10.0
            hub = IMP.core.TruncatedHarmonicUpperBound(
                upperdist,
                self.strength,
                upperdist +
                15,
                limit)

            # This is harmonic for the X-link
            #hub= IMP.core.TruncatedHarmonicBound(17.0,self.strength,upperdist+15,limit)

            df = IMP.core.SphereDistancePairScore(hub)
            dr = IMP.core.PairRestraint(df, (p1, p2))
            self.rs.add_restraint(dr)
            self.pairs.append((p1, p2, dr, r1, c1, r2, c2))

            # Lower-bound restraint
            lowerdist = self.get_lower_bond(pcc)
            limit = self.strength * (lowerdist - 15) ** 2 + 10.0
            hub2 = IMP.core.TruncatedHarmonicLowerBound(
                lowerdist,
                self.strength,
                lowerdist +
                15,
                limit)

            # This is harmonic for the X-link
            #hub2= IMP.core.TruncatedHarmonicBound(17.0,self.strength,upperdist+15,limit)

            df2 = IMP.core.SphereDistancePairScore(hub2)
            dr2 = IMP.core.PairRestraint(df2, (p1, p2))
            self.rs.add_restraint(dr2)
            self.pairs.append((p1, p2, dr2, r1, c1, r2, c2))

    def get_upper_bond(self, pearsoncc):
        # return (pearsoncc-1.)/-0.0075
        return (pearsoncc - .5) / (-0.005415)

    def get_lower_bond(self, pearsoncc):
        return (pearsoncc - 1.) / -0.0551

    def set_label(self, label):
        self.label = label

    def add_to_model(self):
        self.m.add_restraint(self.rs)

    def get_hierarchies(self):
        return self.prot

    def get_restraint_sets(self):
        return self.rs

    def set_output_level(self, level="low"):
            # this might be "low" or "high"
        self.outputlevel = level

    def get_output(self):
        # content of the crosslink database pairs
        # self.pairs.append((p1,p2,dr,r1,c1,r2,c2))
        self.m.update()

        output = {}
        score = self.rs.unprotected_evaluate(None)
        output["_TotalScore"] = str(score)
        output["SimplifiedPEMAP_Score_" + self.label] = str(score)
        for i in range(len(self.pairs)):

            p0 = self.pairs[i][0]
            p1 = self.pairs[i][1]
            crosslinker = 'standard'
            ln = self.pairs[i][2]
            resid1 = self.pairs[i][3]
            chain1 = self.pairs[i][4]
            resid2 = self.pairs[i][5]
            chain2 = self.pairs[i][6]

            label = str(resid1) + ":" + chain1 + "_" + \
                str(resid2) + ":" + chain2
            output["SimplifiedPEMAP_Score_" + crosslinker + "_" +
                   label] = str(ln.unprotected_evaluate(None))

            d0 = IMP.core.XYZ(p0)
            d1 = IMP.core.XYZ(p1)
            output["SimplifiedPEMAP_Distance_" +
                   label] = str(IMP.core.get_distance(d0, d1))

        return output
