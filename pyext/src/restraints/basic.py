"""@namespace IMP.pmi.restraints.basic
Some miscellaneous simple restraints.
"""

from __future__ import print_function
import IMP
import IMP.core
import IMP.base
import IMP.algebra
import IMP.atom
import IMP.container


class ExternalBarrier(object):

    def __init__(
        self,
        representation,
        radius,
        hierarchies=None,
            resolution=None):
        self.m = representation.prot.get_model()
        self.rs = IMP.RestraintSet(self.m, 'barrier')

        self.radius = radius
        self.label = "None"

        c3 = IMP.algebra.Vector3D(0, 0, 0)
        ub3 = IMP.core.HarmonicUpperBound(radius, 10.0)
        ss3 = IMP.core.DistanceToSingletonScore(ub3, c3)
        lsc = IMP.container.ListSingletonContainer(self.m)
        # IMP.atom.get_by_type
        particles = IMP.pmi.tools.select(
            representation,
            resolution=resolution,
            hierarchies=hierarchies)
        lsc.add_particles(particles)
        r3 = IMP.container.SingletonsRestraint(ss3, lsc)
        self.rs.add_restraint(r3)

    def set_label(self, label):
        self.label = label

    def add_to_model(self):
        self.m.add_restraint(self.rs)

    def get_restraint(self):
        return self.rs

    def get_output(self):
        self.m.update()
        output = {}
        score = self.rs.unprotected_evaluate(None)
        output["_TotalScore"] = str(score)
        output["ExternalBarrier_" + self.label] = str(score)
        return output


class DistanceRestraint(object):

    def __init__(
        self,
        representation,
        tuple_selection1,
        tuple_selection2,
        distancemin,
        distancemax,
        resolution=1.0,
            kappa=1.0):
        self.m = representation.prot.get_model()
        self.rs = IMP.RestraintSet(self.m, 'distance')
        self.weight=1.0
        self.label="None"

        #ts = IMP.core.Harmonic(distance,kappa)

        ts1 = IMP.core.HarmonicUpperBound(distancemax, kappa)
        ts2 = IMP.core.HarmonicLowerBound(distancemin, kappa)

        # IMP.atom.get_by_type
        particles1 = IMP.pmi.tools.select(
            representation,
            resolution=resolution,
            name=tuple_selection1[2],
            residue=tuple_selection1[0])
        particles2 = IMP.pmi.tools.select(
            representation,
            resolution=resolution,
            name=tuple_selection2[2],
            residue=tuple_selection2[0])

        for p in particles1:
            print(p.get_name())

        for p in particles2:
            print(p.get_name())

        if len(particles1) > 1 or len(particles2) > 1:
            raise ValueError("more than one particle selected")

        self.rs.add_restraint(
            IMP.core.DistanceRestraint(ts1,
                                       particles1[0],
                                       particles2[0]))
        self.rs.add_restraint(
            IMP.core.DistanceRestraint(ts2,
                                       particles1[0],
                                       particles2[0]))

    def set_weight(self,weight):
        self.weight = weight
        self.rs.set_weight(weight)

    def set_label(self, label):
        self.label = label

    def add_to_model(self):
        self.m.add_restraint(self.rs)

    def get_restraint(self):
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
