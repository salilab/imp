import IMP.test
import IMP.algebra
import IMP.em
import random,math
from IMP.algebra import *

class Tests(IMP.test.TestCase):
    def test_transform(self):
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
        print "coming from"
        mtit= IMP.em.get_transformed(mt, tr.get_inverse(), .01)
        IMP.em.write_map(mtit, self.get_tmp_file_name("backtransformed.mrc"), mrw)
        print "testing"
        print IMP.em.get_bounding_box(m)
        print IMP.em.get_bounding_box(mt)
        print IMP.em.get_bounding_box(mtit)
        errors=0
        for v in range(m.get_number_of_voxels()):
            pt= Vector3D(m.get_location_in_dim_by_voxel(v,0),
                         m.get_location_in_dim_by_voxel(v,1),
                         m.get_location_in_dim_by_voxel(v,2))
            oval= IMP.em.get_density(m, pt)
            nval= IMP.em.get_density(mtit, pt)
            if abs(oval-nval)>(oval+nval+1)*.3:
                errors=errors+1
        self.assertLess(errors, .1 *m.get_number_of_voxels(), errors)

    def test_transformed_into(self):
        """Check functionality of the get_transformed_into function"""
        bb = IMP.algebra.BoundingBox3D(IMP.algebra.Vector3D(-8,-9,-7),
                                       IMP.algebra.Vector3D(4,5,6))

        from_m = IMP.em.DensityMap(IMP.em.create_density_header(bb, 1))

        # Make the map empty except for a blob of density around (-3,-2,-1)
        for i in range(from_m.get_number_of_voxels()):
            from_m.set_value(i, 0.)
        for x in range(-5,0):
            for y in range(-4,1):
                for z in range(-3,2):
                    from_m.set_value(x,y,z, 10.0)
        self.assertAlmostEqual(IMP.em.get_density(from_m,
                                           IMP.algebra.Vector3D(-3,-2,-1)),
                               10.0, delta=0.1)
        self.assertAlmostEqual(IMP.em.get_density(from_m,
                                           IMP.algebra.Vector3D(3,2,1)),
                               0.0, delta=0.1)

        # Transform with a random rotation and then back again
        tr = IMP.algebra.Transformation3D(IMP.algebra.get_random_rotation_3d(),
                                          IMP.algebra.Vector3D(0,0,0))

        into_m = IMP.em.DensityMap()
        IMP.em.get_transformed_into(from_m, tr, into_m)

        back_m = IMP.em.DensityMap()
        IMP.em.get_transformed_into(into_m, tr.get_inverse(), back_m)

        self.assertAlmostEqual(IMP.em.get_density(back_m,
                                           IMP.algebra.Vector3D(-3,-2,-1)),
                               10.0, delta=0.3)
        self.assertAlmostEqual(IMP.em.get_density(back_m,
                                           IMP.algebra.Vector3D(3,2,1)),
                               0.0, delta=0.1)


if __name__ == '__main__':
    IMP.test.main()
