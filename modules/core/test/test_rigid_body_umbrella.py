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
        self.alpha=50
        self.x0 = array([1,0,0,1,0,0,0]).tolist()
        self.rbu = IMP.core.RigidBodyUmbrella(self.m, self.ioth, self.iref,
                self.x0, self.alpha, self.k)
        self.score = IMP.core.RestraintsScoringFunction([self.rbu])

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

    def test_get_set_simple(self):
        self.assertTrue(allclose(self.x0, asarray(self.rbu.get_x0())))
        self.x0 = self.rbu.get_x()
        self.rbu.set_x0(self.x0)
        self.assertTrue(allclose(self.x0, asarray(self.rbu.get_x0())))

    def test_get_set_interp(self):
        self.assertTrue(allclose(self.x0, asarray(self.rbu.get_x0())))
        x1 = asarray(self.x0)
        x2 = asarray(self.rbu.get_x())
        for la in linspace(0,1,num=5):
            expected = zeros(7)
            expected[:3] = x1[:3]*(1-la) + x2[:3]*la
            self.rbu.set_x0(la,x1,x2)
            observed = asarray(self.rbu.get_x0())
            self.assertTrue(allclose(observed[:3],expected[:3]))

    def test_get_set_extremes(self):
        self.assertTrue(allclose(self.x0, asarray(self.rbu.get_x0())))
        x1 = self.x0[:]
        x2 = self.rbu.get_x()
        self.rbu.set_x0(0., x1, x2)
        observed = asarray(self.rbu.get_x0())
        self.assertTrue(allclose(observed, x1))
        self.rbu.set_x0(1., x1, x2)
        observed = asarray(self.rbu.get_x0())
        self.assertTrue(allclose(observed, x2))

if __name__ == '__main__':
    IMP.test.main()
