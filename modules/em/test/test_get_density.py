import unittest
import IMP.test
import IMP.em
import os
import random

class VolumeTest(IMP.test.TestCase):

    def get_center(self, m, i,j,k):
        return IMP.algebra.Vector3D(m.get_origin()[0]+ m.get_spacing()*i,
                                    m.get_origin()[1]+ m.get_spacing()*j,
                                    m.get_origin()[2]+ m.get_spacing()*k)

    def test_image(self):
        """Check get_density"""
        m= IMP.em.read_map(self.get_input_file_name('1z5s.mrc'))
        wid=m.get_top()-m.get_origin()
        for i in random.sample(range(0, m.get_header().nx), 30):
            #print i
            for j in random.sample(range(0, m.get_header().ny), 30):
                for k in random.sample(range(0, m.get_header().nz), 30):
                    v= self.get_center(m, i,j,k)
                    print i,j,k, v
                    #print v
                    val= IMP.em.get_density(m, v)
                    mval= m.get_value(m.xyz_ind2voxel(i,j,k))
                    self.assertInTolerance(val, mval, (val+mval)*.1+.1)
                    self.assertEqual(IMP.em.get_density(m, self.get_center(m, i,j,k) +IMP.algebra.Vector3D(wid[0],0,0)), 0)
                    self.assertEqual(IMP.em.get_density(m, self.get_center(m, i,j,k) +IMP.algebra.Vector3D(0,wid[1],0)), 0)
                    self.assertEqual(IMP.em.get_density(m, self.get_center(m, i,j,k) +IMP.algebra.Vector3D(0,0,wid[2])), 0)
                    self.assertEqual(IMP.em.get_density(m, self.get_center(m, i,j,k) +IMP.algebra.Vector3D(-wid[0],0,0)), 0)
                    self.assertEqual(IMP.em.get_density(m, self.get_center(m, i,j,k) +IMP.algebra.Vector3D(0,-wid[1],0)), 0)
                    self.assertEqual(IMP.em.get_density(m, self.get_center(m, i,j,k) +IMP.algebra.Vector3D(0,0,-wid[2])), 0)


        indexes=[random.randint(0,m.get_header().nx),
                 random.randint(0,m.get_header().ny),
                 random.randint(0,m.get_header().nz)]
        fs=IMP.algebra.Vector3D(random.uniform(0,1),
                                random.uniform(0,1),
                                random.uniform(0,1))
        side= m.get_spacing()
        prod=fs*side
        print type(prod)
        v=self.get_center(m, indexes[0], indexes[1], indexes[2])+prod
        print type(v)
        pt= v
        val= IMP.em.get_density(m, pt)
        lb=100000000
        ub= -lb
        for i in range(0,1):
            for j in range(0,1):
                for k in range(0,1):
                    p= m.get_value(indexes[0]+i, indexes[1]+j, indexes[2]+k)
                    if p < lb: lb=p
                    if p > ub: ub=p
        print val, lb, ub
        self.assert_(val <= ub)
        self.assert_(val >=lb)

if __name__ == '__main__':
    unittest.main()
