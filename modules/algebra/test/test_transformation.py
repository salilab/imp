import IMP
import IMP.test
import IMP.algebra
import math,random

class Tests(IMP.test.TestCase):

    def test_cone_construction(self):
        """Check difference between two transformations"""
        a=IMP.algebra.Transformation3D(IMP.algebra.get_random_rotation_3d(),
                                       IMP.algebra.get_random_vector_in(IMP.algebra.get_unit_bounding_box_3d()))

        b=IMP.algebra.Transformation3D(IMP.algebra.get_random_rotation_3d(),
                                       IMP.algebra.get_random_vector_in(IMP.algebra.get_unit_bounding_box_3d()))

        d= a/b
        x= IMP.algebra.get_random_vector_in(IMP.algebra.get_unit_bounding_box_3d())
        ax= a.get_transformed(x)
        bx= b.get_transformed(x)
        dbx= d.get_transformed(bx)
        x.show()
        dbx.show()
        ax.show()
        self.assertLess((ax-dbx).get_squared_magnitude(), .1)


    def test_transformation_from_point_sets(self):
        """Check building a Transformation2D from point sets"""
        x1 = IMP.algebra.Vector2D(1,2)
        x2 = IMP.algebra.Vector2D(6,8)
        angle_applied = math.pi/4.
        shift_applied = IMP.algebra.Vector2D(-2,4)
        R = IMP.algebra.Rotation2D(angle_applied)
        y1 = R.get_rotated(x1)+shift_applied
        y2 = R.get_rotated(x2)+shift_applied
        set1 = [x1, x2]
        set2 = [y1, y2]
        T = IMP.algebra.get_transformation_aligning_pair(set1,set2)
        self.assertAlmostEqual(angle_applied,T.get_rotation().get_angle(),
                               delta=.01)
        self.assertAlmostEqual(shift_applied[0],T.get_translation()[0],
                               delta=.01)
        self.assertAlmostEqual(shift_applied[1],T.get_translation()[1],
                               delta=.01)



    def test_transformation3d_from2d(self):
        """Check that Z coordinate stays zero after transformation"""
        v = IMP.algebra.Vector3D(random.uniform(-100.,100.),
                                 random.uniform(-200.,200.),
                                 0.)
        angle_applied = random.uniform(-math.pi,math.pi)
        shift_applied = IMP.algebra.Vector2D(random.uniform(-10.,10.),
                                             random.uniform(-10,10))
        R = IMP.algebra.Rotation2D(angle_applied)
        t2d = IMP.algebra.Transformation2D(R,shift_applied)
        t3d=IMP.algebra.get_transformation_3d(t2d)
        v1 = t3d.get_transformed(v)
        print "V1: " + str(v1)
        self.assertAlmostEqual(v1[2],0.0, delta=.01)

    def test_transformation_between_two_reference_frames(self):
        """Check calculating a transformation between two reference frames"""
        #define a first reference frame
        u1 = IMP.algebra.Vector3D(random.uniform(-100.,100.),
                                 random.uniform(-200.,200.),
                                 0.).get_unit_vector()
        w1= IMP.algebra.get_vector_product(u1, IMP.algebra.get_basis_vector_3d(2))
        base1 = IMP.algebra.Vector3D(random.uniform(-50.,50.),
                                 random.uniform(-50.,50.),
                                 0.)
        #define a second reference frame
        u2 = IMP.algebra.Vector3D(1.,0.,0.)
        w2 = IMP.algebra.Vector3D(0.,1.,0.)
        base2 = IMP.algebra.Vector3D(0.,0.,0.)
        #define the first reference frame
        r1= IMP.algebra.ReferenceFrame3D(IMP.algebra.Transformation3D(IMP.algebra.get_rotation_from_x_y_axes(u1, w1), base1))
        #define the second reference frame
        r2= IMP.algebra.ReferenceFrame3D(IMP.algebra.Transformation3D(IMP.algebra.get_rotation_from_x_y_axes(u2, w2), base2))
        #calculate the transformation from the first to the second reference frame
        first_to_second=IMP.algebra.get_transformation_from_first_to_second(r2, r1)
        second_to_first=IMP.algebra.get_transformation_from_first_to_second(r1, r2)
        #preform test
        dist=IMP.algebra.get_distance(first_to_second.get_rotation(),second_to_first.get_inverse().get_rotation())
        self.assertAlmostEqual(dist,0.0, delta=0.001)
        dist=IMP.algebra.get_distance(first_to_second.get_translation(),second_to_first.get_inverse().get_translation())
        self.assertAlmostEqual(dist,0.0, delta=0.001)

    def test_build_from2D(self):
        """Check building a Transformation3D from Transformation2D"""
        angle_applied = math.pi/4.
        shift_applied = IMP.algebra.Vector2D(-2,4)
        r2d = IMP.algebra.Rotation2D(angle_applied)
        t2d = IMP.algebra.Transformation2D(r2d,shift_applied)
        t3d = IMP.algebra.get_transformation_3d(t2d)
        r3d = t3d.get_rotation()
        p2d = IMP.algebra.Vector2D(9,10.5)
        p3d = IMP.algebra.Vector3D(9,10.5,0.0)
        q3d = r3d.get_rotated(p3d) + t3d.get_translation()
        q2d = r2d.get_rotated(p2d) + shift_applied
        self.assertAlmostEqual(q2d[0],q3d[0], delta=.05)
        self.assertAlmostEqual(q2d[1],q3d[1], delta=.05)
        self.assertAlmostEqual(0.0   ,q3d[2], delta=.05)


if __name__ == '__main__':
    IMP.test.main()
