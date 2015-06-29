"""@namespace IMP.pmi.restraints.em2d
Restraints for handling electron microscopy images.
"""

from __future__ import print_function
import IMP
import IMP.core
import IMP.algebra
import IMP.atom
import IMP.em2d
import IMP.pmi.tools

class ElectronMicroscopy2D():

    def __init__(
        self,
        representation,
        images,
        pixel_size,
        image_resolution,
        projection_number,
        resolution=None):

        self.weight=1.0
        self.m = representation.prot.get_model()
        self.rs = IMP.RestraintSet(self.m, 'em2d')
        self.label = "None"

        # IMP.atom.get_by_type
        particles = IMP.pmi.tools.select(
            representation,
            resolution=resolution)

        #print particles


        em2d = IMP.em2d.PCAFitRestraint(
            particles, images, pixel_size, image_resolution, projection_number, True)
        self.rs.add_restraint(em2d)

    def set_label(self, label):
        self.label = label

    def add_to_model(self):
        IMP.pmi.tools.add_restraint_to_model(self.m, self.rs)

    def get_restraint(self):
        return self.rs

    def set_weight(self,weight):
        self.weight=weight
        self.rs.set_weight(self.weight)

    def get_output(self):
        self.m.update()
        output = {}
        score = self.weight*self.rs.unprotected_evaluate(None)
        output["_TotalScore"] = str(score)
        output["ElectronMicroscopy2D_" + self.label] = str(score)
        return output
