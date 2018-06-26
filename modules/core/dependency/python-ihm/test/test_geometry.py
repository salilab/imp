import utils
import os
import unittest
import sys

TOPDIR = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
utils.set_search_paths(TOPDIR)
import ihm.geometry

class Tests(unittest.TestCase):

    def test_center(self):
        """Test Center class"""
        c = ihm.geometry.Center(1,2,3)
        self.assertEqual(c.x, 1)
        self.assertEqual(c.y, 2)
        self.assertEqual(c.z, 3)

    def test_transformation(self):
        """Test Transformation class"""
        t = ihm.geometry.Transformation([[1,0,0],[0,1,0],[0,0,1]], [1,2,3])

    def test_identity_transformation(self):
        """Test identity transformation"""
        t = ihm.geometry.Transformation.identity()
        for i in range(3):
            self.assertAlmostEqual(t.tr_vector[i], 0., places=1)
            for j in range(3):
                self.assertAlmostEqual(t.rot_matrix[i][j],
                                       1. if i == j else 0., places=1)

    def test_geometric_object(self):
        """Test GeometricObject bass class"""
        g = ihm.geometry.GeometricObject(name='foo', description='bar')
        self.assertEqual(g.name, 'foo')
        self.assertEqual(g.description, 'bar')
        self.assertEqual(g.details, None)
        self.assertEqual(g.type, 'other')

    def test_sphere(self):
        """Test Sphere class"""
        c = ihm.geometry.Center(1,2,3)
        t = ihm.geometry.Transformation([[1,0,0],[0,1,0],[0,0,1]], [1,2,3])
        g = ihm.geometry.Sphere(center=c, transformation=t, radius=5)
        self.assertEqual(g.name, None)
        self.assertEqual(g.radius, 5)
        self.assertEqual(g.type, 'sphere')
        g = ihm.geometry.Sphere(center=c, radius=5)
        self.assertEqual(g.transformation, None)

    def test_torus(self):
        """Test Torus class"""
        c = ihm.geometry.Center(1,2,3)
        t = ihm.geometry.Transformation([[1,0,0],[0,1,0],[0,0,1]], [1,2,3])
        g = ihm.geometry.Torus(center=c, transformation=t, major_radius=5,
                               minor_radius=1)
        self.assertEqual(g.name, None)
        self.assertEqual(g.major_radius, 5)
        self.assertEqual(g.type, 'torus')
        g = ihm.geometry.Torus(center=c, major_radius=5,
                               minor_radius=1)
        self.assertEqual(g.transformation, None)

    def test_half_torus(self):
        """Test HalfTorus class"""
        c = ihm.geometry.Center(1,2,3)
        t = ihm.geometry.Transformation([[1,0,0],[0,1,0],[0,0,1]], [1,2,3])
        g = ihm.geometry.HalfTorus(center=c, transformation=t, major_radius=5,
                                   minor_radius=1, thickness=0.1, inner=True)
        self.assertEqual(g.name, None)
        self.assertEqual(g.major_radius, 5)
        self.assertEqual(g.inner, True)
        self.assertEqual(g.type, 'half-torus')
        g = ihm.geometry.HalfTorus(center=c, major_radius=5,
                                   minor_radius=1, thickness=0.1, inner=True)
        self.assertEqual(g.transformation, None)

    def test_axis(self):
        """Test Axis base class"""
        g = ihm.geometry.Axis(name='foo', description='bar')
        self.assertEqual(g.transformation, None)
        self.assertEqual(g.type, 'axis')

    def test_x_axis(self):
        """Test XAxis class"""
        g = ihm.geometry.XAxis(name='foo', description='bar')
        self.assertEqual(g.transformation, None)
        self.assertEqual(g.type, 'axis')
        self.assertEqual(g.axis_type, 'x-axis')

    def test_y_axis(self):
        """Test YAxis class"""
        g = ihm.geometry.YAxis(name='foo', description='bar')
        self.assertEqual(g.transformation, None)
        self.assertEqual(g.type, 'axis')
        self.assertEqual(g.axis_type, 'y-axis')

    def test_z_axis(self):
        """Test ZAxis class"""
        g = ihm.geometry.ZAxis(name='foo', description='bar')
        self.assertEqual(g.transformation, None)
        self.assertEqual(g.type, 'axis')
        self.assertEqual(g.axis_type, 'z-axis')

    def test_plane(self):
        """Test Plane base class"""
        g = ihm.geometry.Plane(name='foo', description='bar')
        self.assertEqual(g.transformation, None)
        self.assertEqual(g.type, 'plane')

    def test_xy_plane(self):
        """Test XYPlane base class"""
        g = ihm.geometry.XYPlane(name='foo', description='bar')
        self.assertEqual(g.transformation, None)
        self.assertEqual(g.type, 'plane')
        self.assertEqual(g.plane_type, 'xy-plane')

    def test_yz_plane(self):
        """Test YZPlane base class"""
        g = ihm.geometry.YZPlane(name='foo', description='bar')
        self.assertEqual(g.transformation, None)
        self.assertEqual(g.type, 'plane')
        self.assertEqual(g.plane_type, 'yz-plane')

    def test_xz_plane(self):
        """Test XZPlane base class"""
        g = ihm.geometry.XZPlane(name='foo', description='bar')
        self.assertEqual(g.transformation, None)
        self.assertEqual(g.type, 'plane')
        self.assertEqual(g.plane_type, 'xz-plane')


if __name__ == '__main__':
    unittest.main()
