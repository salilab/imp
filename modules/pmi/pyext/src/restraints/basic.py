"""@namespace IMP.pmi.restraints.basic
Some miscellaneous simple restraints.
"""

from __future__ import print_function
import IMP
import IMP.core
import IMP.algebra
import IMP.atom
import IMP.container
import IMP.pmi.tools

class ExternalBarrier(object):

    def __init__(self,
                 representation=None,
                 radius=10.0,
                 hierarchies=None,
                 resolution=10,
                 weight=1.0,
                 center=None):
        """Setup external barrier to keep all your structures inside sphere
        @param representation DEPRECATED
        @param center - Center of the external barrier restraint (IMP.algebra.Vector3D object)
        @param radius Size of external barrier
        @param hierarchies Can be one of the following inputs:
               IMP Hierarchy, PMI System/State/Molecule/TempResidue, or a list/set of them
        @param resolution Select which resolutions to act upon
        """
        self.radius = radius
        self.label = "None"
        self.weight = weight

        if representation:
            self.m = representation.prot.get_model()
            particles = IMP.pmi.tools.select(
                representation,
                resolution=resolution,
                hierarchies=hierarchies)
        elif hierarchies:
            hiers = IMP.pmi.tools.input_adaptor(hierarchies,resolution,flatten=True)
            self.m = hiers[0].get_model()
            particles = [h.get_particle() for h in hiers]
        else:
            raise Exception("ExternalBarrier: must pass representation or hierarchies")

        self.rs = IMP.RestraintSet(self.m, 'barrier')

        if center is None:
            c3 = IMP.algebra.Vector3D(0, 0, 0)
        elif type(center) is IMP.algebra.Vector3D:
            c3 = center
        else:
            raise Exception("ExternalBarrier: @param center must be an algebra::Vector3D object")

        ub3 = IMP.core.HarmonicUpperBound(radius, 10.0)
        ss3 = IMP.core.DistanceToSingletonScore(ub3, c3)
        lsc = IMP.container.ListSingletonContainer(self.m)

        lsc.add(particles)
        r3 = IMP.container.SingletonsRestraint(ss3, lsc)
        self.rs.add_restraint(r3)
        self.set_weight(self.weight)

    def set_label(self, label):
        self.label = label

    def add_to_model(self):
        IMP.pmi.tools.add_restraint_to_model(self.m, self.rs)

    def get_restraint(self):
        return self.rs

    def get_output(self):
        self.m.update()
        output = {}
        score = self.evaluate()
        output["_TotalScore"] = str(score)
        output["ExternalBarrier_" + self.label] = str(score)
        return output

    def set_weight(self, weight):
        self.weight = weight
        self.rs.set_weight(weight)

    def evaluate(self):
        return self.weight * self.rs.unprotected_evaluate(None)


class DistanceRestraint(object):
    """A simple distance restraint"""
    def __init__(self,
                 representation=None,
                 tuple_selection1=None,
                 tuple_selection2=None,
                 distancemin=0,
                 distancemax=100,
                 resolution=1.0,
                 kappa=1.0,
                 root_hier = None):
        """Setup distance restraint.
        @param representation DEPRECATED
        @param tuple_selection1 (resnum,resnum,molecule name, copy number (=0))
        @param tuple_selection2 (resnum,resnum,molecule name, copy number (=0))
        @param distancemin The minimum dist
        @param distancemax The maximum dist
        @param resolution For selecting particles
        @param kappa The harmonic parameter
        @param root_hier The hierarchy to select from (use this instead of representation)
        \note Pass the same resnum twice to each tuple_selection. Optionally add a copy number (PMI2 only)
        """
        self.weight=1.0
        self.label="None"
        if tuple_selection1 is None or tuple_selection2 is None:
            raise Exception("You must pass tuple_selection1/2")
        ts1 = IMP.core.HarmonicUpperBound(distancemax, kappa)
        ts2 = IMP.core.HarmonicLowerBound(distancemin, kappa)

        if representation and not root_hier:
            self.m = representation.prot.get_model()
            particles1 = IMP.pmi.tools.select(representation,
                                              resolution=resolution,
                                              name=tuple_selection1[2],
                                              residue=tuple_selection1[0])
            particles2 = IMP.pmi.tools.select(representation,
                                              resolution=resolution,
                                              name=tuple_selection2[2],
                                              residue=tuple_selection2[0])
        elif root_hier and not representation:
            self.m = root_hier.get_model()
            copy_num1 = 0
            if len(tuple_selection1)>3:
                copy_num1 = tuple_selection1[3]
            copy_num2 = 0
            if len(tuple_selection2)>3:
                copy_num2 = tuple_selection2[3]

            sel1 = IMP.atom.Selection(root_hier,
                                      resolution=resolution,
                                      molecule=tuple_selection1[2],
                                      residue_index=tuple_selection1[0],
                                      copy_index=copy_num1)
            particles1 = sel1.get_selected_particles()
            sel2 = IMP.atom.Selection(root_hier,
                                      resolution=resolution,
                                      molecule=tuple_selection2[2],
                                      residue_index=tuple_selection2[0],
                                      copy_index=copy_num2)
            particles2 = sel2.get_selected_particles()
        else:
            raise Exception("Pass representation or root_hier, not both")

        self.rs = IMP.RestraintSet(self.m, 'distance')

        print("Created distance restraint between "
              "%s and %s" % (particles1[0].get_name(),particles2[0].get_name()))

        if len(particles1) > 1 or len(particles2) > 1:
            raise ValueError("more than one particle selected")

        self.rs.add_restraint(
            IMP.core.DistanceRestraint(self.m, ts1,
                                       particles1[0],
                                       particles2[0]))
        self.rs.add_restraint(
            IMP.core.DistanceRestraint(self.m, ts2,
                                       particles1[0],
                                       particles2[0]))

    def set_weight(self,weight):
        self.weight = weight
        self.rs.set_weight(weight)

    def set_label(self, label):
        self.label = label

    def add_to_model(self):
        IMP.pmi.tools.add_restraint_to_model(self.m, self.rs)

    def get_restraint(self):
        return self.rs

    def get_restraint_for_rmf(self):
        return self.rs

    def get_output(self):
        self.m.update()
        output = {}
        score = self.weight * self.rs.unprotected_evaluate(None)
        output["_TotalScore"] = str(score)
        output["DistanceRestraint_" + self.label] = str(score)
        return output

    def evaluate(self):
        return self.weight * self.rs.unprotected_evaluate(None)


