import IMP
import IMP.test
import IMP.core
import IMP.symmetry
import IMP.algebra

from math import *

class Test(IMP.test.TestCase):
    def setUp(self):
        IMP.test.TestCase.setUp(self)
        self.m = IMP.Model()
        self.ps = []
        for i in range(2):
            p = IMP.Particle(self.m)
            self.ps.append(p)
            IMP.core.XYZ.setup_particle(p, IMP.algebra.Vector3D(0,0,0))
        self.ctrs=[]
        for dx in [0.0, -1.0, 1.0]:
            for dy in [0.0, -1.0, 1.0]:
                for dz in [0.0, -1.0, 1.0]:
                    self.ctrs.append(IMP.algebra.Vector3D(dx,dy,dz))
        self.trs=[]
        for ctr in self.ctrs:
            self.trs.append(IMP.algebra.Transformation3D(ctr))
        self.bmv = IMP.symmetry.BallMover(self.ps[0],[self.ps[1]],1.0,self.ctrs,self.trs)

    def test_propose(self):
        for i in range(100):
            ps=self.bmv.propose(1.0)
            self.assertEqual(len(ps), len(self.ps))
            new0 = IMP.core.XYZ(self.ps[0]).get_coordinates()
            new1 = IMP.core.XYZ(self.ps[1]).get_coordinates()
            dist = IMP.algebra.get_distance(new0,new1)
            self.assertTrue(dist<1.0)
            self.bmv.reject()

    def test_reject(self):
        old=[]
        for p in self.ps:
            old.append(IMP.core.XYZ(p).get_coordinates())
        self.bmv.propose(1.0)
        self.bmv.reject()
        new=[]
        for p in self.ps:
            new.append(IMP.core.XYZ(p).get_coordinates())
        for i in range(len(self.ps)):
            for j in range(3):
                self.assertAlmostEqual(new[i][j],old[i][j])

if __name__ == '__main__':
    IMP.test.main()
