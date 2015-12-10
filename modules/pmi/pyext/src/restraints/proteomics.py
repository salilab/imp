"""@namespace IMP.pmi.restraints.proteomics
Restraints for handling various kinds of proteomics data.
"""

from __future__ import print_function
import IMP
import IMP.core
import IMP.algebra
import IMP.atom
import IMP.container
import IMP.pmi.tools

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
        IMP.pmi.tools.add_restraint_to_model(self.m, self.rs)

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

        self.handleparticles = []
        for s in handleparticles_tuples:
            self.handleparticles += IMP.pmi.tools.select_by_tuple(representation, s,
                                                             resolution=resolution, name_is_ambiguous=True)
        self.compositeparticles=[]
        compositeparticle_list = []
        for list in compositeparticles_tuple_list:
            tmplist = []
            for s in list:
                tmplist += IMP.pmi.tools.select_by_tuple(
                    representation, s,
                    resolution=resolution, name_is_ambiguous=True)
            compositeparticle_list.append(tmplist)
            self.compositeparticles+=tmplist


        ln = IMP.pmi.CompositeRestraint(
            self.m,
            self.handleparticles,
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

    def get_handle_particles(self):
        return self.handleparticles

    def get_composite_particles(self):
        return self.compositeparticles

    def get_restraint(self):
        return self.rs

    def add_to_model(self):
        IMP.pmi.tools.add_restraint_to_model(self.m, self.rs)

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
        IMP.pmi.tools.add_restraint_to_model(self.m, self.rs)

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
        IMP.pmi.tools.add_restraint_to_model(self.m, self.rs)

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


class SetupConnectivityNetworkRestraint(object):

    '''
    generates and wraps a IMP.pmi.ConnectivityRestraint between domains
    example:
    cr=restraints.ConnectivityNetworkRestraint(simo,["CCC",(1,100,"TTT"),(100,150,"AAA")])
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
        resolution=1.0,
            label="None"):

        self.weight = 1.0
        self.kappa = kappa
        self.label = label
        if self.label == "None":
            self.label = str(selection_tuples)

        self.m = representation.m
        self.rs = IMP.RestraintSet(self.m, label)

        cr=ConnectivityNetworkRestraint(self.m)

        for s in selection_tuples:
            particles = IMP.pmi.tools.select_by_tuple(representation, s,
                                                      resolution=resolution,
                                                      name_is_ambiguous=False)

            cr.add_particles([p.get_particle() for p in particles])

        self.rs.add_restraint(cr)

    def set_label(self, label):
        self.label = label
        self.rs.set_name(label)
        for r in self.rs.get_restraints():
            r.set_name(label)

    def add_to_model(self):
        IMP.pmi.tools.add_restraint_to_model(self.m, self.rs)

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
        output["ConnectivityNetworkRestraint_" + self.label] = str(score)
        return output



class ConnectivityNetworkRestraint(IMP.Restraint):
    '''
    a python restraint that computes the score for a composite of proteins
    Authors: G. Bouvier, R. Pellarin. Pasteur Institute.
    '''

    import numpy
    import math

    def __init__(self,m,slope=1.0,theta=0.0,plateau=0.0000000001,linear_slope=0.015):
        '''
        input a list of particles, the slope and theta of the sigmoid potential
        theta is the cutoff distance for a protein-protein contact
        '''
        IMP.Restraint.__init__(self, m, "ConnectivityNetworkRestraint %1%")
        self.slope=slope
        self.theta=theta
        self.linear_slope=linear_slope
        self.plateau=plateau
        self.particles_blocks=[]
        self.particle_list=[]

    def get_number_of_particle_blocks(self):
        return len(self.particles_blocks)

    def get_number_of_particles_for_block(self,block_index):
        return len(self.particles_blocks[block_index])

    def add_particles(self,particles):
        self.particles_blocks.append(particles)
        self.particle_list+=particles

    def get_full_graph(self):
        '''
        get the full graph of distances between every particle pair
        '''
        import networkx
        import scipy.spatial

        pdist_array = self.numpy.array(IMP.pmi.get_list_of_bipartite_minimum_sphere_distance(self.particles_blocks))
        pdist_mat=scipy.spatial.distance.squareform(pdist_array)
        pdist_mat[pdist_mat < 0] = 0
        graph = networkx.Graph(pdist_mat)
        return graph

    def get_minimum_spanning_tree(self):
        """
        return the minimum spanning tree
        """
        import networkx
        graph = self.get_full_graph()
        graph = networkx.minimum_spanning_tree(graph)
        return graph

    def sigmoid(self,x):
        '''
        a sigmoid function that scores the probability of a contact
        between two proteins
        '''
        #return 1 - (x)**self.slope/ float(((x)**self.slope + self.theta**self.slope))
        argvalue=(x-self.theta)/self.slope
        return 1.0-(1.0-self.plateau)/(1.0+self.math.exp(-argvalue))

    def unprotected_evaluate(self,da):
        graph = self.get_minimum_spanning_tree()
        score = 0.0
        for e in graph.edges():
            dist=graph.get_edge_data(*e)['weight']
            prob=self.sigmoid(dist)
            score+=-self.numpy.log(prob)
            score+=self.linear_slope*dist
        return score

    def do_get_inputs(self):
        return self.particle_list


class MembraneRestraint(IMP.Restraint):
    import math
    import numpy
    import IMP.isd
    import sys
    def __init__(self,m,z_nuisance,thickness=30.0,softness=3.0,plateau=0.0000000001,linear=0.02):
        '''
        input a list of particles, the slope and theta of the sigmoid potential
        theta is the cutoff distance for a protein-protein contact
        '''
        IMP.Restraint.__init__(self, m, "MembraneRestraint %1%")
        self.thickness=thickness
        self.z_nuisance=z_nuisance
        self.softness=softness
        self.plateau=plateau
        self.particle_list_below=[]
        self.particle_list_above=[]
        self.particle_list_inside=[]
        self.max_float=self.sys.float_info.max
        self.log_max_float=self.numpy.log(self.max_float)
        self.linear=linear

    def add_particles_below(self,particles):
        self.particle_list_below+=particles

    def add_particles_above(self,particles):
        self.particle_list_above+=particles

    def add_particles_inside(self,particles):
        self.particle_list_inside+=particles

    def score_above(self,z):

        argvalue=(z-self.z_slope_center_upper)/self.softness
        prob=(1.0-self.plateau)/(1.0+self.math.exp(-argvalue))
        return -self.numpy.log(prob*self.max_float)+self.log_max_float

    def score_below(self,z):
        argvalue=(z-self.z_slope_center_lower)/self.softness
        prob=(1.0-self.plateau)/(1.0+self.math.exp(argvalue))
        return -self.numpy.log(prob*self.max_float)+self.log_max_float

    def score_inside(self,z):

        argvalue=(z-self.z_slope_center_upper)/self.softness
        prob1=1.0-(1.0-self.plateau)/(1.0+self.math.exp(-argvalue))

        argvalue=(z-self.z_slope_center_lower)/self.softness
        prob2=1.0-(1.0-self.plateau)/(1.0+self.math.exp(argvalue))
        return -self.numpy.log(prob1*self.max_float)-self.numpy.log(prob2*self.max_float)+2*self.log_max_float

    def unprotected_evaluate(self,da):

        z_center=self.IMP.isd.Nuisance(self.z_nuisance).get_nuisance()
        self.z_slope_center_lower=z_center-self.thickness/2.0
        self.z_slope_center_upper=z_center+self.thickness/2.0

        score_above=sum([self.score_above(IMP.core.XYZ(p).get_z()) for p in self.particle_list_above])
        score_below=sum([self.score_below(IMP.core.XYZ(p).get_z()) for p in self.particle_list_below])
        score_inside=sum([self.score_inside(IMP.core.XYZ(p).get_z()) for p in self.particle_list_inside])

        return score_above+score_below+score_inside

    def do_get_inputs(self):
        particle_list=self.particle_list_above+self.particle_list_inside+self.particle_list_below

        return particle_list



class SetupMembranePoreRestraint(object):
    import math

    def __init__(
        self,
        representation,
        selection_tuples_outside=None,
        selection_tuples_membrane=None,
        selection_tuples_inside=None,
        center=(0.0,0.0,0.0),
        z_tickness=100,
        radius=100,
        membrane_tickness=40.0,
        resolution=1,
        label="None"):

        self.weight = 1.0
        self.label = label
        self.m = representation.prot.get_model()
        self.rs = IMP.RestraintSet(self.m, label)
        self.representation=representation
        self.center=center
        self.radius=radius
        self.z_tickness=z_tickness
        self.membrane_tickness=membrane_tickness





    def create_representation(self):
        p=IMP.Particle(self.m)
        h=IMP.atom.Hierarchy.setup_particle(p)
        h.set_name("Membrane_Pore"+self.label)
        self.representation.prot.add_child(h)

        inner=self.z_tickness/2-self.membrane_tickness/2
        outer=self.z_tickness/2+self.membrane_tickness/2

        surface_1=[]
        surface_2=[]
        particles_surface_1=[]
        particles_surface_2=[]

        for i in range(10):
            for j in range(10):
                v=self.math.pi/10.0*float(i)+self.math.pi/2.0
                u=2.0*self.math.pi/10.0*float(j)
                x=(self.radius+inner*self.math.cos(v))*self.math.cos(u)
                y=(self.radius+inner*self.math.cos(v))*self.math.sin(u)
                z=inner*self.math.sin(v)
                p2=IMP.Particle(self.m)
                IMP.atom.Mass.setup_particle(p2,100)
                d=IMP.core.XYZR.setup_particle(p2)
                d.set_coordinates((x,y,z))
                d.set_radius(1)
                h2=IMP.atom.Hierarchy.setup_particle(p2)
                h.add_child(h2)
                surface_1.append((x,y,z))
                particles_surface_1.append(p2)

                x=(self.radius+outer*self.math.cos(v))*self.math.cos(u)
                y=(self.radius+outer*self.math.cos(v))*self.math.sin(u)
                z=outer*self.math.sin(v)
                p2=IMP.Particle(self.m)
                IMP.atom.Mass.setup_particle(p2,100)
                d=IMP.core.XYZR.setup_particle(p2)
                d.set_coordinates((x,y,z))
                d.set_radius(1)
                h2=IMP.atom.Hierarchy.setup_particle(p2)
                h.add_child(h2)
                surface_2.append((x,y,z))
                particles_surface_2.append(p2)

        for i in range(10):
            for j in range(10):
                x=3.0*self.radius/10.0*float(i)-self.radius*1.5
                y=3.0*self.radius/10.0*float(j)-self.radius*1.5
                if(self.math.sqrt(x**2+y**2)<self.radius):
                    continue
                else:
                    for n,z in enumerate([self.z_tickness/2+self.membrane_tickness/2,
                              self.z_tickness/2-self.membrane_tickness/2,
                              -self.z_tickness/2+self.membrane_tickness/2,
                              -self.z_tickness/2-self.membrane_tickness/2]):
                        p2=IMP.Particle(self.m)
                        IMP.atom.Mass.setup_particle(p2,100)
                        d=IMP.core.XYZR.setup_particle(p2)
                        d.set_coordinates((x,y,z))
                        d.set_radius(1)
                        h2=IMP.atom.Hierarchy.setup_particle(p2)
                        h.add_child(h2)
                        if n == 0 or n == 3:
                            surface_1.append((x,y,z))
                            particles_surface_1.append(p2)
                        if n == 1 or n == 2:
                            surface_2.append((x,y,z))
                            particles_surface_2.append(p2)

            from scipy.spatial import Delaunay
            tri = Delaunay(surface_1)

            '''
            edge_points = []
            edges = set()

            def add_edge(i, j):
                """Add a line between the i-th and j-th points, if not in the list already"""
                if (i, j) in edges or (j, i) in edges:
                    # already added
                    return
                edges.add( (i, j) )
                edge_points.append(points[ [i, j] ])

            # loop over triangles:
            # ia, ib, ic = indices of corner points of the triangle
            for ia, ib, ic in tri.vertices:
                add_edge(ia, ib)
                add_edge(ib, ic)
                add_edge(ic, ia)
            '''

            '''
            edges=set()

            for ia, ib, ic, id in tri.simplices:
                edges.add((ia,ib))
                edges.add((ib,ic))
                edges.add((ic,id))

            for e in edges:
                p1=particles_surface_1[e[0]]
                p2=particles_surface_1[e[1]]
                print(p1,p2,e[0],e[1])
                IMP.atom.Bonded.setup_particle(p1)
                IMP.atom.Bonded.setup_particle(p2)
                IMP.atom.create_bond(IMP.atom.Bonded(p1),IMP.atom.Bonded(p2),1)
            '''

            for i in range(len(particles_surface_1)-1):
                p1=particles_surface_1[i]
                p2=particles_surface_1[i+1]
                IMP.atom.Bonded.setup_particle(p1)
                IMP.atom.Bonded.setup_particle(p2)
                IMP.atom.create_bond(IMP.atom.Bonded(p1),IMP.atom.Bonded(p2),1)


class SetupMembraneRestraint(object):

    '''

    '''

    def __init__(
        self,
        representation,
        selection_tuples_above=None,
        selection_tuples_inside=None,
        selection_tuples_below=None,
        z_init=0.0,
        z_min=0.0,
        z_max=0.0,
        thickness=30,
        resolution=1,
        label="None"):

        self.weight = 1.0
        self.label = label
        self.m = representation.prot.get_model()
        self.rs = IMP.RestraintSet(self.m, label)
        self.representation=representation
        self.thickness=thickness

        p=IMP.Particle(self.m)


        self.z_center=IMP.pmi.tools.SetupNuisance(self.m, z_init, z_min, z_max, isoptimized=True).get_particle()

        mr = IMP.pmi.restraints.proteomics.MembraneRestraint(self.m,self.z_center,thickness=self.thickness)

        if selection_tuples_above is not None:
            particles_above = []
            for s in selection_tuples_above:
                particles = IMP.pmi.tools.select_by_tuple(self.representation, s,
                                                          resolution=resolution, name_is_ambiguous=True)
                particles_above+=particles
            mr.add_particles_above([h.get_particle() for h in particles_above])

        if selection_tuples_below is not None:
            particles_below = []
            for s in selection_tuples_below:
                particles = IMP.pmi.tools.select_by_tuple(self.representation, s,
                                                          resolution=resolution, name_is_ambiguous=True)
                particles_below+=particles
            mr.add_particles_below([h.get_particle() for h in particles_below])

        if selection_tuples_inside is not None:
            particles_inside = []
            for s in selection_tuples_inside:
                particles = IMP.pmi.tools.select_by_tuple(self.representation, s,
                                                          resolution=resolution, name_is_ambiguous=True)
                particles_inside+=particles
            mr.add_particles_inside([h.get_particle() for h in particles_inside])

        self.rs.add_restraint(mr)

    def create_box(self, x_center, y_center):

        z=self.z_center.get_nuisance()
        p=IMP.Particle(self.m)
        h=IMP.atom.Hierarchy.setup_particle(p)
        h.set_name("Membrane_"+self.label)
        self.representation.prot.add_child(h)

        particles_box=[]
        p_origin=IMP.Particle(self.m)

        IMP.atom.Mass.setup_particle(p_origin,100)
        d=IMP.core.XYZR.setup_particle(p_origin)
        d.set_coordinates((x_center,y_center,0))
        d.set_radius(1)
        h_origin=IMP.atom.Hierarchy.setup_particle(p_origin)
        h.add_child(h_origin)
        particles_box.append(p_origin)

        p1=IMP.Particle(self.m)
        IMP.atom.Mass.setup_particle(p1,100)
        d=IMP.core.XYZR.setup_particle(p1)
        d.set_coordinates((x_center,y_center,z+100))
        d.set_radius(1)
        h1=IMP.atom.Hierarchy.setup_particle(p1)
        h.add_child(h1)
        particles_box.append(p1)

        p2=IMP.Particle(self.m)
        IMP.atom.Mass.setup_particle(p2,100)
        d=IMP.core.XYZR.setup_particle(p2)
        d.set_coordinates((x_center,y_center,z-100))
        d.set_radius(1)
        h2=IMP.atom.Hierarchy.setup_particle(p2)
        h.add_child(h2)
        particles_box.append(p2)

        IMP.atom.Bonded.setup_particle(p1)
        IMP.atom.Bonded.setup_particle(p2)

        IMP.atom.create_bond(IMP.atom.Bonded(p1),IMP.atom.Bonded(p2),1)

        p1.set_name("z_positive")
        p2.set_name("z_negative")

        # to display the membrane in the rmf file
        for offs in [self.thickness/2,-self.thickness/2]:
            p1=IMP.Particle(self.m)
            IMP.atom.Mass.setup_particle(p1,100)
            d=IMP.core.XYZR.setup_particle(p1)
            d.set_coordinates((-100+x_center,-100+y_center,z+offs))
            d.set_radius(1)
            h1=IMP.atom.Hierarchy.setup_particle(p1)
            h.add_child(h1)
            particles_box.append(p1)

            p2=IMP.Particle(self.m)
            IMP.atom.Mass.setup_particle(p2,100)
            d=IMP.core.XYZR.setup_particle(p2)
            d.set_coordinates((-100+x_center,100+y_center,z+offs))
            d.set_radius(1)
            h2=IMP.atom.Hierarchy.setup_particle(p2)
            h.add_child(h2)
            particles_box.append(p2)

            p3=IMP.Particle(self.m)
            IMP.atom.Mass.setup_particle(p3,100)
            d=IMP.core.XYZR.setup_particle(p3)
            d.set_coordinates((100+x_center,-100+y_center,z+offs))
            d.set_radius(1)
            h3=IMP.atom.Hierarchy.setup_particle(p3)
            h.add_child(h3)
            particles_box.append(p3)

            p4=IMP.Particle(self.m)
            IMP.atom.Mass.setup_particle(p4,100)
            d=IMP.core.XYZR.setup_particle(p4)
            d.set_coordinates((100+x_center,100+y_center,z+offs))
            d.set_radius(1)
            h4=IMP.atom.Hierarchy.setup_particle(p4)
            h.add_child(h4)
            particles_box.append(p4)

            IMP.atom.Bonded.setup_particle(p1)
            IMP.atom.Bonded.setup_particle(p2)
            IMP.atom.Bonded.setup_particle(p3)
            IMP.atom.Bonded.setup_particle(p4)

            IMP.atom.create_bond(IMP.atom.Bonded(p1),IMP.atom.Bonded(p2),1)
            IMP.atom.create_bond(IMP.atom.Bonded(p1),IMP.atom.Bonded(p3),1)
            IMP.atom.create_bond(IMP.atom.Bonded(p4),IMP.atom.Bonded(p2),1)
            IMP.atom.create_bond(IMP.atom.Bonded(p4),IMP.atom.Bonded(p3),1)

        sm = self._MembraneSingletonModifier(p_origin,self.z_center)
        lc = IMP.container.ListSingletonContainer(self.m)
        for p in particles_box:
            IMP.core.XYZ(p).set_coordinates_are_optimized(True)
            lc.add(p.get_index())
            print(p)
        c = IMP.container.SingletonsConstraint(sm, None, lc)
        self.m.add_score_state(c)
        self.m.update()

    class _MembraneSingletonModifier(IMP.SingletonModifier):
        """A class that updates the membrane particles
        """

        def __init__(self, p_origin,z_nuisance):
            IMP.SingletonModifier.__init__(self, "MembraneSingletonModifier%1%")
            self.p_origin_index=p_origin.get_index()
            self.z_nuisance=z_nuisance

        def apply_index(self, m, pi):
            z_center=IMP.core.XYZ(m,self.p_origin_index).get_z()
            new_z=self.z_nuisance.get_nuisance()
            d = IMP.core.XYZ(m, pi)
            current_z=d.get_z()
            d.set_z(current_z+new_z-z_center)

        def do_get_inputs(self, m, pis):
            return IMP.get_particles(m, pis)

        def do_get_outputs(self, m, pis):
            return self.do_get_inputs(m, pis)

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

    def get_particles_to_sample(self):
        ps = {}

        ps["Nuisances_MembraneRestraint_Z_" + self.label] =([self.z_center],2.0)
        return ps

    def get_output(self):
        self.m.update()
        output = {}
        score = self.weight * self.rs.unprotected_evaluate(None)
        output["_TotalScore"] = str(score)
        output["MembraneRestraint_" + self.label] = str(score)
        output["MembraneRestraint_Z_"+ self.label] = str(self.z_center.get_nuisance())
        return output



class FuzzyBoolean(object):
    '''
    Fully Ambiguous Restraint that can be built using boolean logic
    R. Pellarin. Pasteur Institute.
    '''

    def __init__(self,p1,operator=None,p2=None):
        '''
        input a list of particles, the slope and theta of the sigmoid potential
        theta is the cutoff distance for a protein-protein contact
        '''
        if isinstance(p1, FuzzyBoolean) and isinstance(p2, FuzzyBoolean):
            self.operations = [p1, operator, p2]
            self.value = None
        else:
            self.operations = []
            self.value = p1

    def __or__(self, FuzzyBoolean2):
        return FuzzyBoolean(self, self.or_, FuzzyBoolean2)

    def __and__(self, FuzzyBoolean2):
        return FuzzyBoolean(self, self.and_, FuzzyBoolean2)

    def and_(self,a,b):
        return a*b

    def or_(self,a,b):
        return 1.0-(1.0-a)*(1.0-b)

    def evaluate(self):

        if len(self.operations) == 0:
            return self.value
        FuzzyBoolean1, op, FuzzyBoolean2 = self.operations

        return op(FuzzyBoolean1.evaluate(),FuzzyBoolean2.evaluate())

class FuzzyRestraint(IMP.Restraint):
    '''
    Fully Ambiguous Restraint that can be built using boolean logic
    R. Pellarin. Pasteur Institute.
    '''
    import math
    plateau=0.00000000000001
    theta=5.0
    slope=2.0
    innerslope=0.01
    import sys

    def __init__(self,m,p1,p2,operator=None):
        '''
        input a list of particles, the slope and theta of the sigmoid potential
        theta is the cutoff distance for a protein-protein contact
        '''
        IMP.Restraint.__init__(self, m, "FuzzyRestraint %1%")
        self.m=m
        self.min=self.sys.float_info.min
        if isinstance(p1, FuzzyRestraint) and isinstance(p2, FuzzyRestraint):
            self.operations = [p1, operator, p2]
            self.particle_pair = None
        elif isinstance(p1, FuzzyRestraint) and p2 is None:
            self.operations = [p1, operator, None]
            self.particle_pair = None
        else:
            self.operations = []
            self.particle_pair = (p1,p2)

    def __or__(self, FuzzyRestraint2):
        return FuzzyRestraint(self.m, self, FuzzyRestraint2, self.or_)

    def __and__(self, FuzzyRestraint2):
        return FuzzyRestraint(self.m, self, FuzzyRestraint2, self.and_)

    def __invert__(self):
        return FuzzyRestraint(self.m, self, None, self.invert_)

    def and_(self,a,b):
        c=a+b
        return c

    def or_(self,a,b):
        c=self.math.exp(-a)+self.math.exp(-b)-self.math.exp(-a-b)
        return -self.math.log(c)

    def invert_(self,a):
        c=1.0-self.math.exp(-a)
        return -self.math.log(c)

    def evaluate(self):
        if len(self.operations) == 0:
            return self.distance()
        FuzzyRestraint1, op, FuzzyRestraint2 = self.operations

        if FuzzyRestraint2 is not None:
            return op(FuzzyRestraint1.evaluate(),FuzzyRestraint2.evaluate())
        else:
            return op(FuzzyRestraint1.evaluate())

    def distance(self):
        d1=IMP.core.XYZ(self.particle_pair[0])
        d2=IMP.core.XYZ(self.particle_pair[1])
        d=IMP.core.get_distance(d1,d2)
        argvalue=(d-self.theta)/self.slope
        return -self.math.log(1.0-(1.0-self.plateau)/(1.0+self.math.exp(-argvalue)))+self.innerslope*d

    def add_to_model(self):
        IMP.pmi.tools.add_restraint_to_model(self.m, self)

    def unprotected_evaluate(self,da):
        return self.evaluate()

    def __str__(self):
        if len(self.operations) == 0:
            return str(self.particle_pair)
        FuzzyRestraint1, op, FuzzyRestraint2 = self.operations
        if FuzzyRestraint2 is not None:
            return str(FuzzyRestraint1)+str(op)+str(FuzzyRestraint2)
        else:
            return str(FuzzyRestraint1)+str(op)

    def do_get_inputs(self):
        if len(self.operations) == 0:
            return list(self.particle_pair)
        FuzzyRestraint1, op, FuzzyRestraint2 = self.operations
        if FuzzyRestraint2 is not None:
            return list(set(FuzzyRestraint1.do_get_inputs()+FuzzyRestraint2.do_get_inputs()))
        else:
            return list(set(FuzzyRestraint1.do_get_inputs()))