class CylinderRestraint(IMP.Restraint):
    '''
    PMI2 python restraint. Restrains particles within a
    Cylinder aligned along the z-axis and
    centered in x,y=0,0
    '''
    import math
    def __init__(self, m, objects, resolution, radius,label='None'):
        '''
        @param objects PMI2 objects
        @param resolution the resolution you want the restraint to be applied
        @param radius the radius of the cylinder
        '''
        IMP.Restraint.__init__(self, m, "CylinderRestraint %1%")
        self.radius=radius
        self.softness = 3.0
        self.plateau = 1e-10
        self.weight = 1.0
        self.m=m
        hierarchies = IMP.pmi.tools.input_adaptor(objects,
                                            resolution,
                                            flatten=True)
        self.particles = [h.get_particle() for h in hierarchies]
        self.label=label

    def get_probability(self,p):
        xyz=IMP.core.XYZ(p)
        r=self.math.sqrt(xyz.get_x()**2+xyz.get_y()**2)
        argvalue=(r-self.radius) / self.softness
        prob = (1.0 - self.plateau) / (1.0 + self.math.exp(-argvalue))
        return prob

    def unprotected_evaluate(self, da):
        s=0.0
        for p in self.particles:
            s+=-self.math.log(1.0-self.get_probability(p))
        return s

    def do_get_inputs(self):
        return self.particles

    def add_to_model(self):
        IMP.pmi.tools.add_restraint_to_model(self.m, self)

    def get_output(self):
        self.m.update()
        output = {}
        score = self.weight * self.unprotected_evaluate(None)
        output["_TotalScore"] = str(score)
        output["CylinderRestraint_" + self.label] = str(score)
        return output



class BiStableDistanceRestraint(IMP.Restraint):
    '''
    a python restraint with bistable potential
    Authors: G. Bouvier, R. Pellarin. Pasteur Institute.
    '''
    import numpy as np
    import math

    def __init__(self,m,p1,p2,dist1,dist2,sigma1,sigma2,weight1,weight2):
        '''
        input twp particles, the two equilibrium distances, their amplitudes, and their weights (populations)
        '''
        IMP.Restraint.__init__(self, m, "BiStableDistanceRestraint %1%")
        self.dist1=dist1
        self.dist2=dist2

        self.sigma1=sigma1
        self.sigma2=sigma2

        self.weight1=weight1
        self.weight2=weight2

        if self.weight1+self.weight2 != 1:
            raise ValueError("The sum of the weights must be one")

        self.d1=IMP.core.XYZ(p1)
        self.d2=IMP.core.XYZ(p2)
        self.particle_list=[p1,p2]

    def gaussian(self,x, mu, sig, w):
        return w*self.np.exp(-self.np.power(x - mu, 2.) / (2 * self.np.power(sig, 2.)))

    def unprotected_evaluate(self,da):
        dist=IMP.core.get_distance(self.d1,self.d2)
        prob=self.gaussian(dist,self.dist1,self.sigma1,self.weight1)+\
             self.gaussian(dist,self.dist2,self.sigma2,self.weight2)
        return -self.math.log(prob)

    def do_get_inputs(self):
        return self.particle_list
