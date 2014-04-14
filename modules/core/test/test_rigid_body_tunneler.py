import IMP.kernel
import IMP.test
import IMP.core

from numpy import *

class RigidBodyTunnelerTest(IMP.test.TestCase):

    def setUp(self):
        IMP.test.TestCase.setUp(self)
        self.m = IMP.Model()
        #reference rb : rotation of pi/2 around z and translation to (1,0,0)
        refcom = array([1,0,0])
        self.refcom = refcom
        refvecs = array([[0,0,0],[0,1,0],[-1,0,0]])
        self.refvecs = (refvecs+(refcom-refvecs.mean(axis=0))[None,:]).tolist()
        self.rbref = self.make_rb(*self.refvecs)
        self.iref = self.rbref[-1].get_index()
        #other rb : rotation of pi/2 around y and translation to (2,0,0)
        othcom = array([2,0,0])
        self.othcom = othcom
        othvecs = array([[0,0,0],[0,0,-1],[0,1,0]])
        self.othvecs = (othvecs+(othcom-othvecs.mean(axis=0))[None,:]).tolist()
        self.rboth = self.make_rb(*self.othvecs)
        self.ioth = self.rboth[-1].get_index()
        #rbt
        self.k = 0.5
        self.rbt = IMP.core.RigidBodyTunneler(self.m, [self.iref], self.ioth,
                self.k, 1.0)

    def make_rb(self, c1, c2, c3):
        """make rb with three xyzs of coordinates c1,c2,c3"""
        pr1=IMP.Particle(self.m)
        IMP.core.XYZ.setup_particle(pr1, IMP.algebra.Vector3D(c1))
        pr2=IMP.Particle(self.m)
        IMP.core.XYZ.setup_particle(pr2, IMP.algebra.Vector3D(c2))
        pr3=IMP.Particle(self.m)
        IMP.core.XYZ.setup_particle(pr3, IMP.algebra.Vector3D(c3))
        prb=IMP.Particle(self.m)
        d = IMP.core.RigidBody.setup_particle(prb, (pr1,pr2,pr3))
        return pr1,pr2,pr3,prb

    def test_reduced_coords_oth(self):
        #test Referential.get_centroid and Referential.get_rotation on oth
        coords = array(self.rbt.get_reduced_coordinates(self.m, self.ioth))
        self.assertEqual(len(coords), 7)
        self.assertTrue(allclose(coords[:3], self.othcom))
        self.assertAlmostEqual(arccos(coords[3])*2, pi/2.)
        self.assertTrue(allclose(coords[4:]/sin(arccos(coords[3])),
                array([0,1,0])))

    def test_reduced_coords_ref(self):
        #test Referential.get_centroid and Referential.get_rotation on ref
        coords = array(self.rbt.get_reduced_coordinates(self.m, self.iref))
        self.assertEqual(len(coords), 7)
        self.assertTrue(allclose(coords[:3], self.refcom))
        self.assertAlmostEqual(arccos(coords[3])*2, pi/2.)
        self.assertTrue(allclose(coords[4:]/sin(arccos(coords[3])),
                array([0,0,1])))

    def test_reduced_coords_rel_degen(self):
        #test Referential.get_local_coords and
        #Referential.get_local_rotation in the degenerate case
        coords = array(self.rbt.get_reduced_coordinates(self.m, self.iref,
            self.iref))
        self.assertEqual(len(coords), 7)
        self.assertTrue(allclose(coords[:3], zeros(3)))
        #expect identity rotation
        self.assertAlmostEqual(coords[3], 1)
        self.assertTrue(allclose(coords[4:], zeros(3)))

    def test_reduced_coords_rel_real(self):
        #test Referential.get_local_coords and
        #Referential.get_local_rotation in a real case
        coords = array(self.rbt.get_reduced_coordinates(self.m, self.ioth,
            self.iref))
        self.assertEqual(len(coords), 7)
        self.assertTrue(allclose(coords[:3], array([0,-1,0])))
        #expect rotation of -2*pi/3 around (-1,-1,1)/sqrt(3)
        self.assertAlmostEqual(arccos(coords[3])*2, 2*pi/3.)
        self.assertTrue(allclose(coords[4:]/sin(arccos(coords[3])),
                array([1,1,-1])/sqrt(3)))

if __name__ == '__main__':
    IMP.test.main()
