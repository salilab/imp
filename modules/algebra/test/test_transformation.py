import unittest
import IMP
import IMP.test
import IMP.algebra
import math,random

class ConeTests(IMP.test.TestCase):

    def test_cone_construction(self):
        """Check difference between two transformations"""
        a=IMP.algebra.Transformation3D(IMP.algebra.get_random_rotation_3d(),
                                       IMP.algebra.get_random_vector_in(IMP.algebra.get_unit_bounding_box_3d()))

        b=IMP.algebra.Transformation3D(IMP.algebra.get_random_rotation_3d(),
                                       IMP.algebra.get_random_vector_in(IMP.algebra.get_unit_bounding_box_3d()))

        d= a/b
        x= IMP.algebra.get_random_vector_in(IMP.algebra.get_unit_bounding_box_3d());
        ax= a.get_transformed(x)
        bx= b.get_transformed(x)
        dbx= d.get_transformed(bx)
        x.show()
        dbx.show()
        ax.show()
        self.assert_((ax-dbx).get_squared_magnitude() < .1)


class Transformation2DTests(IMP.test.TestCase):

    def test_transformation_from_point_sets(self):
        """Check building a Transformation2D from point sets"""
        x1 = IMP.algebra.Vector2D(1,2);
        x2 = IMP.algebra.Vector2D(6,8);
        angle_applied = math.pi/4.
        shift_applied = IMP.algebra.Vector2D(-2,4);
        R = IMP.algebra.Rotation2D(angle_applied);
        y1 = R.get_rotated(x1)+shift_applied;
        y2 = R.get_rotated(x2)+shift_applied;
        set1 = IMP.algebra.Vector2Ds(2);
        set2 = IMP.algebra.Vector2Ds(2);
        set1[0]=x1; set1[1]=x2;
        set2[0]=y1; set2[1]=y2;
        T = IMP.algebra.get_transformation_aligning_pair(set1,set2);
        self.assertInTolerance(angle_applied,T.get_rotation().get_angle(),.01)
        self.assertInTolerance(shift_applied[0],T.get_translation()[0],.01)
        self.assertInTolerance(shift_applied[1],T.get_translation()[1],.01)



    def test_transformation3d_from2d(self):
        """Check that Z coordinate stays zero after transformation"""
        v = IMP.algebra.Vector3D(random.uniform(-100.,100.),
                                 random.uniform(-200.,200.),
                                 0.)
        angle_applied = random.uniform(-math.pi,math.pi)
        shift_applied = IMP.algebra.Vector2D(random.uniform(-10.,10.),
                                             random.uniform(-10,10))
        R = IMP.algebra.Rotation2D(angle_applied);
        t2d = IMP.algebra.Transformation2D(R,shift_applied);
        t3d=IMP.algebra.get_transformation_3d(t2d)
        v1 = t3d.get_transformed(v)
        print "V1: " + str(v1)
        self.assertInTolerance(v1[2],0.0,.01)


class Transformation3DTests(IMP.test.TestCase):

    def test_build_from2D(self):
        """Check building a Transformation3D from Transformation2D"""
        angle_applied = math.pi/4.
        shift_applied = IMP.algebra.Vector2D(-2,4);
        r2d = IMP.algebra.Rotation2D(angle_applied);
        t2d = IMP.algebra.Transformation2D(r2d,shift_applied);
        t3d = IMP.algebra.get_transformation_3d(t2d)
        r3d = t3d.get_rotation()
        p2d = IMP.algebra.Vector2D(9,10.5)
        p3d = IMP.algebra.Vector3D(9,10.5,0.0)
        q3d = r3d.get_rotated(p3d) + t3d.get_translation();
        q2d = r2d.get_rotated(p2d) + shift_applied;
        self.assertInTolerance(q2d[0],q3d[0],.05)
        self.assertInTolerance(q2d[1],q3d[1],.05)
        self.assertInTolerance(0.0   ,q3d[2],.05)


if __name__ == '__main__':
    unittest.main()
