from __future__ import print_function
import IMP
import IMP.test
import IMP.core
import IMP.base
import IMP.algebra
import IMP.atom
import IMP.container

import IMP.pmi.restraints.stereochemistry
import IMP.pmi.restraints.basic
import IMP.pmi.restraints.proteomics
import IMP.pmi.restraints.crosslinking
import IMP.pmi.restraints.em
import IMP.pmi.representation
import IMP.pmi.tools

import numpy as np


def get_random_gaussian_3d(center):
    std = np.random.random_sample(3,) * 10
    var = [s ** 2 for s in std]
    rot = IMP.algebra.get_random_rotation_3d()
    trans = IMP.algebra.Transformation3D(rot, center)
    return IMP.algebra.Gaussian3D(IMP.algebra.ReferenceFrame3D(trans), var)

class TestEMRestraint(IMP.test.TestCase):

    def setUp(self):
        IMP.test.TestCase.setUp(self)

        self.m = IMP.Model()
        self.p0 = IMP.Particle(self.m)
        self.g0 = IMP.core.Gaussian.setup_particle(
            self.p0, get_random_gaussian_3d([0, 0, 0]))
        IMP.atom.Mass.setup_particle(self.p0, np.random.rand() * 10)
        self.p1 = IMP.Particle(self.m)
        self.g1 = IMP.core.Gaussian.setup_particle(
            self.p1, get_random_gaussian_3d([0, 0, 0]))
        IMP.atom.Mass.setup_particle(self.p1, np.random.rand() * 10)

    @IMP.test.skip("EM interface is changing")
    def test_move_to_center(self):
        target_h = [IMP.atom.Fragment.setup_particle(IMP.Particle(self.m))]
        target_h[0].add_child(self.p0)
        gem = IMP.pmi.restraints.em.GaussianEMRestraint(target_h,
                                                        target_ps=[self.p1],
                                                        cutoff_dist_for_container=10000.0,
                                                        target_radii_scale=10.0,
                                                        model_radii_scale=10.0)
        gem.add_to_model()
        self.m.update()
        print('eval 0')
        s0 = self.m.evaluate(False)
        print('>s0', s0, '\n')

        trans = IMP.algebra.Transformation3D(
            IMP.algebra.Vector3D(np.random.random_sample(3,) * 10))
        print('random trans', trans)
        IMP.core.transform(IMP.core.RigidBody(self.p0), trans)
        self.m.update()
        s1 = self.m.evaluate(False)
        print('>s1', s1, '\n')

        gem.center_model_on_target_density()
        self.m.update()
        s2 = self.m.evaluate(False)
        print('>s2', s2)
        self.assertAlmostEqual(s0, s2)

if __name__ == '__main__':
    IMP.test.main()
