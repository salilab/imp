import unittest
import IMP.test
import IMP.em
import os
import random,math
from IMP.algebra import *

class DensityTransformTest(IMP.test.TestCase):
    def test_tramsfrom(self):
        """Check transforming a density map"""
        m= IMP.em.read_map(self.get_input_file_name('1z5s.mrc'))
        #tr= Transformation3D(get_random_rotation_3d(), Vector3D(0,0,0))
        axis =IMP.algebra.get_random_vector_on(IMP.algebra.get_unit_sphere_3d())
        rand_angle = random.uniform(-15./180*math.pi,15./180*math.pi)
        r= IMP.algebra.get_rotation_about_axis(axis, rand_angle)
        translation = IMP.algebra.get_random_vector_in(IMP.algebra.get_unit_bounding_box_3d())
        tr=IMP.algebra.Transformation3D(r,translation)
        #                             IMP.algebra.get_random_vector_in(Vector3D(-100,-100,-100),
        #                                                             Vector3D(100,100,100)))
        print "going to"
        mt= IMP.em.get_transformed(m, tr, .01)
        mrw= IMP.em.MRCReaderWriter()
        IMP.em.write_map(mt, self.get_tmp_file_name("transformed.mrc"), mrw)
        IMP.em.write_map(mt, "transformed.mrc", mrw)
        print "coming from"
        mtit= IMP.em.get_transformed(mt, tr.get_inverse(), .01);
        IMP.em.write_map(mtit, self.get_tmp_file_name("backtransformed.mrc"), mrw)
        IMP.em.write_map(mtit, "backtransformed.mrc", mrw)
        print "testing"
        print IMP.em.get_bounding_box(m)
        print IMP.em.get_bounding_box(mt)
        print IMP.em.get_bounding_box(mtit)
        errors=0
        for v in range(m.get_number_of_voxels()):
            pt= Vector3D(m.voxel2loc(v,0),
                         m.voxel2loc(v,1),
                         m.voxel2loc(v,2))
            oval= IMP.em.get_density(m, pt)
            nval= IMP.em.get_density(mtit, pt)
            if abs(oval-nval)>(oval+nval+1)*.3:
                errors=errors+1
        self.assert_(errors <.1 *m.get_number_of_voxels(), errors)

    def test_tramsfromed_into(self):
        """Check functionality of the get_transformed_into function"""
        IMP.set_log_level(IMP.TERSE)
        from_m= IMP.em.read_map(self.get_input_file_name('1z5s.mrc'))
        from_m.show()
        into_m = IMP.em.DensityMap()
        back_m = IMP.em.DensityMap()
        tr= Transformation3D(get_random_rotation_3d(), Vector3D(0,0,0))
        IMP.em.get_transformed_into(from_m, tr, into_m)
        into_m.show()
        print "====||1"
        print into_m.get_top()
        print "====||2"
        IMP.em.get_transformed_into(into_m, tr.get_inverse(),back_m)
        errors=0
        for v in range(from_m.get_number_of_voxels()):
            pt= Vector3D(from_m.voxel2loc(v,0),
                         from_m.voxel2loc(v,1),
                         from_m.voxel2loc(v,2))
            oval= IMP.em.get_density(from_m, pt)
            nval= IMP.em.get_density(back_m, pt)
            if abs(oval-nval)>(oval+nval+1)*.3:
                errors=errors+1
        self.assert_(errors <.1 *from_m.get_number_of_voxels(), errors)


if __name__ == '__main__':
    unittest.main()
