import unittest
import IMP.test
import IMP.em
import os
import random
from IMP.algebra import *

class VolumeTest(IMP.test.TestCase):
    def test_image(self):
        """Check transforming a density map"""
        m= IMP.em.read_map(self.get_input_file_name('1z5s.mrc'))
        tr= Transformation3D(random_rotation(), Vector3D(0,0,0))
        #                             IMP.algebra.random_vector_in_box(Vector3D(-100,-100,-100),
        #                                                             Vector3D(100,100,100)))
        print "going to"
        mt= IMP.em.get_transformed(m, tr, .01)
        mrw= IMP.em.MRCReaderWriter()
        IMP.em.write_map(mt, self.get_tmp_file_name("transformed.mrc"), mrw)
        print "coming from"
        mtit= IMP.em.get_transformed(mt, tr.get_inverse(), .01);
        IMP.em.write_map(mtit, self.get_tmp_file_name("backtransformed.mrc"), mrw)
        print "testing"
        print IMP.em.get_bounding_box(m)
        print IMP.em.get_bounding_box(mt)
        print IMP.em.get_bounding_box(mtit)
        for v in range(m.get_number_of_voxels()):
            pt= Vector3D(m.voxel2loc(v,0),
                         m.voxel2loc(v,1),
                         m.voxel2loc(v,2))
            oval= IMP.em.get_density(m, pt)
            nval= IMP.em.get_density(mtit, pt)
            self.assertInTolerance(oval, nval, (oval+nval+1)*.3)

if __name__ == '__main__':
    unittest.main()
