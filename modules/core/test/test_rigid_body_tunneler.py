import IMP.kernel
import IMP.test
import IMP.core

from numpy import *

class RigidBodyTunnelerTest(IMP.test.TestCase):

    def setUp(self):
        IMP.test.TestCase.setUp(self)
        self.m = IMP.Model()
        #origin rb: identity rotation and axes aligned with origin
        oricom = zeros(3)
        self.oricom = oricom
        orivecs = array([[0,0,0],[1,0,0],[0,1,0]])
        self.partori = self.make_rb(oricom, *orivecs)
        self.iori = self.partori[-1].get_index()
        #reference rb : rotation of pi/2 around z and translation to (1,0,0)
        refcom = array([1,0,0])
        self.refcom = refcom
        refvecs = array([[0,0,0],[0,1,0],[-1,0,0]])
        self.partref = self.make_rb(refcom, *refvecs)
        self.iref = self.partref[-1].get_index()
        #other rb : rotation of pi/2 around y and translation to (2,0,0)
        othcom = array([2,0,0])
        self.othcom = othcom
        othvecs = array([[0,0,0],[0,0,-1],[0,1,0]])
        self.partoth = self.make_rb(othcom, *othvecs)
        self.ioth = self.partoth[-1].get_index()
        #rbt
        self.k = 0.5
        self.rbt = IMP.core.RigidBodyTunneler(self.m, [self.ioth], self.iref,
                self.k, 1.0)
        self.rbori = IMP.core.RigidBodyTunneler(self.m, [self.iref], self.iori,
                self.k, 1.0)

    def make_rb(self, com, c1, c2, c3):
        """make rb with three xyzs of coordinates c1,c2,c3
        and five others with random coords (needed because the svd
        of RigidBody becomes singular sometimes)
        the centroid of all of these will be com
        """
        #get coordinates of all, subtract centroid
        coords=[c1,c2,c3]
        for i in xrange(5):
            coo = random.random(3).tolist()
            coords.append(coo)
        coords=array(coords)
        nowcom = coords.mean(axis=0)
        newcoords = (coords + (com - nowcom)[None,:]).tolist()
        pr1=IMP.Particle(self.m)
        IMP.core.XYZ.setup_particle(pr1, IMP.algebra.Vector3D(newcoords[0]))
        pr2=IMP.Particle(self.m)
        IMP.core.XYZ.setup_particle(pr2, IMP.algebra.Vector3D(newcoords[1]))
        pr3=IMP.Particle(self.m)
        IMP.core.XYZ.setup_particle(pr3, IMP.algebra.Vector3D(newcoords[2]))
        prb=IMP.Particle(self.m)
        rbparts=[pr1,pr2,pr3]
        for i in xrange(5):
            p=IMP.Particle(self.m)
            IMP.core.XYZ.setup_particle(p, IMP.algebra.Vector3D(newcoords[3+i]))
            rbparts.append(p)
        d = IMP.core.RigidBody.setup_particle(prb, rbparts)
        d.set_coordinates_are_optimized(True)
        rbparts.append(prb)
        return rbparts

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

    def test_reduced_coords_rel_degen2(self):
        #test Referential.get_local_coords and
        #Referential.get_local_rotation in the degenerate case
        coords = array(self.rbt.get_reduced_coordinates(self.m, self.ioth,
            self.ioth))
        self.assertEqual(len(coords), 7)
        self.assertTrue(allclose(coords[:3], zeros(3)))
        #expect identity rotation
        self.assertAlmostEqual(coords[3], 1)
        self.assertTrue(allclose(coords[4:], zeros(3)))

    def test_reduced_coords_rel_degen3(self):
        #test Referential.get_local_coords and
        #Referential.get_local_rotation in the degenerate case
        coords = array(self.rbt.get_reduced_coordinates(self.m, self.iori,
            self.iori))
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
        #expect rotation of 2*pi/3 around (1,1,-1)/sqrt(3)
        self.assertAlmostEqual(arccos(coords[3])*2, 2*pi/3.)
        self.assertTrue(allclose(coords[4:],
            sin(2*pi/6)*array([1,1,-1])/sqrt(3)))

    def test_reduced_coords_rel_real2(self):
        #test Referential.get_local_coords and
        #Referential.get_local_rotation in a real case
        coords = array(self.rbori.get_reduced_coordinates(self.m, self.iref,
            self.iori))
        self.assertEqual(len(coords), 7)
        self.assertTrue(allclose(coords[:3], self.refcom))
        #expect rotation of pi/2 around Oz
        self.assertAlmostEqual(arccos(coords[3])*2, pi/2)
        self.assertTrue(allclose(coords[4:]/sin(arccos(coords[3])),
            array([0,0,1])))

    def test_entry_translation(self):
        #two entry points, with perfect match
        entry1 = array([1,0,0,sqrt(2)/2.,0,0,sqrt(2)/2.])
        self.rbori.add_entry_point(entry1) #exact match with ref
        entry2 = array([-1,0,0,sqrt(2)/2.,0,0,sqrt(2)/2.])
        self.rbori.add_entry_point(entry2) #ref but on opposite side of origin
        self.rbori.propose()
        coords = self.rbori.get_reduced_coordinates(self.m, self.iref)
        self.assertTrue(allclose(entry2,coords))
        self.rbori.reject()
        coords = self.rbori.get_reduced_coordinates(self.m, self.iref)
        self.assertTrue(allclose(entry1,coords))

    def test_entry_rotation(self):
        #two entry points. ref has rotation pi/2 around Oz
        #5pi/12 rotation along Oz
        oldcoords = self.rbori.get_reduced_coordinates(self.m, self.iref)
        entry1 = array([1,0,0,0.793353,0,0,0.608761])
        self.rbori.add_entry_point(entry1) #exact match with ref
        #pi/12 rotation along Oz
        entry2 = array([1,0,0,0.991445,0,0,0.130526])
        self.rbori.add_entry_point(entry2)
        self.rbori.propose()
        #expect a remaining rotation of pi/6 along Oz
        coords = self.rbori.get_reduced_coordinates(self.m, self.iref)
        expected = array([1,0,0,0.965926,0,0,0.258819])
        self.assertTrue(allclose(expected,coords))
        self.rbori.reject()
        coords = self.rbori.get_reduced_coordinates(self.m, self.iref)
        self.assertTrue(allclose(oldcoords,coords))

    def test_entry_rotation2(self):
        #two entry points. ref has rotation pi/2 around Oz
        #pi/4 rotation along Ox
        oldcoords = self.rbori.get_reduced_coordinates(self.m, self.iref)
        entry1 = array([1,0,0,0.92388,0.382683,0,0])
        self.rbori.add_entry_point(entry1) #exact match with ref
        #pi rotation along Ox
        entry2 = array([1,0,0,0,1,0,0])
        self.rbori.add_entry_point(entry2)
        self.rbori.propose()
        #expect a remaining rotation
        coords = self.rbori.get_reduced_coordinates(self.m, self.iref)
        expected = array([1,0,0,0.270598,0.653281,-0.653281,0.270598])
        self.assertTrue(allclose(expected,coords))
        self.rbori.reject()
        coords = self.rbori.get_reduced_coordinates(self.m, self.iref)
        self.assertTrue(allclose(oldcoords,coords))

    def test_entry_translation2(self):
        #two entry points. 
        oldcoords = self.rbori.get_reduced_coordinates(self.m, self.iref)
        #first entry point at (2,0,0)
        entry1 = array([2,0,0,sqrt(3)/2.,0,0,1/2.])
        self.rbori.add_entry_point(entry1)
        #second entry point at (0,5,0)
        entry2 = array([0,5,0,sqrt(3)/2.,0,0,1/2.])
        self.rbori.add_entry_point(entry2)
        self.rbori.propose()
        coords = self.rbori.get_reduced_coordinates(self.m, self.iref)
        expected = array([-1,5,0]+oldcoords[3:])
        self.assertTrue(allclose(expected,coords))
        self.rbori.reject()
        coords = self.rbori.get_reduced_coordinates(self.m, self.iref)
        self.assertTrue(allclose(oldcoords,coords))

    def test_entry_both(self):
        #combination of rotation and translation2
        oldcoords = self.rbori.get_reduced_coordinates(self.m, self.iref)
        #first entry point at (2,0,0) with 5pi/12 rotation along Oz
        entry1 = array([2,0,0,0.793353,0,0,0.608761])
        self.rbori.add_entry_point(entry1)
        #second entry point at (0,5,0) with pi/12 rotation along Oz
        entry2 = array([0,5,0,0.991445,0,0,0.130526])
        self.rbori.add_entry_point(entry2)
        self.rbori.propose()
        #expect a remaining rotation of pi/6 along Oz at (-1,5,0)
        coords = self.rbori.get_reduced_coordinates(self.m, self.iref)
        expected = array([-1,5,0,0.965926,0,0,0.258819])
        self.assertTrue(allclose(expected,coords))
        self.rbori.reject()
        coords = self.rbori.get_reduced_coordinates(self.m, self.iref)
        self.assertTrue(allclose(oldcoords,coords))

    def test_entry_both2(self):
        #combination of rotation2 and translation2
        oldcoords = self.rbori.get_reduced_coordinates(self.m, self.iref)
        #first entry point at (2,0,0) with pi/4 rotation along Ox
        entry1 = array([2,0,0,0.92388,0.382683,0,0])
        self.rbori.add_entry_point(entry1)
        #second entry point at (0,5,0) with pi rotation along Ox
        entry2 = array([0,5,0,0,1,0,0])
        self.rbori.add_entry_point(entry2)
        self.rbori.propose()
        #expect a remaining rotation at (-1,5,0)
        coords = self.rbori.get_reduced_coordinates(self.m, self.iref)
        expected = array([-1,5,0,0.270598,0.653281,-0.653281,0.270598])
        self.assertTrue(allclose(expected,coords))
        self.rbori.reject()
        coords = self.rbori.get_reduced_coordinates(self.m, self.iref)
        self.assertTrue(allclose(oldcoords,coords))

    def test_entry_real(self):
        #same as test_entry_both2, but in the referential of ref
        oldcoords = self.rbt.get_reduced_coordinates(self.m, self.ioth,
                self.iref)
        #first entry point at (0,-2,0) with pi/4 rotation along Oy
        entry1 = array([0,-2,0,0.92388,0,0.382683,0])
        self.rbt.add_entry_point(entry1)
        #second entry point at (5,0,0) with pi rotation along Oy
        entry2 = array([5,0,0,0,0,1,0])
        self.rbt.add_entry_point(entry2)
        self.rbt.propose()
        #expect a remaining rotation with translation of (5,1,0)
        coords = self.rbt.get_reduced_coordinates(self.m, self.ioth, self.iref)
        expected = array([5,1,0,0.270598,0.270598,-0.653281,0.653281])
        self.assertTrue(allclose(expected,coords))
        self.rbt.reject()
        coords = self.rbt.get_reduced_coordinates(self.m, self.ioth, self.iref)
        self.assertTrue(allclose(oldcoords,coords))


if __name__ == '__main__':
    IMP.test.main()
